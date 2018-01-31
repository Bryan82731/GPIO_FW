#include "bsp.h"
//#include "ssi.h"

static INT8U stage = 0;
//#define SMBIOS
#define PART1
#define PART2
#define PART3
//#define ROMDATA
//#define SSLCERT
//#define IMEM


disable_master_engine()
{
    REG32(0xB2000044) = 0x00; //disable gmac
    REG8(0xB2000036) = 0x00;
    REG8(0xB2001000) = 0x00;  //disable smbus
    REG8(0xB2001810)  = 0x00; //disable tcr
    REG32(0xB1000008) = 0x00000000;  //disable timer
}

void dummy0() { }
void dummy1() { }
void dummy2() { }
void dummy3() { }
void dummy4() { }

int main(void)
{
  INT32U i = 0;
  struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
  INT32U rdid = (spi_read_id(ssi_map)  & 0x00ff0000) >> 16;
  rlx_icache_invalidate_all();
  rlx_dmem_enable(DMEM_BASE,DMEM_TOP);

  disable_master_engine();
  REG8(0xB9000004) = 0x00; 
  //use normal speed

  //erase block 0 & block 1 
  //total 128K at once
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x080000);
  spi_blk_erase(ssi_map, 0x080000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x090000);
  spi_blk_erase(ssi_map, 0x090000);

  //load image from EPI script
  dummy0();

  for(i= 0; i < NUMLOOP; i++) 
  spi_write(ssi_map, 0x080000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1080000, DMEMSIZE) != 0)
		  stage++;

  //erase block 2 & block 3 => total 128K
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x0A0000);
  spi_blk_erase(ssi_map, 0x0A0000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x0B0000);
  spi_blk_erase(ssi_map, 0x0B0000);

  dummy1();

  for(i= 0; i < NUMLOOP; i++) 
  spi_write(ssi_map, 0x0A0000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA10A0000, DMEMSIZE) != 0)
		  stage++;

  //erase block 4 & block 5 => total 128K
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x0C0000);
  spi_blk_erase(ssi_map, 0x0C0000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x0D0000);
  spi_blk_erase(ssi_map, 0x0D0000);

  dummy2();

  for(i= 0; i < NUMLOOP; i++) 
  spi_write(ssi_map, 0x0C0000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA10C0000, DMEMSIZE) != 0)
		  stage++;

  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x0D0000);
  spi_blk_erase(ssi_map, 0x0D0000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x0E0000);
  spi_blk_erase(ssi_map, 0x0E0000);

  dummy3();

  for(i= 0; i < NUMLOOP; i++) 
  spi_write(ssi_map, 0x0D0000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA10D0000, DMEMSIZE) != 0)
		  stage++;

 dummy4();
}
