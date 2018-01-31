/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	arp.h

Abstract:	arp module

*****************************************************************************
*/

/*--------------------------Include File------------------------------------*/
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "bsp.h"
#include "arp.h"
#include "icmp.h"
#include "ip.h"
#include "hwpf.h"

#if (CONFIG_VERSION  > IC_VERSION_A) || defined(CONFIG_BUILDROM)
ROM_EXTERN ARPEntry ArpTable[ARPEntryNum];
ROM_EXTERN int ArpPos;
ROM_EXTERN int Arpv6Pos;
#if defined(CONFIG_IPv6_ENABLED)
unsigned char findIPv6EtherAddr(unsigned char* ipAddr, unsigned char* ethAddr)
{
    int i = 0;
    unsigned char tmpIP[IPv6_ADR_LEN];
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    unsigned char tmpSV6IP[IPv6_ADR_LEN];
    int* tmpSV6IPPtr = (int *) tmpSV6IP;
    char SIPv6FilterOn = 0;
    char tmpV = 0;
    char ibExist = 0;
    char isSIP;
    int *dip;
#endif

    unsigned char err;
    char found = 0;
    
    OSSemPend(ArpTableSem, 0, &err);
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA     
    ibExist = bsp_in_band_exist();
    isSIP = bsp_bits_get(MAC_OOBREG, BIT_SINGLE_IP, 1);
    
    if(ibExist && isSIP)
    {
    	EnableIBIPv6ICMPPFRule();
	}
#endif    
    for ( i = 0 ;i < ARPEntryNum ; i++ )
    {
        if (memcmp(Arpv6Table[i].ipAddr, ipAddr, IPv6_ADR_LEN) == 0)
        {
            if (Arpv6Table[i].state == PENDING)
            {
                memcpy(tmpIP, ipAddr, IPv6_ADR_LEN);
                neighborSolicite(tmpIP);
            }
            else
            {
                found = 1;
            }
            break;
        }
    }

#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
    if(ibExist && isSIP)
    {
        DisableIBIPv6ICMPPFRule();
    }
#endif

    if (found)
    {
        memcpy(ethAddr, Arpv6Table[i].ethAddr, MAC_ADDR_LEN);
        OSSemPost(ArpTableSem);
        return found;
    }

    DEBUGMSG(ARP_DEBUG,"findEtherAddr ==>");

#if CONFIG_VERSION == IC_VERSION_DP_RevF
    ibExist = bsp_in_band_exist();
    isSIP = bsp_bits_get(IO_CONFIG, BIT_SINGLE_IP, 1);
    if (ibExist && isSIP)
    {
        for (i = 0; i < IPv6_ADR_LEN;)
        {
            *(tmpSV6IPPtr + (i/4)) = REG32(IOREG_IOBASE + IO_SIPV6_0 + i);
            i = i + 4;
        }

        for (i = 0; i < IPv6_ADR_LEN;)
        {
            dip = (int*)ipAddr;
            REG32(IOREG_IOBASE + IO_SIPV4_0 + i) = *(dip + (i / 4));
            i = i + 4;
        }

        tmpV = REG8(IOREG_IOBASE + MAC_CMD);
        REG8(IOREG_IOBASE + MAC_CMD) = (tmpV | 0x02);
        if (bsp_bits_get(MAC_RxCR, BIT_FILTERV6SIP0, 1))
            SIPv6FilterOn = 1;
        bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV6SIP0, 1);
    }
#endif

#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
    if(ibExist && isSIP)
    {
        EnableIBIPv6ICMPPFRule();
    }
#endif

    for ( i = 0 ; i < ARPEntryNum ; i++ )
    {
        if (memcmp(Arpv6Table[i].ipAddr, ipAddr, IPv6_ADR_LEN) == 0)
        {
            if (Arpv6Table[i].state == PENDING)
            {
                memcpy(tmpIP, ipAddr, IPv6_ADR_LEN);
                neighborSolicite(tmpIP);
            }
            else
            {
                found = 1;
            }
            break;
        }
    }

    if (!found && (i == ARPEntryNum))
    {
        memset(Arpv6Table[Arpv6Pos].ethAddr, 0, MAC_ADDR_LEN);
        memcpy(Arpv6Table[Arpv6Pos].ipAddr, ipAddr, IPv6_ADR_LEN);
        Arpv6Table[Arpv6Pos].state = PENDING;
        Arpv6Pos++;
        Arpv6Pos = Arpv6Pos%ARPEntryNum;
        memcpy(tmpIP, ipAddr, IPv6_ADR_LEN);
        neighborSolicite(tmpIP);
        DEBUGMSG(ARP_DEBUG,"Add a Entry to find mac address");
    }

    if (found)
    {
        memcpy(ethAddr, Arpv6Table[i].ethAddr, MAC_ADDR_LEN);
    }

