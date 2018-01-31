/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	dhcp.c

Abstract:	DHCP Client

*****************************************************************************
*/

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "bsp.h"
#include "dhcp.h"
#include "tcp.h"
#include "ip.h"
#include "lib.h"
#include "udp.h"
#include "icmp.h"
#include "soap.h"
#include "teredo.h"
#include "hwpf.h"

ROM_EXTERN struct _DHCPv6Conf DHCPv6Conf[MAX_INTF];

#ifdef CONFIG_IPv6_ENABLED
#define OPTION_CLIENTID 0x01
#define OPTION_SERVERID 0x02
#define OPTION_IA_NA 0x03
#define OPTION_IAADDR 0x05
extern DPCONF *dpconf;
extern OS_EVENT *SktSem;
extern _IPInfo IPInfo;
extern enum DHCPv6States DHCPv6State;

#if CONFIG_VERSION  < IC_VERSION_DP_RevF
ROM_EXTERN const char All_DHCPv6_Agents_IP[IPv6_ADR_LEN] = {0xFF,0x02,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x02 };

ROM_EXTERN const char DHCPv6_All_Router_MAR[MAC_ADDR_LEN] = {0x33,0x33,0x00,0x01,0x00,0x02};

ROM_EXTERN unsigned int DHCPTID = 0;
ROM_EXTERN unsigned int LeaseTime = 0;
#else

ROM_EXTERN const char All_DHCPv6_Agents_IP[IPv6_ADR_LEN] = {0xFF,0x02,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x02 };
ROM_EXTERN const char DHCPv6_All_Router_MAR[MAC_ADDR_LEN] = {0x33,0x33,0x00,0x01,0x00,0x02};
unsigned int DHCPTID;
#endif

#endif

ROM_EXTERN DHCPv4Config DHCPv4Conf[MAX_INTF];
extern DPCONF *dpconf;
extern OS_EVENT *SktSem;
extern _IPInfo IPInfo;

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_EP_RevA)
#ifdef CONFIG_DHCP_ENABLED

INT32U getRandTransactionID(int intf){
	INT32U id;
	INT8U clientmac[20];

	
	GetMacAddr_F(clientmac, intf);	
	
	id = rand();
	memcpy(clientmac+6 , &id  ,  4);
	id = rand();
	memcpy(clientmac+6+4 , &id  ,  4);
	id = rand();
	memcpy(clientmac+6+4 + 4, &id  ,  4);
	
	sha1(clientmac , 20 , clientmac);
	memcpy(&id , clientmac , 4);

	return id;
}

