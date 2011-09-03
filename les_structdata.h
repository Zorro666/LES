#ifndef LES_STRUCTDATA_HH
#define LES_STRUCTDATA_HH

#include "les_base.h"
#include "les_struct.h"

/*
LES_StructData
{
	LES_int32 m_numStructDeinfitions; 																		- 4-bytes
	LES_int32 m_settled; 																									- 4-bytes, 0 in file
	LES_uint32 m_structDefinitionOffsets[m_numStructDefinitions];					- 4-bytes * m_numStructDefinitions
	LES_StructDefinition m_structDefinitions[m_numStructDefinitions];	- variable 
};
*/

class LES_StructData
{
public:
	int GetNumStructDefinitions(void) const;
	const LES_StructDefinition* GetStructDefinition(const int index) const;
	int GetStructDefinitionIndex(const LES_Hash nameHash) const;

	friend class LES_DefinitionFile;
private:
	int Settle(void);

	LES_StructData();
	~LES_StructData();

	LES_int32 m_numStructDefinitions;
	LES_int32 m_settled;
	LES_int32 m_structDefinitionOffsets[1];	// m_structDefinitionOffsets[m_numStructDefinitions];
	// LES_StructDefinition m_structDefinitions[m_numStructDefinitions];
};

inline int LES_StructData::GetNumStructDefinitions(void) const
{
	return m_numStructDefinitions;
}

inline const LES_StructDefinition* LES_StructData::GetStructDefinition(const int index) const
{
	if (m_settled == 0)
	{
		return LES_NULL;
	}
	const LES_StructDefinition* pStructDefinition = (const LES_StructDefinition*)m_structDefinitionOffsets[index];
	return pStructDefinition;
}

#endif // #ifndef LES_STRUCTDATA_HH
