#pragma once
#include <stdint.h>
#include <string>
#include "RefCountedObject.h"

using namespace std;

class StackElement
{
public:
	StackElement(char c);
	StackElement(int64_t n);
	StackElement(double d);
	StackElement(bool b);
	StackElement(WordBodyElement** ppWbe);
	StackElement(XT* pXt);
	StackElement(BinaryOperationType opsType);
	StackElement(const StackElement& element);
//	StackElement(WordBodyElement*** pppWbe);
	StackElement(ForthType v);
	StackElement(RefCountedObject* pObject);
	StackElement(ForthType forthType, WordBodyElement* pLiteral);
	StackElement(ForthType forthType, void* pter);
	~StackElement();


	ForthType GetType() const { return elementType; }
	char GetChar() const;
	int64_t GetInt() const;
	double GetFloat() const;
	bool GetBool() const;
	BinaryOperationType GetBinaryOpsType() const;
	ForthType GetValueType() const;
	RefCountedObject* GetObject() const;
	tuple<bool, ForthType, void*> GetObjectOrObjectPter() const;


	StackElement* GetDerefedPterValueAsStackElement() const;
	bool PokeIntoContainedPter(ExecState *pExecState, StackElement* pValueElement);
	WordBodyElement* GetValueAsWordBodyElement() const;

	WordBodyElement** GetWordBodyElement() const;
	//WordBodyElement*** GetWordBodyElementPter() const;

	bool IsPter() const;
	bool IsLiteral() const;
	void* GetContainedPter() const { return valuePter; }
	bool ContainsNumericType() const;


	bool ToString(ExecState* pExecState) const;

private: 
	bool PokeObjectIntoContainedPter(ExecState* pExecState, StackElement* pValueElement);
	bool PokeValueIntoContainedPter(ExecState* pExecState, StackElement* pValueElement);

	//bool ValueTypeToString(ExecState* pExecState, ForthType forthType, const void* pter) const;

private:

	ForthType elementType;
	union {
		char valueChar;
		int64_t valueInt64;
		double valueDouble;
		bool valueBool;
		ForthType valueType;
		WordBodyElement** valueWordBodyPter;
		WordBodyElement*** valueWordBodyPterPter;
		XT* valueXTPter;
		RefCountedObject* valueRefObject;
		RefCountedObject** valueRefObjectPter;
		void* valuePter;
		BinaryOperationType valueBinaryOpsType;
	};
};

