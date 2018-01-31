/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	ip.c

Abstract:	IP module
			IP version -> header length -> checksum(Hw) -> destination address
			-> pass to protocol

*****************************************************************************
*/

/*--------------------------Include File------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include "lib.h"
#include "ip.h"
#include "icmp.h"
#include "tcp.h"
#include "udp.h"
#if CONFIG_TEREDO_ENABLED
#include "teredo.h"
#endif
#include "debug.h"
#include "bsp.h"

extern DPCONF *dpconf;
ROM_EXTERN _IPInfo IPInfo[MAX_INTF];
#if CONFIG_TEREDO_ENABLED
extern UDPAddrInfo TeredoAddrInfo;
extern unsigned short TmpOriginPort;
extern unsigned int TmpOriginIP;
extern TeredoEntry TeredoPeerAddr[TEREDO_ENTRY_NUM];
extern RTSkt* TeredoSkt;
extern void sendTeredoBubblePkt(RTSkt* TeredoSkt, unsigned char* ip, unsigned short originPort, unsigned int originIP);
extern void teredoTxPendEnqueue(PKT* pkt);
extern void teredoTxEnqueue(PKT* pkt, UDPAddrInfo *addr);
#endif

#if !defined(CONFIG_BUILDROM) || CONFIG_ROM_VERIFY
void ipInput(PKT *pkt) {
    IPHdr *iphdr;
    IPv6Hdr *ipv6hdr = NULL;
    unsigned char protocol;
    unsigned char hl;
    DEBUGMSG(IP_DEBUG,"ipInput ==>");

    /* identify the IP header */
    iphdr = (IPHdr *) pkt->wp;
    //DEBUGMSG(IP_DEBUG, "%d %d %d %d",iphdr->src.addr[0]);

    if (iphdr->ver == IPv4)
    {
        if (!dpconf->IPv4Enable)
        {
            DEBUGMSG(IP_DEBUG,"IPv4 protocol disable");
            return;
        }

        pkt->ipVer = IPv4;
        pkt->wp = pkt->wp + IP_HLEN;
        hl = iphdr->hl;

        if (hl * 4 > IP_HLEN) {
            DEBUGMSG(IP_DEBUG, "IP packet dropped since there were IP options.");
            return;
        }

        /* verify checksum */
        /*Pass verify checksum*/
#if 0
        if ((chksum = inet_chksum(iphdr, hl * 4)) != 0) {
            DEBUGF(IP_DEBUG, ("IP packet dropped due to failing checksum 0x%x", chksum));
        }
#endif
        /*Do not check IP address at here for DHCP issue*/

#if 0
        if (!ip_addr_cmp(&(iphdr->dest), &(iphdr->dest))) {
            DEBUGF(IP_DEBUG, ("This IP packet is not for us\n"));
        }
#endif
        protocol = iphdr->proto;
    }
    else if (iphdr->ver == IPv6)
    {
        if (!dpconf->IPv6Enable)
        {
            DEBUGMSG(IP_DEBUG,"IPv6 protocol disable");
            return;
        }

        pkt->ipVer = IPv6;
        ipv6hdr = (IPv6Hdr *)pkt->wp;
        protocol = ipv6hdr->nexthdr;
        pkt->wp = pkt->wp + IPv6_HLEN;
    }
    else
    {
        DEBUGMSG(IP_DEBUG,"Unknow IP version");
        return;
    }

    switch (protocol) {
#ifdef CONFIG_UDP_ENABLED
    case IP_PROTO_UDP:
        DEBUGMSG(IP_DEBUG,"UDP packet");
        udpInput_F(pkt);
        break;
#endif
    case IP_PROTO_TCP:
        DEBUGMSG(IP_DEBUG,"TCP Packet");
        tcpInput_F(pkt);
        break;
    case IP_PROTO_ICMP:
        DEBUGMSG(IP_DEBUG,"ICMPv4 packet");
        if (dpconf->EchoService == off)
        {
            DEBUGMSG(IP_DEBUG,"EchoService off");
            break;
        }
        icmpInput_F(pkt, ntohs(iphdr->len) - IP_HLEN );
        break;
    case IP_PROTO_ICMPv6:
        DEBUGMSG(IP_DEBUG,"ICMPv6 packet");
        icmpv6Input(pkt->wp, ntohs(ipv6hdr->len), pkt->intf);
        break;
    default:
        /* send ICMP destination protocol unreachable */
        //icmp_dest_unreach(p, ICMP_DUR_PROTO);
        DEBUGMSG(IP_DEBUG, "Unsupported transportation protocol %d", iphdr->proto);
    }
    DEBUGMSG(IP_DEBUG,"ipInput <==");
}
#elif defined(CONFIG_BUILDROM)
//Rom version not support ipv6
void ipInput(PKT *pkt) {
    IPHdr *iphdr;
    unsigned char protocol;
    unsigned char hl;
    DEBUGMSG(IP_DEBUG,"ipInput ==>");

    /* identify the IP header */
    iphdr = (IPHdr *) pkt->wp;
    //DEBUGMSG(IP_DEBUG, "%d %d %d %d",iphdr->src.addr[0]);

    if (iphdr->ver == IPv4)
    {
        if (!dpconf->IPv4Enable)
        {
            DEBUGMSG(IP_DEBUG,"IPv4 protocol disable");
            return;
        }

        pkt->ipVer = IPv4;
        pkt->wp = pkt->wp + IP_HLEN;
        hl = iphdr->hl;

        if (hl * 4 > IP_HLEN) {
            DEBUGMSG(IP_DEBUG, "IP packet dropped since there were IP options.");
            return;
        }

        /* verify checksum */
        /*Pass verify checksum*/
#if 0
        if ((chksum = inet_chksum(iphdr, hl * 4)) != 0) {
            DEBUGF(IP_DEBUG, ("IP packet dropped due to failing checksum 0x%x", chksum));
        }
#endif
        /*Do not check IP address at here for DHCP issue*/

#if 0
        if (!ip_addr_cmp(&(iphdr->dest), &(iphdr->dest))) {
            DEBUGF(IP_DEBUG, ("This IP packet is not for us\n"));
        }
#endif
        protocol = iphdr->proto;
    }
    else if (iphdr->ver == IPv6)
    {
    }
    else
    {
        DEBUGMSG(IP_DEBUG,"Unknow IP version");
        return;
    }

    switch (protocol) {
#ifdef CONFIG_UDP_ENABLED
    case IP_PROTO_UDP:
        DEBUGMSG(IP_DEBUG,"UDP packet");
        udpInput_F(pkt);
        break;
#endif
    case IP_PROTO_ICMP:
        DEBUGMSG(IP_DEBUG,"ICMPv4 packet");
        if (dpconf->EchoService == off)
        {
            DEBUGMSG(IP_DEBUG,"EchoService off");
            break;
        }
        icmpInput_F(pkt, ntohs(iphdr->len) - IP_HLEN );
        break;
    default:
        /* send ICMP destination protocol unreachable */
        //icmp_dest_unreach(p, ICMP_DUR_PROTO);
        DEBUGMSG(IP_DEBUG, "Unsupported transportation protocol %d", iphdr->proto);
    }
    DEBUGMSG(IP_DEBUG,"ipInput <==");
}
#endif

