/*
 *******************************************************************************
 * Realtek Semiconductor Corp.
 *
 * Board Support Package for Realtek RLXOCP
 *
 *
 *******************************************************************************
 */

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include "bsp.h"
#include "lib.h"
#include "arp.h"
#include "usb.h"
#include "tcp.h"
#include "wcom.h"
#include "pldm.h"
#include "soap.h"
#include "smbios.h"
#include "rlx_library.h"
#include "soap.h"
#include "telnet.h"
#include "uart.h"
#include "flash.h"

#if(CONFIG_VERSION  >= IC_VERSION_EP_RevA) //+ for setIPAddress_withoutFlash
#include "hwpf.h"
extern TCAM_Allocation TCAMMem[NumOfTCAMSet];
#endif

extern _IPInfo IPInfo;

extern USBCB usbcb;

///OS_EVENT *USBQ;
///INT8U usb_init = 0;
extern asf_config_t *asfconfig;

void *USBMsg[16];
extern FWSIG *fwsig;
extern DPCONF dpdefault ;
ROM_EXTERN DPCONF *dpconf;
ROM_EXTERN INT8U smbuscount;
ROM_EXTERN INT8U RxDescIndex ;
//ROM_EXTERN OS_EVENT *RxQ;
ROM_EXTERN DebugMsg *debugmsg;
//OS_EVENT *PLDMSNRSem;
//OS_EVENT *PLDMSNRQ;
//void *PLDMSNRMsg[MAX_SENSOR_NUMS];
ROM_EXTERN smbiosrmcpdata *smbiosrmcpdataptr;
//extern INT8U smbus_param[][10];
extern const INT8U pldmsuccessbr[14];
extern const INT8U pldmsuccessbw[14];
extern const INT8U MCTPVerSupport[18];
ROM_EXTERN2 TCROPT *bootsel _ATTRIBUTE_ROM_BSS;

extern INT8U smbios_flag;
extern INT8U Timer_flag_VGA;

#ifdef CONFIG_SMBUSLOG_DEBUG
//INT8U smbx[SMBUS_ENTRIES][SMBUS_UNIT];
//INT8U smbidx = 0;
#endif

#if defined(CONFIG_SMBUSTRA_DEBUG) || defined(CONFIG_BUILDROM) || (CONFIG_VERSION  <= IC_VERSION_DP_RevF)
ROM_EXTERN2 OS_EVENT *SMBUSLOGQ;
ROM_EXTERN2 void *SMBUSLOG[SMBUS_ENTRIES];
#endif

ROM_EXTERN TCRCB tcrcb;


#ifdef CONFIG_TASK_STK_CHK
OS_STK_DATA stk_data[32];
OS_STK_DATA stktemp;
INT8U stkindex = 0;
#endif

void bsp_buf_alloc(void) _ATTRIBUTE_SRAM;
static void bsp_ostimer_handler(void) _ATTRIBUTE_TRAP;
//static void bsp_timer_init(void) _ATTRIBUTE_SRAM;
void bsp_timer_init(void) _ATTRIBUTE_SRAM;
#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)
void bsp_irq_disable (void) _ATTRIBUTE_SRAM;
#endif
INT8U check_mac_addr(INT8U *src) _ATTRIBUTE_SRAM;


ROM_EXTERN DebugMsg *debugmsg;
ROM_EXTERN INT8U SMBRxDescIndex;
ROM_EXTERN INT32U CPU_CLK_FREQ;
ROM_EXTERN INT8U *kcsrx , *kcstx ;
ROM_EXTERN INT32U KCSRXOK, KCSTXOK;
ROM_EXTERN INT8U tcr2reset;
ROM_EXTERN INT8U gmac_reset;
ROM_EXTERN INT32U smbiosdatalen;
ROM_EXTERN INT32U BSP_CPU_DEFAULT_FREQ = 250000000;

#if CONFIG_VERSION  == IC_VERSION_DP_RevF
OS_EVENT *TCRQ;
#else
ROM_EXTERN OS_EVENT *TCRQ;
#endif

ROM_EXTERN OS_EVENT *RxQ;

ROM_EXTERN OS_EVENT *SMBQ;
ROM_EXTERN OS_EVENT *EVTQ;
ROM_EXTERN OS_EVENT *AlertQ;
ROM_EXTERN OS_EVENT *SNMPQ;

//ROM_EXTERN int OSPushType;
//move to gmac.c
#if 0
#if CONFIG_VERSION > IC_VERSION_DP_RevA
ROM_EXTERN OS_EVENT *DASHREQSEM;//Shared resource => Task Request Atomic
ROM_EXTERN OS_EVENT *DASHREQLOCK;//Shared resource => Prevent Dash request(0x81) or ack(0x82) at the smae time
ROM_EXTERN OS_EVENT *DASH_OS_Response_Event;//Event
ROM_EXTERN OS_EVENT *DASH_OS_Push_Event;//Event

ROM_EXTERN unsigned char* DASH_ib_write_buffer;
ROM_EXTERN unsigned char* DASH_OS_PSH_Buf;
#endif
#endif

//ROM_EXTERN DASHDesc* DASH_ob_request;
//ROM_EXTERN DASHDesc* DASH_ib_write;

ROM_EXTERN flash_data_t dirty[ENDTBL];
ROM_EXTERN void *RxMsg[RxdescNumber];

void *TCRMsg[32];
///ROM_EXTERN void *SMBUSMsg[SMBRxdescNumber];
ROM_EXTERN void *EVTMsg[32];
ROM_EXTERN void *AlertMsg[32];
ROM_EXTERN void *SNMPMsg[32];
ROM_EXTERN INT8U bsp_done;
ROM_EXTERN INT8U *smbiosptr;
ROM_EXTERN INT8U *timestampdataptr;
ROM_EXTERN pldm_t *pldmdataptr;
ROM_EXTERN char *idata;

extern pldm_res_t *pldmresponse[7];

#ifdef CONFIG_PROFILING
INT32U mgtcycles[16] = {0};
INT32U avg[8] = {0};
#endif


#ifdef CONFIG_POWER_SAVING_ENABLED
	#if CONFIG_VERSION >= IC_VERSION_EP_RevB   
		INT8U pcie_up_timer = 0;		   
	 #endif
#endif	
	
#ifdef CONFIG_LANWAKE_NOTIFY_PCIE
	INT8U gIsLanWake = 0;
#endif

//extern INT8U smbus_param[NUM_SMBUS_CHIPSET][10];
//static INT16U tcrimr = 0x0004;
//tcr pci-e reset patch
#if CONFIG_OTG_ENABLED
#define IO_IB_ACC_DATA           0x00A0
#define IO_IB_ACC_SET            0x00A4
INT32U usb_mode_rd=0;
INT32U usb_mode_wt=0; 
INT32U usb_pcie_rd=0;
INT32U usb_pcie_wt=0;
#endif


