#include "ForthDefs.h"
#include "CompileHelper.h"
#include "ExecState.h"
#include "DataStack.h"
#include "ForthWord.h"
#include "ForthDict.h"
#include "PreBuiltWords.h"

CompileHelper::CompileHelper() {
	this->pWordUnderCreation = nullptr;
	this->pLastWordCreated = nullptr;
}

bool CompileHelper::CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, int64_t literalValue) {
	if (!pExecState->pStack->Push(literalValue)) {
		return pExecState->CreateStackOverflowException();
	}

	// This compiles whatever is on the stack now, as a literal into the word being compiled
	if (!PreBuiltWords::BuiltIn_Literal(pExecState)) {
		return false;
	}
	return true;
}

bool CompileHelper::CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, bool literalValue) {
	if (!pExecState->pStack->Push(literalValue)) {
		return pExecState->CreateStackOverflowException();
	}

	// This compiles whatever is on the stack now, as a literal into the word being compiled
	if (!PreBuiltWords::BuiltIn_Literal(pExecState)) {
		return false;
	}
	return true;
}

bool CompileHelper::CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, double literalValue) {
	if (!pExecState->pStack->Push(literalValue)) {
		return pExecState->CreateStackOverflowException();
	}

	// This compiles whatever is on the stack now, as a literal into the word being compiled
	if (!PreBuiltWords::BuiltIn_Literal(pExecState)) {
		return false;
	}
	return true;
}

bool CompileHelper::CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, char literalValue) {
	if (!pExecState->pStack->Push(literalValue)) {
		return pExecState->CreateStackOverflowException();
	}

	// This compiles whatever is on the stack now, as a literal into the word being compiled
	if (!PreBuiltWords::BuiltIn_Literal(pExecState)) {
		return false;
	}
	return true;
}

//bool CompileHelper::CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, ValueType literalValue) {
//	if (!pExecState->pStack->Push(literalValue)) {
//		return pExecState->CreateStackOverflowException();
//	}
//
//	// This compiles whatever is on the stack now, as a literal into the word being compiled
//	if (!ForthWord::BuiltIn_Literal(pExecState)) {
//		return false;
//	}
//	return true;
//}

bool CompileHelper::CompilePushAndLiteralIntoWordBeingCreated(ExecState* pExecState, RefCountedObject* literalValue) {
	if (!pExecState->pStack->Push(literalValue)) {
		return pExecState->CreateStackOverflowException();
	}

	if (!PreBuiltWords::BuiltIn_Literal(pExecState)) {
		return false;
	}
	return true;
}


void CompileHelper::CompileLiteralIntoWordBeingCreated(ExecState* pExecState, bool literal) {
	this->pWordUnderCreation->CompileLiteralIntoWord(literal);
}

void CompileHelper::CompileLiteralIntoWordBeingCreated(ExecState* pExecState, char literal) {
	this->pWordUnderCreation->CompileLiteralIntoWord(literal);
}

void CompileHelper::CompileLiteralIntoWordBeingCreated(ExecState* pExecState, int64_t literal) {
	this->pWordUnderCreation->CompileLiteralIntoWord(literal);
}

void CompileHelper::CompileLiteralIntoWordBeingCreated(ExecState* pExecState, double literal) {
	this->pWordUnderCreation->CompileLiteralIntoWord(literal);
}

void CompileHelper::CompileWBEIntoWordBeingCreated(ExecState* pExecState, WordBodyElement* pWBE) {
	this->pWordUnderCreation->CompileLiteralIntoWord(pWBE);
}

void CompileHelper::CompileTypeIntoWordBeingCreated(ExecState* pExecState, ForthType forthType) {
	this->pWordUnderCreation->CompileTypeIntoWord(forthType);
}

void CompileHelper::CompilePterIntoWordBeingCreated(ExecState* pExecState, void* voidPter) {
	this->pWordUnderCreation->CompilePterIntoWord(voidPter);
}

bool CompileHelper::PushDPForCurrentlyCreatingWord(ExecState* pExecState) {
	if (this->pWordUnderCreation == nullptr) {
		return pExecState->CreateException("No word being created");
	}
	int64_t size = this->pWordUnderCreation->GetBodySize();
	return pExecState->pStack->Push(size);
}

bool CompileHelper::SetLastWordCreatedToImmediate(ExecState* pExecState) {
	if (this->pLastWordCreated == nullptr) {
		pExecState->CreateException("No word created to set to immediate");
	}
	this->pLastWordCreated->SetImmediate(true);
	return true;
}

