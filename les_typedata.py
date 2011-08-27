#!/usr/bin/python

import les_hash
import les_binaryfile
import les_stringtable

LES_TYPE_INPUT 				= (1 << 0)
LES_TYPE_OUTPUT 			= (1 << 1)
LES_TYPE_INPUT_OUTPUT = (LES_TYPE_INPUT|LES_TYPE_OUTPUT)
LES_TYPE_POD 					= (1 << 2)
LES_TYPE_STRUCT  			= (1 << 3)
LES_TYPE_POINTER 			= (1 << 4)
LES_TYPE_REFERENCE 		= (1 << 5)
LES_TYPE_ALIAS 				= (1 << 6)
LES_TYPE_ARRAY 				= (1 << 7)

# struct LES_TypeEntry
# {
#		unsigned int m_hash;											- 4 bytes
#		unsigned int m_dataSize										- 4 bytes
#		unsigned int m_flags;											- 4 bytes
#		int m_aliasedTypeID;											- 4 bytes
#		int m_numElements;												- 4 bytes
# };
#
# LES_TypeData
# {
# 	int m_numTypes; 													- 4 bytes
#		LES_TypeData m_types[numStrings];					- 20 bytes * m_numTypes
# };

class LES_TypeEntry():
	def __init__(self, hashValue, dataSize, flags, aliasedTypeID, numElements):
		self.hashValue = hashValue
		self.dataSize = dataSize
		self.flags = flags
		self.aliasedTypeID = aliasedTypeID
		self.numElements = numElements

class LES_TypeData():
	def __init__(self, stringTable):
		self.typeEntries = []
		self.typeNames = []
		self.typeAliasedNames = []
		self.stringTable = stringTable

	def addType(self, name, dataSize, flags, aliasedTypeName=None, numElements=1):
		try:
			index = self.typeNames.index(name)
		except ValueError:
			index = -1
		if index >= 0:
		 print "ERROR addType(%s) but type already exists" % (name)
		 return index

		# Add the type name to the string table for good measure
		self.stringTable.addString(name)

		# compute the aliasedTypeNameID which is an index into the string table (future this could be a type ID into the type array)
		aliasedTypeID = 0xFFFFFFFF 
		if aliasedTypeName != None:
			aliasedTypeID = self.stringTable.addString(aliasedTypeName)

		nameHash = les_hash.GenerateHashCaseSensitive(name)

		typeEntry = LES_TypeEntry(nameHash, dataSize, flags, aliasedTypeID, numElements)

		index = len(self.typeEntries)
		self.typeEntries.append(typeEntry)
		self.typeNames.append(name)

		return index

	def writeFile(self, binFile):
		# LES_TypeData
		# {
		# 	int m_numTypes; 													- 4 bytes
		#		LES_TypeData m_types[numStrings];					- 20 bytes * m_numTypes
		# };

		# 	int m_numTypes; 													- 4 bytes
		numTypes = len(self.typeEntries)
		binFile.writeInt(numTypes)

		for typeEntry in self.typeEntries:
			# struct LES_TypeEntry
			# {
			#		unsigned int m_hash;											- 4 bytes
			#		unsigned int m_dataSize										- 4 bytes
			#		unsigned int m_flags;											- 4 bytes
			#		int m_aliasedTypeID;											- 4 bytes
			#		int m_numElements;												- 4 bytes
			# };

			#		unsigned int m_hash;											- 4 bytes
			binFile.writeUint(typeEntry.hashValue)

			#		unsigned int m_dataSize										- 4 bytes
			binFile.writeUint(typeEntry.dataSize)

			#		unsigned int m_flags;											- 4 bytes
			binFile.writeUint(typeEntry.flags)

			#		int m_aliasedTypeID;											- 4 bytes
			binFile.writeUint(typeEntry.aliasedTypeID)

			#		int m_numElements;												- 4 bytes
			binFile.writeUint(typeEntry.numElements)

def runTest():
	stringTable = les_stringtable.LES_StringTable()
	this = LES_TypeData(stringTable)
	index = this.addType("int", 4, LES_TYPE_INPUT|LES_TYPE_POD)
	print "Index[int]= %d" % index
	index = this.addType("int", 2, 0x2)
	print "ERROR: Index[int]= %d" % index
	index = this.addType("int*", 4, LES_TYPE_INPUT_OUTPUT|LES_TYPE_POINTER|LES_TYPE_ALIAS, "int")
	index = this.addType("int[3]", 4*3, LES_TYPE_INPUT_OUTPUT|LES_TYPE_ARRAY|LES_TYPE_ALIAS, "int*", 3)

	binFile = les_binaryfile.LES_BinaryFile("typeDataLittle.bin")
	binFile.setLittleEndian()
	this.writeFile(binFile)
	binFile.close()

if __name__ == '__main__':
	runTest()
