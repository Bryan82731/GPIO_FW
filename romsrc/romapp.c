/*
********************************************************************************
* uC/OS-II
* The Real-Time Kernel
*
* (c) Copyright 1998-2006, Micrium, Weston, FL
* All Rights Reserved
*
*
* MIPS Sample code
*
* File : APP.C
********************************************************************************
*/
#define EXTERN

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/ucos_ii.h>
#include "bsp.h"
#include "snmp.h"
#include "app_cfg.h"
#include "arp.h"
#include "lib.h"
#include "http.h"
#include "icmp.h"
#include "rmcp.h"
#include "rtskt.h"
#include "client.h"
#include "wcom.h"
#include "pldm.h"
#include "dns.h"
#include "soap.h"
#include "fwupdate.h"

extern OS_STK  TCPTimerStk[TASK_TCP_TIMER_STK_SIZE];
extern OS_STK  TaskTCPStk[TASK_TCP_STK_SIZE];
extern OS_STK  AppTaskStartStk[TASK_START_STK_SIZE];
extern OS_STK  TaskWSMANStk[TASK_WSMAN_STK_SIZE];
extern OS_STK  DHCPv4Stk[TASK_DHCPv4_STK_SIZE];
extern OS_STK  TaskFlashStk[TASK_FLASH_STK_SIZE];

static void AppTaskStart_R (void *p_arg);
void tcpTimerTask(void *data);
void DHCPv4Task(void *data);
void initTCP_R();
void provisioningSrv(void *data);

extern OS_EVENT *RxQ;
extern DPCONF *dpconf;
volatile FWSIG *fwsig = (FWSIG *) DPCONF_ADDR;

static void TCPTask_R(void *p_arg);

void  OSTaskIdleHook (void) {}

