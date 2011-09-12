#ifndef LES_THREAD_HH
#define LES_THREAD_HH

#include <pthread.h>

#include "les_base.h"

typedef pthread_t LES_ThreadHandle;
typedef pthread_attr_t LES_ThreadAttributes;
typedef void* LES_ThreadFunction(void* args); 

int LES_CreateThread(LES_ThreadHandle* handle, LES_ThreadAttributes* attributes, LES_ThreadFunction function, void* args);

#endif // #ifndef LES_THREAD_HH

