#!/usr/bin/python

import os.path
import xml.etree.ElementTree

import les_hash
import les_binaryfile
import les_stringtable
import les_logger

LES_TYPE_INPUT 				= (1 << 0)
LES_TYPE_OUTPUT 			= (1 << 1)
LES_TYPE_POD 					= (1 << 2)
LES_TYPE_STRUCT  			= (1 << 3)
LES_TYPE_POINTER 			= (1 << 4)
LES_TYPE_REFERENCE 		= (1 << 5)
LES_TYPE_ALIAS 				= (1 << 6)
LES_TYPE_ARRAY 				= (1 << 7)
LES_TYPE_ENDIANSWAP		= (1 << 8)

s_longlongHash = les_hash.LES_GenerateHashCaseSensitive("long long int")
s_longlongHash = les_hash.LES_GenerateHashCaseSensitive("long long int");
s_intHash = les_hash.LES_GenerateHashCaseSensitive("int");
s_shortHash = les_hash.LES_GenerateHashCaseSensitive("short");
s_charHash = les_hash.LES_GenerateHashCaseSensitive("char");

s_uintHash = les_hash.LES_GenerateHashCaseSensitive("unsigned int");
s_ushortHash = les_hash.LES_GenerateHashCaseSensitive("unsigned short");
s_ucharHash = les_hash.LES_GenerateHashCaseSensitive("unsigned char");

s_floatHash = les_hash.LES_GenerateHashCaseSensitive("float");

def decodeFlags(flags):
	# flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY, ENDIANSWAP delimiter is | e.g. "INPUT|POD"
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
	if flags & LES_TYPE_ENDIANSWAP:
		flagsArray.append("ENDIANSWAP")
	flagsString = "|".join(flagsArray)
	return flagsString

# struct LES_TypeEntry
# {
#		LES_uint32 m_hash;														- 4 bytes
#		LES_uint32 m_dataSize;												- 4 bytes
#		LES_uint32 m_flags;														- 4 bytes
#		LES_int32 m_aliasedTypeID;										- 4 bytes
#		LES_int32 m_numElements;											- 4 bytes
# };
#
# LES_TypeData
# {
# 	LES_int32 m_numTypes; 												- 4-bytes
# 	LES_int32 m_settled; 													- 4-bytes, 0 in file
#		LES_TypeEntry m_typeEntries[m_numTypes];	- 20 bytes * m_numTypes
# };

class LES_TypeEntry():
	def __init__(self, hashValue, dataSize, flags, aliasedTypeID, numElements):
		self.m_hash = hashValue
		self.m_dataSize = dataSize
		self.m_flags = flags
		self.m_aliasedTypeID = aliasedTypeID
		self.m_numElements = numElements

	def GetRootType(self, stringTable, typeData):
		typeEntry = self
		flags = typeEntry.m_flags
		while flags & LES_TYPE_ALIAS:
			aliasedTypeID = typeEntry.m_aliasedTypeID
#			les_logger.Log("Type 0x%X alias:%d flags:0x%X", typeEntry.m_hash, aliasedTypeID, flags)
			aliasedStringType = stringTable.getString(aliasedTypeID)
			if aliasedStringType == None:
				les_logger.Error("GetRootType aliased type:%d string can't be found", aliasedTypeID)
				return None
			aliasedTypeEntry = typeData.getTypeData(aliasedStringType)
			if aliasedTypeEntry == None:
				les_logger.Error("GetRootType aliased type not found aliased type:'%s'", aliasedStringType)
				return None
#			les_logger.Log("GetRootType Type 0x%X alias:%s", typeEntry.m_hash, aliasedStringType)

			typeEntry = aliasedTypeEntry
			flags = typeEntry.m_flags
#			les_logger.Log("Alias Type 0x%X flags:0x%X", typeEntry.m_hash, flags)
		return typeEntry


