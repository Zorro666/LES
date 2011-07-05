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

#define LES_FUNCTION_ADD_PARAM(PARAM_TYPE, NUMBER, TYPE, NAME) \
	const LES_Hash __LES##PARAM_TYPE##TypeHash__##NUMBER = LES_GenerateHashCaseSensitive(#TYPE); \
	const LES_Hash __LES##PARAM_TYPE##NameHash__##NUMBER = LES_GenerateHashCaseSensitive(#NAME); \
	/* Check the parameter index to see if it exceeds the number of declared parameters */ \
	if (NUMBER >= __LESfunctionNum##PARAM_TYPE##s) \
	{ \
		/* ERROR: parameter index exceeds the number of declared parameters */ \
		fprintf(stderr, "LES ERROR: function '%s' : Too many " #PARAM_TYPE " parameters index:%d max:%d parameter:'%s' type:'%s'\n", __LESfunctionName__, \
						NUMBER, __LESfunctionNum##PARAM_TYPE##s, #NAME, #TYPE); \
		return; \
	} \
	/* Check the parameter index */ \
	if (NUMBER != __LESfunctionCurrent##PARAM_TYPE##ParamIndex__) \
	{ \
		/* ERROR: parameter index doesn't match the expected index */ \
		fprintf(stderr, "LES ERROR: function '%s' : Wrong " #PARAM_TYPE " parameter index:%d expected:%d parameter:'%s' type:'%s'\n", __LESfunctionName__, \
						__LESfunctionCurrent##PARAM_TYPE##ParamIndex__, NUMBER, #NAME, #TYPE); \
		return; \
	} \
	const LES_FunctionParameter* const __LES##PARAM_TYPE##Parameter__##NUMBER = __LESfunctionDefinition->Get##PARAM_TYPE##Parameter(NUMBER); \
	if (__LES##PARAM_TYPE##Parameter__##NUMBER == LES_NULL) \
	{ \
		/* ERROR: parameter index entry not found */ \
		fprintf(stderr, "LES ERROR: function '%s' : " #PARAM_TYPE " parameter index:%d parameter:'%s' type:'%s' is NULL\n", __LESfunctionName__, \
						NUMBER, #NAME, #TYPE); \
		return; \
	} \
	/* Check the parameter index */ \
	if (__LES##PARAM_TYPE##Parameter__##NUMBER->m_index != __LESfunctionCurrentParamIndex__) \
	{ \
		/* ERROR: parameter index doesn't match the value stored in the definition file */ \
		fprintf(stderr, "LES ERROR: function '%s' : Wrong function parameter index:%d expected:%d " #PARAM_TYPE " parameter:'%s' type:'%s'\n", __LESfunctionName__, \
						__LESfunctionCurrentParamIndex__, __LES##PARAM_TYPE##Parameter__##NUMBER->m_index, #NAME, #TYPE); \
		return; \
	} \
	/* Check the parameter type */ \
	const LES_StringEntry* const __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER = LES_GetStringEntryForID(__LES##PARAM_TYPE##Parameter__##NUMBER->m_typeID); \
	if (__LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER == LES_NULL) \
	{ \
		/* ERROR: can't find the parameter type */ \
		fprintf(stderr, "LES ERROR: function '%s' : Can't find " #PARAM_TYPE " parameter type for ID:%d parameter:'%s' type:'%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_typeID, #NAME, #TYPE); \
		return; \
	} \
	/* Check the parameter type : hash */ \
	if (__LES##PARAM_TYPE##TypeHash__##NUMBER != __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_hash) \
	{ \
		/* ERROR: parameter type hash doesn't match */ \
		fprintf(stderr, "LES ERROR: function '%s' : parameter:%d '%s' (" #PARAM_TYPE ") type hash doesn't match for ID:%d 0x%X != 0x%X Got '%s' Expected '%s'\n", __LESfunctionName__, \
				__LESfunctionCurrentParamIndex__, \
				#NAME, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_typeID, \
				__LES##PARAM_TYPE##TypeHash__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_hash, \
				#TYPE, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_str); \
		return; \
	} \
	/* Check the parameter type : string */ \
	if (strcmp(#TYPE, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_str) != 0) \
	{ \
		/* ERROR: parameter type string doesn't match */ \
		fprintf(stderr, "LES ERROR: function '%s' : parameter type string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_typeID, \
				#TYPE, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_str, \
				__LES##PARAM_TYPE##TypeHash__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_hash ); \
		return; \
	} \
	/* Check the parameter name */ \
	const LES_StringEntry* const __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER = LES_GetStringEntryForID(__LES##PARAM_TYPE##Parameter__##NUMBER->m_nameID); \
	if (__LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER == LES_NULL) \
	{ \
		/* ERROR: can't find the parameter name */ \
		fprintf(stderr, "LES ERROR: function '%s' : Can't find parameter name for ID:%d parameter:'%s' type:'%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_nameID, #NAME, #TYPE); \
		return; \
	} \
	/* Check the parameter name : hash */ \
	if (__LES##PARAM_TYPE##NameHash__##NUMBER != __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_hash) \
	{ \
		/* ERROR: parameter name hash doesn't match */ \
		fprintf(stderr, "LES ERROR: function '%s' : parameter name hash doesn't match for ID:%d 0x%X != 0x%X '%s':'%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_nameID, \
				__LES##PARAM_TYPE##NameHash__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_hash, \
				#NAME, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_str); \
		return; \
	} \
	/* Check the parameter name : string */ \
	if (strcmp(#NAME, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_str) != 0) \
	{ \
		/* ERROR: parameter name string doesn't match */ \
		fprintf(stderr, "LES ERROR: function '%s' : parameter name string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_nameID, \
				#NAME, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_str, \
				__LES##PARAM_TYPE##NameHash__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_hash ); \
		return; \
	} \
	/* Store the parameter value */ \
	__LESfunctionParamData->AddParam(__LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER, (void*)&NAME); \
	\
	/* Update parameter indexes */ \
	__LESfunctionCurrent##PARAM_TYPE##ParamIndex__ += 1; \
	__LESfunctionCurrentParamIndex__ += 1; \


#define LES_FUNCTION_ADD_INPUT(INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Input, INPUT_NUMBER, INPUT_TYPE, INPUT_NAME) \


#define LES_FUNCTION_ADD_OUTPUT(OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \
	LES_FUNCTION_ADD_PARAM(Output, OUTPUT_NUMBER, OUTPUT_TYPE, OUTPUT_NAME) \


#define LES_FUNCTION_START(FUNC_NAME, RETURN_TYPE) \
	const char* const __LESfunctionName__ = #FUNC_NAME; \
	const char* const __LESfunctionReturnType__ = #RETURN_TYPE; \
	const LES_FunctionDefinition* const __LESfunctionDefinition = LES_GetFunctionDefinition(__LESfunctionName__); \
	if (__LESfunctionDefinition == LES_NULL) \
	{ \
		/* ERROR: function not found */ \
		fprintf(stderr, "LES ERROR: function '%s' : Can't find function definition\n", __LESfunctionName__); \
		return; \
	} \
	const LES_StringEntry* const __LESfunctionReturnTypeStringEntry = LES_GetStringEntryForID(__LESfunctionDefinition->m_returnTypeID); \
	if (__LESfunctionReturnTypeStringEntry == LES_NULL) \
	{ \
		/* ERROR: return type not found */ \
		fprintf(stderr, "LES ERROR: function '%s' : Can't find function return type for ID:%d '%s'\n", __LESfunctionName__, \
				__LESfunctionDefinition->m_returnTypeID, __LESfunctionReturnType__); \
		return; \
	} \
	LES_FunctionParamData* const __LESfunctionParamData = LES_GetFunctionParamData(__LESfunctionDefinition->m_nameID); \
	/* Cache the number of inputs and number of output parameters */ \
	const int __LESfunctionNumInputs = __LESfunctionDefinition->m_numInputs; \
	const int __LESfunctionNumOutputs = __LESfunctionDefinition->m_numOutputs; \
	/* Initialise parameter indexes */ \
	int __LESfunctionCurrentInputParamIndex__ = 0; \
	int __LESfunctionCurrentOutputParamIndex__ = 0; \
	int __LESfunctionCurrentParamIndex__ = 0; \
	__LESfunctionCurrentInputParamIndex__ += 0; \
	__LESfunctionCurrentOutputParamIndex__ += 0; \
	__LESfunctionCurrentParamIndex__ += 0; \


#define LES_FUNCTION_INPUTS_1(INPUT_0_TYPE, INPUT_0_NAME) \
	LES_FUNCTION_ADD_INPUT(0, INPUT_0_TYPE, INPUT_0_NAME) \

#define LES_FUNCTION_INPUTS_2(INPUT_0_TYPE, INPUT_0_NAME, INPUT_1_TYPE, INPUT_1_NAME) \
	LES_FUNCTION_ADD_INPUT(0, INPUT_0_TYPE, INPUT_0_NAME) \
	LES_FUNCTION_ADD_INPUT(1, INPUT_1_TYPE, INPUT_1_NAME) \


#define LES_FUNCTION_OUTPUTS_1(OUTPUT_0_TYPE, OUTPUT_0_NAME) \
	LES_FUNCTION_ADD_OUTPUT(0, OUTPUT_0_TYPE, OUTPUT_0_NAME) \


#define LES_FUNCTION_END() \
	JAKE_WAS_HERE \


#endif //#ifndef LES_CORE_HH
