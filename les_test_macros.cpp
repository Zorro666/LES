#include "les_test_macros.h"
#include "les_function.h"
#include "les_parameter.h"
#include "les_stringentry.h"

extern int LES_AddStringEntry(const char* const str);
extern int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const functionDefinitionPtr);

bool LES_TestFunctionStart(const char* const functionName, const char* const returnType, const int numInputs, const int numOutputs,
													 LES_FunctionDefinition* const functionDefinitionPtr, LES_TEST_FUNCTION_DATA* testFunctionDataPtr)
{
	testFunctionDataPtr->functionName = functionName;
	testFunctionDataPtr->globalParamIndex = 0;
	testFunctionDataPtr->inputParamIndex = 0;
	testFunctionDataPtr->outputParamIndex = 0;

	const int nameID = LES_AddStringEntry(functionName);
	const int returnTypeID = LES_AddStringEntry(returnType);
	LES_FunctionDefinition functionDefinition(nameID, returnTypeID, numInputs, numOutputs);
	*functionDefinitionPtr = functionDefinition;

	return true;
}

bool LES_TestFunctionAddParam(const bool isInput, const char* const type, const char* const name, 
															LES_FunctionDefinition* const functionDefinitionPtr, LES_TEST_FUNCTION_DATA* testFunctionDataPtr)
{
	/* Error if parameter index off the end of the array */
	const int maxNumParam = (isInput ? functionDefinitionPtr->GetNumInputs() : functionDefinitionPtr->GetNumOutputs());
	int* const paramIndex = (isInput ? &testFunctionDataPtr->inputParamIndex : &testFunctionDataPtr->outputParamIndex);
	const char* const paramModeStr = (isInput ? "Input" : "Output");
	const int paramMode = (isInput ? LES_PARAM_MODE_INPUT : LES_PARAM_MODE_OUTPUT);
	if (*paramIndex >= maxNumParam)
	{
		fprintf(stderr, "LES ERROR: TEST function '%s' : %s ParamIndex too big index:%d max:%d parameter:'%s' type:'%s'\n",
						testFunctionDataPtr->functionName, paramModeStr, *paramIndex, maxNumParam, name, type);
		return false;
	}
	const LES_Hash nameHash = LES_GenerateHashCaseSensitive(name);
	/* Test to see if the parameter has already been added */
	if (functionDefinitionPtr->GetParameter(nameHash) != LES_NULL)
	{
		fprintf(stderr, "LES ERROR: TEST function '%s' : parameter '%s' already exists type:'%s'\n",
						testFunctionDataPtr->functionName, name, type);
		return false;
	}
	const int typeID = LES_AddStringEntry(type);
	const LES_StringEntry* const typeStringEntry = LES_GetStringEntryForID(typeID);
	const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntry);
	if (typeEntryPtr != LES_NULL)
	{
		const int typeDataStorageSize = typeEntryPtr->ComputeDataStorageSize();
		if (typeDataStorageSize < 0)
		{
			fprintf(stderr, "LES ERROR: TEST function '%s' : parameter '%s' type:'%s' invalid typeDataStorageSize\n",
							testFunctionDataPtr->functionName, name, type);
				return false;
		}
	}
	const int globalParamIndex = testFunctionDataPtr->globalParamIndex;
	LES_FunctionParameter* const functionParameterPtr = (LES_FunctionParameter* const)(functionDefinitionPtr->GetParameterByIndex(globalParamIndex));
	functionParameterPtr->m_index = testFunctionDataPtr->globalParamIndex;
	functionParameterPtr->m_hash = nameHash;
	functionParameterPtr->m_nameID = LES_AddStringEntry(name);
	functionParameterPtr->m_typeID = typeID;
	functionParameterPtr->m_mode = paramMode;
	testFunctionDataPtr->globalParamIndex++;
	*paramIndex = *paramIndex + 1;

	return true;
}

void LES_TestFunctionStart(LES_FunctionDefinition* const functionDefinitionPtr, LES_TEST_FUNCTION_DATA* testFunctionDataPtr)
{
	if (testFunctionDataPtr->globalParamIndex != functionDefinitionPtr->GetNumParameters())
	{
		fprintf(stderr, "LES ERROR: TEST function '%s' : ERROR not the right number of parameters Added:%d Should be:%d\n",
						testFunctionDataPtr->functionName, testFunctionDataPtr->globalParamIndex, functionDefinitionPtr->GetNumParameters());
		return;
	}
	functionDefinitionPtr->ComputeParameterDataSize();
	LES_AddFunctionDefinition(testFunctionDataPtr->functionName, functionDefinitionPtr);
}

bool LES_TestStructStart(const char* const structName, const int numMembers, 
												 LES_StructDefinition* const structDefinitionPtr, LES_TEST_STRUCT_DATA* testStructDataPtr)
{
	testStructDataPtr->structName = structName;
	testStructDataPtr->globalMemberIndex = 0;
	testStructDataPtr->totalMemberSizeWithPadding = 0;

	const int nameID = LES_AddStringEntry(structName);
	LES_StructDefinition structDefinition(nameID, numMembers);
	*structDefinitionPtr = structDefinition;

	return true;
}