#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
    if(ibExist && isSIP)
    {
        DisableIBIPv6ICMPPFRule();
    }
#endif
    DEBUGMSG(ARP_DEBUG,"findEtherAddr <==");
    OSSemPost(ArpTableSem);
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
    if (ibExist && isSIP)
    {
        OSTimeDly(OS_TICKS_PER_SEC * 0.2);
        for (i = 0; i < IPv6_ADR_LEN;)
        {
            REG32(IOREG_IOBASE + IO_SIPV6_0 + i) = *(tmpSV6IPPtr + (i / 4));
            i = i + 4;
        }

        REG8(IOREG_IOBASE + MAC_CMD) = tmpV;

        if (!SIPv6FilterOn)
            bsp_bits_set(MAC_RxCR, 0, BIT_FILTERV6SIP0, 1);
    }
#endif
    return found;
}
#endif
#endif

//#if (CONFIG_VERSION  <= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)

unsigned char findEtherAddr(unsigned char* ipAddr, unsigned char* ethAddr)
{
    int i = 0;
    unsigned char err;
    char found = 0;
    char exist = 0;
#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    char SIPv4FilterOn = 0;
    char tmpV = 0;
    int ibExist = 0;
    int tip = 0;
    int *dip;
    char isSIP;
#endif

    OSSemPend(ArpTableSem, 0, &err);

    for ( i = 0; i < ARPEntryNum ; i++ )
    {
        if (memcmp(ArpTable[i].ipAddr, ipAddr, IPv4_ADR_LEN) == 0)
        {
            if (ArpTable[i].state == PENDING)
            {
            }
            else if (ArpTable[i].timeOut == 0)
            {
            }
            else
            {
                found = 1;
            }
            break;
        }
    }

    if (found)
    {
        memcpy(ethAddr, ArpTable[i].ethAddr, MAC_ADDR_LEN);
        OSSemPost(ArpTableSem);
        return found;
    }

    DEBUGMSG(ARP_DEBUG,"findEtherAddr ==>");

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    ibExist = bsp_in_band_exist();
#if CONFIG_VERSION  > IC_VERSION_DP_RevF
    isSIP = bsp_bits_get(MAC_OOBREG, BIT_SINGLE_IP, 1);
#else
    isSIP = bsp_bits_get(IO_CONFIG, BIT_SINGLE_IP, 1);
#endif
    if (ibExist && isSIP)
    {
        dip = (int*)ipAddr;
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
        tip = REG32(IOREG_IOBASE + IO_SIPV4_0);
        REG32(IOREG_IOBASE + IO_SIPV4_0) = htonl(*dip);
        tmpV = REG8(IOREG_IOBASE + MAC_CMD);
        REG8(IOREG_IOBASE + MAC_CMD) = (tmpV | 0x02);
        if (bsp_bits_get(MAC_RxCR, BIT_FILTERV4SIP0, 1))
            SIPv4FilterOn = 1;
        bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4SIP0, 1);
#endif
    }
#endif

    for ( i = 0; i < ARPEntryNum ; i++ )
    {
        if (memcmp(ArpTable[i].ipAddr, ipAddr, IPv4_ADR_LEN) == 0)
        {
            if (ArpTable[i].state == PENDING)
            {
                sendARPPkt_F(0, ipAddr, ARP_Request, eth0);
                exist = 1;
                DEBUGMSG(ARP_DEBUG,"Pending state and sendARPPkt");
            }
            else if (ArpTable[i].timeOut == 0)
            {
                sendARPPkt_F(0, ipAddr, ARP_Request,eth0);
                exist = 1;
            }
            else
            {
                found = 1;
                exist = 1;
                DEBUGMSG(ARP_DEBUG,"Find Entry");
            }
            break;
        }
    }

    if (!found && !exist)
    {
        memset(ArpTable[ArpPos].ethAddr, 0, MAC_ADDR_LEN);
        memcpy(ArpTable[ArpPos].ipAddr, ipAddr, IPv4_ADR_LEN);
        ArpTable[ArpPos].state = PENDING;
        ArpTable[i].timeOut = 0;
        ArpPos++;
        ArpPos = ArpPos%ARPEntryNum;
        sendARPPkt_F(0, ipAddr, ARP_Request, eth0);
        DEBUGMSG(ARP_DEBUG,"Add a Entry to find mac address");
    }

    if (found)
        memcpy(ethAddr, ArpTable[i].ethAddr, MAC_ADDR_LEN);

    DEBUGMSG(ARP_DEBUG,"findEtherAddr <==");
    OSSemPost(ArpTableSem);

