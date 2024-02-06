#include "ForthDefs.h"
#include <iostream>
#include <conio.h>
#include <string>
#include <sstream>

#include <Windows.h>
#include "InputProcessor.h"
#include "ForthDict.h"
#include "ForthWord.h"
#include "DataStack.h"
#include "ExecState.h"
#include "TypeSystem.h"
#include "CompileHelper.h"
#include "ForthFile.h"

int InputProcessor::HandlerRoutine(unsigned long fdwCtrlType) {
	if (fdwCtrlType == CTRL_C_EVENT) {
		return TRUE;
	}
	return FALSE;
}

InputProcessor::InputProcessor() {
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	processFromString = false;
	processingFromStringFinished = false;
	commandHistoryLine = 0;
	exitApplication = false;
	historySize = 500;
}

tuple<ForthWord*, bool> InputProcessor::GetForthWordFromVocabOrObject(ExecState* pExecState) {
	bool executeOnTOSObject = false;
	InputWord wordWithDelimiterCount = GetNextWord(pExecState);
	string wordName = wordWithDelimiterCount.word;
	if (pExecState->insideLineComment || wordName.length() == 0) {
		return { nullptr, executeOnTOSObject };
	}
	ForthWord* pWord = nullptr;
	if (pExecState->nextWordIsCharLiteral) {
		pExecState->nextWordIsCharLiteral = false;
		if (pExecState->compileState) {
			pExecState->pStack->Push((char)(wordName[0]));
			pWord = pExecState->pDict->FindWord("literal");
		}
	}
	if (pWord == nullptr) {
		if (pExecState->compileState) {
			pWord = FindWordInObjectDefinition(pExecState, wordName);
		}
		if (pWord == nullptr) {
			pWord = FindWordInTOSWord(pExecState, wordName);
		}
		if (pWord != nullptr) {
			executeOnTOSObject = true;
		}
		if (pWord == nullptr)
		{
			pWord = pExecState->pDict->FindWord(wordName);
			if (pWord == nullptr && pExecState->insideComment == false) {
				int64_t intValue;
				double floatValue;
				if (ConvertToInt(wordName, intValue)) {
					pExecState->pStack->Push(intValue);
				}
				else if (ConvertToFloat(wordName, floatValue)) {
					pExecState->pStack->Push(floatValue);
				}
				else {
					ClearRestOfLine();
					ostream* pStderr = pExecState->GetStderr();
					(*pStderr) << "Unknown word: " << wordName << endl;
				}

				if (pExecState->compileState) {
					pWord = pExecState->pDict->FindWord("literal");
				}
			}
		}
	}
	return { pWord, executeOnTOSObject };
}

bool InputProcessor::WordMatchesXT(ForthWord* pWord, XT xtToMatch) {
	WordBodyElement** pBody = pWord->GetPterToBody();
	if (pWord->GetBodySize() == 0) {
		return false;
	}
	return pBody[0]->wordElement_XT == xtToMatch;
}

bool InputProcessor::Interpret(ExecState* pExecState) {
	while (true) {
		ForthWord* pWord;
		bool executeOnTOSObject;
		tie(pWord,executeOnTOSObject) = GetForthWordFromVocabOrObject(pExecState);
		if (pWord == nullptr) {
			if (processingFromStringFinished) {
				processingFromStringFinished = false;
				if (pExecState->exceptionThrown) {
					return false;
				}
				return true;
			}
			continue;
		}
		if (pExecState->insideComment && WordMatchesXT(pWord, ForthWord::BuiltIn_ParenthesisCommentEnd) == false)
		{
			continue;
		}
		pExecState->exceptionThrown = false;
		WordBodyElement** pCFA = pWord->GetPterToBody();
		// Push address of body, onto stack (for EXECUTE to find)
		StackElement* pCFAElement = new StackElement(pCFA);
		if (!pExecState->pStack->Push(pCFAElement)) {
			ostream* pStderr = pExecState->GetStderr();
			(*pStderr) << "Stack overflow" << endl;
			continue;
		}

		try {
			bool executeWordNow = false;
			if (pExecState->compileState) {
				if (pWord->GetImmediate() && !pExecState->executionPostponed) {
					executeWordNow = true;
				}
				else {
					pExecState->executionPostponed = false;
					// Compile word
					if (!pExecState->pCompiler->CompileWordOnStack(pExecState)) {
						ostream* pStderr = pExecState->GetStderr();
						(*pStderr) << "Exception: " << pExecState->pzException << endl;
						// TODO Stop compilation, clear input, execute QUIT (or return it as no stack-control in CPP)
					}
				}
			}
			else {
				executeWordNow = true;
			}
			if (executeWordNow) {
				if (executeOnTOSObject) {
					pExecState->ExecuteWordDirectly("executeonobject");
				}
				else {
					pExecState->ExecuteWordDirectly("execute");
				}

				if (pExecState->exceptionThrown) {
					ClearRestOfLine();
					ostream* pStderr = pExecState->GetStderr();
					(*pStderr) << "Exception: " << pExecState->pzException << endl;
				}
			}
		}
		catch (...) {
			delete pExecState;
			pExecState = nullptr;
			throw;
		}
	}

	delete pExecState;
	pExecState = nullptr;
	return true;
}

