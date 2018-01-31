/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	lib.h

*****************************************************************************
*/

#ifndef _LIB_INCLUDED
#define _LIB_INCLUDED

#define CKHSUM_OFFLOAD

/*--------------------------Include Files-----------------------------------*/
#include <stdio.h>
#include <ctype.h>
#include <sys/ucos_ii.h>
#include <rlx/rlx_types.h>
#include <sys/dpdef.h>
#include <sys/inet_types.h>

#include <time.h>
#include "debug.h"
#include "tls.h"
#include "krb.h"

#ifdef CONFIG_MAIL_WAKEUP_ENABLED
#include "mail_wakeup.h"
#endif

/*--------------------------Definitions------------------------------------*/


#define BIG_ENDIAN          2
#define LITTLE_ENDIAN       1
#define BYTE_ORDER LITTLE_ENDIAN

//move to inet_types.h
//#define MAC_ADDR_LEN        6
#define ETH_PADDING_SIZE    2

//#define IPv4_ADR_LEN	4
//#define IPv6_ADR_LEN	16

#define ETHERNET_TYPE_IPv4	0x0800
#define ETHERNET_TYPE_IPv6	0x86DD
#define ETHERNET_TYPE_ARP	0x0806

#define IPv4 4
#define IPv6 6

#define TCP_HDR_SIZE	20
#define UDP_HDR_SIZE	8

#define IPv4_HDR_OFFSET		16
#define TCPv4_HDR_OFFSET	36
#define TCPv6_HDR_OFFSET	56

#define ETH_PKT_LEN		1516
#define ETH_PAYLOAD_LEN  1300

#define ETH_HDR_SIZE	16
#define MAX_PAGE_LINE_LEN 255
#define LEAST_PAGE_LINE_LEN 255

#define eth0	0
#define wlan0	1
#define MAX_INTF	2

//size of PKT(4) + ETH_HDR_SIZE(16) + IP max header size 60 + TCP max header size 60
//Teredo Tunnel ETH_HDR_SIZE(16) + IPv4 max header size 60 +UDP Header 8+ teredo tunnel 20 byte(min)?? + IPv6 Header size 40 
#define PKT_HDR_SIZE 140
#define IP_PKT_HDR_SIZE 80
//UDP_HDR_SIZE	8
#define UDP_PKT_HDR_SIZE 140

#define MAXSENDPKTS         3
#define	MAXRTTIMES		12
#define TIMEOUT             3
#define CONNECT_RETRY	3
#define SKT_DLY_TIME	20

#define off	0
#define on	1

#define NEED_PATCH 9
#define MAX_RTR_SOLICITATION_DELAY 1

#define DHCPv4_Cli 68
#define DHCPv4_Srv 67

#define DHCPv6_Cli 546
#define DHCPv6_Srv 547

#define SNMPPort   0x00A2

#define DNS_Srv_PORT 53 

#define BASIC_AUTH 1
#define DIGEST_AUTH 2
#define KERB_AUTH 3
#define MAX_PS_LEN 32 //provisioning server 
#define MAX_DN_LEN 64//domain name
#define MAX_HN_LEN 16 //host name
#define MAX_FQDN_LEN 32 //fqdn (amt like)
#define MAX_PW_LEN 32 //password
#define MAX_OU_LEN 64 //ou
#define MAX_OTP_LEN 32
#define PID_LEN 8
#define PPS_LEN 32

//#define MWU_MAX_STR 32 //this is for mailwakup

#ifdef CONFIG_PRINT_OSMEM
#define debug_printFreeMemory_F() debug_printFreeMemory()
#else
#define debug_printFreeMemory_F() 
#endif


typedef unsigned char 	u8_t;
typedef unsigned short	u16_t;
typedef unsigned int	u32_t;

enum {
    DASH_SUCCESS = 0,
    DASH_FAIL = 2,
};

enum {
    OPT_NOCS = 0,
    OPT_IPCS = 1,
    OPT_TCPCS = 2,
    OPT_UDPCS = 4,
    OPT_FREE  = 16,
    OPT_IPV6  = 64,
};

enum {
	ISR_ACCRISC_bit = 1<<27,
    DASHOK = 1<<15,
    HOSTOK = 1<<14,
    REQDMODE = 1<<13,
	IMR_ACCRISC_bit = 1<<11,
    OCP_ACCIB = 1 << 10,
    OCP_TER   = 1<<9, 
    OCP_RER   = 1<<8,
    OCP_SW_INT = 1<<6,
	OCP_LNKCHG = 1<<5,
    OCP_FOVW = 1<<4,
    OCP_TDU = 1<<3,
    OCP_TOK = 1<<2,
    OCP_RDU = 1<<1,
    OCP_ROK = 1<<0,
};


