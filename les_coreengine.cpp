#include <string.h>
#include <stdlib.h>

#include "les_base.h"
#include "les_coreengine.h"
#include "les_logger.h"
#include "les_network.h"
#include "les_hash.h"

enum { LES_STATE_UNKNOWN, LES_STATE_BOOT, LES_STATE_NOT_CONNECTED, LES_STATE_WAITING_FOR_CONNECT_RESPONSE, LES_STATE_CONNECTED };

static int les_state = LES_STATE_UNKNOWN;
static LES_uint32 les_correctResponseHash;

#define LES_NETMESSAGE_RECV_ID_CONNECT_RESPONSE (0x2)
#define LES_NETMESSAGE_RECV_ID_TEST (0x33)

#define LES_NETMESSAGE_SEND_ID_CONNECT (0x1)
#define LES_NETMESSAGE_SEND_ID_TEST (0x66)

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_TestMessageHandler(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payloadSize, void* payload)
{
	LES_LOG("Received Message type:0x%X id:0x%X payloadSize:%d payload:'%s'", type, id, payloadSize, (char*)payload);
	return LES_RETURN_OK;
}

static int LES_ConnectResponseMessageHandler(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payloadSize, void* payload)
{
	if (type != LES_NETMESSAGE_RECV_ID_CONNECT_RESPONSE)
	{
		LES_ERROR("ConnectResponse wrong type:%d", type);
		return LES_RETURN_ERROR;
	}
	if (id != 234)
	{
		LES_ERROR("ConnectResponse wrong id:%d", id);
		return LES_RETURN_ERROR;
	}
	if (payloadSize != 4)
	{
		LES_ERROR("ConnectResponse wrong payloadSize:%d", payloadSize);
		return LES_RETURN_ERROR;
	}
	LES_uint32 bigResponseHash;
	memcpy(&bigResponseHash, payload, sizeof(LES_uint32));
	const LES_uint32 responseHash = fromBigEndian32(bigResponseHash);
	LES_LOG("responseHash:0x%X", responseHash);
	if (responseHash != les_correctResponseHash)
	{
		LES_ERROR("ConnectResponse wrong responseHash:0x%X Expected:0x%X", responseHash, les_correctResponseHash);
		return LES_RETURN_ERROR;
	}
	les_state = LES_STATE_CONNECTED;
	return LES_RETURN_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public external functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_CoreEngineInit(void)
{
	les_state = LES_STATE_BOOT;
}

void LES_CoreEngineShutdown(void)
{
}

int LES_CoreEngineTick(void)
{
	LES_NetworkTick();

	if (les_state == LES_STATE_BOOT)
	{
		const char* const ip = "127.0.0.1";
		const short port = 3141;
		if (LES_NetworkCreateTCPSocket("127.0.0.1", 3141) == LES_RETURN_OK)
		{
			LES_NetworkRegisterReceivedMessageHandler(LES_NETMESSAGE_RECV_ID_TEST, LES_TestMessageHandler);
			LES_NetworkRegisterReceivedMessageHandler(LES_NETMESSAGE_RECV_ID_CONNECT_RESPONSE, LES_ConnectResponseMessageHandler);
			les_state = LES_STATE_NOT_CONNECTED;
			return LES_COREENGINE_OK;
		}
		else
		{
			LES_ERROR("ERROR creating TCP socket on '%s':%d", ip, port);
			return LES_COREENGINE_ERROR;
		}
	}
	if (les_state == LES_STATE_NOT_CONNECTED)
	{
		const LES_uint16 type = LES_NETMESSAGE_SEND_ID_CONNECT;
		const LES_uint16 id = 234;
		const LES_uint32 payloadSize = 16;
		char payload[17];
		for (LES_uint32 i = 0; i < payloadSize; i++)
		{
			char c = '0';
			const int v = rand() % (10+26+26);
			if (v < 10)
			{
				c = (char)('0' + v);
			}
			else if (v < 36)
			{
				c = (char)('a' + (v-10));
			}
			else
			{
				c = (char)('A' + (v-10-26));
			}
			payload[i] = c;
		}
		payload[16] = '\0';
		LES_NetworkSendItem sendItem;
		sendItem.Create(type, id, payloadSize, payload);
		if (LES_NetworkAddSendItem(&sendItem) == LES_RETURN_ERROR)
		{
			LES_ERROR("Error adding connect send item");
			return LES_COREENGINE_ERROR;
		}
		les_state = LES_STATE_WAITING_FOR_CONNECT_RESPONSE;
		les_correctResponseHash = LES_GenerateHashCaseSensitive(payload);
		LES_LOG("connectHash:0x%X", les_correctResponseHash);
		return LES_COREENGINE_OK;
	}
	else if (les_state == LES_STATE_WAITING_FOR_CONNECT_RESPONSE)
	{
	}
	else if (les_state == LES_STATE_CONNECTED)
	{
	}

	return LES_COREENGINE_OK;
}

int LES_CoreEngineGetState(void)
{
	return les_state;
}
