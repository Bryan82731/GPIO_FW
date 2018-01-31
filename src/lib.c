/* ******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	lib.c

Abstract:	Commom functions for all modules use

*****************************************************************************
*/

/*--------------------------Include File------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include "aes.h"
#include "lib.h"
#include "ip.h"
#include "tcp.h"
#include "bsp.h"
#include "smbios.h"
#include "tls.h"
#include "smbus.h"
#include "arp.h"
#include "ssh.h"
#include <ctype.h>
#include "rlx_library.h"
#include "soap.h"
#include "dhcp.h"
#include "udp.h"
#include "hwpf.h"
#include "bsp.h"
#include "rmcp.h"

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
RoleInfo *rolehead=NULL,*roletail=NULL;
#else
ROM_EXTERN SubEvent *subhead _ATTRIBUTE_ROM_BSS;
ROM_EXTERN SubEvent *subtail _ATTRIBUTE_ROM_BSS;
ROM_EXTERN RoleInfo *rolehead _ATTRIBUTE_ROM_BSS;
ROM_EXTERN RoleInfo *roletail _ATTRIBUTE_ROM_BSS;
ROM_EXTERN WAKE_UP_t *wake_head _ATTRIBUTE_ROM_BSS;
ROM_EXTERN WAKE_UP_t *wake_tail _ATTRIBUTE_ROM_BSS;
#endif

extern SubEvent *subhead,*subtail;
extern flash_data_t dirty[ENDTBL];
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
WAKE_Info_t wake_info[256];
#endif

extern DPCONF *dpconf;
extern _IPInfo IPInfo[];
extern TCAM_Allocation TCAMMem[NumOfTCAMSet];

enum LanguageType mLanguageType;

//dirty_ext has been merge into dirty in EP
//however, we add back asf!
flash_data_t dirty_ext[10];

int idle_times = 0;

INT32U last_notice_tick = 0; //record last idle tick
int sleepControler = 0;
unsigned char stop_sleep_couter = 0;
int m_LucidTimer = -1;
volatile INT32U bsp_wait_cnt = 75;

#ifdef CONFIG_PRINT_OSMEM

#if CONFIG_VERSION >= IC_VERSION_EP_RevB
OS_MEM_DEBUG *m_OsMemTbl = 0x8a0198f0;
#else if CONFIG_VERSION == IC_VERSION_DP_RevF
OS_MEM_DEBUG *m_OsMemTbl = 0x8a0198f0;
#endif


void debug_printFreeMemory(){
	int idx = 0;
	
	for(idx=0;idx<10;idx++){
		
		m_OsMemTbl[idx].OSMemNFree;
	}
	printf("[RTK] (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",m_OsMemTbl[0].OSMemNFree,m_OsMemTbl[1].OSMemNFree
		,m_OsMemTbl[2].OSMemNFree,m_OsMemTbl[3].OSMemNFree,m_OsMemTbl[4].OSMemNFree,m_OsMemTbl[5].OSMemNFree
		,m_OsMemTbl[6].OSMemNFree,m_OsMemTbl[7].OSMemNFree,m_OsMemTbl[8].OSMemNFree,m_OsMemTbl[9].OSMemNFree);
}
#endif

#if CONFIG_VERSION >= IC_VERSION_DP_RevF

void* allocIPPkt_4bytesAlignment(int size, char ipVer)
{	
    int size4;
    PKT *pkt;
    pkt =(PKT*)malloc(PKT_SIZE);

    pkt->next = 0;
    pkt->prev = 0;
    pkt->rTime = 0;
    pkt->queued = 0;
    if (IP_PKT_HDR_SIZE + size > 1536)
    {
        pkt->start =(unsigned char*) malloc(1536);

	if(size%4!=0){
		size4 = (size/4+1)*4; //modify to Four multiples
		pkt->wp = pkt->start + 1536 - size4;
	}else{		
       	pkt->wp = pkt->start + 1536 - size;
	}
		
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

extern void icmpInput_debug(PKT * pkt, int len);
extern void OSTaskIdleHook_pathched (void);
void setupPatchFun()
{
    gmac_software_hook_F = gmac_software_hook;

#if CONFIG_VERSION == IC_VERSION_EP_RevA
    rx_recycle_F     = rx_recycle_patched;
#else
#if CONFIG_VERSION <= IC_VERSION_DP_RevF || defined(CONFIG_WAKE_UP_PATTERN_ENABLED)
    rx_recycle_F = rx_recycle;
#endif
#endif

#if CONFIG_VERSION == IC_VERSION_DP_RevF
    removePCB_F = removePCB;
    gmac_pcie_hook_F = gmac_pcie_hook;
    reserveIBPort_F = reserveIBPort;
    removePortFilterTable_F = removePortFilterTable;
    reloadFilterTable_F = reloadFilterTable;
#endif


#ifdef CONFIG_IPv6_ENABLED
    findIPv6EtherAddr_F = findIPv6EtherAddr;
#endif

#if CONFIG_VERSION >= IC_VERSION_EP_RevA
#if CONFIG_VERSION == IC_VERSION_EP_RevA
    setPortFilterTable_F = setPortFilterTableEP;
    findEtherAddr_F = findEtherAddr;
#elif (CONFIG_VERSION >= IC_VERSION_EP_RevB)     
    SetOOBBasicRule_F = SetOOBBasicRule;
    rtSktEnableOOBPortFilter_F = rtSktEnableOOBPortFilter;
SetOOBPF_F = SetOOBPF;
#endif

	findEtherAddr_F = findEtherAddr;

    reserveIBPort_F = reserveIBPort;
    tlsEncodeData_F  = tlsEncodeData;
    deCodeTLSData_F  = deCodeTLSData,
    doTLSAccept_F    = doTLSAccept,
    sendTLSAlert_F   = sendTLSAlert;

#if !defined(CONFIG_BUILDROM) || (CONFIG_ROM_VERIFY)
    rtSktAccept_F = rtSktAccept;
    addTcpHeader_F = addTcpHeader;
    tcpDequeue_F = tcpDequeue;
    tcpPeriodChk_F = tcpPeriodChk;
    tcpRxInput_F = tcpRxInput;
    tcpInput_F = tcpInput;
    rtSktConnect_F = rtSktConnect;
#endif
#endif

    ipInput_F = ipInput;
#if !defined(CONFIG_BUILDROM) || (CONFIG_ROM_VERIFY)
    getPortNumber_F = getPortNumber;
    rtGethostbyname_F = rtGethostbyname;
    rtSktUDPSendTo_F = rtSktUDPSendTo;
    getUDPAddressInfo_F = getUDPAddressInfo;
    tcpProcess_F = tcpProcess;
#endif

#if CONFIG_VERSION < IC_VERSION_EP_RevB
    updateArpTable_F = updateArpTable;
#endif

#if defined(CONFIG_TEREDO_ENABLED)
    addTcpHeader_F = addTcpHeader;
    addIPHeader_F = addIPHeader;
    tcpDequeue_F = tcpDequeue;
    tcpPeriodChk_F = tcpPeriodChk;
    setIPv6Address_F = setIPv6Address;
    addTcpHeader_F = addTcpHeader;
    rtSktConnect_F = rtSktConnect;	
#endif
	allocIPPkt_F = allocIPPkt_4bytesAlignment;

	#ifdef CONFIG_CPU_SLEEP_ENABLED
        OSTaskIdleHook_F = OSTaskIdleHook_pathched;
	#endif
}
#endif

#ifdef CONFIG_CPU_SLEEP_ENABLED
extern void CPU_Sleep(void);
void  OSTaskIdleHook_pathched (void)
{
	if(stop_sleep_couter == 0){
	CPU_Sleep();	
}
}

void DisableCPUSleep(){
	//stop_sleep_couter = 0;
	stop_sleep_couter++;
}

void EnableCPUSleep(){
	//flag_sleep = 1;	
	if(stop_sleep_couter>0){
		stop_sleep_couter --;
	}
}

void SetLucidTimerInSec(int sec){//Enable CPU Sleep after N secs
	if(sec>=0){
		m_LucidTimer = sec;
	}
}

void wakeCPU(){ //keep CPU from sleep in 3 secs
	//clear the counter  to avoid CPU sleep
	last_notice_tick = 0;
	//SetLucidTimerInSec(3);
}



#endif

#if 1
//Send Alert
extern OS_EVENT *EVTQ;

unsigned int makeAlertEvent( INT8U logtype , unsigned char eventProducer , unsigned char eventType ,  unsigned char append){

#if CONFIG_FWLOG_ENABLED
	
	eventdata evtdata;
          eventdata *evt_ptr;

		 printf("[RTK] + makeAlertEvent , id=0x%x\n", eventProducer); 

	memset(&evtdata, 0, sizeof(evtdata));
	evtdata.Event_Sensor_Type = 0x7F;
	 evtdata.Event_Type = 0x1F;

	 //-------------------------------------------------
	evtdata.EventData[0] = 0x40;//'@'
	evtdata.EventData[1] = 0x1A;
	evtdata.Event_Offset = 0x02;
	
	 evtdata.Sensor_Device = eventType;
	 evtdata.Sensor_Number = append;
           //evtdata.Entity = id;
           //evtdata.Sensor_Device = 99;
            //strcpy((char *)evtdata.EventData, "COMMON1");
	 //evtdata.Event_Offset = 11;	 
	 //evtdata.timestamp = htonl(OSTimeGet());	 
	  evtdata.Event_Severity = 0x01;           
	  evtdata.Event_Source_Type = eventProducer;  //function name or process name ; it means from who .
			
            evt_ptr = event_log(&evtdata.Event_Sensor_Type, 16);
            evt_ptr->interval = (ALERT_INTERVAL/(dpconf->numsent-1));
            evt_ptr->timeout = 0;
            evt_ptr->alertnum = dpconf->numsent;
	  evt_ptr->timestamp = OSTimeGet() /100;
	  evt_ptr->logtype = logtype;
            OSQPost(EVTQ, evt_ptr);

	 printf("[RTK] - makeAlertEvent\n"); 
	 
	return 0 ;
#else
 	return 0;
#endif
	
}

#endif



#if defined(CONFIG_TEREDO_ENABLED) || (CONFIG_VERSION < IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
//extern DPCONF *dpconf;
//extern _IPInfo IPInfo;

void setIPv6Address(unsigned char *ip, unsigned char intf)
{
    int *ipv6;
    unsigned char mar[MAC_ADDR_LEN] = {0};

    memcpy(dpconf->HostIPv6, ip, IPv6_ADR_LEN);

    mar[0] = 0x33;
    mar[1] = 0x33;
    mar[2] = 0xff;
    mar[3] = dpconf->HostIPv6[IPv6_ADR_LEN - 3];
    mar[4] = dpconf->HostIPv6[IPv6_ADR_LEN - 2];
    mar[5] = dpconf->HostIPv6[IPv6_ADR_LEN - 1];
    addMulticasAddress_F(mar);
    setMulticastList_F();

    ipv6 = (int *) ip;
#if(CONFIG_VERSION > IC_VERSION_DP_RevF )
    //Hw store with big endian
    memcpy(&(TCAMMem[TCAMDIPv6Set].data[UniIPv6Addr*IPv6_ADR_LEN]), dpconf->HostIPv6, IPv6_ADR_LEN);
    setTCAMData_F(&TCAMMem[TCAMDIPv6Set]);
#else

    REG32(IOREG_IOBASE+IO_DIPV6)= ipv6[0];
    REG32(IOREG_IOBASE+IO_DIPV6 + 4)= ipv6[1];
    REG32(IOREG_IOBASE+IO_DIPV6 + 8)= ipv6[2];
    REG32(IOREG_IOBASE+IO_DIPV6 + 0xc)= ipv6[3];
    //bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV6DIP1, 1);
    bsp_bits_set(IO_DIPFILTER, 1, BIT_FILTERV6DIP1, 1);	
#endif

#if CONFIG_TEREDO_ENABLED
    if ((dpconf->HostIPv6[0] == 0x20) && (dpconf->HostIPv6[1] == 0x01))
    {
        dpconf->teredoEnable = 1;
    }
    else
    {
        dpconf->teredoEnable = 0;
    }
#endif

    ///srvReset_F(IPv6);
    IPInfo[intf].IPv6Reset = 1;
    setdirty(DPCONFIGTBL);
}
#endif

#ifdef CONFIG_SIMPLE_BACKUP
INT8U verifyUserChksum()
{
    UserInfo *tmp, *rom, *current;
    aes_context *ctx = malloc(sizeof(aes_context));

    tmp = &dpconf->admin;
    rom = (UserInfo *) DPCONF_ROM_START_ADDR;

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    aes_crypt(ctx, (INT8U*) rom, (INT8U *) tmp, AES_DECRYPT);
#else
    memset(ctx, 0, sizeof(aes_context));
    aes_crypt_sw(ctx, (INT8U*) rom, (INT8U *) tmp, AES_DECRYPT);
#endif
    free(ctx);

    if (tmp->crc16 != inet_chksum(tmp, 34))
    {
      	return 0;
    }else{
    	return 1; // user chksum is OK
    }

    

	
}
#endif


#if (CONFIG_VERSION <= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
ROM_EXTERN UserInfo *userhead;

void* allocSSHPkt(void *ptr, int len, int isdata)
{
    SSH *ssh = (SSH *)ptr;
    unsigned char padlen = 0;
    PKT *pkt;
    int pageSize = 0;
    pageSize = PACKET_PAYLOAD_OFF + 1/*msg code*/ + len;
    if (isdata == 1)
        pageSize += 4 /*recvChanId*/;
    padlen = 16-((pageSize)%16);
    if (padlen < 4)
        padlen += 16;
    pageSize+=padlen;
    pageSize += ssh->macsize;
    pkt =(PKT*)malloc(PKT_SIZE);
    pkt->next = 0;
    pkt->prev = 0;
    pkt->rTime = 0;
    pkt->queued = 0;
    pkt->start =(unsigned char*) malloc(PKT_HDR_SIZE + pageSize);
    pkt->len = pageSize;
    pkt->wp = pkt->start + PKT_HDR_SIZE + PACKET_PAYLOAD_OFF + 1/*msg code*/;
    if (isdata == 1)
        pkt->wp += 4;
    return (void*)pkt;
}

