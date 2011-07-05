#include "les_test.h"
#include "les_core.h"

extern int LES_AddStringEntry(const char* const str);
extern int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const functionDefinitionPtr);

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


#define	LES_TEST_ADD_INPUT(TYPE, NAME) \
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[inputParamIndex]; \
	functionParameterPtr->m_index = paramIndex; \
	functionParameterPtr->m_nameID = LES_AddStringEntry(#NAME); \
	functionParameterPtr->m_typeID = LES_AddStringEntry(#TYPE); \
	paramIndex++; \
	inputParamIndex++; \


#define	LES_TEST_ADD_OUTPUT(TYPE, NAME) \
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_outputs[outputParamIndex]; \
	functionParameterPtr->m_index = paramIndex; \
	functionParameterPtr->m_nameID = LES_AddStringEntry(#NAME); \
	functionParameterPtr->m_typeID = LES_AddStringEntry(#TYPE); \


#define LES_TEST_END_FUNCTION() \
		functionDefinition.m_paramDataSize = paramDataSize; \
		LES_AddFunctionDefinition(functionName, &functionDefinition); \
	} \


void LES_TestSetup(void)
{
	LES_TEST_START_FUNCTION(sceNpInit, void, 1, 1);
	LES_TEST_ADD_INPUT(int, a);
	LES_TEST_ADD_INPUT(short, b);
	LES_TEST_ADD_OUTPUT(short, r);
	LES_TEST_END_FUNCTION();
}

