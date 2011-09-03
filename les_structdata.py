#!/usr/bin/python

import les_hash
import les_binaryfile
import les_stringtable
import les_typedata
import xml.etree.ElementTree
import les_logger

#	LES_StructMember
#	{
#		uint32 m_hash;
#		int32 m_nameID;
#		int32 m_typeID;
#		int32 m_dataSize;
#		int32 m_alignmentPadding;
#	};

# LES_StructDefinition
# {
#		int32 m_nameID;
#		int32 m_numMembers;
#		LES_StructMember m_members[m_numMembers];
#	};

# LES_StructData
# {
# 	int32 m_numStructDeinfitions; 												- 4-bytes
# 	int32 m_settled; 																			- 4-bytes, 0 in file
#		int32 m_offsets[m_numStructs];												- 4-bytes * m_numStructs
#		LES_StructDefinition m_structDefinitions[m_numTypes];	- variable 
# };

class LES_StructMember():
	def __init__(self, hashValue, nameID, typeID, dataSize, alignmentPadding):
		self.m_hash = hashValue
		self.m_nameID = nameID
		self.m_typeID = typeID
		self.m_dataSize = dataSize
		self.m_alignmentPadding = alignmentPadding

class LES_StructDefinintion():
	def __init__(self, nameID, numMembers):
		self.__m_nameID__ = nameID
		self.__m_numMembers__ = numMembers
		self.__m_members__ = []
		emptyStructMember = LES_StructMember(0, -1, -1, 0, 0)
		for i in range(numMembers):
			self.__m_members__.append(emptyStructMember)
		self.__m_numAddedMembers__ = 0
		self.__m_totalMemberSizeWithPadding__ = 0

	def GetNameID(self):
			return self.__m_nameID__

	def GetNumMembers(self):
			return self.__m_numMembers__

	def GetMemberByIndex(self, index):
			return self.__m_members__[index]

	def GetMember(self, hashValue):
		for member in self.__m_members__:
			if member.m_hash == hashValue:
				return True
		return False

	def AddMember(self, typeName, name, stringTable, typeData, structData):
		structName = stringTable.getString(self.__m_nameID__)

		# Error if member index off the end of the array
		maxNumMembers = self.GetNumMembers()
		numAddedMembers = self.__m_numAddedMembers__
		if numAddedMembers >= maxNumMembers:
			les_logger.FatalError("TEST struct '%s' : MemberIndex too big index:%d max:%d member:'%s' type:'%s'",
														structName, self.__m_numAddedMembers__,  maxNumMembers, name, typeName)
			return False

		nameHash = les_hash.GenerateHashCaseSensitive(name)
		# Test to see if the member has already been added
		if self.GetMember(nameHash):
			les_logger.FatalError("TEST struct '%s' : member '%s' already exists type:'%s'", structName, name, typeName)
			return False

		typeID = stringTable.getStringID(typeName)
		typeEntry = typeData.getTypeData(typeName)
		if typeEntry == None:
			les_logger.FatalError("TEST struct '%s' : member '%s' type '%s' not found", structName, name, typeName)
			return False

		memberDataSize = typeEntry.m_dataSize
		memberFlags = typeEntry.m_flags
		if memberFlags & les_typedata.LES_TYPE_ARRAY and (memberFlags & les_typedata.LES_TYPE_REFERENCE == 0):
			# Member variables which are arrays and not references are not pointers but are N * TYPE
			numElements = typeEntry.m_numElements
			dataSize = ComputeDataStorageSize(typeEntry, stringTable, typeData, structData)
			memberDataSize = dataSize * numElements
			memberDataSize = dataSize

		memberAlignment = ComputeAlignment(typeEntry, stringTable, typeData, structData)
		alignmentPadding = StructComputeAlignmentPadding(self.__m_totalMemberSizeWithPadding__, memberAlignment)

		nameID = stringTable.addString(name)
		structMember = LES_StructMember(nameHash, nameID, typeID, memberDataSize, alignmentPadding)

		self.__m_members__[numAddedMembers] = structMember
		self.__m_totalMemberSizeWithPadding__ += (memberDataSize + alignmentPadding)
		self.__m_numAddedMembers__ += 1
		les_logger.Log("Struct:'%s' Member:'%s' Type:'%s' 0x%X DataSize:%d aligmentPadding:%d", 
										structName, name, typeName, les_hash.GenerateHashCaseSensitive(typeName), memberDataSize, alignmentPadding)
		return True

