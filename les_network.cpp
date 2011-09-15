#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/select.h>

#if LES_PLATFORM_LINUX == 1
#include <errno.h>
#include <arpa/inet.h>
#endif // #if LES_PLATFORM_LINUX

#if LES_PLATFORM_WINDOWS == 1
#include <winsock.h>
#endif // #if LES_PLATFORM_WINDOWS

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
#define LES_NETWORK_SCOPE_MUTEX LES_ScopeMutex LES_PASTE(__local_mutex__, __COUNTER__)(&s_networkMutexVariable)
#define LES_NETWORK_LOCK_MUTEX s_networkMutex.Lock()
#define LES_NETWORK_UNLOCK_MUTEX s_networkMutex.UnLock()

struct LES_NetworkThreadStartStruct
{
	int m_socketHandle;
};

static LES_MutexVariable s_networkMutexVariable;
static LES_Mutex s_networkMutex(&s_networkMutexVariable);
static LES_ThreadHandle s_networkThreadHandle;
static LES_NetworkThreadStartStruct s_networkThreadStartStruct;

typedef LES_NetworkQueue<LES_NetworkSendItem, LES_NETWORK_SEND_QUEUE_SIZE> LES_NetworkSendQueue;
typedef LES_NetworkQueue<LES_NetworkReceivedItem, LES_NETWORK_RECEIVE_QUEUE_SIZE> LES_NetworkReceivedQueue;

static LES_NetworkSendQueue s_sendItemQueues[2];
static LES_NetworkReceivedQueue s_receivedMessageQueues[2];

static int s_networkThreadQueueIndex;
static LES_NetworkSendQueue* s_pSendItemQueue = LES_NULL;
static LES_NetworkSendQueue* s_pRequestSendItemQueue = LES_NULL;

static LES_NetworkReceivedQueue* s_pReceivedMessageQueue = LES_NULL;
static LES_NetworkReceivedQueue* s_pRequestReceivedMessageQueue = LES_NULL;

#define LES_NETWORK_MAX_NUM_HANDLERS 256
static int s_numRegisteredMessageHandlers = 0;
static LES_uint16 s_receivedMessageHandlerTypes[LES_NETWORK_MAX_NUM_HANDLERS];
static LES_ReceivedMessageHandlerFunction* s_receivedMessageHandlerFunctions[LES_NETWORK_MAX_NUM_HANDLERS];

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_CreateTCPSocket(const char* const ip, const short port)
{
	int err;

#if LES_PLATFORM_WINDOWS == 1
	// Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		// Tell the user that we could not find a usable Winsock DLL
		LES_ERROR("WSAStartup failed with error: %d", err);
		return LES_NETWORK_INVALID_SOCKET;
	}

	// Confirm that the WinSock DLL supports 2.2
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		// Tell the user that we could not find a usable Winsock DLL
		LES_ERROR("Could not find the right version of winsock DLL");
		WSACleanup();
		return LES_NETWORK_INVALID_SOCKET;
	}
#endif // #if LES_PLATFORM_WINDOWS == 1

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
#if LES_PLATFORM_LINUX == 1
		err = errno;
		if (err != EINPROGRESS)
#endif // #if LES_PLATFORM_LINUX == 1
#if LES_PLATFORM_WINDOWS == 1
		err = WSAGetLastError();
		if (err != WSAEINPROGRESS)
#endif // #if LES_PLATFORM_WINDOWS == 1
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
	if (s_pRequestReceivedMessageQueue->Add(&receivedItem) == LES_RETURN_ERROR)
	{
		LES_ERROR("LES_NetworkAddReceivedMessage() failed to add to the queue");
		return LES_RETURN_ERROR;
	}
	//LES_LOG("Added message:'%s'",pReceivedMessage->m_payload);

	return LES_RETURN_OK;
}

#define LES_NETWORK_THREAD_PROCESS_FINISHED (0)
#define LES_NETWORK_THREAD_PROCESS_MORE (1)
#define LES_NETWORK_THREAD_PROCESS_ERROR (-1)

