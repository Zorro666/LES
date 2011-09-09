#!/usr/bin/python

import time
import threading
import SocketServer
import struct

packedInt16 = struct.Struct("<h")
packedUint16 = struct.Struct("<H")
packedInt32 = struct.Struct("<i")
packedUint32 = struct.Struct("<I")

class ThreadedTCPRequestHandler(SocketServer.BaseRequestHandler):
	def handle(self):
		curThread = threading.currentThread()
		threadName = curThread.getName()
		while (1):
 			message = self.request.recv(1024)

			typeValue = packedInt16.unpack(message[0:2])[0]
			idValue = packedInt16.unpack(message[2:4])[0]
			payloadSize = packedInt32.unpack(message[4:8])[0]

			print("type:0x%X id:%d payloadSize:%d" % (typeValue, idValue, payloadSize))
			messageData = message[8:]
			print("messageData:%s" % (messageData))
 			response = "%s: %s" % (threadName, messageData)
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
