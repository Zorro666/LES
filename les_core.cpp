#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "les_core.h"
#include "les_hash.h"
#include "les_stringentry.h"
#include "les_test.h"

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

static const LES_TypeEntry* LES_GetTypeEntry(const LES_StringEntry* const typeStringEntry)
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

static int LES_GetFunctionDefinitionIndexByNameID(const int nameID)
{
	/* This is horribly slow - need hash lookup table */
	for (int i=0; i<les_numFunctionDefinitions; i++)
	{
		const LES_FunctionDefinition* const functionDefinitionPtr = &les_functionDefinitionArray[i];
		if (functionDefinitionPtr->m_nameID == nameID)
		{
			return i;
		}
	}
	return -1;
}

static int LES_GetFunctionDefinitionIndex(const char* const name)
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
				return i;
			}
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public External functions
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
	const int index = LES_GetFunctionDefinitionIndex(name);
	if ((index < 0) || (index >= les_numFunctionDefinitions))
	{
		return LES_NULL;
	}
	const LES_FunctionDefinition* const functionDefinitionPtr = &les_functionDefinitionArray[index];
	return functionDefinitionPtr;
}

LES_FunctionParamData* LES_GetFunctionParamData(const int functionNameID)
{
	const int index = LES_GetFunctionDefinitionIndexByNameID(functionNameID);
	if ((index < 0) || (index >= les_numFunctionDefinitions))
	{
		return LES_NULL;
	}
	const LES_FunctionDefinition* const functionDefinitionPtr = &les_functionDefinitionArray[index];
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
		fprintf(stderr, "LES ERROR: AddParam type:'%s' not found\n", typeStringEntry->m_str);
		return LES_ERROR;
	}
	if (paramPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: AddParam type:'%s' paramPtr is NULL\n", typeStringEntry->m_str);
		return LES_ERROR;
	}
	const int paramDataSize = typeEntryPtr->m_typeDataSize;
	memcpy(m_currentBufferPtr, paramPtr, paramDataSize);
	m_currentBufferPtr += paramDataSize;

	return LES_OK;
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

int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const functionDefinitionPtr)
{
	int index = LES_GetFunctionDefinitionIndex(name);
	if ((index < 0) || (index >= les_numFunctionDefinitions))
	{
		/* Not found so add it */
		index = les_numFunctionDefinitions;
		les_functionDefinitionArray[index] = *functionDefinitionPtr;
		les_numFunctionDefinitions++;
	}

	return index;
}

int LES_AddType(const char* const type, const int typeDataSize)
{
	const LES_Hash hash = LES_GenerateHashCaseSensitive(type);
	int index = LES_GetTypeEntrySlow(hash);
	if ((index < 0) || (index >= les_numTypeEntries))
	{
		/* Not found so add it */
		index = les_numTypeEntries;
		LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[index];
		typeEntryPtr->m_hash = hash;
		typeEntryPtr->m_typeDataSize = typeDataSize;
		les_numTypeEntries++;
	}
	else
	{
		/* Check the data size match */
		LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[index];
		if (typeEntryPtr->m_typeDataSize != typeDataSize)
		{
			fprintf(stderr, "LES ERROR: type hash 0x%X already in list and typeDataSize doesn't match %d != %d\n",
							hash, typeEntryPtr->m_typeDataSize, typeDataSize);
			return LES_ERROR;
		}
	}

	return index;
}

