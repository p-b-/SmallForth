#include <iostream>
#include "ForthFile.h"
#include "ExecState.h"
#include "DataStack.h"
#include "ForthString.h"
#include "PreBuiltWords.h"

ForthFile::ForthFile(ForthType objectType, const std::string& filename) :
	RefCountedObject(nullptr) {
	this->pFile = nullptr;
	this->filename = filename;
	this->objectType = objectType;
	this->systemFile = forth_undefinedSystemFile;

	InitialiseReadWriteFlags();
}

ForthFile::ForthFile(ForthType objectType, SystemFiles systemFile) :
	RefCountedObject(nullptr) {
	this->pFile = nullptr;
	this->objectType = objectType;
	this->systemFile = systemFile;
	switch (systemFile) {
	case forth_stdout: this->filename = "stdout"; break;
	case forth_stderr: this->filename = "stderr"; break;
	default: this->filename = "unknown system file"; break;
	}
	InitialiseReadWriteFlags();
}

ForthFile::~ForthFile() {
	delete this->pFile;
	this->pFile = nullptr;
}

void ForthFile::InitialiseReadWriteFlags() {
	this->readFile = this->objectType == ObjectType_ReadFile || this->objectType == ObjectType_ReadWriteFile;
	this->writeFile = this->objectType == ObjectType_WriteFile || this->objectType == ObjectType_ReadWriteFile;
}

std::string ForthFile::GetObjectType() {
	return "file";
}

