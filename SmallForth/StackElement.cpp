#include "ForthDefs.h"
#include "StackElement.h"
#include "TypeSystem.h"
#include "ExecState.h"
#include "DataStack.h"
#include "ForthString.h"
#include <sstream>

#include "enumPrinters.h"

StackElement::StackElement() {
	elementType = StackElement_Undefined;
}

StackElement::StackElement(const StackElement& element) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	elementType = element.elementType;
	if (!pTS->IsPter(elementType)) {
		if (pTS->TypeIsObject(elementType)) {
			// If the stackelement contains an object it is not contained within a WordBodyElement->pter, but the C++ RefCountedObject* is cast to a void*.  This
			//  is because RefCountedObjects can exist just on the stack, without being part of a word (and therefore in a WordBodyElement).
			// As soon as we take a pointer to a RefCountedObject (by running: variable <variable name>) the reference to the RefCountedObject* becomes encapsulated
			//  inside a WordBodyElement, and this if-clause ( if (!pTS->IsPter() ) does not get executed, but the next one (past the value type assignments).
			this->valuePter = element.valuePter;
			RefCountedObject* pObj = static_cast<RefCountedObject*>(this->valuePter);
			if (pObj != nullptr) {
				pObj->IncReference();
			}
		}
		else {
			switch (elementType) {
			case StackElement_Bool: this->valueBool = element.valueBool; break;
			case StackElement_Char: this->valueChar = element.valueChar; break;
			case StackElement_Int: this->valueInt64 = element.valueInt64; break;
			case StackElement_Float: this->valueDouble = element.valueDouble; break;
			case StackElement_Type: this->valueType = element.valueType; break;
			case StackElement_PterToCFA: this->valueWordBodyPter = element.valueWordBodyPter; break;
			case StackElement_BinaryOpsType: this->valueBinaryOpsType = element.valueBinaryOpsType; break;
			}
		}
	}
	else {
		// It's a pointer to something, just copy the pointer.
		this->valuePter = element.valuePter; 
		if (!pTS->IsValueOrValuePter(elementType)) {
			// Its a pointer to a ref-counted object - increase reference count.  Note, the type system will take care of
			//  getting the reference out of the WordbodyElement** that the this->valuePter points at
			if (this->valuePter != nullptr) {
				pTS->IncReferenceForPter(elementType, this->valuePter);
			}
		}
	}
}

StackElement::StackElement(StackElement&& element) {
	// No need to inc references - taking over the element's data including any reference counting
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	elementType = element.elementType;
	element.elementType = StackElement_Undefined;
	if (!pTS->IsPter(elementType)) {
		if (pTS->TypeIsObject(elementType)) {
			// See copy constructor for details why the object is copied like this, and not as part of the IsPter()=true clause
			this->valuePter = element.valuePter;
			element.valuePter = nullptr;
		}
		else {
			switch (elementType) {
			case StackElement_Bool: this->valueBool = element.valueBool; element.valueBool = false;  break;
			case StackElement_Char: this->valueChar = element.valueChar; element.valueChar = '\0'; break;
			case StackElement_Int: this->valueInt64 = element.valueInt64; element.valueInt64 = 0; break;
			case StackElement_Float: this->valueDouble = element.valueDouble; element.valueDouble = 0.0; break;
			case StackElement_Type: this->valueType = element.valueType; element.valueType = StackElement_Undefined; break;
			case StackElement_PterToCFA: this->valueWordBodyPter = element.valueWordBodyPter; element.valueWordBodyPter = nullptr; break;
			case StackElement_BinaryOpsType: this->valueBinaryOpsType = element.valueBinaryOpsType; element.valueBinaryOpsType = BinaryOp_Undefined; break;
			}
		}
	}
	else {
		// It's a pointer to something, just copy the pointer.
		this->valuePter = element.valuePter;
		element.valuePter = nullptr;
	}
}