class LES_StructData():
	def __init__(self, stringTable):
		self.__m_structDefinitions__ = []
		self.__m_structDefinitionNames__ = []
		self.__m_stringTable__ = stringTable

	def addStructDefinition(self, name, structDefinition):
		if self.doesStructDefinitionExist(name):
			les_logger.Error("LES_StructData::addStructDefinition '%s' but struct definition already exists", name)
			return -1

		# Validate name should match nameID
		nameID = structDefinition.GetNameID()
		nameIDstring = self.__m_stringTable__.getString(nameID)
		if nameIDstring != name:
			les_logger.Error("LES_StructData::addStructDefinition '%s' nameID:%d '%s' doesn't match name:'%s'", name, nameID, nameIDstring, name)
			return -1

		numMembers = structDefinition.GetNumMembers()
		# Validate numMembers > 0
		if numMembers <= 0:
			les_logger.Error("LES_StructData::addStructDefinition '%s' invalid numMembers:%d must be > 0", name, numMembers)
			return -1

		index = len(self.__m_structDefinitions__)
		self.__m_structDefinitions__.append(structDefinition)
		self.__m_structDefinitionNames__.append(name)
		return index

	def doesStructDefinitionExist(self, name):
		return name in self.__m_structDefinitionNames__

	def getStructDefition(self, name):
		try:
			index = self.__m_structDefinitionNames__.index(name)
		except ValueError:
			return None
		return self.__m_structDefinitions__[index]

	def writeFile(self, binFile):
		# LES_StructData
		# {
		# 	int32 m_numStructDeinfitions; 												- 4-bytes
		# 	int32 m_settled; 																			- 4-bytes, 0 in file
		#		int32 m_offsets[m_numStructs];												- 4-bytes * m_numStructs
		#		LES_StructDefinition m_structDefinitions[m_numTypes];	- variable 
		# };

		# 	int32 m_numStructDefinitions; 										- 4 bytes
		numStructDefinitions = len(self.__m_structDefinitions__)
		binFile.writeInt(numStructDefinitions)

		# 	int32 m_settled; 																	- 4-bytes, 0 in file
		settled = 0
		binFile.writeInt(settled)
#
#		for typeEntry in self.__m_typeEntries__:
#			# struct LES_TypeEntry
#			# {
#			#		uint32 m_hash;													- 4 bytes
#			#		uint32 m_dataSize;											- 4 bytes
#			#		uint32 m_flags;													- 4 bytes
#			#		int32 m_aliasedTypeID;									- 4 bytes
#			#		int32 m_numElements;										- 4 bytes
#			# };
#
#			#		uint32 m_hash;													- 4 bytes
#			binFile.writeUint(typeEntry.hashValue)
#
#			#		uint32 m_dataSize;											- 4 bytes
#			binFile.writeUint(typeEntry.dataSize)
#
#			#		uint32 m_flags;													- 4 bytes
#			binFile.writeUint(typeEntry.flags)
#
#			#		int32 m_aliasedTypeID;									- 4 bytes
#			binFile.writeUint(typeEntry.aliasedTypeID)
#
#			#		int32 m_numElements;										- 4 bytes
#			binFile.writeUint(typeEntry.numElements)

	def parseXML(self, xmlSource):
		numErrors = 1
