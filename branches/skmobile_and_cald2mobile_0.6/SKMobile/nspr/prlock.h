#pragma once

typedef CRITICAL_SECTION  PRLock;

PRLock* PR_NewLock(void);

void PR_DestroyLock(PRLock *lock);

void PR_Lock(PRLock *lock);

PRStatus PR_Unlock(PRLock *lock);
