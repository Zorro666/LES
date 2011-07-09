#include <string.h>

#include "les_core.h"
#include "les_hash.h"
#include "les_stringentry.h"
#include "les_test.h"
#include "les_function.h"

struct LES_TypeEntry
{
	LES_Hash m_hash;
	int m_typeDataSize;
};

static LES_StringEntry* les_stringEntryArray = LES_NULL;
static int les_numStringEntries = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_GetStringEntrySlow(const LES_Hash hash, const char* const str)
{
	/* This is horribly slow - need hash lookup table */
	for (int i=0; i<les_numStringEntries; i++)
	{
		const LES_StringEntry* const stringEntryPtr = &les_stringEntryArray[i];
		if (stringEntryPtr->m_hash == hash)
		{
			if (strcmp(stringEntryPtr->m_str, str) == 0)
			{
				return i;
			}
		}
	}
	return -1;
}

/* str - must not be from the stack, must be global so the ptr can just be copied */
static int LES_AddStringEntry(const LES_Hash hash, const char* const str)
{
	int index = LES_GetStringEntrySlow(hash, str);
	if ((index >= 0) && (index < les_numStringEntries))
	{
		return index;
	}

	/* Not found so add it */
	index = les_numStringEntries;
	LES_StringEntry* const stringEntryPtr = &les_stringEntryArray[index];
	stringEntryPtr->m_hash = hash;
	stringEntryPtr->m_str = str;
	les_numStringEntries++;

	return index;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

extern void LES_FunctionInit();
extern void LES_FunctionShutdown();

void LES_Init(void)
{
	les_stringEntryArray = new LES_StringEntry[1024];
	les_numStringEntries = 0;

	LES_FunctionInit();

	LES_TestSetup();
}

void LES_Shutdown(void)
{
	les_numStringEntries = 0;
	delete[] les_stringEntryArray;

	LES_FunctionShutdown();
}

const LES_StringEntry* LES_GetStringEntryForID(const int id)
{
	if ((id < 0) || (id >= les_numStringEntries))
	{
		return LES_NULL;
	}

	const LES_StringEntry* const stringEntry = &les_stringEntryArray[id];
	return stringEntry;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

int LES_AddStringEntry(const char* const str)
{
	const LES_Hash hash = LES_GenerateHashCaseSensitive(str);
	return LES_AddStringEntry(hash, str);
}

