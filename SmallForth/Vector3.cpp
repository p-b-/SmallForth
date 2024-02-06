#include <math.h>
#include <sstream>
using namespace std;
#include "Vector3.h"
#include "ExecState.h"
#include "DataStack.h"
#include "ForthString.h"

Vector3::Vector3(double x, double y, double z) :
	RefCountedObject(nullptr) {
	this->objectType = ObjectType_Vector3;
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3::~Vector3() { 
}

string Vector3::GetObjectType() { 
	return "vector3";
}

bool Vector3::ToString(ExecState* pExecState) const {
	stringstream str;
	str << "(" << x << ", " << y << ", " << z << ")";
	if (!pExecState->pStack->Push(str.str())) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool Vector3::Construct(ExecState* pExecState) {
	double x;
	double y;
	double z;
	if (!GetXYZFromStack(pExecState, x, y, z)) {
		return false;
	}
	Vector3* v = new Vector3(x, y, z);
	StackElement* pNewElement = new StackElement(v);
	return pExecState->pStack->Push(pNewElement);
}

bool Vector3::BinaryOps(ExecState* pExecState) {
	bool incorrectType;
	StackElement* pElementOperator;
	tie(incorrectType, pElementOperator) = pExecState->pStack->PullType(StackElement_BinaryOpsType);
	if (incorrectType) {
		return pExecState->CreateException("Binary operator handler must be supplied with a binary operator type");
	}
	else if (pElementOperator == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}

	BinaryOperationType opType = pElementOperator->GetBinaryOpsType();
	delete pElementOperator;
	pElementOperator = nullptr;
	StackElement* pElementOperand2;
	StackElement* pElementOperand1;

	tie(pElementOperand1, pElementOperand2) = pExecState->pStack->PullTwo();
	if (pElementOperand1 == nullptr) {
		// If one is null, they will both be null
		return pExecState->CreateStackUnderflowException();
	}

	Vector3* pVector = (Vector3*)(pElementOperand1->GetObject());

	ForthType operandType = pElementOperand2->GetType();
	bool numericOperand = operandType == ValueType_Float || operandType == ValueType_Int;
	bool vector3Operand = operandType == ObjectType_Vector3;

	StackElement* pNewElement = nullptr;

	bool success = true;
	switch (opType) {
	case BinaryOp_Multiply:
		if (numericOperand) {
			pNewElement = ScalarMultiply(operandType, pVector, pElementOperand2);
		}
		else {
			success = pExecState->CreateException("Cannot perform multiply operation between a vector3 and this type");
		}
		break;
	case BinaryOp_Divide:
		if (numericOperand) {
			// Can create divide y zero exceptions
			pNewElement = ScalarDivide(pExecState, operandType, pVector, pElementOperand2);
			if (pNewElement == nullptr) {
				if (pExecState->exceptionThrown) {
					success = false;
				}
			}
		}
		else {
			success = pExecState->CreateException("Cannot perform divide operation between a vector3 and this type");
		}
		break;
	case BinaryOp_Add:
		if (vector3Operand) {
			pNewElement = Add(operandType, pVector, pElementOperand2);
		}
		else {
			success = pExecState->CreateException("Cannot perform add operation between a vector3 and this type");
		}
		break;
	case BinaryOp_Subtract:
		if (vector3Operand) {
			pNewElement = Subtract(operandType, pVector, pElementOperand2);
		}
		else {
			success = pExecState->CreateException("Cannot perform subtraction operation between a vector3 and this type");
		}
		break;
	case BinaryOp_Equals:
		if (vector3Operand) {
			pNewElement = VectorsEqualOrNotEquals(true, operandType, pVector, pElementOperand2);
		}
		else {
			success = pExecState->CreateException("Cannot perform equalality operation between a vector3 and this type");
		}
		break;
	case BinaryOp_NotEquals:
		if (vector3Operand) {
			pNewElement = VectorsEqualOrNotEquals(false, operandType, pVector, pElementOperand2);
		}
		else {
			success = pExecState->CreateException("Cannot perform equality operation between a vector3 and this type");
		}
		break;
	default:
		success = pExecState->CreateException("Cannot perform binary operation on vector3 and this type");
	}
	// Cannot delete earlier, as it may delete the vectors (ref-counted objects) that this method operates on
	delete pElementOperand1;
	pElementOperand1 = nullptr;
	delete pElementOperand2;
	pElementOperand2 = nullptr;

	if (pNewElement != nullptr) {
		if (!pExecState->pStack->Push(pNewElement)) {
			return pExecState->CreateStackOverflowException();
		}
	}
	
	return success;
}

bool Vector3::GetXYZFromStack(ExecState* pExecState, double& x, double& y, double& z) {
	StackElement* pElementZ = pExecState->pStack->Pull();
	if (pElementZ == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	StackElement* pElementY = pExecState->pStack->Pull();
	if (pElementY == nullptr) {
		delete pElementZ;
		pElementZ = nullptr;
		return pExecState->CreateStackUnderflowException();
	}
	StackElement* pElementX = pExecState->pStack->Pull();
	if (pElementX == nullptr) {
		delete pElementZ;
		pElementZ = nullptr;
		delete pElementY;
		pElementY = nullptr;
		return pExecState->CreateStackUnderflowException();
	}

	ForthType zType = pElementZ->GetType();
	ForthType yType = pElementY->GetType();
	ForthType xType = pElementX->GetType();
	bool success = true;
	if ( (zType != ValueType_Int && zType != ValueType_Float) || 
		(yType != ValueType_Int && yType != ValueType_Float) ||
		(xType != ValueType_Int && xType != ValueType_Float))
	{
		success= pExecState->CreateException("Construction of vector3 requires ( n/f n/f n/f -- o ) ");
	}
	else {
		x = pElementX->GetFloat();
		y = pElementY->GetFloat();
		z = pElementZ->GetFloat();
	}
	delete pElementZ;
	pElementZ = nullptr;
	delete pElementY;
	pElementY = nullptr;
	delete pElementX;
	pElementX = nullptr;

	return success;
}

StackElement* Vector3::ScalarMultiply(ForthType operandType, Vector3* pOperand1, StackElement* pElementOperand) {
	Vector3* pNewVector = nullptr;

	if (operandType == ValueType_Float) {
		double operand = pElementOperand->GetFloat();
		pNewVector = new Vector3(pOperand1->x * operand, pOperand1->y * operand, pOperand1->z * operand);
	}
	else if (operandType == ValueType_Int) {
		int64_t operand = pElementOperand->GetInt();
		pNewVector = new Vector3(pOperand1->x * operand, pOperand1->y * operand, pOperand1->z * operand);
	}
	if (pNewVector != nullptr) {
		return new StackElement(pNewVector);
	}
	return nullptr;
}

StackElement* Vector3::ScalarDivide(ExecState* pExecState, ForthType operandType, Vector3* pOperand1, StackElement* pElementOperand) { 
	Vector3* pNewVector = nullptr;

	if (operandType == ValueType_Float) {
		double operand = pElementOperand->GetFloat();
		if (operand==0.0f) {
			pExecState->CreateException("Divide by zero");
			return nullptr;
		}
		pNewVector = new Vector3(pOperand1->x / operand, pOperand1->y / operand, pOperand1->z / operand);
	}
	else if (operandType == ValueType_Int) {
		int64_t operand = pElementOperand->GetInt();
		if (operand == 0) {
			pExecState->CreateException("Divide by zero");
			return nullptr;
		}
		pNewVector = new Vector3(pOperand1->x / operand, pOperand1->y / operand, pOperand1->z / operand);
	}
	if (pNewVector != nullptr) {
		return new StackElement(pNewVector);
	}
	return nullptr;
}

StackElement* Vector3::Add(ForthType operandType, Vector3* pOperand1, StackElement* pElementOperand) { 
	Vector3* pNewVector = nullptr;

	if (operandType == ObjectType_Vector3) {
		Vector3* operand = (Vector3* )(pElementOperand->GetObject());
		pNewVector = new Vector3(pOperand1->x + operand->x, pOperand1->y + operand->y, pOperand1->z  + operand->z);
	}

	if (pNewVector != nullptr) {
		return new StackElement(pNewVector);
	}
	return nullptr;
}

StackElement* Vector3::Subtract(ForthType operandType, Vector3* pOperand1, StackElement* pElementOperand) { 
	Vector3* pNewVector = nullptr;

	if (operandType == ObjectType_Vector3) {
		Vector3* operand = (Vector3*)(pElementOperand->GetObject());
		pNewVector = new Vector3(pOperand1->x - operand->x, pOperand1->y - operand->y, pOperand1->z - operand->z);
	}

	if (pNewVector != nullptr) {
		return new StackElement(pNewVector);
	}
	return nullptr;
}

StackElement* Vector3::VectorsEqualOrNotEquals(bool equals, ForthType operandType, Vector3* pOperand1, StackElement* pElementOperand) {
	Vector3* pNewVector = nullptr;

	if (operandType == ObjectType_Vector3) {
		Vector3* operand = (Vector3*)(pElementOperand->GetObject());
		bool result = !equals;
		if (pOperand1->x == operand->x &&
			pOperand1->y == operand->y &&
			pOperand1->z == operand->z) {
			result = equals;
		}
		return new StackElement(result);
	}
	else {
		return nullptr;
	}

	if (pNewVector != nullptr) {
		return new StackElement(pNewVector);
	}
	return nullptr;
}

bool Vector3::InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke) { 
	switch (functionToInvoke) {
	case Function_GetSize: return GetSize(pExecState);
	case Function_ElementAtIndex: return ElementAtIndex(pExecState);
	case Function_SetElementIndex: return SetElementAtIndex(pExecState);
	case Function_Deconstruct: return DeconstructToStack(pExecState);
	default:
		return pExecState->CreateException("Cannot invoke that function on a vector3 object");
	}
}

bool Vector3::GetSize(ExecState* pExecState) {
	double xSquared = this->x * this->x;
	double ySquared = this->y * this->y;
	double zSquared = this->z * this->z;

	double modulus = sqrt(xSquared + ySquared + zSquared);

	if (!pExecState->pStack->Push(modulus)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool Vector3::ElementAtIndex(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	if (pElement == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	else if (pElement->GetType() != StackElement_Int) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Need an element index");
	}

	int index = (int)pElement->GetInt();
	delete pElement;
	pElement = nullptr;

	if (index < 0 || index>2) {
		return pExecState->CreateException("Index into vector3 must be 0,1, or 2");
	}
	double value = 0.0f;
	switch (index) {
	case 0: value = this->x; break;
	case 1: value = this->y; break;
	case 2: value = this->z; break;
	}
	if (!pExecState->pStack->Push(value)) {
		return pExecState->CreateStackOverflowException();
	}

	return true;
}

bool Vector3::SetElementAtIndex(ExecState* pExecState) {
	StackElement* pElementIndex = pExecState->pStack->Pull();
	if (pElementIndex == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	StackElement* pElementElement = pExecState->pStack->Pull();
	if (pElementElement == nullptr) {
		delete pElementIndex;
		pElementIndex = nullptr;
		return pExecState->CreateStackUnderflowException();
	}
	// ei must be i
	// ee must be i or f
	else if (pElementIndex->GetType() != StackElement_Int || (pElementElement->GetType() != StackElement_Float && pElementElement->GetType() != StackElement_Int)) {
		delete pElementIndex;
		pElementIndex = nullptr;
		delete pElementElement;
		pElementElement = nullptr;

		return pExecState->CreateException("Set element must be called with ( f n -- )");
	}

	int index = (int)pElementIndex->GetInt();
	double value = pElementElement->GetFloat();
	bool success = true;
	if (index < 0 || index>2) {
		success = pExecState->CreateException("Index into vector3 must be 0,1, or 2");
	}
	else {
		switch (index) {
		case 0: this->x = value; break;
		case 1: this->y = value; break;
		case 2: this->z = value; break;
		}
	}
	delete pElementIndex;
	pElementIndex = nullptr;
	delete pElementElement;
	pElementElement = nullptr;
	return success;
}

bool Vector3::DeconstructToStack(ExecState* pExecState) {
	int deleteCountIfPushFail = 0;
	bool success = true;
	if (!pExecState->pStack->Push(this->x)) {
		success = false;
	}
	else {
		deleteCountIfPushFail++;
		if (!pExecState->pStack->Push(this->y)) {
			success = false;
		}
		else {
			deleteCountIfPushFail++;
			if (!pExecState->pStack->Push(this->z)) {
				success = false;
			}
			else {
				deleteCountIfPushFail++;
			}

		}
	}
	if (!success) {
		pExecState->CreateStackOverflowException();
		for (int n = 0; n < deleteCountIfPushFail; ++n) {
			StackElement* pElement = pExecState->pStack->Pull();
			delete pElement;
			pElement = nullptr;
		}
	}
	return success;
}
