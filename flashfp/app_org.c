#include "bsp.h"


void bsp_wait(INT32U usec)
{
    INT32U i, j;
    for (i = 0 ; i < usec ; i++)
        //inner loop is around 1u if runnign at 125 MHz
        for (j = 0 ; j < 31; j++)
            asm("nop");

}

void bsp_gmac_disable()
{
    //Check TPPoll to wait TX Finish
    while(REG8(MAC_BASE_ADDR + MAC_TPPoll) & 0x80)
        ;

    //TX FIFO is not empty
    while(REG16(MAC_BASE_ADDR + MAC_CMD) & 0x0C00)
        ;

    REG8(MAC_BASE_ADDR + MAC_CMD) = 0x00;

    //wait engine goest down
    while(REG16(MAC_BASE_ADDR + MAC_CMD) & 0x300)
        ;

}

disable_master_engine()
{
#ifdef RTL8111EP
    bsp_gmac_disable();
#else
    REG32(0xB2000044) = 0x00; //disable gmac
    REG8(0xB2000036) = 0x00;
#endif
  //  REG8(0xB2001000) = 0x00;  //disable smbus
  //  REG8(0xB2001810)  = 0x00; //disable tcr
  //  REG32(0xB1000008) = 0x00000000;  //disable timer
#define SMBUS_BASE_ADDR      0xBAF30000
#define SEIO_SMEn                0x0000
    REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= 0x08;

#define TCR_BASE_ADDR            0xBAF50000	
#define TCR_CONF0                0x10
    REG8(TCR_BASE_ADDR + TCR_CONF0)  = 0x00; //disable tcr

	#define TIMER_IOBASE         0xBA800000
#define TIMER_CR             0x08
    REG32(TIMER_IOBASE + TIMER_CR) = 0x00000000;  //disable timer	
}

int main(void)
{
    INT32U i = 0;
    struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
    INT32U rdid = (spi_read_id(ssi_map)  & 0x00ff0000) >> 16;
    INT8U flag = 0;
    INT32U flash_addr;
    //INT8U *load_addr = (INT8U *) DMEM_V_BASE;
#ifdef RTL8111EP
    INT8U *load_addr = (INT8U *) 0x800B0000;
#else
    INT8U *load_addr = (INT8U *) 0x80200000;
#endif

    INT32U load_size = 64*1024;	
	#if hao
    bsp_gmac_disable();
	#endif
    rlx_icache_invalidate_all();	 
    rlx_dmem_enable(DMEM_BASE,DMEM_TOP);	 


    do
    {
        if (rdid == 0x1f)
            spi_se_unprotect(ssi_map, (flash_addr & 0x00FFFFFF));

        asm volatile("load_flash_addr:");
        spi_blk_erase(ssi_map, (flash_addr & 0x00FFFFFF));
        memset((void *) load_addr, 0xff, load_size);


        asm volatile("load_memory:");
        for(i = 0 ; i < NUMLOOP/2 ; i++)
            spi_write(ssi_map, (flash_addr & 0x00FFFFFF)+i*WRSIZE, load_addr+i*WRSIZE, WRSIZE);


        if(memcmp((void *) load_addr, (void *) (flash_addr | 0x20000000), load_size) != 0)
        {
            asm volatile("compare_error:");
	    flag = 1;
        }

    } while(flag == 0);

    rlx_icache_invalidate_all();
    rlx_dcache_invalidate_all();

}
