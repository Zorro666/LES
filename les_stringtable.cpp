#include "les_stringtable.h"
#include "les_logger.h"

int LES_StringTable::Settle(void)
{
	if (m_settled != 0)
	{
		return LES_RETURN_ERROR;
	}

	// Convert from little endian first
	m_numStrings = fromLittleEndian32(m_numStrings);
	m_settled = fromLittleEndian32(m_settled);

	const int numStrings = m_numStrings;
	for (int i = 0; i < numStrings; i++)
	{
		m_stringTableEntries[i].m_hash = fromLittleEndian32(m_stringTableEntries[i].m_hash);
		m_stringTableEntries[i].m_offset = fromLittleEndian32(m_stringTableEntries[i].m_offset);
	}

	const char* pStringTableStart = (const char*)&m_numStrings;
	pStringTableStart += 8;
	pStringTableStart += (numStrings * 8);
	for (int i = 0; i < numStrings; i++)
	{
		LES_StringTableEntry* const pStringTableEntry = &m_stringTableEntries[i];
		const int offset = pStringTableEntry->m_offset;
		const char* const pString = pStringTableStart + offset;
		LES_StringEntry* const pStringEntry = (LES_StringEntry* const)(&m_stringTableEntries[i]);
		pStringEntry->m_str = pString;
	}
	m_settled = 1;

	return LES_RETURN_OK;
}

