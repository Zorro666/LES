#!/usr/bin/python

import os.path
import xml.etree.ElementTree

import les_hash
import les_binaryfile
import les_stringtable
import les_typedata
import les_logger

#	LES_StructMember
#	{
#		LES_uint32 m_hash;
#		LES_int32 m_nameID;
#		LES_int32 m_typeID;
#		LES_int32 m_dataSize;
#		LES_int32 m_alignmentPadding;
#	};

# LES_StructDefinition
# {
#		LES_int32 m_nameID;
#		LES_int32 m_numMembers;
#		LES_StructMember m_members[m_numMembers];
#	};

# LES_StructData
# {
# 	LES_int32 m_numStructDefinitions; 																		- 4-bytes
# 	LES_int32 m_settled; 																									- 4-bytes, 0 in file
#		LES_uint32 m_structDefinitionOffsets[m_numStructDefinitions];					- 4-bytes * m_numStructDefinitions
#		LES_StructDefinition m_structDefinitions[m_numStructDefinitions];	- variable 
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
		self.__m_maxMemberSizeAlignment__ = 0
		self.__m_finalised__ = False

	def IsFinalised(self):
		return self.__m_finalised__ 

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

	def GetTotalMemberSizeWithPadding(self):
			return self.__m_totalMemberSizeWithPadding__

	def AddMember(self, typeName, name, stringTable, typeData, structData):
		structName = stringTable.getString(self.__m_nameID__)

		# Error if member index off the end of the array
		maxNumMembers = self.GetNumMembers()
		numAddedMembers = self.__m_numAddedMembers__
		if numAddedMembers >= maxNumMembers:
			les_logger.FatalError("AddMember struct '%s' : MemberIndex too big index:%d max:%d member:'%s' type:'%s'",
														structName, self.__m_numAddedMembers__,  maxNumMembers, name, typeName)
			return False

		nameHash = les_hash.GenerateHashCaseSensitive(name)
		# Test to see if the member has already been added
		if self.GetMember(nameHash):
			les_logger.FatalError("AddMember struct '%s' : member '%s' already exists type:'%s'", structName, name, typeName)
			return False

		typeID = stringTable.getStringID(typeName)
		typeEntry = typeData.getTypeData(typeName)
		if typeEntry == None:
			les_logger.FatalError("AddMember struct '%s' : member '%s' type '%s' not found", structName, name, typeName)
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
		if memberAlignment < 0:
			les_logger.FatalError("Struct:'%s' AddMember:'%s' Type:'%s' 0x%X ComputeAlignment failed",
														structName, name, typeName, typeEntry.m_hash)
		if memberAlignment > self.__m_maxMemberSizeAlignment__:
			self.__m_maxMemberSizeAlignment__ = memberAlignment
		alignmentPadding = StructComputeAlignmentPadding(self.__m_totalMemberSizeWithPadding__, memberAlignment)

		nameID = stringTable.addString(name)
		structMember = LES_StructMember(nameHash, nameID, typeID, memberDataSize, alignmentPadding)

		self.__m_members__[numAddedMembers] = structMember
		self.__m_totalMemberSizeWithPadding__ += (memberDataSize + alignmentPadding)
		self.__m_numAddedMembers__ += 1
		les_logger.Log("  Struct:'%s' Member:'%s' Type:'%s' 0x%X DataSize:%d aligmentPadding:%d", 
										structName, name, typeName, les_hash.GenerateHashCaseSensitive(typeName), memberDataSize, alignmentPadding)
		return True

	def Finalise(self, structName):
		self.__m_finalised__ = False
		if self.__m_numAddedMembers__ != self.__m_numMembers__:
				les_logger.Error("LES_StructDefintion::Finalise '%s' not enough members added numAdded:%d numMembers:%d", 
												 structName, numAdded, numMembers)
				return False
		# Align the structure to the biggest size in the structure but a maximum of 4-byte or 8-byte
		# should make the 4-byte or 8-byte a variable (from global XML params)
		structMaxAlignment = 8
		structMaxAlignment = 4
		structAlignment = self.__m_maxMemberSizeAlignment__
		if structAlignment > structMaxAlignment:
			structAlignment = structMaxAlignment

		paddingAmount = StructComputeAlignmentPadding(self.__m_totalMemberSizeWithPadding__, structAlignment)
		self.__m_totalMemberSizeWithPadding__ += paddingAmount

		self.__m_finalised__ = True
		return True

