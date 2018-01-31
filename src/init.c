#include <string.h>
#include "rlx_library.h"
#include "bsp.h"
#include <rlx/rlx_cpu_regs.h>
#include "usb.h"
#include "telnet.h"

#include "PCIE_HOST.h"
#include "OOBMAC_COMMON.h"

extern char __bss_start, __bss_end;
extern char __data_start_lma, __data_start_vma, __data_size;
extern char __text_start_lma, __text_start_vma, __text_size;
extern char __sram_start_lma, __sram_start_vma, __sram_size;
extern char __romdata_start_lma, __romdata_start_vma, __romdata_size;
extern char __rombss_start, __rombss_end;

extern Pci_Dev pci_dev;

extern void WIFIDASH_DLY(INT32U i);

#ifdef CONFIG_DDR_TEST_ENABLED
extern void	DDR_Test_Task();
#endif

extern void bsp_timer_init(void);
extern void bsp_oobmac_init(void);
extern void bsp_cmac2_init(void);
extern void bsp_kcsTest_init(void);
extern void bsp_cmac_init_test(void);
extern void VPD_Inital (void);
extern INT32U Inital_Adapter_cfg(void);
extern void VGA_pre_ini(void);
extern void bsp_ehci_init(void);
extern void bsp_tcrfp_init(void);

//extern void bsp_ostimer_handler(void);
extern void bsp_oobmac_handler(void);
extern void bsp_KCS_RKVM_NCSI_shareHandler(void);
extern void bsp_cmac_handler_test(void);
//extern void Function0_Interrupt_shareHandler(void);
extern void bsp_FUN0_EHCI_shareHandler(void);
extern void bsp_PHost_handler(void);
extern void bsp_dma_handler(void);
extern void bsp_tcrfp_handler(void);


#if CONFIG_VERSION <= IC_VERSION_DP_RevF
INT8U *heap_start _ATTRIBUTE_ROM_BSS;
INT32U heap_size _ATTRIBUTE_ROM_BSS;

void hw_init() _ATTRIBUTE_INIT;
void init() _ATTRIBUTE_INIT;
void rlx_imem_refill(INT32U base, INT32U top, INT32U mode);
#else
extern char __heap_start, __heap_size;
#endif

int main(void);


//c_init should be the first entry to load, no need to give the _ATTRIBUTE_INIT
void c_init() ;


#define CP0_CCTL_IMEMFILL          0x00000010  /* IMEM fill                   */
#define CP0_CCTL_IMEMOFF           0x00000020  /* IMEM off                    */
#define CP0_CCTL_DWB               0x00000100  /* The value of DWB            */
#define CP0_CCTL_DINVAL            0x00000001  /* The value of DInval         */

extern  volatile  INT32U  OSTime;

#if CONFIG_VERSION == IC_VERSION_DP_RevA
void init()
{
    hw_init();
    c_init();
}

void hw_init()
{
#ifdef CONFIG_FPGA_TEST_ROM
    rlx_imem_refill(REFILL_BASE, REFILL_TOP, 1);
    rlx_imem_refill(DUMMY_BASE,  DUMMY_TOP, 0);
#else
    rlx_imem_refill(DUMMY_BASE,  DUMMY_TOP, 1);
#endif
    rlx_imem_refill(IMEM_BASE,  IMEM_TOP,  0);

    rlx_icache_invalidate_all();
    rlx_dcache_invalidate_all();
    rlx_cp3_init();
}
#endif