void setIPAddress_withoutFlash(unsigned int ip, unsigned char intf)
{
    //Store with big endian
    //IPv4Cfg.HostIP[intf].addr = htonl(ip);
    //REG32(IOREG_IOBASE + MAC_RxCR) = 0x00;
    //REG32(IOREG_IOBASE + IO_DIPV4) = 0x0;
    OS_CPU_SR cpu_sr = 0;
    OS_Q_DATA rxqdata;
    INT8U *rxbuf;
    INT8U err;
    dpconf->HostIP[intf].addr = htonl(ip);
    #if(CONFIG_VERSION > IC_VERSION_DP_RevF )
    //Hw store with big endian
    memcpy(&(TCAMMem[TCAMDIPv4Set].data[UniIPv4Addr*IPv4_ADR_LEN]), &dpconf->HostIP[intf].addr, IPv4_ADR_LEN);
    setTCAMData_F(&TCAMMem[TCAMDIPv4Set]);
    #else
    REG32(IOREG_IOBASE+IO_DIPV4)= ip;
    bsp_bits_set(IO_DIPFILTER, 1, BIT_FILTERV4DIP0, 1);
    #endif

    //ip == 0 means DHCP initial
    if (ip != 0)
    {
        DEBUGMSG(LIB_DEBUG, "Set ip address %x\n", ip);
        ///srvReset_F(IPv4);
        IPInfo.IPv4Reset = 1;
        OS_ENTER_CRITICAL();
        OSQQuery(RxQ, &rxqdata);
        while (rxqdata.OSNMsgs--)
        {
            rxbuf = (INT8U *) OSQAccept(RxQ,&err);
            free(rxbuf);
        }
        OS_EXIT_CRITICAL();
    }

    //if (dpconf->ProvisioningState == PROVISIONED)
    //    setdirty(DPCONFIGTBL);
}


#if (CONFIG_VERSION  <= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
INT8U check_mac_addr(INT8U *src)
{
    INT8U rst1[MAC_ADDR_LEN] = {0x00,0x00,0x00,0x00,0x00,0x00};
    INT8U rst2[MAC_ADDR_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    if (memcmp(src, rst1, sizeof(rst1)) == 0)
        return 1;
    if (memcmp(src, rst2, sizeof(rst2)) == 0)
        return 1;

    return 0;
}
#endif

//field is the field offset, width means the bits this field occupy
#if (CONFIG_VERSION  < IC_VERSION_DP_RevF)
INT32U bsp_bits_get(INT8U offset, INT8U field, INT8U width)
{
    INT32U valmask = 0;
    INT32U count = 0;

    INT32U value = REGX(IOREG_IOBASE + offset);

    for (count = 0; count < width; count++)
        valmask |= ( 1 << (count+field));

    value = ((value & valmask) >> field);

    return value;
}
#endif

#if (CONFIG_VERSION  < IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
inline INT32U bsp_in_band_exist()
{
    return bsp_bits_get(MAC_IBREG, BIT_DRIVERRDY, 1);
}
#endif

#if (CONFIG_VERSION  <= IC_VERSION_DP_RevF)
void bsp_bits_set(INT8U offset, INT32U value, INT8U bits, INT8U width)
{
    INT16U count = 0;
    INT32U valmask = 0;

    REG8(IOREG_IOBASE + MAC_FLAG_OOB) = 1;
    while (REG8(IOREG_IOBASE + MAC_FLAG_IB))
    {
        if (REG32(IOREG_IOBASE + IO_TURN) != 1)
        {
            REG8(IOREG_IOBASE + MAC_FLAG_OOB) = 0;
            while (REG32(IOREG_IOBASE + IO_TURN) != 1)
            {
                //add timeout to prevent unconditional fault
                count++;
                bsp_wait(10);
                if (count >= 10000)
                {
                    REG32(IOREG_IOBASE + IO_TURN)  = 0;
                    REG8(IOREG_IOBASE + MAC_FLAG_OOB) = 0;
                    return ;
                }
            }
            REG8(IOREG_IOBASE + MAC_FLAG_OOB) = 1;
        }

        //add timeout to prevent unconditional fault
        count++;
        bsp_wait(10);
        if (count >= 10000)
        {
            REG32(IOREG_IOBASE + IO_TURN)  = 0;
            REG8(IOREG_IOBASE + MAC_FLAG_OOB) = 0;
            return ;
        }
    }

    //mutual exclusion

    for (count = 0; count < width; count++)
        valmask |= (1 << (count+bits));

    //only clear zero value
    value = (valmask & (value << bits));
    valmask ^= value;

    //always clear first, but only for the zero value
    REGX(IOREG_IOBASE + offset) &= ~valmask;

    REGX(IOREG_IOBASE + offset)  |= value ;

    REG32(IOREG_IOBASE + IO_TURN)   = 0;
    REG8(IOREG_IOBASE + MAC_FLAG_OOB)  = 0;
}
#endif

#if defined(CONFIG_BUILDROM)
//field is the field offset, width means the bits this field occupy
INT32U bsp_bits_get(INT16U offset, INT8U field, INT8U width)
{
    INT32U valmask = 0;
    INT32U count = 0;

    INT32U value;

    if((offset % 4) == 0)
        value = REG32(MAC_BASE_ADDR + offset);
    else if((offset % 2) == 0)
        value = REG16(MAC_BASE_ADDR + offset);
    else
        value = REG8(MAC_BASE_ADDR + offset);

    for (count = 0; count < width; count++)
        valmask |= ( 1 << (count+field));

    value = ((value & valmask) >> field);

    return value;
}

void bsp_bits_set(INT16U offset, INT32U value, INT8U bits, INT8U width)
{
    INT16U count = 0;
    INT32U valmask = 0;

    for (count = 0; count < width; count++)
        valmask |= (1 << (count+bits));

    //only clear zero value
    value = (valmask & (value << bits));
    valmask ^= value;

    //always clear first, but only for the zero value
    if((offset % 4) == 0 )
    {
        REG32(MAC_BASE_ADDR + offset) &= ~valmask;

        REG32(MAC_BASE_ADDR + offset)  |= value ;
    }
    else if ((offset %2) == 0)
    {
        REG16(MAC_BASE_ADDR + offset) &= ~valmask;

        REG16(MAC_BASE_ADDR + offset)  |= value ;

    }
    else
    {
        REG8(MAC_BASE_ADDR + offset) &= ~valmask;

        REG8(MAC_BASE_ADDR + offset)  |= value ;

    }

}
void bsp_bits_set_mutex(INT16U offset, INT32U value, INT8U bits, INT8U width)
{
    INT16U count = 0;
    INT32U valmask = 0;

    REG32(MAC_BASE_ADDR + MAC_FLAG_OOB) = 1;

    while (REG32(MAC_BASE_ADDR + MAC_FLAG_IB))
    {
        if (REG32(MAC_BASE_ADDR + MAC_MUTEX_TURN) != 1)
        {
            REG32(MAC_BASE_ADDR + MAC_FLAG_OOB) = 0;

            while (REG32(MAC_BASE_ADDR + MAC_MUTEX_TURN) != 1)
            {
                //add timeout to prevent unconditional fault
                count++;
                bsp_wait(10);
                if (count >= 10000)
                {
                    REG32(MAC_BASE_ADDR + MAC_MUTEX_TURN)  = 0;
                    REG32(MAC_BASE_ADDR + MAC_FLAG_OOB) = 0;
                    return ;
                }
            }
            REG32(MAC_BASE_ADDR + MAC_FLAG_OOB) = 1;
        }

        //add timeout to prevent unconditional fault
        count++;
        bsp_wait(10);
        if (count >= 10000)
        {
            REG32(MAC_BASE_ADDR + MAC_MUTEX_TURN)  = 0;
            REG32(MAC_BASE_ADDR + MAC_FLAG_OOB) = 0;
            return ;
        }
    }

    //mutual exclusion

    for (count = 0; count < width; count++)
        valmask |= (1 << (count+bits));

    //only clear zero value
    value = (valmask & (value << bits));
    valmask ^= value;

    //always clear first, but only for the zero value
    if((offset % 4) == 0 )
    {
        REG32(MAC_BASE_ADDR + offset) &= ~valmask;

        REG32(MAC_BASE_ADDR + offset)  |= value ;
    }
    else if ((offset %2) == 0)
    {
        REG16(MAC_BASE_ADDR + offset) &= ~valmask;

        REG16(MAC_BASE_ADDR + offset)  |= value ;

    }
    else
    {
        REG8(MAC_BASE_ADDR + offset) &= ~valmask;

        REG8(MAC_BASE_ADDR + offset)  |= value ;

    }

    REG32(MAC_BASE_ADDR + MAC_MUTEX_TURN)   = 0;
    REG32(MAC_BASE_ADDR + MAC_FLAG_OOB)  = 0;
}

#endif

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  <= IC_VERSION_EP_RevA)
void disable_master_engine()
{

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    REG32(IOREG_IOBASE + MAC_RxCR) &= 0xFFFFFF00; //disable gmac
    REG8(IOREG_IOBASE+MAC_CMD)  &= 0x80;
#else
    bsp_gmac_disable();
#endif

    //disable SMBus with alert pin unchanged
    //the ROM within DP RevD/F would set this to 0x00 (would change the pin)
    REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= 0x08;

    REG8(TCR_BASE_ADDR + TCR_CONF0)  = 0x00; //disable tcr
	//FP No TCR2
    //REG8(TCR2_BASE_ADDR + TCR_CONF0) = 0x00; //disable tcr2
    REG8(KCS_BASE + KCS_CONF) = 0x00;    //disable kcs

    REG32(TIMER_IOBASE + TIMER_CR) = 0x00000000;  //disable timer
}
#endif

