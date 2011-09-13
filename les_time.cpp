#include <sys/timeb.h>
#include <unistd.h>

#include "les_base.h"
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
	#define SLEEP_MAX_TIME_MILLI_SECONDS (900)
	#define SLEEP_MAX_TIME_MICRO_SECONDS (SLEEP_MAX_TIME_MILLI_SECONDS*1000)
	#define SLEEP_MAX_TIME_SECONDS ((float)SLEEP_MAX_TIME_MILLI_SECONDS/1000.0f)

	float timeToSleep = sleepTimeInSeconds;
	while (timeToSleep > SLEEP_MAX_TIME_SECONDS)
	{
		const int numMicroseconds = SLEEP_MAX_TIME_MICRO_SECONDS;
		usleep(numMicroseconds);
		timeToSleep -= SLEEP_MAX_TIME_SECONDS;
	}
	const int numMicroseconds = (int)(timeToSleep * 1000.0f * 1000.0f);
	usleep(numMicroseconds);
}

