#include <iostream>
#include <iomanip>
#include "DebugHelper.h"
#include "ExecState.h"
#include "InputProcessor.h"
#include "PreBuiltWords.h"
#include "ForthWord.h"
#include "WordBodyElement.h"

Breakpoint::Breakpoint(const Breakpoint& rhs) {
	_enabled = rhs._enabled;
	_ip = rhs._ip; 
}

Breakpoint::Breakpoint(Breakpoint&& rhs) noexcept {
	_enabled = rhs._enabled;
	_ip = rhs._ip;
	rhs._ip = 0;
}

Breakpoint& Breakpoint::operator=(const Breakpoint& other) { 
	_enabled = other._enabled; 
	_ip = other._ip; 
	return *this; 
}

Breakpoint& Breakpoint::operator=(Breakpoint&& other) noexcept {
	_enabled = other._enabled; 
	_ip = other._ip;
	other._ip = 0;
	return *this;
}

bool Breakpoint::operator==(const Breakpoint& rhs) const {
	if (&rhs == this) {
		return true;
	}
	return this->_enabled == rhs._enabled && this->_ip == rhs._ip;
}

DebugHelper::DebugHelper() {

}

DebugHelper::~DebugHelper() {

}

bool DebugHelper::AddBreakpoint(ExecState* pExecState, WordBodyElement** word, int ip) {
	std::list<Breakpoint>* pBreakpoints = GetBreakpointsForWord(word);

	if (pBreakpoints == nullptr) {
		std::list<Breakpoint> blist = std::list<Breakpoint>();
		blist.emplace_back(Breakpoint(ip));
		_breakpoints[word] = blist;
	}
	else {
		std::list<Breakpoint>& blist = *pBreakpoints;
		for (auto& bp : blist) {
			if (bp.GetIP() == ip) {
				return pExecState->CreateException("Breakpoint already exists on that IP");
			}
		}
		pBreakpoints->emplace_back(Breakpoint(ip));
	}
	return true;
}

bool DebugHelper::ToggleBreakpoint(ExecState* pExecState, WordBodyElement** word, int ip) {
	std::list<Breakpoint>* pBreakpoints = GetBreakpointsForWord(word);

	bool found = false;

	if (pBreakpoints != nullptr) {
		std::list<Breakpoint>& blist = *pBreakpoints;
		for (auto& bp : blist) {
			if (bp.GetIP() == ip) {
				bp.ToggleEnabled();
				found = true;
			}
		}
	}
	if (!found) {
		return pExecState->CreateException("No breakpoint at that IP to toggle");
	}
	return true;
}

bool DebugHelper::RemoveBreakpoint(ExecState* pExecState, WordBodyElement** word, int ip) {
	std::list<Breakpoint>* pBreakpoints = GetBreakpointsForWord(word);

	if (pBreakpoints!=nullptr) {
		std::list<Breakpoint>& blist = *pBreakpoints;
		for (auto& bp : blist) {
			if (bp.GetIP() == ip) {
				blist.remove(bp);
				return true;
			}
		}
	}
	return pExecState->CreateException("No break point at the IP to remove");
}

std::list<Breakpoint>* DebugHelper::GetBreakpointsForWord(WordBodyElement** word) {
	auto iter = this->_breakpoints.find(word);
	if (iter == this->_breakpoints.end()) {
		return nullptr;
	}
	else {
		return &iter->second;
	}
}

bool DebugHelper::HasBreakpoints(WordBodyElement** word) {
	std::list<Breakpoint>* pBreakpoints = GetBreakpointsForWord(word);

	if (pBreakpoints != nullptr) {
		std::list<Breakpoint>& blist = *pBreakpoints;
		return blist.size() > 0;
	}
	return false;
}

const Breakpoint* DebugHelper::GetBreakpoint(WordBodyElement** word, int ip) {
	std::list<Breakpoint>* pBreakpoints = GetBreakpointsForWord(word);

	if (pBreakpoints != nullptr) {
		std::list<Breakpoint>& blist = *pBreakpoints;
		for (auto& bp : blist) {
			if (bp.GetIP() == ip) {
				return &bp;
			}
		}
	}
	return nullptr;
}