#if CONFIG_TEREDO_ENABLED
void addTeredoAuthHdr(PKT* pkt)
{
    //Current Client id and authentication all 0, the same as win7
    TeredoAuthHdr *hdr;
    int i = 0;

    pkt->wp = pkt->wp - sizeof(TeredoAuthHdr);
    pkt->len = pkt->len + sizeof(TeredoAuthHdr);
    hdr = (TeredoAuthHdr *)pkt->wp;
    memset(hdr, 0, sizeof(TeredoAuthHdr));
    hdr->IndicatorTypeH = TEREDO_AUTH_TYPE;//Indicator Type for TeredoAuth is 1

    for (i = 0; i < TEREDO_NONCE_LEN; i++)
    {
        hdr->nonce[i] = rand();
    }
    memcpy(IPInfo[pkt->intf].TeredoNonce, hdr->nonce, TEREDO_NONCE_LEN);
}

int chkTeredoAuthHdr(PKT* pkt)
{
    //Current Client id and authentication all 0, the same as win7
    TeredoAuthHdr *hdr;

    if (pkt->len < sizeof(TeredoAuthHdr))
    {
        return TEREDO_FAIL;
    }
    else
    {
        pkt->len = pkt->len - sizeof(TeredoAuthHdr);
    }

    hdr = (TeredoAuthHdr *)pkt->wp;

    if ((hdr->IndicatorTypeL !=0) || (hdr->IndicatorTypeH != TEREDO_AUTH_TYPE) ||
            (hdr->CILen!=0) || (hdr->AuthDataLen != 0) || (hdr->Confirm !=0))
    {
        return TEREDO_FAIL;
    }

    if (memcmp(IPInfo[pkt->intf].TeredoNonce, hdr->nonce, TEREDO_NONCE_LEN) != 0)
    {
        return TEREDO_FAIL;
    }

    pkt->wp = pkt->wp + sizeof(TeredoAuthHdr);

    return 0;
}

