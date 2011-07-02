#include "les_core.h"
#include "les_hash.h"
#include "les_stringentry.h"

static LES_StringEntry* les_stringEntryArray = LES_NULL;
static int les_numStringEntries = 0;

void LES_Init(void)
{
	les_numStringEntries = 0;
	//les_stringEntryArray = new LES_StringEntry;
	float* jake = new float;
	jake[0] = 0;
}

LES_StringEntry* LES_GetStringEntryForID(const int id)
{
	if ((id < 0) || (id >= les_numStringEntries))
	{
		return LES_NULL;
	}

	LES_StringEntry* const stringEntry = &les_stringEntryArray[id];
	return stringEntry;
}


