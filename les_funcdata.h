#ifndef LES_FUNCDATA_HH
#define LES_FUNCDATA_HH

#include "les_base.h"
#include "les_function.h"

/*
LES_FunctionData
{
	LES_int32 m_numFunctionDefinitions; 																		- 4-bytes
	LES_int32 m_settled; 																										- 4-bytes, 0 in file
	LES_uint32 m_functionDefinitionOffsets[m_numFunctionDefinitions];				- 4-bytes * m_numFunctionDefinitions
	LES_FunctionDefinition m_functionDefinitions[m_numFunctionDefinitions];	- variable 
};
*/

class LES_FuncData
{
public:
	int GetNumFunctionDefinitions(void) const;
	const LES_FunctionDefinition* GetFunctionDefinition(const int index) const;
	int GetFunctionDefinitionIndex(const LES_Hash nameHash) const;

	friend class LES_DefinitionFile;
private:
	int Settle(void);

	LES_FuncData();
	~LES_FuncData();

	LES_int32 m_numFunctionDefinitions;
	LES_int32 m_settled;
	LES_uint32 m_functionDefinitionOffsets[1];	// m_functionDefinitionOffsets[m_numFunctionDefinitions];
	//LES_FunctionDefinition m_functionDefinitions[m_numFunctionDefinitions];	- variable 
};

inline int LES_FuncData::GetNumFunctionDefinitions(void) const
{
	return m_numFunctionDefinitions;
}

inline const LES_FunctionDefinition* LES_FuncData::GetFunctionDefinition(const int index) const
{
	if (m_settled == 0)
	{
		return LES_NULL;
	}
	const LES_FunctionDefinition* pFunctionDefinition = (const LES_FunctionDefinition*)m_functionDefinitionOffsets[index];
	return pFunctionDefinition;
}

#endif // #ifndef LES_FUNCDATA_HH
