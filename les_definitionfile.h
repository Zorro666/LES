#ifndef LES_DEFINITIONFILE_HH
#define LES_DEFINITIONFILE_HH

#include "les_base.h"
#include "les_chunkfile.h"

/*
{
	ID: 4-bytes
	numChunks
	Chunk Offsets
	STRING TABLE_CHUNK
	TYPE_DATA_CHUNK
	STRUCT_DATA_CHUNK
	FUNCTION_DATA_CHUNK
}
*/

class LES_StringTable;
class LES_TypeData;

class LES_DefinitionFile
{
public:
	LES_DefinitionFile(const void* chunkFileData, const int dataSize);
	~LES_DefinitionFile();

	const LES_StringTable* GetStringTable(void);
	const LES_TypeData* GetTypeData(void);

private:
	const LES_ChunkFile* m_chunkFileData;
	
	enum { 
					LES_DEFINITION_STRING_TABLE = 0,
				 	LES_DEFINITION_TYPE_DATA = 1,
			 };
};

inline const LES_StringTable* LES_DefinitionFile::GetStringTable(void)
{
	void* chunkDataPtr = m_chunkFileData->GetChunkData(LES_DEFINITION_STRING_TABLE);
}

inline const LES_TypeData* LES_DefinitionFile::GetTypeData(void)
{
	void* chunkDataPtr = m_chunkFileData->GetChunkData(LES_DEFINTIION_TYPE_DATA);
}

#endif // #ifndef LES_DEFINITIONFILE_HH

