#ifndef LES_TYPE_HH
#define LES_TYPE_HH

#include "les_core.h"
#include "les_hash.h"

struct LES_StringEntry;

#define LES_TYPE_INPUT 				(1<< 0)
#define LES_TYPE_OUTPUT 			(1<< 1)
#define LES_TYPE_INPUT_OUTPUT (LES_TYPE_INPUT|LES_TYPE_OUTPUT)
#define LES_TYPE_POD 					(1 << 2)
#define LES_TYPE_STRUCT  			(1 << 3)
#define LES_TYPE_POINTER 			(1 << 4)

struct LES_TypeEntry
{
	LES_Hash m_hash;
	unsigned int m_dataSize;
	unsigned int m_flags;

public:
	static LES_Hash s_longlongHash;
	static LES_Hash s_intHash;
	static LES_Hash s_unsignedintPtrHash;
	static LES_Hash s_shortHash;
	static LES_Hash s_unsignedshortPtrHash;
	static LES_Hash s_charHash;
	static LES_Hash s_unsignedcharPtrHash;
	static LES_Hash s_floatHash;
	static LES_Hash s_floatPtrHash;
	static LES_Hash s_intPtrHash;
};

const LES_TypeEntry* LES_GetTypeEntry(const LES_StringEntry* const typeStringEntry);

#endif // #ifndef LES_TYPE_HH
