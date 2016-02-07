#!/usr/bin/env python
import struct
import sys,os
import socket
import binascii


def listen():
    rawSocket=socket.socket(socket.PF_PACKET,socket.SOCK_RAW,socket.htons(0x0800))
    rawSocket.bind(("eth0", 0))
    #ifconfig eth0 promisc up
    receivedPacket=rawSocket.recv(1024)
    
    #Ethernet Header...
    ethernetHeader=receivedPacket[0:14]
    # arp_header = receivedPacket[0][14:]
    ethrheader=struct.unpack("!6s6s2s",ethernetHeader)
    destinationIP= binascii.hexlify(ethrheader[0])
    sourceIP= binascii.hexlify(ethrheader[1])
    protocol= binascii.hexlify(ethrheader[2])
    
    if destinationIP == binascii.hexlify("\x00\x1b\x24\x07\x57\x9e"):
        print "Destination: " + destinationIP
        print "Source: " + sourceIP
        print "Protocol: "+ protocol
        print "Out: " + receivedPacket[14:] + "\n\n\n"


if __name__ == '__main__':
    # app.debug = True
    print binascii.hexlify("\x78\x24\xaf\x10\x34\x44");
    while True:
        listen()
