#ifndef LES_STRUCT_HH
#define LES_STRUCT_HH

#include "les_core.h"
#include "les_hash.h"

struct LES_StructMember
{
	LES_Hash m_hash;
	int m_nameID;
	int m_typeID;
};

class LES_StructEntry
{
	LES_StructEntry(void);
	LES_StructEntry(const int nameID, const int numMembers);
	~LES_StructEntry(void);

	int GetNameID(void) const;
	int GetNumMembers(void) const;
	const LES_StructMember* GetMemberByIndex(const int index) const;

private:
	int m_nameID;

	int m_numMembers;
	const LES_StructMember* m_members;
};

#endif // #ifndef LES_STRUCT_HH
