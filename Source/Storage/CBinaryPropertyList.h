//----------------------------------------------------------------------------------------------------------------------
//	CBinaryPropertyList.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CByteParceller.h"
#include "CDictionary.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBinaryPropertyList

class CBinaryPropertyList {
	// DictionaryResult
	public:
		struct DictionaryResult {
			// Lifecycle methods
			DictionaryResult(const CDictionary& dictionary) : mDictionary(OI<CDictionary>(dictionary)) {}
			DictionaryResult(const SError& error) : mError(OI<SError>(error)) {}

			// Properties
			OI<CDictionary>	mDictionary;
			OI<SError>		mError;
		};

	// Methods
	public:
									// Class methods
		static	DictionaryResult	dictionaryFrom(const CByteParceller& byteParceller);
//		static	OI<SError>			writeTo(const CFile& file);
};
