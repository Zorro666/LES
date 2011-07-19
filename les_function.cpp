#include <string.h>

#include "les_core.h"
#include "les_log.h"
#include "les_function.h"
#include "les_function_macros.h"
#include "les_stringentry.h"
#include "les_parameter.h"
#include "les_struct.h"

static LES_FunctionDefinition* les_functionDefinitionArray = LES_NULL;
static int les_numFunctionDefinitions = 0;

#define LES_FUNCTION_DEBUG 0

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_GetFunctionDefinitionIndexByNameID(const int nameID)
{
	/* This is horribly slow - need hash lookup table */
	for (int i=0; i<les_numFunctionDefinitions; i++)
	{
		const LES_FunctionDefinition* const functionDefinitionPtr = &les_functionDefinitionArray[i];
		if (functionDefinitionPtr->GetNameID() == nameID)
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
		const LES_StringEntry* const functionNameStringEntryPtr = LES_GetStringEntryForID(functionDefinitionPtr->GetNameID());
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

static int DecodeSingle(const LES_FunctionParameterData* const functionParameterData, 
								 				const int parameterIndex, const int nameID, const int typeID, const int parentParameterIndex,
												const int depth)
{
	const LES_StringEntry* const nameEntry = LES_GetStringEntryForID(nameID);
	const LES_StringEntry* typeEntry = LES_GetStringEntryForID(typeID);
	const LES_TypeEntry* typeDataPtr = LES_GetTypeEntry(typeEntry);

#if LES_FUNCTION_DEBUG
	LES_LOG("DecodeSingle: '%s' '%s' %d\n", nameEntry->m_str, typeEntry->m_str, typeID);
#endif // #if LES_FUNCTION_DEBUG

	const char* const nameStr = nameEntry->m_str;
	const char* const typeStr = typeEntry->m_str;

	if (typeDataPtr == LES_NULL)
	{
		LES_WARNING("DecodeSingle parameter[%d]:'%s' type:'%s' type can't be found\n", parameterIndex, nameStr, typeStr);
		return LES_RETURN_ERROR;
	}

	unsigned int typeFlags = typeDataPtr->m_flags;
	while ( typeFlags & LES_TYPE_ALIAS)
	{
		const int aliasedTypeID = typeDataPtr->m_aliasedTypeID;
		const LES_StringEntry* const aliasedTypeEntry = LES_GetStringEntryForID(aliasedTypeID);
		if (aliasedTypeEntry == LES_NULL)
		{
			LES_WARNING("DecodeSingle parameter[%d]:'%s' type:'%s' aliased type:%d entry can't be found\n", 
									parameterIndex, nameStr, typeStr, aliasedTypeID);
			return LES_RETURN_ERROR;
		}
#if LES_FUNCTION_DEBUG
		LES_LOG("Type:'%s' aliased to '%s'\n", typeEntry->m_str, aliasedTypeEntry->m_str);
#endif // #if LES_FUNCTION_DEBUG
		typeDataPtr = LES_GetTypeEntry(aliasedTypeEntry);
		if (typeDataPtr == LES_NULL)
		{
			LES_WARNING("DecodeSingle parameter[%d]:'%s' type:'%s' type can't be found\n", parameterIndex, nameStr, typeStr);
			return LES_RETURN_ERROR;
		}
		typeFlags = typeDataPtr->m_flags;
		typeEntry = aliasedTypeEntry;
	}

	const int typeDataSize = typeDataPtr->m_dataSize;
	if (typeFlags & LES_TYPE_STRUCT)
	{
		LES_LOG("DecodeSingle parameter[%d]:'%s' type:'%s' size:%d STRUCT\n", parameterIndex, nameStr, typeStr, typeDataSize);
		int returnCode = LES_RETURN_OK;
		const LES_StructDefinition* const structDefinition = LES_GetStructDefinition(typeDataPtr->m_hash);
		if (structDefinition == LES_NULL)
		{
			LES_WARNING("DecodeSingle parameter[%d]:'%s' type:'%s' is a struct but can't be found\n", parameterIndex, nameStr, typeStr);
			return LES_RETURN_ERROR;
		}
		const int numMembers = structDefinition->GetNumMembers();
		const int newDepth = depth + 1;
		for (int i = 0; i < numMembers; i++)
		{
			const LES_StructMember* const structMember = structDefinition->GetMemberByIndex(i);
			const int memberNameID = structMember->m_nameID;
			const int memberTypeID = structMember->m_typeID;
			returnCode = DecodeSingle(functionParameterData, i, memberNameID, memberTypeID, parameterIndex, newDepth);
			if (returnCode == LES_RETURN_ERROR)
			{
				return LES_RETURN_ERROR;
			}
		}
		return returnCode;
	}
	const LES_Hash typeHash = typeDataPtr->m_hash;

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
		LES_WARNING("DecodeSingle valuePtr = LES_NULL parameter[%d]:'%s'\n", parameterIndex, nameStr);
		return LES_RETURN_ERROR;
	}
	int errorCode = functionParameterData->Read(typeEntry, valuePtr);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_WARNING("DecodeSingle Read failed for parameter[%d]:%s;\n", parameterIndex, nameStr);
		return LES_RETURN_ERROR;
	}
	LES_LOG("DecodeSingle ");
	for (int i = 0; i < depth*2; i++)
	{
		LES_LOG(" ");
	}
	LES_LOG("parameter");
	if (parentParameterIndex >= 0)
	{
		LES_LOG("[%d] member", parentParameterIndex);
	}
	LES_LOG("[%d]:'%s' type:'%s' value:", parameterIndex, nameStr, typeStr);

	if (typeHash == LES_TypeEntry::s_longlongHash)
	{
		LES_LOG(fmtStr, longlongValue);
	}
	else if ((typeHash == LES_TypeEntry::s_intHash) || (typeHash == LES_TypeEntry::s_uintHash))
	{
		LES_LOG(fmtStr, intValue);
	}
	else if ((typeHash == LES_TypeEntry::s_shortHash) || (typeHash == LES_TypeEntry::s_ushortHash))
	{
		LES_LOG(fmtStr, shortValue);
	}
	else if ((typeHash == LES_TypeEntry::s_charHash) || (typeHash == LES_TypeEntry::s_ucharHash))
	{
		LES_LOG(fmtStr, charValue);
	}
	else if (typeHash == LES_TypeEntry::s_floatHash)
	{
		LES_LOG(fmtStr, floatValue);
	}
	else
	{
		LES_LOG(":UNKNOWN typeDataSize:%d struct:%d", typeDataSize, (typeDataPtr->m_flags&LES_TYPE_STRUCT));
	}
	LES_LOG("\n");
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
	if ((index < 0) || (index >= les_numFunctionDefinitions))
	{
		return LES_NULL;
	}
	const LES_FunctionDefinition* const functionDefinitionPtr = &les_functionDefinitionArray[index];
	return functionDefinitionPtr;
}

