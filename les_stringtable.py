#!/usr/bin/python

import les_hash
import les_binaryfile

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
		self.strings = []
		self.stringEntries = []

	def addString(self, string):
		index = self.getStringID(string)
		if index >= 0:
		 return index

		index = len(self.strings)
		self.strings.append(string)

		hashValue = les_hash.GenerateHashCaseSensitive(string)

		offset = 0
		if index > 0:
			offset = self.stringEntries[index-1].offset

			# Add on the length of the previous string
			prevStringLen = len(self.strings[index-1])

			# include the null terminator
			prevStringLen += 1
			offset += prevStringLen

		stringEntry = LES_StringEntry(hashValue, offset)
		self.stringEntries.append(stringEntry)

		return index

	def getString(self, index):
		if index < len(self.strings):
			return self.strings[index]
		return -1

	def getHash(self, index):
		if index < len(self.hashes):
			return self.stringEntries[index].hashValue
		return -1

	def getStringID(self, string):
		index = -1
		try:
			index = self.strings.index(string)
		except ValueError:
			index = -1

		return index

	def writeFile(self, binFile):
		# LES_StringTable
		# {
		# 	int m_numStrings; 														- 4 bytes
		#		LES_StringEntry m_stringEntries[numStrings]; 	- 8 bytes * m_numStrings
		#		char stringData[];														- total string table size in bytes
		# };

		#	int m_numStrings; - 4 bytes
		numStrings = len(self.strings)
		binFile.writeInt(numStrings)	

		#	LES_StringEntry stringEntry[numStrings]; 				- 8 bytes * numStrings
		for stringEntry in self.stringEntries:
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
		for string in self.strings:
			binFile.writeCstring(string)	

def runTest():
	this = LES_StringTable()
	index = this.addString("jake")
	print "Index[jake]= %d" % index
	index = this.addString("rowan")
	print "Index[rowan]= %d" % index
	index = this.addString("jake")
	print "Index[jake]= %d" % index
	index = this.addString("Jake")
	print "Index[Jake]= %d " % index

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
