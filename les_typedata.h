#ifndef LES_TYPEDATA_HH
#define LES_TYPEDATA_HH

#include "les_base.h"
#include "les_type.h"

/*
LES_TypeData
{
	LES_int32 m_numTypes; 										// 4-bytes
	LES_int32 m_settled;											// 4-bytes
	LES_TypeEntry m_typeEntries[m_numTypes]; 	// 20-bytes * m_numTypes
};
*/

class LES_TypeData
{
public:
	int GetNumTypes(void) const;
	const LES_TypeEntry* GetTypeEntry(const int index) const;

	friend class LES_DefinitionFile;
private:
	int Settle(void);

	LES_TypeData();
	~LES_TypeData();

	LES_int32 m_numTypes;
	LES_int32 m_settled;
	LES_TypeEntry m_typeEntries[1];		// m_typeEntries[m_numTypes];
};

inline int LES_TypeData::GetNumTypes(void) const
{
	return m_numTypes;
}

inline const LES_TypeEntry* LES_TypeData::GetTypeEntry(const int index) const
{
	if (m_settled == 0)
	{
		return LES_NULL;
	}
	const LES_TypeEntry* pTypeEntry = (const LES_TypeEntry*)&m_typeEntries[index];
	return pTypeEntry;
}

#endif // #ifndef LES_TYPEDATA_HH
