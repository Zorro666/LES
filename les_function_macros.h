#ifndef LES_FUNCTION_MACROS_HH
#define LES_FUNCTION_MACROS_HH

#include "les_base.h"
#include "les_logger.h"
#include "les_function.h"

class LES_FunctionDefinition;
class LES_FunctionParameterData;

////////////////////////////////////////////////////////////////////////////////////////////
//
// Macros to auto-generate stub/wrapper code to wrap functions
//
////////////////////////////////////////////////////////////////////////////////////////////

struct LES_FunctionTempData
{
	const char* functionName;

	int functionCurrentParamIndex;
	int functionCurrentInputIndex;
	int functionCurrentOutputIndex;

	int functionInputMacroParamIndex;
	int functionOutputMacroParamIndex;

	LES_FunctionParameterData* functionParameterData;
	char paramUsed[LES_MAX_NUM_FUNCTION_PARAMS];
};

extern int LES_FunctionStart(const char* const name, const char* const returnType, 
												  	 const LES_FunctionDefinition** functionDefinitionPtr,
														 LES_FunctionTempData* const functionTempData);

#define LES_FUNCTION_START(FUNC_NAME, RETURN_TYPE) \
	{ \
		bool __LES_ok = true; \
		const LES_FunctionDefinition* __LESfunctionDefinition = LES_NULL; \
		LES_FunctionTempData __LESfunctionTempData; \
		if (LES_FunctionStart(#FUNC_NAME, #RETURN_TYPE, \
													&__LESfunctionDefinition, \
													&__LESfunctionTempData) != LES_RETURN_OK) \
		{ \
			LES_FATAL_ERROR("'%s' : Error during LES_FunctionStart", #FUNC_NAME); \
			__LES_ok = false; \
		} \


extern int LES_FunctionGetParameterData(const LES_FunctionTempData* const functionTempData,
																				LES_FunctionParameterData** const functionParameterDataPtr);

#define LES_FUNCTION_GET_PARAMETER_DATA(PARAMETER_DATA_PTR) \
		if (__LES_ok == true) \
		{ \
			if (LES_FunctionGetParameterData(&__LESfunctionTempData, &PARAMETER_DATA_PTR) != LES_RETURN_OK) \
			{ \
				LES_FATAL_ERROR("'%s' : Error during LES_FunctionGetParameterData", __LESfunctionTempData.functionName); \
				__LES_ok = false; \
			} \
		} \

extern int LES_FunctionEnd(const LES_FunctionDefinition* const functionDefinitionPtr,
													 const LES_FunctionTempData* const functionTempData);

#define LES_FUNCTION_END() \
		if (__LES_ok == true) \
		{ \
			if (LES_FunctionEnd(__LESfunctionDefinition, &__LESfunctionTempData) != LES_RETURN_OK) \
			{ \
				LES_FATAL_ERROR("'%s' : Error during LES_FunctionEnd", __LESfunctionTempData.functionName); \
				__LES_ok = false; \
			} \
		} \
	} \


extern int LES_FunctionAddParam(const char* const type, const char* const name, const int index, 
																const char* const mode, const bool isInput, void* const data,
												  			const LES_FunctionDefinition* const functionDefinition,
																LES_FunctionTempData* const functionTempData);

#define LES_FUNCTION_ADD_PARAM(PARAM_TYPE, IS_INPUT, NUMBER, TYPE, NAME) \
		if (__LES_ok == true) \
		{ \
			if (LES_FunctionAddParam(#TYPE, #NAME, NUMBER, #PARAM_TYPE, IS_INPUT, (void*)&NAME, \
						__LESfunctionDefinition, &__LESfunctionTempData) != LES_RETURN_OK) \
			{ \
				LES_FATAL_ERROR("'%s' : Error adding " #PARAM_TYPE " parameter %d '%s' type:'%s'", \
												__LESfunctionTempData.functionName, NUMBER, #NAME, #TYPE); \
				__LES_ok = false; \
			} \
		} \


#define LES_FUNCTION_ADD_INPUT(INPUT_TYPE, INPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Input, true, __LESfunctionTempData.functionInputMacroParamIndex, INPUT_TYPE, INPUT_NAME) \
	__LESfunctionTempData.functionInputMacroParamIndex++; \


#define LES_FUNCTION_ADD_INPUT_EX(INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Input, true, INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \


#define LES_FUNCTION_ADD_OUTPUT(OUTPUT_TYPE, OUTPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Output, false, __LESfunctionTempData.functionOutputMacroParamIndex, OUTPUT_TYPE, OUTPUT_NAME) \
	__LESfunctionTempData.functionOutputMacroParamIndex++; \


#define LES_FUNCTION_ADD_OUTPUT_EX(OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Output, false, OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \


#define LES_FUNCTION_INPUTS_1(INPUT_0_TYPE, INPUT_0_NAME) \
	LES_FUNCTION_ADD_INPUT(INPUT_0_TYPE, INPUT_0_NAME) \


#define LES_FUNCTION_INPUTS_2(INPUT_0_TYPE, INPUT_0_NAME, INPUT_1_TYPE, INPUT_1_NAME) \
	LES_FUNCTION_ADD_INPUT(INPUT_0_TYPE, INPUT_0_NAME) \
	LES_FUNCTION_ADD_INPUT(INPUT_1_TYPE, INPUT_1_NAME) \


#define LES_FUNCTION_OUTPUTS_1(OUTPUT_0_TYPE, OUTPUT_0_NAME) \
	LES_FUNCTION_ADD_OUTPUT(OUTPUT_0_TYPE, OUTPUT_0_NAME) \


#endif // #ifndef LES_FUNCTION_MACROS_HH
