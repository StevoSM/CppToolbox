//----------------------------------------------------------------------------------------------------------------------
//	CThread-Apple.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
void CThread::runOnMain(Proc proc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	dispatch_async(dispatch_get_main_queue(), ^{ proc(userData); });
}
