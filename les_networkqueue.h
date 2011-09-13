#ifndef LES_NETWORKQUEUE_HH
#define LES_NETWORKQUEUE_HH

#include "les_base.h"
#include "les_logger.h"

template <class T, int SIZE> class LES_NetworkQueue
{
public:
	LES_NetworkQueue(void);
	~LES_NetworkQueue(void);
	int Add(const T* const pSendItem);
	T* Pop(void);
	int GetNumItems(void) const
	{
		return m_numItems;
	}

private:
	LES_NetworkQueue(const LES_NetworkQueue& other);
	LES_NetworkQueue& operator =(const LES_NetworkQueue& other);

	int m_numItems;
	int m_headIndex;
	int m_tailIndex;
	T m_items[SIZE];
};

template <class T, int SIZE> LES_NetworkQueue<T, SIZE>::LES_NetworkQueue(void)
{
	m_numItems = 0;
	m_headIndex = 0;
	m_tailIndex = 0;
}

template <class T, int SIZE> LES_NetworkQueue<T, SIZE>::~LES_NetworkQueue(void)
{
	m_numItems = 0;
	m_headIndex = 0;
	m_tailIndex = 0;
}

template <class T, int SIZE> int LES_NetworkQueue<T, SIZE>::Add(const T* const pItem)
{
	T* const pInsertPlace = &m_items[m_tailIndex];

	if (m_numItems >= SIZE)
	{
		// Queue full
		LES_ERROR("LES_NetworkQueue::Add() queue full");
		return LES_RETURN_ERROR;
	}
	if ((m_tailIndex == m_headIndex) && (m_numItems != 0))
	{
		//Queue wrap around
		LES_ERROR("LES_NetworkQueue::Add() queue wrap around");
		return LES_RETURN_ERROR;
	}
	*pInsertPlace = *pItem;

	if ((m_numItems == 0) && (m_headIndex != m_tailIndex))
	{
		LES_ERROR("LES_NetworkQueue::Add() numItems == 0 but head != tail %d != %d", m_headIndex, m_tailIndex);
		return LES_RETURN_ERROR;
	}

	m_tailIndex++;
	m_tailIndex = m_tailIndex % SIZE;
	m_numItems++;
	//LES_LOG("Add: head %d tail %d num %d", m_headIndex, m_tailIndex, m_numItems);

	return LES_RETURN_OK;
}

template <class T, int SIZE> T* LES_NetworkQueue<T, SIZE>::Pop(void)
{
	if ((m_numItems == 0) && (m_headIndex != m_tailIndex))
	{
		LES_ERROR("LES_NetworkQueue::Pop() numItems == 0 but head != tail %d:%d", m_headIndex, m_tailIndex);
		return LES_NULL;
	}
	if (m_numItems == 0)
	{
		return LES_NULL;
	}
	T* const pItem = &m_items[m_headIndex];
	m_numItems--;
	m_headIndex++;
	m_headIndex = m_headIndex % SIZE;
	if (m_numItems == 0)
	{
		if (m_headIndex != m_tailIndex)
		{
			LES_ERROR("LES_NetworkQueue::Pop() numItems == 0 but head != tail %d:%d", m_headIndex, m_tailIndex);
			return LES_NULL;
		}
	}

	//LES_LOG("Pop: head %d tail %d num %d pItem:%p", m_headIndex, m_tailIndex, m_numItems, pItem);
	return pItem;
}

#endif // #ifndef LES_NETWORKQUEUE_HH
