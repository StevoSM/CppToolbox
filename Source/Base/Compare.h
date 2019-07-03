//----------------------------------------------------------------------------------------------------------------------
//	Compare.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Compare result

enum ECompareResult {
	kCompareResultBefore		= -1,
	kCompareResultEquivalent	= 0,
	kCompareResultAfter			= 1,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Compare procs

extern	ECompareResult	eCompare(UInt32 value1, UInt32 value2);
