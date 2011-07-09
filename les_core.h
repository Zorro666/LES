#ifndef LES_CORE_HH
#define LES_CORE_HH

typedef unsigned int LES_uint;

#define LES_NULL (0)

struct LES_StringEntry;

#define LES_OK (1)
#define LES_ERROR (-1)

const LES_StringEntry* LES_GetStringEntryForID(const int id);

void LES_Init(void);
void LES_Shutdown(void);

#endif //#ifndef LES_CORE_HH
