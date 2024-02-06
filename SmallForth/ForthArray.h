#pragma once
#include <vector>
using namespace std;
#include "RefCountedObject.h"

class ExecState;
class StackElement;

class ForthArray :
    public RefCountedObject
{
public:
	ForthArray(ForthDict* pDict);
	~ForthArray();
	virtual string GetObjectType();
	virtual bool ToString(ExecState* pExecState) const;

	virtual bool InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke);

	static bool Construct(ExecState* pExecState);

	void SetContainedType(ForthType t) { this->containedType = t; }
	ForthType GetContainedType() const { return this->containedType; }

private:
	bool GetSize(ExecState* pExecState);
	bool Append(ExecState* pExecState);
	bool ElementAtIndex(ExecState* pExecState);
	bool SetElementAtIndex(ExecState* pExecState);
private:
	ForthType containedType;
	vector<StackElement*> elements;
};

