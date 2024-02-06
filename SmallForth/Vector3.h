#pragma once
#include "RefCountedObject.h"
class StackElement;

class Vector3 :
    public RefCountedObject
{
public:
    Vector3(double x, double y, double z);
    ~Vector3();
    virtual string GetObjectType();
    virtual bool ToString(ExecState* pExecState) const;
    virtual bool InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke);
    static bool Construct(ExecState* pExecState);
    static bool BinaryOps(ExecState* pExecState);
private:
    static bool GetXYZFromStack(ExecState* pExecState, double& x, double& y, double& z);

    static StackElement* ScalarMultiply(ForthType operandType, Vector3* pOperand1, StackElement* pElementOperand);
    static StackElement* ScalarDivide(ExecState* pExecState, ForthType operandType, Vector3* pOperand1, StackElement* pElementOperand);
    static StackElement* Add(ForthType operandType, Vector3* pOperand1, StackElement* pElementOperand);
    static StackElement* Subtract(ForthType operandType, Vector3* pOperand1, StackElement* pElementOperand);
    static StackElement* VectorsEqualOrNotEquals(bool equals, ForthType operandType, Vector3* pOperand1, StackElement* pElementOperand);
    

    bool GetSize(ExecState* pExecState);
    bool ElementAtIndex(ExecState* pExecState);
    bool SetElementAtIndex(ExecState* pExecState);
    bool DeconstructToStack(ExecState* pExecState);
private:
    double x;
    double y;
    double z;
};

