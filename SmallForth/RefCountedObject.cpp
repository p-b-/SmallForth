#include <iostream>
#include "RefCountedObject.h"
#include "ForthDict.h"
#include "ForthString.h"

using std::string;

RefCountedObject::RefCountedObject() {
	this->referenceCount = 0;
	this->pDictionary = nullptr;
	this->objectType = ValueType_Undefined;
	this->markedByReferenceCounter = false;
}

RefCountedObject::RefCountedObject(ForthDict* pDict) :
RefCountedObject() {
	//this->referenceCount = 0;
	//this->pDictionary = nullptr;
	//this->objectType = ValueType_Undefined;
	//this->markedByReferenceCounter = false;
	if (pDict == nullptr) {
		InitialiseDictionary();
	}
	else {
		pDictionary = pDict;
		pDictionary->IncReference();
	}
}

RefCountedObject::~RefCountedObject() {
	if (this->pDictionary != nullptr) {
		this->pDictionary->DecReference();
		this->pDictionary = nullptr;
	}
	/*if (this->referenceCount != 0) {
		cout << "Deleting an object with reference " << this->referenceCount << endl;
	}*/
}

void RefCountedObject::InitialiseDictionary() {
	if (this->pDictionary == nullptr) {
		this->pDictionary = new ForthDict();
		this->pDictionary->IncReference();
	}
}

void RefCountedObject::IncReference() {
	// TODO Add atomic increment
	//if (GetObjectTypeId() == 1026) {
	//	ForthString* fs = (ForthString*)this;
	//	cout << "       : " << GetObjectType() << " " << fs->GetContainedString()<<" " << std::hex << this << std::dec << " inc " << this->referenceCount << " to " << (this->referenceCount+1) << endl;
	//}

	++this->referenceCount;
}

void RefCountedObject::DecReference() {
	// TODO Add atomic decrement

	//if (GetObjectTypeId() == 1026) {
	//	ForthString* fs = (ForthString*)this;
	//	cout << "       : " << GetObjectType() << " " << fs->GetContainedString() << " " << std::hex << this << std::dec << " dec " << this->referenceCount << " to " << (this->referenceCount-1) << endl;
	//}
	--this->referenceCount;
	if (this->referenceCount == 0) {
//		cout << "## Deleting ref obj" << std::hex << this << std::dec << endl;
//
		delete this;
	}
}

void RefCountedObject::IncReferenceBy(int by) {
	// TODO Add atomic increment
	this->referenceCount += by;
}

void RefCountedObject::DecReferenceBy(int by) {
	// TODO Add atomic decrement
	this->referenceCount -= by;
	if (this->referenceCount <= 0) {
		delete this;
	}
}


int RefCountedObject::GetCurrentReferenceCount() {
	return this->referenceCount;
}

void RefCountedObject::AddWord(ForthWord* pWordToAdd) {
	if (this->pDictionary == nullptr) {
		this->pDictionary = new ForthDict();
		this->pDictionary->IncReference();
	}
	pDictionary->AddWord(pWordToAdd);
}

ForthWord* RefCountedObject::GetWordWithName(const string& wordName) const {
	if (this->pDictionary != nullptr) {
		return this->pDictionary->FindWord(wordName);
	}
	return nullptr;
}

int RefCountedObject::GetWordCount() const {
	if (this->pDictionary != nullptr) {
		return this->pDictionary->WordCount();
	}
	return 0;
}