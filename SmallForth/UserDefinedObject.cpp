#include <iostream>
using namespace std;
#include "ForthDefs.h"
#include "UserDefinedObject.h"
#include "ExecState.h"
#include "StackElement.h"
#include "DataStack.h"
#include "TypeSystem.h"
#include "ForthDict.h"
#include "ForthString.h"

UserDefinedObject::UserDefinedObject(string name, int stateCount, ForthDict* pDict) 
: RefCountedObject(pDict) {
	this->objectType = 0;
	this->defaultObject = false;
	this->objectName = name;
	this->stateCount = stateCount;
	this->markedByReferenceCounter = false;
}

UserDefinedObject::~UserDefinedObject() {
	// No need to alter reference counts of sub-objects.  To get to the point of this destructor being called, it's ref
	//  count must have reached zero.  In doing so it would have already decremented the ref counts of sub-objects
	//  as far as it needs to.
}

void UserDefinedObject::SetType(ForthType type) {
	this->objectType = type;
}

bool UserDefinedObject::DefineObjectFromStack(ExecState* pExecState) {
	for (int n = 0; n < this->stateCount; ++n) {
		this->state.push_back(nullptr);
	}
	for (int n = this->stateCount - 1; n >= 0; --n) {
		StackElement* pStateElement = pExecState->pStack->Pull();
		if (pStateElement == nullptr) {
			return pExecState->CreateStackUnderflowException();
		}
		this->state[n] = pStateElement;
	}
	this->defaultObject = true;
	return true;
}

void UserDefinedObject::IncReference() {
	//cout << "       : " << GetObjectType() << " " << std::hex << this << std::dec << " inc " << this->GetReferenceCount() << " to " << (this->GetReferenceCount() + 1) << endl;
	// Potentially marked by the construction method, to prevent the reference count going higher when it is returned to the construction code and it's added to a stack element
	if (!this->markedByReferenceCounter) {
		MarkForReferenceCounter(true);
		for (StackElement* pSE : this->state) {
			ConsiderRefCountChangeForLinkedObject(pSE, 1);
		}
	}
	MarkForReferenceCounter(false);
	RefCountedObject::IncReference();
}

void UserDefinedObject::DecReference() {
	//cout << "       : " << GetObjectType() << " " << std::hex << this << std::dec << " dec " << this->GetReferenceCount() << " to " << (this->GetReferenceCount() - 1) << endl;
	if (!this->markedByReferenceCounter) {
		MarkForReferenceCounter(true);
		for (StackElement* pSE : this->state) {
			ConsiderRefCountChangeForLinkedObject(pSE, -1);
		}
		MarkForReferenceCounter(false);
	}
	RefCountedObject::DecReference();
}

void UserDefinedObject::IncReferenceBy(int by) {
	// Potentially marked by the construction method, to prevent the reference count going higher when it is returned to the construction code and it's added to a stack element
	if (!this->markedByReferenceCounter) {
		MarkForReferenceCounter(true);
		for (StackElement* pSE : this->state) {
			ConsiderRefCountChangeForLinkedObject(pSE, by);
		}
	}
	MarkForReferenceCounter(false);
	RefCountedObject::IncReferenceBy(by);
}

void UserDefinedObject::DecReferenceBy(int by) {
	if (!this->markedByReferenceCounter) {
		MarkForReferenceCounter(true);
		for (StackElement* pSE : this->state) {
			ConsiderRefCountChangeForLinkedObject(pSE, -by);
		}
		MarkForReferenceCounter(false);
	}
	RefCountedObject::DecReference();
}

string UserDefinedObject::GetObjectType() {
	return this->objectName;
}

bool UserDefinedObject::ToString(ExecState* pExecState) const {
	ForthWord* pToString = GetWordWithName("tostring");
	if (pToString == nullptr) {
		if (!pExecState->pStack->Push(this->objectName)) {
			return pExecState->CreateStackOverflowException();
		}
		return true;
	}
	else {
		if (!pExecState->pStack->Push(new StackElement((RefCountedObject*)this))) {
			return pExecState->CreateStackOverflowException("whilst turn an object into a string");
		}
		return pExecState->ExecuteWordDirectly("tostring");
	}
}

bool UserDefinedObject::InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke) {
	switch (functionToInvoke) {
	case Function_ElementAtIndex: return ElementAtIndex(pExecState);
	case Function_SetElementIndex: return SetElementAtIndex(pExecState);
	case Function_Deconstruct: return DeconstructToStack(pExecState);
	default:
		return pExecState->CreateException("Cannot invoke that function on a vector3 object");
	}

	return false;
}

void UserDefinedObject::ConsiderRefCountChangeForLinkedObject(StackElement* pElement, int changeDirection) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	ForthType elementType = pElement->GetType();
	if (pTS->TypeIsObjectOrObjectPter(elementType) &&
		pElement->GetObject()->GetMarkForReferenceCounter()==false) {
		if (changeDirection == 1) {
			//cout << "     : Incrementing sub object of type " << elementType << endl;
			pTS->IncReferenceForPter(elementType, pElement->GetObject());
		}
		else if (changeDirection>1) {
			pTS->IncReferenceForPterBy(elementType, pElement->GetObject(),changeDirection);
		}
		else if (changeDirection==-1) {
			//cout << "     : Decrementing sub object of type " << elementType << endl;
			pTS->DecReferenceForPter(elementType, pElement->GetObject());
		}
		else {
			pTS->DecReferenceForPterBy(elementType, pElement->GetObject(),-changeDirection);
		}
	}
}