InputWord InputProcessor::GetNextWord(ExecState* pExecState) {
	while (this->inputWords.size() == 0) {
		pExecState->insideLineComment = false;
		if (processFromString)
		{
			processFromString = false;
			processingFromStringFinished = true;
			InputWord iw;
			return iw;
		}
		ReadAndProcess(pExecState);
	}
	InputWord wordToReturn = this->inputWords.front();
	this->inputWords.pop_front();
	return wordToReturn;
}

void InputProcessor::ClearRestOfLine() {
	this->inputWords.clear();
}

void InputProcessor::ReadAndProcess(ExecState* pExecState) {
	ostream* pStdout = pExecState->GetStdout();
	istream* pStdin = pExecState->GetStdin();
	try
	{
		while (true) {
			std::string line;
			if (pExecState->insideComment) {
				(*pStdout) << pExecState->commentPrompt;
			}
			else if (pExecState->insideStringLiteral) {
				(*pStdout) << pExecState->stringLiteralPrompt;
			}
			else if (pExecState->compileState) {
				(*pStdout) << pExecState->compilePrompt;
			}
			else {
				(*pStdout) << pExecState->interpetPrompt;
			}

			line = ReadLine(pStdout, pExecState);
			if (exitApplication) {
				exit(0);
			}
			//std::getline(std::cin, line);

			if (line.length() != 0) {
				ProcessLine(line, ' ');
				//stringstream ss(line);
				//string word;
				//while (ss >> word) {
				//	InputWord w;
				//	w.postDelimiterCount = 0;
				//	w.word = word;
				//	this->inputWords.push_back(w);
				//}
				return;
			}
			std::cin.clear();
		}
	}
	catch (...)
	{
		ostream* pStderr = pExecState->GetStderr();
		(*pStderr) << "Exiting\n";
	}
}

void InputProcessor::ProcessLine(const string& line, char delimiter) {
	int len = (int) line.length();
	int currentWordStart = -1;
	int delimitersAfterCurrentWord = 0;
	for (int n =0;n<len;++n) {
		char c = line[n];
		if (currentWordStart == -1) {
			if (c != delimiter) {
				currentWordStart = n;
				delimitersAfterCurrentWord = 0;
			}
		}
		else if (c == delimiter) {
			++delimitersAfterCurrentWord;
		}
		else if (delimitersAfterCurrentWord > 0) {
			// Found start of next word after delimiters
			string word = line.substr(currentWordStart, n - currentWordStart - delimitersAfterCurrentWord);
			InputWord w;
			w.postDelimiterCount = delimitersAfterCurrentWord;
			w.word = word;
			this->inputWords.push_back(w);
			delimitersAfterCurrentWord = 0;
			currentWordStart = n;
		}
	}
	if (currentWordStart != -1) {
		string word = line.substr(currentWordStart, len - currentWordStart - delimitersAfterCurrentWord);
		InputWord w;
		w.postDelimiterCount = delimitersAfterCurrentWord;
		w.word = word;
		this->inputWords.push_back(w);
	}
}