class LES_StructData():
	def __init__(self, stringTable, typeData):
		self.__m_structDefinitions__ = []
		self.__m_structDefinitionNames__ = []
		self.__m_stringTable__ = stringTable
		self.__m_typeData__ = typeData

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

		typeEntry = self.__m_typeData__.getTypeData(name)
		if typeEntry == None:
			les_logger.Error("LES_StructData::addStructDefinition '%s' typeData not found", name)
			return -1

		structSize = structDefinition.GetTotalMemberSizeWithPadding()
		if typeEntry.m_dataSize == 0:
			typeEntry.m_dataSize = structSize

		typeEntry = self.__m_typeData__.getTypeData(name)
		if structSize != typeEntry.m_dataSize:
			les_logger.Error("LES_StructData::addStructDefintiion '%s' structSize doesn't match type definition structSize:%d typeDataSize:%d", 
									 			name, structSize, typeEntry.m_dataSize)
			return -1

		if structDefinition.IsFinalised() == False:
			les_logger.Error("LES_StructData::addStructDefintiion '%s' structure isn't finalised", name)
			return -1

		index = len(self.__m_structDefinitions__)
		self.__m_structDefinitions__.append(structDefinition)
		self.__m_structDefinitionNames__.append(name)

		return index

	def doesStructDefinitionExist(self, name):
		return name in self.__m_structDefinitionNames__

	def getStructDefinitionByHash(self, nameHash):
		name = self.__m_stringTable__.getStringByHash(nameHash)
		return self.getStructDefinition(name)

	def getStructDefinition(self, name):
		try:
			index = self.__m_structDefinitionNames__.index(name)
		except ValueError:
			return None
		return self.__m_structDefinitions__[index]

	def writeFile(self, binFile):
		basePosition = binFile.getIndex()
		# LES_StructData
		# {
		# 	LES_int32 m_numStructDefinitions; 																		- 4-bytes
		# 	LES_int32 m_settled; 																									- 4-bytes, 0 in file
		#		LES_uint32 m_structDefinitionOffsets[m_numStructDefinitions];					- 4-bytes * m_numStructDefinitions
		#		LES_StructDefinition m_structDefinitions[m_numStructDefinitions];	- variable 
		# };

		# 	LES_int32 m_numStructDefinitions; 										- 4 bytes
		numStructDefinitions = len(self.__m_structDefinitions__)
		binFile.writeInt32(numStructDefinitions)

		# 	LES_int32 m_settled; 																	- 4-bytes, 0 in file
		settled = 0
		binFile.writeInt32(settled)

		#		LES_uint32 m_structDefinitionOffsets[m_numStructDefinitions];	- 4-bytes * m_numStructDefinitions
		structDefinitionOffsetsIndex = binFile.getIndex()
		for i in range(numStructDefinitions):
			structDefinitionOffset = 0xFFFFFFFF
			binFile.writeUint32(structDefinitionOffset)

		structDefinitionOffsets = []
		#		LES_StructDefinition m_structDefinitions[m_numStructDefinitions];			- variable 
		for structDefinition in self.__m_structDefinitions__:
			# LES_StructDefinition
			# {
			#		LES_int32 m_nameID;
			#		LES_int32 m_numMembers;
			#		LES_StructMember m_members[m_numMembers];
			#	};
			currentPosition = binFile.getIndex()
			offset = currentPosition - basePosition
			structDefinitionOffsets.append(offset)

			#		LES_int32 m_nameID;
			nameID = structDefinition.GetNameID()
			binFile.writeInt32(nameID)

			#		LES_int32 m_numMembers;
			numMembers = structDefinition.GetNumMembers()
			binFile.writeInt32(numMembers)

			#		LES_StructMember m_members[m_numMembers];
			for m in range(numMembers):
				#	LES_StructMember
				#	{
				#		LES_uint32 m_hash;
				#		LES_int32 m_nameID;
				#		LES_int32 m_typeID;
				#		LES_int32 m_dataSize;
				#		LES_int32 m_alignmentPadding;
				#	};
				structMember = structDefinition.GetMemberByIndex(m)

				#		LES_uint32 m_hash;
				hashValue = structMember.m_hash
				binFile.writeUint32(hashValue)

				#		LES_int32 m_nameID;
				nameID = structMember.m_nameID
				binFile.writeInt32(nameID)

				#		LES_int32 m_typeID;
				typeID = structMember.m_typeID
				binFile.writeInt32(typeID)

				#		LES_int32 m_dataSize;
				dataSize = structMember.m_dataSize
				binFile.writeInt32(dataSize)

				#		LES_int32 m_alignmentPadding;
				alignmentPadding = structMember.m_alignmentPadding
				binFile.writeInt32(alignmentPadding)

		# Go back and fix up the m_structDefinitionOffsets values
		endIndex = binFile.getIndex()
		binFile.seek(structDefinitionOffsetsIndex)
		for i in range(numStructDefinitions):
			structDefinitionOffset = structDefinitionOffsets[i]
			binFile.writeUint32(structDefinitionOffset)

		binFile.seek(endIndex)

	def parseXML(self, xmlSource):
