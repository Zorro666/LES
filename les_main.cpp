#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "les_core.h"
#include "les_logger.h"
#include "les_test.h"
#include "les_definitionfile.h"
#include "les_time.h"
#include "les_network.h"
#include "les_thread.h"
#include "les_hash.h"

#include "les_jake.h"

static int s_state;
static LES_uint32 s_correctResponseHash;
enum { LES_STATE_NOT_CONNECTED, LES_STATE_WAITING_FOR_CONNECT_RESPONSE, LES_STATE_CONNECTED };

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

#define LES_NETMESSAGE_RECV_ID_CONNECT_RESPONSE (0x2)
#define LES_NETMESSAGE_RECV_ID_TEST (0x33)

#define LES_NETMESSAGE_SEND_ID_CONNECT (0x1)
#define LES_NETMESSAGE_SEND_ID_TEST (0x66)

static void* inputThread(void*)
{
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

	LES_LOG("Input thread created handle:%d ret:%d", inputThreadHandle, ret);
}

int JAKE_TestMessageHandler(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payloadSize, void* payload)
{
	LES_LOG("Received Message type:0x%X id:0x%X payloadSize:%d payload:'%s'", type, id, payloadSize, (char*)payload);
	return LES_RETURN_OK;
}

int LES_ConnectResponseMessageHandler(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payloadSize, void* payload)
{
	if (type != LES_NETMESSAGE_RECV_ID_CONNECT_RESPONSE)
	{
		LES_ERROR("ConnectResponse wrong type:%d", type);
		return LES_RETURN_ERROR;
	}
	if (id != 234)
	{
		LES_ERROR("ConnectResponse wrong id:%d", id);
		return LES_RETURN_ERROR;
	}
	if (payloadSize != 4)
	{
		LES_ERROR("ConnectResponse wrong payloadSize:%d", payloadSize);
		return LES_RETURN_ERROR;
	}
	LES_uint32 bigResponseHash;
	memcpy(&bigResponseHash, payload, sizeof(LES_uint32));
	const LES_uint32 responseHash = fromBigEndian32(bigResponseHash);
	LES_LOG("responseHash:0x%X", responseHash);
	if (responseHash != s_correctResponseHash)
	{
		LES_ERROR("ConnectResponse wrong responseHash:0x%X Expected:0x%X", responseHash, s_correctResponseHash);
		return LES_RETURN_ERROR;
	}
	s_state = LES_STATE_CONNECTED;
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
		LES_NetworkRegisterReceivedMessageHandler(LES_NETMESSAGE_RECV_ID_TEST, JAKE_TestMessageHandler);
		LES_NetworkRegisterReceivedMessageHandler(LES_NETMESSAGE_RECV_ID_CONNECT_RESPONSE, LES_ConnectResponseMessageHandler);
		s_state = LES_STATE_NOT_CONNECTED;

		JAKE_CreateInputThread();
		float lastTime = -10000.0f;
		while (1)
		{
			if (s_state == LES_STATE_NOT_CONNECTED)
			{
				const LES_uint16 type = LES_NETMESSAGE_SEND_ID_CONNECT;
				const LES_uint16 id = 234;
				const LES_uint32 payloadSize = 16;
				char payload[17];
				for (LES_uint32 i = 0; i < payloadSize; i++)
				{
					char c = '0';
					const int v = rand() % (10+26+26);
					if (v < 10)
					{
						c = (char)('0' + v);
					}
					else if (v < 36)
					{
						c = (char)('a' + (v-10));
					}
					else
					{
						c = (char)('A' + (v-10-26));
					}
					payload[i] = c;
				}
				payload[16] = '\0';
				LES_NetworkSendItem sendItem;
				sendItem.Create(type, id, payloadSize, payload);
				if (LES_NetworkAddSendItem(&sendItem) == LES_RETURN_ERROR)
				{
					LES_ERROR("Error adding connect send item");
				}
				s_state = LES_STATE_WAITING_FOR_CONNECT_RESPONSE;
				s_correctResponseHash = LES_GenerateHashCaseSensitive(payload);
				LES_LOG("connectHash:0x%X", s_correctResponseHash);
			}
			else if (s_state == LES_STATE_WAITING_FOR_CONNECT_RESPONSE)
			{
			}
			else if (s_state == LES_STATE_CONNECTED)
			{
			}

			const float logDelta = 1.0f;
			const float elapsedTime = LES_GetElapsedTimeInSeconds();
			if ((elapsedTime - lastTime) > logDelta)
			{
				LES_LOG("Time %f State:%d", elapsedTime, s_state);
				lastTime = elapsedTime;
			}
			LES_NetworkTick();
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

