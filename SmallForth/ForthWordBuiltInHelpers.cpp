#include <iostream>
#include "ForthDefs.h"
#include "ForthWord.h"
#include "DataStack.h"
#include "ReturnStack.h"
#include "ExecState.h"
#include "ForthDict.h"
#include "TypeSystem.h"
#include "CompileHelper.h"

// TODO Refactor this
//      This will be altered when changing the way binary operations function.  Simple types will still have code from here, but if type1 (further down the stack) is an object, it
//       will have a binary-operation WORD called on it
bool ForthWord::BuiltInHelper_BinaryOperation(ExecState* pExecState, BinaryOperationType opType) {
	StackElement* pElement2 = pExecState->pStack->Pull();
	StackElement* pElement1 = pExecState->pStack->Pull();

	if (pElement1 == nullptr || pElement2 == nullptr) {
		delete pElement1;
		delete pElement2;
		return pExecState->CreateStackUnderflowException();
	}

	StackElement* pNewElement = nullptr;

	ForthType type1 = pElement1->GetType();
	ForthType type2 = pElement2->GetType();

	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (pTS->TypeIsObject(type1)) {
		return BuiltInHelper_ObjectBinaryOperation(pExecState, opType, pElement1, pElement2);
	}
	else if (type1 == StackElement_Float || type2 == StackElement_Float) {
		if (!TypeSystem::IsNumeric(type1) || !TypeSystem::IsNumeric(type2)) {
			BuiltInHelper_DeleteOperands(pElement1, pElement2);
			return pExecState->CreateException("Binary operation not supported between those types");
		}
		double n1 = pElement1->GetFloat();
		double n2 = pElement2->GetFloat();

		switch (opType) {
		case BinaryOp_Add: pNewElement = new StackElement(n1 + n2); break;
		case BinaryOp_Subtract:pNewElement = new StackElement(n1 - n2); break;
		case BinaryOp_Multiply:pNewElement = new StackElement(n1 * n2); break;
		case BinaryOp_Divide:
			if (n2 == 0) {
				BuiltInHelper_DeleteOperands(pElement1, pElement2);
				return pExecState->CreateException("Divide by zero");
			}
			pNewElement = new StackElement(n1 / n2); break;
		case BinaryOp_Modulus:
			BuiltInHelper_DeleteOperands(pElement1, pElement2);
			return pExecState->CreateException("Modulus must have integers as operands");
		case BinaryOp_LessThan: pNewElement = new StackElement(n1 < n2); break;
		case BinaryOp_LessThanOrEquals: pNewElement = new StackElement(n1 <= n2); break;
		case BinaryOp_Equals: pNewElement = new StackElement(n1 == n2); break;
		case BinaryOp_NotEquals: pNewElement = new StackElement(n1 != n2); break;
		case BinaryOp_GreaterThanOrEquals: pNewElement = new StackElement(n1 >= n2); break;
		case BinaryOp_GreaterThan: pNewElement = new StackElement(n1 > n2); break;
		default:
			BuiltInHelper_DeleteOperands(pElement1, pElement2);
			return pExecState->CreateException("Unsupported operation on types");
		}
	}
	else if (type1 == StackElement_Int || type2 == StackElement_Int) {
		if (!TypeSystem::CanConvertToInt(type1) || !TypeSystem::CanConvertToInt(type2)) {
			BuiltInHelper_DeleteOperands(pElement1, pElement2);
			return pExecState->CreateException("Binary operation not supported between those types");
		}

		int64_t n1 = pElement1->GetInt();
		int64_t n2 = pElement2->GetInt();
		switch (opType) {
		case BinaryOp_Add: pNewElement = new StackElement(n1 + n2); break;
		case BinaryOp_Subtract:pNewElement = new StackElement(n1 - n2); break;
		case BinaryOp_Multiply:pNewElement = new StackElement(n1 * n2); break;
		case BinaryOp_Divide:
			if (n2 == 0) {
				BuiltInHelper_DeleteOperands(pElement1, pElement2);
				return pExecState->CreateException("Divide by zero");
			}
			pNewElement = new StackElement(n1 / n2); break;
		case BinaryOp_Modulus:
			if (type2 != StackElement_Int) {
				BuiltInHelper_DeleteOperands(pElement1, pElement2);
				return pExecState->CreateException("Modulus must have integer as divisor");
			}
			pNewElement = new StackElement(n1 % n2); break;
		case BinaryOp_LessThan: pNewElement = new StackElement(n1 < n2); break;
		case BinaryOp_LessThanOrEquals: pNewElement = new StackElement(n1 <= n2); break;
		case BinaryOp_Equals: pNewElement = new StackElement(n1 == n2); break;
		case BinaryOp_NotEquals: pNewElement = new StackElement(n1 != n2); break;
		case BinaryOp_GreaterThanOrEquals: pNewElement = new StackElement(n1 >= n2); break;
		case BinaryOp_GreaterThan: pNewElement = new StackElement(n1 > n2); break;
		default:
			BuiltInHelper_DeleteOperands(pElement1, pElement2);
			return pExecState->CreateException("Unsupported operation on types");
		}
	}
	else if (type1 == StackElement_Bool && type2 == StackElement_Bool) {
		bool b1 = pElement1->GetBool();
		bool b2 = pElement2->GetBool();
		switch (opType) {
		case BinaryOp_Equals: pNewElement = new StackElement(b1 == b2); break;
		case BinaryOp_NotEquals: pNewElement = new StackElement(b1 != b2); break;
		default:
			BuiltInHelper_DeleteOperands(pElement1, pElement2);
			return pExecState->CreateException("Unsupported operation on booleans");
		}
	}
	else if (type1 == StackElement_Type && type2 == StackElement_Type) {
		ForthType t1 = pElement1->GetValueType();
		ForthType t2 = pElement2->GetValueType();
		switch (opType) {
		case BinaryOp_Equals: pNewElement = new StackElement(t1 == t2); break;
		case BinaryOp_NotEquals: pNewElement = new StackElement(t1 != t2); break;
		default:
			BuiltInHelper_DeleteOperands(pElement1, pElement2);
			return pExecState->CreateException("Unsupported operation on types");
		}
	}
	else if (type1 == StackElement_Char && type2 == StackElement_Char) {
		char c1 = pElement1->GetChar();
		char c2 = pElement2->GetChar();
		switch (opType) {
		case BinaryOp_LessThan: pNewElement = new StackElement(c1 < c2); break;
		case BinaryOp_LessThanOrEquals: pNewElement = new StackElement(c1 <= c2); break;
		case BinaryOp_Equals: pNewElement = new StackElement(c1 == c2); break;
		case BinaryOp_NotEquals: pNewElement = new StackElement(c1 != c2); break;
		case BinaryOp_GreaterThanOrEquals: pNewElement = new StackElement(c1 >= c2); break;
		case BinaryOp_GreaterThan: pNewElement = new StackElement(c1 > c2); break;
		default:
			BuiltInHelper_DeleteOperands(pElement1, pElement2);
			return pExecState->CreateException("Unsupported operation on chars");
		}
	}
	BuiltInHelper_DeleteOperands(pElement1, pElement2);

	if (pNewElement == nullptr) {
		return pExecState->CreateException("Could not perform operation on the types on the stack");
	}
	pExecState->pStack->Push(pNewElement);
	return true;
}

