#include "ForthDefs.h"
#include "DataStack.h"
#include "ForthString.h"
#include "TypeSystem.h"

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

bool DataStack::Push(BinaryOperationType value) {
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

char DataStack::PullAsChar() {
	char defaultValue = '\0';
	if (this->topOfStack == -1) {
		return defaultValue;
	}

	StackElement& el = this->stack[this->topOfStack];
	char toReturn = el.GetChar();
	ShrinkStack();

	return toReturn;
}

double DataStack::PullAsFloat() {
	double defaultValue = 0.0;
	if (this->topOfStack == -1) {
		return defaultValue;
	}

	StackElement& el = this->stack[this->topOfStack];
	double toReturn = el.GetFloat();
	ShrinkStack();

	return toReturn;
}

ForthType DataStack::PullAsType() {
	ForthType defaultValue = StackElement_Undefined;
	if (this->topOfStack == -1) {
		return defaultValue;
	}

	StackElement& el = this->stack[this->topOfStack];
	ForthType toReturn = el.GetValueType();
	ShrinkStack();

	return toReturn;
}

WordBodyElement** DataStack::PullAsCFA() {
	WordBodyElement** defaultValue = nullptr;
	if (this->topOfStack == -1) {
		return defaultValue;
	}
	StackElement& el = this->stack[this->topOfStack];
	WordBodyElement** toReturn = el.GetWordBodyElement();
	ShrinkStack();
	return toReturn;
}

void* DataStack::PullAsVoidPter() {
	void* defaultValue = nullptr;
	if (this->topOfStack == -1) {
		return defaultValue;
	}

	StackElement& el = this->stack[this->topOfStack];
	void* toReturn = el.GetContainedPter();
	ShrinkStack();
	return toReturn;
}

RefCountedObject* DataStack::PullAsObject() {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	if (this->topOfStack == -1) {
		return nullptr;
	}
	StackElement& el = this->stack[this->topOfStack];
	RefCountedObject* pToReturn = nullptr;
	if (!pTS->TypeIsObject(el.GetType())) {
		return nullptr;
	}
	pToReturn = el.GetObject();
	pToReturn->IncReference();
	ShrinkStack();

	return pToReturn;
}

std::tuple<bool, std::string> DataStack::PullAsString() {
	if (this->topOfStack == -1) {
		return { false, "Stack underflow" };;
	}

	StackElement& el = this->stack[this->topOfStack];
	if (el.GetType() != ObjectType_String) {
		return { false, "No string on stack" };
	}
	ForthString* pForthString = (ForthString*)el.GetObject();
	std::string containedString = pForthString->GetContainedString();
	ShrinkStack();
	return { true, containedString };
}

StackElement DataStack::PullNoPter() {
	if (this->topOfStack == -1) {
		stack[0].RelinquishValue();
		return stack[0];
	}

	StackElement& el = this->stack[this->topOfStack];
	StackElement toReturn = std::move(el);

	ShrinkStack();

	return toReturn;
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

ForthType DataStack::GetTOSType() {
	if (this->topOfStack == -1) {
		return StackElement_Undefined;;
	}
	StackElement& tos = this->stack[this->topOfStack];
	return tos.GetType();
}

bool DataStack::SwapTOS() {
	if (this->topOfStack == 0) {
		return false;
	}
	StackElement tos = this->stack[this->topOfStack];
	StackElement nextStackElement = this->stack[this->topOfStack-1];
	this->stack[this->topOfStack] = nextStackElement;
	this->stack[this->topOfStack - 1] = tos;

	return true;
}

bool DataStack::DropTOS() {
	if (this->topOfStack == -1) {
		return false;
	}
	ShrinkStack();
	return true;
}

bool DataStack::DupTOS() {
	if (this->topOfStack == -1 || !MoveToNextSP()) {
		return false;
	}
	StackElement tos = this->stack[this->topOfStack-1];
	this->stack[this->topOfStack]=tos;
	return true;
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
	delete pElement;
	return true;
}

bool DataStack::Push(const StackElement& element) {
	if (!MoveToNextSP()) {
		return false;
	}
	this->stack[this->topOfStack] = element;
	return true;
}

void DataStack::Clear() {
	while (this->topOfStack > -1) {
		this->stack[this->topOfStack].RelinquishValue();
		--this->topOfStack;
	}
}