StackElement& StackElement::operator=(const StackElement& element) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (this != &element) {
		// Dec reference if currently a ref counted object (or pter to) at the end of this method
		bool decObject = false;
		bool decObjectPter = false;
		void* objectToDec = nullptr;
		ForthType typeToDec = StackElement_Undefined;
		if (pTS->TypeIsObject(elementType)) {
			decObject = true;
			objectToDec = this->valuePter;
			typeToDec = this->elementType;
		}
		else if (pTS->IsPter(elementType) && !pTS->IsValueOrValuePter(elementType)) {
			decObjectPter = true;
			objectToDec = this->valuePter;
			typeToDec = this->elementType;
		}

		elementType = element.elementType;
		if (!pTS->IsPter(elementType)) {
			if (pTS->TypeIsObject(elementType)) {
				// See copy constructor for details why the object is copied like this, and not as part of the IsPter()=true clause
				this->valuePter = element.valuePter;
				RefCountedObject* pObj = static_cast<RefCountedObject*>(this->valuePter);
				if (pObj != nullptr) {
					pObj->IncReference();
				}
			}
			else {
				switch (elementType) {
				case StackElement_Bool: this->valueBool = element.valueBool; break;
				case StackElement_Char: this->valueChar = element.valueChar; break;
				case StackElement_Int: this->valueInt64 = element.valueInt64; break;
				case StackElement_Float: this->valueDouble = element.valueDouble; break;
				case StackElement_Type: this->valueType = element.valueType; break;
				case StackElement_PterToCFA: this->valueWordBodyPter = element.valueWordBodyPter; break;
				case StackElement_BinaryOpsType: this->valueBinaryOpsType = element.valueBinaryOpsType; break;
				}
			}
		}
		else {
			// It's a pointer to something, just copy the pointer.
			this->valuePter = element.valuePter;
			if (!pTS->IsValueOrValuePter(elementType)) {
				// Its a pointer to a ref-counted object - increase reference count.  Note, the type system will take care of
				//  getting the reference out of the WordbodyElement** that the this->valuePter points at
				if (this->valuePter != nullptr) {
					pTS->IncReferenceForPter(elementType, this->valuePter);
				}
			}
		}

		if (decObject) {
			RefCountedObject* pObj = static_cast<RefCountedObject*>(objectToDec);
			if (pObj != nullptr) {
				pObj->DecReference();
			}
		}
		else if (decObjectPter) {
			// Previous pointer is to a ref-counted object - decrease reference count.  Note, the type system will take care of
			//  getting the reference out of the WordbodyElement** that the valuePter points at
			if (objectToDec != nullptr) {
				pTS->DecReferenceForPter(typeToDec, objectToDec);
			}
		}
	}
	return *this;
}

StackElement& StackElement::operator=(StackElement&& element) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (this != &element) {
		// Dec reference if currently a ref counted object (or pter to) at the end of this method
		bool decObject = false;
		bool decObjectPter = false;
		void* objectToDec = nullptr;
		ForthType typeToDec = StackElement_Undefined;
		if (pTS->TypeIsObject(elementType)) {
			decObject = true;
			objectToDec = this->valuePter;
			typeToDec = this->elementType;
		}
		else if (pTS->IsPter(elementType) && !pTS->IsValueOrValuePter(elementType)) {
			decObjectPter = true;
			objectToDec = this->valuePter;
			typeToDec = this->elementType;
		}


		elementType = element.elementType;
		element.elementType = StackElement_Undefined;
		if (!pTS->IsPter(elementType)) {
			if (pTS->TypeIsObject(elementType)) {
				// See copy constructor for details why the object is copied like this, and not as part of the IsPter()=true clause
				this->valuePter = element.valuePter;
				element.valuePter = nullptr;
			}
			else {
				switch (elementType) {
				case StackElement_Bool: this->valueBool = element.valueBool; element.valueBool = false;  break;
				case StackElement_Char: this->valueChar = element.valueChar; element.valueChar = '\0'; break;
				case StackElement_Int: this->valueInt64 = element.valueInt64; element.valueInt64 = 0; break;
				case StackElement_Float: this->valueDouble = element.valueDouble; element.valueDouble = 0.0; break;
				case StackElement_Type: this->valueType = element.valueType; element.valueType = StackElement_Undefined; break;
				case StackElement_PterToCFA: this->valueWordBodyPter = element.valueWordBodyPter; element.valueWordBodyPter = nullptr; break;
				case StackElement_BinaryOpsType: this->valueBinaryOpsType = element.valueBinaryOpsType; element.valueBinaryOpsType = BinaryOp_Undefined; break;
				}
			}
		}
		else {
			// It's a pointer to something, just copy the pointer.
			this->valuePter = element.valuePter;
			element.valuePter = nullptr;
		}

		if (decObject) {
			RefCountedObject* pObj = static_cast<RefCountedObject*>(objectToDec);
			if (pObj != nullptr) {
				pObj->DecReference();
			}
		}
		else if (decObjectPter) {
			// Previous pointer is to a ref-counted object - decrease reference count.  Note, the type system will take care of
			//  getting the reference out of the WordbodyElement** that the valuePter points at
			if (objectToDec != nullptr) {
				pTS->DecReferenceForPter(typeToDec, objectToDec);
			}
		}
	}
	return *this;
}