#		#<?xml version='1.0' ?>
#		#<LES_TYPES>
#		#
#		#	<LES_TYPE name="int*" dataSize="4" flags="INPUT|OUTPUT|POD|ARRAY" aliasedName="int" numElements="2" />
#		# 	flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY
#		#		aliasedName is optional, default is value of name
#		#		numElements is optional, default is 0
#		#
#		# <LES_TYPE_POD name="unsigned char" dataSize="1" />
#		#		flags = INPUT|POD, fixed can't be specified
#		#		aliasedName = input_name, fixed can't be specified
#		#		numElements = 0, fixed can't be specified
#
#		# <LES_TYPE_POD_POINTER name="unsigned char" />
#		#		name = input_name + "*"
#		#		dataSize = 4 by default : this could be set by some global params in the XML in the future
#		#		flags = INPUT|OUTPUT|POD|POINTER, fixed can't be specified
#		#		aliasedName = input_name, fixed can't be specified
#		#		numElements = 0, fixed can't be specified
#
#		# <LES_TYPE_POD_REFERENCE name="int" />
#		#		name = input_name + "&"
#		#		dataSize = dataSize value of type with the aliasedName
#		#		flags = INPUT|OUTPUT|POD|REFERENCE, fixed can't be specified
#		#		aliasedName = input_name + "*", fixed can't be specified
#		#		numElements = 0, fixed can't be specified
#
#		# <LES_TYPE_POD_ARRAY name="int" numElements="4" />
#		#		name = input_name + "[<numElements>]"
#		#		dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
#		#		flags = INPUT|OUTPUT|POD|ARRAY, fixed can't be specified
#		#		aliasedName = input_name
#
#		# <LES_TYPE_POD_REFERENCE_ARRAY name="int" numElements="4" />
#		#		name = input_name + "&[<numElements>]"
#		#		dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
#		#		flags = INPUT|OUTPUT|POD|REFERENCE|ARRAY, fixed can't be specified
#		#		aliasedName = input_name+"*"
#
#		# <LES_TYPE_STRUCT name="TestStruct1" dataSize="10" />
#		#		flags = INPUT|STRUCT, fixed can't be specified
#		#		aliasedName = input_name
#
#		# <LES_TYPE_STRUCT_POINTER name="TestStruct1" />
#		#		name = input_name + "*"
#		#		dataSize = 4 by default : this could be set by some global params in the XML in the future
#		#		flags = INPUT|OUTPUT|STRUCT|POINTER, fixed can't be specified
#		#		aliasedName = input_name, fixed can't be specified
#		#		numElements = 0, fixed can't be specified
#
#		# <LES_TYPE_STRUCT_REFERNCE name="TestStruct1" />
#		#		name = input_name + "&"
#		#		dataSize = dataSize value of type with the aliasedName
#		#		flags = INPUT|OUTPUT|STRUCT|REFERENCE, fixed can't be specified
#		#		aliasedName = input_name + "*", fixed can't be specified
#		#		numElements = 0, fixed can't be specified
#
#		# <LES_TYPE_STRUCT_ARRAY
#		#		name = input_name + "[<numElements>]"
#		#		dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
#		#		flags = INPUT|OUTPUT|STRUCT|ARRAY, fixed can't be specified
#		#		aliasedName = input_name
#
#		# <LES_TYPE_STRUCT_REFERENCE_ARRAY name="TestStruct2" numElements="5" />
#		#		name = input_name + "&[<numElements>]"
#		#		dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
#		#		flags = INPUT|OUTPUT|STRUCT|REFERENCE|ARRAY, fixed can't be specified
#		#		aliasedName = input_name+"*"
#
#		typesXML = xml.etree.ElementTree.XML(xmlSource)
#		if typesXML.tag != "LES_TYPES":
#			les_logger.Error("LES_TypeData::parseXML root tag should be LES_TYPES found %d", typesXML.tag)
#			return 1
#
#		numErrors = 0
#		for typeXML in typesXML:
#			needsDataSize = True
#			needsFlags = False
#			needsAlias = False
#			needsNumElements = False
#
#			dataSizeData = ""
#			dataSizeDataDefault = None
#			flagsData = ""
#			nameSuffix = ""
#			aliasSuffix = ""
#			nameSuffixAddNumElements = False
#
#			if typeXML.tag == "LES_TYPE":
#				needsFlags = True
#				needsAlias = True
#				needsNumElements = True
#			elif typeXML.tag == "LES_TYPE_POD":
#				flagsData = "INPUT|POD"
#			elif typeXML.tag == "LES_TYPE_POD_POINTER":
#				flagsData = "INPUT|OUTPUT|POD|POINTER"
#				nameSuffix = "*"
#				dataSizeDataDefault = "4"
#			elif typeXML.tag == "LES_TYPE_POD_REFERENCE":
#				needsDataSize = False
#				flagsData = "INPUT|OUTPUT|POD|REFERENCE"
#				nameSuffix = "&"
#				aliasSuffix = "*"
#			elif typeXML.tag == "LES_TYPE_POD_ARRAY":
#				needsDataSize = False
#				flagsData = "INPUT|OUTPUT|POD|ARRAY"
#				nameSuffix = ""
#				nameSuffixAddNumElements = True
#				aliasSuffix = ""
#				needsNumElements = True
#			elif typeXML.tag == "LES_TYPE_POD_REFERENCE_ARRAY":
#				needsDataSize = False
#				flagsData = "INPUT|OUTPUT|POD|REFERENCE|ARRAY"
#				nameSuffix = "&"
#				nameSuffixAddNumElements = True
#				aliasSuffix = "*"
#				needsNumElements = True
#			elif typeXML.tag == "LES_TYPE_STRUCT":
#				flagsData = "INPUT|STRUCT"
#			elif typeXML.tag == "LES_TYPE_STRUCT_POINTER":
#				flagsData = "INPUT|OUTPUT|STRUCT|POINTER"
#				nameSuffix = "*"
#				dataSizeDataDefault = "4"
#			elif typeXML.tag == "LES_TYPE_STRUCT_REFERENCE":
#				needsDataSize = False
#				flagsData = "INPUT|OUTPUT|STRUCT|REFERENCE"
#				nameSuffix = "&"
#				aliasSuffix = "*"
#			elif typeXML.tag == "LES_TYPE_STRUCT_ARRAY":
#				needsDataSize = False
#				flagsData = "INPUT|OUTPUT|STRUCT|ARRAY"
#				nameSuffix = ""
#				nameSuffixAddNumElements = True
#				aliasSuffix = ""
#				needsNumElements = True
#			elif typeXML.tag == "LES_TYPE_STRUCT_REFERENCE_ARRAY":
#				needsDataSize = False
#				flagsData = "INPUT|OUTPUT|STRUCT|REFERENCE|ARRAY"
#				nameSuffix = "&"
#				nameSuffixAddNumElements = True
#				aliasSuffix = "*"
#				needsNumElements = True
#			else:
#				les_logger.Error("LES_TypeData::parseXML invalid node tag should be LES_TYPE, LES_TYPE_POD, LES_TYPE_POD_POINTER, LES_TYPE_POD_REFERENCE, LES_TYPE_POD_ARRAY, LES_TYPE_POD_REFERENCE_ARRAY, LES_TYPE_STRUCT, LES_TYPE_STRUCT_POINTER, LES_TYPE_STRUCT_REFERENCE, LES_TYPE_STRUCT_ARRAY, LES_TYPE_STRUCT_REFERNCE_ARRAY found %s", typeXML.tag)
#				numErrors += 1
#				continue
#
#			nameData = typeXML.get("name")
#			if nameData == None:
#				les_logger.Error("LES_TypeData::parseXML missing 'name' attribute:%s", xml.etree.ElementTree.tostring(typeXML))
#				numErrors += 1
#				continue
#			name = nameData
#
#			if needsDataSize:
#				dataSizeData = typeXML.get("dataSize")
#				if dataSizeData == None:
#					dataSizeData = dataSizeDataDefault
#					if dataSizeData == None:
#						les_logger.Error("LES_TypeData::parseXML '%s' missing 'dataSize' attribute:%s", name, xml.etree.ElementTree.tostring(typeXML))
#						numErrors += 1
#						continue
#			else:
#				if typeXML.get("dataSize") != None:
#					les_logger.Error("LES_TypeData::parseXML '%s' 'dataSize' attribute not allowed for this type definition:%s", name, xml.etree.ElementTree.tostring(typeXML))
#					numErrors += 1
#					continue
#
#			if needsFlags:
#				flagsData = typeXML.get("flags")
#				if flagsData == None:
#					les_logger.Error("LES_TypeData::parseXML '%s' missing 'flags' attribute:%s", name, xml.etree.ElementTree.tostring(typeXML))
#					numErrors += 1
#					continue
#			else:
#				if typeXML.get("flags") != None:
#					les_logger.Error("LES_TypeData::parseXML '%s' 'flags' attribute not allowed for this type definition:%s", name, xml.etree.ElementTree.tostring(typeXML))
#					numErrors += 1
#					continue
#
#			if flagsData == "":
#				les_logger.Error("LES_TypeData::parseXML '%s' invalid 'flags' attribute:%s", name, xml.etree.ElementTree.tostring(typeXML))
#				numErrors += 1
#				continue
#
#			if needsAlias:
#				aliasedNameData = typeXML.get("aliasedName", nameData)
#			else:
#				if typeXML.get("aliasedName") != None:
#					les_logger.Error("LES_TypeData::parseXML '%s' 'aliasedName' attribute not allowed for this type definition:%s", name, xml.etree.ElementTree.tostring(typeXML))
#					numErrors += 1
#					continue
#				aliasedNameData = nameData + aliasSuffix
#
#			if needsNumElements:
#				numElementsData = typeXML.get("numElements", "0")
#			else:
#				if typeXML.get("numElements") != None:
#					les_logger.Error("LES_TypeData::parseXML '%s' 'numElements' attribute not allowed for this type definition:%s", name, xml.etree.ElementTree.tostring(typeXML))
#					numErrors += 1
#					continue
#				numElementsData = "0"
#					
#			aliasedName = aliasedNameData
#			name = nameData + nameSuffix
#
#			if needsDataSize:
#				try:
#					dataSize = int(dataSizeData)
#				except ValueError:
#					les_logger.Error("LES_TypeData::parseXML '%s' invalid dataSize value:'%s' (must be >= 1)", name, dataSizeData)
#					numErrors += 1
#					continue
#			else:
#				if typeXML.tag == "LES_TYPE_POD_REFERENCE":
#					# Get it from the data size of the alias - the rule
#						typeNameForDataSize = aliasedName
#				elif typeXML.tag == "LES_TYPE_POD_ARRAY":
#					# dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
#					typeNameForDataSize = name + "*"
#				elif typeXML.tag == "LES_TYPE_STRUCT_REFERENCE":
#					# Get it from the data size of the alias - the rule
#						typeNameForDataSize = aliasedName
#				elif typeXML.tag == "LES_TYPE_POD_REFERENCE_ARRAY":
#					# dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
#					typeNameForDataSize = name + "*"
#				elif typeXML.tag == "LES_TYPE_STRUCT_ARRAY":
#					# dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
#					typeNameForDataSize = name + "*"
#				elif typeXML.tag == "LES_TYPE_STRUCT_REFERENCE_ARRAY":
#					# dataSize = dataSize value of type with the input_name+"*" e.g. size of the pointer type
#					typeNameForDataSize = name + "*"
#				if self.doesTypeExist(typeNameForDataSize):
#					typeData = self.getTypeData(typeNameForDataSize)
#					dataSize = typeData.dataSize
#
#			# flags = INPUT, OUTPUT, POD, STRUCT, POINTER, STRUCT, REFERENCE, ALIAS, ARRAY, delimiter is | e.g. "INPUT|POD"
#			flagsArray = flagsData.split('|')
#			flags = int(0)
#			for flag in flagsArray:
#				if flag == "INPUT":
#					flags |= LES_TYPE_INPUT
#				elif flag == "OUTPUT":
#					flags |= LES_TYPE_OUTPUT
#				elif flag == "POD":
#					flags |= LES_TYPE_POD
#				elif flag == "STRUCT":
#					flags |= LES_TYPE_STRUCT
#				elif flag == "POINTER":
#					flags |= LES_TYPE_POINTER
#				elif flag == "REFERENCE":
#					flags |= LES_TYPE_REFERENCE
#				elif flag == "ALIAS":
#					flags |= LES_TYPE_ALIAS
#				elif flag == "ARRAY":
#					flags |= LES_TYPE_ARRAY
#				else:
#					les_logger.Error("LES_TypeData::parseXML '%s' invalid flag:'%s' flags:'%s'", name, flag, flagsData)
#					numErrors += 1
#					continue
#
#			try:
#				numElements = int(numElementsData)
#			except ValueError:
#				les_logger.Error("LES_TypeData::parseXML '%s' invalid numElements value:%s (must be >= 0)", name, numElementsData)
#				numErrors += 1
#				numElements = -1
#				continue
#
#			if nameSuffixAddNumElements:
#				# name = name + "[<numELements>]"
#				name = name + "[" + str(numElements) + "]"
#
#			if aliasedName != name:
#				flags |= LES_TYPE_ALIAS
#
#			flagsDecoded = decodeFlags(flags)
#			les_logger.Log("Type '%s' size:%d flags:0x%X %s aliasedName:'%s' numElements:%d", name, dataSize, flags, flagsDecoded, aliasedName, numElements)
#
#			if self.doesTypeExist(name):
#				les_logger.Warning("LES_TypeData::parseXML '%s' already exists", name)
#
#			index = self.addType(name, dataSize, flags, aliasedName, numElements)
#			if index == -1:
#				les_logger.Error("LES_TypeData::parseXML '%s' failed to add type", name)
#				numErrors += 1
#				continue
#
		return numErrors

	def loadXML(self, fname):
		les_logger.Log("")
		les_logger.Log("#####################")
		les_logger.Log("Loading %s", fname)
		les_logger.Log("#####################")
		les_logger.Log("")
 		fh = open(fname)
		xmlData = fh.read()
		numErrors = self.parseXML(xmlData)
		if numErrors == 0:
			les_logger.Log("SUCCESS")
		else:
			les_logger.Error("numErrors=%d", numErrors)

