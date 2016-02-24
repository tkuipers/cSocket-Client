#!/usr/bin/env python
import struct
import sys,os
import socket
import binascii
import threading
import time 
import fcntl
from multiprocessing import Process


class Client:
	#SELF VARIABLES
	#myAdd = My MAC Address
	#dev = The Device to run the packets through
	#registered = Am i registered to the house?
	#homeAdd the MAC of the server (only initialized if I'm registered)
	@staticmethod
	def getHwAddr(ifname):
		temp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		info = fcntl.ioctl(temp.fileno(), 0x8927,  struct.pack('256s', ifname[:15]))
		return info[18:24]


	def listen(self, printInfo=False, timeout=7):
		rawSocket=socket.socket(socket.PF_PACKET,socket.SOCK_RAW,socket.htons(0x0003))
		# rawSocket.bind(("eth0", 0))
		rawSocket.bind((self.dev, 0))
		rawSocket.settimeout(timeout)
		#ifconfig eth0 promisc up
		receivedPacket=rawSocket.recv(1024)
		
		#Ethernet Header...
		ethernetHeader=receivedPacket[0:14]
		# arp_header = receivedPacket[0][14:]
		ethrheader=struct.unpack("!6s6s2s",ethernetHeader)
		destinationIP= binascii.hexlify(ethrheader[0])
		sourceIP= binascii.hexlify(ethrheader[1])
		protocol= binascii.hexlify(ethrheader[2])
		if printInfo:
			print "Myadd: " + binascii.hexlify(self.myAdd)
			# print "homeAdd  : " + binascii.hexlify(self.homeAdd)
			print "Destination: " + destinationIP
			print "Source: " + sourceIP
			print "Protocol: "+ protocol
			print "Out: " + receivedPacket[14:] + "\n\n\n"
		if not self.registered:
			#print receivedPacket[14:22]
			if(receivedPacket[14:22] == "Accepted"):
				print "Accepted into network"
				self.registered = True;
				self.homeAdd = ethrheader[1]
		try:
			if destinationIP == binascii.hexlify(self.myAdd): #"\x00\x1b\x24\x07\x57\x9e"):
				return receivedPacket[14:];
		except Exception as inst:
			print "Excewpt 1"
			pass
	
	
	def register(self, sleepTime):
		while True:
			try:
				print "Registering on Network"
				self.sendPayload("\xFF\xFF\xFF\xFF\xFF\xFF", ("CheckIn"+binascii.hexlify(self.myAdd)+ binascii.hexlify(str(self.idNum))))
				if self.listen(False):
					if self.registered:
						return
					else:
						# print "Excewpt 2"
						pass
				else:
					# print "Excewpt 3"
					pass
				time.sleep(sleepTime) 
			except KeyboardInterrupt:
				notifier.stop()
				print 'KeyboardInterrupt caught'
				raise
			except Exception as inst:
				print "Excewpt 2"
				print inst
				pass
			
	
	def answerCheck(self):
		while True:
			try:
				chkString = self.listen(False, 50);
				# print "CheckString = " + chkString
				if chkString and chkString[0:7]=="CheckUp"[0:7]:
					print "Recieved polling signal"
					self.sendPayload(self.homeAdd, "Here")
					pass
				else:
					pass
					# print "CheckUp" + " != " + chkString
					# print str(type("CheckUp")) + " != " + str(type(chkString))
			except KeyboardInterrupt:
				notifier.stop()
				print 'KeyboardInterrupt caught'
				raise
			except Exception as inst:
				print "Excewpt 3"
				print inst
				pass

	def listenForInput(self):
		while True:
			try:
				chkString = self.listen(False, 50);
				if chkString:
					print "Recieved: " + chkString
				if chkString and chkString[0:7]!="CheckUp"[0:7]:
					#print "Recieved: "+ chkString
					self.sendPayload(self.homeAdd, "Received")
					pass
				else:
					pass
					# print "Recieved" + " != " + chkString
					# print str(type("CheckUp")) + " != " + str(type(chkString))
			except KeyboardInterrupt:
				notifier.stop()
				print 'KeyboardInterrupt caught'
				raise
			except Exception as inst:
				print "Excewpt 4"
				print inst
				pass

	# def sendSocket(self, myAdd, homeAdd, dev):
		# sendPayload(myAdd, homeAdd, dev, "Received")
	
	
	def sendPayload(self, destAdd, msg):
		s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW)
		# s.settimeout(0.2)
		s.bind((self.dev, 0))
		
		# We're putting together an ethernet frame here, 
		# but you could have anything you want instead
		# Have a look at the 'struct' module for more 
		# flexible packing/unpacking of binary data
		# and 'binascii' for 32 bit CRC
		src_addr = self.myAdd
		dst_addr = destAdd
		payload = msg#"Received"
		
		ethertype = "\x08\x00"
		print "sending: " + dst_addr+src_addr+binascii.hexlify(ethertype)+payload;
		s.send(dst_addr+src_addr+ethertype+payload)
	
	def __init__(self, inDev, inID):
		self.idNum = inID;
		self.dev = inDev
		# print binascii.unhexlify(binascii.hexlify("\x00\x1b\x24\x07\x57\x9e"))
		self.myAdd = self.getHwAddr(inDev)
		#print self.myAdd
		self.registered = False
		self.register(1)
		# self.answerCheck()
		pollCheckThread = Process(target=self.answerCheck)
		# thread.daemon = True
		pollCheckThread.start()

		listenForInput = Process(target=self.listenForInput)
		# thread.daemon = True
		listenForInput.start()

		# thread = threading.Thread(target=self.listenForInput)
		# thread.daemon = True
		# thread.start()


		# :print self.homeAdd
		# app.debug = True
		# print binascii.hexlify("\x78\x24\xaf\x10\x34\x44");
	# 
		# success = 0
		# fail = 0
		# total = 0
		# while True:
			# print "In the true"
			# outString = listen("\x00\x1b\x24\x07\x57\x9e", "\x78\x24\xaf\x10\x34\x44", "eth0", True)
			# if  outString:
				# print outString
				# t = threading.Thread(target=sendSocket, args = ("\x78\x24\xaf\x10\x34\x44","\x00\x1b\x24\x07\x57\x9e", "eth0"))
				# t.daemon = True
				# t.start()
				# success = success + 1
			# else:
				# fail = fail + 1
			# total = total+1
			# print "Successfull: " + str(success) + "/" + str(total) + "\n"
			# print "fail: " + str(fail) + "/" + str(total) + "\n\n\n\n"
if __name__ == '__main__':
	Client("eth0", 2)
	# Client("wlan0")
