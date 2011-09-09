#include <string.h>

#include "les_stringtable.h"
#include "les_logger.h"

int LES_StringTable::Settle(void)
{
	if (m_settled != 0)
	{
		return LES_RETURN_ERROR;
	}

	// Convert from little endian first
	m_numStrings = fromBigEndian32(m_numStrings);
	m_settled = fromBigEndian32(m_settled);

	const int numStrings = m_numStrings;
	for (int i = 0; i < numStrings; i++)
	{
		m_stringTableEntries[i].m_hash = fromBigEndian32(m_stringTableEntries[i].m_hash);
		m_stringTableEntries[i].m_offset = fromBigEndian32(m_stringTableEntries[i].m_offset);
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

int LES_StringTable::GetStringEntrySlow(const LES_Hash hash, const char* const str) const
{
	/* This is horribly slow - need hash lookup table */
	const int numStrings = m_numStrings;
	for (int i = 0; i < numStrings; i++)
	{
		const LES_StringEntry* const pStringEntry = GetStringEntry(i);
		if (pStringEntry->m_hash == hash)
		{
			if (strcmp(pStringEntry->m_str, str) == 0)
			{
				return i;
			}
		}
	}
	return -1;
}

const LES_StringEntry* LES_StringTable::GetStringEntryByHash(const LES_Hash hash) const
{
	/* This is horribly slow - need hash lookup table */
	const int numStrings = m_numStrings;
	for (int i = 0; i < numStrings; i++)
	{
		const LES_StringEntry* const pStringEntry = GetStringEntry(i);
		if (pStringEntry->m_hash == hash)
		{
			return pStringEntry;
		}
	}
	return LES_NULL;
}

