#!/usr/bin/python

import os.path
import xml.etree.ElementTree
import struct

import les_hash
import les_binaryfile
import les_stringtable
import les_typedata
import les_structdata
import les_logger

LES_PARAM_MODE_INPUT 	= les_typedata.LES_TYPE_INPUT
LES_PARAM_MODE_OUTPUT = les_typedata.LES_TYPE_OUTPUT

LES_RETURN_ERROR = -1
LES_RETURN_OK = 0

# LES_FunctionParameter
# {
#		LES_uint32 m_hash;
#		LES_int32 m_nameID;
#		LES_int32 m_typeID;
#		LES_int32 m_index;
#		LES_uint32 m_mode;
# };

# LES_FunctionDefinition
# {
#		LES_int32 m_nameID;
#		LES_int32 m_returnTypeID;
#		LES_int32 m_parameterDataSize;

#		LES_int32 m_numInputs;
#		LES_int32 m_numOutputs;
#		LES_FunctionParameter m_parameters[m_numInputs+m_numOutputs];
#	};

# LES_FunctionData
# {
# 	LES_int32 m_numFunctionDeifinitions; 																		- 4-bytes
# 	LES_int32 m_settled; 																										- 4-bytes, 0 in file
#		LES_uint32 m_functionDefinitionOffsets[m_numFunctionDefinitions];				- 4-bytes * m_numFunctionDefinitions
#		LES_FunctionDefinition m_functionDefinitions[m_numFunctionDefinitions];	- variable 
# };

class LES_FunctionParameterData():
	def __init__(self, memoryBuffer):
		self.__m_memoryBuffer__ = memoryBuffer
		self.__m_readIndex__ = 0

	def Read(self, stringTable, typeData, typeName):
		value = None
		typeEntry = typeData.getTypeData(typeName)
		if typeEntry == None:
			les_logger.Warning("LES_FunctionParameterData::Read type:'%s' not found", typeName)
			return (LES_RETURN_ERROR, value)

		aliasedTypeName = stringTable.getString(typeEntry.m_aliasedTypeID)
		if aliasedTypeName == None:
			les_logger.Warning("LES_FunctionParameterData::Read type:'%s' aliasedStringEntry:%d not found", typeName, typeEntry.m_aliasedTypeID)
			return (LES_RETURN_ERROR, value)

		aliasedTypeEntry = typeData.getTypeData(aliasedTypeName)
		if aliasedTypeEntry == None:
			les_logger.Warning("LES_FunctionParameterData::Read type:'%s' aliased type:'%s' not found", typeName, aliasedTypeName)
			return (LES_RETURN_ERROR, value)

		flags = typeEntry.m_flags
		parameterDataSize = typeEntry.m_dataSize
		if flags & les_typedata.LES_TYPE_POINTER:
			parameterDataSize = aliasedTypeEntry.m_dataSize

		if ((flags & les_typedata.LES_TYPE_POD) or (flags & les_typedata.LES_TYPE_STRUCT)):
#			les_logger.Log("Read type:'%s' size:%d %d", typeName, typeEntry.m_dataSize, self.__m_readIndex__)
			if parameterDataSize == 2:
				bigData = self.__m_memoryBuffer__[self.__m_readIndex__:self.__m_readIndex__ + 2]
				unpackData = struct.unpack(">H", bigData)[0]
				value = struct.pack("=H", unpackData)
			elif parameterDataSize == 4:
				bigData = self.__m_memoryBuffer__[self.__m_readIndex__:self.__m_readIndex__ + 4]
				unpackData = struct.unpack(">I", bigData)[0]
				value = struct.pack("=I", unpackData)
			else:
				value = self.__m_memoryBuffer__[self.__m_readIndex__:self.__m_readIndex__ + parameterDataSize]

			self.__m_readIndex__ += parameterDataSize

		return (LES_RETURN_OK, value)

def __DecodeSingle__(logChannel, paramList, stringTable, typeData, structData, 
										 functionParameterData, parameterIndex, nameID, typeID, parentParameterIndex, depth):
	nameStr = stringTable.getString(nameID)
	typeStr = stringTable.getString(typeID)
	typeEntry = typeData.getTypeData(typeStr)
	newParamList = paramList

#	les_logger.Log("DecodeSingle: Debug name:'%s' type:'%s' typeID:%d", nameStr, typeStr, typeID)

	if typeEntry == None:
		les_logger.Warning("DecodeSingle parameter[%d]:'%s' type:'%s' type can't be found", parameterIndex, nameStr, typeStr)
		return (LES_RETURN_ERROR, newParamList)

	numElements = typeEntry.m_numElements
	aliasedTypeID = typeEntry.m_aliasedTypeID

	typeFlags = typeEntry.m_flags

	typeEntry = typeEntry.GetRootType(stringTable, typeData)
	aliasedTypeID = typeEntry.m_aliasedTypeID
	typeFlags = typeEntry.m_flags
	typeString = stringTable.getString(aliasedTypeID)

	typeDataSize = typeEntry.m_dataSize
	if typeFlags & les_typedata.LES_TYPE_STRUCT:
		localNumElements = numElements
		if localNumElements < 1:
			localNumElements = 1

