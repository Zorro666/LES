#include <math.h>

#include "les_test.h"
#include "les_function.h"
#include "les_stringentry.h"

extern int LES_AddStringEntry(const char* const str);
extern int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const functionDefinitionPtr);
extern int LES_AddType(const char* const name, const unsigned int dataSize, const unsigned int flags);
extern const LES_TypeEntry* LES_GetTypeEntry(const LES_StringEntry* const typeStringEntry);

#define LES_TEST_FUNCTION_START(NAME, RETURN_TYPE, NUM_INPUTS, NUM_OUTPUTS) \
	{ \
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
	bool __LES_ok = true; \
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
		functionDefinition.SetParameterDataSize(parameterDataSize); \
		LES_AddFunctionDefinition(functionName, &functionDefinition); \
	} \


#define LES_TEST_ADD_TYPE_EX(TYPE, SIZE, FLAGS) \
	if (LES_AddType(#TYPE, SIZE, FLAGS) == LES_ERROR) \
	{\
		fprintf(stderr, "LES ERROR: TEST AddType '%s' 0x%X failed\n", #TYPE, LES_GenerateHashCaseSensitive(#TYPE)); \
	}\


#define LES_TEST_ADD_TYPE_POINTER(TYPE, FLAGS) \
	LES_TEST_ADD_TYPE_EX(TYPE*, sizeof(TYPE), FLAGS|LES_TYPE_POINTER) \


#define LES_TEST_ADD_TYPE_POD(TYPE) \
	LES_TEST_ADD_TYPE_EX(TYPE, sizeof(TYPE), LES_TYPE_INPUT|LES_TYPE_POD) \


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

static void LES_Test_InputParamMissing(int input_0, unsigned int* output_0)
{
	LES_FUNCTION_START(LES_Test_InputParamMissing, void);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
}

static void LES_Test_TooManyOutputParameters(int input_0, unsigned int* output_0, unsigned short* output_1, unsigned char* output_2)
{
	LES_FUNCTION_START(LES_Test_TooManyOutputParameters, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_FUNCTION_ADD_OUTPUT(unsigned char*, output_2);
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

static void LES_Test_OutputParamAlreadyUsed(int input_0, unsigned int* output_0)
{
	LES_FUNCTION_START(LES_Test_OutputParamAlreadyUsed, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_OutputParamMissing(int input_0, unsigned int* output_0)
{
	LES_FUNCTION_START(LES_Test_OutputParamMissing, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_END();
}

static void LES_Test_InputOutputMixture(unsigned short input_0, unsigned int* output_0, unsigned char input_1, unsigned short* output_1)
{
	LES_FUNCTION_START(LES_Test_InputOutputMixture, void);
	LES_FUNCTION_ADD_INPUT(unsigned short, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_ADD_INPUT(unsigned char, input_1);
	LES_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
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

static void LES_Test_ReadInputParameters(int input_0, short input_1, char input_2, float input_3, int* input_4)
{
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_ReadInputParameters, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_INPUT(short, input_1);
	LES_FUNCTION_ADD_INPUT(char, input_2);
	LES_FUNCTION_ADD_INPUT(float, input_3);
	LES_FUNCTION_ADD_INPUT(int*, input_4);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	if (parameterData == LES_NULL)
	{
		return;
	}
	const LES_StringEntry* typeEntry = LES_NULL;
	int errorCode = LES_OK;

	typeEntry = LES_GetStringEntry("int");
	int value_0;
	errorCode = parameterData->Read(typeEntry, (void*)&value_0);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputParameters: Read failed for input_0\n");
		return;
	}
	if (input_0 != value_0)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputParameters: parameter data is wrong input_0:%d value_0:%d\n", input_0, value_0);
	}

	typeEntry = LES_GetStringEntry("short");
	short value_1;
	errorCode = parameterData->Read(typeEntry, (void*)&value_1);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputParameters: Read failed for input_1\n");
		return;
	}
	if (input_1 != value_1)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputParameters: parameter data is wrong input_1:%d value_1:%d\n", input_1, value_1);
	}

	typeEntry = LES_GetStringEntry("char");
	char value_2;
	errorCode = parameterData->Read(typeEntry, (void*)&value_2);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputParameters: Read failed for input_2\n");
		return;
	}
	if (input_2 != value_2)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputParameters: parameter data is wrong input_2:%d value_2:%d\n", input_2, value_2);
	}

	typeEntry = LES_GetStringEntry("float");
	float value_3;
	errorCode = parameterData->Read(typeEntry, (void*)&value_3);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputParameters: Read failed for input_3\n");
		return;
	}
	if (fabsf(input_3-value_3) > 1.0e-6f)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputParameters: parameter data is wrong input_3:%f value_3:%f\n", input_3, value_3);
	}

	typeEntry = LES_GetStringEntry("int*");
	int value_4;
	errorCode = parameterData->Read(typeEntry, (void*)&value_4);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputParameters: Read failed for input_4\n");
		return;
	}
	if (*input_4 != value_4)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputParameters: parameter data is wrong input_4:%d value_4:%d\n", *input_4, value_4);
	}
	fprintf(stderr, "LES_Test_ReadInputParameters: input_0:%d value_0:%d\n", input_0, value_0);
	fprintf(stderr, "LES_Test_ReadInputParameters: input_1:%d value_1:%d\n", input_1, value_1);
	fprintf(stderr, "LES_Test_ReadInputParameters: input_2:%d value_2:%d\n", input_2, value_2);
	fprintf(stderr, "LES_Test_ReadInputParameters: input_3:%f value_3:%f\n", input_3, value_3);
	fprintf(stderr, "LES_Test_ReadInputParameters: input_4:%d value_4:%d\n", *input_4, value_4);
}

