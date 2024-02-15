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
#include "ReturnStack.h"
#include "ExecState.h"
#include "TypeSystem.h"
#include "CompileHelper.h"
#include "ForthFile.h"
#include "PreBuiltWords.h"

volatile bool InputProcessor::s_executionToHalt = false;

int InputProcessor::HandlerRoutine(unsigned long fdwCtrlType) {
	if (fdwCtrlType == CTRL_C_EVENT) {
		InputProcessor::s_executionToHalt = true;
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
	bool bInsideCommentLine;
	pExecState->GetVariable("#insideCommentLine", bInsideCommentLine);
	if (bInsideCommentLine || wordName.length() == 0) {
		return { nullptr, executeOnTOSObject };
	}
	pExecState->delimitersAfterCurrentWord = wordWithDelimiterCount.postDelimiterCount;
	ForthWord* pWord = nullptr;
	int64_t nCompileState;
	pExecState->GetVariable("#compileState", nCompileState);

	if (pExecState->nextWordIsCharLiteral) {
		pExecState->nextWordIsCharLiteral = false;

		if (nCompileState == 1) {
			pExecState->pStack->Push((char)(wordName[0]));
			pWord = pExecState->pDict->FindWord("literal");
		}
	}
	if (pWord == nullptr) {
		if (nCompileState == 1) {
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
			if (pWord == nullptr) {
				bool bInsideComment;
				pExecState->GetVariable("#insideComment", bInsideComment);

				if (bInsideComment == false) {
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

					if (nCompileState == 1) {
						pWord = pExecState->pDict->FindWord("literal");
					}
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
		bool bInsideComment;
		pExecState->GetVariable("#insideComment", bInsideComment);

		if (bInsideComment && WordMatchesXT(pWord, PreBuiltWords::BuiltIn_ParenthesisCommentEnd) == false)
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

		int64_t nCompileState;
		pExecState->GetVariable("#compileState", nCompileState);

		try {
			bool executeWordNow = false;
			if (nCompileState==1) {
				bool bExecutePostponed;
				pExecState->GetVariable("#postponeState", bExecutePostponed);
				if (pWord->GetImmediate() && !bExecutePostponed) {
					executeWordNow = true;
				}
				else {
					pExecState->SetVariable("#postponeState", false);

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

					pExecState->SetVariable("#postponestate", false);
					pExecState->SetVariable("#compileState", (int64_t)0);
					pExecState->SetVariable("#insideComment", false);
					pExecState->SetVariable("#insideCommentLine", false);
				}
			}
		}
		// Exception handling outputs the exception to stderr, clears stack, and continues
		catch (const std::runtime_error& e)
		{
			HandleException(pExecState, &e, "Runtime exception: ");
		}
		catch (const std::logic_error& e)
		{
			HandleException(pExecState, &e, "Logic exception: ");
		}
		catch (const std::exception& e) {
			HandleException(pExecState, &e, "Exception: ");
		}
		catch (...) {
			HandleException(pExecState, nullptr, "Non std exception: ");
		}
	}

	delete pExecState;
	pExecState = nullptr;
	return true;
}

InputWord InputProcessor::GetNextWord(ExecState* pExecState) {
	while (this->inputWords.size() == 0) {
		pExecState->SetVariable("#insideCommentLine", false);

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

void InputProcessor::HandleException(ExecState* pExecState, const std::exception* pException, const string& msg) {
	pExecState->pStack->Clear();
	pExecState->pTempStack->Clear();
	pExecState->pSelfStack->Clear();
	pExecState->pReturnStack->Clear();
	ostream* pStderr = pExecState->GetStderr();
	(*pStderr) << msg;
	if (pException != nullptr) {
		(*pStderr) << pException->what();
	}

}

void InputProcessor::ClearRestOfLine() {
	this->inputWords.clear();
}

void InputProcessor::ReadAndProcess(ExecState* pExecState) {
	ostream* pStdout = pExecState->GetStdout();
	istream* pStdin = pExecState->GetStdin();

	int64_t nCompileState;
	pExecState->GetVariable("#compileState", nCompileState);

	try
	{
		while (true) {
			std::string line;
			bool bInsideComment;
			pExecState->GetVariable("#insideComment", bInsideComment);

			if (bInsideComment) {
				(*pStdout) << pExecState->commentPrompt;
			}
			else if (pExecState->insideStringLiteral) {
				(*pStdout) << pExecState->stringLiteralPrompt;
			}
			else if (nCompileState==1) {
				(*pStdout) << pExecState->compilePrompt;
			}
			else {
				(*pStdout) << pExecState->interpetPrompt;
			}
			// Cannot simply call getline, as that will not process ctrl-c correctly (ctrl-c on empy line exits programme. ctrl-c on line with input, cancels line)
			//  As getting input from cin only processes the ctrl-c once enter has been pressed
			line = ReadLine(pStdout, pExecState);
			if (exitApplication) {
				exit(0);
			}

			if (line.length() != 0) {
				ProcessLine(line, ' ');
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
		if (c == '\t') {
			c = ' ';
		}
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
	// Have to process forward/backward arrows, home/end, up and down (through history), insert characters, delete/backspace without relying on the console implementation
	// This is made more complex by the fact that the console, when told to advance from the last character on a console line (to the next line), actually keeps the cursor before the last 
	//  character on the line. Doing this prevents the code from calculating the start position of the current line - cannot just store the start position as it moves around on resizing and scrolling
	int cursorPositionInLine = 0;
	string line;
	while (true) {
		int c = _getch();
		if (c == 3) {
			// ctrl-c
			if (line.length() > 0) {
				(*pStdout) << "^c";
				line = string();
				(*pStdout) << "\n";
				cursorPositionInLine = 0;
				return line;
			}
			else {
				exitApplication = true;
				return "";
			}
		}
		if (c == 0xe0 || c==0x00) {
			// Start of scanline codes
			c = _getch();
			cursorPositionInLine = ProcessScanLineCode(pStdout, line, cursorPositionInLine, c);
		}
		else if (c == 27) {
			// escape
			BlankCurrentLine(pStdout, line, cursorPositionInLine);
			line = "";
			cursorPositionInLine = 0;
		}
		else if (c == 9) {
			// tab
			cursorPositionInLine = InsertIntoLine(pStdout, line, cursorPositionInLine, ' ');
			cursorPositionInLine = InsertIntoLine(pStdout, line, cursorPositionInLine, ' ');
		}
		else if (c == 8) {
			// Backspace
			if (cursorPositionInLine > 0) {
				ProcessBackspace(pStdout, line, cursorPositionInLine);
			}
		}
		else if (c == 13 || c == 10) {
			// This is to ensure that over a multi-(console)-line input, if the cursor isn't on the final console line
			//  that the further console output doesn't overwrite the input
			MoveToLineEnd(pStdout, line, cursorPositionInLine);
			(*pStdout) << '\n';
			break;
		}
		else {
			cursorPositionInLine = InsertIntoLine(pStdout, line, cursorPositionInLine, c);
		}
	}
	if (line.length() > 0) {
		AddLineToHistory(line);
	}
	return line;
}

int InputProcessor::ProcessScanLineCode(ostream* pStdout, string& line, int cursorPositionInLine, int scanlineCode) {
	if (scanlineCode == 0x4b && cursorPositionInLine > 0) {
		// Left arrow
		int x;
		int y;
		int width;
		int height;
		this->GetCursorPositionAndSize(x, y, width, height);
		if (x == 0)
		{
			SetCursorPosition(width - 1, y - 1);
		}
		else {
			SetCursorPosition(x - 1, y);
		}

		//(*pStdout) << '\b';
		cursorPositionInLine--;
	}
	else if (scanlineCode == 0x4d && cursorPositionInLine < line.length()) {
		// Right arrow

		// Normal console behaviour is odd at the end of a console line. It doesn't move the cursor the next console-line, or move it after the last character (on the console line), but keeps 
		//  it on (before) the last character on the line instead of after it.
		// This code moves it to the next line (as can't place it after it), which ensures backspace from end of console line (or next console line), works correctly.
		int x;
		int y;
		int width;
		int height;
		this->GetCursorPositionAndSize(x, y, width, height);
		if (x < width-1) {
			++x;
		}
		else {
			x = 0;
			++y;
		}
		++cursorPositionInLine;
		this->SetCursorPosition(x, y);
	}
	else if (scanlineCode == 0x48 && commandHistoryLine > 0) {
		// Go up in history
		BlankCurrentLine(pStdout, line, cursorPositionInLine);
		line = MoveUpInHistory();
		(*pStdout) << line;
		cursorPositionInLine = (int)line.length();
	}
	else if (scanlineCode == 0x50 && this->commandHistoryLine < this->commandHistory.size()) {
		// Go down in history
		BlankCurrentLine(pStdout, line, cursorPositionInLine);
		line = MoveDownInHistory();
		(*pStdout) << line;
		cursorPositionInLine = (int)line.length();
	}
	else if (scanlineCode == 0x53 && cursorPositionInLine < line.length()) {
		// Delete pressed
		line = line.substr(0, cursorPositionInLine) + line.substr(cursorPositionInLine + 1);
		WriteLineFromPosition(pStdout, line, cursorPositionInLine, true);
	}
	else if (scanlineCode == 0x47) {
		// home
		MoveToPosition(pStdout, line, cursorPositionInLine, 0);
		cursorPositionInLine = 0;
	}
	else if (scanlineCode == 0x4f) {
		// end
		MoveToLineEnd(pStdout, line, cursorPositionInLine);
	}

	return cursorPositionInLine;
}

void InputProcessor::MoveToLineEnd(ostream* pStdout, string& line, int& cursorPositionInLine) {
	MoveToPosition(pStdout, line, cursorPositionInLine, (int)line.length());
	cursorPositionInLine = (int)line.length();
}

void InputProcessor::ProcessBackspace(ostream* pStdout, string& line, int& cursorPositionInLine) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	int startX;
	int startY;
	CalculateLineStartPosition(cursorPositionInLine, startX, startY);

	// Backspace won't work if cursor is currently at the start on the console line (as in the text line overspills)
	int x;
	int y;
	int width;
	int height;
	this->GetCursorPositionAndSize(x, y, width, height);
	if (y > startY && x == 0) {
		SetCursorPosition(width - 1, y - 1);
		(*pStdout) << " ";
		SetCursorPosition(width-1, y - 1);
	}
	else {
		// Backsace is fine here
		(*pStdout) << (char)'\b';
	}

	line = line.substr(0, cursorPositionInLine - 1) + line.substr(cursorPositionInLine);
	--cursorPositionInLine;
	WriteLineFromPosition(pStdout, line, cursorPositionInLine, true);
}

int InputProcessor::InsertIntoLine(ostream* pStdout, string& line, int cursorPositionInLine, char toInsert) {
	// First insert the character into the string line
	if (line.length() == 0) {
		line += toInsert;
	}
	else if (cursorPositionInLine == 0) {
		line.insert(line.begin(), toInsert);
	}
	else {
		line.insert(cursorPositionInLine, 1, toInsert);
	}
	// Then update the console to display line correctly. False means do not add a space on the end (which is used when deleting characters)
	//  Note, this method needs the cursorPosition, as it used to calculate where the line starts in the console, to re-draw the line fully
	WriteLineFromPosition(pStdout, line, cursorPositionInLine, false);
	int x;
	int y;
	int width;
	int height;
	this->GetCursorPositionAndSize(x, y, width, height);

	// Ensure that if the cursor ends up at the end of the line, it is moved to the next line. To prevent errant behaviour by the console at line ends
	if (x == width - 1) {
		x = 0;
		y++;
		// Weirdly this line does not move console down if debugging
		(*pStdout) << "\n";
		if (y >= height) {
			y = height - 1;
		}
	}
	else {
		x++;
	}
	SetCursorPosition(x, y);
	return ++cursorPositionInLine;
}

void InputProcessor::WriteLineFromPosition(ostream* pStdout, const string& line, int cursorPosition, bool addExtraSpaceForDeletion) {
	int x;
	int y;
	GetCursorPosition(x, y);

	int startX;
	int startY;
	this->CalculateLineStartPosition(cursorPosition, startX, startY);
	SetCursorPosition(startX, startY);

	int len = (int)line.length();

	(*pStdout) << line;
	if (addExtraSpaceForDeletion) {
		len++;
		(*pStdout) << " ";
	}
	int startX_AfterLineOut;
	int startY_AfterLineOut;
	this->CalculateLineStartPosition(len, startX_AfterLineOut, startY_AfterLineOut);
	if (startY_AfterLineOut < startY) {
		// Console has scrolled
		y -= startY - startY_AfterLineOut;
	}
	SetCursorPosition(x, y);
}

void InputProcessor::MoveToPosition(ostream* pStdout, const string& line, int currentPosition, int position) {
	int x;
	int y;
	this->CalculatePositionInLine(currentPosition, position, x,y);
	this->SetCursorPosition(x, y);
}

void InputProcessor::BlankCurrentLine(ostream* pStdout, const string& line, int currentPosition) {

	int startX;
	int startY;
	this->CalculateLineStartPosition(currentPosition, startX, startY);
	this->SetCursorPosition(startX, startY);

	for (int i = (int)line.length(); i > 0; --i) {
		(*pStdout) << " ";
	}
	this->SetCursorPosition(startX, startY);
}

void InputProcessor::CalculatePositionInLine(int currenmtPositionInLine, int newPosition, int& x, int& y) {
	int startX;
	int startY;
	CalculateLineStartPosition(currenmtPositionInLine, startX, startY);

	int width;
	int height;
	GetConsoleSize(width, height);

	x = startX;
	y = startY;

	if (newPosition < width - startX) {
		x = startX + newPosition;
	}
	else {
		newPosition -= (width - startX);
		x = 0;
		y++;
		x += (newPosition % width);
		y += (newPosition / width);
	}
}

void InputProcessor::CalculateLineStartPosition(int currentPositionInLine, int& startX, int& startY) {
	int width;
	int height;
	int x;
	int y;
	this->GetCursorPositionAndSize(x, y, width, height);
	int lines = currentPositionInLine / width;
	int offsetX = currentPositionInLine % width;

	startX = x - offsetX;
	startY = y - lines;

	if (startX < 0) {
		// Scrolled to next line
		startY -= 1;
		startX += width;
	}
}

void InputProcessor::GetCursorPositionAndSize(int& x, int& y, int& width, int& height) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	GetConsoleScreenBufferInfo(hStdout, &consoleInfo);
	x = consoleInfo.dwCursorPosition.X;
	y = consoleInfo.dwCursorPosition.Y;
	width = consoleInfo.dwSize.X;
	height = consoleInfo.dwSize.Y;
}

void InputProcessor::GetConsoleSize(int& width, int& height) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	GetConsoleScreenBufferInfo(hStdout, &consoleInfo);
	width = consoleInfo.dwSize.X;
	height = consoleInfo.dwSize.Y;
}

void InputProcessor::GetCursorPosition(int& x, int& y) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	GetConsoleScreenBufferInfo(hStdout, &consoleInfo);
	x = consoleInfo.dwCursorPosition.X;
	y = consoleInfo.dwCursorPosition.Y;
}

void InputProcessor::SetCursorPosition(int x, int y) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coords;
	coords.X = x;
	coords.Y = y;
	SetConsoleCursorPosition(hStdout, coords);
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
	int64_t nCompileState;
	pExecState->GetVariable("#compileState", nCompileState);

	if (pElement != nullptr) {
		ForthType tosType = pElement->GetType();
		
		if (pTS->TypeIsObject(tosType)) {
			ForthWord* pWord = pTS->FindWordWithName(tosType, wordName);
			return pWord;
		}
		else if (nCompileState == 1 && tosType == StackElement_Type) {
			// When compiling words on objects, can use previously defined object words. FindWordInObjectDefinition adds the current
			//  defining object to the stack before call this method.
			return pTS->FindWordWithName(pElement->GetValueType(), wordName);
		}
	}
	return nullptr;
}

ForthWord* InputProcessor::FindWordInObjectDefinition(ExecState* pExecState, const string& wordName) {
	if (!pExecState->PushVariableValueOntoStack("#compileForType")) {
		pExecState->exceptionThrown = false;
		return nullptr;
	}
	ForthWord* pWord = FindWordInTOSWord(pExecState, wordName);
	StackElement* pElementType = pExecState->pStack->Pull();
	delete pElementType;
	pElementType = nullptr;
	return pWord;
}