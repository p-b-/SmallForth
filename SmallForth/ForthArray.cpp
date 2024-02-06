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
	for (StackElement* pElement : this->elements) {
		delete pElement;
		pElement = nullptr;
	}
	this->elements.erase(this->elements.begin(), this->elements.end());
}

string ForthArray::GetObjectType()
{
	return "Array";
}

bool ForthArray::ToString(ExecState* pExecState) const
{
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	string str = "Array: " + pTS->TypeToString(containedType) + " size: ";
	str += to_string(this->elements.size());

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
	StackElement* pElement = pExecState->pStack->TopElement();
	if (pElement == nullptr) {
		return pExecState->CreateStackUnderflowException("constructing an array - cannot find type or value to create array for");
	}
	ForthType type = pElement->GetType();

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
	StackElement* pElementToAppend = pExecState->pStack->Pull();
	if (pElementToAppend == nullptr) {
		return pExecState->CreateStackUnderflowException("appending to an array");
	}
	ForthType elementType = pElementToAppend->GetType();
	if (elementType != this->containedType) {
		delete pElementToAppend;
		pElementToAppend = nullptr;
		return pExecState->CreateException("Incorrect type for array");
	}
	this->elements.push_back(pElementToAppend);

	return true;
}

bool ForthArray::ElementAtIndex(ExecState* pExecState) {
	StackElement* pElementIndex = pExecState->pStack->Pull();
	if (pElementIndex == nullptr) {
		return pExecState->CreateStackUnderflowException("Cannot get element in array - need an index");
	}
	if (pElementIndex->GetType() != StackElement_Int) {
		delete pElementIndex;
		pElementIndex = nullptr;
		return pExecState->CreateException("Cannot get element in array - need an integer index");
	}
	int index = (int)pElementIndex->GetInt();
	delete pElementIndex;
	pElementIndex = nullptr;

	if (index < 0 || index >= (int)this->elements.size()) {
		return pExecState->CreateException("Element index out of range whilst accessing an array");
	}
	StackElement* pElement = elements[index];
	StackElement* pCopiedElement = new StackElement(*pElement);
	if (!pExecState->pStack->Push(pCopiedElement)) {
		delete pCopiedElement;
		pCopiedElement = nullptr;
		return pExecState->CreateStackOverflowException("whilst pushing an array element");
	}
	return true;
}

bool ForthArray::SetElementAtIndex(ExecState* pExecState) {
	StackElement* pElementIndex = pExecState->pStack->Pull();
	if (pElementIndex == nullptr) {
		return pExecState->CreateStackUnderflowException("Cannot set element in array - need an index");
	}
	if (pElementIndex->GetType() != StackElement_Int) {
		delete pElementIndex;
		pElementIndex = nullptr;
		return pExecState->CreateException("Cannot set element in array - need an integer index");
	}
	int index = (int)pElementIndex->GetInt();
	delete pElementIndex;
	pElementIndex = nullptr;
	if (index < 0 || index >= (int)this->elements.size()) {
		return pExecState->CreateException("Element index out of range whilst accessing an array to set array element");
	}

	StackElement* pElementElement = pExecState->pStack->Pull();
	if (pElementElement == nullptr) {
		return pExecState->CreateStackUnderflowException("Cannot set element in array - need an element to set array index to");
	}
	if (pElementElement->GetType() != this->containedType) {
		delete pElementElement;
		pElementElement = nullptr;
		return pExecState->CreateException("Cannot set element in array - need a matching element type");
	}

	StackElement* pCurrentElement = this->elements[index];
	this->elements[index] = pElementElement;
	if (pCurrentElement!=pElementElement) {
		delete pCurrentElement;
		pCurrentElement = nullptr;
	}
	return true;
}
