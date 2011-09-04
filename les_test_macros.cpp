#include "les_test_macros.h"
#include "les_core.h"
#include "les_logger.h"
#include "les_function.h"
#include "les_parameter.h"
#include "les_stringentry.h"

extern int LES_AddStringEntry(const char* const str);

extern LES_FunctionDefinition* LES_CreateFunctionDefinition(const int nameID, const int returnTypeID, 
																														const int numInputs, const int numOutputs);

extern int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const functionDefinitionPtr, 
																		 const int parameterDataSize);


extern LES_StructDefinition* LES_CreateStructDefinition(const int nameID, const int numMembers);

extern int LES_AddStructDefinition(const char* const name, const LES_StructDefinition* const structDefinitionPtr, 
																	 const LES_uint32 structDataSize);

extern int LES_StructComputeAlignmentPadding(const int totalMemberSize, const int memberDataSize);

bool LES_TestFunctionStart(const char* const functionName, const char* const returnType, const int numInputs, const int numOutputs,
													 LES_FunctionDefinition** const functionDefinitionPtr, LES_TEST_FUNCTION_DATA* testFunctionDataPtr)
{
	testFunctionDataPtr->functionName = functionName;
	testFunctionDataPtr->globalParamIndex = 0;
	testFunctionDataPtr->inputParamIndex = 0;
	testFunctionDataPtr->outputParamIndex = 0;

	const int nameID = LES_AddStringEntry(functionName);
	const int returnTypeID = LES_AddStringEntry(returnType);
	*functionDefinitionPtr = LES_CreateFunctionDefinition(nameID, returnTypeID, numInputs, numOutputs);

	return true;
}

bool LES_TestFunctionAddParam(const bool isInput, const char* const type, const char* const name, 
															LES_FunctionDefinition* const functionDefinitionPtr, LES_TEST_FUNCTION_DATA* testFunctionDataPtr)
{
	// Error if parameter index off the end of the array
	const int maxNumParam = (isInput ? functionDefinitionPtr->GetNumInputs() : functionDefinitionPtr->GetNumOutputs());
	int* const paramIndex = (isInput ? &testFunctionDataPtr->inputParamIndex : &testFunctionDataPtr->outputParamIndex);
	const char* const paramModeStr = (isInput ? "Input" : "Output");
	const int paramMode = (isInput ? LES_PARAM_MODE_INPUT : LES_PARAM_MODE_OUTPUT);
	if (*paramIndex >= maxNumParam)
	{
		LES_FATAL_ERROR("TEST function '%s' : %s ParamIndex too big index:%d max:%d parameter:'%s' type:'%s'",
										testFunctionDataPtr->functionName, paramModeStr, *paramIndex, maxNumParam, name, type);
		return false;
	}
	const LES_Hash nameHash = LES_GenerateHashCaseSensitive(name);
	// Test to see if the parameter has already been added
	if (functionDefinitionPtr->GetParameter(nameHash) != LES_NULL)
	{
		LES_FATAL_ERROR("TEST function '%s' : parameter '%s' already exists type:'%s'",
										testFunctionDataPtr->functionName, name, type);
		return false;
	}
	const int typeID = LES_AddStringEntry(type);
	const LES_StringEntry* const typeStringEntry = LES_GetStringEntryForID(typeID);
	const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntry);
	if (typeEntryPtr != LES_NULL)
	{
		const int typeDataStorageSize = typeEntryPtr->ComputeDataStorageSize();
		if (typeDataStorageSize < 0)
		{
			LES_FATAL_ERROR("TEST function '%s' : parameter '%s' type:'%s' invalid typeDataStorageSize",
											testFunctionDataPtr->functionName, name, type);
			return false;
		}
	}
	const int globalParamIndex = testFunctionDataPtr->globalParamIndex;
	LES_FunctionParameter* const functionParameterPtr = (LES_FunctionParameter* const)(functionDefinitionPtr->GetParameterByIndex(globalParamIndex));
	functionParameterPtr->m_index = testFunctionDataPtr->globalParamIndex;
	functionParameterPtr->m_hash = nameHash;
	functionParameterPtr->m_nameID = LES_AddStringEntry(name);
	functionParameterPtr->m_typeID = typeID;
	functionParameterPtr->m_mode = paramMode;
	testFunctionDataPtr->globalParamIndex++;
	*paramIndex = *paramIndex + 1;

	return true;
}

bool LES_TestFunctionEnd(LES_FunctionDefinition* const functionDefinitionPtr, LES_TEST_FUNCTION_DATA* testFunctionDataPtr)
{
	if (testFunctionDataPtr->globalParamIndex != functionDefinitionPtr->GetNumParameters())
	{
		LES_FATAL_ERROR("TEST function '%s' : ERROR not the right number of parameters Added:%d Should be:%d",
										testFunctionDataPtr->functionName, testFunctionDataPtr->globalParamIndex, functionDefinitionPtr->GetNumParameters());
		return false;
	}
	const int parameterDataSize = functionDefinitionPtr->ComputeParameterDataSize();
	LES_AddFunctionDefinition(testFunctionDataPtr->functionName, functionDefinitionPtr, parameterDataSize);

	return true;
}

