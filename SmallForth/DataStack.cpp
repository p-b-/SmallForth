#include "ForthDefs.h"
#include "DataStack.h"
#include "ForthString.h"

// TODO Implement stack using forth

DataStack::DataStack(int stackSize) {
	this->stackSize = stackSize;
	for (int i = 0; i < this->stackSize; ++i) {
		StackElement element;
		this->stack.push_back(element);
	}
	this->topOfStack = -1;
}

DataStack::~DataStack() {
	//while (this->stack.size() > 0) {
	//	StackElement* pTopElement = this->stack.top();
	//	this->stack.pop();
	//	delete pTopElement;
	//	pTopElement = nullptr;
	//}
}

bool DataStack::Push(int64_t value) {
	if (!MoveToNextSP()) {
		return false;
	}
	this->stack[this->topOfStack].SetTo(value);
	return true;
}

bool DataStack::MoveToNextSP() {
	if (this->topOfStack == this->stackSize - 1) {
		return false;
	}
	++this->topOfStack;
	return true;
}

bool DataStack::Push(char value) {
	if (!MoveToNextSP()) {
		return false;
	}
	this->stack[this->topOfStack].SetTo(value);
	return true;
}

bool DataStack::Push(double value) {
	if (!MoveToNextSP()) {
		return false;
	}
	this->stack[this->topOfStack].SetTo(value);
	return true;
}

bool DataStack::Push(bool value) {
	if (!MoveToNextSP()) {
		return false;
	}
	this->stack[this->topOfStack].SetTo(value);
	return true;
}

bool DataStack::Push(ForthType value) {
	if (!MoveToNextSP()) {
		return false;
	}
	this->stack[this->topOfStack].SetTo(value);
	return true;
}

bool DataStack::Push(WordBodyElement** wordBodyPter) {
	if (!MoveToNextSP()) {
		return false;
	}
	this->stack[this->topOfStack].SetTo(wordBodyPter);
	return true;
}

bool DataStack::Push(RefCountedObject* value) {
	if (!MoveToNextSP()) {
		return false;
	}
	this->stack[this->topOfStack].SetTo(value);
	return true;
}

bool DataStack::Push(ForthType forthType, WordBodyElement** ppLiteral) {
	if (!MoveToNextSP()) {
		return false;
	}
	this->stack[this->topOfStack].SetTo(forthType, ppLiteral);
	return true;
}

bool DataStack::Push(ForthType forthType, void* pter) {
	if (!MoveToNextSP()) {
		return false;
	}

	this->stack[this->topOfStack].SetTo(forthType, pter);
	return true;
}


bool DataStack::Push(const std::string& value) {
	ForthString* pForthString = new ForthString(value);
	if (!MoveToNextSP()) {
		pForthString->DecReference();
		return false;
	}
	this->stack[this->topOfStack].SetTo((RefCountedObject*)pForthString);
	pForthString->DecReference();
	return true;
}

StackElement* DataStack::Pull() {
	if (this->topOfStack == -1) {
		return nullptr;
	}

	StackElement& toClone = this->stack[this->topOfStack];
	StackElement* toReturn = new StackElement(toClone);
	ShrinkStack();
	return toReturn;
}

bool DataStack::PullAsBool() {
	bool defaultValue = false;
	if (this->topOfStack == -1) {
		return defaultValue;
	}

	StackElement& el = this->stack[this->topOfStack];
	bool toReturn = el.GetBool();
	ShrinkStack();

	return toReturn;
}

int64_t DataStack::PullAsInt() {
	int64_t defaultValue = 0;
	if (this->topOfStack == -1) {
		return defaultValue;
	}

	StackElement& el = this->stack[this->topOfStack];
	int64_t toReturn = el.GetInt();
	ShrinkStack();

	return toReturn;
}

std::tuple<bool, std::string> DataStack::PullAsString() {
	StackElement* pElement = Pull();
	if (pElement == nullptr) {
		return { false, "Stack underflow" };
	}
	if (pElement->GetType() != ObjectType_String) {
		delete pElement;
		return { false, "No string on stack" };
	}
	ForthString* pForthString = (ForthString* )pElement->GetObject();
	std::string containedString = pForthString->GetContainedString();
	delete pElement;
	pElement = nullptr;
	return { true, containedString };
}

std::tuple<bool, StackElement* > DataStack::PullType(ElementType type) {
	StackElement* pElement = Pull();
	if (pElement == nullptr) {
		return { false, nullptr };
	}
	else if (pElement->GetType() != type) {
		return { true, nullptr };
	}
	return { false, pElement };
}

std::tuple<StackElement*, StackElement* > DataStack::PullTwo() {
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

bool DataStack::TOSIsType(ElementType elementType) {
	if (this->topOfStack == -1) {
		return false;
	}
	StackElement& tos = this->stack[this->topOfStack];
	return tos.GetType() == elementType;
}

StackElement* DataStack::TopElement() {
	if (this->topOfStack == -1) {
		return nullptr;
	}
	StackElement* toReturn = &this->stack[this->topOfStack];
	return toReturn;
}

bool DataStack::Push(StackElement* pElement) {
	if (this->topOfStack == this->stackSize - 1) {
		delete pElement;
		return false;
	}
	++this->topOfStack;
	this->stack[this->topOfStack] = *pElement;
	this->toDelete.push_back(pElement);
	return true;
}

void DataStack::Clear() {
	while (this->topOfStack > -1) {
		this->stack[this->topOfStack].RelinquishValue();
		--this->topOfStack;
	}
}