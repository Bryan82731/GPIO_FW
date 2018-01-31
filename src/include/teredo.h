/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	tereo.h

*****************************************************************************
*/

#ifndef __TEREDO_H__
#define __TEREDO_H__
#define TEREDO_FAIL -1
#define TEREDO_AUTH_TYPE 1
#define TEREDO_ORIGIN_TYPE 0
#define TEREDO_NONCE_LEN 8
#define TEREDO_SRV_PORT 3544
#define TEREDO_PKT_RX_TIMEOUT OS_TICKS_PER_SEC*2
#define TEREDO_MAINTAN_TIME OS_TICKS_PER_SEC*30
#define TEREDO_REPEAT_TIMES 3
#define TEREDO_CONE			1
#define TEREDO_RESTRICTED	2
#define TEREDO_SYMMETRIC	3

#define TEREDO_INIT			0
#define TEREDO_START		1
#define TEREDO_QUAILIFIED	2
#define TEREDO_ENTRY_NUM	10
#define TEREDO_NO_ENTRY			-1
#define TEREDO_NO_TRUST_ENTRY	-2

typedef struct _TeredoAuthHdr{
	unsigned char IndicatorTypeL;
	unsigned char IndicatorTypeH;
	unsigned char CILen;
	unsigned char AuthDataLen;
	unsigned char nonce[TEREDO_NONCE_LEN];
	unsigned char Confirm;
}TeredoAuthHdr;

typedef struct _TeredoIndHdr{
	unsigned char IndicatorTypeL;
	unsigned char IndicatorTypeH;
	unsigned short OriginPort;
	unsigned int OriginIP;
}TeredoIndHdr;

typedef struct _TeredoAddrFmt
{
	unsigned char prefix[4];
	unsigned char SrvIP[4];
	unsigned char z;
	unsigned char UG:2, z1:5, C:1;
	unsigned char CPort[2];
	unsigned char CIP[4];
}TeredoAddrFmt;

typedef struct _TeredoEntry
{
	unsigned char SrcIPv6[IPv6_ADR_LEN];
	unsigned char MappedIP[IPv4_ADR_LEN];
	unsigned int  cv;
	unsigned char MappedPort[2];//bigendian
	unsigned char trust;
	unsigned char time;
}TeredoEntry;

void initTeredo();
void teredoTxSrv(void* data);
void teredoRxSrv(void* data);
void createTeredoAddrEntry(unsigned char *ipv6Addr,int nonce);
void doTeredoConnTest(unsigned char *ipv6Addr);
void teredoTxPendDequeue();
void generateTeredoIPaddr();
int chkTeredoPeerList(unsigned char *ipv6Addr);
void updateTeredoPeerEntry(unsigned char* p, char cmpNonce, unsigned port, unsigned int ip);
#endif
