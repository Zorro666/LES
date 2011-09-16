#!/usr/bin/python

import struct
import cStringIO

import les_logger

class LES_BinaryFile():	
	def __init__(self):
		self.fh = cStringIO.StringIO()
		self.endianFormat = ""
		self.setBigEndian()

	def setFormats(self):
		int8Format = self.endianFormat + "b"
		uint8Format = self.endianFormat + "B"
		int16Format = self.endianFormat + "h"
		uint16Format = self.endianFormat + "H"
		int32Format = self.endianFormat + "i"
		uint32Format = self.endianFormat + "I"
		nullCharFormat = self.endianFormat + "1c"

		self.int8 = struct.Struct(int8Format)
		self.uint8 = struct.Struct(uint8Format)

		self.int16 = struct.Struct(int16Format)
		self.uint16 = struct.Struct(uint16Format)

		self.int32 = struct.Struct(int32Format)
		self.uint32 = struct.Struct(uint32Format)

		self.nullChar = struct.Struct(nullCharFormat)

	def setLittleEndian(self):
		self.endianFormat = "<"
		self.setFormats()

	def setBigEndian(self):
		self.endianFormat = ">"
		self.setFormats()

	def writeInt8(self, value):
		temp = self.int8.pack(value)
		self.fh.write(temp)

	def writeUint8(self, value):
		temp = self.uint8.pack(value)
		self.fh.write(temp)

	def writeInt16(self, value):
		temp = self.int16.pack(value)
		self.fh.write(temp)

	def writeUint16(self, value):
		temp = self.uint16.pack(value)
		self.fh.write(temp)

	def writeInt32(self, value):
		temp = self.int32.pack(value)
		self.fh.write(temp)

	def writeUint32(self, value):
		temp = self.uint32.pack(value)
		self.fh.write(temp)

	def writeString(self, value):
		numChars = len(value)
		strFmt = self.endianFormat + str(numChars) + "s"
		temp = struct.pack(strFmt, value)
		self.fh.write(temp)

	def writeCstring(self, value):
		self.writeString(value)
		# null terminate the string
		temp = self.nullChar.pack(chr(0))
		self.fh.write(temp)

	def getIndex(self):
		return self.fh.tell()

	def seek(self, index):
		self.fh.seek(index)

	def saveToFile(self, fname):
		realFH = open(fname, mode="wb")
		data = self.fh.getvalue()
		realFH.write(data)
		realFH.close()

	def close(self):
		self.fh.close()

	def getData(self):
		return self.fh.getvalue()

def runTest():
	les_logger.Init()
	this = LES_BinaryFile()
	this.setLittleEndian()
	this.writeInt32(15)
	this.writeInt32(0x987654)
	this.writeUint32(0xDEADBEAF)
	this.writeString("testLit")
	this.saveToFile("testLittle.bin")
	this.close()

	this = LES_BinaryFile()
	this.setBigEndian()
	this.writeInt32(15)
	this.writeInt32(0x987654)
	this.writeUint32(0xDEADBEAF)
	this.writeString("testBig")
	this.saveToFile("testBig.bin")
	this.close()

if __name__ == '__main__':
	runTest()
