#ifndef _INET_TYPES_H
#define _INET_TYPES_H

#include <rlx/rlx_types.h>

#define MAC_ADDR_LEN        6

#define IPv4_ADR_LEN    4
#define IPv6_ADR_LEN    16

typedef struct tcp_pcb *ptcp_pcb;
typedef struct _PKT *PPKT;
typedef struct _RTSkt *PRTSkt;
typedef struct _UDPAddrInfo *PUDPAddrInfo;
typedef struct _ARPPkt *PARPPkt;

enum DHCPv4States
{
    InitState = 0x00,
    SelectState = 0x01,
    RequestState = 0x02,
    BoundState = 0x03
};

typedef struct _IPAddress
{
    INT32U addr;
}IPAddress;

#endif
