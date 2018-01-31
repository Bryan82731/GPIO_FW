/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	icmp.c

Abstract:	ICMP module

Note:
*Mapping IPv6 Multicast Address to
Ethernet MAC Address
-Use specific vendor code= 33-33 for multicast use
-Avoid address duplicate in solicited-node address
	(lower 24-bit)
	For examples, a host with
	Ethernet MAC= 00-AA-00-3F-2A-1C
	Link local address= FE80::2AA:FF:FE3F:2A1C
	33-33-00-00-00-01 (link-local all-nodes multicast address => FF02::1)
	33 33 FF 3F 2A 1C
	(solicited node 33-33-FF-3F-2A-1C solicited-multicast address
	=> FF02::1:FF3F:2A1C)

*****************************************************************************
*/

/*--------------------------Include File------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include "tcp.h"
#include "ip.h"
#include "arp.h"
#include "icmp.h"
#include "lib.h"
#include "debug.h"
#include "bsp.h"

extern DPCONF *dpconf;
extern struct _DHCPv6Conf DHCPv6Conf[];
extern int Arpv6Pos;
extern OS_EVENT *ArpTableSem;
extern _IPInfo IPInfo[];
#if CONFIG_TEREDO_ENABLED
extern char TeredoState;
#endif
extern OS_EVENT *SktSem;

#ifdef CONFIG_REMOTE_DEBUG
extern int idle_times;
void icmpInput_debug(PKT *pkt, int len)
{
    PEthHdr	eHdr;
    IPHdr	*ipHdr;
    PKT *outPkt;
    ICMPHdr *outHdr;
    TCPPCB	*newPCB;
    ICMPHdr *inHdr = (ICMPHdr*) pkt->wp;

	INT32U ttimer;
	INT32U tt = 0;

#ifdef CONFIG_PROFILING
    mgtcycles[3] = rlx_cp3_get_counter_lo(0);
#endif
    DEBUGMSG(ICMP_DEBUG,"icmpInput ==>");
    eHdr = (PEthHdr)(pkt->wp - IP_HLEN - ETH_HDR_SIZE);
    ipHdr = (IPHdr *)(pkt->wp - IP_HLEN);

    //Current RTTCP only support echo service
    if (inHdr->type != ECHO_REQ)
    {
        DEBUGMSG(ICMP_DEBUG,"Type not match");
        return;
    }
    if (ipHdr->dest.addr != getIPAddress_F(pkt->intf))
    {
        DEBUGMSG(ICMP_DEBUG,"IP not match");
        return;
    }

    newPCB = (TCPPCB*) malloc(sizeof(TCPPCB));
    memset(newPCB, 0, sizeof(TCPPCB));
    newPCB->protocol = IP_PROTO_ICMP;

    newPCB->dest_MAR[0] = eHdr->srcAddr[0];
    newPCB->dest_MAR[1] = eHdr->srcAddr[1];
    newPCB->dest_MAR[2] = eHdr->srcAddr[2];
    newPCB->dest_MAR[3] = eHdr->srcAddr[3];
    newPCB->dest_MAR[4] = eHdr->srcAddr[4];
    newPCB->dest_MAR[5] = eHdr->srcAddr[5];

    newPCB->destIPAddr.addr = ipHdr->src.addr;

    outPkt = allocIPPkt_F(len, IPv4);

    //ICMP data
    memcpy(outPkt->wp, pkt->wp, len);

    outHdr = (ICMPHdr*)(outPkt->wp);
    outHdr->type = ECHO_REPLY;

    if (inHdr->chksum >= htons(0xffff - (ECHO_REQ << 8)) )
        outHdr->chksum += htons(ECHO_REQ << 8) + 1;
    else
        outHdr->chksum += htons(ECHO_REQ << 8);

    addIPHeader_F(newPCB, (PKT*)outPkt);
    DEBUGMSG(ICMP_DEBUG,"icmpInput <==");


	/*for(ttimer=0;ttimer<5;ttimer++){
		OSTimeDly(10);
		printf("[RTK] Timer 0.1s[%d]\n",ttimer);
	}*/
	tt=idle_times; //Use to Check Idle Task Patched
	printf("[RTK] idletimes 0x%x\n", tt);
	
}
#endif

