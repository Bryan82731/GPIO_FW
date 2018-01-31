/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	dns.h

*****************************************************************************
*/


#ifndef __DNS_H__
#define __DNS_H__

#define QTA		1
#define QPTR	12
#define QAAAA	28
#define QCNAME	5
#define QANY	255

#define mDNSA		0x01
#define DNSPTR	0x0C
#define DNSNB		0x20	
#define DNSNBSTAT	0x21
#define HOST_NAME_MAX_LEN 16

typedef struct _DNSHdr {
	unsigned short id;
#if BYTE_ORDER == LITTLE_ENDIAN	
	unsigned char RD:1,TC:1,AA:1,OpCode:4,QR:1;
	unsigned char rCode:4,Reseverd:3,RA:1;
#else
	unsigned char QR:1, OpCode:4,AA:1,TC:1,RD:1;
	unsigned char RA:1,Reseverd:3,rCode:4;
#endif
	
	unsigned short questions;
	unsigned short ans;	
	unsigned short auths;
	unsigned short additions;
}DNSHdr;

typedef struct _NetBIOSResponse{
	DNSHdr dhr;
	char nameLen;
	char name[32];
	char nameTail;	
	unsigned short NB;
	unsigned short InTernet;
	//TTL have alignment issue
	unsigned short TTL1;
	unsigned short TTL2;
	unsigned short len;
	unsigned short flag;
	unsigned char ip[IPv4_ADR_LEN];
}NetBIOSResponse;

typedef struct _NetBIOSNodeStatusResponse{
	DNSHdr dhr;
	char nameLen;
	char name[HOST_NAME_MAX_LEN*2];
	char nameTail;
	unsigned short NB;
	unsigned short InTernet;
	unsigned short TTL1;
	unsigned short TTL2;
	unsigned short rdLen;
	unsigned char numNames;
	char nodeName[HOST_NAME_MAX_LEN];//Dash only have one name
	char uintID[MAC_ADDR_LEN];
	char others[40];
}NetBIOSNodeStatusResponse;

typedef struct _LLMNRHdr {
	DNSHdr dhr;
}LLMNRHdr;

RTSkt* enableNetBIOSPtl();
void disableNetBIOSPtl(RTSkt* s);

RTSkt* enableLLMNRPtl();
void disableLLMNRPtl(RTSkt* s);

#endif