void initPatchFun_R()
{
   OSTaskIdleHook_F = OSTaskIdleHook;
   GetMacAddr_F = GetMacAddr;
    smbus_send_F = smbus_send;
    smbus_hook_F = smbus_hook;
    smbus_asfpoll_F = smbus_asfpoll;
    BootOptions_F = SMBus_Prepare_BootOptions;
    gmacsend_F = gmacsend;
    rx_recycle_F = rx_recycle;
    gmac_software_hook_F = gmac_software_hook;
    ///gmac_pcie_hook_F = gmac_pcie_hook;

    rtSkt_F = rtSkt;
    EnableDrvWaitOOB_F = EnableDrvWaitOOB;
    DisableDrvWaitOOB_F = DisableDrvWaitOOB;
    //getPortNumber_F = getPortNumber;
    //getUDPAddressInfo_F = getUDPAddressInfo;
    rtClientSkt_F = rtClientSkt;
    //rtGethostbyname_F = rtGethostbyname;
    //rtSktAccept_F = rtSktAccept;
    rtSktBind_F = rtSktBind;
    rtSktClose_F = rtSktClose;
    rtSktCloseSend_F = rtSktCloseSend;
    //rtSktConnect_F = rtSktConnect;
    rtSktEnableIBPortFilter_F = rtSktEnableIBPortFilter;
    rtSktEnableOOBPortFilter_F = rtSktEnableOOBPortFilter;
    rtSktEnableTimeWait_F = rtSktEnableTimeWait;
    rtSktListen_F = rtSktListen;
    rtSktReuse_F = rtSktReuse;
    rtSktRx_F = rtSktRx;
    rtSktSend_F = rtSktSend;
    rtSktSetUDPInfo_F = rtSktSetUDPInfo;
    rtSktShutdown_F = rtSktShutdown;
    rtSktUDPBind_F = rtSktUDPBind;
    //rtSktUDPSendTo_F = rtSktUDPSendTo;
    //hwpf
    RstSharePFRuleMem_F = RstSharePFRuleMem;
	SetOOBBasicRule_F = SetOOBBasicRule;
	setPFRule_F = setPFRule;
	setTCAMData_F = setTCAMData;

    //TCP
    //addTcpHeader_F = addTcpHeader;
    postListenPort_F = postListenPort;
    removePCB_F = removePCB;
    removeSerivce_F = removeSerivce;
#if CONFIG_VERSION	>= IC_VERSION_EP_RevA
	SetIBPF_F =	SetIBPF;
	SetOOBPF_F = SetOOBPF;
    reserveIBPort_F = reserveIBPort;
    reloadFilterTable_F = reloadFilterTableEP;
    removePortFilterTable_F = removePortFilterTableEP;
    setPortFilterTable_F = setPortFilterTableEP;
	setNewIBTCAMPortPF_F = setNewIBTCAMPortPF;
#elif CONFIG_VERSION == IC_VERSION_DP_RevF
    reserveIBPort_F = reserveIBPort;
    reloadFilterTable_F = reloadFilterTable;
    removePortFilterTable_F = removePortFilterTable;
#endif

    resetIPPtl_F = resetIPPtl;
    tcpClose_F = tcpClose;
    tcpConnectionIn_F = tcpConnectionIn;
    //tcpDequeue_F = tcpDequeue;
    tcpEnqueue_F = tcpEnqueue;
    //tcpInput_F = tcpInput;
    tcpListen_F = tcpListen;
    tcpPCBNew_F = tcpPCBNew;
    tcpPCBRegister_F = tcpPCBRegister;
    tcpPCBUnRegister_F = tcpPCBUnRegister;
    //tcpPeriodChk_F = tcpPeriodChk;
    tcpProbe_F = tcpProbe;
    //tcpProcess_F = tcpProcess;
    tcpRx_F = tcpRx;
    //tcpRxInput_F = tcpRxInput;
    tcpSend_F = tcpSend;
    tcpSendData_F = tcpSendData;
    v6pseudochksum_F = v6pseudochksum;
	//DHCP
	dhcpSend_F = dhcpSend;
	dhcpInput_F = dhcpInput;
	dhcpOptions_F = dhcpOptions;
	dhcpGetOptionsValue_F = dhcpGetOptionsValue;
	dhcpReply_F = dhcpReply;
	DHCPv4TimeOut_F = DHCPv4TimeOut;

    //icmp
    icmpInput_F = icmpInput;	
    //Udp
    udpInput_F = udpInput;
    udpSend_F = udpSend;
    //IP
    ipInput_F = ipInput;
    addIPHeader_F = addIPHeader;

    //arp
    arpTableTimeOutReset_F = arpTableTimeOutReset;
    arpTableTimeOutUpdate_F= arpTableTimeOutUpdate;
    findEtherAddr_F = findEtherAddr;
    //findIPv6EtherAddr_F = findIPv6EtherAddr;
    sendARPPkt_F = sendARPPkt;
    updateArpTable_F = updateArpTable;

    //lib
    addEthernetHeader_F = addEthernetHeader;
    addMulticasAddress_F = addMulticasAddress;

    allocIPPkt_F = allocIPPkt;
    allocPkt_F = allocPkt;
    allocUDPPkt_F = allocUDPPkt;
    allocSSHPkt_F = allocSSHPkt;

    freePkt_F = freePkt;
    sendPkt_F = sendPkt;

    getDNSMAR_F = getDNSMAR;
    getGateWayMAR_F = getGateWayMAR;
    getIPAddress_F = getIPAddress;
    getIPv6Address_F = getIPv6Address;

    resetMulticasAddress_F = resetMulticasAddress;

    sendTCPData_F = sendTCPData;
    setDNSIP_F = setDNSIP;
    setGateWayIP_F = setGateWayIP;
    setIPAddress_F = setIPAddress;
    setIPv6Address_F = setIPv6Address;
    setIPv6DNS_F = setIPv6DNS;
    setIPv6GateWay_F = setIPv6GateWay;
    setLinkLocalAddress_F = setLinkLocalAddress;
    setMatchSubnetMask_F = setMatchSubnetMask;
    setMulticastList_F = setMulticastList;
    setSubnetMask_F = setSubnetMask;
    stopconn_F = stopconn;
}

