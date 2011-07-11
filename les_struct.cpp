#include "les_struct.h"

LES_StructEntry::LES_StructEntry(void)
{
	m_nameID = -1;
	m_numMembers = 0;
	m_members = LES_NULL;
}

LES_StructEntry::LES_StructEntry(const int nameID, const int numMembers)
{
	m_nameID = nameID;
	m_numMembers = numMembers;
	m_members = new LES_StructMember[numMembers];
}

LES_StructEntry::~LES_StructEntry(void)
{
	delete[] m_members;
}

int LES_StructEntry::GetNameID(void) const
{
	return m_nameID;
}

int LES_StructEntry::GetNumMembers(void) const
{
	return m_numMembers;
}

const LES_StructMember* LES_StructEntry::GetMemberByIndex(const int index) const
{
	if ((index >=0) && (index < m_numMembers))
	{
		const LES_StructMember* const memberPtr = &m_members[index];
		return memberPtr;
	}
	return LES_NULL;
}
