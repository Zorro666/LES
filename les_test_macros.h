#ifndef LES_TEST_MACROS_HH
#define LES_TEST_MACROS_HH

#include "les_base.h"
#include "les_logger.h"
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
			__LES_function_ok = LES_TestFunctionAddParam( IS_INPUT, #TYPE, #NAME, &functionDefinition, &testFunctionData); \
		} \


#define	LES_TEST_FUNCTION_ADD_INPUT(TYPE, NAME) \
	LES_TEST_FUNCTION_ADD_PARAM(true, TYPE, NAME) \


#define	LES_TEST_FUNCTION_ADD_OUTPUT(TYPE, NAME) \
	LES_TEST_FUNCTION_ADD_PARAM(false, TYPE, NAME) \


bool LES_TestFunctionEnd(LES_FunctionDefinition* const functionDefinitionPtr, LES_TEST_FUNCTION_DATA* testFunctionDataPtr);

#define LES_TEST_FUNCTION_END() \
		if (__LES_function_ok) \
		{ \
			__LES_function_ok = LES_TestFunctionEnd(&functionDefinition, &testFunctionData); \
		} \
		if (__LES_function_ok == false) \
		{ \
			LES_FATAL_ERROR("TEST function '%s' : ERROR cannot create definition\n", testFunctionData.functionName); \
		} \
	} \


#define LES_TEST_ADD_TYPE_EX(TYPE, SIZE, FLAGS, ALIASED_TYPE, NUM_ELEMENTS) \
	{\
		if (LES_AddType(#TYPE, SIZE, FLAGS, #ALIASED_TYPE, NUM_ELEMENTS) == LES_RETURN_ERROR) \
		{\
			LES_FATAL_ERROR("TEST AddType '%s' NumElements:%d 0x%X Alias '%s' failed\n", \
								#TYPE, NUM_ELEMENTS, LES_GenerateHashCaseSensitive(#TYPE), #ALIASED_TYPE); \
		}\
	}\


#define LES_TEST_ADD_TYPE_POD(TYPE) \
	LES_TEST_ADD_TYPE_EX(TYPE, sizeof(TYPE), LES_TYPE_INPUT|LES_TYPE_POD, TYPE, 0) \


#define LES_TEST_ADD_TYPE_POD_POINTER(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE*, sizeof(TYPE*), FLAGS|LES_TYPE_POINTER|LES_TYPE_POD, TYPE, 0) \


#define LES_TEST_ADD_TYPE_POD_REFERENCE(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE&, sizeof(TYPE*), FLAGS|LES_TYPE_REFERENCE|LES_TYPE_POD, TYPE*, 0) \


#define LES_TEST_ADD_TYPE_POD_ARRAY(TYPE, NUM_ELEMENTS) \
	LES_TEST_ADD_TYPE_EX(TYPE[NUM_ELEMENTS], sizeof(TYPE*), LES_TYPE_ARRAY|LES_TYPE_INPUT|LES_TYPE_OUTPUT|LES_TYPE_POD, TYPE, NUM_ELEMENTS) \


#define LES_TEST_ADD_TYPE_POD_REFERENCE_ARRAY(TYPE, NUM_ELEMENTS, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE&[NUM_ELEMENTS], sizeof(TYPE), LES_TYPE_ARRAY|FLAGS|LES_TYPE_REFERENCE|LES_TYPE_POD, TYPE*, NUM_ELEMENTS) \


#define LES_TEST_ADD_TYPE_STRUCT(TYPE) \
	LES_TEST_ADD_TYPE_EX(TYPE, sizeof(TYPE), LES_TYPE_INPUT|LES_TYPE_STRUCT, TYPE, 0) \


#define LES_TEST_ADD_TYPE_STRUCT_POINTER(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE*, sizeof(TYPE*), FLAGS|LES_TYPE_STRUCT|LES_TYPE_POINTER, TYPE, 0) \


#define LES_TEST_ADD_TYPE_STRUCT_REFERENCE(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE&, sizeof(TYPE*), FLAGS|LES_TYPE_REFERENCE|LES_TYPE_STRUCT, TYPE*, 0) \


#define LES_TEST_ADD_TYPE_STRUCT_ARRAY(TYPE, NUM_ELEMENTS) \
	LES_TEST_ADD_TYPE_EX(TYPE[NUM_ELEMENTS], sizeof(TYPE*), LES_TYPE_ARRAY|LES_TYPE_INPUT|LES_TYPE_OUTPUT|LES_TYPE_STRUCT, TYPE, NUM_ELEMENTS) \


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


bool LES_TestStructAddMember(const char* const type, const char* const name,
												 		 LES_StructDefinition* const structDefinitionPtr, LES_TEST_STRUCT_DATA* testStructDataPtr);

#define	LES_TEST_STRUCT_ADD_MEMBER(TYPE, NAME) \
	if (__LES_struct_ok) \
	{ \
		__LES_struct_ok = LES_TestStructAddMember(#TYPE, #NAME, &structDefinition, &testStructData); \
	} \

extern int LES_AddStructDefinition(const char* const name, const LES_StructDefinition* const structDefinitionPtr);

bool LES_TestStructEnd(LES_StructDefinition* const structDefinitionPtr, LES_TEST_STRUCT_DATA* testStructDataPtr);

#define LES_TEST_STRUCT_END() \
		if (__LES_struct_ok) \
		{ \
			__LES_struct_ok = LES_TestStructEnd(&structDefinition, &testStructData); \
		} \
		if (__LES_struct_ok == false) \
		{ \
			LES_FATAL_ERROR("TEST struct '%s' : ERROR cannot create definition\n", testStructData.structName); \
		} \
	} \


#endif // #ifndef LES_TEST_MACROS_HH
