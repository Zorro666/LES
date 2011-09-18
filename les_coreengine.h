#ifndef LES_COREENGINE_HH
#define LES_COREENGINE_HH

#include "les_base.h"

class LES_FunctionDefinition;
struct LES_FunctionParameterData;

#define LES_COREENGINE_ERROR (-1)
#define LES_COREENGINE_FINISHED (0)
#define LES_COREENGINE_OK (1)
#define LES_COREENGINE_SEND_ERROR (2)
#define LES_COREENGINE_NOT_READY (3)

enum { LES_STATE_UNKNOWN, 
			 LES_STATE_BOOT, 
			 LES_STATE_NOT_CONNECTED, 
			 LES_STATE_WAITING_FOR_CONNECT_RESPONSE, 
			 LES_STATE_CONNECTED,
			 LES_STATE_WAITING_FOR_DEFINITIONFILE_RESPONSE,
			 LES_STATE_READY
		 };

void LES_CoreEngineInit(void);
int LES_CoreEngineTick(void);
void LES_CoreEngineShutdown(void);

int LES_CoreEngineGetState(void);
int LES_CoreEngineSendFunctionRPC(const LES_FunctionDefinition* const pFunctionDefinition, 
																	const LES_FunctionParameterData* const pFunctionParameterData);

#endif //#ifndef LES_COREENGINE_HH
