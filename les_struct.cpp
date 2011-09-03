#include <string.h>
#include <malloc.h>

#include "les_struct.h"
#include "les_core.h"
#include "les_logger.h"
#include "les_loggerchannel.h"
#include "les_stringentry.h"

static const LES_StructDefinition** les_structDefinitionArray = LES_NULL;
static int les_numStructDefinitions = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_GetStructDefinitionIndex(const LES_Hash nameHash)
{
	/* This is horribly slow - need hash lookup table */
	for (int i = 0; i < les_numStructDefinitions; i++)
	{
		const LES_StructDefinition* const structDefinitionPtr = les_structDefinitionArray[i];
		const LES_StringEntry* const structNameStringEntryPtr = LES_GetStringEntryForID(structDefinitionPtr->GetNameID());
		if (structNameStringEntryPtr->m_hash == nameHash)
		{
			return i;
		}
	}
	return -1;
}

static const LES_StructDefinition* LES_GetStructDefinitionByID(const int index)
{
	if ((index < 0) || (index >= les_numStructDefinitions))
	{
		return LES_NULL;
	}
	const LES_StructDefinition* const structDefinitionPtr = les_structDefinitionArray[index];
	return structDefinitionPtr;
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
	const LES_StructDefinition* const structDefinitionPtr = LES_GetStructDefinitionByID(index);
	return structDefinitionPtr;
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
	if ((index < 0) || (index >= les_numStructDefinitions))
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
	return structDefinitionPtr;
}

void LES_DebugOutputStructs(LES_LoggerChannel* const pLogChannel)
{
	const int numStructs = les_numStructDefinitions;
	for (int i = 0; i < numStructs; i++)
	{
		const LES_StructDefinition* const structDefinitionPtr = LES_GetStructDefinitionByID(i);
		const int structNameID = structDefinitionPtr->GetNameID();
		const LES_StringEntry* const structNameStringEntryPtr = LES_GetStringEntryForID(structNameID);

		const char* const structName = structNameStringEntryPtr->m_str;
		const int numMembers = structDefinitionPtr->GetNumMembers();
		pLogChannel->Print("Struct '%s' NnumMembers[%d]", structName, numMembers);
		for (int m = 0; m < numMembers; m++)
		{
			const LES_StructMember* const structMemberPtr = structDefinitionPtr->GetMemberByIndex(m);

			const LES_Hash hash = structMemberPtr->m_hash;
			const int nameID = structMemberPtr->m_nameID;
			const int typeID = structMemberPtr->m_typeID;
			const int dataSize = structMemberPtr->m_dataSize;
			const int alignmentPadding = structMemberPtr->m_alignmentPadding;

			const LES_StringEntry* const memberNameStringEntryPtr = LES_GetStringEntryForID(nameID);
			const char* const memberName = memberNameStringEntryPtr->m_str;
			const LES_StringEntry* const typeNameStringEntryPtr = LES_GetStringEntryForID(typeID);
			const char* const typeName = typeNameStringEntryPtr->m_str;
			pLogChannel->Print("  Struct '%s' Member[%d] '%s' 0x%X Type:'%s' size:%d alignmentPadding:%d", 
												 structName, m, memberName, hash, typeName, dataSize, alignmentPadding);
		}
	}
}