enum LanguageType{
	en = 0,
	zh_TW,
	zh
};

//For fwLog
//Producer Enum
enum {nobody , PRODUCER_ICMP };
//Event Type
enum { none , DETAIL ,  ALLOCATE_MEM_FAIL ,};


#define DEFAULT_IMR (OCP_SW_INT|OCP_RDU|OCP_ROK|OCP_LNKCHG)

typedef struct _TCROPT
{
    INT8U  name[16];
    INT8U  code[16];
    INT32U len;
}TCROPT;

typedef struct _WAKE_UP_t
{
    INT8U  valid;
    INT8U  exist;
    INT8U  start;
    INT8U  length;
    INT8U  name[36];
    INT8U  mask[16];
    INT8U  pattern[128];
    struct _WAKE_UP_t *next;
    struct _WAKE_UP_t *prev;
}WAKE_UP_t;

typedef struct _WAKE_Info_t
{
    INT8U index;
    struct _WAKE_UP_t *ptr;
}WAKE_Info_t;

typedef struct _DASHDesc
{
	unsigned int len:16, RSVD:15, own:1;	
	int RESERVED;
	char  *lowAddr;
  char  *highAddr;
}DASHDesc;


typedef struct OpaqueManagementData
{
	INT8U Status:1, dirty:1, rw:1/*0:read, 1:write*/;
	INT8U Owner[16];
	INT8U ElementName[16];
	INT8U DataFormat[8];
	INT32U DataSize;
	INT32U LastAccessed;
	INT32U MaxSize;
	INT16U WriteLimited;
	INT16U Access;
	INT16U DataOrganization;
	INT32U BlockSize;
	INT16U NumberOfBlocks;
	INT16U ConsumableBlocks;

}OPAQDATA;
/*-----------------------------Macros----------------------------------------*/
#define TCP_SEQ_GEQ(a,b)    ((unsigned int)((a)-(b)) >= 0)
#define TCP_SEQ_LEQ(a,b)    ((unsigned int)((a)-(b)) <= 0)

/*-----------------------------Structures------------------------------------*/


typedef struct _ServiceCfg
{
//#if WIN
//	unsigned char IPv4Enable:1, IPv6Enable:1, DHCPv4Enable:1, DHCPv6Enable:1, DHCPRTTime:4;
//	unsigned char EchoService:1, Security:3;
//#else
//Do not use char bit field in 4180
		unsigned int IPv4Enable:1, IPv6Enable:1, DHCPv4Enable:1, DHCPv6Enable:1,
DHCPRTTime:4, DHCPv6NeighborAdver:1, EchoService:1, Security:3, Reserved:19;
		char srcMacAddr[MAC_ADDR_LEN];
//#endif
}ServiceCfg;

//extern ServiceCfg scfg;
extern unsigned char HostIPv6[IPv6_ADR_LEN];
extern OS_EVENT *ArpTableSem;
extern OS_EVENT *SktSem;

typedef struct _EthHdr
{
    u8_t	unUsed[2];
    u8_t	destAddr[MAC_ADDR_LEN];
    u8_t	srcAddr[MAC_ADDR_LEN];
    unsigned short	ethType;
} EthHdr, *PEthHdr;


typedef struct _DataList {
    struct _DataList *next;
    unsigned char *addr;
    unsigned char *payLoadAddr;
    unsigned int len;
}DataList;

#define PKT_SIZE 24

typedef struct _PKT{
	struct _PKT *prev;
	struct _PKT *next;
	unsigned char *start;//Data start address	
    unsigned char *wp;//Data start address
	unsigned short len; // packet lenght
	unsigned char ipVer:4, intf:4;
	unsigned char rTime:7,queued:1;	
}PKT;

typedef struct _UserInfo{
    INT8U  name[16];
    INT8U  passwd[16];
    INT8U  role;
    INT8U  opt;
    INT16U crc16;
    INT8U  caption[23];
    INT8U  opaque;
    struct _UserInfo *next;
}UserInfo;

typedef struct _RoleInfo{
    INT8U  name[23];
    INT8U  mask;
    INT32U privilege;
    struct _RoleInfo *next; 
}RoleInfo;

