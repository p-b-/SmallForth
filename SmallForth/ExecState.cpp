#include <iostream>
using namespace std;
#include "ForthDefs.h"
#include "ExecState.h"
#include "DataStack.h"
#include "ForthDict.h"
#include "ForthWord.h"
#include "InputProcessor.h"
#include "TypeSystem.h"
#include "ForthFile.h"
#include "CompileHelper.h"

ExecState::ExecState() 
: ExecState(nullptr, nullptr, nullptr, nullptr, nullptr) {
}

ExecState::ExecState(DataStack* pStack, ForthDict* pDict, InputProcessor* pInput, ReturnStack* pReturnStack, CompileHelper* pCompiler) {
	this->pDict = pDict;
	this->pDict->IncReference();
	this->pStack = pStack;
	this->pInputProcessor = pInput;
	this->pReturnStack = pReturnStack;
	this->pCompiler = pCompiler;
	this->pExecBody = nullptr;
	this->ip = 0;
	this->exceptionThrown = false;
	this->pzException = nullptr;
	this->nextWordIsCharLiteral = false;
	this->pTempStack = new DataStack(40);
	this->pSelfStack = new DataStack(40);
	
	this->commentPrompt = "C> ";
	this->compilePrompt = ":> ";
	this->interpetPrompt = "? ";
	this->stringLiteralPrompt = "\"> ";

	for (int n = 0; n < c_maxStates; ++n) {
		WordBodyElement* pElementBool = new WordBodyElement();
		pElementBool->wordElement_bool = false;
		boolStates[n] = pElementBool;
		pElementBool = nullptr;

		WordBodyElement* pElementInt = new WordBodyElement();
		pElementInt->wordElement_int = 0;
		intStates[n] = pElementInt;
		pElementInt = nullptr;
	}
}

ExecState::~ExecState() {
	delete this->pTempStack;
	this->pTempStack = nullptr;
	delete this->pSelfStack;
	this->pSelfStack = nullptr;

	this->pDict->DecReference();
	this->pDict = nullptr;
}


void ExecState::SetCFA(WordBodyElement** pCFA, int ip) {
	this->pExecBody = pCFA;
	this->ip = ip;
}

void ExecState::NestAndSetCFA(WordBodyElement** pCFA, int ip) {
	ExecSubState subState;
	subState.ip = this->ip;
	subState.pterToCFA = this->pExecBody;
	this->subStateStack.push(subState);
	SetCFA(pCFA, ip);
}

void ExecState::UnnestCFA() {
	ExecSubState unnestedSubstate = this->subStateStack.top();
	this->subStateStack.pop();
	SetCFA(unnestedSubstate.pterToCFA, unnestedSubstate.ip);
}

WordBodyElement* ExecState::GetNextWordFromCurrentBodyAndIncIP() {
	WordBodyElement* pWBE = this->pExecBody[this->ip];
	this->ip++;
	return pWBE;
}

WordBodyElement** ExecState::GetNextWordPterFromCurrentBodyAndIncIP() {
	WordBodyElement** ppWBE = this->pExecBody+this->ip;
	this->ip++;
	return ppWBE;
}

WordBodyElement* ExecState::GetWordAtOffsetFromCurrentBody(int offset) {
	WordBodyElement* pWBE = this->pExecBody[offset];
	return pWBE;
}

WordBodyElement** ExecState::GetWordPterAtOffsetFromCurrentBody(int offset) {
	WordBodyElement** ppWBE = this->pExecBody + offset;
	return ppWBE;
}

WordBodyElement** ExecState::GetNextWordFromPreviousNestedBodyAndIncIP() {
	int currentIp = ip;
	WordBodyElement** currentCFA = pExecBody;
	UnnestCFA();
	WordBodyElement** ppWBE = this->pExecBody+this->ip;
	this->ip++;
	NestAndSetCFA(currentCFA, currentIp);
	return ppWBE;
}

bool ExecState::CurrentBodyIsInLastCompiledWord() {
	return pCompiler->LastCompiledWordHasBody(this->pExecBody);
}

// This is used to jump.  As jump is inside it's own body, altering the IP would not have any affect, have to alter the IP of 
//  the body that nested the jump
bool ExecState::SetPreviousBodyIP(int setToIP) {
	int currentIp = ip;
	WordBodyElement** currentCFA = pExecBody;
	UnnestCFA();
	//WordBodyElement* pWBE = this->pExecBody[this->ip];
	this->ip = setToIP;
	NestAndSetCFA(currentCFA, currentIp);
	return true;
}

