#include "DebugHelper.h"

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

void DebugHelper::AddBreakpoint(WordBodyElement** word, int ip) {
	std::list<Breakpoint>* pBreakpoints = GetBreakpointsForWord(word);

	if (pBreakpoints == nullptr) {
		std::list<Breakpoint> blist = std::list<Breakpoint>();
		blist.emplace_back(Breakpoint(ip));
		_breakpoints[word] = blist;
	}
	else {
		pBreakpoints->emplace_back(Breakpoint(ip));
	}
}

void DebugHelper::ToggleBreakpoint(WordBodyElement** word, int ip) {
	std::list<Breakpoint>* pBreakpoints = GetBreakpointsForWord(word);

	if (pBreakpoints != nullptr) {
		std::list<Breakpoint>& blist = *pBreakpoints;
		for (auto& bp : blist) {
			if (bp.GetIP() == ip) {
				bp.ToggleEnabled();
			}
		}
	}
}

void DebugHelper::RemoveBreakpoint(WordBodyElement** word, int ip) {
	std::list<Breakpoint>* pBreakpoints = GetBreakpointsForWord(word);

	if (pBreakpoints!=nullptr) {
		std::list<Breakpoint>& blist = *pBreakpoints;
		for (auto& bp : blist) {
			if (bp.GetIP() == ip) {
				blist.remove(bp);
				break;
			}
		}
	}
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

