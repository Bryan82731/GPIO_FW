#include "mDNS.h"
#include "rtskt.h"
#include "DNS.h"
#ifdef  CONFIG_mDNS_OFFLOAD_ENABLED
struct _mDNSOffloadCmd mDNSOffloadCmd = {0};

extern DPCONF *dpconf;
extern _IPInfo IPInfo[];

void dumpCompressmDNSRR(unsigned char* dn, int offset)
{
	unsigned char* tmpPtr = mDNSOffloadCmd.rrRecords;
	unsigned short rrSize = 0;
	//offset include mDNS header
	offset = offset - 12;
	tmpPtr = tmpPtr + offset;

	while(tmpPtr[0])
	{
		if((tmpPtr[0]&0xC0) == 0xC0)
		{
			//Name compression
			//Name compression include 12 byte offet(mDNS header)
			dumpCompressmDNSRR(dn + rrSize, tmpPtr[1]);
			rrSize = rrSize + 2;
			tmpPtr = tmpPtr + 2;		
			break;
		}

		memcpy(dn + rrSize, tmpPtr, tmpPtr[0] + 1);

		rrSize++;
		rrSize = rrSize + tmpPtr[0];
		tmpPtr = tmpPtr + tmpPtr[0] + 1;				
	}
}

void dumpmDNSRR()
{
	unsigned int i = 0;
	unsigned char* dn= (unsigned char*) malloc(MAX_mDNS_DOMAIN_NAME_LENGTH);
	unsigned char* tmpPtr = mDNSOffloadCmd.rrRecords;	
	unsigned short rrSize = 0;
	unsigned short rdLen = 0;
	unsigned char compressionName = 0;

	while(i < mDNSOffloadCmd.numRRRecords)
	{
		memset(dn, 0, MAX_mDNS_DOMAIN_NAME_LENGTH);

		//Get RR size
		rrSize = 0;
		
		//Get RR Domain name length
		compressionName = 0;

		while(tmpPtr[0])
		{
			if((tmpPtr[0]&0xC0) == 0xC0)
			{
				//Name compression
				//Name compression include 12 byte offet(mDNS header)
				dumpCompressmDNSRR(dn + rrSize, ((tmpPtr[0]&0x3f)<<8) + tmpPtr[1]);
				rrSize = rrSize + 2;
				tmpPtr = tmpPtr + 2;
				compressionName = 1;				
				break;
			}

			memcpy(dn + rrSize, tmpPtr, tmpPtr[0] + 1);

			rrSize++;
			rrSize = rrSize + tmpPtr[0];
			tmpPtr = tmpPtr + tmpPtr[0] + 1;				
		}

		if(compressionName == 0)
		{
			rrSize++;//termination char
			tmpPtr++;
		}			
		rrSize = rrSize + 10;//domain type(2) + domain class(2) + TTL(4) + resource data length(2)  
		
		tmpPtr = tmpPtr + 8;
		memcpy(&rdLen, tmpPtr, sizeof(rdLen));
		rdLen = ntohs(rdLen);
		tmpPtr = tmpPtr + 2;

		//Get RR data size
		rrSize = rrSize + rdLen;
		tmpPtr = tmpPtr +  rdLen;
		i++;
	}
	free(dn);
}

unsigned short getDNSCompressName(DNSHdr* hdr, unsigned char* rrPtr, unsigned char* dn, int offset)
{
	unsigned char* tmpPtr = hdr;
	unsigned short dnSize = 0;
	//mDNSResponder offload mDNS data do not include mDNS header
	if(hdr == 0)
	{
		tmpPtr = mDNSOffloadCmd.rrRecords;
		offset = offset - 12;
	}

	tmpPtr = tmpPtr + offset;

	while(tmpPtr[0])
	{
		if((tmpPtr[0]&0xC0) == 0xC0)
		{
			//Name compression
			//Name compression include 12 byte offet(mDNS header)
			dnSize = dnSize + getDNSCompressName(hdr, tmpPtr, dn + dnSize, ((tmpPtr[0]&0x3f)<<8) + tmpPtr[1]);
			tmpPtr = tmpPtr + 2;		
			break;
		}

		memcpy(dn + dnSize, tmpPtr, tmpPtr[0] + 1);

		dnSize++;
		dnSize = dnSize + tmpPtr[0];
		tmpPtr = tmpPtr + tmpPtr[0] + 1;				
	}
	return dnSize;
}

