#pragma once
#include <string>
#include "RefCountedObject.h"
class StackElement;

class ForthString : public RefCountedObject
{
public:
	ForthString(const std::string& pzString);
	~ForthString();
	virtual std::string GetObjectType();
	virtual bool ToString(ExecState* pExecState) const;

	virtual bool InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke);

	static bool Construct(ExecState* pExecState);

	std::string GetContainedString() const { return containedString; }

private:
	bool GetSize(ExecState* pExecState);
	bool ElementAtIndex(ExecState* pExecState);
	bool SetElementAtIndex(ExecState* pExecState);
	bool Contains(ExecState* pExecState);
	bool IndexOf(ExecState* pExecState);
	bool SubString(ExecState* pExecState);
	bool Append(ExecState* pExecState);
	bool AppendToFile(ExecState* pExecState, StackElement* pElementFile);


private:
	std::string containedString;
};

