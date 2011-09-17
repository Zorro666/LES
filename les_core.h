#ifndef LES_CORE_HH
#define LES_CORE_HH

#include "les_base.h"

struct LES_StringEntry;
class LES_DefinitionFile;
class LES_LoggerChannel;

const LES_StringEntry* LES_GetStringEntryForID(const int id);
const LES_StringEntry* LES_GetStringEntry(const char* const str);
const LES_StringEntry* LES_GetStringEntryByHash(const unsigned int hash);
void LES_DebugOutputStringEntries(LES_LoggerChannel* const pLogChannel);

int LES_IsGlobalDefinitionFileValid(void);
int LES_SetGlobalDefinitionFile(const void* definitionFileData, const int fileDataSize);
void LES_DebugOutputGlobalDefinitionFile(LES_LoggerChannel* const pLogChannel);

void LES_Init(void);
void LES_Shutdown(void);

#endif //#ifndef LES_CORE_HH
