#ifndef LES_TEST_MACROS_HH
#define LES_TEST_MACROS_HH

#include "les_core.h"
#include "les_function.h"
#include "les_struct.h"

#define LES_TEST_FUNCTION_START(NAME, RETURN_TYPE, NUM_INPUTS, NUM_OUTPUTS) \
	{ \
		bool __LES_function_ok = true; \
		const char* const functionName = #NAME; \
		const int nameID = LES_AddStringEntry(functionName); \
		const int returnTypeID = LES_AddStringEntry(#RETURN_TYPE); \
		LES_FunctionDefinition functionDefinition(nameID, returnTypeID, NUM_INPUTS, NUM_OUTPUTS); \
		LES_FunctionParameter* functionParameterPtr; \
		int parameterDataSize = 0; \
		int globalParamIndex = 0; \
		int inputParamIndex = 0; \
		int outputParamIndex = 0; \
		globalParamIndex += 0; \
		inputParamIndex += 0; \
		outputParamIndex += 0; \
		functionParameterPtr = NULL; \


#define	LES_TEST_FUNCTION_ADD_PARAM(IS_INPUT, TYPE, NAME) \
{ \
	bool __LES_ok = __LES_function_ok; \
	/* Error if parameter index off the end of the array */ \
	const int maxNumParam = (IS_INPUT ? functionDefinition.GetNumInputs() : functionDefinition.GetNumOutputs()); \
	int* const paramIndex = (IS_INPUT ? &inputParamIndex : &outputParamIndex); \
	const char* const paramModeStr = (IS_INPUT ? "Input" : "Output"); \
	const int paramMode = (IS_INPUT ? LES_PARAM_MODE_INPUT : LES_PARAM_MODE_OUTPUT); \
	if (*paramIndex >= maxNumParam) \
	{ \
		fprintf(stderr, "LES ERROR: TEST function '%s' : %s ParamIndex too big index:%d max:%d parameter:'%s' type:'%s'\n", \
						functionName, paramModeStr, *paramIndex, maxNumParam, #NAME, #TYPE); \
		__LES_ok = false; \
		__LES_function_ok = false; \
	} \
	const LES_Hash nameHash = LES_GenerateHashCaseSensitive(#NAME); \
	/* Test to see if the parameter has already been added */ \
	if (functionDefinition.GetParameter(nameHash) != LES_NULL) \
	{ \
		fprintf(stderr, "LES ERROR: TEST function '%s' : parameter '%s' already exists type:'%s'\n",  \
						functionName, #NAME, #TYPE); \
		__LES_ok = false; \
		__LES_function_ok = false; \
	} \
	if (__LES_ok == true) \
	{ \
		const int typeID = LES_AddStringEntry(#TYPE); \
		const LES_StringEntry* const typeStringEntry = LES_GetStringEntryForID(typeID); \
		const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntry); \
		if (typeEntryPtr != LES_NULL) \
		{ \
			parameterDataSize += typeEntryPtr->m_dataSize; \
		} \
		functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(globalParamIndex)); \
		functionParameterPtr->m_index = globalParamIndex; \
		functionParameterPtr->m_hash = nameHash; \
		functionParameterPtr->m_nameID = LES_AddStringEntry(#NAME); \
		functionParameterPtr->m_typeID = typeID; \
		functionParameterPtr->m_mode = paramMode; \
		globalParamIndex++; \
		*paramIndex = *paramIndex + 1; \
	} \
} \


#define	LES_TEST_FUNCTION_ADD_INPUT(TYPE, NAME) \
	LES_TEST_FUNCTION_ADD_PARAM(true, TYPE, NAME) \


#define	LES_TEST_FUNCTION_ADD_OUTPUT(TYPE, NAME) \
	LES_TEST_FUNCTION_ADD_PARAM(false, TYPE, NAME) \


#define LES_TEST_FUNCTION_END() \
		if (globalParamIndex != functionDefinition.GetNumParameters()) \
		{ \
			__LES_function_ok = false; \
			fprintf(stderr, "LES ERROR: TEST function '%s' : ERROR not the right number of parameters Added:%d Should be:%d\n", \
							functionName, globalParamIndex, functionDefinition.GetNumParameters()); \
		} \
		if (__LES_function_ok == true) \
		{ \
			functionDefinition.SetParameterDataSize(parameterDataSize); \
			LES_AddFunctionDefinition(functionName, &functionDefinition); \
		} \
		else \
		{ \
			fprintf(stderr, "LES ERROR: TEST function '%s' : ERROR cannot create definition\n", functionName); \
		} \
	} \



#define LES_TEST_ADD_TYPE_EX(TYPE, SIZE, FLAGS, ALIASED_TYPE) \
	{\
		if (LES_AddType(#TYPE, SIZE, FLAGS, #ALIASED_TYPE) == LES_ERROR) \
		{\
			fprintf(stderr, "LES ERROR: TEST AddType '%s' 0x%X Alias '%s' failed\n", \
							#TYPE, LES_GenerateHashCaseSensitive(#TYPE), #ALIASED_TYPE); \
		}\
	}\


#define LES_TEST_ADD_TYPE_POD_POINTER(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE*, sizeof(TYPE*), FLAGS|LES_TYPE_POINTER|LES_TYPE_POD,TYPE) \


#define LES_TEST_ADD_TYPE_POD(TYPE) \
	LES_TEST_ADD_TYPE_EX(TYPE, sizeof(TYPE), LES_TYPE_INPUT|LES_TYPE_POD, TYPE) \


#define LES_TEST_ADD_TYPE_STRUCT(TYPE) \
	LES_TEST_ADD_TYPE_EX(TYPE, sizeof(TYPE), LES_TYPE_INPUT|LES_TYPE_STRUCT, TYPE) \

#define LES_TEST_ADD_TYPE_STRUCT_POINTER(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE*, sizeof(TYPE*), FLAGS|LES_TYPE_STRUCT|LES_TYPE_POINTER, TYPE) \


#define LES_TEST_STRUCT_START(NAME, NUM_MEMBERS) \
	{ \
		bool __LES_struct_ok = true; \
		const char* const structName = #NAME; \
		const int nameID = LES_AddStringEntry(structName); \
		LES_StructDefinition structDefinition(nameID, NUM_MEMBERS); \
		LES_StructMember* structMemberPtr; \
		int globalMemberIndex = 0; \
		int totalMemberSize = 0; \
		globalMemberIndex += 0; \
		structMemberPtr = NULL; \


#define	LES_TEST_STRUCT_ADD_MEMBER(TYPE, NAME) \
{ \
	bool __LES_ok = __LES_struct_ok; \
	/* Error if member index off the end of the array */ \
	const int maxNumMembers = structDefinition.GetNumMembers(); \
	if (globalMemberIndex >= maxNumMembers) \
	{ \
		fprintf(stderr, "LES ERROR: TEST struct '%s' : MemberIndex too big index:%d max:%d member:'%s' type:'%s'\n", \
						structName, globalMemberIndex, maxNumMembers, #NAME, #TYPE); \
		__LES_ok = false; \
		__LES_struct_ok = false; \
	} \
	const LES_Hash nameHash = LES_GenerateHashCaseSensitive(#NAME); \
	/* Test to see if the member has already been added */ \
	if (structDefinition.GetMember(nameHash) != LES_NULL) \
	{ \
		fprintf(stderr, "LES ERROR: TEST struct '%s' : member '%s' already exists type:'%s'\n",  \
						structName, #NAME, #TYPE); \
		__LES_ok = false; \
		__LES_struct_ok = false; \
	} \
	if (__LES_ok == true) \
	{ \
		const int typeID = LES_AddStringEntry(#TYPE); \
		const LES_StringEntry* const typeStringEntry = LES_GetStringEntryForID(typeID); \
		const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntry); \
		if (typeEntryPtr == LES_NULL) \
		{ \
			fprintf(stderr, "LES ERROR: TEST struct '%s' : member '%s' type '%s' not found\n",  \
						structName, #NAME, #TYPE); \
			__LES_ok = false; \
			__LES_struct_ok = false; \
		} \
		if (__LES_ok == true) \
		{ \
			const int memberDataSize = typeEntryPtr->m_dataSize; \
			structMemberPtr = (LES_StructMember* const)(structDefinition.GetMemberByIndex(globalMemberIndex)); \
			structMemberPtr->m_hash = nameHash; \
			structMemberPtr->m_nameID = LES_AddStringEntry(#NAME); \
			structMemberPtr->m_typeID = typeID; \
			structMemberPtr->m_dataSize = memberDataSize; \
			const int alignmentPadding = LES_StructComputeAlignmentPadding(totalMemberSize, memberDataSize); \
			structMemberPtr->m_alignmentPadding = alignmentPadding; \
			totalMemberSize += (memberDataSize + alignmentPadding); \
			globalMemberIndex++; \
		} \
	} \
} \


#define LES_TEST_STRUCT_END() \
		if (globalMemberIndex != structDefinition.GetNumMembers()) \
		{ \
			__LES_struct_ok = false; \
			fprintf(stderr, "LES ERROR: TEST struct '%s' : ERROR not the right number of members Added:%d Should be:%d\n", \
							structName, globalMemberIndex, structDefinition.GetNumMembers()); \
		} \
		if (__LES_struct_ok == true) \
		{ \
			structDefinition.SetTotalMemberDataSize(totalMemberSize); \
			LES_AddStructDefinition(structName, &structDefinition); \
		} \
		else \
		{ \
			fprintf(stderr, "LES ERROR: TEST struct '%s' : ERROR cannot create its definition\n", structName ); \
		} \
	} \


#endif // #ifndef LES_TEST_MACROS_HH