/*
 *******************************************************************************
 * bsp_ostimer_handler()
 *
 * Description: Timer interrupt handler. Used for OS counter
 *
 * Arguments  : None
 *
 * Returns    : None
 *******************************************************************************
 */
void bsp_ostimer_handler(void)
{
	#ifdef CONFIG_DEBUG
	static INT32U count = 0;
	#endif
	volatile INT8S eoi;
	
    static INT8U timer_tick = 0;

	OSTimeTick();
	timer_tick++;
	
	if (timer_tick==3)
	{
		#ifdef CONFIG_VGA_TEST_ENABLED
		Timer_flag_VGA=1;
		#endif
		timer_tick=0;
	}
	
    eoi = REG32(TIMER_IOBASE+TIMER_EOI);
	
}

#if 0
void
bsp_ostimer_handler(void)
{
    //static INT8U asfidx = 0;
    //static INT16U count =  3 ;
    static INT8U tcrcnt = 0;
#ifdef CONFIG_DEBUG
    static INT32U count = 0;
#endif
    volatile INT8S eoi;

    //INT8U err = 0;
    eoi = REG32(TIMER_IOBASE+TIMER_EOI);

#ifdef CONFIG_DOORBELL_CHECK_ENABLED
	INT32U rd_tmp;
	static INT8U timercnt = 0;
	timercnt++;	

	if(timercnt == dpconf->ostmr_unit)
#endif		
	{
#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    	if (fwsig->SIG == NORMAL_SIG && fwsig->timerHook == NEED_PATCH)
        	fwsig->timerHook_F();
#endif

    	if(smbiosrmcpdataptr->PollType != POLL_STOP)
    	{
        	smbiosrmcpdataptr->PollTimeOut++;
        	if (smbiosrmcpdataptr->PollTimeOut ==  255)
            	smbiosrmcpdataptr->PollType = POLL_NONE;
    	}
    	else
    	{
        	smbiosrmcpdataptr->PollTimeOut = 0;
    	}
#if 0//redundant code , tom mask@21040219
    	if (REG8(SMBUS_BASE_ADDR+SEIO_Status)&0x01)
    	{
       	 smbuscount++;

        	if (smbuscount == 8)
        	{
            	REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= 0x08;
            	REG8(SMBUS_BASE_ADDR+SEIO_Status) = 0x00;

            	bsp_smbus_init(0);

            	smbuscount = 0;
        	}
    	}
#endif
#if 0
    	//1.5 secs timeout
    	if (fwsig->SIG == NORMAL_SIG)
    	{
        	smbiosrmcpdataptr->PollTimeOut++;
        	if (smbiosrmcpdataptr->PollTimeOut ==  150)
            	smbiosrmcpdataptr->PollType = POLL_NONE;

        	if (fwsig->SIG == NORMAL_SIG && dpconf->asfpoll)
            	smbus_asfpoll_F();
    	}
#endif

#ifdef CONFIG_TASK_STK_CHK
    	err = OSTaskStkChk(stkindex, &stktemp);
    	if (err == OS_ERR_NONE)
    	{
        	if (stktemp.OSUsed > stk_data[stkindex].OSUsed)
        	{
            	stk_data[stkindex].OSUsed = stktemp.OSUsed;
            	stk_data[stkindex].OSFree = stktemp.OSFree;
        	}
    	}
    	stkindex = (stkindex + 1 ) % 30;
#endif

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    	if (fwsig->SIG == NORMAL_SIG && fwsig->SMBPatch == NEED_PATCH)
#else
    	if(1)
#endif
    	{
        	if (REG8(SMBUS_BASE_ADDR+SEIO_Status)&0x01)
            	smbuscount++;
        	else
            	smbuscount=0;//reset counter        	    
        
        	if (smbuscount == 100)//smbus reset timer = 1s
        	{
printf("Reset SMBus\n");
#ifdef CONFIG_GPIO_DEBUG
            	GPIOSet(0);
#endif
            	DEBUGMSG(CRITICAL_ERROR, "!!!!!!!!!!!!!! Reset SMBus # %06d!!!!!!!!!!!!!!\n", ++count);
            	REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= 0x08;
            	REG8(SMBUS_BASE_ADDR+SEIO_Status) = 0x00;

            	bsp_smbus_init(0);
#if defined(CONFIG_ENHANCE_SMB_SAMPLE_RATE)&&defined(CONFIG_ENHANCE_SMB_SAMPLE_RATE)
smbus_frequency(3);
#endif
            	smbuscount = 0;

        	}
    	}

#ifdef CONFIG_TCR_ENABLED
    	if (tcrcb.bootopt && bootsel != 0 )
    	{
        	tcrcnt++;
        	//from 10*10ms of 1st interrupt when getting TCR Rx packet
        	if(tcrcnt > 10)
        	{
           	 tcr_received(bootsel->code, bootsel->len);
            	if(tcrcnt > 15)
            	{
                	bootsel = 0;
                	tcrcb.bootopt = 0;
                	tcrcnt = 0;
            	}
        	}
    	}
#endif
    	OSTimeTick();
#ifdef CONFIG_DOORBELL_CHECK_ENABLED	   	
    	timercnt = 0;
#endif    	
	}
	
#ifdef CONFIG_DOORBELL_CHECK_ENABLED
	if(!dpconf->usbotg){
		rd_tmp=Rd_IBIO(USBCMD);
		if((rd_tmp&0x40) == 0x40){
			Wt_IBIO(USBCMD, (rd_tmp&0xFFFFFFBF));//turn off doorbell  
 		}
 	}
#endif
}
#endif
/*
 *******************************************************************************
 * bsp_timer_init()
 *
 * Description: Initialize all timer devices. Currently only timer1 is used for OS timer
 * Arguments  : None
 * Returns    : None
 *******************************************************************************
 */
 
