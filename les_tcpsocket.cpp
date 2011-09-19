#include <memory.h>
#include <unistd.h>

#if LES_PLATFORM_LINUX == 1
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>
#endif // #if LES_PLATFORM_LINUX

#if LES_PLATFORM_WINDOWS == 1
#include <winsock.h>
#endif // #if LES_PLATFORM_WINDOWS

#include "les_base.h"
#include "les_tcpsocket.h"
#include "les_logger.h"

#define LES_NETWORK_INVALID_SOCKET (-1)

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

int LES_GetLastError(void)
{
#if LES_PLATFORM_LINUX == 1
	return errno;
#endif // #if LES_PLATFORM_LINUX == 1

#if LES_PLATFORM_WINDOWS == 1
	return WSAGetLastError();
#endif // #if LES_PLATFORM_WINDOWS == 1
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public external functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

LES_TCPSocket::LES_TCPSocket(void)
{
	m_socketHandle = LES_NETWORK_INVALID_SOCKET;
	m_address[0] = '\0';
	m_port = -1;
}

LES_TCPSocket::~LES_TCPSocket(void)
{
	Close();
	m_socketHandle = LES_NETWORK_INVALID_SOCKET;
	m_address[0] = '\0';
	m_port = -1;
}

int LES_TCPSocket::Create(void)
{
	if (m_socketHandle != LES_NETWORK_INVALID_SOCKET)
	{
		LES_ERROR("LES_TCPSocket::Create Error socket already created:0x%X", m_socketHandle);
		return LES_RETURN_ERROR;
	}

	const int socketHandle = socket(AF_INET, SOCK_STREAM, 0);
	if (socketHandle == -1)
	{
		LES_ERROR("LES_TCPSocket::Create Error initializing socket errno:0x%X", LES_GetLastError());
		return LES_RETURN_ERROR;
	}

	int option = 1;

	if ((setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(int)) == -1) ||
			(setsockopt(socketHandle, SOL_SOCKET, SO_KEEPALIVE, (char*)&option, sizeof(int)) == -1))
	{
		LES_ERROR("LES_TCPSocket::Create Error setting options errno:0x%X", LES_GetLastError());
		close(socketHandle);
		return LES_RETURN_ERROR;
	}
	m_socketHandle = socketHandle;
	return LES_RETURN_OK;
}

int LES_TCPSocket::Connect(const char* const ip, const short port)
{
	int err;

	const short hostPort = port;
	const char* const hostIP = ip;

	struct sockaddr_in hostAddr;
	hostAddr.sin_family = AF_INET ;
	hostAddr.sin_port = htons(hostPort);

	memset(&(hostAddr.sin_zero), 0, 8);
	hostAddr.sin_addr.s_addr = inet_addr(hostIP);

	if (connect(m_socketHandle, (struct sockaddr*)&hostAddr, sizeof(hostAddr)) == -1)
	{
		err =  LES_GetLastError();
#if LES_PLATFORM_LINUX == 1
		if (err != EINPROGRESS)
#endif // #if LES_PLATFORM_LINUX == 1
#if LES_PLATFORM_WINDOWS == 1
		if (err != WSAEINPROGRESS)
#endif // #if LES_PLATFORM_WINDOWS == 1
		{
			LES_ERROR("LES_CreateTCPSocket::Error connecting socket errno:0x%X",  LES_GetLastError());
			Close();
			return LES_RETURN_ERROR;
		}
	}
	return LES_RETURN_OK;
}

int LES_TCPSocket::InitSystem(void)
{
#if LES_PLATFORM_WINDOWS == 1
	int err;
	// Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		// Tell the user that we could not find a usable Winsock DLL
		LES_ERROR("WSAStartup failed with error: %d", err);
		return LES_RETURN_ERROR;
	}

	// Confirm that the WinSock DLL supports 2.2
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		// Tell the user that we could not find a usable Winsock DLL
		LES_ERROR("Could not find the right version of winsock DLL");
		WSACleanup();
		return LES_RETURN_ERROR;
	}
#endif // #if LES_PLATFORM_WINDOWS == 1
	return LES_RETURN_OK;
}

int LES_TCPSocket::IsValid(void) const
{
	if (m_socketHandle == LES_NETWORK_INVALID_SOCKET)
	{
		return LES_RETURN_ERROR;
	}
	return LES_RETURN_OK;
}

int LES_TCPSocket::Send(const char* const pSendData, const int sendDataSize)
{
	const int socketHandle = m_socketHandle;
	const int bytesSent = send(socketHandle, pSendData, sendDataSize, 0);
	if (bytesSent == -1)
	{
		LES_ERROR("LES_TCPSocket::Send Error sending data errno:0x%X",  LES_GetLastError());
		return -1;
	}
	return bytesSent;
}

int LES_TCPSocket::Recv(char* const pReceiveBuffer, const int bufferSize, int* const pNumBytesReceived)
{
	const int socketHandle = m_socketHandle;
	*pNumBytesReceived = 0;

	fd_set readSocketSet;
	FD_ZERO(&readSocketSet);
	FD_SET(socketHandle, &readSocketSet);

	// Wait 0.01 seconds
	timeval timeOut;
	timeOut.tv_sec = 0;
	timeOut.tv_usec = 10 * 1000;

	const int retval = select(socketHandle+1, &readSocketSet, NULL, NULL, &timeOut);
	if (retval == -1)
	{
		LES_ERROR("LES_TCPSocket::Recv select() failed errno:0x%X",  LES_GetLastError());
		return LES_NETWORK_RECEIVE_ERROR;
	}
	else if (retval)
	{
		if (FD_ISSET(socketHandle, &readSocketSet) == 0)
		{
			LES_ERROR("LES_TCPSocket::Recv select() says data ready but FD_ISSET is 0");
			return LES_NETWORK_RECEIVE_ERROR;
		}
	}
	else
	{
		return LES_NETWORK_RECEIVE_NO_DATA;
	}
	const int bytesReceived = recv(socketHandle, pReceiveBuffer, bufferSize, 0);
	if (bytesReceived == -1)
	{
		LES_ERROR("LES_TCPSocket::Recv Error receiving data errno:0x%X",  LES_GetLastError());
		return LES_NETWORK_RECEIVE_ERROR;
	}
	*pNumBytesReceived = bytesReceived;

	return LES_NETWORK_RECEIVE_OK;
}

void LES_TCPSocket::Close(void)
{
	close(m_socketHandle);
	m_socketHandle = LES_NETWORK_INVALID_SOCKET;
	m_address[0] = '\0';
	m_port = -1;
}
