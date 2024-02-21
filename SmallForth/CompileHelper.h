#pragma once
#include <string>
class ForthWord;
class ExecState;

class CompileHelper
{
public:
	CompileHelper();
	~CompileHelper();

	// Called by other words directly, to compile a pushbvalue into the word, followed by the literal value. 
//  Works by pushing literal onto stack and then calling "," (compiles pushiliteral and TOS literal into word)
	bool CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, int64_t literalValue);
	bool CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, bool literalValue);
	bool CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, double literalValue);
	bool CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, char literalValue);
	//	bool CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, ValueType literalValue);
	bool CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, RefCountedObject* literalValue);

	void CompileLiteralIntoWordBeingCreated(ExecState* pExecState, bool literal);
	void CompileLiteralIntoWordBeingCreated(ExecState* pExecState, int64_t literal);
	void CompileLiteralIntoWordBeingCreated(ExecState* pExecState, char literal);
	void CompileLiteralIntoWordBeingCreated(ExecState* pExecState, double literal);
	void CompileWBEIntoWordBeingCreated(ExecState* pExecState, WordBodyElement* pWBE);
	void CompileTypeIntoWordBeingCreated(ExecState* pExecState, ForthType forthType);
	void CompilePterIntoWordBeingCreated(ExecState* pExecState, void* voidPter);

	bool PushDPForCurrentlyCreatingWord(ExecState* pExecState);
	bool SetLastWordCreatedToImmediate(ExecState* pExecState);
	int BodySizeOfWordUnderCreation(ExecState* pExecState);

	bool HasValidLastWordCreated() const { return this->pLastWordCreated != nullptr; }

	void AbandonWordUnderCreation();
	void StartWordCreation(const std::string& wordName);
	bool RevealWord(ExecState* pExecState, bool revealToVocNotStack);
	bool CompileWordOnStack(ExecState* pExecState);
	bool CompileWord(ExecState* pExecState, WordBodyElement** pCFA);
	bool CompileWord(ExecState* pExecState, const std::string& wordName);
	bool CompileDoesXT(ExecState* pExecState, XT does);

	bool AlterElementInWordUnderCreation(ExecState* pExecState, int addr, WordBodyElement* pReplacementElement);
	bool AlterElementInWord(ExecState* pExecState, ForthWord* pWord, int addr, WordBodyElement* pReplacementElement);
	bool ExecuteLastWordCompiled(ExecState* pExecState); 

	bool ExpandLastWordCompiledBy(ExecState* pExecState, int expandBy);

	bool LastCompiledWordHasBody(WordBodyElement** ppBody);
	void ForgetLastCompiledWord();


private:
	ForthWord* pWordUnderCreation;
	ForthWord* pLastWordCreated;
};


