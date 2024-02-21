#pragma once
#include "RefCountedObject.h"
#include <vector>
#include <string>
class StackElement;
class ExecState;
class ForthDict;

class UserDefinedObject : public RefCountedObject
{
public:
	UserDefinedObject(std::string name, int stateCount, ForthDict* pDict);
	virtual ~UserDefinedObject();

	virtual void IncReference();
	virtual void DecReference();
	virtual void IncReferenceBy(int by);
	virtual void DecReferenceBy(int by);

	virtual std::string GetObjectType();
	virtual bool ToString(ExecState* pExecState) const;
	virtual bool InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke);

	bool DefineObjectFromStack(ExecState* pExecState);
	void SetType(ForthType type);

	UserDefinedObject* Construct(ExecState* pExecState);

private:
	void ConsiderRefCountChangeForLinkedObject(StackElement* pElement, int changeDirection);

	bool ElementAtIndex(ExecState* pExecState);
	bool SetElementAtIndex(ExecState* pExecState);
	bool DeconstructToStack(ExecState* pExecState);

private:
	std::string objectName;
	int stateCount;
	bool defaultObject;

	std::vector<StackElement*> state;
};