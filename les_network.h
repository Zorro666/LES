#ifndef LES_NETWORK_HH
#define LES_NETWORK_HH

#include "les_base.h"
#include "les_networksenditem.h"
#include "les_networkreceiveditem.h"

struct LES_NetworkMessage;

typedef int LES_ReceivedMessageHandlerFunction(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payloadSize, void* payload);

void LES_NetworkInit(void);
void LES_NetworkTick(void);

int LES_NetworkCreateTCPSocket(const char* const ip, const short port);
int LES_NetworkAddSendItem(const LES_NetworkSendItem* const pSendItem);
int LES_NetworkRegisterReceivedMessageHandler(const LES_uint16 type, LES_ReceivedMessageHandlerFunction* pFunction);

#endif // #ifndef LES_NETWORK_HH
