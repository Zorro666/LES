#ifndef LES_TYPE_HH
#define LES_TYPE_HH

#include "les_hash.h"

struct LES_StringEntry;

#define LES_TYPE_INPUT 				(1<< 0)
#define LES_TYPE_OUTPUT 			(1<< 1)
#define LES_TYPE_INPUT_OUTPUT (LES_TYPE_INPUT|LES_TYPE_OUTPUT)
#define LES_TYPE_POD 					(1 << 2)
#define LES_TYPE_POINTER 			(1 << 3)

struct LES_TypeEntry
{
	LES_Hash m_hash;
	unsigned int m_dataSize;
	unsigned int m_flags;
};

const LES_TypeEntry* LES_GetTypeEntry(const LES_StringEntry* const typeStringEntry);

#endif // #ifndef LES_TYPE_HH
