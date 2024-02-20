#pragma once
#include <string>
#include <fstream>

#include "RefCountedObject.h"

class ForthFile :
    public RefCountedObject
{
public:
    ForthFile(ForthType objectType, const std::string& filename);
    ForthFile(ForthType objectType, SystemFiles systemFile);
    ~ForthFile();

    std::fstream* GetContainedStream() { return this->pFile; }
    static bool ConstructReadFile(ExecState* pExecState);
    static bool ConstructWriteFile(ExecState* pExecState);
    static bool ConstructReadWriteFile(ExecState* pExecState);
    virtual std::string GetObjectType();
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
    static bool ConstructWithPath(ForthType objectType, ExecState* pExecState, const std::string& filepath);

protected:
    std::string filename;
    SystemFiles systemFile;

private:
    std::fstream* pFile;

    bool readFile;
    bool writeFile;
};

