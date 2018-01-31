#include <string.h>
#include <stdlib.h>
#include "rtskt.h"
#include "tcp.h"
#include "arp.h"
#include "dns.h"
#include "bsp.h"
#include "udp.h"
#include "wcom.h"


extern _IPInfo IPInfo[];
extern DPCONF *dpconf;
extern OS_EVENT *SktSem;

#define PORT_START 52379

//RFC 1122 MSL is 120sec and TIME WAIT timeout = 2*MSL
//120*2*10(suppose 1s max connections are 10)
#define DMSL 2400
//Start form 8168
ROM_EXTERN unsigned int ClientPortCount;

#if CONFIG_ROM_VERIFY || !defined(CONFIG_BUILDROM)
unsigned char* getDNSAnsRRPos(int qCount, unsigned char* p)
{
    int i = 0;

    for (i = 0; i < qCount; i++)
    {
        while (p[0])
        {
            if ((p[0]&0xC0) == 0xC0)
            {
                p= p+2;
            }
            p = p + p[0] + 1;
        }
        p = p +1;//End
        p = p + 4;//Query type and Query class
    }
    return p;
}


unsigned char* getIPAddrPos(int ansCount,unsigned char* dnsStart, unsigned char* ansPos, char* qName)
{
    int i = 0;
    unsigned char* tmpDNS = (unsigned char*) malloc(256);
    unsigned char* CNAME = 0; //only support one canonical name
    unsigned char* p = ansPos;
    int offset = 0;
    char compressed = 0;
    unsigned short dType = 0;


    for (i = 0; i < ansCount; i++)
    {
        p = ansPos;
        compressed = 0;
        offset = 0;
        memset(tmpDNS, 0, 256);

        while (p[0])
        {
            if ((p[0]&0xC0) == 0xC0)
            {
                p = dnsStart + p[1];

                if (!compressed)
                {
                    ansPos = ansPos + 2;
                }
                compressed = 1;
                continue;
            }

            memcpy(tmpDNS + offset, p, p[0] + 1);
            offset = offset + p[0] + 1;
            if (!compressed)
            {
                ansPos = ansPos + p[0] + 1;
            }
            p = p + p[0] + 1;
        }

        if(!compressed)
	        ansPos++;

        if ((memcmp(tmpDNS, qName, strlen((const char*)qName)) == 0) || (memcmp(tmpDNS, CNAME, strlen((const char*)tmpDNS)) == 0))
        {
            memcpy(&dType, ansPos, sizeof(dType));
            dType = ntohs(dType);

            if ((dType == QCNAME) && CNAME == 0)
            {
                p = ansPos + 10;
                compressed = 0;
                offset = 0;
                CNAME = (unsigned char*)malloc(256);

                while (p[0])
                {
                    if ((p[0]&0xC0) == 0xC0)
                    {
                        p = dnsStart + p[1];
                        continue;
                    }

                    memcpy(CNAME + offset, p, p[0] + 1);
                    offset = offset + p[0] + 1;
                    p = p + p[0] + 1;
                }
            }
            else if ((dType == QTA) || (dType == QAAAA))
            {
                if (CNAME)
                {
                    free(CNAME);
                }
                free(tmpDNS);
                return ansPos + 10;
            }
        }

        ansPos = ansPos + 8; //Domain type + Domain class + TTL

        memcpy(&offset, ansPos, 2);
        ansPos = ansPos + ntohs(offset);
        ansPos = ansPos + 2;
    }

    if (CNAME)
    {
        free(CNAME);
    }
    free(tmpDNS);

    return 0;
}

unsigned short getPortNumber(TCPPCB *pcb)
{
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
	int timeout =0;		
    do
    {
        if (ClientPortCount >= DMSL)
            ClientPortCount = 0;
        ClientPortCount++;
        pcb->localPort = PORT_START + ClientPortCount;
        timeout++;
        //Prevent infinite loop
        if(timeout > 5)
        break;
    }while (bsp_bits_get(IO_CONFIG, BIT_SINGLE_IP, 1) && bsp_in_band_exist() &&  bsp_IODrvAP_exist() && (reserveIBPort_F(pcb, PORT_START + ClientPortCount) == -1));
    return PORT_START + ClientPortCount;
#else
    if (ClientPortCount >= DMSL)
        ClientPortCount = 0;
    return PORT_START + ClientPortCount++;
#endif
}

void rtSktEnableRMCPPortFilter()
{
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
    IPInfo[eth0].RMCPFilter = 1;
    bsp_bits_set(IO_HWFunCtr, 1, BIT_FILTERRMCP, 1);
#endif
}

void rtSktDisableRMCPPortFilter()
{
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
    IPInfo[eth0].RMCPFilter = 0;
    bsp_bits_set(IO_HWFunCtr, 0, BIT_FILTERRMCP, 1);
#endif
}