#if (defined(CONFIG_BUILDROM) && !CONFIG_IPv6_ENABLED) || (CONFIG_VERSION < IC_VERSION_DP_RevF)
void icmpInput(PKT *pkt, int len)
{
    PEthHdr	eHdr;
    IPHdr	*ipHdr;
    PKT *outPkt;
    ICMPHdr *outHdr;
    TCPPCB	*newPCB;
    ICMPHdr *inHdr = (ICMPHdr*) pkt->wp;

#ifdef CONFIG_PROFILING
    mgtcycles[3] = rlx_cp3_get_counter_lo(0);
#endif
    DEBUGMSG(ICMP_DEBUG,"icmpInput ==>");
    eHdr = (PEthHdr)(pkt->wp - IP_HLEN - ETH_HDR_SIZE);
    ipHdr = (IPHdr *)(pkt->wp - IP_HLEN);

    //Current RTTCP only support echo service
    if (inHdr->type != ECHO_REQ)
    {
        DEBUGMSG(ICMP_DEBUG,"Type not match");
        return;
    }
    if (ipHdr->dest.addr != getIPAddress_F(pkt->intf))
    {
        DEBUGMSG(ICMP_DEBUG,"IP not match");
        return;
    }

    newPCB = (TCPPCB*) malloc(sizeof(TCPPCB));
    memset(newPCB, 0, sizeof(TCPPCB));
    newPCB->protocol = IP_PROTO_ICMP;

    newPCB->dest_MAR[0] = eHdr->srcAddr[0];
    newPCB->dest_MAR[1] = eHdr->srcAddr[1];
    newPCB->dest_MAR[2] = eHdr->srcAddr[2];
    newPCB->dest_MAR[3] = eHdr->srcAddr[3];
    newPCB->dest_MAR[4] = eHdr->srcAddr[4];
    newPCB->dest_MAR[5] = eHdr->srcAddr[5];

    newPCB->destIPAddr.addr = ipHdr->src.addr;

    outPkt = allocIPPkt_F(len, IPv4);
	outPkt->intf = pkt->intf;
	newPCB->intf= pkt->intf;

    //ICMP data
    memcpy(outPkt->wp, pkt->wp, len);

    outHdr = (ICMPHdr*)(outPkt->wp);
    outHdr->type = ECHO_REPLY;

    if (inHdr->chksum >= htons(0xffff - (ECHO_REQ << 8)) )
        outHdr->chksum += htons(ECHO_REQ << 8) + 1;
    else
        outHdr->chksum += htons(ECHO_REQ << 8);

    addIPHeader_F(newPCB, (PKT*)outPkt);
    DEBUGMSG(ICMP_DEBUG,"icmpInput <==");
}
#endif

#if CONFIG_ROM_VERIFY || !defined(CONFIG_BUILDROM)
void routerAdvertise(unsigned char *p, int len, unsigned char intf)
{
    IPv6Hdr	*ipv6Hdr;
    ICMPv6Hdr* inHdr;
    RouterAdverMsg* msg;
    ICMPOpt* ICMPOptHdr;
    ICMPPrefixOpt* pOpt;
    char i;

    ipv6Hdr = (IPv6Hdr*) (p - IPv6_HLEN);
    memcpy(dpconf->IPv6GateWay[intf], ipv6Hdr->src, IPv6_ADR_LEN);
    inHdr = (ICMPv6Hdr*)p;
    msg = (RouterAdverMsg*)(p + sizeof(ICMPv6Hdr));
    DHCPv6Conf[intf].M = msg->M;
    DHCPv6Conf[intf].O = msg->O;

    len = len - sizeof(ICMPv6Hdr) - sizeof(RouterAdverMsg);
    p = p + sizeof(ICMPv6Hdr) + sizeof(RouterAdverMsg);
    //IPv6GateWay
    while (len > 0)
    {
        ICMPOptHdr = (ICMPOpt*) p;

        if (ICMPOptHdr->type == 0x03)
        {
            pOpt = (ICMPPrefixOpt*)(p);
            memcpy(dpconf->IPv6Prefix[intf], &pOpt->prefix, pOpt->prefixLen/8);
            if ((pOpt->prefixLen%8) != 0)
            {
                i = pOpt->prefixLen%8;
                i = 1<<(8 - i);
                i = 256 - i;
                i = *(&pOpt->prefix + pOpt->prefixLen/8) & i;
                dpconf->IPv6Prefix[intf][pOpt->prefixLen/8] = i;
                //dpconf->IPv6Prefix[pOpt->prefixLen/8] = *(&pOpt->prefix + pOpt->prefixLen/8);
            }
            dpconf->IPv6PrefixLen[intf] = pOpt->prefixLen;
        }

        len = len - ICMPOptHdr->len * 8;
        p += ICMPOptHdr->len * 8;
    }

}

