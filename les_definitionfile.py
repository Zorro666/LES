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
	if functionData.createTestErrorFunctionDefinitions() == False:
		les_logger.FatalError("ERROR creating test error functionDefinitions")

class LES_DefinitionFile():
	def __init__(self):
		self.__m_chunkDatas__ = []
		self.__m_chunkNames__ = []
		self.__m_numChunks__ = len(self.__m_chunkDatas__)

	def addChunk(self, chunkName, chunkData):
		self.__m_chunkDatas__.append(chunkData)
		self.__m_chunkNames__.append(chunkName)
		self.__m_numChunks__ = len(self.__m_chunkDatas__)

	def getChunk(self, name):
		try:
			index = self.__m_chunkNames__.index(name)
		except ValueError:
			return None
		return self.__m_chunkDatas__[index]

	def makeData(self):
		chunkFileData = les_chunkfile.LES_ChunkFile("LESD", self.__m_numChunks__, bigEndian=True)

		for i in range(self.__m_numChunks__):
			chunkData = self.__m_chunkDatas__[i]
			chunkName = self.__m_chunkNames__[i]

			binFile = chunkFileData.startChunk(chunkName)
			chunkData.write(binFile)
			chunkFileData.endChunk()

		return chunkFileData

	def writeFile(self, fname):
		chunkFileData = self.makeData()
		chunkFileData.saveToFile(fname)
		chunkFileData.close()

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

	def getStringTable(self):
		return self.getChunk("StringTable")

	def getTypeData(self):
		return self.getChunk("TypeData")

	def getStructData(self):
		return self.getChunk("StructData")

	def getFunctionData(self):
		return self.getChunk("FunctionData")

def runTest():
	les_logger.Init()
	this = LES_DefinitionFile()
	this.create()
	this.writeFile("defTest.bin")

if __name__ == '__main__':
	runTest()
