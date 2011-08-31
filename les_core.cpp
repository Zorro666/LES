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

static const LES_StringTable* les_pStringTable = LES_NULL;
static int les_stringTableNumStrings = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_GetStringEntrySlow(const LES_Hash hash, const char* const str)
{
	if (les_pStringTable)
	{
		const int index = les_pStringTable->GetStringEntrySlow(hash, str);
		if (index >= 0)
		{
			return index;
		}
	}
	/* This is horribly slow - need hash lookup table */
	const int numStrings = les_numStringEntries;
	for (int i = 0; i < numStrings; i++)
	{
		const LES_StringEntry* const pStringEntry = &les_stringEntryArray[i];
		if (pStringEntry->m_hash == hash)
		{
			if (strcmp(pStringEntry->m_str, str) == 0)
			{
				return (i + les_stringTableNumStrings);
			}
		}
	}
	return -1;
}

/* str - must not be from the stack, must be global so the ptr can just be copied */
static int LES_AddStringEntry(const LES_Hash hash, const char* const str)
{
	int index = LES_GetStringEntrySlow(hash, str);
	if (index >= 0)
	{
		return index;
	}

	/* Not found so add it */
	index = les_numStringEntries;
	LES_StringEntry* const pStringEntry = &les_stringEntryArray[index];
	pStringEntry->m_hash = hash;
	pStringEntry->m_str = str;
	les_numStringEntries++;
	index += les_stringTableNumStrings;

	return index;
}

static void LES_SetStringTablePtr(const LES_StringTable* const pStringTable)
{
	les_pStringTable = pStringTable;
	const int numStrings = pStringTable->GetNumStrings();
	les_stringTableNumStrings = numStrings;
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
	if (id < 0)
	{
		return LES_NULL;
	}
	const int index = (id - les_stringTableNumStrings);
	if (index < 0)
	{
		// Get it from definition file string table
		const LES_StringEntry* const stringEntry = les_pStringTable->GetStringEntry(id);
		return stringEntry;
	}
	// Get it from internal global list
	const LES_StringEntry* const stringEntry = &les_stringEntryArray[index];
	return stringEntry;
}

const LES_StringEntry* LES_GetStringEntry(const char* const str)
{
	const LES_Hash hash = LES_GenerateHashCaseSensitive(str);
	const int index = LES_GetStringEntrySlow(hash, str);
	return LES_GetStringEntryForID(index);
}

const LES_StringEntry* LES_GetStringEntryByHash(const unsigned int hash)
{
	if (les_pStringTable)
	{
		const LES_StringEntry* const pStringTableEntry = les_pStringTable->GetStringEntryByHash(hash);
		if (pStringTableEntry)
		{
			return pStringTableEntry;
		}
	}

	/* This is horribly slow - need hash lookup table */
	const int numStrings = les_numStringEntries;
	for (int i = 0; i < numStrings; i++)
	{
		const LES_StringEntry* const pStringEntry = &les_stringEntryArray[i];
		if (pStringEntry->m_hash == hash)
		{
			return pStringEntry;
		}
	}
	return LES_NULL;
}

int LES_SetGlobalDefinitionFile(const void* definitionFileData, const int fileDataSize)
{
	if (les_numStringEntries != 0)
	{
		LES_ERROR("LES_SetGlobalDefinitionFile string entries been added before loading global definition file");
		return LES_RETURN_ERROR;
	}

	if (les_definitionFile.Load(definitionFileData, fileDataSize) != LES_RETURN_OK)
	{
		LES_ERROR("LES_SetGlobalDefinitionFile failed to load definitionFileData");
		return LES_RETURN_ERROR;
	}

	const LES_StringTable* const pStringTable = les_definitionFile.GetStringTable();
	LES_SetStringTablePtr(pStringTable);

	const LES_TypeData* const pTypeData = les_definitionFile.GetTypeData();
	LES_Type_SetTypeDataPtr(pTypeData);

	return LES_RETURN_OK;
}

void LES_DebugOutputGlobalDefinnitionFile(LES_LoggerChannel* const pLogChannel)
{
	if (les_definitionFile.Loaded() == false)
	{
		return;
	}
	// Debug output of the definition file
	const char* id = les_definitionFile.GetID();
	const int numChunks = les_definitionFile.GetNumChunks();
	pLogChannel->Print("ID:'%c%c%c%c'", id[0], id[1], id[2], id[3]);
	pLogChannel->Print("NumChunks:%d", numChunks);

	const int numStrings = les_pStringTable->GetNumStrings();
	pLogChannel->Print("numStrings:%d", numStrings);
	//LES_LOG("m_settled:%d", m_settled);
	for (int i = 0; i < numStrings; i++)
	{
		const LES_StringEntry* const pStringTableEntry = les_pStringTable->GetStringEntry(i);
		pLogChannel->Print("String[%d] hash:0x%X string:'%s'", i, pStringTableEntry->m_hash, pStringTableEntry->m_str);
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
		const LES_StringEntry* const nameEntry = LES_GetStringEntryByHash(hash);
		const char* const name = nameEntry ? nameEntry->m_str : "NULL";
		const LES_StringEntry* const aliasedEntry = LES_GetStringEntryForID(aliasedID);
		const char* const aliasedName = aliasedEntry ? aliasedEntry->m_str : "NULL";

		char flagsDecoded[1024];
		LES_Type_DecodeFlags(flagsDecoded, flags);
		pLogChannel->Print("Type[%d] name:'%s' hash:0x%X size:%d flags:0x%X %s aliasedName:'%s' aliasedID:%d numElements:%d",
												i, name, hash, dataSize, flags, flagsDecoded,
												aliasedName, aliasedID, numElements);
	}
}

void LES_DebugOutputStringEntries(LES_LoggerChannel* const pLogChannel)
{
	const int numStringEntries = les_numStringEntries;
	for (int i = 0; i < numStringEntries; i++)
	{
		const LES_StringEntry* const pStringEntry = &les_stringEntryArray[i];
		pLogChannel->Print("String[%d] name:'%s' hash:0x%X", i, pStringEntry->m_str, pStringEntry->m_hash);
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

