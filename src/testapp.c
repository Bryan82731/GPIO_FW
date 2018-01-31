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
#include "dhcp.h"
#include "rtskt.h"
#include "client.h"
#include "wcom.h"
#include "pldm.h"
#include "dns.h"
#include "soap.h"

OS_STK  AppTaskStartStk[TASK_START_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TaskWSMANStk[TASK_WSMAN_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TaskWSMANTLSStk[TASK_WSMAN_TLS_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TCPTimerStk[TASK_TCP_TIMER_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TaskTCPStk[TASK_TCP_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  DHCPv4Stk[TASK_DHCPv4_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  OOBResponseStk[TASK_OOBResponse_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TaskFlashStk[TASK_FLASH_STK_SIZE] _ATTRIBUTE_STK;

OS_STK  ClientC1Stk[TASK_CLIENT_C1_STK_SIZE];

static void AppTaskStart (void *p_arg);
void TCPTask_R(void *p_arg);
void tcpTimerTask_R(void *data);
void DHCPv4Task_R(void *data);
void initTCP_R();
void OOBResponseTask_R(void *p_arg);
void ProvSrvROM(void *data);

extern OS_EVENT *RxQ;
extern DPCONF *dpconf;
extern INT32U sizearr[PAR_SIZE];
extern INT32U parsize[PAR_SIZE];
extern FWSIG *fwsig;

//void (*fwUpdateInit_F)();
//void (*fwInituser_F)();
//void (*fwUpdate_F)(void* pData, int rxLen, struct tcp_pcb* pcb);
//void (*initTCP_F)();
/*
extern void (*TCPTask_F)(void *p_arg);
extern void (*DHCPv4Task_F)(void *p_arg);
extern void (*tcpTimerTask_F)(void *p_arg);
extern void (*OOBResponseTask_F)(void *p_arg);
*/

int main(void) _ATTRIBUTE_INIT ;


int
main (void)
{
    INT8U err;
    INT16U i = 0;
    INT8U* tmp;
    struct ssi_portmap *ssi_map;
    INT8U provisioningstate = 0;
    INT32U flashaddr;

    /* Initialize "uC/OS-II, The Real-Time Kernel"              */
    OSInit();

    /* Create the start task                                    */
    OSTaskCreateExt (AppTaskStart,
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

static void AppTaskStart (void *p_arg)
{
    (void) p_arg;
    eventdata *pevent;

    /* Initialize BSP functions                                 */
    bsp_setup();
#ifdef CONFIG_BUILDROM
    ///dummy_call();
#endif

#if OS_TASK_STAT_EN > 0
    OSStatInit();                          /* Determine CPU capacity                                   */
#endif


    initTCP_R();
    inituser();

    OSTaskCreateExt (FlashTask,
                     (void *) 0,
                     (OS_STK *) & TaskFlashStk[TASK_FLASH_STK_SIZE - 1],
                     TASK_FLASH_PRIO,
                     TASK_FLASH_ID,
                     (OS_STK *) & TaskFlashStk[0],
                     TASK_FLASH_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt (TCPTask_R,
                     (void *) 0,
                     (OS_STK *) & TaskTCPStk[TASK_TCP_STK_SIZE - 1],
                     TASK_TCP_PRIO,
                     TASK_TCP_ID,
                     (OS_STK *) & TaskTCPStk[0],
                     TASK_TCP_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt (DHCPv4Task_R,
                     (void *) 0,
                     (OS_STK *) & DHCPv4Stk[TASK_DHCPv4_STK_SIZE - 1],
                     TASK_DHCPv4_PRIO,
                     TASK_DHCPv4_ID,
                     (OS_STK *) & DHCPv4Stk[0],
                     TASK_DHCPv4_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt (tcpTimerTask_R,
                     (void *) 0,
                     (OS_STK *) & TCPTimerStk[TASK_TCP_TIMER_STK_SIZE - 1],
                     TASK_TCP_TIMER_PRIO,
                     TASK_TCP_TIMER_ID,
                     (OS_STK *) & TCPTimerStk[0],
                     TASK_TCP_TIMER_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt (OOBResponseTask_R,
                     (void *) 0,
                     (OS_STK *) & OOBResponseStk[TASK_OOBResponse_STK_SIZE - 1],
                     TASK_OOBResponse_PRIO,
                     TASK_OOBResponse_ID,
                     (OS_STK *) & OOBResponseStk[0],
                     TASK_OOBResponse_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    if (dpconf->ProvisioningState != PROVISIONED)
    {
        if (fwsig->provPatch == NEED_PATCH)
            OSTaskCreateExt (fwsig->provpatch,
                             (void *) 0,
                             (OS_STK *) & TaskWSMANStk[TASK_WSMAN_STK_SIZE - 1],
                             TASK_Provisioning_PRIO,
                             TASK_Provisioning_ID,
                             (OS_STK *) & TaskWSMANStk[0],
                             TASK_WSMAN_STK_SIZE,
                             (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

        else
            OSTaskCreateExt (ProvSrvROM,
                             (void *) 0,
                             (OS_STK *) & TaskWSMANStk[TASK_WSMAN_STK_SIZE - 1],
                             TASK_Provisioning_PRIO,
                             TASK_Provisioning_ID,
                             (OS_STK *) & TaskWSMANStk[0],
                             TASK_WSMAN_STK_SIZE,
                             (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    }
        OSTaskCreateExt (wsmanSrv,
                         (void *) 0,
                         (OS_STK *) & TaskWSMANStk[TASK_WSMAN_STK_SIZE - 1],
                         TASK_WSMAN_PRIO,
                         TASK_WSMAN_ID,
                         (OS_STK *) & TaskWSMANStk[0],
                         TASK_WSMAN_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

        OSTaskCreateExt (wsmanSrvTLS,
                         (void *) 0,
                         (OS_STK *) & TaskWSMANTLSStk[TASK_WSMAN_TLS_STK_SIZE - 1],
                         TASK_WSMAN_TLS_PRIO,
                         TASK_WSMAN_TLS_ID,
                         (OS_STK *) & TaskWSMANTLSStk[0],
                         TASK_WSMAN_TLS_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

        OSTaskCreateExt (clientTask1,
                         (void *) 0,
                         (OS_STK *) & ClientC1Stk[TASK_CLIENT_C1_STK_SIZE-1],
                         TASK_CLIENT1_PRIO,
                         TASK_CLIENT1_ID,
                         (OS_STK *) & ClientC1Stk[0],
                         TASK_CLIENT_C1_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);


    while (OS_TRUE)
    {

        //kick the dog every second (would expire after 38 seconds)
        bsp_wdt_kick();
        inctimeofday(1);
        OSTimeDly(OS_TICKS_PER_SEC);
    }
}