//or : Owner Name , en : ElementName
INT8U create_opaque(INT8U no, INT32U ms, INT8U *df, INT8U *en, INT8U* or)
{
	int size = sizeof(OPAQDATA);
    if (no > 3)
        return 1;
	
	
	
   // INT8U* opaqdata = (INT8U*)malloc(4096);//-briank.rtk
   // memset(opaqdata, 0, 4096);//-briank.rtk
   INT8U* opaqdata = (INT8U*)malloc(size);
   memset(opaqdata, 0, size);

    ((OPAQDATA*)opaqdata)->Status = 1;
    ((OPAQDATA*)opaqdata)->dirty = 1;
    ((OPAQDATA*)opaqdata)->MaxSize = ms;
    memcpy(((OPAQDATA*)opaqdata)->Owner, or, 16);
    memcpy(((OPAQDATA*)opaqdata)->ElementName, en, 16);
    memcpy(((OPAQDATA*)opaqdata)->DataFormat, df, 8);

//printf("[RTK]create_opaque , no[%d] ElementName[%s] Owner Name[%s]\n",no,((OPAQDATA*)opaqdata)->ElementName,((OPAQDATA*)opaqdata)->Owner);

    //if (write_opaque(no, 0, 4096, (INT8U*)opaqdata) == 1)//-briank.rtk
    if (write_opaque(no, 0, size, (INT8U*)opaqdata) == 1)
    {
        free(opaqdata);
        return 1;
    }
    free(opaqdata);
    return 0;
}

