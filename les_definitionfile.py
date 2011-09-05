#!/usr/bin/python

import les_chunkfile
import les_stringtable
import les_typedata
import les_structdata
import les_funcdata
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
	if typeData.loadXML("data/les_types_basic.xml") == False:
		les_logger.FatalError("ERROR loading data/les_types_basic.xml")
	if typeData.loadXML("data/les_types_test.xml") == False:
		les_logger.FatalError("ERROR loading data/les_types_test.xml")
	typeData.loadXML("data/les_types_errors.xml")

def loadStructData(structData):
	if structData.loadXML("data/les_structs_test.xml") == False:
		les_logger.FatalError("ERROR loading data/les_structs_test.xml")
	structData.loadXML("data/les_structs_errors.xml")

def loadFunctionData(functionData):
	if functionData.loadXML("data/les_functions_test.xml") == False:
		les_logger.FatalError("ERROR loading data/les_functions_test.xml")
	functionData.loadXML("data/les_functions_errors.xml")

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
		structData = les_structdata.LES_StructData(stringTable, typeData)
		functionData = les_funcdata.LES_FunctionData(stringTable, typeData, structData)
		loadTypeData(typeData)
		loadStructData(structData)
		loadFunctionData(functionData)

		self.addChunk("StringTable", stringTable)
		self.addChunk("TypeData", typeData)
		self.addChunk("StructData", structData)
		self.addChunk("FunctionData", functionData)

def runTest():
	les_logger.Init()
	this = LES_DefinitionFile()
	this.create()
	this.writeFile("defTest.bin")

if __name__ == '__main__':
	runTest()
