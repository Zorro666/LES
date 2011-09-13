#ifndef LES_NETWORKMESSAGE_HH
#define LES_NETWORKMESSAGE_HH

#include "les_base.h"

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

#endif //#ifndef LES_NETWORKMESSAGE_HH

