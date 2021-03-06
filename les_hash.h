#ifndef LES_HASH_HH
#define LES_HASH_HH

#include "les_base.h"

typedef LES_uint LES_Hash;

#define LES_INVALID_HASH (LES_Hash)(-1U);

LES_Hash LES_GenerateHash(const char* const str);
LES_Hash LES_GenerateHashCaseSensitive(const char* const str);

#endif // #ifndef LES_HASH_HH

