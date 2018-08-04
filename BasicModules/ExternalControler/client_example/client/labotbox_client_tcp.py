#!/usr/bin/env python
# coding: utf-8

import socket
from labotbox_client import *

class LabotboxClientTcp(LabotboxClient):
	"Classe de pilotage de Labotbox depuis un client TCP"
	def __init__(self, host, port):
		self._rx_buffer_size = 65536 # taille max du buffer pour la réponse
		super(LabotboxClientTcp, self).__init__(host, port)

	def connect(self, host, port):
		self.close()
		self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self._socket.connect((host, port))
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
					print("TCP socket Closed")
	
	def read(self):
		return self._socket.recv(self._rx_buffer_size)
	
	def write(self, command):
		self._socket.send(command)