void getDNSDomainName(DNSHdr* hdr, unsigned char* rrPtr, unsigned char* dn, unsigned short* dnLen, unsigned char** rrDataPtr)
{
	unsigned short dnSize = 0;
	unsigned char compressionName = 0;

	//Get RR Domain name length
	compressionName = 0;

	while(rrPtr[0])
	{
		if((rrPtr[0]&0xC0) == 0xC0)
		{
			//Name compression
			//Name compression include 12 byte offet(mDNS header)
			dnSize = dnSize + getDNSCompressName(hdr, rrPtr, dn + dnSize,((rrPtr[0]&0x3f)<<8) +  rrPtr[1]);
			rrPtr = rrPtr + 2;
			compressionName = 1;
			break;
		}

		memcpy(dn + dnSize, rrPtr, rrPtr[0] + 1);

		dnSize++;
		dnSize = dnSize + rrPtr[0];
		rrPtr = rrPtr + rrPtr[0] + 1;
	}


	if(compressionName == 0)
	{
		//dnSize++;//termination char
		rrPtr++;
	}

	*dnLen = dnSize;
	*rrDataPtr = rrPtr;
}

void setmDNSOffloadAddr(unsigned char intf)
{
	unsigned char* offloadCMDDn = (unsigned char*) malloc(MAX_mDNS_DOMAIN_NAME_LENGTH);	
	unsigned char* rrPtr = mDNSOffloadCmd.rrRecords;
	unsigned char* rrDataPtr;
	unsigned int i = 0;
	unsigned short offloadCMDDnLen;	
	unsigned short rrType = 0;
	unsigned short rdLen = 0;
	unsigned int ip = 0;
	
	for( i = 0; i < mDNSOffloadCmd.numRRRecords; i++)
	{
		memset(offloadCMDDn, 0, MAX_mDNS_DOMAIN_NAME_LENGTH);
		getDNSDomainName(0, rrPtr, offloadCMDDn , &offloadCMDDnLen, &rrDataPtr);
		rrPtr = rrDataPtr;

		memcpy(&rrType, rrPtr, sizeof(rrType));
		rrType = ntohs(rrType);

		rrPtr = rrPtr + 8;
		memcpy(&rdLen, rrPtr, sizeof(rdLen));
		rdLen = ntohs(rdLen);		
		rrPtr = rrPtr + 2;
		
		if((rrType == mDNSA) && (rdLen == IPv4_ADR_LEN))
		{
			ip = getIPAddress_F(intf);
			IPInfo[intf].tmpIP = ntohl(ip);
			memcpy(&ip, rrPtr, IPv4_ADR_LEN);
			ip = ntohl(ip);
			setIPAddress_F(ip, intf);
			break;
		}
		
		rrPtr = rrPtr +  rdLen;
	}

	free(offloadCMDDn);

}

char findMatchRR(unsigned char* dn, unsigned short dnLen, unsigned short queryType, unsigned short queryClass, unsigned char** rrMatchDataPtr, unsigned int* rrTTL)
{
	unsigned char* offloadCMDDn = (unsigned char*) malloc(MAX_mDNS_DOMAIN_NAME_LENGTH);	
	unsigned char* rrPtr = mDNSOffloadCmd.rrRecords;
	unsigned char* rrDataPtr;
	unsigned int i = 0;
	unsigned short offloadCMDDnLen;	
	unsigned short rrType = 0;
	unsigned short rrClass = 0;
	unsigned short rdLen = 0;
	unsigned char find = 0; 
	
	for( i = 0; i < mDNSOffloadCmd.numRRRecords; i++)
	{
		memset(offloadCMDDn, 0, MAX_mDNS_DOMAIN_NAME_LENGTH);
		getDNSDomainName(0, rrPtr, offloadCMDDn , &offloadCMDDnLen, &rrDataPtr);
		rrPtr = rrDataPtr;
		
		if(offloadCMDDnLen == dnLen)
		{
			if(memcmp(dn, offloadCMDDn, dnLen) == 0)
			{
				memcpy(&rrType, rrPtr, sizeof(rrType));
				//rrType = ntohs(rrType);

				memcpy(&rrClass, rrPtr + sizeof(rrType), sizeof(rrClass));
				rrClass = ntohs(rrClass);
				queryClass = ntohs(queryClass);
#if 0
				//TTL test
				*rrTTL = htonl(20);
				memcpy(rrPtr + sizeof(rrType) + sizeof(rrClass), rrTTL, sizeof(*rrTTL));
				*rrTTL = 20;
#else
				memcpy(rrTTL, rrPtr + sizeof(rrType) + sizeof(rrClass), sizeof(*rrTTL));
				*rrTTL = ntohl(*rrTTL);
#endif
				
				if((rrType == queryType) && ((rrClass & 0x7fff) == (queryClass &0x7fff) ))
				{
					*rrMatchDataPtr = rrPtr;
					if(queryClass & 0x8000)
						find = UNICAST_QUERY;
					else
						find = MULTICAST_QUERY;
					break;
				}
			}
		}	

		rrPtr = rrPtr + 8;
		memcpy(&rdLen, rrPtr, sizeof(rdLen));
		rdLen = ntohs(rdLen);
		rrPtr = rrPtr + 2;
		rrPtr = rrPtr +  rdLen;
	}

	free(offloadCMDDn);
	return find;
}