#		#<?xml version='1.0' ?>
#		#<LES_STRUCTS>
#		#	<LES_STRUCT name="TestStruct1" numMembers="5">
#		#		<LES_STRUCT_MEMBER type="long long int" name="m_longlong"/>
#		#		<LES_STRUCT_MEMBER type="char" name="m_char"/>
#		#		<LES_STRUCT_MEMBER type="int" name="m_int"/>
#		#		<LES_STRUCT_MEMBER type="short" name="m_short"/>
#		#		<LES_STRUCT_MEMBER type="float" name="m_float"/>
#		#	</LES_STRUCT>
#		#</LES_STRUCTS>

		structsXML = xml.etree.ElementTree.XML(xmlSource)
		if structsXML.tag != "LES_STRUCTS":
			les_logger.Error("LES_StructData::parseXML root tag should be LES_STRUCTS found %d", structsXML.tag)
			return 1

		numErrors = 0
		for structXML in structsXML:
			if structXML.tag != "LES_STRUCT":
				les_logger.Error("LES_StructData::parseXML invalid node tag should be LES_STRUCT found:'%s'", structXML.tag)
				numErrors += 1
				continue

			structNameData = structXML.get("name")
			if structNameData == None:
				les_logger.Error("LES_StructData::parseXML missing 'name' attribute:%s", xml.etree.ElementTree.tostring(structXML))
				numErrors += 1
				continue
			structName = structNameData

			numMembersData = structXML.get("numMembers")
			if numMembersData == None:
					les_logger.Error("LES_StructData::parseXML '%s' missing 'numMembers' attribute:%s", structName, xml.etree.ElementTree.tostring(structXML))
					numErrors += 1
					continue

			try:
				numMembers = int(numMembersData)
			except ValueError:
				les_logger.Error("LES_StructData::parseXML '%s' invalid numElements value:%s (must be >= 0)", structName, numMembersData)
				numErrors += 1
				numMembers = -1
				continue

			les_logger.Log("Struct '%s' numMembers:%d", structName, numMembers)

			if self.doesStructDefinitionExist(structName):
				les_logger.Error("LES_StructData::parseXML '%s' already exists", structName)
				numErorrs += 1
				continue

			structNameID = self.__m_stringTable__.addString(structName)
			structDefinition = LES_StructDefinintion(structNameID, numMembers)

			numAdded = 0
			for memberXML in structXML:
				if memberXML.tag != "LES_STRUCT_MEMBER":
					les_logger.Error("LES_StructData::parseXML '%s' invalid node tag should be LES_STRUCT_MEMBER found:'%s'", 
														structName, memberXML.tag)
					numErrors += 1
					continue

				memberTypeData = memberXML.get("type")
				if memberTypeData == None:
					les_logger.Error("LES_StructData::parseXML '%s' missing 'type' attribute:%s", 
													structName, xml.etree.ElementTree.tostring(memberXML))
					numErrors += 1
					continue

				memberType = memberTypeData

				memberNameData = memberXML.get("name")
				if memberNameData == None:
					les_logger.Error("LES_StructData::parseXML '%s' missing 'name' attribute:%s", 
													structName, xml.etree.ElementTree.tostring(memberXML))
					numErrors += 1
					continue
				memberName = memberNameData

