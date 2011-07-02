#ifndef LES_CORE_HH
#define LES_CORE_HH

typedef unsigned int LES_uint;

#define LES_NULL (0)

struct LES_StringEntry;

struct LES_FunctionParameter
{
	int nameID;
	int typeID;

	int index;
};

struct LES_FunctionDefinition
{
	int nameID;
	int returnTypeID;

	int numInputs;
	LES_FunctionParameter* const inputs;
	int numOutputs;
	LES_FunctionParameter* const outputs;
	LES_FunctionParameter* GetInputParameter(const int index) const;
	LES_FunctionParameter* GetOutputParameter(const int index) const;
};

struct LES_FunctionParamData
{
	int AddParam(const LES_StringEntry* const typeStringEntry, const void* const paramPtr);
};

LES_StringEntry* LES_GetStringEntryForID(const int id);

LES_FunctionDefinition* LES_GetFunctionDefinition(const char* const name);
LES_FunctionParamData* LES_GetFunctionParamData(const int functionNameID);

#endif //#ifndef LES_CORE_HH
