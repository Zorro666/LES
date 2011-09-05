#ifndef LES_STRUCT_HH
#define LES_STRUCT_HH

#include "les_base.h"
#include "les_hash.h"

class LES_LoggerChannel;

struct LES_StructMember
{
	LES_uint32 m_hash;
	LES_int32 m_nameID;
	LES_int32 m_typeID;
	LES_int32 m_dataSize;
	LES_int32 m_alignmentPadding;
};

class LES_StructDefinition
{
public:
	int GetNameID(void) const;
	int GetNumMembers(void) const;
	const LES_StructMember* GetMemberByIndex(const int index) const;
	const LES_StructMember* GetMember(const LES_Hash nameHash) const;

	friend LES_StructDefinition* LES_CreateStructDefinition(const int nameID, const int numMembers);
	friend class LES_StructData;
private:
	LES_StructDefinition(void);
	~LES_StructDefinition(void);
	LES_StructDefinition(const LES_StructDefinition& other);
	LES_StructDefinition& operator=(const LES_StructDefinition& other);

	LES_int32 m_nameID;
	LES_int32 m_numMembers;
	LES_StructMember m_members[1];	// m_members[m_numMembers]
};

const LES_StructDefinition* LES_GetStructDefinition(const LES_Hash nameHash);
void LES_DebugOutputStructs(LES_LoggerChannel* const pLogChannel);

#endif // #ifndef LES_STRUCT_HH
