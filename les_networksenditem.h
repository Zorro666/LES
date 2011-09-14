#ifndef LES_NETWORKSENDITEM_HH
#define LES_NETWORKSENDITEM_HH

#include "les_base.h"

struct LES_NetworkMessage;

class LES_NetworkSendItem
{
public:
	LES_NetworkSendItem()
	{
		m_message = LES_NULL;
		m_messageSize = 0;
	}
	~LES_NetworkSendItem()
	{
		m_message = LES_NULL;
		m_messageSize = 0;
	}

	LES_NetworkMessage* GetMessagePtr(void) const
	{
		return m_message;
	}
	int GetMessageSize(void) const
	{
		return m_messageSize;
	}
	LES_NetworkSendItem& operator = (const LES_NetworkSendItem& other)
	{
		m_message = other.m_message;
		m_messageSize = other.m_messageSize;

		return *this;
	}

	void Create(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payLoadSize, void* const payload);
	void Free(void);

private:
	LES_NetworkSendItem(const LES_NetworkSendItem& other);

	LES_NetworkMessage* m_message;
	int m_messageSize;
};

#endif // #ifndef LES_NETWORKSENDITEM_HH
