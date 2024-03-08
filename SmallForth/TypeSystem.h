#pragma once
#include <string>
#include <map>
#include <tuple>
class ExecState;
class UserDefinedObject;
class ForthWord;

class RegisteredType {
public:
	RegisteredType(const std::string& name, unsigned int id, XT constructor, XT binaryOps, UserDefinedObject* pDefiningObject) {
		this->id = id;
		this->name = name;
		this->constructorXT = constructor;
		this->binaryOpsXT = binaryOps;
		this->definingObject = pDefiningObject;
	}
	unsigned int id;
	std::string name;
	XT constructorXT;
	XT binaryOpsXT;
	UserDefinedObject* definingObject;
};

class TypeSystem
{
	TypeSystem();
public:
	static TypeSystem* GetTypeSystem();
		
	bool RegisterValueType(ExecState* pExecState, std::string typeName);
	ForthType RegisterObjectType(ExecState* pExecState, std::string typeName, XT constructXT=nullptr, XT binaryOpsXT = nullptr);
	ForthType RegisterUserObjectType(ExecState* pExecState, std::string typeName, UserDefinedObject* pNewObjectDefinition);
	std::string TypeToString(ForthType type) const;
	
public:
	bool Construct(ExecState* pExecState, ForthType type);
	XT GetConstructXTForType(ForthType type) const;
	XT GetBinaryOpsHandlerForType(ForthType type) const;
	ForthType CreatePointerTypeTo(ForthType fromType) const;
	uint32_t GetValueType(ForthType type) const { return type & 0xffff;  }
	bool IsValueOrValuePter(ForthType type) const;
	bool IsValue(ForthType type) const;

	bool TypeIsObject(ForthType type) const;
	bool TypeIsObjectOrObjectPter(ForthType type) const;
	bool TypeIsSystemObject(ForthType type) const;
	bool TypeIsUserObject(ForthType type) const;
	uint32_t GetIndirectionLevel(ForthType type) const;
	void IncReferenceForPter(ForthType type, void* pter);
	void IncReferenceForPterBy(ForthType type, void* pter, int by);

	void DecReferenceForPter(ForthType type, void* pter);
	void DecReferenceForPterBy(ForthType type, void* pter, int by);
	std::tuple<bool, void*> DeferencePointerToObjectPter(ForthType type, void* pter) const;
	std::tuple<bool, const void*> DeferencePointerToObjectPter(ForthType type, const void* pter) const;
	std::tuple<bool, const void*> DeferencePointerToValuePter(ForthType type, const void* pter) const;
	std::tuple<ForthType, void*> DeferencePointer(ForthType type, void* pter);
	bool ValueCompatibleWithAddress(ForthType addressType, ForthType valueType, bool directAssignment) const;
	int GetReferenceCount(ForthType type, void* pter);
	int GetPterReferenceCount(ForthType type, void* pter);


	bool VariableToString(ExecState* pExecState, ForthType type, const void* pter) const;
	bool TypeExists(std::string typeName) const;
	
	bool AddWordToObject(ExecState* pExecState, ForthType type, ForthWord* pWord);
	ForthWord* FindWordWithName(ForthType type, const std::string& wordName);
	ForthWord* FindWordInTOSWord(ExecState* pExecState, const std::string& wordName);


public:
	static bool IsNumeric(ForthType type) { return type == StackElement_Int || 
												   type == StackElement_Float; }
	static bool CanConvertToInt(ForthType type) { return type == StackElement_Int || 
												         type == StackElement_Float || 
												         type == StackElement_Char || 
													     type==StackElement_Bool; }
	static bool IsPter(ForthType type) { return (type & 0xffff0000) != 0; }

public:

	static const unsigned int typeIdInvalid = 1023;

private:
	bool RegisterType(ExecState* pExecState, std::string typeName, int typeId, XT constructXT, XT binaryOpsXT, UserDefinedObject* pDefiningType);
	std::string GetBaseTypeNameForId(unsigned int typeId) const;
	unsigned int GetBaseTypeIdForName(std::string typeName) const;
	bool ValueTypeToString(ExecState* pExecState, ForthType forthType, const void* pter) const;
	RegisteredType* GetRegisteredTypeForTypeId(ForthType type) const;
	bool ConstructUserType(ExecState* pExecState, ForthType type);
	bool ConstructSystemType(ExecState* pExecState, ForthType type);

private:
	unsigned int nextValueTypeId;
	unsigned int firstObjectTypeId;
	unsigned int nextObjectTypeId;
	unsigned int firstUserObjectTypeId;
	unsigned int nextUserObjectTypeId;
	unsigned int maxValueTypeId;
	unsigned int maxObjectTypeId;
	unsigned int maxUserObjectTypeId;

	std::map<std::string, unsigned int> typeNameToId;
	std::map<unsigned int, std::string> typeIdToName;
	std::map<unsigned int, RegisteredType*> typeIdToRegisteredType;

	static TypeSystem* s_pTypeSystem;
};