string InputProcessor::ReadLine(ostream* pStdout, ExecState* pExecState) {
	int cursorPosition = 0;
	string line;
	while (true) {
		int c = _getch();
		if (c == 3) {
			if (line.length() > 0) {
				(*pStdout) << "^c";
				line = string();
				(*pStdout) << "\n";
				cursorPosition = 0;
				return line;
			}
			else {
				exitApplication = true;
				return "";
			}
		}
		if (c == 0xe0 || c==0x00) {
			c = _getch();
			cursorPosition = ProcessScanLineCode(pStdout, line, cursorPosition, c);
		}
		else if (c == 27) {
			// escape
			BlankCurrentLine(pStdout, line, cursorPosition);
			line = "";
			cursorPosition = 0;
		}
		else if (c == 9) {
			// tab
			cursorPosition = InsertIntoLine(pStdout, line, cursorPosition, ' ');
			cursorPosition = InsertIntoLine(pStdout, line, cursorPosition, ' ');
		}
		else if (c == 8) {
			// Backspace
			if (cursorPosition > 0) {
				(*pStdout) << (char)c;
				line = line.substr(0, cursorPosition - 1) + line.substr(cursorPosition);
				--cursorPosition;
				WriteLineFromPosition(pStdout, line, cursorPosition);
			}
		}
		else if (c == 13 || c == 10) {
			(*pStdout) << '\n';
			break;
		}
		else {
			cursorPosition = InsertIntoLine(pStdout, line, cursorPosition, c);
		}
	}
	if (line.length() > 0) {
		AddLineToHistory(line);
	}
	return line;
}

int InputProcessor::ProcessScanLineCode(ostream* pStdout, string& line, int cursorPosition, int scanlineCode) {
	if (scanlineCode == 0x4b && cursorPosition > 0) {
		// Left arrow
		(*pStdout) << '\b';
		cursorPosition--;
	}
	else if (scanlineCode == 0x4d && cursorPosition < line.length()) {
		// Move forward by printing character out
		(*pStdout) << line.substr(cursorPosition, 1);
		++cursorPosition;
	}
	else if (scanlineCode == 0x48 && commandHistoryLine > 0) {
		// Go up in history
		BlankCurrentLine(pStdout, line, cursorPosition);
		line = MoveUpInHistory();
		(*pStdout) << line;
		cursorPosition = (int)line.length();
	}
	else if (scanlineCode == 0x50 && this->commandHistoryLine < this->commandHistory.size()) {
		// Go down in history
		BlankCurrentLine(pStdout, line, cursorPosition);
		line = MoveDownInHistory();
		(*pStdout) << line;
		cursorPosition = (int)line.length();
	}
	else if (scanlineCode == 0x53 && cursorPosition < line.length()) {
		// Delete pressed
		line = line.substr(0, cursorPosition) + line.substr(cursorPosition + 1);
		WriteLineFromPosition(pStdout, line, cursorPosition);
	}
	else if (scanlineCode == 0x47) {
		// home
		MoveToPosition(pStdout, line, cursorPosition, 0);
		cursorPosition = 0;
	}
	else if (scanlineCode == 0x4f) {
		// end
		MoveToPosition(pStdout, line, cursorPosition, (int)line.length());
		cursorPosition = (int)line.length();
	}

	return cursorPosition;
}

int InputProcessor::InsertIntoLine(ostream* pStdout, string& line, int cursorPosition, char toInsert) {
	(*pStdout) << toInsert;
	if (cursorPosition < (int)line.length()) {
		WriteLineFromPosition(pStdout, line, cursorPosition);
	}
	if (line.length() == 0) {
		line += toInsert;
	}
	else if (cursorPosition == 0) {
		line.insert(line.begin(), toInsert);
	}
	else {
		line.insert(cursorPosition, 1, toInsert);
	}
	return ++cursorPosition;
}

void InputProcessor::WriteLineFromPosition(ostream* pStdout, const string& line, int cursorPosition) {
	(*pStdout) << line.substr(cursorPosition);
	(*pStdout) << " ";
	for (int i = 0; i <= line.length() - cursorPosition; ++i) {
		(*pStdout) << '\b';
	}
}

