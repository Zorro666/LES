#include "les_chunkfile.h"

void LES_ChunkFile::Settle(void)
{
	m_numChunks = fromLittleEndian32(m_numChunks);
	for (LES_uint i = 0; i < m_numChunks; i++)
	{
		m_chunkOffsets[i] = fromLittleEndian32(m_chunkOffsets[i]);
	}
}
