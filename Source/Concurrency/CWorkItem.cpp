//----------------------------------------------------------------------------------------------------------------------
//	CWorkItem.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWorkItem.h"

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWorkItemInternals

class CWorkItemInternals {
	public:
		CWorkItemInternals(bool disposeWhenCompletedOrCancelled) :
			mDisposeWhenCompletedOrCancelled(disposeWhenCompletedOrCancelled), mState(kWorkItemStateWaiting)
			{}
		~CWorkItemInternals() {}

		bool			mDisposeWhenCompletedOrCancelled;
		EWorkItemState	mState;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItem

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::CWorkItem(bool disposeWhenCompletedOrCancelled)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CWorkItemInternals(disposeWhenCompletedOrCancelled);
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::~CWorkItem()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
EWorkItemState CWorkItem::getState() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mState;
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::completed() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need dispose
	if (mInternals->mDisposeWhenCompletedOrCancelled) {
		// Dispose
		CWorkItem*	THIS = (CWorkItem*) this;
		Delete(THIS);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::cancelled() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need dispose
	if (mInternals->mDisposeWhenCompletedOrCancelled) {
		// Dispose
		CWorkItem*	THIS = (CWorkItem*) this;
		Delete(THIS);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::transitionTo(EWorkItemState state)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update state
	mInternals->mState = state;

	// Check state
	switch (mInternals->mState) {
		case kWorkItemStateCompleted:	completed();	break;
		case kWorkItemStateCancelled:	cancelled();	break;
		default:										break;
	}
}
