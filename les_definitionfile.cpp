#include <malloc.h>
#include <string.h>

#include "les_definitionfile.h"
#include "les_stringtable.h"

LES_DefinitionFile::LES_DefinitionFile(const void* chunkFileData, const int dataSize)
{
	m_chunkFileData = (LES_ChunkFile*)new char[dataSize];
	memcpy((void*)m_chunkFileData, chunkFileData, dataSize);
	LES_StringTable* const pStringTable = (LES_StringTable*)GetStringTable();
	pStringTable->Settle();
}

LES_DefinitionFile::~LES_DefinitionFile()
{
	delete[] (char*)m_chunkFileData;
}
