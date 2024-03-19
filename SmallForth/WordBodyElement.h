#pragma once
#include "ForthDefs.h"

class WordBodyElement
{
public:
	union {
		XT wordElement_XT;
		int64_t wordElement_int;
		double wordElement_float;
		bool wordElement_bool;
		char wordElement_char;
		WordBodyElement** wordElement_BodyPter;

		void* refCountedPter;
		ForthType forthType;
	};
	int16_t refCount;
};