void bsp_timer_init(void)
{
    INT8U divide;

    divide = (REG8(CPU_REG) & 0x07); //only 3 bits

	if(REG32(CPU2_IOBASE + 0x0C) & 0x40000000)//CPU DCO
	{
		//CPU_CLK_FREQ = ((REG32(CPU_BASE + FREQ_CAL_REG1) >> 16 )*25/100)*1000000;
		CPU_CLK_FREQ = ((REG32(CPU_BASE + FREQ_CAL_REG1) >> 16 )*25)*10000;
	}
	else//CPU default 250MHz
	{
		CPU_CLK_FREQ = BSP_CPU_DEFAULT_FREQ;
	}

	
	if((REG32(CPU_DUMMY0_REG)&0x3ff)==0x3ff)
		CPU_CLK_FREQ = (CPU_CLK_FREQ >> divide);
	
    REG32(TIMER_IOBASE+TIMER_CR) = 0x00000000;             //disable first
    REG32(TIMER_IOBASE+TIMER_LC) = (CPU_CLK_FREQ>>1) / HZ;      //set counter	
    REG32(TIMER_IOBASE+TIMER_CR) = 0x00000003;             //enable
    
	rlx_irq_register(BSP_TIMER1_IRQ,bsp_ostimer_handler);
}

void bsp_timer_reinit(void)
{
    INT8U divide;
		
	if(REG32(CPU2_IOBASE + 0x0C) & 0x40000000)//CPU DCO
	{
		//CPU_CLK_FREQ = ((REG32(CPU_BASE + FREQ_CAL_REG1) >> 16 )*25/100)*1000000;
		CPU_CLK_FREQ = ((REG32(CPU_BASE + FREQ_CAL_REG1) >> 16 )*25)*10000;
	}
	else//CPU default 250MHz
	{
    	if(bsp_bits_get(MAC_MAC_STATUS, BIT_TP1000, 1)){
	    	divide = 0;
    	}
    	else{
    		divide = (REG8(CPU_REG) & 0x07); //only 3 bits
    	}
        CPU_CLK_FREQ = (BSP_CPU_DEFAULT_FREQ >> divide);
    }
#ifdef CONFIG_DOORBELL_CHECK_ENABLED	    	
    if (usbcb.usbstate != DISABLED && !(dpconf->usbotg) && bsp_in_band_exist()){
    	CPU_CLK_FREQ = CPU_CLK_FREQ / 10;	
    	dpconf->ostmr_unit = 10;//timer unit=1ms
    }
    else{
    	dpconf->ostmr_unit = 1;//timer_unit=10ms
    }
#endif
    REG32(TIMER_IOBASE+TIMER_CR) = 0x00000000;             //disable first
    REG32(TIMER_IOBASE+TIMER_LC) = (CPU_CLK_FREQ>>1) / HZ;      //set counter
    REG32(TIMER_IOBASE+TIMER_CR) = 0x00000003;             //enable


    //rlx_irq_register(BSP_TIMER1_IRQ,bsp_ostimer_handler);
}

#if CONFIG_VERSION  < IC_VERSION_DP_RevF
INT8U bsp_phy_access(INT32U value, INT16U *dst)
{

    INT8U i=0;
    INT32U valuex;


    //indicate in band exist, OOB should not access phy any more
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    if (bsp_bits_get(IO_CONFIG, BIT_DRIVERRDY, 1))
        return 1;
    //clear phy access bit if it is set
    bsp_bits_set(IO_HWFunCtr, 0, BIT_PHYCLR, 1);
#else
    if (bsp_bits_get(IO_CONFIG1, BIT_DRIVERRDY, 1))
        return 1;

    //clear phy access bit if it is set
    bsp_bits_set(IO_CONFIG1, 0, BIT_PHYCLR, 1);
#endif

    REG32(IOREG_IOBASE + IO_PHY)  = value;

    //check 5 times, each time dealy 100 usec
    for (i = 0 ; i < 5; i++)
    {
        valuex = REG32(IOREG_IOBASE + IO_PHY) ;
        valuex = (valuex ^ value) >> 31;
        if (valuex)
            break;
        bsp_wait(100);
    }

    if (!(value & 0x80000000))
    {
        *dst = (REG32(IOREG_IOBASE + IO_PHY) & 0x0000FFFF);
        return 0;
    }

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    bsp_bits_set(IO_HWFunCtr, 1, BIT_PHYCLR, 1);
#else
    //clear phy access bit if it is set
    bsp_bits_set(IO_CONFIG1, 1, BIT_PHYCLR, 1);
#endif

    return 0;

}
#endif

#if 0
void bsp_gphy_set()
{
    INT16U linkspeed;
    bsp_gphy_write(0x801F0000);
    //back to page 0
    bsp_gphy_write(0x00110000);
    linkspeed = ((REG32(IOREG_IOBASE + IO_PHY) & 0x0000C000) >> 14) ;
    //read bit 14-15 for speed, 00, 01, 10 (10, 100, 1000)
    if (linkspeed == 0x00)
        bsp_gphy_write(0x801004EE);
    else
        bsp_gphy_write(0x801001EE);

    //patch in-band & oob issue
    //bsp_gphy_write(0x0);

}
#endif

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  == IC_VERSION_DP_RevA)
/*
 *******************************************************************************
 * bsp_irq_disable()
 *
 * Description: Disable all interrupt in processor.
 *
 * Arguments  : None
 *
 * Returns    : None
 *******************************************************************************
 */
void bsp_irq_disable (void)
{
    OS_CPU_SR  cpu_sr = 0;

    OS_ENTER_CRITICAL();
}
#endif

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF) || (CONFIG_VERSION  == IC_VERSION_EP_RevB)
void OOBHwRest()
{
#if CONFIG_VERSION  >= IC_VERSION_EP_RevB
    REG32(KVM_BASE + CMAC_SYN_REG) = 0x2a;
    REG32(MAC_BASE_ADDR + MAC_OOB2IB_INT) = 1;
#elif CONFIG_VERSION  == IC_VERSION_DP_RevF
    //Disable
    bsp_bits_set(IO_PortCtr, 0, BIT_PCIEBDGEN, 1);
    //enable
    bsp_bits_set(IO_PortCtr, 1, BIT_PCIEBDGEN, 1);
#else
    //Disable
    bsp_bits_set(IO_CONFIG1, 0, BIT_PCIEBDGEN, 1);
    //enable
    bsp_bits_set(IO_CONFIG1, 1, BIT_PCIEBDGEN, 1);
#endif
}
#endif

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)

int bsp_AP_exist()
{
    if (!bsp_bits_get(MAC_IBREG, BIT_APRDY, 1) || !bsp_bits_get(MAC_IBREG, BIT_DRIVERRDY, 1))
        return 0;
    else
        return 1;
}

#endif



#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)
void GetMacAddr(INT8U* addr, int intf)
{
    INT32U val32;
    INT16U val16;

	if(intf == eth0)
	{
    val32 = REG32(MAC_BASE_ADDR + MAC_IDR0);
    memcpy(addr, (INT8U *) &val32, 4);

    val32 = REG32(MAC_BASE_ADDR + MAC_IDR4);
    val16 = val32 & 0x0000FFFF;

    memcpy(addr + 4, &val16, 2);
}
	else
	{
		GetPCIMacAddr(addr);
	}
}

void SetMacAddr(INT8U* addr)
{
    INT32U v = 0;

    memcpy(&v, addr, 4);
    REG32(MAC_BASE_ADDR + MAC_IDR0) = v;

    memcpy(&v, addr + 4, 4);
    REG32(MAC_BASE_ADDR + MAC_IDR4) = v & 0x0000FFFF;
}
#endif




/*
 *******************************************************************************
 * bsp_setup()
 *
 * Description: platform initializer
 *
 * Arguments  : None
 *
 * Returns    : None
 *******************************************************************************
 */
