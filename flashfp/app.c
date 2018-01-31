#include "bsp.h"
#include "flash.h"

INT32U FLASH_ID;

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

void disable_master_engine()
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
    INT32U rdid;
    INT8U flag = 0;    
    //INT8U *load_addr = (INT8U *) DMEM_V_BASE;
	//flash_addr, load_addr and load_size will be set by gdb_flash.scr
	INT32U flash_addr;
    INT8U *load_addr;
	INT32U load_size;	

	int test[100] ={0x3c03a800,0x24630518,0x00001021,0x24040064,
	0xac620000,0x24420001,0x1444fffd,0x24630004,0x27bdffe8,0xafbf0014,0xafb00010
	
		
	};
	
	for(i =0; i < 100; i++)
	{
	//	test[i] = 0x225500+ i;
	}

    
	#if hao
    bsp_gmac_disable();
	#endif
    rlx_icache_invalidate_all();	 
    rlx_dmem_enable(DMEM_BASE,DMEM_TOP);
	//Flash_Reset();
	//Set_SPIC_Read_one_channel_15M();	
	FLASH_ID = Flash_RDID(&flash_struct);
	rdid = (FLASH_ID  & 0x00ff0000) >> 16;
	WRSR_Flash_one_two_channel();	

#if 0
	WRSR_Flash_one_two_channel();
		//do not use 0x8200_0000-0x8203_0000(config) to do test
		flash_addr = 0x820a0000;
		load_addr = test;
        asm volatile("load_flash_addr:");
		//spi_blk_erase(ssi_map, (0 & 0x00FFFFFF));
        spi_blk_erase(ssi_map, (flash_addr & 0x00FFFFFF));
       // memset((void *) load_addr, 0xff, load_size);
		
		asm volatile("load_memory:");
        spi_write(ssi_map, (flash_addr&0x00ffffff) , load_addr, WRSIZE);
		//Flash_write_one_channel(256, 0, load_addr);
		return 0;
	#endif	
	#if 1
    do
    {
        if (rdid == 0x1f)
            spi_se_unprotect(&flash_struct,(flash_addr & 0x00FFFFFF));

        asm volatile("load_flash_addr:");//gdb break point for gdb_flash.scr use
        spi_blk_erase(&flash_struct, (flash_addr & 0x00FFFFFF));
        memset((void *) load_addr, 0xff, load_size);


        asm volatile("load_memory:");
        for(i = 0 ; i < NUMLOOP/2 ; i++)
            spi_write(&flash_struct, (flash_addr & 0x00FFFFFF)+i*WRSIZE, load_addr+i*WRSIZE, WRSIZE);


        if(memcmp((void *) load_addr, (void *) (flash_addr | 0x20000000), load_size) != 0)
        {
            asm volatile("compare_error:");
	    flag = 1;
        }

    } while(flag == 0);
	#endif
    rlx_icache_invalidate_all();
    rlx_dcache_invalidate_all();

}
