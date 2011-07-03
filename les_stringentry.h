#ifndef LES_STRINGENTRY_HH
#define LES_STRINGENTRY_HH

#include "les_hash.h"

struct LES_StringEntry
{
	LES_Hash m_hash;
	char* const m_str;
};

#endif // #ifndef LES_STRINGENTRY_HH

