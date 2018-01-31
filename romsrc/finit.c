#include <string.h>
#include "rlx_library.h"
#include "bsp.h"
#include "lib.h"
#include "flash.h"

extern char __romdata_start_lma, __romdata_start_vma, __romdata_size;
extern char __rombss_start, __rombss_end;
extern char __romstk_start, __romstk_end;
extern char __trap_handler_start_lma, __trap_handler_end_lma;
extern volatile INT32U bsp_wait_cnt;
INT8U *heap_start ;
INT32U heap_size ;

void hw_init() ;
void rlx_imem_refill(INT32U base, INT32U top, INT32U mode);
extern int main(void) _ATTRIBUTE_FAR_CALL ;
extern int ROMmain(void) _ATTRIBUTE_FAR_CALL ;
void fc_init();

#define CP0_CCTL_IMEMFILL          0x00000010  /* IMEM fill                   */
#define CP0_CCTL_IMEMOFF           0x00000020  /* IMEM off                    */
void (*nand_boot)();

//Rom code check pattern for DD
#define ROMVER_P1	0x0e
#define ROMVER_P2	0x0e0e

void WDT_Initial(void)
{
	 INT32U *p = WDT_CONFIG;
	 INT32U tmp;
	 
	 if(*p == 0)
		 return;
	 tmp = *p;
	 tmp = tmp & 0x1E;
	 tmp = tmp>>1;
	 
	 REG8(WDT_BASE_ADDR+WDT_TORR)= tmp;
	 tmp = REG8(WDT_BASE_ADDR+WDT_CR);
	 tmp = tmp | 0x1;
	 REG8(WDT_BASE_ADDR+WDT_CR) = tmp; 
	 REG8(WDT_BASE_ADDR+WDT_CRR)= 0x76;//restart WDT	 
}

