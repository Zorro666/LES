#ifndef LES_NETWORK_HH
#define LES_NETWORK_HH

#include "les_base.h"

struct LES_NetworkMessage;

struct LES_NetworkSendItem
{
	LES_NetworkSendItem()
	{
		m_message = LES_NULL;
		m_messageSize = 0;
	}

	void Create(const short type, const short id, const int payLoadSize, void* const payload);
	void Free(void);

	LES_NetworkMessage* m_message;
	int m_messageSize;
};

int LES_NetworkCreateTCPSocket(const char* const ip, const short port);
int LES_NetworkAddSendItem(const LES_NetworkSendItem* const pSendItem);
void LES_NetworkTick(void);

#endif // #ifndef LES_NETWORK_HH
