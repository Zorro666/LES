#include <string.h>
#include <malloc.h>

#include "les_struct.h"
#include "les_core.h"
#include "les_logger.h"
#include "les_loggerchannel.h"
#include "les_stringentry.h"
#include "les_structdata.h"

static const LES_StructDefinition** les_structDefinitionArray = LES_NULL;
static int les_numStructDefinitions = 0;

static const LES_StructData* les_pStructData = LES_NULL;
static int les_structDataNumStructDefinitions = 0;

void LES_DebugOutputStructDefinition(LES_LoggerChannel* const pLogChannel, 
																		 const LES_StructDefinition* const pStructDefinition, const int i);

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_GetStructDefinitionIndex(const LES_Hash nameHash)
{
	if (les_pStructData)
	{
		const int index = les_pStructData->GetStructDefinitionIndex(nameHash);
		if (index >= 0)
		{
			return index;
		}
	}

	/* This is horribly slow - need hash lookup table */
	for (int i = 0; i < les_numStructDefinitions; i++)
	{
		const LES_StructDefinition* const structDefinitionPtr = les_structDefinitionArray[i];
		const LES_StringEntry* const structNameStringEntryPtr = LES_GetStringEntryForID(structDefinitionPtr->GetNameID());
		if (structNameStringEntryPtr->m_hash == nameHash)
		{
			return i + les_structDataNumStructDefinitions;
		}
	}
	return -1;
}