void c_init()
{
    INT8U t;
#if CONFIG_VERSION <= IC_VERSION_EP_RevA
    REG8(SMBUS_BASE_ADDR + SEIO_SMEn) |= 0x08;
#endif

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    asm volatile("mtc0   $0, $12");
    asm volatile("mtlxc0 $0, $0");

#if CONFIG_VERSION < IC_VERSION_DP_RevF
    memset((void *) DMEM_V_BASE, 0 , (DMEM_TOP - DMEM_BASE + 1));
    memcpy((void *) &__romdata_start_vma, (void *) &__romdata_start_lma, (INT32U) &__romdata_size);
    memset((void *) &__rombss_start, 0, (INT32U) (&__rombss_end - &__rombss_start));
    heap_start = (INT8U *) 0x80101000;
    heap_size = 0x1F000;
#endif
#endif

#if 0
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    REG32(0xB2004214) = 0x01;
    REG32(0xB0002014) = 0x01;
    REG32(0xB0001000) = 0x02;
#endif
#endif
#if	CONFIG_DDR_ENABLED
    ddr_asic_phy_init();
    ddr_asic_controllor_init();
#endif
#ifdef CONFIG_DDR_TEST_ENABLED
    DDR_Test_Task();
#endif

    memcpy((void *)&__sram_start_vma, (void *)&__sram_start_lma, (INT32U) &__sram_size);
    memcpy((void *)&__data_start_vma, (void *)&__data_start_lma, (INT32U) &__data_size);
//	memcpy((void *) &__romdata_start_vma, (void *) &__romdata_start_lma, (INT32U) &__romdata_size);
    rlx_icache_invalidate_all();
    rlx_dcache_flush_all();
    memset((void *) &__bss_start, 0, (INT32U) (&__bss_end - &__bss_start));
//    memset((void *) &__rombss_start, 0, (INT32U) (&__rombss_end - &__rombss_start));
#if CONFIG_VERSION <= IC_VERSION_DP_RevD
    rlx_dcache_flush_all();
#endif
    main();
}

/*
 *******************************************************************************
 * rlx_imem_refill()
 *
 * Description: Perform the imem refill
 *
 * Arguments  : base  -- start address of physical memory mapped
 *              top   -- size of memory mapped
                mode  -- 0: only remap, 1: perform refill
 *
 * Returns    : None
 *******************************************************************************
 */
#if  CONFIG_VERSION == IC_VERSION_DP_RevA
void rlx_imem_refill(INT32U base, INT32U top, INT32U mode)
{
    INT32U cctl_imem;
    asm volatile("mtc3   %z0, $0"  : : "Jr" ((INT32U)(base)));
    asm volatile("mtc3   %z0, $1"  : : "Jr" ((INT32U)(top)));

    if (mode == 1)
    {
        asm volatile("mfc0   %0, $20"   : "=r"(cctl_imem));
        cctl_imem &=  ~CP0_CCTL_IMEMFILL;
        asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));

        cctl_imem |=  CP0_CCTL_IMEMFILL;
        asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));
    }
}
#endif

#if CONFIG_VERSION <= IC_VERSION_DP_RevD
void
rlx_dcache_flush_all(void)
{
    INT32U  cctl_val = 0;

    /* Retrive CCTL setting */
    asm volatile("mfc0   %0, $20"   : "=r"(cctl_val));

    /* Clear the bit first */
    cctl_val &= ~(CP0_CCTL_DINVAL| CP0_CCTL_DWB);
    asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));

    /* Write back dcache */
    cctl_val |= CP0_CCTL_DWB;
    asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));

    /* Invalidate dcache */
    cctl_val |= CP0_CCTL_DINVAL;
    asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));

}
#endif

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
void reset_isr_table()
{

    asm volatile("mtc0   $0, $12");
    asm volatile("mtlxc0 $0, $0");
}
#endif


