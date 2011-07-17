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
#define LES_TYPE_REFERENCE 		(1 << 5)
#define LES_TYPE_ALIAS 				(1 << 6)

struct LES_TypeEntry
{
	LES_Hash m_hash;
	unsigned int m_dataSize;
	unsigned int m_flags;
	int m_aliasedTypeID;

public:
	int ComputeDataStorageSize(void) const;

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

#endif // #ifndef LES_TYPE_HH
