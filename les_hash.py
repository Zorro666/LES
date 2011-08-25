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

def runTest():
	hash1 = GenerateHashCaseSensitive("jake")
	hash2 = GenerateHashCaseSensitive("rowan")
	hash3 = GenerateHashCaseSensitive("Jake")

	realHash1 = 1863425725
	realHash2 = 3756861831
	realHash3 = 8686429

	print "Hash[jake]= %x realHash1 = %x" % (hash1, realHash1)
	print "Hash[rowan]= %x realHash2 = %x" % (hash2, realHash2)
	print "Hash[Jake]= %x realHash3 = %x" % (hash3, realHash3)

	if hash1 != realHash1:
		print "hash1 != realHash1"

	if hash2 != realHash2:
		print "hash2 != realHash2"

	if hash3 != realHash3:
		print "hash3 != realHash3"

if __name__ == '__main__':
	runTest()

