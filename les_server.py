#!/usr/bin/python

import time
import threading
import SocketServer
import struct

packedInt16 = struct.Struct(">h")
packedUint16 = struct.Struct(">H")
packedInt32 = struct.Struct(">i")
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
			self.m_type = packedInt16.unpack(messageData[0:2])[0]
			self.m_id = packedInt16.unpack(messageData[2:4])[0]
			self.m_payloadSize = packedInt32.unpack(messageData[4:8])[0]
			self.m_valid = True
		if messageDataLen > 8:
			self.m_payload = messageData[8:messageDataLen]
		return self.m_valid

	def Encode(self):
		packetData = ""
		packetData += packedInt16.pack(self.m_type)
		packetData += packedInt16.pack(self.m_id)
		packetData += packedInt32.pack(self.m_payloadSize)
		if self.m_payload != None:
			packetData += self.m_payload
		return packetData

class ThreadedTCPRequestHandler(SocketServer.BaseRequestHandler):
	def handle(self):
		curThread = threading.currentThread()
		threadName = curThread.getName()
		while (1):
 			receivedData = self.request.recv(1024)
			receivedDataLen = len(receivedData)
			if receivedDataLen == 0:
				continue

			receivedMessage = LES_NetworkMessage()
			if receivedMessage.Decode(receivedData) == False:
				print("%s: Failed to decode message receivedDataLen:%d" % (threadName, receivedDataLen))
				continue
			print("type:0x%X id:%d payloadSize:%d" % (receivedMessage.m_type, receivedMessage.m_id, receivedMessage.m_payloadSize))
			print("payload:%s" % (receivedMessage.m_payload))

			sendMessage = LES_NetworkMessage()
			sendMessage.m_type = 0x33
			sendMessage.m_id = receivedMessage.m_id
			sendMessage.m_payload = "%s:%s" % (threadName, receivedMessage.m_payload)
			sendMessage.m_payloadSize = len(sendMessage.m_payload)
 			response = sendMessage.Encode()
			self.request.send(response)


class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
	pass

def runTest():
	# Port 0 means to select an arbitrary unused port
 	HOST, PORT = "localhost", 3141

 	server = ThreadedTCPServer((HOST, PORT), ThreadedTCPRequestHandler)
 	ip, port = server.server_address

 	# Start a thread with the server -- that thread will then start one
 	# more thread for each request
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