#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION <= IC_VERSION_DP_RevF)
void bsp_buf_alloc(void)
{
    INT8U pldmtmp[14];

    //Software related information
    idata=malloc(1024);
    memset(idata, 0, 1024);
    strncpy(idata,(char *) SYSTEM_SW_INFO,1023);
	
    debugmsg = malloc(512);
    memset(debugmsg, 0, 512);
    //allocate the shared memory for rmcp and smbus
    smbiosrmcpdataptr = (smbiosrmcpdata *) malloc(sizeof(smbiosrmcpdata));
    memset(smbiosrmcpdataptr,0, sizeof(smbiosrmcpdata));

    //stop all sensor polling
    smbiosrmcpdataptr->PollType = POLL_STOP;

    //allocte buffer for smbios, eventlog
    timestampdataptr = malloc(4096);
    //another 4K for patching FSC D2618 issue
    smbiosptr = malloc(4096);
    //clear the timestamp
    smbiosptr = timestampdataptr + SMBIOS_DATA_OFFSET;
    memcpy((void*) timestampdataptr , (void*) SMBIOS_ROM_START_ADDR, 0x2000);
    dirty[SMBIOSTBL].addr = (INT8U *) timestampdataptr;
    dirty[SMBIOSTBL].flashaddr = (INT8U *) SMBIOS_ROM_START_ADDR;
    dirty[SMBIOSTBL].length = *(INT16U *) (smbiosptr + 0x16) + SMBIOS_DATA_OFFSET + SMBIOS_HEADER_SIZE;

#if 0
    dpconf = malloc(sizeof(DPCONF));
    memcpy((void *) dpconf, (void *) DPCONF_ROM_START_ADDR, sizeof(DPCONF));
    dpconf->restart = 0;
#endif

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    dpconf->efusekey = GetKeyStatus();
#endif

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    if (fwsig->CONFSIG != VALID_CONF_SIG  &&  dpconf->HostIP[eth0].addr == 0xFFFFFFFF)
#else
    if (dpconf->HostIP[eth0].addr == 0xFFFFFFFF)
#endif
    {
        memcpy(dpconf, &dpdefault, sizeof(DPCONF));
    }
    //prevent flash write back incorrect value

    //set ip address to 0 before any task started
    //if(dpconf->DHCPv4Enable)
    //   setIPAddress_F(0);
    IPInfo.IPv4Reset = 0;
    IPInfo.IPv6Reset = 0;

    dirty[DPCONFIGTBL].addr = (INT8U *) dpconf;
    dirty[DPCONFIGTBL].flashaddr = (INT8U *) DPCONF_ROM_START_ADDR;
    dirty[DPCONFIGTBL].length = sizeof(DPCONF);
    //check_config(dpconf);
    //should check configuration for correct setting, especially for time and
    //number related counts

    //copy from flash to dmem
    //put the major, minor, extra version numbers into gmac IO

    //default to use ASF method, since PLDM may not be implemented
    dirty[PLDMTBL].flashaddr = (INT8U *) SYSTEM_PLDM_DATA;
    pldmdataptr = malloc(sizeof(pldm_t));
    memcpy((void *) pldmdataptr, (void *) SYSTEM_PLDM_DATA, sizeof(pldm_t));
    pldmdataptr->numwrite[0] = (sizeof(pldm_t) - 1 + 64)/64;
    dirty[PLDMTBL].addr = (INT8U *) pldmdataptr;

    pldmdataptr->ptr[0] = malloc(PLDM_STR_TBL_SIZE);
    pldmdataptr->ptr[1] = malloc(PLDM_ATT_TBL_SIZE);
    pldmdataptr->ptr[2] = malloc(PLDM_ATT_TBL_SIZE);
    pldmdataptr->xferptr[0] = malloc(PLDM_STR_TBL_SIZE);
    pldmdataptr->xferptr[1] = malloc(PLDM_ATT_TBL_SIZE);
    pldmdataptr->xferptr[2] = malloc(PLDM_ATT_TBL_SIZE);

    memcpy(pldmdataptr->ptr[0], (void *) PLDM_STR_TBL, pldmdataptr->len[0]);
    memcpy(pldmdataptr->ptr[1], (void *) PLDM_ATR_TBL, pldmdataptr->len[1]);
    memcpy(pldmdataptr->ptr[2], (void *) PLDM_VAL_TBL, pldmdataptr->len[2]);
    pldmdataptr->dirty = 0;
    pldmdataptr->TBL1 = 1;
    pldmdataptr->TBL2 = 1;
    pldmdataptr->TBL3 = 1;
    //default to use ASF method, since PLDM may not be implemented

    //initialize pldm response buffer
    //the first part for sending attribute pending value table
    //re-assign slave address

    ///bsp_load_asf_default();

    if(dpconf->pldmtype == PLDM_BLOCK_READ)
    {
        memcpy(pldmtmp, pldmsuccessbr, 14);
        pldmtmp[3] = dpconf->pldmslaveaddr;
        //smbus_param[dpconf->chipset][5];
        if (dpconf->bios == AMIBIOS)
        {
            pldmtmp[4] = 0x01;
            pldmtmp[6] = 0x00;
            pldmtmp[7] = 0xC8;
        }
    }
    else
    {
        memcpy(pldmtmp, pldmsuccessbw, 14);
        //assign smbus address of the chipset
        pldmtmp[0] =  dpconf->pldmslaveaddr;
    }
    //smbus_param[dpconf->chipset][5];



    if(dpconf->pldmtype == PLDM_BLOCK_READ)
        pldmresponse[0] = malloc(256) + 2;
    else
        pldmresponse[0] = malloc(256);

    memcpy(pldmresponse[0], pldmtmp, 14);
    pldmresponse[0]->mctpstop = 0;
    pldmresponse[0]->length = 0x0f;
    pldmresponse[0]->val[4] = 0x05;
    pldmresponse[0]->pldmcmd = 0x01;
    //pldmresponse[0]->val[3] = 0x04;

    pldmresponse[1] = pldmresponse[0] + 1;
    if (dpconf->bios == PHOENIXBIOS)
    {
        memcpy(pldmresponse[1], pldmtmp, 14);
        pldmresponse[1]->mctpstart = 0;
    }
    else
        memset(pldmresponse[1], 0 , 39);
    pldmresponse[2] = pldmresponse[0] + 2;
    memcpy(pldmresponse[2], pldmtmp, 14);

    //0, 1, 2 are used for return attribue pending value table
    //need to transfer more than 1 times

    if(dpconf->pldmtype == PLDM_BLOCK_READ)
        pldmresponse[3] = malloc(512) + 2;
    else
        pldmresponse[3] = malloc(512);

    memcpy(pldmresponse[3], pldmtmp, 14);
    //4 is always used to transfer byte count 0x0A
    pldmresponse[4] = pldmresponse[3] + 1;
    memcpy(pldmresponse[4], pldmtmp, 14);
    //5 is alwyas used to tranfer table tags, which is 0x16 long
    pldmresponse[5] = pldmresponse[3] + 2;
    memcpy(pldmresponse[5], pldmtmp, 14);
    pldmresponse[5]->length = 0x16;

    pldmresponse[6] = pldmresponse[3] + 3;
    memcpy(pldmresponse[6],MCTPVerSupport,18);
    //memcpy(pldmresponse[3], pldmtmp, 14);
    //pldmresponse[6]->length = 0x0B;

    //RxQ for Rx packets, TxPenQ for release Tx descriptors
    RxQ = OSQCreate(RxMsg,RxdescNumber);
    EVTQ = OSQCreate(EVTMsg,32);

#ifdef CONFIG_DASH_ENABLED
    AlertQ = OSQCreate(AlertMsg,32);
#endif

#ifdef CONFIG_ASF_ENABLED
    SNMPQ  = OSQCreate(SNMPMsg, 32);
#endif

}
#endif





