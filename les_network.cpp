#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "les_base.h"
#include "les_network.h"
#include "les_logger.h"
#include "les_time.h"
#include "les_thread.h"

#define LES_NETWORK_INVALID_SOCKET (-1)
#define LES_NETWORK_SEND_QUEUE_SIZE (128)
#define LES_NETWORK_RECEIVE_QUEUE_SIZE (32)

#define __LES_PASTE(a,b) a ## b
#define LES_PASTE(a,b) __LES_PASTE(a,b)
#define LES_NETWORK_GET_MUTEX LES_Mutex LES_PASTE(__local_mutex__, __COUNTER__)(&s_networkMutex)

static int s_networkMutex = 0;

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

template <class T, int SIZE> class LES_NetworkQueue
{
public:
	LES_NetworkQueue(void);
	~LES_NetworkQueue(void);
	int Add(const T* const pSendItem);
	T* Pop(void);

private:
	LES_NetworkQueue(const LES_NetworkQueue& other);
	LES_NetworkQueue& operator =(const LES_NetworkQueue& other);

	int m_numItems;
	int m_headIndex;
	int m_tailIndex;
	T m_items[SIZE];
};

class LES_Mutex
{
public:
	LES_Mutex(int* const pMutexVariable) : m_pMutexVariable(pMutexVariable)
	{
		Lock();
	}
	~LES_Mutex(void)
	{
		UnLock();
	}
private:
	LES_Mutex(void);
	LES_Mutex(LES_Mutex& other);
	LES_Mutex& operator = (LES_Mutex& other);

	void Lock(void);
	void UnLock(void);

	int* const m_pMutexVariable;
};

template <class T, int SIZE> LES_NetworkQueue<T, SIZE>::LES_NetworkQueue(void)
{
	m_numItems = 0;
	m_headIndex = 0;
	m_tailIndex = 0;
}

template <class T, int SIZE> LES_NetworkQueue<T, SIZE>::~LES_NetworkQueue(void)
{
	m_numItems = 0;
	m_headIndex = 0;
	m_tailIndex = 0;
}

template <class T, int SIZE> int LES_NetworkQueue<T, SIZE>::Add(const T* const pItem)
{
	T* const pInsertPlace = &m_items[m_tailIndex];

	if (m_numItems >= SIZE)
	{
		// Queue full
		LES_ERROR("LES_NetworkQueue::Add() queue full");
		return LES_RETURN_ERROR;
	}
	if ((m_tailIndex == m_headIndex) && (m_numItems != 0))
	{
		//Queue wrap around
		LES_ERROR("LES_NetworkQueue::Add() queue wrap around");
		return LES_RETURN_ERROR;
	}
	*pInsertPlace = *pItem;

	if ((m_numItems == 0) && (m_headIndex != m_tailIndex))
	{
		LES_ERROR("LES_NetworkQueue::Add() numItems == 0 but head != tail %d != %d", m_headIndex, m_tailIndex);
		return LES_RETURN_ERROR;
	}

	m_tailIndex++;
	m_tailIndex = m_tailIndex % SIZE;
	m_numItems++;
	//LES_LOG("Add: head %d tail %d num %d", m_headIndex, m_tailIndex, m_numItems);

	return LES_RETURN_OK;
}

template <class T, int SIZE> T* LES_NetworkQueue<T, SIZE>::Pop(void)
{
	if ((m_numItems == 0) && (m_headIndex != m_tailIndex))
	{
		LES_ERROR("LES_NetworkQueue::Pop() numItems == 0 but head != tail %d:%d", m_headIndex, m_tailIndex);
		return LES_NULL;
	}
	if (m_numItems == 0)
	{
		return LES_NULL;
	}
	T* const pItem = &m_items[m_headIndex];
	m_numItems--;
	m_headIndex++;
	m_headIndex = m_headIndex % SIZE;
	if (m_numItems == 0)
	{
		if (m_headIndex != m_tailIndex)
		{
			LES_ERROR("LES_NetworkQueue::Pop() numItems == 0 but head != tail %d:%d", m_headIndex, m_tailIndex);
			return LES_NULL;
		}
	}

	//LES_LOG("Pop: head %d tail %d num %d pItem:%p", m_headIndex, m_tailIndex, m_numItems, pItem);
	return pItem;
}

void LES_Mutex::Lock(void)
{
	volatile int* const pMutexVariable = m_pMutexVariable;
	while (*pMutexVariable != 0)
	{
		LES_Sleep(1.0e-3f);
	};
	*pMutexVariable = 1;
}