#if defined(CONFIG_TEREDO_ENABLED) || CONFIG_VERSION != IC_VERSION_DP_RevF
void icmpv6EchoReply(unsigned char *p, int len, unsigned char intf)
{
    TCPPCB	*newPCB;
    PEthHdr	eHdr;
    IPv6Hdr	*ipv6Hdr;

    ICMPHdr *reqHdr;
    ICMPHdr* outHdr;
    unsigned char *outPkt;
    PKT *pkt;

    IPv6PseudoHdr* ipv6PseudoHdr;
    int pktLen = 0;

    eHdr = (PEthHdr)(p - IPv6_HLEN - ETH_HDR_SIZE);
    ipv6Hdr = (IPv6Hdr*) (p - IPv6_HLEN);
    reqHdr = (ICMPHdr*) p;

    if (memcmp(ipv6Hdr->dest, getIPv6Address_F(intf), IPv6_ADR_LEN) != 0)
        return;

    newPCB = (TCPPCB*) malloc(sizeof(TCPPCB));
    memset(newPCB, 0, sizeof(TCPPCB));
    newPCB->protocol = IP_PROTO_ICMPv6;

    memcpy(newPCB->dest_MAR, eHdr->srcAddr, MAC_ADDR_LEN);
    memcpy(newPCB->destIPv6Addr, ipv6Hdr->src, IPv6_ADR_LEN);
    pktLen = htons(ipv6Hdr->len);


    outPkt = allocIPPkt_F(pktLen, IPv6);
    pkt =(PKT*) outPkt;
	pkt->intf = intf;
    //ICMP data
    memcpy(pkt->wp, p, pktLen);
    outHdr = (ICMPHdr*)(pkt->wp);
    outHdr->type = ECHO_REPLYv6;

    if (reqHdr->chksum >= htons(0xffff - (ICMP6_ECHO << 8)) )
        outHdr->chksum += htons(ICMP6_ECHO << 8) + 1;
    else
        outHdr->chksum += htons(ICMP6_ECHO << 8);

    outHdr->chksum = 0;
    ipv6PseudoHdr = (IPv6PseudoHdr*)(pkt->wp - sizeof(IPv6PseudoHdr));
    memcpy(ipv6PseudoHdr->src, getIPv6Address_F(intf), IPv6_ADR_LEN);
    memcpy(ipv6PseudoHdr->dest, newPCB->destIPv6Addr, IPv6_ADR_LEN);
    ipv6PseudoHdr->nextHdr = IP_PROTO_ICMPv6;
    ipv6PseudoHdr->unUsed = 0;
    ipv6PseudoHdr->payLoadLen = htons((unsigned short)len);

//Rtk do not support icmpv6 checksum offload
//#ifndef CKHSUM_OFFLOAD
    outHdr->chksum = inet_chksum(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr) + len);
//#endif

    pkt->len = len;
    addIPHeader_F(newPCB, (PKT*)pkt);

#if CONFIG_TEREDO_ENABLED
    //teredo use udp socket to send
    if (dpconf->useTeredo)
    {
        free(newPCB);
    }
#endif
}

void icmpv6Input(unsigned char *p, int len, unsigned char intf)
{
    ICMPv6Hdr* inHdr;

    DEBUGMSG(ICMP_DEBUG,"icmpv6Input ==>");
    inHdr = (ICMPv6Hdr*) p;

    switch (inHdr->type)
    {
    case SOLICITATION:
        icmpv6Solicitation(p, len, intf);
        break;
    case ADVERTISEMENT:
        icmpv6Advertisement(p, len, intf);
        if (DHCPv6Conf[intf].DHCPv6CheckEUIAddr)
            chkLinkLocalEUIAddr(p, len, intf);
        break;
    case ROUTERADVERTISEMENT:
        routerAdvertise(p, len, intf);
        DHCPv6Conf[intf].DHCPv6RouterAdver = 1;
        break;
    case ECHO_REQv6:
        if (dpconf->EchoService)
            icmpv6EchoReply(p, len, intf);
        break;
#if CONFIG_TEREDO_ENABLED
    case ECHO_REPLYv6:
        updateTeredoPeerEntry(p, 1, 0, 0);
        break;
#endif
    default:
        DEBUGMSG(ICMP_DEBUG,"Unknown type");
        break;
    }
    DEBUGMSG(ICMP_DEBUG,"icmpv6Input <==");
}

#endif

