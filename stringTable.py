#!/usr/bin/python

import ctypes

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

def runTest():
	this = LES_StringTable()
	index = this.addString("jake")
	print "Index[jake]=",index
	index = this.addString("rowan")
	print "Index[rowan]=",index
	index = this.addString("jake")
	print "Index[jake]=",index
	index = this.addString("Jake")
	print "Index[Jake]=",index

	print "Hash[jake]=", this.getHash(0)
	print "Hash[rowan]=", this.getHash(1)
	print "Hash[Jake]=", this.getHash(2)

	print "Hash(jake)=1863425725"
	print "Hash(rowan)=3756861831"
	print "Hash(Jake)=8686429"


if __name__ == '__main__':
	runTest()
