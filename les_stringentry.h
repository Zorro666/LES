#ifndef LES_STRINGENTRY_HH
#define LES_STRINGENTRY_HH

#include "les_hash.h"

struct LES_StringEntry
{
	LES_Hash hash;
	char* const str;
};

#endif // #ifndef LES_STRINGENTRY_HH

