#pragma once
#include "ForthDefs.h"
#include <map>
#include <list>

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

	void AddBreakpoint(WordBodyElement** word, int ip);
	void ToggleBreakpoint(WordBodyElement** word, int ip);
	void RemoveBreakpoint(WordBodyElement** word, int ip);
	std::list<Breakpoint>* GetBreakpointsForWord(WordBodyElement** word);
	static const Breakpoint* GetBreakpointForIP(std::list<Breakpoint>* pBreakpoints, int ip);

private:
	std::map< WordBodyElement**, std::list<Breakpoint>> _breakpoints;
};

