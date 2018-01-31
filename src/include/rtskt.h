/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	rtskt.h

*****************************************************************************
*/

#ifndef _RTSKT_INCLUDED
#define _RTSKT_INCLUDED

/*--------------------------Include Files-----------------------------------*/
#include "tcp.h"
#include "bsp.h"
/*--------------------------Definitions------------------------------------*/

#define SKTDISCONNECT -2
#define SKT_TX_FAIL   -3
#define SKTHOSTNOTFOUND -4
#define SKTCLOSING -5

#define SKT_DISABLE_RX 0
#define SKT_DISABLE_TX 1
#define SKT_DISABLE_TX_RX 2

typedef struct _RTSkt
{
    struct tcp_pcb		*pcb;
    OS_EVENT	*txSem;//connection semaphore
    OS_EVENT	*rxSem;//connection semaphore
	unsigned int broadcast:1, res:31;
}RTSkt;

typedef struct _UDPAddrInfo
{
	IPAddress		destIPAddr;
	unsigned char	destIPv6Addr[IPv6_ADR_LEN];
	unsigned short	destPort;
	unsigned short	srvPort;
}UDPAddrInfo;

#if CONFIG_TEREDO_ENABLED
typedef struct _TeredoPktItem
{
	struct _TeredoPktItem *next;
	PKT* pkt;
	UDPAddrInfo *addr;
}TeredoPktItem;
#endif

/*----------------------------SRAM Functions-----------------------------------*/
void getUDPAddressInfo(PKT* pkt,UDPAddrInfo* addrInfo) _ATTRIBUTE_SRAM;
RTSkt* rtSktAccept(RTSkt* s) _ATTRIBUTE_SRAM;
int rtSktConnect(RTSkt* s,unsigned char* ipAddr, unsigned short port) _ATTRIBUTE_SRAM;
PKT* rtSktRx(RTSkt* s, int timeout, int* status) _ATTRIBUTE_SRAM;
int rtSktSend(RTSkt* s, PKT* pkt, int len) _ATTRIBUTE_SRAM;
int rtSktSetUDPInfo(RTSkt* s, unsigned char *ip, unsigned short srvPort, unsigned short cliPort) _ATTRIBUTE_SRAM;
int rtSktUDPSendTo(RTSkt* s, PKT* pkt, UDPAddrInfo* ai) _ATTRIBUTE_SRAM;
/*----------------------------SRAM Functions-----------------------------------*/

RTSkt* rtSkt(char ipVer, char protocol);
void rtSktAssignIntf(RTSkt* s, char intf);
int rtSktReuse(RTSkt *s,char ipVer, char protocol);
void rtSktEnableTimeWait(RTSkt *s);
void rtSktEnableIBPortFilter(RTSkt *s);
void rtSktEnableOOBPortFilter(RTSkt *s);
void rtSktEnableRMCPPortFilter();
void rtSktDisableRMCPPortFilter();
int rtSktBind(RTSkt* s, unsigned short port);
int rtSktListen(RTSkt* s, int cons);
int rtSktUDPBind(RTSkt* s, unsigned short srvPort);
int rtSktCloseSend(RTSkt* s, PKT* pkt, int len);
int rtSktClose(RTSkt* s);
int rtGethostbyname(char ipVer, char* host, unsigned char* ip, unsigned char intf);
int rtSktShutdown(RTSkt* s, int how);

void EnableDrvWaitOOB();
void DisableDrvWaitOOB();
unsigned short getPortNumber(ptcp_pcb pcb);
RTSkt* rtClientSkt(ptcp_pcb pcb);

#endif

