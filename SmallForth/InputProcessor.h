#pragma once
#include <list>
#include <deque>
#include "ForthDefs.h"
class ForthDict;
class DataStack;
class ForthWord;

class InputProcessor
{
public:
	InputProcessor();

	bool Interpret(ExecState* pExecState);
	void SetInputString(const std::string& line);
	InputWord GetNextWord(ExecState* pExecState);
	char GetNextChar();
	void ClearRestOfLine();

	static bool ExecuteHaltRequested() { return s_executionToHalt; }
	static void ResetExecutionHaltFlag() { s_executionToHalt = false; }

private:
	void HandleException(ExecState* pExecState, const std::exception* pException, const std::string& msg);

	std::tuple<ForthWord*, bool> GetForthWordFromVocabOrObject(ExecState* pExecState);
	bool WordMatchesXT(ForthWord* pWord, XT xtToMatch);

	void ReadAndProcess(ExecState* pExecState);
	std::string ReadLine(std::ostream* pStdout, ExecState* pExecState);

	bool ConvertToInt(const std::string& word, int64_t& n);
	bool ConvertToFloat(const std::string& word, double& n);

	ForthWord* FindWordInTOSWord(ExecState* pExecState, const std::string& wordName);
	ForthWord* FindWordInObjectDefinition(ExecState* pExecState, const std::string& wordName);

private:
	void ProcessLine(const std::string& line, char delimiter);
	int ProcessScanLineCode(std::ostream* pStdout, std::string& line, int cursorPositionInLine, int scanlineCode);
	void MoveToLineEnd(std::ostream* pStdout, std::string& line, int& cursorPositionInLine);
	void ProcessBackspace(std::ostream* pStdout, std::string& line, int& currenmtPositionInLine);
	int InsertIntoLine(std::ostream* pStdout, std::string& line, int cursorPositionInLine, char toInsert);
	void WriteLineFromPosition(std::ostream* pStdout, const std::string& line, int cursorPosition, bool addExtraSpaceForDeletion);
	void MoveToPosition(std::ostream* pStdout, const std::string& line, int currentPosition, int position);
	void BlankCurrentLine(std::ostream* pStdout, const std::string& line, int currentPosition);

	void CalculatePositionInLine(int currenmtPositionInLine, int newPosition, int& x, int& y);
	void CalculateLineStartPosition(int currenmtPositionInLine, int& startX, int& startY);
	void SetCursorPosition(int x, int y);
	void GetCursorPosition(int& x, int& y);
	void GetCursorPositionAndSize(int& x, int& y, int& width, int& height);
	void GetConsoleSize(int& width, int& height);
	
	std::string MoveUpInHistory();
	std::string MoveDownInHistory();
	void AddLineToHistory(const std::string& line);

	static int HandlerRoutine(unsigned long fdwCtrlType);

private:
	std::list<InputWord> inputWords;
	bool processFromString;
	bool processingFromStringFinished;
	int commandHistoryLine;
	int historySize;
	std::deque<std::string> commandHistory;
	bool exitApplication;

	volatile static bool s_executionToHalt;
};