#ifdef CONFIG_CPU_SLEEP_ENABLED
extern USBCB usbcb;
void CPU_Sleep(void) _ATTRIBUTE_SRAM;
extern INT32U last_notice_tick;
extern int sleepControler;
extern volatile  INT32U  OSTime;
void CPU_Sleep(void)
{
    INT32U  sr_val;

    /*IEc off*/

#ifdef CONFIG_DEBUG
    //idleTask_counter++;
#endif

#if 1
    if(last_notice_tick  != 0)
    {
        if(OSTime >= last_notice_tick )
        {
            if((OSTime-last_notice_tick) < 3 * OS_TICKS_PER_SEC)
            {
                return;
            }
            else
            {
                //more than 3 secs
                //just go to sleep
            }
        }
        else
        {
            last_notice_tick = OSTime; //over 0xFFFFFFFF , restart
        }
    }
    else
    {
        //last_idle_tick = 0; restart counter
        last_notice_tick = OSTime;
        return ;
    }
#endif

    if(sleepControler > 0)
    {
        if(OSTime %sleepControler  == 0)
        {
            return;
        }
    }

    /*usb_redirection_enable and disable cpu sleep*/
    if(usbcb.usbstate != DISABLED && usbcb.usbstate != DISCONNECTED)
        return;

    asm volatile("mfc0   %0, $12"   : "=r"(sr_val));// this is correct
    sr_val &= ~0x00000001;
    asm volatile("mtc0   %z0, $12"  : : "Jr" ((INT32U)(sr_val)));// this is correct

    /*set idle status = 0x24*/
    REG32(0xb9000004) |= 0x00240000;

    /*check idle status = 0x24*/
    while( (REG32(0xb9000004)&0x24000000) != 0x24000000) {}

    /*set WAKEUP_INT_EN =1*/
    REG8(MAC_BASE_ADDR+MAC_EXT_INT+2) |= 0x02 ;

    asm volatile("sleep");// this is correct
    asm volatile("nop");//+just for debug

    /*clear WAKEUP_INT_EN*/
    REG8(MAC_BASE_ADDR+MAC_EXT_INT+2) &= ~0x02 ;

    /*set bus halt to 0*/
    /*release bus halt*/
    REG32(0xb9000004) &= ~0x00240000;

    /*IEc on*/
    asm volatile("mfc0   %0, $12"   : "=r"(sr_val));// this is correct
    sr_val |= 0x00000001;
    asm volatile("mtc0   %z0, $12"  : : "Jr" ((INT32U)(sr_val)));// this is correct

}
#endif

#if 0
void DCO_Initial (void )
{
    INT32U temp,i=0;
    INT32U ref_cnt_temp=100*400/25;
    INT32U dco_accuracy_temp=ref_cnt_temp/100;
    INT16U cnt_cal_r;
    INT8U  exceed_flag=0;

    // check whether  using  DCO clocck ,0 ,use ,1 not use
    if(!(REG32(CPU2_IOBASE + 0x10)&0x00000001) )
    {
        //enable DCO
        REG32(CPU1_IOBASE + FREQ_CAL_REG0) |= (1<<EN_DCO_500M);

        do
        {
            //set ref clk parameter,enable DCO Valid
            REG32(CPU1_IOBASE + FREQ_CAL_REG0) = REG32(CPU1_IOBASE + FREQ_CAL_REG0)&0xffff81ff | (i << REF_DCO_500M);
            REG32(CPU1_IOBASE + FREQ_CAL_REG0) |= (1 << REF_DCO_500M_VALID);

            //wait for 1uS
            bsp_wait(1);
            //disable DCO Valid
            REG32(CPU1_IOBASE + FREQ_CAL_REG0) &= ~(1 << REF_DCO_500M_VALID);

            //bsp_wait(1000);

            //set reference count and enable calibration
            REG32(CPU1_IOBASE + FREQ_CAL_REG0) |= (100 << FRE_REF_COUNT);
            REG32(CPU1_IOBASE + FREQ_CAL_REG0) &= ~(FREQ_CAL_EN);

            REG32(CPU1_IOBASE + FREQ_CAL_REG0) |= (FREQ_CAL_EN);


            //current  DCO Calibration done
            while((REG32(CPU1_IOBASE + FREQ_CAL_REG1)&0x3)!=0x3);//caution: here may stuck CPU

            bsp_wait(1000);
            cnt_cal_r = REG32(CPU1_IOBASE + FREQ_CAL_REG1) >> 16 ;

            if((cnt_cal_r <= ref_cnt_temp))
            {
                if (exceed_flag==1)  //have exceed
                {
                    break;
                }
            }
            else
            {
                //because exceed 400M ,back one step
                exceed_flag=1;
                i=i-2;
                continue;
            }

        }
        while((++i)<=25);

        // calibration done, set done bit in CPU2 config
        REG32(CPU2_IOBASE + 0x10) |= 0x02;// set it to 1

        //switch RISC clk to DCO clk, SET ocp reg 0xe018 bit RISC clk from PLL
        OOB_READ_IB(OOBMAC_IOBASE,0xF,0xE018,&temp);
        OOB_WRITE_IB(OOBMAC_IOBASE,0xF,0xE018,temp|(1<<10));

    }

}
#endif

