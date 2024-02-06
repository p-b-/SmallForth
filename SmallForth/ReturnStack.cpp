#include "ReturnStack.h"

ReturnStack::ReturnStack(int stackSize) {
	this->stackSize = stackSize;
}

bool ReturnStack::Push(int n) {
	if (this->_stack.size() == this->stackSize) {
		return false;
	}
	this->_stack.push(n);
	return true;
}

bool ReturnStack::Pull(int& n) {
	if (this->_stack.size() == 0) {
		return false;
	}
	n = this->_stack.top();
	this->_stack.pop();
	return true;
}

void ReturnStack::Clear() {
	while (!this->_stack.empty()) {
		this->_stack.pop();
	}
}