int ExecState::GetPreviousBodyIP() {
	int currentIp = ip;
	WordBodyElement** currentCFA = pExecBody;
	UnnestCFA();
	//WordBodyElement* pWBE = this->pExecBody[this->ip];
	int ipToReturn = this->ip;
	NestAndSetCFA(currentCFA, currentIp);
	return ipToReturn;
}

bool ExecState::CreateStackOverflowException() {
	return CreateException("Stack overflow");
}

bool ExecState::CreateStackOverflowException(const char* pzInfo) {
	string exceptionString = "Stack overflow: ";
	exceptionString.append(pzInfo);
	return CreateException(exceptionString.c_str());
}


bool ExecState::CreateStackUnderflowException() {
	return CreateException("Stack underflow");
}

bool ExecState::CreateStackUnderflowException(const char* pzInfo) {
	string exceptionString = "Stack underflow: ";
	exceptionString.append(pzInfo);
	return CreateException(exceptionString.c_str());
}


bool ExecState::CreateReturnStackOverflowException() {
	return CreateException("Return stack overflow");
}

bool ExecState::CreateReturnStackUnderflowException() {
	return CreateException("Return stack underflow");
}

bool ExecState::CreateSelfStackOverflowException() {
	return CreateException("Self pointer stack overflow");
}

bool ExecState::CreateSelfStackUnderflowException() {
	return CreateException("Self pointer stack underflow");
}

bool ExecState::CreateTempStackOverflowException() {
	return CreateException("Temp stack overflow");
}

bool ExecState::CreateTempStackUnderflowException() {
	return CreateException("Temp stack underflow");
}

void ExecState::SetExeptionIP(int ip) {
	this->nExceptionIP = ip;
}

bool ExecState::CreateException(const char* pzException) {
	this->nExceptionIP = -1;
	this->exceptionThrown = true;
	if (this->pzException != nullptr) {
		delete[] this->pzException;
		this->pzException = nullptr;
	}
	int size = (int)strlen(pzException)+1;
	this->pzException = new char[size];
	strcpy_s(this->pzException, size, pzException);
	return false;
}

bool ExecState::CreateExceptionUsingErrorNo(const char* pzException) {
	const int size = 512;
	int sizeRemaining = size;
	char buff[size];
	char* buffPter = buff;
	if (strcpy_s(buffPter, size, pzException) != 0) {
		buffPter = buff;
		sizeRemaining = size;
	}
	else {
		int len = (int)strlen(buff);
		sizeRemaining -= len;
		buffPter += len;
	}

	strerror_s(buffPter, sizeRemaining, errno);
	return CreateException(buff);
}


ostream* ExecState::GetStdout() {
	RefCountedObject* pObj;
	ostream* pStdoutStream = &cout;
	if (this->GetConstant("stdout", pObj)) {
		ForthFile* pFile = (ForthFile*)pObj;
		pStdoutStream = pFile->GetContainedStream();
		pFile->DecReference();
		pFile = nullptr;
	}
	return pStdoutStream;
}

ostream* ExecState::GetStderr() {
	RefCountedObject* pObj;
	ostream* pStderrStream = &cerr;
	if (this->GetConstant("stderr", pObj)) {
		ForthFile* pFile = (ForthFile*)pObj;
		pStderrStream = pFile->GetContainedStream();
		pFile->DecReference();
		pFile = nullptr;
	}
	return pStderrStream;
}

istream* ExecState::GetStdin() {
	RefCountedObject* pObj;
	istream* pStdinStream = &cin;
	if (this->GetConstant("stdin", pObj)) {
		ForthFile* pFile = (ForthFile*)pObj;
		pStdinStream = pFile->GetContainedStream();
		pFile->DecReference();
		pFile = nullptr;
	}
	return pStdinStream;
}

bool ExecState::PushVariableValueOntoStack(const string& variableName) {
	if (!this->ExecuteWordDirectly(variableName)) {
		this->exceptionThrown = false;
		return false;
	}
	if (!this->ExecuteWordDirectly("@")) {
		this->exceptionThrown = false;
		return false;
	}
	return true;
}

bool ExecState::SetVariable(const string& variableName, int64_t setTo) {
	if (!pStack->Push(setTo)) {
		return CreateStackOverflowException("whilst setting an integer variable");
	}
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	if (!this->ExecuteWordDirectly("!")) {
		return false;
	}
	return true;
}

