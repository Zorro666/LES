#!/usr/bin/python

import struct
import les_logger

class LES_BinaryFile():	
	def __init__(self, fname):
		self.fname = fname
		self.fh = open(fname, mode="wb")
		self.endianFormat = ""
		self.setBigEndian()

	def setFormats(self):
		self.int8Format = self.endianFormat + "b"
		self.uint8Format = self.endianFormat + "B"
		self.int16Format = self.endianFormat + "h"
		self.uint16Format = self.endianFormat + "H"
		self.int32Format = self.endianFormat + "i"
		self.uint32Format = self.endianFormat + "I"

	def setLittleEndian(self):
		self.endianFormat = "<"
		self.setFormats()

	def setBigEndian(self):
		self.endianFormat = ">"
		self.setFormats()

	def writeInt8(self, value):
		temp = struct.pack(self.int8Format, value)
		self.fh.write(temp)

	def writeUint8(self, value):
		temp = struct.pack(self.uint8Format, value)
		self.fh.write(temp)

	def writeInt16(self, value):
		temp = struct.pack(self.int16Format, value)
		self.fh.write(temp)

	def writeUint16(self, value):
		temp = struct.pack(self.uint16Format, value)
		self.fh.write(temp)

	def writeInt32(self, value):
		temp = struct.pack(self.int32Format, value)
		self.fh.write(temp)

	def writeUint32(self, value):
		temp = struct.pack(self.uint32Format, value)
		self.fh.write(temp)

	def writeString(self, value):
		numChars = len(value)
		strFmt = self.endianFormat + str(numChars) + "s"
		temp = struct.pack(strFmt, value)
		self.fh.write(temp)

	def writeCstring(self, value):
		self.writeString(value)
		# null terminate the string
		strFmt = self.endianFormat + "1c"
		temp = struct.pack(strFmt, chr(0))
		self.fh.write(temp)

	def getIndex(self):
		return self.fh.tell()

	def seek(self, index):
		self.fh.seek(index)

	def close(self):
		self.fh.close()

def runTest():
	les_logger.Init()
	this = LES_BinaryFile("testLittle.bin")
	this.setLittleEndian()
	this.writeInt32(15)
	this.writeInt32(0x987654)
	this.writeUint32(0xDEADBEAF)
	this.writeString("testLit")
	this.close()

	this = LES_BinaryFile("testBig.bin")
	this.setBigEndian()
	this.writeInt32(15)
	this.writeInt32(0x987654)
	this.writeUint32(0xDEADBEAF)
	this.writeString("testBig")
	this.close()

if __name__ == '__main__':
	runTest()
