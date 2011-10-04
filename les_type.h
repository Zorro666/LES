#ifndef LES_TYPE_HH
#define LES_TYPE_HH

#include "les_base.h"
#include "les_hash.h"

struct LES_StringEntry;
class LES_LoggerChannel;

#define LES_TYPE_INPUT 				(1<< 0)
#define LES_TYPE_OUTPUT 			(1<< 1)
#define LES_TYPE_INPUT_OUTPUT (LES_TYPE_INPUT|LES_TYPE_OUTPUT)
#define LES_TYPE_POD 					(1 << 2)
#define LES_TYPE_STRUCT  			(1 << 3)
#define LES_TYPE_POINTER 			(1 << 4)
#define LES_TYPE_REFERENCE 		(1 << 5)
#define LES_TYPE_ALIAS 				(1 << 6)
#define LES_TYPE_ARRAY 				(1 << 7)
#define LES_TYPE_ENDIANSWAP		(1 << 8)

struct LES_TypeEntry
{
	LES_uint32 m_hash;
	LES_uint32 m_dataSize;
	LES_uint32 m_flags;
	LES_int32 m_aliasedTypeID;
	LES_int32 m_numElements;

public:
	int ComputeDataStorageSize(void) const;
	int ComputeAlignment(void) const;
	const LES_TypeEntry* GetRootType(void) const;

public:
	static LES_Hash s_longlongHash;
	static LES_Hash s_intHash;
	static LES_Hash s_shortHash;
	static LES_Hash s_charHash;

	static LES_Hash s_uintHash;
	static LES_Hash s_ushortHash;
	static LES_Hash s_ucharHash;

	static LES_Hash s_floatHash;
};

const LES_TypeEntry* LES_GetTypeEntry(const LES_StringEntry* const typeStringEntry);
void LES_DebugOutputTypes(LES_LoggerChannel* const pLogChannel);
void LES_Type_DecodeFlags(char* const flagsDecoded, const LES_uint flags);

#endif // #ifndef LES_TYPE_HH
