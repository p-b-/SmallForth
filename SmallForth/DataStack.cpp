#include "ForthDefs.h"
#include "DataStack.h"
#include "ForthString.h"

// TODO Implement stack using forth

DataStack::DataStack(int stackSize) {
	this->stackSize = stackSize;
}

DataStack::~DataStack() {
	while (this->stack.size() > 0) {
		StackElement* pTopElement = this->stack.top();
		this->stack.pop();
		delete pTopElement;
		pTopElement = nullptr;
	}
}

bool DataStack::Push(int64_t value) {
	StackElement* pElement = new StackElement(value);

	return Push(pElement);
}

bool DataStack::Push(char value) {
	StackElement* pElement = new StackElement(value);

	return Push(pElement);
}

bool DataStack::Push(double value) {
	StackElement* pElement = new StackElement(value);

	return Push(pElement);
}

bool DataStack::Push(bool value) {
	StackElement* pElement = new StackElement(value);

	return Push(pElement);
}

bool DataStack::Push(ForthType value) {
	StackElement* pElement = new StackElement(value);

	return Push(pElement);
}

bool DataStack::Push(WordBodyElement** wordBodyPter) {
	StackElement* pElement = new StackElement(wordBodyPter);
	return Push(pElement);
}

bool DataStack::Push(RefCountedObject* value) {
	StackElement* pElement = new StackElement(value);
	return Push(pElement);
}

bool DataStack::Push(const string& value) {
	ForthString* pForthString = new ForthString(value);
	if (!Push((RefCountedObject*)pForthString)) {
		pForthString->DecReference();
		return false;
	}
	pForthString->DecReference();
	return true;
}


StackElement* DataStack::Pull() {
	if (this->stack.empty()) {
		return nullptr;
	}
	StackElement* toReturn = this->stack.top();
	this->stack.pop();
	return toReturn;
}

tuple<bool, string> DataStack::PullAsString() {
	StackElement* pElement = Pull();
	if (pElement == nullptr) {
		return { false, "Stack underflow" };
	}
	if (pElement->GetType() != ObjectType_String) {
		delete pElement;
		return { false, "No string on stack" };
	}
	ForthString* pForthString = (ForthString* )pElement->GetObject();
	string containedString = pForthString->GetContainedString();
	delete pElement;
	pElement = nullptr;
	return { true, containedString };
}

tuple<bool, StackElement* > DataStack::PullType(ElementType type) {
	StackElement* pElement = Pull();
	if (pElement == nullptr) {
		return { false, nullptr };
	}
	else if (pElement->GetType() != type) {
		return { true, nullptr };
	}
	return { false, pElement };
}

tuple<StackElement*, StackElement* > DataStack::PullTwo() {
	StackElement* pElement2 = Pull();
	if (pElement2 == nullptr) {
		return { nullptr, nullptr };
	}
	StackElement* pElement1 = Pull();
	if (pElement1 == nullptr) {
		delete pElement2;
		pElement2 = nullptr;
		return { nullptr, nullptr };
	}
	return { pElement1, pElement2 };
}


StackElement* DataStack::TopElement() {
	if (this->stack.empty()) {
		return nullptr;
	}
	StackElement* toReturn = this->stack.top();
	return toReturn;

}

bool DataStack::Push(StackElement* pElement) {
	if (this->stack.size() == this->stackSize) {
		delete pElement;
		return false;
	}
	this->stack.push(pElement);
	return true;
}

void DataStack::Clear() {
	while (this->stack.empty() == false) {
		StackElement* pElement = this->stack.top();
		delete pElement;
		pElement = nullptr;
		this->stack.pop();
	}
}