void dhcpSend(RTSkt* s, int type)
{
    DHCPPkt *pktHdr;
    //unsigned char optionData[60];
    INT8U *optionData = malloc(60);
    PKT* pkt;
    unsigned char *p;

    DEBUGMSG(DHCPv4_DEBUG,"dhcpSend ==>");
    //64 is option size
    pkt = allocUDPPkt_F(sizeof(DHCPPkt) + 64, IPv4);
    p = pkt->wp;
    memset(pkt->wp, 0, sizeof(DHCPPkt) + 64);
    pktHdr = (DHCPPkt *)pkt->wp;


    //DHCP pkt
    pktHdr->opCode = BOOT_REQUEST;
    //Ethernet
    pktHdr->hwtype = 0x01;
    //Ethernet
    pktHdr->HwLen = 0x06;
    pktHdr->hopCount = 0x0;//0xff; some DHCP server do not support hop count > 1
    if (type == DHCP_DISCOVERY)
    {
        //DHCPv4Conf[intf].DHCPv4TID = rand();
        DHCPv4Conf[s->pcb->intf].DHCPv4TID = getRandTransactionID(s->pcb->intf);
    }
    pktHdr->TID = htonl(DHCPv4Conf[s->pcb->intf].DHCPv4TID);

    //memcpy(pktHdr->clientHwAddress, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
    GetMacAddr_F(pktHdr->clientHwAddress, s->pcb->intf);

    p = pkt->wp + sizeof(DHCPPkt);

    //Magic cookies
    p[0] =0x63;
    p[1] =0x82;
    p[2] =0x53;
    p[3] =0x63;
    p += 4;
    //DHCP message type
    if (type == DHCP_DISCOVERY)
    {
        optionData[0] = DHCP_DISCOVERY;
        DHCPv4Conf[s->pcb->intf].DHCPv4State = SelectState;
    }
    else
    {
        optionData[0] = DHCP_REQUEST;
        DHCPv4Conf[s->pcb->intf].DHCPv4State = RequestState;
    }

    p += dhcpOptions_F(p, 0x35, 1, optionData);
    //Client Identifier = Hw Type + hardware address
    optionData[0] = 0x01;
    ///memcpy(optionData + 1, dpconf->srcMacAddr[s->pcb->intf], MAC_ADDR_LEN);
    GetMacAddr_F(optionData + 1, s->pcb->intf);
    p += dhcpOptions_F(p , 61, 1 + MAC_ADDR_LEN , optionData);
    //Requested IP Address
    if (type == DHCP_REQUEST)
    {
        memcpy(optionData, &DHCPv4Conf[s->pcb->intf].OfferIP.addr, 4);
        p += dhcpOptions_F(p , 50, 4 , optionData);
    }

     //Server identifier (Type 54)---------------------------------------------
     if(type == DHCP_REQUEST){
	     memcpy(optionData , &(s->pcb->destIPAddr.addr) , 4);
	     memset(&(s->pcb->destIPAddr.addr),0xff ,4);
	     p += dhcpOptions_F(p , 54, 4 ,optionData );
     } 
    //Host name
    optionData[0] = 'd';
    optionData[1] = 'a';
    optionData[2] = 's';
    optionData[3] = 'h';
    p += dhcpOptions_F(p , 12, 4 , optionData);
    //Parameter List
    optionData[0] = 1;
    optionData[1] = 3;
    optionData[2] = 6;
    optionData[3] = 15;
    p += dhcpOptions_F(p , 55, 4 , optionData);
    //Edn of option
    *p = 0xff;

    DEBUGMSG(DHCPv4_DEBUG,"dhcpSend <==");

    free(optionData);
    rtSktSend_F(s, pkt, sizeof(DHCPPkt) + 64);
}

char dhcpInput(RTSkt* s, unsigned char *p, int len)
{
    DHCPPkt* pktHdr = (DHCPPkt*)p;
    unsigned char *opt = p + sizeof(DHCPPkt);
    char msgType = 0;
    INT8U clientmac[6];
    INT32U serverid;

    DEBUGMSG(DHCPv4_DEBUG,"dhcpInput ==>");

    if (pktHdr->opCode != BOOT_REPLY)
    {
        DEBUGMSG(DHCPv4_DEBUG,"Not matching op code");
        return -1;
    }

    if ( ntohl(pktHdr->TID)!= DHCPv4Conf[s->pcb->intf].DHCPv4TID)
    {
        DEBUGMSG(DHCPv4_DEBUG,"Not matching DHCPTID");
        return -1;
    }

    if (pktHdr->yourIP != 0)
        DHCPv4Conf[s->pcb->intf].OfferIP.addr = pktHdr->yourIP;


    //check cookie
    if (	opt[0] !=0x63 || opt[1] != 0x82 || opt[2] !=0x53 || opt[3] != 0x63)
    {
        DEBUGMSG(DHCPv4_DEBUG,"Not matching cookie");
        return -1;
    }

    opt += 4;
    len  = len - sizeof(DHCPPkt) -4;

    if (dhcpGetOptionsValue_F(opt, 53 , sizeof(unsigned char), &msgType, len) == -1)
        return -1;

	if (dhcpGetOptionsValue_F(opt, 54 , 4, &serverid, len) == -1){//Server identifier 	
        		return -1;
	}


	s->pcb->destIPAddr.addr = serverid;

	//pktHdr->clientHwAddress
	//
	memset(clientmac,0x00,6);
	GetMacAddr_F(clientmac, s->pcb->intf);
	if(memcmp(clientmac,pktHdr->clientHwAddress,6) != 0x00){
		printf("[RTK] MAC NOT MATCHING\n");
		return -1;
	}
	

    switch (msgType)
    {
    case DHCP_OFFER:
        if (DHCPv4Conf[s->pcb->intf].DHCPv4State != SelectState)
            break;
        if (dhcpReply_F(DHCP_OFFER, opt, len, s->pcb->intf) == -1)
        {
            DEBUGMSG(DHCPv4_DEBUG | CRITICAL_ERROR, "DHCP offer parse error");
            return -1;
        }
        DEBUGMSG(DHCPv4_DEBUG, "DHCP REQUEST");
        dhcpSend_F(s, DHCP_REQUEST);
        break;
    case DHCP_ACK:
        if (DHCPv4Conf[s->pcb->intf].DHCPv4State != RequestState)
            break;
        if (dhcpReply_F(DHCP_ACK, opt, len, s->pcb->intf) == -1)
        {
            DEBUGMSG(DHCPv4_DEBUG | CRITICAL_ERROR, "DHCP ack parse error");
            return -1;
        }
        DEBUGMSG(DHCPv4_DEBUG, "DHCP ACK");
        setIPAddress_F(ntohl(pktHdr->yourIP), s->pcb->intf);
        setMatchSubnetMask_F(s->pcb->intf);
        DEBUGMSG(DHCPv4_DEBUG | MESSAGE_DEBUG, "DHCP IP: %x\n",ntohl(pktHdr->yourIP));
        DHCPv4Conf[s->pcb->intf].DHCPv4RTTime = 1;
        DHCPv4Conf[s->pcb->intf].DHCPv4State = BoundState;
        break;
    case DHCP_NACK:
        DHCPv4Conf[s->pcb->intf].DHCPv4State = InitState;
        break;
    default:
        return -1;
        break;
    }
    DEBUGMSG(DHCPv4_DEBUG,"dhcpInput <==");
    return 0;
}

