#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "les_core.h"
#include "les_logger.h"
#include "les_test.h"
#include "les_definitionfile.h"
#include "les_time.h"
#include "les_network.h"

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
	if (LES_NetworkCreateTCPSocket("127.0.0.1", 3141) == LES_RETURN_OK)
	{
		float lastTime = -10000.0f;
		const short type = 0x66;
		short int id = 0;
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
			
			const int payLoadSize = stringLen;
			LES_NetworkSendItem sendItem;
			sendItem.Create(type, id, payLoadSize, buffer);
			if (LES_NetworkAddSendItem(&sendItem) == LES_RETURN_ERROR)
			{
				LES_ERROR("Error adding send item");
			}
			id++;

			const float logDelta = 1.0f;
			const float elapsedTime = LES_GetElapsedTimeInSeconds();
			if ((elapsedTime - lastTime) > logDelta)
			{
				LES_LOG("Time %f", elapsedTime);
				lastTime = elapsedTime;
			}
			LES_Sleep(0.1f);
		}
	}

	if (JAKE_LoadDefinitionFile("defTest.bin") != LES_RETURN_OK)
	{
		LES_FATAL_ERROR("Failed to load test definition file");
	}

	if (runTests)
	{
		LES_DebugOutputGlobalDefinitionFile(LES_Logger::GetDefaultChannel(LES_Logger::CHANNEL_LOG));
	}

	if (runTests)
	{
		LES_TestSetup();
		LES_jakeInit(666, 123);
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

