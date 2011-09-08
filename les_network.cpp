#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "les_base.h"
#include "les_logger.h"

#define LES_NETWORK_INVALID_SOCKET (-1)

struct NetworkThreadStartStruct
{
	int m_socketHandle;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_CreateTCPSocket(const char* const ip, const short port)
{
  const int socketHandle = socket(AF_INET, SOCK_STREAM, 0);
  if (socketHandle == -1)
	{
      LES_ERROR("LES_CreateTCPSocket::Error initializing socket %d\n",errno);
      return LES_NETWORK_INVALID_SOCKET;
  }
    
	int option = 1;
        
  if ((setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(int)) == -1) ||
      (setsockopt(socketHandle, SOL_SOCKET, SO_KEEPALIVE, (char*)&option, sizeof(int)) == -1))
	{
    LES_ERROR("LES_CreateTCPSocket::Error setting options %d\n",errno);
		close(socketHandle);
    return LES_NETWORK_INVALID_SOCKET;
  }

  const short hostPort = port;
  const char* const hostIP = ip;
  struct sockaddr_in hostAddr;
	hostAddr.sin_family = AF_INET ;
 	hostAddr.sin_port = htons(hostPort);
    
 	memset(&(hostAddr.sin_zero), 0, 8);
	hostAddr.sin_addr.s_addr = inet_addr(hostIP);

 	if (connect(socketHandle, (struct sockaddr*)&hostAddr, sizeof(hostAddr)) == -1)
	{
		int err = errno;
		if (err != EINPROGRESS)
		{
			LES_ERROR("LES_CreateTCPSocket::Error connecting socket errno:0x%X\n", errno);
  		close(socketHandle);
    	return LES_NETWORK_INVALID_SOCKET;
		}
 	}
	return socketHandle;
}

static void* clientNetworkThread(void* args)
{
	const NetworkThreadStartStruct* const networkThreadStartStruct = (const NetworkThreadStartStruct* const)args;
	LES_LOG("clientNetworkThread Started");

	const int socketHandle = networkThreadStartStruct->m_socketHandle;

	//Now lets do the client related stuff
	while (1)
	{
		const int bufferLen = 1024;
  	char buffer[1024];
		memset(buffer, '\0', bufferLen);

	  LES_LOG("Enter some text to send to the server (press enter)\n");
		fgets(buffer, 1024, stdin);
		buffer[strlen(buffer)-1]='\0';

		if (strcmp(buffer, "quit") == 0)
		{
			break;
		}
			
		int bytecount;
		bytecount = send(socketHandle, buffer, strlen(buffer),0);
		if (bytecount == -1)
		{
			LES_ERROR("Error sending data %d\n", errno);
			break;
		}
		LES_LOG("Sent bytes %d\n", bytecount);

		buffer[0] = '\0';
		bytecount = recv(socketHandle, buffer, bufferLen, 0);
		if (bytecount == -1)
		{
			LES_ERROR("Error receiving data %d\n", errno);
			break;
		}
		LES_LOG("Received bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
	}

  close(socketHandle);
	LES_LOG("clientNetworkThread Ended");
	return LES_NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

int JAKE_SocketTest(const char* const ip, const short port)
{
	const int socketHandle = LES_CreateTCPSocket(ip, port);

	if (socketHandle == LES_NETWORK_INVALID_SOCKET)
	{
		return LES_RETURN_ERROR;
	}

	static NetworkThreadStartStruct networkThreadStartStruct;
	networkThreadStartStruct.m_socketHandle = socketHandle;

	pthread_t networkThread;
	int ret = pthread_create(&networkThread, NULL, clientNetworkThread, &networkThreadStartStruct);

	LES_LOG("pthread_create ret = %d", ret);
	return LES_RETURN_OK;
}

