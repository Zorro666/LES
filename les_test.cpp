#include <stdio.h>

#include "les_test.h"
#include "les_core.h"
#include "les_hash.h"

extern int LES_AddStringEntry(const char* const str);
extern int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const functionDefinitionPtr);
extern int LES_AddType(const char* const type, const int typeDataSize);

#define LES_TEST_START_FUNCTION(NAME, RETURN_TYPE, NUM_INPUTS, NUM_OUTPUTS) \
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


#define	LES_TEST_ADD_INPUT(TYPE, NAME) \
	/* Error if inputParamIndex off the end of the array: >= functionDefinition.m_numInputs */ \
	if (inputParamIndex >= functionDefinition.m_numInputs) \
	{ \
		fprintf(stderr, "LES ERROR: TEST function '%s' : inputParamIndex too big index:%d numInputs:%d parameter:'%s' type:'%s'\n", functionName, \
				inputParamIndex, functionDefinition.m_numInputs, #NAME, #TYPE); \
	} \
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[inputParamIndex]; \
	functionParameterPtr->m_index = paramIndex; \
	functionParameterPtr->m_nameID = LES_AddStringEntry(#NAME); \
	functionParameterPtr->m_typeID = LES_AddStringEntry(#TYPE); \
	paramIndex++; \
	inputParamIndex++; \


#define	LES_TEST_ADD_OUTPUT(TYPE, NAME) \
	/* Error if outputParamIndex off the end of the array: >= functionDefinition.m_numOutputs */ \
	if (outputParamIndex >= functionDefinition.m_numOutputs) \
	{ \
		fprintf(stderr, "LES ERROR: TEST function '%s' : outputParamIndex too big index:%d numOutputs:%d parameter:'%s' type:'%s'\n", functionName, \
				outputParamIndex, functionDefinition.m_numOutputs, #NAME, #TYPE); \
	} \
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_outputs[outputParamIndex]; \
	functionParameterPtr->m_index = paramIndex; \
	functionParameterPtr->m_nameID = LES_AddStringEntry(#NAME); \
	functionParameterPtr->m_typeID = LES_AddStringEntry(#TYPE); \


#define LES_TEST_END_FUNCTION() \
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

void LES_TestSetup(void)
{
	/* Sample test types */
	LES_TEST_ADD_TYPE(int);
	LES_TEST_ADD_TYPE(short);
	LES_TEST_ADD_TYPE(float);

	/* Sample test functions */
	LES_TEST_START_FUNCTION(jakeInit, void, 2, 1);
	LES_TEST_ADD_INPUT(int, a);
	LES_TEST_ADD_INPUT(short, b);
	LES_TEST_ADD_OUTPUT(float, r);
	LES_TEST_END_FUNCTION();

	/* Setup test data for specific tests */
	{
		LES_FunctionDefinition functionDefinition;
		functionDefinition.m_nameID = LES_AddStringEntry("LES_Test_ReturnTypeNotFound");
		functionDefinition.m_returnTypeID = -1;
		functionDefinition.m_numInputs = 0;
		functionDefinition.m_inputs = LES_NULL;
		functionDefinition.m_numOutputs = 0;
		functionDefinition.m_outputs = LES_NULL;
		LES_AddFunctionDefinition("LES_Test_ReturnTypeNotFound", &functionDefinition); 
	}

	/* Run specific tests */
	LES_Test_FunctionNotUsed();
	LES_Test_ReturnTypeNotFound();

}

