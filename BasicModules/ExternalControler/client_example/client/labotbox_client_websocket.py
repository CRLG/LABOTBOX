#!/usr/bin/env python
# coding: utf-8

import websocket
from labotbox_client import *

class LabotboxClientWebSocket(LabotboxClient):
	"Classe de pilotage de Labotbox depuis un client WebSocket"
	def __init__(self, host, port):
		super(LabotboxClientWebSocket, self).__init__(host, port)

	def connect(self, host, port):
		self.close()
		websocket.enableTrace(self._debug)
		str_sock = "ws://" + str(host) + ":" + str(port)
		self._socket = websocket.create_connection(str_sock)
		r = self.read()
		if (self._debug == True):
			print(r)
		return r

	def close(self):
		if hasattr(self, '_socket'):
			if self._socket is not None:
				self._socket.close()
				self._socket = None
				if (self._debug == True):
					print("WebSocket socket Closed")

	def read(self):
		return self._socket.recv()
	
	def write(self, command):
		self._socket.send(command)

