#include "ForthDefs.h"
#include "TypeSystem.h"
#include "RefCountedObject.h"
#include "UserDefinedObject.h"
#include "ExecState.h"
#include "DataStack.h"
#include <algorithm>
#include <sstream>

TypeSystem* TypeSystem::s_pTypeSystem;

TypeSystem* TypeSystem::GetTypeSystem() {
	if (s_pTypeSystem == nullptr) {
		s_pTypeSystem = new TypeSystem();
	}
	return s_pTypeSystem;
}

TypeSystem::TypeSystem() {
	nextValueTypeId = 0;
	maxValueTypeId = 1022;
	firstObjectTypeId = nextObjectTypeId = 1024;
	firstUserObjectTypeId =  nextUserObjectTypeId = 32768;
	maxObjectTypeId = 32767;
	maxUserObjectTypeId = 65535;
}

bool TypeSystem::RegisterValueType(ExecState* pExecState, std::string typeName) {
	// TODO Interlocked increment of type id
	int idToUse = nextValueTypeId++;
	if (!RegisterType(pExecState, typeName, idToUse, nullptr, nullptr, nullptr)) {
		return false;
	}
	return true;
}

ForthType TypeSystem::RegisterObjectType(ExecState* pExecState, std::string typeName, XT constructXT, XT binaryOpsXT) {
	// TODO Interlocked increment of type id
	int idToUse = nextObjectTypeId++;
	if (!RegisterType(pExecState, typeName, idToUse, constructXT, binaryOpsXT, nullptr)) {
		return TypeSystem::typeIdInvalid;
	}
	return idToUse;
}

ForthType TypeSystem::RegisterUserObjectType(ExecState* pExecState, std::string typeName, UserDefinedObject* pNewObjectDefinition) {
	// TODO Interlocked increment of type id
	ForthType idToUse = nextUserObjectTypeId++;
	if (!RegisterType(pExecState, typeName, idToUse, nullptr, nullptr, pNewObjectDefinition)) {
		return TypeSystem::typeIdInvalid;
	}
	pNewObjectDefinition->SetType(idToUse);
	return idToUse;
}

bool TypeSystem::RegisterType(ExecState* pExecState, std::string typeName, int typeId, XT constructXT, XT binaryOpsXT, UserDefinedObject* pDefiningType) {
	std::transform(typeName.begin(), typeName.end(), typeName.begin(),
		[](unsigned char c) { return std::tolower(c); });

	unsigned int existingTypeId = GetBaseTypeIdForName(typeName);
	if (existingTypeId != TypeSystem::typeIdInvalid) {
		return pExecState->CreateException("Type already exists");
	}
	typeNameToId[typeName] = typeId;
	typeIdToName[typeId] = typeName;
	RegisteredType* pRT = new RegisteredType(typeName, typeId, constructXT, binaryOpsXT, pDefiningType);
	typeIdToRegisteredType[typeId] = pRT;
	return true;
}

unsigned int TypeSystem::GetBaseTypeIdForName(std::string typeName) const {
	std::transform(typeName.begin(), typeName.end(), typeName.begin(),
		[](unsigned char c) { return std::tolower(c); });

	std::map<std::string, unsigned int>::const_iterator findType = typeNameToId.find(typeName);

	if (findType == typeNameToId.end()) {
		return TypeSystem::typeIdInvalid;
	}
	return findType->second;
}

std::string TypeSystem::GetBaseTypeNameForId(unsigned int typeId) const {
	std::map<unsigned int, std::string >::const_iterator findType = typeIdToName.find(typeId);

	if (findType == typeIdToName.end()) {
		return "unknown";
	}
	return findType->second;
}

bool TypeSystem::Construct(ExecState* pExecState, ForthType type) {
	if (TypeIsUserObject(type)) {
		return ConstructUserType(pExecState, type);
	}
	else {
		return ConstructSystemType(pExecState, type);
	}
}