RTSkt* rtSktAccept(RTSkt* s)
{
    TCPPCB *tmp = 0;
    RTSkt* cs = 0;
    PKT* pkt = 0;
    char found = 0;
    unsigned char err = 0;

    DEBUGMSG(SKT_DEBUG,"rtSktAccept rxSem is %x==>", s->rxSem);

    do
    {
        DEBUGMSG(SKT_DEBUG,"rtSktAccept pend rxSem is %x", s->rxSem);
        OSSemPend(s->rxSem, 0, &err);
        DEBUGMSG(SKT_DEBUG,"rtSktAccept pend sktSem is %x", SktSem);
        OSSemPend(SktSem, 0, &err);
        //FIFO
        tmp = s->pcb->clientList;
        found = 0;

        while (tmp && (tmp->accepted || tmp->apRx))
        {
            tmp = tmp->clientList;
        }

        if (tmp && tmp->state == ESTABLISHED)
        {
            found = 1;
        }
        else
        {
            tmp = 0;
        }

        if (found)
        {
            if (tmp->TLSEnable && !tmp->TLSDataStage)
            {
                tmp = 0;
            }
            else
            {
                if (tmp->accepted == 1)
                    tmp =0;
                else
                {
                    DEBUGMSG(SKT_DEBUG,"rtSktAccept to new skt");
                    //Create a new socket for client
                    //Before this point, it shares socket with server
                    cs = rtClientSkt_F(tmp);
                    tmp->accepted = 1;
                    //Data have arrived, so post cs->rxSem.
                    pkt = tmp->rxQStart;
                    while (pkt)
                    {
                        OSSemPost(cs->rxSem);
                        pkt = pkt->next;
                    }
                }
            }
        }

        OSSemPost(SktSem);
    }while (!tmp);

    DEBUGMSG(SKT_DEBUG,"rtSktAccept pcb is %x txSem %x,rxem %x<==", cs->pcb, cs->txSem, cs->rxSem);
    return cs;
}