extern INT32U bsp_wait_cnt ;

void init_NIC_SD(void)
{
    INT32U data_temp=0;

/*
    //EP EPHY Parameter
    REG32(0xBAF700A0)=0x800E3713;
    REG32(0xBAF700A4)=0x808FDE20;
    WIFIDASH_DLY(100);
    REG32(0xBAF700A0)=0x80023CEE;
    REG32(0xBAF700A4)=0x808FDE20;
    WIFIDASH_DLY(100);
    REG32(0xBAF700A0)=0x80050004;
    REG32(0xBAF700A4)=0x808FDE20;
    WIFIDASH_DLY(100);
    REG32(0xBAF700A0)=0x800410F0;
    REG32(0xBAF700A4)=0x808FDE20;
    WIFIDASH_DLY(100);
    REG32(0xBAF700A0)=0x804E3713;
    REG32(0xBAF700A4)=0x808FDE20;
    WIFIDASH_DLY(100);
    REG32(0xBAF700A0)=0x80423CEE;
    REG32(0xBAF700A4)=0x808FDE20;
    WIFIDASH_DLY(100);
    REG32(0xBAF700A0)=0x804418F0;
    REG32(0xBAF700A4)=0x808FDE20;
    WIFIDASH_DLY(100);
    REG32(0xBAF700A0)=0x80450004;
    REG32(0xBAF700A4)=0x808FDE20;
    WIFIDASH_DLY(100);
    REG32(0xBAF700A0)=0x8000A800;
    REG32(0xBAF700A4)=0x808FDE20;
    WIFIDASH_DLY(100);
    REG32(0xBAF700A0)=0x8040A800;
    REG32(0xBAF700A4)=0x808FDE20;
    WIFIDASH_DLY(100);

    OOB_WRITE_IB(OOBMAC_IOBASE,0xF,0xE434,0x0bb80004);
    OOB_WRITE_IB(OOBMAC_IOBASE,0xF,0xD3EC,0x1);
    OOB_READ_IB(OOBMAC_IOBASE,0xF,0xDC30,&data_temp);
    OOB_WRITE_IB(OOBMAC_IOBASE,0xF,0xDC30,data_temp|0x10000);
*/
    /*	OOB2IB_W(0xE434,0xF,0x0bb80004);
    	OOB2IB_W(0xD3EC,0xF,0x1);
    	OOB2IB_W(0xDC30,0xF,OOBMACReadIBReg(0xDC30)|0x10000);*/

   	OOB_READ_IB(OOBMAC_IOBASE,0xF,0xEA1C,&data_temp);

	if(!(data_temp & 0x10000))  //if asic,setting risc 400MHz clock
	{
		DCO_Initial();
		bsp_wait_cnt = 43;
		
	}else{
		bsp_wait_cnt = 32;
	}
}
extern DPCONF *dpconf;

