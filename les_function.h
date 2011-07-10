#ifndef LES_FUNCTION_HH
#define LES_FUNCTION_HH

#include <stdio.h>

#include "les_core.h"
#include "les_hash.h"

#define LES_PARAM_MODE_INPUT 	(1<<0)
#define LES_PARAM_MODE_OUTPUT (1<<1)

#define LES_TYPE_INPUT 				(LES_PARAM_MODE_INPUT)
#define LES_TYPE_OUTPUT 			(LES_PARAM_MODE_OUTPUT)
#define LES_TYPE_INPUT_OUTPUT (LES_TYPE_INPUT|LES_TYPE_OUTPUT)
#define LES_TYPE_POD 					(1 << 2)
#define LES_TYPE_POINTER 			(1 << 3)

#define LES_MAX_NUM_FUNCTION_PARAMS (32)

struct LES_FunctionTempData;

struct LES_FunctionParameter
{
	LES_Hash m_hash;
	int m_nameID;
	int m_typeID;

	int m_index;

	int m_mode;
};

class LES_FunctionDefinition
{
public:
	LES_FunctionDefinition(void);
	LES_FunctionDefinition(const int nameID, const int returnTypeID, const int numInputs, const int numOutputs);

	void SetParamDataSize(const int paramDataSize);
	void SetReturnTypeID(const int returnTypeID);

	const LES_FunctionParameter* GetParameter(const LES_Hash hash) const;
	const LES_FunctionParameter* GetParameterByIndex(const int index) const;
	int GetNumParameters(void) const;
	int GetNumInputs(void) const;
	int GetNumOutputs(void) const;
	int GetReturnTypeID(void) const;
	int GetNameID(void) const;
	int GetParamDataSize(void) const;

	friend int LES_FunctionStart(const char* const name, const char* const returnType, 
															 const LES_FunctionDefinition** functionDefinitionPtr,
															 LES_FunctionTempData* const functionTempData);

private:
	int m_nameID;
	int m_returnTypeID;
	int m_paramDataSize;

	int m_numInputs;
	int m_numOutputs;
	//const LES_FunctionParameter* const m_params; - FOR NOW DO PROPER ASSIGNMENT IN CONSTRUCTOR OR PLACEMENT NEW
	const LES_FunctionParameter* m_params;
};

class LES_FunctionParamData
{
public:
	LES_FunctionParamData(char* const bufferPtr);
	~LES_FunctionParamData();

	int AddParamData(const LES_StringEntry* const typeStringEntry, const void* const paramDataPtr, const unsigned int paramMode);
	int GetParamData(const LES_StringEntry* const typeStringEntry, void* const paramDataPtr, const unsigned int paramMode) const;

private:
	char* const m_bufferPtr;
	char* m_currentWriteBufferPtr;
	mutable char* m_currentReadBufferPtr;
};

const LES_FunctionDefinition* LES_GetFunctionDefinition(const char* const name);
LES_FunctionParamData* LES_GetFunctionParamData(const int functionNameID);

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

	LES_FunctionParamData* functionParamData;
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
													&__LESfunctionTempData) == LES_ERROR) \
		{ \
			fprintf(stderr, "LES ERROR: '%s' : Error during LES_FunctionStart\n", #FUNC_NAME); \
			__LES_ok = false; \
		} \


extern int LES_FunctionEnd(const LES_FunctionDefinition* const functionDefinitionPtr,
													 const LES_FunctionTempData* const functionTempData);

#define LES_FUNCTION_END() \
		if (__LES_ok == true) \
		{ \
			if (LES_FunctionEnd(__LESfunctionDefinition, &__LESfunctionTempData) == LES_ERROR) \
			{ \
				fprintf(stderr, "LES ERROR: '%s' : Error during LES_FunctionEnd\n", __LESfunctionTempData.functionName); \
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
						__LESfunctionDefinition, &__LESfunctionTempData) == LES_ERROR) \
			{ \
				fprintf(stderr, "LES ERROR: '%s' : Error adding " #PARAM_TYPE " parameter %d '%s' type:'%s'\n", \
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


#endif //#ifndef LES_CORE_HH