#		les_logger.Log("DecodeSingle: Debug parameter[%d]:'%s' type:'%s' size:%d ARRAY numELements:%d", 
#								 		parameterIndex, nameStr, typeStr, typeDataSize, numElements)
#		les_logger.Log("DecodeSingle: Debug parameter[%d]:'%s' type:'%s' size:%d STRUCT", parameterIndex, nameStr, typeStr, typeDataSize)

		returnCode = LES_RETURN_OK
		structDefinition = structData.getStructDefinitionByHash(typeEntry.m_hash)
		if structDefinition == None:
			les_logger.Warning("DecodeSingle parameter[%d]:'%s' type:'%s' is a struct but can't be found", parameterIndex, nameStr, typeStr)
			return (LES_RETURN_ERROR, newParamList)

		numMembers = structDefinition.GetNumMembers()
		newDepth = depth + 1
		returnParamList = paramList
		for e in range(localNumElements):
			newParamList = []
			for i in range(numMembers):
				structMember = structDefinition.GetMemberByIndex(i)
				memberNameID = structMember.m_nameID
				memberTypeID = structMember.m_typeID
				paramList = newParamList
				(returnCode, newParamList) = __DecodeSingle__(logChannel, paramList, stringTable, typeData, structData, 
																											functionParameterData, i, memberNameID, memberTypeID, parameterIndex, newDepth)
				if returnCode != LES_RETURN_OK:
					break

 			returnParamList.append(newParamList)
			if returnCode != LES_RETURN_OK:
				break

		return (returnCode, returnParamList)

	if numElements > 0:
		returnCode = LES_RETURN_OK
#		les_logger.Log("DecodeSingle: Debug parameter[%d]:'%s' type:'%s' size:%d ARRAY numELements:%d", 
#				 					 parameterIndex, nameStr, typeStr, typeDataSize, numElements)

		elementNameID = nameID
		elementTypeID = aliasedTypeID
		newDepth = depth + 1
		returnParamList = paramList
		newParamList = []
		for i in range(numElements):
			paramList = newParamList
			(returnCode, newParamList) = __DecodeSingle__(logChannel, paramList, stringTable, typeData, structData, 
																										functionParameterData, i, elementNameID, elementTypeID, parameterIndex, newDepth)
			if returnCode != LES_RETURN_OK:
				break

	 	returnParamList.append(newParamList)
		return (returnCode, returnParamList)

	typeHash = typeEntry.m_hash

	value = None
	unpackFormat = None
	output = ""
	fmtStr = "%d"

	if typeHash == les_typedata.s_longlongHash:
		unpackFormat = "=q"
	elif typeHash == les_typedata.s_intHash:
		unpackFormat = "=i"
	elif typeHash == les_typedata.s_uintHash:
		unpackFormat = "=I"
	elif typeHash == les_typedata.s_shortHash:
		unpackFormat = "=h"
	elif typeHash == les_typedata.s_ushortHash:
		unpackFormat = "=H"
	elif typeHash == les_typedata.s_charHash:
		unpackFormat = "=c"
		fmtStr = "'%s'"
	elif typeHash == les_typedata.s_ucharHash:
		unpackFormat = "=c"
		fmtStr = "'%s'"
	elif (typeHash == les_typedata.s_floatHash):
		unpackFormat = "=f"
		fmtStr = "%.3f"
	else:
	 	unpackFormat = None


	(errorCode, binaryValue) = functionParameterData.Read(stringTable, typeData, typeString)
	if errorCode != LES_RETURN_OK:
		les_logger.Warning("DecodeSingle Read failed for parameter[%d]:%s", parameterIndex, nameStr)
		return (LES_RETURN_ERROR, newParamList)

	output += "DecodeSingle "
	for i in range(depth*2):
		output += " "
	output += "parameter"
	if parentParameterIndex >= 0:
		output += ("[%d] member" % (parentParameterIndex))

	output += ("[%d]:'%s' type:'%s' value:" % (parameterIndex, nameStr, typeStr))
	if unpackFormat != None:
		value = struct.unpack(unpackFormat, binaryValue)[0]
		output += (fmtStr % (value))

	if unpackFormat == None:
		output += (" typeDataSize:%d struct:%d" % (typeDataSize, (typeEntry.m_flags & les_typedata.LES_TYPE_STRUCT)))

	les_logger.Log(output)
	if logChannel != None:
		logChannel.Print(output)

	newParamList.append([nameStr, value])
	return (LES_RETURN_OK, newParamList)

