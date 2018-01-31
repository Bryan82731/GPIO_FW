#ifndef __mDNS_Offload_H__
#define __mDNS_Offload_H__

#define MAX_mDNS_DOMAIN_NAME_LENGTH 256
#define UNICAST_QUERY 1
#define MULTICAST_QUERY 2
#define MDNS_UPDATE 1
struct _mDNSOffloadCmd
{
	unsigned int  rrBufferSize;           // number of bytes of RR records
	unsigned int  numUDPPorts;            // number of SRV UDP ports
	unsigned int  numTCPPorts;            // number of SRV TCP ports 
	unsigned int  numRRRecords;           // number of RR records
	unsigned int  curRROffset;
	unsigned char*		rrRecords;              // address of array of pointers to the rr records
	unsigned short*		udpPorts;               // address of udp port list (SRV)
	unsigned short*		tcpPorts;               // address of tcp port list (SRV)
	unsigned char	linkChangeCount;
};

typedef struct _PortList
{
	unsigned int num;
	unsigned short ports[0];
}PortList;

typedef struct _RRList
{
	unsigned short numRRRecords;
	unsigned short firstFrag:1, lastFrag:1, fragLen:10, res:4;
}RRList;

void mDNSOffloadSrv(void *data);
void dumpmDNSRR();
#endif

