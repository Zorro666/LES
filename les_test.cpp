#include "les_test.h"
#include "les_core.h"

extern int LES_AddStringEntry(const char* const str);
extern int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const functionDefinitionPtr);

void LES_TestSetup(void)
{
	LES_FunctionDefinition functionDefinition;
	LES_FunctionParameter* functionParameterPtr;

	functionDefinition.m_nameID = LES_AddStringEntry("sceNpInit");
	functionDefinition.m_returnTypeID = LES_AddStringEntry("void");
	functionDefinition.m_paramDataSize = sizeof(int);

	functionDefinition.m_numInputs = 1;
	functionDefinition.m_inputs = new LES_FunctionParameter[functionDefinition.m_numInputs];

	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_inputs[0];
	functionParameterPtr->m_index = 0;
	functionParameterPtr->m_nameID = LES_AddStringEntry("a");
	functionParameterPtr->m_typeID = LES_AddStringEntry("int");

	functionDefinition.m_numOutputs = 1;
	functionDefinition.m_outputs = new LES_FunctionParameter[functionDefinition.m_numOutputs];

	functionParameterPtr = (LES_FunctionParameter* const)&functionDefinition.m_outputs[0];
	functionParameterPtr->m_index = 1;
	functionParameterPtr->m_nameID = LES_AddStringEntry("r");
	functionParameterPtr->m_typeID = LES_AddStringEntry("short");

	LES_AddFunctionDefinition("sceNpInit", &functionDefinition);
}