class LES_FunctionParameter():
	def __init__(self, hashValue, nameID, typeID, index, mode):
		self.m_hash = hashValue
		self.m_nameID = nameID
		self.m_typeID = typeID
		self.m_index = index
		self.m_mode = mode

class LES_FunctionDefinintion():
	def __init__(self, nameID, returnTypeID, numInputs, numOutputs):
		self.__m_nameID__ = nameID
		self.__m_returnTypeID__ = returnTypeID
		self.__m_parameterDataSize__ = 0
		self.__m_numInputs__ = numInputs
		self.__m_numOutputs__ = numOutputs
		self.__m_parameters__ = []
		emptyFunctionParameter = LES_FunctionParameter(0, -1, -1, -1, 0)
		numParameters = numInputs + numOutputs
		for i in range(numParameters):
			self.__m_parameters__.append(emptyFunctionParameter)
		self.__m_numAddedInputs__ = 0
		self.__m_numAddedOutputs__ = 0
		self.__m_numAddedParameters__ = 0
		self.__m_finalised__ = False

	def IsFinalised(self):
		return self.__m_finalised__ 

	def GetNameID(self):
			return self.__m_nameID__

	def GetReturnTypeID(self):
			return self.__m_returnTypeID__

	def GetParameterDataSize(self):
			return self.__m_parameterDataSize__

	def GetNumInputs(self):
			return self.__m_numInputs__

	def GetNumOutputs(self):
			return self.__m_numOutputs__

	def GetNumParameters(self):
			return (self.__m_numInputs__ + self.__m_numOutputs__)

	def GetParameterByIndex(self, index):
			return self.__m_parameters__[index]

	def GetParameter(self, hashValue):
		for parameter in self.__m_parameters__:
			if parameter.m_hash == hashValue:
				return True
		return False

	def AddParameter(self, isInput, typeName, name, stringTable, typeData, structData):
		functionName = stringTable.getString(self.__m_nameID__)

		# Error if parameter index off the end of the array
		if isInput:
			paramMode = LES_PARAM_MODE_INPUT
			numAddedInputs = self.__m_numAddedInputs__
			maxNumInputs = self.GetNumInputs()
			if numAddedInputs >= maxNumInputs:
				les_logger.Error("AddParameter function '%s' : InputParameterIndex too big index:%d max:%d parameter:'%s' type:'%s'",
												 functionName, self.__m_numAddedInputs__,  maxNumInputs, name, typeName)
				return False
		else:
		 	paramMode = LES_PARAM_MODE_OUTPUT
			numAddedOutputs = self.__m_numAddedOutputs__
			maxNumOutputs = self.GetNumOutputs()
			if numAddedOutputs >= maxNumOutputs:
				les_logger.Error("AddParameter function '%s' : OutputParameterIndex too big index:%d max:%d parameter:'%s' type:'%s'",
												 functionName, self.__m_numAddedOutputs__,  maxNumOutputs, name, typeName)
				return False
		
		nameHash = les_hash.LES_GenerateHashCaseSensitive(name)
		# Test to see if the parameter has already been added
		if self.GetParameter(nameHash):
			les_logger.Error("AddParameter function '%s' : parameter '%s' already exists type:'%s'", functionName, name, typeName)
			return False

		typeID = stringTable.getStringID(typeName)
		typeEntry = typeData.getTypeData(typeName)
		if typeEntry == None:
			les_logger.Error("AddParameter function '%s' : parameter '%s' type '%s' not found", functionName, name, typeName)
			return False
		else:
			typeDataStorageSize = les_structdata.ComputeDataStorageSize(typeEntry, stringTable, typeData, structData)
			if typeDataStorageSize < 0:
				les_logger.Error("AddParameter function '%s' : parameter '%s' type:'%s' invalid typeDataStorageSize",
												 functionName, name, typeName)
				return False

		typeFlags = typeEntry.m_flags
		if (typeFlags & paramMode == 0):
			les_logger.Error("AddParameter function '%s' : parameter '%s' type:'%s' mode:0x%X %s incorrect for typeFlags:0x%X %s",
											 functionName, name, typeName, paramMode, les_typedata.decodeFlags(paramMode), 
														typeFlags, les_typedata.decodeFlags(typeFlags))
			return False

		nameID = stringTable.addString(name)

		numAddedParameters = self.__m_numAddedParameters__
		functionParameter = LES_FunctionParameter(nameHash, nameID, typeID, numAddedParameters, paramMode)

		self.__m_parameters__[numAddedParameters] = functionParameter
		self.__m_numAddedParameters__ += 1

		if isInput:
			self.__m_numAddedInputs__ += 1
		else:
			self.__m_numAddedOutputs__ += 1

		les_logger.Log("  Function:'%s' Parameter:'%s' Type:'%s' 0x%X Input:%d mode:0x%X %s index:%d", 
										functionName, name, typeName, les_hash.LES_GenerateHashCaseSensitive(typeName), 
										isInput, paramMode, les_typedata.decodeFlags(paramMode), numAddedParameters)

		return True

	def Finalise(self, functionName, stringTable, typeData, structData):
		self.__m_finalised__ = False

		if self.__m_numAddedInputs__ != self.GetNumInputs():
				les_logger.Error("LES_FunctionDefintion::Finalise '%s' not enough input parameters added numInputsAdded:%d numInputs:%d", 
												 functionName, self.__m_numAddedInputs__, self.GetNumInputs())
				return False

		if self.__m_numAddedOutputs__ != self.GetNumOutputs():
				les_logger.Error("LES_FunctionDefintion::Finalise '%s' not enough output parameters added numOutputsAdded:%d numOutputs:%d", 
												 functionName, self.__m_numAddedOutputs__, self.GetNumOutputs())
				return False

		parameterDataSize = self.ComputeParameterDataSize(stringTable, typeData, structData)
		self.__m_parameterDataSize__ = parameterDataSize

		self.__m_finalised__ = True
		return True

	def ComputeParameterDataSize(self, stringTable, typeData, structData):
		parameterDataSize = 0
		paramDataSize = 0
		numParams = self.GetNumParameters()
		for i in range(numParams):
			functionParameter = self.GetParameterByIndex(i)
			typeID = functionParameter.m_typeID
			typeString = stringTable.getString(typeID)
			if typeString == None:
				return -1

			typeEntry = typeData.getTypeData(typeString)
			if typeEntry == None:
				return -1

			# This function is recursive following aliases and also looping over the members in structs and handling arrays
			paramDataSize += les_structdata.ComputeDataStorageSize(typeEntry, stringTable, typeData, structData)

		parameterDataSize = paramDataSize
		return parameterDataSize

	def DebugOutput(self, loggerChannel, stringTable):
		functionNameID = self.GetNameID()
		functionName = stringTable.getString(functionNameID)

		numInputs = self.GetNumInputs()
		numOutputs = self.GetNumOutputs()
		numParameters = self.GetNumParameters()
		returnTypeID = self.GetReturnTypeID()
		returnTypeName = stringTable.getString(returnTypeID)
		parameterDataSize = self.GetParameterDataSize()
		loggerChannel.Print("Function '%s' returnType '%s' numParameters:%d numInputs:%d numOutputs:%d parameterDataSize:%d", 
												functionName, returnTypeName, numParameters, numInputs, numOutputs, parameterDataSize)
		for i in range(numParameters):
			functionParameter = self.GetParameterByIndex(i)

			hashValue = functionParameter.m_hash
			nameID = functionParameter.m_nameID
			typeID = functionParameter.m_typeID
			index = functionParameter.m_index
			mode = functionParameter.m_mode

			memberName = stringTable.getString(nameID)
			typeName = stringTable.getString(typeID)
			paramMode = les_typedata.decodeFlags(mode)
			loggerChannel.Print("  Function '%s' Member[%d] '%s' 0x%X Type:'%s' index:%d mode:0x%X %s", 
													functionName, i, memberName, hashValue, typeName, index, mode, paramMode)

	def Decode(self, logChannel, stringTable, typeData, structData, functionParameterData):
		functionNameID = self.GetNameID()
		functionName = stringTable.getString(functionNameID)
		numParameters = self.GetNumParameters()
		les_logger.Log("Decode Function '%s' numParams:%d" % (functionName, numParameters))
		if logChannel != None:
			logChannel.Print("Decode Function '%s' numParams:%d" % (functionName, numParameters))

		newParamList = []
		for i in range(numParameters):
			functionParameter = self.GetParameterByIndex(i)
			nameID = functionParameter.m_nameID
			typeID = functionParameter.m_typeID
			paramList = newParamList
			(returnCode, newParamList) = __DecodeSingle__(logChannel, paramList, stringTable, typeData, structData, 
																										functionParameterData, i, nameID, typeID, -1, 0)
			if returnCode != LES_RETURN_OK:
				return (LES_RETURN_ERROR, newParamList)

		les_logger.Log("newParamList %s", newParamList)
		return (LES_RETURN_OK, newParamList)

