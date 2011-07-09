#include <stdio.h>
#include <string.h>

#include "les_test.h"
#include "les_core.h"
#include "les_hash.h"
#include "les_stringentry.h"

extern int LES_AddStringEntry(const char* const str);
extern int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const functionDefinitionPtr);
extern int LES_AddType(const char* const type, const int typeDataSize);

#define LES_TEST_FUNCTION_START(NAME, RETURN_TYPE, NUM_INPUTS, NUM_OUTPUTS) \
	{ \
		LES_FunctionDefinition functionDefinition; \
		LES_FunctionParameter* functionParameterPtr; \
		int paramDataSize = 0; \
		int paramIndex = 0; \
		int inputParamIndex = 0; \
		int outputParamIndex = 0; \
		const char* const functionName = #NAME; \
		functionDefinition.m_nameID = LES_AddStringEntry(functionName); \
		functionDefinition.m_returnTypeID = LES_AddStringEntry(#RETURN_TYPE); \
		functionDefinition.m_numInputs = NUM_INPUTS; \
		functionDefinition.m_inputs = new LES_FunctionParameter[functionDefinition.m_numInputs]; \
		functionDefinition.m_numOutputs = NUM_OUTPUTS; \
		functionDefinition.m_outputs = new LES_FunctionParameter[functionDefinition.m_numOutputs]; \
		paramIndex += 0; \
		inputParamIndex += 0; \
		outputParamIndex += 0; \
		functionParameterPtr = NULL; \


#define	LES_TEST_FUNCTION_ADD_INPUT(TYPE, NAME) \
	/* Error if inputParamIndex off the end of the array: >= functionDefinition.m_numInputs */ \
	if (inputParamIndex >= functionDefinition.m_numInputs) \
	{ \
		fprintf(stderr, "LES ERROR: TEST function '%s' : inputParamIndex too big index:%d numInputs:%d parameter:'%s' type:'%s'\n", \
						functionName, inputParamIndex, functionDefinition.m_numInputs, #NAME, #TYPE); \
	} \
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[inputParamIndex]; \
	functionParameterPtr->m_index = paramIndex; \
	functionParameterPtr->m_nameID = LES_AddStringEntry(#NAME); \
	functionParameterPtr->m_typeID = LES_AddStringEntry(#TYPE); \
	paramIndex++; \
	inputParamIndex++; \


#define	LES_TEST_FUNCTION_ADD_OUTPUT(TYPE, NAME) \
	/* Error if outputParamIndex off the end of the array: >= functionDefinition.m_numOutputs */ \
	if (outputParamIndex >= functionDefinition.m_numOutputs) \
	{ \
		fprintf(stderr, "LES ERROR: TEST function '%s' : outputParamIndex too big index:%d numOutputs:%d parameter:'%s' type:'%s'\n", \
						functionName, outputParamIndex, functionDefinition.m_numOutputs, #NAME, #TYPE); \
	} \
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_outputs[outputParamIndex]; \
	functionParameterPtr->m_index = paramIndex; \
	functionParameterPtr->m_nameID = LES_AddStringEntry(#NAME); \
	functionParameterPtr->m_typeID = LES_AddStringEntry(#TYPE); \


#define LES_TEST_FUNCTION_END() \
		functionDefinition.m_paramDataSize = paramDataSize; \
		LES_AddFunctionDefinition(functionName, &functionDefinition); \
	} \


#define LES_TEST_ADD_TYPE_EX(TYPE,SIZE) \
	if (LES_AddType(#TYPE, SIZE) == LES_ERROR) \
	{\
		fprintf(stderr, "LES ERROR: TEST AddType '%s' 0x%X failed\n", #TYPE, LES_GenerateHashCaseSensitive(#TYPE)); \
	}\


#define LES_TEST_ADD_TYPE(TYPE) \
	LES_TEST_ADD_TYPE_EX(TYPE, sizeof(TYPE)) \


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal test functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LES_Test_FunctionNotUsed(void)
{
	LES_FUNCTION_START(LES_Test_FunctionNotFound, void);
	LES_FUNCTION_END();
}

void LES_Test_ReturnTypeNotFound(void)
{
	LES_FUNCTION_START(LES_Test_ReturnTypeNotFound, jake);
	LES_FUNCTION_END();
}

void LES_Test_ReturnTypeHashWrong()
{
	LES_FUNCTION_START(LES_Test_ReturnTypeHashWrong, wrongReturnType);
	LES_FUNCTION_END();
}

void LES_Test_TooManyInputParameters(int input_0, int input_1, int input_2, int output_0)
{
	LES_FUNCTION_START(LES_Test_TooManyInputParameters, void);
	LES_FUNCTION_ADD_INPUT(0, int, input_0);
	LES_FUNCTION_ADD_INPUT(1, int, input_1);
	LES_FUNCTION_ADD_INPUT(2, int, input_2);
	LES_FUNCTION_ADD_OUTPUT(0, int, output_0);
	LES_FUNCTION_END();
}

void LES_Test_InputNameHashIsWrong(int input_0, int input_1, int output_0)
{
	LES_FUNCTION_START(LES_Test_InputNameHashIsWrong, void);
	LES_FUNCTION_ADD_INPUT(0, int, input_1);
	LES_FUNCTION_ADD_INPUT(1, int, input_0);
	LES_FUNCTION_ADD_OUTPUT(0, int, output_0);
	LES_FUNCTION_END();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// External functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LES_TestSetup(void)
{
	/* Sample types for development */
	LES_TEST_ADD_TYPE(int);
	LES_TEST_ADD_TYPE(short);
	LES_TEST_ADD_TYPE(float);

	/* Sample functions for development */
	LES_TEST_FUNCTION_START(jakeInit, void, 2, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, a);
	LES_TEST_FUNCTION_ADD_INPUT(short, b);
	LES_TEST_FUNCTION_ADD_OUTPUT(float, r);
	LES_TEST_FUNCTION_END();

	/* Setup test data for specific tests */
	{
		const char* const functionName = "LES_Test_ReturnTypeNotFound";
		LES_FunctionDefinition functionDefinition;
		functionDefinition.m_nameID = LES_AddStringEntry(functionName);
		functionDefinition.m_returnTypeID = -1;
		functionDefinition.m_numInputs = 0;
		functionDefinition.m_inputs = LES_NULL;
		functionDefinition.m_numOutputs = 0;
		functionDefinition.m_outputs = LES_NULL;
		LES_AddFunctionDefinition(functionName, &functionDefinition); 
	}

	LES_TEST_FUNCTION_START(LES_Test_ReturnTypeHashWrong, void, 0, 0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_TooManyInputParameters, void, 2, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_1);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputNameHashIsWrong, void, 2, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_1);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	/* Run specific tests */
	LES_Test_FunctionNotUsed();
	LES_Test_ReturnTypeNotFound();
	LES_Test_ReturnTypeHashWrong();
	LES_Test_TooManyInputParameters(1, 2, 3, 4);
	LES_Test_InputNameHashIsWrong(1, 2, 3);
}