#if CONFIG_TEREDO_ENABLED
void teredoRS()
{
    TCPPCB	*newPCB;
    ICMPv6Hdr* outHdr;
    RouterSolMsg *msg;
    unsigned char *outPkt;
    IPv6PseudoHdr* ipv6PseudoHdr;
    PKT* pkt;
    unsigned char linkLocal[IPv6_ADR_LEN] = {0};
    int pktLen = 0;
    TeredoAddrFmt *tAddr;
    unsigned char err;

    OSSemPend(SktSem, 0, &err);

    newPCB = (TCPPCB*) malloc(sizeof(TCPPCB));
    memset(newPCB, 0, sizeof(TCPPCB));
    newPCB->protocol = IP_PROTO_ICMPv6;

    pktLen = sizeof(ICMPv6Hdr) + sizeof(RouterSolMsg);
    outPkt = allocPkt(pktLen);

    pkt =(PKT*) outPkt;
    pkt->ipVer = IPv6;

    outHdr = (ICMPv6Hdr*)(pkt->wp);
    outHdr->type = (unsigned char)ROUTERSOLICITATION;
    outHdr->code = 0;
    outHdr->chksum = 0;

    msg = (RouterSolMsg*)(pkt->wp + sizeof(ICMPv6Hdr));
    msg->flag = 0;
    msg->opcode = 1;
    msg->opLen = 1;

    memcpy(msg->macAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);

    linkLocal[0] = 0xFF;
    linkLocal[1] = 0x02;
    linkLocal[IPv6_ADR_LEN - 1] = 0x02;
    memcpy(newPCB->destIPv6Addr, linkLocal , IPv6_ADR_LEN);

    getLinkLocalEUIAddr(linkLocal);
    tAddr = (TeredoAddrFmt*)linkLocal;
    tAddr->C = 0; //Cone can be replaced by restricted
    tAddr->z = 0;
    tAddr->z1 = 0;
    tAddr->UG = 0;

    if (TeredoState != TEREDO_QUAILIFIED)
    {
        setLinkLocalAddress(linkLocal);
    }

    ipv6PseudoHdr = (IPv6PseudoHdr*)(pkt->wp - sizeof(IPv6PseudoHdr));
    memcpy(ipv6PseudoHdr->src, getIPv6Address(), IPv6_ADR_LEN);

    memcpy(ipv6PseudoHdr->dest, newPCB->destIPv6Addr, IPv6_ADR_LEN);
    ipv6PseudoHdr->nextHdr = IP_PROTO_ICMPv6;
    ipv6PseudoHdr->unUsed = 0;
    ipv6PseudoHdr->payLoadLen = htons((unsigned short)pktLen);

    outHdr->chksum = inet_chksum(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr) + pktLen);
    addIPHeader_F(newPCB, (PKT*)outPkt);
    free(newPCB);
    OSSemPost(SktSem);
    return;
}

int chkTeredoIndicator(PKT* pkt, unsigned short* originPort, unsigned int* originIP)
{
    TeredoIndHdr* hdr;

    if (pkt->len < sizeof(TeredoIndHdr))
    {
        return TEREDO_FAIL;
    }

    hdr = (TeredoIndHdr*)pkt->wp;

    if ((memcmp(&IPInfo[pkt->intf].TeredoCPort, &hdr->OriginPort, sizeof(hdr->OriginPort))!=0) ||
            (memcmp(&IPInfo[pkt->intf].TeredoCIP, &hdr->OriginIP, sizeof(hdr->OriginIP)!=0)))
    {
        memcpy(&IPInfo[pkt->intf].TeredoCPort, &hdr->OriginPort, sizeof(hdr->OriginPort));
        memcpy(&IPInfo[pkt->intf].TeredoCIP, &hdr->OriginIP, sizeof(hdr->OriginIP));
        IPInfo[pkt->intf].TeredoIPChg = 1;
    }

    if (originPort)
    {
        memcpy(originPort, &hdr->OriginPort, sizeof(hdr->OriginPort));
        memcpy(originIP,&hdr->OriginIP, sizeof(hdr->OriginIP));
    }

    pkt->len = pkt->len - sizeof(TeredoIndHdr);
    pkt->wp = pkt->wp + sizeof(TeredoIndHdr);

    return 0;
}