bool ForthWord::BuiltInHelper_ObjectBinaryOperation(ExecState* pExecState, BinaryOperationType opType, StackElement* pElement1, StackElement* pElement2) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	ForthType type1 = pElement1->GetType();

	XT binaryOpsHandler = pTS->GetBinaryOpsHandlerForType(type1);
	if (binaryOpsHandler == nullptr) {
		BuiltInHelper_DeleteOperands(pElement1, pElement2);
		return pExecState->CreateException("Type does not support binary operations");
	}
	// Push element1, element2, operator
	if (!pExecState->pStack->Push(pElement1) || 
		!pExecState->pStack->Push(pElement2) ||
		!pExecState->pStack->Push(new StackElement(opType))) {
		BuiltInHelper_DeleteOperands(pElement1, pElement2);
		return pExecState->CreateStackOverflowException();
	}
	pElement1 = nullptr;
	pElement2 = nullptr;
	if (!binaryOpsHandler(pExecState)) {
		BuiltInHelper_DeleteOperands(pElement1, pElement2);
		return pExecState->CreateException("Could not perform binary operation on those types");
	}
	BuiltInHelper_DeleteOperands(pElement1, pElement2);
	return true;
}

void ForthWord::BuiltInHelper_DeleteOperands(StackElement*& pElement1, StackElement*& pElement2) {
	delete pElement1;
	pElement1 = nullptr;
	delete pElement2;
	pElement2 = nullptr;
}

