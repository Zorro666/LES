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

class LES_DefinitionFile():
	def __init__(self):
		self.fname = ""
		self.chunkDatas = []
		self.chunkNames = []
		self.numChunks = len(self.chunkDatas)

	def addChunk(self, chunkName, chunkData):
		self.chunkDatas.append(chunkData)
		self.chunkNames.append(chunkName)
		self.numChunks = len(self.chunkDatas)

	def writeFile(self, fname):
		self.fname = fname
		chunkFile = les_chunkfile.LES_ChunkFile(self.fname, "LESD", self.numChunks)

		for i in range(self.numChunks):
			chunkData = self.chunkDatas[i]
			chunkName = self.chunkNames[i]

			binFile = chunkFile.startChunk(chunkName)
			chunkData.writeFile(binFile)
			chunkFile.endChunk()

		chunkFile.close()

def runTest():
	stringTable = les_stringtable.LES_StringTable()
	stringTable.addString("jake")
	stringTable.addString("rowan")
	stringTable.addString("Jake")

	typeData = les_typedata.LES_TypeData(stringTable)
	index = typeData.addType("int", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD)
	index = typeData.addType("int*", 4, les_typedata.LES_TYPE_INPUT_OUTPUT|les_typedata.LES_TYPE_POINTER|les_typedata.LES_TYPE_ALIAS, "int")
	index = typeData.addType("int[3]", 4*3, les_typedata.LES_TYPE_INPUT_OUTPUT|les_typedata.LES_TYPE_ARRAY|les_typedata.LES_TYPE_ALIAS, "int*", 3)

	this = LES_DefinitionFile()
	this.addChunk("StringTable", stringTable)
	this.addChunk("TypeData", typeData)
	this.writeFile("defTest.bin")

if __name__ == '__main__':
	runTest()