/*
Example
{'p','t','t','.','c','c'} => {0x03,'p','t','t',0x2,'c','c', 0};
*/
int rtGethostbyname(char ipVer, char* host, unsigned char* ip, unsigned char intf)
{
    RTSkt* s = 0;
    PKT* pkt = 0;
    int status = 0, len = 0;
    int i = 0;
    char *p;
    char *pn;
    char *token;
    unsigned char *ansPos;
    DNSHdr txDNSHdr={0};
    DNSHdr *rxDNSHdr = 0;
    char *queryName;
    unsigned short queryType = htons(1);
    unsigned short queryClass = htons(1);
    unsigned char err = 0;
    unsigned char v4Ok = 1;
    unsigned char v6Ok = 1;
    unsigned char v4Found = 0;
    unsigned char v6Found = 0;
    unsigned char count = 0;
    unsigned char stage = 0;


    //Check if DNS sevre is available
    if ((dpconf->DNSIP[intf].addr == 0) || (getDNSMAR_F(intf) == -1) || ((dpconf->IPv4Enable & (1<<intf)) == 0))
        v4Ok = 0;

    for (i = 0; i < IPv6_ADR_LEN; i++)
    {
        if (dpconf->IPv6DNS[i] != 0)
            break;
    }

    //Check if IPv6 DNS sevre is available
    if ( i == IPv6_ADR_LEN || (dpconf->IPv6Enable == 0))
    {
        v6Ok = 0;
    }
    else
    {
#if 0
        for ( i = 0; i < CONNECT_RETRY; i++)
        {
            if (findIPv6EtherAddr_F(dpconf->IPv6DNS, ethAddr))
                break;
            OSTimeDly(SKT_DLY_TIME);
        }
        if ( i == CONNECT_RETRY)
            v6Ok = 0;
#endif
    }

    len = strlen(host);
    //_ptt.cc_ => need 2 '_' for head and tail
    queryName = (char *)malloc(len + 2);
    //_host
    memcpy(queryName + 1, host, len);
    //Tail append 0
    queryName[len + 2 - 1] = 0;

    p = queryName;
    pn = queryName + 1;

    do
    {
        //Replace . to sub string length
        token = strchr(pn,'.');
        if (!token)
        {
            token = strchr(pn, 0x0);
        }
        *p = (unsigned char)(token - pn);
        p = token;
        pn = token + 1;
    }while (*token);

    len = sizeof(DNSHdr) + (strlen(queryName) + 1) + sizeof(queryType) + sizeof(queryClass);

    txDNSHdr.id = (unsigned short)(htons((rand() % 65536)));
    //Bit Field
    txDNSHdr.RD = 1;
    txDNSHdr.questions = htons(1);

    if (v4Ok)
    {
        s = rtSkt_F(IPv4, IP_PROTO_UDP);
    }

    stage = IPv4;

    while (stage)
    {
        //Allocate DNS packet
        if (s)
        {
            if (s->pcb->ipVer == IPv4)
                pkt = (PKT*)allocUDPPkt_F(len, IPv4);
            else if (s->pcb->ipVer == IPv6)
                pkt = (PKT*)allocUDPPkt_F(len, IPv6);

            p = (char*)(pkt->wp);
            memcpy(p, &txDNSHdr, sizeof(DNSHdr));
            p+= sizeof(DNSHdr);
            memcpy(p, queryName, strlen(queryName) + 1);
            p = p + strlen(queryName) + 1;
            if (ipVer == IPv4)
            {
                queryType = htons(1);
            }
            else
            {
                queryType = htons(28);
            }
            memcpy(p, &queryType, sizeof(queryType));
            p+= sizeof(queryType);
            memcpy(p, &queryClass, sizeof(queryClass));


            if (rtSktSetUDPInfo_F(s, (s->pcb->ipVer == IPv4)?(unsigned char*)&dpconf->DNSIP[intf].addr: dpconf->IPv6DNS[intf] , DNS_Srv_PORT, 0) == SKTDISCONNECT)
            {
                //At this time, s->pcb do not be registered
                freePkt_F(pkt);
                break;
            }

            if (rtSktSend_F(s, pkt, len) == SKTDISCONNECT)
            {
                break;
            }

            do
            {
                pkt = rtSktRx_F(s, OS_TICKS_PER_SEC / 2, &status);
                if (status == SKTDISCONNECT)
                {
                    break;
                }

                if (pkt)
                {
                    if (pkt->len > sizeof(DNSHdr))
                    {
                        rxDNSHdr = (DNSHdr*)pkt->wp;
                        if (rxDNSHdr->rCode == 0 && rxDNSHdr->QR && rxDNSHdr->ans > 0)
                        {
                            ansPos = getDNSAnsRRPos(ntohs(rxDNSHdr->questions),pkt->wp + sizeof(DNSHdr));
                            p = (char*)getIPAddrPos(ntohs(rxDNSHdr->ans), pkt->wp, ansPos, queryName);

                            if (p)
                            {
                                //p = p + sizeof(queryName) + 2 + 2;
                                //printf("%d %d %d %d", pkt->wp[pkt->len -4], pkt->wp[pkt->len -3], pkt->wp[pkt->len -2], pkt->wp[pkt->len -1]);
                                if (s->pcb->ipVer == IPv4)
                                {
                                    memcpy(ip, p, IPv4_ADR_LEN);
                                    v4Found = 1;
                                }
                                else
                                {
                                    memcpy(ip, p, IPv6_ADR_LEN);
                                    v6Found = 1;
                                }
                                break;
                            }
                        }
                    }
                    freePkt_F(pkt);
                }
            }while (count++ < 5);
            count = 0;

            if (s)
            {
                rtSktClose_F(s);
                OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
                //UDP do not have txSem
                //OSSemDel(s->txSem, OS_DEL_ALWAYS, &err);
                free(s);
            }
        }

        if (stage == IPv4 && v6Ok && !v4Found)
        {
            s = rtSkt_F(IPv6, IP_PROTO_UDP);
            stage = IPv6;
        }
        else
            break;

    };

    free(queryName);

    if (v4Found)
        return IPv4;

    if (v6Found)
        return IPv6;

    return 0;
}

