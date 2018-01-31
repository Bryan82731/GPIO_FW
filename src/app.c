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
#include "mDNS.h"
#include "soap.h"
#include "debug.h"
#include "smbios.h"
#include "telnet.h"
#include "ssh.h"
#include "eap.h"
#include "cmac.h"

#include "rkvm.h"

#if CONFIG_VERSION  >= IC_VERSION_FP_RevA
extern void init_NIC_SD(void);
extern void init_testItem_SD(void);
extern void bsp_kcsTest_handler(void);
extern void VPD_RW_subHandler(void);
extern void bsp_Fun0_subHandler(INT16U isr_tmp);
extern int rtkehci_int_handler_test();

#endif

#if (CONFIG_VERSION  >= IC_VERSION_EP_RevB)
#include "usb.h"
extern USBCB usbcb;
#endif

#ifdef CONFIG_MAIL_WAKEUP_ENABLED
#include "mail_wakeup.h"
#endif

#if CONFIG_VNC_ENABLED
#include "vnc.h"
#include "vga.h"
#endif

#ifdef CONFIG_TCR_ENABLED
//OS_STK  TaskSOLRxStk[TASK_SOL_RX_STK_SIZE] _ATTRIBUTE_STK;
#endif

#ifdef CONFIG_SMBUSTRA_DEBUG
OS_STK TaskSMBUSLOGStk[TASK_SMBUSLOG_STK_SIZE];
extern OS_EVENT *SMBUSLOGQ;
#endif

#ifdef CONFIG_USB_ENABLED
OS_STK  TaskUSBStk[TASK_USB_STK_SIZE] _ATTRIBUTE_STK;
void USBSrvTask(void *data);
#endif

