#include <iostream>
#include <list>
#include "ForthDefs.h"
#include "ForthWord.h"
#include "DataStack.h"
#include "ReturnStack.h"
#include "ExecState.h"
#include "ForthDict.h"
#include "TypeSystem.h"
#include "CompileHelper.h"
#include "DebugHelper.h"
#include "ForthFile.h"
#include "PreBuiltWords.h"

ForthWord::ForthWord(const std::string& name) :
	RefCountedObject(nullptr) {
	this->objectType = ObjectType_Word;
	this->name = name;
	this->bodySize = 0;
	this->body = nullptr;
	this->immediate = false;
	this->visible = false;
}

ForthWord::ForthWord(const std::string& name, XT firstXT) :
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

std::string ForthWord::GetObjectType() {
	return "word";
}

bool ForthWord::ToString(ExecState* pExecState) const {
	std::string str = "Word: " + this->name;
	if (!pExecState->pStack->Push(str)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthWord::InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke) {
	return false;
}


// TODO Fix this
//      This runs to either first EXIT word, or the first contained literal.  Needs to consider length of body.
bool ForthWord::BuiltIn_DescribeWord(ExecState* pExecState) {
	std::ostream* pStdoutStream = pExecState->GetStdout();

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
		std::list<Breakpoint>* pBreakpoints = pExecState->pDebugger->GetBreakpointsForWord(pCFA);
		ForthWord* pInitialWord = pExecState->pDict->FindWordFromCFAPter(pCFA);
		(*pStdoutStream) << "Word: " << pInitialWord->GetName() << std::endl;
		delete pTop;
		pTop = nullptr;
		WordBodyElement* pEl = pCFA[0];

		ForthType upcomingWordType = 0;
		bool upcomingWordIsLiteralType = false;
		bool upcomingWordIsLiteral = false;

		int bodySize = -1;

		std::string firstWord;
		if (pEl->wordElement_XT == PreBuiltWords::BuiltIn_DoCol) {
			firstWord = "DOCOL";
		}
		else if (pEl->wordElement_XT == PreBuiltWords::BuiltIn_PushPter)
		{
			firstWord = "PUSHPTER";
			bodySize = 2;
			upcomingWordIsLiteralType = true;
		}
		else if (pEl->wordElement_XT == PreBuiltWords::BuiltIn_FetchLiteral) {
			firstWord = "FETCHLITERAL";
			bodySize = 2;
			upcomingWordIsLiteralType = true;
		}
		else if (pEl->wordElement_XT != PreBuiltWords::BuiltIn_DoCol) {
			success = pExecState->CreateException("Cannot decompile a level-one word");
		}
		if (success) {
			(*pStdoutStream) << "0: " << firstWord << std::endl;
			int ip = 0;
			bool loop = true;

			while (loop) {
				++ip;
				WordBodyElement* pEl = pCFA[ip];
				ForthWord* pWord = pExecState->pDict->FindWordFromCFAPter(pEl->wordElement_BodyPter);
				if (pWord == nullptr) {
					if (upcomingWordIsLiteralType) {
						upcomingWordType = pEl->forthType;
						std::string typeDescription = pTS->TypeToString(upcomingWordType);
						(*pStdoutStream) << ip << ":    literal type (" << typeDescription << ")" << std::endl;
						upcomingWordIsLiteralType = false;
						upcomingWordIsLiteral = true;
					}
					else if (upcomingWordIsLiteral) {
						(*pStdoutStream) << ip << ":    literal value (";
						std::string contentsOfLiteral;
						bool successAtLiteral;
						if (pTS->IsValueOrValuePter(upcomingWordType)) {
							successAtLiteral = pTS->VariableToString(pExecState, upcomingWordType, reinterpret_cast<void*>(pEl->wordElement_int));
						}
						else {
							successAtLiteral = pTS->VariableToString(pExecState, upcomingWordType, pEl->pter);
						}
						if (!successAtLiteral) {
							(*pStdoutStream) << " could not retrieve)" << std::endl;
							return false;
						}
						successAtLiteral;
						tie(successAtLiteral, contentsOfLiteral) = pExecState->pStack->PullAsString();
						if (!successAtLiteral) {
							return pExecState->CreateException(contentsOfLiteral.c_str());
						}

						(*pStdoutStream) << contentsOfLiteral;
						(*pStdoutStream) << ")" << std::endl;
						upcomingWordIsLiteral = false;
					}
				}
				else {
					if (pWord->body[0]->wordElement_XT == PreBuiltWords::BuiltIn_PushUpcomingLiteral) {
						upcomingWordIsLiteralType = true;
					}
					std::string debugAnnotation = "   ";
					if (pBreakpoints!=nullptr) {
						for (auto& bp : *pBreakpoints) {
							if (bp.GetIP() == ip) {
								if (bp.IsEnabled()) {
									debugAnnotation = "+B ";
								}
								else {
									debugAnnotation = "xB ";
								}
							}
						}
					}
					(*pStdoutStream) << ip << ": " << debugAnnotation << pWord->GetName() << std::endl;
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
	return success;
}