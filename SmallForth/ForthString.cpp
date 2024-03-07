#include <iostream>
#include "ForthDefs.h"
#include "ForthString.h"
#include "DataStack.h"
#include "ExecState.h"
#include "StackElement.h"

ForthString::ForthString(const std::string& pzString) :
	RefCountedObject(nullptr) {
	containedString = pzString;
	objectType = ObjectType_String;
	IncReference();
}

ForthString::~ForthString() {

}

std::string ForthString::GetObjectType() {
	return "string";
}

bool ForthString::ToString(ExecState* pExecState) const {
	if (!pExecState->pStack->Push((RefCountedObject*)this)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthString::Construct(ExecState* pExecState) {
	ForthString* pNewString = new ForthString("");
	if (!pExecState->pStack->Push((RefCountedObject*)pNewString)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthString::InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke) {
	switch (functionToInvoke) {
	case Function_GetSize: return GetSize(pExecState);
	case Function_ElementAtIndex: return ElementAtIndex(pExecState);
	case Function_SetElementIndex: return SetElementAtIndex(pExecState);
	case Function_Contains: return Contains(pExecState);
	case Function_IndexOf: return IndexOf(pExecState);
	case Function_SubRange: return SubString(pExecState);
	case Function_Append: return Append(pExecState);
	case Function_Implements: return false;
	default:
		return pExecState->CreateException("Cannot invoke that function on a string object");
	}
}

bool ForthString::GetSize(ExecState* pExecState) {
	 int64_t size = containedString.size();
	 if (!pExecState->pStack->Push(size)) {
		 return pExecState->CreateStackOverflowException();
	 }

	 return true;
}

bool ForthString::ElementAtIndex(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	if (pElement == nullptr) {
		return pExecState->CreateStackUnderflowException("getting char or codepoint at an element index - cannot get index");
	}
	else if (pElement->GetType() != StackElement_Int) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Need an element index");
	}

	bool success = true;
	int index = (int)pElement->GetInt();
	if (index < 0) {
		success = pExecState->CreateException("Cannot access a negative element index");
	}
	else if (index >= this->containedString.size()) {
		success = pExecState->CreateException("Cannot access an element beyond the object bounds");
	}
	else {
		char c = containedString[index];
		if (!pExecState->pStack->Push(c)) {
			success = pExecState->CreateStackOverflowException();
		}
	}

	delete pElement;
	pElement = nullptr;
	return success;
}

bool ForthString::SetElementAtIndex(ExecState* pExecState) {
	StackElement* pElementIndex = pExecState->pStack->Pull();
	if (pElementIndex == nullptr) {
		return pExecState->CreateStackUnderflowException("setting a char or codepoint at an element index - cannot get index");
	}
	StackElement* pElementElement = pExecState->pStack->Pull();
	if (pElementElement == nullptr) {
		delete pElementIndex;
		pElementIndex = nullptr;
		return pExecState->CreateStackUnderflowException("setting a char or codepoint at an element index - cannot get element to set");
	}
	else if (pElementIndex->GetType() != StackElement_Int || pElementElement->GetType() != StackElement_Char) {
		delete pElementIndex;
		pElementIndex = nullptr;
		delete pElementElement;
		pElementElement = nullptr;

		return pExecState->CreateException("Set element must be called with ( char n -- )");
	}

	int index = (int)pElementIndex->GetInt();
	bool success = true;
	if (index < -1) {

		success = pExecState->CreateException("Cannot access a negative element index");
	}
	else if (index!=-1 && index > this->containedString.size()) {
		success = pExecState->CreateException("Cannot access an element beyond the object bounds");
	}
	else {
		char c = pElementElement->GetChar();

		if (index == -1) {
			this->containedString = c + this->containedString;
		}
		else if (index == this->containedString.size()) {
			// Append
			this->containedString += c;
		}
		else {
			this->containedString[index] = c;
		}
	}
	delete pElementIndex;
	pElementIndex = nullptr;
	delete pElementElement;
	pElementElement = nullptr;
	return success;
}


bool ForthString::Contains(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	if (pElement == nullptr) {
		return pExecState->CreateStackUnderflowException("cannot get char or codepoint to search for");
	}
	else if (pElement->GetType() != StackElement_Char) {
		delete pElement;
		pElement = nullptr;

		return pExecState->CreateException("Contains must be called with ( char -- bool )");
	}
	char c = pElement->GetChar();
	delete pElement;
	pElement = nullptr;

	bool containsChar = this->containedString.find(c, 0) != std::string::npos;
	if (!pExecState->pStack->Push(containsChar)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthString::IndexOf(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	if (pElement == nullptr) {
		return pExecState->CreateStackUnderflowException("cannot get char or codepoint to get index of in string");
	}

	StackElement* pElementFromIndex = pExecState->pStack->Pull();
	if (pElementFromIndex == nullptr) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateStackUnderflowException();
	}
	else if (pElement->GetType() != StackElement_Char || pElementFromIndex->GetType() != StackElement_Int) {
		delete pElementFromIndex;
		pElementFromIndex = nullptr;

		delete pElement;
		pElement = nullptr;

		return pExecState->CreateException("IndexOf must be called with ( n char -- n )");
	}
	int afterIndex = (int)pElementFromIndex->GetInt();
	delete pElementFromIndex;
	pElementFromIndex = nullptr;

	char c = pElement->GetChar();
	delete pElement;
	pElement = nullptr;

	size_t index = this->containedString.find(c, afterIndex);
	if (index == std::string::npos) {
		index = -1;
	}
	if (!pExecState->pStack->Push((int64_t)index)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthString::SubString(ExecState* pExecState) {
	StackElement* pElementEndRange = pExecState->pStack->Pull();
	if (pElementEndRange == nullptr) {
		return pExecState->CreateStackUnderflowException("cannot get range end for substring");
	}
	StackElement* pElementStartRange = pExecState->pStack->Pull();
	if (pElementStartRange == nullptr) {
		delete pElementEndRange;
		pElementStartRange = nullptr;
		return pExecState->CreateStackUnderflowException("cannot get range start for substring");
	}
	else if (pElementEndRange->GetType() != StackElement_Int || pElementStartRange->GetType() != StackElement_Int) {
		delete pElementEndRange;
		pElementEndRange = nullptr;
		delete pElementStartRange;
		pElementStartRange = nullptr;

		return pExecState->CreateException("SubString must be called with ( n(start) n(excl end) -- $ )");
	}

	int rangeSt = (int)pElementStartRange->GetInt();
	int rangeEnd = (int)pElementEndRange->GetInt();
	delete pElementStartRange;
	pElementStartRange = nullptr;
	delete pElementEndRange;
	pElementEndRange = nullptr;
	if (rangeSt >= rangeEnd) {
		return pExecState->CreateException("Substring start must be before end");
	}
	else if (rangeSt < 0 || rangeSt>=this->containedString.length()) {
		return pExecState->CreateException("Substring start must be within the strings bounds");
	}
	else if (rangeEnd > this->containedString.length()) {
		return pExecState->CreateException("Substring end must not be more than one past the string end");
	}

	std::string substring = this->containedString.substr(rangeSt, rangeEnd - rangeSt);

	ForthString* pNewString = new ForthString(substring);
	if (!pExecState->pStack->Push(pNewString)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

// ( element object -- )
bool ForthString::Append(ExecState* pExecState) {
	if (pExecState->pStack->Count() ==0 ) {
		return pExecState->CreateStackUnderflowException("cannot get char or codepoint element to append to string");
	}
	ForthType appendType = pExecState->pStack->GetTOSType();

	char toAppend;
	ForthString* pAppendString;

	switch (appendType) {
	case ObjectType_String:
		pAppendString = (ForthString*)pExecState->pStack->PullAsObject();
		this->containedString += pAppendString->containedString;
		break;
	case StackElement_Char:
		toAppend = pExecState->pStack->PullAsChar();
		this->containedString += toAppend;
		break;
	case ObjectType_ReadWriteFile:
	case ObjectType_WriteFile:
	{
		StackElement* pElementToAppend = pExecState->pStack->Pull();

		return AppendToFile(pExecState, pElementToAppend);
	}
	default:
		return pExecState->CreateException("Can only directly append characters and strings, to strings");
	}
	if (!pExecState->pStack->Push((RefCountedObject*)this)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthString::AppendToFile(ExecState* pExecState, StackElement* pElementFile) {
	if (!pExecState->pStack->Push((RefCountedObject*)this)) {
		delete pElementFile;
		pElementFile=nullptr;
		return pExecState->CreateStackOverflowException("pushing string, to swap string and file to append to file");
	}
	if (!pExecState->pStack->Push(pElementFile)) {
		delete pElementFile;
		pElementFile = nullptr;
		return pExecState->CreateStackOverflowException("pushing file, to swap string and file to append to file");
	}
	return pExecState->ExecuteWordDirectly("append");
}