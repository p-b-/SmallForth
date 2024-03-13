#include <iostream>
#include "ForthDefs.h"
#include "ExecState.h"
#include "DataStack.h"
#include "ForthDict.h"
#include "ForthWord.h"
#include "InputProcessor.h"
#include "TypeSystem.h"
#include "ForthFile.h"
#include "CompileHelper.h"
#include "DebugHelper.h"
#include "WordBodyElement.h"

ExecState::ExecState() 
: ExecState(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) {
}

ExecState::ExecState(DataStack* pStack, ForthDict* pDict, InputProcessor* pInput, ReturnStack* pReturnStack, CompileHelper* pCompiler, DebugHelper* pDebugger) {
	this->pDict = pDict;
	this->pDict->IncReference();
	this->pStack = pStack;
	this->pInputProcessor = pInput;
	this->pReturnStack = pReturnStack;
	this->pCompiler = pCompiler;
	this->pDebugger = pDebugger;

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
		pElementInt->refCount = 99;
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
	std::string exceptionString = "Stack overflow: ";
	exceptionString.append(pzInfo);
	return CreateException(exceptionString.c_str());
}


bool ExecState::CreateStackUnderflowException() {
	return CreateException("Stack underflow");
}

bool ExecState::CreateStackUnderflowException(const char* pzInfo) {
	std::string exceptionString = "Stack underflow: ";
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


std::ostream* ExecState::GetStdout() {
	RefCountedObject* pObj;
	std::ostream* pStdoutStream = &std::cout;
	if (this->GetConstant("stdout", pObj)) {
		ForthFile* pFile = (ForthFile*)pObj;
		pStdoutStream = pFile->GetContainedStream();
		pFile->DecReference();
		pFile = nullptr;
	}
	return pStdoutStream;
}

std::ostream* ExecState::GetStderr() {
	RefCountedObject* pObj;
	std::ostream* pStderrStream = &std::cerr;
	if (this->GetConstant("stderr", pObj)) {
		ForthFile* pFile = (ForthFile*)pObj;
		pStderrStream = pFile->GetContainedStream();
		pFile->DecReference();
		pFile = nullptr;
	}
	return pStderrStream;
}

std::istream* ExecState::GetStdin() {
	RefCountedObject* pObj;
	std::istream* pStdinStream = &std::cin;
	if (this->GetConstant("stdin", pObj)) {
		ForthFile* pFile = (ForthFile*)pObj;
		pStdinStream = pFile->GetContainedStream();
		pFile->DecReference();
		pFile = nullptr;
	}
	return pStdinStream;
}

bool ExecState::PushVariableValueOntoStack(const std::string& variableName) {
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

bool ExecState::SetVariable(const std::string& variableName, int64_t setTo) {
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

bool ExecState::SetVariable(const std::string& variableName, bool setTo) {
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

bool ExecState::SetVariable(const std::string& variableName, double setTo) {
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

bool ExecState::SetVariable(const std::string& variableName, ForthType setTo) {
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

bool ExecState::GetVariable(const std::string& variableName, int64_t& variableValue) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	ForthType topElementType = this->pStack->GetTOSType();
	
	if (topElementType == pTS->CreatePointerTypeTo(StackElement_Int)) {
		if (this->ExecuteWordDirectly("@")) {
			if (this->pStack->Count() == 0) {
				return CreateStackOverflowException("whilst getting int variable");
			}
			ForthType tosType = this->pStack->GetTOSType();
			if (tosType != StackElement_Int && tosType != StackElement_Float && tosType != StackElement_Bool) {
				return CreateException("Variable does not evaluate to an integer");
			}
			variableValue = pStack->PullAsInt();
			return true;
		}
	}
	return false;
}

bool ExecState::GetBoolTLSVariable(int index) {
	WordBodyElement* pWBE = this->boolStates[index];
	return pWBE->wordElement_bool;
}

int64_t ExecState::GetIntTLSVariable(int index) {
	WordBodyElement* pWBE = this->intStates[index];
	return pWBE->wordElement_int;
}

bool ExecState::GetVariable(const std::string& variableName, double& variableValue) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	ForthType topElementType = this->pStack->GetTOSType();

	if (topElementType ==  pTS->CreatePointerTypeTo(StackElement_Float)) {
		if (this->ExecuteWordDirectly("@")) {
			if (this->pStack->Count() == 0) {
				return CreateStackOverflowException("whilst getting float variable");
			}
			ForthType tosType = this->pStack->GetTOSType();
			if (tosType != StackElement_Int && tosType != StackElement_Float && tosType != StackElement_Bool) {
				return CreateException("Variable does not evaluate to an float");
			}
			variableValue = pStack->PullAsFloat();
			return true;
		}
	}
	return false;
}

bool ExecState::GetVariable(const std::string& variableName, bool& variableValue) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	ForthType topElementType = this->pStack->GetTOSType();

	if (topElementType == pTS->CreatePointerTypeTo(StackElement_Bool)) {
		if (this->ExecuteWordDirectly("@")) {
			if (this->pStack->Count() == 0) {
				return CreateStackOverflowException("whilst getting bool variable");
			}
			ForthType tosType = this->pStack->GetTOSType();
			if (tosType != StackElement_Bool) {
				return CreateException("Variable does not evaluate to an bool");
			}
			variableValue = pStack->PullAsBool();
			return true;
		}
	}
	return false;
}

bool ExecState::GetVariable(const std::string& variableName, ForthType& variableValue) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!this->ExecuteWordDirectly(variableName)) {
		return false;
	}
	StackElement* pElementVariableValuePter = this->pStack->TopElement();

	ForthType topElementType = this->pStack->GetTOSType();

	if (topElementType == pTS->CreatePointerTypeTo(StackElement_Type)) {
		if (this->ExecuteWordDirectly("@")) {
			if (this->pStack->Count() == 0) {
				return CreateStackOverflowException("whilst getting type variable");
			}
			ForthType tosType = this->pStack->GetTOSType();
			if (tosType != StackElement_Type) {
				return CreateException("Variable does not evaluate to an type");
			}
			variableValue = pStack->PullAsType();
			return true;
		}
	}
	return false;
}

bool ExecState::PushConstantOntoStack(const std::string& constantName) {
	if (!this->ExecuteWordDirectly(constantName)) {
		this->exceptionThrown = false;
		return false;
	}
	return true;
}

bool ExecState::GetConstant(const std::string& constantName, int64_t& constantValue) {
	if (!PushConstantOntoStack(constantName)) {
		return false;
	}

	if (this->pStack->Count() == 0) {
		return CreateStackOverflowException("whilst getting int constant");
	}
	ForthType tosType = this->pStack->GetTOSType();
	if (tosType != StackElement_Int && tosType != StackElement_Float && tosType != StackElement_Bool) {
		return CreateException("Constant does not evaluate to an integer");
	}
	constantValue = this->pStack->PullAsInt();

	return true;
}

bool ExecState::GetConstant(const std::string& constantName, double& constantValue) {
	if (!PushConstantOntoStack(constantName)) {
		return false;
	}
	if (this->pStack->Count() == 0) {
		return CreateStackOverflowException("whilst getting float constant");
	}
	ForthType tosType = this->pStack->GetTOSType();
	if (tosType != StackElement_Int && tosType != StackElement_Float && tosType != StackElement_Bool) {
		return CreateException("Constant does not evaluate to a float");
	}
	constantValue = this->pStack->PullAsFloat();
	return true;
}

bool ExecState::GetConstant(const std::string& constantName, bool& constantValue) {
	if (!PushConstantOntoStack(constantName)) {
		return false;
	}
	if (this->pStack->Count() == 0) {
		return CreateStackOverflowException("whilst getting bool constant");
	}
	ForthType tosType = this->pStack->GetTOSType();
	if (tosType != StackElement_Bool) {
		return CreateException("Constant does not evaluate to a bool ");
	}
	constantValue = this->pStack->PullAsBool();
	return true;
}

bool ExecState::GetConstant(const std::string& constantName, ForthType& constantValue) {
	if (!PushConstantOntoStack(constantName)) {
		return false;
	}

	if (this->pStack->Count() == 0) {
		return CreateStackOverflowException("whilst getting type constant");
	}
	ForthType tosType = this->pStack->GetTOSType();
	if (tosType != StackElement_Type) {
		return CreateException("Constant does not evaluate to a type");
	}
	constantValue = this->pStack->PullAsType();
	return true;
}

bool ExecState::GetConstant(const std::string& constantName, RefCountedObject*& constantValue) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!PushConstantOntoStack(constantName)) {
		return false;
	}
	if (this->pStack->Count() == 0) {
		return CreateStackOverflowException("whilst getting object constant");
	}
	ForthType tosType = this->pStack->GetTOSType();
	if (!pTS->TypeIsObject(tosType)) {
		return CreateException("Constant does not evaluate to an object");
	}
	constantValue = this->pStack->PullAsObject();
	constantValue->IncReference();
	return true;
}


bool ExecState::ExecuteWordDirectly(const std::string& word) {
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
		if (this->pStack->Count() == 0) {
			return CreateStackUnderflowException("Whilst executing a word directly");
		}
		ForthType tosType = this->pStack->GetTOSType();
		if (!pTS->TypeIsObject(tosType)) {
			return CreateException("Cannot execute a word on an object, where TOS is not an object");
		}
		RefCountedObject* pObjToExecOn = this->pStack->PullAsObject();
		this->NestSelfPointer(pObjToExecOn);
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
	if (this->pSelfStack->Count() == 0) {
		return CreateSelfStackUnderflowException();
	}
	this->pSelfStack->DropTOS();
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