#ifndef LES_NETWORK_HH
#define LES_NETWORK_HH

#include "les_base.h"
#include "les_networksenditem.h"
#include "les_networkreceiveditem.h"

struct LES_NetworkMessage;

void LES_NetworkInit(void);
void LES_NetworkTick(void);

int LES_NetworkCreateTCPSocket(const char* const ip, const short port);
int LES_NetworkAddSendItem(const LES_NetworkSendItem* const pSendItem);

#endif // #ifndef LES_NETWORK_HH
