#include "les_base.h"
#include "les_thread.h"

int LES_CreateThread(LES_ThreadHandle* handle, LES_ThreadAttributes* attributes, LES_ThreadFunction function, void* args)
{
	int ret = pthread_create(handle, attributes, function, args);
	return ret;
}