StackElement::~StackElement() {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!pTS->IsValueOrValuePter(elementType)) {
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
	valuePter = static_cast<void*>(pObject);
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

StackElement::StackElement(ForthType forthType, WordBodyElement** ppLiteral) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	elementType = forthType;
	if (!pTS->IsPter(forthType)) {
		WordBodyElement* pLiteral = *ppLiteral;
		if (pTS->TypeIsObject(elementType)) {
			this->valuePter = pLiteral->refCountedPter.pter;
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
		this->valuePter = (void*)ppLiteral;
		// If pointer to a ref-counted object, increment the object (follow the dereference chain)
		if (!pTS->IsValueOrValuePter(forthType)) {
			pTS->IncReferenceForPter(forthType, (static_cast<WordBodyElement**>(this->valuePter)));
		}
	}
}

void StackElement::SetTo(char value) {
	elementType = StackElement_Char;
	valueChar = value;
}

void StackElement::SetTo(int64_t value) {
	elementType = StackElement_Int;
	valueInt64 = value;
}

void StackElement::SetTo(double value) {
	elementType = StackElement_Float;
	valueDouble = value;
}

void StackElement::SetTo(bool value) {
	elementType = StackElement_Bool;
	valueBool = value;
}

void StackElement::SetTo(WordBodyElement** value) {
	elementType = StackElement_PterToCFA;
	valueWordBodyPter = value;
}

void StackElement::SetTo(XT* value) {
	elementType = StackElement_XT;
	valueXTPter = value;
}

void StackElement::SetTo(BinaryOperationType value) {
	elementType = StackElement_BinaryOpsType;
	valueBinaryOpsType = value;
}

void StackElement::SetTo(ForthType value) {
	elementType = StackElement_Type;
	valueType = value;

}

void StackElement::SetTo(RefCountedObject* value) {
	elementType = value ->GetObjectTypeId();
	valuePter = static_cast<void*>(value);
	value->IncReference();

}

void StackElement::SetTo(ForthType forthType, WordBodyElement** ppLiteral) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	elementType = forthType;
	if (!pTS->IsPter(forthType)) {
		WordBodyElement* pLiteral = *ppLiteral;
		if (pTS->TypeIsObject(elementType)) {
			this->valuePter = pLiteral->refCountedPter.pter;
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
		this->valuePter = (void*)ppLiteral;
		// If pointer to a ref-counted object, increment the object (follow the dereference chain)
		if (!pTS->IsValueOrValuePter(forthType)) {
			pTS->IncReferenceForPter(forthType, (static_cast<WordBodyElement**>(this->valuePter)));
		}
	}
}

void StackElement::SetTo(ForthType forthType, void* value) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	elementType = forthType;
	valuePter = value;
	if (!pTS->IsValueOrValuePter(elementType)) {
		pTS->IncReferenceForPter(elementType, this->valuePter);
	}
}

void StackElement::RelinquishValue() {
	if (elementType == StackElement_Undefined) {
		return;
	}
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!pTS->IsValueOrValuePter(elementType)) {
		pTS->DecReferenceForPter(elementType, this->valuePter);
		this->valuePter = nullptr;
	}
	else {
		switch(elementType) {
		case StackElement_Char: valueChar = '\0'; break;
		case StackElement_Int: valueInt64 = 0; break;
		case StackElement_Float: valueDouble = 0; break;
		case StackElement_Bool: valueBool = false; break;
		case StackElement_Type: valueType = ValueType_Undefined; break;
		case StackElement_XT: valueXTPter = nullptr; break;
		case StackElement_BinaryOpsType: valueBinaryOpsType = BinaryOp_Undefined; break;
		case StackElement_PterToCFA: valueWordBodyPter = nullptr; break;
		default: valuePter = nullptr; break;
		}
	}

	this->elementType = StackElement_Undefined;

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
		return static_cast<RefCountedObject*>(this->valuePter);
	}
	return nullptr;
}