extern TCAMRule TCAMRuleMem[NumOfPFRule];

void EnableUnicastMacAddressPF(){

	TCAMRuleMem[OOBUnicastPFRule].DIPv4BitMap = 0;
	setPFRule_F(&TCAMRuleMem[OOBUnicastPFRule]);
	
}

void DHCPv4Task(void *data)
{
    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_UDP);
    unsigned char ip[]={0xff, 0xff, 0xff, 0xff};
    unsigned char err = 0;
    PKT* pkt;
    int status = 0;

	#ifdef CONFIG_DHCP4_AUTO
	int times = 0;
	#endif

#if CONFIG_VERSION >= 6
    rtSktEnableOOBPortFilter_F(s);
#endif
    rtSktSetUDPInfo_F(s, ip, DHCPv4_Srv, DHCPv4_Cli);

    do
    {
        memset(&DHCPv4Conf[s->pcb->intf], 0, sizeof(DHCPv4Conf));

        while ((dpconf->DHCPv4Enable & (1<< s->pcb->intf)) && DHCPv4Conf[s->pcb->intf].DHCPv4State != BoundState && !bsp_in_band_exist())
        {
            IPInfo.AB = 1;
            DHCPv4Conf[s->pcb->intf].DHCPv4State = InitState;
            DHCPv4Conf[s->pcb->intf].DHCPv4RTTime = 1;

            DEBUGMSG(DHCPv4_DEBUG,"DHCPv4 Start");
            OSSemPend(SktSem, 0, &err);
            setIPAddress_F(0, s->pcb->intf);
			setMatchSubnetMask_F(s->pcb->intf);
#if CONFIG_VERSION == IC_VERSION_DP_RevF
            bsp_bits_set(IO_HWFunCtr, 1, BIT_AAB, 1);
#elif CONFIG_VERSION < IC_VERSION_DP_RevF
            REG32(IOREG_IOBASE+MAC_RxCR) &= ~0x40000000;
#endif
            OSSemPost(SktSem);
            //DHCP dicovery
            DEBUGMSG(DHCPv4_DEBUG,"DHCPv4 DISCOVERY");
			EnableUnicastMacAddressPF(); //+ to Fix issue : firmware cannot reveive the packet with unicast MAC , at DHCP mode.

            dhcpSend_F(s, DHCP_DISCOVERY);

            do
            {
                //If no timeout and no dhcp server, it will pend
                //The time interval between discover and offer is 2 sec

                pkt = rtSktRx_F(s, OS_TICKS_PER_SEC*4, &status);
                if (status == SKTDISCONNECT)
                {
                    DEBUGMSG(CRITICAL_ERROR | DHCPv4_DEBUG,"SKT been closed");
                }

                if (bsp_in_band_exist())
                    break;

                if (pkt == 0)
                {
                    DEBUGMSG(DHCPv4_DEBUG,"TimeOut");
                    DHCPv4TimeOut_F();
                    dhcpSend_F(s, DHCP_DISCOVERY);
                    continue;
                }

                //DHCP offer
                //DHCP request
                DEBUGMSG(DHCPv4_DEBUG,"A DHCPv4 incoming");
                dhcpInput_F(s, pkt->wp, pkt->len);
                freePkt_F(pkt);
            }while (DHCPv4Conf[s->pcb->intf].DHCPv4State != RequestState && (dpconf->DHCPv4Enable & (1<< s->pcb->intf)));

            if (!(dpconf->DHCPv4Enable & (1<< s->pcb->intf)))
                break;

            do
            {
                pkt = rtSktRx_F(s, OS_TICKS_PER_SEC*4, &status);
                if (status == SKTDISCONNECT)
                {
                    DEBUGMSG(CRITICAL_ERROR | DHCPv4_DEBUG,"SKT been closed");
                }

                if (bsp_in_band_exist())
                    break;

                //DHCP ACK
                if (pkt != 0)
                {
                    dhcpInput_F(s, pkt->wp, pkt->len);
                    freePkt_F(pkt);
                    if (DHCPv4Conf[s->pcb->intf].DHCPv4State == InitState)
                        break;
                }
                else
                {
                    DHCPv4TimeOut_F();
                    dhcpSend_F(s, DHCP_REQUEST);
                }
            }while (DHCPv4Conf[s->pcb->intf].DHCPv4State != BoundState && DHCPv4Conf[s->pcb->intf].DHCPv4RTTime != 15 && (dpconf->DHCPv4Enable & (1<< s->pcb->intf)));
        }

        IPInfo.AB = 0;

#if CONFIG_VERSION == 6
        bsp_bits_set(IO_HWFunCtr, 0, BIT_AAB, 1);
#else
        REG32(IOREG_IOBASE+MAC_RxCR) |= 0x40000000;
#endif

        DEBUGMSG(DHCPv4_DEBUG,"DHCPv4 Finish");
        //remove PCB for ActivePCBsList, prevent queue packets
        OSSemPend(SktSem, 0, &err);
        tcpPCBUnRegister_F(s->pcb);
        OSSemPost(SktSem);
        //Use a timer to renew IP
#ifdef CONFIG_TEREDO_ENABLED
        if ((dpconf->DHCPv4Enable & (1<< s->pcb->intf)) && dpconf->useTeredo)
        {
            initTeredo();
        }
#endif
        OSTaskSuspend(OS_PRIO_SELF);
        OSSemPend(SktSem, 0, &err);
        tcpPCBRegister_F(s->pcb);
        OSSemPost(SktSem);
        DHCPv4Conf[s->pcb->intf].DHCPv4State = InitState;
        DHCPv4Conf[s->pcb->intf].DHCPv4RTTime = 1;
    }while (1);
}
#endif

