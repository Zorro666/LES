#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "les_core.h"
#include "les_logger.h"
#include "les_test.h"

#include "les_jake.h"

#if 0
void JAKE_Test(void)
{
	int numTests = 10000;
	while (numTests > 0)
	{
		int r;
		float randFloatUnit;
		const float xScale = 10.0f;
		const float deltaScale = 4.0f;
		const float epsilon = 1.0f;
		const float gridSize = epsilon * 2.0f;

		r = rand();
		randFloatUnit = (float)r/(float)RAND_MAX;

		float x1 = 1.0f + xScale * (randFloatUnit - 0.5f + 0.5f);

		r = rand();
		randFloatUnit = (float)r/(float)RAND_MAX;
		float x2 = 1.0f + x1 + deltaScale * (randFloatUnit - 0.5f + 0.5f);

		const bool theSameTruth = fabsf(x2-x1) < epsilon;

		//const int bin1_minus = (int)(((x1-epsilon)/ gridSize) + 0.5f);
		//const int bin1_plus = (int)(((x1+epsilon) / gridSize) + 0.5f);
		//const int bin1_minus = (int)(((x1-epsilon)/ gridSize));
		const int bin1_minus = (int)(((x1)/ gridSize));
		const int bin1_plus = (int)(((x1+epsilon) / gridSize));

		int bin1Odd = bin1_minus;
		if (bin1_minus == bin1_plus)
		{
		}
		else if ((bin1_minus & 0x1) == 0)
		{
			bin1Odd = bin1_plus;
		}
		if ((bin1_plus - bin1_minus) > 1)
		{
			printf("fsjkfdjfdjkfsdjklf\n");
			exit(-1);
		}

		//const int bin2_minus = (int)(((x2-epsilon)/ gridSize) + 0.5f);
		//const int bin2_plus = (int)(((x2+epsilon) / gridSize) + 0.5f);
		const int bin2_minus = (int)(((x2)/ gridSize));
		const int bin2_plus = (int)(((x2+epsilon) / gridSize));
		int bin2Odd = bin2_minus;
		if (bin2_minus == bin2_plus)
		{
		}
		else if ((bin2_minus & 0x1) == 0)
		{
			bin2Odd = bin2_plus;
		}
		if ((bin2_plus - bin2_minus) > 1)
		{
			printf("fsjkfdjfdjkfsdjklf\n");
			exit(-1);
		}

		const bool theSameBin = (bin1Odd == bin2Odd);

		if (theSameBin != theSameTruth)
		{
			printf("Doesn't match theSameBin:%d theSameTruth:%d\n", theSameBin, theSameTruth);
			printf("x1 %f bin1_minus %d bin1_plus %d bin1Odd %d\n", x1, bin1_minus, bin1_plus, bin1Odd);
			printf("x2 %f bin2_minus %d bin2_plus %d bin2Odd %d\n", x2, bin2_minus, bin2_plus, bin2Odd);
			printf("fabsf(x1 - x2):%f\n", fabsf(x1-x2));
			exit(-1);
		}

		numTests--;
	}
}
#endif // #if 0

int main(const int argc, const char* const argv[])
{
	bool verbose = true;
	bool runTests = false;
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-verbose") == 0)
		{
			verbose = true;
		}
		if (strcmp(argv[i], "-quiet") == 0)
		{
			verbose = false;
		}
		if (strcmp(argv[i], "-tests") == 0)
		{
			runTests = true;
		}
		if (strcmp(argv[i], "-notests") == 0)
		{
			runTests = false;
		}
	}
	//JAKE_Test();
	LES_Logger::Init();
	LES_Logger::SetConsoleOutput(LES_Logger::CHANNEL_LOG, verbose);
	LES_Init();
	if (runTests)
	{
		LES_TestSetup();
		LES_jakeInit(666, 123);
	}

	LES_Logger::SetFileOutput(LES_Logger::CHANNEL_LOG, false);
	LES_Logger::SetConsoleOutput(LES_Logger::CHANNEL_LOG, true);
	for (int i = 0; i < argc; i++)
	{
		LES_LOG("arg[%d] '%s'\n", i, argv[i]);
	}
	LES_Logger::SetFatal(LES_Logger::CHANNEL_WARNING, false);
	LES_Logger::SetFileOutput(LES_Logger::CHANNEL_WARNING, false);
	LES_WARNING("A test warning\n");

	LES_Logger::SetFatal(LES_Logger::CHANNEL_ERROR, false);
	LES_Logger::SetFileOutput(LES_Logger::CHANNEL_ERROR, false);
	LES_ERROR("A test error\n");

	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_Logger::SetFileOutput(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FATAL_ERROR("A test fatal error\n");

	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);

	LES_Shutdown();
	return 0;
}

