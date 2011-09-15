#ifndef LES_COREENGINE_HH
#define LES_COREENGINE_HH

#include "les_base.h"

#define LES_COREENGINE_ERROR (-1)
#define LES_COREENGINE_FINISHED (0)
#define LES_COREENGINE_OK (1)

void LES_CoreEngineInit(void);
int LES_CoreEngineTick(void);
void LES_CoreEngineShutdown(void);

int LES_CoreEngineGetState(void);

#endif //#ifndef LES_COREENGINE_HH
