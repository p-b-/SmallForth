#include "ForthDefs.h"
#include <algorithm>
#include "ForthDict.h"
#include "ForthWord.h"
#include "ExecState.h"
#include "DataStack.h"

ForthDict::ForthDict() :
	RefCountedObject() {
	objectType = ObjectType_Dict;
}

ForthDict::~ForthDict() {
	for (std::map<std::string, ForthWord*>::iterator iter = words.begin(); iter != words.end(); iter++) {
		ForthWord* pWord = iter->second;
		pWord->DecReference();
	}
}

void ForthDict::AddWord(ForthWord* wordToAdd) {
	std::string wordName = wordToAdd->GetName();
	std::transform(wordName.begin(), wordName.end(), wordName.begin(),
		[](unsigned char c) { return std::tolower(c); });
	wordToAdd->IncReference();
	this->words[wordName] = wordToAdd;
}

ForthWord* ForthDict::FindWord(const std::string& wordName) const {
	// This will break if characters in string are UTF-8, or anything more exotic than that.
	// TODO Consider using unicode libraries
	std::string lowerCaseName = wordName;
	std::transform(lowerCaseName.begin(), lowerCaseName.end(), lowerCaseName.begin(),
		[](unsigned char c) { return std::tolower(c); });

	auto wordIter = this->words.find(lowerCaseName);
	if (wordIter == this->words.end()) {
		return nullptr;
	}
	if (wordIter->second->Visible()) {
		return wordIter->second;
	}
	return nullptr;
}

ForthWord* ForthDict::FindWordFromCFAPter(WordBodyElement** pPterToCFA) {
	for (std::map<std::string, ForthWord*>::iterator iter = words.begin(); iter != words.end(); iter++) {
		ForthWord* pWord = iter->second;
		if (pWord->GetPterToBody() == pPterToCFA) {
			return pWord;
		}
	}
	return nullptr;
}

bool ForthDict::ForgetWord(std::string wordName) {
	std::transform(wordName.begin(), wordName.end(), wordName.begin(),
		[](unsigned char c) { return std::tolower(c); });

	std::map<std::string, ForthWord*>::iterator word = this->words.find(wordName);
	if (word != this->words.end()) {
		word->second->SetWordVisibility(false);
		// TODO Forgetting word will delete it - not required.  Will cause points to inside word to point at freed memory
		word->second->DecReference();
	}
	else {
		return false;
	}

	return true;
}

int ForthDict::WordCount() const {
	return (int)this->words.size();
}

std::string ForthDict::GetObjectType() {
	return "dict";
}

bool ForthDict::ToString(ExecState* pExecState) const {
	if (!pExecState->pStack->Push("dict")) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool ForthDict::InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke) {
	return false;
}
