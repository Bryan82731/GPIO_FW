/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	tcp.h

*****************************************************************************
*/

#ifndef _TCP_INCLUDED
#define _TCP_INCLUDED

/*--------------------------Include Files-----------------------------------*/
#include "lib.h"
#include "ip.h"
#include "debug.h"
#include "tls.h"
#include "rtskt.h"
#include "bsp.h"
//#include "http.h"
#ifdef CONFIG_MSAD_ENABLED
#include "krb.h"
#endif

/*--------------------------Definitions------------------------------------*/
#define TCPMAXWINSIZE 8192
#define TCPMAXSEGSIZE 1460
#define MAXCONNECTIONS	64
#define TCPIP_SIZE (2 + ETH_HDR_SIZE + IP_HLEN  + TCP_HDR_SIZE)
/* TCP Control flags */
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

//HttpParseStatus
#define HTTPINIT 0
#define	HTTPHEADER 1
#define HTTPDATA 2

/* PCB state(TCP state) */
#define  CLOSED 0
#define  LISTEN 1
#define  SYN_SENT 2
#define  SYN_RCVD 3
#define  ESTABLISHED 4
#define  FIN_WAIT_1 5
#define  FIN_WAIT_2 6
#define  CLOSE_WAIT 7
#define  CLOSING 8
#define  LAST_ACK 9
#define  TIME_WAIT 10

/* DelayAck state */
#define NoDelayAck 0
#define FirstDelayAck 1
#define SecondDelayAck 2

#define TCP_TIMER_DLY OS_TICKS_PER_SEC * 0.4

#if CONFIG_VERSION	== IC_VERSION_DP_RevF
#define MaxPortMatchRecords 5
#else
#define MaxPortMatchRecords 16 //EP can up to 128 
#endif

enum PostURL
{
    REMOTECTLPOST = 0x00,
    BOOTPOST,
    SECURITYPOST,
    NETPOST,
    SRVPOST,
    AUTHPOST,
#if CONFIG_WIFI_ENABLED
	WIFICFGPOST,
#endif
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED    
    PTNEDTPOST,
    PTNADDPOST,
#endif

#ifdef CONFIG_PRINTER_WEB_PAGE
    SMBPOST, //ht 20110816
#endif
    EzSharePOST,
    //mail_wakeup 20121025 eccheng
    MAIL_ACCPOST,
    MAIL_MATPOST  
};

//Used to record TCP list data
typedef struct _TCPData
{
	struct _TCPData	*next;
	void	*data;//data start address
	int		len;
	char	rtime;
}TCPData;
#ifdef CONFIG_MSAD_ENABLED
typedef struct _MSAD
{
	PKT*					rPkt;
	unsigned char			wpid;
	char					negostatus;
	gss_cred_id_t			server_creds;
	unsigned char*			path;	
	gss_buffer_t auth_buffer;
}MSAD;
#endif

/* the TCP protocol control block */
typedef struct tcp_pcb {	
    struct tcp_pcb			*next;
	struct tcp_pcb			*clientList;
	struct _RTSkt			*skt;
	enum PostURL			postURL;			/*Record post url*/
	/* Function to call when a listener has been connected. */	
	int (* apRx)(void *arg, int len, struct tcp_pcb *newpcb);

	IPAddress				destIPAddr;
	unsigned char			destIPv6Addr[IPv6_ADR_LEN];
	unsigned int			sendAckNum;			//The sequence number that server has 
												//sended
	unsigned int			oppAckNo;			//oppsite ack no
	unsigned int			seqNum;				//sequence number
	int						httpContentLen;		//Http content length
	int						httpReadLen;		//How many data that have been received	

	PKT						*txQStart, *txQEnd;	//outQueue	
	PKT						*rxQStart, *rxQEnd;	//input data
	char					*httpReqData;		//Request data
	SSL						*ssl;
	
	unsigned short			cwnd;					/* Client window size (Flow control used)*/	
	unsigned short			localPort, destPort;
	unsigned short			cmss;					/* Client maximum segment size (Max segmwnt size)*/    
	unsigned short			ipID;
	char					dest_MAR[6];
	unsigned char			protocol;
	unsigned char			ipVer:3,
							state:4,/* TCP state */
							accepted:1;

	unsigned char					sendPkts:5,TLSEnable:1, TLSDataStage:1;
	unsigned char			httpAuthPass:1, httpMethod:2, 
							httpAuthMethod:2, httpParseStatus:3;
	unsigned char			maxConnections:3 //max connections for this service 
							,curConnections:3 //current connections
							,delayAck:2; //has been accepted
	unsigned char			probeCount:4, probe:1, alive:1,rxEnable:1, txEnable:1;
	unsigned char hwIBPortFilterEnable:1/*Unicast*/, hwOOBPortFilterEnable:1/*Broadcast/Multicast*/, timeWaitEnable:1, specIntf:1, intf:4;
#ifdef CONFIG_MSAD_ENABLED
	MSAD	msad;
#endif
}TCPPCB;