#else
#endif

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)

#ifdef CONFIG_DHCP_ENABLED

int dhcpOptions(unsigned char *opt, unsigned char tag, unsigned char len, unsigned char *value)
{
    unsigned char i = 0;

    opt[0] = tag;
    opt[1] = len;

    for (i = 0; i < len;  i++)
    {
        opt[i+2] = value[i];
    }

    return len+2;
}

char dhcpGetOptionsValue(unsigned char *opt, unsigned char tag, unsigned char typeLen, void *value, int dataLen)
{
    unsigned char i = 0;
    char found = -1;

    while (dataLen - i > 0 && opt[i] != 0xff)
    {
        if (opt[i] != tag)
        {
            i += opt[i+1] + 2;
            continue;
        }
        else
        {
            if ((dataLen - i - 2 - typeLen) > 0)
            {
                memcpy(value ,opt + 2 + i, typeLen);
                found = 0;
            }
            else if (typeLen >= opt[i+1])
            {
                memcpy(value ,opt + 2 + i, opt[i+1]);
                found = 0;

            }
            break;
        }
    }

    return found;
}



char dhcpReply(int type, unsigned char *opt, int len, unsigned char intf)
{
    DEBUGMSG(DHCPv4_DEBUG,"dhcpReply ==>");
    if (dhcpGetOptionsValue_F(opt, 1 , sizeof(unsigned int), &(dpconf->SubnetMask[intf].addr), len) == -1)
        return -1;
    else
        setMatchSubnetMask_F(intf);

    DEBUGMSG(DHCPv4_DEBUG, "SubnetMask: %x",ntohl(dpconf->SubnetMask[intf].addr));

    dhcpGetOptionsValue_F(opt, 3 , IPv4_ADR_LEN, (unsigned char*)&dpconf->GateWayIP[intf].addr, len);
    dhcpGetOptionsValue_F(opt, 6 , IPv4_ADR_LEN, (unsigned char*)&dpconf->DNSIP[intf].addr, len);
    if (dpconf->ProvisioningState != PROVISIONED)
        dhcpGetOptionsValue_F(opt, 15 , MAX_DN_LEN, (unsigned char*)&dpconf->DomainName, len);
    dhcpGetOptionsValue_F(opt, 51 , sizeof(unsigned int), &(DHCPv4Conf[intf].DHCPv4LeaseTime), len);
    DHCPv4Conf[intf].DHCPv4LeaseTime = ntohl(DHCPv4Conf[intf].DHCPv4LeaseTime);
    DEBUGMSG(DHCPv4_DEBUG,"dhcpReply <==");

    return 0;
}