int CompileHelper::BodySizeOfWordUnderCreation(ExecState* pExecState) {
	if (this->pWordUnderCreation == nullptr) {
		pExecState->CreateException("No word being created");
		return -1;
	}
	return this->pWordUnderCreation->GetBodySize();
}

void CompileHelper::AbandonWordUnderCreation() {
	if (this->pWordUnderCreation != nullptr) {
		this->pWordUnderCreation->DecReference();
		this->pWordUnderCreation = nullptr;
	}
	//delete this->pWordUnderCreation;
}

void CompileHelper::StartWordCreation(const string& wordName) {
	this->pWordUnderCreation = new ForthWord(wordName);
	this->pWordUnderCreation->IncReference();
}

bool CompileHelper::RevealWord(ExecState* pExecState, bool revealToVocNotStack) {
	if (this->pWordUnderCreation == nullptr) {
		return pExecState->CreateException("No word to reveal");
	}
	if (revealToVocNotStack) {
		pExecState->pDict->AddWord(this->pWordUnderCreation);
	}
	else {
		if (!pExecState->pStack->Push(this->pWordUnderCreation)) {
			pWordUnderCreation->DecReference();
			pWordUnderCreation = nullptr;
			return pExecState->CreateStackOverflowException();
		}
	}
	this->pWordUnderCreation->SetWordVisibility(true);
	if (this->pLastWordCreated != nullptr) {
		this->pLastWordCreated->DecReference();
		this->pLastWordCreated = nullptr;
	}

	this->pLastWordCreated = this->pWordUnderCreation;
	this->pLastWordCreated->IncReference();
	this->pWordUnderCreation->DecReference();
	this->pWordUnderCreation = nullptr;
	return true;
}

bool CompileHelper::CompileWordOnStack(ExecState* pExecState) {
	if (this->pWordUnderCreation == nullptr) {
		return pExecState->CreateException("No word created to compile to");
	}

	StackElement* pElement = pExecState->pStack->Pull();
	if (pElement == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (pElement->GetType() != StackElement_PterToCFA) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Top of stack is wrong type to compile into a word definition");
	}
	// Body is a pointer to the first item in an array of WordBodyElement*
	WordBodyElement** pBody = pElement->GetWordBodyElement();
	CompileWord(pExecState, pBody);
	//this->pWordUnderCreation->CompileCFAPterIntoWord(pBody);
	return true;
}

bool CompileHelper::CompileWord(ExecState* pExecState, WordBodyElement** pCFA) {
	int64_t nCompileState;
	pExecState->GetVariable("#compileState", nCompileState);
	if (nCompileState==2) {
		pLastWordCreated->CompileXTIntoWord(pCFA[0]->wordElement_XT, 0);
	}
	else if (this->pWordUnderCreation->GetBodySize() == 0) {
		// TODO This code doesn't work.  Fix it once looking at other defining words
		//      The issue is, when defining a defining word, the first element of newly minted words (from defining word)
		//       currently has to be level-1, because the CFA has to be a direct XT.  
		//      If the word is a second-level word, the CFA direct XT would be DOCOL, but, the rest of that word wouldn't be
		//       present in the newly minted word's body, that body would have whatever was put there by the new defining word
		//      This code doesn't work because it prevents any DOCOLs (even valid onces) from being placed in the CFA XT, and there
		//       is no easy way of determining if the word being compiled here is a second-level word (CFA XT of DOCOL) or the first-level
		//       word DOCOL (CFA XT of DOCOL).  
		// Potential fix:
		//      Have any defining words that need a second-level word as their function, to have the following as there code:
		//      DOCOL
		//      HERE
		//      4        <=== different from HERE to data
		//      +
		//      PUSHDATA <=== this is the word we want to actually execute.  It is called with a value on the stack of where the data is stored
		//      EXIT
		//      data
		// Non-functioning code:
		//if (pCFA[0]->wordElement_XT == ForthWord::BuiltIn_DoCol) {
		//	// The word that is being compiled is a second level word, which means on executing DoCol, the rest of the body would contain
		//	//  this word under compilation, not the body that the DoCol expects.
		//	// Instead, have the CFA point to the XT for IndirectDoCol, which expects the correct cfa point in the next word
		//	this->pWordUnderCreation->CompileXTIntoWord(ForthWord::BuiltIn_IndirectDoCol);
		//	this->pWordUnderCreation->CompileCFAPterIntoWord(pCFA);
		//}
		//else {
			// First code word has to be a WordBodyElement containing an XT, not a pointer to a body whose first element contains an XT.
		this->pWordUnderCreation->CompileXTIntoWord(pCFA[0]->wordElement_XT);
		//}
	}
	else {
		this->pWordUnderCreation->CompileCFAPterIntoWord(pCFA);
	}
	return true;
}

