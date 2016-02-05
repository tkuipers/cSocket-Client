/*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*/

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <unistd.h>

//For through WIFI
#define HOME_MAC0	0x54
#define HOME_MAC1	0x35
#define HOME_MAC2	0x30
#define HOME_MAC3	0x85
#define HOME_MAC4	0x61
#define HOME_MAC5	0xa3

//For through eth0
// #define HOME_MAC0	0x78
// #define HOME_MAC1	0x24
// #define HOME_MAC2	0xaf
// #define HOME_MAC3	0x10
// #define HOME_MAC4	0x34
// #define HOME_MAC5	0x44


//ON ETHERNET
// #define MY_DEST_MAC0	0x00
// #define MY_DEST_MAC1	0x1b
// #define MY_DEST_MAC2	0x24
// #define MY_DEST_MAC3	0x07
// #define MY_DEST_MAC4	0x57
// #define MY_DEST_MAC5	0x9e

//ON WIFI
#define MY_DEST_MAC0	0x00
#define MY_DEST_MAC1	0x19
#define MY_DEST_MAC2	0x7e
#define MY_DEST_MAC3	0x24
#define MY_DEST_MAC4	0xe4
#define MY_DEST_MAC5	0x0c

#define ETHER_TYPE	0x0800

 // #define DEFAULT_IF	"eth0" 
#define DEFAULT_IF	"wlan0"
// #define DEFAULT_IF	"enp3s0f2"
// #define DEFAULT_IF  "wlp2s0f0"

#define BUF_SIZ		1024

void sendPacket(long, long, long, long, long, long, char*);
int recievePacket();
int main(int argc, char *argv[])
{
	while(1){
		if(recievePacket() != 0){
			nanosleep((const struct timespec[]){{0, 1000000000L}}, NULL);
			sendPacket(HOME_MAC0, HOME_MAC1, HOME_MAC2, HOME_MAC3, HOME_MAC4, HOME_MAC5, "Received");
		}
	}
}



void sendPacket(long mac0, long mac1, long mac2, long mac3, long mac4, long mac5, char* data){
	int sockfd;
	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0;
	char sendbuf[BUF_SIZ];
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
	struct sockaddr_ll socket_address;
	char ifName[IFNAMSIZ];
	
	/* Get interface name */
	// if (argc > 1)
		// strcpy(ifName, argv[1]);
	// else
		strcpy(ifName, DEFAULT_IF);

	/* Open RAW socket to send on */
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("socket");
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");
	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
	    perror("SIOCGIFHWADDR");

	/* Construct the Ethernet header */
	memset(sendbuf, 0, BUF_SIZ);
	/* Ethernet header */
	eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
	eh->ether_dhost[0] = mac0;
	eh->ether_dhost[1] = mac1;
	eh->ether_dhost[2] = mac2;
	eh->ether_dhost[3] = mac3;
	eh->ether_dhost[4] = mac4;
	eh->ether_dhost[5] = mac5;
	/* Ethertype field */
	eh->ether_type = htons(ETH_P_IP);
	tx_len += sizeof(struct ether_header);
	printf("%hu", htons(ETH_P_IP));
	/* Packet data */
	for(int i = 0; i < strlen(data); i++){
		sendbuf[tx_len++] = data[i];
		
	}

	// sendbuf[tx_len++] = strtol("R", &"r", 16);
	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
	socket_address.sll_addr[0] = mac0;
	socket_address.sll_addr[1] = mac1;
	socket_address.sll_addr[2] = mac2;
	socket_address.sll_addr[3] = mac3;
	socket_address.sll_addr[4] = mac4;
	socket_address.sll_addr[5] = mac5;
	printf("SENDING PACKET\n");
	/* Send packet */
	if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
	    printf("Send failed\n");

	return;
}