typedef struct _ADUserInfo{
	unsigned char sid[28];
	unsigned char ad;
	unsigned char dummy[3];
	unsigned char role;
	INT8U  opt;
	INT16U crc16;
	INT8U  caption[23];
	INT8U  opaque;
	struct _UserInfo *next;
}ADUserInfo;

#define max_data_size 64
typedef struct _SubEvent {
    char subid[38];
    char notifyto[80];
    char query[80];
    INT16U enable;
    INT32U heartbeatLast;
    INT32U interval;
    INT8U opt[44];
    struct _SubEvent *next;
}SubEvent;

typedef struct _pkidata
{	
	INT8U fqdn[MAX_FQDN_LEN];
	INT8U dnssuf[MAX_FQDN_LEN];	
	INT8U certhash[6][20];
}PKIDATA;
typedef struct _pskdata
{	
	INT8U PID[PID_LEN];
	INT8U PPS[PPS_LEN];	
}PSKDATA;

typedef struct _MulticastList{
   struct _MulticastList *next;
   unsigned char addr[6];
}MulticastList;

#if CONFIG_WIFI_ENABLED	
typedef struct _WIFICfg
{
	char wifion;
	char auth;
	char enc;
	char scv;
	char pf_name[16];
	char SSID[16];
	char pwd[16];	
}WIFICfg;
#endif

typedef struct _dpconf
{    
    UserInfo admin;
    INT32U wdt;
    IPAddress HostIP[MAX_INTF];
    IPAddress SubnetMask[MAX_INTF];
    IPAddress GateWayIP[MAX_INTF];
    IPAddress DNSIP[MAX_INTF];
    IPAddress ConsoleIP[MAX_INTF];

    INT32U asfon:1, chipset: 3, numsent: 4, hbtime : 8, polltime : 6, asfpoll: 1, lspoll: 1, arpaddr : 8 ; 
    INT32U IPv4Enable:MAX_INTF, IPv6Enable:MAX_INTF, DHCPv4Enable:MAX_INTF, DHCPv6Enable: MAX_INTF, EchoService:1, Security:3, httpService:1, httpsService:1, wsmanService:1, isDHCPv4Enable:MAX_INTF;
    INT32U  MatchSubnetMaskValue[MAX_INTF];
    //INT8U DNSMAR[MAC_ADDR_LEN];
	
    INT8U srcMacAddr[MAX_INTF][MAC_ADDR_LEN];
    INT16U efusever;
    INT32U fwMajorVer;
    INT32U fwMinorVer;
    INT32U fwExtraVer;
    INT32U fwBuildVer;
    INT32U builddate;

    INT8U HostIPv6[MAX_INTF][IPv6_ADR_LEN];
    INT8U IPv6Prefix[MAX_INTF][IPv6_ADR_LEN];
    INT8U IPv6DNS[MAX_INTF][IPv6_ADR_LEN];
    INT8U IPv6GateWay[MAX_INTF][IPv6_ADR_LEN];
    INT8U IPv6PrefixLen[MAX_INTF];	   
    INT8U BMCPoll:1, BMCPollTime: 7;
    INT8U DBGMacAddr[MAC_ADDR_LEN];
    IPAddress DBGIP;
    INT16U DASHPort;
    INT16U DASHTLSPort;
    INT8U  HostName[16];
    INT8U  Profiles[12];
    INT8U DomainName[MAX_DN_LEN];
    INT16U ProvisioningSetup:1, ProvisioningMode:1, ProvisioningState: 2, PKIEnabled:1, PSKEnabled:1, dummy1:10;
    INT16U ProvisionServerPort; 
    IPAddress ProvisionServerIP;  
    INT8U ProvisionServerName[MAX_PS_LEN];
    struct _pkidata pkidata;
    struct _pskdata pskdata;
    INT8U OTP[MAX_OTP_LEN];
    INT8U OUString[MAX_OU_LEN];
    INT8U UUID[20];
    INT32U ipmode;
    INT8U  aesmsg[20]; //runtime usage, to get key from setup binary
    INT32U efusekey;
    INT32U bios:8, biostype: 8,  vendor:8, pldmtype: 4, pldmmultitx: 1, pldmdbg:1, enpldmsnr :1, pldmreset:1;
    INT8U  restart;    //inform dash to reget the in-band data
    INT8U eap;
    INT8U usbotg:1, ehcipatch:1, usbrsvd:6;
    INT8U useTeredo:1, teredoEnable:1, teredorsvd:6;
    INT8U teredoSrv[32];
    //PLDMRCC means the slave address of Remote Control and PLDM are the same
    INT32U pldmsnr:8, numofsnr:8, pldmfromasf: 1, pldmpec: 1, snrpec: 1, pldmrsvd: 13;
    INT8U  pldmslaveaddr;
    INT8U  pldmsrvd[3];
    struct _EAPTLS *eaptls;
    unsigned char eappro;
    unsigned char enctype;
    unsigned char enckey[16];
 	//INT8U DomainRealm[MAX_DN_LEN];
	
    //mail_wakeup module by eccheng
    #ifdef CONFIG_MAIL_WAKEUP_ENABLED
    INT8U mail_wake_up; //on/off {on : != 0, off : = 0}
    INT8U email_server[MWU_MAX_STR]; //[max.length=128] {IP, Domain Name}//POP3(110) and POP3S(995)
    INT8U smtp_SSL; //enable {disable (port=110), enable (port=995)}
    INT8U email_acount[MWU_MAX_STR]; //[max.length=128] 
    INT8U email_passwd[MWU_MAX_STR]; //[max.length=128] 
    INT8U match_address[MWU_MAX_STR]; //(from) [max.length=128]
    INT8U match_subject[MWU_MAX_STR]; //[max.length=128] 
    INT8U match_keyword[MWU_MAX_STR]; //[max.length=128]„²only check email content ¡¥s first 1024 bytes
    INT8U Activate_condition; //{condition code = 1..13 }, now only support 7,12,13
    INT8U mailPeriod;// (second) {default:3 second}
    //System Variables
    struct mailwakeup_ts ts_base;
     #endif		
	
     INT16U counter; 	 //+briankuo , 20140909 : Add for recording reset times
#ifdef CONFIG_DOORBELL_CHECK_ENABLED
     INT16U ostmr_unit;
#endif 
#if CONFIG_WIFI_ENABLED	
	WIFICfg wificfg;
#endif
	
}DPCONF;

