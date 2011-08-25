#!/usr/bin/python

import les_chunkfile

#{
#	STRING TABLE_CHUNK
#	TYPE_DATA_CHUNK
#	STRUCT_DATA_CHUNK
#	FUNCTION_DATA_CHUNK
#}

def runTest():
	this = les_chunkfile.LES_ChunkFile("chunkTest.bin", "BAGA", 2)

if __name__ == '__main__':
	runTest()