class LES_FunctionData():
	def __init__(self, stringTable, typeData, structData):
		self.__m_functionDefinitions__ = []
		self.__m_functionDefinitionNames__ = []
		self.__m_stringTable__ = stringTable
		self.__m_typeData__ = typeData
		self.__m_structData__ = structData

	def addFunctionDefinition(self, name, functionDefinition):
		if self.doesFunctionDefinitionExist(name):
			les_logger.Error("LES_FunctionData::addFunctionDefinition '%s' but function definition already exists", name)
			return -1

		# Validate name should match nameID
		nameID = functionDefinition.GetNameID()
		nameIDstring = self.__m_stringTable__.getString(nameID)
		if nameIDstring != name:
			les_logger.Error("LES_FunctionData::addFunctionDefinition '%s' nameID:%d '%s' doesn't match name:'%s'", name, nameID, nameIDstring, name)
			return -1

		numInputs = functionDefinition.GetNumInputs()
		# Validate numInputs >= 0
		if numInputs < 0:
			les_logger.Error("LES_FunctionData::addFunctionDefinition '%s' invalid numInputs:%d must be >= 0", name, numInputs)
			return -1

		numOutputs = functionDefinition.GetNumOutputs()
		# Validate numOutputs >= 0
		if numOutputs < 0:
			les_logger.Error("LES_FunctionData::addFunctionDefinition '%s' invalid numOutputs:%d must be >= 0", name, numOutputs)
			return -1

		if functionDefinition.IsFinalised() == False:
			les_logger.Error("LES_FunctionData::addFunctionDefintiion '%s' function isn't finalised", name)
			return -1

		index = len(self.__m_functionDefinitions__)
		self.__m_functionDefinitions__.append(functionDefinition)
		self.__m_functionDefinitionNames__.append(name)

		return index

	def doesFunctionDefinitionExist(self, name):
		return name in self.__m_functionDefinitionNames__

	def getFunctionDefinitionByHash(self, nameHash):
		name = self.__m_stringTable__.getStringByHash(nameHash)
		return self.getFunctionDefinition(name)

	def getFunctionDefinition(self, name):
		try:
			index = self.__m_functionDefinitionNames__.index(name)
		except ValueError:
			return None
		return self.__m_functionDefinitions__[index]

	def write(self, binFile):
		basePosition = binFile.getIndex()
		# LES_FunctionData
		# {
		# 	LES_int32 m_numFunctionDefinitions; 																		- 4-bytes
		# 	LES_int32 m_settled; 																										- 4-bytes, 0 in file
		#		LES_uint32 m_functionDefinitionOffsets[m_numFunctionDefinitions];				- 4-bytes * m_numFunctionDefinitions
		#		LES_FunctionDefinition m_functionDefinitions[m_numFunctionDefinitions];	- variable 
		# };

		# 	LES_int32 m_numFunctionDefinitions; 										- 4 bytes
		numFunctionDefinitions = len(self.__m_functionDefinitions__)
		binFile.writeInt32(numFunctionDefinitions)

		# 	LES_int32 m_settled; 																	- 4-bytes, 0 in file
		settled = 0
		binFile.writeInt32(settled)

		#		LES_uint32 m_functionDefinitionOffsets[m_numFunctionDefinitions];	- 4-bytes * m_numFunctionDefinitions
		functionDefinitionOffsetsIndex = binFile.getIndex()
		for i in range(numFunctionDefinitions):
			functionDefinitionOffset = 0xFFFFFFFF
			binFile.writeUint32(functionDefinitionOffset)

		functionDefinitionOffsets = []
		#		LES_FunctionDefinition m_functionDefinitions[m_numFunctionDefinitions];			- variable 
		for functionDefinition in self.__m_functionDefinitions__:
			# LES_FunctionDefinition
			# {
			#		LES_int32 m_nameID;
			#		LES_int32 m_returnTypeID;
			#		LES_int32 m_parameterDataSize;
			#		LES_int32 m_numInputs;
			#		LES_int32 m_numOutputs;
			#		LES_FunctionParameter m_parameters[m_numInputs+m_numOutputs];
			#	};
			currentPosition = binFile.getIndex()
			offset = currentPosition - basePosition
			functionDefinitionOffsets.append(offset)

			#		LES_int32 m_nameID;
			nameID = functionDefinition.GetNameID()
			binFile.writeInt32(nameID)

			#		LES_int32 m_returnTypeID;
			returnTypeID = functionDefinition.GetReturnTypeID()
			binFile.writeInt32(returnTypeID)

			#		LES_int32 m_parameterDataSize;
			parameterDataSize = functionDefinition.GetParameterDataSize()
			binFile.writeInt32(parameterDataSize)

			#		LES_int32 m_nnumInputs;
			numInputs = functionDefinition.GetNumInputs()
			binFile.writeInt32(numInputs)

			#		LES_int32 m_numOutputs;
			numOutputs = functionDefinition.GetNumOutputs()
			binFile.writeInt32(numOutputs)

			#		LES_FunctionParameter m_parameters[m_numInputs+m_numOutputs];
			numParameters = numInputs + numOutputs
			for p in range(numParameters):
				# LES_FunctionParameter
				# {
				#		LES_uint32 m_hash;
				#		LES_int32 m_nameID;
				#		LES_int32 m_typeID;
				#		LES_int32 m_index;
				#		LES_uint32 m_mode;
				# };
				functionParameter = functionDefinition.GetParameterByIndex(p)

				#		LES_uint32 m_hash;
				hashValue = functionParameter.m_hash
				binFile.writeUint32(hashValue)

				#		LES_int32 m_nameID;
				nameID = functionParameter.m_nameID
				binFile.writeInt32(nameID)

				#		LES_int32 m_typeID;
				typeID = functionParameter.m_typeID
				binFile.writeInt32(typeID)

				#		LES_int32 m_index;
				index = functionParameter.m_index
				binFile.writeInt32(index)

				#		LES_uint32 m_mode;
				mode = functionParameter.m_mode
				binFile.writeUint32(mode)

		# Go back and fix up the m_functionDefinitionOffsets values
		endIndex = binFile.getIndex()
		binFile.seek(functionDefinitionOffsetsIndex)
		for i in range(numFunctionDefinitions):
			functionDefinitionOffset = functionDefinitionOffsets[i]
			binFile.writeUint32(functionDefinitionOffset)

		binFile.seek(endIndex)

	def parseXML(self, xmlSource):