#ifndef false
#define false 0
#endif

#ifndef true
#define true (!false)
#endif


typedef struct os_mem_debug {                   /* MEMORY CONTROL BLOCK                                      */
    void   *OSMemAddr;                    /* Pointer to beginning of memory partition                  */
    void   *OSMemFreeList;                /* Pointer to list of free memory blocks                     */
    INT32U  OSMemBlkSize;                 /* Size (in bytes) of each block of memory                   */
    INT32U  OSMemNBlks;                   /* Total number of blocks in this partition                  */
    INT32U  OSMemNFree;                   /* Number of memory blocks remaining in this partition       */
    INT8U   OSMemName[16];  /* Memory partition name                                     */
} OS_MEM_DEBUG;

extern OS_MEM_DEBUG *m_OsMemTbl;

extern enum LanguageType mLanguageType;

/*-----------------------------Functions------------------------------------*/

#if BYTE_ORDER == BIG_ENDIAN

#define htons(x) (x)
#define ntohs(x) (x)
#define htonl(x) (x)
#define ntohl(x) (x)
#else
unsigned short htons(unsigned short x) _ATTRIBUTE_SRAM;
unsigned short ntohs(unsigned short x) _ATTRIBUTE_SRAM;
unsigned int htonl(unsigned int x) _ATTRIBUTE_SRAM;
unsigned int ntohl(unsigned int x) _ATTRIBUTE_SRAM;
#endif