OS_STK  TCPTimerStk[TASK_TCP_TIMER_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TaskTCPStk[TASK_TCP_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  AppTaskStartStk[TASK_START_STK_SIZE] _ATTRIBUTE_STK;

#ifdef CONFIG_DASH_ENABLED
OS_STK  TaskWSMANStk[TASK_WSMAN_STK_SIZE] _ATTRIBUTE_STK;
//OS_STK  TaskWSMANIPv6Stk[TASK_WSMAN_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TaskWSMANTLSStk[TASK_WSMAN_TLS_STK_SIZE] _ATTRIBUTE_STK;
//OS_STK  TaskWSMANTLSIPv6Stk[TASK_WSMAN_TLS_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  IndicationStk[TASK_INDICATION_STK_SIZE] _ATTRIBUTE_STK;
#endif


OS_STK  TaskSNRReadStk[TASK_SNR_READ_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TaskSMBUSStk[TASK_SMBUS_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TaskFlashStk[TASK_FLASH_STK_SIZE] _ATTRIBUTE_STK;

#ifdef CONFIG_ASF_ENABLED
OS_STK  TaskASFV1Stk[TASK_ASF_V1_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TaskASFV2Stk[TASK_ASF_V2_STK_SIZE] _ATTRIBUTE_STK;
OS_STK  TaskSNMPStk[TASK_SNMP_STK_SIZE] _ATTRIBUTE_STK;
#endif

#ifdef CONFIG_DHCP_ENABLED
OS_STK  DHCPv4Stk[TASK_DHCPv4_STK_SIZE] _ATTRIBUTE_STK;
#ifdef CONFIG_IPv6_ENABLED
OS_STK  DHCPv6Stk[TASK_DHCPv6_STK_SIZE] _ATTRIBUTE_STK;
#endif
#endif

#if CONFIG_VERSION > 1
OS_STK	OOBResponseStk[TASK_OOBResponse_STK_SIZE] _ATTRIBUTE_STK;
#endif

#if CONFIG_PCI_Bridge_Test
OS_STK	OOBTestStk[TASK_OOBTest_STK_SIZE] _ATTRIBUTE_STK;
#endif

#if CONFIG_IPERF_Test || defined (CONFIG_UDPSRV_Test) || defined (CONFIG_Client_Test)
OS_STK	IperfTestStk[TASK_Iperf_STK_SIZE] _ATTRIBUTE_STK;
#endif

#if  CONFIG_WEB_SKT_STYLE || defined (CONFIG_EZSHAE_TASK_ENABLED)
OS_STK  WebSrvStk[TASK_WEB_SRV_STK_SIZE] _ATTRIBUTE_STK;
#endif

#ifdef  CONFIG_mDNS_OFFLOAD_ENABLED
OS_STK  mDNSSrvStk[TASK_mDNS_SRV_STK_SIZE] _ATTRIBUTE_STK;
#endif

#if defined (CONFIG_TELNET_ENABLED) || defined (CONFIG_SSH_ENABLED)
OS_STK  TelnetSrvStk[TASK_TELNET_SRV_STK_SIZE] _ATTRIBUTE_STK;
#endif
#if defined (CONFIG_8021X_ENABLED) && CONFIG_VERSION >= IC_VERSION_DP_RevF
OS_STK Task8021xStk[TASK_8021X_STK_SIZE] _ATTRIBUTE_STK;
#endif

#if CONFIG_TEREDO_ENABLED
OS_STK  TeredoRxSrvStk[TASK_TEREDO_RX_SRV_STK_SIZE];
#endif

//Bill
#if CONFIG_MAIL_WAKEUP_ENABLED
OS_STK  MailWakeUpStk[TASK_MAIL_WAKE_UP_SRV_STK_SIZE];
#endif

#if CONFIG_VNC_ENABLED
OS_STK VNCSrvStk[TASK_VNC_SRV_STK_SIZE];
#endif

//combine test items @melody 20160808
#ifdef CONFIG_PCIE_ENABLED
OS_STK PCIDevDrvStk[TASK_PCI_DEV_DRV_STK_SIZE];
extern void PCIDevDrvTask();
#endif

#ifdef CONFIG_OOBMAC_TEST_ENABLED
OS_STK OOBMACTestStk[TASK_OOBMAC_TEST_STK_SIZE];
extern void OOBMAC_Test_Task();
#endif

#ifdef CONFIG_RKVM_TEST_ENABLED
OS_STK RKVMTestStk[TASK_RKVM_TEST_STK_SIZE];
extern void RKVM_Test_Task();
#endif

#ifdef CONFIG_KCS_TEST_ENABLED
OS_STK KCSTestStk[TASK_KCS_TEST_STK_SIZE];
extern void KCS_Test_Task();
#endif

#ifdef CONFIG_CMAC_TEST_ENABLED
OS_STK CMACTestStk[TASK_CMAC_TEST_STK_SIZE];
extern void CMAC_Test_Task();
#endif

#ifdef CONFIG_VPD_TEST_ENABLED
OS_STK VPDTestStk[TASK_VPD_TEST_STK_SIZE];
extern void VPD_Test_Task();
#endif

#ifdef CONFIG_DDR_TEST_ENABLED
extern void	DDR_Test_Task();
#endif

#ifdef CONFIG_VGA_TEST_ENABLED
OS_STK VGAStk[TASK_VGA_TEST_STK_SIZE];
extern void VGA_Test_Task();
#endif

#ifdef CONFIG_EHCI_TEST_ENABLED
OS_STK EHCIStk[TASK_EHCI_TEST_STK_SIZE];
extern void EHCI_Test_Task();
#endif

#ifdef CONFIG_TCR_TEST_ENABLED
OS_STK TCRStk[TASK_TCR_TEST_STK_SIZE];
extern void TCR_Test_Task();
#endif

#ifdef CONFIG_SMBUS_TEST_ENABLED
OS_STK SMBUSTestStk[TASK_SMBUS_TEST_STK_SIZE];
extern void SMBUS_Test_Task();
#endif

//OS_STK MMDTestStk[TASK_MMD_TEST_STK_SIZE]_ATTRIBUTE_STK;
extern void MMD_Test_Task();


#ifdef CONFIG_PROFILING
extern INT32U mgtcycles[16];
#endif
extern OS_EVENT *RxQ, *SMBQ, *SNMPQ;
extern OS_EVENT *EVTQ, *AlertQ;
//extern OS_EVENT *PLDMSNRSem;
extern OS_EVENT *PLDMSNRQ;
extern _IPInfo IPInfo;
extern asf_config_t *asfconfig;

extern INT32U CPU_CLK_FREQ;

extern INT8U *timestampdataptr;
extern smbiosrmcpdata *smbiosrmcpdataptr;
extern INT32U smbiosdatalen;
extern char *idata;

//extern INT8U smbus_param[][10];
extern pldm_t *pldmdataptr;
extern pldm_res_t *pldmresponse[7];
extern INT8U *smbiosptr;
extern sensor_t sensor[MAX_SENSOR_NUMS];
extern RTSkt *dashcs;
extern RTSkt *dashtlscs;
extern INT8U smbios_flag;

extern int m_LucidTimer;
extern INT32U BSP_TIMER_FREQ;

void SOAPTask(void*data);
static void AppTaskStart (void *p_arg);
static void SMBUSTask(void *p_arg) _ATTRIBUTE_SRAM;
static void SMBUSLOGTask(void *p_arg) _ATTRIBUTE_SRAM;
static void SNRReadTask(void *p_arg) _ATTRIBUTE_SRAM;

#ifdef CONFIG_MAIL_WAKEUP_ENABLED
static void MailWakeUpTask(void *data);
#endif

void bsp_setup() __attribute__((far_call));
//extern void bsp_timer_init(void);


//just for app.c
//#if defined(CONFIG_BUILDROM)
//extern void (*tcpProcess_F)(PKT *pkt, TCPPCB *pcbPtr);
//#endif

static void TCPTask(void *p_arg) _ATTRIBUTE_SRAM;
static void initTCP();

void DHCPv4Task(void *data);

#ifdef CONFIG_DHCP4_AUTO
void DHCPv4Task_auto(void *data);
#endif

void tcpTimerTask(void *data);
void OOBResponseTask(void *p_arg);
void tcpProcess(PPKT pkt, ptcp_pcb pcbPtr);

extern INT8U const inquiry_data[3][40]; //for test purpose
extern DPCONF *dpconf;

static const INT8U RTUDID_Value[16] = {0};
extern flash_data_t dirty[ENDTBL];

extern INT32U sizearr[PAR_SIZE];
extern INT32U parsize[PAR_SIZE];
extern FWSIG *fwsig;

#ifdef CONFIG_UDPSRV_Test
static void udpSrvTestTask(void *data);
#endif

#ifdef CONFIG_UDPSRV_Test
static void clientTestTask(void *data);
#endif

#ifdef CONFIG_Client_Test
static void clientTestTask(void *data);
#endif

int
main (void)
{
    INT8U err;
#ifdef CONFIG_PROVISIONING
    INT16U i = 0;
    INT8U* tmp;
    struct ssi_portmap *ssi_map;
    INT8U provisioningstate = 0;
    INT32U flashaddr;
#endif

    /* Disable all interrupts until we are ready to accept them */
    bsp_irq_disable();


#if defined(CONFIG_BUILDROM) && !defined(CONFIG_FPGA_REFILL)
    if(fwsig->SIG != NORMAL_SIG)
        dummy_call();
#endif
    //bsp_wait_t(1);
    //bsp_wait(1);

#ifdef CONFIG_MEM_ADJUST_1
    parsize[9] = parsize[9] -1;
    parsize[5] = parsize[5] + 8;
    parsize[1] = parsize[1] + 128;
#endif

#ifdef CONFIG_USB_BULK_TEST
    parsize[7] = 22;
    parsize[8] = 3;
    parsize[9] = 3;
    sizearr[10] = 16384;
    parsize[10] = 2;
#endif

#ifdef CONFIG_PROVISIONING
#if CONFIG_VERSION == IC_VERSION_DP_RevA
    ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
    tmp = (INT8U *)(HEAP_START_ADDR);
    flashaddr = (INT32U ) (MEM_MAP_ADDR - 0x81000000);

    memcpy(tmp, (INT8U*)MEM_MAP_ADDR, 4096);

    //check provisioning state
    provisioningstate = REG8(PROVISION_ADDR)  & 0x08;
    if (provisioningstate == 0)
    {
        tmp[17*4] = 38;
        tmp[18*4] = 5;
        tmp[37] = 0x14;
    }
    else //if (provisioningstate == 11)
    {
        tmp[17*4] = 42;
        tmp[18*4] = 4;
        tmp[37] = 0x10;
    }

    spi_se_erase(ssi_map, flashaddr);
    for (i = 0; i<64; i++)
        spi_write(ssi_map, flashaddr+i*64, tmp+i*64, 64);
#else
    provisioningstate = REG8(PROVISION_ADDR)  & 0x08;
    if (provisioningstate == 0)
    {
        parsize[7] = 38;
        parsize[8] = 5;
        sizearr[9] = 5120;
    }

#endif
#endif
    //parsize[4] = 48;   // fix tcr for the lenovo
    //parsize[7] = 40;

#ifdef CONFIG_POWER_SAVING_ENABLED
#if CONFIG_VERSION >= IC_VERSION_EP_RevB
    /*
    //0x60 : MAC_CPUSLOW
    REG16(MAC_BASE_ADDR+0x60) = 0x0202;
       REG8(MAC_BASE_ADDR+0x60+2) = 0x02;
       REG8(MAC_BASE_ADDR+0x60+3) = 0x3F;
       */
#endif
#endif

    //initFunPtr();
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


extern void buildKeyworkTable();
//extern void bsp_timer_reinit();

/****
sharepin interrupt
***/
void bsp_KCS_RKVM_NCSI_shareHandler(void)
{

    volatile INT8U rkvm_temp=0;
    volatile INT16U ncsi_temp=0,kcs_temp=0;

#ifdef CONFIG_RKVM_TEST_ENABLED
    rkvm_temp = REG8(CMAC2_IOBASE+CMAC2_OSR0) & REG8(CMAC2_IOBASE+CMAC2_OMR0);
#endif

#ifdef CONFIG_KCS_TEST_ENABLED
    kcs_temp = REG16(KCS_BASE+KCS_ISR) & REG16(KCS_BASE + KCS_IMR);
#endif

#ifdef CONFIG_NCSI_TEST_ENABLED
//	ncsi_temp = REG16(NCSI_IOBASE+MAC_ISR) & REG16(NCSI_IOBASE+MAC_IMR);
#endif

#ifdef CONFIG_RKVM_TEST_ENABLED
    if(rkvm_temp)
    {
        bsp_cmac2_handler();
    }
#endif

#ifdef CONFIG_KCS_TEST_ENABLED
    if(kcs_temp)
        bsp_kcsTest_handler();
#endif

#ifdef CONFIG_NCSI_TEST_ENABLED
//	if(ncsi_temp)
//		bsp_gmac2_handler();
#endif

}

void bsp_FUN0_EHCI_shareHandler(void)
{
    volatile INT16U fun0_temp=0;
    volatile INT32U ehci_temp=0;

#if defined(CONFIG_VPD_TEST_ENABLED) || defined(CONFIG_PCIE_ENABLED) || defined(CONFIG_VGA_TEST_ENABLED)
    fun0_temp = REG16(BMC_ISR) & REG16(BMC_IMR);
#endif

#ifdef CONFIG_EHCI_TEST_ENABLED
    ehci_temp = REG32(EHCI_ISR) & REG32(EHCI_IMR);
#endif

#if defined(CONFIG_VPD_TEST_ENABLED) || defined(CONFIG_PCIE_ENABLED) || defined(CONFIG_VGA_TEST_ENABLED) || defined(CONFIG_RKVM_TEST_ENABLED) || defined(CONFIG_CMAC_TEST_ENABLED)
    if(fun0_temp)
    {
        Function0_Interrupt_shareHandler();
    }
#endif

#ifdef CONFIG_EHCI_TEST_ENABLED
    if(ehci_temp)
    {
        rtkehci_int_handler_test();
    }
#endif

}

void Function0_Interrupt_shareHandler (void)
{
    volatile INT16U isr_temp=0,IMRvalue=0;

    IMRvalue = REG16(BMC_IMR);
    REG16(BMC_IMR) = 0x0000;
    isr_temp = REG16(BMC_ISR);
//	REG16(BMC_ISR) = REG16(BMC_ISR);
    REG16(BMC_ISR) = PERSTB_R_STS | PERSTB_F_STS;

#ifdef CONFIG_VPD_TEST_ENABLED
    if (isr_temp & VPD_STS)   //VPD access event
    {
        VPD_RW_subHandler();
    }
#endif

#if defined(CONFIG_PCIE_ENABLED) || defined(CONFIG_VGA_TEST_ENABLED) || defined(CONFIG_RKVM_TEST_ENABLED) || defined(CONFIG_CMAC_TEST_ENABLED)
    if ((isr_temp & PERSTB_R_STS) || (isr_temp & PERSTB_F_STS))
    {
        bsp_Fun0_subHandler(isr_temp);
    }
#endif

    REG16(BMC_IMR) = IMRvalue;

}

//function divide RISC  Frequency Ratio
void RISC_FRE_divide(INT32U ratio)
{
	INT32U temp;
	
	temp =REG32(CPU1_IOBASE+0x14);
	temp|= 0x3FF;
	//REG32(CPU1_IOBASE+0x14)=temp;

	temp = REG32(CPU1_IOBASE+4);//get current divide ratio
	temp &= ~((0x07)|(1<<11));
	//temp |= (1<<6);
	temp |= (ratio & 0x7);
	
	REG32(CPU1_IOBASE+4) = temp;//set  divide ratio
	bsp_wait(3);//wait for  a few  cycs
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
static void AppTaskStart (void *p_arg)
{

#if CONFIG_VERSION  >= IC_VERSION_FP_RevA
	//rc_ephy_init();
    init_NIC_SD();
    init_testItem_SD();
    bsp_uart_init((REG32(CPU1_IOBASE + FREQ_CAL_REG1) >> 16)*125000);//FPGA uart bus clock is ~200Mhz
    printk("AppTaskStart \r\n");//bulit-in API
#endif

#if OS_TASK_STAT_EN > 0
    OSStatInit();						   //initial statistic task Determine CPU capacity									 */
#endif

#ifdef CONFIG_OOBMAC_TEST_ENABLED

    OSTaskCreateExt (OOBMAC_Test_Task,
                     (void *) 0,
                     (OS_STK *) & OOBMACTestStk[TASK_OOBMAC_TEST_STK_SIZE - 1],
                     TASK_OOBMAC_TEST_PRIO,
                     TASK_OOBMAC_TEST_ID,
                     (OS_STK *) & OOBMACTestStk[0],
                     TASK_OOBMAC_TEST_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif


#ifdef CONFIG_RKVM_TEST_ENABLED
    OSTaskCreateExt (RKVM_Test_Task,
                     (void *) 0,
                     (OS_STK *) & RKVMTestStk[TASK_RKVM_TEST_STK_SIZE - 1],
                     TASK_RKVM_TEST_PRIO,
                     TASK_RKVM_TEST_ID,
                     (OS_STK *) & RKVMTestStk[0],
                     TASK_RKVM_TEST_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#ifdef CONFIG_KCS_TEST_ENABLED
    OSTaskCreateExt (KCS_Test_Task,
                     (void *) 0,
                     (OS_STK *) & KCSTestStk[TASK_KCS_TEST_STK_SIZE - 1],
                     TASK_KCS_TEST_PRIO,
                     TASK_KCS_TEST_ID,
                     (OS_STK *) & KCSTestStk[0],
                     TASK_KCS_TEST_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#ifdef CONFIG_CMAC_TEST_ENABLED
    OSTaskCreateExt (CMAC_Test_Task,
                     (void *) 0,
                     (OS_STK *) & CMACTestStk[TASK_CMAC_TEST_STK_SIZE - 1],
                     TASK_CMAC_TEST_PRIO,
                     TASK_CMAC_TEST_ID,
                     (OS_STK *) & CMACTestStk[0],
                     TASK_CMAC_TEST_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#endif

#ifdef CONFIG_VPD_TEST_ENABLED
    OSTaskCreateExt (VPD_Test_Task,
                     (void *) 0,
                     (OS_STK *) & VPDTestStk[TASK_VPD_TEST_STK_SIZE - 1],
                     TASK_VPD_TEST_PRIO,
                     TASK_VPD_TEST_ID,
                     (OS_STK *) & VPDTestStk[0],
                     TASK_VPD_TEST_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#endif

#ifdef CONFIG_PCIE_ENABLED
#ifdef CONFIG_PCIE_DEV_ENABLED
    OSTaskCreateExt (PCIDevDrvTask,
                     (void *) 0,
                     (OS_STK *) & PCIDevDrvStk[TASK_PCI_DEV_DRV_STK_SIZE - 1],
                     TASK_PCI_DEV_DRV_PRIO,
                     TASK_PCI_DEV_DRV_ID,
                     (OS_STK *) & PCIDevDrvStk[0],
                     TASK_PCI_DEV_DRV_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif
#endif

#ifdef CONFIG_VGA_TEST_ENABLED
    OSTaskCreateExt (VGA_Test_Task,
                     (void *) 0,
                     (OS_STK *) &VGAStk[TASK_VGA_TEST_STK_SIZE - 1],
                     TASK_VGA_TEST_PRIO,
                     TASK_VGA_TEST_ID,
                     (OS_STK *) & VGAStk[0],
                     TASK_VGA_TEST_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#ifdef CONFIG_EHCI_TEST_ENABLED
    OSTaskCreateExt (EHCI_Test_Task,
                     (void *) 0,
                     (OS_STK *) &EHCIStk[TASK_EHCI_TEST_STK_SIZE - 1],
                     TASK_EHCI_TEST_PRIO,
                     TASK_EHCI_TEST_ID,
                     (OS_STK *) & EHCIStk[0],
                     TASK_EHCI_TEST_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#ifdef CONFIG_TCR_TEST_ENABLED
    OSTaskCreateExt (TCR_Test_Task,
                     (void *) 0,
                     (OS_STK *) &TCRStk[TASK_TCR_TEST_STK_SIZE - 1],
                     TASK_TCR_TEST_PRIO,
                     TASK_TCR_TEST_ID,
                     (OS_STK *) & TCRStk[0],
                     TASK_TCR_TEST_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

	#ifdef CONFIG_SMBUS_TEST_ENABLED
	OSTaskCreateExt (SMBUS_Test_Task,
					 (void *) 0,
					 (OS_STK *) & SMBUSTestStk[TASK_SMBUS_TEST_STK_SIZE - 1],
					 TASK_SMBUS_TEST_PRIO,
					 TASK_SMBUS_TEST_ID,
					 (OS_STK *) & SMBUSTestStk[0],
					 TASK_SMBUS_TEST_STK_SIZE,
					 (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	#endif
	#if defined(CONFIG_USB_BULK_TEST) || defined(CONFIG_USB_FLASH)
	OSTaskCreateExt (USBSrvTask,
					 (void *) 0,
					 (OS_STK *) & TaskUSBStk[TASK_USB_STK_SIZE - 1],
					 TASK_USB_PRIO,
					 TASK_USB_ID,
					 (OS_STK *) & TaskUSBStk[0],
					 TASK_USB_STK_SIZE,
					 (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	#endif
/*	
	OSTaskCreateExt (MMD_Test_Task,
					 (void *) 0,
					 (OS_STK *) & MMDTestStk[TASK_MMD_TEST_STK_SIZE - 1],
					 TASK_MMD_PRIO,
					 TASK_MMD_ID,
					 (OS_STK *) & MMDTestStk[0],
					 TASK_MMD_TEST_STK_SIZE,
					 (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

*/ 
    while (OS_TRUE)
    {
        //kick the dog every second (would expire after 38 seconds)
        //	bsp_wdt_kick();
        inctimeofday(1);
        OSTimeDly(OS_TICKS_PER_SEC);
    }
}

#if 0
static void AppTaskStart (void *p_arg)
{
    (void) p_arg;
    eventdata *pevent;
    OS_Q_DATA qdata;
    INT8U err;
    INT32U count = rand()%10;
    OS_CPU_SR  cpu_sr = 0;
    INT8U rmttimeout = 0;
#if (CONFIG_VERSION  >= IC_VERSION_EP_RevB)
    INT8U rstcount = 0;
    INT32U rd_tmp;
#endif

#ifdef CONFIG_UART_DEBUG
    bsp_uart_init(31250000);//FPGA uart bus clock is 31250000
    printk("AppTaskStart");//bulit-in API
#endif
    //hao
    rd_tmp = ReadIBReg(FPGA_INDIC);
    if(rd_tmp & 0x10000)
    {
        BSP_TIMER_FREQ = 31250000;
    }

#ifdef CONFIG_CPU_SLEEP_ENABLED
    SetLucidTimerInSec(0);//set m_LucidTimer = 0;
#endif


    bsp_load_asf_default();
#if (CONFIG_VERSION >= IC_VERSION_EP_RevA)
#if !defined(CONFIG_BUILDROM) || (CONFIG_ROM_VERIFY)
    setupPatchFun();
#endif
    bsp_cmac_init();
#endif

    /* Initialize BSP functions                                 */
    bsp_setup();

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
    wake_up_pattern_init();//+briank.rtk
#endif

#ifdef CONFIG_FPGA_TEST
    //since FPGA is slower than ASIC by 4 fold, need to patch
    CPU_CLK_FREQ = CPU_CLK_FREQ >> 2;
    REG32(TIMER_IOBASE+TIMER_LC) = CPU_CLK_FREQ /HZ;      //set counter
#endif


    //idata is allocated in bsp_buf_alloc, however newer version of wcom.c
    //need the size of it to be 4096 instead of 1024
    //just free it and allocate it again

    rlx_irq_init();             /* Initialize the interrupt controller */

#if CONFIG_CODEC_TEST
    while (1)
        tls_test();
#endif

    //init_cert();
    //updateCert(dpconf->aesmsg, 16);

#if OS_TASK_STAT_EN > 0
    OSStatInit();                          /* Determine CPU capacity                                   */
#endif

    //delay before GMAC/GPHY ready, auto negotiation needs 2 secs
    //OSTimeDly(3*OS_TICKS_PER_SEC);
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    hwPFInit();
#endif
    /* Initialize TCP/IP parameters*/
    initTCP();

    //start of combine test items @melody 20160808
#ifdef CONFIG_PCIE_ENABLED
#ifdef CONFIG_PCIE_DEV_ENABLED
    OSTaskCreateExt (PCIDevDrvTask,
                     (void *) 0,
                     (OS_STK *) & PCIDevDrvStk[TASK_PCI_DEV_DRV_STK_SIZE - 1],
                     TASK_PCI_DEV_DRV_PRIO,
                     TASK_PCI_DEV_DRV_ID,
                     (OS_STK *) & PCIDevDrvStk[0],
                     TASK_PCI_DEV_DRV_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif
#endif

#ifdef CONFIG_OOBMAC_TEST_ENABLED
    OSTaskCreateExt (OOBMAC_Test_Task,
                     (void *) 0,
                     (OS_STK *) & OOBMACTestStk[TASK_OOBMAC_TEST_STK_SIZE - 1],
                     TASK_OOBMAC_TEST_PRIO,
                     TASK_OOBMAC_TEST_ID,
                     (OS_STK *) & OOBMACTestStk[0],
                     TASK_OOBMAC_TEST_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif


    //end of combine test @melody 20160808


//mask by melody 20160809
#if 0
    OSTaskCreateExt (FlashTask,
                     (void *) 0,
                     (OS_STK *) & TaskFlashStk[TASK_FLASH_STK_SIZE - 1],
                     TASK_FLASH_PRIO,
                     TASK_FLASH_ID,
                     (OS_STK *) & TaskFlashStk[0],
                     TASK_FLASH_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt (SMBUSTask,
                     (void *) 0,
                     (OS_STK *) & TaskSMBUSStk[TASK_SMBUS_STK_SIZE - 1],
                     TASK_SMBUS_PRIO,
                     TASK_SMBUS_ID,
                     (OS_STK *) & TaskSMBUSStk[0],
                     TASK_SMBUS_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt (TCPTask,
                     (void *) 0,
                     (OS_STK *) & TaskTCPStk[TASK_TCP_STK_SIZE - 1],
                     TASK_TCP_PRIO,
                     TASK_TCP_ID,
                     (OS_STK *) & TaskTCPStk[0],
                     TASK_TCP_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt (tcpTimerTask,
                     (void *) 0,
                     (OS_STK *) & TCPTimerStk[TASK_TCP_TIMER_STK_SIZE - 1],
                     TASK_TCP_TIMER_PRIO,
                     TASK_TCP_TIMER_ID,
                     (OS_STK *) & TCPTimerStk[0],
                     TASK_TCP_TIMER_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#ifdef CONFIG_8021X_ENABLED
    OSTaskCreateExt (EAPTask,
                     (void *) 0,
                     (OS_STK *) & Task8021xStk[TASK_8021X_STK_SIZE - 1],
                     TASK_8021X_PRIO,
                     TASK_8021X_ID,
                     (OS_STK *) & Task8021xStk[0],
                     TASK_8021X_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#ifdef CONFIG_DHCP_ENABLED

#ifdef CONFIG_DHCP4_AUTO
    OSTaskCreateExt (DHCPv4Task_auto,
#else
    OSTaskCreateExt (DHCPv4Task,
#endif
                     (void *) 0,
                     (OS_STK *) & DHCPv4Stk[TASK_DHCPv4_STK_SIZE - 1],
                     TASK_DHCPv4_PRIO,
                     TASK_DHCPv4_ID,
                     (OS_STK *) & DHCPv4Stk[0],
                     TASK_DHCPv4_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#ifdef CONFIG_IPv6_ENABLED
    OSTaskCreateExt (DHCPv6Task,
                     (void *) 0,
                     (OS_STK *) & DHCPv6Stk[TASK_DHCPv6_STK_SIZE - 1],
                     TASK_DHCPv6_PRIO,
                     TASK_DHCPv6_ID,
                     (OS_STK *) & DHCPv6Stk[0],
                     TASK_DHCPv6_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#endif

#if defined(CONFIG_USB_BULK_TEST) || defined(CONFIG_USB_FLASH)
    OSTaskCreateExt (USBSrvTask,
                     (void *) 0,
                     (OS_STK *) & TaskUSBStk[TASK_USB_STK_SIZE - 1],
                     TASK_USB_PRIO,
                     TASK_USB_ID,
                     (OS_STK *) & TaskUSBStk[0],
                     TASK_USB_STK_SIZE,
                     (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#ifdef CONFIG_PROVISIONING
    if (dpconf->ProvisioningState != PROVISIONED)
    {

        OSTaskCreateExt (provisioningSrv,
                         (void *) 0,
                         (OS_STK *) & TaskWSMANStk[TASK_WSMAN_STK_SIZE - 1],
                         TASK_Provisioning_PRIO,
                         TASK_Provisioning_ID,
                         (OS_STK *) & TaskWSMANStk[0],
                         TASK_WSMAN_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

        //ht 20120112 : add for writing data to flash
#if CONFIG_VERSION > 1
        OSTaskCreateExt (OOBResponseTask,
                         (void *) 0,
                         (OS_STK *) & OOBResponseStk[TASK_OOBResponse_STK_SIZE - 1],
                         TASK_OOBResponse_PRIO,
                         TASK_OOBResponse_ID,
                         (OS_STK *) & OOBResponseStk[0],
                         TASK_OOBResponse_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif
    }
    else
#endif
    {

#ifdef CONFIG_DASH_ENABLED
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
        /*

        OSTaskCreateExt (wsmanSrvIPv6,
                         (void *) 0,
                         (OS_STK *) & TaskWSMANIPv6Stk[TASK_WSMAN_STK_SIZE - 1],
                         TASK_WSMAN_IPv6_PRIO,
                         TASK_WSMAN_IPv6_ID,
                         (OS_STK *) & TaskWSMANIPv6Stk[0],
                         TASK_WSMAN_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
        OSTaskCreateExt (wsmanSrvTLSIPv6,
                         (void *) 0,
                         (OS_STK *) & TaskWSMANTLSIPv6Stk[TASK_WSMAN_TLS_STK_SIZE - 1],
                         TASK_WSMAN_TLS_IPv6_PRIO,
                         TASK_WSMAN_TLS_IPv6_ID,
                         (OS_STK *) & TaskWSMANTLSIPv6Stk[0],
                         TASK_WSMAN_TLS_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
        	     */

        OSTaskCreateExt (clientTask1,
                         (void *) 0,
                         (OS_STK *) & IndicationStk[TASK_INDICATION_STK_SIZE-1],
                         TASK_INDICATION_PRIO,
                         TASK_INDICATION_ID,
                         (OS_STK *) & IndicationStk[0],
                         TASK_INDICATION_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif
#ifdef CONFIG_ASF_ENABLED
        OSTaskCreateExt (SNMPTask,
                         (void *) 0,
                         (OS_STK *) & TaskSNMPStk[TASK_SNMP_STK_SIZE - 1],
                         TASK_SNMP_PRIO,
                         TASK_SNMP_ID,
                         (OS_STK *) & TaskSNMPStk[0],
                         TASK_SNMP_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

        OSTaskCreateExt (ASFV1Task,
                         (void *) 0,
                         (OS_STK *) & TaskASFV1Stk[TASK_ASF_V1_STK_SIZE - 1],
                         TASK_ASF_V1_PRIO,
                         TASK_ASF_V1_ID,
                         (OS_STK *) & TaskASFV1Stk[0],
                         TASK_ASF_V1_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

        OSTaskCreateExt (ASFV2Task,
                         (void *) 0,
                         (OS_STK *) & TaskASFV2Stk[TASK_ASF_V2_STK_SIZE - 1],
                         TASK_ASF_V2_PRIO,
                         TASK_ASF_V2_ID,
                         (OS_STK *) & TaskASFV2Stk[0],
                         TASK_ASF_V2_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#if defined(CONFIG_PLDM_SENSOR) //|| defined(CONFIG_VENDOR_FSC)
        OSTaskCreateExt (SNRReadTask,
                         (void *) 0,
                         (OS_STK *) & TaskSNRReadStk[TASK_SNR_READ_STK_SIZE - 1],
                         TASK_SNR_READ_PRIO,
                         TASK_SNR_READ_ID,
                         (OS_STK *) & TaskSNRReadStk[0],
                         TASK_SNR_READ_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#if CONFIG_SMBUSTRA_DEBUG
        OSTaskCreateExt (SMBUSLOGTask,
                         (void *) 0,
                         (OS_STK *) & TaskSMBUSLOGStk[TASK_SMBUSLOG_STK_SIZE - 1],
                         TASK_SMBUSLOG_PRIO,
                         TASK_SMBUSLOG_ID,
                         (OS_STK *) & TaskSMBUSLOGStk[0],
                         TASK_SMBUSLOG_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#if CONFIG_IPERF_Test
        OSTaskCreateExt (iperfTask,
                         (void *) 0,
                         (OS_STK *) & IperfTestStk[TASK_Iperf_STK_SIZE - 1],
                         TASK_IPERF_PRIO,
                         TASK_IPERF_PRIO,
                         (OS_STK *) & IperfTestStk[0],
                         TASK_Iperf_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#if CONFIG_UDPSRV_Test
        OSTaskCreateExt (udpSrvTestTask,
                         (void *) 0,
                         (OS_STK *) & IperfTestStk[TASK_Iperf_STK_SIZE - 1],
                         TASK_IPERF_PRIO,
                         TASK_IPERF_PRIO,
                         (OS_STK *) & IperfTestStk[0],
                         TASK_Iperf_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#if CONFIG_Client_Test
        OSTaskCreateExt (clientTestTask,
                         (void *) 0,
                         (OS_STK *) & IperfTestStk[TASK_Iperf_STK_SIZE - 1],
                         TASK_IPERF_PRIO,
                         TASK_IPERF_PRIO,
                         (OS_STK *) & IperfTestStk[0],
                         TASK_Iperf_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#if CONFIG_VERSION > 1
        OSTaskCreateExt (OOBResponseTask,
                         (void *) 0,
                         (OS_STK *) & OOBResponseStk[TASK_OOBResponse_STK_SIZE - 1],
                         TASK_OOBResponse_PRIO,
                         TASK_OOBResponse_ID,
                         (OS_STK *) & OOBResponseStk[0],
                         TASK_OOBResponse_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#if CONFIG_PCI_Bridge_Test
        OSTaskCreateExt (ComWinTask,
                         (void *) 0,
                         (OS_STK *) & OOBTestStk[TASK_OOBTest_STK_SIZE - 1],
                         TASK_OOBTest_PRIO,
                         TASK_OOBTest_PRIO,
                         (OS_STK *) & OOBTestStk[0],
                         TASK_OOBTest_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#endif

#if CONFIG_EZSHAE_TASK_ENABLED
        OSTaskCreateExt (webEZshareSrvThread,
                         (void *) 0,
                         (OS_STK *) & WebSrvStk[TASK_WEB_SRV_STK_SIZE - 1],
                         TASK_WEB_SRV_PRIO,
                         TASK_WEB_SRV_PRIO,
                         (OS_STK *) & WebSrvStk[0],
                         TASK_WEB_SRV_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#elif  CONFIG_WEB_SKT_STYLE
        OSTaskCreateExt (webSrv,
                         (void *) 0,
                         (OS_STK *) & WebSrvStk[TASK_WEB_SRV_STK_SIZE - 1],
                         TASK_WEB_SRV_PRIO,
                         TASK_WEB_SRV_PRIO,
                         (OS_STK *) & WebSrvStk[0],
                         TASK_WEB_SRV_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#ifdef  CONFIG_mDNS_OFFLOAD_ENABLED
        OSTaskCreateExt (mDNSOffloadSrv,
                         (void *) 0,
                         (OS_STK *) & mDNSSrvStk[TASK_mDNS_SRV_STK_SIZE - 1],
                         TASK_mDNS_OFFLOAD_PRIO,
                         TASK_mDNS_OFFLOAD_PRIO,
                         (OS_STK *) & mDNSSrvStk[0],
                         TASK_mDNS_SRV_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif
#ifdef CONFIG_TELNET_ENABLED
        OSTaskCreateExt (telnetSrv,
                         (void *) 0,
                         (OS_STK *) & TelnetSrvStk[TASK_TELNET_SRV_STK_SIZE - 1],
                         TASK_TELNET_SRV_PRIO,
                         TASK_TELNET_SRV_PRIO,
                         (OS_STK *) & TelnetSrvStk[0],
                         TASK_TELNET_SRV_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif
#ifdef CONFIG_SSH_ENABLED
        OSTaskCreateExt (sshSrv,
                         (void *) 0,
                         (OS_STK *) & TelnetSrvStk[TASK_TELNET_SRV_STK_SIZE - 1],
                         TASK_TELNET_SRV_PRIO,
                         TASK_TELNET_SRV_PRIO,
                         (OS_STK *) & TelnetSrvStk[0],
                         TASK_TELNET_SRV_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#if CONFIG_TEREDO_ENABLED
        OSTaskCreateExt (teredoRxSrv,
                         (void *) 0,
                         (OS_STK *) & TeredoRxSrvStk[TASK_TEREDO_RX_SRV_STK_SIZE - 1],
                         TASK_TEREDO_RX_SRV_PRIO,
                         TASK_TEREDO_RX_SRV_PRIO,
                         (OS_STK *) & TeredoRxSrvStk[0],
                         TASK_TEREDO_RX_SRV_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

//Bill
#ifdef CONFIG_MAIL_WAKEUP_ENABLED
        OSTaskCreateExt (MailWakeUpTask,
                         (void *) 0,
                         (OS_STK *) & MailWakeUpStk[TASK_MAIL_WAKE_UP_SRV_STK_SIZE - 1],
                         TASK_MAIL_WAKE_UP_SRV_PRIO,
                         TASK_MAIL_WAKE_UP_SRV_PRIO,
                         (OS_STK *) & MailWakeUpStk[0],
                         TASK_MAIL_WAKE_UP_SRV_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif

#if CONFIG_VNC_ENABLED
        OSTaskCreateExt (VNCSrv,
                         (void *) 0,
                         (OS_STK *) & VNCSrvStk[TASK_VNC_SRV_STK_SIZE - 1],
                         TASK_VNC_SRV_PRIO,
                         TASK_VNC_SRV_PRIO,
                         (OS_STK *) & VNCSrvStk[0],
                         TASK_VNC_SRV_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#ifdef CONFIG_VNC_VIDEO_ENABLED
        VGA_pre_Initial();
#endif
#endif
    }


    ///OSTimeDly(5*OS_TICKS_PER_SEC);
#ifdef CONFIG_REMOTE_DEBUG
    //if using remote debug, we must ensure that gmac is initalized first
    //and no one will use the printf before set_remote_dbg is completed
    set_remote_dbg(226);
#endif

    DEBUGMSG(MESSAGE_DEBUG, "%s\n", "Remote Debug Enabled");

    OSTimeDly(100 * 1);
    //printf("[RTK] (001)Start Remote Debug , dirty[DPCONFIGTBL].length=%d , sizeof(DPCONF)=%d\n",dirty[DPCONFIGTBL].length,sizeof(DPCONF));


#endif  //mask by melody 20160809

#if CONFIG_RESET_COUNTER
    //Add a counter to count the reset times of firmware
    if(dpconf->counter < 0xFFFF)
    {
        dpconf->counter ++;
    }
    else
    {
        dpconf->counter  = 0;
    }
    setdirty(DPCONFIGTBL);//+briankuo , 20140909 : Add for recording reset times , "dpconf->counter"
#endif


    bsp_bits_set(0xB0,1,0,1);//+briank.rtk TCRDiable:TCR By Pass mode


    SetPowerSavingParameter();
    //-------------------------



    //printf("[RTK] CPU_CLK_FREQ =%x , divider now=%x , m_divide = 0x(%x , %x ,  %x) , \n",CPU_CLK_FREQ, REG8(CPU_REG),m_divide1,m_divide2,m_divide3);
    //printf("[RTK]reg1MAC_STATUS=%x , now = 0x%x\n",reg1MAC_STATUS,REG32(MAC_BASE_ADDR +0x104));

#if 0	   //mask by melody 20160809
    buildKeyworkTable();
#endif

    while (OS_TRUE)
    {

        //kick the dog every second (would expire after 38 seconds)
        bsp_wdt_kick();

#ifdef CONFIG_CPU_SLEEP_ENABLED
        if(m_LucidTimer > 0)
        {
            m_LucidTimer --;
        }
        else if(m_LucidTimer == 0)
        {
            m_LucidTimer = -1;
            EnableCPUSleep();
        }
#endif


#if 0//(CONFIG_VERSION  == IC_VERSION_EP_RevB)   //patch tx reset issue     
        //static INT8U rstcount = 0;
        //INT32U rd_tmp;
        if(usbcb.usbstate != DISABLED && usbcb.usbstate != DISCONNECTED)
        {
            rd_tmp=REG32(0xb2003050);
            if((rd_tmp&0x3000) == 0x3000)
            {
                rstcount++;
                if(rstcount==2)//keep the same value to do ehci_patch() during 2sec
                {
                    rstcount=0;
                    ehci_patch();
                }
            }
            else
                rstcount=0;
        }
#endif
        if (dpconf->ProvisioningState == PROVISIONED)
        {

#ifndef CONFIG_PROFILING

            //legacy sensor polling time is the same as that of heartbeat
            if (dpconf->lspoll && (((count+4) % dpconf->hbtime)  == 0) && (smbiosrmcpdataptr->PollType != POLL_STOP))
            {
                //only some MB supports Legacy sensor polling
                if (smbiosrmcpdataptr->PollType == POLL_NONE  && !SMBus_Prepare_LSPMsg())
                {
                    OS_ENTER_CRITICAL();
                    smbiosrmcpdataptr->PollType = POLL_LS;
                    smbiosrmcpdataptr->PollTimeOut = 0;
                    OS_EXIT_CRITICAL();
                }
            }

            OSQQuery(EVTQ, &qdata);

            while (qdata.OSNMsgs--)
            {
                pevent = (eventdata *) OSQAccept(EVTQ,&err);
                pevent->timeout = (pevent->timeout > 1) ?  (pevent->timeout - 1) : 0;

                if (!pevent->timeout)
                {
                    if (pevent->watchdog)
                    {
                        if (pevent->alertnum > 0)
                        {
                            smbiosrmcpdataptr->expired = 1;
                            pevent->watchdog = 0;
                            //log this event and post to queue for handling
                            pevent->logtype = LOG_ERROR;
                            pevent = event_log(&pevent->Event_Sensor_Type, 10);
                            OSQPost(EVTQ, pevent);
                        }
                        //receive a stop watchdog timer
                        else
                            smbiosrmcpdataptr->expired = 0;
                    }
                    else if (--pevent->alertnum > 0)
                    {
                        //handling PushAlertMsgWR
                        pevent->timeout = pevent->interval;

                        OSQPost(EVTQ, pevent);
                    }

                    if (!pevent->watchdog)
                    {
                        //only asf alerts need to retransmission
                        //dash use tcp, no need to do restransmission
#ifdef CONFIG_ASF_ENABLED
                        OSQPost(SNMPQ, pevent);
#endif

#ifdef CONFIG_DASH_ENABLED
                        if (!pevent->sent)
                        {
                            pevent->sent = 1;
                            OSQPost(AlertQ, pevent);
                        }
#endif
                    }
                }
                else if (pevent->alertnum > 0)
                    //if not timeout => put it back, should be the watch dog case
                {
                    OSQPost(EVTQ, pevent);
                }
            }//end of while
#endif

            count++;
            inctimeofday(1);
        }

        if (bsp_get_sstate() == S_S0)
        {
            smbiosrmcpdataptr->Boottime = (smbiosrmcpdataptr->Boottime == 255 ) ? (255) : (smbiosrmcpdataptr->Boottime + 1);
            //after  20 secs when entering S0, resume the polling and flash write process
            if (smbiosrmcpdataptr->Boottime == WAIT_PLDM_SECS)
            {
                OS_ENTER_CRITICAL();
                smbiosrmcpdataptr->PollType = POLL_NONE;
                OS_EXIT_CRITICAL();
            }
        }
        else
        {
            OS_ENTER_CRITICAL();
            smbiosrmcpdataptr->Boottime = 0;
            smbiosrmcpdataptr->PollType = POLL_STOP;
            OS_EXIT_CRITICAL();
        }

        if (smbiosrmcpdataptr->IBRmtl)
        {
            rmttimeout++;

            if (!bsp_bits_get(MAC_SYNC1, BIT_RMTCTL, 3))
            {
                smbiosrmcpdataptr->IBRmtl = 0;
                rmttimeout = 0;
            }
            else if (rmttimeout == 10) //wait for 10 seconds timeout
            {
                rmttimeout = 0;
                smbiosrmcpdataptr->IBRmtl = 0;
                SMBus_Prepare_RmtCtrl(smbiosrmcpdataptr->IBRmtlCmd,1);
            }
        }

        //fix IPMI issue on ubuntu : ht 20121011 briank.rtk 20130828
#if defined(CONFIG_KCS_AMD_PATCH_ENABLED) && (defined(CONFIG_CHIPSET_AMD)|| defined(CONFIG_CHIPSET_AMDSOC))
        if(REG8(KCS_BASE + KCS_STATUS2) & 0x02)
        {
            REG8(KCS_BASE + KCS_STATUS2) = 0x0;
        }
#endif

        OSTimeDly(OS_TICKS_PER_SEC);
    }
}
#endif

#if defined(CONFIG_PLDM_SENSOR) || defined(CONFIG_VENDOR_FSC)
static void SNRReadTask(void *p_arg)
{
    (void) p_arg;
    INT8U err;
    INT8U snrnum = 0;
    INT8U errcount = 0;
    smbrxdesc_t *msg;
    INT8U *addr;
    OS_CPU_SR  cpu_sr = 0;

#if CONFIG_VENDOR_FSC
    INT8U curindex = 0;
    static const INT8U snraddr[18] = {0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0x59, 0x69, 0x79, 0x89, 0x99, 0xA9, 0xB9, 0xC9, 0xD9, 0xE9, 0xF9};
    static const INT8U snrtype[18] = {SNR_TACHOMETER,SNR_TACHOMETER,SNR_TACHOMETER,SNR_TACHOMETER,SNR_TACHOMETER,SNR_TACHOMETER,SNR_TACHOMETER,SNR_TEMPERATURE,SNR_TEMPERATURE,SNR_TEMPERATURE,SNR_TEMPERATURE,SNR_TEMPERATURE,SNR_TEMPERATURE,SNR_TEMPERATURE,SNR_TEMPERATURE,SNR_TEMPERATURE,SNR_TEMPERATURE, SNR_TEMPERATURE};
    static const INT8U snrname[18][20] = {"FAN1", "FAN2", "FAN3", "FAN4", "FAN5", "FAN6", "FANPS", "CPU1 SEN1", "CPU1 SEN2", "CPU2 SEN1", "SYS1", "SYS2", "SYS3", "HDD", "SYS4", "SYS5", "SYS6", "SYS7"};

    dpconf->numofsnr = 0;
    while(curindex <18)
    {
        if(smbiosrmcpdataptr->PollType == POLL_NONE)
        {
            sensor[dpconf->numofsnr].offset[0] = snraddr[curindex];
            sensor[dpconf->numofsnr].index = dpconf->numofsnr;
            sensor[dpconf->numofsnr].type  = snrtype[curindex];

            if(!Fan_Poll(dpconf->numofsnr, 0))
            {
                OS_ENTER_CRITICAL();
                smbiosrmcpdataptr->PollType = POLL_SENSOR;
                smbiosrmcpdataptr->PollTimeOut = 0;
                OS_EXIT_CRITICAL();
            }
            else
            {
                OSTimeDly(1);
                continue;
            }

            msg = (smbrxdesc_t *) OSQPend(PLDMSNRQ,5,&err);

            OS_ENTER_CRITICAL();
            smbiosrmcpdataptr->PollType = POLL_NONE;
            OS_EXIT_CRITICAL();

            if (err == OS_ERR_NONE)
            {
                addr = (INT8U *) PA2VA(msg->rx_buf_addr);
                Get_FSC_Sensor(dpconf->numofsnr, *addr);

                if(sensor[dpconf->numofsnr].exist == SNR_READ)
                {
                    strcpy(sensor[dpconf->numofsnr].name, snrname[curindex]);
                    if(sensor[dpconf->numofsnr].type == SNR_TACHOMETER)
                        sensor[dpconf->numofsnr++].offset[1] = snraddr[curindex] + 7;
                    else
                        sensor[dpconf->numofsnr++].offset[1] = snraddr[curindex] - 1;
                }
                SMBUSFree(msg);
            }
            curindex++;
        }
        else
            OSTimeDly(1);
    }

#endif


    while (OS_TRUE)
    {
        snrnum = 0;
        err = OSTaskSuspend(OS_PRIO_SELF);
        while (snrnum != (dpconf->numofsnr-asfconfig->numofsnr))
        {
            if(sensor[snrnum].exist !=SNR_READ)
                break;

            if ((bsp_get_sstate() != S_S5) && (smbiosrmcpdataptr->PollType != POLL_STOP))   //add polltype for sensor request timing@20141104
            {
#ifdef CONFIG_PLDM_SENSOR
                pldm_snr_read(snrnum);
                msg = (smbrxdesc_t *) OSQPend(PLDMSNRQ,5,&err);
                if (err == OS_ERR_NONE)
                {
                    addr = (INT8U *) PA2VA(msg->rx_buf_addr) ;

                    if((addr[13] == PLDM_sint8) || (addr[13] == PLDM_sint16) || (addr[13] == PLDM_sint32))
                        sensor[snrnum].signvalue = 1;
                    else
                        sensor[snrnum].signvalue = 0;


                    sensor[snrnum].value  = addr[19];
                    if(addr[13] >= PLDM_uint16)
                        sensor[snrnum].value += (addr[20] << 8);
                    if(addr[13] >= PLDM_uint32)
                    {
                        sensor[snrnum].value += (addr[21] << 16);
                        sensor[snrnum].value += (addr[22] << 24);
                    }

                    //for SMSC 5504, need to do conversion
                    if(dpconf->pldmsnr == 0x99)
                    {
                        if (sensor[snrnum].type == SNR_TACHOMETER)
                            sensor[snrnum].value = 5400000/sensor[snrnum].value;
                        else
                            sensor[snrnum].value = addr[20] - 64;
                    }

                    sensor[snrnum].fault = 5;
                    sensor[snrnum].state = addr[16];
                    SMBUSFree(msg);
                    snrnum++;
                    errcount = 0;
                }
                else
                    errcount++;

#else
                if(smbiosrmcpdataptr->PollType == POLL_NONE)
                {

                    if(!Fan_Poll(snrnum, 1))
                    {
                        OS_ENTER_CRITICAL();
                        smbiosrmcpdataptr->PollType = POLL_SENSOR;
                        smbiosrmcpdataptr->PollTimeOut = 0;
                        OS_EXIT_CRITICAL();
                    }
                    else
                    {
                        OSTimeDly(1);
                        continue;
                    }

                    msg = (smbrxdesc_t *) OSQPend(PLDMSNRQ,5,&err);

                    OS_ENTER_CRITICAL();
                    smbiosrmcpdataptr->PollType = POLL_NONE;
                    OS_EXIT_CRITICAL();

                    if (err == OS_ERR_NONE)
                    {
                        addr = (INT8U *) PA2VA(msg->rx_buf_addr);
                        if (sensor[snrnum].type == SNR_TACHOMETER)
                        {
                            sensor[snrnum].signvalue = 0;
                            //from RPS to RPM
                            sensor[snrnum].value = *addr*60;
                        }
                        else
                        {
                            sensor[snrnum].signvalue = 1;
                            sensor[snrnum].value = *addr - 128;
                            //may has some problem for SYS3 sensor
#if 0
                            if(strcmp(sensor[snrnum].name, "SYS3") == 0)
                                sensor[snrnum].value = (*addr & 0x80) ? ( -(256 - *addr)) : (*addr) ;
                            else
                                sensor[snrnum].value = *addr - 128;
#endif
                        }
                        SMBUSFree(msg);
                        snrnum++;
                        errcount = 0;

                    }
                    else
                        errcount++;
                }
                else
                    OSTimeDly(1);
#endif

                if (errcount == 4)
                {
                    sensor[snrnum].value = 0;
                    sensor[snrnum].fault = 0;
                    sensor[snrnum].state = 0;
                    snrnum++;
                    errcount = 0;
                }

                OSTimeDly(10);
            }
            else
            {

                while ((snrnum != (dpconf->numofsnr-asfconfig->numofsnr))&&(snrnum < MAX_SENSOR_NUMS))
                {
                    sensor[snrnum].state = 0;//unknown
                    snrnum++;
                }

                break;
            }

        }
    }
}
#endif

#if CONFIG_SMBUSTRA_DEBUG
static void SMBUSLOGTask(void *p_arg)
{
    (void) p_arg;
    INT8U *ptr;
    INT8U iy;
    INT8U err;
    INT8U count;
    INT8U str[512];

    while (OS_TRUE)
    {
        ptr = (INT8U *) OSQPend(SMBUSLOGQ,0,&err);

        if(IPInfo.LinkChange)
            OSTimeDly(1000);

        if(ptr[0] == 0x80)
        {
            DEBUGMSG(SMBUSTRA_DEBUG,"BIOS read: ");
        }
        else if (ptr[0] == 0x40)
        {
            DEBUGMSG(SMBUSTRA_DEBUG,"NIC  read: ");
        }
        else if (ptr[0] == 0x20)
        {
            DEBUGMSG(SMBUSTRA_DEBUG,"NIC  send: ");
            if(REG8(SMBUS_BASE_ADDR + SEIO_SMEn) == 0xCD)
                ptr[1]--;
        }
        else
        {
            DEBUGMSG(SMBUSTRA_DEBUG,"BIOS send: ");
        }

        for (iy = 0; iy <= ptr[1] ; iy++)
        {
            sprintf(str + iy*3,"%02x ", ptr[iy+2]);
        }
        DEBUGMSG(SMBUSTRA_DEBUG, "%s\n", str);

    }
}
#endif

static void SMBUSTask(void *p_arg)
{
    (void) p_arg;
    INT8U err;
    INT8U *addr;
    smbrxdesc_t *msg;
    while (OS_TRUE)
    {

        msg = (smbrxdesc_t *) OSQPend(SMBQ,0,&err);
        addr = (INT8U *) PA2VA(msg->rx_buf_addr) ;

#ifdef CONFIG_CPU_SLEEP_ENABLED
        wakeCPU();
#endif

        //recycle block read here
        if (addr[0] == dpconf->arpaddr && addr[2] == (dpconf->arpaddr +1))
        {
            SMBUSFree(msg);
            continue;
        }

        //handling sensor polling case
        if (addr[0] != dpconf->arpaddr)
		{
			smbus_process_polling(addr);
		}
        else
        {
            switch (addr[1])
            {
                case SnsrSystemState:
                    if (addr[2] == 0xC0)
					{
						//smbus  arp
					}
					else if (addr[2] == 0x03)
                    {
                        if (addr[3] == SetSystemState)
                        {
                            if (addr[4] == 0x10)
                                bsp_set_sstate(addr[5]);
                        }
                    }

                    break;

                case ManagementControl:
                    smbus_process_watchdog(addr);
                    break;

                case ASFConfiguration:
                    if (addr[2] == 0x02 && addr[3] == ClearBootOptions)
                    {
                        if (addr[4] == 0x10)
                        {
                            //clear the boot option
                            memset(smbiosrmcpdataptr->bootopt, 0, sizeof(smbiosrmcpdataptr->bootopt));
                        }
                    }

                    break;

                case Message:
                    smbus_process_alerts(addr);
                    break;


                case  0x0f:
                    //Parsing PLDM
                    smbus_process_pldm(addr);

                    //tom modified for Block Read Method @20140219
                    if ((*(addr+3) == 0x71 || *(addr+3) == 0x89) && (*(addr+4) == 0x01) && ((*(addr+7) & 0xC8) == 0xC8) && (*(addr+8) == 0x01) && (*(addr+10) == 0x01) && (*(addr+11) == 0x02))
                        ParserSensor();
                    break;

                default :
                    break;
            }

        }
        //recycle block write here
        SMBUSFree(msg);
    }

}

static void initTCP()
{
    //unsigned int ip = 0;

    unsigned char ipv6[IPv6_ADR_LEN] = {0};
    unsigned char mar[MAC_ADDR_LEN] = {0};
    //int v = 0;

    ArpTableSem = OSSemCreate(1);
    SktSem = OSSemCreate(1);

    ActivePCBsList = tcpPCBNew_F();

#ifdef CONFIG_IPv6_ENABLED
    dpconf->IPv6Enable = 1;
    //dpconf->DHCPv6Enable = 1;
#endif

#ifdef CONFIG_DHCP4_AUTO
    dpconf->DHCPv4Enable = 1;
#endif


    //+briank.rtk
    if(dpconf->isDHCPv4Enable)
    {
        dpconf->isDHCPv4Enable = 0x00;
        dpconf->DHCPv4Enable = 1;
    }


    if (!dpconf->DHCPv4Enable)
    {
        if (dpconf->IPv4Enable && !dpconf->DHCPv4Enable)
        {
            setMatchSubnetMask_F(eth0);
        }
    }

    if (dpconf->IPv6Enable && !dpconf->DHCPv6Enable)
    {
        resetMulticasAddress_F();
        //Solicited multicast address
        //all node
        mar[0] = 0x33;
        mar[1] = 0x33;
        mar[2] = 0x00;
        mar[3] = 0x00;
        mar[4] = 0x00;
        mar[5] = 0x01;
        addMulticasAddress_F(mar);

        //ipv6[IPv6_ADR_LEN -1] = 10;
        //ipv6[IPv6_ADR_LEN -2] = 0;
        //ipv6[IPv6_ADR_LEN -3] = 168;
        //ipv6[IPv6_ADR_LEN -4] = 192;
        ipv6[0] = 0xfe;
        ipv6[1] = 0xc0;
        ipv6[IPv6_ADR_LEN -4] = 0x00;
        ipv6[IPv6_ADR_LEN -3] = 0x00;
        ipv6[IPv6_ADR_LEN -2] = 0x00;
        ipv6[IPv6_ADR_LEN -1] = 0x10;
        setIPv6Address_F(ipv6, eth0);
        /*setIPv6Address_F will do follow thing
        //Solicited multicast address for this IP
        mar[0] = 0x33;
        mar[1] = 0x33;
        mar[2] = 0xff;
        mar[3] = 0x00;
        mar[4] = 0x00;
        mar[5] = 0x10;
        addMulticasAddress_F(mar);
        setMulticastList_F();
        */
        //Version D must disbale all filter, or IPv6 multicast packets can not be received
        //REG32(IOREG_IOBASE + MAC_RxCR) = REG32(IOREG_IOBASE + MAC_RxCR) & 0x3fffffff;
    }

#if CONFIG_LLMNR_ENABLED
    if (dpconf->IPv6Enable)
    {
        mar[0] = 0x33;
        mar[1] = 0x33;
        mar[2] = 0x00;
        mar[3] = 0x01;
        mar[4] = 0x00;
        mar[5] = 0x03;
        addMulticasAddress_F(mar);
        setMulticastList_F();
        enableLLMNRPtl();
    }
#endif
    //SIP
    //ip = 192<<24 | 168<<16 | 0<<8 | 100;
    //REG32(IOREG_IOBASE + OCP_SIPV4_0)   = ip;

    //v = REG32(IOREG_IOBASE + MAC_RxCR) | 0x0200000;
    //REG32(IOREG_IOBASE + MAC_RxCR) = v;

    srand(REG32(TIMER_IOBASE + TIMER_CV));

    //To prevent lock
    ///GetMacAddr((unsigned char*)&dpconf->srcMacAddr[intf][0]);
    //mac address is getting from flash
#if !defined(CONFIG_DASH_ENABLED) && defined(CONFIG_HTTP_ENABLED)
    initTCPService();
#endif
}

static void TCPTask(void *p_arg)
{
    (void) p_arg;

    INT8U *rxbuf;
    INT8U err;
    PKT* pkt;
    int i = 0;
    RxQMsg *tmpmsg;

    while (OS_TRUE)
    {
        i = 0;
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
        tmpmsg = (RxQMsg *) OSQPend(RxQ,0,&err);

        lan_wake_up(tmpmsg->ptr+2, tmpmsg->length);

        rxbuf = tmpmsg->ptr;

        free(tmpmsg);
#else

        rxbuf = (INT8U *) OSQPend(RxQ,0,&err);
#endif

#if 1

        //Process at most 8 packet one time
        OSSemPend(SktSem, 0, &err);

        do
        {

#ifdef CONFIG_PROFILING
            mgtcycles[2] = rlx_cp3_get_counter_lo(0);
#endif
            if (rxbuf == NULL)
                break;

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
            if(i != 0)
            {
                tmpmsg = rxbuf;

                lan_wake_up(tmpmsg->ptr+2, tmpmsg->length);

                rxbuf = tmpmsg->ptr;

                free(tmpmsg);
            }
#endif

#ifdef CONFIG_CPU_SLEEP_ENABLED
            if (*rxbuf != ARP_PKT)  // Ignore ARP , too much ARP packet to Sleep.
            {
                wakeCPU();
            }
#endif

            if (*rxbuf == ARP_PKT)
            {
                sendARPPkt_F((ARPPkt*)rxbuf, 0, ARP_Reply, *(rxbuf+1));
                free(rxbuf);
            }
#if defined(CONFIG_8021X_ENABLED) && (CONFIG_VERSION >= IC_VERSION_DP_RevF)
            else if (*rxbuf != EAP_PKT)
#else
            else
#endif
            {
                pkt =  (PKT*) malloc(PKT_SIZE);
                memset(pkt, 0, PKT_SIZE);
                pkt->start = rxbuf;
                pkt->wp = rxbuf + ETH_HDR_SIZE;
                pkt->intf = *(rxbuf+1);


                ipInput_F(pkt);
                if (pkt)
                {
                    if (!pkt->queued)
                        freePkt_F(pkt);
                }
            }
#if defined(CONFIG_8021X_ENABLED) && (CONFIG_VERSION >= IC_VERSION_DP_RevF)
            else
            {
                if (dpconf->eaptls->eap >= 2)
                {
                    recvEAPReq((EAPPKT**)&rxbuf, strlen(rxbuf));
                }
                if (rxbuf != NULL)
                {
                    free(rxbuf);
                    rxbuf = NULL;
                }
            }
#endif
            i++;
            if (i > 8)
            {
                i = 0;
                break;
            }
        }
        while ((rxbuf = (INT8U *) OSQAccept(RxQ,&err)));
        OSSemPost(SktSem);
#else

#ifdef CONFIG_PROFILING
        mgtcycles[2] = rlx_cp3_get_counter_lo(0);
#endif
        if (rxbuf == NULL)
            continue;


        if (*rxbuf == ARP_PKT)
        {
            sendARPPkt_F((ARPPkt*)rxbuf, 0, ARP_Reply, *(rxbuf+1));
            free(rxbuf);
        }
        else
        {
            pkt =  (PKT*) malloc(PKT_SIZE);
            memset(pkt, 0, PKT_SIZE);
            pkt->start = rxbuf;
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
#endif
    }
}

#ifdef CONFIG_UDPSRV_Test
static void udpSrvTestTask(void *data)
{
    RTSkt* s = rtSkt(IPv6, IP_PROTO_UDP);
    unsigned char err = 0;
    PKT* rxPkt;
    PKT* txPkt;
    int status = 0, i = 0, len = 0;
    UDPAddrInfo addrInfo = {0};

    rtSktUDPBind_F(s, 1234);

    while (1)
    {
        do
        {
            rxPkt = rtSktRx_F(s, 0, &status);
        }
        while (rxPkt == 0);
        txPkt = (PKT*)allocPkt(rxPkt->len);

        getUDPAddressInfo_F(rxPkt, &addrInfo);
        memcpy(txPkt->wp, rxPkt->wp, rxPkt->len);
        for (i = 0; i < rxPkt->len ; i++)
            txPkt->wp[i] = txPkt->wp[i] + 0x10;

        rtSktUDPSendTo_F(s, txPkt, &addrInfo);
        freePkt(rxPkt);
    }
}
#endif

#ifdef CONFIG_Client_Test
static void clientTestTask(void *data)
{
    RTSkt* s = rtSkt(IPv6, IP_PROTO_TCP);
    //unsigned char ip[4] = {192,168,0,100};
    unsigned char ip[IPv6_ADR_LEN] = {0};
    PKT* pkt;
    int status = 0, j = 0, len = 0;
    int loop = 0;
    unsigned char err = 0;

    //When system startup, TCP/IP task is lowest priority, it will cause connection fail.
    //Wait for a while to prevent connection fail
    OSTimeDly(OS_TICKS_PER_SEC*10);
    srand(time(NULL));

    do
    {

        ip[0] = 0x20;
        ip[1] = 0x02;
        ip[2] = 0x3d;
        ip[3] = 0xd8;
        ip[4] = 0xa9;
        ip[5] = 0x7f;
        ip[IPv6_ADR_LEN - 4] = 0x3d;//1;
        ip[IPv6_ADR_LEN - 3] = 0xd8;//23;
        ip[IPv6_ADR_LEN - 2] = 0xa9;//1;
        ip[IPv6_ADR_LEN - 1] = 0x7f;//23;


        /*

        	ip[0] = 0x20;
        	ip[1] = 0x01;
        	ip[2] = 0x00;
        	ip[3] = 0x00;
        	ip[4] = 0x53;
        	ip[5] = 0xaa;
        	ip[6] = 0x06;
        	ip[7] = 0x4c;
        	ip[8] = 0xc;
        	ip[9] = 0xe2;
        	ip[10] = 0xfb;
        	ip[11] = 0xe8;
        	ip[12] = 0xc2;
        	ip[13] = 0x27;
        	ip[14] = 0x52;
        	ip[15] = 0xb1;
        	*/

        while (rtSktConnect_F(s, ip, 1234) == -1)
        {
        }

        len = 1 + rand() % 1000;

        pkt = (PKT*)allocPkt(len);

        for (j =0; j<len; j++)
            pkt->wp[j] = j %256;

        if (rtSktSend_F(s, pkt, 0) == SKTDISCONNECT)
        {
        }

        do
        {
            pkt = rtSktRx_F(s, 0, &status);

            if (status == SKTDISCONNECT)
            {
            }
        }
        while ((pkt == 0));

        if (pkt->len != len)
        {
        }

        for (j =0; j<len; j++)
        {
            if (pkt->wp[j] !=( j %256))
            {
            }
        }

        if (pkt)
        {
            freePkt(pkt);
        }



        rtSktClose_F(s);
        OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
        OSSemDel(s->txSem, OS_DEL_ALWAYS, &err);
        s->rxSem = s->txSem = 0;

        free(s);

        OSTimeDly(OS_TICKS_PER_SEC*5);

        s = rtSkt(IPv6, IP_PROTO_TCP);
    }
    while (loop++ < 10000);
}
#endif
