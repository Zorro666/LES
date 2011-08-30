#ifndef LES_CHUNKFILE_HH
#define LES_CHUNKFILE_HH

#include "les_base.h"

// Chunk File Format: overlaid onto a constant class

class LES_ChunkFile
{
public:
	const char* GetID(void) const;
	int GetNumChunks(void) const;
	const void* GetChunkData(const int chunk) const;

private:
	LES_ChunkFile();
	~LES_ChunkFile();

	char m_ID[4];
	LES_uint m_numChunks;
	int m_chunkOffsets[1];	// m_chunkOffsets[m_numChunks]
	// chunkData[0]
	// chunkData[1]
	// ....
	// chunkData[numChunks-1]]
};

inline const char* LES_ChunkFile::GetID(void) const
{
	return m_ID;
}

inline int LES_ChunkFile::GetNumChunks(void) const
{
	return m_numChunks;
}

inline const void* LES_ChunkFile::GetChunkData(const int chunk) const
{
	const int chunkOffset = m_chunkOffsets[chunk];
	const char* basePtr = m_ID;
	const char* chunkPtr = basePtr + chunkOffset;
	return chunkPtr;
}

#endif // #ifndef LES_CHUNKFILE_HH
