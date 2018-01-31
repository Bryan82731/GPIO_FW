/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	dhcp.h

*****************************************************************************
*/

#ifndef __DHCP_H__
#define __DHCP_H__
#include <sys/inet_types.h>
#include "icmp.h"

#define BOOT_REQUEST 0x01
#define BOOT_REPLY 0x02

#define DHCP_DISCOVERY 0x01
#define DHCP_OFFER 0x02
#define DHCP_REQUEST 0x03
#define DHCP_ACK 0x05
#define DHCP_NACK 0x06

#define DHCPv6_SOLICIT 0x01
#define DHCPv6_Advertise 0x02
#define DHCPv6_OptClientID 0x02
#define DHCPv6_OptIANA 0x03
#define DHCPv6_OPTION_ORO 0x06
#define DHCPv6_Reply 0x07
#define DHCPv6_OptDNS 0x17

struct _DHCPv6Conf
{
	enum DHCPv6States DHCPv6State;	
	unsigned char DHCPv6RTTime:5, DHCPv6NeighborAdver:1, DHCPv6CheckEUIAddr:1, M:1, O:1;
	unsigned char DHCPv6RouterAdver:1;//0~128
//	unsigned char prefixLen: 7,DHCPv6RouterAdver:1;//0~128
//	unsigned char prefix[IPv6_ADR_LEN];
//	unsigned char DNSAddr[IPv6_ADR_LEN];
};

typedef struct _DHCPv4Config
{
    IPAddress OfferIP;
    unsigned int DHCPv4TID;
    unsigned int DHCPv4LeaseTime;
    enum DHCPv4States DHCPv4State;
    unsigned int DHCPv4RTTime: 4;
}DHCPv4Config;

typedef struct _DHCPPkt
{
    char opCode;
    char hwtype;
    char HwLen;
    char hopCount;
    unsigned int TID;
    unsigned short seconds;
    unsigned short unused;
    unsigned int clientIP;
    unsigned int yourIP;
    unsigned int serverIP;
    unsigned int gatewayIP;
    char clientHwAddress[16];
    char serverName[64];
    char bootFileName[128];
}DHCPPkt;

typedef struct _DHCPv6Pkt
{
    unsigned char opCode;
    unsigned char TID[3];
}DHCPv6Pkt;

typedef struct _DHCPv6OptHdr
{
    unsigned short opCode;
    unsigned short len;
}DHCPv6OptHdr;

typedef struct _DUIDLLT
{
    unsigned short opCode;
    unsigned short hwType;
    unsigned int time;
}DUIDLLT;

typedef struct _MacAdr
{
    unsigned char macAdr[MAC_ADDR_LEN];
}MacAdr;

typedef struct _Identifierv6
{
    DHCPv6OptHdr OptHdr;
    DUIDLLT DUID;
}Identifierv6;

typedef struct _IA_NA
{
    DHCPv6OptHdr OptHdr;
    unsigned int IAID;
    unsigned int T1;
    unsigned int T2;
}IA_NA;

typedef struct _IA_NA_ADDROPTIONS
{
    DHCPv6OptHdr OptHdr;
    unsigned char addr[IPv6_ADR_LEN];
    unsigned int preferTime;
    unsigned int validTime;

}IA_NA_ADDROPTIONS;

typedef struct _ElapsedTime
{
    unsigned short opCode;
    unsigned short len;
    unsigned short time;
}ElapsedTime;

typedef struct _OptionReq
{
    unsigned short opCode;
    unsigned short len;
    unsigned short op1;
    unsigned short op2;
    unsigned short op3;
//	unsigned short op4;
}OptionReq;

int dhcpOptions(unsigned char *opt, unsigned char tag, unsigned char len, unsigned char *value);
void dhcpSend(PRTSkt s, int type);
char dhcpInput(PRTSkt s, unsigned char *p, int len);
char dhcpv6Input(PRTSkt s, unsigned char *p, int len);
void dhcpv6Solicit(PRTSkt s);
void DHCPv4Task(void *data);
void DHCPv4TaskPatch(void *data);
void DHCPv6Task(void *data);
char dhcpReply(int type, unsigned char *opt, int len, unsigned char intf);
void DHCPv4TimeOut(unsigned char intf);
char dhcpGetOptionsValue(unsigned char *opt, unsigned char tag, unsigned char typeLen, void *value, int dataLen);

#endif