#	def DebugOutputTypes(self, loggerChannel):
#		for typeEntry in self.__m_typeEntries__:
#			hashValue = typeEntry.hashValue
#			dataSize = typeEntry.dataSize
#			flags = typeEntry.flags
#			aliasedTypeID = typeEntry.aliasedTypeID
#			numElements = typeEntry.numElements
#
#			name = self.__m_stringTable__.getStringByHash(hashValue)
#			flagsDecoded = decodeFlags(flags)
#			aliasedName = self.__m_stringTable__.getString(aliasedTypeID)
#			loggerChannel.Print("Type '%s' size:%d flags:0x%X %s aliasedName:'%s' numElements:%d", name, dataSize, flags, flagsDecoded, aliasedName, numElements)

def GetRootType(typeEntry, stringTable, typeData):
	flags = typeEntry.m_flags
	while flags & les_typedata.LES_TYPE_ALIAS:
		aliasedTypeID = typeEntry.m_aliasedTypeID
		les_logger.Log("Type 0x%X alias:%d flags:0x%X", typeEntry.m_hash, aliasedTypeID, flags)
		aliasedStringType = stringTable.getString(aliasedTypeID)
		if aliasedStringType == None:
			les_logger.FatalError("ComputeAlignment aliased type:%d string can't be found", aliasedTypeID)
			return -1;
		aliasedTypeEntry = typeData.getTypeData(aliasedStringType)
		if aliasedTypeEntry == None:
			les_logger.FatalError("ComputeAlignment aliased type not found aliased type:'%s'", aliasedStringType)
			return -1;
		les_logger.Log("Compute Alignment Type 0x%X alias:%s", typeEntry.m_hash, aliasedStringType)

		typeEntry = aliasedTypeEntry
		flags = typeEntry.m_flags
		les_logger.Log("Alias Type 0x%X flags:0x%X", typeEntry.m_hash, flags)
	return typeEntry

