#include "ForthDefs.h"
#include <sstream>
using namespace std;
#include "enumPrinters.h"

string ToString(ValueType valueType) {
	ostringstream out;

	switch (valueType) {
	case ValueType_Undefined:
		out << "undefined";
		break;
	case ValueType_Char:
		out << "char";
		break;
	case ValueType_Int:
		out << "int";
		break;
	case ValueType_Float:
		out << "float";
		break;
	case ValueType_Bool:
		out << "bool";
		break;
	case ValueType_Type:
		out << "type";
		break;
	//case ValueType_CharPter:
	//	out << "charpter";
	//	break;
	//case ValueType_IntPter:
	//	out << "intpter";
	//	break;
	//case ValueType_FloatPter:
	//	out << "floatpter";
	//	break;
	//case ValueType_BoolPter:
	//	out << "boolpter";
	//	break;
	//case ValueType_TypePter:
	//	out << "typepter";
	//	break;
	case ValueType_PterToCFA:
		out << "cfa";
		break;
	//case ValueType_PterToPterToCFA:
	//	out << "cfapter";
	//	break;
	case ValueType_XT:
		out << "xt";
		break;
	}
	return out.str();
}
