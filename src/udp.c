/*
******************************* SOURCE FILE *******************************
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	udp.c

Abstract:	UDP module

*****************************************************************************
*/

/*--------------------------Include Files-----------------------------------*/

#include <string.h>
#include "lib.h"
#include "ip.h"
#include "udp.h"
#include "tcp.h"
#include "dhcp.h"
#include "arp.h"
#include "bsp.h"
#if CONFIG_mDNS_OFFLOAD_ENABLED
#include "mDNS.h"
extern struct _mDNSOffloadCmd mDNSOffloadCmd;
#endif

#if (CONFIG_VERSION  < IC_VERSION_DP_RevF) || CONFIG_BUILDROM
extern TCPPCB *ActivePCBsList;

void udpSend(void *p,void* data, unsigned short len)
{
    UDPHdr *uh;
    TCPPCB *pcb = (TCPPCB*)p;
    PKT* pkt = (PKT*) data;
    IPv6PseudoHdr* ipv6PseudoHdr;
    IPv4PseudoHdr* ipv4PseudoHdr;
    int ip;

    DEBUGMSG(UDP_DEBUG,"udpSend ==>");

    pkt->wp = pkt->wp - UDP_HDR_SIZE;
    pkt->len = len + UDP_HDR_SIZE;
    pkt->intf = pcb->intf;

    uh = (UDPHdr *)(pkt->wp);
    uh->srcPort = htons(pcb->localPort);
    uh->desPort = htons(pcb->destPort);
    uh->length =  htons(pkt->len);
    uh->chkSum = 0;
    pkt->ipVer = pcb->ipVer; 

    if (pkt->ipVer == IPv4)
    {
        ip = getIPAddress_F(pkt->intf);
        ipv4PseudoHdr = (IPv4PseudoHdr*)(pkt->wp - sizeof(IPv4PseudoHdr));
        memcpy(ipv4PseudoHdr->src, &ip, IPv4_ADR_LEN);
        memcpy(ipv4PseudoHdr->dest, &pcb->destIPAddr, IPv4_ADR_LEN);
        ipv4PseudoHdr->unUsed = 0;
        ipv4PseudoHdr->protocol = IP_PROTO_UDP;
        ipv4PseudoHdr->totalLen = htons((unsigned short)len + UDP_HDR_SIZE);
#ifndef CKHSUM_OFFLOAD
        uh->chkSum = inet_chksum(pkt->wp - sizeof(IPv4PseudoHdr),  sizeof(IPv4PseudoHdr) + len + UDP_HDR_SIZE);
#endif
        DEBUGMSG(UDP_DEBUG,"udpSend IPv4");
    }
    else //pkt->ipVer == IPv6
    {
        ipv6PseudoHdr = (IPv6PseudoHdr*)(pkt->wp - sizeof(IPv6PseudoHdr));
        memcpy(ipv6PseudoHdr->src, getIPv6Address_F(pkt->intf), IPv6_ADR_LEN);
        memcpy(ipv6PseudoHdr->dest, pcb->destIPv6Addr, IPv6_ADR_LEN);
        ipv6PseudoHdr->nextHdr = IP_PROTO_UDP;
        ipv6PseudoHdr->unUsed = 0;
        ipv6PseudoHdr->payLoadLen = htons((unsigned short)len + UDP_HDR_SIZE);
        uh->chkSum = 0;
        uh->chkSum = v6pseudochksum_F(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr));
#ifndef CKHSUM_OFFLOAD
        uh->chkSum = inet_chksum(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr) + len + UDP_HDR_SIZE);
#endif
        DEBUGMSG(UDP_DEBUG,"udpSend IPv6");
    }
    addIPHeader_F(p, data);
    DEBUGMSG(UDP_DEBUG,"udpSend <==");
}

void udpInput(PKT *pkt)
{
    TCPPCB *ipcb;
    UDPHdr *uh = (UDPHdr *)(pkt->wp);
    IPHdr *ipHdr = NULL;
    IPv6Hdr *ipv6Hdr = NULL;
#if CONFIG_mDNS_OFFLOAD_ENABLED
	int i = 0;

	for(i = 0; i < mDNSOffloadCmd.numUDPPorts; i++)
	{
		if(mDNSOffloadCmd.udpPorts[i] == uh->desPort)
		{
			//Power on
			SMBus_Prepare_RmtCtrl(RMCP_PWR_On, 0);
			return;
		}
	}		
#endif

    for (ipcb = ActivePCBsList; ipcb != NULL; ipcb = ipcb->next) {
        if (ipcb->protocol != IP_PROTO_UDP)
            continue;
        //(ntohs(uh->srcPort) == ipcb->destPort) &&
        if ((ntohs(uh->desPort) == ipcb->localPort) && ipcb->rxEnable && matchIntf(ipcb, pkt)) {
            //Calculate AP data length
            updateArpTable_F(pkt->start + 2 + MAC_ADDR_LEN, pkt->start + ETH_HDR_SIZE + IP_HLEN - IPv4_ADR_LEN *2);
            pkt->wp = pkt->wp + UDP_HDR_SIZE;
            pkt->len = ntohs(uh->length) - UDP_HDR_SIZE;
            DEBUGMSG(UDP_DEBUG,"Found a matching UDP PCB ==>\n");

            if (ipcb->apRx)
            {
                if (pkt->ipVer == IPv4 && ipcb->ipVer == IPv4)
                {
                    ipHdr = (IPHdr *)(pkt->wp - IP_HLEN - UDP_HDR_SIZE);
                    memcpy(ipcb->dest_MAR, pkt->start + 2 + MAC_ADDR_LEN, MAC_ADDR_LEN);
                    ipcb->destIPAddr.addr = ipHdr->src.addr;
                    ipcb->destPort = ntohs(uh->srcPort);
                    ipcb->apRx(pkt->wp, pkt->len,ipcb);
                    break;
                }

                if (pkt->ipVer == IPv6 && ipcb->ipVer == IPv6)
                {
                    ipv6Hdr = (IPv6Hdr *)(pkt->wp - IPv6_HLEN - UDP_HDR_SIZE);
                    memcpy(ipcb->dest_MAR, pkt->start + 2 + MAC_ADDR_LEN, MAC_ADDR_LEN);
                    memcpy(ipcb->destIPv6Addr, ipv6Hdr->src, IPv6_ADR_LEN);
                    ipcb->destPort = ntohs(uh->srcPort);
                    ipcb->apRx(pkt->wp, pkt->len,ipcb);
                    break;
                }
            }
            else
            {
                tcpRxInput_F(pkt, ipcb, 0);
            		break;
            		
            }
        }
    }
    DEBUGMSG(UDP_DEBUG,"udpInput <==");
}
#endif