int teredoRA(PKT* pkt)
{
    IPv6Hdr	*ipv6Hdr;
    ICMPv6Hdr* icmpHdr;
    RouterAdverMsg* msg;
    ICMPOpt* ICMPOptHdr;
    ICMPPrefixOpt* pOpt;
    char i;

    if (chkTeredoAuthHdr(pkt) == TEREDO_FAIL)
    {
        return TEREDO_FAIL;
    }

    if (chkTeredoIndicator(pkt, 0, 0) == TEREDO_FAIL)
    {
        return TEREDO_FAIL;
    }

    if (pkt->len < IPv6_HLEN)
    {
        return TEREDO_FAIL;
    }

    ipv6Hdr = (IPv6Hdr*) pkt->wp;
    memcpy(dpconf->IPv6GateWay[pkt->intf], ipv6Hdr->src, IPv6_ADR_LEN);

    if (ipv6Hdr->nexthdr != IP_PROTO_ICMPv6)
    {
        return TEREDO_FAIL;
    }

    pkt->wp = pkt->wp + IPv6_HLEN;
    pkt->len = pkt->len - IPv6_HLEN;

    if (pkt->len < sizeof(ICMPv6Hdr))
    {
        return TEREDO_FAIL;
    }

    icmpHdr = (ICMPv6Hdr*)pkt->wp;
    if (icmpHdr->type != ROUTERADVERTISEMENT)
    {
        return TEREDO_FAIL;
    }

    pkt->wp = pkt->wp + sizeof(ICMPv6Hdr);
    pkt->len = pkt->len - sizeof(ICMPv6Hdr);

    if (pkt->len < sizeof(RouterAdverMsg))
    {
        return TEREDO_FAIL;
    }

    msg = (RouterAdverMsg*)pkt->wp;

    pkt->wp = pkt->wp + sizeof(RouterAdverMsg);
    pkt->len = pkt->len - sizeof(RouterAdverMsg);

    //IPv6GateWay
    while (pkt->len > 0)
    {
        ICMPOptHdr = (ICMPOpt*) pkt->wp;

        if (ICMPOptHdr->type == 0x03)
        {
            pOpt = (ICMPPrefixOpt*)(pkt->wp);
            memcpy(dpconf->IPv6Prefix[pkt->intf], &pOpt->prefix, pOpt->prefixLen/8);
            if ((pOpt->prefixLen%8) != 0)
            {
                i = pOpt->prefixLen%8;
                i = 1<<(8 - i);
                i = 256 - i;
                i = *(&pOpt->prefix + pOpt->prefixLen/8) & i;
                dpconf->IPv6Prefix[pOpt->prefixLen/8] = i;
                //dpconf->IPv6Prefix[pOpt->prefixLen/8] = *(&pOpt->prefix + pOpt->prefixLen/8);
            }
            dpconf->IPv6PrefixLen[pkt->intf] = pOpt->prefixLen;
        }

        pkt->len = pkt->len - ICMPOptHdr->len * 8;
        pkt->wp += ICMPOptHdr->len * 8;
    }

    if (IPInfo.TeredoIPChg)
    {
        generateTeredoIPaddr();
        IPInfo[pkt->intf].TeredoIPChg = 0;
    }

    return 0;
}

void icmpv6EchoRequest(unsigned char *addr, int* nonce)
{
    TCPPCB	*newPCB;
    ICMPHdr *outHdr;
    PKT *pkt;
    IPv6PseudoHdr* ipv6PseudoHdr;
    int pktLen = sizeof(ICMPHdr) + TEREDO_NONCE_LEN;

    newPCB = (TCPPCB*) malloc(sizeof(TCPPCB));
    memset(newPCB, 0, sizeof(TCPPCB));
    newPCB->protocol = IP_PROTO_ICMPv6;

    memcpy(newPCB->destIPv6Addr, addr, IPv6_ADR_LEN);

    pkt = allocPkt(pktLen);
    pkt->ipVer = IPv6;

    //ICMP data
    memcpy(pkt->wp + sizeof(ICMPHdr), nonce, TEREDO_NONCE_LEN);
    outHdr = (ICMPHdr*)(pkt->wp);
    outHdr->type = ECHO_REQv6;
    outHdr->code = 0;
    outHdr->id = 0;
    outHdr->seqNum = 0;

    outHdr->chksum = 0;
    ipv6PseudoHdr = (IPv6PseudoHdr*)(pkt->wp - sizeof(IPv6PseudoHdr));
    memcpy(ipv6PseudoHdr->src, getIPv6Address(), IPv6_ADR_LEN);
    memcpy(ipv6PseudoHdr->dest, newPCB->destIPv6Addr, IPv6_ADR_LEN);
    ipv6PseudoHdr->nextHdr = IP_PROTO_ICMPv6;
    ipv6PseudoHdr->unUsed = 0;
    ipv6PseudoHdr->payLoadLen = htons((unsigned short)pktLen);
    outHdr->chksum = inet_chksum(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr) + pktLen);

    addIPHeader_F(newPCB, (PKT*)pkt);
}
#endif

#if (CONFIG_VERSION  !=  IC_VERSION_DP_RevF)
#ifdef CONFIG_PROFILING
extern INT32U mgtcycles[16];
#endif

/*----------------------------Functions-------------------------------------*/