INT8U delete_opaque(INT8U no)
{
    if (no > 3)
        return 1;
    OPAQDATA* opaqdata = (OPAQDATA*)malloc(sizeof(OPAQDATA));
    opaqdata->Status = 0;
    opaqdata->dirty = 0;
    write_opaque(no, 0, sizeof(OPAQDATA), (INT8U*)opaqdata);
    free(opaqdata);
    return 0;
}

INT8U write_opaque(INT8U no, INT16U offset, INT16U size, INT8U *data)
{
    struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
    INT32U flashaddr;
    int i = 0;
    INT8U *dataptr;
    OS_CPU_SR  cpu_sr = 0;

    if (no > 3 || (offset+size) > 4096)
        return 1;

    INT32U rdid = (spi_read_id(ssi_map)  & 0x00ff0000) >> 16;

    dataptr = malloc(4096);

    flashaddr = DPCONF_ADDR_CACHED + 0x2000 + no*0x1000;

    memcpy(dataptr, flashaddr, 4096);
    memcpy(dataptr+offset, data, size);

    flashaddr &= (0x00FFFFFF);

    OS_ENTER_CRITICAL();

    if (rdid == 0x1f)
        spi_se_unprotect(ssi_map, flashaddr);
    spi_se_erase(ssi_map, flashaddr);

    for (i =0; i < 64; i++)
        spi_write(ssi_map, flashaddr+i*64, dataptr+i*64, 64);

    OS_EXIT_CRITICAL();

    free(dataptr);

    return 0;


}

INT8U read_opaque(INT8U no, INT16U offset, INT16U size, INT8U *data)
{
    INT8U* flashaddr = (INT8U *) DPCONF_ADDR + no*0x1000 + 0x2000 + offset;

    if (no > 3 || (offset+size) > 4096)
        return 1;

    memcpy(data, flashaddr, size);

    return 0;

}


#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)

#if CONFIG_VERSION < IC_VERSION_DP_RevF
volatile FWSIG *fwsig = (FWSIG *) (0xA1000000);
unsigned char* SSI_MAP = (unsigned char*) FLASH_BASE_ADDR;
#endif


extern OS_EVENT *RxQ;
extern const INT32U crc_table[256];
//IPV4Cfg IPv4Cfg;
//unsigned char HostIPv6[IPv6_ADR_LEN];
//IPAddress *ASFIP;
//static INT8U *boottbl;
//ROM_EXTERN UserInfo *userhead;
ROM_EXTERN UserInfo *usertail;

ROM_EXTERN OS_EVENT *ArpTableSem;
ROM_EXTERN OS_EVENT *SktSem;

ROM_EXTERN clock_t DHCPSleepTime;
//extern INT8U *smbiosdataptr;
//extern INT8U *timestampdataptr;
extern TCPPCB *ActivePCBsList;
#ifdef CONFIG_PROFILING
extern INT32U mgtcycles[16];
#endif

//const INT8U aes_key[16];
//const INT8U aes_iv[16];

ROM_EXTERN MulticastList* MulticastListRoot = 0;

/*----------------------------Functions-------------------------------------*/

/**
* Description:	Allocate unused space for an object whose size in bytes is specified
				by size and whose value is 0
*
* Parameters:
*				size -> The size that want to be allocated
*
* Returns:		Allocate start address
*
*/
void inituser()
{
    UserInfo *tmp, *rom, *current;
    aes_context *ctx = malloc(sizeof(aes_context));

    tmp = &dpconf->admin;
    rom = (UserInfo *) DPCONF_ROM_START_ADDR;

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    aes_crypt(ctx, (INT8U*) rom, (INT8U *) tmp, AES_DECRYPT);
#else
    memset(ctx, 0, sizeof(aes_context));
    aes_crypt_sw(ctx, (INT8U*) rom, (INT8U *) tmp, AES_DECRYPT);
#endif

    if (tmp->crc16 != inet_chksum(tmp, 34))
    {
        //if null set the default one
        memset(tmp, 0, sizeof(UserInfo));
        strcpy((char *)tmp->name, "Administrator");
        strcpy((char *)tmp->passwd, "Realtek");
        tmp->role = 0x7F;
        tmp->opt  = 1;
        tmp->crc16 = inet_chksum(tmp,34);
        ///setdirty(DPCONFIGTBL);
    }
//    tmp->opaque=0;
    strcpy((char *)tmp->caption, "System Administrator");

    dirty[USERTBL].length = 0;

    tmp->next = 0;
    //copy the 1st admin to dpconf (decrypted) for PXE access
    //memcpy(&dpconf->admin, tmp, sizeof(*tmp));
    //free(tmp);

    //userhead = &dpconf->admin;
    userhead = tmp;
    current = tmp;

    //chain the user info from flash
    rom = (UserInfo *) SYSTEM_USER_INFO;

    while (1) {

        tmp = malloc(sizeof(UserInfo));
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
        aes_crypt(ctx, (INT8U *) rom, (INT8U *) tmp, AES_DECRYPT);
#else
        memset(ctx, 0, sizeof(aes_context));
        aes_crypt_sw(ctx, (INT8U *) rom, (INT8U *) tmp, AES_DECRYPT);
#endif

        //check a valid link
        if (tmp->crc16 == inet_chksum(tmp, 34) )
        {
            current->next = tmp;
            current = tmp;
            dirty[USERTBL].length++;
            if (tmp->next)
            {
                rom++;
            }
            else
            {
                usertail = tmp;
                break;
            }
        }
        else
        {
            free(tmp);
            usertail = current;
            break;
        }

    }

    free(ctx);

    dirty[USERTBL].flashaddr = (INT8U*) SYSTEM_USER_INFO;

}

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  == 1)
char *trimspace(char *str)
{
    char *end;
    str = strltrim(str);
    end = str + strlen(str);
    end = strrtrim(end);
    *end = '\0';

    return str;

}

