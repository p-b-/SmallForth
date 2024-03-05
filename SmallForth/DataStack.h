#pragma once
#include <vector>
#include <tuple>
#include "StackElement.h"

class DataStack
{
public:
	DataStack(int stackSize);
	~DataStack();

	bool Push(int64_t value);
	bool Push(char value);
	bool Push(double value);
	bool Push(bool value);
	bool Push(WordBodyElement** wordBodyPter);
	bool Push(ForthType value);
	bool Push(RefCountedObject* value);
	bool Push(ForthType forthType, WordBodyElement** ppLiteral);
	bool Push(ForthType forthType, void* pter);
	bool Push(const std::string& value);

	void Clear();

	bool TOSIsType(ElementType elementType);
	ForthType GetTOSType();
	bool SwapTOS();
	bool DropTOS();
	bool DupTOS();

	StackElement* TopElement();
	StackElement* Pull();
	bool PullAsBool();
	int64_t PullAsInt();
	StackElement PullAsRef();

	std::tuple<bool, std::string> PullAsString();
	std::tuple<StackElement*, StackElement* > PullTwo();
	std::tuple<bool, StackElement* > PullType(ElementType type);

	bool Push(StackElement* pElement);
	bool Push(const StackElement& pElement);

	int Count() const { return topOfStack+1; }

private:
	bool MoveToNextSP();
	inline void ShrinkStack() {
		this->stack[this->topOfStack].RelinquishValue();
		--this->topOfStack;
	}

private:
	int stackSize;

	std::vector<StackElement> stack;
	int topOfStack;

//	std::stack<StackElement* > stack;
};

