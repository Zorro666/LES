#include <malloc.h>

#include "les_base.h"
#include "les_networkreceiveditem.h"

void LES_NetworkReceivedItem::Free(void)
{
	free(m_message);
	m_message = LES_NULL;
}

