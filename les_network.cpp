#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "les_base.h"
#include "les_logger.h"

void* clientNetworkThread(void* args);

struct NetworkThreadStartStruct
{
	int m_hsock;
};

void JAKE_SocketTest(void)
{
  const int hsock = socket(AF_INET, SOCK_STREAM, 0);
  if (hsock == -1)
	{
      LES_ERROR("Error initializing socket %d\n",errno);
      return;
  }
    
	int option = 1;
        
  if ((setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(int)) == -1) ||
      (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)&option, sizeof(int)) == -1))
	{
    LES_ERROR("Error setting options %d\n",errno);
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
			LES_ERROR("Error connecting socket errno:0x%X\n", errno);
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

	  LES_LOG("Enter some text to send to the server (press enter)\n");
		fgets(buffer, 1024, stdin);
		buffer[strlen(buffer)-1]='\0';
			
		int bytecount;
		bytecount = send(hsock, buffer, strlen(buffer),0);
		if (bytecount == -1)
		{
			LES_ERROR("Error sending data %d\n", errno);
			close(hsock);
			return LES_NULL;
		}
		LES_LOG("Sent bytes %d\n", bytecount);

		buffer[0] = '\0';
		bytecount = recv(hsock, buffer, bufferLen, 0);
		if (bytecount == -1)
		{
			LES_ERROR("Error receiving data %d\n", errno);
			close(hsock);
			return LES_NULL;
		}
		LES_LOG("Received bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
	}

  close(hsock);
	LES_LOG("clientNetworkThread Ended");
	return LES_NULL;
}

