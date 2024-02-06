#include "ForthDefs.h"
#include <iostream>
#include <sstream>
using namespace std;
#include "InputProcessor.h"
#include "ForthDict.h"
#include "ForthWord.h"
#include "DataStack.h"
#include "ReturnStack.h"
#include "ExecState.h"
#include "TypeSystem.h"
#include "ForthString.h"
#include "ForthFile.h"
#include "Vector3.h"
#include "PreBuiltWords.h"
#include "CompileHelper.h"
#include "ForthArray.h"

void initialiseDict(ForthDict* pDict,ExecState* pExecState);
bool interpretForth(ExecState* pExecState, const string& toExecute);
void createTypeWords(ExecState* pExecState);
void createFileTypes(ExecState* pExecState);
void initialiseWord(ForthDict* pDict, const string& wordName, XT wordCode);
void initialiseImmediateWord(ForthDict* pDict, const string& wordName, XT wordCode);
void initialiseTypeSystem(ExecState* pExecState);

int main()
{
    ForthDict* pDict = new ForthDict();
    pDict->IncReference();
    
    InputProcessor* pProcessor = new InputProcessor();
    DataStack* pDataStack = new DataStack(40);
    ReturnStack* pReturnStack = new ReturnStack(40);
    CompileHelper* pCompiler = new CompileHelper();
    ExecState* pExecState = new ExecState(pDataStack, pDict, pProcessor, pReturnStack, pCompiler);

    initialiseTypeSystem(pExecState);

    initialiseDict(pDict, pExecState);

    pProcessor->Interpret(pExecState);

    delete pProcessor;
    delete pDataStack;
    delete pExecState;

    pDict->DecReference();
}

void initialiseTypeSystem(ExecState* pExecState) {
    TypeSystem* ts = TypeSystem::GetTypeSystem();
    ts->RegisterValueType(pExecState, "undefined");
    ts->RegisterValueType(pExecState, "char");
    ts->RegisterValueType(pExecState, "int");
    ts->RegisterValueType(pExecState, "float");
    ts->RegisterValueType(pExecState, "bool");
    ts->RegisterValueType(pExecState, "type");
    ts->RegisterValueType(pExecState, "xt");
    ts->RegisterValueType(pExecState, "binaryopstype");
    ts->RegisterValueType(pExecState, "ptertocfa");

    ts->RegisterObjectType(pExecState, "word", nullptr);
    ts->RegisterObjectType(pExecState, "dict", nullptr);
    ts->RegisterObjectType(pExecState, "string", ForthString::Construct);
    ts->RegisterObjectType(pExecState, "readfile", ForthFile::ConstructReadFile);
    ts->RegisterObjectType(pExecState, "writefile", ForthFile::ConstructWriteFile);
    ts->RegisterObjectType(pExecState, "readwritefile", ForthFile::ConstructReadWriteFile);
    ts->RegisterObjectType(pExecState, "array", ForthArray::Construct);
    ts->RegisterObjectType(pExecState, "vector3", Vector3::Construct, Vector3::BinaryOps);
}

void initialiseDict(ForthDict* pDict, ExecState* pExecState) {
    PreBuiltWords::RegisterWords(pDict);
    PreBuiltWords::CreateSecondLevelWords(pExecState);

    createTypeWords(pExecState);
    createFileTypes(pExecState);
}

bool interpretForth(ExecState* pExecState, const string &toExecute) {
    pExecState->pInputProcessor->SetInputString(toExecute);
    return pExecState->pInputProcessor->Interpret(pExecState);
}

void createTypeWords(ExecState* pExecState) {
    initialiseWord(pExecState->pDict, "typefromint", PreBuiltWords::BuiltIn_TypeFromInt); // (v addr -- )
    stringstream ss;
    ss << ": word_type " << ObjectType_Word << " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": dict_type " << ObjectType_Dict<< " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": string_type " << ObjectType_String << " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": readfile_type " << ObjectType_ReadFile << " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": writefile_type " << ObjectType_WriteFile << " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": readwritefile_type " << ObjectType_ReadWriteFile << " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": array_type " << ObjectType_Array << " typefromint ; ";
    interpretForth(pExecState, ss.str());

    ss << ": vector3_type " << ObjectType_Vector3 << " typefromint ; ";
    interpretForth(pExecState, ss.str());
}

void createFileTypes(ExecState* pExecState) {
    stringstream ss;
    ss << ": #stdout " << forth_stdout << " ; ";
    interpretForth(pExecState, ss.str());

    ss.str(string());
    ss << ": #stderr " << forth_stderr << " ; ";
    interpretForth(pExecState, ss.str());

    ss.str(string());
    ss << ": #stdin " << forth_stdin << " ; ";
    interpretForth(pExecState, ss.str());

    ss.str(string());

    interpretForth(pExecState, "#stdout writefile_type construct constant stdout");
    interpretForth(pExecState, "#stderr writefile_type construct constant stderr");
    interpretForth(pExecState, "#stdin readfile_type construct constant stdin");
}

void initialiseWord(ForthDict* pDict, const string& wordName, XT wordCode) {
    ForthWord* pForthWord = new ForthWord(wordName, wordCode);
    pForthWord->SetWordVisibility(true);
    pDict->AddWord(pForthWord);
}

void initialiseImmediateWord(ForthDict* pDict, const string& wordName, XT wordCode) {
    ForthWord* pForthWord = new ForthWord(wordName, wordCode);
    pForthWord->SetImmediate(true);
    pForthWord->SetWordVisibility(true);
    pDict->AddWord(pForthWord);
}