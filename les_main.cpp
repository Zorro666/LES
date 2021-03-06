#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "les_core.h"
#include "les_coreengine.h"
#include "les_logger.h"
#include "les_test.h"
#include "les_definitionfile.h"
#include "les_time.h"
#include "les_network.h"
#include "les_thread.h"
#include "les_hash.h"

#include "les_jake.h"

void JAKE_Test()
{
#if 0
	const char* read = "Jake";
	char chr;
	int hash = 0;
	while ((chr = *read) != 0)
	{
		hash += (hash << 7); // hash=hash+128*hash
		hash += (hash << 1); // hash=hash+128*hash+2*hash=131*hash
		hash += (int)chr; // hash=131*hash+char
		printf("hashValue=%u c=%c %d\n",hash, chr, chr);
		read++;
	}
#endif // #if 0
}

int JAKE_LoadDefinitionFile(const char* const fname)
{
	FILE* fh = fopen(fname, "rb");
	fseek(fh, 0, SEEK_END);
	int dataSize = ftell(fh);
	fseek(fh, 0, SEEK_SET);
	char* fileData = new char[dataSize];
	fread(fileData, sizeof(char), dataSize, fh);
	fclose(fh);

	if (LES_SetGlobalDefinitionFile(fileData, dataSize) != LES_RETURN_OK)
	{
		LES_FATAL_ERROR("LES_SetGlobalDefinitionFile '%s' Size:%d failed", fname, dataSize);
	}

	delete[] fileData;

	return LES_RETURN_OK;
}

static void* inputThread(void*)
{
	#define LES_NETMESSAGE_SEND_ID_TEST (0x66)
	const LES_uint16 type = LES_NETMESSAGE_SEND_ID_TEST;
	LES_uint16 id = 0;
	while (1)
	{
		const int bufferLen = 128;
		char buffer[128];
		memset(buffer, '\0', bufferLen);

		LES_LOG("Enter some text to send to the server (press enter)");
		fgets(buffer, 128, stdin);
		const int stringLen = strlen(buffer);
		buffer[stringLen-1]='\0';

		if (strcmp(buffer, "quit") == 0)
		{
			break;
		}

		const LES_uint32 payloadSize = stringLen;
		LES_NetworkSendItem sendItem;
		sendItem.Create(type, id, payloadSize, buffer);
		// This is not thread safe - it is meant to be run on the main thread only
		if (LES_NetworkAddSendItem(&sendItem) == LES_RETURN_ERROR)
		{
			LES_ERROR("Error adding send item");
		}
		id++;
	}
	return LES_NULL;
}

void JAKE_CreateInputThread(void)
{
	static LES_ThreadHandle inputThreadHandle;
	int ret = LES_CreateThread(&inputThreadHandle, LES_NULL, inputThread, LES_NULL);

	LES_LOG("Input thread created handle:0x%X ret:%d", inputThreadHandle, ret);
}

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
	JAKE_Test();
	LES_Logger::Init();
	LES_Logger::SetConsoleOutput(LES_Logger::CHANNEL_LOG, verbose);
	LES_Init();

	bool inputThreadAlive = false;
	bool debugOutputDefinitionFile = false;
	float lastTime = -10000.0f;
	while (1)
	{
		const int state = LES_CoreEngineGetState();

		const float logDelta = 1.0f;
		const float elapsedTime = LES_GetElapsedTimeInSeconds();
		if ((elapsedTime - lastTime) > logDelta)
		{
			LES_LOG("Time %f State:%d", elapsedTime, state);
			lastTime = elapsedTime;
		}

		const int ret = LES_CoreEngineTick();
		if (ret == LES_RETURN_ERROR)
		{
			LES_ERROR("ERROR during LES_CoreEngineTick()");
			break;
		}
		else if (ret == LES_COREENGINE_FINISHED)
		{
			LES_LOG("LES_CoreEngineTick() finished");
			break;
		}
		if (state == LES_STATE_READY)
		{
			if (runTests)
			{
				if (LES_TestSetup() == LES_RETURN_ERROR)
				{
					LES_LOG("LES_TestSetup() finished");
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (inputThreadAlive == false)
		{
			JAKE_CreateInputThread();
			inputThreadAlive = true;
		}
		LES_Sleep(0.033333f);
	}

	if (LES_IsGlobalDefinitionFileValid() != LES_RETURN_OK)
	{
		if (JAKE_LoadDefinitionFile("defTest.bin") != LES_RETURN_OK)
		{
			LES_FATAL_ERROR("Failed to load test definition file");
		}
		debugOutputDefinitionFile = true;
	}

	if (runTests)
	{
		int retVal = 0;
		do
		{
			retVal = LES_TestSetup();
		}
		while (retVal == LES_RETURN_OK);
		LES_jakeInit(666, 123);
		debugOutputDefinitionFile = true;
	}

	if (debugOutputDefinitionFile)
	{
		LES_DebugOutputGlobalDefinitionFile(LES_Logger::GetDefaultChannel(LES_Logger::CHANNEL_LOG));
	}

	LES_Logger::SetChannelOutputFileName(LES_Logger::CHANNEL_WARNING, "warning.txt");
	LES_Logger::SetChannelOutputFileName(LES_Logger::CHANNEL_ERROR, "error.txt");
	LES_Logger::SetChannelOutputFileName(LES_Logger::CHANNEL_LOG, "console.txt");

	LES_Logger::SetFileOutput(LES_Logger::CHANNEL_LOG, true);
	LES_Logger::SetConsoleOutput(LES_Logger::CHANNEL_LOG, true);

	LES_LoggerChannel* jakeChannel = LES_Logger::CreateChannel("Jake", "Custom: ", "jake.txt", LES_Logger::FLAGS_DEFAULT);
	LES_Logger::ClearErrorStatus();
	LES_LOG("0 ErrorStatus:%d", LES_Logger::GetErrorStatus());
	for (int i = 0; i < argc; i++)
	{
		jakeChannel->Print("arg[%d] '%s'", i, argv[i]);
		LES_LOG("arg[%d] '%s'", i, argv[i]);
	}
	LES_LOG("0 ErrorStatus:%d", LES_Logger::GetErrorStatus());
	LES_Logger::SetFatal(LES_Logger::CHANNEL_WARNING, false);
	LES_Logger::SetFileOutput(LES_Logger::CHANNEL_WARNING, true);
	LES_WARNING("A test warning");
	LES_LOG("0 ErrorStatus:%d", LES_Logger::GetErrorStatus());

	LES_Logger::SetFatal(LES_Logger::CHANNEL_ERROR, false);
	LES_Logger::SetFileOutput(LES_Logger::CHANNEL_ERROR, true);
	LES_ERROR("A test error");
	LES_LOG("1 ErrorStatus:%d", LES_Logger::GetErrorStatus());
	LES_Logger::ClearErrorStatus();
	LES_LOG("0 ErrorStatus:%d", LES_Logger::GetErrorStatus());

	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_Logger::SetFileOutput(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_LOG("0 ErrorStatus:%d", LES_Logger::GetErrorStatus());
	LES_FATAL_ERROR("A test fatal error");
	LES_LOG("1 ErrorStatus:%d", LES_Logger::GetErrorStatus());

	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);

	LES_Shutdown();
	return 0;
}