UserDefinedObject* UserDefinedObject::Construct(ExecState* pExecState) {
	ForthWord* pConstruct = GetWordWithName("construct");
	if (pConstruct != nullptr)
	{
		// TODO Finish this code.  Need to pull the created object off the stack and return it

		// Currently, construct is always called CPP code.  Move to constructable user-defined objects that rather than 
		//  copy the default state elements, creates the object from information on the stack
		if (!pExecState->pStack->Push(new StackElement((RefCountedObject*)this))) {
			pExecState->CreateStackOverflowException("whilst construct an object");
			return nullptr;
		}
		pExecState->ExecuteWordDirectly("construct");
		return nullptr;
	}
	else {
		TypeSystem* pTS = TypeSystem::GetTypeSystem();
		UserDefinedObject* pConstructed = new UserDefinedObject(this->objectName, this->stateCount, this->pDictionary);
		pConstructed->objectType = this->objectType;
		pConstructed->MarkForReferenceCounter(true);
		//cout << "  : iterating over state elements " << endl;
		for (StackElement* pSE : this->state) {
			/*	if (pTS->TypeIsObjectOrObjectPter(pSE->GetType())) {
					cout << "  : copy default element, increment... " << endl;
				}*/
			StackElement* pNewElement = new StackElement(*pSE);
			pConstructed->state.push_back(pNewElement);
		}
		// Leaving this construction method with the object 'marked'.  This will stop
		//  the impending ref inc (on the constructed object) from incorrectly incrementing
		//  the subobjects
		// 
		//cout << "  : finished iterating over state elements " << endl;
		return pConstructed;
	}
}

bool UserDefinedObject::ElementAtIndex(ExecState* pExecState) {
	StackElement* pElementIndex;
	bool incorrectType;
	tie(incorrectType, pElementIndex) = pExecState->pStack->PullType(StackElement_Int);
	if (incorrectType) {
		return pExecState->CreateException("Need an element index");
	} else if (pElementIndex == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}

	int index = (int)pElementIndex->GetInt();
	delete pElementIndex;
	pElementIndex = nullptr;

	if (index < 0 || index>this->stateCount) {
		return pExecState->CreateException("Index into user defined type must be within the range of its state count");
	}
	StackElement* pStateItem = this->state[index];
	StackElement* pStateItemToReturn = new StackElement(*pStateItem);
	if (!pExecState->pStack->Push(pStateItemToReturn)) {
		return pExecState->CreateStackOverflowException();
	}

	return true;
}

bool UserDefinedObject::SetElementAtIndex(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElementIndex;
	bool incorrectType;
	tie(incorrectType, pElementIndex) = pExecState->pStack->PullType(StackElement_Int);
	if (incorrectType) {
		return pExecState->CreateException("Set element must be called with ( e n -- ) - need an element index");
	}
	else if (pElementIndex == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}

	int index = (int)pElementIndex->GetInt();
	delete pElementIndex;
	pElementIndex = nullptr;

	StackElement* pElementElement = pExecState->pStack->Pull();
	if (pElementElement == nullptr) {
		delete pElementIndex;
		pElementIndex = nullptr;
		return pExecState->CreateStackUnderflowException();
	}
	else {
		ForthType typePassed = pElementElement->GetType();
		ForthType typeToSet = this->state[index]->GetType();

		if (typePassed != typeToSet) {
			delete pElementElement;
			pElementElement = nullptr;
			return pExecState->CreateException("Set element must be called with ( e n -- ) where e is a type compatible with state element being set");
		}
	}
	if (index < 0 || index>this->stateCount) {
		delete pElementElement;
		pElementElement = nullptr;
		return pExecState->CreateException("Index into user defined type must be within the range of its state count");
	}

	StackElement* pCurrentElement = this->state[index];
	if (pTS->TypeIsObjectOrObjectPter(pElementElement->GetType())) {
		// The -1 is because we are NULLing this pElementElement pter, not deleting it.  Deleting it
		//  would cause the item to lose a reference point
		pElementElement->GetObject()->IncReferenceBy(this->GetCurrentReferenceCount()-1);
	}
	this->state[index] = pElementElement;
	pElementElement = nullptr;

	if (pTS->TypeIsObjectOrObjectPter(pCurrentElement->GetType())) {
		pCurrentElement->GetObject()->DecReferenceBy(this->GetCurrentReferenceCount());
	}
	delete pCurrentElement;
	pCurrentElement = nullptr;
	return true;
}

bool UserDefinedObject::DeconstructToStack(ExecState* pExecState) {
	int deleteCountIfPushFail = 0;
	bool success = true;
	for (StackElement* pSE : this->state) {
		StackElement* pCloned = new StackElement(*pSE);
		if (!pExecState->pStack->Push(pCloned)) {
			success = false;
			break;
		}
		deleteCountIfPushFail++;
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
