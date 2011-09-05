#include <string.h>
#include <malloc.h>

#include "les_core.h"
#include "les_logger.h"
#include "les_function.h"
#include "les_function_macros.h"
#include "les_stringentry.h"
#include "les_parameter.h"
#include "les_struct.h"
#include "les_funcdata.h"

static const LES_FunctionDefinition** les_functionDefinitionArray = LES_NULL;
static int les_numFunctionDefinitions = 0;

static const LES_FuncData* les_pFuncData = LES_NULL;
static int les_funcDataNumFunctionDefinitions = 0;

void LES_DebugOutputFunctionDefinition(LES_LoggerChannel* const pLogChannel, 
																			 const LES_FunctionDefinition* const pFunctionDefinition, const int i);

#define LES_FUNCTION_DEBUG 0

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static const LES_FunctionDefinition* LES_GetFunctionDefinitionForID(const int id)
{
	if (id < 0)
	{
		return LES_NULL;
	}
	const int index = (id - les_funcDataNumFunctionDefinitions);
	if (index < 0)
	{
		// Get it from definition file func data
		const LES_FunctionDefinition* const pFunctionDefinition = les_pFuncData->GetFunctionDefinition(id);
		return pFunctionDefinition;
	}
	const LES_FunctionDefinition* const pFunctionDefinition = les_functionDefinitionArray[index];
	return pFunctionDefinition;
}

static int LES_GetFunctionDefinitionIndexByNameID(const int nameID)
{
	if (les_pFuncData)
	{
		const int index = les_pFuncData->GetFunctionDefinitionIndexByNameID(nameID);
		if (index >= 0)
		{
			return index;
		}
	}
	/* This is horribly slow - need hash lookup table */
	for (int i = 0; i < les_numFunctionDefinitions; i++)
	{
		const LES_FunctionDefinition* const functionDefinitionPtr = les_functionDefinitionArray[i];
		if (functionDefinitionPtr->GetNameID() == nameID)
		{
			return i + les_funcDataNumFunctionDefinitions;
		}
	}
	return -1;
}

static int LES_GetFunctionDefinitionIndex(const char* const name)
{
	const LES_Hash functionNameHash = LES_GenerateHashCaseSensitive(name);
	if (les_pFuncData)
	{
		const int index = les_pFuncData->GetFunctionDefinitionIndex(functionNameHash);
		if (index >= 0)
		{
			return index;
		}
	}

	/* This is horribly slow - need hash lookup table */
	for (int i = 0; i < les_numFunctionDefinitions; i++)
	{
		const LES_FunctionDefinition* const functionDefinitionPtr = les_functionDefinitionArray[i];
		const LES_StringEntry* const functionNameStringEntryPtr = LES_GetStringEntryForID(functionDefinitionPtr->GetNameID());
		if (functionNameStringEntryPtr->m_hash == functionNameHash)
		{
			if (strcmp(functionNameStringEntryPtr->m_str, name) == 0)
			{
				return i + les_funcDataNumFunctionDefinitions;
			}
		}
	}
	return -1;
}

