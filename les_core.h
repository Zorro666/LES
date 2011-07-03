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
	LES_FunctionParameter* const m_inputs;
	int m_numOutputs;
	LES_FunctionParameter* const m_outputs;

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
	const char* const __LES##PARAM_TYPE##Type__##NUMBER = #TYPE; \
	const LES_Hash __LES##PARAM_TYPE##TypeHash__##NUMBER = LES_GenerateHashCaseSensitive(#TYPE); \
	const char* const __LES##PARAM_TYPE##Name__##NUMBER = #NAME; \
	const LES_Hash __LES##PARAM_TYPE##NameHash__##NUMBER = LES_GenerateHashCaseSensitive(#NAME); \
	/* Check the parameter index */ \
	if (NUMBER != __LESfunctionCurrent##PARAM_TYPE##ParamIndex__) \
	{ \
		/* ERROR: parameter index doesn't match the expected index */ \
		fprintf(stderr, "LES ERROR: function '%s' : Wrong function ##PARAM_TYPE parameter index:%d expected:%d parameter:'%s'\n", __LESfunctionName__, \
						__LESfunctionCurrent##PARAM_TYPE##ParamIndex__, NUMBER, __LES##PARAM_TYPE##Name__##NUMBER ); \
		return; \
	} \
	const LES_FunctionParameter* const __LES##PARAM_TYPE##Parameter__##NUMBER = __LESfunctionDefinition->Get##PARAM_TYPE##Parameter(NUMBER); \
	/* Check the parameter index */ \
	if (__LES##PARAM_TYPE##Parameter__##NUMBER->m_index != __LESfunctionCurrentParamIndex__) \
	{ \
		/* ERROR: parameter index doesn't match the value stored in the definition file */ \
		fprintf(stderr, "LES ERROR: function '%s' : Wrong function parameter index:%d expected:%d parameter:'%s'\n", __LESfunctionName__, \
						__LESfunctionCurrent##PARAM_TYPE##ParamIndex__, __LES##PARAM_TYPE##Parameter__##NUMBER->m_index, __LES##PARAM_TYPE##Name__##NUMBER ); \
		return; \
	} \
	/* Check the parameter type */ \
	const LES_StringEntry* const __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER = LES_GetStringEntryForID(__LES##PARAM_TYPE##Parameter__##NUMBER->m_typeID); \
	if (__LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER == LES_NULL) \
	{ \
		/* ERROR: can't find the parameter type */ \
		fprintf(stderr, "LES ERROR: function '%s' : Can't find ##PARAM_TYPE parameter type for ID:%d '%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_typeID, __LES##PARAM_TYPE##Type__##NUMBER); \
		return; \
	} \
	/* Check the parameter type : hash */ \
	if (__LES##PARAM_TYPE##TypeHash__##NUMBER != __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_hash) \
	{ \
		/* ERROR: parameter type hash doesn't match */ \
		fprintf(stderr, "LES ERROR: function '%s' : ##PARAM_TYPE parameter type hash doesn't match for ID:%d 0x%X != 0x%X '%s':'%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_typeID, \
				__LES##PARAM_TYPE##TypeHash__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_hash, \
				__LES##PARAM_TYPE##Type__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_str); \
		return; \
	} \
	/* Check the parameter type : string */ \
	if (strcmp(__LES##PARAM_TYPE##Type__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_str) != 0) \
	{ \
		/* ERROR: parameter type string doesn't match */ \
		fprintf(stderr, "LES ERROR: function '%s' : parameter type string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_typeID, \
				__LES##PARAM_TYPE##Type__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_str, \
				__LES##PARAM_TYPE##TypeHash__##NUMBER, __LES##PARAM_TYPE##ParameterTypeStringEntry__##NUMBER->m_hash ); \
		return; \
	} \
	/* Check the parameter name */ \
	const LES_StringEntry* const __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER = LES_GetStringEntryForID(__LES##PARAM_TYPE##Parameter__##NUMBER->m_nameID); \
	if (__LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER == LES_NULL) \
	{ \
		/* ERROR: can't find the parameter name */ \
		fprintf(stderr, "LES ERROR: function '%s' : Can't find parameter name for ID:%d '%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_nameID, __LES##PARAM_TYPE##Name__##NUMBER); \
		return; \
	} \
	/* Check the parameter name : hash */ \
	if (__LES##PARAM_TYPE##NameHash__##NUMBER != __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_hash) \
	{ \
		/* ERROR: parameter name hash doesn't match */ \
		fprintf(stderr, "LES ERROR: function '%s' : parameter name hash doesn't match for ID:%d 0x%X != 0x%X '%s':'%s'\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_nameID, \
				__LES##PARAM_TYPE##NameHash__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_hash, \
				__LES##PARAM_TYPE##Name__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_str); \
		return; \
	} \
	/* Check the parameter name : string */ \
	if (strcmp(__LES##PARAM_TYPE##Name__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_str) != 0) \
	{ \
		/* ERROR: parameter name string doesn't match */ \
		fprintf(stderr, "LES ERROR: function '%s' : parameter name string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n", __LESfunctionName__, \
				__LES##PARAM_TYPE##Parameter__##NUMBER->m_nameID, \
				__LES##PARAM_TYPE##Name__##NUMBER, __LES##PARAM_TYPE##ParameterNameStringEntry__##NUMBER->m_str, \
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
	/* Initialise parameter indexes */ \
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
	JAKE_WAS_HERE \


#endif //#ifndef LES_CORE_HH