def ComputeAlignment(typeEntry, stringTable, typeData, structData):
	flags = typeEntry.m_flags
	if flags & les_typedata.LES_TYPE_ARRAY:
		typeEntry = GetRootType(typeEntry, stringTable, typeData)
		flags = typeEntry.m_flags

	if flags & les_typedata.LES_TYPE_STRUCT:
		typeEntry = GetRootType(typeEntry, stringTable, typeData)
		flags = typeEntry.m_flags

		structDefinition = structData.getStructDefinition(typeEntry.m_hash)
		if structDefinition == None:
			les_logger.FatalError("ComputeAlignment type:0x%X is a struct but can't be found", typeEntry.m_hash)
			return -1

		numMembers = structDefinition.GetNumMembers()
		maxAlignment = 0
		for i in range(numMembers):
			structMember = structDefinition.GetMemberByIndex(i)
			memberTypeID = structMember.m_typeID
			memberTypeString = stringTable.getString(memberTypeID)
			memberTypeEntry = typedata.getTypeData(memberTypeString)
			alignment = ComputeAlignment(memberTypeEntry, stringTable, typeData, structData)
			if alignment > maxAlignment:
				maxAlignment = alignment
			les_logger.Log("Compute Alignment Type 0x%X Struct:%d alignment:%d", typeEntry.m_hash, structDefinition.GetNameID(), maxAlignment)
		return maxAlignment;

	dataSize = typeEntry.m_dataSize
	alignment = dataSize
