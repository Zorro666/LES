#include <stdio.h>
#include <string.h>

#include "les_core.h"
#include "les_hash.h"
#include "les_stringentry.h"

#define LES_FUNCTION_ADD_INPUT(INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \
	const char* const __LESinputType__##INPUT_NUMBER = #INPUT_TYPE; \
	const LES_Hash __LESinputTypeHash__##INPUT_NUMBER = LES_GenerateHashCaseSensitive(#INPUT_TYPE); \
	const char* const __LESinputName__##INPUT_NUMBER = #INPUT_NAME; \
	const LES_Hash __LESinputNameHash__##INPUT_NUMBER = LES_GenerateHashCaseSensitive(#INPUT_NAME); \
	/* Check the input parameter index */ \
	if (INPUT_NUMBER != __LESfunctionCurrentInputParamIndex__) \
	{ \
		/* ERROR: input parameter index doesn't match the expected input index */ \
		fprintf(stderr, "ERROR:function '%s' : Wrong function input param index:%d expected:%d param:'%s'\n", __LESfunctionName__, \
						__LESfunctionCurrentInputParamIndex__, INPUT_NUMBER, __LESinputName__##INPUT_NUMBER ); \
		return; \
	} \
	const LES_FunctionParameter* const __LESinputParameter__##INPUT_NUMBER = __LESfunctionDefinition->GetInputParameter(INPUT_NUMBER); \
	/* Check the parameter index */ \
	if (__LESinputParameter__##INPUT_NUMBER->index != __LESfunctionCurrentParamIndex__) \
	{ \
		/* ERROR: input parameter index doesn't match the value stored in the definition file */ \
		fprintf(stderr, "ERROR:function '%s' : Wrong function param index:%d expected:%d param:'%s'\n", __LESfunctionName__, \
						__LESfunctionCurrentParamIndex__, __LESinputParameter__##INPUT_NUMBER->index, __LESinputName__##INPUT_NUMBER ); \
		return; \
	} \
	/* Check the input parameter type */ \
	const LES_StringEntry* const __LESinputParameterTypeStringEntry__##INPUT_NUMBER = LES_GetStringEntryForID(__LESinputParameter__##INPUT_NUMBER->typeID); \
	if (__LESinputParameterTypeStringEntry__##INPUT_NUMBER == LES_NULL) \
	{ \
		/* ERROR: can't find the input parameter type */ \
		fprintf(stderr, "ERROR:function '%s' : Can't find input parameter type for ID:%d '%s'\n", __LESfunctionName__, \
				__LESinputParameter__##INPUT_NUMBER->typeID, __LESinputType__##INPUT_NUMBER); \
		return; \
	} \
	/* Check the input parameter type : hash */ \
	if (__LESinputTypeHash__##INPUT_NUMBER != __LESinputParameterTypeStringEntry__##INPUT_NUMBER->hash) \
	{ \
		/* ERROR : input parameter type hash doesn't match */ \
		fprintf(stderr, "ERROR:function '%s' : input parameter type hash doesn't match for ID:%d 0x%X != 0x%X '%s':'%s'\n", __LESfunctionName__, \
				__LESinputParameter__##INPUT_NUMBER->typeID, \
				__LESinputTypeHash__##INPUT_NUMBER, __LESinputParameterTypeStringEntry__##INPUT_NUMBER->hash, \
				__LESinputType__##INPUT_NUMBER, __LESinputParameterTypeStringEntry__##INPUT_NUMBER->str); \
		return; \
	} \
	/* Check the input parameter type : string */ \
	if (strcmp(__LESinputType__##INPUT_NUMBER, __LESinputParameterTypeStringEntry__##INPUT_NUMBER->str) != 0) \
	{ \
		/* ERROR : input parameter type string doesn't match */ \
		fprintf(stderr, "ERROR:function '%s' : input parameter type string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n", __LESfunctionName__, \
				__LESinputParameter__##INPUT_NUMBER->typeID, \
				__LESinputType__##INPUT_NUMBER, __LESinputParameterTypeStringEntry__##INPUT_NUMBER->str, \
				__LESinputTypeHash__##INPUT_NUMBER, __LESinputParameterTypeStringEntry__##INPUT_NUMBER->hash ); \
		return; \
	} \
	/* Check the input parameter name */ \
	const LES_StringEntry* const __LESinputParameterNameStringEntry__##INPUT_NUMBER = LES_GetStringEntryForID(__LESinputParameter__##INPUT_NUMBER->nameID); \
	if (__LESinputParameterNameStringEntry__##INPUT_NUMBER == LES_NULL) \
	{ \
		/* ERROR: can't find the input parameter name */ \
		fprintf(stderr, "ERROR:function '%s' : Can't find input parameter name for ID:%d '%s'\n", __LESfunctionName__, \
				__LESinputParameter__##INPUT_NUMBER->nameID, __LESinputName__##INPUT_NUMBER); \
		return; \
	} \
	/* Check the input parameter name : hash */ \
	if (__LESinputNameHash__##INPUT_NUMBER != __LESinputParameterNameStringEntry__##INPUT_NUMBER->hash) \
	{ \
		/* ERROR : input parameter name hash doesn't match */ \
		fprintf(stderr, "ERROR:function '%s' : input parameter name hash doesn't match for ID:%d 0x%X != 0x%X '%s':'%s'\n", __LESfunctionName__, \
				__LESinputParameter__##INPUT_NUMBER->nameID, \
				__LESinputNameHash__##INPUT_NUMBER, __LESinputParameterNameStringEntry__##INPUT_NUMBER->hash, \
				__LESinputName__##INPUT_NUMBER, __LESinputParameterNameStringEntry__##INPUT_NUMBER->str); \
		return; \
	} \
	/* Check the input parameter name : string */ \
	if (strcmp(__LESinputName__##INPUT_NUMBER, __LESinputParameterNameStringEntry__##INPUT_NUMBER->str) != 0) \
	{ \
		/* ERROR : input parameter name string doesn't match */ \
		fprintf(stderr, "ERROR:function '%s' : input parameter name string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n", __LESfunctionName__, \
				__LESinputParameter__##INPUT_NUMBER->nameID, \
				__LESinputName__##INPUT_NUMBER, __LESinputParameterNameStringEntry__##INPUT_NUMBER->str, \
				__LESinputNameHash__##INPUT_NUMBER, __LESinputParameterNameStringEntry__##INPUT_NUMBER->hash ); \
		return; \
	} \
	/* Store the input parameter value */ \



#define LES_FUNCTION_ADD_OUTPUT(OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \
	const char* const __LESoutputType__##OUTPUT_NUMBER = #OUTPUT_TYPE; \
	const char* const __LESoutputName__##OUTPUT_NUMBER = #OUTPUT_NAME; \


#define LES_FUNCTION_START(FUNC_NAME, RETURN_TYPE) \
	const char* const __LESfunctionName__ = #FUNC_NAME; \
	const char* const __LESfunctionReturnType__ = #RETURN_TYPE; \
	const LES_FunctionDefinition* const __LESfunctionDefinition = LES_GetFunctionDefinition(__LESfunctionName__); \
	if (__LESfunctionDefinition == LES_NULL) \
	{ \
		/*Throw an ERROR*/ \
		fprintf(stderr, "ERROR:function '%s' : Can't find function definition\n", __LESfunctionName__); \
		return; \
	} \
	const LES_StringEntry* const __LESfunctionReturnTypeStringEntry = LES_GetStringEntryForID(__LESfunctionDefinition->returnTypeID); \
	if (__LESfunctionReturnTypeStringEntry == LES_NULL) \
	{ \
		/*Throw an ERROR*/ \
		fprintf(stderr, "ERROR:function '%s' : Can't find function return type for ID:%d '%s'\n", __LESfunctionName__, \
				__LESfunctionDefinition->returnTypeID, __LESfunctionReturnType__); \
		return; \
	} \
	int __LESfunctionCurrentInputParamIndex__ = 0; \
	int __LESfunctionCurrentOutpuParamIndex__ = 0; \
	int __LESfunctionCurrentParamIndex__ = 0; \
	__LESfunctionCurrentInputParamIndex__ += 0; \
	__LESfunctionCurrentOutpuParamIndex__ += 0; \
	__LESfunctionCurrentParamIndex__ += 0; \


#define LES_FUNCTION_INPUTS_1(INPUT_0_TYPE, INPUT_0_NAME) \
	LES_FUNCTION_ADD_INPUT(0, INPUT_0_TYPE, INPUT_0_NAME) \


#define LES_FUNCTION_OUTPUTS_1(OUTPUT_0_TYPE, OUTPUT_0_NAME) \
	LES_FUNCTION_ADD_OUTPUT(0, OUTPUT_0_TYPE, OUTPUT_0_NAME) \


#define LES_FUNCTION_END() \
	) \


int main(const int argc, const char* const argv[])
{
	printf("argc=%d\n", argc);
	for (int i=0; i<argc;i++)
	{
		printf("argv[%d]='%s'\n", i, argv[i]);
	}

	return -1;
}

void LES_sceNpInit(int a)
{
	LES_FUNCTION_START(sceNpInit, void)
	LES_FUNCTION_INPUTS_1(int, a)
}
