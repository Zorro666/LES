#!/usr/bin/python

import les_hash
import les_binaryfile
import les_logger

#	struct LES_StringEntry
#	{
#		unsigned int m_hash;													- 4 bytes
#		const char* m_str;														- 4 bytes : saved as offset into stringTable, ptr settled on load
#	};
#
class LES_StringEntry():
	def __init__(self, hashValue, offset):
		self.hashValue = hashValue
		self.offset = offset

# LES_StringTable
# {
# 	int m_numStrings; 														- 4 bytes
#		LES_StringEntry m_stringEntries[numStrings]; 	- 8 bytes * m_numStrings
#		char stringData[];														- total string table size in bytes
# };

class LES_StringTable():
	def __init__(self):
		self.__m_strings__ = []
		self.__m_stringEntries__ = []

	def addString(self, string):
		index = self.getStringID(string)
		if index >= 0:
		 return index

		index = len(self.__m_strings__)
		self.__m_strings__.append(string)

		hashValue = les_hash.GenerateHashCaseSensitive(string)

		offset = 0
		if index > 0:
			offset = self.__m_stringEntries__[index-1].offset

			# Add on the length of the previous string
			prevStringLen = len(self.__m_strings__[index-1])

			# include the null terminator
			prevStringLen += 1
			offset += prevStringLen

		stringEntry = LES_StringEntry(hashValue, offset)
		self.__m_stringEntries__.append(stringEntry)

		return index

	def getString(self, index):
		if index < len(self.__m_strings__):
			return self.__m_strings__[index]
		return -1

	def getHash(self, index):
		if index < len(self.hashes):
			return self.__m_stringEntries__[index].hashValue
		return -1

	def getStringID(self, string):
		index = -1
		try:
			index = self.__m_strings__.index(string)
		except ValueError:
			index = -1

		return index

	def getStringByHash(self, hashValue):
		numStrings = len(self.__m_strings__)
		for index in range(numStrings):
			stringEntry = self.__m_stringEntries__[index]
			if stringEntry.hashValue == hashValue:
				return self.__m_strings__[index]
		return None

	def writeFile(self, binFile):
		# LES_StringTable
		# {
		# 	int m_numStrings; 														- 4 bytes
		#		LES_StringEntry m_stringEntries[numStrings]; 	- 8 bytes * m_numStrings
		#		char stringData[];														- total string table size in bytes
		# };

		#	int m_numStrings; - 4 bytes
		numStrings = len(self.__m_strings__)
		binFile.writeInt(numStrings)	

		#	LES_StringEntry stringEntry[numStrings]; 				- 8 bytes * numStrings
		for stringEntry in self.__m_stringEntries__:
			#	struct LES_StringEntry
			#	{
			#		unsigned int m_hash;												- 4 bytes
			#		const char* m_str;													- 4 bytes : saved as offset into stringTable, ptr settled on load
			#	};

			#		unsigned int m_hash;												- 4 bytes
			binFile.writeUint(stringEntry.hashValue)	

			#		const char* m_str;													- 4 bytes : saved as offset into stringTable, ptr settled on load
			binFile.writeInt(stringEntry.offset)	

		#	char stringData[];															- total string table size in bytes
		for string in self.__m_strings__:
			binFile.writeCstring(string)	

def runTest():
	les_logger.Init()
	this = LES_StringTable()
	index = this.addString("jake")
	les_logger.Log("Index[jake]= %d", index)
	index = this.addString("rowan")
	les_logger.Log("Index[rowan]= %d", index)
	index = this.addString("jake")
	les_logger.Log("Index[jake]= %d", index)
	index = this.addString("Jake")
	les_logger.Log("Index[Jake]= %d", index)

	binFile = les_binaryfile.LES_BinaryFile("stringTableBig.bin")
	binFile.setBigEndian()
	this.writeFile(binFile)
	binFile.close()

	binFile = les_binaryfile.LES_BinaryFile("stringTableLittle.bin")
	binFile.setLittleEndian()
	this.writeFile(binFile)
	binFile.close()

if __name__ == '__main__':
	runTest()