int recievePacket(){
	char sender[INET6_ADDRSTRLEN];
	int sockfd, ret, i;
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	struct ifreq if_ip;	/* get ip addr */
	struct sockaddr_storage their_addr;
	uint8_t buf[BUF_SIZ];
	char ifName[IFNAMSIZ];
	/* Get interface name */
	// if (argc > 1)
		// strcpy(ifName, argv[1]);
	// else
		strcpy(ifName, DEFAULT_IF);

	/* Header structures */
	struct ether_header *eh = (struct ether_header *) buf;
	struct iphdr *iph = (struct iphdr *) (buf + sizeof(struct ether_header));
	struct udphdr *udph = (struct udphdr *) (buf + sizeof(struct iphdr) + sizeof(struct ether_header));

	memset(&if_ip, 0, sizeof(struct ifreq));

	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		perror("listener: socket");	
		return -1;
	}

	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("setsockopt");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		perror("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	ifopts.ifr_flags |= IFF_PROMISC;
	if( ioctl(sockfd, SIOCSIFFLAGS, &ifopts) != 0 )
	{
		perror("ERROR SETTING PROMISCOUS");
	    // handle error here
	}
	while(1){
	
		// printf("listener: Waiting to recvfrom...\n");
		numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
	
		/* Check the packet is for me */
		// printf("recieved IP: %s:%s:%s:%s:%s:%s\n", eh->ether_dhost[0]);
		if (eh->ether_dhost[0] == MY_DEST_MAC0 &&
				eh->ether_dhost[1] == MY_DEST_MAC1 &&
				eh->ether_dhost[2] == MY_DEST_MAC2 &&
				eh->ether_dhost[3] == MY_DEST_MAC3 &&
				eh->ether_dhost[4] == MY_DEST_MAC4 &&
				eh->ether_dhost[5] == MY_DEST_MAC5) {
			printf("\nCorrect destination MAC address\n");
			printf("\tlistener: got packet %zd bytes\n", numbytes);
		} else {
			printf("Wrong destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
							eh->ether_dhost[0],
							eh->ether_dhost[1],
							eh->ether_dhost[2],
							eh->ether_dhost[3],
							eh->ether_dhost[4],
							eh->ether_dhost[5]);
			ret = -1;
			continue;
		}
	
		/* Get source IP */
		((struct sockaddr_in *)&their_addr)->sin_addr.s_addr = iph->saddr;
		inet_ntop(AF_INET, &((struct sockaddr_in*)&their_addr)->sin_addr, sender, sizeof sender);
	
		/* Look up my device IP addr if possible */
		strncpy(if_ip.ifr_name, ifName, IFNAMSIZ-1);
		if (ioctl(sockfd, SIOCGIFADDR, &if_ip) >= 0) { /* if we can't check then don't */
			printf("\tSource IP: %s\n\tMy IP: %s\n", sender, 
					inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
			/* ignore if I sent it */
			if (strcmp(sender, inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr)) == 0)	{
				printf("but I sent it :(\n");
				ret = -1;
				// goto done;
				continue;
			}
		}
	
		/* UDP payload length */
		ret = ntohs(udph->len) - sizeof(struct udphdr);
		// int good = 0;
		// int check = checkSum(buf);
		// if(checkSum(buf, numbytes) != -1){
			// good = 1;
		// }
		/* Print packet */
		printf("\tTo MAC Address: ");
		for(i = 0; i < 6; i++){
			printf("%02x:", buf[i]);
		}
		printf("\n\tFrom MAC Address: ");
		for(i = 6; i < 12; i++){
			printf("%02x:", buf[i]);
		}
		printf("\n\tProtocol Type: %02x", buf[12]);
		printf("\n\tData: ");
		for (i=13; i<numbytes; i++){
			// if(i < 12)
				// printf("%02x:", buf[i]);
			// else
				printf("%c",buf[i]);//, buf[i++]));
	
		}
		printf("\n");
		return 1;
		// if(good){
			// break;
		// }
	}

	close(sockfd);
	return ret;

}