void icmpv6Solicitation(unsigned char *p, int len, unsigned char intf)
{
    TCPPCB	*newPCB;
    IPv6Hdr	*ipv6Hdr;
    ICMPv6Hdr* outHdr;
    NeighborMsg *msg;
    LinkLayerAddrOpt* llOpt;
    unsigned char *outPkt;
    IPv6PseudoHdr* ipv6PseudoHdr;
    PKT* pkt;
    int pktLen = 0;

    DEBUGMSG(ICMP_DEBUG,"icmpv6Solicitation ==>");

    ipv6Hdr = (IPv6Hdr*) (p - IPv6_HLEN);

    if (len < sizeof(ICMPv6Hdr) + 8)
    {
        DEBUGMSG(ICMP_DEBUG,"Length too short");
        return;
    }

    msg = (NeighborMsg *) (p + sizeof(ICMPv6Hdr));

    if (memcmp(msg->targetIP, getIPv6Address_F(intf), IPv6_ADR_LEN) != 0)
    {
        DEBUGMSG(ICMP_DEBUG,"IP not match");
        return;
    }

    llOpt = (LinkLayerAddrOpt*)(p + sizeof(ICMPv6Hdr) + sizeof(NeighborMsg));
    if (llOpt->type != SRC_LINK_ADDR)
    {
        DEBUGMSG(ICMP_DEBUG,"Opcode not match");
        return;
    }

    newPCB = (TCPPCB*) malloc(sizeof(TCPPCB));
    memset(newPCB, 0, sizeof(TCPPCB));
    newPCB->protocol = IP_PROTO_ICMPv6;
	newPCB->intf = intf;

    memcpy(newPCB->dest_MAR, llOpt->macAddr, MAC_ADDR_LEN);

    memcpy(newPCB->destIPv6Addr, ipv6Hdr->src, IPv6_ADR_LEN);
    pktLen = sizeof(ICMPv6Hdr) + sizeof(NeighborMsg) + sizeof(LinkLayerAddrOpt);


    outPkt = allocIPPkt_F(pktLen, IPv6);
    pkt =(PKT*) outPkt;

    outHdr = (ICMPv6Hdr*)(pkt->wp);
    outHdr->type = (unsigned char)ADVERTISEMENT;
    outHdr->code = 0;
    outHdr->chksum = 0;

    msg = (NeighborMsg*)(pkt->wp + sizeof(ICMPv6Hdr));

    msg->flag = 0x60;//Solicited flag and override flag enable
    memset(msg->Res, 0 ,3);
    memcpy(msg->targetIP, getIPv6Address_F(intf), IPv6_ADR_LEN);

    llOpt = (LinkLayerAddrOpt*)(pkt->wp + sizeof(ICMPv6Hdr) + sizeof(NeighborMsg));

    llOpt->type = 0x02;
    llOpt->opLen = 0x01;

    ///memcpy(llOpt->macAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
    GetMacAddr_F(llOpt->macAddr, intf);

    ipv6PseudoHdr = (IPv6PseudoHdr*)(pkt->wp - sizeof(IPv6PseudoHdr));
    memcpy(ipv6PseudoHdr->src, getIPv6Address_F(intf), IPv6_ADR_LEN);
    memcpy(ipv6PseudoHdr->dest, newPCB->destIPv6Addr, IPv6_ADR_LEN);
    ipv6PseudoHdr->nextHdr = IP_PROTO_ICMPv6;
    ipv6PseudoHdr->unUsed = 0;
    ipv6PseudoHdr->payLoadLen = htons(pktLen);//sizeof(ICMPv6Hdr) + sizeof(NeighborMsg));
//Rtk do not support icmpv6 checksum offload
//#ifndef CKHSUM_OFFLOAD
    outHdr->chksum = inet_chksum(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr) + pktLen);//sizeof(IPv6PseudoHdr) + sizeof(ICMPv6Hdr) + sizeof(NeighborMsg));
//#endif
    addIPHeader_F(newPCB, (PKT *) outPkt);
    DEBUGMSG(ICMP_DEBUG,"icmpv6Solicitation <==");
}

void getLinkLocalEUIAddr(unsigned char* addr, unsigned char intf)
{
    const int M0 = 0;
    const int M1 = 1;

    const int M8 = 8;
    //const int M9 = 9;
    //const int M10 = 10;

    const int M11 = 11;
    const int M12 = 12;

    //const int M13 = 13;
    //const int M14 = 14;
    //const int M15 = 15;

    const int P = IPv6_ADR_LEN - MAC_ADDR_LEN;

    ///memcpy(addr + P, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
    GetMacAddr_F(addr + P, intf);
    addr[M0] = 0xFE;
    addr[M1] = 0x80;
    memcpy(&addr[M8], dpconf->srcMacAddr[intf], 3);
    addr[M11] = 0xFF;
    addr[M12] = 0xFE;
    addr[M8] |= 0x02;
}

void chkLinkLocalEUIAddr(unsigned char *p, int len, unsigned char intf)
{
    unsigned char linkLocal[IPv6_ADR_LEN] = {0};
    NeighborMsg *msg;

    getLinkLocalEUIAddr(linkLocal,eth0);

    msg = (NeighborMsg *) (p + sizeof(ICMPv6Hdr));

    if (memcmp(msg->targetIP, linkLocal, IPv6_ADR_LEN) == 0)
        DHCPv6Conf[intf].DHCPv6NeighborAdver = 1;
}