#				les_logger.Log("Struct '%s' AddMember[%d] type:'%s' name:'%s'", structName, numAdded, memberType, memberName)

				if structDefinition.AddMember(memberType, memberName, self.__m_stringTable__, self.__m_typeData__, self) == False:
					les_logger.Error("LES_StructData::parseXML '%s' AddMember Type:'%s' Name:'%s' failed attribute:%s", 
														structName, memberType, memberName, xml.etree.ElementTree.tostring(memberXML))
					numErrors += 1
					continue
				numAdded += 1

			if structDefinition.Finalise(structName) == False:
				les_logger.Error("LES_StructData::parseXML '%s' error during Finalise", structName)
				numErrors += 1
				continue

			structSize = structDefinition.GetTotalMemberSizeWithPadding()
			les_logger.Log("Struct '%s' structSize:%d", structName, structSize)

			index = self.addStructDefinition(structName, structDefinition)
			if index == -1:
				les_logger.Error("LES_StructData::parseXML '%s' failed to add type", structName)
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

	def DebugOutputStructs(self, loggerChannel):
		for structDefinition in self.__m_structDefinitions__:
			structNameID = structDefinition.GetNameID()
			structName = self.__m_stringTable__.getString(structNameID)

			numMembers = structDefinition.GetNumMembers()
			loggerChannel.Print("Struct '%s' numMembers[%d]", structName, numMembers)
			for i in range(numMembers):
				structMember = structDefinition.GetMemberByIndex(i)

				hashValue = structMember.m_hash
				nameID = structMember.m_nameID
				typeID = structMember.m_typeID
				dataSize = structMember.m_dataSize
				alignmentPadding = structMember.m_alignmentPadding

				memberName = self.__m_stringTable__.getString(nameID)
				typeName = self.__m_stringTable__.getString(typeID)
				loggerChannel.Print("  Struct '%s' Member[%d] '%s' 0x%X Type:'%s' size:%d alignmentPadding:%d", 
														structName, i, memberName, hashValue, typeName, dataSize, alignmentPadding)

# Global helper functions to do with LES_TypeEntry data - could move these to les_typedata.py but only used in this module
def GetRootType(typeEntry, stringTable, typeData):
	flags = typeEntry.m_flags
	while flags & les_typedata.LES_TYPE_ALIAS:
		aliasedTypeID = typeEntry.m_aliasedTypeID
#		les_logger.Log("Type 0x%X alias:%d flags:0x%X", typeEntry.m_hash, aliasedTypeID, flags)
		aliasedStringType = stringTable.getString(aliasedTypeID)
		if aliasedStringType == None:
			les_logger.FatalError("GetRootType aliased type:%d string can't be found", aliasedTypeID)
			return -1
		aliasedTypeEntry = typeData.getTypeData(aliasedStringType)
		if aliasedTypeEntry == None:
			les_logger.FatalError("GetRootType aliased type not found aliased type:'%s'", aliasedStringType)
			return -1
#		les_logger.Log("GetRootType Type 0x%X alias:%s", typeEntry.m_hash, aliasedStringType)

		typeEntry = aliasedTypeEntry
		flags = typeEntry.m_flags
#		les_logger.Log("Alias Type 0x%X flags:0x%X", typeEntry.m_hash, flags)
	return typeEntry

