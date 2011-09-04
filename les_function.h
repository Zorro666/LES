#ifndef LES_FUNCTION_HH
#define LES_FUNCTION_HH

#include "les_base.h"
#include "les_hash.h"
#include "les_type.h"
#include "les_parameter.h"

#define LES_MAX_NUM_FUNCTION_PARAMS (32)

#define LES_PARAM_MODE_INPUT 	(LES_TYPE_INPUT)
#define LES_PARAM_MODE_OUTPUT (LES_TYPE_OUTPUT)

struct LES_FunctionTempData;
struct LES_FunctionParameter;
class LES_FunctionParameterData;

class LES_FunctionDefinition
{
public:
	int ComputeParameterDataSize(void) const;

	const LES_FunctionParameter* GetParameter(const LES_Hash nameHash) const;
	const LES_FunctionParameter* GetParameterByIndex(const int index) const;
	int GetNumParameters(void) const;
	int GetNumInputs(void) const;
	int GetNumOutputs(void) const;
	int GetReturnTypeID(void) const;
	int GetNameID(void) const;
	int GetParameterDataSize(void) const;

	int Decode(const LES_FunctionParameterData* const functionParameterData) const;

	friend LES_FunctionDefinition* LES_CreateFunctionDefinition(const int nameID, const int returnTypeID, 
																															const int numInputs, const int numOutputs);
	friend int LES_AddFunctionDefinition(const char* const name, const LES_FunctionDefinition* const pFunctionDefinition, 
																			 const int parameterDataSize);
	friend void LES_TestSetup(void);
private:
	LES_FunctionDefinition(void);
	LES_FunctionDefinition(const LES_FunctionDefinition& other);
	LES_FunctionDefinition& operator = (const LES_FunctionDefinition& other);
	~LES_FunctionDefinition(void);

	LES_int32 m_nameID;
	LES_int32 m_returnTypeID;
	LES_int32 m_parameterDataSize;

	LES_int32 m_numInputs;
	LES_int32 m_numOutputs;
	LES_FunctionParameter m_paramDatas[1];  // m_paramDatas[m_numInputs+m_numOutputs];
};

const LES_FunctionDefinition* LES_GetFunctionDefinition(const char* const name);
LES_FunctionParameterData* LES_GetFunctionParameterData(const int functionNameID);

#endif //#ifndef LES_FUNCTION_HH
