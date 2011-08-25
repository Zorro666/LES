#!/usr/bin/python

import les_chunkFile.py

#{
#	STRING TABLE_CHUNK
#	TYPE_DATA_CHUNK
#	STRUCT_DATA_CHUNK
#	FUNCTION_DATA_CHUNK
#}

def runTest():
	this = les_ChunkFile.LES_ChunkFile("chunkTest.bin", "BAGA", 2)

if __name__ == '__main__':
	runTest()
