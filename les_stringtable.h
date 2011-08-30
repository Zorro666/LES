#ifndef LES_STRINGTABLE_HH
#define LES_STRINGTABLE_HH

#include "les_base.h"
#include "les_stringentry.h"

struct LES_StringTableEntry
{
	LES_uint32 m_hash;								// 4-bytes
	LES_int32 m_offset;								// 4-bytes: settled on load and converted to m_str
};

/*
LES_StringTable
{
	LES_int32 m_numStrings; 																// 4-bytes
	LES_int32 m_settled;																		// 4-bytes
	LES_StringTableEntry m_stringTableEntries[numStrings]; 	// 8-bytes * m_numStrings, settled on load -> LES_StringEntry
	char stringData[];																			// total string table size in bytes
};
*/

class LES_StringTable
{
public:
	int GetNumStrings(void) const;
	const LES_StringTableEntry* GetStringTableEntry(const int index) const;
	const LES_StringEntry* GetStringEntry(const int index) const;

	friend class LES_DefinitionFile;
private:
	int Settle(void);

	LES_StringTable();
	~LES_StringTable();

	LES_int32 m_numStrings;
	LES_int32 m_settled;
	LES_StringTableEntry m_stringTableEntries[1];		// m_stringTableEntries[m_numStrings];
	// char stringData[];									
};

inline int LES_StringTable::GetNumStrings(void) const
{
	return m_numStrings;
}

inline const LES_StringTableEntry* LES_StringTable::GetStringTableEntry(const int index) const
{
	if (m_settled == 1)
	{
		return LES_NULL;
	}
	const LES_StringTableEntry* pStringTableEntry = &m_stringTableEntries[index];
	return pStringTableEntry;
}

inline const LES_StringEntry* LES_StringTable::GetStringEntry(const int index) const
{
	if (m_settled == 0)
	{
		return LES_NULL;
	}
	const LES_StringEntry* pStringEntry = (const LES_StringEntry*)&m_stringTableEntries[index];
	return pStringEntry;
}

#endif // #ifndef LES_STRINGTABLE_HH

