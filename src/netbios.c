#include <string.h>
#include <stdlib.h>
#include "tcp.h"
#include "rtskt.h"
#include "dns.h"
#include "udp.h"


extern DPCONF *dpconf;

#define HOST_NAME_MAX_LEN 16
ROM_EXTERN unsigned char HostName[HOST_NAME_MAX_LEN];

int netBIOSRx(void *pData, int len, struct tcp_pcb *pcb);

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION < IC_VERSION_DP_RevF)
void setHostName(char* str)
{
    int i = 0;
    unsigned char c;

    memset(HostName, 0x20, sizeof(HostName));

    if (str!=0)
    {

        for (i = 0; i < strlen(str); i++)
        {
            HostName[i] = toupper(str[i]);
        }

        for (i = 0 ; i < 4; i++)
#if(CONFIG_VERSION <= IC_VERSION_DP_RevF)
            REG32(IOREG_IOBASE + IO_HOSTNAME + 4*i) = *(INT32U *)&dpconf->HostName[4*i];
#elif (CONFIG_VERSION > IC_VERSION_DP_RevF) 
            REG32(MAC_BASE_ADDR + MAC_HOSTNAME + 4*i) = *(INT32U *)&dpconf->HostName[4*i];
#endif

    }
    else
    {

        memset(dpconf->HostName, 0x20, sizeof(dpconf->HostName));
        for (i = 0; i < 16; i++)
        {
#if(CONFIG_VERSION <= IC_VERSION_DP_RevF)
            c = REG8(IOREG_IOBASE + IO_HOSTNAME + i);
#elif (CONFIG_VERSION > IC_VERSION_DP_RevF)
            c = REG8(MAC_BASE_ADDR + MAC_HOSTNAME + i);
#endif
            if (c != 0x20 && c != 0)
                dpconf->HostName[i] = toupper(c);
            else
            {
                dpconf->HostName[i] = 0;
            }

            if (c == 0)
                break;
            HostName[i] = toupper(c);
        }
        setdirty(DPCONFIGTBL);
    }


}
#endif

RTSkt* enableNetBIOSPtl()
{
    unsigned char err = 0;
    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_UDP);


#if CONFIG_VERSION >= 6
    rtSktEnableOOBPortFilter_F(s);
#else
    bsp_bits_set(IO_CONFIG2, 1, BIT_AAB, 1);
#endif

    s->pcb->apRx = netBIOSRx;

    rtSktUDPBind_F(s, 137);
    if (s->rxSem)
    {
        OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
    }
    return s;
}

