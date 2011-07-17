#include <string.h>

#include "les_function_macros.h"
#include "les_parameter.h"
#include "les_stringentry.h"

int LES_FunctionStart(const char* const name, const char* const returnType, 
											const LES_FunctionDefinition** functionDefinitionPtr,
											LES_FunctionTempData* const functionTempData)
{
	functionTempData->functionName = name;
	functionTempData->functionCurrentParamIndex = 0;
	functionTempData->functionCurrentInputIndex = 0;
	functionTempData->functionCurrentOutputIndex = 0;
	functionTempData->functionInputMacroParamIndex = 0;
	functionTempData->functionOutputMacroParamIndex = 0;
	functionTempData->functionParameterData = LES_NULL;
	memset(functionTempData->paramUsed, 0, sizeof(char)*LES_MAX_NUM_FUNCTION_PARAMS);

	const LES_Hash functionReturnTypeTypeHash = LES_GenerateHashCaseSensitive(returnType);
	const LES_FunctionDefinition* const functionDefinition = LES_GetFunctionDefinition(name);
	if (functionDefinition == LES_NULL)
	{
		/* ERROR: function not found */
		fprintf(stderr, "LES ERROR: '%s' : Can't find function definition\n", name);
		return LES_ERROR;
	}
	*functionDefinitionPtr = functionDefinition;

	const LES_StringEntry* const functionReturnTypeStringEntry = LES_GetStringEntryForID(functionDefinition->GetReturnTypeID());
	/* Check the return type : exists */
	if (functionReturnTypeStringEntry == LES_NULL)
	{
		/* ERROR: return type not found */
		fprintf(stderr, "LES ERROR: '%s' : Can't find function return type for ID:%d '%s'\n", 
						name, functionDefinition->GetReturnTypeID(), returnType);
		return LES_ERROR;
	}
	/* Check the return type : hash */
	if (functionReturnTypeTypeHash != functionReturnTypeStringEntry->m_hash)
	{
		/* ERROR: return type hash doesn't match function definition */
		fprintf(stderr, "LES_ERROR: '%s' : Return type hash doesn't match function definition 0x%X != 0x%X Code:'%s' Definition:'%s'\n",
						name, functionReturnTypeTypeHash, functionReturnTypeStringEntry->m_hash,
						returnType, functionReturnTypeStringEntry->m_str);
		return LES_ERROR;
	}
	/* Check the return type : string */
	if (strcmp(returnType, functionReturnTypeStringEntry->m_str) != 0)
	{
		/* ERROR: return type string doesn't match function definition */
		fprintf(stderr, "LES_ERROR: '%s' : Return type string doesn't match function definition '%s' != '%s'\n",
						name, returnType, functionReturnTypeStringEntry->m_str);
		return LES_ERROR;
	}
	LES_FunctionParameterData* const functionParameterData = LES_GetFunctionParameterData(functionDefinition->GetNameID());
	if (functionParameterData == LES_NULL)
	{
		/* ERROR: functionParameterData should only be LES_NULL if no inputs or outputs */
		if (functionDefinition->GetNumParameters() > 0)
		{
			fprintf(stderr, "LES_ERROR: '%s' : functionParameterData is NULL numInputs:%d numOutputs:%d nameID:%d\n",
							name, functionDefinition->GetNumInputs(),  functionDefinition->GetNumOutputs(), functionDefinition->GetNameID());
			return LES_ERROR;
		}
	}
	functionTempData->functionParameterData = functionParameterData;

	/* Initialise parameter indexes */
	functionTempData->functionCurrentParamIndex = 0;
	functionTempData->functionCurrentInputIndex = 0;
	functionTempData->functionCurrentOutputIndex = 0;

	return LES_OK;
}

