#include "ForthDefs.h"
#include "ForthArray.h"
#include "ExecState.h"
#include "DataStack.h"
#include "StackElement.h"
#include "TypeSystem.h"

ForthArray::ForthArray(ForthDict* pDict) :
	RefCountedObject(pDict) {
	objectType = ObjectType_Array;
}

ForthArray::~ForthArray()
{
	this->elements.erase(this->elements.begin(), this->elements.end());
}

std::string ForthArray::GetObjectType()
{
	return "Array";
}

bool ForthArray::ToString(ExecState* pExecState) const {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	std::string str = "Array: " + pTS->TypeToString(containedType) + " size: ";
	str += std::to_string(this->elements.size());

	if (!pExecState->pStack->Push(str)) {
		return pExecState->CreateStackOverflowException("whilst creating string description of an array");
	}
	return true;
}

bool ForthArray::InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke)
{
	switch (functionToInvoke) {
	case Function_GetSize: return GetSize(pExecState);
	case Function_ElementAtIndex: return ElementAtIndex(pExecState);
	case Function_SetElementIndex: return SetElementAtIndex(pExecState);
	/*case Function_Contains: return Contains(pExecState);
	case Function_IndexOf: return IndexOf(pExecState);*/
//	case Function_SubRange: return SubString(pExecState);
	case Function_Append: return Append(pExecState);
	//case Function_Implements: return false;
	default:
		return pExecState->CreateException("Cannot invoke that function on an array object");
	}
}

bool ForthArray::Construct(ExecState* pExecState) {
	if (pExecState->pStack->Count() == 0) {
		return pExecState->CreateStackUnderflowException("constructing an array - cannot find type or value to create array for");
	}
	ForthType type = pExecState->pStack->GetTOSType();
	ForthArray* pNewArray = new ForthArray(nullptr);
	pNewArray->SetContainedType(type);
	if (!pNewArray->Append(pExecState)) {
		return false;
	}

	if (!pExecState->pStack->Push((RefCountedObject*)pNewArray)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}


bool ForthArray::GetSize(ExecState* pExecState) {
	int64_t size = this->elements.size();
	if (!pExecState->pStack->Push(size)) {
		return pExecState->CreateStackOverflowException("whilst getting size of array");
	}
	return true;
}

bool ForthArray::Append(ExecState* pExecState) {
	StackElement elementToAppend = pExecState->pStack->PullNoPter();
	if (elementToAppend.GetType() == StackElement_Undefined) {
		return pExecState->CreateStackUnderflowException("appending to an array");
	}
	ForthType elementType = elementToAppend.GetType();
	if (elementType != this->containedType) {
		return pExecState->CreateException("Incorrect type for array");
	}
	this->elements.push_back(elementToAppend);
	return true;
}

bool ForthArray::ElementAtIndex(ExecState* pExecState) {
	if (pExecState->pStack->Count() == 0) {
		return pExecState->CreateStackUnderflowException("Cannot get element in array - need an index");
	}
	if (!pExecState->pStack->TOSIsType(StackElement_Int)) {
		return pExecState->CreateException("Cannot get element in array - need an integer index");
	}
	int index = (int)pExecState->pStack->PullAsInt();
	if (index < 0 || index >= (int)this->elements.size()) {
		return pExecState->CreateException("Element index out of range whilst accessing an array");
	}
	StackElement& elementAtIndex = elements[index];
	if (!pExecState->pStack->Push(elementAtIndex)) {
		return pExecState->CreateStackOverflowException("whilst pushing an array element");
	}
	return true;
}

bool ForthArray::SetElementAtIndex(ExecState* pExecState) {
	if (pExecState->pStack->Count() == 0) {
		return pExecState->CreateStackUnderflowException("Cannot set element in array - need an index");
	}
	if (!pExecState->pStack->TOSIsType(StackElement_Int)) {
		return pExecState->CreateException("Cannot set element in array - need an integer index");
	}
	int index = (int)pExecState->pStack->PullAsInt();
	if (index < 0 || index >= (int)this->elements.size()) {
		return pExecState->CreateException("Element index out of range whilst accessing an array to set an element");
	}
	if (pExecState->pStack->Count() == 0) {
		return pExecState->CreateStackUnderflowException("Cannot set element in array - need an element to set array index to");
	}
	if (!pExecState->pStack->TOSIsType((ElementType)this->containedType)) {
		return pExecState->CreateException("Cannot set element in array - need a matching element type");
	}
	StackElement elementToSet = pExecState->pStack->PullNoPter();

	this->elements[index] = elementToSet;
	return true;
}
