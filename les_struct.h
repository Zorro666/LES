#ifndef LES_STRUCT_HH
#define LES_STRUCT_HH

#include "les_core.h"
#include "les_hash.h"

struct LES_StructMember
{
	LES_Hash m_hash;
	int m_nameID;
	int m_typeID;

	int m_dataSize;
	int m_alignmentPadding;
};

class LES_StructDefinition
{
public:
	LES_StructDefinition(void);
	LES_StructDefinition(const LES_StructDefinition& other);
	LES_StructDefinition& operator=(const LES_StructDefinition& other);
	LES_StructDefinition(const int nameID, const int numMembers);
	~LES_StructDefinition(void);

	int GetNameID(void) const;
	int GetNumMembers(void) const;
	const LES_StructMember* GetMemberByIndex(const int index) const;
	const LES_StructMember* GetMember(const LES_Hash nameHash) const;

private:
	int m_nameID;

	int m_numMembers;
	const LES_StructMember* m_members;
	mutable bool m_ownsMembersMemory;
};

const LES_StructDefinition* LES_GetStructDefinition(const LES_Hash nameHash);

#endif // #ifndef LES_STRUCT_HH