bool ForthFile::ToString(ExecState* pExecState) const {
	std::string pushString;
	
	switch (this->objectType) {
	case ObjectType_ReadFile: pushString = "readfile: " + this->filename; break;
	case ObjectType_WriteFile: pushString = "writefile: " + this->filename; break;
	case ObjectType_ReadWriteFile: pushString = "readwritefile: " + this->filename; break;
	default: pushString = "unknown file object type: " + this->filename; break;
	}
	
	if (!pExecState->pStack->Push(pushString)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;

}

bool ForthFile::InvokeFunctionIndex(ExecState* pExecState, ObjectFunction functionToInvoke) {
	switch (functionToInvoke) {
	case Function_Append: return Append(pExecState);
	case Function_Close: return CloseFile(pExecState);
	case Function_Read: return Read(pExecState);
	case Function_ReadLine: return ReadLine(pExecState);
	case Function_ReadChar: return ReadChar(pExecState);
	case Function_EOF: return IsEOF(pExecState);
	}
	return false;
}

bool ForthFile::ConstructReadFile(ExecState* pExecState) {
	StackElement* pElementConstructWith = pExecState->pStack->Pull();
	if (pElementConstructWith == nullptr) {
		return pExecState->CreateStackUnderflowException("when opening a file to read");
	}
	bool success;
	if (pElementConstructWith->GetType() == StackElement_Int) {
		SystemFiles fileToConstruct = (SystemFiles)pElementConstructWith->GetInt();
		success = ConstructStandardFile(pExecState, fileToConstruct);
	}
	else if (pElementConstructWith->GetType() == ObjectType_String) {
		ForthString* pFilename = (ForthString* )pElementConstructWith->GetObject();
		std::string filename = pFilename->GetContainedString();
		success = ConstructWithPath(ObjectType_ReadFile, pExecState, filename);
	}
	else {
		success = pExecState->CreateException("Cannot construct read file, requires ( $\n -- file )");
	}
	delete pElementConstructWith;
	pElementConstructWith = nullptr;

	return success;
}

bool ForthFile::ConstructWriteFile(ExecState* pExecState) {
	StackElement* pElementConstructWith = pExecState->pStack->Pull();
	if (pElementConstructWith == nullptr) {
		return pExecState->CreateStackUnderflowException("when opening a file to write");
	}
	bool success;
	if (pElementConstructWith->GetType() == StackElement_Int) {
		SystemFiles fileToConstruct = (SystemFiles)pElementConstructWith->GetInt();
		success = ConstructStandardFile(pExecState, fileToConstruct);
	}
	else if (pElementConstructWith->GetType() == ObjectType_String) {
		ForthString* pFilename = (ForthString*)pElementConstructWith->GetObject();
		std::string filename = pFilename->GetContainedString();
		success = ConstructWithPath(ObjectType_WriteFile, pExecState, filename);
	}
	else {
		success = pExecState->CreateException("Cannot construct write file, requires ( $\n -- file )");
	}
	delete pElementConstructWith;
	pElementConstructWith = nullptr;

	return success;
}

bool ForthFile::ConstructReadWriteFile(ExecState* pExecState) {
	StackElement* pElementConstructWith = pExecState->pStack->Pull();
	if (pElementConstructWith == nullptr) {
		return pExecState->CreateStackUnderflowException("when opening a file for read/write");
	}
	bool success;
	if (pElementConstructWith->GetType() == ObjectType_String) {
		ForthString* pFilename = (ForthString*)pElementConstructWith->GetObject();
		std::string filename = pFilename->GetContainedString();
		success = ConstructWithPath(ObjectType_ReadWriteFile, pExecState, filename);
	}
	else {
		success = pExecState->CreateException("Cannot construct read/write file, requires ( $ -- file )");
	}
	delete pElementConstructWith;
	pElementConstructWith = nullptr;

	return success;
}


bool ForthFile::ConstructStandardFile(ExecState* pExecState, SystemFiles stdFileToConstruct) {
	ForthFile* pForthFile = nullptr;
	
	bool success = false;
	if (stdFileToConstruct == forth_stdout) {
		pForthFile = new ForthFile(ObjectType_WriteFile, stdFileToConstruct);
		pForthFile->pFile = new std::fstream(stdout);
		success = true;
	}
	else if (stdFileToConstruct == forth_stderr) {
		pForthFile = new ForthFile(ObjectType_WriteFile, stdFileToConstruct);
		pForthFile->pFile = new std::fstream(stderr);
		success = true;
	}
	else if (stdFileToConstruct == forth_stdin) {
		pForthFile = new ForthFile(ObjectType_ReadFile, stdFileToConstruct);
		pForthFile->pFile = new std::fstream(stdin);
		success = true;
	}
	else {
		return pExecState->CreateException("Could not create standard file of that type");
	}

	if (success == false) {
		delete pForthFile;
		pForthFile = nullptr;
	}
	else if (!pExecState->pStack->Push((RefCountedObject*)pForthFile)) {
		success = pExecState->CreateStackOverflowException("when pushing a new standard file to stack");
	}

	return success;
}


bool ForthFile::ConstructWithPath(ForthType objectType, ExecState* pExecState, const std::string& filepath) {
	std::ios::openmode mode;

	switch (objectType) {
	case ObjectType_ReadFile: mode = std::ios::in;
		break;
	case ObjectType_WriteFile: mode = std::ios::out | std::ios::app;
		break;
	case ObjectType_ReadWriteFile: mode = std::ios::in | std::ios::out || std::ios::app;
		break;
	default:
		return pExecState->CreateException("Could not open a file as read/write access is unknown");
	}

	ForthFile* pForthFile = new ForthFile(objectType, filepath);
	pForthFile->pFile = new std::fstream();

	pForthFile->pFile->open(filepath, mode);
	bool success = true;
	if (pForthFile->pFile->bad()) {
		success = pExecState->CreateExceptionUsingErrorNo("Could not open file, bad state: ");
	}
	else if (pForthFile->pFile->fail()) {
		success = pExecState->CreateExceptionUsingErrorNo("Could not open file, operation failed: ");
	}
	else if (!pForthFile->pFile->good()) {
		success = pExecState->CreateExceptionUsingErrorNo("Could not open file, not good state");
	}
	if (success == false) {
		delete pForthFile;
		pForthFile = nullptr;
	}
	else if (!pExecState->pStack->Push((RefCountedObject*)pForthFile)) {
		success = pExecState->CreateStackOverflowException("when pushing a new file to stack");
	}
	return success;
}

bool ForthFile::Append(ExecState* pExecState) {
	if (pFile->is_open() == false) {
		return pExecState->CreateException("Cannot append to a file that is not open");
	}
	else if (!this->writeFile) {
		return pExecState->CreateException("Cannot append to a file that is not open with write permissions");
	}
	if (!PreBuiltWords::ToString(pExecState)) {
		return false;
	}

	std::string str;
	bool success;
	tie(success, str) = pExecState->pStack->PullAsString();
	if (success) {
		(*this->pFile) << str;
		return true;
	}
	else {
		return pExecState->CreateException(str.c_str());
	}
}


bool ForthFile::CloseFile(ExecState* pExecState) {
	if (pFile->is_open() == false) {
		return pExecState->CreateException("Cannot close a file that is not open");
	}
	else if (this->systemFile != forth_undefinedSystemFile) {
		return pExecState->CreateException("Cannot close stdin, stdout or stderr");
	}
	pFile->close();
	return true;
}

bool ForthFile::Read(ExecState* pExecState) {
	if (pFile->is_open() == false) {
		return pExecState->CreateException("Cannot read from a file that is not open");
	}
	else if (!this->readFile) {
		return pExecState->CreateException("Cannot read from a file that is not open with read permissions");
	}

	std::string next;
	(*pFile) >> next;
	if (!pExecState->pStack->Push(next)) {
		return pExecState->CreateStackOverflowException("whilst pushing the output from reading a file");
	}

	return true;
}

bool ForthFile::ReadLine(ExecState* pExecState) {
	if (pFile->is_open() == false) {
		return pExecState->CreateException("Cannot read from a file that is not open");
	}
	else if (!this->readFile) {
		return pExecState->CreateException("Cannot read from a file that is not open with read permissions");
	}
	std::string line;
	// Could use this instead:
	//getline(*this->pFile, line, '\n');
	const int buffLen = 10;
	char buff[buffLen];
	bool loop = true;
	while (loop) {
		pFile->getline(buff, buffLen, '\n');
		int receivedLen = (int)strlen(buff);

 		if (pFile->rdstate() & std::ios_base::failbit) {
			if (buff[0] == '\0') {
				// no data read
				loop = false;
			}
			else {
				if (buff[receivedLen - 1] == '\r') {
					// Remove any carriage returns caused by windows formatting
					buff[receivedLen - 1] = '\0';
				}
				// Fail bit set - clear it and loop to get more charactesr
				line.append(buff);
				pFile->clear(pFile->rdstate() & ~std::ios_base::failbit);
			}
		}
		else if (receivedLen>0) {
			if (buff[receivedLen - 1] == '\r') {
				// Remove any carriage returns caused by windows formatting
				buff[receivedLen - 1] = '\0';
			}
			line.append(buff);
			if (receivedLen < buffLen) {
				loop = false;
			}
		}
		else {
			loop = false;
		}
	}

	if (!pExecState->pStack->Push(line)) {
		return pExecState->CreateStackOverflowException("could not push line from read file");
	}
	return true;
}

bool ForthFile::ReadChar(ExecState* pExecState) {
	if (pFile->is_open() == false) {
		return pExecState->CreateException("Cannot read from a file that is not open");
	}
	else if (!this->readFile) {
		return pExecState->CreateException("Cannot read from a file that is not open with read permissions");
	}
	int c = pFile->get();
	char chToPush;
	if (c == EOF) {
		chToPush = '\0';
	}
	else {
		chToPush = (char)c;
	}
	bool success = true;
	if (!pExecState->pStack->Push(chToPush)) {
		success = pExecState->CreateStackOverflowException("whilst reading a character from a file");
	}
	return success;
}

bool ForthFile::IsEOF(ExecState* pExecState) {
	if (pFile->is_open() == false) {
		return pExecState->CreateException("Cannot determine EOF state from a file that is not open");
	}
	bool success = true;
	if (!pExecState->pStack->Push(pFile->eof())) {
		success = pExecState->CreateStackOverflowException("whilst reading eof state of a file");
	}
	return success;
}
