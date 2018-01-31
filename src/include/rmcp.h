#ifndef __RMCP_H__
#define __RMCP_H__

#include <rlx/rlx_types.h>
#include <sys/inet_types.h>

#define WindowSIZE             0x20
//RMCP Header
#define RMCP_V1_PORT             0x26f
#define RMCP_V2_PORT             0x298 
#define RMCP_HdrVersion          0x06
#define RMCP_HdrRsvd             0x00
#define RMCP_ClassofMessage      0x06
#define RMCP_ClassofACK          0x86


//RMCP Data
#define RMCP_DatRsvd      0x00
#define Payload_Type_AA   0x01
#define Payload_Type_IA   0x02

//role
#define ASF_Administrator 0x01
#define ASF_Operator      0x00

#define SIK_KEY_SIZE      0x14
#define SIK_KEY_SIZE_USE  0x0C

#define RMCP_OFFSET       (ETH_HDR_SIZE + IP_HLEN + UDP_HDR_SIZE)

//dash profile
#define DASH_PWR_On    "2"
#define DASH_Sleep     "4"
#define DASH_Reset     "5"
#define DASH_Hibernate "7"
#define DASH_Shutdown  "8"
#define DASH_PWR_CR    "9"

//in-band remote control
#define IB_Reset     0x01
#define IB_Sleep     0x02
#define IB_Hibernate 0x03
#define IB_Shutdown  0x04

//RMCP Message Type
#define RMCP_Reset   0x10
#define RMCP_PWR_On  0x11
#define RMCP_PWR_Off 0x12
#define RMCP_PWR_CR  0x13
#define RMCP_HIBER   0x14
#define RMCP_STANDBY 0x15

#define RMCP_TEST    0x20
#define RMCP_PONG    0x40
#define RMCP_CAP_RES 0x41
#define RMCP_SST_RES 0x42
#define RMCP_OPS_RES 0x43
#define RMCP_CLS_RES 0x44
#define RMCP_PING    0x80
#define RMCP_CAP_REQ 0x81
#define RMCP_SST_REQ 0x82
#define RMCP_OPS_REQ 0x83
#define RMCP_CLS_REQ 0x84
#define RMCP_RAKP_M1 0xC0
#define RMCP_RAKP_M2 0xC1
#define RMCP_RAKP_M3 0xC2
//remote control 0310

#define RMCP_SEND_NONE    0x00
#define RMCP_SEND_ACK     0x01
#define RMCP_SEND_RES     0x10

//structure for rmcp state, rmcp control block
typedef struct
{
    INT32U session_id;  //session id
    INT32U seq_num;  //sequence number
}RSPHdr;

typedef struct
{
    INT16U padding;
    INT8U  pad_len;
    INT8U  next_header;
    INT8U  integrity[SIK_KEY_SIZE_USE];
}RSPTrailer;

typedef struct
{
    INT8U version;
    INT8U reserved;
    INT8U seq_num;
    INT8U com; //class of message
}RMCPHdr;

typedef struct
{
    INT8U  IANA[4];
    INT8U  MsgType;
    INT8U  MsgTag;
    INT8U  rsvd;
    INT8U  Data_Length;
}RMCPData;


typedef struct
{
    INT8U    version;
    INT8U    state;
    INT8U    type;
    INT8U    seq_num;
    INT8U    status;
    INT8U    role;
    INT8U    name_length;
    INT8U    rsvd;
    INT32U   session_seq;
    INT32U   client_sid;
    RSPHdr   *rsp_hdr;
    RMCPHdr  *rmcp_hdr;
    RMCPData *rmcp_data;
    INT8U    *var_data;
    INT8U    *key;
    INT8U    MgtSID[4];
    INT8U    CltRandom[16];
    INT8U    MgtRandom[16];
    INT8U    username[16];
}RMCPCB;

void rmcpInput(PRTSkt s, PPKT rxPkt);
void ASFV2Task(void *p_arg);
void ASFV1Task(void *p_arg);

#endif
