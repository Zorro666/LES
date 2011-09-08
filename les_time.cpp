#include <sys/timeb.h>
#include <time.h>

#include "les_time.h"

static LES_uint64 les_startTime = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

LES_uint64 LES_GetElapsedTimeTicks(void)
{
	timeb tb;
	ftime(&tb);

	LES_uint64 startTime = les_startTime;
	LES_uint64 newTicks = (tb.time * 1000) + tb.millitm;
	LES_uint64 deltaTicks = (newTicks - startTime);

	if (startTime == 0)
	{
		les_startTime = newTicks;
	}
	return deltaTicks;
}

float LES_GetElapsedTimeInSeconds(void)
{
	const LES_uint64 elapsedTicks = LES_GetElapsedTimeTicks();
	float elapsedSeconds = (float)(elapsedTicks)/1000.0f;
	return elapsedSeconds;
}

void LES_Sleep(const float sleepTimeInSeconds)
{
	timespec sleepTime;
	timespec remainingTime;
	const int numSeconds = (int)sleepTimeInSeconds;
	const int numMicroseconds = (int)((sleepTimeInSeconds-(float)numSeconds)*1000000000.0f);
	sleepTime.tv_sec = numSeconds;
	sleepTime.tv_nsec = numMicroseconds;
	nanosleep(&sleepTime, &remainingTime);
}

