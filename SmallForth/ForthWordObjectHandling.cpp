#include <iostream>
#include <sstream>

#include "ForthDefs.h"
#include "ForthWord.h"
#include "DataStack.h"
#include "ReturnStack.h"
#include "ExecState.h"
#include "ForthDict.h"
#include "InputProcessor.h"
#include "RefCountedObject.h"
#include "ForthString.h"
#include "TypeSystem.h"
#include "UserDefinedObject.h"
#include "CompileHelper.h"
#include "PreBuiltWords.h"

bool PreBuiltWords::BuiltIn_StringLiteral(ExecState* pExecState) {
	pExecState->insideStringLiteral = true;
	InputWord inputWord = pExecState->pInputProcessor->GetNextWord(pExecState);
	std::string partOfWord = inputWord.word;

	std::string literal;
	bool firstPart = true;
	while (partOfWord.compare("\"") != 0) {
		if (firstPart) {
			// literals start with "_ and that first space (represented by underscore) should not be added, so it is -1 here on the loop comparison
			firstPart = false;
			for (int i = 0; i < pExecState->delimitersAfterCurrentWord - 1; ++i) {
				literal += " ";
			}
		}
		else {
			// In previous loop, added one less space than the delimiter count specified, in case it was the last iteration of the loop
			//  String literals end _" and that final space should not be included.  This line is adding that space, as it wasn't the final iteration
			literal += " ";
		}
		literal += partOfWord;
		for (int i = 0; i < inputWord.postDelimiterCount - 1; ++i) {
			literal += " ";
		}
		inputWord = pExecState->pInputProcessor->GetNextWord(pExecState);
		partOfWord = inputWord.word;
	}
	ForthString* pForthString = new ForthString(literal);

	int64_t nCompileState = pExecState->GetIntTLSVariable(ExecState::c_compileStateIndex);

	if (nCompileState == 1) {
		pExecState->pCompiler->CompilePushAndLiteralIntoWordBeingCreated(pExecState, pForthString);
	}
	else {
		StackElement* pStackElement = new StackElement(pForthString);
		pExecState->pStack->Push(pStackElement);
	}
	pForthString->DecReference();
	pExecState->insideStringLiteral = false;
	return true;
}

bool PreBuiltWords::BuiltIn_CallObject(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	StackElement* pElementAddress;
	StackElement* pElementFunctionIndex;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pElementFunctionIndex, pElementAddress)) {
		return pExecState->CreateStackUnderflowException();
	}

	ForthType addressType = pElementAddress->GetType();
	ForthType indexType = pElementFunctionIndex->GetType();
	if (!pTS->TypeIsObject(addressType) || indexType != StackElement_Int) {
		ForthWord::BuiltInHelper_DeleteOperands(pElementFunctionIndex, pElementAddress);
		return pExecState->CreateException("Invoking call must have ( index objaddr -- )");
	}
	RefCountedObject* pObj = pElementAddress->GetObject();
	int n = (int)pElementFunctionIndex->GetInt();
	bool success = pObj->InvokeFunctionIndex(pExecState, (ObjectFunction)n);

	ForthWord::BuiltInHelper_DeleteOperands(pElementFunctionIndex, pElementAddress);
	return success;
}

bool PreBuiltWords::BuiltIn_Construct(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	if (pExecState->pStack->Count() == 0) {
		return pExecState->CreateStackUnderflowException("Whilst constructing object");
	}
	if (pExecState->pStack->GetTOSType() != StackElement_Type) {
		return pExecState->CreateException("Must have a type to construct with");
	}
	ForthType type = pExecState->pStack->PullAsType();

	if (pTS->GetIndirectionLevel(type) > 0) {
		return pExecState->CreateException("Currently constructing pointers to objects is not supported");
	}

	return pTS->Construct(pExecState, type);
}

bool PreBuiltWords::BuiltIn_DefineObject(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	if (pExecState->pStack->Count() < 2) {
		return pExecState->CreateStackUnderflowException("whilst defining an object - need ( n $ -- )");
	}

	if (!pExecState->pStack->TOSIsType((ElementType)ObjectType_String)) {
		return pExecState->CreateException("Need an object name when defining an object");
	}

	bool successGettingString;
	std::string newTypeName;
	std::tie(successGettingString, newTypeName) = pExecState->pStack->PullAsString();
	if (!successGettingString) {
		return pExecState->CreateException(newTypeName.c_str());
	}
	if (!pExecState->pStack->TOSIsType(StackElement_Int)) {
		return pExecState->CreateException("Need an element count when defining an object");
	}
	int stateCount = (int)pExecState->pStack->PullAsInt();

	if (pTS->TypeExists(newTypeName)) {
		return pExecState->CreateException("Object type already exists");
	}
	if (pExecState->pDict->FindWord(newTypeName + "_") != nullptr) {
		return pExecState->CreateException("Cannot create <objectname>_type word for this object type, as it already exists");
	}

	// Construct a UserDefinedObject with name and state count
	// Request UserDefinedObject pulls elements from the stack, to construct its state
	// TOS is last state element
	// Store stack elements as defaults
	//  Any elements that are ref-counted objects, should have their reference counts incremented
	// Register new type with typesystem, as user object
	//  Type system will have to store an object-specific construct method
	// Deleting any instance of a user-object should decrement the ref-count of any state objects
	//  Garbage collection.  Mark each object, then iterate over state objects, calling dec ref count on objects.
	//   If an state object is already marked, decrement ref count but do not iterate over its state objects
	//   After iterating over state objects, unmark the object and return

	// Register type word for new object type (vector3_type, string_type etc)
	// 

	UserDefinedObject* pNewObjectType = new UserDefinedObject(newTypeName, stateCount, nullptr);
	if (!pNewObjectType->DefineObjectFromStack(pExecState)) {
		delete pNewObjectType;
		pNewObjectType = nullptr;
		return false;
	}
	ForthType newType = pTS->RegisterUserObjectType(pExecState, newTypeName, pNewObjectType);
	if (newType == TypeSystem::typeIdInvalid) {
		delete pNewObjectType;
		pNewObjectType = nullptr;
		return false;
	}

	std::stringstream ss;
	ss << ": " << newTypeName << "_type " << newType << " typefromint ; ";
	pExecState->pInputProcessor->SetInputString(ss.str());
	// TODO Capture any exception thrown by interpreter rather than have it display via interpret()
	if (!pExecState->pInputProcessor->Interpret(pExecState)) {
		// TODO Tell type system that this type ID is invalid
		//      Or
		//      Tell use that the type word hasn't been created
		//      Or
		//      Create a similar name, and store the name somewhere
		delete pNewObjectType;
		pNewObjectType = nullptr;
		return pExecState->CreateException("Could not create <objectname>_type word for this object type");
	}

	return true;
}