int LES_FunctionStart( const char* const name, const char* const returnType, 
											 const LES_FunctionDefinition** functionDefinitionPtr,
											 LES_FunctionTempData* const functionTempData)
{
	functionTempData->functionName = name;
	functionTempData->functionCurrentParamIndex = 0;
	functionTempData->functionCurrentInputIndex = 0;
	functionTempData->functionCurrentOutputIndex = 0;
	functionTempData->functionParamData = LES_NULL;

	const LES_Hash functionReturnTypeTypeHash = LES_GenerateHashCaseSensitive(returnType);
	const LES_FunctionDefinition* const functionDefinition = LES_GetFunctionDefinition(name);
	if (functionDefinition == LES_NULL)
	{
		/* ERROR: function not found */
		fprintf(stderr, "LES ERROR: function '%s' : Can't find function definition\n", name);
		return LES_ERROR;
	}
	*functionDefinitionPtr = functionDefinition;

	const LES_StringEntry* const functionReturnTypeStringEntry = LES_GetStringEntryForID(functionDefinition->m_returnTypeID);
	/* Check the return type : exists */
	if (functionReturnTypeStringEntry == LES_NULL)
	{
		/* ERROR: return type not found */
		fprintf(stderr, "LES ERROR: function '%s' : Can't find function return type for ID:%d '%s'\n", 
						name, functionDefinition->m_returnTypeID, returnType);
		return LES_ERROR;
	}
	/* Check the return type : hash */
	if (functionReturnTypeTypeHash != functionReturnTypeStringEntry->m_hash)
	{
		/* ERROR: return type hash doesn't match function definition */
		fprintf(stderr, "LES_ERROR: function '%s' : Return type hash doesn't match function definition 0x%X != 0x%X Got:'%s' Expected:'%s'\n",
						name, functionReturnTypeTypeHash, functionReturnTypeStringEntry->m_hash,
						returnType, functionReturnTypeStringEntry->m_str );
		return LES_ERROR;
	}
	/* Check the return type : string */
	if (strcmp(returnType, functionReturnTypeStringEntry->m_str) != 0)
	{
		/* ERROR: return type string doesn't match function definition */
		fprintf(stderr, "LES_ERROR: function '%s' : Return type string doesn't match function definition '%s' != '%s'\n",
						name, returnType, functionReturnTypeStringEntry->m_str );
		return LES_ERROR;
	}
	LES_FunctionParamData* const functionParamData = LES_GetFunctionParamData(functionDefinition->m_nameID);
	if (functionParamData == LES_NULL)
	{
		/* ERROR: functionParamData should only be LES_NULL if no inputs or outputs */
		if ((functionDefinition->m_numInputs > 0) || (functionDefinition->m_numOutputs > 0))
		{
			fprintf(stderr, "LES_ERROR: function '%s' : functionParamData is NULL numInputs:%d numOutputs:%d nameID:%d\n",
							name, functionDefinition->m_numInputs,  functionDefinition->m_numOutputs, functionDefinition->m_nameID);
			return LES_ERROR;
		}
	}
	functionTempData->functionParamData = functionParamData;

	/* Initialise parameter indexes */
	functionTempData->functionCurrentParamIndex = 0;
	functionTempData->functionCurrentInputIndex = 0;
	functionTempData->functionCurrentOutputIndex = 0;

	return LES_OK;
}

