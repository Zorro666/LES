#!/usr/bin/python

import les_hash
import binaryFile

class LES_StringTable():
	def __init__(self ):
		self.strings = []
		self.hashes = []
		self.stringOffsets = []

	def addString(self, string):
		index = -1
		try:
			index = self.strings.index(string)
		except ValueError:
			index = -1
		if index >= 0:
		 return index

		index = len(self.strings)
		self.strings.append(string)

		hashValue = les_hash.GenerateHashCaseSensitive(string)
		self.hashes.append(hashValue)

		stringOffset = 0
		if index > 0:
			stringOffset = self.stringOffsets[index-1]

		stringLen = len(string)
		stringOffset += stringLen
		# include the null terminator
		stringOffset += 1
		self.stringOffsets.append(stringOffset)

		return index

	def getString(self, index):
		if index < len(self.strings):
			return self.strings[index]
		return -1

	def getHash(self, index):
		if index < len(self.hashes):
			return self.hashes[index]
		return -1

	def writeFile(self, binFile):
		# LES_StringTable
		# {
		# 	int m_numStrings; - 4 bytes
		# 	int m_stringOffsets[numStrings]; - 4 bytes * numStrings
		# 	u_int m_hashes[numStrings]; - 4 bytes * numStrings
		#		char stringData[];	- total string table size in bytes
		# }

		numStrings = len(self.strings)
		# 	int m_numStrings; - 4 bytes
		binFile.writeInt(numStrings)	

		# 	int m_stringOffsets[numStrings]; - 4 bytes * numStrings
		for stringOffset in self.stringOffsets:
			binFile.writeInt(stringOffset)	

		# 	u_int m_hashes[numStrings]; - 4 bytes * numStrings
		for hashValue in self.hashes:
			binFile.writeUint(hashValue)	

		#		char stringData[];	- total string table size in bytes
		for string in self.strings:
			binFile.writeString(string)	

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

	print "Hash[jake]= %x" % this.getHash(0)
	print "Hash[rowan]= %x" % this.getHash(1)
	print "Hash[Jake]= %x" % this.getHash(2)

	print "Hash(jake)=1863425725 %x" % 1863425725
	print "Hash(rowan)=3756861831 %x" % 3756861831
	print "Hash(Jake)=8686429 %x" % 8686429

	binFile = binaryFile.LES_BinaryFile("stringTable.bin")
	this.writeFile(binFile)
	binFile.close()

if __name__ == '__main__':
	runTest()