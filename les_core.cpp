#include <string.h>
#include <sys/timeb.h>
#include <time.h>

#include "les_core.h"
#include "les_logger.h"
#include "les_hash.h"
#include "les_stringentry.h"
#include "les_function.h"
#include "les_definitionfile.h"
#include "les_stringtable.h"
#include "les_typedata.h"
#include "les_structdata.h"

static LES_StringEntry* les_stringEntryArray = LES_NULL;
static int les_numStringEntries = 0;

static LES_DefinitionFile les_definitionFile;

static const LES_StringTable* les_pStringTable = LES_NULL;
static int les_stringTableNumStrings = 0;

static LES_uint64 les_startTime = 0;

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
		if (les_pStringTable)
		{
			const int i = les_pStringTable->GetStringEntrySlow(hash, str);
			const LES_StringEntry* const pStringEntry = les_pStringTable->GetStringEntry(i);
			if (pStringEntry->m_hash != hash)
			{
				LES_FATAL_ERROR("LES_AddStringEntry '%s' 0x%X hash doesn't match entry in string table definition file Existing:'%s' 0x%X", 
												str, hash, pStringEntry->m_str, pStringEntry->m_hash);
				return LES_RETURN_ERROR;
			}
			if (strcmp(pStringEntry->m_str, str) != 0)
			{
				LES_FATAL_ERROR("LES_AddStringEntry '%s' 0x%X string doesn't match entry in string table definition file Existing:'%s' 0x%X", 
												str, hash, pStringEntry->m_str, pStringEntry->m_hash);
				return LES_RETURN_ERROR;
			}
		}
		return index;
	}

	if (les_pStringTable)
	{
		LES_ERROR("LES_AddStringEntry '%s' 0x%X not found in string table definition file", str, hash);
		return LES_RETURN_ERROR;
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

static void LES_DebugOutputStringEntry(LES_LoggerChannel* const pLogChannel, const LES_StringEntry* const pStringEntry, const int i)
{
	pLogChannel->Print("String[%d] name:'%s' hash:0x%X", i, pStringEntry->m_str, pStringEntry->m_hash);
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

extern void LES_Type_SetTypeDataPtr(const LES_TypeData* const pTypeData);
extern void LES_Struct_SetStructDataPtr(const LES_StructData* const pStructData);
extern void LES_Function_SetFuncDataPtr(const LES_FuncData* const pFuncData);

extern void LES_DebugOutputTypeEntry(LES_LoggerChannel* const pLogChannel, const LES_TypeEntry* const pTypeEntry, const int i);
extern void LES_DebugOutputStructDefinition(LES_LoggerChannel* const pLogChannel, 
																						const LES_StructDefinition* const pStructDefinition, const int i);

void LES_Init(void)
{
	les_stringEntryArray = new LES_StringEntry[1024];
	les_numStringEntries = 0;

	les_startTime = LES_GetElapsedTimeTicks();

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

	const LES_StructData* const pStructData = les_definitionFile.GetStructData();
	LES_Struct_SetStructDataPtr(pStructData);

	const LES_FuncData* const pFuncData = les_definitionFile.GetFuncData();
	LES_Function_SetFuncDataPtr(pFuncData);

	return LES_RETURN_OK;
}

void LES_DebugOutputGlobalDefinitionFile(LES_LoggerChannel* const pLogChannel)
{
	if (les_definitionFile.Loaded() == false)
	{
		return;
	}
	// Debug output of the definition file
	const char* id = les_definitionFile.GetID();
	const int numChunks = les_definitionFile.GetNumChunks();
	pLogChannel->Print("");
	pLogChannel->Print("ID:'%c%c%c%c'", id[0], id[1], id[2], id[3]);
	pLogChannel->Print("NumChunks:%d", numChunks);

	const int numStrings = les_pStringTable->GetNumStrings();
	pLogChannel->Print("");
	pLogChannel->Print("numStrings:%d", numStrings);
	for (int i = 0; i < numStrings; i++)
	{
		const LES_StringEntry* const pStringEntry = les_pStringTable->GetStringEntry(i);
		LES_DebugOutputStringEntry(pLogChannel, pStringEntry, i);
	}

	const LES_TypeData* const pTypeData = les_definitionFile.GetTypeData();
	const int numTypes = pTypeData->GetNumTypes();
	pLogChannel->Print("");
	pLogChannel->Print("numTypes:%d", numTypes);
	for (int i = 0; i < numTypes; i++)
	{
		const LES_TypeEntry* const pTypeEntry = pTypeData->GetTypeEntry(i);
		LES_DebugOutputTypeEntry(pLogChannel, pTypeEntry, i);
	}
	const LES_StructData* const pStructData = les_definitionFile.GetStructData();
	const int numStructDefinitions = pStructData->GetNumStructDefinitions();
	pLogChannel->Print("");
	pLogChannel->Print("numStructDefinitions:%d", numStructDefinitions);
	for (int i = 0; i < numStructDefinitions; i++)
	{
		const LES_StructDefinition* const pStructDefinition = pStructData->GetStructDefinition(i);
		LES_DebugOutputStructDefinition(pLogChannel, pStructDefinition, i);
	}
	pLogChannel->Print("");
	pLogChannel->Print("NEED DBEUG OUTPUT OF FUNCTION DEFINITIONS");
}

void LES_DebugOutputStringEntries(LES_LoggerChannel* const pLogChannel)
{
	const int numStringEntries = les_stringTableNumStrings + les_numStringEntries;
	pLogChannel->Print("numStringEntries:%d StringTable:%d Internal:%d", numStringEntries, les_stringTableNumStrings, les_numStringEntries);
	for (int i = 0; i < numStringEntries; i++)
	{
		const LES_StringEntry* const pStringEntry = LES_GetStringEntryForID(i);
		LES_DebugOutputStringEntry(pLogChannel, pStringEntry, i);
	}
}

LES_uint64 LES_GetElapsedTimeTicks(void)
{
	timeb tb;
	ftime(&tb);

	LES_uint64 newTicks = (tb.time * 1000) + tb.millitm;
	LES_uint64 deltaTicks = (newTicks - les_startTime);
	return deltaTicks;
}

float LES_GetElapsedTimeInSeconds(void)
{
	const LES_uint64 elapsedTicks = LES_GetElapsedTimeTicks();
	float elapsedSeconds = (float)(elapsedTicks)/1000.0f;
	return elapsedSeconds;
}

void LES_Sleep(const float sleepTimeInSeconds)
{
	timespec sleepTime;
	timespec remainingTime;
	const int numSeconds = (int)sleepTimeInSeconds;
	const int numMicroseconds = (int)((sleepTimeInSeconds-(float)numSeconds)*1000000000.0f);
	sleepTime.tv_sec = numSeconds;
	sleepTime.tv_nsec = numMicroseconds;
	nanosleep(&sleepTime, &remainingTime);
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

int LES_GetStringEntryID(const LES_Hash hash, const char* const str)
{
	const int index = LES_GetStringEntrySlow(hash, str);
	return index;
}