#if CONFIG_VERSION  >=  IC_VERSION_DP_RevF
    if (ibExist && isSIP)
    {
        //OSTimeDly(OS_TICKS_PER_SEC * 0.2);
#if CONFIG_VERSION	== IC_VERSION_DP_RevF

        REG32(IOREG_IOBASE + IO_SIPV4_0) = tip;
        REG8(IOREG_IOBASE + MAC_CMD) = tmpV;

        if (!SIPv4FilterOn)
            bsp_bits_set(MAC_RxCR, 0, BIT_FILTERV4SIP0, 1);
#endif
    }
#endif
    return found;
}

#if (CONFIG_VERSION  <= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)

void updateArpTable(unsigned char* ethAddr, unsigned char* ipAddr)
{
    int i = 0;
    unsigned char err;
    OSSemPend(ArpTableSem, 0, &err);
    DEBUGMSG(ARP_DEBUG,"updateArpTable ==>");

    for ( ; i < ARPEntryNum; i++ )
    {
        if (memcmp(ArpTable[i].ipAddr, ipAddr, IPv4_ADR_LEN) == 0)
        {
            memcpy(ArpTable[i].ethAddr, ethAddr, MAC_ADDR_LEN);
            ArpTable[i].state = RESOLVED;
            ArpTable[i].timeOut = ARPTIMEOUT;
            DEBUGMSG(ARP_DEBUG,"Update a table entry");
            break;
        }
    }

    if (i == ARPEntryNum)
    {
        memcpy(ArpTable[ArpPos].ethAddr, ethAddr, MAC_ADDR_LEN);
        memcpy(ArpTable[ArpPos].ipAddr, ipAddr, IPv4_ADR_LEN);
        ArpTable[ArpPos].state = RESOLVED;
        ArpTable[ArpPos].timeOut = ARPTIMEOUT;
        ArpPos++;
        ArpPos = ArpPos%ARPEntryNum;
        DEBUGMSG(ARP_DEBUG,"Add a table entry");
    }

    OSSemPost(ArpTableSem);
    DEBUGMSG(ARP_DEBUG,"updateArpTable <==");
    return;
}
#endif

#if (CONFIG_VERSION  < IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
/*--------------------------Exported Variables------------------------------------*/
extern OS_EVENT *ArpTableSem;
extern DPCONF *dpconf;
extern _IPInfo IPInfo[];

/*--------------------------Globale Variable------------------------------------*/

ROM_EXTERN unsigned char baoadCastMacAddr[MAC_ADDR_LEN]   = {0xff,0xff,0xff,0xff,0xff,0xff};
ROM_EXTERN ARPv6Entry Arpv6Table[ARPEntryNum];

/*----------------------------Functions-------------------------------------*/