void DHCPv4TimeOut(unsigned char intf)
{
    if (DHCPv4Conf[intf].DHCPv4RTTime < 16 )
        DHCPv4Conf[intf].DHCPv4RTTime<<=1;
    else
        DHCPv4Conf[intf].DHCPv4RTTime = 5*6;//5 min

    if (DHCPv4Conf[intf].DHCPv4RTTime == 0)
        DHCPv4Conf[intf].DHCPv4RTTime = 15;

    //If delay too short, it will send many dhcp discovery message
    OSTimeDly(DHCPv4Conf[intf].DHCPv4RTTime * OS_TICKS_PER_SEC);
}



#endif
#endif

#ifdef CONFIG_IPv6_ENABLED
void dhcpv6Solicit(RTSkt* s)
{
    TCPPCB	*pcb = s->pcb;
    unsigned char *p;
    DHCPv6Pkt* pktHdr;
    Identifierv6 *id;
    IA_NA *iana;
    ElapsedTime *et;
    OptionReq *opt;
    unsigned short pktLen = 0;
    PKT* pkt;

    pktLen = sizeof(DHCPv6Pkt) + sizeof(IA_NA) + sizeof(Identifierv6) + sizeof(MacAdr) + sizeof(ElapsedTime) + sizeof(OptionReq);
    //64 is option size
    pkt = allocUDPPkt_F(pktLen, IPv6);
    p = pkt->wp;
    //Multicast
    memcpy(pcb->dest_MAR, DHCPv6_All_Router_MAR, MAC_ADDR_LEN);

    //IP layer
    pcb->protocol = IP_PROTO_UDP;
    //Multicast
    memcpy(pcb->destIPv6Addr, All_DHCPv6_Agents_IP, IPv6_ADR_LEN);
    //DHCP pkt

    pktHdr = (DHCPv6Pkt*)p;
    pktHdr->opCode = DHCPv6_SOLICIT;

    pktHdr->TID[0] = rand() & 0xff;
    pktHdr->TID[1] = rand() & 0xff;
    pktHdr->TID[2] = rand() & 0xff;

    memcpy(&DHCPTID, pktHdr->TID, 3);

    //IA_NA
    p = p + sizeof(DHCPv6Pkt);
    iana = (IA_NA*)p;
    iana->OptHdr.opCode = htons(OPTION_IA_NA);
    iana->OptHdr.len = sizeof(IA_NA) - 4;
    iana->OptHdr.len = htons(iana->OptHdr.len);
    iana->IAID = rand();
    iana->T1 = 0;
    iana->T2 = 0;

    //Client Identifier
    p = p + sizeof(IA_NA);
    id = (Identifierv6*) p;
    id->OptHdr.opCode = htons(OPTION_CLIENTID);
    //Byte alignment issue
    id->OptHdr.len = sizeof(DUIDLLT) + sizeof(MacAdr);
    id->OptHdr.len = htons(id->OptHdr.len);

    //DUID
    //Ethernet
    id->DUID.opCode = htons(0x01);
    id->DUID.hwType = htons(0x01);
    id->DUID.time = time(0);

    p = p + sizeof(Identifierv6);
    ///memcpy(p, dpconf->srcMacAddr[intf],MAC_ADDR_LEN);
    GetMacAddr_F(p, s->pcb->intf);

    p = p + sizeof(MacAdr);
    et = (ElapsedTime*)p;
    et->opCode = htons(0x08);
    et->len = htons(0x02);
    et->time = 0;

    p = p + sizeof(ElapsedTime);

    opt = (OptionReq*)p;
    opt->opCode = htons(DHCPv6_OPTION_ORO);
    opt->len = htons(0x06);
    opt->op1 = htons(24);
    opt->op2 = htons(DHCPv6_OptDNS);
    opt->op3 = htons(17);

    DHCPv6Conf[intf].DHCPv6State = SolicitateState;

    rtSktSend_F(s, pkt, pktLen);
}

