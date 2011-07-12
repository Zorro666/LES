#include <string.h>

#include "les_struct.h"
#include "les_stringentry.h"

static LES_StructDefinition* les_structDefinitionArray = LES_NULL;
static int les_numStructDefinitions = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
static int LES_GetStructDefinitionIndexByNameID(const int nameID)
{
	/* This is horribly slow - need hash lookup table */
	for (int i=0; i<les_numStructDefinitions; i++)
	{
		const LES_StructDefinition* const structDefinitionPtr = &les_structDefinitionArray[i];
		if (structDefinitionPtr->GetNameID() == nameID)
		{
			return i;
		}
	}
	return -1;
}
#endif // #if 0

static int LES_GetStructDefinitionIndex(const char* const name)
{
	const LES_Hash structNameHash = LES_GenerateHashCaseSensitive(name);

	/* This is horribly slow - need hash lookup table */
	for (int i=0; i<les_numStructDefinitions; i++)
	{
		const LES_StructDefinition* const structDefinitionPtr = &les_structDefinitionArray[i];
		const LES_StringEntry* const structNameStringEntryPtr = LES_GetStringEntryForID(structDefinitionPtr->GetNameID());
		if (structNameStringEntryPtr->m_hash == structNameHash)
		{
			if (strcmp(structNameStringEntryPtr->m_str, name) == 0)
			{
				return i;
			}
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// External Public functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

LES_StructDefinition::LES_StructDefinition(void)
{
	m_nameID = -1;
	m_totalMemberDataSize = 0;
	m_numMembers = 0;
	m_members = LES_NULL;
	m_ownsMembersMemory = false;
}

LES_StructDefinition::LES_StructDefinition(const LES_StructDefinition& other)
{
	*this = other;
}

LES_StructDefinition& LES_StructDefinition::operator=(const LES_StructDefinition& other)
{
	m_nameID = other.m_nameID;

	m_totalMemberDataSize = other.m_totalMemberDataSize;
	m_numMembers = other.m_numMembers;
	m_members = other.m_members;
	m_ownsMembersMemory = true;
	other.m_ownsMembersMemory = false;

	return *this;
}

LES_StructDefinition::LES_StructDefinition(const int nameID, const int numMembers)
{
	m_nameID = nameID;
	m_totalMemberDataSize = 0;
	m_numMembers = numMembers;
	m_members = new LES_StructMember[numMembers];
	m_ownsMembersMemory = true;
}

LES_StructDefinition::~LES_StructDefinition(void)
{
	if (m_ownsMembersMemory)
	{
		delete[] m_members;
	}
	m_nameID = -1;
	m_totalMemberDataSize = 0;
	m_numMembers = 0;
	m_members = LES_NULL;
	m_ownsMembersMemory = false;
}

void LES_StructDefinition::SetTotalMemberDataSize(const int totalMemberDataSize)
{
	m_totalMemberDataSize = totalMemberDataSize;
}

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
	if ((index >=0) && (index < m_numMembers))
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

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_StructInit()
{
	les_structDefinitionArray = new LES_StructDefinition[1024];
	les_numStructDefinitions = 0;
}

void LES_StructShutdown()
{
	les_numStructDefinitions = 0;
	delete[] les_structDefinitionArray;
}

int LES_AddStructDefinition(const char* const name, const LES_StructDefinition* const structDefinitionPtr)
{
	int index = LES_GetStructDefinitionIndex(name);
	if ((index < 0) || (index >= les_numStructDefinitions))
	{
		/* Not found so add it */
		index = les_numStructDefinitions;
		les_structDefinitionArray[index] = *structDefinitionPtr;
		les_numStructDefinitions++;
	}

	return index;
}