bool ExecState::SetVariable(const string& variableName, bool setTo) {
	if (!pStack->Push(setTo)) {
		return CreateStackOverflowException("whilst setting a bool variable");
	}
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	if (!this->ExecuteWordDirectly("!")) {
		return false;
	}
	return true;
}

bool ExecState::SetVariable(const string& variableName, double setTo) {
	if (!pStack->Push(setTo)) {
		return CreateStackOverflowException("whilst setting a float variable");
	}
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	if (!this->ExecuteWordDirectly("!")) {
		return false;
	}
	return true;
}

bool ExecState::SetVariable(const string& variableName, ForthType setTo) {
	if (!pStack->Push(setTo)) {
		return CreateStackOverflowException("whilst setting a type variable");
	}
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	if (!this->ExecuteWordDirectly("!")) {
		return false;
	}
	return true;
}

bool ExecState::GetVariable(const string& variableName, int64_t& variableValue) { 
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	StackElement* pElementVariableValuePter = this->pStack->TopElement();
	
	if (pElementVariableValuePter->GetType() == pTS->CreatePointerTypeTo(StackElement_Int)) {
		pElementVariableValuePter = nullptr;
		if (this->ExecuteWordDirectly("@")) {
			StackElement* pElementVariableValue;
			pElementVariableValue = pStack->Pull();
			if (pElementVariableValue == nullptr) {
				return CreateStackOverflowException("whilst getting variable");
			}
			variableValue = pElementVariableValue->GetInt();
			delete pElementVariableValue;
			pElementVariableValue = nullptr;
			return true;
		}
	}
	return false;
}

bool ExecState::GetVariable(const string& variableName, double& variableValue) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	StackElement* pElementVariableValuePter = this->pStack->TopElement();

	if (pElementVariableValuePter->GetType() == pTS->CreatePointerTypeTo(StackElement_Float)) {
		pElementVariableValuePter = nullptr;
		if (this->ExecuteWordDirectly("@")) {
			StackElement* pElementVariableValue;
			pElementVariableValue = pStack->Pull();
			if (pElementVariableValue == nullptr) {
				return CreateStackOverflowException("whilst getting variable");
			}
			variableValue = pElementVariableValue->GetFloat();
			delete pElementVariableValue;
			pElementVariableValue = nullptr;
			return true;
		}
	}
	return false;
}

bool ExecState::GetVariable(const string& variableName, bool& variableValue) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	StackElement* pElementVariableValuePter = this->pStack->TopElement();

	if (pElementVariableValuePter->GetType() == pTS->CreatePointerTypeTo(StackElement_Bool)) {
		pElementVariableValuePter = nullptr;
		if (this->ExecuteWordDirectly("@")) {
			StackElement* pElementVariableValue;
			pElementVariableValue = pStack->Pull();
			if (pElementVariableValue == nullptr) {
				return CreateStackOverflowException("whilst getting variable");
			}
			variableValue = pElementVariableValue->GetBool();
			delete pElementVariableValue;
			pElementVariableValue = nullptr;
			return true;
		}
	}
	return false;
}

bool ExecState::GetVariable(const string& variableName, ForthType& variableValue) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	StackElement* pElementVariableValuePter = this->pStack->TopElement();

	if (pElementVariableValuePter->GetType() == pTS->CreatePointerTypeTo(StackElement_Type)) {
		pElementVariableValuePter = nullptr;
		if (this->ExecuteWordDirectly("@")) {
			StackElement* pElementVariableValue;
			pElementVariableValue = pStack->Pull();
			if (pElementVariableValue == nullptr) {
				return CreateStackOverflowException("whilst getting variable");
			}
			variableValue = pElementVariableValue->GetValueType();
			delete pElementVariableValue;
			pElementVariableValue = nullptr;
			return true;
		}
	}
	return false;
}

bool ExecState::PushConstantOntoStack(const string& constantName) {
	if (!this->ExecuteWordDirectly(constantName)) {
		this->exceptionThrown = false;
		return false;
	}
	return true;
}

bool ExecState::GetConstant(const string& constantName, int64_t& constantValue) {
	if (!PushConstantOntoStack(constantName)) {
		return false;
	}
	StackElement* pElementConstantValue = this->pStack->Pull();
	bool success = false;
	if (pElementConstantValue->GetType() == StackElement_Int)
	{
		constantValue = pElementConstantValue->GetInt();
		success = true;
	}
	delete pElementConstantValue;
	pElementConstantValue = nullptr;

	return success;
}