static const LES_StructDefinition* LES_GetStructDefinitionForID(const int id)
{
	if (id < 0)
	{
		return LES_NULL;
	}
	const int index = (id - les_structDataNumStructDefinitions);
	if (index < 0)
	{
		// Get it from definition file type data
		const LES_StructDefinition* const pStructDefinition = les_pStructData->GetStructDefinition(id);
		return pStructDefinition;
	}
	const LES_StructDefinition* const pStructDefinition = les_structDefinitionArray[index];
	return pStructDefinition;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// External Public functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

int LES_StructDefinition::GetNameID(void) const
{
	return m_nameID;
}

int LES_StructDefinition::GetNumMembers(void) const
{
	return m_numMembers;
}

const LES_StructMember* LES_StructDefinition::GetMemberByIndex(const int index) const
{
	if ((index >= 0) && (index < m_numMembers))
	{
		const LES_StructMember* const memberPtr = &m_members[index];
		return memberPtr;
	}
	return LES_NULL;
}

const LES_StructMember* LES_StructDefinition::GetMember(const LES_Hash nameHash) const
{
	const int numMembers = m_numMembers;
	for (int i = 0; i < numMembers; i++)
	{
		const LES_StructMember* const memberPtr = &m_members[i];
		if (memberPtr->m_hash == nameHash)
		{
			return memberPtr;
		}
	}
	return LES_NULL;
}

const LES_StructDefinition* LES_GetStructDefinition(const LES_Hash nameHash)
{
	const int index = LES_GetStructDefinitionIndex(nameHash);
	const LES_StructDefinition* const structDefinitionPtr = LES_GetStructDefinitionForID(index);
	return structDefinitionPtr;
}

void LES_DebugOutputStructs(LES_LoggerChannel* const pLogChannel)
{
	const int numStructDefinitions = les_structDataNumStructDefinitions + les_numStructDefinitions;
	for (int i = 0; i < numStructDefinitions; i++)
	{
		const LES_StructDefinition* const pStructDefinition = LES_GetStructDefinitionForID(i);
		LES_DebugOutputStructDefinition(pLogChannel, pStructDefinition, i);
	}
}

void LES_Struct_SetStructDataPtr(const LES_StructData* const pStructData)
{
	les_pStructData = pStructData;
	const int numStructDefintiions = pStructData->GetNumStructDefinitions();
	les_structDataNumStructDefinitions = numStructDefintiions;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_StructInit()
{
	les_structDefinitionArray = new const LES_StructDefinition*[1024];
	les_numStructDefinitions = 0;
}

void LES_StructShutdown()
{
	les_numStructDefinitions = 0;
	for (int i = 0; i < les_numStructDefinitions; i++)
	{
		// Free the memory because of special way memory is done for these structures
		void* memoryPtr = (void*)les_structDefinitionArray[i];
		free(memoryPtr);
	}
	delete[] les_structDefinitionArray;
}

int LES_AddStructDefinition(const char* const name, const LES_StructDefinition* const structDefinitionPtr)
{
	const LES_Hash nameHash = LES_GenerateHashCaseSensitive(name);
	int index = LES_GetStructDefinitionIndex(nameHash);
	if (index < 0)
	{
		/* Not found so add it - just store the ptr to the memory */
		index = les_numStructDefinitions;
		les_structDefinitionArray[index] = structDefinitionPtr;
		les_numStructDefinitions++;
	}
	else
	{
		// Free the memory because of special way memory is done for these structures
		void* memoryPtr = (void*)structDefinitionPtr;
		free(memoryPtr);
	}

	return index;
}

int LES_StructComputeAlignmentPadding(const int totalMemberSize, const int memberDataSize)
{
	int memberAlignmentSizeMinus1 = memberDataSize - 1;
	// Max alignment is 4 bytes
	if (memberAlignmentSizeMinus1 > 3)
	{
		memberAlignmentSizeMinus1 = 3;
	}
	const int alignedOffset = ((totalMemberSize + memberAlignmentSizeMinus1) & ~memberAlignmentSizeMinus1);
	const int alignmentPadding = alignedOffset - totalMemberSize;

	//LES_LOG("TotalMemberSize:0x%X MemberDataSize:0x%X Alignment:%d", totalMemberSize, memberDataSize, alignmentPadding);
	if (alignmentPadding < 0)
	{
		LES_FATAL_ERROR("HELP");
	}

	return alignmentPadding;
}

LES_StructDefinition* LES_CreateStructDefinition(const int nameID, const int numMembers)
{
	if (numMembers < 1)
	{
		LES_FATAL_ERROR("LES_CreateStructDefinition nameID:%d invalid numMembers %d must be > 0", nameID, numMembers);
		return LES_NULL;
	}
	int memorySize = sizeof(LES_StructDefinition);
	memorySize += sizeof(LES_StructMember) * numMembers - 1;
	LES_StructDefinition* const structDefinitionPtr = (LES_StructDefinition*)malloc(memorySize);
	structDefinitionPtr->m_nameID = nameID;
	structDefinitionPtr->m_numMembers = numMembers;
	LES_StructMember emptyStructMember;
	emptyStructMember.m_hash = 0;
	emptyStructMember.m_nameID = -1;
	emptyStructMember.m_typeID = -1;
	emptyStructMember.m_dataSize = 0;
	emptyStructMember.m_alignmentPadding = 0;
	for (int i = 0; i < numMembers; i++)
	{
		LES_StructMember* const pStructMember = (LES_StructMember* const)structDefinitionPtr->GetMemberByIndex(i);
		*pStructMember = emptyStructMember;
	}
	return structDefinitionPtr;
}

void LES_DebugOutputStructDefinition(LES_LoggerChannel* const pLogChannel, const LES_StructDefinition* const pStructDefinition, 
																		 const int i)
{
	const int structNameID = pStructDefinition->GetNameID();
	const LES_StringEntry* const pStructNameStringEntry = LES_GetStringEntryForID(structNameID);

	const char* const structName = pStructNameStringEntry->m_str;
	const int numMembers = pStructDefinition->GetNumMembers();
	pLogChannel->Print("Struct[%d] '%s' numMembers[%d]", i, structName, numMembers);
	for (int m = 0; m < numMembers; m++)
	{
		const LES_StructMember* const pStructMember = pStructDefinition->GetMemberByIndex(m);

		const LES_Hash hash = pStructMember->m_hash;
		const int nameID = pStructMember->m_nameID;
		const int typeID = pStructMember->m_typeID;
		const int dataSize = pStructMember->m_dataSize;
		const int alignmentPadding = pStructMember->m_alignmentPadding;

		const LES_StringEntry* const pMemberNameStringEntry = LES_GetStringEntryForID(nameID);
		const char* const memberName = pMemberNameStringEntry->m_str;
		const LES_StringEntry* const pTypeNameStringEntry = LES_GetStringEntryForID(typeID);
		const char* const typeName = pTypeNameStringEntry->m_str;
		pLogChannel->Print("  Struct '%s' Member[%d] Type:'%s' Name:'%s' 0x%X size:%d alignmentPadding:%d", 
											 structName, m, typeName, memberName, hash, dataSize, alignmentPadding);
	}
}
