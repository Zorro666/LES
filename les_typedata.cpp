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
	LES_LOG("m_numTypes:%d", m_numTypes);
	LES_LOG("m_settled:%d", m_settled);

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

	for (int i = 0; i < numTypes; i++)
	{
		LES_TypeEntry* const typeEntryPtr = &m_typeEntries[i];
		const unsigned int hash = typeEntryPtr->m_hash;
		const unsigned int dataSize = typeEntryPtr->m_dataSize;
		const unsigned int flags = typeEntryPtr->m_flags;
		const int aliasedID = typeEntryPtr->m_aliasedTypeID;
		const int numElements = typeEntryPtr->m_numElements;

		char flagsDecoded[1024];
		LES_Type_DecodeFlags(flagsDecoded, flags);
		LES_LOG("Type[%d] hash:0x%X size:%d flags:0x%X %s aliasedID:%d numElements:%d",
			 			i, hash, dataSize, flags, flagsDecoded, aliasedID, numElements);
	}

	return LES_RETURN_OK;
}

