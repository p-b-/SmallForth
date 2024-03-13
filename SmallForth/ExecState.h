#pragma once
#include <vector>
#include <stack>
#include <string>

class StackElement;
class DataStack;
class ReturnStack;
class ForthDict;
class InputProcessor;
class ForthWord;
class TypeSystem;
class CompileHelper;
class DebugHelper;
class WordBodyElement;

struct ExecSubState {
	WordBodyElement** pterToCFA;
	int ip;
};

class ExecState {
public:
	ExecState();
	ExecState(DataStack* pStack, ForthDict* pDict, InputProcessor* pInput, ReturnStack* pReturnStack, CompileHelper* pCompiler, DebugHelper* pDebugHelper);
	~ExecState();

	void SetCFA(WordBodyElement** pCFA, int ip);
	void NestAndSetCFA(WordBodyElement** pCFA, int ip);
	void UnnestCFA();

	WordBodyElement* GetNextWordFromCurrentBodyAndIncIP();
	WordBodyElement** GetNextWordPterFromCurrentBodyAndIncIP();
	WordBodyElement* GetWordAtOffsetFromCurrentBody(int offset);
	WordBodyElement** GetWordPterAtOffsetFromCurrentBody(int offset);
	WordBodyElement** GetNextWordFromPreviousNestedBodyAndIncIP();
	bool CurrentBodyIsInLastCompiledWord();

	bool SetPreviousBodyIP(int setToIP);
	int GetPreviousBodyIP();

	void SetExeptionIP(int ip);
	bool CreateException(const char* pzException);
	bool CreateExceptionUsingErrorNo(const char* pzException);

	std::ostream* GetStdout();
	std::ostream* GetStderr();
	std::istream* GetStdin();

	bool PushVariableValueOntoStack(const std::string& variableName);
	bool SetVariable(const std::string& variableName, int64_t setTo);
	bool SetVariable(const std::string& variableName, bool setTo);
	bool SetVariable(const std::string& variableName, double setTo);
	bool SetVariable(const std::string& variableName, ForthType setTo);
	bool GetVariable(const std::string& variableName, int64_t& variableValue);
	bool GetVariable(const std::string& variableName, double& variableValue);
	bool GetVariable(const std::string& variableName, bool& variableValue);
	bool GetVariable(const std::string& variableName, ForthType& variableValue);
	bool GetBoolTLSVariable(int index);
	int64_t GetIntTLSVariable(int index);

	bool PushConstantOntoStack(const std::string& constantName);
	bool GetConstant(const std::string& constantName, int64_t& constantValue);
	bool GetConstant(const std::string& constantName, double& constantValue);
	bool GetConstant(const std::string& constantName, bool& constantValue);
	bool GetConstant(const std::string& constantName, ForthType& constantValue);
	bool GetConstant(const std::string& constantName, RefCountedObject*& constantValue);

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

	bool ExecuteWordDirectly(const std::string& word);
	InputWord GetNextWordFromInput();

	bool NestSelfPointer(RefCountedObject* pSelf);
	bool UnnestSelfPointer();
	RefCountedObject* GetCurrentSelfPter();
	WordBodyElement** GetPointerToBoolStateVariable(int index) { return boolStates + index; }
	WordBodyElement** GetPointerToIntStateVariable(int index) { return intStates + index; }

public:
	ForthDict* pDict;
	DataStack* pStack;
	DataStack* pTempStack;
	ReturnStack* pReturnStack;
	DataStack* pSelfStack;

	std::stack<ExecSubState> subStateStack;
	WordBodyElement** pExecBody;
	int ip;

	bool nextWordIsCharLiteral;
	int delimitersAfterCurrentWord;

	bool exceptionThrown;
	char* pzException;
	int nExceptionIP;
	InputProcessor* pInputProcessor;
	bool insideStringLiteral;
	std::string commentPrompt;
	std::string stringLiteralPrompt;
	std::string compilePrompt;
	std::string interpetPrompt;

	ForthWord* pWordBeingInterpreted;

	CompileHelper* pCompiler;
	DebugHelper* pDebugger;

	static const int c_compileStateIndex = 0; // Index into int threadlocal variables
	static const int c_debugStateIndex = 1; // Index into int threadlocal variables
	static const int c_postponedExecIndex = 0; // Index into bool threadlocal variables
	static const int c_insideCommentIndex = 1; // Index into bool threadlocal variables
	static const int c_insideCommentLineIndex = 2; // Index into bool threadlocal variables

private:
	static const int c_maxStates = 10;
	WordBodyElement* boolStates[c_maxStates];
	WordBodyElement* intStates[c_maxStates];
};
