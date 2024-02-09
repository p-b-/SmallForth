#pragma once
#include <string>
#include <vector>
#include <iostream>
using namespace std;
#include "RefCountedObject.h"
#include "StackElement.h"
#include "DataStack.h"
#include "TypeSystem.h"

class StackElement;
class ExecState;
class ForthDict;

class PreBuiltWords
{
private:
	static void InitialiseWord(ForthDict* pDict, const string& wordName, XT wordCode);
	static void InitialiseImmediateWord(ForthDict* pDict, const string& wordName, XT wordCode);
	static bool InterpretForth(ExecState* pExecState, const string& toExecute);
	static bool CompileWordIntoWord(ForthDict* pDict, ForthWord* pForthWord, const string& wordName);
	static void CompileTypeIntoWord(ForthWord* pForthWord, ForthType type);
	static void CompileLiteralValueIntoWord(ForthWord* pForthWord, int value);
	static void CompileLiteralValueIntoWord(ForthWord* pForthWord, bool value);


public:
	static void RegisterWords(ForthDict* pDict);
	static void CreateSecondLevelWords(ExecState* pExecState);
	// Low level words
	static bool BuiltIn_ThreadSafeBoolVariable(ExecState* pExecState);
	static bool BuiltIn_ThreadSafeIntVariable(ExecState* pExecState);
	static bool BuiltIn_IndirectDoCol(ExecState* pExecState);
	static bool BuiltIn_CompileState(ExecState* pExecState);
	static bool BuiltIn_PostponeState(ExecState* pExecState);
	static bool BuiltIn_DoCol(ExecState* pExecState);
	static bool BuiltIn_Immediate(ExecState* pExecState);
	static bool BuiltIn_Here(ExecState* pExecState);
	static bool BuiltIn_Execute(ExecState* pExecState);
	static bool BuiltIn_ExecuteOnObject(ExecState* pExecState);
	static bool PushSelf(ExecState* pExecState);
	static bool BuiltIn_Exit(ExecState* pExecState);
	static bool BuiltIn_Jump(ExecState* pExecState);
	static bool BuiltIn_JumpOnTrue(ExecState* pExecState);
	static bool BuiltIn_JumpOnFalse(ExecState* pExecState);
	static bool BuiltIn_PokeIntegerInWord(ExecState* pExecState);
	static bool PushRefCount(ExecState* pExecState);
	static bool BuiltIn_WordCFAFromInputStream(ExecState* pExecState);
	static bool BuiltIn_WordCFAFromDefinition(ExecState* pExecState);
	static bool Quit(ExecState* pExecState);
	static bool BuiltIn_Find(ExecState* pExecState);


	// Altering interpretation words
	static bool BuiltIn_ParenthesisCommentStart(ExecState* pExecState);
	static bool BuiltIn_ParenthesisCommentEnd(ExecState* pExecState);
	static bool BuiltIn_LineCommentStart(ExecState* pExecState);

	// Definitions 
	static bool BuiltIn_Allot(ExecState* pExecState);
	static bool BuiltIn_Reveal(ExecState* pExecState);
	static bool BuiltIn_RevealToStack(ExecState* pExecState);
	static bool BuiltIn_Does(ExecState* pExecState);
	static bool BuiltIn_Create(ExecState* pExecState);
	static bool BuiltIn_AddWordToObject(ExecState* pExecState);
	static bool BuiltIn_StartCompilation(ExecState* pExecState);
	static bool BuiltIn_EndCompilation(ExecState* pExecState);
	static bool BuiltIn_Forget(ExecState* pExecState);
	// When compiling, the next word won't be executed, it will be compiled into the new word
	//  Even if executing a docol, the postpone command will compile the next word into the command being compiled (if the docol is involved in compiling a word).
	static bool BuiltIn_Postpone(ExecState* pExecState);
	static bool BuiltIn_PostponePostpone(ExecState* pExecState);
	static bool BuiltIn_ResetPostponeState(ExecState* pExecState);

	static bool BuiltIn_CallObject(ExecState* pExecState);
	static bool BuiltIn_Construct(ExecState* pExecState);
	static bool BuiltIn_DefineObject(ExecState* pExecState);

	// Strings and string output
	static bool ToString(ExecState* pExecState); // ( e -- $) formats element e and converts to string $
	
	// Value type manipulation
	static bool ToChar(ExecState* pExecState); // ( n -- c)
	static bool ToInt(ExecState* pExecState); // ( $/c/f -- n)
	static bool ToFloat(ExecState* pExecState); // ( $/n -- f)
	static bool WordToInt(ExecState* pExecState); // ( $ -- n)
	static bool WordToFloat(ExecState* pExecState); // ( $ -- f)

	static bool IsObject(ExecState* pExecState); // ( [e] -- b ) true if [e] is object, false if [e] is pter to anything or value
	static bool IsPter(ExecState* pExecState);

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
	static bool BuiltIn_StackSize(ExecState* pExecState);
	static bool BuiltIn_RStackSize(ExecState* pExecState);
	static bool BuiltIn_TStackSize(ExecState* pExecState);
	static bool BuiltIn_PrintStackTop(ExecState* pExecState);
	static bool BuiltIn_Emit(ExecState* pExecState);

	// Binary operators
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

	// Logical operators
	static bool BuiltIn_Not(ExecState* pExecState);
	static bool BuiltIn_Or(ExecState* pExecState);
	static bool BuiltIn_And(ExecState* pExecState);
	static bool BuiltIn_Xor(ExecState* pExecState);

	// Maths operators
	static bool BuiltIn_Power(ExecState* pExecState);
	static bool BuiltIn_Cos(ExecState* pExecState);
	static bool BuiltIn_Sin(ExecState* pExecState);
	static bool BuiltIn_Tan(ExecState* pExecState);
	static bool BuiltIn_Arccosine(ExecState* pExecState);
	static bool BuiltIn_Arcsine(ExecState* pExecState);
	static bool BuiltIn_Arctan(ExecState* pExecState);
	static bool BuiltIn_Sqrt(ExecState* pExecState);

	// Pointers
	static bool BuiltIn_Peek(ExecState* pExecState);
	static bool BuiltIn_Poke(ExecState* pExecState);
	static bool BuiltIn_PushFloatPter(ExecState* pExecState);
	static bool BuiltIn_PushIntPter(ExecState* pExecState);
	static bool BuiltIn_PushCharPter(ExecState* pExecState);
	static bool BuiltIn_PushBoolPter(ExecState* pExecState);
	static bool BuiltIn_PushTypePter(ExecState* pExecState);
	static bool BuiltIn_PushPter(ExecState* pExecState);

	// Variables, constants and literals
	static bool BuiltIn_Variable(ExecState* pExecState);
	static bool BuiltIn_True(ExecState* pExecState);
	static bool BuiltIn_False(ExecState* pExecState);
	static bool BuiltIn_PushType(ExecState* pExecState);
	static bool BuiltIn_TypeFromInt(ExecState* pExecState);
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
	static bool BuiltIn_LiteralNoPush(ExecState* pExecState);

	// When compiling, mark next word in stream as a character literal
	static bool BuiltIn_CharLiteral(ExecState* pExecState);
	static bool BuiltIn_FetchLiteral(ExecState* pExecState);
	static bool BuiltIn_PushUpcomingLiteral(ExecState* pExecState);
	static bool BuiltIn_StringLiteral(ExecState* pExecState);

	// Control flow
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
};

