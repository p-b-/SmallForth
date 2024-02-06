#include <iostream>
#include "ForthDefs.h"
#include "ForthWord.h"
#include "DataStack.h"
#include "ReturnStack.h"
#include "ExecState.h"
#include "ForthDict.h"
#include "TypeSystem.h"
#include "CompileHelper.h"
#include "ForthFile.h"

ForthWord::ForthWord(const string& name) :
	RefCountedObject(nullptr) {
	this->objectType = ObjectType_Word;
	this->name = name;
	this->bodySize = 0;
	this->body = nullptr;
	this->immediate = false;
	this->visible = false;
}

ForthWord::ForthWord(const string& name, XT firstXT) :
	RefCountedObject(nullptr) {
	this->objectType = ObjectType_Word;
	this->visible = false;
	this->name = name;
	this->bodySize = 1;
	this->body = new WordBodyElement* [this->bodySize];
	this->immediate = false;
	WordBodyElement* wbe = new WordBodyElement();
	wbe->wordElement_XT = firstXT;
	this->body[0] = wbe;
}

void ForthWord::CompileXTIntoWord(XT xt, int pos /*= -1 */) {
	// TODO Ensure when deleting words, that the WordBodyElement objects are deleted
	//      Note, currently words do not get deleted, as other words or stackelements may be referencing pointers inside them
	WordBodyElement* pNewElement = new WordBodyElement();
	pNewElement->wordElement_XT = xt;
	if (pos == -1) {
		GrowByAndAdd(1, pNewElement);
	}
	else {
		if (bodySize == 0) {
			GrowByAndAdd(1, pNewElement);
		}
		else {
			// TODO Consider garbage collection here... 
			this->body[pos] = pNewElement;
		}
	}
}

void ForthWord::CompileCFAPterIntoWord(WordBodyElement** pterToBody) {
	// TODO Ensure when deleting words, that the WordBodyElement objects are deleted
	WordBodyElement* pNewElement = new WordBodyElement();
	pNewElement->wordElement_BodyPter = pterToBody;
	GrowByAndAdd(1, pNewElement);
}

void ForthWord::CompileLiteralIntoWord(bool literal) {
	WordBodyElement* pNewElement = new WordBodyElement();
	pNewElement->wordElement_bool= literal;
	GrowByAndAdd(1, pNewElement);
}

void ForthWord::CompileLiteralIntoWord(char literal) {
	WordBodyElement* pNewElement = new WordBodyElement();
	pNewElement->wordElement_char = literal;
	GrowByAndAdd(1, pNewElement);
}

void ForthWord::CompileLiteralIntoWord(int64_t literal) {
	WordBodyElement* pNewElement = new WordBodyElement();
	pNewElement->wordElement_int = literal;
	GrowByAndAdd(1, pNewElement);
}

void ForthWord::CompileLiteralIntoWord(double literal) {
	WordBodyElement* pNewElement = new WordBodyElement();
	pNewElement->wordElement_float = literal;
	GrowByAndAdd(1, pNewElement);
}

void ForthWord::CompileLiteralIntoWord(ValueType literal) {
	WordBodyElement* pNewElement = new WordBodyElement();
	pNewElement->wordElement_type = literal;
	GrowByAndAdd(1, pNewElement);
}

void ForthWord::CompileLiteralIntoWord(WordBodyElement* literal) {
	GrowByAndAdd(1, literal);
}


void ForthWord::CompileTypeIntoWord(ForthType forthType) {
	WordBodyElement* pNewElement = new WordBodyElement();
	pNewElement->forthType = forthType;
	GrowByAndAdd(1, pNewElement);
}

void ForthWord::CompilePterIntoWord(void* pter) {
	WordBodyElement* pNewElement = new WordBodyElement();
	pNewElement->pter = pter;
	GrowByAndAdd(1, pNewElement);
}

void ForthWord::AddElementToWord(WordBodyElement* pElement) {
	GrowByAndAdd(1, pElement);
}

void ForthWord::ExpandBy(int expandBy) {
	GrowByAndAdd(expandBy, nullptr);
}

void ForthWord::GrowBy(int growBy) {
	WordBodyElement** pNewBody = new WordBodyElement * [this->bodySize + growBy];

	for (int n = 0; n < this->bodySize; n++) {
		pNewBody[n] = this->body[n];
	}
	for (int n = this->bodySize; n < this->bodySize + growBy; n++) {
		pNewBody[n] = nullptr;
	}
	delete this->body;
	this->body = pNewBody;
	this->bodySize+=growBy;
}

void ForthWord::GrowByAndAdd(int growBy, WordBodyElement* pElement) {
	WordBodyElement** pNewBody = new WordBodyElement * [this->bodySize + growBy];

	for (int n = 0; n < this->bodySize; n++) {
		pNewBody[n] = this->body[n];
	}
	for (int n = this->bodySize; n < this->bodySize + growBy; n++) {
		if (pElement == nullptr) {
			WordBodyElement* newElement = new WordBodyElement();
			newElement->wordElement_int = 0;
			pNewBody[n] = newElement;
		}
		else {
			pNewBody[n] = pElement;
		}
	}

	delete this->body;
	this->body = pNewBody;
	this->bodySize+=growBy;
}

bool ForthWord::BuiltIn_Exit(ExecState* pExecState) {
	// Ensure that exception state is reset, to let the calling method know this is the exit of the level-2 word.
	pExecState->exceptionThrown = false;
	return false;
}

