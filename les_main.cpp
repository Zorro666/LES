#include <stdio.h>
#include <string.h>

#include "les_core.h"
#include "les_hash.h"
#include "les_stringentry.h"

#define LES_FUNCTION_ADD_PARAM(PARAM_TYPE, NUMBER, TYPE, NAME) \
	const char* const __LES##PARAM_TYPE##Type__##NUMBER = #TYPE; \
	const LES_Hash __LES##PARAM_TYPE##TypeHash__##NUMBER = LES_GenerateHashCaseSensitive(#TYPE); \
	const char* const __LES##PARAM_TYPE##Name__##NUMBER = #NAME; \
	const LES_Hash __LES##PARAM_TYPE##NameHash__##NUMBER = LES_GenerateHashCaseSensitive(#NAME); \
	/* Check the parameter index */ \
	if (NUMBER != __LESfunctionCurrent##PARAM_TYPE##ParamIndex__) \
	{ \
		/* ERROR: parameter index doesn't match the expected index */ \
		fprintf(stderr, "ERROR:function '%s' : Wrong function ##PARAM_TYPE param index:%d expected:%d param:'%s'\n", __LESfunctionName__, \
						__LESfunctionCurrent##PARAM_TYPE##ParamIndex__, NUMBER, __LES##PARAM_TYPE##Name__##NUMBER ); \
		return; \
	} \
	const LES_FunctionParameter* const __LES##PARAM_TYPE##Parameter__##NUMBER = __LESfunctionDefinition->Get##PARAM_TYPE##Parameter(NUMBER); \
	/* Check the parameter index */ \
	if (__LES##PARAM_TYPE##Parameter__##NUMBER->index != __LESfunctionCurrentParamIndex__) \
	{ \
		/* ERROR: parameter index doesn't match the value stored in the definition file */ \
		fprintf(stderr, "ERROR:function '%s' : Wrong function param index:%d expected:%d param:'%s'\n", __LESfunctionName__, \
						__LESfunctionCurrent##PARAM_TYPE##ParamIndex__, __LES##PARAM_TYPE##Parameter__##NUMBER->index, __LES##PARAM_TYPE##Name__##NUMBER ); \
		return; \
	} \
	/* Check the parameter type */ \
	const LES_StringEntry* const __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER = LES_GetStringEntryForID(__LES##PARAM_TYPE##Parameter__##NUMBER->typeID); \
	if (__LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER == LES_NULL) \
	{ \
		/* ERROR: can't find the parameter type */ \
		fprintf(stderr, "ERROR:function '%s' : Can't find ##PARAM_TYPE parameter type for ID:%d '%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->typeID, __LES##PARAM_TYPE##Type__##NUMBER); \
		return; \
	} \
	/* Check the parameter type : hash */ \
	if (__LES##PARAM_TYPE##TypeHash__##NUMBER != __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->hash) \
	{ \
		/* ERROR : parameter type hash doesn't match */ \
		fprintf(stderr, "ERROR:function '%s' : ##PARAM_TYPE parameter type hash doesn't match for ID:%d 0x%X != 0x%X '%s':'%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->typeID, \
				__LES##PARAM_TYPE##TypeHash__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->hash, \
				__LES##PARAM_TYPE##Type__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->str); \
		return; \
	} \
	/* Check the parameter type : string */ \
	if (strcmp(__LES##PARAM_TYPE##Type__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->str) != 0) \
	{ \
		/* ERROR : parameter type string doesn't match */ \
		fprintf(stderr, "ERROR:function '%s' : parameter type string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->typeID, \
				__LES##PARAM_TYPE##Type__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->str, \
				__LES##PARAM_TYPE##TypeHash__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->hash ); \
		return; \
	} \
	/* Check the parameter name */ \
	const LES_StringEntry* const __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER = LES_GetStringEntryForID(__LES##PARAM_TYPE##Parameter__##NUMBER->nameID); \
	if (__LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER == LES_NULL) \
	{ \
		/* ERROR: can't find the parameter name */ \
		fprintf(stderr, "ERROR:function '%s' : Can't find parameter name for ID:%d '%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->nameID, __LES##PARAM_TYPE##Name__##NUMBER); \
		return; \
	} \
	/* Check the parameter name : hash */ \
	if (__LES##PARAM_TYPE##NameHash__##NUMBER != __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->hash) \
	{ \
		/* ERROR : parameter name hash doesn't match */ \
		fprintf(stderr, "ERROR:function '%s' : parameter name hash doesn't match for ID:%d 0x%X != 0x%X '%s':'%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->nameID, \
				__LES##PARAM_TYPE##NameHash__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->hash, \
				__LES##PARAM_TYPE##Name__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->str); \
		return; \
	} \
	/* Check the parameter name : string */ \
	if (strcmp(__LES##PARAM_TYPE##Name__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->str) != 0) \
	{ \
		/* ERROR : parameter name string doesn't match */ \
		fprintf(stderr, "ERROR:function '%s' : parameter name string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->nameID, \
				__LES##PARAM_TYPE##Name__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->str, \
				__LES##PARAM_TYPE##NameHash__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->hash ); \
		return; \
	} \
	/* Store the parameter value */ \
	__LESfunctionParamData->AddParam(__LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER, (void*)&NAME); \
	\
	/* Update param indecies */ \
	__LESfunctionCurrent##PARAM_TYPE##ParamIndex__ += 1; \
	__LESfunctionCurrentParamIndex__ += 1; \



#define LES_FUNCTION_ADD_INPUT(INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Input, INPUT_NUMBER, INPUT_TYPE, INPUT_NAME)

#define LES_FUNCTION_ADD_OUTPUT(OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Output, OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME)

#define LES_FUNCTION_START(FUNC_NAME, RETURN_TYPE) \
	const char* const __LESfunctionName__ = #FUNC_NAME; \
	const char* const __LESfunctionReturnType__ = #RETURN_TYPE; \
	const LES_FunctionDefinition* const __LESfunctionDefinition = LES_GetFunctionDefinition(__LESfunctionName__); \
	if (__LESfunctionDefinition == LES_NULL) \
	{ \
		/* ERROR: function not found */ \
		fprintf(stderr, "ERROR:function '%s' : Can't find function definition\n", __LESfunctionName__); \
		return; \
	} \
	const LES_StringEntry* const __LESfunctionReturnTypeStringEntry = LES_GetStringEntryForID(__LESfunctionDefinition->returnTypeID); \
	if (__LESfunctionReturnTypeStringEntry == LES_NULL) \
	{ \
		/* ERROR: return type not found */ \
		fprintf(stderr, "ERROR:function '%s' : Can't find function return type for ID:%d '%s'\n", __LESfunctionName__, \
				__LESfunctionDefinition->returnTypeID, __LESfunctionReturnType__); \
		return; \
	} \
	LES_FunctionParamData* const __LESfunctionParamData = LES_GetFunctionParamData(__LESfunctionDefinition->nameID); \
	/* Initialise param indecies */ \
	int __LESfunctionCurrentInputParamIndex__ = 0; \
	int __LESfunctionCurrentOutputParamIndex__ = 0; \
	int __LESfunctionCurrentParamIndex__ = 0; \
	__LESfunctionCurrentInputParamIndex__ += 0; \
	__LESfunctionCurrentOutputParamIndex__ += 0; \
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
	int r;
	LES_FUNCTION_START(sceNpInit, void)
	LES_FUNCTION_INPUTS_1(int, a)
	LES_FUNCTION_OUTPUTS_1(int, r)
}
