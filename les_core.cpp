#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "les_core.h"
#include "les_hash.h"
#include "les_stringentry.h"

struct LES_TypeEntry
{
	LES_Hash m_hash;
	int m_typeDataSize;
};

static LES_StringEntry* les_stringEntryArray = LES_NULL;
static int les_numStringEntries = 0;

static LES_FunctionDefinition* les_functionDefinitionArray = LES_NULL;
static int les_numFunctionDefinitions = 0;

static LES_TypeEntry* les_typeEntryArray = LES_NULL;
static int les_numTypeEntries = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

const LES_TypeEntry* LES_GetTypeEntry(const LES_StringEntry* const typeStringEntry)
{
	/* This is horribly slow - need hash lookup table */
	for (int i=0; i<les_numTypeEntries; i++)
	{
		const LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[i];
		if (typeEntryPtr->m_hash == typeStringEntry->m_hash)
		{
			return typeEntryPtr;
		}
	}
	return LES_NULL;
}

int LES_GetStringEntrySlow(const LES_Hash hash, const char* const str)
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
int LES_AddStringEntry(const LES_Hash hash, const char* const str)
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

int LES_AddStringEntry(const char* const str)
{
	const LES_Hash hash = LES_GenerateHashCaseSensitive(str);
	return LES_AddStringEntry(hash, str);
}

void LES_TestSetup(void)
{
	LES_FunctionDefinition* const functionDefinitionPtr = &les_functionDefinitionArray[les_numFunctionDefinitions];
	functionDefinitionPtr->m_nameID = LES_AddStringEntry("sceNpInit");
	functionDefinitionPtr->m_returnTypeID = LES_AddStringEntry("void");
	functionDefinitionPtr->m_paramDataSize = sizeof(int);

	functionDefinitionPtr->m_numInputs = 1;
	functionDefinitionPtr->m_inputs = new LES_FunctionParameter[functionDefinitionPtr->m_numInputs];

	LES_FunctionParameter* functionParameterPtr;
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinitionPtr->m_inputs[0];
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_nameID = LES_AddStringEntry("a");
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");

	functionDefinitionPtr->m_numOutputs = 1;
	functionDefinitionPtr->m_outputs = new LES_FunctionParameter[functionDefinitionPtr->m_numOutputs];

	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinitionPtr->m_outputs[0];
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_nameID = LES_AddStringEntry("r");
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");

	les_numFunctionDefinitions++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_Init(void)
{
	les_stringEntryArray = new LES_StringEntry[1024];
	les_numStringEntries = 0;

	les_functionDefinitionArray = new LES_FunctionDefinition[1024];
	les_numFunctionDefinitions = 0;

	les_typeEntryArray = new LES_TypeEntry[1024];
	les_numTypeEntries = 0;

	LES_TestSetup();
}

void LES_Shutdown(void)
{
	les_numStringEntries = 0;
	delete[] les_stringEntryArray;

	les_numFunctionDefinitions = 0;
	delete[] les_functionDefinitionArray;
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

const LES_FunctionDefinition* LES_GetFunctionDefinition(const char* const name)
{
	const LES_Hash functionNameHash = LES_GenerateHashCaseSensitive(name);

	/* This is horribly slow - need hash lookup table */
	for (int i=0; i<les_numFunctionDefinitions; i++)
	{
		const LES_FunctionDefinition* const functionDefinitionPtr = &les_functionDefinitionArray[i];
		const LES_StringEntry* const functionNameStringEntryPtr = LES_GetStringEntryForID(functionDefinitionPtr->m_nameID);
		if (functionNameStringEntryPtr->m_hash == functionNameHash)
		{
			if (strcmp(functionNameStringEntryPtr->m_str, name) == 0)
			{
				return functionDefinitionPtr;
			}
		}
	}
	return LES_NULL;
}

LES_FunctionParamData* LES_GetFunctionParamData(const int functionNameID)
{
	if ((functionNameID < 0) || (functionNameID >= les_numFunctionDefinitions))
	{
		return LES_NULL;
	}
	const LES_FunctionDefinition* const functionDefinitionPtr = &les_functionDefinitionArray[functionNameID];
	const int paramDataSize = functionDefinitionPtr->m_paramDataSize;
	char* paramBuffer = new char[paramDataSize];
	LES_FunctionParamData* paramData = new LES_FunctionParamData;
	paramData->m_bufferPtr = paramBuffer;
	paramData->m_currentBufferPtr = paramData->m_bufferPtr;

	return paramData;
}

const LES_FunctionParameter* LES_FunctionDefinition::GetInputParameter(const int index) const
{
	if ((index < 0) || (index >= m_numInputs))
	{
		return LES_NULL;
	}

	const LES_FunctionParameter* functionParam = &m_inputs[index];
	return functionParam;
}

const LES_FunctionParameter* LES_FunctionDefinition::GetOutputParameter(const int index) const
{
	if ((index < 0) || (index >= m_numOutputs))
	{
		return LES_NULL;
	}

	const LES_FunctionParameter* functionParam = &m_outputs[index];
	return functionParam;
}

int LES_FunctionParamData::AddParam(const LES_StringEntry* const typeStringEntry, const void* const paramPtr)
{
	const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntry);
	if (typeEntryPtr == NULL)
	{
		return LES_ERROR;
	}
	if (paramPtr == NULL)
	{
		return LES_ERROR;
	}
	const int paramDataSize = typeEntryPtr->m_typeDataSize;
	memcpy(m_currentBufferPtr, paramPtr, paramDataSize);
	return LES_OK;
}