static int DecodeSingle(const LES_FunctionParameterData* const functionParameterData, 
								 				const int parameterIndex, const int nameID, const int typeID, const int parentParameterIndex,
												const int depth)
{
	const LES_StringEntry* const nameEntry = LES_GetStringEntryForID(nameID);
	const LES_StringEntry* typeStringEntry = LES_GetStringEntryForID(typeID);
	const LES_TypeEntry* typeEntryPtr = LES_GetTypeEntry(typeStringEntry);

#if LES_FUNCTION_DEBUG
	LES_LOG("DecodeSingle: '%s' '%s' %d", nameEntry->m_str, typeStringEntry->m_str, typeID);
#endif // #if LES_FUNCTION_DEBUG

	const char* const nameStr = nameEntry->m_str;
	const char* const typeStr = typeStringEntry->m_str;

	if (typeEntryPtr == LES_NULL)
	{
		LES_WARNING("DecodeSingle parameter[%d]:'%s' type:'%s' type can't be found", parameterIndex, nameStr, typeStr);
		return LES_RETURN_ERROR;
	}
	const int numElements = typeEntryPtr->m_numElements;
	int aliasedTypeID = typeEntryPtr->m_aliasedTypeID;

	unsigned int typeFlags = typeEntryPtr->m_flags;

	typeEntryPtr = typeEntryPtr->GetRootType();
	aliasedTypeID = typeEntryPtr->m_aliasedTypeID;
	typeFlags = typeEntryPtr->m_flags;
	typeStringEntry = LES_GetStringEntryForID(aliasedTypeID);

	const int typeDataSize = typeEntryPtr->m_dataSize;
	if (typeFlags & LES_TYPE_STRUCT)
	{
		int localNumElements = numElements;
		if (localNumElements < 1)
		{
			localNumElements = 1;
		}

#if LES_FUNCTION_DEBUG
		LES_LOG("DecodeSingle parameter[%d]:'%s' type:'%s' size:%d ARRAY numELements:%d", 
						parameterIndex, nameStr, typeStr, typeDataSize, numElements);
		LES_LOG("DecodeSingle parameter[%d]:'%s' type:'%s' size:%d STRUCT", parameterIndex, nameStr, typeStr, typeDataSize);
#endif // #if LES_FUNCTION_DEBUG
		int returnCode = LES_RETURN_OK;
		const LES_StructDefinition* const structDefinition = LES_GetStructDefinition(typeEntryPtr->m_hash);
		if (structDefinition == LES_NULL)
		{
			LES_WARNING("DecodeSingle parameter[%d]:'%s' type:'%s' is a struct but can't be found", parameterIndex, nameStr, typeStr);
			return LES_RETURN_ERROR;
		}
		const int numMembers = structDefinition->GetNumMembers();
		const int newDepth = depth + 1;
		for (int e = 0; e < localNumElements; e++)
		{
			for (int i = 0; i < numMembers; i++)
			{
				const LES_StructMember* const structMember = structDefinition->GetMemberByIndex(i);
				const int memberNameID = structMember->m_nameID;
				const int memberTypeID = structMember->m_typeID;
				returnCode = DecodeSingle(functionParameterData, i, memberNameID, memberTypeID, parameterIndex, newDepth);
				if (returnCode != LES_RETURN_OK)
				{
					return LES_RETURN_ERROR;
				}
			}
		}
		return returnCode;
	}
	if (numElements > 0)
	{
		int returnCode = LES_RETURN_OK;
#if LES_FUNCTION_DEBUG
		LES_LOG("DecodeSingle parameter[%d]:'%s' type:'%s' size:%d ARRAY numELements:%d", 
						parameterIndex, nameStr, typeStr, typeDataSize, numElements);
#endif // #if LES_FUNCTION_DEBUG
		const int elementNameID = nameID;
		const int elementTypeID = aliasedTypeID;
		const int newDepth = depth + 1;
		for (int i = 0; i < numElements; i++)
		{
			returnCode = DecodeSingle(functionParameterData, i, elementNameID, elementTypeID, parameterIndex, newDepth);
			if (returnCode != LES_RETURN_OK)
			{
				return LES_RETURN_ERROR;
			}
		}
		return returnCode;
	}

	const LES_Hash typeHash = typeEntryPtr->m_hash;

	long long int longlongValue;
	int intValue;
	short shortValue;
	char charValue;
	float floatValue;

	void* valuePtr = LES_NULL;
	const char* fmtStr = LES_NULL;

	if (typeHash == LES_TypeEntry::s_longlongHash)
	{
		valuePtr = &longlongValue;
		fmtStr = "%ld";
	}
	else if ((typeHash == LES_TypeEntry::s_intHash) || (typeHash == LES_TypeEntry::s_uintHash))
	{
		valuePtr = &intValue;
		fmtStr = "%d";
	}
	else if ((typeHash == LES_TypeEntry::s_shortHash) || (typeHash == LES_TypeEntry::s_ushortHash))
	{
		valuePtr = &shortValue;
		fmtStr = "%d";
	}
	else if ((typeHash == LES_TypeEntry::s_charHash) || (typeHash == LES_TypeEntry::s_ucharHash))
	{
		valuePtr = &charValue;
		fmtStr = "'%c'";
	}
	else if (typeHash == LES_TypeEntry::s_floatHash)
	{
		valuePtr = &floatValue;
		fmtStr = "%f";
	}
	// An unknown type just grab it
	if (valuePtr == LES_NULL)
	{
		valuePtr = new char[typeDataSize];
	}
	if (valuePtr == LES_NULL)
	{
		LES_WARNING("DecodeSingle valuePtr = LES_NULL parameter[%d]:'%s'", parameterIndex, nameStr);
		return LES_RETURN_ERROR;
	}
	int errorCode = functionParameterData->Read(typeStringEntry, valuePtr);
	if (errorCode != LES_RETURN_OK)
	{
		LES_WARNING("DecodeSingle Read failed for parameter[%d]:%s;", parameterIndex, nameStr);
		return LES_RETURN_ERROR;
	}
	char output[1024];
	output[0] = '\0';
	sprintf(output, "%sDecodeSingle ", output);
	for (int i = 0; i < depth*2; i++)
	{
		sprintf(output, "%s ", output);
	}
	sprintf(output, "%s%s", output, "parameter");
	if (parentParameterIndex >= 0)
	{
		sprintf(output, "%s[%d] member", output, parentParameterIndex);
	}
	sprintf(output, "%s[%d]:'%s' type:'%s' value:", output, parameterIndex, nameStr, typeStr);

	char tempString[1024];
	if (typeHash == LES_TypeEntry::s_longlongHash)
	{
		sprintf(tempString, fmtStr, longlongValue);
	}
	else if ((typeHash == LES_TypeEntry::s_intHash) || (typeHash == LES_TypeEntry::s_uintHash))
	{
		sprintf(tempString, fmtStr, intValue);
	}
	else if ((typeHash == LES_TypeEntry::s_shortHash) || (typeHash == LES_TypeEntry::s_ushortHash))
	{
		sprintf(tempString, fmtStr, shortValue);
	}
	else if ((typeHash == LES_TypeEntry::s_charHash) || (typeHash == LES_TypeEntry::s_ucharHash))
	{
		sprintf(tempString, fmtStr, charValue);
	}
	else if (typeHash == LES_TypeEntry::s_floatHash)
	{
		sprintf(tempString, fmtStr, floatValue);
	}
	else
	{
		sprintf(tempString, ":UNKNOWN typeDataSize:%d struct:%d", typeDataSize, (typeEntryPtr->m_flags&LES_TYPE_STRUCT));
	}
	sprintf(output, "%s%s", output, tempString);
	LES_LOG(output);
	return LES_RETURN_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

const LES_FunctionDefinition* LES_GetFunctionDefinition(const char* const name)
{
	const int index = LES_GetFunctionDefinitionIndex(name);
	if (index < 0)
	{
		return LES_NULL;
	}
	const LES_FunctionDefinition* const functionDefinitionPtr = LES_GetFunctionDefinitionForID(index);
	return functionDefinitionPtr;
}

LES_FunctionParameterData* LES_GetFunctionParameterData(const int functionNameID)
{
	const int index = LES_GetFunctionDefinitionIndexByNameID(functionNameID);
	if (index < 0)
	{
		return LES_NULL;
	}
	const LES_FunctionDefinition* const functionDefinitionPtr = LES_GetFunctionDefinitionForID(index);
	const int parameterDataSize = functionDefinitionPtr->GetParameterDataSize();
	char* parameterBuffer = LES_NULL;
	if (parameterDataSize > 0)
	{
		parameterBuffer = new char[parameterDataSize];
	}
	LES_FunctionParameterData* const parameterData = new LES_FunctionParameterData(parameterBuffer);
	return parameterData;
}

int LES_FunctionDefinition::ComputeParameterDataSize(void) const
{
	int parameterDataSize = 0;
	int paramDataSize = 0;
	int numParams = GetNumParameters();
	for (int i = 0; i < numParams; i++)
	{
		const LES_FunctionParameter* const memberPtr = GetParameterByIndex(i);
		const int typeID = memberPtr->m_typeID;
		const LES_StringEntry* const typeStringEntryPtr = LES_GetStringEntryForID(typeID);
		if (typeStringEntryPtr == LES_NULL)
		{
			return -1;
		}
		const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntryPtr);
		if (typeEntryPtr == LES_NULL)
		{
			return -1;
		}
		// This function is recursive following aliases and also looping over the members in structs and handling arrays
		paramDataSize += typeEntryPtr->ComputeDataStorageSize();
	}
	parameterDataSize = paramDataSize;
	return parameterDataSize;
}

