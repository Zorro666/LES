#!/usr/bin/python

import les_hash
import les_binaryfile
import les_stringtable
import xml.etree.ElementTree

LES_TYPE_INPUT 				= (1 << 0)
LES_TYPE_OUTPUT 			= (1 << 1)
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

	def addType(self, name, dataSize, flags, aliasedName=None, numElements=1):
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
		if aliasedName != None:
			aliasedTypeID = self.stringTable.addString(aliasedName)

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

	def loadXML(self, xmlSource):
		#<?xml version='1.0' ?>
		#<LES_TYPES>
		#	<LES_TYPE name="int*" dataSize="4" flags="INPUT|OUTPUT|POD" />
		#	<LES_TYPE name="int*" dataSize="4" flags="INPUT|OUTPUT|POD|ARRAY" aliasedName="int" numElements="2" />
		# aliasedName = optional, default is name
		# numELements = optional, default is 0
		# flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY
		typesXML = xml.etree.ElementTree.XML(xmlSource)
		if typesXML.tag != "LES_TYPES":
			print "ERROR les_typedata::loadXML root tag should be LES_TYPES found %d" % (typesXML.tag)
			return False

		for typeXML in typesXML:
			if typeXML.tag != "LES_TYPE":
				print "ERROR les_typedata::loadXML invalid node tag should be LES_TYPE found %d" % (typeXML.tag)
				return False

			nameData = typeXML.get("name")
			if nameData == None:
				print "ERROR les_typedata::loadXML LES_TYPE missing 'name' attribute:%s" % (xml.etree.ElementTree.tostring(typeXML))
				return False

			dataSizeData = typeXML.get("dataSize")
			if dataSizeData == None:
				print "ERROR les_typedata::loadXML LES_TYPE missing 'dataSize' attribute:%s" % (xml.etree.ElementTree.tostring(typeXML))
				return False

			flagsData = typeXML.get("flags")
			if flagsData == None:
				print "ERROR les_typedata::loadXML LES_TYPE missing 'flags' attribute:%s" % (xml.etree.ElementTree.tostring(typeXML))
				return False

			aliasedNameData = typeXML.get("aliasedName", nameData)
			numElementsData = typeXML.get("numElements", "0")
					
			name = nameData

			try:
				dataSize = int(dataSizeData)
			except ValueError:
				print "ERROR les_typedata::loadXML LES_TYPE invalid dataSize value:%s (must be >= 1)" % (dataSizeData)
				return False

			if dataSize < 1:
				print "ERROR les_typedata::loadXML LES_TYPE invalid dataSize value:%d (must be >= 1)" % (dataSize)
				return False

			# flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY, delimiter is | e.g. "INPUT|POD"
			#flags = flagsData
			flagsArray = flagsData.split('|')
			flags = int(0)
			for flag in flagsArray:
				if flag == "INPUT":
					flags |= LES_TYPE_INPUT
				elif flag == "OUTPUT":
					flags |= LES_TYPE_OUTPUT
				elif flag == "POD":
					flags |= LES_TYPE_POD
				elif flag == "STRUCT":
					flags |= LES_TYPE_STRUCT
				elif flag == "POINTER":
					flags |= LES_TYPE_POINTER
				elif flag == "REFERENCE":
					flags |= LES_TYPE_REFERENCE
				elif flag == "ALIAS":
					flags |= LES_TYPE_ALIAS
				elif flag == "ARRAY":
					flags |= LES_TYPE_ARRAY
				else:
					print "ERROR les_typedata::loadXML LES_TYPE invalid flag:'%s' flags:'%s'" % (flag, flagsData)
					return False

			aliasedName = aliasedNameData

			try:
				numElements = int(numElementsData)
			except ValueError:
				print "ERROR les_typedata::loadXML LES_TYPE invalid numElements value:%s (must be >= 0)" % (numElementsData)
				return False

			if numElements < 0:
				print "ERROR les_typedata::loadXML LES_TYPE invalid numElements value:%d (must be >= 0)" % (numElements)
				return False

			print "Type '%s' size:%d flags:0x%X aliasedName:'%s' numElements:%d" % (name, dataSize, flags, aliasedName, numElements)

			index = self.addType(name, dataSize, flags, aliasedName, numElements)
			if index == -1:
				print "ERROR les_typedata::loadXML failed to add type:'%s'" % (name)
				return False

def runTest():
	stringTable = les_stringtable.LES_StringTable()
	this = LES_TypeData(stringTable)
	index = this.addType("int", 4, LES_TYPE_INPUT|LES_TYPE_POD)
	print "Index[int]= %d" % index
	index = this.addType("int", 2, 0x2)
	print "ERROR: Index[int]= %d" % index
	index = this.addType("int*", 4, LES_TYPE_INPUT|LES_TYPE_OUTPUT|LES_TYPE_POINTER|LES_TYPE_ALIAS, "int")
	index = this.addType("int[3]", 4*3, LES_TYPE_INPUT|LES_TYPE_OUTPUT|LES_TYPE_ARRAY|LES_TYPE_ALIAS, "int*", 3)

	binFile = les_binaryfile.LES_BinaryFile("typeDataLittle.bin")
	binFile.setLittleEndian()
	this.writeFile(binFile)
	binFile.close()

	fh = open("data/les_types.xml")
	xmlData = fh.read()
	this.loadXML(xmlData)


if __name__ == '__main__':
	runTest()