#		#<?xml version='1.0' ?>
#		#<LES_FUNCTIONS>
#		#	<LES_FUNCTION name="TestFunction1" returnType="int" numInputs="5" numOutputs="2">
#		#		<LES_FUNCTION_INPUT_PARAMETER type="long long int" name="m_longlong"/>
#		#		<LES_FUNCTION_INPUT_PARAMETER type="char" name="m_char"/>
#		#		<LES_FUNCTION_OUTPUT_PARAMETER type="int*" name="m_int"/>
#		#		<LES_FUNCTION_INPUT_PARAMETER type="short" name="m_short"/>
#		#		<LES_FUNCTION_OUTPUT_PARAMETER type="float*" name="m_float"/>
#		#	</LES_FUNCTION>
#		#</LES_FUNCTIONS>

		functionsXML = xml.etree.ElementTree.XML(xmlSource)
		if functionsXML.tag != "LES_FUNCTIONS":
			les_logger.Error("LES_FunctionData::parseXML root tag should be LES_FUNCTIONS found %d", functionsXML.tag)
			return 1

		numErrors = 0
		for functionXML in functionsXML:
			if functionXML.tag != "LES_FUNCTION":
				les_logger.Error("LES_FunctionData::parseXML invalid node tag should be LES_FUNCTION found:'%s'", functionXML.tag)
				numErrors += 1
				continue

			functionNameData = functionXML.get("name")
			if functionNameData == None:
				les_logger.Error("LES_FunctionData::parseXML missing 'name' attribute:%s", xml.etree.ElementTree.tostring(functionXML))
				numErrors += 1
				continue
			functionName = functionNameData

			returnTypeData = functionXML.get("returnType")
			if returnTypeData == None:
				les_logger.Error("LES_FunctionData::parseXML missing 'returnType' attribute:%s", xml.etree.ElementTree.tostring(functionXML))
				numErrors += 1
				continue
			returnTypeName = returnTypeData

			numInputsData = functionXML.get("numInputs")
			if numInputsData == None:
					les_logger.Error("LES_FunctionData::parseXML '%s' missing 'numInputs' attribute:%s", 
													 functionName, xml.etree.ElementTree.tostring(functionXML))
					numErrors += 1
					continue

			try:
				numInputs = int(numInputsData)
			except ValueError:
				les_logger.Error("LES_FunctionData::parseXML '%s' invalid numInputs value:%s (must be >= 0)", functionName, numInputsData)
				numErrors += 1
				numInputs = -1
				continue

			numOutputsData = functionXML.get("numOutputs")
			if numOutputsData == None:
					les_logger.Error("LES_FunctionData::parseXML '%s' missing 'numOutputs' attribute:%s", 
													 functionName, xml.etree.ElementTree.tostring(functionXML))
					numErrors += 1
					continue

			try:
				numOutputs = int(numOutputsData)
			except ValueError:
				les_logger.Error("LES_FunctionData::parseXML '%s' invalid numOutputs value:%s (must be >= 0)", functionName, numOutputsData)
				numErrors += 1
				numInputs = -1
				continue

			les_logger.Log("Function '%s' returnType:%s numInputs:%d numOutputs:%d", functionName, returnTypeName, numInputs, numOutputs)

			if self.doesFunctionDefinitionExist(functionName):
				les_logger.Error("LES_FunctionData::parseXML '%s' already exists", functionName)
				numErrors += 1
				continue

			functionNameID = self.__m_stringTable__.addString(functionName)
			returnTypeID = self.__m_stringTable__.addString(returnTypeName)
			functionDefinition = LES_FunctionDefinintion(functionNameID, returnTypeID, numInputs, numOutputs)

			numAdded = 0
			for memberXML in functionXML:
				if memberXML.tag != "LES_FUNCTION_INPUT_PARAMETER" and memberXML.tag != "LES_FUNCTION_OUTPUT_PARAMETER":
					les_logger.Error("LES_FunctionData::parseXML '%s' invalid node tag should be LES_FUNCTION_INPUT_PARAMETER or LES_FUNCTION_OUTPUT_PARAMETER found:'%s'", 
														functionName, memberXML.tag)
					numErrors += 1
					continue

				if memberXML.tag == "LES_FUNCTION_INPUT_PARAMETER":
					isInput = True
				if memberXML.tag == "LES_FUNCTION_OUTPUT_PARAMETER":
					isInput = False

				memberTypeData = memberXML.get("type")
				if memberTypeData == None:
					les_logger.Error("LES_FunctionData::parseXML '%s' missing 'type' attribute:%s", 
													functionName, xml.etree.ElementTree.tostring(memberXML))
					numErrors += 1
					continue

				memberType = memberTypeData

				memberNameData = memberXML.get("name")
				if memberNameData == None:
					les_logger.Error("LES_FunctionData::parseXML '%s' missing 'name' attribute:%s", 
													functionName, xml.etree.ElementTree.tostring(memberXML))
					numErrors += 1
					continue
				memberName = memberNameData

