//----------------------------------------------------------------------------------------------------------------------
//	CThread-Apple.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
void CThread::runOnMainThread(CThreadRunOnMainThreadProc runOnMainThreadProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	dispatch_async(dispatch_get_main_queue(), ^{ runOnMainThreadProc(userData); });
}