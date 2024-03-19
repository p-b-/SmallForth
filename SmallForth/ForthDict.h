#pragma once
#include "RefCountedObject.h"
#include <string>
#include <map>

class ForthWord;
class WordBodyElement;

class ForthDict : public RefCountedObject
{
public:
	ForthDict();
	~ForthDict();

	void AddWord(ForthWord* wordToAdd);
	ForthWord* FindWord(const std::string& word) const;

	ForthWord* FindWordFromCFAPter(WordBodyElement** pPterToCFA);
	bool ForgetWord(std::string wordName);
	int WordCount() const;

	virtual std::string GetObjectType();
	virtual bool ToString(ExecState* pExecState) const;
	virtual bool InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke);

private:
	std::map<std::string, ForthWord*> words;
};