#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION == IC_VERSION_DP_RevA)
inline void bsp_wdt_kick()
{
    REG32(WDT_BASE_ADDR + WDT_CRR) = 0x76;
}
#endif

#if CONFIG_VERSION  == IC_VERSION_DP_RevF

#if defined(CONFIG_BUILDROM)
void bsp_ison_handler(void)
{
    rlx_irq_set_mask(BSP_ISON_IRQ, OS_TRUE);
    bsp_set_sstate(S_S0);
    rlx_irq_set_mask(BSP_ISO_IRQ, OS_FALSE);
}
#endif

void bsp_iso_handler(void)
{
    rlx_irq_set_mask(BSP_ISO_IRQ, OS_TRUE);
    if (bsp_get_sstate() == S_S0)
        bsp_set_sstate(S_S5);
    rlx_irq_set_mask(BSP_ISON_IRQ, OS_FALSE);
}
#endif

#ifdef CONFIG_CPU_SLEEP_ENABLED
void CPU_wakeup_handler(void)
{
	INT16U  tmp=0;

	tmp = REG16(MAC_BASE_ADDR+MAC_EXT_INT+2)&(~0x02);
	REG16(MAC_BASE_ADDR+MAC_EXT_INT+2) = 0; //clear IRQ15 IMR
	REG16(MAC_BASE_ADDR+MAC_EXT_INT) = REG16(MAC_BASE_ADDR+MAC_EXT_INT) ; //clear ISR
	REG16(MAC_BASE_ADDR+MAC_EXT_INT+2) = tmp; //resotre IRQ_15 IMR except wake up 
}
#endif

#if 0
#define BSP_ISON_IRQ         RLX_IRQ_ID_14

void bsp_ison_handler_patch(void)
{
	DisableGigaPowerSavingParameter();
	pcie_up_timer = 12;	

    rlx_irq_set_mask(BSP_ISON_IRQ, OS_TRUE);
    bsp_set_sstate(S_S0);
    rlx_irq_set_mask(BSP_ISO_IRQ, OS_FALSE);
}
#endif

void SetPowerSavingParameter(){

	INT8U regValue;

	regValue = 0x3B;
	#ifdef CONFIG_POWER_SAVING_ENABLED
	       #if CONFIG_VERSION >= IC_VERSION_EP_RevB
	           //0x60 : MAC_CPUSLOW
	           //1. REG32(0xB9000000 + 4) |= (0x43 << 8); 
	           REG32(CPU_REG) |= (0x43 << 8);  //+base on Firmware Note v1.2
	           REG16(MAC_BASE_ADDR+0x60) = 0x0202;
	           REG8(MAC_BASE_ADDR+0x60+2) = 0x02;
		#ifdef CONFIG_GIGAIDLE_SAVING_ENABLED   
	           regValue = regValue + 0x04;
		#endif
		    REG8(MAC_BASE_ADDR+0x60+3) = regValue;	

		    //--Check CPU divider manually
		    OSTimeDly(25) ;
		    bsp_timer_reinit();			   
	       #endif
       #endif
}
/*
void DisableGigaPowerSavingParameter(){ //Disable Giga Idle Power Saving
	INT8U tmpReg;
	
	#ifdef CONFIG_POWER_SAVING_ENABLED
		#ifdef CONFIG_GIGAIDLE_SAVING_ENABLED   
	       #if CONFIG_VERSION == IC_VERSION_EP_RevB           
			tmpReg = REG8(MAC_BASE_ADDR+0x60+3) ;
			tmpReg = tmpReg&0xFB;
			REG8(MAC_BASE_ADDR+0x60+3) = tmpReg;
		    //--Check CPU divider manually		    
	       #endif
       #endif
       #endif
}

void EnableGigaPowerSavingParameter(){//Enable Giga Idle Power Saving
	INT8U tmpReg;
	
	#ifdef CONFIG_POWER_SAVING_ENABLED
		#ifdef CONFIG_GIGAIDLE_SAVING_ENABLED   
	       #if CONFIG_VERSION == IC_VERSION_EP_RevB           
			tmpReg = REG8(MAC_BASE_ADDR+0x60+3) ;		   
			tmpReg =  tmpReg|0x04;
			REG8(MAC_BASE_ADDR+0x60+3) = tmpReg;
		    //--Check CPU divider manually		    
	       #endif
       #endif
       #endif
}
*/
void
bsp_setup (void)
{
#ifdef CONFIG_MEM_ADJUST_1
	INT8U* tmp1;
#endif


    //indicate that firmware update mode to communicate with client tool
    
    //define MAC_OOBREG for DP_RevF as well
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    if (fwsig->SIG != NORMAL_SIG)
        bsp_bits_set(MAC_OOBREG, 1, BIT_FWMODE, 1);
    else
        bsp_bits_set(MAC_OOBREG, 0, BIT_FWMODE, 1);

#endif

#ifdef CONFIG_CPU_SLEEP_ENABLED
rlx_irq_register(BSP_ISO_IRQ, CPU_wakeup_handler);
#endif

#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
    //clear the cpu reset bit for rom to call cpu reset
    REG8(0xB9000004) &= ~0x08;

    //if dash already enabled, do not set the power state
    if (!bsp_bits_get(IO_CONFIG1, BIT_DASHEN, 1))
    {
        bsp_set_sstate(S_UNKNOWN);
    }
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    else //means OOB has run for a while, but CPU Reset
    {
        //WDT not set, which means that WDT expire
        if (bsp_bits_get(IO_CONFIG1, BIT_OOB_WDT, 1))
        {
            //inform IB about the change
            REG16(IOREG_IOBASE+MAC_IMR) |= 0x0080;
            bsp_wait(1);
            REG16(IOREG_IOBASE+MAC_IMR) &= ~0x0080;
        }

    }
    bsp_bits_set(IO_CONFIG1, 1, BIT_OOB_WDT, 1);
#endif

#if CONFIG_VERSION >= IC_VERSION_DP_RevD
    //increase the flash speed to 2x
    //REG8(0xB9000004) = 0x40;
#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    if (fwsig->FlashSpeed == 0x40)
        REG8(0xB9000004) = 0x40;
    else
        REG8(0xB9000004) = 0x00;
#else
    REG8(0xB9000004) = 0x40;
#endif
#else
    REG8(0xB9000004) = 0x00;
#endif
#endif


    bsp_timer_init();		    /* Initialize timer */

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
    wake_up_pattern_init();
#endif

    bsp_buf_alloc();

#ifdef CONFIG_MAIL_WAKEUP_ENABLED
    dirty[DPCONFIGTBL].length = sizeof(DPCONF);	 //+briank.rtk
#endif

	#ifdef CONFIG_MEM_ADJUST_1
	tmp1 = timestampdataptr + 0x1000;
	free(tmp1);
	tmp1 = NULL;
	#endif


    bsp_gmac_init();
    event_log_load();

#ifdef CONFIG_TCR_ENABLED
    bsp_tcr_init();
    //REG16(TCR_BASE_ADDR + TCR_IMR) = 0x0006;
#endif

	bsp_kcs_init_patch();


    if (dpconf->vendor == FSC || dpconf->vendor == SAMSUNG)
        bsp_tcr2_init();

    bsp_smbus_init(1);
#if defined(CONFIG_ENHANCE_SMB_SAMPLE_RATE)&&defined(CONFIG_ENHANCE_SMB_SAMPLE_RATE)
smbus_frequency(3);
#endif
    //Fujitsu D3230 Standard PLDM, Method: SMBUS Block_Read Protocol
    #if defined(CONFIG_BLOCK_READ) && defined(CONFIG_BIOS_AMI)
    handle_pldm = handle_pldm_br_ami;
    smbus_process_pldm = smbus_process_pldm_br_ami;
    #endif
    #ifdef CONFIG_OTG_ENABLED
    usb_otg_patch();
    #endif  
	#ifdef CONFIG_USB_ENABLED	
    REG32(0xBAF00008) |= 0x04 ;
    usb_config_init();
	#endif
    #ifdef CONFIG_OTG_ENABLED
    usbcb.bsp_usb_enabled = otg_usb_enabled;//patch for usb otg, tomadd@20131212
    Wt_UPHY(0xE2,0x79);//tomadd @20131218 (base on firmware note v1.1)
    #endif

    if(!bsp_set_asftbl((asf_header_t *) SYSTEM_ASF_TABLE))
        ParserASFTable();

    if(dpconf->pldmfromasf)
        bsp_set_pldm_slave_address(asfconfig->asf_rctl[0].slaveaddr & 0xFE);

#if defined(CONFIG_USB_ENABLED) ///&& !defined(CONFIG_BUILDROM)
    usbcb.bsp_usb_init();


    #if defined(CONFIG_EHCI_INTEP)
    ehci_intep_disabled(1);
    ehci_intep_disabled(2);
    #endif
    usbcb.bsp_usb_disabled();

#endif

#if defined(CONFIG_KCS_ENABLED) && (CONFIG_VERSION <= IC_VERSION_DP_RevD)
    bsp_kcs_init();
#endif

#if defined(CONFIG_CODEC_ENABLED) && ((CONFIG_VERSION  <= IC_VERSION_DP_RevF) || (CONFIG_VERSION  >= IC_VERSION_FP_RevA))
    bsp_tls_init();
#endif

    inituser();


    //need to check EP part
#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
#if CONFIG_VERSION >= IC_VERSION_DP_RevD
    REG32(IOREG_IOBASE + IO_PROFILE1) = 0xAFC01F00;
#else
    REG32(IOREG_IOBASE + IO_PROFILE1) = 0x8DC01F00;
#endif
    REG32(IOREG_IOBASE + IO_PROFILE2) = 0x0042A83D;

	#ifdef CONFIG_CPU_SLEEP_ENABLED
	rlx_irq_register(BSP_ISO_IRQ, CPU_wakeup_handler); // + CPU_SLEEP
	#endif


#if CONFIG_VERSION > IC_VERSION_DP_RevD
#ifdef CONFIG_TCR_ENABLED
    rlx_irq_register(BSP_TCR_IRQ, bsp_tcr_handler);
#endif

    rlx_irq_register(BSP_ISO_IRQ, bsp_iso_handler);
    rlx_irq_register(BSP_ISON_IRQ, bsp_ison_handler);

    if (fwsig->SIG == NORMAL_SIG && fwsig->bspPatch == NEED_PATCH)
        fwsig->bsppatch();
#endif
#endif

    bsp_bits_set(MAC_OOBREG, 1, BIT_DASHEN, 1);


#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
    REG32(MAC_BASE_ADDR + MAC_FVID)   = (dpconf->fwMajorVer << 5) + ((dpconf->fwMinorVer) << 8) + dpconf->fwExtraVer;

#elif CONFIG_VERSION  == IC_VERSION_DP_RevF
    INT32U ver = 0;
    ver = REG32(IOREG_IOBASE+IO_FWVER);
    ver = ver | ((INT8U) dpconf->fwMajorVer);
    ver = ((ver & 0x000000ff) << 5) + (INT8U) dpconf->fwMinorVer;
    ver = ver |	((INT8U) dpconf->fwExtraVer) << 8;
    REG32(IOREG_IOBASE+IO_FWVER) = ver;

#else
    REG8(IOREG_IOBASE+IO_FWVER1)   = (INT8U) dpconf->fwMajorVer ;
    REG8(IOREG_IOBASE+IO_FWVER1)   = (REG8(IOREG_IOBASE+IO_FWVER1) << 5 ) + (INT8U) dpconf->fwMinorVer;
    REG8(IOREG_IOBASE+IO_FWVER2)   = (INT8U) dpconf->fwExtraVer;
#endif

#if defined(CONFIG_SMBUSTRA_DEBUG) || defined(CONFIG_BUILDROM)
    SMBUSLOGQ = OSQCreate(SMBUSLOG, SMBUS_ENTRIES);
#endif

        #if 0
	 rlx_irq_register(BSP_ISON_IRQ, bsp_ison_handler_patch);
        #endif
}
void bsp_setup_R (void)
{
    bsp_timer_init();		    /* Initialize timer */

#ifdef CONFIG_FPGA_TEST
    //since FPGA is slower than ASIC by 4 fold, need to patch
    //useful for loading to IMEM testing
    CPU_CLK_FREQ = CPU_CLK_FREQ >> 2;
    REG32(TIMER_IOBASE+TIMER_LC) = CPU_CLK_FREQ /HZ;      //set counter
#endif


    bsp_buf_alloc();    
	bsp_gmac_init();
	//smbus.c have been remove from ROM
	//event_log_load();

    inituser();
    bsp_bits_set(MAC_OOBREG, 1, BIT_DASHEN, 1);
    rlx_irq_init();             /* Initialize the interrupt controller */
}


