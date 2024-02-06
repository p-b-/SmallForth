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
	static bool BuiltIn_Find(ExecState* pExecState);
	static bool BuiltIn_ErrantWord(ExecState* pExecState);
	static bool BuiltIn_ParenthesisCommentStart(ExecState* pExecState);
	static bool BuiltIn_ParenthesisCommentEnd(ExecState* pExecState);
	static bool BuiltIn_LineCommentStart(ExecState* pExecState);
	static bool BuiltIn_IndirectDoCol(ExecState* pExecState);

	static bool BuiltIn_DoCol(ExecState* pExecState);
	static bool BuiltIn_Exit(ExecState* pExecState);
	static bool BuiltIn_Jump(ExecState* pExecState);
	static bool BuiltIn_JumpOnTrue(ExecState* pExecState);
	static bool BuiltIn_JumpOnFalse(ExecState* pExecState);
	static bool BuiltIn_Immediate(ExecState* pExecState);
	static bool BuiltIn_Here(ExecState* pExecState);
	static bool BuiltIn_Execute(ExecState* pExecState);
	static bool BuiltIn_ExecuteOnObject(ExecState* pExecState);

	// Stack operations
	static bool BuiltIn_Dup(ExecState* pExecState);
	static bool BuiltIn_Swap(ExecState* pExecState);
	static bool BuiltIn_Drop(ExecState* pExecState);
	static bool BuiltIn_Over(ExecState* pExecState);
	static bool BuiltIn_Rot(ExecState* pExecState);
	static bool BuiltIn_ReverseRot(ExecState* pExecState);
	static bool BuiltIn_PushDataStackToReturnStack(ExecState* pExecState);
	static bool BuiltIn_PushReturnStackToDataStack(ExecState* pExecState);
	static bool BuiltIn_PushDataStackToTempStack(ExecState* pExecState);
	static bool BuiltIn_PushTempStackToDataStack(ExecState* pExecState);

	static bool BuiltIn_Add(ExecState* pExecState);
	static bool BuiltIn_Subtract(ExecState* pExecState);
	static bool BuiltIn_Multiply(ExecState* pExecState);
	static bool BuiltIn_Divide(ExecState* pExecState);
	static bool BuiltIn_Modulus(ExecState* pExecState);

	static bool BuiltIn_LessThan(ExecState* pExecState);
	static bool BuiltIn_LessThanOrEquals(ExecState* pExecState);
	static bool BuiltIn_Equals(ExecState* pExecState);
	static bool BuiltIn_NotEquals(ExecState* pExecState);
	static bool BuiltIn_GreaterThanOrEquals(ExecState* pExecState);
	static bool BuiltIn_GreaterThan(ExecState* pExecState);

	static bool BuiltIn_Not(ExecState* pExecState);
	static bool BuiltIn_Or(ExecState* pExecState);
	static bool BuiltIn_And(ExecState* pExecState);
	static bool BuiltIn_Xor(ExecState* pExecState);

	static bool BuiltIn_True(ExecState* pExecState);
	static bool BuiltIn_False(ExecState* pExecState);
	static bool BuiltIn_PushType(ExecState* pExecState);

	static bool BuiltIn_PokeIntegerInWord(ExecState* pExecState);

	static bool BuiltIn_Peek(ExecState* pExecState);
	static bool BuiltIn_Poke(ExecState* pExecState);
	static bool BuiltIn_PushFloatPter(ExecState* pExecState);
	static bool BuiltIn_PushIntPter(ExecState* pExecState);
	static bool BuiltIn_PushCharPter(ExecState* pExecState);
	static bool BuiltIn_PushBoolPter(ExecState* pExecState);
	static bool BuiltIn_PushTypePter(ExecState* pExecState);
	static bool BuiltIn_PushPter(ExecState* pExecState);

	static bool BuiltIn_Constant(ExecState* pExecState);
	static bool BuiltIn_Variable(ExecState* pExecState);
	static bool BuiltIn_TypeFromInt(ExecState* pExecState);

	static bool BuiltIn_Reveal(ExecState* pExecState);
	static bool BuiltIn_RevealToStack(ExecState* pExecState);
	static bool BuiltIn_Forget(ExecState* pExecState);
	static bool BuiltIn_Does(ExecState* pExecState);

	static bool BuiltIn_Create(ExecState* pExecState);
	static bool BuiltIn_AddWordToObject(ExecState* pExecState);
	static bool BuiltIn_StartCompilation(ExecState* pExecState);
	static bool BuiltIn_EndCompilation(ExecState* pExecState);
	// When compiling, the next word won't be executed, it will be compiled into the new word
	//  Even if executing a docol, the postpone command will compile the next word into the command being compiled (if the docol is involved in compiling a word).
	static bool BuiltIn_Postpone(ExecState* pExecState);

	// Is used when input stream adds a literal to stack, before deciding compiling or executing.  
	//  If executing, the literal is already on the stack, so no need for further action
	//  If compiling, this is called to take the literal off the stack and to add to word being defined
	// Note, also used when creating defining words that need a literal inside all newly minted words from the defining word:
	// : :a create postpone ] postpone postpone docol [char] a postpone literal ;
	// :a printa . ;
	// printa
	// a
	//
	// '[char] a' compiles the 'BuiltIn_PushUpcomingCharLiteral' and 'a' into the defining word
	//  on executing the defining word, 'BuiltIn_PushUpcomingCharLiteral' and 'a' push 'a' onto the stack.  
	//  'literal'/BuiltIn_Literal takes it off the stack and compiles it into the newly minted word
	static bool BuiltIn_Literal(ExecState* pExecState);
	// When compiling, mark next word in stream as a character literal
	static bool BuiltIn_CharLiteral(ExecState* pExecState);
	static bool BuiltIn_FetchLiteral(ExecState* pExecState);

	static bool BuiltIn_PushUpcomingLiteral(ExecState* pExecState);

	static bool BuiltIn_WordCFAFromInputStream(ExecState* pExecState);
	static bool BuiltIn_WordCFAFromDefinition(ExecState* pExecState);

	static bool BuiltIn_StackSize(ExecState* pExecState);
	static bool BuiltIn_RStackSize(ExecState* pExecState);
	static bool BuiltIn_TStackSize(ExecState* pExecState);
	static bool BuiltIn_PrintStackTop(ExecState* pExecState);
	static bool BuiltIn_Emit(ExecState* pExecState);

	static bool BuiltIn_DescribeWord(ExecState* pExecState);

	static bool BuiltIn_Begin(ExecState* pExecState);
	static bool BuiltIn_Until(ExecState* pExecState);
	static bool BuiltIn_Again(ExecState* pExecState);
	static bool BuiltIn_While(ExecState* pExecState);
	static bool BuiltIn_Repeat(ExecState* pExecState);
	static bool BuiltIn_Do(ExecState* pExecState);
	static bool BuiltIn_Loop(ExecState* pExecState);
	static bool BuiltIn_Leave(ExecState* pExecState);
	static bool BuiltIn_PlusLoop(ExecState* pExecState);
	static bool BuiltIn_If(ExecState* pExecState);
	static bool BuiltIn_Then(ExecState* pExecState);
	static bool BuiltIn_Else(ExecState* pExecState);

	static bool BuiltIn_StringLiteral(ExecState* pExecState);
	static bool BuiltIn_CallObject(ExecState* pExecState);
	static bool BuiltIn_Construct(ExecState* pExecState);
	static bool BuiltIn_DefineObject(ExecState* pExecState);

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

