#include <malloc.h>
#include <string.h>

#include "les_definitionfile.h"
#include "les_logger.h"
#include "les_stringtable.h"
#include "les_typedata.h"

LES_DefinitionFile::LES_DefinitionFile(const void* chunkFileData, const int dataSize)
{
	LES_ChunkFile* newChunkFileData = (LES_ChunkFile*)new char[dataSize];
	memcpy((void*)newChunkFileData, chunkFileData, dataSize);

	newChunkFileData->Settle();
	m_chunkFileData = newChunkFileData;

	const char* id = m_chunkFileData->GetID();
	const int numChunks = m_chunkFileData->GetNumChunks();
	LES_LOG("ID:'%c%c%c%c'", id[0], id[1], id[2], id[3]);
	LES_LOG("NumChunks:%d", numChunks);

	const char defID[4] = {'L', 'E', 'S', 'D'};
	const int defNumChunks = 2;

	// CHECK ID = defID
	if ((id[0] != defID[0]) || (id[1] != defID[1]) || (id[2] != defID[2]) || (id[3] != defID[3]))
	{
		LES_ERROR("LES_DefinitionFile:Invalid id '%c%c%c%c' should be '%c%c%c%c'",
							id[0], id[1], id[2], id[3], defID[0], defID[1], defID[2], defID[3]);
		return;
	}
	// CHECK numChunks = defNumChunks
	if (numChunks != defNumChunks)
	{
		LES_ERROR("LES_DefinitionFile:Invalid numChunks %d should be %d", numChunks, defNumChunks);
		return;
	}

	LES_StringTable* const pStringTable = (LES_StringTable* const)GetStringTable();
	if (pStringTable->Settle() != LES_RETURN_OK)
	{
		LES_ERROR("LES_DefinitionFile::LES_StringTable::Settle() failed");
		return;
	}

	LES_TypeData* const pTypeData = (LES_TypeData* const)GetTypeData();
	if (pTypeData->Settle() != LES_RETURN_OK)
	{
		LES_ERROR("LES_DefinitionFile::LES_TypeData::Settle() failed");
		return;
	}
}

LES_DefinitionFile::~LES_DefinitionFile()
{
	delete[] (char*)m_chunkFileData;
}
