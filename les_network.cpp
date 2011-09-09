#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>

#include "les_base.h"
#include "les_network.h"
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

static LES_NetworkMessage* LES_CreateNetworkMessage(const short type, const short id, const int payLoadSize, void* const payload, const int messageSize)
{
	LES_NetworkMessage* const pNetworkMessage = (LES_NetworkMessage*)malloc(messageSize);
	pNetworkMessage->m_type = toBigEndian16(type);
	pNetworkMessage->m_id = toBigEndian16(id);
	pNetworkMessage->m_payloadSize = toBigEndian32(payLoadSize);
	memcpy(pNetworkMessage->m_payload, payload, payLoadSize);

	return pNetworkMessage;
}

static int LES_CreateTCPSocket(const char* const ip, const short port)
{
  const int socketHandle = socket(AF_INET, SOCK_STREAM, 0);
  if (socketHandle == -1)
	{
      LES_ERROR("LES_CreateTCPSocket::Error initializing socket %d",errno);
      return LES_NETWORK_INVALID_SOCKET;
  }
    
	int option = 1;
        
  if ((setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(int)) == -1) ||
      (setsockopt(socketHandle, SOL_SOCKET, SO_KEEPALIVE, (char*)&option, sizeof(int)) == -1))
	{
    LES_ERROR("LES_CreateTCPSocket::Error setting options %d",errno);
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
			LES_ERROR("LES_CreateTCPSocket::Error connecting socket errno:0x%X", errno);
  		close(socketHandle);
    	return LES_NETWORK_INVALID_SOCKET;
		}
 	}
	return socketHandle;
}

LES_NetworkSendItem s_sendItem;

static void* clientNetworkThread(void* args)
{
	const LES_NetworkThreadStartStruct* const networkThreadStartStruct = (const LES_NetworkThreadStartStruct* const)args;
	LES_LOG("clientNetworkThread Started");

	const int socketHandle = networkThreadStartStruct->m_socketHandle;

	//Now lets do the client related stuff
	while (1)
	{
		// NEEDS A MUTEX LOCK
		// Get the sendItem from a queue and make main thread add to the sendItem

		LES_NetworkSendItem* const pSendItem = &s_sendItem;
		void* pSendData = (void*)(pSendItem->m_message);
		const int sendDataSize = pSendItem->m_messageSize;
		if (sendDataSize == 0)
		{
			continue;
		}

		const int bytesSent = send(socketHandle, pSendData, sendDataSize, 0);
		if (bytesSent == -1)
		{
			LES_ERROR("Error sending data %d", errno);
			break;
		}
		LES_LOG("Sent bytes %d (%d)", bytesSent, sendDataSize);
		pSendItem->Free();

 		char buffer[128];
		const int bufferLen = 128;
		buffer[0] = '\0';
		const int bytesReceived = recv(socketHandle, buffer, bufferLen, 0);
		if (bytesReceived == -1)
		{
			LES_ERROR("Error receiving data %d", errno);
			break;
		}
		// Make a NetReceiveItem struct and put data into it
		LES_NetworkMessage* const pReceivedMessage = (LES_NetworkMessage* const)buffer;
		short receivedType = fromBigEndian16(pReceivedMessage->m_type);
		short receivedId = fromBigEndian16(pReceivedMessage->m_id);
		int receivedPayloadSize = fromBigEndian32(pReceivedMessage->m_payloadSize);

		LES_LOG("Received bytes %d", bytesReceived);
		LES_LOG("Received message: type:0x%X id:%d payloadSize:%d", receivedType, receivedId, receivedPayloadSize);
		LES_LOG("payload:'%s'", (char*)pReceivedMessage->m_payload);
	}

  close(socketHandle);
	LES_LOG("clientNetworkThread Ended");
	return LES_NULL;
}

void LES_NetworkSendItem::Create(const short type, const short id, const int payLoadSize, void* const payload)
{
	const int messageSize = LES_NetworkMessageSize(payLoadSize);
	m_message = LES_CreateNetworkMessage(type, id, payLoadSize, payload, messageSize);
	m_messageSize = messageSize;
}

void LES_NetworkSendItem::Free(void)
{
	free(m_message);
	m_message = LES_NULL;
	m_messageSize = 0;
}

int LES_NetworkAddSendItem(const LES_NetworkSendItem* const pSendItem)
{
	if (s_sendItem.m_messageSize != 0)
	{
		return LES_RETURN_ERROR;
	}
	if (s_sendItem.m_message != LES_NULL)
	{
		return LES_RETURN_ERROR;
	}

	// NEEDS A MUTEX LOCK
	s_sendItem = *pSendItem;
	return LES_RETURN_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

int LES_NetworkCreateTCPSocket(const char* const ip, const short port)
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

