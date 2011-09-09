#include "les_chunkfile.h"

void LES_ChunkFile::Settle(void)
{
	m_numChunks = fromBigEndian32(m_numChunks);
	for (LES_uint i = 0; i < m_numChunks; i++)
	{
		m_chunkOffsets[i] = fromBigEndian32(m_chunkOffsets[i]);
	}
}