int rtSktUDPSendTo(RTSkt* s, PKT* pkt, UDPAddrInfo* ai)
{
    int ret = 0;
    unsigned char* eAddr;
    unsigned char err, count = 0;
    unsigned char ethAddr[MAC_ADDR_LEN];

    OSSemPend(SktSem, 0, &err);

    do
    {
        if (s->pcb == 0)
        {
            freePkt_F(pkt);
            ret = SKTDISCONNECT;
            break;
        }
        else
        {
            //Broadcat or not
            if (s->pcb->ipVer == IPv4 && ai->destIPAddr.addr == 0xffffffff)
            {
                memset(s->pcb->dest_MAR, 0xff, MAC_ADDR_LEN);
                memset(&s->pcb->destIPAddr.addr, 0xff, IPv4_ADR_LEN);
            }//Multicast
            else if (s->pcb->ipVer == IPv4 && (ntohl(ai->destIPAddr.addr) >= 0xe0000000 && htonl(ai->destIPAddr.addr) < 0xf0000000))
            {
                s->pcb->dest_MAR[0] = 0x01;
                s->pcb->dest_MAR[1] = 0x00;
                s->pcb->dest_MAR[2] = 0x5e;
                eAddr =  (unsigned char*)(&ai->destIPAddr.addr);
                s->pcb->dest_MAR[3] = eAddr[1];
                s->pcb->dest_MAR[4] = eAddr[2];
                s->pcb->dest_MAR[5] = eAddr[3];
                //s->pcb->dest_MAR[3] = eAddr[2];
                //s->pcb->dest_MAR[4] = eAddr[1];
                //s->pcb->dest_MAR[5] = eAddr[0];
                s->pcb->dest_MAR[3] &= 0x7f;

                memcpy(&s->pcb->destIPAddr.addr, &ai->destIPAddr.addr, IPv4_ADR_LEN);
            }
            else if (s->pcb->ipVer == IPv4)
            {
                if ((ai->destIPAddr.addr & dpconf->SubnetMask[s->pcb->intf].addr) != dpconf->MatchSubnetMaskValue[s->pcb->intf])
                {
                    if (getGateWayMAR_F((unsigned char*)s->pcb->dest_MAR) == -1)
                    {
                        freePkt_F(pkt);
                        break;
                    }
                    else
                    {
                        if (s->pcb == 0)
                        {
                            freePkt_F(pkt);
                            ret = SKTDISCONNECT;
                            break;
                        }
                        memcpy(&s->pcb->destIPAddr.addr, &ai->destIPAddr.addr, IPv4_ADR_LEN);
                    }
                }
                else
                {
                    OSSemPost(SktSem);

                    while (count < CONNECT_RETRY)
                    {
                        OSSemPend(SktSem, 0, &err);

                        if (!s->pcb)
                        {
                            freePkt_F(pkt);
                            OSSemPost(SktSem);
                            return SKTDISCONNECT;
                        }

                        if (findEtherAddr_F((unsigned char*)&ai->destIPAddr, ethAddr))
                        {
                            memcpy(s->pcb->dest_MAR, ethAddr, MAC_ADDR_LEN);
                            memcpy(&s->pcb->destIPAddr.addr, &ai->destIPAddr.addr, IPv4_ADR_LEN);
                            OSSemPost(SktSem);
                            break;
                        }
                        OSSemPost(SktSem);
                        OSTimeDly(SKT_DLY_TIME);
                        count++;
                    }
                    OSSemPend(SktSem, 0, &err);

                    if (count == CONNECT_RETRY)
                    {
                        //AP will not free pkt
                        freePkt_F(pkt);
                        ret = SKTHOSTNOTFOUND;
                        break;
                    }
                }
            }

            if (s->pcb->ipVer == IPv4)
                pkt->ipVer = IPv4;
            else
            {
                pkt->ipVer = IPv6;
                memcpy(s->pcb->destIPv6Addr, ai->destIPv6Addr, IPv6_ADR_LEN);
            }

            s->pcb->destPort = ai->destPort;

#ifdef CONFIG_UDP_ENABLED
            if (s->pcb->protocol == IP_PROTO_UDP)
            {
                udpSend_F(s->pcb, pkt, pkt->len);
            }
#endif
        }
    }while (0);
    OSSemPost(SktSem);
    return ret;
}

void getUDPAddressInfo(PKT* pkt,UDPAddrInfo* addrInfo)
{
    unsigned short *port;
    IPv6Hdr *ipv6Hdr;

    if (pkt->ipVer == IPv4)
    {
        //12 => Source ip offset
        memcpy((unsigned char*)&addrInfo->destIPAddr.addr, (pkt->start + ETH_HDR_SIZE + 12), IPv4_ADR_LEN);
        //12 => Source port offset
        port = (unsigned short*)(pkt->start + ETH_HDR_SIZE + IP_HLEN);
        addrInfo->destPort = ntohs(*port);
    }
    else if (pkt->ipVer == IPv6)
    {

        //12 => Source ip offset
        ipv6Hdr = (IPv6Hdr *)(pkt->wp - UDP_HDR_SIZE - IPv6_HLEN);
        memcpy((unsigned char*)&addrInfo->destIPv6Addr, ipv6Hdr->src, IPv6_ADR_LEN);
        //12 => Source port offset
        port = (unsigned short*)(pkt->wp - UDP_HDR_SIZE);
        addrInfo->destPort = ntohs(*port);
    }
}
#endif