void icmpv6Advertisement(unsigned char *p, int len, unsigned char intf)
{
    NeighborMsg *msg;
    LinkLayerAddrOpt* llopt;
    int i = 0;

    msg = (NeighborMsg *) (p + sizeof(ICMPv6Hdr));
    llopt = (LinkLayerAddrOpt*) (p + sizeof(ICMPv6Hdr) + sizeof(NeighborMsg));

    for ( ;i < ARPEntryNum; i++ )
    {
        if (memcmp(Arpv6Table[i].ipAddr, msg->targetIP, IPv6_ADR_LEN) == 0)
        {
            memcpy(Arpv6Table[i].ethAddr, llopt->macAddr, MAC_ADDR_LEN);
            Arpv6Table[i].state = RESOLVED;
            DEBUGMSG(ARP_DEBUG,"Update a table entry");
            break;
        }
    }

    if (i == ARPEntryNum)
    {
        memcpy(Arpv6Table[Arpv6Pos].ethAddr, llopt->macAddr, MAC_ADDR_LEN);
        memcpy(Arpv6Table[Arpv6Pos].ipAddr, msg->targetIP, IPv6_ADR_LEN);
        Arpv6Table[Arpv6Pos].state = RESOLVED;
        Arpv6Pos++;
        Arpv6Pos = Arpv6Pos%ARPEntryNum;
        DEBUGMSG(ARP_DEBUG,"Add a table entry");
    }
    OSSemPost(ArpTableSem);
}

//For DHCPv6 use
void neighborSolicite(unsigned char* ip, unsigned char intf)
{
    TCPPCB	*newPCB;
    ICMPv6Hdr* outHdr;
    NeighborMsg* msg;
    LinkLayerAddrOpt* llopt;
    unsigned char *outPkt;
    IPv6PseudoHdr* ipv6PseudoHdr;
    PKT* pkt;
    int pktLen = 0;
    unsigned char linkLocal[IPv6_ADR_LEN] = {0};

    newPCB = (TCPPCB*) malloc(sizeof(TCPPCB));
    memset(newPCB, 0, sizeof(TCPPCB));
    newPCB->protocol = IP_PROTO_ICMPv6;

    //When IP address do not specify,
    //do no include option
    if (ip == 0)
        pktLen = sizeof(ICMPv6Hdr) + sizeof(NeighborMsg);
    else
        pktLen = sizeof(ICMPv6Hdr) + sizeof(NeighborMsg) + sizeof(LinkLayerAddrOpt);

    outPkt = allocIPPkt_F(pktLen, IPv6);
    pkt =(PKT*) outPkt;

    outHdr = (ICMPv6Hdr*)(pkt->wp);
    outHdr->type = (unsigned char)SOLICITATION;
    outHdr->code = 0;
    outHdr->chksum = 0;

    msg = (NeighborMsg*)(pkt->wp + sizeof(ICMPv6Hdr));
    msg->flag = 0;
    //Duplicate Address Detection
    //Generate EUI-64 Address

    if (ip == 0)
    {
        getLinkLocalEUIAddr(linkLocal, eth0);
        memcpy(msg->targetIP, linkLocal, IPv6_ADR_LEN);

        /*Solicited-node address
        	-Facilitates the efficient query of network node during address
        	 resolution (ICMPv6 Neighbor Discovery)
        	-Prefix= FF02::1:FF00:0000/104 and the last 24-bits of IPv6
        	 address*/

        linkLocal[0] = 0xFF;
        linkLocal[1] = 0x02;

        linkLocal[8] = 0x00;
        linkLocal[9] = 0x00;
        linkLocal[10] = 0x00;
        linkLocal[11] = 0x01;

        linkLocal[12] = 0xFF;

        memcpy(newPCB->destIPv6Addr, linkLocal , IPv6_ADR_LEN);

        //Solicate Multicast address
        memcpy(newPCB->dest_MAR + 2, &linkLocal[12], MAC_ADDR_LEN - 2);
        newPCB->dest_MAR[0] =0x33;
        newPCB->dest_MAR[1] = 0x33;
    }
    else
    {
        memcpy(msg->targetIP, ip, IPv6_ADR_LEN);
        ip[0] = 0xff;
        ip[1] = 0x02;
        //Solicited-Node Address:  FF02:0:0:0:0:1:FFXX:XXXX
        //Mac: 33-33-FF-XX-XX-XX
        memset(ip + 2, 0, 9);
        ip[11] = 0x01;
        ip[12] = 0xff;

        memcpy(newPCB->destIPv6Addr, ip, IPv6_ADR_LEN);
        memcpy(newPCB->dest_MAR + 3, &ip[13], MAC_ADDR_LEN - 3);
        newPCB->dest_MAR[0] =0x33;
        newPCB->dest_MAR[1] = 0x33;
        newPCB->dest_MAR[2] = 0xff;

        llopt = (LinkLayerAddrOpt*) (pkt->wp + sizeof(ICMPv6Hdr) + sizeof(NeighborMsg));
        llopt->type = SRC_LINK_ADDR;
        //units of 8 ocects
        llopt->opLen = 1;
        ///memcpy(llopt->macAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
        GetMacAddr_F(llopt->macAddr, intf);
    }

    ipv6PseudoHdr = (IPv6PseudoHdr*)(pkt->wp - sizeof(IPv6PseudoHdr));
    memcpy(ipv6PseudoHdr->src, getIPv6Address_F(intf), IPv6_ADR_LEN);
    memcpy(ipv6PseudoHdr->dest, newPCB->destIPv6Addr, IPv6_ADR_LEN);
    ipv6PseudoHdr->nextHdr = IP_PROTO_ICMPv6;
    ipv6PseudoHdr->unUsed = 0;
    ipv6PseudoHdr->payLoadLen = htons((unsigned short)pktLen);

//Rtk do not support icmpv6 checksum offload
//#ifndef CKHSUM_OFFLOAD
    outHdr->chksum = inet_chksum(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr) + pktLen);
