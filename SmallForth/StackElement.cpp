#include "ForthDefs.h"
#include "StackElement.h"
#include "TypeSystem.h"
#include "ExecState.h"
#include "DataStack.h"
#include "ForthString.h"
#include <sstream>
using namespace std;

#include "enumPrinters.h"

StackElement::StackElement(const StackElement& element) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	elementType = element.elementType;
	if (!pTS->IsPter(elementType)) {
		if (pTS->TypeIsObject(elementType)) {
			this->valuePter = element.valuePter;
			RefCountedObject* pObj = reinterpret_cast<RefCountedObject*>(this->valuePter);
			pObj->IncReference();
		}
		else {
			switch (elementType) {
			case StackElement_Bool: this->valueBool = element.valueBool; break;
			case StackElement_Char: this->valueChar = element.valueChar; break;
			case StackElement_Int: this->valueInt64 = element.valueInt64; break;
			case StackElement_Float: this->valueDouble = element.valueDouble; break;
			case StackElement_Type: this->valueType = element.valueType; break;
			case StackElement_PterToCFA: this->valueWordBodyPter = element.valueWordBodyPter; break;
			}
		}
	}
	else {
		this->valuePter = element.valuePter; 
		if (!pTS->IsValueOrValuePter(elementType)) {
			pTS->IncReferenceForPter(elementType, this->valuePter);
		}
	}
}

StackElement::~StackElement() {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!pTS->IsValueOrValuePter(elementType)) {
		// Is object, or object pter
		pTS->DecReferenceForPter(elementType, this->valuePter);
	}
}

StackElement::StackElement(char c) {
	elementType = StackElement_Char;
	valueChar = c;
}

StackElement::StackElement(int64_t n) {
	elementType = StackElement_Int;
	valueInt64 = n;
}

StackElement::StackElement(double d) {
	elementType = StackElement_Float;
	valueDouble = d;
}

StackElement::StackElement(bool b) {
	elementType = StackElement_Bool;
	valueBool = b;
}

StackElement::StackElement(ForthType v) {
	elementType = StackElement_Type;
	valueType = v;
}

StackElement::StackElement(WordBodyElement** ppWbe) {
	elementType = StackElement_PterToCFA;
	valueWordBodyPter = ppWbe;
}

//StackElement::StackElement(WordBodyElement*** pppWbe) {
//	elementType = StackElement_PterToPterToCFA;
//	valueWordBodyPterPter = pppWbe;
//}

StackElement::StackElement(XT* pXt) {
	elementType = StackElement_XT;
	valueXTPter = pXt;
}

StackElement::StackElement(BinaryOperationType opsType) {
	elementType = StackElement_BinaryOpsType;
	valueBinaryOpsType = opsType;
}

StackElement::StackElement(RefCountedObject* pObject) {
	elementType = pObject->GetObjectTypeId();
	valuePter = reinterpret_cast<void*>(pObject);
	pObject->IncReference();
}

StackElement::StackElement(ForthType forthType, void* pter) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	elementType = forthType;
	valuePter = pter;
	if (!pTS->IsValueOrValuePter(elementType)) {
		pTS->IncReferenceForPter(elementType, this->valuePter);
	}
}

StackElement::StackElement(ForthType forthType, WordBodyElement* pLiteral) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	elementType = forthType;
	if (!pTS->IsPter(forthType)) {
		if (pTS->TypeIsObject(elementType)) {
			this->valuePter = pLiteral->pter;
			pTS->IncReferenceForPter(elementType, this->valuePter);
		}
		else {
			switch (forthType) {
			case StackElement_Bool: this->valueBool = pLiteral->wordElement_bool; break;
			case StackElement_Char: this->valueChar = pLiteral->wordElement_char; break;
			case StackElement_Int: this->valueInt64 = pLiteral->wordElement_int; break;
			case StackElement_Float: this->valueDouble = pLiteral->wordElement_float; break;
			case StackElement_Type: this->valueType = pLiteral->forthType; break;
			case StackElement_PterToCFA: this->valueWordBodyPter = pLiteral->wordElement_BodyPter; break;
			}
		}
	}
	else {
		this->valuePter = pLiteral->pter;
		// If pointer to a ref-counted object, increment the object (follow the dereference chain)
		if (!pTS->IsValueOrValuePter(forthType)) {
			pTS->IncReferenceForPter(forthType, this->valuePter);
		}
	}
}

char StackElement::GetChar() const 
{
	switch (elementType) {
	case StackElement_Char: return valueChar;
	case StackElement_Int: return (char)(valueInt64 && 0xff);
	}
	return '\0';
}


double StackElement::GetFloat() const {
	switch (elementType) {
	case StackElement_Float: return valueDouble;
	case StackElement_Int: return (double)valueInt64;
	case StackElement_Bool: return valueBool ? -1 : 0;
	}
	return 0.0;
}