std::tuple<bool, ForthType, void*> StackElement::GetObjectOrObjectPter() const {
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
		WordBodyElement** ppWBE = (WordBodyElement**)pter;
		return new StackElement(newType, ppWBE);
	}
	else {
		if (pTS->TypeIsObject(newType)) {
			return new StackElement((RefCountedObject*)pter);
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

	WordBodyElement** ppWBE = static_cast<WordBodyElement**>(valuePter);
	WordBodyElement* pWBE = *ppWBE;
	if (pTS->IsPter(pValueElement->elementType)) {
		// TODO Dec reference in current pter
		pWBE->refCountedPter.pter = pValueElement->GetContainedPter();
		// TODO Inc reference in this pter
	}
	else {
		switch (pTS->GetValueType(pValueElement->elementType)) {
		case StackElement_Int:
			pWBE->wordElement_int = pValueElement->GetInt();
			break;
		case StackElement_Float:
			pWBE->wordElement_float = pValueElement->GetFloat();
			break;
		case StackElement_Char:
			pWBE->wordElement_char = pValueElement->GetChar();
			break;
		case StackElement_Bool:
			pWBE->wordElement_bool = pValueElement->GetBool();
			break;
		case StackElement_Type:
			pWBE->forthType = pValueElement->GetValueType();
			break;
		default:
			return pExecState->CreateException("Undefined type when setting a value into a pointer");
		}
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

	std::tie(success, objectType, objectPter) = pObjectElement->GetObjectOrObjectPter();
	addressType = this->elementType;
	addressPter = this->valuePter;

	int indirectionCount = pTS->GetIndirectionLevel(addressType);
	if (indirectionCount > 1) {
		// Read comments on if clause for indirectionCount = 0.  This clause is similar, but
		//  as indirection levels >1 go WordBodyElement** -> WordBodyElement* -> pter 
		//			(this is for indirection level 2					      (contains)
		//           for address element)                                        WordBodyElement** -> WordBodyElement* -> pter
		//                                                                                                             (contains)
		//																											      RefCountedObject*
		WordBodyElement** ppWBEAddress = static_cast<WordBodyElement**>(valuePter);
		WordBodyElement** ppWBEObject = static_cast<WordBodyElement**>(objectPter);
		// TODO Test this change
		WordBodyElement** ppWBEContainedInAddress = static_cast<WordBodyElement**>((*ppWBEAddress)->refCountedPter.pter);
		// Currently, the contained pointer has this many references held on it.
		int currentValuePterCount = (*ppWBEAddress)->refCountedPter.refCount;

		if (ppWBEContainedInAddress != ppWBEObject && ppWBEContainedInAddress != nullptr) {
			pTS->DecReferenceForPterBy(addressType, valuePter, currentValuePterCount-1);
			pTS->DecReferenceForPter(addressType, valuePter);
			(*ppWBEAddress)->refCountedPter.pter = nullptr;
			(*ppWBEAddress)->refCountedPter.pter = ppWBEObject;
			pTS->IncReferenceForPterBy(addressType, valuePter, currentValuePterCount - 1);
			pTS->IncReferenceForPter(addressType, valuePter);
		}
	}
	else {
		RefCountedObject* pObject = static_cast<RefCountedObject*>(objectPter);
		WordBodyElement** ppWBEAddress = static_cast<WordBodyElement**>(valuePter);
		//RefCountedObject* pContainedInAddress = static_cast<RefCountedObject*>((*ppWBEAddress)->pter);
		// TODO Test this change
		RefCountedObject* pContainedInAddress = static_cast<RefCountedObject*>((*ppWBEAddress)->refCountedPter.pter);

		// Currently, the contained pointer has this many references held on it.
		int currentValuePterCount = (*ppWBEAddress)->refCountedPter.refCount;
		// Not calling dec/inc reference directly only the objects, but using the type system to do so
		// This is because we may not actually be working with pointers to objects directly, but levels of redirection and
		//  the direction dec/inc only work on pointers to the objects, not pointers to pointers to pointers to objects.
		if (pContainedInAddress != pObject && pContainedInAddress !=nullptr)  {
			// Removing reference to whatever pAddress currently points at - decrease reference
			pTS->DecReferenceForPterBy(addressType, valuePter, currentValuePterCount-1);

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
			pTS->DecReferenceForPter(addressType, valuePter);

			(*ppWBEAddress)->refCountedPter.pter = nullptr;
			(*ppWBEAddress)->refCountedPter.pter = pObject;

			pTS->IncReferenceForPterBy(addressType, valuePter, currentValuePterCount-1);

			// *** Comment B
			// Stop deleting pAddressElement (after exiting) down from deleting the object
			// See above comment marked *** Comment A
			pTS->IncReferenceForPter(addressType, valuePter);
		}
	}
	return true;
}

WordBodyElement* StackElement::GetValueAsWordBodyElement() const {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	WordBodyElement* pWBE = new WordBodyElement();
	if (!pTS->IsPter(elementType)) {
		if (pTS->TypeIsObject(elementType)) {
			pWBE->refCountedPter.pter = this->valuePter;
			pWBE->refCountedPter.refCount = 1;
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
		pWBE->refCountedPter.pter = this->valuePter;
		pWBE->refCountedPter.refCount = 1;
		if (!pTS->IsValueOrValuePter(elementType)) {
			pTS->IncReferenceForPter(elementType, this->valuePter);
		}
	}
	return pWBE;
}

bool StackElement::ToString(ExecState* pExecState ) const {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	return pTS->VariableToString(pExecState, elementType, valuePter);
}