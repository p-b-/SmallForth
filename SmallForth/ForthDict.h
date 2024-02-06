#pragma once
#include "RefCountedObject.h"
#include <string>
#include <map>
using namespace std;

class ForthWord;

class ForthDict : public RefCountedObject
{
public:
	ForthDict();
	~ForthDict();

	void AddWord(ForthWord* wordToAdd);
	ForthWord* FindWord(const string& word) const;

	ForthWord* FindWordFromCFAPter(WordBodyElement** pPterToCFA);
	bool ForgetWord(string wordName);
	int WordCount() const;

	virtual string GetObjectType();
	virtual bool ToString(ExecState* pExecState) const;
	virtual bool InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke);

private:
	map<string, ForthWord*> words;
};
