#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>

#include "les_base.h"
#include "les_network.h"
#include "les_logger.h"
#include "les_time.h"

#define LES_NETWORK_INVALID_SOCKET (-1)
#define LES_NETWORK_SEND_QUEUE_SIZE (128)

struct LES_NetworkThreadStartStruct
{
	int m_socketHandle;
};

struct LES_NetworkMessage
{
public:
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

class LES_NetworkSendQueue
{
public:
	LES_NetworkSendQueue(void);
	~LES_NetworkSendQueue(void);
	int Add(const LES_NetworkSendItem* const pSendItem);
	LES_NetworkSendItem* Pop(void);

private:
	LES_NetworkSendQueue(const LES_NetworkSendQueue& other);
	LES_NetworkSendQueue& operator =(const LES_NetworkSendQueue& other);

	int m_numItems;
	int m_headIndex;
	int m_tailIndex;
	LES_NetworkSendItem m_items[LES_NETWORK_SEND_QUEUE_SIZE];
};

LES_NetworkSendQueue::LES_NetworkSendQueue(void)
{
	m_numItems = 0;
	m_headIndex = -1;
	m_tailIndex = 0;
}

LES_NetworkSendQueue::~LES_NetworkSendQueue(void)
{
	m_numItems = 0;
	m_headIndex = -1;
	m_tailIndex = 0;
}

int LES_NetworkSendQueue::Add(const LES_NetworkSendItem* const pSendItem)
{
	LES_NetworkSendItem* const pInsertPlace = &m_items[m_tailIndex];

	if (m_numItems >= LES_NETWORK_SEND_QUEUE_SIZE)
	{
		// Queue full
		LES_ERROR("LES_NetworkSendQueue::Add() queue full");
		return LES_RETURN_ERROR;
	}
	if (m_tailIndex == m_headIndex)
	{
		//Queue wrap around
		LES_ERROR("LES_NetworkSendQueue::Add() queue wrap around");
		return LES_RETURN_ERROR;
	}
	*pInsertPlace = *pSendItem;

	if ((m_headIndex == -1) && (m_numItems != 0))
	{
		LES_ERROR("LES_NetworkSendQueue::Add() head is -1 but numItems != 0 %d", m_numItems);
		return LES_RETURN_ERROR;
	}
	if ((m_numItems == 0) && (m_headIndex != -1))
	{
		LES_ERROR("LES_NetworkSendQueue::Add() numItems == 0 but head is not -1 %d", m_headIndex);
		return LES_RETURN_ERROR;
	}

	if (m_headIndex == -1)
	{
		m_headIndex = m_tailIndex;
		m_numItems = 0;
	}
	m_tailIndex++;
	m_tailIndex = m_tailIndex % LES_NETWORK_SEND_QUEUE_SIZE;
	m_numItems++;

	return LES_RETURN_OK;
}

LES_NetworkSendItem* LES_NetworkSendQueue::Pop(void)
{
	if ((m_headIndex == -1) && (m_numItems != 0))
	{
		LES_ERROR("LES_NetworkSendQueue::Pop() head is -1 but numItems != 0 %d", m_numItems);
		return LES_NULL;
	}
	if ((m_numItems == 0) && (m_headIndex != -1))
	{
		LES_ERROR("LES_NetworkSendQueue::Pop() numItems == 0 but head is -1 %d", m_headIndex);
		return LES_NULL;
	}
	if (m_numItems == 0)
	{
		return LES_NULL;
	}
	LES_NetworkSendItem* const pItem = &m_items[m_headIndex];
	m_numItems--;
	m_headIndex++;
	m_headIndex = m_headIndex % LES_NETWORK_SEND_QUEUE_SIZE;
	if (m_numItems == 0)
	{
		m_headIndex = -1;
	}

	return pItem;
}

static LES_NetworkSendQueue s_sendItemQueue;

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
		// Swap the queues between main & network thread
		LES_NetworkSendItem* const pSendItem = s_sendItemQueue.Pop();
		if (pSendItem == LES_NULL)
		{
			LES_Sleep(0.1f);
			LES_Sleep(10.0f);
			continue;
		}
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
	if (pSendItem->m_messageSize == 0)
	{
		LES_ERROR("LES_NetworkAddSendItem() messageSize is 0");
		return LES_RETURN_ERROR;
	}
	if (pSendItem->m_message == LES_NULL)
	{
		LES_ERROR("LES_NetworkAddSendItem() message is NULL");
		return LES_RETURN_ERROR;
	}

	// NEEDS A MUTEX LOCK
	if (s_sendItemQueue.Add(pSendItem) == LES_RETURN_ERROR)
	{
		LES_ERROR("LES_NetworkAddSendItem() failed to add to the queue");
		return LES_RETURN_ERROR;
	}

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

