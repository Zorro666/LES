#ifndef LES_CORE_HH
#define LES_CORE_HH

typedef unsigned int LES_uint;

#define LES_NULL (0)

struct LES_StringEntry;

#define LES_OK (1)
#define LES_ERROR (-1)

struct LES_FunctionParameter
{
	int m_nameID;
	int m_typeID;

	int m_index;
};

struct LES_FunctionDefinition
{
	int m_nameID;
	int m_returnTypeID;
	int m_paramDataSize;

	int m_numInputs;
	//const LES_FunctionParameter* const m_inputs; - FOR NOW DO PROPER ASSIGNMENT IN CONSTRUCTOR OR PLACEMENT NEW
	const LES_FunctionParameter* m_inputs;
	int m_numOutputs;
	//const LES_FunctionParameter* const m_outputs; - FOR NOW DO PROPER ASSIGNMENT IN CONSTRUCTOR OR PLACEMENT NEW
	const LES_FunctionParameter* m_outputs;

	const LES_FunctionParameter* GetInputParameter(const int index) const;
	const LES_FunctionParameter* GetOutputParameter(const int index) const;
};

struct LES_FunctionParamData
{
	char* m_bufferPtr;
	char* m_currentBufferPtr;
	int AddParam(const LES_StringEntry* const typeStringEntry, const void* const paramPtr);
};

const LES_StringEntry* LES_GetStringEntryForID(const int id);

void LES_Init(void);
void LES_Shutdown(void);

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

	LES_FunctionParamData* functionParamData;
};

extern int LES_FunctionStart( const char* const name, const char* const returnType, 
												  		const LES_FunctionDefinition** functionDefinitionPtr,
															LES_FunctionTempData* const functionTempData);

#define LES_FUNCTION_START(FUNC_NAME, RETURN_TYPE) \
{ \
	const LES_FunctionDefinition* __LESfunctionDefinition = LES_NULL; \
	LES_FunctionTempData __LESfunctionTempData; \
	if (LES_FunctionStart(#FUNC_NAME, #RETURN_TYPE, \
												&__LESfunctionDefinition, \
												&__LESfunctionTempData) == LES_ERROR) \
	{ \
		fprintf(stderr, "LES ERROR: function '%s' : Error during LES_FunctionStart\n", #FUNC_NAME); \
		return; \
	} \


extern int LES_FunctionAddParam( const char* const type, const char* const name, const int index, 
																 const char* const mode, const bool isInput, void* const data,
												  			 const LES_FunctionDefinition* const functionDefinition,
																 LES_FunctionTempData* const functionTempData);

#define LES_FUNCTION_ADD_PARAM(PARAM_TYPE, IS_INPUT, NUMBER, TYPE, NAME) \
	if (LES_FunctionAddParam(#TYPE, #NAME, NUMBER, #PARAM_TYPE, IS_INPUT, (void*)&NAME, \
													 __LESfunctionDefinition, &__LESfunctionTempData) == LES_ERROR) \
	{ \
		fprintf(stderr, "LES ERROR: function '%s' : Error adding " #PARAM_TYPE " parameter %d '%s' type:'%s'\n", \
						__LESfunctionTempData.functionName, NUMBER, #NAME, #TYPE); \
		return; \
	} \


#define LES_FUNCTION_ADD_INPUT(INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Input, true, INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \


#define LES_FUNCTION_ADD_OUTPUT(OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Output, false, OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \


#define LES_FUNCTION_INPUTS_1(INPUT_0_TYPE, INPUT_0_NAME) \
	LES_FUNCTION_ADD_INPUT(0, INPUT_0_TYPE, INPUT_0_NAME) \


#define LES_FUNCTION_INPUTS_2(INPUT_0_TYPE, INPUT_0_NAME, INPUT_1_TYPE, INPUT_1_NAME) \
	LES_FUNCTION_ADD_INPUT(0, INPUT_0_TYPE, INPUT_0_NAME) \
	LES_FUNCTION_ADD_INPUT(1, INPUT_1_TYPE, INPUT_1_NAME) \


#define LES_FUNCTION_OUTPUTS_1(OUTPUT_0_TYPE, OUTPUT_0_NAME) \
	LES_FUNCTION_ADD_OUTPUT(0, OUTPUT_0_TYPE, OUTPUT_0_NAME) \


#define LES_FUNCTION_END() \
} \


#endif //#ifndef LES_CORE_HH