bool ExecState::GetConstant(const string& constantName, double& constantValue) {
	if (!PushConstantOntoStack(constantName)) {
		return false;
	}
	StackElement* pElementConstantValue = this->pStack->Pull();
	bool success = false;
	if (pElementConstantValue->GetType() == StackElement_Float)
	{
		constantValue = pElementConstantValue->GetFloat();
		success = true;
	}
	delete pElementConstantValue;
	pElementConstantValue = nullptr;

	return success;
}

bool ExecState::GetConstant(const string& constantName, bool& constantValue) {
	if (!PushConstantOntoStack(constantName)) {
		return false;
	}
	StackElement* pElementConstantValue = this->pStack->Pull();
	bool success = false;
	if (pElementConstantValue->GetType() == StackElement_Bool)
	{
		constantValue = pElementConstantValue->GetBool();
		success = true;
	}
	delete pElementConstantValue;
	pElementConstantValue = nullptr;

	return success;
}

bool ExecState::GetConstant(const string& constantName, ForthType& constantValue) {
	if (!PushConstantOntoStack(constantName)) {
		return false;
	}
	StackElement* pElementConstantValue = this->pStack->Pull();
	bool success = false;
	if (pElementConstantValue->GetType() == StackElement_Type)
	{
		constantValue = pElementConstantValue->GetValueType();
		success = true;
	}
	delete pElementConstantValue;
	pElementConstantValue = nullptr;

	return success;
}

bool ExecState::GetConstant(const string& constantName, RefCountedObject*& constantValue) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!PushConstantOntoStack(constantName)) {
		return false;
	}
	StackElement* pElementConstantValue = this->pStack->Pull();
	bool success = false;

	ForthType t = pElementConstantValue->GetType();
	if (pTS->TypeIsObject(t)) {
		constantValue = pElementConstantValue->GetObject();
		constantValue->IncReference();
		success = true;
	}
	delete pElementConstantValue;
	pElementConstantValue = nullptr;

	return success;
}


bool ExecState::ExecuteWordDirectly(const string& word) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	ForthWord* pWord = pTS->FindWordInTOSWord(this, word);
	bool executeOnTOSObject = false;
	if (pWord != nullptr) {
		executeOnTOSObject = true;
	}
	else {
		pWord = this->pDict->FindWord(word);
	}
	if (pWord == nullptr) {
		return CreateException("Could not find word in dictionary");
	}

	WordBodyElement** ppExecBody = pWord->GetPterToBody();
	XT executeXT = ppExecBody[0]->wordElement_XT;

	if (executeOnTOSObject) {
		StackElement* pElementObjectToExecOn = this->pStack->Pull();
		RefCountedObject* pObjToExecOn = pElementObjectToExecOn->GetObject();
		this->NestSelfPointer(pObjToExecOn);
		delete pElementObjectToExecOn;
		pElementObjectToExecOn = nullptr;
	}

	NestAndSetCFA(ppExecBody, 1);
	bool returnResult = true;
	try {
		returnResult = executeXT(this);
	}
	catch (...) {
		// TODO Create a better message here
		//      Or do not catch the exception at all
		CreateException("Execution caused exception");
		if (executeOnTOSObject) {
			this->UnnestSelfPointer();
		}
		UnnestCFA();
		throw;
	}
	if (executeOnTOSObject) {
		this->UnnestSelfPointer();
	}
	UnnestCFA();
	return returnResult;
}

InputWord ExecState::GetNextWordFromInput() {
	return pInputProcessor->GetNextWord(this);
}

bool ExecState::NestSelfPointer(RefCountedObject* pSelf) {
	if (!this->pSelfStack->Push(pSelf)) {
		return CreateSelfStackOverflowException();
	}
	return true;
}

bool ExecState::UnnestSelfPointer() {
	StackElement* pElement = this->pSelfStack->Pull();
	if (pElement == nullptr) {
		return CreateSelfStackUnderflowException();
	}
	delete pElement;
	pElement = nullptr;
	return true;
}

RefCountedObject* ExecState::GetCurrentSelfPter() {
	if (this->pSelfStack->Count() == 0) {
		return nullptr;
	}
	StackElement* pElement = this->pSelfStack->TopElement();

	RefCountedObject* pSelf = pElement->GetObject();
	pSelf->IncReference();
	return pSelf;
}