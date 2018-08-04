#!/usr/bin/env python
# coding: utf-8

# Cet énuméré doit être en cohérence avec celui dans Labotbox Instructions.h 
class ErrorCode():
	INSTRUCTION_OK = 0
	INVALID_INSTRUCTION = 1
	INVALID_ARGUMENT = 2
	INTERNAL_ERROR = 3

class LabotboxClient(object):
	"Classe de pilotage de Labotbox"
	def __init__(self, host, port):
		self._host = host
		self._port = port
		self._debug = True

		self.connect(host, port)

	def connect(self, host, port):
		print "connect"

	def close(self):
		print "close"

	def setDebug(self, on_off):
		self._debug = on_off
		if (self._debug):
			print ("Debug trace is ON")
		

	def formatArgument(self, argument):
		print("formatArgument: " + argument) 

	def isInteger(self, s):
		try:
			int(s)
			return True
		except ValueError:
			return False

	def isFloat(self, s):
		try:
			float(s)
			return True
		except ValueError:
			return False

	def errorCodeToString(self, errorCode):
		if errorCode == ErrorCode.INSTRUCTION_OK:
			return 'INSTRUCTION_OK'
		if errorCode == ErrorCode.INVALID_INSTRUCTION:
			return 'INVALID_INSTRUCTION'
		if errorCode == ErrorCode.INVALID_ARGUMENT:
			return 'INVALID_ARGUMENT'
		if errorCode == ErrorCode.INTERNAL_ERROR:
			return 'INTERNAL_ERROR'
		return '??UNKNOWN_ERROR_CODE'

	def sendCommand(self, command):
		if (self._debug == True):
			print(">" + command)
		self.write(command)
		r = self.read() # toute commande reçoit une réponse avec au moins le code d'erreur comme premier paramètre de retour
		if (self._debug == True):
			print("<" + r)
		ret_param = r.split('|')
		errorCode = int(ret_param[0])
		if (errorCode != ErrorCode.INSTRUCTION_OK):
			raise ValueError('[' + str(errorCode) + ']' + self.errorCodeToString(errorCode) + ' : error while executing command ' + command)
		del ret_param[0]  # remove error code from the response
		if (len(ret_param) != 0):
			return ret_param

	def help(self):
		ret_param = self.sendCommand("help()")
		return str(ret_param[0])

	def helpInstruction(self, instruction):
		ret_param = self.sendCommand("helpInstruction(" + str(instruction) + ")")
		return str(ret_param[0])

	def setData(self, data, value):
		self.sendCommand("setData(" + str(data) + "," + str(value) +")")

	def getData(self, data):
		ret_param = self.sendCommand("getData(" + str(data) +")")
		return ret_param  # ret_param est une liste dans ce cas

	def getDataNum(self, data):
		ret_param = self.getData(data)
		if (self.isInteger(ret_param[0])):
			return int(ret_param[0])
		elif (self.isFloat(ret_param[0])):
			return float(ret_param[0])
		else :
			raise ValueError(str(ret_param[0]) + " is not a number")

	def getDataStr(self, data):
		ret_param = self.getData(data)
		return str(ret_param[0])

	def getDatasStr(self):
		ret_param = self.sendCommand("getDatas()")
		return str(ret_param[0])

	def getDatas(self):
		ret_param = self.sendCommand("getDatas()")
		table = ret_param[0].split('\n')
		table.sort()
		list_dataname_value = []
		for data in table:
			pair = data.split('=')
			if (len(pair) >=2):
				list_dataname_value.append(pair)
		return list_dataname_value # une liste de {"nom" "valeur"}

	def createData(self, data, init_value):
		self.sendCommand("createData(" + str(data) + "," + str(init_value) +")")