char dhcpv6Input(RTSkt* s, unsigned char *p, int len)
{
    DHCPv6Pkt* pktHdr = (DHCPv6Pkt*)p;
    DHCPv6OptHdr* OptHdr;
    DHCPv6OptHdr* ianaOptHdr;
    IA_NA* ianaOpt;
    IA_NA_ADDROPTIONS *ianaAddr;
    OptionReq *opt;
    ElapsedTime *et;
    unsigned short pktLen = 0;
    int ianaOptLen;
    PKT* pkt;
    unsigned char* wp;
    unsigned char mar[MAC_ADDR_LEN] = {0};

    switch (DHCPv6Conf[intf].DHCPv6State)
    {
    case SolicitateState:
        if (pktHdr->opCode != DHCPv6_Advertise)
            return -1;
        break;
    case AdvertiseState:
        if (pktHdr->opCode != DHCPv6_Reply)
            return -1;
        break;
    default:
        return -1;
    }

    if (memcmp(&DHCPTID, pktHdr->TID, 3) != 0)
        return -1;

    pktLen = sizeof(DHCPv6Pkt) + (sizeof(Identifierv6) + sizeof(MacAdr))*2 + sizeof(IA_NA) + + sizeof(IA_NA_ADDROPTIONS) + sizeof(OptionReq) + sizeof(ElapsedTime);
    //64 is option size
    pkt = allocUDPPkt_F(pktLen, IPv6);
    wp = pkt->wp;

    memcpy(wp, pktHdr, sizeof(DHCPv6Pkt));
    pktHdr = (DHCPv6Pkt*)wp;

    if (pktHdr->opCode == DHCPv6_Advertise)
        pktHdr->opCode = DHCP_REQUEST;

    wp += sizeof(DHCPv6Pkt);

    len -= sizeof(DHCPv6Pkt);
    p += sizeof(DHCPv6Pkt);
    OptHdr = (DHCPv6OptHdr*)(p);

    //Process Options
    while (len>0)
    {
        OptHdr->opCode = ntohs(OptHdr->opCode);
        //Use Link layer address plus time,
        //so do not check client ID and server ID
        if (OptHdr->opCode == OPTION_SERVERID)
        {
            OptHdr->opCode = htons(OptHdr->opCode);
            memcpy(wp, OptHdr, sizeof(Identifierv6) + sizeof(MacAdr));
            wp = wp + sizeof(Identifierv6) + sizeof(MacAdr);
        }

        if (OptHdr->opCode == OPTION_CLIENTID)
        {
            OptHdr->opCode = htons(OptHdr->opCode);
            memcpy(wp, OptHdr, sizeof(Identifierv6) + sizeof(MacAdr));
            wp = wp + sizeof(Identifierv6) + sizeof(MacAdr);
        }

        if (OptHdr->opCode == OPTION_IA_NA)
        {
            OptHdr->opCode = htons(OptHdr->opCode);
            memcpy(wp, OptHdr, sizeof(IA_NA));
            //Assign output Length, we only copy a OPTION_IAADDR
            ianaOpt = (IA_NA*) wp;
            //length do not include DHCPv6 header
            ianaOpt->OptHdr.len = htons(sizeof(IA_NA) + sizeof(IA_NA_ADDROPTIONS) - sizeof(DHCPv6OptHdr));
            wp = wp + sizeof(IA_NA);

            ianaOpt = (IA_NA*) OptHdr;
            ianaOptLen = ntohs(ianaOpt->OptHdr.len);
            ianaOptLen -= sizeof(IA_NA);
            //ianaOptHdr is a tmp DHCPv6OptHdr
            ianaOptHdr = (DHCPv6OptHdr*)((char*)OptHdr + sizeof(IA_NA));

            while (ianaOptLen > 0)
            {
                ianaOptHdr->opCode = ntohs(ianaOptHdr->opCode);
                //Copy first assign address
                if (ianaOptHdr->opCode == OPTION_IAADDR)
                {
                    ianaOptHdr->opCode = htons(ianaOptHdr->opCode);
                    memcpy(wp, ianaOptHdr, sizeof(IA_NA_ADDROPTIONS));
                    wp = wp + sizeof(IA_NA_ADDROPTIONS);
                    ianaAddr = (IA_NA_ADDROPTIONS*)ianaOptHdr;

                    if (pktHdr->opCode == DHCPv6_Reply)
                    {
                        resetMulticasAddress_F();
                        //Solicited multicast address
                        //all node
                        mar[0] = 0x33;
                        mar[1] = 0x33;
                        mar[2] = 0x00;
                        mar[3] = 0x00;
                        mar[4] = 0x00;
                        mar[5] = 0x01;
                        addMulticasAddress_F(mar);
                        setMulticastList_F();
                        setIPv6Address_F(ianaAddr->addr);
                        //dpconf->DHCPv6Enable = 0;
                        //DHCPv6Conf[intf].DHCPv6State = ReplyState;
                        //free(pkt);
                        //return 0;
                    }
                    break;
                }
                ianaOptLen = ianaOptLen - ntohs(ianaOptHdr->len) - sizeof(DHCPv6OptHdr);
                ianaOptHdr = (DHCPv6OptHdr*)((char*)ianaOptHdr + ntohs(ianaOptHdr->len) + sizeof(DHCPv6OptHdr));
            }
        }

        if (OptHdr->opCode == DHCPv6_OptDNS && ntohs(OptHdr->len) == IPv6_ADR_LEN)
        {
            memcpy(dpconf->IPv6DNS, OptHdr + 1, IPv6_ADR_LEN);
        }

        len = len - ntohs(OptHdr->len) - sizeof(DHCPv6OptHdr);
        p = p + ntohs(OptHdr->len) + sizeof(DHCPv6OptHdr);
        OptHdr = (DHCPv6OptHdr*)p;
    }

    if (pktHdr->opCode == DHCPv6_Reply)
    {
        DHCPv6Conf[intf].DHCPv6State = ReplyState;
        freePkt_F(pkt);
        return 0;
    }

    et = (ElapsedTime*)wp;
    et->opCode = htons(0x08);
    et->len = htons(0x02);
    et->time = 0;

    wp = wp + sizeof(ElapsedTime);

    opt = (OptionReq*)wp;
    opt->opCode = htons(DHCPv6_OPTION_ORO);
    opt->len = htons(0x06);
    opt->op1 = htons(24);
    opt->op2 = htons(23);
    opt->op3 = htons(17);
    rtSktSend_F(s, pkt, pktLen);
    DHCPv6Conf[intf].DHCPv6State = AdvertiseState;
    return 0;
}

