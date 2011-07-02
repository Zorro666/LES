#include <string.h>

#include "les_core.h"
#include "les_hash.h"
#include "les_stringentry.h"

static LES_StringEntry* les_stringEntryArray = LES_NULL;
static int les_numStringEntries = 0;

static LES_FunctionDefinition* les_functionDefinitionArray = LES_NULL;
static int les_numFunctionDefinitions = 0;

void LES_Init(void)
{
	les_stringEntryArray = new LES_StringEntry[1024];
	les_numStringEntries = 0;

	les_functionDefinitionArray = new LES_FunctionDefinition[1024];
	les_numFunctionDefinitions = 0;
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
		const LES_StringEntry* const functionNameStringEntryPtr = LES_GetStringEntryForID(functionDefinitionPtr->nameID);
		if (functionNameStringEntryPtr->hash == functionNameHash)
		{
			if (strcmp(functionNameStringEntryPtr->str, name) == 0)
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
	const int paramDataSize = functionDefinitionPtr->paramDataSize;
	char* paramBuffer = new char[paramDataSize];
	LES_FunctionParamData* paramData = new LES_FunctionParamData;
	paramData->buffer = paramBuffer;

	return paramData;
}

const LES_FunctionParameter* LES_FunctionDefinition::GetInputParameter(const int index) const
{
	if ((index < 0) || (index >= numInputs))
	{
		return LES_NULL;
	}

	const LES_FunctionParameter* functionParam = &inputs[index];
	return functionParam;
}

const LES_FunctionParameter* LES_FunctionDefinition::GetOutputParameter(const int index) const
{
	if ((index < 0) || (index >= numOutputs))
	{
		return LES_NULL;
	}

	const LES_FunctionParameter* functionParam = &outputs[index];
	return functionParam;
}

int LES_FunctionParamData::AddParam(const LES_StringEntry* const typeStringEntry, const void* const paramPtr)
{
	//char* buffer;
	return -1;
}

