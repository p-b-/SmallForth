#pragma once
#include <stack>
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
	bool Push(const std::string& value);

	void Clear();

	StackElement* TopElement();
	StackElement* Pull();
	std::tuple<bool, std::string> PullAsString();
	std::tuple<StackElement*, StackElement* > PullTwo();
	std::tuple<bool, StackElement* > PullType(ElementType type);

	bool Push(StackElement* pElement);
	int Count() const { return (int)stack.size(); }


private:
	int stackSize;

	std::stack<StackElement* > stack;
};

