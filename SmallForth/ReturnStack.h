#pragma once
#include <stack>
#include "ForthDefs.h"

class ReturnStack
{
public:
	ReturnStack(int stackSize);
	bool Push(int n);
	bool Pull(int& n);
	void Clear();
	int Count() const { return (int)_stack.size();  }

private:
	int stackSize;

	std::stack<int> _stack;
};

