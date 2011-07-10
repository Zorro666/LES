#include "les_test.h"
#include "les_function.h"
#include "les_stringentry.h"

extern int LES_AddStringEntry(const char* const str);
extern int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const functionDefinitionPtr);
extern int LES_AddType(const char* const name, const unsigned int dataSize, const unsigned int flags);

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
	if (__LES_ok == true) \
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


#define LES_TEST_ADD_TYPE_EX(TYPE, SIZE, FLAGS) \
	if (LES_AddType(#TYPE, SIZE, FLAGS) == LES_ERROR) \
	{\
		fprintf(stderr, "LES ERROR: TEST AddType '%s' 0x%X failed\n", #TYPE, LES_GenerateHashCaseSensitive(#TYPE)); \
	}\


#define LES_TEST_ADD_TYPE(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE, sizeof(TYPE), FLAGS) \


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal test functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void LES_Test_FunctionNotFound(void)
{
	LES_FUNCTION_START(LES_Test_FunctionNotFound, void);
	LES_FUNCTION_END();
}

static void LES_Test_ReturnTypeNotFound(void)
{
	LES_FUNCTION_START(LES_Test_ReturnTypeNotFound, jake);
	LES_FUNCTION_END();
}

static void LES_Test_ReturnTypeHashIsWrong()
{
	LES_FUNCTION_START(LES_Test_ReturnTypeHashIsWrong, wrongReturnType);
	LES_FUNCTION_END();
}

static void LES_Test_ReturnTypeStringIsWrong()
{
	LES_FUNCTION_START(LES_Test_ReturnTypeStringIsWrong, ReturnTypeBrokenHash);
	LES_FUNCTION_END();
}

static void LES_Test_TooManyInputParameters(int input_0, int input_1, int input_2, int output_0)
{
	LES_FUNCTION_START(LES_Test_TooManyInputParameters, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_INPUT(int, input_1);
	LES_FUNCTION_ADD_INPUT(int, input_2);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputWrongIndex(int input_0, int input_1, int output_0)
{
	LES_FUNCTION_START(LES_Test_InputWrongIndex, void);
	LES_FUNCTION_ADD_INPUT(int, input_1);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputNameDoesntExist(int inputNotExist)
{
	LES_FUNCTION_START(LES_Test_InputNameDoesntExist, void);
	LES_FUNCTION_ADD_INPUT(int, inputNotExist);
	LES_FUNCTION_END();
}

static void LES_Test_InputNameIDNotFound(int input_0)
{
	LES_FUNCTION_START(LES_Test_InputNameIDNotFound, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputNameHashIsWrong(int input_0)
{
	LES_FUNCTION_START(LES_Test_InputNameHashIsWrong, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputNameStringIsWrong(int InputNameBrokenHash)
{
	LES_FUNCTION_START(LES_Test_InputNameStringIsWrong, void);
	LES_FUNCTION_ADD_INPUT(int, InputNameBrokenHash);
	LES_FUNCTION_END();
}

static void LES_Test_InputTypeIDNotFound(int input_0)
{
	LES_FUNCTION_START(LES_Test_InputTypeIDNotFound, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputTypeHashIsWrong(int input_0)
{
	LES_FUNCTION_START(LES_Test_InputTypeHashIsWrong, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputTypeStringIsWrong(int input_0)
{
	LES_FUNCTION_START(LES_Test_InputTypeStringIsWrong, void);
	LES_FUNCTION_ADD_INPUT(InputTypeBrokenHash, input_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputGlobalIndexIsWrong(int input_0, int output_0)
{
	LES_FUNCTION_START(LES_Test_InputGlobalIndexIsWrong, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputUsedAsOutput(int input_0)
{
	LES_FUNCTION_START(LES_Test_InputUsedAsOutput, void);
	LES_FUNCTION_ADD_OUTPUT(int, input_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputParamAlreadyUsed(int input_0, int output_0)
{
	LES_FUNCTION_START(LES_Test_InputParamAlreadyUsed, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT_EX(2, int, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputParamMissing(int input_0, int output_0)
{
	LES_FUNCTION_START(LES_Test_InputParamMissing, void);
	LES_FUNCTION_ADD_OUTPUT(int*, output_0);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
}

static void LES_Test_TooManyOutputParameters(int input_0, int output_0, int output_1, int output_2)
{
	LES_FUNCTION_START(LES_Test_TooManyOutputParameters, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(int*, output_0);
	LES_FUNCTION_ADD_OUTPUT(int*, output_1);
	LES_FUNCTION_ADD_OUTPUT(int*, output_2);
	LES_FUNCTION_END();
}

static void LES_Test_OutputWrongIndex(int input_0, int output_0, int output_1)
{
	LES_FUNCTION_START(LES_Test_OutputWrongIndex, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT_EX(1, int, output_1);
	LES_FUNCTION_ADD_OUTPUT_EX(2, int, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputNameDoesntExist(int outputNotExist)
{
	LES_FUNCTION_START(LES_Test_OutputNameDoesntExist, void);
	LES_FUNCTION_ADD_OUTPUT(int, outputNotExist);
	LES_FUNCTION_END();
}

static void LES_Test_OutputNameIDNotFound(int output_0)
{
	LES_FUNCTION_START(LES_Test_OutputNameIDNotFound, void);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputNameHashIsWrong(int output_0)
{
	LES_FUNCTION_START(LES_Test_OutputNameHashIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputNameStringIsWrong(int OutputNameBrokenHash)
{
	LES_FUNCTION_START(LES_Test_OutputNameStringIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(int, OutputNameBrokenHash);
	LES_FUNCTION_END();
}

static void LES_Test_OutputTypeIDNotFound(int output_0)
{
	LES_FUNCTION_START(LES_Test_OutputTypeIDNotFound, void);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputTypeHashIsWrong(int output_0)
{
	LES_FUNCTION_START(LES_Test_OutputTypeHashIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputTypeStringIsWrong(int output_0)
{
	LES_FUNCTION_START(LES_Test_OutputTypeStringIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(OutputTypeBrokenHash, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputGlobalIndexIsWrong(int input_0, int output_0)
{
	LES_FUNCTION_START(LES_Test_OutputGlobalIndexIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputUsedAsInput(int output_0)
{
	LES_FUNCTION_START(LES_Test_OutputUsedAsInput, void);
	LES_FUNCTION_ADD_INPUT(int, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputParamAlreadyUsed(int input_0, int output_0)
{
	LES_FUNCTION_START(LES_Test_OutputParamAlreadyUsed, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(int*, output_0);
	LES_FUNCTION_ADD_OUTPUT(int*, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputParamMissing(int input_0, int output_0)
{
	LES_FUNCTION_START(LES_Test_OutputParamMissing, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(int*, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputOutputMixture(unsigned int input_0, unsigned short output_0, unsigned char input_1, float output_1)
{
	LES_FUNCTION_START(LES_Test_InputOutputMixture, void);
	LES_FUNCTION_ADD_INPUT(unsigned int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned short*, output_0);
	LES_FUNCTION_ADD_INPUT(unsigned char, input_1);
	LES_FUNCTION_ADD_OUTPUT(float*, output_1);
	LES_FUNCTION_END();
}

static void LES_Test_InputParamUsedAsOutput(unsigned int input_0, unsigned short output_0)
{
	LES_FUNCTION_START(LES_Test_InputParamUsedAsOutput, void);
	LES_FUNCTION_ADD_INPUT(unsigned int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned short, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputParamUsedAsInput(unsigned int input_0, unsigned short output_0)
{
	LES_FUNCTION_START(LES_Test_OutputParamUsedAsInput, void);
	LES_FUNCTION_ADD_INPUT(output_only, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned short, output_0);
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
	LES_TEST_ADD_TYPE(unsigned char, LES_TYPE_INPUT);
	LES_TEST_ADD_TYPE(unsigned short, LES_TYPE_INPUT);
	LES_TEST_ADD_TYPE(unsigned int, LES_TYPE_INPUT);
	LES_TEST_ADD_TYPE(unsigned long, LES_TYPE_INPUT);

	LES_TEST_ADD_TYPE(char, LES_TYPE_INPUT);
	LES_TEST_ADD_TYPE(short, LES_TYPE_INPUT);
	LES_TEST_ADD_TYPE(int, LES_TYPE_INPUT);
	LES_TEST_ADD_TYPE(long, LES_TYPE_INPUT);

	LES_TEST_ADD_TYPE(float, LES_TYPE_INPUT);
	LES_TEST_ADD_TYPE(double, LES_TYPE_INPUT);

	LES_TEST_ADD_TYPE(unsigned char*, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE(unsigned short*, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE(unsigned int*, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE(unsigned long*, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_ADD_TYPE(char*, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE(short*, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE(int*, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE(long*, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_ADD_TYPE(float*, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE(double*, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_ADD_TYPE_EX(output_only, 4, LES_TYPE_OUTPUT);

	/* Sample functions for development */
	LES_TEST_FUNCTION_START(jakeInit, void, 2, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, a);
	LES_TEST_FUNCTION_ADD_INPUT(short, b);
	LES_TEST_FUNCTION_ADD_OUTPUT(float*, rPtr);
	LES_TEST_FUNCTION_END();

	/* Setup test data for specific tests */
	LES_TEST_FUNCTION_START(LES_Test_ReturnTypeNotFound, void, 0, 0);
	functionDefinition.m_returnTypeID = -1;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReturnTypeHashIsWrong, void, 0, 0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReturnTypeStringIsWrong, ReturnTypeBrokenHash, 0, 0);
	LES_StringEntry* const brokenHashEntry = (LES_StringEntry* const)LES_GetStringEntryForID(functionDefinition.m_returnTypeID);
	brokenHashEntry->m_str = "brokenString";
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

	LES_TEST_FUNCTION_START(LES_Test_InputNameIDNotFound, void, 1, 0);
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[0];
	LES_AddStringEntry("input_0");
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = -1;
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputNameDoesntExist, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputNameHashIsWrong, void, 1, 0);
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[0];
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputNameStringIsWrong, void, 1, 0);
	LES_TEST_FUNCTION_ADD_INPUT(int, InputNameBrokenHash);
	{
		LES_FunctionParameter* const inputFunctionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[0];
		LES_StringEntry* const brokenHashEntry = (LES_StringEntry* const)LES_GetStringEntryForID(inputFunctionParameterPtr->m_nameID);
		brokenHashEntry->m_str = "brokenString";
	}
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputTypeIDNotFound, void, 1, 0);
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[0];
	LES_AddStringEntry("int");
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("input_0");
	functionParameterPtr->m_typeID = -1;
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputTypeHashIsWrong, void, 1, 0);
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[0];
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("input_0");
	functionParameterPtr->m_typeID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputTypeStringIsWrong, void, 1, 0);
	LES_TEST_FUNCTION_ADD_INPUT(InputTypeBrokenHash, input_0);
	{
		LES_FunctionParameter* const inputFunctionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[0];
		LES_StringEntry* const brokenHashEntry = (LES_StringEntry* const)LES_GetStringEntryForID(inputFunctionParameterPtr->m_typeID);
		brokenHashEntry->m_str = "brokenString";
	}
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputGlobalIndexIsWrong, void, 1, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputUsedAsOutput, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputParamAlreadyUsed, void, 2, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_1);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputParamMissing, void, 3, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(int*, output_0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_2);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_TooManyOutputParameters, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int*, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int*, output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputWrongIndex, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputNameIDNotFound, void, 0, 1);
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_outputs[0];
	LES_AddStringEntry("output_0");
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = -1;
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputNameDoesntExist, void, 0, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputNameHashIsWrong, void, 0, 1);
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_outputs[0];
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputNameStringIsWrong, void, 0, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, OutputNameBrokenHash);
	{
		LES_FunctionParameter* const outputFunctionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_outputs[0];
		LES_StringEntry* const brokenHashEntry = (LES_StringEntry* const)LES_GetStringEntryForID(outputFunctionParameterPtr->m_nameID);
		brokenHashEntry->m_str = "brokenString";
	}
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputTypeIDNotFound, void, 0, 1);
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_outputs[0];
	LES_AddStringEntry("int");
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("output_0");
	functionParameterPtr->m_typeID = -1;
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputTypeHashIsWrong, void, 0, 1);
	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_outputs[0];
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("output_0");
	functionParameterPtr->m_typeID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputTypeStringIsWrong, void, 0, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(OutputTypeBrokenHash, output_0);
	{
		LES_FunctionParameter* const outputFunctionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_outputs[0];
		LES_StringEntry* const brokenHashEntry = (LES_StringEntry* const)LES_GetStringEntryForID(outputFunctionParameterPtr->m_typeID);
		brokenHashEntry->m_str = "brokenString";
	}

	LES_TEST_FUNCTION_END();
	LES_TEST_FUNCTION_START(LES_Test_OutputGlobalIndexIsWrong, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputUsedAsInput, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputParamAlreadyUsed, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int*, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int*, output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputParamMissing, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int*, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int*, output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputOutputMixture, void, 2, 2);
	LES_TEST_FUNCTION_ADD_INPUT(unsigned int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short*, output_0);
	LES_TEST_FUNCTION_ADD_INPUT(unsigned char, input_1);
	LES_TEST_FUNCTION_ADD_OUTPUT(float*, output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputParamUsedAsOutput, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(unsigned int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputParamUsedAsInput, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(output_only, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short, output_0);
	LES_TEST_FUNCTION_END();

	/* Run specific tests */
	/* Function header definition tests */
	fprintf(stderr, "#### Function header definition tests ####\n");
	LES_Test_FunctionNotFound();
	fprintf(stderr, "\n");
	LES_Test_ReturnTypeNotFound();
	fprintf(stderr, "\n");
	LES_Test_ReturnTypeHashIsWrong();
	fprintf(stderr, "\n");
	LES_Test_ReturnTypeStringIsWrong();
	fprintf(stderr, "\n");

	/* Input parameter tests */
	fprintf(stderr, "#### Input parameter tests ####\n");
	LES_Test_TooManyInputParameters(1, 2, 3, 4);
	fprintf(stderr, "\n");
	LES_Test_InputWrongIndex(1, 2, 3);
	fprintf(stderr, "\n");
	LES_Test_InputGlobalIndexIsWrong(1, 2);
	fprintf(stderr, "\n");
	LES_Test_InputUsedAsOutput(1);
	fprintf(stderr, "\n");
	LES_Test_InputParamAlreadyUsed(1, 2);
	fprintf(stderr, "\n");
	LES_Test_InputParamMissing(1, 2);
	fprintf(stderr, "\n");

	/* Input name tests */
	fprintf(stderr, "#### Input name tests ####\n");
	LES_Test_InputNameDoesntExist(1);
	fprintf(stderr, "\n");
	LES_Test_InputNameIDNotFound(1);
	fprintf(stderr, "\n");
	LES_Test_InputNameHashIsWrong(1);
	fprintf(stderr, "\n");
	LES_Test_InputNameStringIsWrong(1);
	fprintf(stderr, "\n");

	/* Input type tests */
	fprintf(stderr, "#### Input type tests ####\n");
	LES_Test_InputTypeIDNotFound(1);
	fprintf(stderr, "\n");
	LES_Test_InputTypeHashIsWrong(1);
	fprintf(stderr, "\n");
	LES_Test_InputTypeStringIsWrong(1);
	fprintf(stderr, "\n");

	/* Output parameter tests */
	fprintf(stderr, "#### Output parameter tests ####\n");
	LES_Test_TooManyOutputParameters(1, 2, 3, 4);
	fprintf(stderr, "\n");
	LES_Test_OutputWrongIndex(1, 2, 3);
	fprintf(stderr, "\n");
	LES_Test_OutputGlobalIndexIsWrong(1, 2);
	fprintf(stderr, "\n");
	LES_Test_OutputUsedAsInput(1);
	fprintf(stderr, "\n");
	LES_Test_OutputParamAlreadyUsed(1, 2);
	fprintf(stderr, "\n");
	LES_Test_OutputParamMissing(1, 2);
	fprintf(stderr, "\n");

	/* Output name tests */
	fprintf(stderr, "#### Output name tests ####\n");
	LES_Test_OutputNameDoesntExist(1);
	fprintf(stderr, "\n");
	LES_Test_OutputNameIDNotFound(1);
	fprintf(stderr, "\n");
	LES_Test_OutputNameHashIsWrong(1);
	fprintf(stderr, "\n");
	LES_Test_OutputNameStringIsWrong(1);
	fprintf(stderr, "\n");

	/* Output type tests */
	fprintf(stderr, "#### Output type tests ####\n");
	LES_Test_OutputTypeIDNotFound(1);
	fprintf(stderr, "\n");
	LES_Test_OutputTypeHashIsWrong(1);
	fprintf(stderr, "\n");
	LES_Test_OutputTypeStringIsWrong(1);
	fprintf(stderr, "\n");

	/* Parameter already exists tests */
	fprintf(stderr, "#### Parameter already exists tests ####\n");
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

	/* Complex Input Output Parameter tests */
	fprintf(stderr, "#### Complex Input Output Parameter tests ####\n");
	LES_Test_InputOutputMixture(1, 2, 3, 4);
	fprintf(stderr, "LES_Test_InputOutputMixture: success if no error output\n");
	fprintf(stderr, "\n");
	LES_Test_InputParamUsedAsOutput(1, 2);
	fprintf(stderr, "\n");
	LES_Test_OutputParamUsedAsInput(1, 2);
	fprintf(stderr, "\n");

	/* Add Type tests */
	fprintf(stderr, "#### Add Type tests ####\n");
	LES_TEST_ADD_TYPE_EX(unsigned char, 2, LES_TYPE_INPUT);
	LES_TEST_ADD_TYPE(unsigned char, LES_TYPE_OUTPUT);
	fprintf(stderr, "\n");
}