void init_testItem_SD(void)
{

    bsp_timer_init();

    //bsp_oobmac_init();
    //rlx_irq_register(BSP_GMAC_IRQ,bsp_oobmac_handler); 		//register OOBMAC interrupt

#ifdef CONFIG_RKVM_TEST_ENABLED
    bsp_cmac2_init();
#endif
#ifdef CONFIG_KCS_TEST_ENABLED
    bsp_kcsTest_init();
#endif
#if defined(CONFIG_RKVM_TEST_ENABLED)||defined(CONFIG_KCS_TEST_ENABLED)||defined(CONFIG_NCSI_TEST_ENABLED)
    rlx_irq_register(BSP_KCS_IRQ,bsp_KCS_RKVM_NCSI_shareHandler); 		//register OOBMAC interrupt
#endif

#ifdef CONFIG_CMAC_TEST_ENABLED
    bsp_cmac_init_test();
    rlx_irq_register(BSP_CMAC_IRQ, bsp_cmac_handler_test);
#endif

#ifdef CONFIG_VPD_TEST_ENABLED
    VPD_Inital();
#endif
#ifdef CONFIG_PCIE_ENABLED
    pci_dev.pci_bus_scan_status=Inital_Adapter_cfg();
    rlx_irq_register(BSP_PHOST_IRQ,bsp_PHost_handler);		//register wifi dash interrupt
#endif
#ifdef CONFIG_VGA_TEST_ENABLED
    VGA_pre_ini();	//set 30ms, clear dummy reg OKbit ,clear diff flag ,DDR address
    //rlx_irq_register(BSP_DMA_IRQ, bsp_dma_handler); 	 //VGA need
#endif
#ifdef CONFIG_EHCI_TEST_ENABLED
    bsp_ehci_init();
#endif
#if defined(CONFIG_VPD_TEST_ENABLED)||defined(CONFIG_PCIE_ENABLED)||defined(CONFIG_VGA_TEST_ENABLED)||defined(CONFIG_EHCI_TEST_ENABLED)||defined(CONFIG_RKVM_TEST_ENABLED)||defined(CONFIG_CMAC_TEST_ENABLED)
    rlx_irq_register(BSP_FUN0_IRQ, bsp_FUN0_EHCI_shareHandler);  //share handler:VPD/PCIEHOST/VGA/EHCI
#endif

#ifdef CONFIG_TCR_TEST_ENABLED
    bsp_tcrfp_init();
    rlx_irq_register(BSP_TCR_IRQ, bsp_tcrfp_handler);
#endif

    //share Interrupt Configuration
#ifndef CONFIG_KCS_TEST_ENABLED    //default pciereset IMR is 1
    REG16(KCS_BASE + KCS_IMR)=0x0000;
    REG16(KCS_BASE + KCS_ISR)=REG16(KCS_BASE + KCS_ISR);
#endif

#ifndef CONFIG_TCR_TEST_ENABLED		//default pciereset IMR is 1
    REG16(TCR_BASE_ADDR + TCR_IMR)=0x0000;
    REG16(TCR_BASE_ADDR + TCR_ISR)=REG16(TCR_BASE_ADDR + TCR_ISR);
#endif

#ifndef CONFIG_NCSI_TEST_ENABLED
    REG16(NCSI_IOBASE + MAC_IMR)=0x0000;
    REG16(NCSI_IOBASE + MAC_ISR)=REG16(NCSI_IOBASE + MAC_ISR);
#endif

#ifdef CONFIG_SMBUS_TEST_ENABLED
    smbus_variable_init(0);
    smbus_engine_init(0);
#endif

#ifdef CONFIG_USB_ENABLED
	dpconf = malloc(sizeof(DPCONF));
	memcpy((void *) dpconf, (void *) DPCONF_ROM_START_ADDR, sizeof(DPCONF));
	dpconf->restart = 0;

#if defined(CONFIG_USB_BULK_TEST) || defined(CONFIG_USB_FLASH)
	usb_config_init();
#endif
#endif

	rlx_irq_init();	 //open interrupt IE

}


