#ifndef LES_TEST_MACROS_HH
#define LES_TEST_MACROS_HH

#include "les_core.h"
#include "les_function.h"
#include "les_struct.h"

struct LES_TEST_FUNCTION_DATA
{
	const char* functionName;
	int globalParamIndex;
	int inputParamIndex;
	int outputParamIndex;
};

bool LES_TestFunctionStart(const char* const functionName, const char* const returnType, const int numInputs, const int numOutputs,
													 LES_FunctionDefinition* const functionDefinitionPtr, LES_TEST_FUNCTION_DATA* testFunctionDataPtr);

#define LES_TEST_FUNCTION_START(NAME, RETURN_TYPE, NUM_INPUTS, NUM_OUTPUTS) \
	{ \
		LES_TEST_FUNCTION_DATA testFunctionData; \
		LES_FunctionDefinition functionDefinition; \
		bool __LES_function_ok = true; \
		__LES_function_ok = LES_TestFunctionStart( #NAME, #RETURN_TYPE, NUM_INPUTS, NUM_OUTPUTS, \
																							 &functionDefinition, &testFunctionData); \


bool LES_TestFunctionAddParam(const bool isInput, const char* const type, const char* const name, 
															LES_FunctionDefinition* const functionDefinitionPtr, LES_TEST_FUNCTION_DATA* testFunctionDataPtr);

#define	LES_TEST_FUNCTION_ADD_PARAM(IS_INPUT, TYPE, NAME) \
	if (__LES_function_ok) \
	{ \
		__LES_function_ok = LES_TestFunctionAddParam( IS_INPUT, #TYPE, #NAME, \
																									&functionDefinition, &testFunctionData); \
	} \


#define	LES_TEST_FUNCTION_ADD_INPUT(TYPE, NAME) \
	LES_TEST_FUNCTION_ADD_PARAM(true, TYPE, NAME) \


#define	LES_TEST_FUNCTION_ADD_OUTPUT(TYPE, NAME) \
	LES_TEST_FUNCTION_ADD_PARAM(false, TYPE, NAME) \


void LES_TestFunctionStart(LES_FunctionDefinition* const functionDefinitionPtr, LES_TEST_FUNCTION_DATA* testFunctionDataPtr);

#define LES_TEST_FUNCTION_END() \
		if (__LES_function_ok) \
		{ \
			LES_TestFunctionStart(&functionDefinition, &testFunctionData); \
		} \
		else \
		{ \
			fprintf(stderr, "LES ERROR: TEST function '%s' : ERROR cannot create definition\n", testFunctionData.functionName); \
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


#define LES_TEST_ADD_TYPE_POD(TYPE) \
	LES_TEST_ADD_TYPE_EX(TYPE, sizeof(TYPE), LES_TYPE_INPUT|LES_TYPE_POD, TYPE) \


#define LES_TEST_ADD_TYPE_POD_POINTER(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE*, sizeof(TYPE*), FLAGS|LES_TYPE_POINTER|LES_TYPE_POD, TYPE) \


#define LES_TEST_ADD_TYPE_POD_REFERENCE(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE&, sizeof(TYPE*), FLAGS|LES_TYPE_REFERENCE|LES_TYPE_POD, TYPE*) \


#define LES_TEST_ADD_TYPE_STRUCT(TYPE) \
	LES_TEST_ADD_TYPE_EX(TYPE, sizeof(TYPE), LES_TYPE_INPUT|LES_TYPE_STRUCT, TYPE) \


#define LES_TEST_ADD_TYPE_STRUCT_POINTER(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE*, sizeof(TYPE*), FLAGS|LES_TYPE_STRUCT|LES_TYPE_POINTER, TYPE) \


#define LES_TEST_ADD_TYPE_STRUCT_REFERENCE(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE&, sizeof(TYPE*), FLAGS|LES_TYPE_REFERENCE|LES_TYPE_STRUCT, TYPE*) \


struct LES_TEST_STRUCT_DATA
{
	const char* structName;
	int globalMemberIndex;
	int totalMemberSizeWithPadding;
};

bool LES_TestStructStart(const char* const structName, const int numMembers, 
												 LES_StructDefinition* const structDefinitionPtr, LES_TEST_STRUCT_DATA* testStructDataPtr);

#define LES_TEST_STRUCT_START(NAME, NUM_MEMBERS) \
	{ \
		LES_StructDefinition structDefinition; \
		LES_TEST_STRUCT_DATA testStructData; \
		bool __LES_struct_ok = LES_TestStructStart(#NAME, NUM_MEMBERS, &structDefinition, &testStructData); \


#define	LES_TEST_STRUCT_ADD_MEMBER(TYPE, NAME) \
{ \
	bool __LES_ok = __LES_struct_ok; \
	/* Error if member index off the end of the array */ \
	const int maxNumMembers = structDefinition.GetNumMembers(); \
	if (testStructData.globalMemberIndex >= maxNumMembers) \
	{ \
		fprintf(stderr, "LES ERROR: TEST struct '%s' : MemberIndex too big index:%d max:%d member:'%s' type:'%s'\n", \
						testStructData.structName, testStructData.globalMemberIndex, maxNumMembers, #NAME, #TYPE); \
		__LES_ok = false; \
		__LES_struct_ok = false; \
	} \
	const LES_Hash nameHash = LES_GenerateHashCaseSensitive(#NAME); \
	/* Test to see if the member has already been added */ \
	if (structDefinition.GetMember(nameHash) != LES_NULL) \
	{ \
		fprintf(stderr, "LES ERROR: TEST struct '%s' : member '%s' already exists type:'%s'\n",  \
						testStructData.structName, #NAME, #TYPE); \
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
						testStructData.structName, #NAME, #TYPE); \
			__LES_ok = false; \
			__LES_struct_ok = false; \
		} \
		if (__LES_ok == true) \
		{ \
			const int memberDataSize = typeEntryPtr->m_dataSize; \
			LES_StructMember* const structMemberPtr = (LES_StructMember* const)(structDefinition.GetMemberByIndex(testStructData.globalMemberIndex)); \
			structMemberPtr->m_hash = nameHash; \
			structMemberPtr->m_nameID = LES_AddStringEntry(#NAME); \
			structMemberPtr->m_typeID = typeID; \
			structMemberPtr->m_dataSize = memberDataSize; \
			const int alignmentPadding = LES_StructComputeAlignmentPadding(testStructData.totalMemberSizeWithPadding, memberDataSize); \
			structMemberPtr->m_alignmentPadding = alignmentPadding; \
			testStructData.totalMemberSizeWithPadding += (memberDataSize + alignmentPadding); \
			testStructData.globalMemberIndex++; \
			/*printf("%s %s DataSize:%d aligmentPadding:%d\n", testStructData.structName, #NAME, memberDataSize, alignmentPadding);*/ \
		} \
	} \
} \


#define LES_TEST_STRUCT_END() \
		if (testStructData.globalMemberIndex != structDefinition.GetNumMembers()) \
		{ \
			__LES_struct_ok = false; \
			fprintf(stderr, "LES ERROR: TEST struct '%s' : ERROR not the right number of members Added:%d Should be:%d\n", \
							testStructData.structName, testStructData.globalMemberIndex, structDefinition.GetNumMembers()); \
		} \
		if (__LES_struct_ok == true) \
		{ \
			LES_AddStructDefinition(testStructData.structName, &structDefinition); \
		} \
		else \
		{ \
			fprintf(stderr, "LES ERROR: TEST struct '%s' : ERROR cannot create its definition\n", testStructData.structName ); \
		} \
	} \


#endif // #ifndef LES_TEST_MACROS_HH
