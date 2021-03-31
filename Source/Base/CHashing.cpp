//----------------------------------------------------------------------------------------------------------------------
//	CHasher.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CHashing.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

// Based on https://stackoverflow.com/questions/8317508/hash-function-for-a-string
const	UInt32	kA = 54059; /* a prime */
const	UInt32	kB = 76963; /* another prime */
//const	UInt32	kC = 86969; /* yet another prime */
const	UInt32	kInitialValue = 37; /* also prime */

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CHasherInternals

class CHasherInternals {
	public:
				CHasherInternals() : mValue(kInitialValue) {}
				~CHasherInternals() {}

		void	add(const char* string)
					{
						// Iterate all characters
						for (; *string != 0; string++)
							// Update value
							mValue = (mValue * kA) ^ ((UInt32) string[0] * kB);
					}

		UInt32	mValue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CHasher

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CHasher::CHasher()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CHasherInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CHasher::~CHasher()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CHasher::add(const char* string)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->add(string);
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CHasher::getValue() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mValue;	 // or return mValue % kC
}
