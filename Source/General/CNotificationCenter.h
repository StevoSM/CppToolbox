//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

typedef	void	(*NotificationProc)(const CString& notificationName, const void* senderRef, const CDictionary& info,
						void* userData);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SNotificationObserverInfo

struct SNotificationObserverInfo {
			// Lifecycle methods
			SNotificationObserverInfo(const void* observerRef, NotificationProc proc, void* userData) :
				mObserverRef(observerRef), mProc(proc), mUserData(userData)
				{}
			SNotificationObserverInfo(const SNotificationObserverInfo& other) :
				mObserverRef(other.mObserverRef), mProc(other.mProc), mUserData(other.mUserData)
				{}

			// Instance methods
	void	callProc(const CString& notificationName, const void* senderRef, const CDictionary& info)
				{ mProc(notificationName, senderRef, info, mUserData); }

	// Properties
	const	void*				mObserverRef;
			NotificationProc	mProc;
			void*				mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CNotificationCenter

class CNotificationCenterInternals;
class CNotificationCenter {
	// Methods
	public:
						// Lifcycle methods
						CNotificationCenter();
		virtual			~CNotificationCenter();

						// Instance methods
				void	registerObserver(const CString& notificationName, const void* senderRef,
								const SNotificationObserverInfo& notificationObserverInfo);
				void	registerObserver(const CString& notificationName,
								const SNotificationObserverInfo& notificationObserverInfo);
				void	unregisterObserver(const CString& notificationName, const void* observerRef);
				void	unregisterObserver(const void* observerRef);

				void	send(const CString& notificationName, const void* senderRef = nil,
								const CDictionary& info = CDictionary::mEmpty) const;
//				void	postOnMainThread(const CString& notificationName, const void* senderRef = nil,
//								const CDictionary& info = CDictionary::mEmpty);

	// Properties
	private:
				CNotificationCenterInternals*	mInternals;

	public:
		static	CNotificationCenter				mStandard;
};