int64_t StackElement::GetInt() const {
	switch (elementType) {
	case StackElement_Float: return (int64_t)valueDouble;
	case StackElement_Int: return valueInt64;
	case StackElement_Bool: return valueBool ? -1 : 0;
	}
	return 0;
}

bool StackElement::GetBool() const {
	if (elementType == StackElement_Bool) {
		return valueBool;
	}
	return false;
}

BinaryOperationType StackElement::GetBinaryOpsType() const {
	if (elementType == StackElement_BinaryOpsType) {
		return valueBinaryOpsType;
	}
	return BinaryOp_Undefined;
}

ForthType StackElement::GetValueType() const {
	if (elementType == StackElement_Type) {
		return valueType;
	}
	return ValueType_Undefined;
}

RefCountedObject* StackElement::GetObject() const {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (pTS->TypeIsObject(elementType))
	{
		return reinterpret_cast<RefCountedObject*>(this->valuePter);
	}
	return nullptr;
}

tuple<bool, ForthType, void*> StackElement::GetObjectOrObjectPter() const {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (pTS->TypeIsObjectOrObjectPter(this->elementType))
	{
		return { true, this->elementType, this->valuePter };
	}
	return {false, 0, nullptr };
}

WordBodyElement** StackElement::GetWordBodyElement() const {
	if (elementType != StackElement_PterToCFA) {
		return nullptr;
	}
	return valueWordBodyPter;
}

//WordBodyElement*** StackElement::GetWordBodyElementPter() const {
//	if (elementType != StackElement_PterToPterToCFA) {
//		return nullptr;
//	}
//	return valueWordBodyPterPter;
//}

bool StackElement::IsPter() const {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	return pTS->IsPter(this->elementType);
}

bool StackElement::IsLiteral() const {
	switch (elementType) {
	case StackElement_Int:
	case StackElement_Float:
	case StackElement_Char:
	case StackElement_Bool:
	case StackElement_Type:
		return true;
	default:
		return false;
	}
}

bool StackElement::ContainsNumericType() const {
	return elementType == StackElement_Int || elementType == StackElement_Float;
}

StackElement* StackElement::GetDerefedPterValueAsStackElement() const {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	void* pter;
	ForthType newType;

	std::tie(newType, pter) = pTS->DeferencePointer(this->elementType, this->valuePter);

	if (pTS->IsPter(newType)) {
		return new StackElement(newType, pter);
	}
	else {
		if (pTS->TypeIsObject(newType)) {
			return new StackElement(newType, pter);
		}
		else {
			// After the call to deference, the void* pter it has returned hasnt't been dereferenced, but returned as is.  This is only 
			//  true for value types, not object types.
			// Up to the caller to deference with the correct type
			switch (pTS->GetValueType(newType)) {
			case StackElement_Int:
				return new StackElement(*(int64_t*)pter);
			case StackElement_Float:
				return new StackElement(*(double*)pter);
			case StackElement_Char:
				return new StackElement(*(char*)pter);
			case StackElement_Bool:
				return new StackElement(*(bool*)pter);
			case StackElement_Type:
				return new StackElement(*(ForthType*)pter);
			}
		}
	}
	return nullptr;
}

bool StackElement::PokeIntoContainedPter(ExecState* pExecState, StackElement* pValueElement) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!pTS->ValueCompatibleWithAddress(this->elementType, pValueElement->elementType, true)) {
		return pExecState->CreateException("Pointer type is incompatible with address type");
	}
	if (pTS->TypeIsObjectOrObjectPter(this->elementType)) {
		return PokeObjectIntoContainedPter(pExecState, pValueElement);
	}
	else {
		return PokeValueIntoContainedPter(pExecState, pValueElement);
	}
}

bool StackElement::PokeValueIntoContainedPter(ExecState* pExecState, StackElement* pValueElement) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	switch (pTS->GetValueType(pValueElement->elementType)) {
	case StackElement_Int:
		*reinterpret_cast<int64_t*>(valuePter) = pValueElement->GetInt();
		break;
	case StackElement_Float:
		*reinterpret_cast<double*>(valuePter) = pValueElement->GetFloat();
		break;
	case StackElement_Char:
		*reinterpret_cast<char*>(valuePter) = pValueElement->GetChar();
		break;
	case StackElement_Bool:
		*reinterpret_cast<bool*>(valuePter) = pValueElement->GetBool();
		break;
	case StackElement_Type:
		*reinterpret_cast<ForthType*>(valuePter) = pValueElement->GetValueType();
		break;
	default:
		return pExecState->CreateException("Undefined type when setting a value into a pointer");
	}
	return true;
}

