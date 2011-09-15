#!/usr/bin/python

import time
import threading
import SocketServer
import struct

import les_hash

packedUint16 = struct.Struct(">H")
packedUint32 = struct.Struct(">I")

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
	payloadSize = len(payload)

	packetData = ""
	packetData += packedUint16.pack(typeValue)
	packetData += packedUint16.pack(idValue)
	packetData += packedUint32.pack(payloadSize)
	if payload != None:
		packetData += payload

	return packetData

LES_NETMESSAGE_SEND_ID_CONNECT_RESPONSE=0x2
LES_NETMESSAGE_SEND_ID_TEST=0x33

LES_NETMESSAGE_RECV_ID_CONNECT=0x1
LES_NETMESSAGE_RECV_ID_TEST=0x66

class ThreadedTCPRequestHandler(SocketServer.BaseRequestHandler):
	def LES_HandleTestMessage(self, msgType, msgId, msgPayloadSize, msgPayload):
		print("Test: type:0x%X id:%d payloadSize:%d" % (msgType, msgId, msgPayloadSize))
		print("Test: payload:%s" % (msgPayload))

		payload = "%s:%s" % (self.m_threadName, msgPayload)
		response = LES_CreateNetworkMessage(LES_NETMESSAGE_SEND_ID_TEST, msgId, payload)
		self.request.send(response)

	def LES_HandleConnectMessage(self, msgType, msgId, msgPayloadSize, msgPayload):
		print("Connect: type:0x%X id:%d payloadSize:%d" % (msgType, msgId, msgPayloadSize))
		print("Connect: payload:%s" % (msgPayload))
		hashValue = les_hash.GenerateHashCaseSensitive(msgPayload)
		print("Connect: connectResponse:0x%X" % (hashValue))
		payload = packedUint32.pack(hashValue)
		response = LES_CreateNetworkMessage(LES_NETMESSAGE_SEND_ID_CONNECT_RESPONSE, msgId, payload)
		self.request.send(response)

	def handle(self):
		s_receivedMessageHandlers = {}
		s_receivedMessageHandlers[LES_NETMESSAGE_RECV_ID_TEST] = self.LES_HandleTestMessage
		s_receivedMessageHandlers[LES_NETMESSAGE_RECV_ID_CONNECT] = self.LES_HandleConnectMessage

		self.m_curThread = threading.currentThread()
		self.m_threadName = self.m_curThread.getName()
		while (1):
 			receivedData = self.request.recv(100*1024)
			receivedDataLen = len(receivedData)
			if receivedDataLen == 0:
				time.sleep(0.01)
				continue

			receivedMessage = LES_NetworkMessage()
			if receivedMessage.Decode(receivedData) == False:
				print("%s: Failed to decode message receivedDataLen:%d" % (self.m_threadName, receivedDataLen))
				continue

			msgType = receivedMessage.m_type
			msgId = receivedMessage.m_id
			msgPayloadSize = receivedMessage.m_payloadSize
			msgPayload = receivedMessage.m_payload
			if msgType in s_receivedMessageHandlers:
				s_receivedMessageHandlers[msgType](msgType, msgId, msgPayloadSize, msgPayload)
			else:
				print("Unhandled: type:0x%X id:%d payloadSize:%d" % (msgType, msgId, msgPayloadSize))
				print("Unhandled: payload:%s" % (msgPayload))

#			payload = "%s:%s" % (self.m_threadName, msgPayload)
#			response = LES_CreateNetworkMessage(LES_NETMESSAGE_SEND_ID_TEST, msgId, payload)
#			self.request.send(response)

class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
	pass

def runTest():
	# Port 0 means to select an arbitrary unused port
 	HOST, PORT = "localhost", 3141

 	server = ThreadedTCPServer((HOST, PORT), ThreadedTCPRequestHandler)
 	ip, port = server.server_address

 	# Start a thread with the server -- that thread will then start one more thread for each request
 	server_thread = threading.Thread(target=server.serve_forever)
 	server_thread.setDaemon(True)
 	server_thread.start()
 	print "Server loop running in thread:", server_thread.getName()

	while server_thread.isAlive():
		time.sleep(1.0)

 	# Exit the server thread when the main thread terminates
	server.shutdown()

if __name__ == "__main__":
	runTest()
