/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	arp.c

*****************************************************************************
*/

#ifndef _ARP_INCLUDED
#define _ARP_INCLUDED

/*--------------------------Include File------------------------------------*/
#include "lib.h"

/*-----------------------------Struct---------------------------------------*/
#define   ARP_Protocol           0x0806
#define   Ethernet_Protocol_IP   0x0800
#define   EAP8021x_Protocol   0x888e
#define   ARP_Request            0x0001
#define   ARP_Reply              0x0002
#define	  ETH_HW_TYPE			 0x0001
#define	  ARPEntryNum	16

/*-----------------------------Struct---------------------------------------*/
typedef struct
{
    unsigned char	srcEthAddr[MAC_ADDR_LEN];//Sender HW Address
    unsigned char	srcIPAddr[IPv4_ADR_LEN];//Sender Protocol Address
    unsigned char	targetEthAddr[MAC_ADDR_LEN];//Target HW Address
    unsigned char	targetIPAddr[IPv4_ADR_LEN];//Target protocol Address
}ARPv4;

typedef struct
{
    EthHdr			ethHdr;
    unsigned short	hwType;
    unsigned short	protocolType;
    unsigned char	hwLen;
    unsigned char	protocolLen;
    unsigned short	operation;
}ARPHdr;

typedef struct _ARPPkt
{
    ARPHdr ah;
    union
    {
        ARPv4 v4;
    }ad;
}ARPPkt;

#define PENDING	1
#define RESOLVED 2
#define ARPTIMEOUT 10

typedef struct
{
    unsigned char	ethAddr[MAC_ADDR_LEN];
    unsigned char	ipAddr[IPv4_ADR_LEN];
    unsigned char	state;
	unsigned char	timeOut;
}ARPEntry;

typedef struct
{
	unsigned char	ethAddr[MAC_ADDR_LEN];
	unsigned char	ipAddr[IPv6_ADR_LEN];
	unsigned char	state;
	unsigned char	attempt;
}ARPv6Entry;

extern ARPv6Entry Arpv6Table[];
/*----------------------------Functions-------------------------------------*/
void sendARPPkt(ARPPkt *, unsigned char* clientIP, unsigned char, unsigned char intf);
void updateArpTable(unsigned char* ethAddr, unsigned char* ipAddr);
unsigned char findEtherAddr(unsigned char* ipAddr, unsigned char* ethAddr);
unsigned char findIPv6EtherAddr(unsigned char* ipAddr, unsigned char* ethAddr);
void arpTableTimeOutUpdate() _ATTRIBUTE_SRAM;
void arpTableTimeOutReset() _ATTRIBUTE_SRAM;

#endif

