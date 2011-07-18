#ifndef LES_FUNCTION_HH
#define LES_FUNCTION_HH

#include "les_base.h"
#include "les_hash.h"
#include "les_type.h"

#define LES_MAX_NUM_FUNCTION_PARAMS (32)

#define LES_PARAM_MODE_INPUT 	(LES_TYPE_INPUT)
#define LES_PARAM_MODE_OUTPUT (LES_TYPE_OUTPUT)

struct LES_FunctionTempData;
struct LES_FunctionParameter;
class LES_FunctionParameterData;

class LES_FunctionDefinition
{
public:
	LES_FunctionDefinition(void);
	LES_FunctionDefinition(const LES_FunctionDefinition& other);
	LES_FunctionDefinition& operator = (const LES_FunctionDefinition& other);
	LES_FunctionDefinition(const int nameID, const int returnTypeID, const int numInputs, const int numOutputs);
	~LES_FunctionDefinition(void);

	void ComputeParameterDataSize(void);
	void SetReturnTypeID(const int returnTypeID);

	const LES_FunctionParameter* GetParameter(const LES_Hash nameHash) const;
	const LES_FunctionParameter* GetParameterByIndex(const int index) const;
	int GetNumParameters(void) const;
	int GetNumInputs(void) const;
	int GetNumOutputs(void) const;
	int GetReturnTypeID(void) const;
	int GetNameID(void) const;
	int GetParameterDataSize(void) const;

	int Decode(const LES_FunctionParameterData* const functionParameterData) const;
private:
	int m_nameID;
	int m_returnTypeID;
	int m_parameterDataSize;

	int m_numInputs;
	int m_numOutputs;
	//const LES_FunctionParameter* const m_params; - FOR NOW DO PROPER ASSIGNMENT IN CONSTRUCTOR OR PLACEMENT NEW
	const LES_FunctionParameter* m_params;
	mutable bool m_ownsParamsMemory;
};

const LES_FunctionDefinition* LES_GetFunctionDefinition(const char* const name);
LES_FunctionParameterData* LES_GetFunctionParameterData(const int functionNameID);

#endif //#ifndef LES_FUNCTION_HH