static int LES_NetworkThreadProcessOneLoop(const LES_NetworkThreadStartStruct* const pNetworkThreadStartStruct)
{
	LES_NETWORK_SCOPE_MUTEX;
	const int socketHandle = pNetworkThreadStartStruct->m_socketHandle;
	if (socketHandle < 0)
	{
		return LES_NETWORK_THREAD_PROCESS_FINISHED;
	}

	// Get the sendItem from a queue and make main thread add to the sendItem
	// Swap the queues between main & network thread
	LES_NetworkSendItem* const pSendItem = s_pSendItemQueue->Pop();
	if (pSendItem == LES_NULL)
	{
		return LES_NETWORK_THREAD_PROCESS_FINISHED;
	}
	
	const char* pSendData = (const char*)(pSendItem->GetMessagePtr());
	const int sendDataSize = pSendItem->GetMessageSize();
	if (sendDataSize > 0)
	{
		const int bytesSent = send(socketHandle, pSendData, sendDataSize, 0);
		if (bytesSent == -1)
		{
			LES_ERROR("Error sending data %d", errno);
			return LES_NETWORK_THREAD_PROCESS_ERROR;
		}
		LES_LOG("Sent bytes %d (%d)", bytesSent, sendDataSize);
		pSendItem->Free();
	}

	fd_set readSocketSet;
	FD_ZERO(&readSocketSet);
	FD_SET(socketHandle, &readSocketSet);

	// Wait 0.01 seconds
	timeval timeOut;
	timeOut.tv_sec = 0;
	timeOut.tv_usec = 100 * 1000;

	const int retval = select(socketHandle+1, &readSocketSet, NULL, NULL, &timeOut);
	if (retval == -1)
	{
		LES_ERROR("select() failed");
		return LES_NETWORK_THREAD_PROCESS_ERROR;
	}
	else if (retval)
	{
		if (FD_ISSET(socketHandle, &readSocketSet) == 0)
		{
			LES_ERROR("select() says data ready but FD_ISSET is 0");
			return LES_NETWORK_THREAD_PROCESS_ERROR;
		}
	}
	else
	{
		return LES_NETWORK_THREAD_PROCESS_MORE;
	}

	char buffer[128];
	const int bufferLen = 128;
	buffer[0] = '\0';
	const int bytesReceived = recv(socketHandle, buffer, bufferLen, 0);
	if (bytesReceived == -1)
	{
		LES_ERROR("Error receiving data %d", errno);
		return LES_RETURN_ERROR;
	}
	if (bytesReceived > 0)
	{
		LES_LOG("Received bytes %d", bytesReceived);
		// Make a NetReceiveItem struct and put data into it
		LES_NetworkMessage* const pReceivedMessage = (LES_NetworkMessage* const)malloc(bytesReceived);
		memcpy(pReceivedMessage, buffer, bytesReceived);
		if (LES_NetworkAddReceivedMessage(pReceivedMessage) == LES_RETURN_ERROR)
		{
			LES_ERROR("Error adding received message");
		}
	}
	return LES_NETWORK_THREAD_PROCESS_MORE;
}

static void* LES_NetworkThreadProcess(void* args)
{
	const LES_NetworkThreadStartStruct* const pNetworkThreadStartStruct = (const LES_NetworkThreadStartStruct* const)args;
	LES_LOG("LES_NetworkThreadProcess Started");

	//Now lets do the client related stuff
	while (1)
	{
		const int ret = LES_NetworkThreadProcessOneLoop(pNetworkThreadStartStruct);
		if (ret == LES_NETWORK_THREAD_PROCESS_ERROR)
		{
			break;
		}
		if (ret == LES_NETWORK_THREAD_PROCESS_FINISHED)
		{
			LES_Sleep(0.1f);
			//LES_Sleep(4.1f);
		}
		//LES_Sleep(1.1f);
	}
	const int socketHandle = pNetworkThreadStartStruct->m_socketHandle;
  close(socketHandle);
	LES_LOG("LES_NetworkThreadProcess Ended");
	return LES_NULL;
}

