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
#include "les_mutex.h"
#include "les_networkqueue.h"
#include "les_networkmessage.h"

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

static LES_NetworkQueue<LES_NetworkSendItem, LES_NETWORK_SEND_QUEUE_SIZE> s_sendItemQueues[2];
static LES_NetworkQueue<LES_NetworkReceivedItem, LES_NETWORK_RECEIVE_QUEUE_SIZE> s_receivedMessageQueues[2];

static int s_networkThreadQueueIndex;
static LES_NetworkQueue<LES_NetworkSendItem, LES_NETWORK_SEND_QUEUE_SIZE>* s_pSendItemQueue = LES_NULL;
static LES_NetworkQueue<LES_NetworkSendItem, LES_NETWORK_SEND_QUEUE_SIZE>* s_pRequestSendItemQueue = LES_NULL;

static LES_NetworkQueue<LES_NetworkReceivedItem, LES_NETWORK_RECEIVE_QUEUE_SIZE>* s_pReceivedMessageQueue = LES_NULL;
static LES_NetworkQueue<LES_NetworkReceivedItem, LES_NETWORK_RECEIVE_QUEUE_SIZE>* s_pRequestReceivedMessageQueue = LES_NULL;

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
	if (pReceivedMessage == LES_NULL)
	{
		LES_ERROR("LES_NetworkAddReceivedMessage() message is NULL");
		return LES_RETURN_ERROR;
	}
	LES_NetworkReceivedItem receivedItem(pReceivedMessage);
	if (s_pReceivedMessageQueue->Add(&receivedItem) == LES_RETURN_ERROR)
	{
		LES_ERROR("LES_NetworkAddReceivedMessage() failed to add to the queue");
		return LES_RETURN_ERROR;
	}
	LES_LOG("Added message:'%s'",pReceivedMessage->m_payload);

	return LES_RETURN_OK;
}

static void* LES_NetworkThreadProcess(void* args)
{
	const LES_NetworkThreadStartStruct* const networkThreadStartStruct = (const LES_NetworkThreadStartStruct* const)args;
	LES_LOG("LES_NetworkThreadProcess Started");

	const int socketHandle = networkThreadStartStruct->m_socketHandle;

	//Now lets do the client related stuff
	while (1)
	{
		// Get the sendItem from a queue and make main thread add to the sendItem
		// Swap the queues between main & network thread
		LES_NetworkSendItem* const pSendItem = s_pSendItemQueue->Pop();
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
	LES_LOG("LES_NetworkThreadProcess Ended");
	return LES_NULL;
}

static void LES_NetworkSwapQueues(void)
{
	LES_NETWORK_GET_MUTEX;
	const int oldNetworkThreadQueueIndex = s_networkThreadQueueIndex;
	const int networkThreadQueueIndex = oldNetworkThreadQueueIndex ^ 1;
	const int mainThreadQueueIndex = oldNetworkThreadQueueIndex;
	s_networkThreadQueueIndex = networkThreadQueueIndex;

	s_pSendItemQueue = &s_sendItemQueues[networkThreadQueueIndex];
	s_pRequestSendItemQueue = &s_sendItemQueues[mainThreadQueueIndex];

	s_pRequestReceivedMessageQueue = &s_receivedMessageQueues[networkThreadQueueIndex];
	s_pReceivedMessageQueue = &s_receivedMessageQueues[mainThreadQueueIndex];
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
	int ret = LES_CreateThread(&networkThreadHandle, LES_NULL, LES_NetworkThreadProcess, &networkThreadStartStruct);

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

	if (s_pSendItemQueue->Add(pSendItem) == LES_RETURN_ERROR)
	{
		LES_ERROR("LES_NetworkAddSendItem() failed to add to the queue");
		return LES_RETURN_ERROR;
	}

	return LES_RETURN_OK;
}

void LES_NetworkInit(void)
{
	s_networkThreadQueueIndex = 0;
	LES_NetworkSwapQueues();
}

void LES_NetworkTick(void)
{
	// Swap the send and receive queues over
	LES_NetworkSwapQueues();

	while (1)
	{
		LES_NetworkReceivedItem* const pReceivedItem = s_pReceivedMessageQueue->Pop();
		if (pReceivedItem == LES_NULL)
		{
			break;
		}
		LES_NetworkMessage* const pReceivedMessage = pReceivedItem->GetMessage();

		short receivedType = fromBigEndian16(pReceivedMessage->m_type);
		short receivedId = fromBigEndian16(pReceivedMessage->m_id);
		int receivedPayloadSize = fromBigEndian32(pReceivedMessage->m_payloadSize);

		LES_LOG("Received message: type:0x%X id:%d payloadSize:%d", receivedType, receivedId, receivedPayloadSize);
		LES_LOG("payload:'%s'", (char*)pReceivedMessage->m_payload);

		pReceivedItem->Free();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

