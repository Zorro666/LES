#ifndef LES_DEFINITIONFILE_HH
#define LES_DEFINITIONFILE_HH

#include "les_base.h"
#include "les_chunkfile.h"

/*
{
	ID: 4-bytes
	numChunks
	Chunk Offsets
	STRINGTABLE_CHUNK
	TYPEDATA_CHUNK
	STRUCTDATA_CHUNK
	FUNCTIONDATA_CHUNK
}
*/

class LES_StringTable;
class LES_TypeData;

class LES_DefinitionFile
{
public:
	LES_DefinitionFile();
	LES_DefinitionFile(const void* chunkFileData, const int dataSize);
	~LES_DefinitionFile();

	int Load(const void* chunkFileData, const int dataSize);
	int UnLoad(void);

	const char* GetID(void);
	int GetNumChunks(void);

	const LES_StringTable* GetStringTable(void) const;
	const LES_TypeData* GetTypeData(void) const;

	bool Loaded(void) const;
private:
	LES_DefinitionFile(const LES_DefinitionFile& other);
	LES_DefinitionFile& operator =(const LES_DefinitionFile& other);

	const LES_ChunkFile* m_chunkFileData;
	enum { 
					LES_DEFINITION_STRINGTABLE = 0,
				 	LES_DEFINITION_TYPEDATA = 1,
			 };
};

inline LES_DefinitionFile::LES_DefinitionFile()
{
	m_chunkFileData = LES_NULL;
}

inline const LES_StringTable* LES_DefinitionFile::GetStringTable(void) const
{
	const void* chunkDataPtr = m_chunkFileData->GetChunkData(LES_DEFINITION_STRINGTABLE);
	const LES_StringTable* stringTablePtr = (const LES_StringTable*)chunkDataPtr;
	return stringTablePtr;
}

inline const LES_TypeData* LES_DefinitionFile::GetTypeData(void) const
{
	const void* chunkDataPtr = m_chunkFileData->GetChunkData(LES_DEFINITION_TYPEDATA);
	const LES_TypeData* typeDataPtr = (const LES_TypeData*)chunkDataPtr;
	return typeDataPtr;
}

#endif // #ifndef LES_DEFINITIONFILE_HH