static void LES_Test_ReadOutputParameters(unsigned int* output_0, unsigned short* output_1, unsigned char* output_2, float* output_3)
{
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_ReadOutputParameters, void);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_FUNCTION_ADD_OUTPUT(unsigned char*, output_2);
	LES_FUNCTION_ADD_OUTPUT(float*, output_3);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	if (parameterData == LES_NULL)
	{
		return;
	}
	const LES_StringEntry* typeEntry = LES_NULL;
	int errorCode = LES_OK;

	typeEntry = LES_GetStringEntry("unsigned int*");
	unsigned int value_0;
	errorCode = parameterData->Read(typeEntry, (void*)&value_0);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadOutputParameters: Read failed for output_0\n");
		return;
	}
	if (*output_0 != value_0)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadOutputParameters: parameter data is wrong output_0:%d value_0:%d\n", *output_0, value_0);
	}

	typeEntry = LES_GetStringEntry("unsigned short*");
	unsigned short value_1;
	errorCode = parameterData->Read(typeEntry, (void*)&value_1);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadOutputParameters: Read failed for output_1\n");
		return;
	}
	if (*output_1 != value_1)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadOutputParameters: parameter data is wrong output_1:%d value_1:%d\n", *output_1, value_1);
	}

	typeEntry = LES_GetStringEntry("unsigned char*");
	unsigned char value_2;
	errorCode = parameterData->Read(typeEntry, (void*)&value_2);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadOutputParameters: Read failed for output_2\n");
		return;
	}
	if (*output_2 != value_2)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadOutputParameters: parameter data is wrong output_2:%d value_2:%d\n", *output_2, value_2);
	}

	typeEntry = LES_GetStringEntry("float*");
	float value_3;
	errorCode = parameterData->Read(typeEntry, (void*)&value_3);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadOutputParameters: Read failed for output_3\n");
		return;
	}
	if (fabsf(*output_3-value_3) > 1.0e-6f)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadOutputParameters: parameter data is wrong output_3:%f value_3:%f\n", *output_3, value_3);
	}

	fprintf(stderr, "LES_Test_ReadOutputParameters: output_0:%d value_0:%d\n", *output_0, value_0);
	fprintf(stderr, "LES_Test_ReadOutputParameters: output_1:%d value_1:%d\n", *output_1, value_1);
	fprintf(stderr, "LES_Test_ReadOutputParameters: output_2:%d value_2:%d\n", *output_2, value_2);
	fprintf(stderr, "LES_Test_ReadOutputParameters: output_3:%f value_3:%f\n", *output_3, value_3);
}

