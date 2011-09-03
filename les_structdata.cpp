#include "les_structdata.h"
#include "les_logger.h"
#include "les_stringentry.h"

int LES_StructData::Settle(void)
{
	if (m_settled != 0)
	{
		LES_LOG("m_settled != 0");
		return LES_RETURN_ERROR;
	}

	// Convert from little endian first
	m_numStructDefinitions = fromLittleEndian32(m_numStructDefinitions);
	m_settled = fromLittleEndian32(m_settled);

	const char* const basePtr = (char* const)&m_numStructDefinitions;
	const int numStructDefinitions = m_numStructDefinitions;
	// Settle the struct definition offsets
	for (int i = 0; i < numStructDefinitions; i++)
	{
		const int offset = fromLittleEndian32(m_structDefinitionOffsets[i]);
		const char* const pStruct = basePtr + offset;
		LES_LOG("offset = %d %p", offset, pStruct);
		m_structDefinitionOffsets[i] = (LES_uint32)pStruct;
		
		// Settle the struct definition
		LES_StructDefinition* const pStructDefinition  = (LES_StructDefinition* const)pStruct;
		pStructDefinition->m_nameID = fromLittleEndian32(pStructDefinition->m_nameID);
		pStructDefinition->m_numMembers = fromLittleEndian32(pStructDefinition->m_numMembers);
		// Settle the struct members
		const int numMembers = pStructDefinition->m_numMembers;
		for (int m = 0; m < numMembers; m++)
		{
			LES_StructMember* const pStructMember = &pStructDefinition->m_members[m];
			pStructMember->m_hash = fromLittleEndian32(pStructMember->m_hash);
			pStructMember->m_nameID = fromLittleEndian32(pStructMember->m_nameID);
			pStructMember->m_typeID = fromLittleEndian32(pStructMember->m_typeID);
			pStructMember->m_dataSize = fromLittleEndian32(pStructMember->m_dataSize);
			pStructMember->m_alignmentPadding = fromLittleEndian32(pStructMember->m_alignmentPadding);
		}
	}

	m_settled = 1;
	return LES_RETURN_OK;
}
