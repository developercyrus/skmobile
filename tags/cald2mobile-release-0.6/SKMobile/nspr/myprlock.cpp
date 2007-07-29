
#include <Windows.h>
#include "prtypes.h"
#include "prlock.h"

PRLock* PR_NewLock(void)
{
	CRITICAL_SECTION * pMutex = new CRITICAL_SECTION;
	::InitializeCriticalSection (pMutex);
	return pMutex;
}

void PR_DestroyLock(PRLock *lock)
{
	DeleteCriticalSection(lock);
	delete lock;
}

void PR_Lock(PRLock *lock)
{
	EnterCriticalSection(lock);
}

PRStatus PR_Unlock(PRLock *lock)
{
	LeaveCriticalSection(lock);
	return PR_SUCCESS;
}