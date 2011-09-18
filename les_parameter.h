#ifndef LES_PARAMETER_HH
#define LES_PARAMETER_HH

#include "les_base.h"
#include "les_hash.h"

struct LES_StringEntry;
struct LES_TypeEntry;

struct LES_FunctionParameter
{
	LES_uint32 m_hash;
	LES_int32 m_nameID;
	LES_int32 m_typeID;
	LES_int32 m_index;
	LES_uint32 m_mode;
};

class LES_FunctionParameterData
{
public:
	LES_FunctionParameterData(char* const bufferPtr);
	~LES_FunctionParameterData();

	int Write(const LES_StringEntry* const typeStringEntry, const void* const parameterDataPtr, const unsigned int paramMode);
	int Read(const LES_StringEntry* const typeStringEntry, void* const parameterDataPtr) const;

	int GetNumBytesWritten(void) const;
	const char* GetBufferPtr(void) const { return m_bufferPtr; }
private:

	int WriteInternal(const LES_StringEntry* const typeStringEntry, const LES_TypeEntry* const rawTypeEntryPtr, 
										const void* const parameterDataPtr);
	int WriteItem(const LES_StringEntry* const typeStringEntry, const LES_TypeEntry* const rawTypeEntryPtr, 
								const void* const parameterDataPtr);

	char* const m_bufferPtr;
	char* m_currentWriteBufferPtr;
	mutable char* m_currentReadBufferPtr;
};

#endif // #ifndef LES_PARAMETER_HH
