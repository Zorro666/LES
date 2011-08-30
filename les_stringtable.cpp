#include "les_stringtable.h"
#include "les_logger.h"

int LES_StringTable::Settle(void)
{
	if (m_settled != 0)
	{
		return LES_RETURN_ERROR;
	}
	const int numStrings = m_numStrings;
	const char* pBase = (const char*)&m_numStrings;
	for (int i = 0; i < numStrings; i++)
	{
		LES_StringTableEntry* const pStringTableEntry = &m_stringTableEntries[i];
		const int offset = pStringTableEntry->m_offset;
		const char* const pString = pBase + offset;
		LES_StringEntry* const pStringEntry = (LES_StringEntry* const)(&m_stringTableEntries[i]);
		pStringEntry->m_str = pString;
	}
	m_settled = 1;

	return LES_RETURN_OK;
}

