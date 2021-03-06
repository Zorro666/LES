#include "les_funcdata.h"
#include "les_core.h"
#include "les_logger.h"
#include "les_stringentry.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

int LES_FuncData::GetFunctionDefinitionIndex(const LES_Hash nameHash) const
{
	/* This is horribly slow - need hash lookup table */
	const int numFunctionDefinitions = m_numFunctionDefinitions;
	for (int i = 0; i < numFunctionDefinitions; i++)
	{
		const LES_FunctionDefinition* const pFunctionDefinition = GetFunctionDefinition(i);
		const int nameID = pFunctionDefinition->GetNameID();
		const LES_StringEntry* const pFunctionNameStringEntry = LES_GetStringEntryForID(nameID);
		if (pFunctionNameStringEntry->m_hash == nameHash)
		{
			return i;
		}
	}
	return -1;
}

int LES_FuncData::GetFunctionDefinitionIndexByNameID(const int nameID) const
{
	/* This is horribly slow - need hash lookup table */
	const int numFunctionDefinitions = m_numFunctionDefinitions;
	for (int i = 0; i < numFunctionDefinitions; i++)
	{
		const LES_FunctionDefinition* const pFunctionDefinition = GetFunctionDefinition(i);
		if (pFunctionDefinition->GetNameID() == nameID)
		{
			return i;
		}
	}
	return -1;
}

int LES_FuncData::Settle(void)
{
	if (m_settled != 0)
	{
		LES_LOG("m_settled != 0");
		return LES_RETURN_ERROR;
	}

	// Convert from little endian first
	m_numFunctionDefinitions = fromBigEndian32(m_numFunctionDefinitions);
	m_settled = fromBigEndian32(m_settled);

	const char* const basePtr = (char* const)&m_numFunctionDefinitions;
	const int numFunctionDefinitions = m_numFunctionDefinitions;
	// Settle the function definition offsets
	for (int i = 0; i < numFunctionDefinitions; i++)
	{
		const int offset = fromBigEndian32(m_functionDefinitionOffsets[i]);
		const char* const pFunction = basePtr + offset;
		m_functionDefinitionOffsets[i] = (LES_uint32)pFunction;
		
		// Settle the function definition
		LES_FunctionDefinition* const pFunctionDefinition  = (LES_FunctionDefinition* const)pFunction;
		pFunctionDefinition->m_nameID = fromBigEndian32(pFunctionDefinition->m_nameID);
		pFunctionDefinition->m_returnTypeID = fromBigEndian32(pFunctionDefinition->m_returnTypeID);
		pFunctionDefinition->m_parameterDataSize = fromBigEndian32(pFunctionDefinition->m_parameterDataSize);
		pFunctionDefinition->m_numInputs = fromBigEndian32(pFunctionDefinition->m_numInputs);
		pFunctionDefinition->m_numOutputs = fromBigEndian32(pFunctionDefinition->m_numOutputs);
		// Settle the function parameters
		const int numParameters = pFunctionDefinition->GetNumParameters();
		for (int m = 0; m < numParameters; m++)
		{
			LES_FunctionParameter* const pFunctionParameter = &pFunctionDefinition->m_parameters[m];
			pFunctionParameter->m_hash = fromBigEndian32(pFunctionParameter->m_hash);
			pFunctionParameter->m_nameID = fromBigEndian32(pFunctionParameter->m_nameID);
			pFunctionParameter->m_typeID = fromBigEndian32(pFunctionParameter->m_typeID);
			pFunctionParameter->m_index = fromBigEndian32(pFunctionParameter->m_index);
			pFunctionParameter->m_mode = fromBigEndian32(pFunctionParameter->m_mode);
		}
	}

	m_settled = 1;
	return LES_RETURN_OK;
}