int LES_FunctionAddParam( const char* const type, const char* const name, const int index, 
													const char* const mode, const bool isInput, void* const data,
												  const LES_FunctionDefinition* const functionDefinition,
													LES_FunctionTempData* const functionTempData)
{
	const LES_Hash typeHash = LES_GenerateHashCaseSensitive(type);
	const LES_Hash nameHash = LES_GenerateHashCaseSensitive(name);
	/* Check the parameter index to see if it exceeds the number of declared parameters */
	const int functionMaxParamTypeIndex = (isInput ? functionDefinition->m_numInputs : functionDefinition->m_numOutputs);
	if (index >= functionMaxParamTypeIndex)
	{
		/* ERROR: parameter index exceeds the number of declared parameters */
		fprintf(stderr, "LES ERROR: function '%s' : Too many %s parameters index:%d max:%d parameter:'%s' type:'%s'\n",
						functionTempData->functionName, mode, index, functionMaxParamTypeIndex, name, type);
		return LES_ERROR;
	}
	/* Check the parameter index */
	const int functionCurrentParamTypeIndex = (isInput ? functionTempData->functionCurrentInputIndex : functionTempData->functionCurrentOutputIndex);
	if (index != functionCurrentParamTypeIndex)
	{
		/* ERROR: parameter index doesn't match the expected index */
		fprintf(stderr, "LES ERROR: function '%s' : Wrong %s parameter index:%d expected:%d parameter:'%s' type:'%s'\n",
						functionTempData->functionName, mode, functionCurrentParamTypeIndex, index, name, type);
		return LES_ERROR;
	}
	const LES_FunctionParameter* const functionParameterPtr = isInput ? 
												functionDefinition->GetInputParameter(index) : functionDefinition->GetOutputParameter(index);
	if (functionParameterPtr == LES_NULL)
	{
		/* ERROR: parameter index entry not found */
		fprintf(stderr, "LES ERROR: function '%s' : %s parameter index:%d parameter:'%s' type:'%s' is NULL\n",
						functionTempData->functionName, mode, index, name, type);
		return LES_ERROR;
	}
	/* Check the parameter index */
	const int functionParameterIndex = functionParameterPtr->m_index;
	const int functionCurrentParamIndex = functionTempData->functionCurrentParamIndex;
	if (functionCurrentParamIndex != functionParameterIndex)
	{
		/* ERROR: parameter index doesn't match the value stored in the definition file */
		fprintf(stderr, "LES ERROR: function '%s' : Wrong function parameter index:%d expected:%d %s parameter:'%s' type:'%s'\n", 
						functionTempData->functionName, functionCurrentParamIndex, functionParameterIndex, mode, name, type);
		return LES_ERROR;
	}
	/* Check the parameter type */
	const int functionParameterTypeID = functionParameterPtr->m_typeID;
	const LES_StringEntry* const parameterTypeStringEntry = LES_GetStringEntryForID(functionParameterTypeID);
	if (parameterTypeStringEntry == LES_NULL)
	{
		/* ERROR: can't find the parameter type */
		fprintf(stderr, "LES ERROR: function '%s' : Can't find %s parameter type for ID:%d parameter:'%s' type:'%s'\n", 
						functionTempData->functionName, mode, functionParameterTypeID, name, type);
		return LES_ERROR;
	}
	/* Check the parameter type : hash */
	if (typeHash != parameterTypeStringEntry->m_hash)
	{
		/* ERROR: parameter type hash doesn't match */
		fprintf(stderr, "LES ERROR: function '%s' : parameter:%d '%s' (%s) type hash doesn't match for ID:%d 0x%X != 0x%X Got '%s' Expected '%s'\n",
					 functionTempData->functionName, functionCurrentParamIndex, name, mode, functionParameterTypeID,
					 typeHash, parameterTypeStringEntry->m_hash,
					 type, parameterTypeStringEntry->m_str);
		return LES_ERROR;
	}
	/* Check the parameter type : string */
	if (strcmp(type, parameterTypeStringEntry->m_str) != 0)
	{
		/* ERROR: parameter type string doesn't match */
		fprintf(stderr, "LES ERROR: function '%s' : Adding parameter '%s' type string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n",
						functionTempData->functionName, name, functionParameterTypeID,
						type, parameterTypeStringEntry->m_str,
						typeHash, parameterTypeStringEntry->m_hash);
		return LES_ERROR;
	}
	/* Check the parameter name */
	const int functionParameterNameID = functionParameterPtr->m_nameID;
	const LES_StringEntry* const parameterNameStringEntry = LES_GetStringEntryForID(functionParameterNameID);
	if (parameterNameStringEntry == LES_NULL)
	{
		/* ERROR: can't find the parameter name */
		fprintf(stderr, "LES ERROR: function '%s' : Can't find parameter name for ID:%d parameter:'%s' type:'%s'\n",
						functionTempData->functionName, functionParameterNameID, name, type);
		return LES_ERROR;
	}
	/* Check the parameter name : hash */
	if (nameHash != parameterNameStringEntry->m_hash)
	{
		/* ERROR: parameter name hash doesn't match */
		fprintf(stderr, "LES ERROR: function '%s' : Adding parameter '%s' name hash doesn't match for ID:%d 0x%X != 0x%X Got:'%s' Expected:'%s'\n",
						functionTempData->functionName, name, functionParameterNameID,
						nameHash, parameterNameStringEntry->m_hash,
						name, parameterNameStringEntry->m_str);
		return LES_ERROR;
	}
	/* Check the parameter name : string */
	if (strcmp(name, parameterNameStringEntry->m_str) != 0)
	{
		/* ERROR: parameter name string doesn't match */
		fprintf(stderr, "LES ERROR: function '%s' : Adding parameter '%s' name string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n", 
						functionTempData->functionName, name, functionParameterNameID,
						name, parameterNameStringEntry->m_str,
						nameHash, parameterNameStringEntry->m_hash);
		return LES_ERROR;
	}
	/* Store the parameter value */
	LES_FunctionParamData* const functionParamData = functionTempData->functionParamData;
	if (functionParamData == LES_NULL)
	{
		/* ERROR: functionParamData is NULL */
		fprintf(stderr, "LES ERROR: function '%s' : functionParamData is NULL\n", functionTempData->functionName);
		return LES_ERROR;
	}
	functionParamData->AddParam(parameterTypeStringEntry, data);

	/* Update parameter indexes */
	functionTempData->functionCurrentParamIndex++;
	if (isInput)
	{
		functionTempData->functionCurrentInputIndex++;
	}
	else
	{
		functionTempData->functionCurrentOutputIndex++;
	}

	return LES_OK;
}