int sendTeredoPkt(TCPPCB *pcb, PKT* pkt)
{
    TCPHdr	*tcphdr;
    unsigned char ret = 0;

    UDPAddrInfo* tmpTeredoAddr = 0;
    int peerIdx = 0;
    PKT* tmpPkt = 0;

    if (pkt->ipVer == IPv4)
        tcphdr = (TCPHdr*)(pkt->wp + ETH_HDR_SIZE + IP_HLEN);
    else
        tcphdr = (TCPHdr*)(pkt->wp + ETH_HDR_SIZE + IPv6_HLEN);

    //If retransmission is greater than MAXRTTIMES, then reset the connection
    if (pkt->rTime >= MAXRTTIMES )
    {
        tcphdr->flags |= TCP_RST;
        ret = 1;
    }

    tmpPkt = (PKT*)malloc(PKT_SIZE);
    memcpy(tmpPkt, pkt, PKT_SIZE);
    tmpPkt->start =(unsigned char*) malloc(pkt->wp - pkt->start + IPv6_HLEN + TCP_HDR_SIZE + pkt->len);
    tmpPkt->wp = tmpPkt->start + (pkt->wp - pkt->start) ;
    memcpy(tmpPkt->wp, pkt->wp, IPv6_HLEN + TCP_HDR_SIZE + pkt->len);
    tmpPkt->len = tmpPkt->len + IPv6_HLEN + TCP_HDR_SIZE;

    if ((peerIdx = chkTeredoPeerList(pcb->destIPv6Addr)) == TEREDO_NO_ENTRY)
    {
        if (tmpPkt)
        {
            teredoTxPendEnqueue(tmpPkt);
        }
        else
        {
            teredoTxPendEnqueue(pkt);
        }
        doTeredoConnTest(pcb->destIPv6Addr);
    }
    else if (peerIdx == TEREDO_NO_TRUST_ENTRY)
    {
        if (tmpPkt)
        {
            teredoTxPendEnqueue(tmpPkt);
        }
        else
        {
            teredoTxPendEnqueue(pkt);
        }
    }
    else
    {
        tmpTeredoAddr = (UDPAddrInfo*)malloc(sizeof(UDPAddrInfo));
        memcpy(&tmpTeredoAddr->destIPAddr.addr, TeredoPeerAddr[peerIdx].MappedIP, IPv4_ADR_LEN);
        memcpy(&tmpTeredoAddr->destPort ,TeredoPeerAddr[peerIdx].MappedPort, sizeof(tmpTeredoAddr->destPort));
        tmpPkt->ipVer = IPv4;
        teredoTxEnqueue(tmpPkt, tmpTeredoAddr);
    }

    return ret;
}
#endif

