#ifndef LES_PARAMETER_HH
#define LES_PARAMETER_HH

#include "les_hash.h"

struct LES_FunctionParameter
{
	LES_Hash m_hash;
	int m_nameID;
	int m_typeID;

	int m_index;

	int m_mode;
};

class LES_FunctionParameterData
{
public:
	LES_FunctionParameterData(char* const bufferPtr);
	~LES_FunctionParameterData();

	int Write(const LES_StringEntry* const typeStringEntry, const void* const parameterDataPtr, const unsigned int paramMode);
	int Read(const LES_StringEntry* const typeStringEntry, void* const parameterDataPtr) const;

private:
	char* const m_bufferPtr;
	char* m_currentWriteBufferPtr;
	mutable char* m_currentReadBufferPtr;
};

#endif // #ifdef LES_PARAMETER_HH