int
ROMmain (void)
{
    INT8U err;

    /* Disable all interrupts until we are ready to accept them */
    bsp_irq_disable();

    /* Initialize "uC/OS-II, The Real-Time Kernel"              */
    OSInit();

    /* Create the start task                                    */
    OSTaskCreateExt (AppTaskStart_R,
                     (void *) 0,
                     (OS_STK *) & AppTaskStartStk[TASK_START_STK_SIZE - 1],
                     TASK_START_PRIO,
                     TASK_START_ID,
                     (OS_STK *) & AppTaskStartStk[0],
                     TASK_START_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#if OS_TASK_NAME_SIZE > 13
    OSTaskNameSet (TASK_START_PRIO, "Startup", &err);
#endif

    /* Start multitasking (i.e. give control to uC/OS-II) */
    OSStart ();

    return 0;
}

/*
********************************************************************************
* STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text,
*               you MUST initialize the ticker only once multitasking has started.
* Arguments   : p_arg   is the argument passed to 'AppStartTask()' by 'OSTaskCreate()'.
********************************************************************************
*/
static void AppTaskStart_R (void *p_arg)
{
    (void) p_arg;
	
    //copy the default configuration into asfconfig
    dpconf = malloc(sizeof(DPCONF));
    memcpy((void *) dpconf, (void *) DPCONF_ROM_START_ADDR, sizeof(DPCONF));
    dpconf->restart = 0;

    bsp_cmac_init(); 
    /* Initialize BSP functions                                 */
    bsp_setup_R();
	hwPFInit();

    //initPatchFun_R();

#if OS_TASK_STAT_EN > 0
    OSStatInit();                          /* Determine CPU capacity                                   */
#endif

    //delay before GMAC/GPHY ready, auto negotiation needs 2 secs
    //OSTimeDly(3*OS_TICKS_PER_SEC);

    /* Initialize TCP/IP parameters*/

    initTCP_R();

    fwUpdateInit_R();

#if 0
    OSTaskCreateExt (FlashTask,
                     (void *) 0,
                     (OS_STK *) & TaskFlashStk[TASK_FLASH_STK_SIZE - 1],
                     TASK_FLASH_PRIO,
                     TASK_FLASH_ID,
                     (OS_STK *) & TaskFlashStk[0],
                     TASK_FLASH_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

    OSTaskCreateExt (TCPTask_R,
                     (void *) 0,
                     (OS_STK *) & TaskTCPStk[TASK_TCP_STK_SIZE - 1],
                     TASK_TCP_PRIO,
                     TASK_TCP_ID,
                     (OS_STK *) & TaskTCPStk[0],
                     TASK_TCP_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt (DHCPv4Task,
                     (void *) 0,
                     (OS_STK *) & DHCPv4Stk[TASK_DHCPv4_STK_SIZE - 1],
                     TASK_DHCPv4_PRIO,
                     TASK_DHCPv4_ID,
                     (OS_STK *) & DHCPv4Stk[0],
                     TASK_DHCPv4_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt (tcpTimerTask,
                     (void *) 0,
                     (OS_STK *) & TCPTimerStk[TASK_TCP_TIMER_STK_SIZE - 1],
                     TASK_TCP_TIMER_PRIO,
                     TASK_TCP_TIMER_ID,
                     (OS_STK *) & TCPTimerStk[0],
                     TASK_TCP_TIMER_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	
    while (OS_TRUE)
    {

        //kick the dog every second (would expire after 38 seconds)
        bsp_wdt_kick();
        inctimeofday(1);
        OSTimeDly(OS_TICKS_PER_SEC);	
    }
}

void initTCP_R()
{

    ArpTableSem = OSSemCreate(1);
    SktSem = OSSemCreate(1);

    ActivePCBsList = tcpPCBNew_F();


    if (!intfDHCPv4Enable(eth0))
    {
        if (intfIPv4Enable(eth0))
        {
            setMatchSubnetMask_F(eth0);
        }
    }

    srand(REG32(TIMER_IOBASE + TIMER_CV));

}

void TCPTask_R(void *p_arg)
{
    (void) p_arg;

    INT8U *rxbuf;
    INT8U err;
    PKT* pkt;

    while (OS_TRUE)
    {

        rxbuf = (INT8U *) OSQPend(RxQ,0,&err);
		
        if (rxbuf == NULL)
            continue;

        if (*rxbuf == ARP_PKT)
        {
            sendARPPkt_F((ARPPkt*)rxbuf, 0, ARP_Reply, *(rxbuf + 1));
            free(rxbuf);
        }
        else
        {
            pkt =  (PKT*) malloc(PKT_SIZE);
            memset(pkt, 0, PKT_SIZE);
            pkt->start = rxbuf;
			pkt->intf = *(rxbuf + 1);
            pkt->wp = rxbuf + ETH_HDR_SIZE;

            OSSemPend(SktSem, 0, &err);
            ipInput_F(pkt);
            if (pkt)
            {
                if (!pkt->queued)
                    freePkt_F(pkt);
            }
            OSSemPost(SktSem);
        }
    }
}

