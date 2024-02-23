#pragma once
#include "ForthDefs.h"
#include <map>
#include <list>
class ExecState;
class ForthWord;

class Breakpoint {
public:
	Breakpoint(int ip) { _enabled = true, _ip = ip; }
	Breakpoint(const Breakpoint& rhs);
	Breakpoint(Breakpoint&& rhs) noexcept;
	Breakpoint& operator=(const Breakpoint& other);
	Breakpoint& operator=(Breakpoint&& other) noexcept;
	bool operator==(const Breakpoint& rhs) const;

	bool IsEnabled() const { return _enabled; }
	void ToggleEnabled() { _enabled = !_enabled; }
	int GetIP() const { return _ip; }
private:
	bool _enabled;
	int _ip;
};

class DebugHelper
{
public:
	DebugHelper();
	~DebugHelper();

	bool AddBreakpoint(ExecState* pExecState, WordBodyElement** word, int ip);
	bool ToggleBreakpoint(ExecState* pExecState, WordBodyElement** word, int ip);
	bool RemoveBreakpoint(ExecState* pExecState, WordBodyElement** word, int ip);
	bool HasBreakpoints(WordBodyElement** word);

	std::list<Breakpoint>* GetBreakpointsForWord(WordBodyElement** word);
	const Breakpoint* GetBreakpoint(WordBodyElement** word, int ip);
	bool ProcessDebuggerInput(ExecState* pExecState, WordBodyElement** pWordBodyBeingDebugged, ForthWord* pWord, int executingIP, XT xtToExecute, int64_t& nDebugState, bool& stepOver, std::ostream* pStdoutStream, int indentation);
private:
	bool ProcessBreakpoint(ExecState* pExecState, WordBodyElement** pWordBodyBeingDebugged, int executingIP, int64_t& nDebugState, bool& hitBreakpoint, bool& allowEnableBreakpoint, bool& allowDisableBreakpoint);
	std::string CreateBreakpointCommand(bool allowAddBreakpoint, bool allowRemoveBreakpoint, bool allowEnableBreakpoint, bool allowDisableBreakpoint);
	bool ProcessDebuggerInput(ExecState* pExecState, char c, WordBodyElement** pWordBodyBeingDebugged, int executingIP, int64_t& nDebugState, bool& stepOver, bool& loopOnDebugLine, bool& hasBreakpoints, bool allowAddBreakpoint, bool allowRemoveBreakpoint, bool allowEnableBreakpoint, bool allowDisableBreakpoint, std::ostream* pStdoutStream);

private:
	std::map< WordBodyElement**, std::list<Breakpoint>> _breakpoints;
};