#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)
INT8U bsp_get_sstate()
{
    INT32U state, acpi;

#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    //isolate pin 1 equals S0
    if(bsp_bits_get(MAC_MAC_STATUS, BIT_ISOLATE, 1))
        state = S_S0;
    else
        state = S_S5;

    acpi = bsp_bits_get(MAC_ACPI, BIT_ACPI, 8);

    //isolate will reflect the S5 only
    //if console tool or others change the state to S3 or S4
    //the state chould be S3 or S4 instead of S5
    if(state == S_S5 && (acpi == S_S3 || acpi == S_S4))
        state = acpi;
#else

    state = bsp_bits_get(MAC_ACPI, BIT_ACPI, 8);

    if (state == S_BIOS) //bios would set S0 to 0x06, patch it
        state = S_S0;
    else if (state > S_BIOS)
        state = S_UNKNOWN;
#endif


    //fill_rmttbl(state);

    return state;
}

void bsp_set_sstate(INT8U state)
{
    bsp_bits_set(MAC_ACPI, state, BIT_ACPI, 8);

    //fill_rmttbl(state);
}
#endif


//for EP VerA, cpu_sw_reset is the same with the following call
#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)
void cpu_reset()
{
#if 0
#if CONFIG_VERSION > 6
    bsp_gmac_rx_stop();

    //clear the WDT bit to let firmware know that it is from normal cpu reset
    bsp_bits_set(IO_CONFIG1, 0, BIT_OOB_WDT, 1);
#endif
#endif

#if 0
    REG8(0xB9000004) &= ~0x20;
    REG8(0xB9000004) &= ~0x08;
    bsp_wait(1);
    REG8(0xB9000004) |= 0x08;
    //soft reset

#else
    //instead of cpu_reset, perform software restart
    bsp_irq_disable();
    _boot();
#endif
}
#endif

int bsp_IODrvAP_exist()
{
    if (!bsp_bits_get(MAC_IBREG, BIT_APRDY, 1))
        return 0;
    else
        return 1;
}

#if (CONFIG_VERSION  <= IC_VERSION_DP_RevF)
INT8U bsp_get_link_status()
{
#if CONFIG_VERSION  < IC_VERSION_DP_RevF
    return 1;
#else
    return (REG16(IOREG_IOBASE+MAC_IMR) & 0x0400);
#endif

}
#endif