#	les_logger.Log("Compute Alignment Type 0x%X alignment:%d", typeEntry.m_hash, alignment)
	return alignment

def ComputeDataStorageSize(typeEntry, stringTable, typeData, structData):
	flags = typeEntry.m_flags
	numElements = typeEntry.m_numElements
	typeData = GetRootType(typeEntry, stringTable, typeData)
	flags = typeEntry.m_flags

	if flags & les_typedata.LES_TYPE_STRUCT:
		structDefinition = structData.getStructDefinition(typeEntry.m_hash)
		if structDefinition == None:
			les_logger.Warning("ComputeDataStorage type:0x%X is a struct but can't be found", typeEntry.m_hash)
			return -1

		numMembers = structDefinition.GetNumMembers()
		totalDataSize = 0
		for i in range(numMembers):
			structMember = structDefinition.GetMemberByIndex(i)
			memberTypeID = structMember.m_typeID
			memberTypeString = stringTable.getString(memberTypeID)
			memberTypeEntry = typeData.getTypeData(memberTypeString)
			dataSize = ComputeDataStorageSize(memberTypeEntry, stringTable, typeData, structData)
			if dataSize == -1:
				return -1
			totalDataSize += dataSize

		if numElements > 1:
#			les_logger.Log("Struct Type:0x%X Size:%d NumELements:%d", typeEntry.m_hash, totalDataSize, numElements)
			totalDataSize *= numElements
		return totalDataSize

	dataSize = typeEntry.m_dataSize
	if numElements > 1:
#		les_logger.Log("POD Type:0x%X Size:%d NumELements:%d", typeEntry.m_hash, dataSize, numElements)
		dataSize *= numElements

	return dataSize

def StructComputeAlignmentPadding(totalMemberSize, memberDataSize):
	memberAlignmentSizeMinus1 = memberDataSize - 1
	# Max alignment is 4 bytes
	if memberAlignmentSizeMinus1 > 3:
		memberAlignmentSizeMinus1 = 3

	alignedOffset = ((totalMemberSize + memberAlignmentSizeMinus1) & ~memberAlignmentSizeMinus1)
	alignmentPadding = alignedOffset - totalMemberSize

#	les_logger.Log("TotalMemberSize:0x%X MemberDataSize:0x%X Alignment:%d", totalMemberSize, memberDataSize, alignmentPadding)
	if alignmentPadding < 0:
		les_logger.FatalError("HELP");

	return alignmentPadding;

def runTest():
	les_logger.Init()

	stringTable = les_stringtable.LES_StringTable()
	typeData = les_typedata.LES_TypeData(stringTable)

	this = LES_StructData(stringTable)

	nameID = stringTable.addString("Jake")
	structDefinition = LES_StructDefinintion(nameID,1)
	this.addStructDefinition("Jake", structDefinition)

	structDefinition = LES_StructDefinintion(nameID,1)
	this.addStructDefinition("Jake", structDefinition)

	structDefinition = LES_StructDefinintion(nameID,0)
	this.addStructDefinition("Bob", structDefinition)

	nameID = stringTable.addString("Dick")
	structDefinition = LES_StructDefinintion(nameID,0)
	this.addStructDefinition("Dick", structDefinition)

	typeData.addType("char", 1, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "char")
	typeData.addType("short", 2, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "short")
	typeData.addType("int", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "int")
	typeData.addType("float", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "float")
	nameID = stringTable.addString("TestStruct1")
	structDefinition = LES_StructDefinintion(nameID, 2)
	structDefinition.AddMember("char", "m_char", stringTable, typeData, this)
	structDefinition.AddMember("float", "m_float", stringTable, typeData, this)
	this.addStructDefinition("TestStruct1", structDefinition)

	binFile = les_binaryfile.LES_BinaryFile("structDefinitionLittle.bin")
	binFile.setLittleEndian()
	this.writeFile(binFile)
	binFile.close()

	stringTable = les_stringtable.LES_StringTable()
	this = LES_StructData(stringTable)

#	this.loadXML("data/les_types_basic.xml")
#	this.loadXML("data/les_types_test.xml")
#	this.loadXML("data/les_types_errors.xml")

#	testTypeChan = les_logger.CreateChannel("TypeDebug", "", "typeDebug_py.txt", les_logger.LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT | les_logger.LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT)
#	this.DebugOutputTypes(testTypeChan)

if __name__ == '__main__':
	runTest()