char chkAnsTTL(DNSHdr* hdr, unsigned char* ansRRPtr, unsigned short ans, unsigned char* dn, unsigned short dnLen, unsigned short queryType, unsigned short queryClass, unsigned char* rrMatchDataPtr, unsigned int rrTTL) 
{
	unsigned char* ansDn = (unsigned char*) malloc(MAX_mDNS_DOMAIN_NAME_LENGTH);
	unsigned char* tmpDn = (unsigned char*) malloc(MAX_mDNS_DOMAIN_NAME_LENGTH);
	unsigned short ansDnLen = 0;
	unsigned short tmpDnLen = 0;
	unsigned char* rrDataPtr;
	unsigned char* tmpDataPtr;
	unsigned short ansType = 0;
	unsigned short ansClass = 0;
	unsigned short rdLen = 0;
	unsigned int ansTTL = 0;
	unsigned int i = 0;


	char result = MDNS_UPDATE;

	queryClass = ntohs(queryClass) & 0x7fff;

	for( i = 0; i < ans; i++)
	{
		memset(ansDn, 0, MAX_mDNS_DOMAIN_NAME_LENGTH);
		getDNSDomainName(hdr, ansRRPtr, ansDn , &ansDnLen, &rrDataPtr);

		ansRRPtr = rrDataPtr;

		if((ansDnLen == dnLen) && (memcmp(ansDn, dn, dnLen) == 0))
		{
			memcpy(&ansType, ansRRPtr, sizeof(ansType));				
			memcpy(&ansClass, ansRRPtr + sizeof(ansType) , sizeof(ansClass));
			ansClass = ntohs(ansClass) & 0x7fff;			
			
			if((ansType == queryType) && (ansClass == queryClass))
			{
				memcpy(&ansTTL, ansRRPtr + sizeof(ansType) + sizeof(ansClass), sizeof(ansTTL));
				ansTTL = ntohl(ansTTL);
				
				if(ntohs(ansType) == DNSPTR)
				{
					getDNSDomainName(hdr, ansRRPtr + 10, ansDn , &ansDnLen, &tmpDataPtr);
					getDNSDomainName(0, rrMatchDataPtr + 10, tmpDn , &tmpDnLen, &tmpDataPtr);
					if(ansDnLen == tmpDnLen)
					{
						if(memcmp(ansDn, tmpDn, tmpDnLen) == 0)
						{
							if((ansTTL*2 > rrTTL))
							{
								result = 0;
								break;
							}
						}
					}
				}
				else
				{
					if((ansTTL*2 > rrTTL))
					{
						result = 0;
						break;
					}
				}
			}			
		}

		ansRRPtr = ansRRPtr + 8;
		memcpy(&rdLen, ansRRPtr, sizeof(rdLen));
		rdLen = ntohs(rdLen);
		ansRRPtr = ansRRPtr + 2;
		ansRRPtr = ansRRPtr +  rdLen;
	}
	free(ansDn);

	return result;
}