void init()
{	
    //consistent with memory trunks
    INT8U *tmpbuf = (INT8U *) (0x8005B000);
    INT32U i = 0, j = 0;
    INT32U crc;
    INT32U status = 0x11;
    FWSIG *sig = (FWSIG *) DPCONF_ADDR;
    INT8U *SSI_MAP = (INT8U*) FLASH_BASE_ADDR;
    INT32U *crcptr;
	
	j = ReadIBReg(FPGA_INDIC);
	if(j & 0x10000)
	{
		//FPGA
		bsp_wait_cnt = 32;	
	}
	else
	{		
		//ASIC
		//Clock is 250MHZ
		bsp_wait_cnt = 32;
		ddr_asic_phy_init();		
	}
	
	j = ReadIBReg(PCR);
	//hao
	//j = TESTIO_ACT;
	if(j & TESTIO_ACT)
	{
		rlx_cp3_init();
		rlx_imem_writeable();
		memcpy((void *)IMEM_V_BASE, (void *)& __trap_handler_start_lma, (INT32U)(&__trap_handler_end_lma - &__trap_handler_start_lma));
		rlx_imem_readonly();
		DCO_Initial();		
		//Clock is 400MHZ
		bsp_wait_cnt = 50;
		while(1)
		{
			IB_Access_OOB_Handler();
			bsp_wait(1);
			REG32(MAC_BASE_ADDR + MAC_ROMVER) = ROMVER_P1;
		}	
	}

	//DCO_Initial();	

	//hao
	//j=0x800;
	//j=0;
	if(j & BIT11)
	{
		memset(0x800C0000, 0 ,120*1024);
		i = flash_nand_rom_read();
		sig = (FWSIG *) 0x800C0000;
		if(sig->SIG == NORMAL_SIG)
		{
			nand_boot = 0x800C1000;		
			nand_boot();
		}
		else
		{
			while(1)
			{
				bsp_wait(100000);
			}
		}
	}
	else
	{		
		Flash_Init();
		WDT_Initial();
	}
	
	REG32(MAC_BASE_ADDR + MAC_ROMVER) = ROMVER_P1;
    hw_init();	

    disable_master_engine();

    reset_isr_table();	

    if (sig->SIG == NORMAL_SIG)
    {
		REG32(MAC_BASE_ADDR + MAC_ROMVER) = ROMVER_P2;
        c_init();
    }
    else if (sig->SIG == DASH_DISABLED)
    {
        while (1)
        {
            bsp_wait(100000);
            bsp_wdt_kick();
        }
    }

    //console tool has complete its part, let firmware go 2nd part
    else if (sig->SIG == FWUPDATENEW)
    {
        //if (sig->FWStartAddr != sig->ROMStartAddr)
        {
            crc = crc32(0, (INT8U *) (sig->IMGStartAddr + sig->CRC32Start), sig->IMGSize);
            crcptr = ((INT32U *) sig->IMGStartAddr ) + 4;
            if (crc == *crcptr)
            {
                for (i = 0 ; i < ifloor(sig->IMGSize, 4096); i++)
                {
                    memcpy(tmpbuf, (INT8U *) (sig->IMGStartAddr + i*4096), 4096);
                    spi_se_erase(SSI_MAP, sig->ROMStartAddr + i*4096);
                    for (j = 0; j < 32; j++)
                        spi_write(SSI_MAP, sig->ROMStartAddr + i*4096 + j*128,  tmpbuf+j*128, 128);

                }
                status = 0x00;

            }
	    //should back to normal mode, since these two flash region is not overlapped
        }
        //else
        //    status = 0x00;

        //on-line firmware mode, just rewrite the signature back to normal
        writesig(tmpbuf, NORMAL_SIG, status);
        cpu_reset();

    }
    else if (sig->SIG == FWUPDATEROM)
    {
        if (sig->FWStartAddr != sig->ROMStartAddr)
        {
            crc = crc32(0, (INT8U *) (sig->FWStartAddr + sig->CRC32Start), sig->CRC32Size);
            if (crc == sig->CRC32)
            {
                for (i = 0 ; i < ifloor(sig->FWSize, 4096); i++)
                {
                    memcpy(tmpbuf, (INT8U *) (sig->FWStartAddr + i*4096), 4096);
                    spi_se_erase(SSI_MAP, sig->ROMStartAddr + i*4096);
                    for (j = 0; j < 32; j++)
                        spi_write(SSI_MAP, sig->ROMStartAddr + i*4096 + j*128,  tmpbuf+j*128, 128);

                }
                status = 0x00;

            }
	    //should back to normal mode, since these two flash region is not overlapped
        }
        else
            status = 0x00;

        //on-line firmware mode, just rewrite the signature back to normal
        writesig(tmpbuf, NORMAL_SIG, status);
        cpu_reset();

    }
    else
    {
        //do the rom version for firmware update
        //or do the original firmware update
        ROMmain();
    }
}

void hw_init()
{

#if CONFIG_VERSION <= IC_VERSION_EP_RevA
#ifdef CONFIG_FPGA_REFILL 
    rlx_imem_refill(REFILL_BASE, REFILL_TOP, 1);
    rlx_imem_refill(IMEM_BASE,  IMEM_TOP,  0);
#endif
#endif
	rlx_cp3_init();	
    fc_init();
}

void fc_init()
{	
    FWSIG *sig = (FWSIG *) DPCONF_ADDR;
	
	rlx_imem_writeable();
	memcpy((void *)IMEM_V_BASE, (void *)& __trap_handler_start_lma, (INT32U)(&__trap_handler_end_lma - &__trap_handler_start_lma));
	rlx_imem_readonly();

    memset((void *) DMEM_V_BASE, 0 , (DMEM_TOP - DMEM_BASE + 1));

    memcpy((void *) &__romdata_start_vma, (void *) &__romdata_start_lma, (INT32U) &__romdata_size);
    memset((void *) &__rombss_start, 0, (INT32U) (&__rombss_end - &__rombss_start));

    //stk is the stack used by applications running within ROM, subject to change due to development of applications
    if(sig->SIG != NORMAL_SIG)
        memset((void *) &__romstk_start, 0, (INT32U) (&__romstk_end - &__romstk_start));

    if (sig->sramPatch == NEED_PATCH)
    {
        memcpy((void *) sig->textvma, (void *) sig->textlma, sig->textsize);
        memcpy((void *) sig->datavma, (void *) sig->datalma, sig->datasize);
        memset((void *) sig->bssstart, 0, (sig->bssend - sig->bssstart));
    }

    heap_start = (INT8U *) (DMEM_V_BASE + DESC_SIZE);
    heap_size = 0x3F000;

    //set default function pointers
    initPatchFun_R();
	
    //checking new machanism
    aes_init();

    if (sig->cinitPatch == NEED_PATCH)
        sig->cinitpatch();
}

