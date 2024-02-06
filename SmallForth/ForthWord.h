#pragma once
#include <string>
#include <vector>
using namespace std;
#include "RefCountedObject.h"

class StackElement;
class ExecState; 

class ForthWord : public RefCountedObject
{
public:
	ForthWord(const string& name);
	ForthWord(const string& name, XT firstXT);
	void GrowByAndAdd(int growBy, WordBodyElement* pElement);
	void GrowBy(int growBy);
	void CompileXTIntoWord(XT xt, int pos = -1);
	void CompileCFAPterIntoWord(WordBodyElement** pterToBody);
	void CompileLiteralIntoWord(bool literal);
	void CompileLiteralIntoWord(char literal);
	void CompileLiteralIntoWord(int64_t literal);
	void CompileLiteralIntoWord(double literal);
	void CompileLiteralIntoWord(ValueType literal);
//	void CompileLiteralIntoWord(RefCountedObject* literal);
	void CompileLiteralIntoWord(WordBodyElement* literal);
	void CompileTypeIntoWord(ForthType forthType);
	void CompilePterIntoWord(void* pter);

	void AddElementToWord(WordBodyElement* pElement);
	void ExpandBy(int expandBy);
	void SetWordVisibility(bool visibleFlag) { visible = visibleFlag; }
	bool Visible() const { return visible; }

	WordBodyElement** GetPterToBody() const { return body; }
	int GetBodySize() const { return bodySize; }

	string GetName() { return this->name; }
	bool GetImmediate() const { return this->immediate; }
	void SetImmediate(bool flag) { this->immediate = flag; }

	virtual string GetObjectType();
	virtual bool ToString(ExecState* pExecState) const;
	virtual bool InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke);

public:
	static bool BuiltIn_DescribeWord(ExecState* pExecState);

	static bool BuiltInHelper_BinaryOperation(ExecState* pExecState, BinaryOperationType opType);
	static bool BuiltInHelper_ObjectBinaryOperation(ExecState* pExecState, BinaryOperationType opType, StackElement* pElement1, StackElement* pElement2);
	static void BuiltInHelper_DeleteOperands(StackElement*& pElement1, StackElement*& pElement2);
	static void BuiltInHelper_DeleteStackElement(StackElement*& pElement);
	static bool BuiltInHelper_GetOneTempStackElement(ExecState* pExecState, StackElement*& pElement1);
	static bool BuiltInHelper_GetOneStackElement(ExecState* pExecState, StackElement*& pElement1);
	static bool BuiltInHelper_GetTwoStackElements(ExecState* pExecState, StackElement*& pElement1, StackElement*& pElement2);
	static bool BuiltInHelper_GetThreeStackElements(ExecState* pExecState, StackElement*& pElement1, StackElement*& pElement2, StackElement*& pElement3);
	static bool BuiltInHelper_UpdateForwardJump(ExecState* pExecState);
	static bool BuiltInHelper_FetchLiteralWithOffset(ExecState* pExecState, int offset);
	static bool BuiltInHelper_CompileTOSLiteral(ExecState* pExecState, bool includePushWord);

private:
	string name;

	int bodySize;
	WordBodyElement** body;
	bool immediate;
	bool visible;
};

