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

using namespace std;

bool ForthWord::BuiltIn_StringLiteral(ExecState* pExecState) {
	pExecState->insideStringLiteral = true;
	// TODO Change this to get individual characters, so can absorb multiple spaces
	//      This can be done when input processor is changed to absorb characters through a KEY word
	InputWord inputWord = pExecState->pInputProcessor->GetNextWord(pExecState);
	string partOfWord = inputWord.word;

	string literal;
	while (partOfWord.compare("\"") != 0) {
		if (literal.length() > 0) {
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

	if (pExecState->compileState) {
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

bool ForthWord::BuiltIn_CallObject(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	StackElement* pElementAddress;
	StackElement* pElementFunctionIndex;
	if (!BuiltInHelper_GetTwoStackElements(pExecState, pElementFunctionIndex, pElementAddress)) {
		return pExecState->CreateStackUnderflowException();
	}

	ForthType addressType = pElementAddress->GetType();
	ForthType indexType = pElementFunctionIndex->GetType();
	if (!pTS->TypeIsObject(addressType) || indexType!=StackElement_Int) {
		BuiltInHelper_DeleteOperands(pElementFunctionIndex, pElementAddress);
		return pExecState->CreateException("Invoking call must have ( index objaddr -- )");
	}
	RefCountedObject* pObj = pElementAddress->GetObject();
	int n = (int)pElementFunctionIndex->GetInt();
	bool success = pObj->InvokeFunctionIndex(pExecState, (ObjectFunction)n);

	BuiltInHelper_DeleteOperands(pElementFunctionIndex, pElementAddress);
	return success;
}

bool ForthWord::BuiltIn_Construct(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	StackElement* pElementType;
	bool incorrectType;
	tie(incorrectType, pElementType) = pExecState->pStack->PullType(StackElement_Type);
	if (incorrectType) {
		return pExecState->CreateException("Must have a type to construct with");
	} else if (pElementType == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}

	ForthType type = pElementType->GetValueType();
	BuiltInHelper_DeleteStackElement(pElementType);

	if (pTS->GetIndirectionLevel(type) > 0) {
		return pExecState->CreateException("Currently constructing pointers to objects is not supported");
	}

	return pTS->Construct(pExecState, type);
}

bool ForthWord::BuiltIn_DefineObject(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	StackElement* pElementObjectName;
	StackElement* pElementStateCount;
	if (!BuiltInHelper_GetTwoStackElements(pExecState, pElementStateCount, pElementObjectName)) {
		return false;
	}
	if (pElementObjectName->GetType() != ObjectType_String ||
		pElementStateCount->GetType() != StackElement_Int) {
		BuiltInHelper_DeleteOperands(pElementStateCount, pElementObjectName);
		return pExecState->CreateException("Define objects must be called with ( [state] n $ -- ");
	}
	ForthString* pString = (ForthString* )pElementObjectName->GetObject();
	string newTypeName = pString->GetContainedString();
	int stateCount = (int)pElementStateCount->GetInt();

	BuiltInHelper_DeleteOperands(pElementStateCount, pElementObjectName);

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

	stringstream ss;
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