int mDNSRx(RTSkt* s, PKT* pkt)
{
	DNSHdr* hdr = (DNSHdr*)pkt->wp;
	//DNSHdr* txHdr;
	unsigned char* rrPtr = (unsigned char *)pkt->wp +  sizeof(DNSHdr);
	unsigned char* rrDataPtr;
	unsigned char* rrMatchDataPtr;
	unsigned char* ansRRPtr = 0;
	unsigned int i = 0;
	unsigned short id = hdr->id;
	unsigned short dnLen = 0;
	unsigned char* dn = (unsigned char*) malloc(MAX_mDNS_DOMAIN_NAME_LENGTH);
	unsigned short rdLen = 0;
	unsigned short queryType = 0;
	unsigned short queryClass = 0;
	unsigned int   rrTTL = 0;	
	unsigned char  find = 0;
	
	PKT* txPkt;
	UDPAddrInfo ai;

	hdr->questions = ntohs(hdr->questions);
	hdr->ans = ntohs(hdr->ans);

	if(hdr->ans)
	{
		for( i = 0; i < hdr->questions; i++)
		{			
			getDNSDomainName(hdr, rrPtr, dn , &dnLen, &rrDataPtr);
			rrPtr = rrDataPtr;
			rrPtr = rrPtr + sizeof(queryType) + sizeof(queryClass);
		}
		ansRRPtr = rrPtr;
		rrPtr = (unsigned char *)pkt->wp +  sizeof(DNSHdr);
	}


	for( i = 0; i < hdr->questions; i++)
	{
		memset(dn, 0, MAX_mDNS_DOMAIN_NAME_LENGTH);
		getDNSDomainName(hdr, rrPtr, dn , &dnLen, &rrDataPtr);
		rrPtr = rrDataPtr;
		
		memcpy(&queryType, rrPtr, sizeof(queryType));
		//queryType = ntohs(queryType);
		rrPtr = rrPtr + sizeof(queryType);

		memcpy(&queryClass, rrPtr, sizeof(queryClass));
		//queryClass = ntohs(queryClass);
		rrPtr = rrPtr + sizeof(queryClass);

		find = findMatchRR(dn, dnLen, queryType, queryClass, &rrMatchDataPtr, &rrTTL);
#if 1
		if(find && hdr->ans)
		{
			if(chkAnsTTL(hdr, ansRRPtr, hdr->ans, dn, dnLen, queryType, queryClass, rrMatchDataPtr, rrTTL) != MDNS_UPDATE)
				find = 0;
		}				
#endif		
		if(find)
		{
			break;
		}
	}

	free(dn);

	if(find == MULTICAST_QUERY)
	{
		ai.destIPAddr.addr = htonl(0xe00000fb);
		ai.destPort = 5353;
    		
		txPkt = (PKT*)allocUDPPkt(mDNSOffloadCmd.rrBufferSize + sizeof(DNSHdr), IPv4);
		hdr = (DNSHdr*)txPkt->wp;
		memset(hdr, 0, sizeof(DNSHdr));
		hdr->id = id;
		hdr->QR = 1;
		hdr->AA = 1;		
		hdr->ans = htons(mDNSOffloadCmd.numRRRecords);
		memcpy(hdr + 1, mDNSOffloadCmd.rrRecords, mDNSOffloadCmd.rrBufferSize);
		rtSktUDPSendTo_F(s, txPkt,&ai);
	}
	else if(find == UNICAST_QUERY)
	{
		getUDPAddressInfo_F(pkt ,&ai);
		
		if ((ai.destIPAddr.addr & dpconf->SubnetMask[pkt->intf].addr) != dpconf->MatchSubnetMaskValue[pkt->intf])
			return 0;

		txPkt = (PKT*)allocUDPPkt(mDNSOffloadCmd.rrBufferSize + sizeof(DNSHdr), IPv4);
		hdr = (DNSHdr*)txPkt->wp;
		memset(hdr, 0, sizeof(DNSHdr));
		hdr->id = id;
		hdr->QR = 1;
		hdr->AA = 1;		
		hdr->ans = htons(mDNSOffloadCmd.numRRRecords);
		memcpy(hdr + 1, mDNSOffloadCmd.rrRecords, mDNSOffloadCmd.rrBufferSize);
		//udpSend(pcb, txPkt, txPkt->len);
		rtSktUDPSendTo_F(s, txPkt,&ai);
	}

	return 0;
}


int mDNSRxV6(void *pData, int len, struct tcp_pcb *pcb)
{
	return 0;
}

void mDNSOffloadSrv(void *data)
{
    unsigned char err;
    unsigned char mar[MAC_ADDR_LEN];
	RTSkt* s;
	RTSkt* s1;
	PKT* pkt;
	int status = 0;

	
	s = rtSkt_F(IPv4, IP_PROTO_UDP);
    rtSktEnableOOBPortFilter_F(s);
//	s->pcb->apRx = mDNSRx;
	rtSktUDPBind_F(s, 5353);
	
	if(dpconf->IPv6Enable)
	{
		s1 = rtSkt_F(IPv6, IP_PROTO_UDP);
		rtSktEnableOOBPortFilter_F(s1);
//		s1->pcb->apRx = mDNSRxV6;
		rtSktUDPBind_F(s1, 5353);	
	}
	
	resetMulticasAddress();
	mar[0] = 0x01;
	mar[1] = 0x00;
	mar[2] = 0x5e;
	mar[3] = 0x00;
	mar[4] = 0x00;
	mar[5] = 0xfb;
	addMulticasAddress(mar);
	setMulticastList_F();

    while (1)
    {
		pkt = rtSktRx_F(s, 0, &status);

		if (status == SKTDISCONNECT ||status == OS_ERR_TIMEOUT)
		{
			printf("mDNS Rx Error\n");
			break;
		}
		if(pkt)
		{
			mDNSRx(s, pkt);
			freePkt(pkt);
		}
    }

    for (;;)
    {
        OSTimeDlyHMSM(1, 1, 1, 0);              // delay task for 1 second
    }
}

#endif
