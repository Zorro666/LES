#include <string.h>
#include <malloc.h>

#include "les_struct.h"
#include "les_core.h"
#include "les_logger.h"
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

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// External Public functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
LES_StructDefinition::LES_StructDefinition(void)
{
	m_nameID = -1;
	m_numMembers = 0;
	m_members = LES_NULL;
}

LES_StructDefinition::LES_StructDefinition(const LES_StructDefinition& other)
{
	*this = other;
}

LES_StructDefinition& LES_StructDefinition::operator=(const LES_StructDefinition& other)
{
	m_nameID = other.m_nameID;

	const int numMembers = other.m_numMembers;
	m_numMembers = numMembers;
	m_members = new LES_StructMember[numMembers];
	for (int i = 0; i < numMembers; i++)
	{
		LES_StructMember* const pMember = (LES_StructMember*)&m_members[i];
		*pMember = other.m_members[i];
	}
	return *this;
}

LES_StructDefinition::LES_StructDefinition(const int nameID, const int numMembers)
{
	m_nameID = nameID;
	m_numMembers = numMembers;
	m_members = new LES_StructMember[numMembers];

	LES_StructMember emptyMember;
	emptyMember.m_hash = 0;
	emptyMember.m_nameID = 0;
	emptyMember.m_typeID = 0;
	emptyMember.m_dataSize = 0;
	emptyMember.m_alignmentPadding = 0;

	for (int i = 0; i < numMembers; i++)
	{
		LES_StructMember* const pMember = (LES_StructMember*)&m_members[i];
		*pMember = emptyMember;
	}
}

LES_StructDefinition::~LES_StructDefinition(void)
{
	m_nameID = -1;
	m_numMembers = 0;
}
#endif // #if 0

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
	if ((index < 0) || (index >= les_numStructDefinitions))
	{
		return LES_NULL;
	}
	const LES_StructDefinition* const structDefinitionPtr = les_structDefinitionArray[index];
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

