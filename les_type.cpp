#include <stdio.h>

#include "les_type.h"
#include "les_core.h"
#include "les_stringentry.h"

#define LES_TYPE_DEBUG 0

static LES_TypeEntry* les_typeEntryArray = LES_NULL;
static int les_numTypeEntries = 0;

LES_Hash LES_TypeEntry::s_longlongHash = LES_GenerateHashCaseSensitive("long long int");
LES_Hash LES_TypeEntry::s_intHash = LES_GenerateHashCaseSensitive("int");
LES_Hash LES_TypeEntry::s_unsignedintPtrHash = LES_GenerateHashCaseSensitive("unsigned int*");
LES_Hash LES_TypeEntry::s_shortHash = LES_GenerateHashCaseSensitive("short");
LES_Hash LES_TypeEntry::s_unsignedshortPtrHash = LES_GenerateHashCaseSensitive("unsigned short*");
LES_Hash LES_TypeEntry::s_charHash = LES_GenerateHashCaseSensitive("char");
LES_Hash LES_TypeEntry::s_unsignedcharPtrHash = LES_GenerateHashCaseSensitive("unsigned char*");
LES_Hash LES_TypeEntry::s_floatHash = LES_GenerateHashCaseSensitive("float");
LES_Hash LES_TypeEntry::s_floatPtrHash = LES_GenerateHashCaseSensitive("float*");
LES_Hash LES_TypeEntry::s_intPtrHash = LES_GenerateHashCaseSensitive("int*");

extern int LES_AddStringEntry(const char* const str);

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

int LES_TypeEntry::ComputeDataStorageSize(void) const
{
	const LES_TypeEntry* typeEntry = this;
	int flags = typeEntry->m_flags;
	while (flags & LES_TYPE_ALIAS)
	{
		const int aliasedTypeID = typeEntry->m_aliasedTypeID;
#if LES_TYPE_DEBUG
		printf("Type 0x%X alias:%d flags:0x%X\n", typeEntry->m_hash, aliasedTypeID, flags);
#endif // #if LES_TYPE_DEBUG
		const LES_StringEntry* const aliasedStringTypeEntry = LES_GetStringEntryForID(aliasedTypeID);
		if (aliasedStringTypeEntry == LES_NULL)
		{
			fprintf(stderr, "LES ERROR: ComputeDataStorageSize aliased type:%d entry can't be found\n", aliasedTypeID);
			return -1;
		}
		const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringTypeEntry);
		if (aliasedTypeEntryPtr == LES_NULL)
		{
			fprintf(stderr, "LES ERROR: ComputeDataStorageSize aliased type not found aliased type:'%s'\n", aliasedStringTypeEntry->m_str);
			return -1;
		}
		typeEntry = (const LES_TypeEntry*)aliasedTypeEntryPtr;
		flags = typeEntry->m_flags;
#if LES_TYPE_DEBUG
		printf("Alias Type 0x%X flags:0x%X\n", typeEntry->m_hash, flags);
#endif // #if LES_TYPE_DEBUG
	}
	const int dataSize = typeEntry->m_dataSize;
	return dataSize;
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

int LES_AddType(const char* const name, const unsigned int dataSize, const unsigned int flags, const char* const aliasedName)
{
	LES_AddStringEntry(name);

	const LES_StringEntry* const aliasedEntryPtr = LES_GetStringEntry(aliasedName);
	if (aliasedEntryPtr == LES_NULL)
	{
		fprintf(stderr, "LES ERROR: AddType '%s' : aliasedEntry '%s' not found\n", name, aliasedName);
		return LES_ERROR;
	}
	const int aliasedTypeID = LES_AddStringEntry(aliasedName);
#if LES_TYPE_DEBUG
	printf("aliasedTypeID:%d name:'%s' aliasedName:'%s'\n", aliasedTypeID, name, aliasedName);
#endif // #if LES_TYPE_DEBUG

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
		typeEntryPtr->m_aliasedTypeID = aliasedTypeID;

		const LES_Hash aliasedHash = LES_GenerateHashCaseSensitive(aliasedName);
		if (aliasedHash != hash)
		{
			typeEntryPtr->m_flags |= LES_TYPE_ALIAS;
#if LES_TYPE_DEBUG
			printf("Type '%s' has an alias to '%s'\n", name, aliasedName);
#endif // #if LES_TYPE_DEBUG
		}

		les_numTypeEntries++;
	}
	else
	{
		/* Check the type data matchs */
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
		if (typeEntryPtr->m_aliasedTypeID != aliasedTypeID)
		{
			fprintf(stderr, "LES ERROR: AddType '%s' hash 0x%X already in list and aliasedTypeID doesn't match Existing:%d New:%d\n",
							name, hash, typeEntryPtr->m_aliasedTypeID, aliasedTypeID);
			return LES_ERROR;
		}
	}
	return index;
}
