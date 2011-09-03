#!/usr/bin/python

import les_chunkfile
import les_stringtable
import les_typedata
import les_structdata
import les_logger

#{
# ID: 4-bytes
# LES_int32 numChunks
# Chunk Offsets
#	STRING TABLE_CHUNK
#	TYPE_DATA_CHUNK
#	STRUCT_DATA_CHUNK
#	FUNCTION_DATA_CHUNK
#}

def loadTypeData(typeData):
	typeData.loadXML("data/les_types_basic.xml")
	typeData.loadXML("data/les_types_test.xml")
	typeData.loadXML("data/les_types_errors.xml")

def loadStructData(structData, typeData, stringTable):
	nameID = stringTable.addString("PyTestStruct1")
	structDefinition = les_structdata.LES_StructDefinintion(nameID, 2)
	structDefinition.AddMember("char", "m_char", stringTable, typeData, structData)
	structDefinition.AddMember("float", "m_float", stringTable, typeData, structData)
	structData.addStructDefinition("PyTestStruct1", structDefinition)

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
		chunkFile = les_chunkfile.LES_ChunkFile(self.fname, "LESD", self.numChunks, bigEndian=True)

		for i in range(self.numChunks):
			chunkData = self.chunkDatas[i]
			chunkName = self.chunkNames[i]

			binFile = chunkFile.startChunk(chunkName)
			chunkData.writeFile(binFile)
			chunkFile.endChunk()

		chunkFile.close()

	def create(self):
		stringTable = les_stringtable.LES_StringTable()
		typeData = les_typedata.LES_TypeData(stringTable)
		structData = les_structdata.LES_StructData(stringTable)
		loadTypeData(typeData)
		loadStructData(structData, typeData, stringTable)

		self.addChunk("StringTable", stringTable)
		self.addChunk("TypeData", typeData)
		self.addChunk("StructData", structData)

def runTest():
	les_logger.Init()
	this = LES_DefinitionFile()
	this.create()
	this.writeFile("defTest.bin")

if __name__ == '__main__':
	runTest()
