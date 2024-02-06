#pragma once
#include <string>
#include <vector>
#include <iostream>
using namespace std;
#include "RefCountedObject.h"
#include "StackElement.h"
#include "DataStack.h"
#include "TypeSystem.h"

class StackElement;
class ExecState;

class PreBuiltWords
{
public:
	// Low level words
	static bool PushSelf(ExecState* pExecState);
	static bool PushRefCount(ExecState* pExecState);

	// Definitions 
	static bool BuiltIn_Allot(ExecState* pExecState);

	// Strings and string output
	static bool ToString(ExecState* pExecState); // ( e -- $) formats element e and converts to string $
	
	// Value type manipulation
	static bool ToChar(ExecState* pExecState); // ( n -- c)
	static bool ToInt(ExecState* pExecState); // ( $/c/f -- n)
	static bool ToFloat(ExecState* pExecState); // ( $/n -- f)
	static bool WordToInt(ExecState* pExecState); // ( $ -- n)
	static bool WordToFloat(ExecState* pExecState); // ( $ -- f)

	static bool IsObject(ExecState* pExecState); // ( [e] -- b ) true if [e] is object, false if [e] is pter to anything or value
	static bool IsPter(ExecState* pExecState);

};

