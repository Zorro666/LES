#!/usr/bin/python

import les_chunkfile
import les_stringtable

#{
#	STRING TABLE_CHUNK
#	TYPE_DATA_CHUNK
#	STRUCT_DATA_CHUNK
#	FUNCTION_DATA_CHUNK
#}

def runTest():
	stringTable = les_stringtable.LES_StringTable()
	stringTable.addString("jake")
	stringTable.addString("rowan")
	stringTable.addString("Jake")

	chunkFile = les_chunkfile.LES_ChunkFile("chunkTest.bin", "LESD", 1)

	binFile = chunkFile.startChunk("StringTable")
	stringTable.writeFile(binFile)
	chunkFile.endChunk()

	chunkFile.close()

if __name__ == '__main__':
	runTest()
