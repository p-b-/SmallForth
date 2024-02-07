#pragma once
#include <vector>
#include <stack>
#include <string>
using namespace std;

class StackElement;
class DataStack;
class ReturnStack;
class ForthDict;
class InputProcessor;
class ForthWord;
class TypeSystem;
class CompileHelper;

struct ExecSubState {
	WordBodyElement** pterToCFA;
	int ip;
};

class ExecState {
public:
	ExecState();
	ExecState(DataStack* pStack, ForthDict* pDict, InputProcessor* pInput, ReturnStack* pReturnStack, CompileHelper* pCompiler);
	~ExecState();

	void SetCFA(WordBodyElement** pCFA, int ip);
	void NestAndSetCFA(WordBodyElement** pCFA, int ip);
	void UnnestCFA();

	WordBodyElement* GetNextWordFromCurrentBodyAndIncIP();
	WordBodyElement** GetNextWordPterFromCurrentBodyAndIncIP();
	WordBodyElement* GetWordAtOffsetFromCurrentBody(int offset);
	WordBodyElement** GetWordPterAtOffsetFromCurrentBody(int offset);

	WordBodyElement* GetNextWordFromPreviousNestedBodyAndIncIP();
	WordBodyElement** GetNextWordPterFromPreviousNestedBodyAndIncIP();

	bool SetPreviousBodyIP(int setToIP);
	int GetPreviousBodyIP();

	void SetExeptionIP(int ip);
	bool CreateException(const char* pzException);
	bool CreateExceptionUsingErrorNo(const char* pzException);

	ostream* GetStdout();
	ostream* GetStderr();
	istream* GetStdin();

	bool PushVariableOntoStack(const string& constantName);
	bool PushConstantOntoStack(const string& constantName);
	bool GetConstant(const string& constantName, int64_t& constantValue);
	bool GetConstant(const string& constantName, double& constantValue);
	bool GetConstant(const string& constantName, bool& constantValue);
	bool GetConstant(const string& constantName, ForthType& constantValue);
	bool GetConstant(const string& constantName, RefCountedObject*& constantValue);

	bool CreateStackOverflowException();
	bool CreateStackUnderflowException();
	bool CreateStackOverflowException(const char* pzInfo);
	bool CreateStackUnderflowException(const char* pzInfo);

	bool CreateReturnStackOverflowException();
	bool CreateReturnStackUnderflowException();
	bool CreateSelfStackOverflowException();
	bool CreateSelfStackUnderflowException();
	bool CreateTempStackOverflowException();
	bool CreateTempStackUnderflowException();

	bool ExecuteWordDirectly(const string& word);
	InputWord GetNextWordFromInput();

	bool NestSelfPointer(RefCountedObject* pSelf);
	bool UnnestSelfPointer();
	RefCountedObject* GetCurrentSelfPter();
	bool* GetPointerToBoolStateVariable(int index) { return boolStates + index; }
	int64_t* GetPointerToIntStateVariable(int index) { return intStates + index; }

public:
	ForthDict* pDict;
	DataStack* pStack;
	DataStack* pTempStack;
	ReturnStack* pReturnStack;
	DataStack* pSelfStack;

	stack<ExecSubState> subStateStack;
	WordBodyElement** pExecBody;
	int ip;

	bool compileState;
	bool runtimeCompileState;
	bool executionPostponed;
	bool nextWordIsCharLiteral;
	int delimitersAfterCurrentWord;

	bool exceptionThrown;
	char* pzException;
	int nExceptionIP;
	InputProcessor* pInputProcessor;
	bool insideComment;
	bool insideStringLiteral;
	bool insideLineComment;
	string commentPrompt;
	string stringLiteralPrompt;
	string compilePrompt;
	string interpetPrompt;

	CompileHelper* pCompiler;

private:
	static const int c_maxStates = 10;
	bool boolStates[c_maxStates];
	int64_t intStates[c_maxStates];
};
