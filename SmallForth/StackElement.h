#pragma once
#include <stdint.h>
#include <string>
#include "RefCountedObject.h"
class WordBodyElement;

class StackElement
{
public:
	StackElement();
	StackElement(char c);
	StackElement(int64_t n);
	StackElement(double d);
	StackElement(bool b);
	StackElement(WordBodyElement** ppWbe);
	StackElement(XT* pXt);
	StackElement(BinaryOperationType opsType);
	StackElement(const StackElement& element);
	StackElement(StackElement&& element);
	StackElement& operator=(const StackElement& element);
	StackElement& operator=(StackElement&& element);

//	StackElement(WordBodyElement*** pppWbe);
	StackElement(ForthType v);
	StackElement(RefCountedObject* pObject);
	StackElement(ForthType forthType, WordBodyElement** ppLiteral);
	StackElement(ForthType forthType, void* pter);
	~StackElement();

	void SetTo(char value);
	void SetTo(int64_t value);
	void SetTo(double value);
	void SetTo(bool value);
	void SetTo(WordBodyElement** value);
	void SetTo(XT* value);
	void SetTo(BinaryOperationType value);
	void SetTo(ForthType value);
	void SetTo(RefCountedObject* value);
	void SetTo(ForthType forthType, WordBodyElement** value);
	void SetTo(ForthType forthType, void* value);
	void RelinquishValue();


	ForthType GetType() const { return elementType; }
	char GetChar() const;
	int64_t GetInt() const;
	double GetFloat() const;
	bool GetBool() const;
	BinaryOperationType GetBinaryOpsType() const;
	ForthType GetValueType() const;
	RefCountedObject* GetObject() const;
	std::tuple<bool, ForthType, void*> GetObjectOrObjectPter() const;


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

