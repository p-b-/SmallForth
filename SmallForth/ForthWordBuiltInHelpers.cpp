#include <iostream>
#include "ForthDefs.h"
#include "ForthWord.h"
#include "DataStack.h"
#include "ReturnStack.h"
#include "ExecState.h"
#include "ForthDict.h"
#include "TypeSystem.h"
#include "CompileHelper.h"
#include "PreBuiltWords.h"

// TODO Refactor this
//      This will be altered when changing the way binary operations function.  Simple types will still have code from here, but if type1 (further down the stack) is an object, it
//       will have a binary-operation WORD called on it
bool ForthWord::BuiltInHelper_BinaryOperation(ExecState* pExecState, BinaryOperationType opType) {
	if (pExecState->pStack->Count() < 2) {
		return pExecState->CreateStackUnderflowException("need two operands for a binary operation");
	}
	StackElement* pNewElement = nullptr;

	ForthType type2 = pExecState->pStack->GetTOSType();
	pExecState->pStack->SwapTOS();
	ForthType type1 = pExecState->pStack->GetTOSType();
	pExecState->pStack->SwapTOS();

	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (pTS->TypeIsObject(type1)) {
		StackElement element2 = pExecState->pStack->PullAsRef();
		StackElement element1 = pExecState->pStack->PullAsRef();
		return BuiltInHelper_ObjectBinaryOperation(pExecState, opType, element1, element2);
	}
	else if (type1 == StackElement_Float || type2 == StackElement_Float) {
		if (!TypeSystem::IsNumeric(type1) || !TypeSystem::IsNumeric(type2)) {
			return pExecState->CreateException("Binary operation not supported between those types");
		}
		double n2 = pExecState->pStack->PullAsFloat();
		double n1 = pExecState->pStack->PullAsFloat();

		switch (opType) {
		case BinaryOp_Add: pNewElement = new StackElement(n1 + n2); break;
		case BinaryOp_Subtract:pNewElement = new StackElement(n1 - n2); break;
		case BinaryOp_Multiply:pNewElement = new StackElement(n1 * n2); break;
		case BinaryOp_Divide:
			if (n2 == 0) {
				return pExecState->CreateException("Divide by zero");
			}
			pNewElement = new StackElement(n1 / n2); break;
		case BinaryOp_Modulus:
			return pExecState->CreateException("Modulus must have integers as operands");
		case BinaryOp_LessThan: pNewElement = new StackElement(n1 < n2); break;
		case BinaryOp_LessThanOrEquals: pNewElement = new StackElement(n1 <= n2); break;
		case BinaryOp_Equals: pNewElement = new StackElement(n1 == n2); break;
		case BinaryOp_NotEquals: pNewElement = new StackElement(n1 != n2); break;
		case BinaryOp_GreaterThanOrEquals: pNewElement = new StackElement(n1 >= n2); break;
		case BinaryOp_GreaterThan: pNewElement = new StackElement(n1 > n2); break;
		default:
			return pExecState->CreateException("Unsupported operation on types");
		}
	}
	else if (pTS->IsPter(type1) && type2 == StackElement_Int) {
		int64_t n2 = pExecState->pStack->PullAsInt();
		void* pVoid = pExecState->pStack->PullAsVoidPter();

		// Cannot do pointer arithmetic on a void*
		int64_t* pObject1 = (int64_t*)pVoid;
		switch (opType) {
		case BinaryOp_Add: pNewElement = new StackElement(type1, pObject1 + n2); break;
		case BinaryOp_Subtract: pNewElement = new StackElement(type1, pObject1 - n2); break;
		default:
			return pExecState->CreateException("Unsupported operation on pter");
		}
	}
	else if (type1 == StackElement_Int || type2 == StackElement_Int) {
		if (!TypeSystem::CanConvertToInt(type1) || !TypeSystem::CanConvertToInt(type2)) {
			return pExecState->CreateException("Binary operation not supported between those types");
		}
		int64_t n2 = pExecState->pStack->PullAsInt();
		int64_t n1 = pExecState->pStack->PullAsInt();

		switch (opType) {
		case BinaryOp_Add: pNewElement = new StackElement(n1 + n2); break;
		case BinaryOp_Subtract:pNewElement = new StackElement(n1 - n2); break;
		case BinaryOp_Multiply:pNewElement = new StackElement(n1 * n2); break;
		case BinaryOp_Divide:
			if (n2 == 0) {
				return pExecState->CreateException("Divide by zero");
			}
			pNewElement = new StackElement(n1 / n2); break;
		case BinaryOp_Modulus:
			if (type2 != StackElement_Int) {
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
			return pExecState->CreateException("Unsupported operation on types");
		}
	}
	else if (type1 == StackElement_Bool && type2 == StackElement_Bool) {
		bool b2 = pExecState->pStack->PullAsBool();
		bool b1 = pExecState->pStack->PullAsBool();
		switch (opType) {
		case BinaryOp_Equals: pNewElement = new StackElement(b1 == b2); break;
		case BinaryOp_NotEquals: pNewElement = new StackElement(b1 != b2); break;
		default:
			return pExecState->CreateException("Unsupported operation on booleans");
		}
	}
	else if (type1 == StackElement_Type && type2 == StackElement_Type) {
		ForthType t2 = pExecState->pStack->PullAsType();
		ForthType t1 = pExecState->pStack->PullAsType();
		switch (opType) {
		case BinaryOp_Equals: pNewElement = new StackElement(t1 == t2); break;
		case BinaryOp_NotEquals: pNewElement = new StackElement(t1 != t2); break;
		default:
			return pExecState->CreateException("Unsupported operation on types");
		}
	}
	else if (type1 == StackElement_Char && type2 == StackElement_Char) {
		char c2 = pExecState->pStack->PullAsChar();
		char c1 = pExecState->pStack->PullAsChar();
		switch (opType) {
		case BinaryOp_LessThan: pNewElement = new StackElement(c1 < c2); break;
		case BinaryOp_LessThanOrEquals: pNewElement = new StackElement(c1 <= c2); break;
		case BinaryOp_Equals: pNewElement = new StackElement(c1 == c2); break;
		case BinaryOp_NotEquals: pNewElement = new StackElement(c1 != c2); break;
		case BinaryOp_GreaterThanOrEquals: pNewElement = new StackElement(c1 >= c2); break;
		case BinaryOp_GreaterThan: pNewElement = new StackElement(c1 > c2); break;
		default:
			return pExecState->CreateException("Unsupported operation on chars");
		}
	}
	else {
		return pExecState->CreateException("Binary operations unsupported on these types");
	}

	if (pNewElement == nullptr) {
		return pExecState->CreateException("Could not perform operation on the types on the stack");
	}
	pExecState->pStack->Push(pNewElement);
	return true;
}

bool ForthWord::BuiltInHelper_ObjectBinaryOperation(ExecState* pExecState, BinaryOperationType opType, const StackElement& element1, const StackElement& element2) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	ForthType type1 = element1.GetType();

	XT binaryOpsHandler = pTS->GetBinaryOpsHandlerForType(type1);
	if (binaryOpsHandler == nullptr) {
		return pExecState->CreateException("Type does not support binary operations");
	}
	// Push element1, element2, operator
	if (!pExecState->pStack->Push(element1) ||
		!pExecState->pStack->Push(element2) ||
		!pExecState->pStack->Push(opType)) {
		return pExecState->CreateStackOverflowException();
	}
	if (!binaryOpsHandler(pExecState)) {
		return pExecState->CreateException("Could not perform binary operation on those types");
	}
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
	int64_t nCompileState = pExecState->GetIntTLSVariable(ExecState::c_compileStateIndex);

	if (nCompileState == 0) {
		return pExecState->CreateException("Cannot update forward jump when not compiling");
	}
	if (!PreBuiltWords::BuiltIn_PushReturnStackToDataStack(pExecState)) {
		return false;
	}

	if (!pExecState->pStack->Push((int64_t)2)) return pExecState->CreateStackOverflowException("whilst updating forward jump");
	if (!pExecState->ExecuteWordDirectly("+")) return false;

	if (!PreBuiltWords::BuiltIn_Here(pExecState)) {
		return false;
	}

	// poke ( addr:value -- ) *addr=value
	if (!PreBuiltWords::BuiltIn_PokeIntegerInWord(pExecState)) {
		return false;
	}
	return true;
}

bool ForthWord::BuiltInHelper_FetchLiteralWithOffset(ExecState* pExecState, int offset) {
	WordBodyElement* pWBE_Type = pExecState->GetWordAtOffsetFromCurrentBody(offset + 1);
	if (pWBE_Type == nullptr) {
		return pExecState->CreateException("Cannot fetch literal as cannot find a literal type in word body");
	}
	WordBodyElement** ppWBE_Word = pExecState->GetWordPterAtOffsetFromCurrentBody(offset + 2);
	if (ppWBE_Word == nullptr) {
		return pExecState->CreateException("Cannot fetch literal as cannot find a literal in word body");
	}
//	ValueType toPush = (*ppWBE_Word)->wordElement_type;
	if (!pExecState->pStack->Push(new StackElement(pWBE_Type->forthType, ppWBE_Word))) {
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