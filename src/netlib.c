#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lib.h"
#include "ip.h"
#include "tcp.h"
#include "bsp.h"
#include "tls.h"
#include "dhcp.h"
#include "udp.h"
#include "soap.h"
#if(CONFIG_VERSION  >= IC_VERSION_EP_RevA)
#include "hwpf.h"
extern TCAM_Allocation TCAMMem[NumOfTCAMSet];
#endif

#if(CONFIG_VERSION  >=IC_VERSION_EP_RevA)
#define send_F(a,b,c,d) gmacsend_F(a,b,c,d)
#endif

extern DPCONF *dpconf;
extern _IPInfo IPInfo[];
extern OS_EVENT *RxQ;
ROM_EXTERN MulticastList* MulticastListRoot;
extern FWSIG *fwsig;

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)
extern const INT32U crc_table[256];
#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

void* setPkt(unsigned char *start, int size)
{
    PKT *pkt;
    pkt =(PKT*)malloc(PKT_SIZE);

    pkt->next = 0;
    pkt->prev = 0;
    pkt->rTime = 0;
    pkt->queued = 0;
    pkt->start = start;
    //pkt->start =(unsigned char*) malloc(PKT_HDR_SIZE + size);
    //memset(pkt->start, 0, PKT_HDR_SIZE + size);
    pkt->len = size;
    pkt->wp = pkt->start + PKT_HDR_SIZE;

    return (void*)pkt;
}

int getGateWayMAR(unsigned char* cpEthAddr, unsigned char intf)
{
    int i  = 0;
    unsigned int count = 0;

    if (!dpconf->GateWayIP[intf].addr)
        return -1;

    for (i = 0; i < MAC_ADDR_LEN; i++)
    {
        if (IPInfo[intf].IPv4GateWayMAR[i])
        {
            DEBUGMSG(LIB_DEBUG,"IPv4GateWayMAR Exist");
            i = -1;
            break;
        }
    }

    if (i == -1)
    {
        if (cpEthAddr)
            memcpy(cpEthAddr, IPInfo[intf].IPv4GateWayMAR , MAC_ADDR_LEN);
        return 0;
    }
    //If GateWayIP do not be set, GateWay MAC do not be search
    if (dpconf->GateWayIP[intf].addr != 0)
    {
        DEBUGMSG(LIB_DEBUG,"Try to find GateWay MAC address");
        while (count < CONNECT_RETRY)
        {
            if (findEtherAddr_F((unsigned char*)&dpconf->GateWayIP[intf].addr, IPInfo[intf].IPv4GateWayMAR))
                break;
            OSTimeDly(SKT_DLY_TIME);
            count++;
        }
    }

    if (count == CONNECT_RETRY)
    {
        DEBUGMSG(LIB_DEBUG,"Can not find GateWay MAC address");
        return -1;
    }
    else
    {
        if (cpEthAddr)
            memcpy(cpEthAddr, IPInfo[intf].IPv4GateWayMAR , MAC_ADDR_LEN);
        return 0;
    }
}

