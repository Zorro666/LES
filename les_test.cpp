#include <math.h>
#include <string.h>

#include "les_test.h"
#include "les_core.h"
#include "les_logger.h"
#include "les_test_macros.h"
#include "les_function.h"
#include "les_function_macros.h"
#include "les_stringentry.h"
#include "les_parameter.h"
#include "les_struct.h"

#define LES_TEST_DEBUG 0

extern int LES_AddStringEntry(const char* const str);
extern int LES_AddType(const char* const name, const unsigned int dataSize, const unsigned int flags, 
											 const char* const aliasedName, const int numElements);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal test functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void LES_Test_FunctionNotFound(void)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_FunctionNotFound, void);
	LES_FUNCTION_END();
	LES_WARNING("Function not found: '%s'\n", "LES_Test_functionNotFound");
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_ReturnTypeNotFound(void)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_ReturnTypeNotFound, jake);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_ReturnTypeHashIsWrong()
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_ReturnTypeHashIsWrong, wrongReturnType);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_ReturnTypeStringIsWrong()
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_ReturnTypeStringIsWrong, ReturnTypeBrokenHash);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_TooManyInputParameters(int input_0, int input_1, int input_2, int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_TooManyInputParameters, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_INPUT(int, input_1);
	LES_FUNCTION_ADD_INPUT(int, input_2);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputWrongIndex(int input_0, int input_1, int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputWrongIndex, void);
	LES_FUNCTION_ADD_INPUT(int, input_1);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputNameDoesntExist(int inputNotExist)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputNameDoesntExist, void);
	LES_FUNCTION_ADD_INPUT(int, inputNotExist);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputNameIDNotFound(int input_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputNameIDNotFound, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputNameHashIsWrong(int input_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputNameHashIsWrong, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputNameStringIsWrong(int InputNameBrokenHash)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputNameStringIsWrong, void);
	LES_FUNCTION_ADD_INPUT(int, InputNameBrokenHash);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputTypeIDNotFound(int input_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputTypeIDNotFound, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputTypeHashIsWrong(int input_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputTypeHashIsWrong, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputTypeStringIsWrong(int input_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputTypeStringIsWrong, void);
	LES_FUNCTION_ADD_INPUT(InputTypeBrokenHash, input_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputGlobalIndexIsWrong(int input_0, int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputGlobalIndexIsWrong, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputUsedAsOutput(int input_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputUsedAsOutput, void);
	LES_FUNCTION_ADD_OUTPUT(int, input_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputParamAlreadyUsed(int input_0, int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputParamAlreadyUsed, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT_EX(2, int, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_InputParamMissing(int input_0, unsigned int* output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputParamMissing, void);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_TooManyOutputParameters(int input_0, unsigned int* output_0, unsigned short* output_1, unsigned char* output_2)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_TooManyOutputParameters, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_FUNCTION_ADD_OUTPUT(unsigned char*, output_2);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputWrongIndex(int input_0, int output_0, int output_1)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputWrongIndex, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT_EX(1, int, output_1);
	LES_FUNCTION_ADD_OUTPUT_EX(2, int, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputNameDoesntExist(int outputNotExist)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputNameDoesntExist, void);
	LES_FUNCTION_ADD_OUTPUT(int, outputNotExist);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputNameIDNotFound(int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputNameIDNotFound, void);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputNameHashIsWrong(int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputNameHashIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputNameStringIsWrong(int OutputNameBrokenHash)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputNameStringIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(int, OutputNameBrokenHash);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputTypeIDNotFound(int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputTypeIDNotFound, void);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputTypeHashIsWrong(int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputTypeHashIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputTypeStringIsWrong(int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputTypeStringIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(OutputTypeBrokenHash, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputGlobalIndexIsWrong(int input_0, int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputGlobalIndexIsWrong, void);
	LES_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputUsedAsInput(int output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputUsedAsInput, void);
	LES_FUNCTION_ADD_INPUT(int, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputParamAlreadyUsed(int input_0, unsigned int* output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputParamAlreadyUsed, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputParamMissing(int input_0, unsigned int* output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputParamMissing, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
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
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_InputParamUsedAsOutput, void);
	LES_FUNCTION_ADD_INPUT(unsigned int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned short, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_OutputParamUsedAsInput(unsigned int input_0, unsigned short output_0)
{
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_FUNCTION_START(LES_Test_OutputParamUsedAsInput, void);
	LES_FUNCTION_ADD_INPUT(output_only, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned short, output_0);
	LES_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
}

static void LES_Test_ReadInputParameters(int input_0, short input_1, char input_2, float input_3, int* input_4)
{
	const char* const testFuncName = "LES_Test_ReadInputParameters";
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
		LES_FATAL_ERROR("%s: parameterData is NULL\n", testFuncName);
		return;
	}
	const LES_StringEntry* typeEntry = LES_NULL;
	int errorCode = LES_RETURN_OK;

	typeEntry = LES_GetStringEntry("int");
	int value_0;
	errorCode = parameterData->Read(typeEntry, (void*)&value_0);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for input_0\n", testFuncName);
		return;
	}
	if (input_0 != value_0)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong input_0:%d value_0:%d\n", testFuncName, input_0, value_0);
	}

	typeEntry = LES_GetStringEntry("short");
	short value_1;
	errorCode = parameterData->Read(typeEntry, (void*)&value_1);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for input_1\n", testFuncName);
		return;
	}
	if (input_1 != value_1)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong input_1:%d value_1:%d\n", testFuncName, input_1, value_1);
	}

	typeEntry = LES_GetStringEntry("char");
	char value_2;
	errorCode = parameterData->Read(typeEntry, (void*)&value_2);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for input_2\n", testFuncName);
		return;
	}
	if (input_2 != value_2)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong input_2:%d value_2:%d\n", testFuncName, input_2, value_2);
	}

	typeEntry = LES_GetStringEntry("float");
	float value_3;
	errorCode = parameterData->Read(typeEntry, (void*)&value_3);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for input_3\n", testFuncName);
		return;
	}
	if (fabsf(input_3-value_3) > 1.0e-6f)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong input_3:%f value_3:%f\n", testFuncName, input_3, value_3);
	}

	typeEntry = LES_GetStringEntry("int*");
	int value_4;
	errorCode = parameterData->Read(typeEntry, (void*)&value_4);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for input_4\n", testFuncName);
		return;
	}
	if (*input_4 != value_4)
	{
		LES_FATAL_ERROR("LES_Test_ReadInputParameters: parameter data is wrong input_4:%d value_4:%d\n", testFuncName, *input_4, value_4);
	}
	LES_LOG("%s: input_0:%d value_0:%d\n", testFuncName, input_0, value_0);
	LES_LOG("%s: input_1:%d value_1:%d\n", testFuncName, input_1, value_1);
	LES_LOG("%s: input_2:%d value_2:%d\n", testFuncName, input_2, value_2);
	LES_LOG("%s: input_3:%f value_3:%f\n", testFuncName, input_3, value_3);
	LES_LOG("%s: input_4:%d value_4:%d\n", testFuncName, *input_4, value_4);
}

static void LES_Test_ReadOutputParameters(unsigned int* output_0, unsigned short* output_1, unsigned char* output_2, float* output_3)
{
	const char* const testFuncName = "LES_Test_ReadOutputParameters";
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
		LES_FATAL_ERROR("%s: parameterData is NULL\n", testFuncName);
		return;
	}
	const LES_StringEntry* typeEntry = LES_NULL;
	int errorCode = LES_RETURN_OK;

	typeEntry = LES_GetStringEntry("unsigned int*");
	unsigned int value_0;
	errorCode = parameterData->Read(typeEntry, (void*)&value_0);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for output_0\n", testFuncName);
		return;
	}
	if (*output_0 != value_0)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong output_0:%d value_0:%d\n", testFuncName, *output_0, value_0);
		return;
	}

	typeEntry = LES_GetStringEntry("unsigned short*");
	unsigned short value_1;
	errorCode = parameterData->Read(typeEntry, (void*)&value_1);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for output_1\n", testFuncName);
		return;
	}
	if (*output_1 != value_1)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong output_1:%d value_1:%d\n", testFuncName, *output_1, value_1);
		return;
	}

	typeEntry = LES_GetStringEntry("unsigned char*");
	unsigned char value_2;
	errorCode = parameterData->Read(typeEntry, (void*)&value_2);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for output_2\n", testFuncName);
		return;
	}
	if (*output_2 != value_2)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong output_2:%c value_2:%c\n", testFuncName, *output_2, value_2);
		return;
	}

	typeEntry = LES_GetStringEntry("float*");
	float value_3;
	errorCode = parameterData->Read(typeEntry, (void*)&value_3);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for output_3\n", testFuncName);
		return;
	}
	if (fabsf(*output_3-value_3) > 1.0e-6f)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong output_3:%f value_3:%f\n", testFuncName, *output_3, value_3);
		return;
	}

	LES_LOG("%s: output_0:%d value_0:%d\n", testFuncName, *output_0, value_0);
	LES_LOG("%s: output_1:%d value_1:%d\n", testFuncName, *output_1, value_1);
	LES_LOG("%s: output_2:%d value_2:%d\n", testFuncName, *output_2, value_2);
	LES_LOG("%s: output_3:%f value_3:%f\n", testFuncName, *output_3, value_3);
}

