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

extern int LES_FunctionAddParam( const char* const type, const char* const name, const int index, 
																 const char* const mode, const bool isInput, void* const data,
												  			 const LES_FunctionDefinition* const functionDefinition,
																 const char* const functionName, const int functionMaxParamTypeIndex,
																 LES_FunctionParamData* const functionParamData,
																 int* functionCurrentParamTypeIndexPtr,
																 int* functionCurrentParamIndexPtr);

#define LES_FUNCTION_ADD_PARAM(PARAM_TYPE, IS_INPUT, NUMBER, TYPE, NAME) \
	if (LES_FunctionAddParam(#TYPE, #NAME, NUMBER, #PARAM_TYPE, IS_INPUT, (void*)&NAME, \
													 __LESfunctionDefinition, \
													 __LESfunctionName__, \
													 __LESfunctionNum##PARAM_TYPE##s, \
													 __LESfunctionParamData, \
													 &__LESfunctionCurrent##PARAM_TYPE##ParamIndex__, \
													 &__LESfunctionCurrentParamIndex__ \
													 ) == LES_ERROR) \
	{ \
		fprintf(stderr, "LES ERROR: function '%s' : Error adding " #PARAM_TYPE " parameter %d '%s' type:'%s'\n", \
						__LESfunctionName__, NUMBER, #NAME, #TYPE); \
	} \


#define LES_FUNCTION_ADD_INPUT(INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Input, true, INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \


#define LES_FUNCTION_ADD_OUTPUT(OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Output, false, OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \


#define LES_FUNCTION_START(FUNC_NAME, RETURN_TYPE) \
{ \
	const char* const __LESfunctionName__ = #FUNC_NAME; \
	const LES_Hash __LESfunctionReturnTypeTypeHash = LES_GenerateHashCaseSensitive(#RETURN_TYPE); \
	const LES_FunctionDefinition* const __LESfunctionDefinition = LES_GetFunctionDefinition(__LESfunctionName__); \
	if (__LESfunctionDefinition == LES_NULL) \
	{ \
		/* ERROR: function not found */ \
		fprintf(stderr, "LES ERROR: function '%s' : Can't find function definition\n", __LESfunctionName__); \
		return; \
	} \
	const LES_StringEntry* const __LESfunctionReturnTypeStringEntry = LES_GetStringEntryForID(__LESfunctionDefinition->m_returnTypeID); \
	/* Check the return type : exists */ \
	if (__LESfunctionReturnTypeStringEntry == LES_NULL) \
	{ \
		/* ERROR: return type not found */ \
		fprintf(stderr, "LES ERROR: function '%s' : Can't find function return type for ID:%d '%s'\n", __LESfunctionName__, \
				__LESfunctionDefinition->m_returnTypeID, #RETURN_TYPE); \
		return; \
	} \
	/* Check the return type : hash */ \
	if (__LESfunctionReturnTypeTypeHash != __LESfunctionReturnTypeStringEntry->m_hash) \
	{ \
		/* ERROR: return type hash doesn't match function definition */ \
		fprintf(stderr, "LES_ERROR: function '%s' : Return type hash doesn't match function definition 0x%X != 0x%X Got:'%s' Expected:'%s'\n", \
						__LESfunctionName__, \
						__LESfunctionReturnTypeTypeHash, __LESfunctionReturnTypeStringEntry->m_hash, \
						#RETURN_TYPE, __LESfunctionReturnTypeStringEntry->m_str ); \
	} \
	/* Check the return type : string */ \
	if (strcmp(#RETURN_TYPE, __LESfunctionReturnTypeStringEntry->m_str) != 0)\
	{ \
		/* ERROR: return type string doesn't match function definition */ \
		fprintf(stderr, "LES_ERROR: function '%s' : Return type string doesn't match function definition '%s' != '%s'\n", \
						__LESfunctionName__, \
						#RETURN_TYPE, __LESfunctionReturnTypeStringEntry->m_str ); \
	} \
	LES_FunctionParamData* const __LESfunctionParamData = LES_GetFunctionParamData(__LESfunctionDefinition->m_nameID); \
	/* Cache the number of inputs and number of output parameters */ \
	const int __LESfunctionNumInputs = __LESfunctionDefinition->m_numInputs; \
	const int __LESfunctionNumOutputs = __LESfunctionDefinition->m_numOutputs; \
	/* Initialise parameter indexes */ \
	int __LESfunctionCurrentInputParamIndex__ = 0; \
	int __LESfunctionCurrentOutputParamIndex__ = 0; \
	int __LESfunctionCurrentParamIndex__ = 0; \
	__LESfunctionCurrentInputParamIndex__ = __LESfunctionNumInputs; \
	__LESfunctionCurrentOutputParamIndex__ = __LESfunctionNumOutputs; \
	__LESfunctionCurrentParamIndex__ = (unsigned int)__LESfunctionParamData; \
	__LESfunctionCurrentInputParamIndex__ = 0; \
	__LESfunctionCurrentOutputParamIndex__ = 0; \
	__LESfunctionCurrentParamIndex__ = 0; \


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
