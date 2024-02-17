#pragma once
#include <stdint.h>
#include <string>

using namespace std;

class ExecState;
class RefCountedObject;

typedef bool (*XT)(ExecState* pExecState);

enum ElementType {
	StackElement_Char = 1,
	StackElement_Int,
	StackElement_Float,
	StackElement_Bool,
	StackElement_Type,
	StackElement_XT,
	StackElement_BinaryOpsType,
	StackElement_PterToCFA
};

typedef uint32_t ForthType;

enum ValueType {
	ValueType_Undefined,
	ValueType_Char,
	ValueType_Int,
	ValueType_Float,
	ValueType_Bool,
	ValueType_Type,

	ValueType_XT,
	ValueType_BinaryOpsType,
	ValueType_PterToCFA
};

enum BinaryOperationType {
	BinaryOp_Undefined,
	BinaryOp_Add,
	BinaryOp_Subtract,
	BinaryOp_Multiply,
	BinaryOp_Divide,
	BinaryOp_Modulus,
	BinaryOp_LessThan,
	BinaryOp_LessThanOrEquals,
	BinaryOp_Equals,
	BinaryOp_GreaterThanOrEquals,
	BinaryOp_GreaterThan,
	BinaryOp_NotEquals
};

union WordBodyElement {

	XT wordElement_XT;
	int64_t wordElement_int;
	double wordElement_float;
	bool wordElement_bool;
	char wordElement_char;
	WordBodyElement** wordElement_BodyPter;
	void* pter;
	ForthType forthType;
};

enum ObjectFunction {
	Function_GetSize = 0,
	Function_Hash,
	Function_ElementAtIndex,
	Function_SetElementIndex,
	Function_Contains,
	Function_IndexOf,
	Function_SubRange,
	Function_Deconstruct,
	Function_Append,
	Function_Implements,
	Function_Close,
	Function_Read,
	Function_ReadLine,
	Function_ReadChar,
	Function_EOF
};

enum ObjectTypes {
	ObjectType_Word = 1024,
	ObjectType_Dict,
	ObjectType_String,
	ObjectType_ReadFile,
	ObjectType_WriteFile,
	ObjectType_ReadWriteFile,
	ObjectType_Array,
	ObjectType_Vector3
};

enum SystemFiles {
	forth_undefinedSystemFile = 0,
	forth_stdin = 1,
	forth_stdout = 2,
	forth_stderr = 3
};

struct InputWord {
	string word;
	int postDelimiterCount;
};