bool CompileHelper::CompileWord(ExecState* pExecState, const string& wordName) {
	ForthWord* pWord = pExecState->pDict->FindWord(wordName);
	if (pWord == nullptr) {
		return pExecState->CreateException("Could not find word to compile");
	}

	WordBodyElement** pCFA = pWord->GetPterToBody();
	this->pWordUnderCreation->CompileCFAPterIntoWord(pCFA);
	return true;
}

bool CompileHelper::CompileDoesXT(ExecState* pExecState, XT does) {
	if (pWordUnderCreation == nullptr) {
		return pExecState->CreateException("Cannot call does> when not compiling");
	}
	this->pWordUnderCreation->CompileXTIntoWord(does, 0);
	return true;
}

/// <summary>
/// Alter an element in the word-under-creation's bbody.  This should only be called for non-garbaged collected words such as integers.  Typically 
///  this is used to alter literal used for jumping, when executing control words whilst defining other words.
/// </summary>
/// <param name="pExecState">Current execution state</param>
/// <param name="addr">Offset in word body to alter</param>
/// <param name="pReplacementElement">WordBodyElement to replace element with</param>
/// <returns>true if successful, false otherwise</returns>
/// <returns></returns>
bool CompileHelper::AlterElementInWordUnderCreation(ExecState* pExecState, int addr, WordBodyElement* pReplacementElement) {
	if (this->pWordUnderCreation == nullptr) {
		return pExecState->CreateException("No word under creation, so cannot alter element within it");
	}

	// TODO Send this change to word itself, to check to see if it is within limits of the array
	WordBodyElement* pWBE = this->pWordUnderCreation->GetPterToBody()[addr];
	delete pWBE;
	pWBE = nullptr;
	this->pWordUnderCreation->GetPterToBody()[addr] = pReplacementElement;
	return true;
}

/// <summary>
/// Alter an element in a words body.  This should only be called for non-garbaged collected words such as integers.  Typically 
///  this is used to alter literal used for jumping, when executing control words whilst defining other words.
/// </summary>
/// <param name="pExecState">Current execution state</param>
/// <param name="pWord">Word being altered</param>
/// <param name="addr">Offset in word body to alter</param>
/// <param name="pReplacementElement">WordBodyElement to replace element with</param>
/// <returns>true if successful, false otherwise</returns>
bool CompileHelper::AlterElementInWord(ExecState* pExecState, ForthWord* pWord, int addr, WordBodyElement* pReplacementElement) {
	if (pWord == nullptr) {
		return pExecState->CreateException("No word specified, cannot alter its contents");
	}

	WordBodyElement* pWBE = pWord->GetPterToBody()[addr];
	delete pWBE;
	pWBE = nullptr;
	pWord->GetPterToBody()[addr] = pReplacementElement;
	return true;
}

bool CompileHelper::ExecuteLastWordCompiled(ExecState* pExecState) {
	if (this->pLastWordCreated == nullptr) {
		return pExecState->CreateException("Cannot execute last word created - there isn't one");
	}

	WordBodyElement** ppExecBody = this->pLastWordCreated->GetPterToBody();
	bool response = true;

	XT executeXT = ppExecBody[0]->wordElement_XT;
	pExecState->NestAndSetCFA(ppExecBody, 1);
	try {
		response = executeXT(pExecState);
	}
	catch (...) {
		// TODO Create a better message here
		pExecState->CreateException("Execution caused exception");
		pExecState->UnnestCFA();
		throw;
	}

	pExecState->UnnestCFA();

	return response;
}

bool CompileHelper::ExpandLastWordCompiledBy(ExecState* pExecState, int expandBy) {
	if (this->pLastWordCreated == nullptr) {
		return pExecState->CreateException("Cannot allot to last word compiled - there isn't one");
	}
	pLastWordCreated->ExpandBy(expandBy);
	return true;
}