#include <malloc.h>
#include <string.h>

#include "les_base.h"
#include "les_networksenditem.h"
#include "les_networkmessage.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_NetworkMessageSize(const int payloadSize)
{
	int payloadExtraMemmory = payloadSize - sizeof(int);
	if (payloadExtraMemmory < 0)
	{
		payloadExtraMemmory = 0;
	}
	payloadExtraMemmory = (payloadExtraMemmory + 3) & ~3;
	const int memorySize = sizeof(LES_NetworkMessage) + payloadExtraMemmory;
	return memorySize;
}

static LES_NetworkMessage* LES_CreateNetworkMessage(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payloadSize, 
																										const int messageSize)
{
	LES_NetworkMessage* const pNetworkMessage = (LES_NetworkMessage*)malloc(messageSize);
	pNetworkMessage->m_type = toBigEndian16(type);
	pNetworkMessage->m_id = toBigEndian16(id);
	pNetworkMessage->m_payloadSize = toBigEndian32(payloadSize);

	return pNetworkMessage;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public external functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_NetworkSendItem::Create(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payloadSize)
{
	const int messageSize = LES_NetworkMessageSize(payloadSize);
	m_messageSize = messageSize;
	m_message = LES_CreateNetworkMessage(type, id, payloadSize, messageSize);
}

void LES_NetworkSendItem::Create(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payloadSize, void* const payload)
{
	Create(type, id, payloadSize);
	memcpy(m_message->m_payload, payload, payloadSize);
}

void LES_NetworkSendItem::Free(void)
{
	free(m_message);
	m_message = LES_NULL;
	m_messageSize = 0;
}

