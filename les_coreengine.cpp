#include <string.h>
#include <stdlib.h>

#include "les_base.h"
#include "les_coreengine.h"
#include "les_logger.h"
#include "les_network.h"
#include "les_hash.h"
#include "les_definitionfile.h"
#include "les_core.h"
#include "les_function.h"
#include "les_networkmessage.h"
#include "les_stringentry.h"

static int les_state = LES_STATE_UNKNOWN;
static LES_uint32 les_correctResponseHash;
static LES_uint16 les_functionID;

#define LES_NETMESSAGE_SEND_ID_CONNECT (0x1)
#define LES_NETMESSAGE_SEND_ID_GETDEFINITIONFILE (0x3)
#define LES_NETMESSAGE_SEND_ID_FUNCTIONRPC (0x5)
#define LES_NETMESSAGE_SEND_ID_TEST (0xF1)

#define LES_NETMESSAGE_RECV_ID_CONNECT_RESPONSE (0x2)
#define LES_NETMESSAGE_RECV_ID_GETDEFINITIONFILE_RESPONSE (0x4)
#define LES_NETMESSAGE_RECV_ID_FUNCTIONRPC_RESPONSE (0x6)
#define LES_NETMESSAGE_RECV_ID_TEST_RESPONSE (0xF2)

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal Static functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_TestResponseMessageHandler(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payloadSize, void* payload)
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

static int LES_GetDefinitionFileResponseMessageHandler(const LES_uint16 type, const LES_uint16 id, const LES_uint32 payloadSize, 
																											 void* payload)
{
	if (type != LES_NETMESSAGE_RECV_ID_GETDEFINITIONFILE_RESPONSE)
	{
		LES_ERROR("GetDefinitionFileResponse wrong type:%d", type);
		return LES_RETURN_ERROR;
	}
	if (id != 432)
	{
		LES_ERROR("GetDefinitionFileResponse wrong id:%d", id);
		return LES_RETURN_ERROR;
	}
	if (payloadSize < sizeof(LES_DefinitionFile))
	{
		LES_ERROR("GetDefinitionFileResponse wrong payloadSize:%d", payloadSize);
		return LES_RETURN_ERROR;
	}

	if (LES_SetGlobalDefinitionFile(payload, payloadSize) != LES_RETURN_OK)
	{
		LES_FATAL_ERROR("LES_SetGlobalDefinitionFile Size:%d failed", payloadSize);
	}

	les_state = LES_STATE_READY;
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
	les_functionID = 0;
}

void LES_CoreEngineShutdown(void)
{
}

int LES_CoreEngineTick(void)
{
	LES_NetworkTick();
	const int currentState = les_state;

	if (currentState == LES_STATE_BOOT)
	{
		const char* const ip = "127.0.0.1";
		const short port = 3141;
		if (LES_NetworkCreateTCPSocket("127.0.0.1", 3141) == LES_RETURN_OK)
		{
			LES_NetworkRegisterReceivedMessageHandler(LES_NETMESSAGE_RECV_ID_TEST_RESPONSE, LES_TestResponseMessageHandler);
			LES_NetworkRegisterReceivedMessageHandler(LES_NETMESSAGE_RECV_ID_CONNECT_RESPONSE, LES_ConnectResponseMessageHandler);
			LES_NetworkRegisterReceivedMessageHandler(LES_NETMESSAGE_RECV_ID_GETDEFINITIONFILE_RESPONSE, LES_GetDefinitionFileResponseMessageHandler);
			les_state = LES_STATE_NOT_CONNECTED;
			return LES_COREENGINE_OK;
		}
		else
		{
			LES_ERROR("ERROR creating TCP socket on '%s':%d", ip, port);
			return LES_COREENGINE_ERROR;
		}
	}
	if (currentState == LES_STATE_NOT_CONNECTED)
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
	else if (currentState == LES_STATE_WAITING_FOR_CONNECT_RESPONSE)
	{
		return LES_COREENGINE_OK;
	}
	else if (currentState == LES_STATE_CONNECTED)
	{
		const LES_uint16 type = LES_NETMESSAGE_SEND_ID_GETDEFINITIONFILE;
		const LES_uint16 id = 432;
		LES_NetworkSendItem sendItem;
		const int payloadSize = 0;
		void* payload = LES_NULL;	
		sendItem.Create(type, id, payloadSize, payload);
		if (LES_NetworkAddSendItem(&sendItem) == LES_RETURN_ERROR)
		{
			LES_ERROR("Error adding getdefinitionfile send item");
			return LES_COREENGINE_ERROR;
		}
		les_state = LES_STATE_WAITING_FOR_DEFINITIONFILE_RESPONSE;
		return LES_COREENGINE_OK;
	}
	else if (currentState == LES_STATE_WAITING_FOR_DEFINITIONFILE_RESPONSE)
	{
		return LES_COREENGINE_OK;
	}
	else if (currentState == LES_STATE_READY)
	{
		//les_state = LES_STATE_UNKNOWN;
		//return LES_COREENGINE_FINISHED;
		return LES_COREENGINE_OK;
	}

	LES_ERROR("LES_CoreEngineTick: unknown state:%d", currentState);
	return LES_COREENGINE_ERROR;
}

int LES_CoreEngineGetState(void)
{
	return les_state;
}

int LES_CoreEngineSendFunctionRPC(const LES_FunctionDefinition* const pFunctionDefinition, 
																	const LES_FunctionParameterData* const pFunctionParameterData)
{
	const LES_uint32 functionNameID = pFunctionDefinition->GetNameID();
	const int functionParameterDataSize = pFunctionParameterData->GetNumBytesWritten();
	const char* const pFunctionParameterDataBuffer = pFunctionParameterData->GetBufferPtr();

	const LES_uint16 type = LES_NETMESSAGE_SEND_ID_FUNCTIONRPC;
	const LES_uint16 id = les_functionID;
	const int payloadSize = functionParameterDataSize + sizeof(LES_uint32);
	const LES_StringEntry* const pFunctionNameEntry = LES_GetStringEntryForID(functionNameID);
	const char* const functionName = pFunctionNameEntry? pFunctionNameEntry->m_str : "UNKNOWN";

	LES_LOG("SendRPC functionID:%d '%s' paramDataSize:%d msgID:%d msgPayloadSize:%d", 
					functionNameID, functionName, functionParameterDataSize, id, payloadSize);

	if (les_state != LES_STATE_READY)
	{
		return LES_COREENGINE_NOT_READY;
	}

	les_functionID++;
	LES_NetworkSendItem sendItem;
	sendItem.Create(type, id, payloadSize);
	char* pPayload = (char*)(sendItem.GetMessagePtr()->m_payload);
	const LES_uint32 bigFunctionNameID = toBigEndian32(functionNameID);
	memcpy(pPayload, &bigFunctionNameID, sizeof(LES_uint32));
	pPayload += sizeof(LES_uint32);
	memcpy(pPayload, pFunctionParameterDataBuffer, functionParameterDataSize);
	if (LES_NetworkAddSendItem(&sendItem) == LES_RETURN_ERROR)
	{
		LES_ERROR("Error adding getdefinitionfile send item");
		return LES_COREENGINE_SEND_ERROR;
	}
	return LES_COREENGINE_OK;
}
