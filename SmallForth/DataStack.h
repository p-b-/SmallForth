#pragma once
#include <stack>
#include <tuple>
#include "StackElement.h"
using namespace std;

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
	bool Push(const string& value);

	StackElement* TopElement();
	StackElement* Pull();
	tuple<bool, string> PullAsString();
	tuple<StackElement*, StackElement* > PullTwo();
	tuple<bool, StackElement* > PullType(ElementType type);

	bool Push(StackElement* pElement);
	int Count() const { return (int)stack.size(); }


private:
	int stackSize;

	stack<StackElement* > stack;
};

