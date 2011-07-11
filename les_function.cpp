#include <string.h>

#include "les_core.h"
#include "les_function.h"
#include "les_function_macros.h"
#include "les_stringentry.h"
#include "les_parameter.h"

static LES_FunctionDefinition* les_functionDefinitionArray = LES_NULL;
static int les_numFunctionDefinitions = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal static functions
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
}

LES_FunctionDefinition::LES_FunctionDefinition(const int nameID, const int returnTypeID, const int numInputs, const int numOutputs)
{
	m_nameID = nameID;
	m_returnTypeID = returnTypeID;
	m_numInputs = numInputs;
	m_numOutputs = numOutputs;
	m_params = new LES_FunctionParameter[numInputs+numOutputs];
}

void LES_FunctionDefinition::SetParameterDataSize(const int parameterDataSize)
{
	m_parameterDataSize = parameterDataSize;
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

const LES_FunctionParameter* LES_FunctionDefinition::GetParameter(const LES_Hash hash) const
{
	const int numParams = m_numInputs + m_numOutputs;
	for (int i = 0; i < numParams; i++)
	{
		const LES_FunctionParameter* const paramPtr = &m_params[i];
		if (paramPtr->m_hash == hash)
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
	for (int i = 0; i < numParams; i++)
	{
		const LES_FunctionParameter* const functionParameterPtr = GetParameterByIndex(i);
		const int nameID = functionParameterPtr->m_nameID;
		const int typeID = functionParameterPtr->m_typeID;
		const LES_StringEntry* const nameEntry = LES_GetStringEntryForID(nameID);
		const LES_StringEntry* const typeEntry = LES_GetStringEntryForID(typeID);

		const char* const nameStr = nameEntry->m_str;
		const LES_Hash typeHash = typeEntry->m_hash;
		const char* const typeStr = typeEntry->m_str;
			
		int intValue;
		short shortValue;
		char charValue;
		float floatValue;

		void* valuePtr = LES_NULL;
		const char* fmtStr = LES_NULL;

		if ((typeHash == LES_TypeEntry::s_intHash) || 
				(typeHash == LES_TypeEntry::s_intPtrHash) || 
				(typeHash == LES_TypeEntry::s_unsignedintPtrHash))
		{
			valuePtr = &intValue;
			fmtStr = "%d";
		}
		else if ((typeHash == LES_TypeEntry::s_shortHash) || 
						 (typeHash == LES_TypeEntry::s_unsignedshortPtrHash))
		{
			valuePtr = &shortValue;
			fmtStr = "%d";
		}
		else if ((typeHash == LES_TypeEntry::s_charHash) || 
						 (typeHash == LES_TypeEntry::s_unsignedcharPtrHash))
		{
			valuePtr = &charValue;
			fmtStr = "%d";
		}
		else if ((typeHash == LES_TypeEntry::s_floatHash) || 
						 (typeHash == LES_TypeEntry::s_floatPtrHash))
		{
			valuePtr = &floatValue;
			fmtStr = "%f";
		}
		if (valuePtr == LES_NULL)
		{
			fprintf(stderr, "LES ERROR: LES_FunctionDefinition::Decode valuePtr = LES_NULL parameter[%d]:'%s'\n", i, nameStr);
			return LES_ERROR;
		}
		int errorCode = functionParameterData->Read(typeEntry, valuePtr);
		if (errorCode == LES_ERROR)
		{
			fprintf(stderr, "LES ERROR: LES_FunctionDefinition::Decode Read failed for parameter[%d]:%s;\n", i, nameStr);
			return LES_ERROR;
		}
		printf("LES_FunctionDefinition::Decode parameter[%d]:'%s' type:'%s' value:", i, nameStr, typeStr);
		if ((typeHash == LES_TypeEntry::s_intHash) || 
				(typeHash == LES_TypeEntry::s_intPtrHash) || 
				(typeHash == LES_TypeEntry::s_unsignedintPtrHash))
		{
			printf(fmtStr, intValue);
		}
		else if ((typeHash == LES_TypeEntry::s_shortHash) || 
						 (typeHash == LES_TypeEntry::s_unsignedshortPtrHash))
		{
			printf(fmtStr, shortValue);
		}
		else if ((typeHash == LES_TypeEntry::s_charHash) || 
						 (typeHash == LES_TypeEntry::s_unsignedcharPtrHash))
		{
			printf(fmtStr, charValue);
		}
		else if ((typeHash == LES_TypeEntry::s_floatHash) || 
						 (typeHash == LES_TypeEntry::s_floatPtrHash))
		{
			printf(fmtStr, floatValue);
		}
		printf("\n");
	}
	return LES_OK;
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