bool StackElement::PokeObjectIntoContainedPter(ExecState* pExecState, StackElement* pObjectElement) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	bool success;
	void* objectPter;
	ForthType objectType;
	void* addressPter;
	ForthType addressType;

	tie(success, objectType, objectPter) = pObjectElement->GetObjectOrObjectPter();
	addressType = this->elementType;
	addressPter = this->valuePter;
	RefCountedObject* pObject = reinterpret_cast<RefCountedObject*>(objectPter);
	RefCountedObject** pAddress = reinterpret_cast<RefCountedObject**>(valuePter);

	// Not calling dec/inc refernce directly only the objects, but using the type system to do so
	// This is because we may not actually be working with pointers to objects directly, but levels of redirection and
	//  the direction dec/inc only work on pointers to the objects, not pointers to pointers to pointers to objects.

	if (*pAddress != pObject && *pAddress != nullptr) {
		// Removing reference to whatever pAddress  currently points at - decrease reference
		pTS->DecReferenceForPter(addressType, addressPter);

		// *** Comment A
		// The reference count belongs to the object, not the object pointer.
		// After exiting this method, the pAddress Element stack element will be deleted, which should call the DecReference on this object being 'null'-ed here.
		// However, by the time that it gets to delete the pAddressElement, the pointer it contains will point at a different object (the value object being set
		//  here).  To prevent the value object having its reference decremented incorrectly (and possibly deleting it), after setting the object pter's to point
		//  at the value object, we increment it's reference (see below, *** Comment B), and the delete then decrements the reference.
		// 
		// However, there is still a missing reference decrement on the current whatever the object pter currently points at.  This should happen in the
		//  delete pAddressElement after this switch statement, however, that pAddressElement will contain a pter to a pter that is the value object, not, the old object
		// So as it won't be dec-ed there, we do it here.
		pTS->DecReferenceForPter(addressType, addressPter);

		*pAddress = nullptr;

		*pAddress = pObject;
		pTS->IncReferenceForPter(objectType, objectPter);


		// *** Comment B
		// Stop deleting pAddressElement (after exiting) down from deleting the object
		// See above comment marked *** Comment A
		pTS->IncReferenceForPter(objectType, objectPter);
	}
	return true;
}

WordBodyElement* StackElement::GetValueAsWordBodyElement() const {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	WordBodyElement* pWBE = new WordBodyElement();
	if (!pTS->IsPter(elementType)) {
		if (pTS->TypeIsObject(elementType)) {
			pWBE->pter = this->valuePter;
			pTS->IncReferenceForPter(elementType, this->valuePter);
		}
		else {
			switch (pTS->GetValueType(elementType)) {
			case StackElement_Char: pWBE->wordElement_char = this->valueChar; break;
			case StackElement_Int: pWBE->wordElement_int = this->valueInt64; break;
			case StackElement_Float: pWBE->wordElement_float = this->valueDouble; break;
			case StackElement_Bool: pWBE->wordElement_bool = this->valueBool; break;
			case StackElement_Type: pWBE->forthType = this->valueType; break;
			}
		}
	}
	else {
		pWBE->pter = this->valuePter;
		if (!pTS->IsValueOrValuePter(elementType)) {
			pTS->IncReferenceForPter(elementType, this->valuePter);
		}
	}
	return pWBE;
}
// TODO Remove this code
//bool StackElement::ValueTypeToString(ExecState* pExecState, ForthType forthType, const void* pter) const {
//	TypeSystem* pTS = TypeSystem::GetTypeSystem();
//	ostringstream out;
//
//	if (pTS->TypeIsObject(forthType)) {
//		const RefCountedObject* pObj = reinterpret_cast<const RefCountedObject* >(pter);
//		if (!pObj->ToString(pExecState)) {
//			return false;
//		}
//		else {
//			string str;
//			bool success;
//			tie(success, str) = pExecState->pStack->PullAsString();
//			if (!success) {
//				return pExecState->CreateException(str.c_str());
//			}
//			out << str;
//		}
//	}
//	else {
//		switch (pTS->GetValueType(forthType)) {
//		case StackElement_PterToCFA:
//			out << "Word body";
//			break;
//		case StackElement_XT:
//			out << "XT";
//			break;
//		case StackElement_Char:
//			out << *((char*)pter);
//			break;
//		case StackElement_Int:
//			out << *((int64_t*)pter);
//			break;
//		case StackElement_Float:
//			out << *((double*)pter);
//			break;
//		case StackElement_Bool:
//			if (*((double*)pter)) {
//				out << "true";
//			}
//			else {
//				out << "false";
//			}
//			break;
//		case StackElement_Type:
//			out << pTS->TypeToString(forthType);
//			out << " " << pTS->TypeToString(*((ForthType*)pter));
//			break;
//		}
//	}
//	if (!pExecState->pStack->Push(out.str())) {
//		return pExecState->CreateStackOverflowException();
//	}
//
//	return true;
//}

bool StackElement::ToString(ExecState* pExecState ) const {
	// TODO Some of this functionality has migrated to the typesystem.  Call that instead.
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	return pTS->VariableToString(pExecState, elementType, valuePter);

// TODO Remove this code	
	if (pTS->TypeIsObjectOrObjectPter(elementType)) {
		return pTS->VariableToString(pExecState, elementType, valuePter);
	}
	else {
		return pTS->VariableToString(pExecState, elementType, reinterpret_cast<const void*>(&valueInt64));
	}
	return false;
}