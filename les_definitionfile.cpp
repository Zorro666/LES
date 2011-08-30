#include <malloc.h>
#include <string.h>

#include "les_definitionfile.h"
#include "les_logger.h"
#include "les_stringtable.h"

LES_DefinitionFile::LES_DefinitionFile(const void* chunkFileData, const int dataSize)
{
	m_chunkFileData = (LES_ChunkFile*)new char[dataSize];
	memcpy((void*)m_chunkFileData, chunkFileData, dataSize);

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
	}
	// CHECK numChunks = defNumChunks
	if (numChunks != defNumChunks)
	{
		LES_ERROR("LES_DefinitionFile:Invalid numChunks %d should be %d", numChunks, defNumChunks);
	}

	LES_StringTable* const pStringTable = (LES_StringTable* const)GetStringTable();
	pStringTable->Settle();
}

LES_DefinitionFile::~LES_DefinitionFile()
{
	delete[] (char*)m_chunkFileData;
}