#if defined(CONFIG_TEREDO_ENABLED) || (CONFIG_VERSION < IC_VERSION_DP_RevF) || (CONFIG_VERSION >= IC_VERSION_EP_RevA) || defined(CONFIG_ROM_VERIFY)
int rtSktConnect(RTSkt* s,unsigned char* ipAddr, unsigned short port)
{
    unsigned int* ip;
    unsigned short clientPort = 0;
    unsigned char err = 0;
    unsigned char ethAddr[MAC_ADDR_LEN];
    unsigned char count = 0;
    char ret = 0;
    char ipVer = 0;
#if defined(CONFIG_IPv6_ENABLED)
    char sameDomain = 0;
    char i = 0;
    char j = 0;
#endif

    OSSemPend(SktSem, 0, &err);
    if (s->pcb)
        ipVer = s->pcb->ipVer;
    OSSemPost(SktSem);

    if (ipVer != IPv4 && ipVer != IPv6)
        return -1;

    ip = (unsigned int*) ipAddr;

    if (ipVer == IPv4)
    {
        //Not at same domain
        if ((*ip & dpconf->SubnetMask[s->pcb->intf].addr) != dpconf->MatchSubnetMaskValue[s->pcb->intf])
        {
            if (getGateWayMAR_F(ethAddr) == -1)
                return -1;
        }
        else
        {
            while (count < CONNECT_RETRY)
            {
                if (findEtherAddr_F(ipAddr, ethAddr))
                    break;
                OSTimeDly(SKT_DLY_TIME);
                count++;
            }

            if ( count == CONNECT_RETRY)
            {
                DEBUGMSG(SKT_DEBUG,"Can not find destination MAC address");
                return -1;
            }
        }
    }
#if defined(CONFIG_IPv6_ENABLED)
#if CONFIG_TEREDO_ENABLED
    else if (ipVer == IPv6 && !dpconf->teredoEnable)
#else
    else if (ipVer == IPv6)
#endif
    {
        if (memcmp(dpconf->IPv6Prefix, ipAddr, dpconf->IPv6PrefixLen/8) == 0)
        {
            i = dpconf->IPv6PrefixLen%8;

            if (i)
            {
                i = 1<<(8 - i);
                i = 256 - i;
                j = dpconf->IPv6Prefix[dpconf->IPv6PrefixLen/8] & i;
                i = ipAddr[dpconf->IPv6PrefixLen/8] & i;

                if (i == j)
                    sameDomain = 1;
            }
            else
                sameDomain = 1;
        }

        while (count < CONNECT_RETRY)
        {
            if (sameDomain)
            {
                if (findIPv6EtherAddr_F(ipAddr, ethAddr))
                    break;
            }
            else
            {
                if (findIPv6EtherAddr_F(dpconf->IPv6GateWay, ethAddr))
                    break;
            }
            OSTimeDly(SKT_DLY_TIME);
            count++;
        }

        if ( count == CONNECT_RETRY)
        {
            DEBUGMSG(SKT_DEBUG,"Can not find destination MAC address");
            return -1;
        }

    }
#endif

    OSSemPend(SktSem, 0, &err);

    if (!s->pcb)
    {
        OSSemPost(SktSem);
        return -1;
    }
    memcpy(s->pcb->dest_MAR, ethAddr, MAC_ADDR_LEN);

    if (s->pcb->ipVer == IPv4)
        memcpy(&(s->pcb->destIPAddr), ipAddr, IPv4_ADR_LEN);
    else
        memcpy(s->pcb->destIPv6Addr, ipAddr, IPv6_ADR_LEN);

    s->pcb->destPort = port;
    //First connect
    if (s->pcb->localPort ==0 )
    {
        clientPort = getPortNumber_F(s->pcb);
        s->pcb->localPort = clientPort;
        tcpPCBRegister_F(s->pcb);
    }

    if (s->pcb->protocol == IP_PROTO_TCP)
    {
        tcpSend_F(s->pcb , NULL, TCP_SYN);
    }

    if ( s->pcb->protocol == IP_PROTO_UDP)
    {
        OSSemPost(SktSem);
        return 0;
    }

    OSSemPost(SktSem);

    count = 0;

    while ((count < CONNECT_RETRY) && (ret == 0))
    {
        //Set a time out value to prvent hang
        OSSemPend(s->rxSem, OS_TICKS_PER_SEC, &err);
        OSSemPend(SktSem, 0, &err);

        if (!s->pcb)
        {
            ret = -1;
            count = CONNECT_RETRY;
        }
        else if (s->pcb->state == ESTABLISHED)
        {
            count = CONNECT_RETRY;
        }
        OSSemPost(SktSem);
        count++;
        if (count == CONNECT_RETRY)
            ret = -1;
    }

    DEBUGMSG(SKT_DEBUG,"rtSktConnect<==");
    return ret;
}
#endif

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF) || (CONFIG_VERSION  == IC_VERSION_EP_RevB) 
void rtSktEnableOOBPortFilter(RTSkt *s)
{
    unsigned char err = 0;
//IC_VERSION_DP_RevF filter Broadcast/Multicast packet only can by IP address
#if CONFIG_VERSION == IC_VERSION_DP_RevF
    if (bsp_bits_get(IO_CONFIG, BIT_SINGLE_IP, 1) == 0)
        return;
#endif

    OSSemPend(SktSem, 0, &err);
    if (s->pcb)
        s->pcb->hwOOBPortFilterEnable = 1;
    OSSemPost(SktSem);
}
#endif

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)

RTSkt* rtSkt(char ipVer, char protocol)
{
    RTSkt *s = (RTSkt*)malloc(sizeof(RTSkt));

    s->pcb = tcpPCBNew_F();
    s->pcb->ipVer = ipVer;
    s->pcb->protocol = protocol;
    s->pcb->skt = s;
    s->pcb->txEnable = 1;
    s->pcb->rxEnable = 1;
    s->pcb->specIntf = 0;

    //Create a event semaphore
    s->rxSem = OSSemCreate(0);

    if (protocol == IP_PROTO_UDP)
        s->txSem = 0;
    else
    {
        s->txSem = OSSemCreate(0);
        s->pcb->timeWaitEnable = 1;
    }
    return s;
}