typedef struct _TCPHdr {
  unsigned short src;
  unsigned short dest;
  unsigned int seqno;
  unsigned int ackno;
#if BYTE_ORDER == LITTLE_ENDIAN
  unsigned char unused:4,
    offset:4;
#else
  unsigned char offset:4,
    unused:4;
#endif
  unsigned char flags;
  unsigned short wnd;
  unsigned short chksum;
  unsigned short urgp;
}TCPHdr;

typedef struct _TCPHdrOpt
{
    unsigned char optCode;
    unsigned char optLen;
    unsigned short optMss;
}TCPHdrOpt;

typedef struct _PortRecord
{
	unsigned short port;
	unsigned short ipv4UDP:1, ipv4TCP:1, ipv6UDP:1, ipv6TCP:1;
}PortRecord;

#ifdef CONFIG_PortMatch_Test
typedef struct _PMMsg
{
	char token[3];
	unsigned char Add:1, PMTIPv4:1, PMTIPv6:1 , PMTTCP:1 , PMTUDP:1, unicast:1;
	unsigned short port;
}PMMsg;

typedef struct _PMTPortRecord
{
	unsigned short port;
	unsigned short ipv4UDP:1,ipv4TCP:1,ipv6UDP:1,ipv6TCP:1;		
	struct _RTSkt* sv4UDP;
	struct _RTSkt* sv4TCP;
	struct _RTSkt* sv6UDP;
	struct _RTSkt* sv6TCP;
}PMTPortRecord;

#endif

/*--------------------------Global variables----------------------------------*/
extern TCPPCB *ActivePCBsList;

/*----------------------------SRAM Functions-----------------------------------*/
#ifdef CONFIG_IPERF_Test
int iperfRx(void *pData, int len, struct tcp_pcb *pcb) _ATTRIBUTE_SRAM;
void iperfTask(void *data) _ATTRIBUTE_SRAM;
#endif
void addTcpHeader(TCPPCB *pcb, PKT* pkt, unsigned char flag) _ATTRIBUTE_SRAM;
void postListenPort(TCPPCB *pcb);
void tcpDequeue(TCPPCB *pcb, unsigned int ackNum) _ATTRIBUTE_SRAM;
void tcpEnqueue(TCPPCB *pcb, PKT *pkt) _ATTRIBUTE_SRAM;
void tcpInput(PKT *pkt) _ATTRIBUTE_SRAM;
int tcpPeriodChk(TCPPCB *pcb) _ATTRIBUTE_SRAM;
void tcpProcess(PKT *pkt, TCPPCB *pcbPtr) _ATTRIBUTE_SRAM;
void tcpRxInput(PKT *pkt, TCPPCB *pcb, INT8U TLS1st) _ATTRIBUTE_SRAM;
void tcpSend(TCPPCB *pcb, PKT* pkt,unsigned char flag) _ATTRIBUTE_SRAM;
void tcpSendData(TCPPCB *pcb, PKT *pkt) _ATTRIBUTE_SRAM;
void tcpTimerTask(void *data) _ATTRIBUTE_SRAM;
unsigned int v6pseudochksum(void *data,int len) _ATTRIBUTE_SRAM;
/*-----------------------------Functions------------------------------------*/
int reserveIBPort(TCPPCB *pcb, unsigned short port);
TCPPCB *tcpPCBNew(void);

void removeSerivce(int port);
void removePCB(TCPPCB *pcb);

int tcpListen(TCPPCB *pcb);

void tcpRx(TCPPCB *pcb,
           int (* apRx)(void *arg, int len,TCPPCB *pcb),
           void *apRxArg);

int tcrCurBufPkts();
void tcpPCBRegister(TCPPCB *pcb);
void tcpPCBUnRegister(TCPPCB *pcb);
void tcpClose(TCPPCB *pcb, PKT* pkt);
int tcpProbe(TCPPCB *pcb);

void reloadFilterTable();
void removePortFilterTable(TCPPCB *pcb);
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
void reloadFilterTableEP();
void removePortFilterTableEP(TCPPCB *pcb);
void setNewIBTCAMPortPF(unsigned char ipVer, unsigned char protocol);
void setPortFilterTableEP(TCPPCB *pcb);
void SetIBPF();
void SetOOBPF();
void setNewIBTCAMPortPF(unsigned char ipVer, unsigned char protocol);
#endif
int resetIPPtl(unsigned char IPVer);
void setPortFilterTable(TCPPCB *pcb);
void tcpConnectionIn(TCPPCB *srvPcb, TCPPCB *clientPcb);
int tcpPeriodChk(TCPPCB *pcb);
#endif