int LES_FunctionDefinition::GetNumParameters(void) const 
{
	return (m_numInputs + m_numOutputs);
}

int LES_FunctionDefinition::GetNumInputs(void) const 
{
	return m_numInputs;
}

int LES_FunctionDefinition::GetNumOutputs(void) const
{
	return m_numOutputs;
}

int LES_FunctionDefinition::GetReturnTypeID(void) const
{
	return m_returnTypeID;
}

int LES_FunctionDefinition::GetNameID(void) const
{
	return m_nameID;
}

int LES_FunctionDefinition::GetParameterDataSize(void) const
{
	return m_parameterDataSize;
}

const LES_FunctionParameter* LES_FunctionDefinition::GetParameter(const LES_Hash nameHash) const
{
	const int numParams = m_numInputs + m_numOutputs;
	for (int i = 0; i < numParams; i++)
	{
		const LES_FunctionParameter* const paramPtr = &m_parameters[i];
		if (paramPtr->m_hash == nameHash)
		{
			return paramPtr;
		}
	}
	return LES_NULL;
}

const LES_FunctionParameter* LES_FunctionDefinition::GetParameterByIndex(const int index) const
{
	const int numParams = m_numInputs + m_numOutputs;
	if ((index >= 0) && (index < numParams))
	{
		const LES_FunctionParameter* const paramPtr = &m_parameters[index];
		return paramPtr;
	}
	return LES_NULL;
}