bool ForthWord::BuiltIn_Jump(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	if (pElement == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	int64_t newIp = pElement->GetInt();

	delete pElement;
	pElement = nullptr;

	if (newIp == 0) {
		// Initial CFA is generally DOCOL. However, it is an XT not a pointer to a body, which is what IP expects
		return pExecState->CreateException("Cannot jump to initial CFA in level-2 word");
	}
	// Jump the body that nested this jump call to a different IP
	pExecState->SetPreviousBodyIP((int)newIp);
	return true;
}

bool ForthWord::BuiltIn_JumpOnTrue(ExecState* pExecState) {
	StackElement* pAddrElement = nullptr;
	StackElement* pBoolElement = nullptr;
	if (!BuiltInHelper_GetTwoStackElements(pExecState, pAddrElement, pBoolElement)) {
		return false;
	}
	int64_t newIp = pAddrElement->GetInt();
	bool flag = pBoolElement->GetBool();

	BuiltInHelper_DeleteOperands(pAddrElement, pBoolElement);

	if (newIp == 0) {
		return pExecState->CreateException("Cannot jump to initialise CFA in level-2 word");
	}
	if (flag) {
		// Jump the body that nested this jump call to a different IP
		pExecState->SetPreviousBodyIP((int)newIp);
	}
	return true;
}

bool ForthWord::BuiltIn_JumpOnFalse(ExecState* pExecState) {
	StackElement* pAddrElement = nullptr;
	StackElement* pBoolElement = nullptr;
	if (!BuiltInHelper_GetTwoStackElements(pExecState, pAddrElement, pBoolElement)) {
		return false;
	}
	int64_t newIp = pAddrElement->GetInt();
	bool flag = pBoolElement->GetBool();

	BuiltInHelper_DeleteOperands(pAddrElement, pBoolElement);

	if (newIp == 0) {
		return pExecState->CreateException("Cannot jump to initialise CFA in level-2 word");
	}
	if (!flag) {
		// Jump the body that nested this jump call to a different IP
		pExecState->SetPreviousBodyIP((int)newIp);
	}
	return true;
}


bool ForthWord::BuiltIn_Immediate(ExecState* pExecState) {
	return pExecState->pCompiler->SetLastWordCreatedToImmediate(pExecState);
}

bool ForthWord::BuiltIn_Here(ExecState* pExecState) {
	// Pushes 'dictionary pointer' to parameter stack
	return pExecState->pCompiler->PushDPForCurrentlyCreatingWord(pExecState);
}

// Execute XT that is on the stack
bool ForthWord::BuiltIn_Execute(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	WordBodyElement** pWBE = pElement->GetWordBodyElement();
	delete pElement;
	pElement = nullptr;

	pExecState->NestAndSetCFA(pWBE, 1);

	XT execFirst = pExecState->pExecBody[0]->wordElement_XT;
	bool returnValue = execFirst(pExecState);

	pExecState->UnnestCFA();

	return returnValue;
}

// Execute XT that is on the stack on object that next on the stack
bool ForthWord::BuiltIn_ExecuteOnObject(ExecState* pExecState) {
	StackElement* pElementWordToExecute = pExecState->pStack->Pull();
	WordBodyElement** pWBE = pElementWordToExecute->GetWordBodyElement();
	delete pElementWordToExecute;
	pElementWordToExecute = nullptr;

	StackElement* pElementObjectToExecOn = pExecState->pStack->Pull();
	RefCountedObject* pObjToExecOn = pElementObjectToExecOn->GetObject();
	pExecState->NestSelfPointer(pObjToExecOn);
	delete pElementObjectToExecOn;
	pElementObjectToExecOn = nullptr;

	pExecState->NestAndSetCFA(pWBE, 1);

	XT execFirst = pExecState->pExecBody[0]->wordElement_XT;
	bool returnValue = execFirst(pExecState);

	pExecState->UnnestSelfPointer();

	pExecState->UnnestCFA();

	return returnValue;
}

bool ForthWord::BuiltIn_ErrantWord(ExecState* pExecState) {
	return pExecState->CreateException("Word not defined correctly");
}

bool ForthWord::BuiltIn_ParenthesisCommentStart(ExecState* pExecState) {
	pExecState->insideComment = true;
	return true;
}

bool ForthWord::BuiltIn_ParenthesisCommentEnd(ExecState* pExecState) {
	pExecState->insideComment = false;
	return true;
}

bool ForthWord::BuiltIn_LineCommentStart(ExecState* pExecState) {
	pExecState->insideLineComment = true;
	return true;
}


// ( addr:value -- ) *addr=value
bool ForthWord::BuiltIn_PokeIntegerInWord(ExecState* pExecState) {
	if (!pExecState->compileState) {
		return pExecState->CreateException("Cannot execute !INWORD when not compiling");
	}
	StackElement* pAddressElement = nullptr;
	StackElement* pValueElement = nullptr;
	if (!BuiltInHelper_GetTwoStackElements(pExecState, pAddressElement, pValueElement)) {
		return false;
	}
	if (pValueElement->GetType() != StackElement_Int) {
		BuiltInHelper_DeleteOperands(pValueElement, pAddressElement);
		return pExecState->CreateException("Need an integer value to poke into word");
	}
	if (pAddressElement->GetType() != StackElement_Int) {
		BuiltInHelper_DeleteOperands(pValueElement, pAddressElement);
		return pExecState->CreateException("Need an integer address to poke into word");
	}
	int v = (int)pValueElement->GetInt();
	int addr = (int)pAddressElement->GetInt();
	BuiltInHelper_DeleteOperands(pValueElement, pAddressElement);

	WordBodyElement* pLiteral = new WordBodyElement();
	pLiteral->wordElement_int = v;
	return pExecState->pCompiler->AlterElementInWordUnderCreation(pExecState, addr, pLiteral);
}

bool ForthWord::BuiltIn_Peek(ExecState* pExecState) {
	StackElement* pElementAddress = pExecState->pStack->TopElement();

	if (pElementAddress == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (!pElementAddress->IsPter()) {
		return pExecState->CreateException("Cannot load pter value, as top of stack is not a pointer");
	}
	pExecState->pStack->Pull();

	bool returnValue = true;
	StackElement* pValueElement = pElementAddress->GetDerefedPterValueAsStackElement();
	if (pValueElement == nullptr) {
		returnValue = pExecState->CreateException("Could not read value of pter on stack");
	}
	else {
		pExecState->pStack->Push(pValueElement);
	}

	delete pElementAddress;
	pElementAddress = nullptr;

	return returnValue;
}

bool ForthWord::BuiltIn_Poke(ExecState* pExecState) {
	StackElement* pAddressElement = pExecState->pStack->TopElement();

	if (pAddressElement == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (!pAddressElement->IsPter()) {
		return pExecState->CreateException("Cannot store into pter value, as top of stack is not a pointer");
	}
	pExecState->pStack->Pull();
	StackElement* pValueElement = pExecState->pStack->Pull();
	if (pValueElement == nullptr) {
		delete pAddressElement;
		pAddressElement = nullptr;
		return pExecState->CreateException("Cannot get value to set pter to, as stack empty.  Stack underflow");
	}
	bool returnValue = true;

	bool success;
	success = pAddressElement->PokeIntoContainedPter(pExecState, pValueElement);
	delete pAddressElement;
	pAddressElement = nullptr;
	delete pValueElement;
	pValueElement = nullptr;
	return success;
}

bool ForthWord::BuiltIn_PushPter(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	WordBodyElement** ppWBE_Type = pExecState->GetWordPterAtOffsetFromCurrentBody(1);
	WordBodyElement** ppWBE_Literal = pExecState->GetWordPterAtOffsetFromCurrentBody(2);

	if (ppWBE_Type == nullptr || ppWBE_Literal == nullptr) {
		return pExecState->CreateException("Cannot push a pointer a word-contained literal, as has to have both a type and a literal value");
	}
	ForthType currentType = (*ppWBE_Type)->forthType;
	ForthType pointerType = pTS->CreatePointerTypeTo(currentType);
	// Note: commenting this line and adding the next one, currently breaks peek and poke
	void* pter = reinterpret_cast<void* >(ppWBE_Literal[0]);
	//void* pter = reinterpret_cast<void*>(ppWBE_Literal);
	StackElement* pNewStackElement = new StackElement( pointerType, pter);
	if (!pExecState->pStack->Push(pNewStackElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_PushFloatPter(ExecState* pExecState) {
	WordBodyElement** ppWBE = pExecState->GetWordPterAtOffsetFromCurrentBody(1);

	double* pValue = &(*ppWBE)->wordElement_float;
	
	StackElement* pNewStackElement = new StackElement(pValue);
	if (!pExecState->pStack->Push(pNewStackElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_PushIntPter(ExecState* pExecState) {
	WordBodyElement** ppWBE = pExecState->GetWordPterAtOffsetFromCurrentBody(1);

	int64_t* pValue = &(*ppWBE)->wordElement_int;

	StackElement* pNewStackElement = new StackElement(pValue);
	if (!pExecState->pStack->Push(pNewStackElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_PushCharPter(ExecState* pExecState) {
	WordBodyElement** ppWBE = pExecState->GetWordPterAtOffsetFromCurrentBody(1);

	char* pValue = &(*ppWBE)->wordElement_char;

	StackElement* pNewStackElement = new StackElement(pValue);
	if (!pExecState->pStack->Push(pNewStackElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_PushBoolPter(ExecState* pExecState) {
	WordBodyElement** ppWBE = pExecState->GetWordPterAtOffsetFromCurrentBody(1);

	bool* pValue = &(*ppWBE)->wordElement_bool;

	StackElement* pNewStackElement = new StackElement(pValue);
	if (!pExecState->pStack->Push(pNewStackElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

// Push the pointer to word one of the current word, as a pointer to a type.
//  Used by VARIABLE, when the variable contained is a type
bool ForthWord::BuiltIn_PushTypePter(ExecState* pExecState) {
	WordBodyElement** ppWBE = pExecState->GetWordPterAtOffsetFromCurrentBody(1);

	ValueType* pValue = &(*ppWBE)->wordElement_type;

	StackElement* pNewStackElement = new StackElement(pValue);
	if (!pExecState->pStack->Push(pNewStackElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_Constant(ExecState* pExecState) {
	if (!ForthWord::BuiltIn_Create(pExecState)) {
		return false;
	}
	if (!ForthWord::BuiltIn_Dup(pExecState)) return false;
	if (!ForthWord::BuiltIn_PushType(pExecState)) return false;
	StackElement* pTypeElement = pExecState->pStack->Pull();
	ForthType vt = pTypeElement->GetValueType();
	delete pTypeElement;
	pTypeElement = nullptr;
	if (!ForthWord::BuiltInHelper_CompileTOSLiteral(pExecState, false)) return false;

	pExecState->pCompiler->CompileDoesXT(pExecState, ForthWord::BuiltIn_FetchLiteral);
	pExecState->pCompiler->RevealWord(pExecState, true);

	return true;
}

bool ForthWord::BuiltIn_Variable(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!ForthWord::BuiltIn_Create(pExecState)) {
		return false;
	}

	if (!ForthWord::BuiltInHelper_CompileTOSLiteral(pExecState, false)) return false;
	pExecState->pCompiler->CompileDoesXT(pExecState, ForthWord::BuiltIn_PushPter);
	pExecState->pCompiler->RevealWord(pExecState, true);

	return true;
}

bool ForthWord::BuiltIn_TypeFromInt(ExecState* pExecState) {
	StackElement* pElement;
	if (!BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (pElement->GetType() != ValueType_Int) {
		return pExecState->CreateException("Only ints can be cast to types");
	}
	int64_t typeAsInt = pElement->GetInt();
	BuiltInHelper_DeleteStackElement(pElement);

	ForthType typeAsType = (ForthType)typeAsInt;
	pExecState->pStack->Push(typeAsType);

	return true;
}


bool ForthWord::BuiltIn_IndirectDoCol(ExecState* pExecState) {
	WordBodyElement* pWbe = pExecState->pExecBody[pExecState->ip];
	pExecState->ip++;

	// Second element of indirect docol, is a pointer to the body which contains the elements to be executed
	// Used when creating words where the CFA needs to be point to a second level word, but the created word contains data
	WordBodyElement** secondayWordBody = pWbe->wordElement_BodyPter;
	// Push address of next element - where the data is stored
	pExecState->pStack->Push(pExecState->pExecBody + pExecState->ip);

	pExecState->NestAndSetCFA(secondayWordBody, 1);
	// The secondary word being called should know that there is a pointer on the stack to data
	bool success = BuiltIn_DoCol(pExecState);
	pExecState->UnnestCFA();
	return success;
}


bool ForthWord::BuiltIn_DoCol(ExecState* pExecState) {
	bool exitFound = false;

	while (!exitFound) {
		// Each execution state object contains a pointer to a word body.
		//  Traditionally in forth, this contain either machine, or in the case of a call to DoCol, it contains
		// pWbe[0]  :   address of docol (this method)
		// pWbe[1]  :   address of the first word's CFA to execute   <== pExecState->ip starts here, at 1
		// pWbe[2]  :   address of the second word's CFAto execute 
		// pWbe[3]  :   address of the third word's CFA to execute.  <== this may be the exit word, which is a function pointer that
		//                                                                 returns false but does not create an exception

		// A CFA is always a pointer to a pWbe[0], though, it is stored as a WordBodyElement union, and must be extracted from the .wordElement_BodyPter element
		// The first entry in pWbe, pWbe[0], is always of union element .wordElement_XT, and is therefore callable.

		WordBodyElement* pWbe = pExecState->pExecBody[pExecState->ip];
		// The WordBodyElement is a union that can contain pointers to CFAs (in this case) or other data (when the body is storing a constant for instance)
		WordBodyElement** pCFA = pWbe->wordElement_BodyPter;
		WordBodyElement* actualCFA = pCFA[0];
		XT exec = actualCFA->wordElement_XT;

		pExecState->ip++;

		// It would be easier, when creating defining words at least, if all words in a DOCOL were compiled, apart from immediate ones.
		//  A defining word definition has a lot of postpones in it:
		// : loop postpone , (postpone) postpone 2<r (postpone) postpone 1+ (postpone) 2dup (postpone) != (postpone) -rot (postpone) postpone 2>r (postpone) jumpontrue (postpone) 2<r (postpone) 2drop ; immediate

		// (note - (postpone) is equivalent to postpone postpone).
		// However, in DOCOL, we do not have the precedence bit - so do not know it's immediate.  This causes issues with every DOCOL called for compilation, as
		//  it would compile the EXIT and continue past into undefined behaviour.


		if (pExecState->executionPostponed) {
			pExecState->executionPostponed = false;
			if (pExecState->compileState || pExecState->runtimeCompileState) {
				// Compile this word
				pExecState->pCompiler->CompileWord(pExecState, pCFA);
				pExecState->executionPostponed = false;
				continue;
			}
		}
		pExecState->NestAndSetCFA(pCFA, 1);
		try
		{
			if (!exec(pExecState)) {
				if (pExecState->exceptionThrown) {
					// TODO Fix this so it reports IP stack trace properly, and is guarded by an environment variable
					//int prevIP = pExecState->GetPreviousBodyIP() - 1;
					//pExecState->SetExeptionIP(prevIP);
					//cout << " Exception IP " << prevIP << endl;
					pExecState->UnnestCFA();
					return false;
				}
				exitFound = true;
			}
		}
		catch (...) {
			pExecState->UnnestCFA();
			throw;
		}
		pExecState->UnnestCFA();
	}

	return true;
}

// ( n -- n n )
bool ForthWord::BuiltIn_Dup(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();

	if (pElement == nullptr) {
		return pExecState->CreateException("Stack underflow");
	}

	StackElement* pClonedElement = new StackElement(*pElement);

	pExecState->pStack->Push(pElement);
	pExecState->pStack->Push(pClonedElement);

	return true;
}

// ( n m -- m n )
bool ForthWord::BuiltIn_Swap(ExecState* pExecState) {
	StackElement* pElement1 = nullptr; 
	StackElement* pElement2 = nullptr;
	// Element1 is further in the stack than element2
	// ( Element1 Element2 -- Element2 Element1)
	if (!BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}

	pExecState->pStack->Push(pElement2);
	pExecState->pStack->Push(pElement1);

	return true;
}

// ( n -- )
bool ForthWord::BuiltIn_Drop(ExecState* pExecState) {
	StackElement* pElement1 = nullptr;
	if (!BuiltInHelper_GetOneStackElement(pExecState, pElement1)) {
		return false;
	}
	delete pElement1;
	pElement1 = nullptr;
	return true;
}

// ( n m -- n m n)
bool ForthWord::BuiltIn_Over(ExecState* pExecState) {
	StackElement* pElement1 = nullptr;
	StackElement* pElement2 = nullptr;
	// Element1 is further in the stack than element2
	// ( Element1 Element2 -- Element2 Element1)
	if (!BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}
	pExecState->pStack->Push(pElement1);
	pExecState->pStack->Push(pElement2);
	StackElement* pElement3 = new StackElement(*pElement1);
	pExecState->pStack->Push(pElement3);
	return true;
}

// ( m n p -- n p m)
bool ForthWord::BuiltIn_Rot(ExecState* pExecState) {
	StackElement* pElementm = nullptr;
	StackElement* pElementn = nullptr;
	StackElement* pElementp = nullptr;
	if (!BuiltInHelper_GetThreeStackElements(pExecState, pElementm, pElementn, pElementp)) {
		return false;
	}

	pExecState->pStack->Push(pElementn);
	pExecState->pStack->Push(pElementp);
	pExecState->pStack->Push(pElementm);
	return true;
}

// ( m n p -- p m n)
bool ForthWord::BuiltIn_ReverseRot(ExecState* pExecState) {
	StackElement* pElementm = nullptr;
	StackElement* pElementn = nullptr;
	StackElement* pElementp = nullptr;
	if (!BuiltInHelper_GetThreeStackElements(pExecState, pElementm, pElementn, pElementp)) {
		return false;
	}

	pExecState->pStack->Push(pElementp);
	pExecState->pStack->Push(pElementm);
	pExecState->pStack->Push(pElementn);
	return true;
}

// >R ( a -- R: a )
bool ForthWord::BuiltIn_PushDataStackToReturnStack(ExecState* pExecState) {
	StackElement* pElement = nullptr;
	if (!BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (pElement->GetType() != StackElement_Int) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Cannot push a non-integer to the return stack");
	}
	int nToPush = (int)pElement->GetInt();
	delete pElement;
	pElement = nullptr;
	if (!pExecState->pReturnStack->Push(nToPush)) {
		return pExecState->CreateReturnStackOverflowException();
	}
	return true;
}

// R> ( R: a -- a )
bool ForthWord::BuiltIn_PushReturnStackToDataStack(ExecState* pExecState) {
	int nFromReturnStack;
	if (!pExecState->pReturnStack->Pull(nFromReturnStack)) {
		return pExecState->CreateReturnStackUnderflowException();
	}
	if (!pExecState->pStack->Push((int64_t)nFromReturnStack)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

// >T ( a -- T: a)
bool ForthWord::BuiltIn_PushDataStackToTempStack(ExecState* pExecState) {
	StackElement* pElement = nullptr;
	if (!BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pExecState->pTempStack->Push(pElement)) {
		return pExecState->CreateTempStackOverflowException();
	}
	return true;
}

// <T ( T:a -- a)
bool ForthWord::BuiltIn_PushTempStackToDataStack(ExecState* pExecState) {
	StackElement* pElement = nullptr; 
	if (!BuiltInHelper_GetOneTempStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pExecState->pStack->Push(pElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_Add(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Add);
}

bool ForthWord::BuiltIn_Subtract(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Subtract);
}

bool ForthWord::BuiltIn_Multiply(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Multiply);
}

bool ForthWord::BuiltIn_Divide(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Divide);
}

bool ForthWord::BuiltIn_Modulus(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Modulus);
}

bool ForthWord::BuiltIn_LessThan(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_LessThan);
}

bool ForthWord::BuiltIn_LessThanOrEquals(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_LessThanOrEquals);
}

bool ForthWord::BuiltIn_Equals(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Equals);
}

bool ForthWord::BuiltIn_NotEquals(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_NotEquals);
}

bool ForthWord::BuiltIn_GreaterThanOrEquals(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_GreaterThanOrEquals);
}

bool ForthWord::BuiltIn_GreaterThan(ExecState* pExecState) {
	return BuiltInHelper_BinaryOperation(pExecState, BinaryOp_GreaterThan);
}

bool ForthWord::BuiltIn_Not(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	if (pElement == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (pElement->GetType() != StackElement_Bool) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Cannot invert non-boolean type");
	}
	bool value = pElement->GetBool();
	delete pElement;
	pElement = nullptr;

	pElement = new StackElement(!value);
	if (!pExecState->pStack->Push(pElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_Or(ExecState* pExecState) { 
	StackElement* pElement1;
	StackElement* pElement2;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}
	if (pElement1->GetType() != StackElement_Bool ||
		pElement2->GetType() != StackElement_Bool) {
		BuiltInHelper_DeleteOperands(pElement1, pElement2);
		return pExecState->CreateException("OR operation needs two booleans");
	}
	bool value1 = pElement1->GetBool();
	bool value2 = pElement2->GetBool();
	BuiltInHelper_DeleteOperands(pElement1, pElement2);

	StackElement* pElementResult = new StackElement(value1 || value2);
	if (!pExecState->pStack->Push(pElementResult)) {
		return pExecState->CreateStackUnderflowException();
	}

	return true;
}

bool ForthWord::BuiltIn_And(ExecState* pExecState) { 
	StackElement* pElement1;
	StackElement* pElement2;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}
	if (pElement1->GetType() != StackElement_Bool ||
		pElement2->GetType() != StackElement_Bool) {
		BuiltInHelper_DeleteOperands(pElement1, pElement2);
		return pExecState->CreateException("AMD operation needs two booleans");
	}
	bool value1 = pElement1->GetBool();
	bool value2 = pElement2->GetBool();
	BuiltInHelper_DeleteOperands(pElement1, pElement2);

	StackElement* pElementResult = new StackElement(value1 && value2);
	if (!pExecState->pStack->Push(pElementResult)) {
		return pExecState->CreateStackUnderflowException();
	}

	return true;
}

bool ForthWord::BuiltIn_Xor(ExecState* pExecState) { 
	StackElement* pElement1;
	StackElement* pElement2;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}
	if (pElement1->GetType() != StackElement_Bool ||
		pElement2->GetType() != StackElement_Bool) {
		BuiltInHelper_DeleteOperands(pElement1, pElement2);
		return pExecState->CreateException("XOR operation needs two booleans");
	}
	bool value1 = pElement1->GetBool();
	bool value2 = pElement2->GetBool();
	BuiltInHelper_DeleteOperands(pElement1, pElement2);

	StackElement* pElementResult = new StackElement(value1 != value2);
	if (!pExecState->pStack->Push(pElementResult)) {
		return pExecState->CreateStackUnderflowException();
	}

	return true;
}

// TODO Do something with found word.
// Get next word from input stream, and find it in the dictionary
bool ForthWord::BuiltIn_Find(ExecState* pExecState) {
	InputWord iw = pExecState->GetNextWordFromInput();
	string word = iw.word;
	return true;
}

bool ForthWord::BuiltIn_Reveal(ExecState* pExecState) {
	return pExecState->pCompiler->RevealWord(pExecState, true);
}

bool ForthWord::BuiltIn_RevealToStack(ExecState* pExecState) {
	return pExecState->pCompiler->RevealWord(pExecState, false);
}

bool ForthWord::BuiltIn_Forget(ExecState* pExecState) {
	InputWord iw = pExecState->GetNextWordFromInput();
	string word = iw.word;

	return pExecState->pDict->ForgetWord(word);;
}

bool ForthWord::BuiltIn_Does(ExecState* pExecState) {
	if (!pExecState->pCompiler->HasValidLastWordCreated()) {
		return pExecState->CreateException("Cannot execute does> when not compiling");
	}
	pExecState->runtimeCompileState = true;
	pExecState->executionPostponed = true;
	return true;
}

bool ForthWord::BuiltIn_Create(ExecState* pExecState) {
	pExecState->pCompiler->AbandonWordUnderCreation();
	// Get next word from input stream

	InputWord iw = pExecState->GetNextWordFromInput();
	string word = iw.word;

	pExecState->pCompiler->StartWordCreation(word);
	pExecState->pCompiler->CompileDoesXT(pExecState, ForthWord::BuiltIn_PushIntPter);

	return true;
}

bool ForthWord::BuiltIn_AddWordToObject(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	// ( word object -- )
	StackElement* pElementType = pExecState->pStack->Pull();
	if (pElementType == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (pElementType->GetType() != StackElement_Type ||
		!pTS->TypeIsUserObject(pElementType->GetValueType())) {
		delete pElementType;
		pElementType = nullptr;
		return pExecState->CreateException("Adding a word to an object requires ( word type -- ), where type is a type of user object.  No such type found");
	}

	StackElement* pElementWord = pExecState->pStack->Pull();
	if (pElementWord == nullptr) {
		delete pElementType;
		pElementType = nullptr;
		return pExecState->CreateStackUnderflowException();
	}
	if (pElementWord->GetType() != ObjectType_Word) {
		delete pElementType;
		pElementType = nullptr;
		delete pElementWord;
		pElementWord=nullptr;
		return pExecState->CreateException("Adding a word to an object requires ( word object -- ), no word found on stack");
	}

	ForthType objectType = pElementType->GetValueType();
	ForthWord* pWord = (ForthWord* )pElementWord->GetObject();

	bool success = true;
	success = pTS->AddWordToObject(pExecState, objectType, pWord);

	// Cannot delete these before adding the word to the object, as it could bring the ref count to 0.  (Although the word
	//  has likely just been created, it will still have a reference for the last word created.  However, the object may
	//  only exist on the stack).
	delete pElementWord;
	pElementWord = nullptr;
	delete pElementType;
	pElementType = nullptr;

	return success;
}

// TODO Once variables are working, remove compile state from ExecState and implement this word in Forth
bool ForthWord::BuiltIn_StartCompilation(ExecState* pExecState) {
	pExecState->compileState = true;
	pExecState->runtimeCompileState = false;
	return true;
}

// TODO Once variables are working, remove compile state from ExecState and implement this word in Forth
bool ForthWord::BuiltIn_EndCompilation(ExecState* pExecState) {
	pExecState->compileState = false;
	pExecState->runtimeCompileState = false;
	return true;
}

// TODO Once variables are working, remove flag from ExecState and implement this word in Forth
bool ForthWord::BuiltIn_Postpone(ExecState* pExecState) {
	pExecState->executionPostponed = true;
	return true;
}

bool ForthWord::BuiltIn_WordCFAFromInputStream(ExecState* pExecState) {
	InputWord iw = pExecState->GetNextWordFromInput();
	string word = iw.word;
	ForthWord* pWord = pExecState->pDict->FindWord(word);
	if (pWord == nullptr) {
		return pExecState->CreateException("Cannot find word in dictionary");
	}
	WordBodyElement** pCFA = pWord->GetPterToBody();
	StackElement* pNewElement = new StackElement(pCFA);
	pExecState->pStack->Push(pNewElement);
	return true;
}

bool ForthWord::BuiltIn_WordCFAFromDefinition(ExecState* pExecState) {
	WordBodyElement* pWBE = pExecState->GetNextWordFromCurrentBodyAndIncIP();

	if (pWBE == nullptr) {
		return pExecState->CreateException("Expecting a word pointer in definition");
	}
	if (!pExecState->pStack->Push(new StackElement(pWBE->wordElement_BodyPter))) {
		return pExecState->CreateStackUnderflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_Literal(ExecState* pExecState) {
	//if (pExecState->compileState == false) {
	//	// Does nothing when not compiling
	//	return true;
	//}
	return BuiltInHelper_CompileTOSLiteral(pExecState, true);
}

bool ForthWord::BuiltIn_CharLiteral(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		// Does nothing when not compiling
		return true;
	}
	pExecState->nextWordIsCharLiteral = true;
	return true;
}

bool ForthWord::BuiltIn_FetchLiteral(ExecState* pExecState) {
	return BuiltInHelper_FetchLiteralWithOffset(pExecState, 0);
}

bool ForthWord::BuiltIn_PushUpcomingLiteral(ExecState* pExecState) {
	WordBodyElement* pWBE_Type= pExecState->GetNextWordFromPreviousNestedBodyAndIncIP();
	if (pWBE_Type == nullptr) {
		return pExecState->CreateException("Push literal cannot find a literal type in word body");
	}
	WordBodyElement* pWBE_Literal = pExecState->GetNextWordFromPreviousNestedBodyAndIncIP();
	if (pWBE_Literal == nullptr) {
		return pExecState->CreateException("Push literal cannot find a literal in word body");
	}
	ForthType forthType = pWBE_Type->forthType;

	if (!pExecState->pStack->Push(new StackElement(forthType, pWBE_Literal))) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_True(ExecState* pExecState) {
	if (!pExecState->pStack->Push(true)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_False(ExecState* pExecState) {
	if (!pExecState->pStack->Push(false)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_PushType(ExecState* pExecState) {
	StackElement* pSE = pExecState->pStack->Pull();
	if (pSE == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	//ValueType t = pSE->GetTypeAsTypeValue();
	ForthType t = pSE->GetType();
	delete pSE;
	pSE = nullptr;

	if (!pExecState->pStack->Push(t)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_StackSize(ExecState* pExecState) {
	int64_t stackSize = pExecState->pStack->Count();
	StackElement* pNewElement = new StackElement(stackSize);
	// If push fails, it deletes the element
	if (!pExecState->pStack->Push(pNewElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_RStackSize(ExecState* pExecState) {
	int64_t stackSize = pExecState->pReturnStack->Count();
	StackElement* pNewElement = new StackElement(stackSize);
	// If push fails, it deletes the element
	if (!pExecState->pStack->Push(pNewElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_TStackSize(ExecState* pExecState) {
	int64_t stackSize = pExecState->pTempStack->Count();
	StackElement* pNewElement = new StackElement(stackSize);
	// If push fails, it deletes the element
	if (!pExecState->pStack->Push(pNewElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::BuiltIn_PrintStackTop(ExecState* pExecState) {
	bool success = true;

	StackElement* pTop = pExecState->pStack->Pull();
	if (pTop == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (!pTop->ToString(pExecState)) {
		delete pTop;
		pTop = nullptr;
		return false;
	}
	delete pTop;
	pTop = nullptr;
	string str;
	tie(success, str) = pExecState->pStack->PullAsString();
	if (success) {
		ostream* pStdoutStream = pExecState->GetStdout();
		(*pStdoutStream) << str;
		return true;
	}
	else {
		return pExecState->CreateException(str.c_str());
	}
}

bool ForthWord::BuiltIn_Emit(ExecState* pExecState) {
	StackElement* pTop = pExecState->pStack->Pull();
	if (pTop == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	ForthType t = pTop->GetType();

	ostream* pStdoutStream = pExecState->GetStdout();
	bool success = true;
	if (t == StackElement_Int) {
		char c = (pTop->GetInt() & 0xff);

		(*pStdoutStream) << c;
	}
	else if (t == StackElement_Char) {
		char c = pTop->GetChar();
		(*pStdoutStream) << c;
	}
	else {
		success = pExecState->CreateException("Could not convert top to char");
	}
	delete pTop;
	pTop = nullptr;
	return success;
}

// TODO Fix this
//      This runs to either first EXIT word, or the first contained literal.  Needs to consider length of body.
bool ForthWord::BuiltIn_DescribeWord(ExecState* pExecState) {
	ostream* pStdoutStream = pExecState->GetStdout();

	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	StackElement* pTop = pExecState->pStack->Pull();
	if (pTop == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	ForthType t = pTop->GetType();
	bool success = true;
	if (t != StackElement_PterToCFA) {
		success = pExecState->CreateException("Cannot describe the word CFA on stack, as, it is not a pointer to a CFA");
	}
	else {
		WordBodyElement** pCFA = pTop->GetWordBodyElement();
		WordBodyElement* pEl = pCFA[0];

		ForthType upcomingWordType = 0;
		bool upcomingWordIsLiteralType = false;
		bool upcomingWordIsLiteral = false;

		int bodySize = -1;

		string firstWord;
		if (pEl->wordElement_XT == ForthWord::BuiltIn_DoCol) {
			firstWord = "DOCOL";
		}
		else if (pEl->wordElement_XT == ForthWord::BuiltIn_PushPter)
		{
			firstWord = "PUSHPTER";
			bodySize = 2;
			upcomingWordIsLiteralType = true;
		}
		else if (pEl->wordElement_XT == ForthWord::BuiltIn_FetchLiteral) {
			firstWord = "FETCHLITERAL";
			bodySize = 2;
			upcomingWordIsLiteralType = true;
		}
		else if (pEl->wordElement_XT != ForthWord::BuiltIn_DoCol) {
			success = pExecState->CreateException("Cannot decompile a level-one word");
		}
		if (success) {
			(*pStdoutStream) << "0: "<< firstWord << endl;
			int ip = 0;
			bool loop = true;

			while (loop) {
				++ip;
				WordBodyElement* pEl = pCFA[ip];
				ForthWord* pWord = pExecState->pDict->FindWordFromCFAPter(pEl->wordElement_BodyPter);
				if (pWord == nullptr) {
					if (upcomingWordIsLiteralType) {
						upcomingWordType = pEl->wordElement_type;
						string typeDescription = pTS->TypeToString(upcomingWordType);
						(*pStdoutStream) << ip << ":  literal type (" << typeDescription << ")" << endl;
						upcomingWordIsLiteralType = false;
						upcomingWordIsLiteral = true;
					}
					else if (upcomingWordIsLiteral) {
						(*pStdoutStream) << ip << ":  literal value (";
						string contentsOfLiteral;
						bool successAtLiteral;
						if (pTS->IsValueOrValuePter(upcomingWordType)) {
							successAtLiteral = pTS->VariableToString(pExecState, upcomingWordType, reinterpret_cast<void* >(pEl->wordElement_int));
						}
						else {
							successAtLiteral = pTS->VariableToString(pExecState, upcomingWordType, pEl->pter);
						}
						if (!successAtLiteral) {
							(*pStdoutStream) << " could not retrieve)" << endl;
							return false;
						}
						successAtLiteral;
						tie(successAtLiteral, contentsOfLiteral) = pExecState->pStack->PullAsString();
						if (!successAtLiteral) {
							return pExecState->CreateException(contentsOfLiteral.c_str());
						}

						(*pStdoutStream) << contentsOfLiteral;
						(*pStdoutStream) << ")" << endl;
						upcomingWordIsLiteral = false;
					}
				}
				else {
					if (pWord->body[0]->wordElement_XT == ForthWord::BuiltIn_PushUpcomingLiteral) {
						upcomingWordIsLiteralType = true;
					}
					(*pStdoutStream) << ip << ":  " << pWord->GetName() << endl;
					if (pWord->GetName() == "exit") {
						loop = false;
					}
				}
				if (ip == bodySize) {
					loop = false;
				}
			}
		}
	}
	delete pTop;
	pTop = nullptr;
	return success;
}

bool ForthWord::BuiltIn_Leave(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute LEAVE when not compiling");
	}
	// Make a duplicate of where to leave to
	if (!pExecState->pCompiler->CompileWord(pExecState, "<r")) {
		return false;
	}
	if (!pExecState->pCompiler->CompileWord(pExecState, "dup")) {
		return false;
	}
	if (!pExecState->pCompiler->CompileWord(pExecState, ">r")) {
		return false;
	}
	if (!pExecState->pCompiler->CompileWord(pExecState, "jump")) {
		return false;
	}

	return true;
}

bool ForthWord::BuiltIn_Begin(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute BEGIN when not compiling");
	}

	pExecState->pCompiler->CompilePushAndLiteralIntoWordBeingCreated(pExecState, (int64_t)0);
	pExecState->pCompiler->CompilePushAndLiteralIntoWordBeingCreated(pExecState, (int64_t)0);
	// Allow LOOP to change this 0, into the IP of the LOOP instruction for LEAVE
	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}

	// This is the literal for the leave jump pointer
	pExecState->pCompiler->CompilePushAndLiteralIntoWordBeingCreated(pExecState, (int64_t)0);
	pExecState->pCompiler->CompileWord(pExecState, "3>r");

	// This where to jump back to
	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}
	return true;
}

bool ForthWord::BuiltIn_Until(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute UNTIL when not compiling");
	}

	// Stack ==> IP to jump back to when looping
	//           IP where literal is stored for LEAVE word (points to start of LOOP word compiled into new word)
	// Compile what is top of stack now, into word as push literal word, and literal
	// This was the HERE in the DO word, where we are to jump back to
	// 
	if (!ForthWord::BuiltIn_Literal(pExecState)) {
		return false;
	}

	// Stack ==> IP where literal is stored for LEAVE word (points to start of LOOP word compiled into new word)
	// Inc addr past BuiltIn_PushUpcomingIntLiteral word and to actual value
	if (!pExecState->ExecuteWordDirectly("2+")) {
		return false;
	}
	// Increment counter (all loops have counters)
	if (!pExecState->pCompiler->CompileWord(pExecState, "3<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, ">r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "1+")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "3>r")) return false;

	if (!pExecState->pCompiler->CompileWord(pExecState, "swap")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "jumponfalse")) return false;
	// Mark IP on stack where LEAVE is to jump to
	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}
	// Stack ==> Location for leave to jump to (HERE)
	//			 IP where literal is actually stored for LEAVE word (points to start of LOOP word compiled into new word)

	// poke ( addr:value -- ) *addr=value
	if (!ForthWord::BuiltIn_PokeIntegerInWord(pExecState)) {
		return false;
	}
	if (!pExecState->pCompiler->CompileWord(pExecState, "3<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "3drop")) return false;

	return true;
}

bool ForthWord::BuiltIn_While(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute WHILE when not compiling");
	}

	if (!pExecState->pCompiler->CompileWord(pExecState, "not")) return false;
	if (!ForthWord::BuiltIn_If(pExecState)) return false;
	if (!ForthWord::BuiltIn_Leave(pExecState)) return false;
	if (!ForthWord::BuiltIn_Then(pExecState)) return false;

	return true;
}

bool ForthWord::BuiltIn_Repeat(ExecState* pExecState) {
	// Same code as AGAIN apart from this error message
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute REPEAT when not compiling");
	}

	// Stack ==> IP to jump back to when looping
	//           IP where literal is stored for LEAVE word (points to start of LOOP word compiled into new word)
	// Compile what is top of stack now, into word as push literal word, and literal
	// This was the HERE in the DO word, where we are to jump back to
	// 
	if (!ForthWord::BuiltIn_Literal(pExecState)) {
		return false;
	}

	// Stack ==> IP where literal is stored for LEAVE word (points to start of LOOP word compiled into new word)
	// Inc addr past BuiltIn_PushUpcomingIntLiteral word and to actual value
	if (!pExecState->ExecuteWordDirectly("2+")) {
		return false;
	}

	// Increment counter (all loops have counters)
	if (!pExecState->pCompiler->CompileWord(pExecState, "3<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, ">r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "1+")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "3>r")) return false;

	if (!pExecState->pCompiler->CompileWord(pExecState, "jump")) return false;
	// Mark IP on stack where LEAVE is to jump to
	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}
	// Stack ==> Location for leave to jump to (HERE)
	//			 IP where literal is actually stored for LEAVE word (points to start of LOOP word compiled into new word)

	// poke ( addr:value -- ) *addr=value
	if (!ForthWord::BuiltIn_PokeIntegerInWord(pExecState)) {
		return false;
	}
	if (!pExecState->pCompiler->CompileWord(pExecState, "3<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "3drop")) return false;
	return true;
}

bool ForthWord::BuiltIn_Again(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute AGAIN when not compiling");
	}

	// Stack ==> IP to jump back to when looping
	//           IP where literal is stored for LEAVE word (points to start of LOOP word compiled into new word)
	// Compile what is top of stack now, into word as push literal word, and literal
	// This was the HERE in the DO word, where we are to jump back to
	// 
	if (!ForthWord::BuiltIn_Literal(pExecState)) {
		return false;
	}

	// Stack ==> IP where literal is stored for LEAVE word (points to start of LOOP word compiled into new word)
	// Inc addr past BuiltIn_PushUpcomingIntLiteral word and to actual value
	if (!pExecState->ExecuteWordDirectly("2+")) {
		return false;
	}

	// Increment counter (all loops have counters)
	if (!pExecState->pCompiler->CompileWord(pExecState, "3<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, ">r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "1+")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "3>r")) return false;

	if (!pExecState->pCompiler->CompileWord(pExecState, "jump")) return false;
	// Mark IP on stack where LEAVE is to jump to
	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}
	// Stack ==> Location for leave to jump to (HERE)
	//			 IP where literal is actually stored for LEAVE word (points to start of LOOP word compiled into new word)

	// poke ( addr:value -- ) *addr=value
	if (!ForthWord::BuiltIn_PokeIntegerInWord(pExecState)) {
		return false;
	}
	if (!pExecState->pCompiler->CompileWord(pExecState, "3<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "3drop")) return false;

	return true;
}

bool ForthWord::BuiltIn_Do(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute DO when not compiling");
	}

	// Allow LOOP to change this 0, into the IP of the LOOP instruction
	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}
	pExecState->pCompiler->CompilePushAndLiteralIntoWordBeingCreated(pExecState, (int64_t)0);
	pExecState->pCompiler->CompileWord(pExecState, "3>r");

	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}
	return true;
}

bool ForthWord::BuiltIn_Loop(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute LOOP when not compiling");
	}

	// Stack ==> IP to jump back to when looping
	//           IP where literal is stored for LEAVE word (points to start of LOOP word compiled into new word)
	// Compile what is top of stack now, into word as push literal word, and literal
	// This was the HERE in the DO word, where we are to jump back to
	// 
	if (!ForthWord::BuiltIn_Literal(pExecState)) {
		return false;
	}

	// Stack ==> IP where literal is stored for LEAVE word (points to start of LOOP word compiled into new word)
	// Inc addr past BuiltIn_PushUpcomingIntLiteral word and to actual value
	if (!pExecState->ExecuteWordDirectly("2+")) {
		return false;
	}

	if (!pExecState->pCompiler->CompileWord(pExecState, "3<r")) return false;
	// Move leave IP back to return stack
	if (!pExecState->pCompiler->CompileWord(pExecState, ">r")) return false;

	if (!pExecState->pCompiler->CompileWord(pExecState, "1+")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "2dup")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "!=")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "-rot")) return false;
	// Move leave IP back 
	if (!pExecState->pCompiler->CompileWord(pExecState, "<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "3>r")) return false;

	if (!pExecState->pCompiler->CompileWord(pExecState, "jumpontrue")) return false;
	// Mark IP on stack where LEAVE is to jump to
	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}
	// Stack ==> Location for leave to jump to (HERE)
	//			 IP where literal is actually stored for LEAVE word (points to start of LOOP word compiled into new word)

	// poke ( addr:value -- ) *addr=value
	if (!ForthWord::BuiltIn_PokeIntegerInWord(pExecState)) {
		return false;
	}
	if (!pExecState->pCompiler->CompileWord(pExecState, "3<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "3drop")) return false;

	return true;
}


bool ForthWord::BuiltIn_PlusLoop(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute LOOP when not compiling");
	}

	// Stack ==> IP to jump back to when looping
	//           IP where literal is stored for LEAVE word (points to start of LOOP word compiled into new word)
	// Compile what is top of stack now, into word as push literal word, and literal
	// This was the HERE in the DO word, where we are to jump back to
	// 
	if (!ForthWord::BuiltIn_Literal(pExecState)) {
		return false;
	}

	// Stack ==> IP where literal is stored for LEAVE word (points to start of LOOP word compiled into new word)
	// Inc addr past BuiltIn_PushUpcomingIntLiteral word and to actual value
	if (!pExecState->ExecuteWordDirectly("2+")) {
		return false;
	}

	pExecState->pCompiler->CompileWord(pExecState, "swap");
	pExecState->pCompiler->CompileWord(pExecState, "3<r");
	if (!pExecState->pCompiler->CompileWord(pExecState, ">r")) return false;

	pExecState->pCompiler->CompileWord(pExecState, "rot");
	pExecState->pCompiler->CompileWord(pExecState, "+");
	pExecState->pCompiler->CompileWord(pExecState, "2dup");
	pExecState->pCompiler->CompileWord(pExecState, "!=");
	pExecState->pCompiler->CompileWord(pExecState, "-rot");
	if (!pExecState->pCompiler->CompileWord(pExecState, "<r")) return false;
	pExecState->pCompiler->CompileWord(pExecState, "3>r");
	pExecState->pCompiler->CompileWord(pExecState, "jumpontrue");

	// Mark IP on stack where LEAVE is to jump to
	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}
	// Stack ==> Location for leave to jump to (HERE)
	//			 IP where literal is actually stored for LEAVE word (points to start of LOOP word compiled into new word)

	// poke ( addr:value -- ) *addr=value
	if (!ForthWord::BuiltIn_PokeIntegerInWord(pExecState)) {
		return false;
	}
	if (!pExecState->pCompiler->CompileWord(pExecState, "3<r")) return false;
	if (!pExecState->pCompiler->CompileWord(pExecState, "3drop")) return false;

	return true;
}

bool ForthWord::BuiltIn_If(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute IF when not compiling");
	}
	// Pushes dictionary pointer (where the next word will compile to) to stack
	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}
	if (!ForthWord::BuiltIn_Dup(pExecState)) {
		return false;
	}
	if (!ForthWord::BuiltIn_PushDataStackToReturnStack(pExecState)) {
		return false;
	}

	// Compiles a BuiltIn_PushUpcomingIntLiteral and then compiles the TOS integer.  This gives us something to alter in the THEN
	//  (altering the WordBodyElement at the location+1 that's on the return stack
	if (!ForthWord::BuiltIn_Literal(pExecState)) {
		return false;
	}

	// Stack needs to be address:flag, so have to swap
	pExecState->pCompiler->CompileWord(pExecState, "swap");
	pExecState->pCompiler->CompileWord(pExecState, "jumponfalse");

	return true;
}

bool ForthWord::BuiltIn_Then(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot update forward jump when not compiling");
	}
	return BuiltInHelper_UpdateForwardJump(pExecState);
}

bool ForthWord::BuiltIn_Else(ExecState* pExecState) {
	if (pExecState->compileState == false) {
		return pExecState->CreateException("Cannot execute ELSE when not compiling");
	}

	// Pushes dictionary pointer (where the next word will compile to) to stack
	if (!ForthWord::BuiltIn_Here(pExecState)) {
		return false;
	}
	if (!ForthWord::BuiltIn_Dup(pExecState)) {
		return false;
	}
	// Need to push here, but further down the stack as need the current TOS first
	//  This is like doing >R RSWAP, only without an RSWAP, we are doing <R SWAP >R >R
	if (!ForthWord::BuiltIn_PushReturnStackToDataStack(pExecState)) {
		return false;
	}
	if (!ForthWord::BuiltIn_Swap(pExecState)) {
		return false;
	}
	if (!ForthWord::BuiltIn_PushDataStackToReturnStack(pExecState)) {
		return false;
	}
	if (!ForthWord::BuiltIn_PushDataStackToReturnStack(pExecState)) {
		return false;
	}

	// Compiles a BuiltIn_PushUpcomingIntLiteral and then compiles the TOS integer.  This gives us something to alter in the THEN
	//  (altering the WordBodyElement at the location+1 that's on the return stack
	// Note, here, when THEN is executed, it will be updates the jump address of this jump.
	//  Further down this method, we are altering the forward jump from the IF's jumponfalse, to the DUPlicated HERE address from at the top of this method
	if (!ForthWord::BuiltIn_Literal(pExecState)) {
		return false;
	}
	pExecState->pCompiler->CompileWord(pExecState, "jump");


	// Return stack  top => A1   (address of IF literal for jumponfalse)
	//                      A2   (address of jump before ELSE literal (part of if)
	// Alter a prior jump, based on the address in the top of the return stack, and the word being built's DP (dictionary pointer, where next compiled word gets put).
	if (!BuiltInHelper_UpdateForwardJump(pExecState)) {
		return false;
	}

	return true;
}

string ForthWord::GetObjectType() {
	return "word";
}

bool ForthWord::ToString(ExecState* pExecState) const {
	string str = "Word: " + this->name;
	if (!pExecState->pStack->Push(str)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke) {
	return false;
}
