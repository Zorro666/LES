#ifndef LES_STRUCT_HH
#define LES_STRUCT_HH

#include "les_core.h"
#include "les_hash.h"

struct LES_StructMember
{
	LES_Hash m_hash;
	int m_nameID;
	int m_typeID;
	// Need to include padding offset for natural alignment
};

class LES_StructDefinition
{
public:
	LES_StructDefinition(void);
	LES_StructDefinition(const LES_StructDefinition& other);
	LES_StructDefinition& operator=(const LES_StructDefinition& other);
	LES_StructDefinition(const int nameID, const int numMembers);
	~LES_StructDefinition(void);

	void SetTotalMemberDataSize(const int totalMemberDataSize);

	int GetNameID(void) const;
	int GetNumMembers(void) const;
	const LES_StructMember* GetMemberByIndex(const int index) const;
	const LES_StructMember* GetMember(const LES_Hash nameHash) const;
	int GetTotalMemberDataSize(void) const;

private:
	int m_nameID;

	int m_totalMemberDataSize;
	int m_numMembers;
	const LES_StructMember* m_members;
	mutable bool m_ownsMembersMemory;
};

#endif // #ifndef LES_STRUCT_HH