//#endif
    addIPHeader_F(newPCB, (PKT *) outPkt);
}

void routerSolicite()
{
    TCPPCB	*newPCB;
    ICMPv6Hdr* outHdr;
    RouterSolMsg *msg;
    unsigned char *outPkt;
    IPv6PseudoHdr* ipv6PseudoHdr;
    PKT* pkt;
    unsigned char linkLocal[IPv6_ADR_LEN] = {0};
    int pktLen = 0;

    memset(dpconf->IPv6Prefix, 0, IPv6_ADR_LEN);
    dpconf->IPv6PrefixLen[eth0]= 0;
    newPCB = (TCPPCB*) malloc(sizeof(TCPPCB));
    memset(newPCB, 0, sizeof(TCPPCB));
    newPCB->protocol = IP_PROTO_ICMPv6;

    pktLen = sizeof(ICMPv6Hdr) + sizeof(RouterSolMsg);
    outPkt = allocIPPkt_F(pktLen, IPv6);
    pkt =(PKT*) outPkt;

    outHdr = (ICMPv6Hdr*)(pkt->wp);
    outHdr->type = (unsigned char)ROUTERSOLICITATION;
    outHdr->code = 0;
    outHdr->chksum = 0;

    msg = (RouterSolMsg*)(pkt->wp + sizeof(ICMPv6Hdr));
    msg->flag = 0;
    msg->opcode = 1;
    msg->opLen = 1;

    ///memcpy(msg->macAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
    GetMacAddr_F(msg->macAddr, eth0);

    linkLocal[0] = 0xFF;
    linkLocal[1] = 0x02;
    linkLocal[IPv6_ADR_LEN - 1] = 0x02;
    memcpy(newPCB->destIPv6Addr, linkLocal , IPv6_ADR_LEN);

    getLinkLocalEUIAddr(linkLocal, eth0);
    setLinkLocalAddress_F(linkLocal, eth0);

    //memcpy(newPCB->dest_MAR + 2, &linkLocal[12], MAC_ADDR_LEN - 2);
    newPCB->dest_MAR[0] =0x33;
    newPCB->dest_MAR[1] = 0x33;
    newPCB->dest_MAR[2] = 0x0;
    newPCB->dest_MAR[3] = 0x0;
    newPCB->dest_MAR[4] = 0x0;
    newPCB->dest_MAR[5] = 0x02;


    ipv6PseudoHdr = (IPv6PseudoHdr*)(pkt->wp - sizeof(IPv6PseudoHdr));
    memcpy(ipv6PseudoHdr->src, getIPv6Address_F(eth0), IPv6_ADR_LEN);

    memcpy(ipv6PseudoHdr->dest, newPCB->destIPv6Addr, IPv6_ADR_LEN);
    ipv6PseudoHdr->nextHdr = IP_PROTO_ICMPv6;
    ipv6PseudoHdr->unUsed = 0;
    ipv6PseudoHdr->payLoadLen = htons((unsigned short)pktLen);

//Rtk do not support icmpv6 checksum offload
//#ifndef CKHSUM_OFFLOAD
    outHdr->chksum = inet_chksum(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr) + pktLen);
//#endif
    addIPHeader_F(newPCB, (PKT *) outPkt);
}
#endif
#endif
