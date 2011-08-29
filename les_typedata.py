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

def decodeFlags(flags):
	# flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY, delimiter is | e.g. "INPUT|POD"
	flagsArray = []
	if flags & LES_TYPE_INPUT:
		flagsArray.append("INPUT")
	if flags & LES_TYPE_OUTPUT:
		flagsArray.append("OUTPUT")
	if flags & LES_TYPE_POD:
		flagsArray.append("POD")
	if flags & LES_TYPE_STRUCT:
		flagsArray.append("STRUCT")
	if flags & LES_TYPE_POINTER:
		flagsArray.append("POINTER")
	if flags & LES_TYPE_REFERENCE:
		flagsArray.append("REFERENCE")
	if flags & LES_TYPE_ALIAS:
		flagsArray.append("ALIAS")
	if flags & LES_TYPE_ARRAY:
		flagsArray.append("ARRAY")
	flagsString = "|".join(flagsArray)
	return flagsString

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
			print "WARNING: LES_TypeData::addType '%s' but type already exists" % (name)
			typeEntry = self.typeEntries[index]
			# Check the existing type data matches the new type data
			if typeEntry.dataSize != dataSize:
				print "ERROR: LES_TypeData::addType '%s' already in list and dataSize doesn't match Existing:%d New:%d" % (name, typeEntry.dataSize, dataSize)
				return -1

			if typeEntry.flags != flags:
				print "ERROR: LES_TypeData::addType '%s' already in list and flags doesn't match Existing:0x%X %s New:0x%X %s" % (name, typeEntry.flags, decodeFlags(typeEntry.flags), flags, decodeFlags(flags))
				return -1

			aliasedTypeID = self.stringTable.getStringID(aliasedName)
			if typeEntry.aliasedTypeID != aliasedTypeID:
				existingAliasedName = self.stringTable.getString(typeEntry.aliasedTypeID)
				print "ERROR: LES_TypeData::addType '%s' already in list and aliasedTypeID doesn't match Existing:%d '%s' New:%d '%s' " % (name, typeEntry.aliasedTypeID, existingAliasedName, aliasedTypeID, aliasedName)
				return -1

			if typeEntry.numElements != numElements:
				print "ERROR: LES_TypeData::addType '%s' already in list and numElements doesn't match Existing:%d New:%d" % (name, typeEntry.numElements, numElements)
				return -1

		# Validate the input values for this type
		if name != aliasedName:
			flags |= LES_TYPE_ALIAS

		if dataSize < 1:
			print "ERROR: LES_TypeData::addType '%s' invalid dataSize value:%d (must be >= 1)" % (name, dataSize)
			return -1

		if flags == 0:
			print "ERROR: LES_TypeData::addType '%s' invalid flags value:0x%X (must be >= 0)" % (name, flags)
			return -1

		if flags & LES_TYPE_ALIAS:
			if self.doesTypeExist(aliasedName) == False:
				print "ERROR: LES_TypeData::addType '%s' aliasedName:'%s' doesn't exist" % (name, aliasedName)
				return -1

		if numElements > 0:
			if (flags & LES_TYPE_ARRAY) == 0:
				print "ERROR: LES_TypeData::addType '%s' numElements > 0 but ARRAY not in flags:'%s'" % (name, decodeFlags(flags))
				return -1

		if numElements < 0:
			print "ERROR: LES_TypeData::addType '%s' invalid numElements value:%d (must be >= 0)" % (name, numElements)
			return -1

		if numElements <= 0:
			if flags & LES_TYPE_ARRAY:
				print "ERROR: LES_TypeData::addType '%s' invalid numElements value:%d (must be >= 0)" % (name, numElements)
				return -1

		if flags & LES_TYPE_ARRAY:
			# Array types must be aliased to non-pointer, non-reference type
			if flags & LES_TYPE_ALIAS == 0:
				print "ERROR: LES_TypeData::addType '%s' is an array with %d elements but not aliased, arrays must be aliased" % (name, numElements)
				return -1
			typeEntry = self.getTypeData(aliasedName)
			aliasedFlags = typeEntry.flags
			# Arrays of non-references must not be aliased to pointer or reference
			if ((flags & LES_TYPE_REFERENCE) == 0) and (aliasedFlags & LES_TYPE_POINTER) or (aliasedFlags & LES_TYPE_REFERENCE):
				print "ERROR: LES_TyepData::addType '%s' non-reference array types must be aliased to a non-pointer, non-reference type Alias:'%s' Flags:0x%X %s" % (name, aliasedName, aliasedFlags, decodeFlags(aliasedFlags))
				return -1

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
		# 	flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY
		#		aliasedName is optional, default is value of name
		#		numElements is optional, default is 0
		#
		# <LES_TYPE_POD name="unsigned char" dataSize="1" />
		#		flags = INPUT|POD, fixed can't be specified
		#		aliasedName = input_name, fixed can't be specified
		#		numElements = 0, fixed can't be specified

		# <LES_TYPE_POD_POINTER name="unsigned char" />
		#		name = input_name + "*"
		#		dataSize = 4 by default : this could be set by some global params in the XML in the future
		#		flags = INPUT|OUTPUT|POD|POINTER, fixed can't be specified
		#		aliasedName = input_name, fixed can't be specified
		#		numElements = 0, fixed can't be specified

		# <LES_TYPE_POD_REFERENCE name="int" />
		#		name = input_name + "&"
		#		dataSize = dataSize value of type with the aliasedName
		#		flags = INPUT|OUTPUT|POD|REFERENCE, fixed can't be specified
		#		aliasedName = input_name + "*", fixed can't be specified
		#		numElements = 0, fixed can't be specified

		# <LES_TYPE_POD_ARRAY name="int" numElements="4" />
		#		name = input_name + "[<numElements>]"
		#		dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
		#		flags = INPUT|OUTPUT|POD|ARRAY, fixed can't be specified
		#		aliasedName = input_name

		# <LES_TYPE_POD_REFERENCE_ARRAY name="int" numElements="4" />
		#		name = input_name + "&[<numElements>]"
		#		dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
		#		flags = INPUT|OUTPUT|POD|REFERENCE|ARRAY, fixed can't be specified
		#		aliasedName = input_name+"*"

		# <LES_TYPE_STRUCT name="TestStruct1" dataSize="10" />
		#		flags = INPUT|STRUCT, fixed can't be specified
		#		aliasedName = input_name

		# <LES_TYPE_STRUCT_POINTER name="TestStruct1" />
		#		name = input_name + "*"
		#		dataSize = 4 by default : this could be set by some global params in the XML in the future
		#		flags = INPUT|OUTPUT|STRUCT|POINTER, fixed can't be specified
		#		aliasedName = input_name, fixed can't be specified
		#		numElements = 0, fixed can't be specified

		# <LES_TYPE_STRUCT_REFERNCE name="TestStruct1" />
		#		name = input_name + "&"
		#		dataSize = dataSize value of type with the aliasedName
		#		flags = INPUT|OUTPUT|STRUCT|REFERENCE, fixed can't be specified
		#		aliasedName = input_name + "*", fixed can't be specified
		#		numElements = 0, fixed can't be specified

		# <LES_TYPE_STRUCT_ARRAY
		#		name = input_name + "[<numElements>]"
		#		dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
		#		flags = INPUT|OUTPUT|STRUCT|ARRAY, fixed can't be specified
		#		aliasedName = input_name

		# <LES_TYPE_STRUCT_REFERENCE_ARRAY name="TestStruct2" numElements="5" />
		#		name = input_name + "&[<numElements>]"
		#		dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
		#		flags = INPUT|OUTPUT|STRUCT|REFERENCE|ARRAY, fixed can't be specified
		#		aliasedName = input_name+"*"

		typesXML = xml.etree.ElementTree.XML(xmlSource)
		if typesXML.tag != "LES_TYPES":
			print "ERROR: LES_TypeData::loadXML root tag should be LES_TYPES found %d" % (typesXML.tag)
			return 1

		numErrors = 0
		for typeXML in typesXML:
			needsDataSize = True
			needsFlags = False
			needsAlias = False
			needsNumElements = False

			dataSizeData = ""
			dataSizeDataDefault = None
			flagsData = ""
			nameSuffix = ""
			aliasSuffix = ""
			nameSuffixAddNumElements = False

			if typeXML.tag == "LES_TYPE":
				needsFlags = True
				needsAlias = True
				needsNumElements = True
			elif typeXML.tag == "LES_TYPE_POD":
				flagsData = "INPUT|POD"
			elif typeXML.tag == "LES_TYPE_POD_POINTER":
				flagsData = "INPUT|OUTPUT|POD|POINTER"
				nameSuffix = "*"
				dataSizeDataDefault = "4"
			elif typeXML.tag == "LES_TYPE_POD_REFERENCE":
				needsDataSize = False
				flagsData = "INPUT|OUTPUT|POD|REFERENCE"
				nameSuffix = "&"
				aliasSuffix = "*"
			elif typeXML.tag == "LES_TYPE_POD_ARRAY":
				needsDataSize = False
				flagsData = "INPUT|OUTPUT|POD|ARRAY"
				nameSuffix = ""
				nameSuffixAddNumElements = True
				aliasSuffix = ""
				needsNumElements = True
			elif typeXML.tag == "LES_TYPE_POD_REFERENCE_ARRAY":
				needsDataSize = False
				flagsData = "INPUT|OUTPUT|POD|REFERENCE|ARRAY"
				nameSuffix = "&"
				nameSuffixAddNumElements = True
				aliasSuffix = "*"
				needsNumElements = True
			elif typeXML.tag == "LES_TYPE_STRUCT":
				flagsData = "INPUT|STRUCT"
			elif typeXML.tag == "LES_TYPE_STRUCT_POINTER":
				flagsData = "INPUT|OUTPUT|STRUCT|POINTER"
				nameSuffix = "*"
				dataSizeDataDefault = "4"
			elif typeXML.tag == "LES_TYPE_STRUCT_REFERENCE":
				needsDataSize = False
				flagsData = "INPUT|OUTPUT|STRUCT|REFERENCE"
				nameSuffix = "&"
				aliasSuffix = "*"
			elif typeXML.tag == "LES_TYPE_STRUCT_ARRAY":
				needsDataSize = False
				flagsData = "INPUT|OUTPUT|STRUCT|ARRAY"
				nameSuffix = ""
				nameSuffixAddNumElements = True
				aliasSuffix = ""
				needsNumElements = True
			elif typeXML.tag == "LES_TYPE_STRUCT_REFERENCE_ARRAY":
				needsDataSize = False
				flagsData = "INPUT|OUTPUT|STRUCT|REFERENCE|ARRAY"
				nameSuffix = "&"
				nameSuffixAddNumElements = True
				aliasSuffix = "*"
				needsNumElements = True
			else:
				print "ERROR: LES_TypeData::loadXML invalid node tag should be LES_TYPE, LES_TYPE_POD, LES_TYPE_POD_POINTER, LES_TYPE_POD_REFERENCE, LES_TYPE_POD_ARRAY, LES_TYPE_POD_REFERENCE_ARRAY, LES_TYPE_STRUCT, LES_TYPE_STRUCT_POINTER, LES_TYPE_STRUCT_REFERENCE, LES_TYPE_STRUCT_ARRAY, LES_TYPE_STRUCT_REFERNCE_ARRAY found %s" % (typeXML.tag)
				numErrors += 1
				continue

			nameData = typeXML.get("name")
			if nameData == None:
				print "ERROR: LES_TypeData::loadXML missing 'name' attribute:%s" % (xml.etree.ElementTree.tostring(typeXML))
				numErrors += 1
				continue
			name = nameData

			if needsDataSize:
				dataSizeData = typeXML.get("dataSize")
				if dataSizeData == None:
					dataSizeData = dataSizeDataDefault
					if dataSizeData == None:
						print "ERROR: LES_TypeData::loadXML '%s' missing 'dataSize' attribute:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
						numErrors += 1
						continue
			else:
				if typeXML.get("dataSize") != None:
					print "ERROR: LES_TypeData::loadXML '%s' 'dataSize' attribute not allowed for this type definition:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue

			if needsFlags:
				flagsData = typeXML.get("flags")
				if flagsData == None:
					print "ERROR: LES_TypeData::loadXML '%s' missing 'flags' attribute:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
			else:
				if typeXML.get("flags") != None:
					print "ERROR: LES_TypeData::loadXML '%s' 'flags' attribute not allowed for this type definition:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue

			if flagsData == "":
				print "ERROR: LES_TypeData::loadXML '%s' invalid 'flags' attribute:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
				numErrors += 1
				continue

			if needsAlias:
				aliasedNameData = typeXML.get("aliasedName", nameData)
			else:
				if typeXML.get("aliasedName") != None:
					print "ERROR: LES_TypeData::loadXML '%s' 'aliasedName' attribute not allowed for this type definition:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
				aliasedNameData = nameData + aliasSuffix

			if needsNumElements:
				numElementsData = typeXML.get("numElements", "0")
			else:
				if typeXML.get("numElements") != None:
					print "ERROR: LES_TypeData::loadXML '%s' 'numElements' attribute not allowed for this type definition:%s" % (name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
				numElementsData = "0"
					
			aliasedName = aliasedNameData
			name = nameData + nameSuffix

			if needsDataSize:
				try:
					dataSize = int(dataSizeData)
				except ValueError:
					print "ERROR: LES_TypeData::loadXML '%s' invalid dataSize value:'%s' (must be >= 1)" % (name, dataSizeData)
					numErrors += 1
					continue
			else:
				if typeXML.tag == "LES_TYPE_POD_REFERENCE":
					# Get it from the data size of the alias - the rule
						typeNameForDataSize = aliasedName
				elif typeXML.tag == "LES_TYPE_POD_ARRAY":
					# dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
					typeNameForDataSize = name + "*"
				elif typeXML.tag == "LES_TYPE_STRUCT_REFERENCE":
					# Get it from the data size of the alias - the rule
						typeNameForDataSize = aliasedName
				elif typeXML.tag == "LES_TYPE_POD_REFERENCE_ARRAY":
					# dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
					typeNameForDataSize = name + "*"
				elif typeXML.tag == "LES_TYPE_STRUCT_ARRAY":
					# dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
					typeNameForDataSize = name + "*"
				elif typeXML.tag == "LES_TYPE_STRUCT_REFERENCE_ARRAY":
					# dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
					typeNameForDataSize = name + "*"
				if self.doesTypeExist(typeNameForDataSize):
					typeData = self.getTypeData(typeNameForDataSize)
					dataSize = typeData.dataSize

			# flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY, delimiter is | e.g. "INPUT|POD"
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
					print "ERROR: LES_TypeData::loadXML '%s' invalid flag:'%s' flags:'%s'" % (name, flag, flagsData)
					numErrors += 1
					continue

			try:
				numElements = int(numElementsData)
			except ValueError:
				print "ERROR: LES_TypeData::loadXML '%s' invalid numElements value:%s (must be >= 0)" % (name, numElementsData)
				numErrors += 1
				numElements = -1
				continue

			if nameSuffixAddNumElements:
				# name = name + "[<numELements>]"
				name = name + "[" + str(numElements) + "]"

			if aliasedName != name:
				flags |= LES_TYPE_ALIAS

			flagsDecoded = decodeFlags(flags)
			print "Type '%s' size:%d flags:0x%X %s aliasedName:'%s' numElements:%d" % (name, dataSize, flags, flagsDecoded, aliasedName, numElements)

			if self.doesTypeExist(name):
				print "WARNING: LES_TypeData::loadXML '%s' already exists" % (name)

			index = self.addType(name, dataSize, flags, aliasedName, numElements)
			if index == -1:
				print "ERROR: LES_TypeData::loadXML '%s' failed to add type" % (name)
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

	print ""
	print "#####################"
	print "Loading data/les_types_basic.xml"
	print "#####################"
	print ""
 	fh = open("data/les_types_basic.xml")
	xmlData = fh.read()
	numErrors = this.loadXML(xmlData)
	if numErrors == 0:
		print "SUCCESS"
	else:
		print "ERROR numErrors=%d" % (numErrors)

	print ""
	print "#####################"
	print "Loading data/les_types_test.xml"
	print "#####################"
	print ""
	fh = open("data/les_types_test.xml")
	xmlData = fh.read()
	numErrors = this.loadXML(xmlData)
	if numErrors == 0:
		print "SUCCESS"
	else:
		print "ERROR numErrors=%d" % (numErrors)

	print ""
	print "#####################"
	print "Loading data/les_types_errors.xml"
	print "#####################"
	print ""
	fh = open("data/les_types_errors.xml")
	xmlData = fh.read()
	numErrors = this.loadXML(xmlData)
	if numErrors == 0:
		print "SUCCESS"
	else:
		print "ERROR numErrors=%d" % (numErrors)

if __name__ == '__main__':
	runTest()