void DCO_Initial (void )
{	
	INT32U temp,i=0;
	INT32U ref_cnt_temp=100*388/25;
	INT32U dco_accuracy_temp=ref_cnt_temp/100;
	INT16U cnt_cal_r;
	INT8U  exceed_flag=0;
	
	// check whether  using  DCO clocck ,0 ,use ,1 not use 
	if(!(REG32(CPU2_IOBASE + 0x0C)&0x80000000) )
	{
		//before DCO ,use RISC clk
		temp = ReadIBReg(0xE018) ;
		WriteIBReg( 0XE018, temp & (~(1<<10))) ;

		
		
		//enable DCO
		REG32(CPU1_IOBASE + FREQ_CAL_REG0) |= (1<<EN_DCO_500M);

		do{
			//set ref clk parameter,enable DCO Valid
			REG32(CPU1_IOBASE + FREQ_CAL_REG0) = REG32(CPU1_IOBASE + FREQ_CAL_REG0)&0xffff81ff | (i << REF_DCO_500M);
			REG32(CPU1_IOBASE + FREQ_CAL_REG0) |= (1 << REF_DCO_500M_VALID);
			
			//wait for 15uS 
			bsp_wait(15);
			//disable DCO Valid
			REG32(CPU1_IOBASE + FREQ_CAL_REG0) &= ~(1 << REF_DCO_500M_VALID);

			//bsp_wait(1000);
			
			//set reference count and enable calibration
			REG32(CPU1_IOBASE + FREQ_CAL_REG0) |= (100 << FRE_REF_COUNT);
			REG32(CPU1_IOBASE + FREQ_CAL_REG0) &= ~(FREQ_CAL_EN);
			REG32(CPU1_IOBASE + FREQ_CAL_REG0) |= (FREQ_CAL_EN);
			
			//wait for 1uS for wrong samle  DCO Calibration done
			bsp_wait(10);
	
			//current  DCO Calibration done
			while((REG32(CPU1_IOBASE + FREQ_CAL_REG1)&0x3)!=0x3);//caution: here may stuck CPU

			
			cnt_cal_r = REG32(CPU1_IOBASE + FREQ_CAL_REG1) >> 16 ;	
			
			if((cnt_cal_r <= ref_cnt_temp)){
				if (exceed_flag==1){ //have exceed
					break;
				}
			}
			else{
				//because exceed 400M	,back one step
				exceed_flag=1;
				i=i-2;
				continue;
			}
			
//		}while((++i)<=25);
		}while(++i);


		// calibration done, set done bit in CPU2 config
		REG32(CPU2_IOBASE + 0x0C) |= 0x40000000;// set it to 1

		//switch RISC clk to DCO clk, SET ocp reg 0xe018 bit RISC clk from PLL
		temp = ReadIBReg(0xE018) ;
		WriteIBReg( 0XE018, temp|(1<<10)) ;
	}

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

void rlx_imem_refill(INT32U base, INT32U top, INT32U mode)
{
    INT32U cctl_imem;
asm volatile("mfc0   %0, $20"   : "=r"(cctl_imem));
    cctl_imem &=  ~CP0_CCTL_IMEMOFF;
asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem))); 
    cctl_imem |=  CP0_CCTL_IMEMOFF;
asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));

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
    else
    {
asm volatile("mfc0   %0, $20"   : "=r"(cctl_imem));
        cctl_imem &=  ~0x00000040;
asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));
        cctl_imem |=  0x00000040;
asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));
    }


}

void reset_isr_table()
{
    asm volatile("mtc0   $0, $12");
    asm volatile("mtlxc0 $0, $0");
}
