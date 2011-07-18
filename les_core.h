#ifndef LES_CORE_HH
#define LES_CORE_HH

#include "les_base.h"

struct LES_StringEntry;

const LES_StringEntry* LES_GetStringEntryForID(const int id);
const LES_StringEntry* LES_GetStringEntry(const char* const str);

void LES_Init(void);
void LES_Shutdown(void);

#endif //#ifndef LES_CORE_HH
