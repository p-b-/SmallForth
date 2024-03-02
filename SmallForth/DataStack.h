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
	bool Push(const std::string& value);

	void Clear();

	StackElement* TopElement();
	StackElement* Pull();
	std::tuple<bool, std::string> PullAsString();
	std::tuple<StackElement*, StackElement* > PullTwo();
	std::tuple<bool, StackElement* > PullType(ElementType type);

	bool Push(StackElement* pElement);
	int Count() const { return topOfStack+1; }

private:
	bool MoveToNextSP();


private:
	int stackSize;

	std::vector<StackElement> stack;
	int topOfStack;
	std::vector<StackElement* > toDelete;

//	std::stack<StackElement* > stack;
};

