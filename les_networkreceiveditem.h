#ifndef LES_NETWORKRECEIVEITEM_HH
#define LES_NETWORKRECEIVEITEM_HH

struct LES_NetworkMessage;

class LES_NetworkReceivedItem
{
public:
	LES_NetworkReceivedItem()
	{
		m_message = LES_NULL;
	}
	LES_NetworkReceivedItem(LES_NetworkMessage* const pMessage)
	{
		m_message = pMessage;
	}
	~LES_NetworkReceivedItem()
	{
		m_message = LES_NULL;
	}

	LES_NetworkReceivedItem& operator = (const LES_NetworkReceivedItem& other)
	{
		m_message = other.m_message;

		return *this;
	}

	LES_NetworkMessage* GetMessagePtr(void)
	{
		return m_message;
	}

	void Free(void);

private:
	LES_NetworkReceivedItem(const LES_NetworkReceivedItem& other);

	LES_NetworkMessage* m_message;
};

#endif // #ifndef LES_NETWORKRECEIVEITEM_HH