char *strltrim(char *str)
{
    while (isspace(*str))
        str++;
    return str;
}

char *strrtrim(char *str)
{
    //while(*--str == 0x20)
    while (isspace(*--str))
        ;

    str++;
    return str;
}
#endif
#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION == 1 )
int parsebios(unsigned char val)
{
    int val32;
    val32 =  (val & 0x0f)  + ((val & 0xf0) >> 4) *10;
    return val32;
}
#endif
//#ifdef CONFIG_SMBIOS_ENABLED
//since use new getUUID method, no need to use this function
#if 0
void SMBIOSget(int type, int offset, INT8U *out)
{
    INT8U *addr;
    SMBIOSHeader sh = {0};
    addr = getSMBIOSTypeAddr(type);
    if (addr)
    {
        toSMBIOSHeader(&sh, addr);
        getSMBIOSElement(addr, &sh,  type, offset, out);
    }
    else
        out = NULL;
}
#endif

#if 1
void getGUID(INT8U *RTGUID)
{
    memcpy(RTGUID, dpconf->UUID, 16);
}

#else
void getGUID(INT8U *RTGUID)
{
    //INT8U tempout[80];
    INT8U *tempout = malloc(80);
    INT8U tempint[3];
    INT8U i= 0 , j = 0;

    SMBIOSget(SYSTEM_INFO, SYSINFO_UUID, tempout);
    if (tempout)
    {
        while (tempout[i] && !isspace(tempout[i]))
        {
            if (tempout[i] != '-')
            {
                tempint[0] = tempout[i++];
                tempint[1] = tempout[i];
                tempint[2] = '\0';
                RTGUID[j++] = atoi(tempint);
            }
            i++;
        }
    }
    else
    {
        for (j=0; j <16 ; j++)
            RTGUID[j] = 0;
    }

    free(tempout);

}
#endif

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION == 1)
void set_time(INT8U *timestampdataptr)
{
    struct tm time_tm;
    struct timeval now;

    time_tm.tm_year = parsebios(*timestampdataptr)  + 100;
    time_tm.tm_mon  = parsebios(*(timestampdataptr+1)) -1 ;
    time_tm.tm_mday = parsebios(*(timestampdataptr+2));
    time_tm.tm_hour = parsebios(*(timestampdataptr+3));
    time_tm.tm_min  = parsebios(*(timestampdataptr+4));
    time_tm.tm_sec  = parsebios(*(timestampdataptr+5));
    now.tv_sec = mktime(&time_tm);
    settimeofday(&now, 0);
}
#endif

INT8U *getpwd(INT8U *name)
{
    UserInfo *tmp = userhead;
    while (tmp)
    {
        if (strcmp((const char *)tmp->name, (const char *)name ) == 0)
            return tmp->passwd;
        tmp = tmp->next;
    }
    return NULL;
}
INT8U getrole(INT8U *name)
{
    UserInfo *tmp = userhead;
    while (tmp)
    {
        if (strcmp((const char *) tmp->name, (const char *)name ) == 0)
            return tmp->role;
        tmp = tmp->next;
    }
    return 0;
}
UserInfo* getuserinfo()
{
    return userhead;
}
UserInfo* get1stAdmin()
{
    //check admin
    UserInfo *tmp = userhead;
    while (tmp->role != 1)
    {
        tmp = tmp->next;
    }

    if (tmp->role == 1)
        return tmp;
    else
        return NULL;

}

UserInfo *adduser(INT8U *name, INT8U *passwd)
{
    UserInfo *tmp = userhead;
    UserInfo *user;
    OS_CPU_SR cpu_sr = 0;

    if (dirty[USERTBL].length == MAX_USERS)
        return NULL;
    if (strlen((const char *) name) > sizeof(userhead->name) || strlen((const char *)passwd) > sizeof(userhead->passwd))
        return NULL;

    while (tmp)
    {
        if (!strcmp((const char *)tmp->name, (const char *)name))
            return NULL;
        ///usertail=tmp;
        tmp = tmp->next;
    }

    user = malloc(sizeof(UserInfo));
    memset(user, 0, sizeof(UserInfo));
    strncpy((char *)user->name, (char *) name, sizeof(user->name));
    strncpy((char *)user->passwd, (char *)passwd, sizeof(user->passwd));
    user->crc16 = inet_chksum(user, 34);

    OS_ENTER_CRITICAL();
    usertail->next = user;
    usertail = user;
    OS_EXIT_CRITICAL();

    dirty[USERTBL].length++;
    setdirty(USERTBL);
    return user;
}

void setrole(INT8U *name, INT8U role)
{
    UserInfo *tmp = userhead;
    while (tmp)
    {
        if (strcmp((char *)tmp->name, (char *)name ) == 0)
        {
            if (tmp == userhead)
                tmp->role = 0x7F;
            else
                tmp->role = role;
            tmp->crc16 = inet_chksum(tmp, 34);
            break;
        }
        tmp = tmp->next;
    }
}
INT8U deluser(INT8U *name)
{
    UserInfo *tmp = userhead;
    UserInfo *now = NULL;
    OS_CPU_SR cpu_sr = 0;
    while (tmp)
    {
        if (strcmp((char *)tmp->name, (char *)name ) == 0)
        {
            if (tmp == userhead)
                return DASH_FAIL;
            //not allowed to delete the default user
            //if (now)
            now->next =  tmp->next;
            if (tmp == usertail)
            {
                OS_ENTER_CRITICAL();
                usertail = now;
                OS_EXIT_CRITICAL();
            }
            //else
            ///    userhead = tmp->next;
            free(tmp);
        }
        now = tmp;
        tmp = tmp->next;
    }
    dirty[USERTBL].length--;
    setdirty(USERTBL);
    return 0;
}
void moduser(INT8U *name,INT8U *passwd)
{

    UserInfo *ptr = userhead;

    while (ptr)
    {
        if (strcmp((char *)ptr->name, (char *)name ) == 0)
        {
            memset(ptr->passwd, 0 , sizeof(ptr->passwd));
            strcpy((char *)ptr->passwd,(char *)passwd);
            ptr->crc16 = inet_chksum(ptr, 34);
            break;
        }

        ptr=ptr->next;
    }

    //according to the SDK, we can not just modify a user
    //must delete the old and then create a new
    //which means that we can not modify the default system administrator
    if (ptr == userhead)
        setdirty(DPCONFIGTBL);
    else
        setdirty(USERTBL);
}

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < 4)
/* delay function for 1us */
void bsp_wait_1us(void)
{
    volatile INT32U i;
	//50 is the parameter tuning at 400M clk(after DCO)
	//32 is the parameter tuning at 250M clk(default)
    for (i = 0 ; i < bsp_wait_cnt ; i++)
		asm volatile("ssnop");
}

