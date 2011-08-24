#!/usr/bin/python

import ctypes
import struct
import array

def GenerateHashCaseSensitive(string):
	hashValue = int(0)
	for c in string:
		hashValue += (hashValue << 7)		# hashValue=hashValue+128*hasValue=129*hashValue
		hashValue += (hashValue << 1)		# hashValue=hashValue+(hashValue+128*hashValue)*2=(129+129*2)*hashValue=387*hashValue
		hashValue += ord(c)							# hashValue=387*hashValue+char
		hashValue = ctypes.c_uint32(hashValue).value
#		print "hashValue=",hashValue, "c=",c, ord(c)

	return hashValue

class LES_StringTable():
	def __init__(self ):
		self.strings = []
		self.hashes = []

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
		hashValue = GenerateHashCaseSensitive(string)
		self.hashes.append(hashValue)
		return index

	def getString(self, index):
		if index < len(self.strings):
			return self.strings[index]
		return -1

	def getHash(self, index):
		if index < len(self.hashes):
			return self.hashes[index]
		return -1

	def writeFile(self, fh):
		# LES_StringTable
		# {
		# 	int m_numStrings; - 4 bytes
		# 	int m_stringOffsets[numStrings]; - 4 bytes * numStrings
		# 	u_int m_hashes[numStrings]; - 4 bytes * numStrings
		#		char stringData[];	- total string table size in bytes
		# }

		numStrings = len(self.strings)
		# 	int m_numStrings; - 4 bytes
		temp = struct.pack("=i", numStrings)
		fh.write(temp)

		# 	int m_stringOffsets[numStrings]; - 4 bytes * numStrings
		stringOffset = int(0)
		for string in self.strings:
			temp = struct.pack("=i", stringOffset)
			fh.write(temp)
			stringLen = len(string)
			stringOffset += stringLen
			# include the null terminator
			stringOffset += 1

		# 	u_int m_hashes[numStrings]; - 4 bytes * numStrings
		for hashValue in self.hashes:
			hValue = ctypes.c_uint32(hashValue).value
			print "Hash= %x" % hValue
			t = struct.pack("=I", hValue)
			for c in t:
				print "%x" % ord(c)
				fh.write(c)
				fh.flush()


		#		char stringData[];	- total string table size in bytes
		for string in self.strings:
			stringLen = len(string)
			strFmt="="+str(stringLen)+"s"
			temp = struct.pack(strFmt, string)
			print "stringLen=", stringLen, "len(temp)=", len(temp)
			fh.write(temp)
			# null terminate the string
			temp = struct.pack("=1c", chr(0))
			fh.write(temp)

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

	fh = open("stringTable.bin", mode="wb")
	this.writeFile(fh)

	for i in range(255):
		c = chr(i)
		fh.write(c)
		print "%x" % ord(c)

	fh.close()

if __name__ == '__main__':
	runTest()