void sendARPPkt(ARPPkt *ptr, unsigned char* clientIP, unsigned char type, unsigned char intf)
{
    unsigned char *dstIPAddr = 0,*dstEthAddr = 0;
    unsigned int i = 0;
#if !WIN
    //char err;
#endif
    ARPPkt *arpPkt = 0;

    DEBUGMSG(ARP_DEBUG,"sendARPPkt ==>");

    if (dpconf->HostIP[intf].addr == 0)
        return;

    if (ptr == NULL && type == ARP_Reply)
    {
        DEBUGMSG(ARP_DEBUG,"ptr is NULL and ARP_Reply");
        return;
    }

    if (clientIP == NULL && type == ARP_Request)
    {
        DEBUGMSG(ARP_DEBUG,"clientIP is NULL and ARP_Request");
        goto ArpFree;
        return;
    }

    if (ptr)
    {
        //Clients response to host arp requests
        if (ptr->ah.operation == htons(ARP_Reply))
        {
            if (memcmp(ptr->ad.v4.srcIPAddr, &dpconf->HostIP[intf].addr, IPv4_ADR_LEN) == 0)
                IPInfo[intf].IPv4Gratuitous = 1;
            else
                updateArpTable_F(ptr->ad.v4.srcEthAddr, ptr->ad.v4.srcIPAddr);
            goto ArpFree;
            return;
        }
        else
        {
            //Client arp request and host need to do arp reply
            if (!intfIPv4Enable(intf) || ptr->ah.protocolLen != IPv4_ADR_LEN)
            {
                DEBUGMSG(ARP_DEBUG,"IPv4 do not enable");
                goto ArpFree;
                return;
            }
        }
    }

    arpPkt = (ARPPkt *)malloc(sizeof(ARPPkt));

    if (type == ARP_Reply)
    {
        DEBUGMSG(ARP_DEBUG,"ARP_Reply");

        if (*((unsigned int*)(ptr->ad.v4.targetIPAddr)) != getIPAddress_F(intf)) //check IP address match
        {
            goto ArpFree;
            return;
        }

        dstIPAddr = ptr->ad.v4.srcIPAddr;
        dstEthAddr = ptr->ad.v4.srcEthAddr;
        memcpy(arpPkt->ad.v4.targetEthAddr, dstEthAddr, MAC_ADDR_LEN);
    }
    else if (type == ARP_Request)
    {
        DEBUGMSG(ARP_DEBUG,"ARP_Request");
        if (dpconf->IPv4Enable)
        {
            memset(arpPkt->ad.v4.targetEthAddr, 0 , MAC_ADDR_LEN);
            dstEthAddr = baoadCastMacAddr;
            dstIPAddr = clientIP;
        }
    }
    else
    {
        DEBUGMSG(ARP_DEBUG,"Unknown Arp type");
        goto ArpFree;
        return;
    }

    arpPkt->ah.hwType = htons(ETH_HW_TYPE);
    arpPkt->ah.protocolType = htons(Ethernet_Protocol_IP);
    arpPkt->ah.operation = htons(type);
    arpPkt->ah.hwLen = 6;
    arpPkt->ah.protocolLen = IPv4;

    //memcpy(arpPkt->ad.v4.srcEthAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
    GetMacAddr_F(arpPkt->ad.v4.srcEthAddr, intf);
    i = getIPAddress_F(intf);
    memcpy(arpPkt->ad.v4.srcIPAddr, &i,IPv4_ADR_LEN);
    memcpy(arpPkt->ad.v4.targetIPAddr, dstIPAddr, IPv4_ADR_LEN);

    //Ethernet Header
    //memcpy(arpPkt->ah.ethHdr.srcAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
	
    GetMacAddr_F(arpPkt->ah.ethHdr.srcAddr, intf);
    memcpy(arpPkt->ah.ethHdr.destAddr, dstEthAddr, MAC_ADDR_LEN);
    arpPkt->ah.ethHdr.ethType = htons(ETHERNET_TYPE_ARP);

#if WIN
    DMWGWriteProc(((unsigned char*)arpPkt) + 2, sizeof(ARPHdr) + sizeof(ARPv4) - 2);
    free(arpPkt);
#else
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
	if(intf == eth0)
		gmacsend_F(((unsigned char*)arpPkt) + 2 , sizeof(ARPHdr) + sizeof(ARPv4) - 2, OPT_NOCS | OPT_FREE, arpPkt);
	else
		pci_tx(((unsigned char*)arpPkt) + 2 , sizeof(ARPHdr) + sizeof(ARPv4) - 2, OPT_NOCS | OPT_FREE, arpPkt);
#else
		send_F(((unsigned char*)arpPkt) + 2 , sizeof(ARPHdr) + sizeof(ARPv4) - 2, OPT_NOCS | OPT_FREE, arpPkt);

#endif
#endif
    DEBUGMSG(ARP_DEBUG,"sendARPPkt <==");
    return ;

ArpFree:
    if (arpPkt)
        free(arpPkt);

    DEBUGMSG(ARP_DEBUG,"sendARPPkt Free <==");
    return;
}



void arpTableTimeOutUpdate()
{
    int i = 0;

    for ( ; i < ARPEntryNum ; i++ )
    {
        if (ArpTable[i].timeOut > 0)
        {
            ArpTable[i].timeOut = ArpTable[i].timeOut - 5;
        }
    }
}

void arpTableTimeOutReset()
{
    int i = 0;

    for ( ; i < ARPEntryNum ; i++ )
    {
        ArpTable[i].timeOut = 0;
    }
}

#endif