void bsp_wait(INT32U usec)
{
    while(usec--)
        bsp_wait_1us();
}
#endif

void setdirtyaddr(INT8U type, INT32U addr)
{
    dirty[type].flashaddr = (INT8U*) addr;
}

void setdirty(INT8U type)
{
    dirty[type].dirty = 1;
}

void setreset(INT8U type)
{
    dirty[type].reset = 1;
}

void setwait(INT8U type)
{
    dirty[type].wait = 1;

}

INT32U ifloor(INT32U a, INT32U b)
{
    if (a == 0)
        return 1;

    if (a%b)
        return (a/b + 1);
    else
        return (a/b);
}

void GPIOEnable()
{
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    REG16(IOREG_IOBASE+MAC_CPCR) |= 0x8000;
#else
    //setting GPO as output
    bsp_bits_set(MAC_GPIO, 1, BIT_GPO_OE, 1);
    bsp_bits_set(MAC_GPIO, 1, BIT_GPO_EN, 1);
#endif
}

void GPIODisable()
{
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    REG16(IOREG_IOBASE+MAC_CPCR) &= ~0x8000;
#else
    bsp_bits_set(MAC_GPIO, 0, BIT_GPO_EN, 1);
#endif
}

void GPIOSet(INT8U value)
{
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    if (value)
        REG8(IOREG_IOBASE+MAC_CMD)   |= 0x10;
    else
        REG8(IOREG_IOBASE+MAC_CMD)   &= ~0x10;
#else
    if(value)
        bsp_bits_set(MAC_GPIO, 1, BIT_GPO_I, 1);
    else
        bsp_bits_set(MAC_GPIO, 0, BIT_GPO_I, 1);

#endif

}

INT8U GPIOGet(INT8U *value)
{
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    *value = REG8(IOREG_IOBASE+MAC_CMD) & 0x10 ;
    return (REG16(IOREG_IOBASE+MAC_CPCR) & 0x8000);
#else
    return bsp_bits_get(MAC_GPIO, BIT_GPO_C, 1);
#endif
}



#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)
void ltostr(unsigned long long value, INT8U *str)
{
    //static INT8U buf[65];
    INT8U *buf = malloc(128);
    INT8U i = 63;

    memset(buf, 0, 128);
    for (; value && i; i--, value /= 10)
        buf[i] = "0123456789"[value % 10];

    strcpy((char *)str, (char *)(buf+i+1));
    free(buf);

}

void hextostr(INT8U value, INT8U *str)
{
    static const INT8U hexstr[] = "0123456789ABCDEF";
    INT8U index;

    index = ((value >> 4 ) & 0x0F);
    str[0] = hexstr[index];
    index = (value & 0x0F);
    str[1] = hexstr[index];

}

void getGUIDstr(INT8U *str)
{
    INT8U i = 0, j = 0;
    for (i = 0 ; i < 16; i++, j += 2)
    {
        if (i == 4 || i == 7 || i == 10 || i == 13)
            str[j++] = '-';
        hextostr(dpconf->UUID[i], &str[j]);
    }
    str[j] =  '\0';

}

INT32U DigitLength(INT32U value)
{
    INT8U val = 0;
    while ((value = value/10))
    {
        val++;
    }
    val++;
    return val;
}

void writesig(INT8U *buffer,INT32U sig, INT8U status)
{
    volatile FWSIG *romsig = (FWSIG *) DPCONF_ADDR;
    INT8U *SSI_MAP = (INT8U*) FLASH_BASE_ADDR;
    INT8U j = 0;
    OS_CPU_SR  cpu_sr = 0;

    memset(buffer, 0xff, 4096);
    memcpy(buffer, (INT8U *) romsig, 4096);

    ((FWSIG *)(buffer))->SIG = sig;
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    ((FWSIG *)(buffer))->Status = status;
#endif

    OS_ENTER_CRITICAL();
    spi_se_erase(SSI_MAP, (DPCONF_ADDR & 0x00FFFFFF));
    for (j = 0 ; j < 64 ; j++)
    {
        spi_write(SSI_MAP, (DPCONF_ADDR & 0x00FFFFFF)+j*64, buffer+j*64, 64);
        bsp_wdt_kick();
    }
    OS_EXIT_CRITICAL();
}

void urldecode(INT8U *ptr, INT8U *token, INT8U *dst)
{

    INT8U i = 0;
    INT8U numstr[3];
    INT8U *tmp = malloc(64);
    INT8U *start, *end;
    INT8U *str = tmp;

    if ((start = strstr((const char *)ptr, (const char *)token)))
    {
        start += strlen((const char *)token);
        end = strchr((const char *)start, '&');
        strncpy((char *)str, (char *)start, (end-start));
        str[(end-start)] = 0;
    }

    while (*str)
    {
        if (*str == '%')
        {
            strncpy((char *)numstr, (char *)str+1, 2);
            numstr[2] = 0;
            dst[i++] = strtol((const char *)numstr, NULL, 16);
            str += 3;

        }
        else
        {
            dst[i++] = *str++;
        }
    }
    dst[i] = 0;
    free(tmp);
}

#if CONFIG_VERSION >= IC_VERSION_DP_RevB
void cp3_wait(unsigned long long cycles)
{
    CP3_COUNTER scounters[4] = {0};
    CP3_COUNTER ecounters[4] = {0};
    unsigned long long diff = 0;
    rlx_cp3_init();
    rlx_cp3_start(CP3_PERFMODE2);
    rlx_cp3_get_counters(scounters);
    //rlx_cp3_print_counters(0x1a121110, scounters);

    do {
        asm("nop");

        rlx_cp3_get_counters(ecounters);
        //rlx_cp3_print_counters(0x1a121110, ecounters);
        diff = ecounters[3] - scounters[3];
    } while (diff < cycles);

    rlx_cp3_stop();

}
#endif

INT32U OSGetTimeSec()
{
    return OSTimeGet() / OS_TICKS_PER_SEC;
}