INT8U write_opaque(INT8U no, INT16U offset, INT16U size, INT8U *data) _ATTRIBUTE_SRAM;
INT8U read_opaque(INT8U no, INT16U offset, INT16U size, INT8U *data) _ATTRIBUTE_SRAM;
INT8U create_opaque(INT8U no, INT32U ms, INT8U *df, INT8U *en, INT8U* or) _ATTRIBUTE_SRAM;
INT8U delete_opaque(INT8U no) _ATTRIBUTE_SRAM;
unsigned int chksum(void *data, int len);
u16_t inet_chksum(void *data, int len);
u16_t inet_chksum_pseudo(void *p, void *src, void *dest, u8_t proto, u16_t proto_len);
void* allocPkt(int size) _ATTRIBUTE_SRAM;
void* allocSSHPkt(void *ptr, int len, int isdata);
void* allocTLSPkt(void *ptr, int len) _ATTRIBUTE_SRAM;
void* allocIPPkt(int size, char ipVer) _ATTRIBUTE_SRAM;
void* allocUDPPkt(int size, char ipVer) _ATTRIBUTE_SRAM;
unsigned int getIPAddress(unsigned char intf) _ATTRIBUTE_SRAM;
void setIPAddress(unsigned int ipAddr, unsigned char intf);
void setSubnetMask(unsigned int ipAddr, unsigned char intf);
void setGateWayIP(unsigned int ipAddr, unsigned char intf);
void setDNSIP(unsigned int ipAddr, unsigned char intf);
void setIPv6Address(unsigned char *ip, unsigned char intf);
void setIPv6GateWay(unsigned char *ip, unsigned char intf);
void setIPv6DNS(unsigned char *ip, unsigned char intf);
void setLinkLocalAddress(unsigned char *ip, unsigned char intf);
unsigned char* getIPv6Address(unsigned char intf) _ATTRIBUTE_SRAM;
void resetMulticasAddress();
void addMulticasAddress(unsigned char addr[6]);
void setMulticastList();
DataList* newDataList();
DataList* chkDataListBufSize(DataList* dl);
unsigned int getDataListBufSize(DataList* head);
void sendDataList(ptcp_pcb pcb, DataList* head) ;
char* strrtrim(char *ptr);
char* strltrim(char *ptr);
char* trim0x09toNULL(char *str);
char* trimspace(char *ptr);
int   inctimeofday(int);
int   parsebios(unsigned char val);
void SMBIOSget(int type, int offset, INT8U *out) _ATTRIBUTE_EXCEPT;
INT32U get_log(void);
void start_log(void);
void inituser(void) ;
INT8U *getpwd(INT8U *name);
void getGUID(INT8U *RTGUID);
INT8U getrole(INT8U *name);
UserInfo* getuserinfo();
UserInfo *adduser(INT8U *name, INT8U *passwd);
void setrole(INT8U *name, INT8U role);
INT8U deluser(INT8U *name);
void moduser(INT8U *name,INT8U *passwd);
void freePkt(PKT *pkt) _ATTRIBUTE_SRAM;
void sendTCPData(void *ptr) _ATTRIBUTE_SRAM;
void addEthernetHeader(void *ptr,PKT* pkt) _ATTRIBUTE_SRAM;
void* setPkt(unsigned char *start, int size);
void initconf(DPCONF **ptr) ;
int getGateWayMAR(unsigned char* cpEthAddr, unsigned char intf);
int getDNSMAR(unsigned char intf);
void set_time(INT8U *timestampdataptr);
void setMatchSubnetMask(unsigned char intf);
int sendPkt(PKT *pkt) _ATTRIBUTE_SRAM;
INT8U* getBootOrder();
INT8U *getBootstr(INT8U index);
INT8U getBootID(INT8U *str);
void sotpTask(PRTSkt cs);
void ltostr(unsigned long long value, INT8U *str);
void tcrgetdata(PKT *pkt);
PRTSkt stopconn(PRTSkt cs);
INT32U ifloor(INT32U a, INT32U b);
INT32U crc32(INT32U crc, INT8U *buf, INT32U len);
void GPIOEnable();
void GPIODisable();
void GPIOSet(INT8U value);
INT8U GPIOGet(INT8U *value);
void getGUIDstr(INT8U *str);
void hextostr(INT8U value, INT8U *str);
INT32U DigitLength(INT32U value);
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
void setupPatchFun() _ATTRIBUTE_EXCEPT;
#elif CONFIG_VERSION <= IC_VERSION_DP_RevF
void setupPatchFun() _ATTRIBUTE_INIT;
#endif
void UserOpaque(INT8U id,INT8U opaque,INT8U mode);
void wake_up_pattern_init() _ATTRIBUTE_SRAM;
void WakePtnAdd(WAKE_UP_t *ptr) _ATTRIBUTE_SRAM;
void WakePtnDel(WAKE_UP_t *ptr) _ATTRIBUTE_SRAM;
WAKE_UP_t *GetWakePtnHead() _ATTRIBUTE_SRAM;
INT8U write_pattern(INT16U size, INT8U *data) _ATTRIBUTE_SRAM;
INT8U* lan_wake_up(INT8U *buf, INT32U size) _ATTRIBUTE_SRAM;
INT8U  PrinterRmtl(INT8U MsgType);
void pldm_snr_read(INT8U snrnum);
//void UserOpt(INT8U,INT8U);

VOID AcquireIbOobMutex();
VOID ReleaseIbOobMutex();

#ifdef CONFIG_CPU_SLEEP_ENABLED
void SetLucidTimerInSec(int sec);
void EnableCPUSleep();
void DisableCPUSleep();
#endif


#endif


