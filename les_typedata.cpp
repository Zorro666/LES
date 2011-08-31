#include "les_typedata.h"
#include "les_logger.h"
#include "les_stringentry.h"

int LES_TypeData::Settle(void)
{
	if (m_settled != 0)
	{
		return LES_RETURN_ERROR;
	}

	// Convert from little endian first
	m_numTypes = fromLittleEndian32(m_numTypes);
	m_settled = fromLittleEndian32(m_settled);

	const int numTypes = m_numTypes;
	for (int i = 0; i < numTypes; i++)
	{
		m_typeEntries[i].m_hash = fromLittleEndian32(m_typeEntries[i].m_hash);
		m_typeEntries[i].m_dataSize = fromLittleEndian32(m_typeEntries[i].m_dataSize);
		m_typeEntries[i].m_flags = fromLittleEndian32(m_typeEntries[i].m_flags);
		m_typeEntries[i].m_aliasedTypeID = fromLittleEndian32(m_typeEntries[i].m_aliasedTypeID);
		m_typeEntries[i].m_numElements = fromLittleEndian32(m_typeEntries[i].m_numElements);
	}
	m_settled = 1;

	return LES_RETURN_OK;
}

int LES_TypeData::GetTypeEntrySlow(const LES_uint hash) const
{
	/* This is horribly slow - need hash lookup table */
	const int numTypes = m_numTypes;
	for (int i = 0; i < numTypes; i++)
	{
		const LES_TypeEntry* const typeEntryPtr = GetTypeEntry(i);
		if (typeEntryPtr->m_hash == hash)
		{
			return i;
		}
	}
	return -1;
}

