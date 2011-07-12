#include <stdio.h>

#include "les_type.h"
#include "les_core.h"
#include "les_stringentry.h"

static LES_TypeEntry* les_typeEntryArray = LES_NULL;
static int les_numTypeEntries = 0;

LES_Hash LES_TypeEntry::s_intHash = LES_GenerateHashCaseSensitive("int");
LES_Hash LES_TypeEntry::s_unsignedintPtrHash = LES_GenerateHashCaseSensitive("unsigned int*");
LES_Hash LES_TypeEntry::s_shortHash = LES_GenerateHashCaseSensitive("short");
LES_Hash LES_TypeEntry::s_unsignedshortPtrHash = LES_GenerateHashCaseSensitive("unsigned short*");
LES_Hash LES_TypeEntry::s_charHash = LES_GenerateHashCaseSensitive("char");
LES_Hash LES_TypeEntry::s_unsignedcharPtrHash = LES_GenerateHashCaseSensitive("unsigned char*");
LES_Hash LES_TypeEntry::s_floatHash = LES_GenerateHashCaseSensitive("float");
LES_Hash LES_TypeEntry::s_floatPtrHash = LES_GenerateHashCaseSensitive("float*");
LES_Hash LES_TypeEntry::s_intPtrHash = LES_GenerateHashCaseSensitive("int*");

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Static Internal functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_GetTypeEntrySlow(const LES_Hash hash)
{
	/* This is horribly slow - need hash lookup table */
	for (int i=0; i<les_numTypeEntries; i++)
	{
		const LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[i];
		if (typeEntryPtr->m_hash == hash)
		{
			return i;
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

const LES_TypeEntry* LES_GetTypeEntry(const LES_StringEntry* const typeStringEntry)
{
	const LES_Hash hash = typeStringEntry->m_hash;
	int index = LES_GetTypeEntrySlow(hash);
	if ((index >= 0) && (index < les_numTypeEntries))
	{
		const LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[index];
		return typeEntryPtr;
	}
	return LES_NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_TypeInit(void)
{
	les_typeEntryArray = new LES_TypeEntry[1024];
	les_numTypeEntries = 0;
}

void LES_TypeShutdown(void)
{
	les_numTypeEntries = 0;
	delete[] les_typeEntryArray;
}

int LES_AddType(const char* const name, const unsigned int dataSize, const unsigned int flags)
{
	const LES_Hash hash = LES_GenerateHashCaseSensitive(name);
	int index = LES_GetTypeEntrySlow(hash);
	if ((index < 0) || (index >= les_numTypeEntries))
	{
		/* Not found so add it */
		index = les_numTypeEntries;
		LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[index];
		typeEntryPtr->m_hash = hash;
		typeEntryPtr->m_dataSize = dataSize;
		typeEntryPtr->m_flags = flags;
		les_numTypeEntries++;
	}
	else
	{
		/* Check the data size match */
		LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[index];
		if (typeEntryPtr->m_dataSize != dataSize)
		{
			fprintf(stderr, "LES ERROR: AddType '%s' hash 0x%X already in list and dataSize doesn't match Existing:%d New:%d\n",
							name, hash, typeEntryPtr->m_dataSize, dataSize);
			return LES_ERROR;
		}
		if (typeEntryPtr->m_flags != flags)
		{
			fprintf(stderr, "LES ERROR: AddType '%s' hash 0x%X already in list and flags doesn't match Existing:0x%X New:0x%X\n",
							name, hash, typeEntryPtr->m_flags, flags);
			return LES_ERROR;
		}
	}
	return index;
}