void itostr(int value, INT8U *str)
{
    static INT8U buf[33];
    INT8U i = 31;
    for (; value && i; i--, value /= 10)
        buf[i] = "0123456789"[value % 10];

    strcpy((char *)str, (char *)(buf+i+1));

}
INT32U KeyCheck(INT8U *ptr, INT8U *token, INT8U *key)
{
    INT8U *tmp, *tmpend;
    INT8U i = 0;
    INT8U numstr[3];

    if ((tmp= strstr((const char *)ptr, (const char *)token)))
    {
        tmp += strlen((char *)token);
        tmpend = strchr((const char *)tmp, '&');
        if ((tmpend - tmp) != 40)
            return 2;
    }

    for (i = 0 ; i < 20; i++)
    {
        strncpy((char *)numstr, (char *)tmp, 2);
        numstr[2] = '\0';
        tmp += 2;
        key[i] = strtol((const char *)numstr, NULL, 16);
    }

    return 0;

}
INT32U IntCheck(INT8U *ptr, INT8U *token, INT32U *value, INT8U digit)
{

    INT8U *tmp, *tmpend;
    INT8U numstr[4];

    if ((tmp = strstr((const char *)ptr, (const char *)token)))
    {
        tmp += strlen((char *)token);
        tmpend = strchr((const char *)tmp, '&');
        if ((tmpend - tmp) > digit)
            return 3;

        strncpy((char *)numstr, (char *)tmp, (tmpend-tmp));
        numstr[tmpend-tmp] = 0;
        *value = strtol(numstr, NULL, 10);
    }

    return 0;

}
#endif
#endif

void getstatestr(INT32U val, INT8U *str)
{
    switch (val)
    {
    case 0:
        strcpy(str, "Unknown");
        break;

    case 1:
        strcpy(str, "Normal");
        break;

    case 5:
        strcpy(str, "Lower Non-Critical");
        break;

    case 6:
        strcpy(str, "Lower Critical");
        break;

    case 8:
        strcpy(str, "Upper Non-Critical");
        break;

    case 9:
        strcpy(str, "Upper Critical");
        break;

    default:
        strcpy(str, "Unknown");

    }
}

void gethealthstr(INT32U val, INT8U *str)
{
    switch (val)
    {
    case 0:
        strcpy(str, "Unknown");
        break;

    case 5:
        strcpy(str, "OK");
        break;

    case 10:
        strcpy(str, "Degraded/Warning");
        break;

    case 15:
        strcpy(str, "Minor Failure");
        break;

    case 20:
        strcpy(str, "Major Failure");
        break;

    case 25:
        strcpy(str, "Critical Failure");
        break;

    case 30:
        strcpy(str, "Non-Recoverable Error");
        break;

    default:
        strcpy(str, "DMTF-Reserved");
        break;
    }
}
#endif

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
//only for VC only
void wake_up_pattern_init()
{
    WAKE_UP_t *wake_ptr = NULL, *tmp = NULL;
    WAKE_UP_t *ptr = (WAKE_UP_t *) (PATTERN_START_ADDR);

    dirty[WAKETBL].length = 0;
    while (ptr->exist == 0x01)
    {
        wake_ptr = malloc(sizeof(WAKE_UP_t));
        memcpy(wake_ptr, ptr, sizeof(WAKE_UP_t));
        wake_ptr->prev = tmp;

        if (wake_head == NULL)
            wake_head = wake_ptr;
        if (tmp)
            tmp->next = wake_ptr;

        tmp = wake_ptr;
        ptr++;
        dirty[WAKETBL].length++;
    }
    wake_tail = wake_ptr;

    dirty[WAKETBL].addr = (INT8U *) wake_head;
    dirty[WAKETBL].flashaddr = (INT8U *) PATTERN_START_ADDR;
}
void WakePtnAdd(WAKE_UP_t *ptr)
{
    OS_CPU_SR cpu_sr = 0;
    if (dirty[WAKETBL].length < 32)
    {
        if (!strcmp(ptr->name, ""))
            sprintf(ptr->name, "Rule Name %d", dirty[WAKETBL].length+1);
        OS_ENTER_CRITICAL();
        if (wake_head == NULL)
            wake_head = ptr;
        else
            wake_tail->next = ptr;

        ptr->prev = wake_tail;
        wake_tail = ptr;
        dirty[WAKETBL].length++;
        OS_ENTER_CRITICAL();
        setdirty(WAKETBL);
    }
    else
        free(ptr);

}
void WakePtnDel(WAKE_UP_t *ptr)
{
    OS_CPU_SR cpu_sr = 0;
    WAKE_UP_t *prev, *next;
    prev = ptr->prev;
    next = ptr->next;

    if (prev)
        prev->next = next;
    if (next)
        next->prev = prev;

    OS_ENTER_CRITICAL();
    if (ptr == wake_head)
        wake_head = next;
    if (ptr == wake_tail)
        wake_tail = prev;

    dirty[WAKETBL].length--;
    OS_EXIT_CRITICAL();
    setdirty(WAKETBL);
    free(ptr);
}
#endif

#if defined(CONFIG_WAKE_UP_PATTERN_ENABLED) || defined(CONFIG_BUILDROM)
WAKE_UP_t *GetWakePtnHead()
{
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    return wake_head;
    OS_EXIT_CRITICAL();
}

INT8U write_pattern(INT16U size, INT8U *data)
{
    struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
    INT32U flashaddr;
    int i = 0;
    OS_CPU_SR  cpu_sr = 0;
    INT8U *tmpptr;

    INT32U rdid = (spi_read_id(ssi_map)  & 0x00ff0000) >> 16;


    //write back data to 0x8100C000
    flashaddr = ((DPCONF_ADDR_CACHED + 0xC000) & 0x00FFFFFF);

    tmpptr = malloc(size+4);
    *(INT32U *)tmpptr = size;
    memcpy(tmpptr+4, data, size);


    OS_ENTER_CRITICAL();

    if (rdid == 0x1f)
        spi_se_unprotect(ssi_map, flashaddr);
    spi_se_erase(ssi_map, flashaddr);

    for (i =0; i < ifloor(size+2,64); i++)
        spi_write(ssi_map, flashaddr+i*64, tmpptr+i*64, 64);

    OS_EXIT_CRITICAL();

    free(tmpptr);

    return 0;


}

INT8U* lan_wake_up(INT8U *buf, INT32U size)
{
    INT8U i = 0;
    WAKE_UP_t *wake;
    INT8U *ptr;
    INT8U count = 0;

    if (bsp_get_sstate() == S_S0)
        return NULL;

    wake = GetWakePtnHead();
    while (wake)
    {
        if (wake->valid)
        {
            ptr = buf + wake->start;
            for (i = 0; i < wake->length; i++)
            {
                if ((wake->mask[i/8] >> (i%8)) & 0x01)
                {
                    if (wake->pattern[i] != ptr[i])
                        break;
                }
                if (i == (wake->length-1) )
                {

                    size |= (count << 16);
                    //write_pattern(size, buf);
                    bsp_bits_set(MAC_LAN_WAKE, 1, BIT_OOB_LANWAKE, 1);
                    //bsp_bits_set(MAC_LAN_WAKE, 0, BIT_OOB_LANWAKE, 1);
                    return buf;
                }
            }
        }
        count++;
        wake = wake->next;
    }

    return NULL;
}

