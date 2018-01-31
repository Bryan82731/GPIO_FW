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

void dummy() { }
void dummy0() { }
void dummy1() { }
void dummy2() { }
void dummy3() { }
void dummy4() { }
void dummy5() { }
void dummy6() { }
void dummy7() { }
void dummy8() { }

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

  //erase block 0 => total 64k

  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x000000);
  spi_blk_erase(ssi_map, 0x000000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);

  dummy();

  for(i = 0 ; i < NUMLOOP/2 ; i++)
    spi_write(ssi_map, 0x000000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
  
  if(memcmp((void *) 0x80100000, (void *) 0xA1000000, DMEMSIZE/2) != 0)
	  stage++;

#ifdef PART1
  //erase block 0 & block 1 
  //total 128K at once
  do{
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x010000);
  spi_blk_erase(ssi_map, 0x010000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x020000);
  spi_blk_erase(ssi_map, 0x020000);

  //load image from EPI script
  dummy0();

  for(i= 0; i < NUMLOOP; i++) 
  spi_write(ssi_map, 0x010000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1010000, DMEMSIZE) != 0)
		  stage++;
  }while(0);
#endif

#ifdef PART2
  //erase block 2 & block 3 => total 128K
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x030000);
  spi_blk_erase(ssi_map, 0x030000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x040000);
  spi_blk_erase(ssi_map, 0x040000);

  dummy1();

  for(i= 0; i < NUMLOOP; i++) 
  spi_write(ssi_map, 0x030000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1030000, DMEMSIZE) != 0)
		  stage++;

#endif

#ifdef PART3
  //erase block 4 & block 5 => total 128K
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x050000);
  spi_blk_erase(ssi_map, 0x050000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x060000);
  spi_blk_erase(ssi_map, 0x060000);

  dummy2();

  for(i= 0; i < NUMLOOP; i++) 
  spi_write(ssi_map, 0x050000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1050000, DMEMSIZE) != 0)
		  stage++;

#endif

#ifdef SMBIOS
  //erase block 6 => total 64k
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x060000);
  spi_blk_erase(ssi_map, 0x060000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);

  dummy3();

  for(i = 0 ; i < NUMLOOP/2 ; i++)
    spi_write(ssi_map, 0x060000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
  
  if(memcmp((void *) 0x80100000, (void *) 0xA1060000, DMEMSIZE/2) != 0)
	  stage++;
#endif

#ifdef  ROMDATA 
  //erase block 7 => total 64k
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x070000);
  spi_blk_erase(ssi_map, 0x070000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);

  dummy4();

  for(i = 0 ; i < NUMLOOP/2; i++)
      spi_write(ssi_map, 0x070000+i*WRSIZE, 0x80100000+i*WRSIZE,WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1070000, DMEMSIZE/2) != 0)
		  stage++;

#endif

#ifdef  SSLCERT 
  //erase block 8 => total 64k
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x080000);
  spi_blk_erase(ssi_map, 0x080000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);

  dummy5();

  for(i = 0 ; i < NUMLOOP/2; i++)
      spi_write(ssi_map, 0x080000+i*WRSIZE, 0x80100000+i*WRSIZE,WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1080000, DMEMSIZE/2) != 0)
		  stage++;

#endif

#ifdef IMEM 
  //erase block 16 - 19  if using 16 Mbits Flash 
  //for test IMEM Refill
  //total 128K at once
  do{
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x100000);
  spi_blk_erase(ssi_map, 0x100000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x110000);
  spi_blk_erase(ssi_map, 0x110000);

  //load image from EPI script
  dummy6();

  for(i= 0; i < NUMLOOP; i++) 
  spi_write(ssi_map, 0x100000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1100000, DMEMSIZE) != 0)
		  stage++;

  if (rdid == 0x1f)
      spi_se_unprotect(ssi_map, 0x120000);
  spi_blk_erase(ssi_map, 0x120000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);

  //load image from EPI script
  dummy7();

  for(i= 0; i < NUMLOOP/2; i++) 
  spi_write(ssi_map, 0x120000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1120000, DMEMSIZE/2) != 0)
		  stage++;
  }while(0);
#endif


 dummy8();
}
