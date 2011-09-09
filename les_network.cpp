#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>

#include "les_base.h"
#include "les_logger.h"

#define LES_NETWORK_INVALID_SOCKET (-1)

struct LES_NetworkThreadStartStruct
{
	int m_socketHandle;
};

struct LES_NetworkMessage
{
	short m_type;
	short m_id;
	int m_payloadSize;
	int m_payload[1];		// m_payload[m_payloadSize/4]
private:
	LES_NetworkMessage();
	LES_NetworkMessage(const LES_NetworkMessage& other);
	LES_NetworkMessage& operator =(const LES_NetworkMessage& other);
	~LES_NetworkMessage();
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_NetworkMessageSize(const int payLoadSize)
{
	int payloadExtraMemmory = payLoadSize - sizeof(int);
	if (payloadExtraMemmory < 0)
	{
		payloadExtraMemmory = 0;
	}
	payloadExtraMemmory = (payloadExtraMemmory + 3) & ~3;
	const int memorySize = sizeof(LES_NetworkMessage) + payloadExtraMemmory;
	return memorySize;
}

static LES_NetworkMessage* LES_CreateNetworkMessage(const short type, const short id, const int payLoadSize, const int messageSize)
{
	LES_NetworkMessage* const pNetworkMessage = (LES_NetworkMessage*)malloc(messageSize);
	pNetworkMessage->m_type = type;
	pNetworkMessage->m_id = id;
	pNetworkMessage->m_payloadSize = payLoadSize;

	return pNetworkMessage;
}

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
	const LES_NetworkThreadStartStruct* const networkThreadStartStruct = (const LES_NetworkThreadStartStruct* const)args;
	LES_LOG("clientNetworkThread Started");

	const int socketHandle = networkThreadStartStruct->m_socketHandle;

	//Now lets do the client related stuff
	const short type = 0x66;
	short int id = 0;

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
			
		id++;
		const int payLoadSize = strlen(buffer);
		const int messageSize = LES_NetworkMessageSize(payLoadSize);
		LES_NetworkMessage* const pNetMessage = LES_CreateNetworkMessage(type, id, payLoadSize, messageSize);
		memcpy(pNetMessage->m_payload, buffer, payLoadSize);

		int bytecount;
		bytecount = send(socketHandle, pNetMessage, messageSize, 0);
		if (bytecount == -1)
		{
			LES_ERROR("Error sending data %d\n", errno);
			break;
		}
		LES_LOG("Sent bytes %d (%d)\n", bytecount, messageSize);

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

	static LES_NetworkThreadStartStruct networkThreadStartStruct;
	networkThreadStartStruct.m_socketHandle = socketHandle;

	pthread_t networkThread;
	int ret = pthread_create(&networkThread, NULL, clientNetworkThread, &networkThreadStartStruct);

	LES_LOG("pthread_create ret = %d", ret);
	return LES_RETURN_OK;
}

