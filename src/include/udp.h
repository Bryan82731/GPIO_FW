#ifndef _UDP_H_
#define _UDP_H

#include <rlx/rlx_types.h>
#include "ip.h"
#include "lib.h"
//#include "arp.h"
//#include "smbus.h"


#define IPHdrLen                 0x05
#define TypeOfService            0x04
#define TimeToLive               0x40
#define Identifier             0x0000
#define FlagmentOffset         0x0000
#define Hdr_ChkSum             0x0000
#define UDP_ChkSum             0x0000

typedef struct
{
    INT8U   srcIPAddr[4];
    INT8U   destIPAddr[4];
    INT8U   srcEthAddr[MAC_ADDR_LEN];
    INT8U   destEthAddr[MAC_ADDR_LEN];
    INT16U  srcPort;
    INT16U  destPort;
}AddrInfo;

typedef struct _UDPHdr {
    unsigned short srcPort;
    unsigned short desPort;
    unsigned short length;
    unsigned short chkSum;
}UDPHdr;

typedef struct
{
    EthHdr ethHdr;
    IPHdr  ipHdr;
    UDPHdr udpHdr;
}UDP;

void udpSend(void *pcb,void* data, unsigned short len);
void udpInput(PKT *pkt);
#endif