bool DebugHelper::ProcessDebuggerInput(ExecState* pExecState, WordBodyElement** pWordBodyBeingDebugged, ForthWord* pWord, int executingIP, XT xtToExecute, int64_t& nDebugState, bool& stepOver, std::ostream* pStdoutStream, int indentation) {
	bool hasBreakpoints = HasBreakpoints(pWordBodyBeingDebugged);

	bool loopOnDebugLine = true;

	while (loopOnDebugLine) {
		if (InputProcessor::ExecuteHaltRequested()) {
			InputProcessor::ResetExecutionHaltFlag();
			return pExecState->CreateException("Halted");
		}

		bool hitBreakpoint = false;
		bool allowEnableBreakpoint = false;
		bool allowDisableBreakpoint = false;

		if (hasBreakpoints) {
			if (!ProcessBreakpoint(pExecState, pWordBodyBeingDebugged, executingIP, nDebugState, hitBreakpoint, allowEnableBreakpoint, allowDisableBreakpoint)) {
				return false;
			}
		}
		bool allowRemoveBreakpoint = hitBreakpoint || allowEnableBreakpoint;
		bool allowAddBreakpoint = !allowRemoveBreakpoint;

		std::string breakpointCommands = CreateBreakpointCommand(allowAddBreakpoint, allowRemoveBreakpoint, allowEnableBreakpoint, allowDisableBreakpoint);
		
		loopOnDebugLine = false;
		if (nDebugState < 3 && pWord != nullptr) {
			(*pStdoutStream) << std::setw(3) << executingIP << std::setw(0) << std::string(indentation, ' ');
			bool allowStepInto = false;

			if (xtToExecute != PreBuiltWords::BuiltIn_DoCol) {
				(*pStdoutStream) << pWord->GetName();
				if (nDebugState == 2) {
					(*pStdoutStream) << std::endl;
				}
				else {
					(*pStdoutStream) << " (1,2,3,O,R," << breakpointCommands << ") ";
				}
			}
			else {
				(*pStdoutStream) << pWord->GetName();
				if (nDebugState == 2) {
					(*pStdoutStream) << std::endl;
				}
				else {
					(*pStdoutStream) << " (1,2,3,I,O,R," << breakpointCommands << ") ";
				}
				allowStepInto = true;
			}
			if (nDebugState == 1) {
				loopOnDebugLine = true;
				char c = pExecState->pInputProcessor->GetNextChar();
				if (!ProcessDebuggerInput(pExecState, c, pWordBodyBeingDebugged, executingIP, nDebugState, stepOver, loopOnDebugLine, hasBreakpoints, allowAddBreakpoint, allowRemoveBreakpoint, allowEnableBreakpoint, allowDisableBreakpoint, pStdoutStream)) {
					return false;
				}

				(*pStdoutStream) << std::endl;
			}
		}
	}

	return true;
}

bool DebugHelper::ProcessBreakpoint(ExecState* pExecState, WordBodyElement** pWordBodyBeingDebugged, int executingIP, int64_t& nDebugState, bool& hitBreakpoint, bool& allowEnableBreakpoint, bool& allowDisableBreakpoint) {
	const Breakpoint* bp = GetBreakpoint(pWordBodyBeingDebugged, executingIP);
	if (bp != nullptr) {
		if (bp->IsEnabled()) {
			allowDisableBreakpoint = true;
			hitBreakpoint = true;
			if (nDebugState != 1) {
				nDebugState = 1;
				if (!pExecState->SetVariable("#debugState", nDebugState)) {
					return pExecState->CreateException("Could not set debugstate to DEBUG");
				}
			}
		}
		else {
			allowEnableBreakpoint = true;
		}
	}

	return true;
}

std::string DebugHelper::CreateBreakpointCommand(bool allowAddBreakpoint, bool allowRemoveBreakpoint, bool allowEnableBreakpoint, bool allowDisableBreakpoint) {
	std::string breakpointCommands;
	if (allowAddBreakpoint) {
		breakpointCommands += "B";
	}
	else if (allowRemoveBreakpoint) {
		breakpointCommands += "X";
	}
	if (allowEnableBreakpoint) {
		breakpointCommands += ",E";
	}
	else if (allowDisableBreakpoint) {
		breakpointCommands += ",D";
	}
	return breakpointCommands;
}

