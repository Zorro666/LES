
import struct

class LES_BinaryFile():	
	def __init__(self, fname):
		self.fname = fname
		self.fh= open(fname, mode="wb")

	def writeInt(self, value):
		temp = struct.pack(">i", value)
		self.fh.write(temp)

	def writeUint(self, value):
		temp = struct.pack(">I", value)
		self.fh.write(temp)

	def writeString(self, value):
		stringLen = len(value)
		strFmt=">"+str(stringLen)+"s"
		temp = struct.pack(strFmt, value)
		self.fh.write(temp)
		# null terminate the string
		temp = struct.pack(">1c", chr(0))
		self.fh.write(temp)

	def close(self):
		self.fh.close()