void rtSktAssignIntf(RTSkt* s, char intf)
{
	s->pcb->intf = intf;
	s->pcb->specIntf = 1;
}

int rtSktReuse(RTSkt *s,char ipVer, char protocol)
{
    unsigned char err = 0;

    if (!s)
        return -1;

    OSSemPend(SktSem, 0, &err);
    s->pcb = tcpPCBNew_F();
    s->pcb->ipVer = ipVer;
    s->pcb->protocol = protocol;
    s->pcb->skt = s;
    s->pcb->txEnable = 1;
    s->pcb->rxEnable = 1;
    OSSemPost(SktSem);

    return 0;
}

void rtSktEnableIBPortFilter(RTSkt *s)
{
    unsigned char err = 0;
#if CONFIG_VERSION == IC_VERSION_DP_RevF
    if (bsp_bits_get(IO_CONFIG, BIT_SINGLE_IP, 1) == 0)
        return;
#endif
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    if (bsp_bits_get(MAC_OOBREG, BIT_SINGLE_IP, 1) == 0)
	return;
#endif
	
    OSSemPend(SktSem, 0, &err);
    if (s->pcb)
        s->pcb->hwIBPortFilterEnable = 1;
    OSSemPost(SktSem);
}

void rtSktEnableTimeWait(RTSkt *s)
{
    unsigned char err = 0;

    OSSemPend(SktSem, 0, &err);
    if (s->pcb)
        s->pcb->timeWaitEnable = 1;
    OSSemPost(SktSem);
}

RTSkt* rtClientSkt(TCPPCB *pcb)
{
    RTSkt *s = (RTSkt*)malloc(sizeof(RTSkt));

    s->pcb = pcb;
    s->pcb->skt = s;
    //Create a event semaphore
    s->rxSem = OSSemCreate(0);
    s->txSem = OSSemCreate(0);
    if (s->pcb->protocol == IP_PROTO_TCP)
        s->pcb->timeWaitEnable = 1;
    return s;
}

int rtSktBind(RTSkt* s, unsigned short port)
{
    unsigned char err = 0;
    //Bind service port
    OSSemPend(SktSem, 0, &err);
    s->pcb->localPort = port;
    OSSemPost(SktSem);
    return 0;
}

int rtSktListen(RTSkt* s, int cons)
{
    unsigned char err = 0;

    OSSemPend(SktSem, 0, &err);
    tcpListen_F(s->pcb);
    s->pcb->maxConnections = cons;
    OSSemPost(SktSem);
    return 0;
}



//If run as udp server, use rtSktUDPRegister to bind server port
//If run as udp client, use rtSktSetUDPInfo to set connection info


int rtSktSetUDPInfo(RTSkt* s, unsigned char *ip, unsigned short srvPort, unsigned short cliPort)
{
    int count = 0;
    unsigned char ethAddr[MAC_ADDR_LEN];
    unsigned char err = 0;

    DEBUGMSG(SKT_DEBUG,"rtSktSetUDPInfo==>");

    //Broadcat or not

    if (s->pcb->ipVer == IPv4)
    {
        if ((ip[0] == 0xff)&&(ip[1] == 0xff)&&(ip[2] == 0xff)&&(ip[3] == 0xff))
        {
            memset(ethAddr, 0xff, MAC_ADDR_LEN);
        }
        else
        {
            if ((*((int*)ip) & dpconf->SubnetMask[s->pcb->intf].addr) != dpconf->MatchSubnetMaskValue[s->pcb->intf])
            {
                if (getGateWayMAR_F(ethAddr) == -1)
                    return -1;
            }
            else
            {
                while (count < CONNECT_RETRY)
                {
                    if (findEtherAddr_F(ip, ethAddr))
                        break;
                    OSTimeDly(SKT_DLY_TIME);
                    count++;
                }

                if ( count == CONNECT_RETRY)
                {
                    return -1;
                }
            }
        }
    }
#if defined(CONFIG_IPv6_ENABLED)
    else
    {
        while (count < CONNECT_RETRY)
        {
            if (findIPv6EtherAddr_F(ip, ethAddr))
                break;
            OSTimeDly(SKT_DLY_TIME);
            count++;
        }

        if ( count == CONNECT_RETRY)
        {
            DEBUGMSG(SKT_DEBUG,"Can not find destination MAC address");
            return -1;
        }
    }
#endif

    //set UDP destination port
    OSSemPend(SktSem, 0, &err);
    if (s->pcb == 0)
    {
        OSSemPost(SktSem);
        return SKTDISCONNECT;
    }

    if (s->pcb->ipVer == IPv4)
        memcpy(&(s->pcb->destIPAddr), ip, IPv4_ADR_LEN);
    else
        memcpy(&(s->pcb->destIPv6Addr), ip, IPv6_ADR_LEN);

    memcpy(s->pcb->dest_MAR, ethAddr, MAC_ADDR_LEN);

    //Srver port
    s->pcb->destPort = srvPort;

    //client port
    if (cliPort != 0)
    {
        s->pcb->localPort = cliPort;
    }
    else
    {
        s->pcb->localPort = getPortNumber_F(s->pcb);
    }

    tcpPCBRegister_F(s->pcb);
    DEBUGMSG(SKT_DEBUG,"rtSktSetUDPInfo <==");
    OSSemPost(SktSem);
    return 0;
}