int LES_FunctionAddParam(const char* const type, const char* const name, const int index, 
												 const char* const mode, const bool isInput, void* const data,
												 const LES_FunctionDefinition* const functionDefinition,
												 LES_FunctionTempData* const functionTempData)
{
	if (functionTempData == LES_NULL)
	{
		fprintf(stderr, "LES ERROR: '%s' : functionTempData is NULL parameter:'%s' type:'%s'\n",
						functionTempData->functionName, name, type);
		return LES_ERROR;
	}

	const LES_Hash typeHash = LES_GenerateHashCaseSensitive(type);
	const LES_Hash nameHash = LES_GenerateHashCaseSensitive(name);
	/* Check the parameter index to see if it exceeds the number of declared parameters */
	const int functionMaxParamTypeIndex = (isInput ? functionDefinition->GetNumInputs() : functionDefinition->GetNumOutputs());
	if (index >= functionMaxParamTypeIndex)
	{
		/* ERROR: parameter index exceeds the number of declared parameters */
		fprintf(stderr, "LES ERROR: '%s' : Too many %s parameters index:%d max:%d parameter:'%s' type:'%s'\n",
						functionTempData->functionName, mode, index, functionMaxParamTypeIndex, name, type);
		return LES_ERROR;
	}
	/* Check the parameter index */
	const int functionCurrentParamTypeIndex = (isInput ? functionTempData->functionCurrentInputIndex : functionTempData->functionCurrentOutputIndex);
	if (index != functionCurrentParamTypeIndex)
	{
		/* ERROR: parameter index doesn't match the expected index */
		fprintf(stderr, "LES ERROR: '%s' : Wrong %s parameter index:%d expected:%d parameter:'%s' type:'%s'\n",
						functionTempData->functionName, mode, functionCurrentParamTypeIndex, index, name, type);
		return LES_ERROR;
	}
	const LES_FunctionParameter* const functionParameterPtr = functionDefinition->GetParameter(nameHash);
	if (functionParameterPtr == LES_NULL)
	{
		/* ERROR: parameter index entry not found */
		fprintf(stderr, "LES ERROR: '%s' : %s parameter index:%d parameter:'%s' type:'%s' not found\n",
						functionTempData->functionName, mode, index, name, type);
		return LES_ERROR;
	}
	/* Check the parameter hasn't already been added */
	const int functionParameterIndex = functionParameterPtr->m_index;
	if ((functionParameterIndex < 0) || (functionParameterIndex >= LES_MAX_NUM_FUNCTION_PARAMS))
	{
		/* ERROR: invalid parameter index */
		fprintf(stderr, "LES ERROR: '%s' : %s parameter:'%s' type:'%s' invalid parameter index:%d\n",
						functionTempData->functionName, mode, name, type, functionParameterIndex);
		return LES_ERROR;
	}
	if (functionTempData->paramUsed[functionParameterIndex] == 1)
	{
		/* ERROR: parameter has already been added */
		fprintf(stderr, "LES ERROR: '%s' : %s parameter:'%s' type:'%s' has already been added param index:%d\n",
						functionTempData->functionName, mode, name, type, functionParameterIndex);
		return LES_ERROR;
	}

	/* Check the parameter mode: input or output */
	const int currentMode = (isInput ? LES_PARAM_MODE_INPUT : LES_PARAM_MODE_OUTPUT);
	if (functionParameterPtr->m_mode != currentMode)
	{
		/* ERROR: parameter mode doesn't match the definition */
		const char* const foundParamMode = (functionParameterPtr->m_mode == LES_PARAM_MODE_INPUT ? "Input" : "Output");
		fprintf(stderr, "LES ERROR: '%s' : %s parameter wrong mode Code:'%s' Definition:'%s'\n",
						 functionTempData->functionName, name, mode, foundParamMode);
		return LES_ERROR;
	}

	/* Check the parameter index */
	const int functionCurrentParamIndex = functionTempData->functionCurrentParamIndex;
	if (functionCurrentParamIndex != functionParameterIndex)
	{
		/* ERROR: parameter index doesn't match the value stored in the definition file */
		fprintf(stderr, "LES ERROR: '%s' : Wrong function parameter index:%d expected:%d %s parameter:'%s' type:'%s'\n", 
						functionTempData->functionName, functionCurrentParamIndex, functionParameterIndex, mode, name, type);
		return LES_ERROR;
	}
	/* Check the parameter type */
	const int functionParameterTypeID = functionParameterPtr->m_typeID;
	const LES_StringEntry* const parameterTypeStringEntry = LES_GetStringEntryForID(functionParameterTypeID);
	if (parameterTypeStringEntry == LES_NULL)
	{
		/* ERROR: can't find the parameter type */
		fprintf(stderr, "LES ERROR: '%s' : Can't find %s parameter type for ID:%d parameter:'%s' type:'%s'\n", 
						functionTempData->functionName, mode, functionParameterTypeID, name, type);
		return LES_ERROR;
	}
	/* Check the parameter type : hash */
	if (typeHash != parameterTypeStringEntry->m_hash)
	{
		/* ERROR: parameter type hash doesn't match */
		fprintf(stderr, "LES ERROR: '%s' : parameter:%d '%s' (%s) type hash doesn't match for ID:%d 0x%X != 0x%X Code:'%s' Definition:'%s'\n",
					  functionTempData->functionName, functionCurrentParamIndex, name, mode, functionParameterTypeID,
					  typeHash, parameterTypeStringEntry->m_hash,
					  type, parameterTypeStringEntry->m_str);
		return LES_ERROR;
	}
	/* Check the parameter type : string */
	if (strcmp(type, parameterTypeStringEntry->m_str) != 0)
	{
		/* ERROR: parameter type string doesn't match */
		fprintf(stderr, "LES ERROR: '%s' : Adding parameter '%s' type string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n",
						functionTempData->functionName, name, functionParameterTypeID,
						type, parameterTypeStringEntry->m_str,
						typeHash, parameterTypeStringEntry->m_hash);
		return LES_ERROR;
	}
	/* Check the parameter name */
	const int functionParameterNameID = functionParameterPtr->m_nameID;
	const LES_StringEntry* const parameterNameStringEntry = LES_GetStringEntryForID(functionParameterNameID);
	if (parameterNameStringEntry == LES_NULL)
	{
		/* ERROR: can't find the parameter name */
		fprintf(stderr, "LES ERROR: '%s' : Can't find parameter name for ID:%d parameter:'%s' type:'%s'\n",
						functionTempData->functionName, functionParameterNameID, name, type);
		return LES_ERROR;
	}
	/* Check the parameter name : hash */
	if (nameHash != parameterNameStringEntry->m_hash)
	{
		/* ERROR: parameter name hash doesn't match */
		fprintf(stderr, "LES ERROR: '%s' : Adding parameter '%s' name hash doesn't match for ID:%d 0x%X != 0x%X Code:'%s' Definition:'%s'\n",
						functionTempData->functionName, name, functionParameterNameID,
						nameHash, parameterNameStringEntry->m_hash,
						name, parameterNameStringEntry->m_str);
		return LES_ERROR;
	}
	/* Check the parameter name : string */
	if (strcmp(name, parameterNameStringEntry->m_str) != 0)
	{
		/* ERROR: parameter name string doesn't match */
		fprintf(stderr, "LES ERROR: '%s' : Adding parameter '%s' name string doesn't match for ID:%d '%s' != '%s' 0x%X : 0x%X\n", 
						functionTempData->functionName, name, functionParameterNameID,
						name, parameterNameStringEntry->m_str,
						nameHash, parameterNameStringEntry->m_hash);
		return LES_ERROR;
	}
	/* Store the parameter value */
	LES_FunctionParameterData* const functionParameterData = functionTempData->functionParameterData;
	if (functionParameterData == LES_NULL)
	{
		/* ERROR: functionParameterData is NULL */
		fprintf(stderr, "LES ERROR: '%s' : functionParameterData is NULL\n", functionTempData->functionName);
		return LES_ERROR;
	}
	if (functionParameterData->Write(parameterTypeStringEntry, data, functionParameterPtr->m_mode) == LES_ERROR)
	{
		/* ERROR: during AddParam */
		fprintf(stderr, "LES ERROR: '%s' : Write parameter:'%s' type:'%s' failed\n", functionTempData->functionName, name, type);
		return LES_ERROR;
	}

	/* Update parameter indexes */
	functionTempData->functionCurrentParamIndex++;
	if (isInput)
	{
		functionTempData->functionCurrentInputIndex++;
	}
	else
	{
		functionTempData->functionCurrentOutputIndex++;
	}
	functionTempData->paramUsed[functionParameterIndex] = 1;

	return LES_OK;
}

