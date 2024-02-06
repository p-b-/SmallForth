#pragma once
#include <list>
#include <deque>
#include "ForthDefs.h"
using namespace std;
class ForthDict;
class DataStack;
class ForthWord;

class InputProcessor
{
public:
	InputProcessor();

	bool Interpret(ExecState* pExecState);
	void SetInputString(const string& line);
	InputWord GetNextWord(ExecState* pExecState);
	void ClearRestOfLine();

private:
	tuple<ForthWord*, bool> GetForthWordFromVocabOrObject(ExecState* pExecState);
	bool WordMatchesXT(ForthWord* pWord, XT xtToMatch);

	void ReadAndProcess(ExecState* pExecState);
	string ReadLine(ostream* pStdout, ExecState* pExecState);

	bool ConvertToInt(const string& word, int64_t& n);
	bool ConvertToFloat(const string& word, double& n);

	ForthWord* FindWordInTOSWord(ExecState *pExecState, const string& wordName);
	ForthWord* FindWordInObjectDefinition(ExecState* pExecState, const string& wordName);

private:
	void ProcessLine(const string& line, char delimiter);
	int ProcessScanLineCode(ostream* pStdout, string& line, int cursorPosition, int scanlineCode);
	int InsertIntoLine(ostream* pStdout, string& line, int cursorPosition, char toInsert);
	void WriteLineFromPosition(ostream* pStdout, const string& line, int cursorPosition);
	void MoveToPosition(ostream* pStdout, const string& line, int currentPosition, int position);
	void BlankCurrentLine(ostream* pStdout, const string& line, int currentPosition);
	string MoveUpInHistory();
	string MoveDownInHistory();
	void AddLineToHistory(const string& line);

	static int HandlerRoutine(unsigned long fdwCtrlType);

private:

	list<InputWord> inputWords;
	bool processFromString;
	bool processingFromStringFinished;
	int commandHistoryLine;
	int historySize;
	deque<string> commandHistory;
	bool exitApplication;
};