static void LES_Test_ReadInputOutputParameters(int input_0, short input_1, char input_2, float input_3, int* input_4,
																							 unsigned int* output_0, unsigned short* output_1, unsigned char* output_2, float* output_3)
{
	const char* const testFuncName = "LES_Test_ReadInputOutputParameters";
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
		LES_FATAL_ERROR("%s: parameterData is NULL\n", testFuncName);
		return;
	}

	const LES_StringEntry* typeEntry = LES_NULL;
	int errorCode = LES_RETURN_OK;

	typeEntry = LES_GetStringEntry("int");
	int input_value_0;
	errorCode = parameterData->Read(typeEntry, (void*)&input_value_0);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for input_0\n", testFuncName);
		return;
	}
	if (input_0 != input_value_0)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong input_0:%d value_0:%d\n", testFuncName, input_0, input_value_0);
		return;
	}

	typeEntry = LES_GetStringEntry("unsigned int*");
	unsigned int output_value_0;
	errorCode = parameterData->Read(typeEntry, (void*)&output_value_0);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for output_0\n", testFuncName);
		return;
	}
	if (*output_0 != output_value_0)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong output_0:%d value_0:%d\n", testFuncName, *output_0, output_value_0);
		return;
	}

	typeEntry = LES_GetStringEntry("short");
	short input_value_1;
	errorCode = parameterData->Read(typeEntry, (void*)&input_value_1);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for input_1\n", testFuncName);
		return;
	}
	if (input_1 != input_value_1)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong input_1:%d value_1:%d\n", testFuncName, input_1, input_value_1);
		return;
	}

	typeEntry = LES_GetStringEntry("unsigned short*");
	unsigned short output_value_1;
	errorCode = parameterData->Read(typeEntry, (void*)&output_value_1);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for output_1\n", testFuncName);
		return;
	}
	if (*output_1 != output_value_1)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong output_1:%d value_1:%d\n", testFuncName, *output_1, output_value_1);
		return;
	}

	typeEntry = LES_GetStringEntry("char");
	char input_value_2;
	errorCode = parameterData->Read(typeEntry, (void*)&input_value_2);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for input_2\n", testFuncName);
		return;
	}
	if (input_2 != input_value_2)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong input_2:%d value_2:%d\n", testFuncName, input_2, input_value_2);
		return;
	}

	typeEntry = LES_GetStringEntry("unsigned char*");
	unsigned char output_value_2;
	errorCode = parameterData->Read(typeEntry, (void*)&output_value_2);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for output_2\n", testFuncName);
		return;
	}
	if (*output_2 != output_value_2)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong output_2:%d value_2:%d\n", testFuncName, *output_2, output_value_2);
		return;
	}

	typeEntry = LES_GetStringEntry("float");
	float input_value_3;
	errorCode = parameterData->Read(typeEntry, (void*)&input_value_3);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for input_3\n", testFuncName);
		return;
	}
	if (fabsf(input_3-input_value_3) > 1.0e-6f)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong input_3:%f value_3:%f\n", testFuncName, input_3, input_value_3);
		return;
	}

	typeEntry = LES_GetStringEntry("float*");
	float output_value_3;
	errorCode = parameterData->Read(typeEntry, (void*)&output_value_3);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for output_3\n", testFuncName);
		return;
	}
	if (fabsf(*output_3-output_value_3) > 1.0e-6f)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong output_3:%f value_3:%f\n", testFuncName, *output_3, output_value_3);
		return;
	}

	typeEntry = LES_GetStringEntry("int*");
	int input_value_4;
	errorCode = parameterData->Read(typeEntry, (void*)&input_value_4);
	if (errorCode == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Read failed for input_4\n", testFuncName);
		return;
	}
	if (*input_4 != input_value_4)
	{
		LES_FATAL_ERROR("%s: parameter data is wrong input_4:%d value_4:%d\n", testFuncName, *input_4, input_value_4);
		return;
	}

	const LES_FunctionDefinition* const functionDefinitionPtr = LES_GetFunctionDefinition(testFuncName);
	if (functionDefinitionPtr == LES_NULL)
	{
		LES_FATAL_ERROR("%s: can't find the function definition\n", testFuncName);
		return;
	}
	const int parameterDataSize = functionDefinitionPtr->GetParameterDataSize();
	const int realParameterDataSize = sizeof(int)+sizeof(unsigned int)+
																		sizeof(short)+sizeof(unsigned short)+
																		sizeof(char)+sizeof(unsigned char)+
																		sizeof(float)+sizeof(float)+
																		sizeof(int);
	if (parameterDataSize != realParameterDataSize)
	{
		LES_FATAL_ERROR("%s: parameterDataSize is wrong Code:%d Should be:%d\n", testFuncName, parameterDataSize, realParameterDataSize);
		return;
	}

	LES_LOG("%s: input_0:%d value_0:%d\n", testFuncName, input_0, input_value_0, testFuncName);
	LES_LOG("%s: output_0:%d value_0:%d\n", testFuncName, *output_0, output_value_0, testFuncName);
	LES_LOG("%s: input_1:%d value_1:%d\n", testFuncName, input_1, input_value_1, testFuncName);
	LES_LOG("%s: output_1:%d value_1:%d\n", testFuncName, *output_1, output_value_1, testFuncName);
	LES_LOG("%s: input_2:'%c' value_2:'%c'\n", testFuncName, input_2, input_value_2, testFuncName);
	LES_LOG("%s: output_2:'%c' value_2:'%c'\n", testFuncName, *output_2, output_value_2, testFuncName);
	LES_LOG("%s: input_3:%f value_3:%f\n", testFuncName, input_3, input_value_3, testFuncName);
	LES_LOG("%s: output_3:%f value_3:%f\n", testFuncName, *output_3, output_value_3, testFuncName);
	LES_LOG("%s: input_4:%d value_4:%d\n", testFuncName, *input_4, input_value_4, testFuncName);
	LES_LOG("%s: parameterDataSize:%d\n", testFuncName, parameterDataSize, testFuncName);
}

static int LES_Test_GenericDecodeHelper(const char* const testFuncName, LES_FunctionParameterData* parameterData, 
																				const int realParameterDataSize)
{
	if (parameterData == LES_NULL)
	{
		LES_FATAL_ERROR("%s: parameterData is NULL\n", testFuncName);
		return LES_RETURN_ERROR;
	}

	const LES_FunctionDefinition* const functionDefinitionPtr = LES_GetFunctionDefinition(testFuncName);
	if (functionDefinitionPtr == LES_NULL)
	{
		LES_FATAL_ERROR("%s: can't find the function definition\n", testFuncName);
		return LES_RETURN_ERROR;
	}
	const int parameterDataSize = functionDefinitionPtr->GetParameterDataSize();
	if (parameterDataSize != realParameterDataSize)
	{
		LES_FATAL_ERROR("%s: parameterDataSize is wrong Code:%d Should be:%d\n", testFuncName, parameterDataSize, realParameterDataSize);
		return LES_RETURN_ERROR;
	}

	if (functionDefinitionPtr->Decode(parameterData) == LES_RETURN_ERROR)
	{
		LES_FATAL_ERROR("%s: Decode failed\n", testFuncName);
		return LES_RETURN_ERROR;
	}
	LES_LOG("%s: parameterDataSize:%d\n", testFuncName, parameterDataSize);

	return LES_RETURN_OK;
}