int rtSktSend(RTSkt* s, PKT* pkt, int len)
{
    unsigned char err = 0;
    //unsigned int ackNo = 0;
    int ret = SKT_TX_FAIL;

    DEBUGMSG(SKT_DEBUG,"rtSktSend==>");
    OSSemPend(SktSem, 0, &err);

    if (s->pcb == 0)
    {
        freePkt_F(pkt);
        ret = SKTDISCONNECT;
    }
    else
    {
        if (s->pcb->ipVer == IPv4)
            pkt->ipVer = IPv4;
        else
            pkt->ipVer = IPv6;

#ifdef CONFIG_UDP_ENABLED
        if (s->pcb->protocol == IP_PROTO_UDP)
        {
            udpSend_F(s->pcb, pkt, len);
            OSSemPost(SktSem);
            return 0;
        }
#endif

        tcpEnqueue_F(s->pcb, pkt);
        tcpSend_F(s->pcb, pkt, TCP_ACK | TCP_PSH);
        s->pcb->sendPkts++;
    }

    OSSemPost(SktSem);

    //Wait for ACK
    while ( (ret != SKTDISCONNECT) && (s->pcb->sendPkts >= MAXSENDPKTS))
    {
        OSSemPend(s->txSem, OS_TICKS_PER_SEC, &err);
        OSSemPend(SktSem, 0, &err);

        if (s->pcb == 0)
            ret = SKTDISCONNECT;
        else
        {
            if (s->pcb->sendPkts < MAXSENDPKTS)
            {
                ret = 0;
                OSSemPost(SktSem);
                break;
            }
        }
        OSSemPost(SktSem);
    }
    DEBUGMSG(SKT_DEBUG,"rtSktSend<==");
    return ret;
}

int rtSktUDPBind(RTSkt* s, unsigned short srvPort)
{
    unsigned char err = 0;
    OSSemPend(SktSem, 0, &err);
    s->pcb->localPort = srvPort;
    tcpPCBRegister_F(s->pcb);
    OSSemPost(SktSem);
    return 0;
}





/*After process rtSktRx, pkt should be freed by hand*/
PKT* rtSktRx(RTSkt* s, int timeout, int* status)
{
    unsigned char err = 0;
	unsigned short r = 0;
    PKT *pkt;

	if(timeout == 1)
	{
		r = OSSemAccept(s->rxSem);
		if(r == 0)
		{
		    pkt = 0;

			if (s->pcb == 0)
				*status = SKTDISCONNECT;
			else
				*status = OS_ERR_PEVENT_NULL;
			return pkt;
		}
	}
	else
	{
		OSSemPend(s->rxSem, timeout, &err);
	}
	
    if (err == OS_ERR_TIMEOUT )
    {
        pkt = 0;

        if (s->pcb == 0)
            *status = SKTDISCONNECT;
        else
            *status = OS_ERR_TIMEOUT;
        return pkt;
    }

    OSSemPend(SktSem, 0, &err);

    //Connection closed
    if (s->pcb == 0)
    {
        pkt = 0;
        *status = SKTDISCONNECT;
        DEBUGMSG(SKT_DEBUG,"Connection has been disconnected");
    }
    else
    {
        if (s->pcb->rxQStart)
        {
            pkt = s->pcb->rxQStart;
            if (pkt->next)
            {
                s->pcb->rxQStart = pkt->next;
                s->pcb->rxQStart->prev = 0;
                //Only two packets
                if (s->pcb->rxQEnd->prev == pkt)
                {
                    s->pcb->rxQEnd->prev = 0;
                    s->pcb->rxQEnd = pkt->next;
                }
            }
            else
            {
                //Only a packet in list
                s->pcb->rxQStart = 0;
                s->pcb->rxQEnd = 0;
            }
        }
        else
        {
            pkt = 0;
        }
        *status = 0;

        //all packets have been rx
        if (s->pcb->state == TIME_WAIT)
        {
            if (pkt == 0)
                *status = SKTDISCONNECT;
            else if (s->pcb->rxQStart == 0)
                *status = SKTCLOSING;
        }
    }

    OSSemPost(SktSem);

    return pkt;
}