#if defined(CONFIG_TEREDO_ENABLED) || CONFIG_VERSION < IC_VERSION_DP_RevF || CONFIG_BUILDROM
void addIPHeader(TCPPCB *pcb, PKT* pkt)
{
    IPHdr* iphdr;
    IPv6Hdr* ipv6hdr;
#if CONFIG_TEREDO_ENABLED
    ICMPv6Hdr* icmpv6Hdr;
    UDPAddrInfo* tmpTeredoAddr = 0;
    int peerIdx = 0;
    PKT* tmpPkt = 0;
    unsigned int tmpIP;
    unsigned short tmpPort;
#endif

    DEBUGMSG(IP_DEBUG,"addIPHeader ==>");

    if (pkt->ipVer == IPv4)
    {
        pkt->wp = pkt->wp - IP_HLEN;
    }
    else if (pkt->ipVer == IPv6)
    {
        pkt->wp = pkt->wp - IPv6_HLEN;
    }

    if (pkt->ipVer == IPv4)
    {
        iphdr = (IPHdr*)(pkt->wp);
        //Make IP header
        iphdr->ver = IPv4;
        iphdr->hl = 5;
        iphdr->tos = 0;

        if (pcb->protocol == IP_PROTO_TCP)
        {
            iphdr->len = htons((unsigned short)(IP_HLEN + TCP_HDR_SIZE + pkt->len));
            iphdr->proto = IP_PROTO_TCP;
        }
        else if (pcb->protocol == IP_PROTO_ICMP)
        {
            iphdr->len = htons((unsigned short)(IP_HLEN + pkt->len));
            iphdr->proto = IP_PROTO_ICMP;
        }
        else if (pcb->protocol == IP_PROTO_UDP)
        {
            iphdr->len = htons((unsigned short)(IP_HLEN + pkt->len));
            iphdr->proto = IP_PROTO_UDP;
        }

        //Do not support fragement
        iphdr->offset = htons(IP_DF);
        iphdr->id = htons(pcb->ipID++);
        iphdr->ttl = IP_DEFAULT_TTL;
        iphdr->src.addr = getIPAddress_F(pkt->intf);
        iphdr->dest.addr = pcb->destIPAddr.addr;
        iphdr->chksum = 0;
#ifndef CKHSUM_OFFLOAD
        iphdr->chksum = inet_chksum(iphdr, IP_HLEN);
#endif
    }

    if (pkt->ipVer == IPv6)
    {
        ipv6hdr = (IPv6Hdr*)(pkt->wp);
        ipv6hdr->ver = IPv6;
        ipv6hdr->pri = 0;
        ipv6hdr->flow1 = 0;
        ipv6hdr->flow2 = 0;

        if (pcb->protocol == IP_PROTO_TCP)
        {
            ipv6hdr->len = htons((unsigned short)(TCP_HDR_SIZE + pkt->len));
            ipv6hdr->nexthdr = IP_PROTO_TCP;
        }
        else if (pcb->protocol == IP_PROTO_ICMPv6)
        {
            ipv6hdr->len = htons((unsigned short)(pkt->len));
            ipv6hdr->nexthdr = IP_PROTO_ICMPv6;
        }
        else if (pcb->protocol == IP_PROTO_UDP)
        {
            ipv6hdr->len = htons((unsigned short)(pkt->len));
            ipv6hdr->nexthdr = IP_PROTO_UDP;

        }
#if CONFIG_TEREDO_ENABLED
        else if (pcb->protocol == IP_PROTO_NONHDR)
        {
            ipv6hdr->len = htons((unsigned short)(pkt->len));
            ipv6hdr->nexthdr = IP_PROTO_NONHDR;
        }
#endif
        ipv6hdr->hoplim = IP_DEFAULT_TTL;
        memcpy(ipv6hdr->dest, pcb->destIPv6Addr, IPv6_ADR_LEN);
        memcpy(ipv6hdr->src, getIPv6Address_F(pcb->intf), IPv6_ADR_LEN);

#if CONFIG_TEREDO_ENABLED
        if (dpconf->teredoEnable || dpconf->useTeredo)
        {
            pkt->len = pkt->len + IPv6_HLEN;
            if (pcb->protocol == IP_PROTO_ICMPv6)
            {
                icmpv6Hdr = (ICMPv6Hdr*)(pkt->wp + IPv6_HLEN);
                if (icmpv6Hdr->type == ROUTERSOLICITATION)
                {
                    addTeredoAuthHdr(pkt);
                    tmpTeredoAddr = (UDPAddrInfo*)malloc(sizeof(UDPAddrInfo));
                    memcpy(tmpTeredoAddr, &TeredoAddrInfo, sizeof(UDPAddrInfo));
                }
                else if (icmpv6Hdr->type == ECHO_REQv6)
                {
                    //Request alway send to Teredo Server
                    tmpTeredoAddr = (UDPAddrInfo*)malloc(sizeof(UDPAddrInfo));
                    memcpy(tmpTeredoAddr, &TeredoAddrInfo, sizeof(UDPAddrInfo));
                }
                else if ((peerIdx = chkTeredoPeerList(pcb->destIPv6Addr)) == TEREDO_NO_ENTRY)
                {
                    //echo reply or .....
                    teredoTxPendEnqueue(pkt);

                    if ((pcb->destIPv6Addr[0] == 0x20) && (pcb->destIPv6Addr[1] == 0x01))
                    {
                        createTeredoAddrEntry(pcb->destIPv6Addr, 0);
                        memcpy(&tmpIP, pcb->destIPv6Addr + 12, IPv4_ADR_LEN);
                        memcpy(&tmpPort, pcb->destIPv6Addr + 10, 2);
                        sendTeredoBubblePkt(TeredoSkt, pcb->destIPv6Addr, tmpPort, tmpIP);

                        memcpy(&tmpIP, pcb->destIPv6Addr + 4, IPv4_ADR_LEN);
                        tmpIP = tmpIP ^ 0xffffffff;
                        tmpPort = htons(3544);
                        tmpPort = tmpPort ^ 0xffff;
                        sendTeredoBubblePkt(TeredoSkt, pcb->destIPv6Addr, tmpPort, tmpIP);
                    }
                    else
                    {
                        doTeredoConnTest(pcb->destIPv6Addr);
                    }
                    return;
                }
                else if (peerIdx == TEREDO_NO_TRUST_ENTRY)
                {
                    teredoTxPendEnqueue(pkt);
                    return;
                }
                else
                {
                    tmpTeredoAddr = (UDPAddrInfo*)malloc(sizeof(UDPAddrInfo));
                    memcpy(&tmpTeredoAddr->destIPAddr.addr, TeredoPeerAddr[peerIdx].MappedIP, IPv4_ADR_LEN);
                    memcpy(&tmpTeredoAddr->destPort ,TeredoPeerAddr[peerIdx].MappedPort, sizeof(tmpTeredoAddr->destPort));
                }
            }
            else if (pcb->protocol == IP_PROTO_NONHDR)
            {
                //bubble packet
                tmpTeredoAddr = (UDPAddrInfo*)malloc(sizeof(UDPAddrInfo));
                memcpy(tmpTeredoAddr, &TeredoAddrInfo, sizeof(UDPAddrInfo));
                memcpy(&(tmpTeredoAddr->destIPAddr), &TmpOriginIP, IPv4_ADR_LEN);
                tmpTeredoAddr->destPort = TmpOriginPort;
            }
            else
            {
                //tcp or udp
                if (pcb->protocol == IP_PROTO_TCP)
                {
                    tmpPkt = (PKT*)malloc(PKT_SIZE);
                    memcpy(tmpPkt, pkt, PKT_SIZE);
                    tmpPkt->start =(unsigned char*) malloc(pkt->wp - pkt->start + IPv6_HLEN + TCP_HDR_SIZE + pkt->len);
                    tmpPkt->wp = tmpPkt->start + (pkt->wp - pkt->start) ;
                    memcpy(tmpPkt->wp, pkt->wp, IPv6_HLEN + TCP_HDR_SIZE + pkt->len);
                    tmpPkt->len = tmpPkt->len + IPv6_HLEN + TCP_HDR_SIZE;
                    pkt->len = pkt->len - IPv6_HLEN;

                    //free only ack packet
                    if (!pkt->queued)
                    {
                        freePkt(pkt);
                    }
                }
                else if (pcb->protocol == IP_PROTO_UDP)
                {
                    pkt->len = pkt->len + IPv6_HLEN + UDP_HDR_SIZE;
                }

                if ((peerIdx = chkTeredoPeerList(pcb->destIPv6Addr)) == TEREDO_NO_ENTRY)
                {
                    if (tmpPkt)
                    {
                        teredoTxPendEnqueue(tmpPkt);
                    }
                    else
                    {
                        teredoTxPendEnqueue(pkt);
                    }

                    if ((pcb->destIPv6Addr[0] == 0x20) && (pcb->destIPv6Addr[1] == 0x01))
                    {
                        createTeredoAddrEntry(pcb->destIPv6Addr, 0);
                        memcpy(&tmpIP, pcb->destIPv6Addr + 12, IPv4_ADR_LEN);
                        memcpy(&tmpPort, pcb->destIPv6Addr + 10, 2);
                        sendTeredoBubblePkt(TeredoSkt, pcb->destIPv6Addr, tmpPort, tmpIP);

                        memcpy(&tmpIP, pcb->destIPv6Addr + 4, IPv4_ADR_LEN);
                        tmpIP = tmpIP ^ 0xffffffff;
                        tmpPort = htons(3544);
                        tmpPort = tmpPort ^ 0xffff;
                        sendTeredoBubblePkt(TeredoSkt, pcb->destIPv6Addr, tmpPort, tmpIP);
                    }
                    else
                    {
                        doTeredoConnTest(pcb->destIPv6Addr);
                    }
                    return;
                }
                else if (peerIdx == TEREDO_NO_TRUST_ENTRY)
                {
                    if (tmpPkt)
                    {
                        teredoTxPendEnqueue(tmpPkt);
                    }
                    else
                    {
                        teredoTxPendEnqueue(pkt);
                    }
                    return;
                }
                else
                {
                    tmpTeredoAddr = (UDPAddrInfo*)malloc(sizeof(UDPAddrInfo));
                    memcpy(&tmpTeredoAddr->destIPAddr.addr, TeredoPeerAddr[peerIdx].MappedIP, IPv4_ADR_LEN);
                    memcpy(&tmpTeredoAddr->destPort ,TeredoPeerAddr[peerIdx].MappedPort, sizeof(tmpTeredoAddr->destPort));
                }
            }


            if (tmpPkt)
            {
                tmpPkt->ipVer = IPv4;
                teredoTxEnqueue(tmpPkt, tmpTeredoAddr);
            }
            else
            {
                pkt->ipVer = IPv4;
                teredoTxEnqueue(pkt, tmpTeredoAddr);
            }
            return;
        }
#endif
    }
    DEBUGMSG(IP_DEBUG,"addIPHeader <==");
    addEthernetHeader_F(pcb, pkt);
}
#endif
