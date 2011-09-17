#ifndef LES_TCPSOCKET_HH
#define LES_TCPSOCKET_HH

#define LES_NETWORK_RECEIVE_ERROR (-2)
#define LES_NETWORK_RECEIVE_NO_DATA (-1)
#define LES_NETWORK_RECEIVE_OK (0)

class LES_TCPSocket
{
public:
	LES_TCPSocket(void);
	~LES_TCPSocket(void);
	int Create(void);
	int Connect(const char* const ip, const short port);
	int Send(const void* const pSendData, const int sendDataSize);
	int Recv(void* const pReceiveBuffer, const int bufferSize, int* const pNumBytesReceived);
	void Close(void);

	int IsValid(void) const;

	static int InitSystem(void);
private:
	LES_TCPSocket& operator =(LES_TCPSocket& other);
	LES_TCPSocket(LES_TCPSocket& other);

	int m_socketHandle;
	char m_address[128];
	short m_port;
};

#endif // #ifndef LES_TCPSOCKET_HH
