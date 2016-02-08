#!/usr/bin/env python
import struct
import sys,os
import socket
import binascii
import threading


def listen(myAdd, homeAdd, dev, printInfo):
    rawSocket=socket.socket(socket.PF_PACKET,socket.SOCK_RAW,socket.htons(0x0800))
    # rawSocket.bind(("eth0", 0))
    rawSocket.bind((dev, 0))
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
        print "Myadd: " + binascii.hexlify(myAdd)
        print "homeAdd  : " + binascii.hexlify(homeAdd)
        print "Destination: " + destinationIP
        print "Source: " + sourceIP
        print "Protocol: "+ protocol
        print "Out: " + receivedPacket[14:] + "\n\n\n"
    if destinationIP == binascii.hexlify(myAdd): #"\x00\x1b\x24\x07\x57\x9e"):
        return receivedPacket[14:];


def sendSocket(myAdd, homeAdd, dev):
    s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW)
    s.bind((dev, 0))
    
    # We're putting together an ethernet frame here, 
    # but you could have anything you want instead
    # Have a look at the 'struct' module for more 
    # flexible packing/unpacking of binary data
    # and 'binascii' for 32 bit CRC
    src_addr = myAdd
    dst_addr = homeAdd
    payload = "Received"
    
    ethertype = "\x08\x00"
    s.send(homeAdd+myAdd+ethertype+payload)

def start():
    # app.debug = True
    # print binascii.hexlify("\x78\x24\xaf\x10\x34\x44");
    success = 0
    fail = 0
    total = 0
    while True:
        outString = listen("\x00\x1b\x24\x07\x57\x9e", "\x78\x24\xaf\x10\x34\x44", "eth0", True)
        if  outString:
            print outString
            t = threading.Thread(target=sendSocket, args = ("\x78\x24\xaf\x10\x34\x44","\x00\x1b\x24\x07\x57\x9e", "eth0"))
            t.daemon = True
            t.start()
            success = success + 1
            # sendSocket()
        else:
            fail = fail + 1
            # print "Didn't get there"
        total = total+1
        print "Successfull: " + str(success) + "/" + str(total) + "\n"
        print "fail: " + str(fail) + "/" + str(total) + "\n\n\n\n"
if __name__ == '__main__':
    start()
