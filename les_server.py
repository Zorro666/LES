#!/usr/bin/python

import time
import threading
import SocketServer
import struct
import random
import math

import les_hash
import les_definitionfile
import les_logger
import les_funcdata

packedUint16 = struct.Struct(">H")
packedUint32 = struct.Struct(">I")

s_DecodeLogChannel = les_logger.CreateChannel("Decode", "", "decode_py.txt", les_logger.LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT)

class LES_NetworkMessage():
	def __init__(self):
		self.m_type = -1
		self.m_id = -1
		self.m_payloadSize = -1
		self.m_payload = None
		self.m_valid = False

	def Decode(self, messageData):
		self.m_type = -1
		self.m_id = -1
		self.m_payloadSize = -1
		self.m_payload = None
		self.m_valid = False
		messageDataLen = len(messageData)
		if messageDataLen > 7:
			self.m_type = packedUint16.unpack(messageData[0:2])[0]
			self.m_id = packedUint16.unpack(messageData[2:4])[0]
			self.m_payloadSize = packedUint32.unpack(messageData[4:8])[0]
			self.m_valid = True
		if messageDataLen > 8:
			self.m_payload = messageData[8:messageDataLen]
		return self.m_valid

def LES_CreateNetworkMessage(typeValue, idValue, payload):
	# Needs to do memory padding like the client does on its sending
	payloadSize = 0
	if payload != None:
		payloadSize = len(payload)

	packetData = ""
	packetData += packedUint16.pack(typeValue)
	packetData += packedUint16.pack(idValue)
	packetData += packedUint32.pack(payloadSize)
	if payload != None:
		packetData += payload

	return packetData

LES_NETMESSAGE_RECV_ID_CONNECT=0x1
LES_NETMESSAGE_RECV_ID_GETDEFINITIONFILE=0x3
LES_NETMESSAGE_RECV_ID_FUNCTIONRPC=0x5
LES_NETMESSAGE_RECV_ID_TEST=0xF1

LES_NETMESSAGE_SEND_ID_CONNECT_RESPONSE=0x2
LES_NETMESSAGE_SEND_ID_GETDEFINITIONFILE_RESPONSE=0x4
LES_NETMESSAGE_SEND_ID_FUNCTIONRPC_RESPONSE=0x6
LES_NETMESSAGE_SEND_ID_TEST_RESPONSE=0xF2

s_enableDebugSleep = 0

def debugRandomSleep():
	if s_enableDebugSleep:
		sleepTime = random.uniform(0.1, 0.5)
		les_logger.Log("SleepTime:%f" % (sleepTime))
		time.sleep(sleepTime)

class ThreadedTCPRequestHandler(SocketServer.BaseRequestHandler):
	def LES_HandleTestMessage(self, msgType, msgId, msgPayloadSize, msgPayload):
		les_logger.Log("Test: type:0x%X id:%d payloadSize:%d" % (msgType, msgId, msgPayloadSize))
		les_logger.Log("Test: payload:%s" % (msgPayload))

		payload = "%s:%s" % (self.m_threadName, msgPayload)
		response = LES_CreateNetworkMessage(LES_NETMESSAGE_SEND_ID_TEST_RESPONSE, msgId, payload)
		debugRandomSleep()
		self.request.send(response)

	def LES_HandleConnectMessage(self, msgType, msgId, msgPayloadSize, msgPayload):
		les_logger.Log("Connect: type:0x%X id:%d payloadSize:%d" % (msgType, msgId, msgPayloadSize))
		les_logger.Log("Connect: payload:%s" % (msgPayload))
		hashValue = les_hash.LES_GenerateHashCaseSensitive(msgPayload)
		les_logger.Log("Connect: connectResponse:0x%X" % (hashValue))
		payload = packedUint32.pack(hashValue)
		response = LES_CreateNetworkMessage(LES_NETMESSAGE_SEND_ID_CONNECT_RESPONSE, msgId, payload)
		debugRandomSleep()
		self.request.send(response)

	def LES_HandleGetDefinitionFileMessage(self, msgType, msgId, msgPayloadSize, msgPayload):
		les_logger.Log("GetDefinitionFile: type:0x%X id:%d payloadSize:%d" % (msgType, msgId, msgPayloadSize))
