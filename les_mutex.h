#ifndef LES_MUTEX_HH
#define LES_MUTEX_HH

#include "les_base.h"

#define LES_USE_PTHREAD_MUTEX (1)

#if LES_USE_PTHREAD_MUTEX
typedef pthread_mutex_t LES_MutexVariable;
#else // #if LES_USE_PTHREAD_MUTEX
#error LES_MUTEX_WONT_WORK_WITHOUT_PTHREADS
typedef int LES_MutexVariable;
#endif // #if LES_USE_PTHREAD_MUTEX

void LES_MutexVariableInit(LES_MutexVariable* pMutexVariable);

class LES_Mutex
{
public:
	LES_Mutex(LES_MutexVariable* const pMutexVariable) : m_pMutexVariable(pMutexVariable)
	{
	}
	~LES_Mutex(void)
	{
	}

	void Lock(void);
	void UnLock(void);
private:
	LES_Mutex(void);
	LES_Mutex(LES_Mutex& other);
	LES_Mutex& operator = (LES_Mutex& other);

	LES_MutexVariable* const m_pMutexVariable;
};

class LES_ScopeMutex
{
public:
	LES_ScopeMutex(LES_MutexVariable* const pMutexVariable) : m_mutex(pMutexVariable)
	{
		m_mutex.Lock();
	}
	~LES_ScopeMutex(void)
	{
		m_mutex.UnLock();
	}
private:
	LES_ScopeMutex(void);
	LES_ScopeMutex(LES_Mutex& other);
	LES_ScopeMutex& operator = (LES_ScopeMutex& other);

	LES_Mutex m_mutex;
};

#endif // #ifndef LES_MUTEX_HH

