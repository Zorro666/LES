#include <stdio.h>

#include "les_hash.h"

typedef unsigned int LES_uint;

struct LES_String
{
	LES_Hash hash;
	char* const str;
};

LES_String* LES_GetStringForID(const int id)
{
	return NULL;
}

struct LES_FunctionParameter
{
	int nameID;
	int typeID;

	int index;
};

struct LES_FunctionDefinition
{
	int nameID;
	int returnTypeID;

	int numInputs;
	LES_FunctionParameter* const inputs;
	int numOutputs;
	LES_FunctionParameter* const outputs;
};

#define LES_FUNCTION_ADD_INPUT(INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \
	const char* const __LESinputType__##INPUT_NUMBER = #INPUT_TYPE; \
	const char* const __LESinputName__##INPUT_NUMBER = #INPUT_NAME;

#define LES_FUNCTION_ADD_OUTPUT(OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \
	const char* const __LESoutputType__##OUTPUT_NUMBER = #OUTPUT_TYPE; \
	const char* const __LESoutputName__##OUTPUT_NUMBER = #OUTPUT_NAME;

#define LES_FUNCTION_START(FUNC_NAME, RETURN_TYPE) \
	const char* const __LESfunctionName__ = #FUNC_NAME; \
	const char* const __LESfunctionReturnType__ = #RETURN_TYPE;

#define LES_FUNCTION_INPUTS_1(INPUT_0_TYPE, INPUT_0_NAME) \
	LES_FUNCTION_ADD_INPUT(0, INPUT_0_TYPE, INPUT_0_NAME)

#define LES_FUNCTION_OUTPUTS_1(OUTPUT_0_TYPE, OUTPUT_0_NAME) \
	LES_FUNCTION_ADD_OUTPUT(0, OUTPUT_0_TYPE, OUTPUT_0_NAME)

#define LES_FUNCTION_END() \
	)

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
