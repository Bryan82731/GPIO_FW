/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	ip.h

*****************************************************************************
*/

#ifndef _ICMP_INCLUDED
#define _ICMP_INCLUDED
#include <sys/inet_types.h>
#if CONFIG_TEREDO_ENABLED
#include "teredo.h"
#endif

/*--------------------------Definitions------------------------------------*/
#define ECHO_REQ	0x08
#define ECHO_REPLY	0x00

#define ICMP6_ECHO 128    /* echo */

#define ROUTERSOLICITATION	133
#define ROUTERADVERTISEMENT	134
#define SOLICITATION	135
#define ADVERTISEMENT	136

#define ECHO_REQv6		128
#define ECHO_REPLYv6	129

#define SRC_LINK_ADDR 1
#define TARGETLINK_ADDR 2


/*-----------------------------Structures------------------------------------*/
typedef struct _ICMPHdr{
    unsigned char type;
    unsigned char code;
    unsigned short chksum;
    unsigned short id;
    unsigned short seqNum;
} ICMPHdr;

typedef struct _ICMPv6Hdr{
    unsigned char type;
    unsigned char code;
    unsigned short chksum;
}ICMPv6Hdr;

typedef struct _NeighborMsg{
    unsigned char flag;
    /*
    #if BYTE_ORDER == LITTLE_ENDIAN
    unsigned char unUsed:5,O:1,S:1,R:1;
    #else
    unsigned char R:1,S:1,O:1,unUsed:5;
    #endif */
    unsigned char Res[3];

    unsigned char targetIP[16];
}NeighborMsg;

typedef struct _LinkLayerAddrOpt{
	unsigned char type;
	unsigned char opLen;
	unsigned char macAddr[MAC_ADDR_LEN];
}LinkLayerAddrOpt;

typedef struct _RouterSolMsg{
    unsigned int flag;
    unsigned char opcode;
    unsigned char opLen;
    unsigned char macAddr[MAC_ADDR_LEN];
}RouterSolMsg;

typedef struct _RouterAdver{
	unsigned char hopLimit;
	unsigned char Res:6, O:1, M:1;
	unsigned short LifeTime;
	unsigned int reachableTime;
	unsigned int reTxTimer;
}RouterAdverMsg;

typedef struct _ICMPOpt{
	unsigned char type;
	unsigned char len;
}ICMPOpt;

typedef struct _ICMPPrefixOpt{
	ICMPOpt ICMPOptHdr;
	unsigned char prefixLen;
	unsigned char Res:6, A:1, L:1;
	unsigned int validLifeTime;
	unsigned int preferredLifeTime;
	unsigned int Res1;
	unsigned char prefix;
}ICMPPrefixOpt;

enum DHCPv6States
{
    UnknownDHCPState = 0x00,
    NSolicitateState = 0x01,
    RSolicitateState = 0x03,
    RouterAdver = 0x04,
    SolicitateState = 0x10,
    AdvertiseState = 0x11,
//	RequestState = 0x12,
    ReplyState = 0x13
};

extern enum DHCPv6States DHCPv6State;
/*-----------------------------Functions------------------------------------*/
void icmpInput(PPKT pkt, int len);
void neighborSolicite(unsigned char* ip, unsigned char intf);
void routerSolicite();
void icmpv6Input(unsigned char *p, int len, unsigned char intf);
void icmpv6Solicitation(unsigned char *p, int len, unsigned char intf);
void icmpv6Advertisement(unsigned char *p, int len, unsigned char intf);
void chkLinkLocalEUIAddr(unsigned char *p, int len, unsigned char intf);
void getLinkLocalEUIAddr(unsigned char* addr, unsigned char intf);
void icmpv6EchoReply(unsigned char *p, int len, unsigned char intf);

#if CONFIG_TEREDO_ENABLED
void teredoRS();
void icmpv6EchoRequest(unsigned char *addr, int* nonce);

#endif
#endif