#if (CONFIG_VERSION  >= IC_VERSION_EP_RevA)
INT32U ReadIBReg(INT16U reg)
{
    REG32(MAC_BASE_ADDR + MAC_IB_ACC_SET) = (0x800f0000 | reg);

    while((REG32(MAC_BASE_ADDR + MAC_IB_ACC_SET) & 0x80000000))
        ;

    return REG32(MAC_BASE_ADDR + MAC_IB_ACC_DATA);

}

void WriteIBReg(INT16U reg , INT32U ibData)
{


	REG32(MAC_BASE_ADDR + MAC_IB_ACC_DATA) = ibData;

       REG32(MAC_BASE_ADDR + MAC_IB_ACC_SET) = (0x808f0000 | reg);//bit23 for write

    while((REG32(MAC_BASE_ADDR + MAC_IB_ACC_SET) & 0x80000000))
        ;

    return;
}
#if defined(CONFIG_LANWAKE_NOTIFY_PCIE) || defined(CONFIG_VENDOR_FSC)

#define UMAC_IO_CSIDR 0xe82c
#define UMAC_IO_CSIAR 0xe830
INT32U readCSIChannel(INT8U fun, INT16U reg){

	INT32U regValue = 0x00;
	INT8U times = 0;

	
	//printf("[RKT]  + readCSIChannel");
	//regValue = (0x0000f000 | reg);
	regValue = ((((INT32U)fun<<16) | 0x0000f000) | reg);
	WriteIBReg(UMAC_IO_CSIAR , regValue ) ;
	//printf("[RKT] readCSIChannel : 0x68 = 0x%x\n",regValue);
	while(((ReadIBReg(UMAC_IO_CSIAR) & 0x80000000) == 0x00)&&(times<100)) {
       	times ++;
	}
	//printf("[RKT]  - readCSIChannel 0x68 = 0x%x , timeout=0x%x\n",ReadIBReg(UMAC_IO_CSIAR),times);
    	return ReadIBReg(UMAC_IO_CSIDR);

}

void writeCSIChannel(INT8U fun,INT16U reg , INT32U value){
	
	INT32U regValue = 0x00;
	INT8U times = 0;

	
	//printf("[RKT]  + readCSIChannel");
	WriteIBReg(UMAC_IO_CSIDR , value ) ;	
	//regValue = (0x8000f000 | reg);
	regValue = ((((INT32U)fun<<16) | 0x8000f000) | reg);
	WriteIBReg(UMAC_IO_CSIAR , regValue ) ;
	//printf("[RKT] readCSIChannel : 0x68 = 0x%x\n",regValue);
	while(((ReadIBReg(UMAC_IO_CSIAR) & 0x00000000) )&&(times<100)) {
       	times ++;
	}		
}

#endif


#endif

//tomadd @20140821
#ifdef CONFIG_USB_FLASH
void lan_wake()
{
	        		
//		printf("[RTK]lanwake\n");		
		bsp_bits_set(MAC_LAN_WAKE , 1,BIT_LANWAKE_IB_DIS,1);//set DIS_IB_WAKE to 1
		bsp_bits_set(MAC_LAN_WAKE , 1,BIT_OOB_LANWAKE,1);//set OOB_PINLANWAKE 1
		bsp_bits_set(MAC_LAN_WAKE , 0,BIT_OOB_LANWAKE,1);//set OOB_PINLANWAKE 0
		bsp_bits_set(MAC_LAN_WAKE , 0,BIT_LANWAKE_IB_DIS,1);//set DIS_IB_WAKE to 0
		bsp_bits_set(MAC_OOB_LANWAKE , 1,BIT_OOBLANWAKE,1);//set BIT_OOBLANWAKE to 1

}
#endif

#if defined(CONFIG_ENHANCE_SMB_SAMPLE_RATE)&&defined(CONFIG_ENHANCE_SMB_SAMPLE_RATE)
void smbus_frequency(INT8U freq_div)
{
	
    REG16(SMBUS_BASE_ADDR + SEIO_BusFree) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_HoldTimeRStart_1) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_RS_SetupTime_1) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_StopSetupTime_1) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_DataHoldTime) = (0x03E / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_MasterClkLow) = (0x0768 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_MasterClkHigh) = (0x0768 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_MasterRBControl) = (0x03B4 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_MasterTDControl) = (0x03B4 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_HoldTimeRStart_2) = (0x00FE / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_RS_SetupTime_2) = (0x00FE / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_StopSetupTime_2) = (0x00FE / freq_div);
}
#endif

#if defined(CONFIG_TRNG_ENABLED)
void enableTRNG()
{
	REG32(TRNG_BASE + TRNG_ANALOG) = REG32(TRNG_BASE + TRNG_ANALOG) | TRNG_POW_BIT;
	while((REG32(TRNG_BASE + TRNG_RETURN3) & TRNG_RDY_BIT) == 0)
	{
		 bsp_wait(1);
	}
}
#endif
#if CONFIG_OTG_ENABLED 
void UMAC_RST() //for FPGA release reset pin
{
    volatile INT32U temp = 0;
    volatile INT32U i = 0;
//printk("FPGA_INIT===>\r\n");   
    // set uphy connect, bit23 = 1
    REG32(CPU1_IOBASE+0x00) |= 0x00800000;
    
    // toggle UMAC reset
    REG32(CPU2_IOBASE+0x08) &= (~0x00000004);

    REG32(CPU2_IOBASE+0x08) |= 0x00000004;
    for(i=0;i<1000;i++){

	temp = REG32(CPU1_IOBASE+0x00);
	if((temp&0x00004000) != 0){break;}
    }
}

INT32U OOB_READ_IB_0(INT16U addr)/* addr should be OCP addr */
{
	REG32(OOB_MAC_IOBASE + IO_IB_ACC_SET) = 0x800f0000 | addr;
	while(REG32(OOB_MAC_IOBASE + IO_IB_ACC_SET)&0x80000000);
	return(REG32(OOB_MAC_IOBASE + IO_IB_ACC_DATA));
}

void OOB_WRITE_IB_0(INT16U addr,INT32U data)/* addr should be OCP addr */
{
	REG32(OOB_MAC_IOBASE + IO_IB_ACC_DATA) = data;
	REG32(OOB_MAC_IOBASE + IO_IB_ACC_SET) = 0x808f0000 | addr;
	while(REG32(OOB_MAC_IOBASE + IO_IB_ACC_SET)&0x80000000);
}

void usb_otg_patch()
{   
    #if 1 //these code are for PCIe find card patch
    usb_mode_rd=OOB_READ_IB_0(0xE434);
    usb_mode_wt=0x0BB8004;
    OOB_WRITE_IB_0(0xE434,usb_mode_wt);
    usb_mode_rd=OOB_READ_IB_0(0xE434);
 
    usb_mode_rd=OOB_READ_IB_0(0xD3EC);
    usb_mode_wt=0x1;
    OOB_WRITE_IB_0(0xD3EC,usb_mode_wt);
    usb_mode_rd=OOB_READ_IB_0(0xD3EC);
    #endif
    
    UMAC_RST();
    #if 0
    //
    usb_pcie_rd=REG32(0xbaf00008);
    usb_pcie_wt=0x14;
    REG32(0xbaf00008)= usb_pcie_wt;//cpu_to_le32(usb_pcie_wt);
    usb_pcie_rd=REG32(0xbaf00008);
    #endif
//use ICE, set breakpoint here
// connect PCIe to PC, power-on,
// after BIOS, you can access USB reg	
}

#endif