int rtSktCloseSend(RTSkt* s, PKT* pkt, int len)
{
    unsigned char closed = 0;
    unsigned char err = 0;
    unsigned int ackNo = 0;
    int ret = -1;

    DEBUGMSG(SKT_DEBUG,"rtSktCloseSend==>");

    OSSemPend(SktSem, 0, &err);

    if (s->pcb == 0)
    {
        ret = SKTDISCONNECT;
        OSSemPost(SktSem);
        return ret;
    }
    else
    {
        if (s->pcb->ipVer == IPv4)
            pkt->ipVer = IPv4;
        else
            pkt->ipVer = IPv6;

        ackNo = s->pcb->seqNum;
        tcpEnqueue_F(s->pcb, pkt);
        tcpSend_F(s->pcb, pkt, TCP_ACK | TCP_FIN);
    }

    OSSemPost(SktSem);

    do
    {
        DEBUGMSG(SKT_DEBUG,"rtSktCloseSend pend rx sem:%x",s->rxSem);
        OSSemPend(s->rxSem, 0, &err);
        DEBUGMSG(SKT_DEBUG,"rtSktCloseSend pend skt sem");
        OSSemPend(SktSem, 0, &err);
        if (!s->pcb)
            closed = true;
        DEBUGMSG(SKT_DEBUG,"rtSktCloseSend pcb is %x", s->pcb);
        OSSemPost(SktSem);
    }while (!closed);

    DEBUGMSG(SKT_DEBUG,"rtSktCloseSend<==");
    return ret;
}

int rtSktClose(RTSkt* s)
{
    int closed = false;
    unsigned char err = 0;
    char queue = 1;
    char count = 0;

    while (queue)
    {
        OSSemPend(SktSem, 0, &err);
        if (!s->pcb)
            queue  = 0;
        else  if (!s->pcb->txQStart)
            queue  = 0;
        else if (/*s->pcb->txQStart->len == 0 &&*/ s->pcb->txQStart->rTime > 8)
        {
            //Do not block at ack packet or client disconnect
            queue  = 0;
        }

        OSSemPost(SktSem);
        if (queue)
            OSTimeDly(TCP_TIMER_DLY);
    }

    OSSemPend(SktSem, 0, &err);

    if (!s->pcb)
    {
        OSSemPost(SktSem);
        return 0;
    }
    if (s->pcb->protocol == IP_PROTO_UDP || s->pcb->destPort == 0)
    {
        removePCB_F(s->pcb);
        OSSemPost(SktSem);
        return 0;
    }

    s->pcb->state = FIN_WAIT_1;
    tcpSend_F(s->pcb, 0, TCP_ACK|TCP_FIN);
    OSSemPost(SktSem);
    do
    {
        OSSemPend(s->rxSem, OS_TICKS_PER_SEC*0.4, &err);
        OSSemPend(SktSem, 0, &err);
        if (!s->pcb)
            closed = true;

        if (count++ == 10 && s->pcb)
        {
            removePCB_F(s->pcb);
            closed = true;
        }

        OSSemPost(SktSem);
    }while (!closed);

    return 0;
}

int rtSktShutdown(RTSkt* s, int how)
{
    unsigned char err = 0;
    PKT* pkt;
    PKT* npkt;

    OSSemPend(SktSem, 0, &err);
    if (s->pcb)
    {
        switch (how)
        {
        case SKT_DISABLE_RX:
            s->pcb->rxEnable = 0;
            break;
        case SKT_DISABLE_TX:
            s->pcb->txEnable = 0;
            break;
        case SKT_DISABLE_TX_RX:
            s->pcb->rxEnable = 0;
            s->pcb->txEnable = 0;
            break;
        default:
            err = 1;
        }
    }
    else
    {
        err = 1;
    }

    if (s->pcb)
    {
        if (how == SKT_DISABLE_RX || how == SKT_DISABLE_TX_RX)
        {
            pkt = s->pcb->rxQStart;
            while (pkt)
            {
                npkt = pkt->next;
                freePkt_F(pkt);
                pkt = npkt;
            }
            s->pcb->rxQStart = 0;
        }

        if (how == SKT_DISABLE_TX || how == SKT_DISABLE_TX_RX)
        {
            pkt = s->pcb->txQStart;
            while (pkt)
            {
                npkt = pkt->next;
                freePkt_F(pkt);
                pkt = npkt;
            }
            s->pcb->txQStart = 0;
        }
    }

    OSSemPost(SktSem);
    if (err)
        return 1;
    else
        return 0;
}

void EnableDrvWaitOOB()
{
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
    bsp_bits_set(IO_CONFIG, 1, BIT_DRV_WAIT_OOB, 1);
#endif
}

void DisableDrvWaitOOB()
{
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
    bsp_bits_set(IO_CONFIG, 0, BIT_DRV_WAIT_OOB, 1);
#endif
}
#endif
