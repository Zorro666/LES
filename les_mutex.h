#ifndef LES_MUTEX_HH
#define LES_MUTEX_HH

#include "les_base.h"

class LES_Mutex
{
public:
	LES_Mutex(int* const pMutexVariable) : m_pMutexVariable(pMutexVariable)
	{
		Lock();
	}
	~LES_Mutex(void)
	{
		UnLock();
	}
private:
	LES_Mutex(void);
	LES_Mutex(LES_Mutex& other);
	LES_Mutex& operator = (LES_Mutex& other);

	void Lock(void);
	void UnLock(void);

	int* const m_pMutexVariable;
};

#endif // #ifndef LES_MUTEX_HH