#				les_logger.Log("Function '%s' AddParameter[%d] type:'%s' name:'%s'", functionName, numAdded, memberType, memberName)

				if functionDefinition.AddParameter(isInput, memberType, memberName, 
																					 self.__m_stringTable__, self.__m_typeData__, self.__m_structData__) == False:
					les_logger.Error("LES_FunctionData::parseXML '%s' AddParameter Type:'%s' Name:'%s' failed attribute:%s", 
														functionName, memberType, memberName, xml.etree.ElementTree.tostring(memberXML))
					numErrors += 1
					continue
				numAdded += 1

			if functionDefinition.Finalise(functionName, self.__m_stringTable__, self.__m_typeData__, self.__m_structData__) == False:
				les_logger.Error("LES_FunctionData::parseXML '%s' error during Finalise", functionName)
				numErrors += 1
				continue

			index = self.addFunctionDefinition(functionName, functionDefinition)
			if index == -1:
				les_logger.Error("LES_FunctionData::parseXML '%s' failed to add type", functionName)
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

	def createTestErrorFunctionDefinitions(self):
		les_logger.Log("")
		les_logger.Log("#####################")
		les_logger.Log("Creating Test Error Function Definitions")
		les_logger.Log("#####################")
		les_logger.Log("")

		functionName = "LES_Test_InputNameIDNotFound"
		functionNameHash = les_hash.LES_GenerateHashCaseSensitive(functionName)
		functionDefinition = self.getFunctionDefinitionByHash(functionNameHash)
		functionParameter = functionDefinition.GetParameterByIndex(0)
		functionParameter.m_nameID = -1

		functionName = "LES_Test_InputNameHashIsWrong"
		functionNameHash = les_hash.LES_GenerateHashCaseSensitive(functionName)
		functionDefinition = self.getFunctionDefinitionByHash(functionNameHash)
		functionParameter = functionDefinition.GetParameterByIndex(0)
		functionParameter.m_nameID = self.__m_stringTable__.addString("wrongHash")

		functionName = "LES_Test_InputTypeIDNotFound"
		functionNameHash = les_hash.LES_GenerateHashCaseSensitive(functionName)
		functionDefinition = self.getFunctionDefinitionByHash(functionNameHash)
		functionParameter = functionDefinition.GetParameterByIndex(0)
		functionParameter.m_typeID = -1
		functionDefinition.__m_parameterDataSize__ = -1

		functionName = "LES_Test_OutputNameIDNotFound"
		functionNameHash = les_hash.LES_GenerateHashCaseSensitive(functionName)
		functionDefinition = self.getFunctionDefinitionByHash(functionNameHash)
		functionParameter = functionDefinition.GetParameterByIndex(0)
		functionParameter.m_nameID = -1

		functionName = "LES_Test_OutputNameHashIsWrong"
		functionNameHash = les_hash.LES_GenerateHashCaseSensitive(functionName)
		functionDefinition = self.getFunctionDefinitionByHash(functionNameHash)
		functionParameter = functionDefinition.GetParameterByIndex(0)
		functionParameter.m_nameID = self.__m_stringTable__.addString("wrongHash")

		functionName = "LES_Test_OutputTypeIDNotFound"
		functionNameHash = les_hash.LES_GenerateHashCaseSensitive(functionName)
		functionDefinition = self.getFunctionDefinitionByHash(functionNameHash)
		functionParameter = functionDefinition.GetParameterByIndex(0)
		functionParameter.m_typeID = -1
		functionDefinition.__m_parameterDataSize__ = -1

		functionName = "LES_Test_ReturnTypeNotFound"
		functionNameHash = les_hash.LES_GenerateHashCaseSensitive(functionName)
		functionDefinition = self.getFunctionDefinitionByHash(functionNameHash)
		functionDefinition.__m_returnTypeID__ = -1

		functionName = "LES_Test_InputParamUsedAsOutput"
		functionNameHash = les_hash.LES_GenerateHashCaseSensitive(functionName)
		functionDefinition = self.getFunctionDefinitionByHash(functionNameHash)
		functionParameter = functionDefinition.GetParameterByIndex(1)
		functionParameter.m_typeID = self.__m_stringTable__.addString("unsigned short")

		functionName = "LES_Test_OutputParamUsedAsInput"
		functionNameHash = les_hash.LES_GenerateHashCaseSensitive(functionName)
		functionDefinition = self.getFunctionDefinitionByHash(functionNameHash)
		functionParameter = functionDefinition.GetParameterByIndex(0)
		functionParameter.m_typeID = self.__m_stringTable__.addString("output_only")

		self.__m_stringTable__.addString("CantAddNewFunctions")
		return True

	def DebugOutputFunctions(self, loggerChannel):
		for functionDefinition in self.__m_functionDefinitions__:
			functionDefinition.DebugOutput(loggerChannel, self.__m_stringTable__)