void InputProcessor::MoveToPosition(ostream* pStdout, const string& line, int currentPosition, int position) {
	if (position < currentPosition) {
		for (int i = currentPosition - position; i > 0; --i) {
			(*pStdout) << '\b';
		}
	}
	else {
		for (int i = currentPosition; i < position; ++i) {
			(*pStdout) << line[i];
		}
	}
}

void InputProcessor::BlankCurrentLine(ostream* pStdout, const string& line, int currentPosition) {
	for (int i = 0; i < currentPosition; ++i) {
		cout << '\b';
	}
	for (int i = (int)line.length(); i > 0; --i) {
		cout << " ";
	}
	for (int i = (int)line.length(); i > 0; --i) {
		cout << '\b';
	}
}

string InputProcessor::MoveUpInHistory() {
	--commandHistoryLine;
	return this->commandHistory[commandHistoryLine];
}

string InputProcessor::MoveDownInHistory() {
	++this->commandHistoryLine;
	if (this->commandHistoryLine == this->commandHistory.size()) {
		return string();
	}
	else {
		return this->commandHistory[this->commandHistoryLine];
	}
}

void InputProcessor::AddLineToHistory(const string& line) {
	if (commandHistoryLine == this->commandHistory.size()) {
		this->commandHistory.push_back(line);
		++commandHistoryLine;
		while (this->commandHistory.size() > this->historySize) {
			this->commandHistory.pop_front();
			--commandHistoryLine;
		}
	}
	else if (this->commandHistory[commandHistoryLine].compare(line) == 0) {
		// Stay on current line
	}
	else {
		while (this->commandHistory.size() > this->commandHistoryLine) {
			this->commandHistory.pop_back();
		}
		this->commandHistory.push_back(line);
		++this->commandHistoryLine;
	}

}

void InputProcessor::SetInputString(const string& line) {
	this->processFromString = true;
	this->processingFromStringFinished = false;
	if (line.length() != 0) {
		ProcessLine(line, ' ');
		//stringstream ss(line);
		//string word;
		//while (ss >> word) {
		//	InputWord w;
		//	w.postDelimiterCount = 0;
		//	w.word = word;
		//	this->inputWords.push_back(w);
		//}
	}
}

// TODO Implement BASE
bool InputProcessor::ConvertToInt(const string& word, int64_t& n) {
	if (word.find(".")!=string::npos || word[word.length()-1]=='f') {
		return false;
	}

	char* end;
	n = strtoll(word.c_str(), &end,10);
	if (end == word.c_str() || *end != '\0') {
		return false;
	}
	return true;
}

bool InputProcessor::ConvertToFloat(const string& word, double& n) {
	string convertWord = word;
	if (word[word.length() - 1] == 'f') {
		convertWord = word.substr(0, word.length() - 1);
	}
	char* end;
	n = strtod(convertWord.c_str(), &end);
	if (end == convertWord.c_str() || *end != '\0') {
		return false;
	}
	return true;
}

ForthWord* InputProcessor::FindWordInTOSWord(ExecState* pExecState, const string& wordName) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement = pExecState->pStack->TopElement();
	if (pElement != nullptr) {
		ForthType tosType = pElement->GetType();
		
		if (pTS->TypeIsObject(tosType)) {
			ForthWord* pWord = pTS->FindWordWithName(tosType, wordName);
			return pWord;
		}
		else if (pExecState->compileState && tosType == StackElement_Type) {
			// When compiling words on objects, can use previously defined object words. FindWordInObjectDefinition adds the current
			//  defining object to the stack before call this method.
			return pTS->FindWordWithName(pElement->GetValueType(), wordName);
		}
	}
	return nullptr;
}

ForthWord* InputProcessor::FindWordInObjectDefinition(ExecState* pExecState, const string& wordName) {
	if (!pExecState->PushVariableOntoStack("#compileForType")) {
		pExecState->exceptionThrown = false;
		return nullptr;
	}
	ForthWord* pWord = FindWordInTOSWord(pExecState, wordName);
	StackElement* pElementType = pExecState->pStack->Pull();
	delete pElementType;
	pElementType = nullptr;
	return pWord;
}