bool LES_TestStructStart(const char* const structName, const int numMembers, 
												 LES_StructDefinition** structDefinitionPtr, LES_TEST_STRUCT_DATA* testStructDataPtr)
{
	testStructDataPtr->structName = structName;
	testStructDataPtr->globalMemberIndex = 0;
	testStructDataPtr->totalMemberSizeWithPadding = 0;
	testStructDataPtr->maxMemberSizeAlignment = 0;

	const int nameID = LES_AddStringEntry(structName);
	*structDefinitionPtr = LES_CreateStructDefinition(nameID, numMembers);

	return true;
}

bool LES_TestStructAddMember(const char* const type, const char* const name,
												 		 LES_StructDefinition* const structDefinitionPtr, LES_TEST_STRUCT_DATA* testStructDataPtr)
{
	// Error if member index off the end of the array
	const int maxNumMembers = structDefinitionPtr->GetNumMembers();
	if (testStructDataPtr->globalMemberIndex >= maxNumMembers)
	{
		LES_FATAL_ERROR("TEST struct '%s' : MemberIndex too big index:%d max:%d member:'%s' type:'%s'",
										testStructDataPtr->structName, testStructDataPtr->globalMemberIndex, maxNumMembers, name, type);
		return false;
	}
	const LES_Hash nameHash = LES_GenerateHashCaseSensitive(name);
	// Test to see if the member has already been added
	if (structDefinitionPtr->GetMember(nameHash) != LES_NULL)
	{
		LES_FATAL_ERROR("TEST struct '%s' : member '%s' already exists type:'%s'", testStructDataPtr->structName, name, type);
		return false;
	}
	const int typeID = LES_AddStringEntry(type);
	const LES_StringEntry* const typeStringEntry = LES_GetStringEntryForID(typeID);
	const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntry);
	if (typeEntryPtr == LES_NULL)
	{
		LES_FATAL_ERROR("TEST struct '%s' : member '%s' type '%s' not found", testStructDataPtr->structName, name, type);
		return false;
	}
	int memberDataSize = typeEntryPtr->m_dataSize;
	if ((typeEntryPtr->m_flags & LES_TYPE_ARRAY) && ((typeEntryPtr->m_flags & LES_TYPE_REFERENCE) == 0))
	{
		// Member variables which are arrays and not references are not pointers but are N * TYPE
		const int numElements = typeEntryPtr->m_numElements;
		int dataSize = typeEntryPtr->ComputeDataStorageSize();
		memberDataSize = dataSize * numElements;
		memberDataSize = dataSize;
	}
	const int globalMemberIndex = testStructDataPtr->globalMemberIndex;
	LES_StructMember* const structMemberPtr = (LES_StructMember* const)(structDefinitionPtr->GetMemberByIndex(globalMemberIndex));
	structMemberPtr->m_hash = nameHash;
	structMemberPtr->m_nameID = LES_AddStringEntry(name);
	structMemberPtr->m_typeID = typeID;
	structMemberPtr->m_dataSize = memberDataSize;
	const int memberAlignment = typeEntryPtr->ComputeAlignment();
	const int alignmentPadding = LES_StructComputeAlignmentPadding(testStructDataPtr->totalMemberSizeWithPadding, memberAlignment);
	if (memberAlignment > testStructDataPtr->maxMemberSizeAlignment)
	{
		testStructDataPtr->maxMemberSizeAlignment = memberAlignment;
	}
	structMemberPtr->m_alignmentPadding = alignmentPadding;
	testStructDataPtr->totalMemberSizeWithPadding += (memberDataSize + alignmentPadding);
	testStructDataPtr->globalMemberIndex++;
#if 0
	LES_LOG("Struct:%s Member:%s Type:%s 0x%X DataSize:%d aligmentPadding:%d", 
					testStructDataPtr->structName, name, type, LES_GenerateHashCaseSensitive(type), memberDataSize, alignmentPadding);
#endif //#if 0

	return true;
}

bool LES_TestStructEnd(LES_StructDefinition* const structDefinitionPtr, LES_TEST_STRUCT_DATA* testStructDataPtr)
{
	if (testStructDataPtr->globalMemberIndex != structDefinitionPtr->GetNumMembers())
	{
		LES_FATAL_ERROR("TEST struct '%s' : ERROR not the right number of members Added:%d Should be:%d",
										testStructDataPtr->structName, testStructDataPtr->globalMemberIndex, structDefinitionPtr->GetNumMembers());
		return false;
	}
	// Align the structure to the biggest size in the structure but a maximum of 4-byte or 8-byte
	// should make the 4-byte or 8-byte a variable (from global params)
	//const int structMaxAlignment = 8;
	const int structMaxAlignment = 4;
	int structAlignment = testStructDataPtr->maxMemberSizeAlignment;
	if (structAlignment > structMaxAlignment)
	{
			structAlignment = structMaxAlignment;
	}

	const int paddingAmount = LES_StructComputeAlignmentPadding(testStructDataPtr->totalMemberSizeWithPadding, structAlignment);
	testStructDataPtr->totalMemberSizeWithPadding += paddingAmount;

	if (LES_AddStructDefinition(testStructDataPtr->structName, structDefinitionPtr, testStructDataPtr->totalMemberSizeWithPadding) < 0)
	{
		LES_FATAL_ERROR("TEST struct '%s' : ERROR adding structure definition", testStructDataPtr->structName);
		return false;
	}

	return true;
}
