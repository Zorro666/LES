#include <string.h>

#include "les_core.h"
#include "les_logger.h"
#include "les_hash.h"
#include "les_stringentry.h"
#include "les_function.h"
#include "les_definitionfile.h"
#include "les_stringtable.h"
#include "les_typedata.h"

static LES_StringEntry* les_stringEntryArray = LES_NULL;
static int les_numStringEntries = 0;
static LES_DefinitionFile les_definitionFile;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
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

extern void LES_TypeInit();
extern void LES_TypeShutdown();

extern void LES_StructInit();
extern void LES_StructShutdown();

void LES_Init(void)
{
	les_stringEntryArray = new LES_StringEntry[1024];
	les_numStringEntries = 0;

	LES_FunctionInit();
	LES_TypeInit();
	LES_StructInit();
}

void LES_Shutdown(void)
{
	LES_StructShutdown();
	LES_TypeShutdown();
	LES_FunctionShutdown();

	les_numStringEntries = 0;
	delete[] les_stringEntryArray;
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

const LES_StringEntry* LES_GetStringEntry(const char* const str)
{
	const LES_Hash hash = LES_GenerateHashCaseSensitive(str);
	const int index = LES_GetStringEntrySlow(hash, str);
	return LES_GetStringEntryForID(index);
}

const LES_StringEntry* LES_GetStringEntryByHash(unsigned int hash)
{
	/* This is horribly slow - need hash lookup table */
	for (int i=0; i<les_numStringEntries; i++)
	{
		const LES_StringEntry* const stringEntryPtr = &les_stringEntryArray[i];
		if (stringEntryPtr->m_hash == hash)
		{
			return stringEntryPtr;
		}
	}
	return LES_NULL;
}

int LES_SetGlobalDefinitionFile(const void* definitionFileData, const int fileDataSize)
{
	if (les_definitionFile.Load(definitionFileData, fileDataSize) != LES_RETURN_OK)
	{
		LES_ERROR("LES_SetGlobalDefinitionFile failed to load definitionFileData");
		return LES_RETURN_ERROR;
	}

	return LES_RETURN_OK;
}

void LES_DebugOutputGlobalDefinnitionFile(LES_LoggerChannel* const pLogChannel)
{
	// Debug output of the definition file
	const char* id = les_definitionFile.GetID();
	const int numChunks = les_definitionFile.GetNumChunks();
	pLogChannel->Print("ID:'%c%c%c%c'", id[0], id[1], id[2], id[3]);
	pLogChannel->Print("NumChunks:%d", numChunks);

	const LES_StringTable* const pStringTable = les_definitionFile.GetStringTable();
	const int numStrings = pStringTable->GetNumStrings();
	pLogChannel->Print("numStrings:%d", numStrings);
	//LES_LOG("m_settled:%d", m_settled);
	for (int i = 0; i < numStrings; i++)
	{
		const LES_StringEntry* const pStringEntry = pStringTable->GetStringEntry(i);
		pLogChannel->Print("String[%d] hash:0x%X string:'%s'", i, pStringEntry->m_hash, pStringEntry->m_str);
	}

	const LES_TypeData* const pTypeData = les_definitionFile.GetTypeData();
	const int numTypes = pTypeData->GetNumTypes();
	pLogChannel->Print("numTypes:%d", numTypes);
	//LES_LOG("m_settled:%d", m_settled);
	for (int i = 0; i < numTypes; i++)
	{
		const LES_TypeEntry* const typeEntryPtr = pTypeData->GetTypeEntry(i);
		const unsigned int hash = typeEntryPtr->m_hash;
		const unsigned int dataSize = typeEntryPtr->m_dataSize;
		const unsigned int flags = typeEntryPtr->m_flags;
		const int aliasedID = typeEntryPtr->m_aliasedTypeID;
		const int numElements = typeEntryPtr->m_numElements;

		char flagsDecoded[1024];
		LES_Type_DecodeFlags(flagsDecoded, flags);
		pLogChannel->Print("Type[%d] hash:0x%X size:%d flags:0x%X %s aliasedID:%d numElements:%d",
			 									i, hash, dataSize, flags, flagsDecoded, aliasedID, numElements);
	}
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