void LES_Mutex::UnLock(void)
{
	volatile int* const pMutexVariable = m_pMutexVariable;
	if (*pMutexVariable != 1)
	{
		LES_ERROR("Unlock called but mutex not locked %d", *pMutexVariable);
	}
	*m_pMutexVariable = 0;
}

static LES_NetworkQueue<LES_NetworkSendItem, LES_NETWORK_SEND_QUEUE_SIZE> s_sendItemQueue;
static LES_NetworkQueue<LES_NetworkReceivedItem, LES_NETWORK_RECEIVE_QUEUE_SIZE> s_receiveMessageQueue;

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

static int LES_NetworkAddReceivedMessage(LES_NetworkMessage* const pReceivedMessage)
{
	LES_NETWORK_GET_MUTEX;
	if (pReceivedMessage == LES_NULL)
	{
		LES_ERROR("LES_NetworkAddReceivedMessage() message is NULL");
		return LES_RETURN_ERROR;
	}
	LES_NetworkReceivedItem receivedItem(pReceivedMessage);
	if (s_receiveMessageQueue.Add(&receivedItem) == LES_RETURN_ERROR)
	{
		LES_ERROR("LES_NetworkAddReceivedMessage() failed to add to the queue");
		return LES_RETURN_ERROR;
	}
	LES_LOG("Added message:'%s'",pReceivedMessage->m_payload);

	return LES_RETURN_OK;
}

static void* clientNetworkThread(void* args)
{
	const LES_NetworkThreadStartStruct* const networkThreadStartStruct = (const LES_NetworkThreadStartStruct* const)args;
	LES_LOG("clientNetworkThread Started");

	const int socketHandle = networkThreadStartStruct->m_socketHandle;

	//Now lets do the client related stuff
	while (1)
	{
		// Get the sendItem from a queue and make main thread add to the sendItem
		// Swap the queues between main & network thread
		LES_NetworkSendItem* const pSendItem = s_sendItemQueue.Pop();
		if (pSendItem == LES_NULL)
		{
			LES_Sleep(0.1f);
			LES_Sleep(4.1f);
			continue;
		}
		void* pSendData = (void*)(pSendItem->GetMessage());
		const int sendDataSize = pSendItem->GetMessageSize();
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
		LES_LOG("Received bytes %d", bytesReceived);
		// Make a NetReceiveItem struct and put data into it
		LES_NetworkMessage* const pReceivedMessage = (LES_NetworkMessage* const)malloc(bytesReceived);
		memcpy(pReceivedMessage, buffer, bytesReceived);
		if (LES_NetworkAddReceivedMessage(pReceivedMessage) == LES_RETURN_ERROR)
		{
				LES_ERROR("Error adding received message");
		}
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

void LES_NetworkReceivedItem::Free(void)
{
	free(m_message);
	m_message = LES_NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public external functions
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

	static LES_ThreadHandle networkThreadHandle;
	int ret = LES_CreateThread(&networkThreadHandle, LES_NULL, clientNetworkThread, &networkThreadStartStruct);

	LES_LOG("Network thread created handle:%d ret:%d", networkThreadHandle, ret);
	return LES_RETURN_OK;
}

int LES_NetworkAddSendItem(const LES_NetworkSendItem* const pSendItem)
{
	if (pSendItem->GetMessageSize() == 0)
	{
		LES_ERROR("LES_NetworkAddSendItem() messageSize is 0");
		return LES_RETURN_ERROR;
	}
	if (pSendItem->GetMessage() == LES_NULL)
	{
		LES_ERROR("LES_NetworkAddSendItem() message is NULL");
		return LES_RETURN_ERROR;
	}

	if (s_sendItemQueue.Add(pSendItem) == LES_RETURN_ERROR)
	{
		LES_ERROR("LES_NetworkAddSendItem() failed to add to the queue");
		return LES_RETURN_ERROR;
	}

	return LES_RETURN_OK;
}

void LES_NetworkTick(void)
{
	LES_NETWORK_GET_MUTEX;
	while (1)
	{
		LES_NetworkReceivedItem* const pItem = s_receiveMessageQueue.Pop();
		if (pItem == LES_NULL)
		{
			break;
		}
		LES_NetworkMessage* const pReceivedMessage = pItem->GetMessage();

		short receivedType = fromBigEndian16(pReceivedMessage->m_type);
		short receivedId = fromBigEndian16(pReceivedMessage->m_id);
		int receivedPayloadSize = fromBigEndian32(pReceivedMessage->m_payloadSize);

		LES_LOG("Received message: type:0x%X id:%d payloadSize:%d", receivedType, receivedId, receivedPayloadSize);
		LES_LOG("payload:'%s'", (char*)pReceivedMessage->m_payload);

		pItem->Free();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

