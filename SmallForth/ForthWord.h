#pragma once
#include <string>
#include <vector>
#include "RefCountedObject.h"

class StackElement;
class ExecState; 

class ForthWord : public RefCountedObject
{
public:
	ForthWord(const std::string& name);
	ForthWord(const std::string& name, XT firstXT);
	void GrowByAndAdd(int growBy, WordBodyElement* pElement);
	void GrowBy(int growBy);
	void CompileXTIntoWord(XT xt, int pos = -1);
	void CompileCFAPterIntoWord(WordBodyElement** pterToBody);
	void CompileLiteralIntoWord(bool literal);
	void CompileLiteralIntoWord(char literal);
	void CompileLiteralIntoWord(int64_t literal);
	void CompileLiteralIntoWord(double literal);
	void CompileLiteralIntoWord(WordBodyElement* literal);
	void CompileTypeIntoWord(ForthType forthType);
	void CompilePterIntoWord(void* pter);

	void AddElementToWord(WordBodyElement* pElement);
	void ExpandBy(int expandBy);
	void SetWordVisibility(bool visibleFlag) { visible = visibleFlag; }
	bool Visible() const { return visible; }

	WordBodyElement** GetPterToBody() const { return body; }
	int GetBodySize() const { return bodySize; }

	std::string GetName() { return this->name; }
	bool GetImmediate() const { return this->immediate; }
	void SetImmediate(bool flag) { this->immediate = flag; }

	virtual std::string GetObjectType();
	virtual bool ToString(ExecState* pExecState) const;
	virtual bool InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke);

public:
	static bool BuiltIn_DescribeWord(ExecState* pExecState);

	static bool BuiltInHelper_BinaryOperation(ExecState* pExecState, BinaryOperationType opType);
	static bool BuiltInHelper_ObjectBinaryOperation(ExecState* pExecState, BinaryOperationType opType, const StackElement& element1, const StackElement& element2);
	static void BuiltInHelper_DeleteOperands(StackElement*& pElement1, StackElement*& pElement2);
	static void BuiltInHelper_DeleteStackElement(StackElement*& pElement);
	static bool BuiltInHelper_GetOneTempStackElement(ExecState* pExecState, StackElement*& pElement1);
	static bool BuiltInHelper_GetOneStackElement(ExecState* pExecState, StackElement*& pElement1);
	static bool BuiltInHelper_UpdateForwardJump(ExecState* pExecState);
	static bool BuiltInHelper_FetchLiteralWithOffset(ExecState* pExecState, int offset);
	static bool BuiltInHelper_CompileTOSLiteral(ExecState* pExecState, bool includePushWord);

private:
	std::string name;

	int bodySize;
	WordBodyElement** body;
	bool immediate;
	bool visible;
};

