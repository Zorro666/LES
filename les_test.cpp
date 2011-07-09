#include "les_test.h"
#include "les_function.h"

extern int LES_AddStringEntry(const char* const str);
extern int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const functionDefinitionPtr);
extern int LES_AddType(const char* const type, const int typeDataSize);

#define LES_TEST_FUNCTION_START(NAME, RETURN_TYPE, NUM_INPUTS, NUM_OUTPUTS) \
	{ \
		LES_FunctionDefinition functionDefinition; \
		LES_FunctionParameter* functionParameterPtr; \
		int paramDataSize = 0; \
		int globalParamIndex = 0; \
		int inputParamIndex = 0; \
		int outputParamIndex = 0; \
		const char* const functionName = #NAME; \
		functionDefinition.m_nameID = LES_AddStringEntry(functionName); \
		functionDefinition.m_returnTypeID = LES_AddStringEntry(#RETURN_TYPE); \
		functionDefinition.m_numInputs = NUM_INPUTS; \
		functionDefinition.m_inputs = new LES_FunctionParameter[functionDefinition.m_numInputs]; \
		functionDefinition.m_numOutputs = NUM_OUTPUTS; \
		functionDefinition.m_outputs = new LES_FunctionParameter[functionDefinition.m_numOutputs]; \
		globalParamIndex += 0; \
		inputParamIndex += 0; \
		outputParamIndex += 0; \
		functionParameterPtr = NULL; \


#define	LES_TEST_FUNCTION_ADD_PARAM(IS_INPUT, TYPE, NAME) \
{ \
	bool __LES_ok = true; \
	/* Error if parameter index off the end of the array */ \
	const int maxNumParam = (IS_INPUT ? functionDefinition.m_numInputs : functionDefinition.m_numOutputs); \
	int* const paramIndex = (IS_INPUT ? &inputParamIndex : &outputParamIndex); \
	const char* const paramModeStr = (IS_INPUT ? "Input" : "Output"); \
	const int paramMode = (IS_INPUT ? LES_PARAM_MODE_INPUT : LES_PARAM_MODE_OUTPUT); \
	if (*paramIndex >= maxNumParam) \
	{ \
		fprintf(stderr, "LES ERROR: TEST function '%s' : %s ParamIndex too big index:%d max:%d parameter:'%s' type:'%s'\n", \
						functionName, paramModeStr, *paramIndex, maxNumParam, #NAME, #TYPE); \
		__LES_ok = false; \
	} \
	const LES_Hash nameHash = LES_GenerateHashCaseSensitive(#NAME); \
	/* Test to see if the parameter has already been added */ \
	if (functionDefinition.GetParameter(nameHash) != LES_NULL) \
	{ \
		fprintf(stderr, "LES ERROR: TEST function '%s' : parameter '%s' already exists type:'%s'\n",  \
						functionName, #NAME, #TYPE); \
		__LES_ok = false; \
	} \
	if (__LES_ok == true ) \
	{ \
		functionParameterPtr = (LES_FunctionParameter* const)(IS_INPUT ? \
				&functionDefinition.m_inputs[*paramIndex] : &functionDefinition.m_outputs[*paramIndex]); \
		functionParameterPtr->m_index = globalParamIndex; \
		functionParameterPtr->m_hash = nameHash; \
		functionParameterPtr->m_nameID = LES_AddStringEntry(#NAME); \
		functionParameterPtr->m_typeID = LES_AddStringEntry(#TYPE); \
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

void LES_Test_FunctionNotFound(void)
{
	LES_FUNCTION_START(LES_Test_FunctionNotFound, void);
	LES_FUNCTION_END();
}

void LES_Test_ReturnTypeNotFound(void)
{
	LES_FUNCTION_START(LES_Test_ReturnTypeNotFound, jake);
	LES_FUNCTION_END();
}

void LES_Test_ReturnTypeHashIsWrong()
{
	LES_FUNCTION_START(LES_Test_ReturnTypeHashIsWrong, wrongReturnType);
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

void LES_Test_InputWrongIndex(int input_0, int input_1, int output_0)
{
	LES_FUNCTION_START(LES_Test_InputWrongIndex, void);
	LES_FUNCTION_ADD_INPUT(0, int, input_1);
	LES_FUNCTION_ADD_INPUT(1, int, input_0);
	LES_FUNCTION_ADD_OUTPUT(0, int, output_0);
	LES_FUNCTION_END();
}

void LES_Test_InputNameDoesntExist(int inputNotExist)
{
	LES_FUNCTION_START(LES_Test_InputNameDoesntExist, void);
	LES_FUNCTION_ADD_INPUT(0, int, inputNotExist);
	LES_FUNCTION_END();
}

void LES_Test_InputNameIDNotFound(int input_0)
{
	LES_FUNCTION_START(LES_Test_InputNameIDNotFound, void);
	LES_FUNCTION_ADD_INPUT(0, int, input_0);
	LES_FUNCTION_END();
}

void LES_Test_InputNameHashIsWrong(int input_0)
{
	LES_FUNCTION_START(LES_Test_InputNameHashIsWrong, void);
	LES_FUNCTION_ADD_INPUT(0, int, input_0);
	LES_FUNCTION_END();
}

void LES_Test_OutputGlobalIndexIsWrong(int input_0, int output_0)
{
	LES_FUNCTION_START(LES_Test_OutputGlobalIndexIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(0, int, output_0);
	LES_FUNCTION_ADD_INPUT(0, int, input_0);
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

	LES_TEST_FUNCTION_START(LES_Test_InputWrongIndex, void, 2, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_1);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	{
		LES_TEST_FUNCTION_START(LES_Test_InputNameIDNotFound, void, 1, 0);

		functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[0];
		LES_AddStringEntry("input_0");
		functionParameterPtr->m_index = 0;
		functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
		functionParameterPtr->m_nameID = -1;
		functionParameterPtr->m_typeID = LES_AddStringEntry("int");
		functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;

		LES_TEST_FUNCTION_END();
	}

	LES_TEST_FUNCTION_START(LES_Test_InputNameDoesntExist, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	{
		LES_TEST_FUNCTION_START(LES_Test_InputNameHashIsWrong, void, 1, 0);
		functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[0];
		functionParameterPtr->m_index = 0;
		functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
		functionParameterPtr->m_nameID = LES_AddStringEntry("wrongHash");
		functionParameterPtr->m_typeID = LES_AddStringEntry("int");
		functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
		LES_TEST_FUNCTION_END();
	}
	LES_TEST_FUNCTION_START(LES_Test_OutputGlobalIndexIsWrong, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	/* Run specific tests */
	LES_Test_FunctionNotFound();
	fprintf(stderr, "\n");
	LES_Test_ReturnTypeNotFound();
	fprintf(stderr, "\n");
	LES_Test_ReturnTypeHashIsWrong();
	fprintf(stderr, "\n");
	LES_Test_TooManyInputParameters(1, 2, 3, 4);
	fprintf(stderr, "\n");

	LES_Test_InputWrongIndex(1, 2, 3);
	fprintf(stderr, "\n");
	LES_Test_InputNameDoesntExist(1);
	fprintf(stderr, "\n");
	LES_Test_InputNameIDNotFound(1);
	fprintf(stderr, "\n");
	LES_Test_InputNameHashIsWrong(1);
	fprintf(stderr, "\n");

	LES_TEST_FUNCTION_START(LES_Test_InputParameterAlreadyExists, void, 2, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_INPUT(char, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();
	fprintf(stderr, "\n");

	LES_TEST_FUNCTION_START(LES_Test_OutputParameterAlreadyExists, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(short, output_0);
	LES_TEST_FUNCTION_END();
	fprintf(stderr, "\n");

	LES_TEST_FUNCTION_START(LES_Test_ParameterAlreadyExists, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(float, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();
	fprintf(stderr, "\n");

	LES_Test_OutputGlobalIndexIsWrong(1, 2);
	fprintf(stderr, "\n");

}