LES_FunctionParameterData* LES_GetFunctionParameterData(const int functionNameID)
{
	const int index = LES_GetFunctionDefinitionIndexByNameID(functionNameID);
	if ((index < 0) || (index >= les_numFunctionDefinitions))
	{
		return LES_NULL;
	}
	const LES_FunctionDefinition* const functionDefinitionPtr = &les_functionDefinitionArray[index];
	const int parameterDataSize = functionDefinitionPtr->GetParameterDataSize();
	//char* parameterBuffer = new char[parameterDataSize + 16 * 1024];
	char* parameterBuffer = new char[parameterDataSize];
	LES_FunctionParameterData* parameterData = new LES_FunctionParameterData(parameterBuffer);

	return parameterData;
}

LES_FunctionDefinition::LES_FunctionDefinition(void)
{
	m_nameID = -1;
	m_returnTypeID = -1;
	m_parameterDataSize = 0;

	m_numInputs = 0;
	m_numOutputs = 0;
	m_params = LES_NULL;
	m_ownsParamsMemory = false;
}

LES_FunctionDefinition::LES_FunctionDefinition(const LES_FunctionDefinition& other)
{
	*this = other;
}

LES_FunctionDefinition& LES_FunctionDefinition::operator = (const LES_FunctionDefinition& other)
{
	m_nameID = other.m_nameID;
	m_returnTypeID = other.m_returnTypeID;
	m_parameterDataSize = other.m_parameterDataSize;

	m_numInputs = other.m_numInputs;
	m_numOutputs = other.m_numOutputs;
	m_params = other.m_params;
	m_ownsParamsMemory = true;
	other.m_ownsParamsMemory = false;

	return *this;
}

LES_FunctionDefinition::LES_FunctionDefinition(const int nameID, const int returnTypeID, const int numInputs, const int numOutputs)
{
	m_nameID = nameID;
	m_returnTypeID = returnTypeID;
	m_parameterDataSize = 0;

	m_numInputs = numInputs;
	m_numOutputs = numOutputs;
	m_params = new LES_FunctionParameter[numInputs+numOutputs];
	m_ownsParamsMemory = true;
}

LES_FunctionDefinition::~LES_FunctionDefinition(void)
{
	if (m_ownsParamsMemory)
	{
		delete[] m_params;
	}
	m_nameID = -1;
	m_returnTypeID = -1;
	m_parameterDataSize = 0;

	m_numInputs = 0;
	m_numOutputs = 0;
	m_params = LES_NULL;
	m_ownsParamsMemory = false;
}

void LES_FunctionDefinition::ComputeParameterDataSize(void)
{
	m_parameterDataSize = 0;
	int paramDataSize = 0;
	int numParams = GetNumParameters();
	for (int i = 0; i < numParams; i++)
	{
		const LES_FunctionParameter* const memberPtr = GetParameterByIndex(i);
		const int typeID = memberPtr->m_typeID;
		const LES_StringEntry* const typeStringEntryPtr = LES_GetStringEntryForID(typeID);
		if (typeStringEntryPtr == LES_NULL)
		{
			return;
		}
		const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntryPtr);
		if (typeEntryPtr == LES_NULL)
		{
			return;
		}
		// Need to make this recursively on structs, looping over the members
		paramDataSize += typeEntryPtr->ComputeDataStorageSize();
	}
	m_parameterDataSize = paramDataSize;
}

void LES_FunctionDefinition::SetReturnTypeID(const int returnTypeID)
{
	m_returnTypeID = returnTypeID;
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
		const LES_FunctionParameter* const paramPtr = &m_params[i];
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
		const LES_FunctionParameter* const paramPtr = &m_params[index];
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
		if (returnCode == LES_RETURN_ERROR)
		{
			return LES_RETURN_ERROR;
		}
	}
	return returnCode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_FunctionInit()
{
	les_functionDefinitionArray = new LES_FunctionDefinition[1024];
	les_numFunctionDefinitions = 0;
}

void LES_FunctionShutdown()
{
	les_numFunctionDefinitions = 0;
	delete[] les_functionDefinitionArray;
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