def runTest():
	les_logger.Init()

	stringTable = les_stringtable.LES_StringTable()
	typeData = les_typedata.LES_TypeData(stringTable)
	structData = les_structdata.LES_StructData(stringTable, typeData)

	this = LES_FunctionData(stringTable, typeData, structData)

	nameID = stringTable.addString("Jake")
	returnTypeID = stringTable.addString("JakeReturn")
	functionDefinition = LES_FunctionDefinintion(nameID, returnTypeID, 1, 1)
	this.addFunctionDefinition("Jake", functionDefinition)

	functionDefinition = LES_FunctionDefinintion(nameID, returnTypeID, 1, 1)
	this.addFunctionDefinition("Jake", functionDefinition)

	functionDefinition = LES_FunctionDefinintion(nameID, returnTypeID, 1, 1)
	this.addFunctionDefinition("Bob", functionDefinition)

	nameID = stringTable.addString("Dick")
	functionDefinition = LES_FunctionDefinintion(nameID, returnTypeID, 1, 1)
	this.addFunctionDefinition("Dick", functionDefinition)

	typeData.addType("char", 1, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "char")
	typeData.addType("short", 2, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "short")
	typeData.addType("int", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "int")
	typeData.addType("float", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "float")
	typeData.addType("float*", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_OUTPUT|les_typedata.LES_TYPE_POD, "float")

	functionName = "PyTestFunction1"
	nameID = stringTable.addString(functionName)
	functionDefinition = LES_FunctionDefinintion(nameID, returnTypeID, 1, 1)
	functionDefinition.AddParameter(True, "char", "m_char", stringTable, typeData, structData)
	functionDefinition.AddParameter(False, "float*", "m_float", stringTable, typeData, structData)
	if functionDefinition.Finalise(functionName, stringTable, typeData, structData) == False:
		les_logger.Error("'%s' error during Finalise", functionName)

	this.addFunctionDefinition(functionName, functionDefinition)

	binFile = les_binaryfile.LES_BinaryFile()
	binFile.setLittleEndian()
	this.write(binFile)
	binFile.saveToFile("functionDefinitionLittle.bin")
	binFile.close()

	les_logger.Log("")
	testFunctionChan = les_logger.CreateChannel("FunctionDebug", "", "functionDebug_py.txt", les_logger.LES_LOGGERCHANNEL_FLAGS_DEFAULT)
	this.DebugOutputFunctions(testFunctionChan)

	stringTable = les_stringtable.LES_StringTable()
	typeData = les_typedata.LES_TypeData(stringTable)
	structData = les_structdata.LES_StructData(stringTable, typeData)
	this = LES_FunctionData(stringTable, typeData, structData)

	typeData.addType("char", 1, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "char")
	typeData.addType("short", 2, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "short")
	typeData.addType("int", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "int")
	typeData.addType("float", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "float")
	typeData.addType("long long int", 8, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_POD, "long long int")
	typeData.addType("int*", 4, les_typedata.LES_TYPE_INPUT|les_typedata.LES_TYPE_OUTPUT|les_typedata.LES_TYPE_POD|les_typedata.LES_TYPE_POINTER, "int")

	this.loadXML("data/les_functions_test.xml")

	les_logger.Log("")
	testFunctionChan = les_logger.CreateChannel("FunctionDebug", "", "functionDebug_py.txt", les_logger.LES_LOGGERCHANNEL_FLAGS_DEFAULT)
	this.DebugOutputFunctions(testFunctionChan)

if __name__ == '__main__':
	runTest()
