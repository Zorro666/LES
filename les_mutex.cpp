#include <pthread.h>

#include "les_base.h"
#include "les_mutex.h"
#include "les_time.h"
#include "les_logger.h"

#if LES_USE_PTHREAD_MUTEX
static int les_lockVar = 0;
#endif // #if LES_USE_PTHREAD_MUTEX

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public external functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_Mutex::Lock(void)
{
#if LES_USE_PTHREAD_MUTEX
	volatile int* const pLockVar = &les_lockVar;
#else // #if LES_USE_PTHREAD_MUTEX
	volatile int* const pLockVar = m_pMutexVariable;
#endif // #if LES_USE_PTHREAD_MUTEX

#if LES_USE_PTHREAD_MUTEX
	pthread_mutex_lock(m_pMutexVariable);
#else // #if LES_USE_PTHREAD_MUTEX
	while (*pLockVar != 0)
	{
		LES_Sleep(1.0e-3f);
	};
#endif // #if LES_USE_PTHREAD_MUTEX

	*pLockVar = 1;
}

void LES_Mutex::UnLock(void)
{
#if LES_USE_PTHREAD_MUTEX
	volatile int* const pLockVar = &les_lockVar;
#else // #if LES_USE_PTHREAD_MUTEX
	volatile int* const pLockVar = m_pMutexVariable;
#endif // #if LES_USE_PTHREAD_MUTEX

	if (*pLockVar != 1)
	{
		LES_FATAL_ERROR("Unlock called but mutex not locked %d", *pLockVar);
	}

#if LES_USE_PTHREAD_MUTEX
	pthread_mutex_unlock(m_pMutexVariable);
#else // #if LES_USE_PTHREAD_MUTEX
	*pLockVar = 0;
#endif // #if LES_USE_PTHREAD_MUTEX
}

void LES_MutexVariableInit(LES_MutexVariable* pMutexVariable)
{
#if LES_USE_PTHREAD_MUTEX
	LES_MutexVariable initMutex = PTHREAD_MUTEX_INITIALIZER;
#else // #if LES_USE_PTHREAD_MUTEX
	LES_MutexVariable initMutex = 0;
#endif // #if LES_USE_PTHREAD_MUTEX

	*pMutexVariable = initMutex;
}
