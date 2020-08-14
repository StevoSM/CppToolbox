//----------------------------------------------------------------------------------------------------------------------
//	ConcurrencyPrimitives-POSIX.cpp			©2018 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "ConcurrencyPrimitives.h"

#include <pthread.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CLockInternals

class CLockInternals {
	public:
		CLockInternals()
			{ ::pthread_mutex_init(&mMutex, nil); }
		~CLockInternals()
			{ ::pthread_mutex_destroy(&mMutex); }

		pthread_mutex_t	mMutex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLock

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CLock::CLock()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CLockInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CLock::~CLock()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CLock::tryLock() const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::pthread_mutex_trylock(&mInternals->mMutex) == 0;
}

//----------------------------------------------------------------------------------------------------------------------
void CLock::lock() const
//----------------------------------------------------------------------------------------------------------------------
{
	::pthread_mutex_lock(&mInternals->mMutex);
}

//----------------------------------------------------------------------------------------------------------------------
void CLock::unlock() const
//----------------------------------------------------------------------------------------------------------------------
{
	::pthread_mutex_unlock(&mInternals->mMutex);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CReadPreferringLockInternals

class CReadPreferringLockInternals {
	public:
		CReadPreferringLockInternals()
			{ ::pthread_rwlock_init(&mRWLock, nil); }
		~CReadPreferringLockInternals()
			{ pthread_rwlock_destroy(&mRWLock); }

		pthread_rwlock_t	mRWLock;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CReadPreferringLock

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CReadPreferringLock::CReadPreferringLock()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CReadPreferringLockInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CReadPreferringLock::~CReadPreferringLock()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CReadPreferringLock::lockForReading() const
//----------------------------------------------------------------------------------------------------------------------
{
	::pthread_rwlock_rdlock(&mInternals->mRWLock);
}

//----------------------------------------------------------------------------------------------------------------------
void CReadPreferringLock::unlockForReading() const
//----------------------------------------------------------------------------------------------------------------------
{
	::pthread_rwlock_unlock(&mInternals->mRWLock);
}

//----------------------------------------------------------------------------------------------------------------------
void CReadPreferringLock::lockForWriting() const
//----------------------------------------------------------------------------------------------------------------------
{
	::pthread_rwlock_wrlock(&mInternals->mRWLock);
}

//----------------------------------------------------------------------------------------------------------------------
void CReadPreferringLock::unlockForWriting() const
//----------------------------------------------------------------------------------------------------------------------
{
	::pthread_rwlock_unlock(&mInternals->mRWLock);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSemaphoreInternals

class CSemaphoreInternals {
	public:
		CSemaphoreInternals()
			{
				::pthread_cond_init(&mCond, nil);
				::pthread_mutex_init(&mMutex, nil);
			}
		~CSemaphoreInternals()
			{
				::pthread_cond_destroy(&mCond);
				::pthread_mutex_destroy(&mMutex);
			}

		pthread_cond_t	mCond;
		pthread_mutex_t	mMutex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSemaphore

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSemaphore::CSemaphore()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSemaphoreInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CSemaphore::~CSemaphore()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CSemaphore::signal() const
//----------------------------------------------------------------------------------------------------------------------
{
	::pthread_cond_signal(&mInternals->mCond);
}

//----------------------------------------------------------------------------------------------------------------------
void CSemaphore::waitFor() const
//----------------------------------------------------------------------------------------------------------------------
{
	::pthread_cond_wait(&mInternals->mCond, &mInternals->mMutex);
}
