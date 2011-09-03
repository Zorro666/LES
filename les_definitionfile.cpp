#include <malloc.h>
#include <string.h>

#include "les_definitionfile.h"
#include "les_logger.h"
#include "les_stringtable.h"
#include "les_typedata.h"
#include "les_structdata.h"

LES_DefinitionFile::LES_DefinitionFile(const void* chunkFileData, const int dataSize)
{
	if (Load(chunkFileData, dataSize) != LES_RETURN_OK)
	{
		LES_ERROR("LES_DefinitionFile::Load() failed");
	}
}

int LES_DefinitionFile::Load(const void* chunkFileData, const int dataSize)
{
	if (m_chunkFileData != LES_NULL)
	{
		LES_ERROR("LES_DefinitionFile::Load() m_chunkFileData isn't NULL");
		return LES_RETURN_ERROR;
	}

	LES_ChunkFile* newChunkFileData = (LES_ChunkFile*)new char[dataSize];
	memcpy((void*)newChunkFileData, chunkFileData, dataSize);

	newChunkFileData->Settle();
	m_chunkFileData = newChunkFileData;

	const char* id = GetID();
	const int numChunks = GetNumChunks();

	const char defID[4] = {'L', 'E', 'S', 'D'};
	const int defNumChunks = LES_DEFINITION_NUMCHUNKS;

	// CHECK ID = defID
	if ((id[0] != defID[0]) || (id[1] != defID[1]) || (id[2] != defID[2]) || (id[3] != defID[3]))
	{
		LES_ERROR("LES_DefinitionFile:Invalid id '%c%c%c%c' should be '%c%c%c%c'",
							id[0], id[1], id[2], id[3], defID[0], defID[1], defID[2], defID[3]);
		return LES_RETURN_ERROR;
	}
	// CHECK numChunks = defNumChunks
	if (numChunks != defNumChunks)
	{
		LES_ERROR("LES_DefinitionFile:Invalid numChunks %d should be %d", numChunks, defNumChunks);
		return LES_RETURN_ERROR;
	}

	LES_StringTable* const pStringTable = (LES_StringTable* const)GetStringTable();
	if (pStringTable->Settle() != LES_RETURN_OK)
	{
		LES_ERROR("LES_DefinitionFile::LES_StringTable::Settle() failed");
		return LES_RETURN_ERROR;
	}

	LES_TypeData* const pTypeData = (LES_TypeData* const)GetTypeData();
	if (pTypeData->Settle() != LES_RETURN_OK)
	{
		LES_ERROR("LES_DefinitionFile::LES_TypeData::Settle() failed");
		return LES_RETURN_ERROR;
	}

	LES_StructData* const pStructData = (LES_StructData* const)GetStructData();
	if (pStructData->Settle() != LES_RETURN_OK)
	{
		LES_ERROR("LES_DefinitionFile::LES_StructData::Settle() failed");
		return LES_RETURN_ERROR;
	}

	return LES_RETURN_OK;
}

int LES_DefinitionFile::UnLoad(void)
{
	if (m_chunkFileData == LES_NULL)
	{
		LES_ERROR("LES_DefinitionFile::UnLoad m_chunkFileData == LES_NULL");
		return LES_RETURN_ERROR;
	}

	delete[] (char*)m_chunkFileData;
	m_chunkFileData = LES_NULL;
	return LES_RETURN_OK;
}

LES_DefinitionFile::~LES_DefinitionFile()
{
	UnLoad();
}

const char* LES_DefinitionFile::GetID(void)
{
	if (m_chunkFileData == LES_NULL)
	{
		LES_ERROR("LES_DefinitionFile::GetID() not loaded");
		return LES_NULL;
	}
	const char* id = m_chunkFileData->GetID();
	return id;
}

int LES_DefinitionFile::GetNumChunks(void)
{
	if (m_chunkFileData == LES_NULL)
	{
		LES_ERROR("LES_DefinitionFile::GetNumChunks() not loaded");
		return -1;
	}
	const int numChunks = m_chunkFileData->GetNumChunks();
	return numChunks;
}

bool LES_DefinitionFile::Loaded(void) const
{
	return (m_chunkFileData != LES_NULL);
}
