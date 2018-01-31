/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	FWUpdate.h

*****************************************************************************
*/
#include <sys/inet_types.h>

#ifndef _FWUPDATE_INCLUDED
#define _FWUPDATE_INCLUDED

/*--------------------------Definitions------------------------------------*/

#define ETH_HDR_SIZE	16
#define IP_HDR_SIZE		20
#define UDP_HDR_SIZE	8

#define MAC_ADDR_LEN	6
#define IPv4_ADR_LEN	4
#define FWUpdateSrvPt		0x8168

//FW update OP Code
#define FW_AUTH_REQ	 0x10
#define FW_AUTH_FAIL 0x11
#define FW_AUTH_PFAIL 0x12
#define FW_AUTH_PASS 0x13

#define FW_VERIFY_REQ	 0x20
#define FW_VERIFY_FAIL 0x21
#define FW_VERIFY_PASS 0x22

#define FW_WRQ		 0x31
#define FW_WCFGRQ	 0x32
#define FW_ACK		 0x33
#define FW_ACKWCFG 0x34
#define FW_FIN		 0x35
#define FW_WADDR	 0x36

#define FW_ABORT	 0x40 //Not expect client
#define FW_BLK_ERR	 0x41

#define FW_ICINFO	 0x51

typedef struct
{
	unsigned char opCode;
	unsigned char unUsed;
	union
	{		
	unsigned char auth[16];
		unsigned char username[16];
	};
	unsigned short blkNum;
	unsigned short size;//Opcode = FW_ACK => how many byte received
						//Opcode = FW_WRQ => how many byte want to update 
}FWUpdateHdr;

typedef struct _FWUserInfoHdr{
    unsigned int length;
    unsigned int number;
    unsigned char res[56];
}FWUserInfoHdr;

typedef struct _ICInfo{
	unsigned int  pkg_det;
    unsigned char ver;
}ICInfo;

/*
typedef struct _FWUserInfo{
    unsigned char name[16];
    unsigned char passwd[16];
    unsigned char role;
    unsigned char opt[3];
    unsigned char caption[24];
    struct _FWUserInfo *next;
}FWUserInfo;
*/

void fwInituser_R();
void fwUpdateInit_R();
void fwUpdate_R(void* pData, int rxLen, ptcp_pcb pcb);
void fwUpdateInput(void *rxbuf);
void DHCPv4Task_R(void *data);
void tcpTimerTask_R(void *data);
void tcpProcess_R(PPKT pkt, ptcp_pcb pcbPtr);
void OOBResponseTask_R(void *data);

#endif