static void LES_Test_DecodeInputOutputParameters(int input_0, short input_1, char input_2, float input_3, 
																								 int* input_4, short* input_5, char* input_6,
																							   unsigned int* output_0, unsigned short* output_1, unsigned char* output_2, float* output_3)
{
	const char* const testFuncName = "LES_Test_DecodeInputOutputParameters";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeInputOutputParameters, void);
	LES_FUNCTION_ADD_INPUT(int, input_0);
	LES_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_FUNCTION_ADD_INPUT(short, input_1);
	LES_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_FUNCTION_ADD_INPUT(char, input_2);
	LES_FUNCTION_ADD_OUTPUT(unsigned char*, output_2);
	LES_FUNCTION_ADD_INPUT(float, input_3);
	LES_FUNCTION_ADD_OUTPUT(float*, output_3);
	LES_FUNCTION_ADD_INPUT(int*, input_4);
	LES_FUNCTION_ADD_INPUT(short*, input_5);
	LES_FUNCTION_ADD_INPUT(char*, input_6);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(int)+sizeof(unsigned int)+
																		sizeof(short)+sizeof(unsigned short)+
																		sizeof(char)+sizeof(unsigned char)+
																		sizeof(float)+sizeof(float)+
																		sizeof(int)+sizeof(short)+sizeof(char);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

