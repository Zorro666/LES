#include "les_base.h"
#include "les_mutex.h"
#include "les_time.h"
#include "les_logger.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public external functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_Mutex::Lock(void)
{
	volatile int* const pMutexVariable = m_pMutexVariable;
	while (*pMutexVariable != 0)
	{
		LES_Sleep(1.0e-3f);
	};
	*pMutexVariable = 1;
}

void LES_Mutex::UnLock(void)
{
	volatile int* const pMutexVariable = m_pMutexVariable;
	if (*pMutexVariable != 1)
	{
		LES_ERROR("Unlock called but mutex not locked %d", *pMutexVariable);
	}
	*m_pMutexVariable = 0;
}