void DHCPv6Task(void *data)
{
    RTSkt* s = rtSkt_F(IPv6, IP_PROTO_UDP);
    PKT* pkt;
    int status = 0;
    unsigned char err = 0;
    unsigned char mar[MAC_ADDR_LEN] = {0};

    if (dpconf->DHCPv6Enable)
    {
        memset(dpconf->IPv6Prefix, 0,IPv6_ADR_LEN);
        memset(dpconf->IPv6DNS, 0 ,IPv6_ADR_LEN);
        dpconf->IPv6PrefixLen = 0;
    }

    do
    {
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
        rtSktEnableOOBPortFilter_F(s);
#endif
        OSSemPend(SktSem, 0, &err);
        s->pcb->destPort = 547;
        s->pcb->localPort = 546;
        tcpPCBRegister_F(s->pcb);
        OSSemPost(SktSem);
DHCPv6_Restart:
        memset(&DHCPv6Conf[intf], 0, sizeof(DHCPv6Conf));
        DHCPv6Conf[intf].DHCPv6State = UnknownDHCPState;

        while (dpconf->DHCPv6Enable && DHCPv6Conf[intf].DHCPv6State != ReplyState && dpconf->IPv6Enable)
        {
            DEBUGMSG(DHCPv6_DEBUG,"DHCPv6 Start");
#if CONFIG_VERSION <= IC_VERSION_DP_RevF			
            REG32(IOREG_IOBASE+MAC_RxCR) &= ~0x80000000;
#endif

            resetMulticasAddress_F();
            //Solicited multicast address
            //all node
            mar[0] = 0x33;
            mar[1] = 0x33;
            mar[2] = 0x00;
            mar[3] = 0x00;
            mar[4] = 0x00;
            mar[5] = 0x01;
            addMulticasAddress_F(mar);
            setMulticastList_F();

            OSSemPend(SktSem, 0, &err);

            if (DHCPv6Conf[intf].DHCPv6State == UnknownDHCPState)
            {
                DHCPv6Conf[intf].DHCPv6NeighborAdver = 0;
                DHCPv6Conf[intf].DHCPv6CheckEUIAddr = 1;
                neighborSolicite(0);
                DHCPv6Conf[intf].DHCPv6State = NSolicitateState;
            }
            OSSemPost(SktSem);

            OSTimeDly(MAX_RTR_SOLICITATION_DELAY  * OS_TICKS_PER_SEC);

            if (DHCPv6Conf[intf].DHCPv6NeighborAdver == 1)
            {
                //Stop address auto configuration
                DHCPv6Conf[intf].DHCPv6State = UnknownDHCPState;
                break;
            }

            OSSemPend(SktSem, 0, &err);
            DHCPv6Conf[intf].DHCPv6RouterAdver = 0;
            routerSolicite();
            mar[0] = 0x33;
            mar[1] = 0x33;
            mar[2] = 0x00;
            mar[3] = 0x00;
            mar[4] = 0x00;
            mar[5] = 0x01;
            addMulticasAddress_F(mar);
            setMulticastList_F();

            DHCPv6Conf[intf].DHCPv6State = RSolicitateState;
            OSSemPost(SktSem);
            OSTimeDly(MAX_RTR_SOLICITATION_DELAY  * OS_TICKS_PER_SEC);
            DHCPv6Conf[intf].DHCPv6RTTime = 1;
            //DHCPv6 reply may be unicast or multicast, and current is temp ipv6 address
            //Default IPv6 filter will filter TCP icmp unicast/multicast and multicast UDP, NO unicast ipv6
            #if CONFIG_VERSION  >= IC_VERSION_EP_RevA
            EnableOOBDHCPv6PFRule();
            #endif

            do
            {
                do
                {
                    if (DHCPv6Conf[intf].DHCPv6RTTime == 16)
                        goto DHCPv6_Restart;
                    dhcpv6Solicit(s);
                    pkt = rtSktRx_F(s, DHCPv6Conf[intf].DHCPv6RTTime*OS_TICKS_PER_SEC, &status);
                    if (status == SKTDISCONNECT)
                    {
                        DEBUGMSG(CRITICAL_ERROR | DHCPv6_DEBUG, "DHCP Critical Error \n");
                    }
                    DHCPv6Conf[intf].DHCPv6RTTime++;
                }while (pkt == 0);
                status = dhcpv6Input(s, pkt->wp, pkt->len);
                freePkt_F(pkt);
            }while (status != 0);

            DHCPv6Conf[intf].DHCPv6RTTime = 1;
            do
            {
                do
                {
                    if (DHCPv6Conf[intf].DHCPv6RTTime == 16)
                        goto DHCPv6_Restart;

                    pkt = rtSktRx_F(s, DHCPv6Conf[intf].DHCPv6RTTime*OS_TICKS_PER_SEC, &status);
                    if (status == SKTDISCONNECT)
                    {
                        DEBUGMSG(CRITICAL_ERROR | DHCPv6_DEBUG ,"DHCP Critical Error \n");
                    }
                }while (pkt == 0);
                status = dhcpv6Input(s, pkt->wp, pkt->len);
                freePkt_F(pkt);
            }while (status != 0);
        }
        REG32(IOREG_IOBASE+MAC_RxCR) |= 0x80000000;
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
        DisableOOBDHCPv6PFRule();
#endif
        OSSemPend(SktSem, 0, &err);
        tcpPCBUnRegister_F(s->pcb);
        OSSemPost(SktSem);
        OSTaskSuspend(OS_PRIO_SELF);
    }while (1);
}
#endif
