#ifndef LES_NETWORKMESSAGE_HH
#define LES_NETWORKMESSAGE_HH

#include "les_base.h"

#define LES_NETWORK_INVALID_MESSAGE_TYPE (0xFFFF)

struct LES_NetworkMessage
{
public:
	LES_uint16 m_type;
	LES_uint16 m_id;
	LES_uint32 m_payloadSize;
	int m_payload[1];		// m_payload[m_payloadSize/4]
private:
	LES_NetworkMessage();
	LES_NetworkMessage(const LES_NetworkMessage& other);
	LES_NetworkMessage& operator =(const LES_NetworkMessage& other);
	~LES_NetworkMessage();
};

#endif //#ifndef LES_NETWORKMESSAGE_HH

