#pragma once
#include <string>
#include <fstream>

#include "RefCountedObject.h"

using namespace std;

class ForthFile :
    public RefCountedObject
{
public:
    ForthFile(ForthType objectType, const string& filename);
    ForthFile(ForthType objectType, SystemFiles systemFile);
    ~ForthFile();

    fstream* GetContainedStream() { return this->pFile; }
    static bool ConstructReadFile(ExecState* pExecState);
    static bool ConstructWriteFile(ExecState* pExecState);
    static bool ConstructReadWriteFile(ExecState* pExecState);
    virtual string GetObjectType();
    virtual bool ToString(ExecState* pExecState) const;
    virtual bool InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke);
private:
    void InitialiseReadWriteFlags();
    bool Append(ExecState* pExecState);
    bool Read(ExecState* pExecState);
    bool ReadLine(ExecState* pExecState);
    bool ReadChar(ExecState* pExecState);
    bool IsEOF(ExecState* pExecState);
    bool CloseFile(ExecState* pExecState);

    static bool ConstructStandardFile(ExecState* pExecState, SystemFiles stdFileToConstruct);
    static bool ConstructWithPath(ForthType objectType, ExecState* pExecState, const string& filepath);

protected:
    string filename;
    SystemFiles systemFile;

private:
    fstream* pFile;

    bool readFile;
    bool writeFile;
};

