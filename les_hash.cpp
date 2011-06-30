#include "les_hash.h"
#include "les_core.h"

LES_Hash LES_GenerateHash(const char* const str)
{
	const char* read;
	char chr;
	LES_uint hash = 0;

	if (str == LES_NULL)
	{
		return LES_INVALID_HASH;
	}

	read = str;
	const char lowerDelta = ('a' - 'A');

	while ((chr = *read) != 0)
	{
		if ((chr >= 'A') && (chr <= 'Z'))
		{
			// convert to lower case
			chr = (char)(chr + lowerDelta);
		}

		if (chr == '\\')
		{
			// convert back to forward slashes
			chr = '/';
		}

		hash += (hash << 7); // hash=hash+128*hash
		hash += (hash << 1); // hash=hash+128*hash+2*hash=131*hash
		hash += (LES_Hash)chr; // hash=131*hash+char
		read++;
	}

	return hash;
}

LES_Hash LES_GenerateHashCaseSensitive(const char* const str)
{
	const char* read;
	char chr;
	LES_Hash hash = 0;

	if (str == LES_NULL)
	{
		return LES_INVALID_HASH;
	}

	read = str;

	while ((chr = *read) != 0)
	{
		hash += (hash << 7); // hash=hash+128*hash
		hash += (hash << 1); // hash=hash+128*hash+2*hash=131*hash
		hash += (LES_Hash)chr; // hash=131*hash+char
		read++;
	}

	return hash;
}