class LES_TypeData():
	def __init__(self, stringTable):
		self.__m_typeEntries__ = []
		self.__m_typeNames__ = []
		self.__m_stringTable__ = stringTable

	def addType(self, name, dataSize, flags, aliasedName, numElements=0):
		try:
			index = self.__m_typeNames__.index(name)
		except ValueError:
			index = -1

		if index >= 0:
			les_logger.Warning("LES_TypeData::addType '%s' but type already exists", name)
			typeEntry = self.__m_typeEntries__[index]
			# Check the existing type data matches the new type data
			if typeEntry.m_dataSize != dataSize:
				les_logger.Error("LES_TypeData::addType '%s' already in list and dataSize doesn't match Existing:%d New:%d", name, typeEntry.m_dataSize, dataSize)
				return -1

			if typeEntry.m_flags != flags:
				les_logger.Error("LES_TypeData::addType '%s' already in list and flags doesn't match Existing:0x%X %s New:0x%X %s", name, typeEntry.m_flags, decodeFlags(typeEntry.m_flags), flags, decodeFlags(flags))
				return -1

			aliasedTypeID = self.__m_stringTable__.getStringID(aliasedName)
			if typeEntry.m_aliasedTypeID != aliasedTypeID:
				existingAliasedName = self.__m_stringTable__.getString(typeEntry.m_aliasedTypeID)
				les_logger.Error("LES_TypeData::addType '%s' already in list and aliasedTypeID doesn't match Existing:%d '%s' New:%d '%s' ", name, typeEntry.m_aliasedTypeID, existingAliasedName, aliasedTypeID, aliasedName)
				return -1

			if typeEntry.m_numElements != numElements:
				les_logger.Error("LES_TypeData::addType '%s' already in list and numElements doesn't match Existing:%d New:%d", name, typeEntry.m_numElements, numElements)
				return -1

		# Validate the input values for this type
		if name != aliasedName:
			flags |= LES_TYPE_ALIAS

		if dataSize < 1:
			if flags & LES_TYPE_POD  or flags & LES_TYPE_POINTER or flags & LES_TYPE_REFERENCE or flags & LES_TYPE_ARRAY:
				les_logger.Error("LES_TypeData::addType '%s' invalid dataSize value:%d (must be >= 1)", name, dataSize)
				return -1

		if flags & LES_TYPE_STRUCT and (flags & LES_TYPE_POINTER == 0) and (flags & LES_TYPE_REFERENCE == 0) and (flags & LES_TYPE_ARRAY == 0) and (flags & LES_TYPE_ALIAS == 0):
			if dataSize != 0:
				les_logger.Error("LES_TypeData::addType '%s' invalid dataSize value:%d (must be 0)", name, dataSize)
				return -1

		if flags == 0:
			les_logger.Error("LES_TypeData::addType '%s' invalid flags value:0x%X (must be >= 0)", name, flags)
			return -1

		if flags & LES_TYPE_ALIAS:
			if self.doesTypeExist(aliasedName) == False:
				les_logger.Error("LES_TypeData::addType '%s' aliasedName:'%s' doesn't exist", name, aliasedName)
				return -1

		if numElements > 0:
			if (flags & LES_TYPE_ARRAY) == 0:
				les_logger.Error("LES_TypeData::addType '%s' numElements > 0 but ARRAY not in flags:'%s'", name, decodeFlags(flags))
				return -1

		if numElements < 0:
			les_logger.Error("LES_TypeData::addType '%s' invalid numElements value:%d (must be >= 0)", name, numElements)
			return -1

		if numElements <= 0:
			if flags & LES_TYPE_ARRAY:
				les_logger.Error("LES_TypeData::addType '%s' invalid numElements value:%d (must be >= 0)", name, numElements)
				return -1

		if flags & LES_TYPE_ARRAY:
			# Array types must be aliased to non-pointer, non-reference type
			if flags & LES_TYPE_ALIAS == 0:
				les_logger.Error("LES_TypeData::addType '%s' is an array with %d elements but not aliased, arrays must be aliased", name, numElements)
				return -1
			typeEntry = self.getTypeData(aliasedName)
			aliasedFlags = typeEntry.m_flags
			# Arrays of non-references must not be aliased to pointer or reference
			if ((flags & LES_TYPE_REFERENCE) == 0) and (aliasedFlags & LES_TYPE_POINTER) or (aliasedFlags & LES_TYPE_REFERENCE):
				les_logger.Error("LES_TyepData::addType '%s' non-reference array types must be aliased to a non-pointer, non-reference type Alias:'%s' Flags:0x%X %s", name, aliasedName, aliasedFlags, decodeFlags(aliasedFlags))
				return -1

		# Add the type name to the string table for good measure
		self.__m_stringTable__.addString(name)

		# compute the aliasedTypeNameID which is an index into the string table (future this could be a type ID into the type array)
		aliasedTypeID = 0xFFFFFFFF 
		if aliasedName != None:
			aliasedTypeID = self.__m_stringTable__.addString(aliasedName)

		nameHash = les_hash.LES_GenerateHashCaseSensitive(name)

		typeEntry = LES_TypeEntry(nameHash, dataSize, flags, aliasedTypeID, numElements)

		index = len(self.__m_typeEntries__)
		self.__m_typeEntries__.append(typeEntry)
		self.__m_typeNames__.append(name)

		return index

	def doesTypeExist(self, name):
		return name in self.__m_typeNames__

	def getTypeData(self, name):
		try:
			index = self.__m_typeNames__.index(name)
		except ValueError:
			return None
		return self.__m_typeEntries__[index]

	def write(self, binFile):
		# LES_TypeData
		# {
		# 	LES_int32 m_numTypes; 												- 4 bytes
		# 	LES_int32 m_settled; 													- 4-bytes, 0 in file
		#		LES_TypeEntry m_typeEntries[m_numTypes];	- 20 bytes * m_numTypes
		# };

		# 	LES_int32 m_numTypes; 												- 4 bytes
		numTypes = len(self.__m_typeEntries__)
		binFile.writeInt32(numTypes)

		# 	LES_int32 m_settled; 													- 4-bytes, 0 in file
		settled = 0
		binFile.writeInt32(settled)

		for typeEntry in self.__m_typeEntries__:
			# struct LES_TypeEntry
			# {
			#		LES_uint32 m_hash;													- 4 bytes
			#		LES_uint32 m_dataSize;											- 4 bytes
			#		LES_uint32 m_flags;													- 4 bytes
			#		LES_int32 m_aliasedTypeID;									- 4 bytes
			#		LES_int32 m_numElements;										- 4 bytes
			# };

			#		LES_uint32 m_hash;													- 4 bytes
			binFile.writeUint32(typeEntry.m_hash)

			#		LES_uint32 m_dataSize;											- 4 bytes
			binFile.writeUint32(typeEntry.m_dataSize)

			#		LES_uint32 m_flags;													- 4 bytes
			binFile.writeUint32(typeEntry.m_flags)

			#		LES_int32 m_aliasedTypeID;									- 4 bytes
			binFile.writeInt32(typeEntry.m_aliasedTypeID)

			#		LES_int32 m_numElements;										- 4 bytes
			binFile.writeInt32(typeEntry.m_numElements)

	def parseXML(self, xmlSource):
		#<?xml version='1.0' ?>
		#<LES_TYPES>
		#
		#	<LES_TYPE name="int*" dataSize="4" flags="INPUT|OUTPUT|POD|ARRAY" aliasedName="int" numElements="2" endianSwap="true" />
		# 	flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY, ENDIANSWAP
		#		aliasedName is optional, default is value of name
		#		numElements is optional, default is 0
		#		endianSwap is optional, default is false

		# <LES_TYPE_POD name="unsigned char" dataSize="1" />
		#		flags = INPUT|POD, fixed can't be specified
		#		aliasedName = input_name, fixed can't be specified
		#		numElements = 0, fixed can't be specified
		#		endianSwap = false, fixed can't be specified

		# <LES_TYPE_POD_ENDIANSWAP name="unsigned char" dataSize="1" />
		#		flags = INPUT|POD, fixed can't be specified
		#		aliasedName = input_name, fixed can't be specified
		#		numElements = 0, fixed can't be specified
		#		endianSwap = true, fixed can't be specified

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

		# <LES_TYPE_STRUCT name="TestStruct1" />
		#		flags = INPUT|STRUCT, fixed can't be specified
		#		aliasedName = input_name
		#		dataSize = 0 (to be computed later by the struct module)

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
			les_logger.Error("LES_TypeData::parseXML root tag should be LES_TYPES found %d", typesXML.tag)
			return 1

		numErrors = 0
		for typeXML in typesXML:
			needsDataSize = True
			needsFlags = False
			needsAlias = False
			needsNumElements = False
			needsEndianSwap = False

			dataSizeData = ""
			dataSizeDataDefault = None
			endianSwapData = ""
			endianSwapDataDefault = "false"
			flagsData = ""
			nameSuffix = ""
			aliasSuffix = ""
			nameSuffixAddNumElements = False

			if typeXML.tag == "LES_TYPE":
				needsFlags = True
				needsAlias = True
				needsNumElements = True
				needsEndianSwap = True
			elif typeXML.tag == "LES_TYPE_POD":
				flagsData = "INPUT|POD"
			elif typeXML.tag == "LES_TYPE_POD_ENDIANSWAP":
				flagsData = "INPUT|POD|ENDIANSWAP"
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
				needsDataSize = False
				flagsData = "INPUT|STRUCT"
				dataSizeDataDefault = "0"
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
				les_logger.Error("LES_TypeData::parseXML invalid node tag should be LES_TYPE, LES_TYPE_POD, LES_TYPE_POD_ENDIANSWAP, LES_TYPE_POD_POINTER, LES_TYPE_POD_REFERENCE, LES_TYPE_POD_ARRAY, LES_TYPE_POD_REFERENCE_ARRAY, LES_TYPE_STRUCT, LES_TYPE_STRUCT_POINTER, LES_TYPE_STRUCT_REFERENCE, LES_TYPE_STRUCT_ARRAY, LES_TYPE_STRUCT_REFERNCE_ARRAY found %s", typeXML.tag)
				numErrors += 1
				continue

			nameData = typeXML.get("name")
			if nameData == None:
				les_logger.Error("LES_TypeData::parseXML missing 'name' attribute:%s", xml.etree.ElementTree.tostring(typeXML))
				numErrors += 1
				continue
			name = nameData

			if needsDataSize:
				dataSizeData = typeXML.get("dataSize")
				if dataSizeData == None:
					dataSizeData = dataSizeDataDefault
					if dataSizeData == None:
						les_logger.Error("LES_TypeData::parseXML '%s' missing 'dataSize' attribute:%s", name, xml.etree.ElementTree.tostring(typeXML))
						numErrors += 1
						continue
			else:
				if typeXML.get("dataSize") != None:
					les_logger.Error("LES_TypeData::parseXML '%s' 'dataSize' attribute not allowed for this type definition:%s", name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue

			if needsFlags:
				flagsData = typeXML.get("flags")
				if flagsData == None:
					les_logger.Error("LES_TypeData::parseXML '%s' missing 'flags' attribute:%s", name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
			else:
				if typeXML.get("flags") != None:
					les_logger.Error("LES_TypeData::parseXML '%s' 'flags' attribute not allowed for this type definition:%s", name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue

			if flagsData == "":
				les_logger.Error("LES_TypeData::parseXML '%s' invalid 'flags' attribute:%s", name, xml.etree.ElementTree.tostring(typeXML))
				numErrors += 1
				continue

			if needsAlias:
				aliasedNameData = typeXML.get("aliasedName", nameData)
			else:
				if typeXML.get("aliasedName") != None:
					les_logger.Error("LES_TypeData::parseXML '%s' 'aliasedName' attribute not allowed for this type definition:%s", name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
				aliasedNameData = nameData + aliasSuffix

			if needsNumElements:
				numElementsData = typeXML.get("numElements", "0")
			else:
				if typeXML.get("numElements") != None:
					les_logger.Error("LES_TypeData::parseXML '%s' 'numElements' attribute not allowed for this type definition:%s", name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
				numElementsData = "0"
					
			if needsEndianSwap:
				endianSwapData = typeXML.get("endianSwap", endianSwapDataDefault)
			else:
				if typeXML.get("endianSwap") != None:
					les_logger.Error("LES_TypeData::parseXML '%s' 'endianSwap' attribute not allowed for this type definition:%s", name, xml.etree.ElementTree.tostring(typeXML))
					numErrors += 1
					continue
				endianSwapData = "false"

			aliasedName = aliasedNameData
			name = nameData + nameSuffix

			if needsDataSize:
				try:
					dataSize = int(dataSizeData)
				except ValueError:
					les_logger.Error("LES_TypeData::parseXML '%s' invalid dataSize value:'%s' (must be >= 1)", name, dataSizeData)
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
				if typeXML.tag == "LES_TYPE_STRUCT":
					dataSize = int(dataSizeDataDefault)
				else:
					if self.doesTypeExist(typeNameForDataSize):
						typeData = self.getTypeData(typeNameForDataSize)
						dataSize = typeData.m_dataSize

			if endianSwapData == "true":
				endianSwap = True
			elif endianSwapData == "false":
				endianSwap = False
			else:
				les_logger.Error("LES_TypeData::parseXML '%s' invalid endianSwap:'%s' expected 'true' or 'false'", name, endianSwapData)
				numErrors += 1
				continue

			# flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY, ENDIANSWAP delimiter is | e.g. "INPUT|POD"
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
				elif flag == "ENDIANSWAP":
					flags |= LES_TYPE_ENDIANSWAP
				else:
					les_logger.Error("LES_TypeData::parseXML '%s' invalid flag:'%s' flags:'%s'", name, flag, flagsData)
					numErrors += 1
					continue

			if endianSwap:
				flags |= LES_TYPE_ENDIANSWAP

			try:
				numElements = int(numElementsData)
			except ValueError:
				les_logger.Error("LES_TypeData::parseXML '%s' invalid numElements value:%s (must be >= 0)", name, numElementsData)
				numErrors += 1
				numElements = -1
				continue

			if nameSuffixAddNumElements:
				# name = name + "[<numELements>]"
				name = name + "[" + str(numElements) + "]"

			if aliasedName != name:
				flags |= LES_TYPE_ALIAS

			flagsDecoded = decodeFlags(flags)
			les_logger.Log("Type '%s' size:%d flags:0x%X %s aliasedName:'%s' numElements:%d", name, dataSize, flags, flagsDecoded, aliasedName, numElements)

			if self.doesTypeExist(name):
				les_logger.Warning("LES_TypeData::parseXML '%s' already exists", name)

			index = self.addType(name, dataSize, flags, aliasedName, numElements)
			if index == -1:
				les_logger.Error("LES_TypeData::parseXML '%s' failed to add type", name)
				numErrors += 1
				continue

		return numErrors

	def loadXML(self, fname):
		les_logger.Log("")
		les_logger.Log("#####################")
		les_logger.Log("Loading '%s'", fname)
		les_logger.Log("#####################")
		les_logger.Log("")
		if os.path.exists(fname) == False:
			les_logger.Error("File:'%s' not found", fname)
			return False

 		fh = open(fname)
		xmlData = fh.read()
		numErrors = self.parseXML(xmlData)
		if numErrors == 0:
			les_logger.Log("SUCCESS")
			return True
		else:
			les_logger.Error("numErrors=%d", numErrors)
			return False

	def DebugOutputTypes(self, loggerChannel):
		for typeEntry in self.__m_typeEntries__:
			hashValue = typeEntry.m_hash
			dataSize = typeEntry.m_dataSize
			flags = typeEntry.m_flags
			aliasedTypeID = typeEntry.m_aliasedTypeID
			numElements = typeEntry.m_numElements

			name = self.__m_stringTable__.getStringByHash(hashValue)
			flagsDecoded = decodeFlags(flags)
			aliasedName = self.__m_stringTable__.getString(aliasedTypeID)
			loggerChannel.Print("Type '%s' size:%d flags:0x%X %s aliasedName:'%s' numElements:%d", name, dataSize, flags, flagsDecoded, aliasedName, numElements)

def runTest():
	les_logger.Init()

	stringTable = les_stringtable.LES_StringTable()
	this = LES_TypeData(stringTable)
	index = this.addType("int", 4, LES_TYPE_INPUT|LES_TYPE_POD, "int")
	les_logger.Log("Index[int]= %d", index)
	index = this.addType("int", 2, 0x2, "int")
	les_logger.Error("Index[int]= %d", index)
	index = this.addType("int*", 4, LES_TYPE_INPUT|LES_TYPE_OUTPUT|LES_TYPE_POINTER|LES_TYPE_ALIAS, "int")
	index = this.addType("int[3]", 4*3, LES_TYPE_INPUT|LES_TYPE_OUTPUT|LES_TYPE_ARRAY|LES_TYPE_ALIAS, "int*", 3)

	binFile = les_binaryfile.LES_BinaryFile()
	binFile.setLittleEndian()
	this.write(binFile)
	binFile.saveToFile("typeDataLittle.bin")
	binFile.close()

	stringTable = les_stringtable.LES_StringTable()
	this = LES_TypeData(stringTable)

	this.loadXML("data/les_types_basic.xml")
	this.loadXML("data/les_types_test.xml")
	this.loadXML("data/les_types_errors.xml")

	testTypeChan = les_logger.CreateChannel("TypeDebug", "", "typeDebug_py.txt", les_logger.LES_LOGGERCHANNEL_FLAGS_DEFAULT)
	this.DebugOutputTypes(testTypeChan)

if __name__ == '__main__':
	runTest()