int netBIOSRx(void *pData, int rxLen, struct tcp_pcb *pcb)
{
    PKT* txPkt;
    int i = 0;
    DNSHdr* dhr;
    unsigned char* ptr;//,*ch;
    NetBIOSResponse* NBRHdr;
	NetBIOSNodeStatusResponse* nodeStatus;
    unsigned char name[16];
    int IP = getIPAddress_F(pcb->intf);
	unsigned short type = 0;

    dhr = (DNSHdr*)(pData);
    dhr->questions = htons(dhr->questions);

    //Only support first question
    if (dhr->questions>=1)
    {
        ptr = (unsigned char*)pData + sizeof(DNSHdr);

        ptr++;

        memset(name, 0, HOST_NAME_MAX_LEN);

        for (i = 0; i < HOST_NAME_MAX_LEN; i++)
        {
            if (ptr[2*i] < 0x41 || ptr[2*i + 1] < 0x41)
                break;
            name[i] = (ptr[2*i] - 0x41)*16+ (ptr[2*i+1] - 0x41);
        }
		
		memcpy(&type, ptr + HOST_NAME_MAX_LEN*2 + 1, sizeof(short));
		type = ntohs(type);

		if(type == DNSNB)
		{	
			//  ||
			// (strncmp((const char*)name, (const char*)HostNmae, HOST_NAME_MAX_LEN-1) == 0 && name[HOST_NAME_MAX_LEN-1] == 0 && HostNmae[HOST_NAME_MAX_LEN-1] == 0x20)
			if (strncmp((const char*)name, (const char*)HostName, HOST_NAME_MAX_LEN) == 0 ||
					(strncmp((const char*)name, (const char*)HostName, HOST_NAME_MAX_LEN-1) == 0 && name[HOST_NAME_MAX_LEN-1] == 0 && HostName[HOST_NAME_MAX_LEN-1] == 0x20))
			{
				txPkt = (PKT*)allocUDPPkt_F(sizeof(NetBIOSResponse), IPv4);
				NBRHdr = (NetBIOSResponse*) txPkt->wp;
				memset(NBRHdr, 0, sizeof(NetBIOSResponse));
				NBRHdr->dhr.id = dhr->id;
				NBRHdr->dhr.QR = 1;
				NBRHdr->dhr.AA = 1;
				NBRHdr->dhr.RD = 1;
				NBRHdr->dhr.ans = htons(1);
				NBRHdr->nameLen = 0x20;
				memcpy(NBRHdr->name, ptr, HOST_NAME_MAX_LEN*2);
				NBRHdr->nameTail = 0;
				NBRHdr->NB = htons(0x0020);
				NBRHdr->InTernet = htons(0x0001);
				NBRHdr->TTL2 = htons(0x100);//Use DHCP lease time?
				NBRHdr->len = htons(0x06);//only one answer flag + IP size
				NBRHdr->flag = htons(0x6000);
				memcpy(NBRHdr->ip, &IP, sizeof(IP));
				udpSend_F(pcb, txPkt, txPkt->len);
			}
		}
		
		if(type == DNSNBSTAT)
		{
			//  || 
			// (strncmp((const char*)name, (const char*)HostNmae, HOST_NAME_MAX_LEN-1) == 0 && name[HOST_NAME_MAX_LEN-1] == 0 && HostNmae[HOST_NAME_MAX_LEN-1] == 0x20)
					if ((name[0] =='*')|| strncmp((const char*)name, (const char*)HostName, HOST_NAME_MAX_LEN) == 0 ||
							(strncmp((const char*)name, (const char*)HostName, HOST_NAME_MAX_LEN-1) == 0 && name[HOST_NAME_MAX_LEN-1] == 0 && HostName[HOST_NAME_MAX_LEN-1] == 0x20))
			{
				txPkt = (PKT*)allocUDPPkt_F(sizeof(NetBIOSNodeStatusResponse), IPv4);
				nodeStatus = (NetBIOSNodeStatusResponse*) txPkt->wp;
				memset(nodeStatus, 0, sizeof(NetBIOSNodeStatusResponse));
				nodeStatus->dhr.id = dhr->id;
				nodeStatus->dhr.QR = 1;
				nodeStatus->dhr.AA = 1;
				nodeStatus->dhr.ans = htons(1);
				nodeStatus->nameLen = 0x20;
				memcpy(nodeStatus->name, ptr, HOST_NAME_MAX_LEN*2);
				nodeStatus->nameTail = 0;
				nodeStatus->NB = htons(DNSNBSTAT);
				nodeStatus->InTernet = htons(0x0001);
				nodeStatus->numNames = 1;
				nodeStatus->rdLen = htons(63);//Only one name, used fixed length
				memcpy(nodeStatus->nodeName, HostName, HOST_NAME_MAX_LEN);
				memcpy(nodeStatus->uintID, pcb->dest_MAR, MAC_ADDR_LEN);
				udpSend_F(pcb, txPkt, txPkt->len);				
			}
		}

    }
    return 0;

}

void disableNetBIOSPtl(RTSkt* s)
{
    tcpPCBUnRegister_F(s->pcb);
    free(s->pcb);
    free(s);
}