#		les_logger.Log("GetDefinitionFile: payload:%s" % (msgPayload))
		self.s_definitionFile = les_definitionfile.LES_DefinitionFile()
		self.s_definitionFile.create()
		chunkFileData = self.s_definitionFile.makeData()
		definitionFileData = chunkFileData.getData()
		payload = definitionFileData
		response = LES_CreateNetworkMessage(LES_NETMESSAGE_SEND_ID_GETDEFINITIONFILE_RESPONSE, msgId, payload)
		chunkFileData.close()
		les_logger.Log("GetDefinitionFile: sending:%d" % (len(payload)))
		debugRandomSleep()
		self.request.send(response)

	def LES_HandleFunctionRPCMessage(self, msgType, msgId, msgPayloadSize, msgPayload):
		les_logger.Log("FunctionRPC: type:0x%X id:%d payloadSize:%d" % (msgType, msgId, msgPayloadSize))
		#les_logger.Log("FunctionRPC: payload:%s" % (msgPayload))
		debugRandomSleep()
		functionNameID = packedUint32.unpack(msgPayload[0:4])[0]
		functionParameterData = les_funcdata.LES_FunctionParameterData(msgPayload[4:])

		stringTable = self.s_definitionFile.getStringTable()
		typeData = self.s_definitionFile.getTypeData()
		structData = self.s_definitionFile.getStructData()
		functionData = self.s_definitionFile.getFunctionData()
		functionName = stringTable.getString(functionNameID)
		les_logger.Log("FunctionRPC: id:%d nameID:%d '%s'" % (msgId, functionNameID, functionName))
		functionDefinition = functionData.getFunctionDefinition(functionName)
		(retVal, paramDict) = functionDefinition.Decode(s_DecodeLogChannel, stringTable, typeData, structData, functionParameterData)
		if retVal == les_funcdata.LES_RETURN_ERROR:
			les_logger.Error("FunctionRPC: ERROR during Decode()")

		if retVal == les_funcdata.LES_RETURN_OK:
			les_logger.Log("ParamDict:%s", paramDict)

		if functionName == "LES_Test_ReadInputParameters":
			testFuncName = functionName

			input_value_0 = 102
			input_value_1 = 23453
			input_value_2 = 'n'
			input_value_3 = -4.0332
			input_value_4 = 12345

			input_0 = int(paramDict['input_0'])
			input_1 = int(paramDict['input_1'])
			input_2 = paramDict['input_2']
			input_3 = float(paramDict['input_3'])
			input_4 = int(paramDict['input_4'])

			if input_0 != input_value_0:
				les_logger.Warning("%s: parameter data is wrong input_0:%d input_value_0:%d", testFuncName, input_0, input_value_0)

			if input_1 != input_value_1:
				les_logger.Warning("%s: parameter data is wrong input_1:%d input_value_1:%d", testFuncName, input_1, input_value_1)

			if input_2 != input_value_2:
				les_logger.Warning("%s: parameter data is wrong input_2:%c input_value_2:%c", testFuncName, input_2, input_value_2)

			if math.fabs(input_3 - input_value_3) > 1.0e-4:
				les_logger.Warning("%s: parameter data is wrong input_3:%f input_value_3:%f", testFuncName, input_3, input_value_3)

			if input_4 != input_value_4:
				les_logger.Warning("%s: parameter data is wrong input_4:%d input_value_4:%d", testFuncName, input_4, input_value_4)
					
			les_logger.Log("%s: input_0:%d input_value_0:%d", testFuncName, input_0, input_value_0);
			les_logger.Log("%s: input_1:%d input_value_1:%d", testFuncName, input_1, input_value_1);
			les_logger.Log("%s: input_2:%c input_value_2:%c", testFuncName, input_2, input_value_2);
			les_logger.Log("%s: input_3:%f input_value_3:%f", testFuncName, input_3, input_value_3);
			les_logger.Log("%s: input_4:%d input_value_4:%d", testFuncName, input_4, input_value_4);


		if functionName == "LES_Test_ReadOutputParameters":
			testFuncName = functionName

			output_value_0 = 1
			output_value_1 = 2
			output_value_2 = 'O'
			output_value_3 = -123.456

			output_0 = int(paramDict['output_0'])
			output_1 = int(paramDict['output_1'])
			output_2 = paramDict['output_2']
			output_3 = float(paramDict['output_3'])

			if output_0 != output_value_0:
				les_logger.Warning("%s: parameter data is wrong output_0:%d output_value_0:%d", testFuncName, output_0, output_value_0)

			if output_1 != output_value_1:
				les_logger.Warning("%s: parameter data is wrong output_1:%d output_value_1:%d", testFuncName, output_1, output_value_1)

			if output_2 != output_value_2:
				les_logger.Warning("%s: parameter data is wrong output_2:%c output_value_2:%c", testFuncName, output_2, output_value_2)

			if math.fabs(output_3 - output_value_3) > 1.0e-4:
				les_logger.Warning("%s: parameter data is wrong output_3:%f output_value_3:%f", testFuncName, output_3, output_value_3)

			les_logger.Log("%s: output_0:%d output_value_0:%d", testFuncName, output_0, output_value_0);
			les_logger.Log("%s: output_1:%d output_value_1:%d", testFuncName, output_1, output_value_1);
			les_logger.Log("%s: output_2:%c output_value_2:%c", testFuncName, output_2, output_value_2);
			les_logger.Log("%s: output_3:%f output_value_3:%f", testFuncName, output_3, output_value_3);

		if functionName == "LES_Test_ReadInputOutputParameters":
			testFuncName = functionName

			input_value_0 = 102
			input_value_1 = 23453
			input_value_2 = '*'
			input_value_3 = -4.0332
			input_value_4 = 12345

			output_value_0 = 1
			output_value_1 = 2
			output_value_2 = 'O'
			output_value_3 = -123.456

			input_0 = int(paramDict['input_0'])
			input_1 = int(paramDict['input_1'])
			input_2 = paramDict['input_2']
			input_3 = float(paramDict['input_3'])
			input_4 = int(paramDict['input_4'])

			output_0 = int(paramDict['output_0'])
			output_1 = int(paramDict['output_1'])
			output_2 = paramDict['output_2']
			output_3 = float(paramDict['output_3'])

			if input_0 != input_value_0:
				les_logger.Warning("%s: parameter data is wrong input_0:%d input_value_0:%d", testFuncName, input_0, input_value_0)

			if input_1 != input_value_1:
				les_logger.Warning("%s: parameter data is wrong input_1:%d input_value_1:%d", testFuncName, input_1, input_value_1)

			if input_2 != input_value_2:
				les_logger.Warning("%s: parameter data is wrong input_2:%c input_value_2:%c", testFuncName, input_2, input_value_2)

			if math.fabs(input_3 - input_value_3) > 1.0e-4:
				les_logger.Warning("%s: parameter data is wrong input_3:%f input_value_3:%f", testFuncName, input_3, input_value_3)

			if input_4 != input_value_4:
				les_logger.Warning("%s: parameter data is wrong input_4:%d input_value_4:%d", testFuncName, input_4, input_value_4)
					
			if output_0 != output_value_0:
				les_logger.Warning("%s: parameter data is wrong output_0:%d output_value_0:%d", testFuncName, output_0, output_value_0)

			if output_1 != output_value_1:
				les_logger.Warning("%s: parameter data is wrong output_1:%d output_value_1:%d", testFuncName, output_1, output_value_1)

			if output_2 != output_value_2:
				les_logger.Warning("%s: parameter data is wrong output_2:%c output_value_2:%c", testFuncName, output_2, output_value_2)

			if math.fabs(output_3 - output_value_3) > 1.0e-4:
				les_logger.Warning("%s: parameter data is wrong output_3:%f output_value_3:%f", testFuncName, output_3, output_value_3)

			les_logger.Log("%s: input_0:%d input_value_0:%d", testFuncName, input_0, input_value_0);
			les_logger.Log("%s: input_1:%d input_value_1:%d", testFuncName, input_1, input_value_1);
			les_logger.Log("%s: input_2:%c input_value_2:%c", testFuncName, input_2, input_value_2);
			les_logger.Log("%s: input_3:%f input_value_3:%f", testFuncName, input_3, input_value_3);
			les_logger.Log("%s: input_4:%d input_value_4:%d", testFuncName, input_4, input_value_4);

			les_logger.Log("%s: output_0:%d output_value_0:%d", testFuncName, output_0, output_value_0);
			les_logger.Log("%s: output_1:%d output_value_1:%d", testFuncName, output_1, output_value_1);
			les_logger.Log("%s: output_2:%c output_value_2:%c", testFuncName, output_2, output_value_2);
			les_logger.Log("%s: output_3:%f output_value_3:%f", testFuncName, output_3, output_value_3);

	def handle(self):
		s_receivedMessageHandlers = {}
		s_receivedMessageHandlers[LES_NETMESSAGE_RECV_ID_TEST] = self.LES_HandleTestMessage
		s_receivedMessageHandlers[LES_NETMESSAGE_RECV_ID_CONNECT] = self.LES_HandleConnectMessage
		s_receivedMessageHandlers[LES_NETMESSAGE_RECV_ID_GETDEFINITIONFILE] = self.LES_HandleGetDefinitionFileMessage
		s_receivedMessageHandlers[LES_NETMESSAGE_RECV_ID_FUNCTIONRPC] = self.LES_HandleFunctionRPCMessage

		self.s_definitionFile = None
		self.m_curThread = threading.currentThread()
		self.m_threadName = self.m_curThread.getName()

		while (1):
 			receivedData = self.request.recv(100*1024)
			receivedDataLen = len(receivedData)
			if receivedDataLen == 0:
				time.sleep(0.001)
				continue

			receivedMessage = LES_NetworkMessage()
			if receivedMessage.Decode(receivedData) == False:
				les_logger.Warning("%s: Failed to decode message receivedDataLen:%d" % (self.m_threadName, receivedDataLen))
				continue

			msgType = receivedMessage.m_type
			msgId = receivedMessage.m_id
			msgPayloadSize = receivedMessage.m_payloadSize
			msgPayload = receivedMessage.m_payload
			if msgType in s_receivedMessageHandlers:
				s_receivedMessageHandlers[msgType](msgType, msgId, msgPayloadSize, msgPayload)
			else:
				les_logger.Warning("Unhandled: type:0x%X id:%d payloadSize:%d" % (msgType, msgId, msgPayloadSize))
				#les_logger.Warning("Unhandled: payload:%s" % (msgPayload))

class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
	pass

def runTest():
	les_logger.Init()

	# Port 0 means to select an arbitrary unused port
 	HOST, PORT = "localhost", 3141

 	server = ThreadedTCPServer((HOST, PORT), ThreadedTCPRequestHandler)
 	ip, port = server.server_address

 	# Start a thread with the server -- that thread will then start one more thread for each request
 	server_thread = threading.Thread(target=server.serve_forever)
 	server_thread.setDaemon(True)
 	server_thread.start()
 	les_logger.Log("Server loop running in thread:%s", server_thread.getName())

	while server_thread.isAlive():
		time.sleep(1.0)

 	# Exit the server thread when the main thread terminates
	server.shutdown()

if __name__ == "__main__":
	runTest()