INT8U  PrinterRmtl(INT8U MsgType)
{

    if (MsgType == RMCP_PWR_Off)
        bsp_bits_set_mutex(MAC_SYNC1, IB_Shutdown, BIT_RMTCTL, 3);
    else if (MsgType == RMCP_Reset)
        bsp_bits_set_mutex(MAC_SYNC1, IB_Reset, BIT_RMTCTL, 3);
    else if ((MsgType == RMCP_PWR_On) && (bsp_get_sstate() == S_S5))
        bsp_bits_set(MAC_LAN_WAKE, 1, BIT_OOB_LANWAKE, 1);
    else
        bsp_bits_set_mutex(MAC_SYNC1, IB_Reset, BIT_RMTCTL, 3);
}
#endif

#if (CONFIG_VERSION <= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
#ifdef CONFIG_DASH_ENABLED
INT8U AddRole(RoleInfo *NewRole)
{

    OS_CPU_SR  cpu_sr = 0;
    RoleInfo *tmp=rolehead;
    RoleInfo *now=NULL;

    while (tmp)
    {
        if(!strcmp(tmp->name, NewRole->name))
            return 1;
        now=tmp;
        tmp=tmp->next;
    }
    NewRole->next=NULL;

    OS_ENTER_CRITICAL();
    if (now==NULL)
        rolehead=NewRole;
    else
        now->next = NewRole;
    OS_EXIT_CRITICAL();
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    dirty_ext[ROLETBL].length++;
    dirty_ext[ROLETBL].dirty=1;
#else
    dirty[ROLETBL].length++;
    dirty[ROLETBL].dirty=1;
#endif
    return 0;
}

void init_role()
{

    RoleInfo *roletmp,*rolerom,*now=NULL;
    rolerom = (RoleInfo *) SYSTEM_ROLE_INFO;
    roletmp=rolehead;

    rolehead=NULL;

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    dirty_ext[ROLETBL].length=0;
#else
    dirty[ROLETBL].length=0;
#endif
    while(rolerom->mask <= 0x7F && rolerom->mask != 0)
    {
        roletmp = malloc(sizeof(RoleInfo));
        strcpy(roletmp->name,rolerom->name);
        roletmp->mask=rolerom->mask;
        roletmp->privilege=rolerom->privilege;
        roletmp->next=NULL;
        if (now==NULL)
            rolehead=roletmp;
        else
            now->next = roletmp;

        now=roletmp;
        rolerom++;
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
        dirty_ext[ROLETBL].length++;
#else
        dirty[ROLETBL].length++;
#endif
    }

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    dirty_ext[ROLETBL].flashaddr = (INT8U*) SYSTEM_ROLE_INFO;
#else
    dirty[ROLETBL].flashaddr = (INT8U*) SYSTEM_ROLE_INFO;
#endif
    if(!rolehead)
    {
        roletmp = malloc(sizeof(RoleInfo));
        strcpy(roletmp->name,"Role:0");
        roletmp->mask=0x1;
        roletmp->privilege=0x13;
        AddRole(roletmp);

        roletmp = malloc(sizeof(RoleInfo));
        strcpy(roletmp->name,"Role:1");
        roletmp->mask=0x2;
        roletmp->privilege=0x10000c;
        AddRole(roletmp);

        roletmp = malloc(sizeof(RoleInfo));
        strcpy(roletmp->name,"Role:2");
        roletmp->mask=0x4;
        roletmp->privilege=0xffff0;
        AddRole(roletmp);
    }

}
void init_subscribe()
{
    SubEvent *subtmp,*subrom,*now=NULL;
    INT32U tmp;
    subrom = (SubEvent *) SYSTEM_SUB_INFO;

    subhead=NULL;
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    dirty_ext[SUBTBL].length = 0;
#else
    dirty[SUBTBL].length = 0;
#endif
    while(1)
    {
        tmp=*(INT32U *)subrom;
        if(tmp==0xffffffff || tmp==0)
            break;
        subtmp = malloc(sizeof(SubEvent));
        strcpy(subtmp->subid,subrom->subid);
        strcpy(subtmp->notifyto,subrom->notifyto);
        strcpy(subtmp->query,subrom->query);
        subtmp->enable=subrom->enable;
        subtmp->heartbeatLast=subrom->heartbeatLast;
        subtmp->interval=subrom->interval;
        subtmp->next=NULL;
        if (now==NULL)
            subhead=subtmp;
        else
            now->next =subtmp;

        now=subtmp;
        subrom++;
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
        dirty_ext[SUBTBL].length=dirty_ext[SUBTBL].length+sizeof(SubEvent);
#else
        dirty[SUBTBL].length=dirty[SUBTBL].length+sizeof(SubEvent);
#endif
    }
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    dirty_ext[SUBTBL].flashaddr = (INT8U*) SYSTEM_SUB_INFO;
#else
    dirty[SUBTBL].flashaddr = (INT8U*) SYSTEM_SUB_INFO;
#endif
}


INT8U DeleteRole(INT8U count)
{
    UserInfo *tmp_user=userhead;
    RoleInfo *tmp=rolehead;
    RoleInfo *now = NULL;
    OS_CPU_SR  cpu_sr = 0;
    INT8U i=0;
    while (tmp)
    {
        if(i==count)
        {
            while(tmp_user)
            {
                if(tmp_user!=userhead)
                    tmp_user->role=tmp_user->role & (~(tmp->mask));
                tmp_user=tmp_user->next;
            }

            OS_ENTER_CRITICAL();
            if (now)
                now->next =  tmp->next;
            else
                rolehead = tmp->next;
            OS_EXIT_CRITICAL();

            free(tmp);
            break;
        }
        i++;
        now = tmp;
        tmp = tmp->next;
    }
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    dirty_ext[ROLETBL].length--;
    dirty_ext[ROLETBL].dirty=1;
#else
    dirty[ROLETBL].length--;
    dirty[ROLETBL].dirty=1;
#endif
    setdirty(USERTBL);
}
#endif
#endif
#if CONFIG_VERSION  > IC_VERSION_DP_RevF
void setdirtylen(INT8U type, INT32U length, INT8U mode)
{
    if(mode==DIRTY_ADD)
        dirty[type].length += length;
    else if (mode==DIRTY_SUB)
        dirty[type].length -= length;
    else
        dirty[type].length = length;
}
#endif



#define FLASH_START_ADDR FLASH_DATA_ADDR
#define FLASH_SECTOR_LEN 4096
#define FLASH_PAGE_LEN 256

