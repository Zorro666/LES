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

	def doesTypeExist(self, name):
		return name in self.typeNames

	def getTypeData(self, name):
		try:
			index = self.typeNames.index(name)
		except ValueError:
			return None
		return self.typeEntries[index]

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
		#
		#	<LES_TYPE name="int*" dataSize="4" flags="INPUT|OUTPUT|POD|ARRAY" aliasedName="int" numElements="2" />
		#		aliasedName is optional, default is value of name
		#		numElements is optional, default is 0
		# 	flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY
		#
		# <LES_TYPE_POD name="unsigned char" dataSize="1" />
		#		flags = INPUT|POD, fixed can't be specified
		#		aliasedName = name, fixed can't be specified
		#		numElements = 0, fixed can't be specified
		# <LES_TYPE_POD_POINTER name="unsigned char" dataSize="4" />
		#		name = name + "*"
		#		flags = INPUT|OUTPUT|POD|POINTER, fixed can't be specified
		#		aliasedName = name, fixed can't be specified
		#		numElements = 0, fixed can't be specified
		# <LES_TYPE_POD_REFERENCE name="int" />
		#		name = name + "&"
		#		dataSize = dataSize value of type with the aliasedName
		#		flags = INPUT|OUTPUT|POD|REFERENCE, fixed can't be specified
		#		aliasedName = name + "*", fixed can't be specified
		#		numElements = 0, fixed can't be specified
		typesXML = xml.etree.ElementTree.XML(xmlSource)
		if typesXML.tag != "LES_TYPES":
			print "ERROR les_typedata::loadXML root tag should be LES_TYPES found %d" % (typesXML.tag)
			return 1

		numErrors = 0
		for typeXML in typesXML:
			needsDataSize = True
			needsFlags = False
			needsAlias = False
			needsNumElements = False

			dataSizeData = ""
			flagsData = ""
			nameSuffix = ""
			aliasSuffix = ""

			if typeXML.tag == "LES_TYPE":
				needsFlags = True
				needsAlias = True
				needsNumElements = True
			elif typeXML.tag == "LES_TYPE_POD":
				flagsData = "INPUT|POD"
			elif typeXML.tag == "LES_TYPE_POD_POINTER":
				flagsData = "INPUT|OUTPUT|POINTER|POD"
				nameSuffix = "*"
			elif typeXML.tag == "LES_TYPE_POD_REFERENCE":
				needsDataSize = False
				flagsData = "INPUT|OUTPUT|REFERENCE|POD"
				nameSuffix = "&"
				aliasSuffix = "*"
			else:
				print "ERROR les_typedata::loadXML invalid node tag should be LES_TYPE found %s" % (typeXML.tag)
				numErrors += 1
				continue

			nameData = typeXML.get("name")
			if nameData == None:
				print "ERROR les_typedata::loadXML LES_TYPE missing 'name' attribute:%s" % (xml.etree.ElementTree.tostring(typeXML))
				numErrors += 1
				continue

			if needsDataSize:
				dataSizeData = typeXML.get("dataSize")
				if dataSizeData == None:
					print "ERROR les_typedata::loadXML LES_TYPE '%s' missing 'dataSize' attribute:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
			else:
				if typeXML.get("dataSize") != None:
					print "ERROR les_typedata::loadXML LES_TYPE '%s' 'dataSize' attribute not allowed for this type definition:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue

			if needsFlags:
				flagsData = typeXML.get("flags")
				if flagsData == None:
					print "ERROR les_typedata::loadXML LES_TYPE '%s' missing 'flags' attribute:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
			else:
				if typeXML.get("flags") != None:
					print "ERROR les_typedata::loadXML LES_TYPE '%s' 'flags' attribute not allowed for this type definition:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue

			if flagsData == "":
				print "ERROR les_typedata::loadXML LES_TYPE '%s' invalid 'flags' attribute:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
				numErrors += 1
				continue

			if needsAlias:
				aliasedNameData = typeXML.get("aliasedName", nameData)
			else:
				if typeXML.get("aliasedName") != None:
					print "ERROR les_typedata::loadXML LES_TYPE '%s' 'aliasedName' attribute not allowed for this type definition:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
				aliasedNameData = nameData + aliasSuffix

			if needsNumElements:
				numElementsData = typeXML.get("numElements", "0")
			else:
				if typeXML.get("numElements") != None:
					print "ERROR les_typedata::loadXML LES_TYPE '%s' 'numElements' attribute not allowed for this type definition:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
				numElementsData = "0"
					
			aliasedName = aliasedNameData
			name = nameData + nameSuffix

			if needsDataSize:
				try:
					dataSize = int(dataSizeData)
				except ValueError:
					print "ERROR les_typedata::loadXML LES_TYPE '%s' invalid dataSize value:'%s' (must be >= 1)" % (name, dataSizeData)
					numErrors += 1
					continue
			else:
				# Get it from the data size of the alias - the rule
				if self.doesTypeExist(aliasedName):
					typeData = self.getTypeData(aliasedName)
					dataSize = typeData.dataSize

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
					print "ERROR les_typedata::loadXML LES_TYPE '%s' invalid flag:'%s' flags:'%s'" % (name, flag, flagsData)
					numErrors += 1
					continue

			try:
				numElements = int(numElementsData)
			except ValueError:
				print "ERROR les_typedata::loadXML LES_TYPE '%s' invalid numElements value:%s (must be >= 0)" % (name, numElementsData)
				numErrors += 1
				continue

			if aliasedName != name:
				flags |= LES_TYPE_ALIAS

			print "Type '%s' size:%d flags:0x%X aliasedName:'%s' numElements:%d" % (name, dataSize, flags, aliasedName, numElements)

			if self.doesTypeExist(name):
				print "ERROR les_typedata::loadXML can't add type:'%s' it already exists" % (name)
				numErrors += 1
				continue

			if dataSize < 1:
				print "ERROR les_typedata::loadXML LES_TYPE '%s' invalid dataSize value:%d (must be >= 1)" % (name, dataSize)
				numErrors += 1
				continue

			if flags == 0:
				print "ERROR les_typedata::loadXML LES_TYPE '%s' invalid flags value:%d (must be >= 0)" % (name, flags)
				numErrors += 1
				continue

			if flags & LES_TYPE_ALIAS:
				if self.doesTypeExist(aliasedName) == False:
					print "ERROR les_typedata::loadXML can't add type:'%s' aliasedName:'%s' doesn't exist" % (name, aliasedName)
					numErrors += 1
					continue

			if numElements > 0:
				if (flags & LES_TYPE_ARRAY) == 0:
					print "ERROR les_typedata::loadXML LES_TYPE '%s' numElements > 0 but ARRAY not in flags:'%s'" % (name, flagsData)
					numErrors += 1
					continue

			if numElements < 0:
				print "ERROR les_typedata::loadXML LES_TYPE '%s' invalid numElements value:%d (must be >= 0)" % (name, numElements)
				numErrors += 1
				continue

			if numElements <= 0:
				if flags & LES_TYPE_ARRAY:
					print "ERROR les_typedata::loadXML LES_TYPE '%s' invalid numElements value:%d (must be >= 0)" % (name, numElements)
					numErrors += 1
					continue

			index = self.addType(name, dataSize, flags, aliasedName, numElements)
			if index == -1:
				print "ERROR les_typedata::loadXML failed to add type:'%s'" % (name)
				numErrors += 1
				continue

		return numErrors

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

	stringTable = les_stringtable.LES_StringTable()
	this = LES_TypeData(stringTable)

	fh = open("data/les_types.xml")
	xmlData = fh.read()
	this.loadXML(xmlData)


if __name__ == '__main__':
	runTest()
