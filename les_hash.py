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

