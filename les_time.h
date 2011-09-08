#ifndef LES_TIME_HH
#define LES_TIME_HH

#include "les_base.h"

float LES_GetElapsedTimeInSeconds(void);
LES_uint64 LES_GetElapsedTimeTicks(void);
void LES_Sleep(const float sleepTimeInSeconds);

#endif //#ifndef LES_TIME_HH