bool DebugHelper::ProcessDebuggerInput(ExecState* pExecState, char c, WordBodyElement** pWordBodyBeingDebugged, 
	int executingIP, int64_t& nDebugState, bool& stepOver, bool& loopOnDebugLine, bool& hasBreakpoints, bool allowAddBreakpoint, bool allowRemoveBreakpoint, bool allowEnableBreakpoint, bool allowDisableBreakpoint, std::ostream* pStdoutStream) {
	bool reloadBreakpoints = false;

	c = std::tolower(c);
	if (c == 'r') {
		// RUN!
		nDebugState = 2;
		if (!pExecState->SetVariable("#debugState", (int64_t)2)) {
			return pExecState->CreateException("Could not set debugstate to RUN");
		}
		loopOnDebugLine = false;
	}
	else if (c == 'o') {
		// When control returns to this level of recursion, with this flag set the debug state will be set back to 1
		stepOver = true;
		nDebugState = 3;
		if (!pExecState->SetVariable("#debugState", (int64_t)3)) {
			return pExecState->CreateException("Could not set debugstate to DEBUGOVER");
		}
		loopOnDebugLine = false;
	}
	else if (c == 'b') {
		if (allowAddBreakpoint) {
			if (!pExecState->pDebugger->AddBreakpoint(pExecState, pWordBodyBeingDebugged, executingIP)) {
				if (pExecState->exceptionThrown) {
					pExecState->exceptionThrown = false;
				}
			}
			reloadBreakpoints = true;
		}
	}
	else if (c == 'x') {
		if (allowRemoveBreakpoint) {
			if (!pExecState->pDebugger->RemoveBreakpoint(pExecState, pWordBodyBeingDebugged, executingIP)) {
				if (pExecState->exceptionThrown) {
					pExecState->exceptionThrown = false;
				}
			}
			reloadBreakpoints = true;
		}
	}
	else if (c == 'e') {
		if (allowEnableBreakpoint) {
			if (!pExecState->pDebugger->ToggleBreakpoint(pExecState, pWordBodyBeingDebugged, executingIP)) {
				if (pExecState->exceptionThrown) {
					pExecState->exceptionThrown = false;
				}
			}
			reloadBreakpoints = true;
		}
	}
	else if (c == 'd') {
		if (allowDisableBreakpoint) {
			if (!pExecState->pDebugger->ToggleBreakpoint(pExecState, pWordBodyBeingDebugged, executingIP)) {
				if (pExecState->exceptionThrown) {
					pExecState->exceptionThrown = false;
				}
			}
			reloadBreakpoints = true;
		}
	}
	else if (c == '1') {
		int64_t currentDebugState = nDebugState;
		if (!pExecState->SetVariable("#debugState", (int64_t)0)) {
			return pExecState->CreateException("Could not set debugstate to NODEBUG to print stack");
		}
		(*pStdoutStream) << std::endl;
		pExecState->ExecuteWordDirectly(".s");
		if (!pExecState->SetVariable("#debugState", currentDebugState)) {
			return pExecState->CreateException("Could not set debugstate to debug state after print stack");
		}
	}
	else if (c == '2') {
		int64_t currentDebugState = nDebugState;
		if (!pExecState->SetVariable("#debugState", (int64_t)0)) {
			return pExecState->CreateException("Could not set debugstate to NODEBUG to print return stack");
		}
		(*pStdoutStream) << std::endl;
		pExecState->ExecuteWordDirectly(".rs");
		if (!pExecState->SetVariable("#debugState", currentDebugState)) {
			return pExecState->CreateException("Could not set debugstate to debug state after print return stack");
		}
	}
	else if (c == '3') {
		int64_t currentDebugState = nDebugState;
		if (!pExecState->SetVariable("#debugState", (int64_t)0)) {
			return pExecState->CreateException("Could not set debugstate to NODEBUG to print temp stack");
		}
		(*pStdoutStream) << std::endl;
		pExecState->ExecuteWordDirectly(".ts");
		if (!pExecState->SetVariable("#debugState", currentDebugState)) {
			return pExecState->CreateException("Could not set debugstate to debug state after print temp stack");
		}
	}
	else {
		loopOnDebugLine = false;
	}
	if (reloadBreakpoints) {
		hasBreakpoints = HasBreakpoints(pWordBodyBeingDebugged);
	}
	return true;
}
