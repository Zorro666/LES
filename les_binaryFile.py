#!/usr/bin/python

import struct

class LES_BinaryFile():	
	def __init__(self, fname):
		self.fname = fname
		self.fh= open(fname, mode="wb")
		self.endianFormat = ""
		self.setBigEndian()

	def setFormats(self):
		self.intFormat = self.endianFormat + "i"
		self.uintFormat = self.endianFormat + "I"

	def setLittleEndian(self):
		self.endianFormat = "<"
		self.setFormats()

	def setBigEndian(self):
		self.endianFormat = ">"
		self.setFormats()

	def writeInt(self, value):
		temp = struct.pack(self.intFormat, value)
		self.fh.write(temp)

	def writeUint(self, value):
		temp = struct.pack(self.uintFormat, value)
		self.fh.write(temp)

	def writeString(self, value):
		stringLen = len(value)
		strFmt = self.endianFormat + str(stringLen) + "s"
		temp = struct.pack(strFmt, value)
		self.fh.write(temp)
		# null terminate the string
		strFmt = self.endianFormat + "1c"
		temp = struct.pack(strFmt, chr(0))
		self.fh.write(temp)

	def close(self):
		self.fh.close()

def runTest():
	this = LES_BinaryFile("testLittle.bin")
	this.setLittleEndian()
	this.writeInt(15)
	this.writeInt(0x987654)
	this.writeUint(0xDEADBEAF)
	this.writeString("testLit")
	this.close()

	this = LES_BinaryFile("testBig.bin")
	this.setBigEndian()
	this.writeInt(15)
	this.writeInt(0x987654)
	this.writeUint(0xDEADBEAF)
	this.writeString("testBig")
	this.close()

if __name__ == '__main__':
	runTest()