bool TypeSystem::ConstructUserType(ExecState* pExecState, ForthType type) {
	RegisteredType* pRT = GetRegisteredTypeForTypeId(type);
	UserDefinedObject* pDefiningObject = pRT->definingObject;

	if (pDefiningObject==nullptr) {
		return pExecState->CreateException("Cannot construct object, as it does not have an object definition");
	}
	UserDefinedObject* newObject = pDefiningObject->Construct(pExecState);
	if (newObject == nullptr) {
		return pExecState->CreateException("Could not construct object");
	}
	if (!pExecState->pStack->Push(newObject)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool TypeSystem::ConstructSystemType(ExecState* pExecState, ForthType type) {
	XT constructXT = GetConstructXTForType(type);
	if (constructXT != nullptr) {
		if (!constructXT(pExecState)) {
			if (!pExecState->exceptionThrown) {
				return pExecState->CreateException("Could not construct object");
			}
			else {
				return false;
			}
		}
	}
	else {
		return pExecState->CreateException("No constructor for that type");
	}
	return true;
}

XT TypeSystem::GetConstructXTForType(ForthType type) const {
	RegisteredType* pRT = GetRegisteredTypeForTypeId(type);
	if (pRT == nullptr) {
		return nullptr;
	}
	return pRT->constructorXT;
}

XT TypeSystem::GetBinaryOpsHandlerForType(ForthType type) const {
	RegisteredType* pRT = GetRegisteredTypeForTypeId(type);
	if (pRT == nullptr) {
		return nullptr;
	}
	return pRT->binaryOpsXT;
}

RegisteredType* TypeSystem::GetRegisteredTypeForTypeId(ForthType type) const {
	if (GetIndirectionLevel(type) > 0) {
		return nullptr;
	}
	int typeId = type & 0xffff;
	std::map<unsigned int, RegisteredType*>::const_iterator foundIterator = typeIdToRegisteredType.find(typeId);
	if (foundIterator == typeIdToRegisteredType.end()) {
		return nullptr;
	}
	return foundIterator->second;
}

std::string TypeSystem::TypeToString(ForthType type) const {
	std::string typeName = GetBaseTypeNameForId(type & 0xffff);

	std::stringstream str;

	unsigned int pterCount = type >> 16;
	for (unsigned int n = 0; n < pterCount; ++n) {
		if (n == 0) {
			str << "Pter to ";
		}
		else {
			str << "pter to ";
		}
	}
	str << typeName;

	return str.str();
}

ForthType TypeSystem::CreatePointerTypeTo(ForthType fromType) const {
	uint32_t indirectionLevel = fromType >> 16;
	indirectionLevel++;
	return (indirectionLevel << 16) | (fromType & 0xffff);
}

bool TypeSystem::IsValueOrValuePter(ForthType type) const {
	uint32_t justType = type & 0xffff;
	if (justType <= maxValueTypeId) {
		return true;
	}
	return false;
}

bool TypeSystem::IsValue(ForthType type) const {
	uint32_t justType = type & 0xffff;
	if (justType <= maxValueTypeId) {
		if (GetIndirectionLevel(type) > 0) {
			return false;
		}
		return true;
	}
	return false;
}

bool TypeSystem::TypeIsObject(ForthType type) const {
	return type >= this->firstObjectTypeId && type <= this->maxUserObjectTypeId;
}

bool TypeSystem::TypeIsObjectOrObjectPter(ForthType type) const {
	return (type & 0xffff) >= this->firstObjectTypeId && (type & 0xffff) <= this->maxUserObjectTypeId;
}

bool TypeSystem::TypeIsSystemObject(ForthType type) const {
	return (type & 0xffff) >= this->firstObjectTypeId && (type & 0xffff) <= this->maxObjectTypeId;
}

bool TypeSystem::TypeIsUserObject(ForthType type) const {
	return (type & 0xffff) >= this->firstUserObjectTypeId && (type & 0xffff) <= this->maxUserObjectTypeId;
}

uint32_t TypeSystem::GetIndirectionLevel(ForthType type) const {
	return type >> 16;
}

std::tuple<ForthType, void*> TypeSystem::DeferencePointer(ForthType type, void* pter) {
	int indirectionCount = GetIndirectionLevel(type);
	if (TypeIsObjectOrObjectPter(type)) {
		--indirectionCount;
		WordBodyElement** ppWBE = static_cast<WordBodyElement**>(pter);
		void* pterInWBE = (*ppWBE)->refCountedPter.pter;
		ForthType newForthType = (indirectionCount << 16) | GetValueType(type);
		return { newForthType, pterInWBE };
	}
	else {
		if (indirectionCount > 1) {
			--indirectionCount;

			WordBodyElement** ppWBE = static_cast<WordBodyElement**>(pter);
			void* pterInWBE = (*ppWBE)->refCountedPter.pter;
			ForthType newForthType = (indirectionCount << 16) | GetValueType(type);
			return { newForthType, pterInWBE };
		}
		else {
			--indirectionCount;
			ForthType newForthType = (indirectionCount << 16) | GetValueType(type);
			WordBodyElement** ppWBE = static_cast<WordBodyElement**>(pter);
			pter = &(*ppWBE)->refCountedPter.pter;

			return { newForthType, pter };
		}
	}
}

std::tuple<bool, void*> TypeSystem::DeferencePointerToObjectPter(ForthType type, void* pter) const {
	int indirectionCount = GetIndirectionLevel(type);
	void** ppValue = static_cast<void**>(pter);
	void* pValue = static_cast<RefCountedObject*>(pter);

	for (int n = 0; n < indirectionCount; ++n) {
		if (ppValue != nullptr) {
			WordBodyElement** ppWBE = reinterpret_cast<WordBodyElement**>(ppValue);
			ppValue = static_cast<void**>((*ppWBE)->refCountedPter.pter);
			if (n == indirectionCount - 1) {
				pValue = static_cast<void*>(ppValue);
			}
		}
		else {
			break;
		}
	}
	if (ppValue != nullptr) {
		return { true, pValue };
	}
	else {
		return { false,nullptr };
	}
}

std::tuple<bool, const void*> TypeSystem::DeferencePointerToObjectPter(ForthType type, const void* pter) const {
	int indirectionCount = GetIndirectionLevel(type);
	const void* const* ppValue = static_cast<const void* const*>(pter);
	const void* pValue = static_cast<const RefCountedObject*>(pter);

	for (int n = 0; n < indirectionCount; ++n) {
		if (ppValue != nullptr) {
			const WordBodyElement* const* ppWBE = reinterpret_cast<const WordBodyElement* const*>(ppValue);
			ppValue = static_cast<const void* const*>((*ppWBE)->refCountedPter.pter);
			if (n == indirectionCount - 1) {
				pValue = static_cast<const void*>(ppValue);
			}
		}
		else {
			break;
		}
	}
	if (ppValue != nullptr) {
		return { true, pValue };
	}
	else {
		return { false,nullptr };
	}
}

std::tuple<bool, const void*> TypeSystem::DeferencePointerToValuePter(ForthType type, const void* pter) const {
	int indirectionCount = GetIndirectionLevel(type);
	const void* const* ppValue = static_cast<const void* const*>(pter);
	const void* pValue = static_cast<const RefCountedObject*>(pter);

	const WordBodyElement* const* ppWBE = static_cast<const WordBodyElement* const*>(pter);
	for (int n = 0; n < indirectionCount; ++n) {
		if (ppWBE != nullptr) {
			void* pterInWBE = (*ppWBE)->refCountedPter.pter;
			if (n < indirectionCount - 1) {
				ppWBE = static_cast<const WordBodyElement* const*>(pterInWBE);
			}
			else {
				pValue = static_cast<const void*>(*ppWBE);
			}
		}
	}
	if (pValue != nullptr) {
		return { true, pValue };
	}
	else {
		return { false,nullptr };
	}
}

void TypeSystem::IncReferenceForPter(ForthType type, void* pter) {
	WordBodyElement** ppWBE = static_cast<WordBodyElement**>(pter);
	RefCountedObject* pObject = static_cast<RefCountedObject*>(pter);
	int indirectionCount = GetIndirectionLevel(type);
	for (int n = 0; n < indirectionCount; ++n) {
		if (ppWBE != nullptr) {
			WordBodyElement* pWBE = *ppWBE;
			if (n < indirectionCount - 1) {
				ppWBE = static_cast<WordBodyElement**>(pWBE->refCountedPter.pter);
			}
			else {
				pObject = static_cast<RefCountedObject*>(pWBE->refCountedPter.pter);
			}
		}
	}
	if (pObject != nullptr) {
		pObject->IncReference();
	}
}

void TypeSystem::IncReferenceForPterBy(ForthType type, void* pter, int by) {
	WordBodyElement** ppWBE = static_cast<WordBodyElement**>(pter);
	RefCountedObject* pObject = static_cast<RefCountedObject*>(pter);
	int indirectionCount = GetIndirectionLevel(type);
	for (int n = 0; n < indirectionCount; ++n) {
		if (ppWBE != nullptr) {
			WordBodyElement* pWBE = *ppWBE;
			if (n < indirectionCount - 1) {
				ppWBE = static_cast<WordBodyElement**>(pWBE->refCountedPter.pter);
			}
			else {
				pObject = static_cast<RefCountedObject*>(pWBE->refCountedPter.pter);
			}
		}
	}
	if (pObject != nullptr) {
		pObject->IncReferenceBy(by);
	}
}

void TypeSystem::DecReferenceForPter(ForthType type, void* pter) {
	WordBodyElement** ppWBE = static_cast<WordBodyElement**>(pter);
	RefCountedObject* pObject = static_cast<RefCountedObject*>(pter);
	int indirectionCount = GetIndirectionLevel(type);
	for (int n = 0; n < indirectionCount; ++n) {
		if (ppWBE != nullptr) {
			WordBodyElement* pWBE = *ppWBE;
			if (n < indirectionCount - 1) {
				ppWBE = static_cast<WordBodyElement**>(pWBE->refCountedPter.pter);
			}
			else {
				pObject = static_cast<RefCountedObject*>(pWBE->refCountedPter.pter);

			}
		}
	}
	if (pObject != nullptr) {
		pObject->DecReference();
	}
}

void TypeSystem::DecReferenceForPterBy(ForthType type, void* pter, int by) {
	WordBodyElement** ppWBE = static_cast<WordBodyElement**>(pter);
	RefCountedObject* pObject = static_cast<RefCountedObject*>(pter);
	int indirectionCount = GetIndirectionLevel(type);
	for (int n = 0; n < indirectionCount; ++n) {
		if (ppWBE != nullptr) {
			WordBodyElement* pWBE = *ppWBE;
			if (n < indirectionCount - 1) {
				ppWBE = static_cast<WordBodyElement**>(pWBE->refCountedPter.pter);
			}
			else {
				pObject = static_cast<RefCountedObject*>(pWBE->refCountedPter.pter);
			}
		}
	}
	if (pObject != nullptr) {
		pObject->DecReferenceBy(by);
	}
}

int TypeSystem::GetReferenceCount(ForthType type, void* pter) {
	int indirectionCount = GetIndirectionLevel(type);
	WordBodyElement** ppWBE = static_cast<WordBodyElement**>(pter);
	RefCountedObject* pRefCountedObj = static_cast<RefCountedObject*>(pter);

	for (int n = 0; n < indirectionCount; ++n) {
		void* pterInWBE = (*ppWBE)->refCountedPter.pter;
		if (n < indirectionCount - 1) {
			ppWBE = static_cast<WordBodyElement**>(pterInWBE);
		}
		else {
			pRefCountedObj = static_cast<RefCountedObject*>(pterInWBE);
		}
	}

	if (pRefCountedObj != nullptr) {
		return pRefCountedObj->GetCurrentReferenceCount();
	}
	else {
		return 0;
	}
}

int TypeSystem::GetPterReferenceCount(ForthType type, void* pter) {
	int indirectionCount = GetIndirectionLevel(type);
	WordBodyElement** ppWBE = static_cast<WordBodyElement**>(pter);
	RefCountedObject* pRefCountedObj = static_cast<RefCountedObject*>(pter);
	if (indirectionCount > 0) {
		return (*ppWBE)->refCountedPter.refCount;
	}
	else {
		return 1;
	}
}

bool TypeSystem::ValueCompatibleWithAddress(ForthType addressType, ForthType valueType, bool directAssignment) const {
	uint32_t indirectionLevelAddress = GetIndirectionLevel(addressType);
	uint32_t indirectionLevelValue = GetIndirectionLevel(valueType);
	uint32_t addressValueType = GetValueType(addressType);
	uint32_t valueValueType = GetValueType(addressType);

	if (addressValueType != valueValueType) {
		return false;
	}
	if (directAssignment) {
		return indirectionLevelValue == indirectionLevelAddress - 1;
	}
	return indirectionLevelValue < indirectionLevelAddress;
}

bool TypeSystem::VariableToString(ExecState* pExecState, ForthType type, const void* pter) const {
	std::ostringstream out;

	if (!IsPter(type)) {
		bool success;
		if (TypeIsObject(type)) {
			success = ValueTypeToString(pExecState, type, pter);
		}
		else {
			success = ValueTypeToString(pExecState, type, static_cast<const void*>(&pter));
		}
		if (!success) {
			return false;
		}
		std::string str;
		tie(success, str) = pExecState->pStack->PullAsString();
		if (!success) {
			return pExecState->CreateException(str.c_str());
		}
		out << str;
	}
	else {
		std::string typeAsString = TypeToString(type);

		bool validPter;
		const void* returnedPter;
		if (TypeIsObject(GetValueType(type))) {
			std::tie(validPter, returnedPter) = DeferencePointerToObjectPter(type, pter);
		}
		else {
			std::tie(validPter, returnedPter) = DeferencePointerToValuePter(type, pter);
		}
		out << typeAsString << ": ";
		if (validPter) {
			if (!ValueTypeToString(pExecState, GetValueType(type), returnedPter)) {
				return false;
			}
			else {
				std::string str;
				bool success;
				tie(success, str) = pExecState->pStack->PullAsString();
				if (!success) {
					return pExecState->CreateException(str.c_str());
				}
				out << str;
			}
		}
		else {
			out << " = null";
		}
	}
	std::string s = out.str();
	if (!pExecState->pStack->Push(s)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool TypeSystem::ValueTypeToString(ExecState* pExecState, ForthType forthType, const void* pter) const {
	std::ostringstream out;

	if (TypeIsObject(forthType)) {
		const RefCountedObject* pObj = static_cast<const RefCountedObject*>(pter);
		if (!pObj->ToString(pExecState)) {
			return false;
		}
		else {
			std::string str;
			bool success;
			tie(success, str) = pExecState->pStack->PullAsString();
			if (!success) {
				return pExecState->CreateException(str.c_str());
			}
			out << str;
		}
	}
	else {
		switch (GetValueType(forthType)) {
		case StackElement_PterToCFA:
			out << "Word body";
			break;
		case StackElement_XT:
			out << "XT";
			break;
		case StackElement_Char:
			out << *((char*)pter);
			break;
		case StackElement_Int:
			out << *((int64_t*)pter);
			break;
		case StackElement_Float:
			out << *((double*)pter);
			break;
		case StackElement_Bool:
			if (*((double*)pter)) {
				out << "true";
			}
			else {
				out << "false";
			}
			break;
		case StackElement_Type:
			out << TypeToString(forthType);
			out << " " << TypeToString(*((ForthType*)pter));
			break;
		}
	}
	if (!pExecState->pStack->Push(out.str())) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool TypeSystem::TypeExists(std::string typeName) const {
	unsigned int typeId = GetBaseTypeIdForName(typeName);
	return typeId != TypeSystem::typeIdInvalid;
}

bool TypeSystem::AddWordToObject(ExecState* pExecState, ForthType type, ForthWord* pWord) {
	RegisteredType* pType = GetRegisteredTypeForTypeId(type);

	if (pType == nullptr) {
		return pExecState->CreateException("Object is not a registered type - it does not have registered type information with the type system");
	}
	if (pType->definingObject != nullptr) {
		pType->definingObject->AddWord(pWord);
	}
	else {
		return pExecState->CreateException("Object is registered but does not have a defining object");
	}

	return true;
}

ForthWord* TypeSystem::FindWordWithName(ForthType type, const std::string& wordName) {
	RegisteredType* pType = GetRegisteredTypeForTypeId(type);

	if (pType == nullptr || pType->definingObject==nullptr) {
		return nullptr;
	}
	return pType->definingObject->GetWordWithName(wordName);
}

ForthWord* TypeSystem::FindWordInTOSWord(ExecState* pExecState, const std::string& wordName) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement = pExecState->pStack->TopElement();
	if (pElement != nullptr) {
		ForthType tosType = pElement->GetType();

		if (pTS->TypeIsObject(tosType)) {
			ForthWord* pWord = pTS->FindWordWithName(tosType, wordName);
			return pWord;
		}
	}
	return nullptr;
}