int getDNSMAR(unsigned char intf)
{
    int count = 0;

    if (dpconf->DNSIP[intf].addr == 0)
        return -1;

    //Not at same domain
    if ((dpconf->DNSIP[intf].addr & dpconf->SubnetMask[intf].addr) != dpconf->MatchSubnetMaskValue[intf])
    {
        if (getGateWayMAR_F(IPInfo[intf].IPv4DNSMAR) == -1)
            return -1;
    }
    else
    {
        while (count < CONNECT_RETRY)
        {
            if (findEtherAddr_F((unsigned char*)&dpconf->DNSIP[intf].addr, IPInfo[intf].IPv4DNSMAR))
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

    return 0;
}

/**
* Description:	Allocate unused space for an object whose size in bytes is specified
				by size and init value is 0
*
* Parameters:
*				size -> The size that want to be allocated not include header size
*
* Returns:		Allocate start address
*
*/
void* allocPkt(int size)
{
    PKT *pkt;
    pkt =(PKT*)malloc(PKT_SIZE);

    pkt->next = 0;
    pkt->prev = 0;
    pkt->rTime = 0;
    pkt->queued = 0;

    pkt->start =(unsigned char*) malloc(PKT_HDR_SIZE + size);
    if (!pkt->start)
    {
        free(pkt);
        return NULL;
    }
    pkt->len = size;
    pkt->wp = pkt->start + PKT_HDR_SIZE;

    return (void*)pkt;
}

#ifdef CONFIG_HTTP_ENABLED
void* allocTLSPkt(void *ptr, int len)
{
    TCPPCB *pcb = (TCPPCB *)ptr;
    PKT *pkt;
    int pageSize = 0;

    pageSize = len + pcb->ssl->macLen + TLS_RECORD_SIZE +  checkPadLen(pcb->ssl->ivLen, len + pcb->ssl->macLen);
    if (checkPadLen(pcb->ssl->ivLen, len + pcb->ssl->macLen))
        pageSize++;

    pkt =(PKT*)malloc(PKT_SIZE);

    pkt->next = 0;
    pkt->prev = 0;
    pkt->rTime = 0;
    pkt->queued = 0;
    //HMAC(20) + TLS_RECORD_SIZE(5) + padding 32
    pkt->start =(unsigned char*) malloc(PKT_HDR_SIZE + pageSize);
    pkt->len = pageSize;
    pkt->wp = pkt->start + PKT_HDR_SIZE + TLS_RECORD_SIZE;

    return (void*)pkt;
}
#endif

void* allocUDPPkt(int size, char ipVer)
{
    PKT *pkt;
    pkt =(PKT*)malloc(PKT_SIZE);

    pkt->next = 0;
    pkt->prev = 0;
    pkt->rTime = 0;
    pkt->queued = 0;
    pkt->start =(unsigned char*) malloc(UDP_PKT_HDR_SIZE + size);
    pkt->len = size;
    pkt->wp = pkt->start + UDP_PKT_HDR_SIZE;
    pkt->ipVer = ipVer;

    return (void*)pkt;
}

void* allocIPPkt(int size, char ipVer)
{
    PKT *pkt;
    pkt =(PKT*)malloc(PKT_SIZE);

    pkt->next = 0;
    pkt->prev = 0;
    pkt->rTime = 0;
    pkt->queued = 0;
    if (IP_PKT_HDR_SIZE + size > 1536)
    {
        pkt->start =(unsigned char*) malloc(1536);
        pkt->wp = pkt->start + 1536 - size;
    }
    else
    {
        pkt->start =(unsigned char*) malloc(IP_PKT_HDR_SIZE + size);
        pkt->wp = pkt->start + IP_PKT_HDR_SIZE;
    }

    pkt->len = size;
    pkt->ipVer = ipVer;

    return (void*)pkt;
}

int sendPkt(PKT *pkt)
{
    TCPHdr	*tcphdr;
    unsigned char ret = 0;	

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

    if (pkt->ipVer == IPv4)
#if WIN
        DMWGWriteProc( pkt->wp + 2, ETH_HDR_SIZE + IP_HLEN + TCP_HDR_SIZE + pkt->len - 2);
#else
		if(pkt->intf == eth0)
			send_F(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN + TCP_HDR_SIZE + pkt->len - 2, OPT_TCPCS, 0);
		else
			pci_tx(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN + TCP_HDR_SIZE + pkt->len - 2, OPT_TCPCS, 0);
#endif
    else
#if WIN
        DMWGWriteProc(pkt->wp + 2, ETH_HDR_SIZE + IPv6_HLEN + TCP_HDR_SIZE + pkt->len - 2);
#else
		if(pkt->intf == eth0)
			send_F(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + TCP_HDR_SIZE + pkt->len - 2,OPT_TCPCS, 0);
		else
			pci_tx(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + TCP_HDR_SIZE + pkt->len - 2,OPT_TCPCS, 0);
#endif
    return ret;
}
/**
* Description:	Send packets that are buffered in TCP queue
*
* Parameters:
*				ptr -> Connection
*
* Returns:		None
*
*/

void sendTCPData(void *ptr)
{
    TCPPCB *pcb = (TCPPCB *)ptr;
    TCPHdr	*tcphdr;
    PKT *pkt = pcb->txQStart;

    //Check if have packets been queued
    if (pcb->txQStart == NULL)
        return;

    //Max packets can be send one time is MAXSENDPKTS
    while ((pcb->sendPkts < MAXSENDPKTS) && pkt)
    {
        if (pkt->ipVer == IPv4)
            tcphdr = (TCPHdr*)(pkt->wp + ETH_HDR_SIZE + IP_HLEN);
        else
            tcphdr = (TCPHdr*)(pkt->wp + ETH_HDR_SIZE + IPv6_HLEN);

        if (pkt->ipVer == IPv4)
		{
#if WIN
            DMWGWriteProc( pkt->wp + 2, ETH_HDR_SIZE + IP_HLEN + TCP_HDR_SIZE + pkt->len - 2);
#else
			if(pcb->intf == eth0)
				send_F(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN + TCP_HDR_SIZE + pkt->len - 2,OPT_TCPCS, 0);
			else
				pci_tx(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN + TCP_HDR_SIZE + pkt->len - 2,OPT_TCPCS, 0);			
#endif
		}
        else
		{
#if WIN
            DMWGWriteProc(pkt->wp + 2, ETH_HDR_SIZE + IPv6_HLEN + TCP_HDR_SIZE + pkt->len - 2);
#else
			if(pcb->intf == eth0)
				send_F(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + TCP_HDR_SIZE + pkt->len - 2,OPT_NOCS, 0);
			else
				pci_tx(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + TCP_HDR_SIZE + pkt->len - 2,OPT_NOCS, 0);
#endif
		}
        pkt->rTime++;
        pcb->sendPkts++;
        //If retransmission is greater than MAXRTTIMES, then reset the connection
        if (pkt->rTime >= MAXRTTIMES )
        {
            tcphdr->flags |= TCP_RST;
        }
        pkt = pkt->next;
    }

    return;
}

/**
* Description:	Add Ehternet header and send the packet
*
* Parameters:
*				ptr -> Connection
*				data-> Pcket start sddress
*				len	-> Packet length
*
* Returns:		None
*
*/
void addEthernetHeader(void *ptr,PKT* pkt)
{
    TCPPCB* pcb = (TCPPCB *) ptr;
    TCPHdr	*tcphdr;
    PEthHdr eHdr;

    DEBUGMSG(LIB_DEBUG,"addEthernetHeader ==>");

    pkt->wp = pkt->wp - ETH_HDR_SIZE;
    eHdr = (PEthHdr)pkt->wp;

    //Make ethernet header
    memcpy(eHdr->destAddr, &pcb->dest_MAR[0], MAC_ADDR_LEN);
    ///memcpy(eHdr->srcAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
    GetMacAddr_F(eHdr->srcAddr, pcb->intf);

#ifdef CONFIG_PROFILING
    mgtcycles[4] = rlx_cp3_get_counter_lo(0);
#endif
    if (pkt->ipVer == IPv4)
    {
        tcphdr = (TCPHdr*)((char*)pkt->wp + ETH_HDR_SIZE + IP_HLEN);
        eHdr->ethType = htons(ETHERNET_TYPE_IPv4);
    }
    else
    {
        tcphdr = (TCPHdr*)((char*)pkt->wp + ETH_HDR_SIZE + IPv6_HLEN);
        eHdr->ethType = htons(ETHERNET_TYPE_IPv6);
    }

    if (pkt->ipVer == IPv4)
    {
        if (pcb->protocol == IP_PROTO_TCP)
        {
#if WIN
            DMWGWriteProc( pkt->wp + 2, ETH_HDR_SIZE + IP_HLEN + TCP_HDR_SIZE + pkt->len - 2);
#else
			if(pcb->intf == eth0)
				send_F(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN + TCP_HDR_SIZE + pkt->len - 2,OPT_TCPCS, 0);
			else
				pci_tx(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN + TCP_HDR_SIZE + pkt->len - 2,OPT_TCPCS, 0);
#endif
            DEBUGMSG(LIB_DEBUG,"IP_PROTO_TCP");
        }
        else if (pcb->protocol == IP_PROTO_ICMP)
        {
#if WIN
            DMWGWriteProc( pkt->wp + 2, ETH_HDR_SIZE + IP_HLEN  + pkt->len - 2);
#else
			if(pcb->intf == eth0)
				send_F(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN  + pkt->len - 2, OPT_IPCS | OPT_FREE, pkt->start);
			else
				pci_tx(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN  + pkt->len - 2, OPT_IPCS | OPT_FREE, pkt->start); 
#endif
            DEBUGMSG(LIB_DEBUG,"IP_PROTO_ICMP");
        }
        else if (pcb->protocol == IP_PROTO_UDP)
        {
#if WIN
            DMWGWriteProc( ((char*) pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN  + pkt->len -2);
#else
			if(pcb->intf == eth0)
				send_F(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN  + pkt->len - 2,OPT_UDPCS | OPT_FREE, pkt->start);
			else
				pci_tx(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IP_HLEN  + pkt->len - 2,OPT_UDPCS | OPT_FREE, pkt->start);
#endif
            DEBUGMSG(LIB_DEBUG,"IP_PROTO_UDP");
        }
        else
        {
            DEBUGMSG(LIB_DEBUG,"Unknown protocol");
        }
    }
    else
    {
        if (pcb->protocol == IP_PROTO_TCP)
        {
#if WIN
            DMWGWriteProc( ((char*) pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + TCP_HDR_SIZE + pkt->len -2);
#else
			if(pcb->intf == eth0)
				send_F(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + TCP_HDR_SIZE + pkt->len - 2, OPT_IPV6|OPT_TCPCS, 0);
			else
				pci_tx(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + TCP_HDR_SIZE + pkt->len - 2, OPT_IPV6|OPT_TCPCS, 0);
#endif
            DEBUGMSG(LIB_DEBUG,"IP_PROTO_TCP");
        }
        else if (pcb->protocol == IP_PROTO_ICMPv6)
        {
#if WIN
            DMWGWriteProc( ((char*) pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + pkt->len -2);
#else
			if(pcb->intf == eth0)
				send_F(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + pkt->len - 2, OPT_IPV6 | OPT_NOCS | OPT_FREE, pkt->start);
			else
				pci_tx(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + pkt->len - 2, OPT_IPV6 | OPT_NOCS | OPT_FREE, pkt->start);
#endif
            DEBUGMSG(LIB_DEBUG,"IP_PROTO_ICMPv6");
        }
        else if (pcb->protocol == IP_PROTO_UDP)
        {
#if WIN
            DMWGWriteProc( ((char*) pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + pkt->len -2);
#else
			if(pcb->intf == eth0)
				send_F(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + pkt->len - 2, OPT_IPV6 | OPT_UDPCS | OPT_FREE, pkt->start);
			else
				pci_tx(((char*)pkt->wp) + 2, ETH_HDR_SIZE + IPv6_HLEN + pkt->len - 2, OPT_IPV6 | OPT_UDPCS | OPT_FREE, pkt->start);	
#endif
            DEBUGMSG(LIB_DEBUG,"IP_PROTO_UDP");
        }
        else
        {
            DEBUGMSG(LIB_DEBUG,"Unknown protocol");
        }
    }

    if (pcb->protocol != IP_PROTO_TCP)
    {

        if (pcb->protocol != IP_PROTO_UDP)
            free(pcb);

        //Data will be freed at driver
        //freePkt(pkt);
        free(pkt);
        pkt = 0;
        DEBUGMSG(LIB_DEBUG,"addEthernetHeader Non TCP <==");
        return;
    }

    /*
            //Do not count three way handshaking
        if (tcphdr->flags == TCP_ACK || tcphdr->flags == TCP_FIN || (tcphdr->flags == (TCP_ACK|TCP_FIN)))
    		;//pcb->sendPkts++;
        else
            //Do not free the data that wait for acknowledgment
    		freePkt(pkt);//If free too fast, driver will send strange data
    */
    if (!pkt->queued)
        freePkt_F(pkt);//If free too fast, driver will send strange data

    DEBUGMSG(LIB_DEBUG,"addEthernetHeader TCP <==");
}

void setIPAddress(unsigned int ip, unsigned char intf)
{
    //Store with big endian
    //IPv4Cfg.HostIP.addr = htonl(ip);
    //REG32(IOREG_IOBASE + MAC_RxCR) = 0x00;
    //REG32(IOREG_IOBASE + IO_DIPV4) = 0x0;
    OS_CPU_SR cpu_sr = 0;
    OS_Q_DATA rxqdata;
    INT8U *rxbuf;
    INT8U err;
    dpconf->HostIP[intf].addr = htonl(ip);
    #if(CONFIG_VERSION > IC_VERSION_DP_RevF )
    //Hw store with big endian
    memcpy(&(TCAMMem[TCAMDIPv4Set].data[UniIPv4Addr*IPv4_ADR_LEN]), &dpconf->HostIP[intf].addr, IPv4_ADR_LEN);
    setTCAMData_F(&TCAMMem[TCAMDIPv4Set]);
    #else
    REG32(IOREG_IOBASE+IO_DIPV4)= ip;
    bsp_bits_set(IO_DIPFILTER, 1, BIT_FILTERV4DIP0, 1);
    #endif

    //ip == 0 means DHCP initial
    if (ip != 0)
    {
        DEBUGMSG(LIB_DEBUG, "Set ip address %x\n", ip);
        ///srvReset_F(IPv4);
        IPInfo[intf].IPv4Reset = 1;
        OS_ENTER_CRITICAL();
        OSQQuery(RxQ, &rxqdata);
        while (rxqdata.OSNMsgs--)
        {
            rxbuf = (INT8U *) OSQAccept(RxQ,&err);
            free(rxbuf);
        }
        OS_EXIT_CRITICAL();
    }

    if (dpconf->ProvisioningState == PROVISIONED)
        setdirty(DPCONFIGTBL);
}


void setMatchSubnetMask(unsigned char intf)
{
#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    unsigned int dip;
    char *p;
    char i1, i2;
    //DIP1
    p = (char *)&dpconf->SubnetMask[intf].addr;

    i1 = i2 = 0;
    while (i1 < IPv4_ADR_LEN)
    {
        if (*p == 0)
            i2++;
        i1++;
        p++;
    }

    memcpy(&dip, &dpconf->HostIP[intf].addr, IPv4_ADR_LEN - i2);
    p = (char *)&dip;
    p = p + (IPv4_ADR_LEN - i2);
    memset(p, 0xff, i2);
    
    #if (CONFIG_VERSION > IC_VERSION_DP_RevF )
    memcpy(&(TCAMMem[TCAMDIPv4Set].data[GBIPv4Addr*IPv4_ADR_LEN]), &dip, IPv4_ADR_LEN);
    dip = 0xffffffff;
    memcpy(&(TCAMMem[TCAMDIPv4Set].data[LBIPv4Addr*IPv4_ADR_LEN]), &dip, IPv4_ADR_LEN);
    //setIPv4AddrSet(&HostIPv4AddrSet);
    #else
    REG32(IOREG_IOBASE + IO_DIP1V4)= htonl(dip);
    bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP1, 1);

    //DIP2
    REG32(IOREG_IOBASE + IO_DIP2V4)= 0xffffffff;
    bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP2, 1);
    #endif
#endif

    dpconf->MatchSubnetMaskValue[intf] = dpconf->HostIP[intf].addr & dpconf->SubnetMask[intf].addr;
}

void setSubnetMask(unsigned int ip, unsigned char intf)
{
    //Store with big endian
    dpconf->SubnetMask[intf].addr = htonl(ip);
    setMatchSubnetMask_F(intf);
    setdirty(DPCONFIGTBL);
}


void setGateWayIP(unsigned int ip, unsigned char intf)
{
    dpconf->GateWayIP[intf].addr = htonl(ip);
    setdirty(DPCONFIGTBL);
}

void setDNSIP(unsigned int ip, unsigned char intf)
{
    //Store with big endian
    dpconf->DNSIP[intf].addr = htonl(ip);
    setdirty(DPCONFIGTBL);
}

unsigned int getIPAddress(unsigned char intf)
{
    //big endian
    return dpconf->HostIP[intf].addr;
}

void setIPv6GateWay(unsigned char *ip, unsigned char intf)
{
    memcpy(dpconf->IPv6GateWay[intf], ip, IPv6_ADR_LEN);
    setdirty(DPCONFIGTBL);
}

void setIPv6DNS(unsigned char *ip, unsigned char intf)
{
    memcpy(dpconf->IPv6DNS[intf], ip, IPv6_ADR_LEN);
    setdirty(DPCONFIGTBL);
}



void setLinkLocalAddress(unsigned char *ip, unsigned char intf)
{
    unsigned char mar[MAC_ADDR_LEN] = {0};

    //Do not reset IPv6 protocol, or router advertisement msg will be skiped
    memcpy(dpconf->HostIPv6[intf], ip, IPv6_ADR_LEN);

    mar[0] = 0x33;
    mar[1] = 0x33;
    mar[2] = 0xff;
    mar[3] = dpconf->HostIPv6[intf][IPv6_ADR_LEN - 3];
    mar[4] = dpconf->HostIPv6[intf][IPv6_ADR_LEN - 2];
    mar[5] = dpconf->HostIPv6[intf][IPv6_ADR_LEN - 1];
    addMulticasAddress_F(mar);
    setMulticastList_F();
}

unsigned char* getIPv6Address(unsigned char intf)
{
    //big endian
    return dpconf->HostIPv6[intf];
}

void resetMulticasAddress()
{
    MulticastList* tmp = 0;
    MulticastList* next = 0;

    if (MulticastListRoot == 0)
    {
        return;
    }
    else
    {
        tmp = MulticastListRoot;
        next = tmp->next;

        do
        {
            free(tmp);
            tmp = next;
            if (tmp)
                next = tmp->next;
        }while (tmp);
    }
    MulticastListRoot = 0;
}

void addMulticasAddress(unsigned char addr[6])
{
    MulticastList* node;
    MulticastList* tmp;
    node = (MulticastList*)malloc(sizeof(MulticastList));
    node->next = 0;
    memcpy(node->addr, addr, 6);

    if (MulticastListRoot == 0)
    {
        MulticastListRoot = node;
    }
    else
    {
        tmp = MulticastListRoot;
        while (tmp->next)
        {
            tmp = tmp->next;
        }
        tmp->next = node;
    }
}

unsigned int rtl_ether_crc( int length, const unsigned char* data )
{
    unsigned const ethernet_polynomial = 0x04c11db7U;
    int crc = -1;
    int bit;
    unsigned char current_octet;

    while (--length >= 0) {
        current_octet = *data++;
        for (bit = 0; bit < 8; bit++, current_octet >>= 1)
            crc = (crc << 1) ^
                  ((crc < 0) ^ (current_octet & 1) ? ethernet_polynomial : 0);
    }
    return crc;
}

void setMulticastList()
{
    MulticastList *mList = MulticastListRoot;
    unsigned int regM0 = 0;
    unsigned int regM4 = 0;
    unsigned int tmp = 0;
    int bit;

    while (mList)
    {
        bit = rtl_ether_crc(6, (const char *) mList->addr) >> 26;
        if (bit < 32)
            regM0 |= (1 << bit);
        else
            regM4 |= (1 << (bit - 32));

        mList = mList->next;
    }

    tmp = ((regM4&0xff000000)>>24) | ((regM4&0x00ff0000)>>8) | ((regM4&0x0000ff00)<<8) | ((regM4&0x000000ff)<<24);
    REG32(IOREG_IOBASE+MAC_MAR0)   = tmp;
    tmp = ((regM0&0xff000000)>>24) | ((regM0&0x00ff0000)>>8) | ((regM0&0x0000ff00)<<8) | ((regM0&0x000000ff)<<24);
    REG32(IOREG_IOBASE+MAC_MAR4)   = tmp;
}


DataList* newDataList()
{
    PKT* pkt;
    DataList *ptr = malloc(sizeof(DataList));
    memset(ptr, 0 , sizeof(DataList));
    ptr->addr = (unsigned char*)allocPkt_F(ETH_PAYLOAD_LEN);
    pkt = (PKT*)ptr->addr;

    ptr->payLoadAddr = pkt->wp;
    ptr->payLoadAddr[0] = 0;

    //Set ethernet , ip ,tcp header to 0
#if 0
    memset(ptr->addr, 0, PKT_HDR_SIZE);
    ptr->payLoadAddr = ptr->addr + PKT_HDR_SIZE;
    ptr->payLoadAddr[0] = 0;
    pkt->start = ptr->addr;
    pkt->wp = ptr->payLoadAddr;
#endif
    return ptr;
}

DataList* chkDataListBufSize(DataList* dl)
{
    DataList *ptr = dl;
    if ( (ETH_PAYLOAD_LEN - dl->len) < LEAST_PAGE_LINE_LEN)
    {
        ptr = newDataList();
        dl->next = ptr;
    }
    return ptr;
}

unsigned int getDataListBufSize(DataList* head)
{
    DataList *ptr = head;
    unsigned int size = 0;

    do
    {
        size+=ptr->len;
        ptr = ptr->next;
    }while (ptr);

    return size;
}

#ifdef CONFIG_HTTP_ENABLED
void sendDataList(struct tcp_pcb *pcb, DataList* head)
{
    DataList *ptr = head;
    PKT* pkt;
    PKT* tlsPkt;

    do
    {
        if (pcb->TLSEnable)
        {
            tlsPkt = allocTLSPkt(pcb, ptr->len);
            pkt = (PKT*)ptr->addr;
            memcpy(tlsPkt->wp, pkt->wp, ptr->len);
            freePkt_F(pkt);
#if CONFIG_VERSION == IC_VERSION_DP_RevF
            fwsig->tlsEncodeData_F(tlsPkt, ptr->len, pcb, RECORD_APP_PROTOCOL_DATA);
#else
            tlsEncodeData_F(tlsPkt, ptr->len, pcb, RECORD_APP_PROTOCOL_DATA);
#endif
        }
        else
        {
            pkt = (PKT*)ptr->addr;
            pkt->len = ptr->len;
            tcpSendData_F(pcb, pkt);
        }

        head = ptr;
        ptr = ptr->next;
        free(head);
    }while (ptr);
}
#endif

void freePkt(PKT *pkt)
{
    if (pkt->start)
    {
        free(pkt->start);
    }

    free(pkt);
}

RTSkt *stopconn(RTSkt* cs)
{
    INT8U err;
    rtSktClose_F(cs);
    OSSemDel(cs->rxSem, OS_DEL_ALWAYS, &err);
    OSSemDel(cs->txSem, OS_DEL_ALWAYS, &err);
    cs->rxSem = 0;
    cs->txSem = 0;
    if (cs)
        free(cs);
    return NULL;
}

INT32U crc32(INT32U crc, INT8U *buf, INT32U len)
{
    if (buf == NULL ) return 0;
    crc = crc ^ 0xffffffff;

    while (len >= 8)
    {
        DO8(buf);
        len -= 8;
    }
    if (len) do {

            DO1(buf);
        } while (--len);

    return crc ^ 0xffffffff;
}
#endif

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION == IC_VERSION_DP_RevA)
#if BYTE_ORDER != BIG_ENDIAN
/*-----------------------------------------------------------------------------------*/
u16_t
htons(u16_t n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}
/*-----------------------------------------------------------------------------------*/
u16_t
ntohs(u16_t n)
{
    return htons(n);
}
/*-----------------------------------------------------------------------------------*/
u32_t
htonl(u32_t n)
{
    return ((n & 0xff) << 24) |
           ((n & 0xff00) << 8) |
           ((n & 0xff0000) >> 8) |
           ((n & 0xff000000) >> 24);
}
/*-----------------------------------------------------------------------------------*/
u32_t
ntohl(u32_t n)
{
    return htonl(n);
}
#endif

unsigned int chksum(void *data,int len)
{
    unsigned short *sdata = data;
    unsigned int acc;

    for (acc = 0; len > 1; len -= 2) {
        acc += *sdata++;
    }

    /* add up any odd byte */
    if (len == 1) {
        acc += (unsigned short)(*(unsigned char *)sdata);
    }

    return acc;
}

/**
* Description:	Calculates the Internet checksum over a portion of memory.
*
* Parameters:
*				data->	Data start address
*				len->	Data length
*
* Returns:		checksum
*
* Note:			Refenerce lwip
*/
u16_t inet_chksum(void *data, int len)
{
    u32_t acc, sum;

    acc = chksum(data, len);
    sum = (acc & 0xffff) + (acc >> 16);
    sum += (sum >> 16);
    return ~(sum & 0xffff);
}
/**
* Description:	Calculates the pseudo Internet checksum used by TCP
*
* Parameters:
*				p->			tcp start address
*				src->		IP source address
*				dest->		IP sestination address
*				proto->		Protocol
*				proto_len->	TCP header size + TCP payload size
*
* Returns:		checksum
*
* Note:			Refenerce lwip
*/
unsigned short inet_chksum_pseudo(void *p, void *srcptr, void *destptr,
                                  unsigned char proto, unsigned short proto_len)
{
    IPAddress *src = (IPAddress *)srcptr;
    IPAddress *dest = (IPAddress *)destptr;
    unsigned int acc;
    char swapped;

    acc = 0;
    swapped = 0;

    acc += chksum(p, proto_len);
    while (acc >> 16) {
        acc = (acc & 0xffff) + (acc >> 16);
    }
    if (proto_len % 2 != 0) {
        swapped = 1 - swapped;
        acc = ((acc & 0xff) << 8) | ((acc & 0xff00) >> 8);
    }

    if (swapped) {
        acc = ((acc & 0xff) << 8) | ((acc & 0xff00) >> 8);
    }
    acc += (src->addr & 0xffff);
    acc += ((src->addr >> 16) & 0xffff);
    acc += (dest->addr & 0xffff);
    acc += ((dest->addr >> 16) & 0xffff);
    acc += (u32_t)htons((u16_t)proto);
    acc += (u32_t)htons(proto_len);

    while (acc >> 16) {
        acc = (acc & 0xffff) + (acc >> 16);
    }

    return ~(acc & 0xffff);
}

int intfDHCPv4Enable(int intf)
{
	return (dpconf->DHCPv4Enable & (1<<intf));
}

void onoffIntfDHCPv4(int intf, int state)
{
	if(state)
		dpconf->DHCPv4Enable |= (1<<intf);
	else
		dpconf->DHCPv4Enable &= (~(1<<intf));

}

int intfIPv4Enable(int intf)
{
	return (dpconf->IPv4Enable & (1<<intf));
}

int intfIPv6Enable(int intf)
{
	return (dpconf->IPv6Enable & (1<<intf));
}


void onoffIntfIPv4(int intf, int state)
{
	if(state)
		dpconf->IPv4Enable |= (1<<intf);
	else
		dpconf->IPv4Enable &= (~(1<<intf));

}

int Wlan0Active()
{
	return 1;
}
#endif