void ForthWord::BuiltInHelper_DeleteStackElement(StackElement*& pElement) {
	delete pElement;
	pElement = nullptr;

}

bool ForthWord::BuiltInHelper_GetOneTempStackElement(ExecState* pExecState, StackElement*& pElement1) {
	pElement1 = pExecState->pTempStack->Pull();

	if (pElement1 == nullptr) {
		return pExecState->CreateTempStackUnderflowException();
	}
	return true;
}

bool ForthWord::BuiltInHelper_GetOneStackElement(ExecState* pExecState, StackElement*& pElement1) {
	pElement1 = pExecState->pStack->Pull();

	if (pElement1 == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	return true;
}

bool ForthWord::BuiltInHelper_GetTwoStackElements(ExecState* pExecState, StackElement*& pElement1, StackElement*& pElement2) {
	pElement2 = pExecState->pStack->Pull();
	pElement1 = pExecState->pStack->Pull();

	if (pElement1 == nullptr || pElement2 == nullptr) {
		delete pElement1;
		delete pElement2;
		return pExecState->CreateStackUnderflowException();
	}
	return true;
}

bool ForthWord::BuiltInHelper_GetThreeStackElements(ExecState* pExecState, StackElement*& pElement1, StackElement*& pElement2, StackElement*& pElement3) {
	pElement3 = pExecState->pStack->Pull();
	pElement2 = pExecState->pStack->Pull();
	pElement1 = pExecState->pStack->Pull();

	if (pElement1 == nullptr || pElement2 == nullptr || pElement3 == nullptr) {
		delete pElement1;
		delete pElement2;
		delete pElement3;
		return pExecState->CreateStackUnderflowException();
	}
	return true;
}

// Relies on return stack TOS have the index into the word being defines literal element
//  (The actual pushlit that preceeds the actual value - it will have 1 added to it)
bool ForthWord::BuiltInHelper_UpdateForwardJump(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot update forward jump when not compiling");
	}
	if (!ForthWord::BuiltIn_PushReturnStackToDataStack(pExecState)) {
		return false;
	}
	if (!pExecState->ExecuteWordDirectly("2+")) {
		return false;
	}

	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}

	// poke ( addr:value -- ) *addr=value
	if (!ForthWord::BuiltIn_PokeIntegerInWord(pExecState)) {
		return false;
	}
	return true;
}

bool ForthWord::BuiltInHelper_FetchLiteralWithOffset(ExecState* pExecState, int offset) {
	WordBodyElement* pWBE_Type = pExecState->GetWordAtOffsetFromCurrentBody(offset + 1);
	if (pWBE_Type == nullptr) {
		return pExecState->CreateException("Cannot fetch literal as cannot find a literal type in word body");
	}
	WordBodyElement* pWBE_Word = pExecState->GetWordAtOffsetFromCurrentBody(offset + 2);
	if (pWBE_Word == nullptr) {
		return pExecState->CreateException("Cannot fetch literal as cannot find a literal in word body");
	}
	ValueType toPush = pWBE_Word->wordElement_type;
	if (!pExecState->pStack->Push(new StackElement(pWBE_Type->forthType, pWBE_Word))) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltInHelper_CompileTOSLiteral(ExecState* pExecState, bool includePushWord) {
	StackElement* pTopElement = pExecState->pStack->Pull();
	if (pTopElement == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}

	bool success = true;
	if (includePushWord && !pExecState->pCompiler->CompileWord(pExecState, "pushliteral")) {
		success = false;
	}
	else {
		pExecState->pCompiler->CompileTypeIntoWordBeingCreated(pExecState, pTopElement->GetType());
		pExecState->pCompiler->CompileWBEIntoWordBeingCreated(pExecState, pTopElement->GetValueAsWordBodyElement());
	}

	delete pTopElement;
	pTopElement = nullptr;
	return success;
}