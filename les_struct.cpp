#include "les_struct.h"

static LES_FunctionDefinition* les_functionDefinitionArray = LES_NULL;
static int les_numFunctionDefinitions = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// External Public functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

LES_StructDefinition::LES_StructDefinition(void)
{
	m_nameID = -1;
	m_numMembers = 0;
	m_members = LES_NULL;
}

LES_StructDefinition::LES_StructDefinition(const int nameID, const int numMembers)
{
	m_nameID = nameID;
	m_numMembers = numMembers;
	m_members = new LES_StructMember[numMembers];
}

LES_StructDefinition::~LES_StructDefinition(void)
{
	delete[] m_members;
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
	return LES_ERROR;
}
