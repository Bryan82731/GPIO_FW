/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	htm.h

Abstract:	This file records html source code and relative data

*****************************************************************************
*/

#ifndef __HTM_H__
#define __HTM_H__

#include <sys/inet_types.h>
#include <sys/autoconf.h>
#ifdef CONFIG_MSAD_ENABLED
#include "gssapi.h"
#endif

typedef enum _WebPageID
{
    NOTFOUND    = 0,
    RDCSS       = 1,
    LOGOGIF     = 2,
    BANNERPAGE  = 3,
    HOMEPAGE    = 4,
    WELCOMEPAGE = 5,
    INFOPAGE    = 6,
    REMOTECTLPAGE   = 7,
    REMOTECTLINFO = 8,
    EVENTLOGPAGE = 0x61,
    BOOTSETPAGE = 9,
    SECSETPAGE  = 10,
    NETSETPAGE  = 11,
    NETSETv6PAGE  = 12,
    ABOUTPAGE	  = 13,
    AUTHPAGE	  = 15,
    AUTHFAILPAGE  = 16,
    OKPAGE = 17,
    EZSHAREPAGE = 18,
    EZSHAREBIN = 19,
   #if CONFIG_ACER_WEB_UI
    ACERWBGTRIGGIF = 20,
    ACERINFOABGGIF = 21,
    ACERINFOBGGIF = 22,
    ACERBANBGGIF = 23,
    ACERLISTLABELGIF = 24,
    ACERBGTRIGIF = 25,
   #endif
    SRVSETPAGE = 0x60,
    JOBINFOPAGE = 19,
    STAINFOPAGE = 20,
    PROINFOPAGE = 21,
    MAIINFOPAGE = 22,
    SUPINFOPAGE = 23,
    WAKEINFOPAGE = 24,
    JOBLISTPAGE = 25,
    /*JOBALLPAGE = 25*/
    JOBREPORTPAGE = 26,
    GENERALPAGE = 27,
    PAPERTRAYPAGE = 28,
    OUTPUTTRAYPAGE = 29,
    DRUMPAGE = 30,
    MACHINEPAGE = 31,
    MEMORYPAGE = 32,
    COUNTERPAGE = 33,
    PAPERTRAYATTRIPAGE = 34,
    PAPERSETTINGPAGE = 35,
    POWERSETTINGPAGE = 36,
    INTERNETSETTINGPAGE = 37,
    PORTSTATUSPAGE = 38,
    ETHERNETPAGE = 39,
    USBPAGE = 40,
    SMBPAGE = 41,
    TCPIPPAGE = 42,
    LPDPAGE = 43,
    SNMPPAGE = 44,
    IPPPAGE = 45,
    PORT9100PAGE = 46,
    HTTPPAGE = 47,
    PRINTMODEPAGE = 48,
    POSTSCRIPTPAGE = 49,
    HPGl2PAGE = 50,
    TIFFPAGE = 51,
    PCLPAGE = 52,
    MEMORYSETTINGPAGE = 53,
    ERRORHISTORYPAGE = 54,
    SUPPORTPAGE = 55,
    //mail_wakeup 201211025 eccheng
    MAIL_WAKEUP_ACC = 61,
    MAIL_WAKEUP_MAT = 62,
    COMPUTERSYSPAGE = 63,
    OPERATIONSYSPAGE = 64,
    SOFTWAREIDPAGE = 65,
    BIOSPAGE = 66,
    PROCESSORPAGE = 67,
    FANPAGE = 68,
    SENSORPAGE = 69,
    ASSETPAGE = 70,
    SHOWCOUNTER = 71,
    RESETCOUNTER = 72,
	WIFICONFPAGE = 80
   
}WebPageID;
int sendWebPages(ptcp_pcb pcb, char *url);
void sendStaticWebPage(struct tcp_pcb *pcb,int wpid);
#ifdef CONFIG_MSAD_ENABLED
WebPageID getWebPageID(char *url);
#endif
#endif
