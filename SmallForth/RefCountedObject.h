#pragma once
#include <string>
#include "ForthDefs.h"
class ExecState;
class ForthWord;
class ForthDict;

class RefCountedObject
{
public:
	RefCountedObject(ForthDict* pDict);
	RefCountedObject();
	virtual ~RefCountedObject();

	virtual void IncReference();
	virtual void DecReference();
	virtual void IncReferenceBy(int by);
	virtual void DecReferenceBy(int by);

	int GetCurrentReferenceCount();
	uint32_t GetObjectTypeId() const { return objectType; }

	virtual std::string GetObjectType() = 0;
	virtual bool ToString(ExecState* pExecState) const = 0;
	virtual bool InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke) = 0;

	void AddWord(ForthWord* pWordToAdd);
	ForthWord* GetWordWithName(const std::string& wordName) const;
	int GetWordCount() const;

	void MarkForReferenceCounter(bool mark) { this->markedByReferenceCounter = mark; }
	bool GetMarkForReferenceCounter() const { return this->markedByReferenceCounter; }
protected:
	void InitialiseDictionary();
	int GetReferenceCount() const { return referenceCount; }

private:
	int referenceCount;

protected:
	uint32_t objectType;

	ForthDict* pDictionary;
	bool markedByReferenceCounter;
};

