#include <iostream>
using namespace std;
#include "PreBuiltWords.h"
#include "ExecState.h"
#include "ForthString.h"
#include "CompileHelper.h"

bool PreBuiltWords::PushSelf(ExecState* pExecState) {
	// This call increments reference count on pter before returning, if it is valid.
	RefCountedObject* self = pExecState->GetCurrentSelfPter();

	if (self == nullptr) {
		return pExecState->CreateException("Not currently in object - self not defined");
	}

	bool success = true;
	if (!pExecState->pStack->Push(self)) {
		success = pExecState->CreateStackOverflowException();
	}
	self->DecReference();

	return success;
}

bool PreBuiltWords::PushRefCount(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	StackElement* pElement = pExecState->pStack->TopElement();
	if (pElement == nullptr) {
		return pExecState->CreateException("No TOS to out reference count for");
	}
	if (pTS->TypeIsObjectOrObjectPter(pElement->GetType())) {
		int refCount = pTS->GetReferenceCount(pElement->GetType(), pElement->GetContainedPter());
		ostream* pStdoutStream = pExecState->GetStdout();
		(*pStdoutStream) << "Reference count is " << refCount << endl;
	}
	else {
		return pExecState->CreateException("Only object types have reference counts");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Allot(ExecState* pExecState) {
	StackElement* pElementAllotBy = pExecState->pStack->Pull();
	if (pElementAllotBy == nullptr) {
		return pExecState->CreateStackUnderflowException("whilst attempting to allot cells to a word");
	}
	else if (pElementAllotBy->GetType() != StackElement_Int) {
		delete pElementAllotBy;
		pElementAllotBy = nullptr;
		return pExecState->CreateException("ALLOT requires an integer");
	}
	int64_t n = pElementAllotBy->GetInt();
	delete pElementAllotBy;
	pElementAllotBy = nullptr;

	pExecState->pCompiler->ExpandLastWordCompiledBy(pExecState, (int)n);

	return true;
}

// ( e -- $) formats element e and converts to string $
bool PreBuiltWords::ToString(ExecState* pExecState) {
	StackElement* pElementToStringify = pExecState->pStack->Pull();
	if (pElementToStringify == nullptr) {
		return pExecState->CreateReturnStackUnderflowException();
	}
	bool success = pElementToStringify->ToString(pExecState);
	delete pElementToStringify;
	pElementToStringify = nullptr;

	

	return success;
}

// ( n -- c)
bool PreBuiltWords::ToChar(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType elementType = pElement->GetType();
	if (elementType != StackElement_Char && elementType != StackElement_Int) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Can only convert ints and chars, to chars");
	}
	if (elementType == StackElement_Char) {
		if (!pExecState->pStack->Push(pElement)) {
			return pExecState->CreateStackOverflowException();
		}
	}
	else {
		int64_t n = pElement->GetInt();
		delete pElement;
		pElement = nullptr;
		// Only supporting ascii at the moment
		char c = (char)(n & 0xff);
		if (!pExecState->pStack->Push(c)) {
			return pExecState->CreateStackOverflowException();
		}
	}
	return true;
}

// ( $/c/f -- n)
bool PreBuiltWords::ToInt(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType elementType = pElement->GetType();
	if (elementType!= StackElement_Int && elementType != StackElement_Char && elementType != StackElement_Float && elementType != ObjectType_String) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Can only convert floats, ints, chars and strings, to ints");
	}
	if (elementType == StackElement_Int) {
		if (!pExecState->pStack->Push(pElement)) {
			return pExecState->CreateStackOverflowException("whilst converting an int to an int");
		}
		return true;
	}
	int64_t n;
	if (elementType == StackElement_Char) {
		char c = pElement->GetChar();

		n = (int64_t)c;
	}
	else if (elementType == StackElement_Float) {
		double f = pElement->GetFloat();
		n = (int64_t)f;
	}
	else if (elementType == ObjectType_String) {
		ForthString* pString = (ForthString*)pElement->GetObject();
		string s = pString->GetContainedString();

		char* end;
		n = strtoll(s.c_str(), &end, 10);
		if (end == s.c_str()) {
			delete pElement;
			pElement = nullptr;
			return pExecState->CreateException("Could not convert string to integer");
		}
	}

	delete pElement;
	pElement = nullptr;

	if (!pExecState->pStack->Push(n)) {
		return pExecState->CreateStackOverflowException("whilst converting TOS to an integer");
	}
	return true;
}

