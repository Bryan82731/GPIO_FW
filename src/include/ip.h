/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	ip.h

*****************************************************************************
*/

#ifndef _IP_INCLUDED
#define _IP_INCLUDED

/*--------------------------Include Files-----------------------------------*/
#include "lib.h"
#include "dhcp.h"
#if CONFIG_TEREDO_ENABLED
#include "teredo.h"
#endif
#include <sys/inet_types.h>

/*--------------------------Definitions------------------------------------*/
#define IP_HLEN      20
#define IPv6_HLEN 40
#define IP_DEFAULT_TTL  255
#define IP_PROTO_ICMP 0x01
#define IP_PROTO_ICMPv6 0x3A
#define IP_PROTO_UDP 0x11
#define IP_PROTO_TCP 0x06
#define IP_ADDR_BROADCAST 0xffffffff
#if CONFIG_TEREDO_ENABLED
#define IP_PROTO_NONHDR 0x3b
#endif
/*-----------------------------Structures------------------------------------*/
//move to inet_types.h
/*
typedef struct _IPAddress
{
    u32_t addr;
}IPAddress;
*/

typedef struct _IPHdr
{

#if BYTE_ORDER == LITTLE_ENDIAN
unsigned char hl:4,                /* header length */
ver:4;                    /* version */
#else
unsigned char ver:    4,                 /* version */
hl:    4;                   /* header length */
#endif

    u8_t tos;                 /* type of service */
    u16_t len;                /* total length */
    u16_t id;                 /* identification */
    u16_t offset;             /* fragment offset field */
#define IP_RF 0x8000        /* reserved fragment flag */
#define IP_DF 0x4000        /* dont fragment flag */
#define IP_MF 0x2000        /* more fragments flag */
#define IP_OFFMASK 0x1fff   /* mask for fragmenting bits */
    u8_t ttl;                 /* time to live */
    u8_t proto;               /* protocol */
    u16_t chksum;             /* checksum */
    IPAddress src, dest;
}IPHdr;

typedef struct _IPv6Hdr
{
#if BYTE_ORDER == LITTLE_ENDIAN
unsigned char pri:4, ver:4;
    unsigned char flow1;
#else
unsigned char ver:4, pri:4;
    unsigned char flow1;
#endif
    unsigned short flow2;
    unsigned short len;                /* payload length */
    unsigned char nexthdr;             /* next header */
    unsigned char hoplim;              /* hop limit (TTL) */
    unsigned char src[IPv6_ADR_LEN];
    unsigned char dest[IPv6_ADR_LEN];          /* source and destination IP addresses */
}IPv6Hdr;

typedef struct _IPv6PseudoHdr
{
    unsigned char src[IPv6_ADR_LEN];
    unsigned char dest[IPv6_ADR_LEN];          /* source and destination IP addresses */
    unsigned char unUsed;
    char nextHdr;
    unsigned short payLoadLen;
}IPv6PseudoHdr;

typedef struct _IPv4PseudoHdr
{
  unsigned char src[IPv4_ADR_LEN];
  unsigned char dest[IPv4_ADR_LEN];          /* source and destination IP addresses */
  unsigned char unUsed;
  unsigned char protocol;
  unsigned short totalLen;  
}IPv4PseudoHdr;

typedef struct _IPInfo
{
	unsigned char IPv4Reset:1, IPv6Reset:1, AB:1, RMCPFilter:1, DoReloadFilterTable:1, IPv4Gratuitous:1, LinkChange:1;
	unsigned char IPv4GateWayMAR[MAC_ADDR_LEN];
	unsigned char IPv4DNSMAR[MAC_ADDR_LEN];
	unsigned char IPv6GateWayMAR[MAC_ADDR_LEN];
	unsigned char IPv6DNSMAR[MAC_ADDR_LEN];
#if CONFIG_mDNS_OFFLOAD_ENABLED	
	unsigned int 	tmpIP;
	unsigned char OOBMAR[MAC_ADDR_LEN + 2];
#endif
#if CONFIG_TEREDO_ENABLED
	unsigned char TeredoNonce[TEREDO_NONCE_LEN];
	unsigned int  TeredoCIP;
	unsigned short TeredoCPort;
	unsigned char TeredoIPChg;
#endif	
}_IPInfo;

/*-----------------------------Functions------------------------------------*/
void ipInput(PKT *pkt);
void addIPHeader(ptcp_pcb pcb, PKT* pkt);
#if CONFIG_TEREDO_ENABLED
int chkTeredoAuthHdr(PKT* pkt);
#endif
#endif
