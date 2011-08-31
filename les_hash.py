#!/usr/bin/python

import ctypes
import les_logger
import sys

def GenerateHashCaseSensitive(string):
	hashValue = int(0)
	for c in string:
		hashValue += (hashValue << 7)		# hashValue=hashValue+128*hasValue=129*hashValue
		hashValue += (hashValue << 1)		# hashValue=hashValue+(hashValue+128*hashValue)*2=(129+129*2)*hashValue=387*hashValue
		hashValue += ord(c)							# hashValue=387*hashValue+char
		hashValue = ctypes.c_uint32(hashValue).value
#		les_logger.Log("hashValue=0x%X c=%c %d", hashValue, c, ord(c))

	return hashValue

def runTest():
	les_logger.Init()
	hash1 = GenerateHashCaseSensitive("jake")
	hash2 = GenerateHashCaseSensitive("rowan")
	hash3 = GenerateHashCaseSensitive("Jake")

	realHash1 = 1863425725
	realHash2 = 3756861831
	realHash3 = 8686429

	les_logger.Log("Hash[jake]= 0x%X realHash1 = 0x%X", hash1, realHash1)
	les_logger.Log("Hash[rowan]= 0x%X realHash2 = 0x%X", hash2, realHash2)
	les_logger.Log("Hash[Jake]= 0x%X realHash3 = 0x%X", hash3, realHash3)

	if hash1 != realHash1:
		les_logger.Error("hash1 != realHash1")

	if hash2 != realHash2:
		les_logger.Error("hash2 != realHash2")

	if hash3 != realHash3:
		les_logger.Error("hash3 != realHash3")

	for arg in sys.argv[1:]:
		les_logger.Log("Hash[%s]= 0x%X", arg, GenerateHashCaseSensitive(arg))

if __name__ == '__main__':
	runTest()