static void LES_Test_ReadInputOutputParameters(int input_0, short input_1, char input_2, float input_3, int* input_4,
																							 unsigned int* output_0, unsigned short* output_1, unsigned char* output_2, float* output_3)
{
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_ReadInputOutputParameters, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_ADD_INPUT(short, input_1);
	LES_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_FUNCTION_ADD_INPUT(char, input_2);
	LES_FUNCTION_ADD_OUTPUT(unsigned char*, output_2);
	LES_FUNCTION_ADD_INPUT(float, input_3);
	LES_FUNCTION_ADD_OUTPUT(float*, output_3);
	LES_FUNCTION_ADD_INPUT(int*, input_4);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	if (parameterData == LES_NULL)
	{
		return;
	}

	const LES_StringEntry* typeEntry = LES_NULL;
	int errorCode = LES_OK;

	typeEntry = LES_GetStringEntry("int");
	int input_value_0;
	errorCode = parameterData->Read(typeEntry, (void*)&input_value_0);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: Read failed for input_0\n");
		return;
	}
	if (input_0 != input_value_0)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: parameter data is wrong input_0:%d value_0:%d\n",
						input_0, input_value_0);
	}

	typeEntry = LES_GetStringEntry("unsigned int*");
	unsigned int output_value_0;
	errorCode = parameterData->Read(typeEntry, (void*)&output_value_0);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: Read failed for output_0\n");
		return;
	}
	if (*output_0 != output_value_0)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: parameter data is wrong output_0:%d value_0:%d\n",
						*output_0, output_value_0);
	}

	typeEntry = LES_GetStringEntry("short");
	short input_value_1;
	errorCode = parameterData->Read(typeEntry, (void*)&input_value_1);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: Read failed for input_1\n");
		return;
	}
	if (input_1 != input_value_1)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: parameter data is wrong input_1:%d value_1:%d\n",
						input_1, input_value_1);
	}

	typeEntry = LES_GetStringEntry("unsigned short*");
	unsigned short output_value_1;
	errorCode = parameterData->Read(typeEntry, (void*)&output_value_1);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: Read failed for output_1\n");
		return;
	}
	if (*output_1 != output_value_1)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: parameter data is wrong output_1:%d value_1:%d\n",
						*output_1, output_value_1);
	}

	typeEntry = LES_GetStringEntry("char");
	char input_value_2;
	errorCode = parameterData->Read(typeEntry, (void*)&input_value_2);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: Read failed for input_2\n");
		return;
	}
	if (input_2 != input_value_2)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: parameter data is wrong input_2:%d value_2:%d\n",
						input_2, input_value_2);
	}

	typeEntry = LES_GetStringEntry("unsigned char*");
	unsigned char output_value_2;
	errorCode = parameterData->Read(typeEntry, (void*)&output_value_2);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: Read failed for output_2\n");
		return;
	}
	if (*output_2 != output_value_2)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: parameter data is wrong output_2:%d value_2:%d\n",
						*output_2, output_value_2);
	}

	typeEntry = LES_GetStringEntry("float");
	float input_value_3;
	errorCode = parameterData->Read(typeEntry, (void*)&input_value_3);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: Read failed for input_3\n");
		return;
	}
	if (fabsf(input_3-input_value_3) > 1.0e-6f)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: parameter data is wrong input_3:%f value_3:%f\n",
						input_3, input_value_3);
	}

	typeEntry = LES_GetStringEntry("float*");
	float output_value_3;
	errorCode = parameterData->Read(typeEntry, (void*)&output_value_3);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: Read failed for output_3\n");
		return;
	}
	if (fabsf(*output_3-output_value_3) > 1.0e-6f)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: parameter data is wrong output_3:%f value_3:%f\n",
						*output_3, output_value_3);
	}

	typeEntry = LES_GetStringEntry("int*");
	int input_value_4;
	errorCode = parameterData->Read(typeEntry, (void*)&input_value_4);
	if (errorCode == LES_ERROR)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: Read failed for input_4\n");
		return;
	}
	if (*input_4 != input_value_4)
	{
		fprintf(stderr, "LES ERROR: LES_Test_ReadInputOutputParameters: parameter data is wrong input_4:%d value_4:%d\n",
						*input_4, input_value_4);
	}

	fprintf(stderr, "LES_Test_ReadInputOutputParameters: input_0:%d value_0:%d\n", input_0, input_value_0);
	fprintf(stderr, "LES_Test_ReadInputOutputParameters: output_0:%d value_0:%d\n", *output_0, output_value_0);
	fprintf(stderr, "LES_Test_ReadInputOutputParameters: input_1:%d value_1:%d\n", input_1, input_value_1);
	fprintf(stderr, "LES_Test_ReadInputOutputParameters: output_1:%d value_1:%d\n", *output_1, output_value_1);
	fprintf(stderr, "LES_Test_ReadInputOutputParameters: input_2:%d value_2:%d\n", input_2, input_value_2);
	fprintf(stderr, "LES_Test_ReadInputOutputParameters: output_2:%d value_2:%d\n", *output_2, output_value_2);
	fprintf(stderr, "LES_Test_ReadInputOutputParameters: input_3:%f value_3:%f\n", input_3, input_value_3);
	fprintf(stderr, "LES_Test_ReadInputOutputParameters: output_3:%f value_3:%f\n", *output_3, output_value_3);
	fprintf(stderr, "LES_Test_ReadInputOutputParameters: input_4:%d value_4:%d\n", *input_4, input_value_4);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// External functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LES_TestSetup(void)
{
	/* Sample types for development */
	LES_TEST_ADD_TYPE_POD(unsigned char);
	LES_TEST_ADD_TYPE_POD(unsigned short);
	LES_TEST_ADD_TYPE_POD(unsigned int);
	LES_TEST_ADD_TYPE_POD(unsigned long);

	LES_TEST_ADD_TYPE_POD(char);
	LES_TEST_ADD_TYPE_POD(short);
	LES_TEST_ADD_TYPE_POD(int);
	LES_TEST_ADD_TYPE_POD(long);

	LES_TEST_ADD_TYPE_POD(float);
	LES_TEST_ADD_TYPE_POD(double);

	LES_TEST_ADD_TYPE_POINTER(unsigned char, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POINTER(unsigned short, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POINTER(unsigned int, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POINTER(unsigned long, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_ADD_TYPE_POINTER(char, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POINTER(short, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POINTER(int, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POINTER(long, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_ADD_TYPE_POINTER(float, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POINTER(double, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_ADD_TYPE_EX(output_only, 4, LES_TYPE_OUTPUT|LES_TYPE_POD);

	/* Sample functions for development */
	LES_TEST_FUNCTION_START(jakeInit, void, 2, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, a);
	LES_TEST_FUNCTION_ADD_INPUT(short, b);
	LES_TEST_FUNCTION_ADD_OUTPUT(float*, rPtr);
	LES_TEST_FUNCTION_END();

	/* Setup test data for specific tests */
	LES_TEST_FUNCTION_START(LES_Test_ReturnTypeNotFound, void, 0, 0);
	functionDefinition.SetReturnTypeID(-1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReturnTypeHashIsWrong, void, 0, 0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReturnTypeStringIsWrong, ReturnTypeBrokenHash, 0, 0);
	LES_StringEntry* const brokenHashEntry = (LES_StringEntry* const)LES_GetStringEntryForID(functionDefinition.GetReturnTypeID());
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
	functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
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
	functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputNameStringIsWrong, void, 1, 0);
	LES_TEST_FUNCTION_ADD_INPUT(int, InputNameBrokenHash);
	{
		LES_FunctionParameter* const inputFunctionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
		LES_StringEntry* const brokenHashEntry = (LES_StringEntry* const)LES_GetStringEntryForID(inputFunctionParameterPtr->m_nameID);
		brokenHashEntry->m_str = "brokenString";
	}
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputTypeIDNotFound, void, 1, 0);
	functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	LES_AddStringEntry("int");
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("input_0");
	functionParameterPtr->m_typeID = -1;
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputTypeHashIsWrong, void, 1, 0);
	functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("input_0");
	functionParameterPtr->m_typeID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputTypeStringIsWrong, void, 1, 0);
	LES_TEST_FUNCTION_ADD_INPUT(InputTypeBrokenHash, input_0);
	{
		LES_FunctionParameter* const inputFunctionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
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
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_2);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_TooManyOutputParameters, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputWrongIndex, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputNameIDNotFound, void, 0, 1);
	functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
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
	functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputNameStringIsWrong, void, 0, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, OutputNameBrokenHash);
	{
		LES_FunctionParameter* const outputFunctionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
		LES_StringEntry* const brokenHashEntry = (LES_StringEntry* const)LES_GetStringEntryForID(outputFunctionParameterPtr->m_nameID);
		brokenHashEntry->m_str = "brokenString";
	}
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputTypeIDNotFound, void, 0, 1);
	functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	LES_AddStringEntry("int");
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("output_0");
	functionParameterPtr->m_typeID = -1;
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputTypeHashIsWrong, void, 0, 1);
	functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("output_0");
	functionParameterPtr->m_typeID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputTypeStringIsWrong, void, 0, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(OutputTypeBrokenHash, output_0);
	{
		LES_FunctionParameter* const outputFunctionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
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
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputParamMissing, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputOutputMixture, void, 2, 2);
	LES_TEST_FUNCTION_ADD_INPUT(unsigned short, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_TEST_FUNCTION_ADD_INPUT(unsigned char, input_1);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputParamUsedAsOutput, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(unsigned int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputParamUsedAsInput, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(output_only, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReadInputParameters, void, 5, 0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_INPUT(short, input_1);
	LES_TEST_FUNCTION_ADD_INPUT(char, input_2);
	LES_TEST_FUNCTION_ADD_INPUT(float, input_3);
	LES_TEST_FUNCTION_ADD_INPUT(int*, input_4);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReadOutputParameters, void, 0, 4);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned char*, output_2);
	LES_TEST_FUNCTION_ADD_OUTPUT(float*, output_3);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReadInputOutputParameters, void, 5, 4);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_TEST_FUNCTION_ADD_INPUT(short, input_1);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_TEST_FUNCTION_ADD_INPUT(char, input_2);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned char*, output_2);
	LES_TEST_FUNCTION_ADD_INPUT(float, input_3);
	LES_TEST_FUNCTION_ADD_OUTPUT(float*, output_3);
	LES_TEST_FUNCTION_ADD_INPUT(int*, input_4);
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

	unsigned int output_0 = 1;
	unsigned short output_1 = 2;
	unsigned char output_2 = 3;
	float output_3 = -123.456f;
	int input_4 = 12345;

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
	LES_Test_InputParamMissing(1, &output_0);
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
	LES_Test_TooManyOutputParameters(1, &output_0, &output_1, &output_2);
	fprintf(stderr, "\n");
	LES_Test_OutputWrongIndex(1, 2, 3);
	fprintf(stderr, "\n");
	LES_Test_OutputGlobalIndexIsWrong(1, 2);
	fprintf(stderr, "\n");
	LES_Test_OutputUsedAsInput(1);
	fprintf(stderr, "\n");
	LES_Test_OutputParamAlreadyUsed(1, &output_0);
	fprintf(stderr, "\n");
	LES_Test_OutputParamMissing(1, &output_0);
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
	LES_Test_InputOutputMixture(1, &output_0, 3, &output_1);
	fprintf(stderr, "LES_Test_InputOutputMixture: success if no error output\n");
	fprintf(stderr, "\n");
	LES_Test_InputParamUsedAsOutput(1, 2);
	fprintf(stderr, "\n");
	LES_Test_OutputParamUsedAsInput(1, 2);
	fprintf(stderr, "\n");

	/* Add Type tests */
	fprintf(stderr, "#### Add Type tests ####\n");
	LES_TEST_ADD_TYPE_EX(unsigned char, 2, LES_TYPE_POD|LES_TYPE_INPUT);
	fprintf(stderr, "\n");
	LES_TEST_ADD_TYPE_EX(unsigned char, 1, LES_TYPE_POINTER|LES_TYPE_OUTPUT);
	fprintf(stderr, "\n");

	/* Read Parameter tests */
	fprintf(stderr, "#### Read Parameter tests ####\n");
	LES_Test_ReadInputParameters(102, 23453, 110, -4.0332f, &input_4);
	fprintf(stderr, "\n");
	LES_Test_ReadOutputParameters(&output_0, &output_1, &output_2, &output_3);
	fprintf(stderr, "\n");
	LES_Test_ReadInputOutputParameters(102, 23453, 110, -4.0332f, &input_4, &output_0, &output_1, &output_2, &output_3);
	fprintf(stderr, "\n");
}