static void LES_Test_DecodeOutputParameters(char& output_0)
{
	const char* const testFuncName = "LES_Test_DecodeOutputParameters";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeOutputParameters, void);
	LES_FUNCTION_ADD_OUTPUT(char&, output_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(char);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

struct TestStruct1
{
	long long int m_longlong;
	char m_char;
	int m_int;
	short m_short;
	float m_float;
};

struct TestStruct2
{
	float m_float;
	TestStruct1 m_testStruct1;
	char m_char;
	short m_short;
	int m_int;
};

struct TestStruct3
{
	short m_short;
	float m_float;
	int m_int;
	char m_char;
};

struct TestStruct4
{
	float m_float;
	int m_int;
	char m_char;
	short m_short;
	short m_short2;
	TestStruct3 m_testStruct3;
};

struct TestStruct5
{
	char m_char;
	short* m_shortPtr;
};

struct TestStruct6
{
	short m_short;
	TestStruct5* m_testStruct5Ptr;
	char* m_charPtr;
};

struct TestStruct7
{
	short m_short;
	float& m_floatRef;
	char m_char;
	char& m_charRef;
	char* m_charPtr;
};

struct TestStruct8
{
	short m_short;
	TestStruct5& m_testStruct5Ref;
	char* m_charPtr;
};

struct TestStruct9
{
	short m_short[3];
};

struct TestStruct10
{
	short m_short[3];
	TestStruct9 m_testStruct9[2];
};

struct TestStruct11
{
	short (&m_short)[3];
	int (&m_int)[2];
	char (&m_char)[5];
};

struct TestStruct12
{
	short (&m_short)[3];
	TestStruct9 (&m_testStruct9)[2];
};

void LES_Test_StructInputParam(TestStruct2 input_0, int input_1, TestStruct1 input_2, TestStruct3 input_3, TestStruct4* input_4)
{
	const char* const testFuncName = "LES_Test_StructInputParam";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_StructInputParam, void);
	LES_FUNCTION_ADD_INPUT(TestStruct2, input_0);
	LES_FUNCTION_ADD_INPUT(int, input_1);
	LES_FUNCTION_ADD_INPUT(TestStruct1, input_2);
	LES_FUNCTION_ADD_INPUT(TestStruct3, input_3);
	LES_FUNCTION_ADD_INPUT(TestStruct4*, input_4);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(float)+
																		sizeof(long long int)+sizeof(char)+sizeof(int)+sizeof(short)+sizeof(float)+
																		sizeof(char)+sizeof(short)+sizeof(int)+
																		sizeof(int)+
																		sizeof(long long int)+sizeof(char)+sizeof(int)+sizeof(short)+sizeof(float)+
																		sizeof(short)+sizeof(float)+sizeof(int)+sizeof(char)+
																		sizeof(float)+sizeof(int)+sizeof(char)+sizeof(short)+sizeof(short)+
																		sizeof(short)+sizeof(float)+sizeof(int)+sizeof(char);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_StructOutputParam(TestStruct3* out_0, TestStruct4* out_1)
{
	const char* const testFuncName = "LES_Test_StructOutputParam";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_StructOutputParam, void);
	LES_FUNCTION_ADD_OUTPUT(TestStruct3*, out_0);
	LES_FUNCTION_ADD_OUTPUT(TestStruct4*, out_1);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(short)+sizeof(float)+sizeof(int)+sizeof(char)+
																		sizeof(float)+sizeof(int)+sizeof(char)+sizeof(short)+sizeof(short)+
																		sizeof(short)+sizeof(float)+sizeof(int)+sizeof(char);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_StructInputOutputParam(TestStruct5* in_0, TestStruct6* out_0)
{
	const char* const testFuncName = "LES_Test_StructInputOutputParam";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_StructInputOutputParam, void);
	LES_FUNCTION_ADD_INPUT(TestStruct5*, in_0);
	LES_FUNCTION_ADD_OUTPUT(TestStruct6*, out_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(char)+sizeof(short)+
																		sizeof(short)+
																		sizeof(char)+sizeof(short)+
																		sizeof(char);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_ReferenceInputPODParam(int& input_0)
{
	const char* const testFuncName = "LES_Test_ReferenceInputPODParam";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_ReferenceInputPODParam, void);
	LES_FUNCTION_ADD_INPUT(int&, input_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	if (parameterData == LES_NULL)
	{
		LES_FATAL_ERROR("%s: parameterData is NULL\n", testFuncName);
		return;
	}

	const int realParameterDataSize = sizeof(int);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_ReferenceInputStructParam(TestStruct1& input_0)
{
	const char* const testFuncName = "LES_Test_ReferenceInputStructParam";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_ReferenceInputStructParam, void);
	LES_FUNCTION_ADD_INPUT(TestStruct1&, input_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(long long int)+sizeof(char)+sizeof(int)+sizeof(short)+sizeof(float);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_ReferenceOutputPODParam(char& output_0)
{
	const char* const testFuncName = "LES_Test_ReferenceOutputPODParam";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_ReferenceOutputPODParam, void);
	LES_FUNCTION_ADD_OUTPUT(char&, output_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(char);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_ReferenceOutputStructParam(TestStruct2& output_0)
{
	const char* const testFuncName = "LES_Test_ReferenceOutputStructParam";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_ReferenceOutputStructParam, void);
	LES_FUNCTION_ADD_OUTPUT(TestStruct2&, output_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(float)+
																		sizeof(long long int)+sizeof(char)+sizeof(int)+sizeof(short)+sizeof(float)+
																		sizeof(char)+ sizeof(short)+ sizeof(int);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_ReferenceStructInputParam(TestStruct7& input_0)
{
	const char* const testFuncName = "LES_Test_ReferenceStructInputParam";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_ReferenceStructInputParam, void);
	LES_FUNCTION_ADD_INPUT(TestStruct7&, input_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(short)+ sizeof(float)+ sizeof(char)+ sizeof(char)+ sizeof(char);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_ReferenceStructOutputParam(TestStruct8& output_0)
{
	const char* const testFuncName = "LES_Test_ReferenceStructOutputParam";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_ReferenceStructOutputParam, void);
	LES_FUNCTION_ADD_OUTPUT(TestStruct8&, output_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(short)+
																		sizeof(char)+ sizeof(short)+
																		sizeof(char);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_DecodeInputArrayPOD(char input_0[3], short input_1[3])
{
	const char* const testFuncName = "LES_Test_DecodeInputArrayPOD";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeInputArrayPOD, void);
	LES_FUNCTION_ADD_INPUT(char[3], input_0);
	LES_FUNCTION_ADD_INPUT(short[3], input_1);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(char) * 3 + sizeof(short) * 3;

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_DecodeOutputArrayPOD(short output_0[3], int output_1[3])
{
	const char* const testFuncName = "LES_Test_DecodeOutputArrayPOD";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeOutputArrayPOD, void);
	LES_FUNCTION_ADD_OUTPUT(short[3], output_0);
	LES_FUNCTION_ADD_OUTPUT(int[3], output_1);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(int) * 3 + sizeof(short) * 3;

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_DecodeInputArraySTRUCTA(TestStruct2 input_0[2], TestStruct1 input_1[1])
{
	const char* const testFuncName = "LES_Test_DecodeInputArraySTRUCTA";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeInputArraySTRUCTA, void);
	LES_FUNCTION_ADD_INPUT(TestStruct2[2], input_0);
	LES_FUNCTION_ADD_INPUT(TestStruct1[1], input_1);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = 0 +
												2 * (sizeof(float)+
														 sizeof(long long int)+sizeof(char)+sizeof(int)+sizeof(short)+sizeof(float)+
														 sizeof(char)+ sizeof(short)+ sizeof(int)) +
												1 * (sizeof(long long int)+sizeof(char)+sizeof(int)+sizeof(short)+sizeof(float));

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_DecodeInputArraySTRUCTB(TestStruct1 input_0[2], TestStruct2 input_1[1])
{
	const char* const testFuncName = "LES_Test_DecodeInputArraySTRUCTB";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeInputArraySTRUCTB, void);
	LES_FUNCTION_ADD_INPUT(TestStruct1[2], input_0);
	LES_FUNCTION_ADD_INPUT(TestStruct2[1], input_1);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = 0 +
												2 * (sizeof(long long int)+sizeof(char)+sizeof(int)+sizeof(short)+sizeof(float)) +
												1 * (sizeof(float)+
														 sizeof(long long int)+sizeof(char)+sizeof(int)+sizeof(short)+sizeof(float)+
														 sizeof(char)+ sizeof(short)+ sizeof(int));

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_DecodeOutputArraySTRUCT(TestStruct3 output_0[2], TestStruct4 output_1[1])
{
	const char* const testFuncName = "LES_Test_DecodeOutputArraySTRUCT";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeOutputArraySTRUCT, void);
	LES_FUNCTION_ADD_OUTPUT(TestStruct3[2], output_0);
	LES_FUNCTION_ADD_OUTPUT(TestStruct4[1], output_1);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = 0 +
												2 * (sizeof(short) + sizeof(float) + sizeof(int) + sizeof(char)) +
											 	1 * (sizeof(float) + sizeof(int) + sizeof(char) + sizeof(short) + sizeof(short) +
															(sizeof(short) + sizeof(float) + sizeof(int) + sizeof(char))) +
												0;

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_DecodeInputArrayReference(char (&input_0)[3], short (&input_1)[3])
{
	const char* const testFuncName = "LES_Test_DecodeInputArrayReference";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeInputArrayReference, void);
	LES_FUNCTION_ADD_INPUT(char&[3], input_0);
	LES_FUNCTION_ADD_INPUT(short&[3], input_1);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(char) * 3 + sizeof(short) * 3;

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_DecodeOutputArrayReference(short (&output_0)[3], int (&output_1)[3])
{
	const char* const testFuncName = "LES_Test_DecodeOutputArrayReference";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeOutputArrayReference, void);
	LES_FUNCTION_ADD_OUTPUT(short&[3], output_0);
	LES_FUNCTION_ADD_OUTPUT(int&[3], output_1);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = sizeof(short) * 3 + sizeof(int) * 3;

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_DecodeInputStructPODArray(TestStruct9 input_0)
{
	const char* const testFuncName = "LES_Test_DecodeInputStructPODArray";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeInputStructPODArray, void);
	LES_FUNCTION_ADD_INPUT(TestStruct9, input_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = 3 * sizeof(short);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_DecodeInputStructStructArray(TestStruct10 input_0)
{
	const char* const testFuncName = "LES_Test_DecodeInputStructStructArray";
	LES_FunctionParameterData* parameterData = LES_NULL;

#if 0
	LES_LOG("m_short[0] = %p\n", &(input_0.m_short[0]));
	LES_LOG("m_short[1] = %p\n", &(input_0.m_short[1]));
	LES_LOG("m_short[2] = %p\n", &(input_0.m_short[2]));
	LES_LOG("m_testStruct9[0] = %p\n", &(input_0.m_testStruct9[0]));
	LES_LOG("m_testStruct9[0].m_short[0] = %p\n", &(input_0.m_testStruct9[0].m_short[0]));
	LES_LOG("m_testStruct9[0].m_short[1] = %p\n", &(input_0.m_testStruct9[0].m_short[1]));
	LES_LOG("m_testStruct9[0].m_short[2] = %p\n", &(input_0.m_testStruct9[0].m_short[2]));
	LES_LOG("m_testStruct9[1] = %p\n", &(input_0.m_testStruct9[1]));
	LES_LOG("m_testStruct9[1].m_short[0] = %p\n", &(input_0.m_testStruct9[1].m_short[0]));
	LES_LOG("m_testStruct9[1].m_short[1] = %p\n", &(input_0.m_testStruct9[1].m_short[1]));
	LES_LOG("m_testStruct9[1].m_short[2] = %p\n", &(input_0.m_testStruct9[1].m_short[2]));
#endif // #if 0
	LES_FUNCTION_START(LES_Test_DecodeInputStructStructArray, void);
	LES_FUNCTION_ADD_INPUT(TestStruct10, input_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = 3 * sizeof(short) +
																		2 * (sizeof(short) * 3);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
}

void LES_Test_DecodeInputStructPODArrayReference(TestStruct11 input_0)
{
	const char* const testFuncName = "LES_Test_DecodeInputStructPODArrayReference";
	LES_FunctionParameterData* parameterData = LES_NULL;

	LES_FUNCTION_START(LES_Test_DecodeInputStructPODArrayReference, void);
	LES_FUNCTION_ADD_INPUT(TestStruct11, input_0);
	LES_FUNCTION_GET_PARAMETER_DATA(parameterData);
	LES_FUNCTION_END();

	const int realParameterDataSize = 3 * sizeof(short) + 2 * sizeof(int) + 5 * sizeof(char);

	LES_Test_GenericDecodeHelper(testFuncName, parameterData, realParameterDataSize);
	return;
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
	LES_TEST_ADD_TYPE_POD(long long int);

	LES_TEST_ADD_TYPE_POD(float);
	LES_TEST_ADD_TYPE_POD(double);

	LES_TEST_ADD_TYPE_POD_POINTER(unsigned char, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_POINTER(unsigned short, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_POINTER(unsigned int, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_POINTER(unsigned long, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_ADD_TYPE_POD_POINTER(char, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_POINTER(short, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_POINTER(int, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_POINTER(long, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_ADD_TYPE_POD_POINTER(float, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_POINTER(double, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_ADD_TYPE_POD_REFERENCE(int, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_REFERENCE(short, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_REFERENCE(char, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_REFERENCE(float, LES_TYPE_INPUT_OUTPUT);
	
	LES_TEST_ADD_TYPE_POD_ARRAY(char, 3);
	LES_TEST_ADD_TYPE_POD_ARRAY(short, 3);
	LES_TEST_ADD_TYPE_POD_ARRAY(int, 3);

	LES_TEST_ADD_TYPE_POD_REFERENCE_ARRAY(char, 3, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_REFERENCE_ARRAY(short, 3, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_REFERENCE_ARRAY(int, 3, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_REFERENCE_ARRAY(int, 2, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_POD_REFERENCE_ARRAY(char, 5, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_ADD_TYPE_EX(output_only, 4, LES_TYPE_OUTPUT|LES_TYPE_POD, output_only, 0);

	LES_TEST_STRUCT_START(TestStruct1, 5);
	LES_TEST_STRUCT_ADD_MEMBER(long long int, m_longlong);
	LES_TEST_STRUCT_ADD_MEMBER(char, m_char);
	LES_TEST_STRUCT_ADD_MEMBER(int, m_int);
	LES_TEST_STRUCT_ADD_MEMBER(short, m_short);
	LES_TEST_STRUCT_ADD_MEMBER(float, m_float);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct1);
	LES_TEST_ADD_TYPE_STRUCT_POINTER(TestStruct1, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_STRUCT_REFERENCE(TestStruct1, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_STRUCT_ARRAY(TestStruct1, 1);
	LES_TEST_ADD_TYPE_STRUCT_ARRAY(TestStruct1, 2);

	LES_TEST_STRUCT_START(TestStruct2, 5);
	LES_TEST_STRUCT_ADD_MEMBER(float, m_float);
	LES_TEST_STRUCT_ADD_MEMBER(TestStruct1, m_testStruct1);
	LES_TEST_STRUCT_ADD_MEMBER(char, m_char);
	LES_TEST_STRUCT_ADD_MEMBER(short, m_short);
	LES_TEST_STRUCT_ADD_MEMBER(int, m_int);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct2);
	LES_TEST_ADD_TYPE_STRUCT_POINTER(TestStruct2, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_STRUCT_REFERENCE(TestStruct2, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_STRUCT_ARRAY(TestStruct2, 1);
	LES_TEST_ADD_TYPE_STRUCT_ARRAY(TestStruct2, 2);

	LES_TEST_STRUCT_START(TestStruct3, 4);
	LES_TEST_STRUCT_ADD_MEMBER(short, m_short);
	LES_TEST_STRUCT_ADD_MEMBER(float, m_float);
	LES_TEST_STRUCT_ADD_MEMBER(int, m_int);
	LES_TEST_STRUCT_ADD_MEMBER(char, m_char);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct3);
	LES_TEST_ADD_TYPE_STRUCT_POINTER(TestStruct3, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_STRUCT_ARRAY(TestStruct3, 2);

	LES_TEST_STRUCT_START(TestStruct4, 6);
	LES_TEST_STRUCT_ADD_MEMBER(float, m_float);
	LES_TEST_STRUCT_ADD_MEMBER(int, m_int);
	LES_TEST_STRUCT_ADD_MEMBER(char, m_char);
	LES_TEST_STRUCT_ADD_MEMBER(short, m_short);
	LES_TEST_STRUCT_ADD_MEMBER(short, m_short2);
	LES_TEST_STRUCT_ADD_MEMBER(TestStruct3, m_testStruct3);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct4);
	LES_TEST_ADD_TYPE_STRUCT_POINTER(TestStruct4, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_STRUCT_ARRAY(TestStruct4, 1);

	LES_TEST_STRUCT_START(TestStruct5, 2);
	LES_TEST_STRUCT_ADD_MEMBER(char, m_char);
	LES_TEST_STRUCT_ADD_MEMBER(short*, m_shortPtr);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct5);
	LES_TEST_ADD_TYPE_STRUCT_POINTER(TestStruct5, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_STRUCT_REFERENCE(TestStruct5, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_STRUCT_START(TestStruct6, 3);
	LES_TEST_STRUCT_ADD_MEMBER(short, m_short);
	LES_TEST_STRUCT_ADD_MEMBER(TestStruct5*, m_testStructPtr);
	LES_TEST_STRUCT_ADD_MEMBER(char*, m_charPtr);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct6);
	LES_TEST_ADD_TYPE_STRUCT_POINTER(TestStruct6, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_STRUCT_START(TestStruct7, 5);
	LES_TEST_STRUCT_ADD_MEMBER(short, m_short);
	LES_TEST_STRUCT_ADD_MEMBER(float&, m_floatRef);
	LES_TEST_STRUCT_ADD_MEMBER(char, m_char);
	LES_TEST_STRUCT_ADD_MEMBER(char&, m_charRef);
	LES_TEST_STRUCT_ADD_MEMBER(char*, m_charPtr);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct7);
	LES_TEST_ADD_TYPE_STRUCT_POINTER(TestStruct7, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_STRUCT_REFERENCE(TestStruct7, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_STRUCT_START(TestStruct8, 3);
	LES_TEST_STRUCT_ADD_MEMBER(short, m_short);
	LES_TEST_STRUCT_ADD_MEMBER(TestStruct5&, m_testStruct5Ref);
	LES_TEST_STRUCT_ADD_MEMBER(char*, m_charPtr);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct8);
	LES_TEST_ADD_TYPE_STRUCT_POINTER(TestStruct8, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_STRUCT_REFERENCE(TestStruct8, LES_TYPE_INPUT_OUTPUT);

	LES_TEST_STRUCT_START(TestStruct9, 1);
	LES_TEST_STRUCT_ADD_MEMBER(short[3], m_short);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct9);
	LES_TEST_ADD_TYPE_STRUCT_POINTER(TestStruct9, LES_TYPE_INPUT_OUTPUT);
	LES_TEST_ADD_TYPE_STRUCT_ARRAY(TestStruct9, 2);

	LES_TEST_STRUCT_START(TestStruct10, 2);
	LES_TEST_STRUCT_ADD_MEMBER(short[3], m_short);
	LES_TEST_STRUCT_ADD_MEMBER(TestStruct9[2], m_testStruct9);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct10);

	LES_TEST_STRUCT_START(TestStruct11, 3);
	LES_TEST_STRUCT_ADD_MEMBER(short&[3], m_short);
	LES_TEST_STRUCT_ADD_MEMBER(int&[2], m_int);
	LES_TEST_STRUCT_ADD_MEMBER(char&[5], m_char);
	LES_TEST_STRUCT_END();

	LES_TEST_ADD_TYPE_STRUCT(TestStruct11);

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
	LES_FunctionParameter* const functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	LES_AddStringEntry("input_0");
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = -1;
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	testFunctionData.globalParamIndex++;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputNameDoesntExist, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputNameHashIsWrong, void, 1, 0);
	LES_FunctionParameter* const functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	testFunctionData.globalParamIndex++;
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
	LES_FunctionParameter* const functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	LES_AddStringEntry("int");
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("input_0");
	functionParameterPtr->m_typeID = -1;
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	testFunctionData.globalParamIndex++;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_InputTypeHashIsWrong, void, 1, 0);
	LES_FunctionParameter* const functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("input_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("input_0");
	functionParameterPtr->m_typeID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_mode = LES_PARAM_MODE_INPUT;
	testFunctionData.globalParamIndex++;
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
	LES_FunctionParameter* const functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	LES_AddStringEntry("output_0");
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = -1;
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	testFunctionData.globalParamIndex++;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputNameDoesntExist, void, 0, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputNameHashIsWrong, void, 0, 1);
	LES_FunctionParameter* const functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	testFunctionData.globalParamIndex++;
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
	LES_FunctionParameter* const functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	LES_AddStringEntry("int");
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("output_0");
	functionParameterPtr->m_typeID = -1;
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	testFunctionData.globalParamIndex++;
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_OutputTypeHashIsWrong, void, 0, 1);
	LES_FunctionParameter* const functionParameterPtr = (LES_FunctionParameter* const)(functionDefinition.GetParameterByIndex(0));
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_hash = LES_GenerateHashCaseSensitive("output_0");
	functionParameterPtr->m_nameID = LES_AddStringEntry("output_0");
	functionParameterPtr->m_typeID = LES_AddStringEntry("wrongHash");
	functionParameterPtr->m_mode = LES_PARAM_MODE_OUTPUT;
	testFunctionData.globalParamIndex++;
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

	LES_TEST_FUNCTION_START(LES_Test_DecodeInputOutputParameters, void, 7, 4);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned int*, output_0);
	LES_TEST_FUNCTION_ADD_INPUT(short, input_1);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned short*, output_1);
	LES_TEST_FUNCTION_ADD_INPUT(char, input_2);
	LES_TEST_FUNCTION_ADD_OUTPUT(unsigned char*, output_2);
	LES_TEST_FUNCTION_ADD_INPUT(float, input_3);
	LES_TEST_FUNCTION_ADD_OUTPUT(float*, output_3);
	LES_TEST_FUNCTION_ADD_INPUT(int*, input_4);
	LES_TEST_FUNCTION_ADD_INPUT(short*, input_5);
	LES_TEST_FUNCTION_ADD_INPUT(char*, input_6);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_StructInputParam, void, 5, 0);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct2, input_0);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_1);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct1, input_2);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct3, input_3);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct4*, input_4);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_StructOutputParam, void, 0, 2);
	LES_TEST_FUNCTION_ADD_OUTPUT(TestStruct3*, out_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(TestStruct4*, out_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_StructInputOutputParam, void, 1, 1);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct5*, in_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(TestStruct6*, out_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReferenceInputPODParam, void, 1, 0);
	LES_TEST_FUNCTION_ADD_INPUT(int&, input_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReferenceInputStructParam, void, 1, 0);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct1&, input_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReferenceOutputPODParam, void, 0, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(char&, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReferenceOutputStructParam, void, 0, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(TestStruct2&, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReferenceStructInputParam, void, 1, 0);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct7&, input_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_ReferenceStructOutputParam, void, 0, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(TestStruct8&, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeOutputParameters, void, 0, 1);
	LES_TEST_FUNCTION_ADD_OUTPUT(char&, output_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeInputArrayPOD, void, 2, 0);
	LES_TEST_FUNCTION_ADD_INPUT(char[3], input_0);
	LES_TEST_FUNCTION_ADD_INPUT(short[3], input_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeOutputArrayPOD, void, 0, 2);
	LES_TEST_FUNCTION_ADD_OUTPUT(short[3], output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int[3], output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeInputArraySTRUCTA, void, 2, 0);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct2[2], input_0);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct1[1], input_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeInputArraySTRUCTB, void, 2, 0);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct1[2], input_0);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct2[1], input_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeOutputArraySTRUCT, void, 0, 2);
	LES_TEST_FUNCTION_ADD_OUTPUT(TestStruct3[2], output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(TestStruct4[1], output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeInputArrayReference, void, 2, 0);
	LES_TEST_FUNCTION_ADD_INPUT(char&[3], input_0);
	LES_TEST_FUNCTION_ADD_INPUT(short&[3], input_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeOutputArrayReference, void, 0, 2);
	LES_TEST_FUNCTION_ADD_OUTPUT(short&[3], output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int&[3], output_1);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeInputStructPODArray, void, 1, 0);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct9, input_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeInputStructStructArray, void, 1, 0);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct10, input_0);
	LES_TEST_FUNCTION_END();

	LES_TEST_FUNCTION_START(LES_Test_DecodeInputStructPODArrayReference, void, 1, 0);
	LES_TEST_FUNCTION_ADD_INPUT(TestStruct11, input_0);
	LES_TEST_FUNCTION_END();

	/* Run specific tests */
	/* Function header definition tests */
	LES_LOG("#### Function header definition tests ####\n");
	LES_Test_FunctionNotFound();
	LES_LOG("\n");
	LES_Test_ReturnTypeNotFound();
	LES_LOG("\n");
	LES_Test_ReturnTypeHashIsWrong();
	LES_LOG("\n");
	LES_Test_ReturnTypeStringIsWrong();
	LES_LOG("\n");

	unsigned int output_0 = 1;
	unsigned short output_1 = 2;
	unsigned char output_2 = 'O';
	float output_3 = -123.456f;
	int input_4 = 12345;

	/* Input parameter tests */
	LES_LOG("#### Input parameter tests ####\n");
	LES_Test_TooManyInputParameters(1, 2, 3, 4);
	LES_LOG("\n");
	LES_Test_InputWrongIndex(1, 2, 3);
	LES_LOG("\n");
	LES_Test_InputGlobalIndexIsWrong(1, 2);
	LES_LOG("\n");
	LES_Test_InputUsedAsOutput(1);
	LES_LOG("\n");
	LES_Test_InputParamAlreadyUsed(1, 2);
	LES_LOG("\n");
	LES_Test_InputParamMissing(1, &output_0);
	LES_LOG("\n");

	/* Input name tests */
	LES_LOG("#### Input name tests ####\n");
	LES_Test_InputNameDoesntExist(1);
	LES_LOG("\n");
	LES_Test_InputNameIDNotFound(1);
	LES_LOG("\n");
	LES_Test_InputNameHashIsWrong(1);
	LES_LOG("\n");
	LES_Test_InputNameStringIsWrong(1);
	LES_LOG("\n");

	/* Input type tests */
	LES_LOG("#### Input type tests ####\n");
	LES_Test_InputTypeIDNotFound(1);
	LES_LOG("\n");
	LES_Test_InputTypeHashIsWrong(1);
	LES_LOG("\n");
	LES_Test_InputTypeStringIsWrong(1);
	LES_LOG("\n");

	/* Output parameter tests */
	LES_LOG("#### Output parameter tests ####\n");
	LES_Test_TooManyOutputParameters(1, &output_0, &output_1, &output_2);
	LES_LOG("\n");
	LES_Test_OutputWrongIndex(1, 2, 3);
	LES_LOG("\n");
	LES_Test_OutputGlobalIndexIsWrong(1, 2);
	LES_LOG("\n");
	LES_Test_OutputUsedAsInput(1);
	LES_LOG("\n");
	LES_Test_OutputParamAlreadyUsed(1, &output_0);
	LES_LOG("\n");
	LES_Test_OutputParamMissing(1, &output_0);
	LES_LOG("\n");

	/* Output name tests */
	LES_LOG("#### Output name tests ####\n");
	LES_Test_OutputNameDoesntExist(1);
	LES_LOG("\n");
	LES_Test_OutputNameIDNotFound(1);
	LES_LOG("\n");
	LES_Test_OutputNameHashIsWrong(1);
	LES_LOG("\n");
	LES_Test_OutputNameStringIsWrong(1);
	LES_LOG("\n");

	/* Output type tests */
	LES_LOG("#### Output type tests ####\n");
	LES_Test_OutputTypeIDNotFound(1);
	LES_LOG("\n");
	LES_Test_OutputTypeHashIsWrong(1);
	LES_LOG("\n");
	LES_Test_OutputTypeStringIsWrong(1);
	LES_LOG("\n");

	/* Parameter already exists tests */
	LES_LOG("#### Parameter already exists tests ####\n");
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_TEST_FUNCTION_START(LES_Test_InputParameterAlreadyExists, void, 2, 1);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_INPUT(char, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
	LES_LOG("\n");

	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_TEST_FUNCTION_START(LES_Test_OutputParameterAlreadyExists, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(short, output_0);
	LES_TEST_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
	LES_LOG("\n");

	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_TEST_FUNCTION_START(LES_Test_ParameterAlreadyExists, void, 1, 2);
	LES_TEST_FUNCTION_ADD_INPUT(int, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(float, input_0);
	LES_TEST_FUNCTION_ADD_OUTPUT(int, output_0);
	LES_TEST_FUNCTION_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
	LES_LOG("\n");

	/* Complex Input Output Parameter tests */
	LES_LOG("#### Complex Input Output Parameter tests ####\n");
	LES_Test_InputOutputMixture(1, &output_0, 3, &output_1);
	LES_LOG("LES_Test_InputOutputMixture: success if no error output\n");
	LES_LOG("\n");
	LES_Test_InputParamUsedAsOutput(1, 2);
	LES_LOG("\n");
	LES_Test_OutputParamUsedAsInput(1, 2);
	LES_LOG("\n");

	/* Add Type tests */
	LES_LOG("#### Add Type tests ####\n");
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_TEST_ADD_TYPE_EX(unsigned char, 2, LES_TYPE_POD|LES_TYPE_INPUT, unsigned char, 0);
	LES_LOG("\n");
	LES_TEST_ADD_TYPE_EX(unsigned char, 1, LES_TYPE_POINTER|LES_TYPE_OUTPUT, unsigned char, 0);
	LES_LOG("\n");
	LES_TEST_ADD_TYPE_EX(unsigned char*, 4, LES_TYPE_POINTER|LES_TYPE_POD|LES_TYPE_INPUT|LES_TYPE_OUTPUT, unsigned int, 0);
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
	LES_LOG("\n");

	/* Read Parameter tests */
	LES_LOG("#### Read Parameter tests ####\n");
	LES_Test_ReadInputParameters(102, 23453, 110, -4.0332f, &input_4);
	LES_LOG("\n");
	LES_Test_ReadOutputParameters(&output_0, &output_1, &output_2, &output_3);
	LES_LOG("\n");
	LES_Test_ReadInputOutputParameters(102, 23453, '*', -4.0332f, &input_4, &output_0, &output_1, &output_2, &output_3);
	LES_LOG("\n");
	short input_5 = 8989;
	char input_6 = '6';
	LES_Test_DecodeInputOutputParameters(201, 12863, ';', -8.9832f, &input_4, &input_5, &input_6, 
																			 &output_0, &output_1, &output_2, &output_3);
	LES_LOG("\n");

	/* Struct tests */
	LES_LOG("#### Struct tests ####\n");
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_TEST_STRUCT_START(LES_Struct_TooManyMembers, 1);
	LES_TEST_STRUCT_ADD_MEMBER(int, m_one);
	LES_TEST_STRUCT_ADD_MEMBER(short, m_two);
	LES_TEST_STRUCT_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
	LES_LOG("\n");
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_TEST_STRUCT_START(LES_Struct_MemberAlreadyExists, 2);
	LES_TEST_STRUCT_ADD_MEMBER(int, m_one);
	LES_TEST_STRUCT_ADD_MEMBER(short, m_one);
	LES_TEST_STRUCT_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
	LES_LOG("\n");
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_TEST_STRUCT_START(LES_Struct_NotEnoughMembers, 3);
	LES_TEST_STRUCT_ADD_MEMBER(long long int, m_longlong);
	LES_TEST_STRUCT_ADD_MEMBER(char, m_char);
	LES_TEST_STRUCT_END();
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
	LES_LOG("\n");

	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	TestStruct1 in_2;
	in_2.m_longlong = -100048;
	in_2.m_int = 65;
	in_2.m_short = -123;
	in_2.m_char = 'A';
	in_2.m_float = 666.987f;
	TestStruct2 in_0;
	in_0.m_float = 666.987f;
	in_0.m_testStruct1 = in_2;
	in_0.m_char = 'B';
	in_0.m_short = -123;
	in_0.m_int = 65;
	TestStruct3 in_3;
	in_3.m_char = 'J';
	in_3.m_float = 2.173f;
	in_3.m_int = -542;
	in_3.m_short = 209;
	TestStruct4 in_4;
	in_4.m_float = 987.123f;
	in_4.m_int = 12;
	in_4.m_char = 'L';
	in_4.m_short = +321;
	in_4.m_short2 = +322;
	in_4.m_testStruct3.m_char = '#';
	in_4.m_testStruct3.m_float = 3.141f;
	in_4.m_testStruct3.m_int = 2;
	in_4.m_testStruct3.m_short = 3;
	LES_Test_StructInputParam(in_0, 7243, in_2, in_3, &in_4);
	LES_LOG("\n");

	TestStruct3 out_0;
	out_0.m_short = +1718;
	out_0.m_float = 271.387f;
	out_0.m_int = 14;
	out_0.m_char = 'a';
	TestStruct4 out_1;
	out_1.m_float = 1414.987f;
	out_1.m_int = -2713;
	out_1.m_char = 'Z';
	out_1.m_short = +321;
	out_1.m_short2 = +322;
	out_1.m_testStruct3 = out_0;
#if LES_TEST_DEBUG
	LES_LOG("TestStruct3: m_short:%p\n", &out_0.m_short);
	LES_LOG("TestStruct3: m_float:%p\n", &out_0.m_float);
	LES_LOG("TestStruct3: m_int:%p\n", &out_0.m_int);
	LES_LOG("TestStruct3: m_char:%p\n", &out_0.m_char);
	LES_LOG("TestStruct4: m_float:%p\n", &out_1.m_float);
	LES_LOG("TestStruct4: m_int:%p\n", &out_1.m_int);
	LES_LOG("TestStruct4: m_char:%p\n", &out_1.m_char);
	LES_LOG("TestStruct4: m_short:%p\n", &out_1.m_short);
	LES_LOG("TestStruct4: m_short2:%p\n", &out_1.m_short2);
	LES_LOG("TestStruct4: m_testStruct3:%p\n", &out_1.m_testStruct3);
#endif // #if LES_TEST_DEBUG
	LES_Test_StructOutputParam(&out_0, &out_1);
	LES_LOG("\n");
	TestStruct5 testStruct5;
	testStruct5.m_char = '5';
	short shortTest = 4242;
	testStruct5.m_shortPtr = &shortTest;
	TestStruct6 testStruct6;
	testStruct6.m_short = -5656;
	testStruct6.m_testStruct5Ptr = &testStruct5;
	char charTest = '6';
	testStruct6.m_charPtr = &charTest;
	LES_Test_StructInputOutputParam(&testStruct5, &testStruct6);
	LES_LOG("\n");

	/* Reference tests */
	LES_LOG("#### Reference tests ####\n");

	int ref_input_0 = 954;
	LES_Test_ReferenceInputPODParam(ref_input_0);
	LES_LOG("\n");

	TestStruct1 ref_input_1;
	ref_input_1.m_char = '1';
	ref_input_1.m_float = 16.2131f;
	ref_input_1.m_int = 14;
	ref_input_1.m_longlong = 162132;
	ref_input_1.m_short = 936;
	LES_Test_ReferenceInputStructParam(ref_input_1);
	LES_LOG("\n");

	char ref_output_0 = 'R';
	LES_Test_ReferenceOutputPODParam(ref_output_0);
	LES_LOG("\n");

	TestStruct2 ref_output_1;
	ref_output_1.m_char = 'Z';
	ref_output_1.m_float = 2131.16f;
	ref_output_1.m_int = 1564;
	ref_output_1.m_short = 2526;
	ref_output_1.m_testStruct1 = ref_input_1;
	LES_Test_ReferenceOutputStructParam(ref_output_1);
	LES_LOG("\n");

#if LES_TEST_DEBUG
	{
		const LES_StringEntry* stringEntry = LES_NULL;
		stringEntry = LES_GetStringEntry("char*");
		int charPtrSize = LES_GetTypeEntry(stringEntry)->ComputeDataStorageSize();
		charPtrSize = LES_GetTypeEntry(stringEntry)->m_dataSize;
		stringEntry = LES_GetStringEntry("char&");
		int charRefSize = LES_GetTypeEntry(stringEntry)->ComputeDataStorageSize();
		charRefSize = LES_GetTypeEntry(stringEntry)->m_dataSize;
		LES_LOG("charPtrSize:%d charRefSize:%d\n", charPtrSize, charRefSize);

		stringEntry = LES_GetStringEntry("short*");
		int shortPtrSize = LES_GetTypeEntry(stringEntry)->ComputeDataStorageSize();
		shortPtrSize = LES_GetTypeEntry(stringEntry)->m_dataSize;
		stringEntry = LES_GetStringEntry("short&");
		int shortRefSize = LES_GetTypeEntry(stringEntry)->ComputeDataStorageSize();
		shortRefSize = LES_GetTypeEntry(stringEntry)->m_dataSize;
		LES_LOG("shortPtrSize:%d shortRefSize:%d\n", shortPtrSize, shortRefSize);
	}
#endif // #if LES_TEST_DEBUG

	float ref_float = 76.1232f;
	char ref_char = 'C';
	TestStruct7 refStruct_input_0 = { 936, ref_float, 'A', ref_char, &ref_output_0 };
#if LES_TEST_DEBUG
	LES_LOG("m_floatRef = %f ref_float:%p m_floatRef:%p\n", refStruct_input_0.m_floatRef, &ref_float, &refStruct_input_0.m_floatRef);
	LES_LOG("m_charRef = '%c' ref_char:%p m_charRef:%p\n", refStruct_input_0.m_charRef, &ref_char, &refStruct_input_0.m_charRef);
#endif // #if LES_TEST_DEBUG
	LES_Test_ReferenceStructInputParam(refStruct_input_0);
	LES_LOG("\n");

	TestStruct8 refStruct_output_0 = { 2526, testStruct5, &ref_output_0 };
	LES_Test_ReferenceStructOutputParam(refStruct_output_0);
	LES_LOG("\n");

	LES_Test_DecodeOutputParameters(ref_char);
	LES_LOG("\n");

	LES_LOG("#### Array tests ####\n");
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, false);
	LES_TEST_ADD_TYPE_EX(arrayNotAliasedToPtr[4], sizeof(int), LES_TYPE_ARRAY|LES_TYPE_INPUT|LES_TYPE_POD, int, 4);
	LES_LOG("\n");
	LES_TEST_ADD_TYPE_EX(arrayNotAliased[5], sizeof(int), LES_TYPE_ARRAY|LES_TYPE_INPUT|LES_TYPE_POD, arrayNotAliased[5], 4);
	LES_LOG("\n");
	LES_TEST_ADD_TYPE_EX(arrayExistsWrongNumberOfElements[1], sizeof(int), LES_TYPE_ARRAY|LES_TYPE_INPUT|LES_TYPE_POD, int*, 1);
	LES_TEST_ADD_TYPE_EX(arrayExistsWrongNumberOfElements[1], sizeof(int), LES_TYPE_ARRAY|LES_TYPE_INPUT|LES_TYPE_POD, int*, 2);
	LES_LOG("\n");
	LES_TEST_ADD_TYPE_EX(arrayCantFindAlias[1], sizeof(int), LES_TYPE_ARRAY|LES_TYPE_INPUT|LES_TYPE_POD, jake*, 1);
	LES_LOG("\n");
	LES_TEST_ADD_TYPE_EX(arrayInvalidNumElements[1], sizeof(int), LES_TYPE_ARRAY|LES_TYPE_INPUT|LES_TYPE_POD, int*, 0);
	LES_Logger::SetFatal(LES_Logger::CHANNEL_FATAL_ERROR, true);
	LES_LOG("\n");
	char arrpod_in_0[3];
	arrpod_in_0[0] = '0';
	arrpod_in_0[1] = '1';
	arrpod_in_0[2] = '2';
	short arrpod_in_1[3];
	arrpod_in_1[0] = 3141;
	arrpod_in_1[1] = 3142;
	arrpod_in_1[2] = 3143;
	LES_Test_DecodeInputArrayPOD(arrpod_in_0, arrpod_in_1);
	LES_LOG("\n");
	short arrpod_out_0[3];
	arrpod_out_0[0] = 2713;
	arrpod_out_0[1] = 2714;
	arrpod_out_0[2] = 2715;
	int arrpod_out_1[3];
	arrpod_out_1[0] = -1954;
	arrpod_out_1[1] = -1955;
	arrpod_out_1[2] = -1956;
	LES_Test_DecodeOutputArrayPOD(arrpod_out_0, arrpod_out_1);
	LES_LOG("\n");

	TestStruct1 arrstra_in_1[1];
	arrstra_in_1[0].m_longlong = 18;
	arrstra_in_1[0].m_char = 'M';
	arrstra_in_1[0].m_int = 230421;
	arrstra_in_1[0].m_short = 421;
	arrstra_in_1[0].m_float = 1525.0f;
	TestStruct2 arrstra_in_0[2];
	arrstra_in_0[0].m_float = 2137.0f;
	arrstra_in_0[0].m_testStruct1 = arrstra_in_1[0];
	arrstra_in_0[0].m_char = 'Y';
	arrstra_in_0[0].m_short = 2627;
	arrstra_in_0[0].m_int = 21412932;
	arrstra_in_0[1].m_float = 2138.0f;
	arrstra_in_0[1].m_testStruct1 = arrstra_in_1[0];
	arrstra_in_0[1].m_char = 'Z';
	arrstra_in_0[1].m_short = 2628;
	arrstra_in_0[1].m_int = 21412933;
	LES_Test_DecodeInputArraySTRUCTA(arrstra_in_0, arrstra_in_1);
	LES_LOG("\n");
	TestStruct1 arrstrb_in_0[2];
	arrstrb_in_0[0].m_longlong = 18;
	arrstrb_in_0[0].m_char = 'M';
	arrstrb_in_0[0].m_int = 230421;
	arrstrb_in_0[0].m_short = 421;
	arrstrb_in_0[0].m_float = 1525.0f;
	arrstrb_in_0[1].m_longlong = 19;
	arrstrb_in_0[1].m_char = 'N';
	arrstrb_in_0[1].m_int = 230422;
	arrstrb_in_0[1].m_short = 422;
	arrstrb_in_0[1].m_float = 1526.0f;
	TestStruct2 arrstrb_in_1[1];
	arrstrb_in_1[0].m_float = 2137.0f;
	arrstrb_in_1[0].m_testStruct1 = arrstrb_in_0[1];
	arrstrb_in_1[0].m_char = 'Y';
	arrstrb_in_1[0].m_short = 2627;
	arrstrb_in_1[0].m_int = 21412932;
	LES_Test_DecodeInputArraySTRUCTB(arrstrb_in_0, arrstrb_in_1);
	LES_LOG("\n");
	TestStruct3 arrstra_out_0[2];
	arrstra_out_0[0].m_short = 1953;
	arrstra_out_0[0].m_float = 29.30f;
	arrstra_out_0[0].m_int = 230810;
	arrstra_out_0[0].m_char = '<';
	arrstra_out_0[1].m_short = 1954;
	arrstra_out_0[1].m_float = 29.40f;
	arrstra_out_0[1].m_int = 230811;
	arrstra_out_0[1].m_char = '>';
	TestStruct4 arrstra_out_1[1];
	arrstra_out_1[0].m_float = 19.63f;
	arrstra_out_1[0].m_int = 1963;
	arrstra_out_1[0].m_char = '#';
	arrstra_out_1[0].m_short = 2930;
	arrstra_out_1[0].m_short2 = 2931;
	arrstra_out_1[0].m_testStruct3 = arrstra_out_0[1];
	LES_Test_DecodeOutputArraySTRUCT(arrstra_out_0, arrstra_out_1);
	LES_LOG("\n");
	LES_Test_DecodeInputArrayReference(arrpod_in_0, arrpod_in_1);
	LES_LOG("\n");
	LES_Test_DecodeOutputArrayReference(arrpod_out_0, arrpod_out_1);
	LES_LOG("\n");
	TestStruct9 struct_arrpod_in_0;
	struct_arrpod_in_0.m_short[0] = 2063;
	struct_arrpod_in_0.m_short[1] = 2064;
	struct_arrpod_in_0.m_short[2] = 2065;
	LES_Test_DecodeInputStructPODArray(struct_arrpod_in_0);
	LES_LOG("\n");
	TestStruct10 struct_arrstr_in_0;
	struct_arrstr_in_0.m_short[0] = 3435;
	struct_arrstr_in_0.m_short[1] = 3436;
	struct_arrstr_in_0.m_short[2] = 3437;
	struct_arrstr_in_0.m_testStruct9[0] = struct_arrpod_in_0;
	struct_arrstr_in_0.m_testStruct9[1].m_short[0] = 2071;
	struct_arrstr_in_0.m_testStruct9[1].m_short[1] = 2072;
	struct_arrstr_in_0.m_testStruct9[1].m_short[2] = 2073;
	LES_Test_DecodeInputStructStructArray(struct_arrstr_in_0);
	LES_LOG("\n");
	short jake[3] = { 2180, 2181, 2182 };
 	int chris[2] = { 2098, 2099 };
 	char rowan[5] = { 'R', 'O', 'W', 'A', 'M' };
	TestStruct11 struct_arrpodref_in_0 = {jake, chris, rowan};
	LES_Test_DecodeInputStructPODArrayReference(struct_arrpodref_in_0);
	LES_LOG("\n");

#if 0
	LES_LOG("TestStruct4.m_float %p\n", &(in_4.m_float));
	LES_LOG("TestStruct4.m_int %p\n", &(in_4.m_int));
	LES_LOG("TestStruct4.m_char %p\n", &(in_4.m_char));
	LES_LOG("TestStruct4.m_short %p\n", &(in_4.m_short));
	LES_LOG("TestStruct4.m_short2 %p\n", &(in_4.m_short2));
	LES_LOG("TestStruct4.m_testStruct3 %p\n", &(in_4.m_testStruct3));
	LES_LOG("TestStruct4.m_testStruct3.m_short %p\n", &(in_4.m_testStruct3.m_short));
	LES_LOG("TestStruct4.m_testStruct3.m_float %p\n", &(in_4.m_testStruct3.m_float));
	LES_LOG("TestStruct4.m_testStruct3.m_int %p\n", &(in_4.m_testStruct3.m_int));
	LES_LOG("TestStruct4.m_testStruct3.m_char %p\n", &(in_4.m_testStruct3.m_char));
struct TestStruct66
{
	short m_short[3];
	TestStruct3 m_testStruct3[1];
};
	TestStruct66 in_66;
	LES_LOG("TestStruct66.m_short[0] %p\n", &(in_66.m_short[0]));
	LES_LOG("TestStruct66.m_short[1] %p\n", &(in_66.m_short[1]));
	LES_LOG("TestStruct66.m_short[2] %p\n", &(in_66.m_short[2]));
	LES_LOG("TestStruct66.m_testStruct3 %p\n", &(in_66.m_testStruct3[0]));
	LES_LOG("TestStruct66.m_testStruct3.m_short %p\n", &(in_66.m_testStruct3[0].m_short));
	LES_LOG("TestStruct66.m_testStruct3.m_float %p\n", &(in_66.m_testStruct3[0].m_float));
	LES_LOG("TestStruct66.m_testStruct3.m_int %p\n", &(in_66.m_testStruct3[0].m_int));
	LES_LOG("TestStruct66.m_testStruct3.m_char %p\n", &(in_66.m_testStruct3[0].m_char));
#endif // #if 0
}