void writeflash(INT32U flashaddr,INT8U *data, INT32U length) {
    INT32U numwrite;
    unsigned char *ssi_map = (unsigned char *)flashaddr;
    INT32U idx = 0;
    INT32U rdid = (spi_read_id(ssi_map)&0x00ff0000) >> 16;
    OS_CPU_SR  cpu_sr = 0;
    INT8U *buffer;
    INT32U secAddr=0,secAddr2=0,endAddr=0,offset=0,len1=0;
   

    flashaddr &= (0x00FFFFFF);

    OS_ENTER_CRITICAL();

    if(rdid == 0x1f) {	 
        spi_se_unprotect(ssi_map,(INT32U)flashaddr);
    }  
   

	
	buffer = malloc(FLASH_SECTOR_LEN);	
	if(buffer){
		endAddr = flashaddr+length;
		offset = flashaddr&0x0FFF;
		secAddr = flashaddr&0xFFFFF000;	
		secAddr2 = endAddr&0xFFFFF000;	
		if(secAddr == secAddr2){			
			memcpy(buffer,FLASH_START_ADDR+secAddr,FLASH_SECTOR_LEN);
			memcpy(buffer + offset, data  ,length);
			 spi_se_erase(ssi_map,(INT32U)secAddr);			 
			 numwrite = FLASH_SECTOR_LEN/FLASH_PAGE_LEN;			 			 
			 
			 for(idx=0; idx<numwrite; idx++) {			 	
			        	spi_write(ssi_map, (INT32U)secAddr + idx*FLASH_PAGE_LEN , buffer + (idx*FLASH_PAGE_LEN), FLASH_PAGE_LEN);
			 }
			 
		}else{
			//Sector 1 :
			memcpy(buffer,FLASH_START_ADDR+secAddr,FLASH_SECTOR_LEN);
			len1 = FLASH_SECTOR_LEN-offset;
			memcpy(buffer + offset, data  ,length);
			 spi_se_erase(ssi_map,(INT32U)secAddr);
			 numwrite = FLASH_SECTOR_LEN/FLASH_PAGE_LEN;		
			 
			 for(idx=0; idx<numwrite; idx++) {
			        spi_write(ssi_map, (INT32U)secAddr + idx*FLASH_PAGE_LEN , buffer + (idx*FLASH_PAGE_LEN), FLASH_PAGE_LEN);
			 }
			 //sector 2
			 memcpy(buffer,FLASH_START_ADDR+secAddr2,FLASH_SECTOR_LEN);			 			
			 memcpy(buffer , data+ len1  ,(length -len1));
			 spi_se_erase(ssi_map,(INT32U)secAddr2);
			 len1 = length - len1;
			  numwrite = FLASH_SECTOR_LEN/FLASH_PAGE_LEN;		
 			for(idx=0; idx<numwrite; idx++) {
 			        spi_write(ssi_map, (INT32U)secAddr2 + idx*FLASH_PAGE_LEN , buffer + (idx*FLASH_PAGE_LEN), FLASH_PAGE_LEN);
 			 }
		}

		free(buffer);
	}else{
		
		 spi_se_erase(ssi_map,(INT32U)flashaddr);
	       numwrite = (length + (FLASH_PAGE_LEN-1)) /FLASH_PAGE_LEN;
	    	for(idx=0; idx<numwrite; idx++) {
			if( (((INT32U)flashaddr + idx*FLASH_PAGE_LEN)%0x1000) == 0x00 ){						
				spi_se_erase(ssi_map,(INT32U)flashaddr + idx*FLASH_PAGE_LEN);
			}	
			if((idx != (numwrite-1)) || (length%FLASH_PAGE_LEN == 0)){		
		        	spi_write(ssi_map, (INT32U)flashaddr + idx*FLASH_PAGE_LEN , data + (idx*FLASH_PAGE_LEN), FLASH_PAGE_LEN);
			}else{
				spi_write(ssi_map, (INT32U)flashaddr + idx*FLASH_PAGE_LEN , data + (idx*FLASH_PAGE_LEN), length%FLASH_PAGE_LEN);
			}		
	    	}
	}
    OS_EXIT_CRITICAL();
}


#if 0
void test(){
	INT32U a;
	a = bsp_bits_get(MAC_FLAG_IB ,0, 1);
	
	printf("[RTK] + testfunction ,a=0x%x\n",a);
	if((bsp_bits_get(MAC_FLAG_IB,0,1)) == 0x00){
		printf("[RTK] yes i am 0x00\n");
	}

	
	while((bsp_bits_get(MAC_FLAG_IB,0,1)) == 0x00 ){

		printf("[RTK] test 1\n");
		OSTimeDly(100);
	}

	printf("[RTK] Done\n");

	
}
#endif
//how to avoid diffent task in OOB , access this mutex simultaneously ?
void AcquireIbOobMutex(){

	INT32U oobflag = 0,ibFlag =0;
	
	//set flag[1] = true;
	bsp_bits_set(MAC_FLAG_OOB ,1,0, 1);
	
	//while (flag[0] == true){
	while(bsp_bits_get(MAC_FLAG_IB ,0, 1) == 0x01 ){
		//if (turn ¡Ú 1)
		if(bsp_bits_get(MAC_MUTEX_TURN ,0, 1) == 0x01){
			
			//flag[1] = false;
			bsp_bits_set(MAC_FLAG_OOB ,0,0, 1);
				
			//while (turn ¡Ú 0)
			while(bsp_bits_get(MAC_MUTEX_TURN ,0, 0) != 0x01){
				// busy wait
				printf("[RTK] busy wait\n");
				OSTimeDly(10);
				
			}
			//flag[1] = true;			
			bsp_bits_set(MAC_FLAG_OOB ,1 ,0, 1);
			
		}
	}

}
void ReleaseIbOobMutex(){
	INT32U turn = 0,oobflag=0;
	INT32U flagBit = 0x01;
	
	//turn    = 0;
	bsp_bits_set(MAC_MUTEX_TURN ,0 ,0, 1);
   	//flag[1] = false;
	bsp_bits_set(MAC_FLAG_OOB ,0 ,0, 1);

}

void setSleepDeep(int deep){
	// 0 means always sleep
	// 1 means no sleep
	// 10 means --> no sleep 1/10 ,  sleep 9/10
	
	sleepControler = deep;
	
}


void TCRDisable(){
	setSleepDeep(0);
	bsp_bits_set(0xB0,1,0,1);
}

void TCREnable(){
	setSleepDeep(2);
	bsp_bits_set(0xB0,0,0,1);
}

#if 1
int getFwLogParser(eventdata *ed, unsigned char* buf)
{
    int rv = 0;
    int offset = 0;
	// [From who][what happen]
	
	if(ed->logtype == LOG_INFO ){
		if(ed->Sensor_Device == DETAIL){
			 rv = sprintf(buf + offset, "Common Info [0x%x][0x%x] .(%d) \n" , ed->Event_Source_Type ,ed->Sensor_Device ,ed->Sensor_Number );	 
		}else{
			 rv = sprintf(buf + offset, "Common Info [0x%x][0x%x]  \n" , ed->Event_Source_Type ,ed->Sensor_Device );	 
		}
	}
	else if(ed->logtype == LOG_ERROR){
		 rv = sprintf(buf + offset, "Common Alert [0x%x][0x%x] \n" , ed->Event_Source_Type,ed->Sensor_Device  );	 
	}else{
		rv = sprintf(buf + offset, "Common Message [0x%x][0x%x] \n" , ed->Event_Source_Type,ed->Sensor_Device  );	 
	}
	
	 
	return rv;
}
#endif