/*
RFC:4795
RFC:2308
*/
#if CONFIG_LLMNR_ENABLED
int LLMNRRx(void *pData, int rxLen, struct tcp_pcb *pcb)
{	
	PKT* txPkt;
	unsigned char* ptr;
	unsigned short qType;
	unsigned short ts;
	unsigned int ti;
	LLMNRHdr* LHdr;
	LLMNRHdr* rxLHdr;
	char nLen = 0;
	
	rxLHdr = (LLMNRHdr*)(pData);
	rxLHdr->dhr.questions = htons(rxLHdr->dhr.questions);
	
	
	//Only support first question
	if(rxLHdr->dhr.questions>=1)
	{
		ptr = (unsigned char*)pData + sizeof(LLMNRHdr);

		if((ptr[0] == strlen(dpconf->HostName)) && (memcmp(ptr+1, dpconf->HostName, ptr[0]) == 0))
		{
			nLen = ptr[0];
			ptr += 1; //len field
			ptr += nLen; //Name len
			ptr += 1; //'\0'
			memcpy(&qType, ptr, 2);
			qType = ntohs(qType);
			//Query Type = 1 => IPv4
			//Quert Type = 28 => IPv6

			//Query Type = 1 => This we response no data(RFC2308, No Data, Type3 )
			if(qType == 1)
				txPkt = (PKT*)allocUDPPkt(sizeof(LLMNRHdr) + nLen + 2 + 2 + 2, IPv6);
			else if(qType == 28)
				txPkt = (PKT*)allocUDPPkt(sizeof(LLMNRHdr) + (nLen + 2 + 2 + 2)*2
				+ 4 + 2 + 16, IPv6);
			else
				return 0;
			
			memset(txPkt->wp, 0, txPkt->len);
			LHdr = (LLMNRHdr*) txPkt->wp;
			LHdr->dhr.id = rxLHdr->dhr.id;
			LHdr->dhr.QR = 1;

			LHdr->dhr.questions = htons(1);	

			if(qType == 28)
			{
				LHdr->dhr.ans = htons(1);
			}
			
			ptr = (unsigned char*)txPkt->wp + sizeof(LLMNRHdr);

			//Question record			
			ptr[0] = nLen;
			ptr++;
			memcpy(ptr, dpconf->HostName, nLen);
			ptr += nLen;
			ptr[0] = 0;//pad '\0'
			ptr++;
			
			//Resource type				
			ts = htons(28);
			memcpy(ptr, &ts, 2);
			ptr+=2;
			//Resource class
			ts = htons(1);
			memcpy(ptr, &ts, 2);
			ptr+=2;

			if(qType == 28)
			{
				//Ans record
				ptr[0] = nLen;
				ptr++;
				memcpy(ptr, dpconf->HostName, nLen);
				ptr += nLen;

				//Resource type
				ptr++;
				ts = htons(28);
				memcpy(ptr, &ts, 2);
				ptr+=2;
				//Resource class
				ts = htons(1);
				memcpy(ptr, &ts, 2);
				ptr+=2;
				//TTL
				ti = htonl(30);
				memcpy(ptr, &ti, 4);
				ptr+=4;
				//Resource Length
				ts = htons(16);
				memcpy(ptr, &ts, 2);
				ptr+=2;
				//IP				
				memcpy(ptr,getIPv6Address(pcb->intf), 16);
			}
			udpSend_F(pcb, txPkt, txPkt->len);
						
		}		
	}
	return 0;
}

RTSkt* enableLLMNRPtl()
{
	unsigned char err = 0;
	RTSkt* s = rtSkt_F(IPv6, IP_PROTO_UDP);	

	s->pcb->apRx = LLMNRRx;

#if CONFIG_VERSION >= 6
    rtSktEnableOOBPortFilter_F(s);
#endif    
	//Listen on 5355, UDP for multicast, TCP for unicast
	rtSktUDPBind_F(s, 5355);
	if(s->rxSem)
	{
		OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
	}
	return s;
}

void disableLLMNRPtl(RTSkt* s)
{
	tcpPCBUnRegister_F(s->pcb);
	free(s->pcb);
	free(s);
}
#endif