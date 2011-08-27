#!/usr/bin/python

import les_chunkfile
import les_stringtable
import les_typedata

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

	typeData = les_typedata.LES_TypeData(stringTable)
	index = typeData.addType("int", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD)
	index = typeData.addType("int*", 4, les_typedata.LES_TYPE_INPUT_OUTPUT|les_typedata.LES_TYPE_POINTER|les_typedata.LES_TYPE_ALIAS, "int")
	index = typeData.addType("int[3]", 4*3, les_typedata.LES_TYPE_INPUT_OUTPUT|les_typedata.LES_TYPE_ARRAY|les_typedata.LES_TYPE_ALIAS, "int*", 3)

	chunkFile = les_chunkfile.LES_ChunkFile("chunkTest.bin", "LESD", 2)

	binFile = chunkFile.startChunk("StringTable")
	stringTable.writeFile(binFile)
	chunkFile.endChunk()

	binFile = chunkFile.startChunk("TypeData")
	typeData.writeFile(binFile)
	chunkFile.endChunk()

	chunkFile.close()

if __name__ == '__main__':
	runTest()