int LES_FunctionEnd(const LES_FunctionDefinition* const functionDefinitionPtr, 
										const LES_FunctionTempData* const functionTempData)
{
	const int numParams = functionDefinitionPtr->GetNumParameters();

	int numMissing = 0;
	for (int i = 0; i < numParams; i++)
	{
		if (functionTempData->paramUsed[i] == 0)
		{
			/* ERROR: not all parameters were set */
			const LES_FunctionParameter* const functionParameterPtr = functionDefinitionPtr->GetParameterByIndex(i);
			if (functionParameterPtr != LES_NULL)
			{
				const int nameID = functionParameterPtr->m_nameID;
				const LES_StringEntry* const nameEntryPtr = LES_GetStringEntryForID(nameID);
				const int typeID = functionParameterPtr->m_typeID;
				const LES_StringEntry* const typeEntryPtr = LES_GetStringEntryForID(typeID);
				if (nameEntryPtr && typeEntryPtr)
				{
					fprintf(stderr, "LES ERROR: '%s' : parameter index:%d not set mode:'%s' name:'%s' type:'%s'\n", 
									functionTempData->functionName, i, 
									((functionParameterPtr->m_mode == LES_PARAM_MODE_INPUT) ? "Input" : "Output"),
									nameEntryPtr->m_str, typeEntryPtr->m_str);
				}
			}
			else
			{
				fprintf(stderr, "LES ERROR: '%s' : parameter index:%d not set\n", functionTempData->functionName, i);
			}
			numMissing++;
		}
	}
	if (numMissing > 0)
	{
		fprintf(stderr, "LES ERROR: '%s' : not all parameters were set num missing:%d\n", functionTempData->functionName, numMissing);
		return LES_ERROR;
	}
	// Check the number of bytes written matched parameter data size
	LES_FunctionParameterData* const functionParameterData = functionTempData->functionParameterData;
	const int parameterDataSize = functionDefinitionPtr->GetParameterDataSize();
	const int numBytesWritten = functionParameterData->GetNumBytesWritten();
	if (parameterDataSize != numBytesWritten)
	{
		fprintf(stderr, "LES ERROR: '%s' : wrong number of bytes written NumBytesWritten:%d ParameterDataSize:%d\n", 
						functionTempData->functionName, numBytesWritten, parameterDataSize);
		return LES_ERROR;
	}
	return LES_OK;
}

int LES_FunctionGetParameterData(const LES_FunctionTempData* const functionTempData,
																 LES_FunctionParameterData** const functionParameterDataPtr)
{
	LES_FunctionParameterData* const functionParameterData = functionTempData->functionParameterData;
	if (functionParameterData == LES_NULL)
	{
		/* ERROR: functionParameterData is NULL */
		fprintf(stderr, "LES ERROR: '%s' : functionParameterData is NULL\n", functionTempData->functionName);
		return LES_ERROR;
	}
	*functionParameterDataPtr = functionParameterData;
	return LES_OK;
}