int LES_FunctionDefinition::Decode(const LES_FunctionParameterData* const functionParameterData) const
{
	const int numParams = GetNumParameters();
	int returnCode = LES_RETURN_OK;
	for (int i = 0; i < numParams; i++)
	{
		const LES_FunctionParameter* const functionParameterPtr = GetParameterByIndex(i);
		const int nameID = functionParameterPtr->m_nameID;
		const int typeID = functionParameterPtr->m_typeID;
		returnCode = DecodeSingle(functionParameterData, i, nameID, typeID, -1, 0);
		if (returnCode != LES_RETURN_OK)
		{
			return LES_RETURN_ERROR;
		}
	}
	return returnCode;
}

void LES_DebugOutputFunctionDefinitions(LES_LoggerChannel* const pLogChannel)
{
	const int numFunctionDefinitions = les_funcDataNumFunctionDefinitions + les_numFunctionDefinitions;
	pLogChannel->Print("numFunctionDefinitions:%d FuncData:%d Internal:%d", 
										 numFunctionDefinitions, les_funcDataNumFunctionDefinitions, les_numFunctionDefinitions);
	for (int i = 0; i < numFunctionDefinitions; i++)
	{
		const LES_FunctionDefinition* const pFunctionDefinition = LES_GetFunctionDefinitionForID(i);
		LES_DebugOutputFunctionDefinition(pLogChannel, pFunctionDefinition, i);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_FunctionInit()
{
	les_functionDefinitionArray = new const LES_FunctionDefinition*[1024];
	les_numFunctionDefinitions = 0;
}

void LES_FunctionShutdown()
{
	les_numFunctionDefinitions = 0;
	delete[] les_functionDefinitionArray;
}

int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const pFunctionDefinition, 
															const int parameterDataSize)
{
	int index = LES_GetFunctionDefinitionIndex(name);
	if (index < 0)
	{
		/* Not found so add it - just store the ptr to the memory */
		index = les_numFunctionDefinitions;
		les_functionDefinitionArray[index] = pFunctionDefinition;
		LES_FunctionDefinition* const pFunctionDefinition2 = (LES_FunctionDefinition* const)les_functionDefinitionArray[index];
		pFunctionDefinition2->m_parameterDataSize = parameterDataSize;
		les_numFunctionDefinitions++;
	}
	else
	{
		const LES_FunctionDefinition* const pFunctionDefinition2 = LES_GetFunctionDefinitionForID(index);
		if (pFunctionDefinition2->GetParameterDataSize() != parameterDataSize)
		{
			LES_ERROR("AddFunctionDefinition '%s' parameterDataSize doesn't match FuncData:%d Code:%d", name, 
								pFunctionDefinition2->GetParameterDataSize(), parameterDataSize);
			return -1;
		}
	}

	return index;
}

LES_FunctionDefinition* LES_CreateFunctionDefinition(const int nameID, const int returnTypeID, const int numInputs, const int numOutputs)
{
	if (numInputs < 0)
	{
		LES_FATAL_ERROR("LES_CreateFunctionDefinition nameID:%d invalid numInputs %d must be >= 0", nameID, numInputs);
		return LES_NULL;
	}
	if (numOutputs < 0)
	{
		LES_FATAL_ERROR("LES_CreateFunctionDefinition nameID:%d invalid numOutputs %d must be >= 0", nameID, numOutputs);
		return LES_NULL;
	}
	const int numParams = numInputs + numOutputs;
	int memorySize = sizeof(LES_FunctionDefinition);
	if (numParams > 1)
	{
		memorySize += sizeof(LES_FunctionParameter) * (numParams - 1);
	}
	LES_FunctionDefinition* const pFunctionDefinition = (LES_FunctionDefinition*)malloc(memorySize);
	pFunctionDefinition->m_nameID = nameID;
	pFunctionDefinition->m_returnTypeID = returnTypeID;
	pFunctionDefinition->m_parameterDataSize = 0;
	pFunctionDefinition->m_numInputs = numInputs;
	pFunctionDefinition->m_numOutputs = numOutputs;

	LES_FunctionParameter emptyFunctionParameter;
	emptyFunctionParameter.m_hash = 0;
	emptyFunctionParameter.m_nameID = -1;
	emptyFunctionParameter.m_typeID = -1;
	emptyFunctionParameter.m_index = -1;
	emptyFunctionParameter.m_mode = 0;

	for (int i = 0; i < numParams; i++)
	{
		LES_FunctionParameter* const pFunctionParameter = (LES_FunctionParameter* const)pFunctionDefinition->GetParameterByIndex(i);
		*pFunctionParameter = emptyFunctionParameter;
	}
	return pFunctionDefinition;
}

void LES_DebugOutputFunctionDefinition(LES_LoggerChannel* const pLogChannel, 
																			 const LES_FunctionDefinition* const pFunctionDefinition, const int i)
{
	const int functionNameID = pFunctionDefinition->GetNameID();
	const LES_StringEntry* const pFunctionNameEntry = LES_GetStringEntryForID(functionNameID);
	const char* const functionName = pFunctionNameEntry->m_str;

	const int numInputs = pFunctionDefinition->GetNumInputs();
	const int numOutputs = pFunctionDefinition->GetNumOutputs();
	const int numParameters = pFunctionDefinition->GetNumParameters();
	const int returnTypeID = pFunctionDefinition->GetReturnTypeID();
	const LES_StringEntry* const pReturnTypeNameEntry = LES_GetStringEntryForID(returnTypeID);
	const char* const returnTypeName = pReturnTypeNameEntry ? pReturnTypeNameEntry->m_str : "NULL";
	const int parameterDataSize = pFunctionDefinition->GetParameterDataSize();

	pLogChannel->Print("Function[%d] '%s' returnType '%s' numParameters:%d numInputs:%d numOutputs:%d parameterDataSize:%d", 
										 i, functionName, returnTypeName, numParameters, numInputs, numOutputs, parameterDataSize);

	for (int p = 0; p < numParameters; p++)
	{
		const LES_FunctionParameter* const pFunctionParameter = pFunctionDefinition->GetParameterByIndex(p);

		const LES_Hash hashValue = pFunctionParameter->m_hash;
		const int nameID = pFunctionParameter->m_nameID;
		const int typeID = pFunctionParameter->m_typeID;
		const int index = pFunctionParameter->m_index;
		const int mode = pFunctionParameter->m_mode;

		const LES_StringEntry* const pParameterNameEntry = LES_GetStringEntryForID(nameID);
		const char* const memberName = pParameterNameEntry ? pParameterNameEntry->m_str : "NULL";
		const LES_StringEntry* const pTypeNameEntry = LES_GetStringEntryForID(typeID);
		const char* const typeName = pTypeNameEntry ? pTypeNameEntry->m_str : "NULL";
		char flagsDecoded[1024];
		LES_Type_DecodeFlags(flagsDecoded, mode);
		pLogChannel->Print("  Function '%s' Member[%d] '%s' 0x%X Type:'%s' index:%d mode:0x%X %s", 
											 functionName, p, memberName, hashValue, typeName, index, mode, flagsDecoded);
	}
}

void LES_Function_SetFuncDataPtr(const LES_FuncData* const pFuncData)
{
	les_pFuncData = pFuncData;
	les_funcDataNumFunctionDefinitions = les_pFuncData->GetNumFunctionDefinitions();
}

