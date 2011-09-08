#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "les_core.h"
#include "les_logger.h"
#include "les_test.h"
#include "les_definitionfile.h"
#include "les_time.h"

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

struct NetworkThreadStartStruct
{
	int m_hsock;
};

#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <pthread.h>

void* clientNetworkThread(void* args);

void JAKE_SocketTest()
{
  const int hsock = socket(AF_INET, SOCK_STREAM, 0);
  if (hsock == -1)
	{
      printf("Error initializing socket %d\n",errno);
      return;
  }
    
	int option = 1;
        
  if ((setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(int)) == -1) ||
      (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)&option, sizeof(int)) == -1))
	{
    printf("Error setting options %d\n",errno);
		close(hsock);
    return;
  }

  short host_port = 3141;
  const char* const host_name = "127.0.0.1";
  struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET ;
 	my_addr.sin_port = htons(host_port);
    
 	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = inet_addr(host_name);

 	if (connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1)
	{
		int err = errno;
		if (err != EINPROGRESS)
		{
			fprintf(stderr, "Error connecting socket errno:0x%X\n", errno);
  		close(hsock);
      return;
		}
 	}
	pthread_t networkThread;
	static NetworkThreadStartStruct networkThreadStartStruct;
	networkThreadStartStruct.m_hsock = hsock;
	int ret = pthread_create(&networkThread, NULL, clientNetworkThread, &networkThreadStartStruct);
	LES_LOG("ret = %d", ret);
}

void* clientNetworkThread(void* args)
{
	const NetworkThreadStartStruct* const networkThreadStartStruct = (const NetworkThreadStartStruct* const)args;
	LES_LOG("clientNetworkThread Started");

	const int hsock = networkThreadStartStruct->m_hsock;

	//Now lets do the client related stuff
	while (1)
	{
		const int bufferLen = 1024;
  	char buffer[1024];
		memset(buffer, '\0', bufferLen);

	  printf("Enter some text to send to the server (press enter)\n");
		fgets(buffer, 1024, stdin);
		buffer[strlen(buffer)-1]='\0';
			
		int bytecount;
		bytecount = send(hsock, buffer, strlen(buffer),0);
		if (bytecount == -1)
		{
			fprintf(stderr, "Error sending data %d\n", errno);
			close(hsock);
			return LES_NULL;
		}
		printf("Sent bytes %d\n", bytecount);

		buffer[0] = '\0';
		bytecount = recv(hsock, buffer, bufferLen, 0);
		if (bytecount == -1)
		{
			fprintf(stderr, "Error receiving data %d\n", errno);
			close(hsock);
			return LES_NULL;
		}
		printf("Received bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
	}

  close(hsock);
	LES_LOG("clientNetworkThread Ended");
	return LES_NULL;
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
	JAKE_SocketTest();
	float lastTime = -10000.0f;
	while (1)
	{
		const float logDelta = 1.0f;
		const float elapsedTime = LES_GetElapsedTimeInSeconds();
		if ((elapsedTime - lastTime) > logDelta)
		{
			LES_LOG("Time %f", elapsedTime);
			lastTime = elapsedTime;
		}
		LES_Sleep(2.5f);
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