// ( $/n -- f)
bool PreBuiltWords::ToFloat(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType elementType = pElement->GetType();
	if (elementType != StackElement_Int && elementType != StackElement_Float && elementType != ObjectType_String) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Can only convert floats, chars and strings, to ints");
	}
	if (elementType == StackElement_Float) {
		if (!pExecState->pStack->Push(pElement)) {
			return pExecState->CreateStackOverflowException("whilst converting a float to a float");
		}
		return true;
	}
	double f;

	if (elementType == StackElement_Int) {
		int64_t n = pElement->GetInt();
		f = (double)n;
	}
	else if (elementType == ObjectType_String) {
		ForthString* pString = (ForthString*)pElement->GetObject();
		string s = pString->GetContainedString();

		string convertWord = s;
		/*if (s[s.length() - 1] == 'f') {
			convertWord = s.substr(0, s.length() - 1);
		}*/
		char* end;
		f = strtod(convertWord.c_str(), &end);
		if (end == convertWord.c_str()) {
			delete pElement;
			pElement = nullptr;
			return pExecState->CreateException("Could not convert string to float");
		}
	}

	delete pElement;
	pElement = nullptr;

	if (!pExecState->pStack->Push(f)) {
		return pExecState->CreateStackOverflowException("whilst converting TOS to float");
	}
	return true;
}

// ( $ -- n)
bool PreBuiltWords::WordToInt(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType elementType = pElement->GetType();
	if (elementType != ObjectType_String) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("WordToInt requires a string");
	}

	int64_t n;
	ForthString* pString = (ForthString*)pElement->GetObject();
	string s = pString->GetContainedString();
	delete pElement;
	pElement = nullptr;
	if (s.find(".") != string::npos || s[s.length() - 1] == 'f') {
		return pExecState->CreateException("Could not convert float string to integer");
	}

	char* end;
	n = strtoll(s.c_str(), &end, 10);
	if (end == s.c_str() || *end != '\0') {
		return pExecState->CreateException("Could not convert entire string to integer");
	}
	else if (!pExecState->pStack->Push(n)) {
		return pExecState->CreateStackOverflowException("whilst converting TOS word to an integer");
	}
	return true;
}


// ( $ -- f)
bool PreBuiltWords::WordToFloat(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType elementType = pElement->GetType();
	if (elementType != ObjectType_String) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("WordToFloat requires a string");
	}

	double f;

	ForthString* pString = (ForthString*)pElement->GetObject();
	string s = pString->GetContainedString();
	delete pElement;
	pElement = nullptr;

	string convertWord = s;
	if (s[s.length() - 1] == 'f') {
		convertWord = s.substr(0, s.length() - 1);
	}
	char* end;
	f = strtod(convertWord.c_str(), &end);
	if (end == convertWord.c_str() || *end != '\0') {
		return pExecState->CreateException("Could not convert entire word to float");
	}
	else if (!pExecState->pStack->Push(f)) {
		return pExecState->CreateStackOverflowException("whilst converting TOS word to float");
	}
	return true;
}

// ( [e] -- b ) true if [e] is object, false if [e] is pter to anything or value
bool PreBuiltWords::IsObject(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType t = pElement->GetType();
	delete pElement;
	pElement = nullptr;
	StackElement* pReturnElement = new StackElement(pTS->TypeIsObject(t));
	if (!pExecState->pStack->Push(pReturnElement)) {
		return pExecState->CreateStackOverflowException("whilst determining if a type is an object");
	}
	return true;
};

// ( [e] -- b ) true if [e] is pter to object or value, false otherwise
bool PreBuiltWords::IsPter(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType t = pElement->GetType();
	delete pElement;
	pElement = nullptr;
	StackElement* pReturnElement = new StackElement(TypeSystem::IsPter(t));
	if (!pExecState->pStack->Push(pReturnElement)) {
		return pExecState->CreateStackOverflowException("whilst determining if a type is a pter");
	}
	return true;
};