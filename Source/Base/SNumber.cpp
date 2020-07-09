//----------------------------------------------------------------------------------------------------------------------
//	SNumber.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SNumber.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	bool	sRandHasBeenSeeded = false;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SNumber

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UInt16 SNumber::getNextPowerOf2(UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Use bit operations to round up to the next power of 2 value
	value--;
	value |= (value >> 8);
	value |= (value >> 4);
	value |= (value >> 2);
	value |= (value >> 1);

	return ++value;
}

//----------------------------------------------------------------------------------------------------------------------
bool SNumber::randomBool()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if seeded
	if (!sRandHasBeenSeeded) {
		// Seed
		::srand((unsigned) ::time(nil));
		sRandHasBeenSeeded = true;
	}

	return (::rand() % 2) == 0;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 SNumber::randomFloat32(Float32 min, Float32 max)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if seeded
	if (!sRandHasBeenSeeded) {
		// Seed
		::srand((unsigned) ::time(nil));
		sRandHasBeenSeeded = true;
	}

	// Setup
	Float32	delta = max - min;

	return (delta == 0.0) ? min : (::rand() % (UInt32) delta) + min;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 SNumber::randomUInt32(UInt32 min, UInt32 max)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if seeded
	if (!sRandHasBeenSeeded) {
		// Seed
		::srand((unsigned) ::time(nil));
		sRandHasBeenSeeded = true;
	}

	// Setup
	UInt32	delta = max - min;

	return (delta == 0) ? min : (::rand() % delta) + min;
}