static void LES_NetworkSwapQueues(void)
{
	LES_NETWORK_LOCK_MUTEX;

	const int oldNetworkThreadQueueIndex = s_networkThreadQueueIndex;
	const int networkThreadQueueIndex = oldNetworkThreadQueueIndex ^ 1;
	const int mainThreadQueueIndex = oldNetworkThreadQueueIndex;
	s_networkThreadQueueIndex = networkThreadQueueIndex;

	s_pSendItemQueue = &s_sendItemQueues[networkThreadQueueIndex];
	s_pRequestSendItemQueue = &s_sendItemQueues[mainThreadQueueIndex];

	s_pRequestReceivedMessageQueue = &s_receivedMessageQueues[networkThreadQueueIndex];
	s_pReceivedMessageQueue = &s_receivedMessageQueues[mainThreadQueueIndex];

	LES_NETWORK_UNLOCK_MUTEX;
}

static void LES_NetworkProcessReceivedMessages(void)
{
	const int numRegisteredHandlers = s_numRegisteredMessageHandlers;
	while (1)
	{
		LES_NetworkReceivedItem* const pReceivedItem = s_pReceivedMessageQueue->Pop();
		if (pReceivedItem == LES_NULL)
		{
			break;
		}
		LES_NetworkMessage* const pReceivedMessage = pReceivedItem->GetMessagePtr();

		const LES_uint16 receivedType = fromBigEndian16(pReceivedMessage->m_type);
		LES_ReceivedMessageHandlerFunction* pHandlerFunction = LES_NULL;
		for (int i = 0; i < numRegisteredHandlers; i++)
		{
			if (s_receivedMessageHandlerTypes[i] == receivedType)
			{
				pHandlerFunction = s_receivedMessageHandlerFunctions[i];
				break;
			}
		}
		if (pHandlerFunction)
		{
			const LES_uint16 receivedId = fromBigEndian16(pReceivedMessage->m_id);
			const LES_uint32 receivedPayloadSize = fromBigEndian32(pReceivedMessage->m_payloadSize);
			void* const payload = (void*)pReceivedMessage->m_payload;
			const int ret = pHandlerFunction(receivedType, receivedId, receivedPayloadSize, payload);
			if (ret == LES_RETURN_ERROR)
			{
				LES_ERROR("LES_NetworkProcessReceivedMessages ERROR returned by message handler");
			}
		}
		else
		{
			LES_ERROR("Received message: type:0x%X unhandled", receivedType);
		}

		pReceivedItem->Free();
	}
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

	LES_NETWORK_LOCK_MUTEX;
	s_networkThreadStartStruct.m_socketHandle = socketHandle;
	LES_NETWORK_UNLOCK_MUTEX;

	return LES_RETURN_OK;
}

int LES_NetworkAddSendItem(const LES_NetworkSendItem* const pSendItem)
{
	if (pSendItem->GetMessageSize() == 0)
	{
		LES_ERROR("LES_NetworkAddSendItem() messageSize is 0");
		return LES_RETURN_ERROR;
	}
	if (pSendItem->GetMessagePtr() == LES_NULL)
	{
		LES_ERROR("LES_NetworkAddSendItem() message is NULL");
		return LES_RETURN_ERROR;
	}

	if (s_pRequestSendItemQueue->Add(pSendItem) == LES_RETURN_ERROR)
	{
		LES_ERROR("LES_NetworkAddSendItem() failed to add to the queue");
		return LES_RETURN_ERROR;
	}

	return LES_RETURN_OK;
}