def ComputeAlignment(typeEntry, stringTable, typeData, structData):
	flags = typeEntry.m_flags
	if flags & les_typedata.LES_TYPE_ARRAY:
		typeEntry = GetRootType(typeEntry, stringTable, typeData)
		flags = typeEntry.m_flags

	if flags & les_typedata.LES_TYPE_STRUCT:
		typeEntry = GetRootType(typeEntry, stringTable, typeData)
		flags = typeEntry.m_flags

		structDefinition = structData.getStructDefinitionByHash(typeEntry.m_hash)
		if structDefinition == None:
			les_logger.Error("ComputeAlignment type:0x%X is a struct but can't be found", typeEntry.m_hash)
			return -1

		numMembers = structDefinition.GetNumMembers()
		maxAlignment = 0
		for i in range(numMembers):
			structMember = structDefinition.GetMemberByIndex(i)
			memberTypeID = structMember.m_typeID
			memberTypeString = stringTable.getString(memberTypeID)
			memberTypeEntry = typeData.getTypeData(memberTypeString)
			alignment = ComputeAlignment(memberTypeEntry, stringTable, typeData, structData)
			if alignment > maxAlignment:
				maxAlignment = alignment
#			les_logger.Log("Compute Alignment Type 0x%X Struct:%d alignment:%d", typeEntry.m_hash, structDefinition.GetNameID(), maxAlignment)
		return maxAlignment

	dataSize = typeEntry.m_dataSize
	alignment = dataSize
#	les_logger.Log("Compute Alignment Type 0x%X alignment:%d", typeEntry.m_hash, alignment)
	return alignment

def ComputeDataStorageSize(typeEntry, stringTable, typeData, structData):
	this = typeEntry
	flags = typeEntry.m_flags
	numElements = typeEntry.m_numElements
	typeEntry = GetRootType(typeEntry, stringTable, typeData)
	flags = typeEntry.m_flags

	if flags & les_typedata.LES_TYPE_STRUCT:
		structDefinition = structData.getStructDefinitionByHash(typeEntry.m_hash)
		if structDefinition == None:
			les_logger.Error("ComputeDataStorageSize type:0x%X is a struct but can't be found", typeEntry.m_hash)
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
#		les_logger.Log("POD 0x%X Type:0x%X Size:%d NumELements:%d", this.m_hash, typeEntry.m_hash, dataSize, numElements)
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
		les_logger.FatalError("StructComputeAlignmentPadding alignmentPadding < 0 :%d", alignmentPadding)

	return alignmentPadding

def runTest():
	les_logger.Init()

	stringTable = les_stringtable.LES_StringTable()
	typeData = les_typedata.LES_TypeData(stringTable)

	this = LES_StructData(stringTable, typeData)

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
	nameID = stringTable.addString("PyTestStruct1")
	structDefinition = LES_StructDefinintion(nameID, 2)
	structDefinition.AddMember("char", "m_char", stringTable, typeData, this)
	structDefinition.AddMember("float", "m_float", stringTable, typeData, this)
	this.addStructDefinition("PyTestStruct1", structDefinition)

	binFile = les_binaryfile.LES_BinaryFile("structDefinitionLittle.bin")
	binFile.setLittleEndian()
	this.writeFile(binFile)
	binFile.close()

	testStructChan = les_logger.CreateChannel("StructDebug", "", "structDebug_py.txt", les_logger.LES_LOGGERCHANNEL_FLAGS_DEFAULT)
	this.DebugOutputStructs(testStructChan)

	stringTable = les_stringtable.LES_StringTable()
	typeData = les_typedata.LES_TypeData(stringTable)
	this = LES_StructData(stringTable, typeData)

	typeData.addType("char", 1, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "char")
	typeData.addType("short", 2, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "short")
	typeData.addType("int", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "int")
	typeData.addType("float", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "float")
	typeData.addType("long long int", 8, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "long long int")

	this.loadXML("data/les_structs_test.xml")

	testStructChan = les_logger.CreateChannel("StructDebug", "", "structDebug_py.txt", les_logger.LES_LOGGERCHANNEL_FLAGS_DEFAULT)
	this.DebugOutputStructs(testStructChan)

if __name__ == '__main__':
	runTest()
