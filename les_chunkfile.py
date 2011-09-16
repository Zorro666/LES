#!/usr/bin/python

import les_binaryfile
import les_logger

# ChunkData file format
# m_ID											: 4-bytes
# m_numChunks								: 4-bytes
# m_chunkOffsets[numChunks]	: 4-bytes * numChunks
# chunkData[0]
# chunkData[1]
# ....
# chunkData[numChunks-1]]

class LES_ChunkFile():	
	def __init__(self, magicName, numChunks, bigEndian=True):
		self.binFile = les_binaryfile.LES_BinaryFile()
		self.chunkNames = []
		self.chunkOffsetValues = []
		self.chunkOffsetIndexes = []
		self.numChunks = numChunks
		self.magicName = magicName
		self.currentChunkIndex = 0
		self.activeChunkIndex = -1
		self.activeChunkName = "__NOT_ACTIVE__"

		if bigEndian == True:
			self.binFile.setBigEndian()
		else:
			self.binFile.setLittleEndian()

		for i in range(self.numChunks):
				self.chunkOffsetValues.append(0xDEADBEAF)

		# Write the header
		# m_ID											: 4-bytes
		self.binFile.writeString(self.magicName)

		# m_numChunks								: 4-bytes
		self.binFile.writeUint32(numChunks)

		# Write out the chunkOffset table
		# m_chunkOffsets[numChunks]	: 4-bytes * numChunks
		for offset in self.chunkOffsetValues:
			currentFileIndex = self.binFile.getIndex()
			self.chunkOffsetIndexes.append(currentFileIndex)
			self.chunkNames.append("__NOT_WRITTEN__")
			self.binFile.writeUint32(offset)

	def startChunk(self, chunkName):
		if self.activeChunkIndex != -1:
			les_logger.Error("LES_ChunkFile::startChunk '%s' can't start a new chunk inside an existing chunk '%s'", chunkName, self.activeChunkName)
			return None

		if self.currentChunkIndex >= self.numChunks:
			les_logger.Error("LES_ChunkFile::startChunk '%s' all chunks have been written", chunkName)
			return None

		self.activeChunkName = chunkName
		self.activeChunkIndex = self.currentChunkIndex
		self.chunkNames[self.activeChunkIndex] = chunkName

		currentFileIndex = self.binFile.getIndex()
		# Make sure a chunk starts on 4-byte boundary
		alignedIndex = (currentFileIndex + 3) & ~3
		alignmentPadding = alignedIndex - currentFileIndex;
		for i in range(alignmentPadding):
			self.binFile.writeUint8(0x66)
		currentFileIndex = self.binFile.getIndex()
		self.chunkOffsetValues[self.activeChunkIndex] = currentFileIndex

		offsetIndex = self.chunkOffsetIndexes[self.activeChunkIndex]
		self.binFile.seek(offsetIndex)
		self.binFile.writeUint32(currentFileIndex)
		self.binFile.seek(currentFileIndex)

		return self.binFile

	def endChunk(self):
		if self.activeChunkIndex != self.currentChunkIndex:
			les_logger.Error("LES_ChunkFile::endChunk chunk not started")
			return

		self.currentChunkIndex += 1
		self.activeChunkIndex = -1
		self.activeChunkName = "__NOT_ACTIVE__"

	def saveToFile(self, fname):
		self.binFile.saveToFile(fname)

	def getData(self):
		return self.binFile.getData()

	def close(self):
		if self.activeChunkIndex != -1:
			les_logger.Error("LES_ChunkFile::close current chunk isn't closed '%s'", self.activeChunkName)
			return

		if self.currentChunkIndex != self.numChunks:
			les_logger.Error("LES_ChunkFile::close not all chunks have been written %d numChunks %d", self.currentChunkIndex, self.numChunks)
			return

		self.binFile.close()

def runTest():
	les_logger.Init()
	this = LES_ChunkFile("BAGA", 2)

	binFile = this.startChunk("StringTable")
	binFile.writeCstring("string table data")
	this.endChunk()
	this.saveToFile("chunkTest.bin")
	this.close()

	binFile = this.startChunk("TypeData")
	binFile.writeCstring("type data")
	this.startChunk("ChunkNotFinished")
	this.endChunk()

	binFile = this.startChunk("TooManyChunks")

if __name__ == '__main__':
	runTest()