int LES_NetworkRegisterReceivedMessageHandler(const LES_uint16 type, LES_ReceivedMessageHandlerFunction* pFunction)
{
	const int index = s_numRegisteredMessageHandlers;
	if (type == LES_NETWORK_INVALID_MESSAGE_TYPE)
	{
		LES_ERROR("LES_NetworkRegisterReceivedMessageHandler type:%d illegal type", type);
		return LES_RETURN_ERROR;
	}
	if (index >= LES_NETWORK_MAX_NUM_HANDLERS)
	{
		LES_ERROR("LES_NetworkRegisterReceivedMessageHandler type:%d no free space for any more handlers MAX:%d", 
							type, LES_NETWORK_MAX_NUM_HANDLERS);
		return LES_RETURN_ERROR;
	}
	for (int i = 0; i < index; i++)
	{
		if (s_receivedMessageHandlerTypes[index] == type)
		{
			LES_ERROR("LES_NetworkRegisterReceivedMessageHandler type:%d already registered", type);
			return LES_RETURN_ERROR;
		}
	}
	if (s_receivedMessageHandlerTypes[index] != LES_NETWORK_INVALID_MESSAGE_TYPE)
	{
		LES_ERROR("LES_NetworkRegisterReceivedMessageHandler type:%d ERROR handler type entry not empty:%d", 
							type, s_receivedMessageHandlerTypes[index]);
		return LES_RETURN_ERROR;
	}
	if (s_receivedMessageHandlerFunctions[index] != LES_NULL)
	{
		LES_ERROR("LES_NetworkRegisterReceivedMessageHandler type:%d ERROR handler function entry not empty:%p", 
							type, s_receivedMessageHandlerFunctions[index]);
		return LES_RETURN_ERROR;
	}

	s_receivedMessageHandlerTypes[index] = type;
	s_receivedMessageHandlerFunctions[index] = pFunction;
	s_numRegisteredMessageHandlers++;

	return LES_RETURN_OK;
}

void LES_NetworkInit(void)
{
	LES_MutexVariableInit(&s_networkMutexVariable);
	s_networkThreadQueueIndex = 0;
	LES_NetworkSwapQueues();

	LES_NETWORK_LOCK_MUTEX;
	s_networkThreadStartStruct.m_socketHandle = -1;
	LES_NETWORK_UNLOCK_MUTEX;

	for (int i = 0; i < LES_NETWORK_MAX_NUM_HANDLERS; i++)
	{
		s_receivedMessageHandlerTypes[i] = LES_NETWORK_INVALID_MESSAGE_TYPE;
		s_receivedMessageHandlerFunctions[i] = LES_NULL;
	}
	s_numRegisteredMessageHandlers = 0;

	const int ret = LES_CreateThread(&s_networkThreadHandle, LES_NULL, LES_NetworkThreadProcess, &s_networkThreadStartStruct);
	LES_LOG("Network thread created handle:%d ret:%d", s_networkThreadHandle, ret);
}

void LES_NetworkTick(void)
{
	// s_pRequestSendItemQueue - can have data in it (main thread send requests -> network thread)
	// s_pRequestReceivedMessageQueue - can have data in it (network thread received requests -> main thread)

	// s_pSendItemQueue must be empty - otherwise network thread still processing messages to be sent
	// Loop without mutex lock until it is empty
	while (1)
	{
		LES_NETWORK_LOCK_MUTEX;
		const int numItems = s_pSendItemQueue->GetNumItems();
		LES_NETWORK_UNLOCK_MUTEX;

		if (numItems == 0)
		{
			break;
		}
		LES_Sleep(0.001f);
	}

	// s_pReceivedMessageQueue must be empty - otherwise main thread still processing received messages
	// Drain the received message queue
	LES_NetworkProcessReceivedMessages();

	// Swap the send queues and the receive queues between the network & main thread versions
	LES_NetworkSwapQueues();

	LES_NetworkProcessReceivedMessages();
}

void LES_NetworkShutdown(void)
{
	// Stop the network thread
	// Close the socket
	LES_LOG("TODO:STOP THE NETWORK THREAD");
	LES_LOG("TODO:CLOSE THE SOCKET");
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

