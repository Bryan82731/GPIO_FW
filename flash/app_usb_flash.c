#include "bsp.h"

static INT8U stage = 0;
#define SMBIOS
#define PART1
#define PART2
#define PART3
#define CMP
//#define IMEM
#define ROMDATA
disable_master_engine()
{
    REG32(0xB2000044) = 0x00; //disable gmac
    REG8(0xB2000036) = 0x00;
    REG8(0xB2001000) = 0x00;  //disable smbus
    REG8(0xB2001810)  = 0x00; //disable tcr
    REG32(0xB1000008) = 0x00000000;  //disable timer
}

void do_loop(INT32U count)
{
   INT32U i = 0, j;
   for(i = 0; i < count; i++)
   {
     for(j = 0 ; j < (BSP_TIMER_FREQ/2000) ; j++)
     //increase delay for stability (1000 -> 800)	     
       ;
     //the inner loop is around 10ms 
   }
}
//char flash_done[128];
/*
void do_loop(INT32U count)
{
   INT32U i = 0, j;
   for(i = 0; i < count; i++)
   {
     for(j = 0 ; j < (BSP_TIMER_FREQ/4000) ; j++)
     //increase delay for stability (1000 -> 800)	     
       ;
     //the inner loop is around 10ms 
   }
}
*/
/*
void flash_check_busy()
{
       INT8U sr = 0, flash_done, count, i;
   
       do{
       	  sr = REG8(FLASH_BASE_ADDR+FLASH_SR) & 0x01;
       	}while(sr == 1);
       
       do {
       REG8(FLASH_BASE_ADDR+FLASH_SSIENR) = 0x00;      //disable DW_apb_ssi
       REG16(FLASH_BASE_ADDR+FLASH_CTRLR0) = 0x73c7;
       //config CFS:0111 SRL:0 SLV_OE:0 TMOD:11 SCPOL:1 SCPH:1 FRF:00 DFS:0111
       REG16(FLASH_BASE_ADDR+FLASH_NDF)= 0 ;           //Number of Data Frames
       //REG8(FLASH_BASE_ADDR+FLASH_RXFTLR)= NDF-1;    //Set FIFO Full interrupt
       //REG8(FLASH_BASE_ADDR+FLASH_IMR) = FLASH_INT_RXFIS;
       //Set FIFO Full enable(IMR)
       REG8(FLASH_BASE_ADDR+FLASH_SSIENR) = 0x01;
       //enable DW_apb_ssi
       
       REG8(FLASH_BASE_ADDR+FLASH_DR) = FLASH_RDSR_COM ;  //Read command
       //REG8(FLASH_BASE_ADDR+FLASH_DR) = 0; 
       //REG8(FLASH_BASE_ADDR+FLASH_DR) = 0; 

       do{
       	  sr = REG8(FLASH_BASE_ADDR+FLASH_SR) & 0x01;
       	}while(sr == 1);
       	
       
       do {
       	  flash_done = REG8(FLASH_BASE_ADDR+FLASH_DR) & 0x01;
       		sr = REG8(FLASH_BASE_ADDR+FLASH_SR) & 0x08;
       	}while( sr != 0);
       

       //flash_done = REG8(FLASH_BASE_ADDR+FLASH_DR);//dummy read
       //flash_done = REG8(FLASH_BASE_ADDR+FLASH_DR);// & 0x01;
       #if 0
       count = REG8(FLASH_BASE_ADDR+ 0x24);
       for(i = 0 ; i < count ; i++)
       {
       	flash_done = REG8(FLASH_BASE_ADDR+FLASH_DR) & 0x01;
       }
       
       while(REG8(FLASH_BASE_ADDR+ 0x24))
       {
         flash_done = REG8(FLASH_BASE_ADDR+FLASH_DR) & 0x01;//dummy read
         sr  = REG8(FLASH_BASE_ADDR+FLASH_SR) & 0x09;
         if( sr == 0)
         	break;
       }
       #endif
         
       //do_loop(100);  

   } while(flash_done != 0);
   //check MXIC25L1605D 
}
*/

void flash_check_busy()
{
    int i=0, busy = 0;
    do                      //wait Busy->0
    {
    	 busy = REG8(FLASH_BASE_ADDR + FLASH_SR) & 0x05; 
         if(busy == 4)
             break;

    	 i++;
         do_loop(10);
    }while(busy != 4);    
}

void flash_enable()
{

    REG8(FLASH_BASE_ADDR+FLASH_SSIENR) = 0x00;             //disable DW_apb_ssi
    REG16(FLASH_BASE_ADDR+FLASH_CTRLR0) = 0x71c7; 		  
    //config CFS:0111 SRL:0 SLV_OE:0 TMOD:01 SCPOL:1 SCPH:1 FRF:00 DFS:0111    
    REG8(FLASH_BASE_ADDR+FLASH_SSIENR) = 0x01;             //enable DW_apb_ssi
}

void flash_done() 
{
    do_loop(2);
}
void bsp_flash_disable_write()
{
   flash_enable();
   int i;

   REG8(FLASH_BASE_ADDR+FLASH_DR) = 0x05 ;   //Write enable command
   REG8(FLASH_BASE_ADDR+FLASH_DR) = 0x00 ;   //Write enable command
   i++;
   REG8(FLASH_BASE_ADDR+FLASH_DR) = 0x01 ;   //Write enable command
   REG8(FLASH_BASE_ADDR+FLASH_DR) = 0x00 ;   //Write enable command
   i++;
   REG8(FLASH_BASE_ADDR+FLASH_DR) = 0x05 ;   //Write enable command
   REG8(FLASH_BASE_ADDR+FLASH_DR) = 0x00 ;   //Write enable command

}

void bsp_flash_erase_all()
{

   flash_enable();
   REG8(FLASH_BASE_ADDR+FLASH_DR) = FLASH_WRITEEN_COM ;   //Write enable command

   flash_check_busy();
   REG8(FLASH_BASE_ADDR+FLASH_DR) = FLASH_CHIP_ERA_COM ;  //Erase sector command
   do_loop(6000);
   // 200 arounds 10 s
   //typicall need 14 s

}
//each block is 64k bytes
//each sector is 4K bytes
void bsp_flash_erase(INT32U Address,INT8U TYPE)
{
   //return 0;	
   flash_enable();
   REG8(FLASH_BASE_ADDR+FLASH_DR) = FLASH_WRITEEN_COM ;   //Write enable command

   flash_check_busy();
   flash_erase(Address);

   flash_check_busy();
   //flash_done();
   do_loop(200);
}
void bsp_flash_write(INT8U NDF, INT32U Address, INT8U *ReadBuffer)
{

    flash_enable(); 
    REG8(FLASH_BASE_ADDR+FLASH_DR) = FLASH_WRITEEN_COM ;  //Write enable command
    flash_check_busy();
    flash_write_data(Address, ReadBuffer, NDF);
    flash_check_busy();
    do_loop(10);
    //flash_done();
}

void dummy0() { }
void dummy1() { }
void dummy2() { }
void dummy3() { }
void dummy4() { }
void dummy5() { }
void dummy6() { }
void dummy7() { }
//void dummy8() { }
//void dummy9() { }
//void dummya() { }

int main(void)
{
  INT32U i = 0;
  rlx_icache_invalidate_all();
  rlx_dmem_enable(DMEM_BASE,DMEM_TOP);
  disable_master_engine();

  REG8(0xB9000004) = 0x00; //use full speed for programming
  //REG16(0xB2000048) = 0x8000;
  //REG8(0xB2000036) = 0x00;

#ifdef PART1
  //erase block 0 
  //if(argc == 3)
  //    i++;	  
  //total 128K at once
  do{
#if 0
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  dummy0();

  if(memcmp((void *) 0x80100000, (void *) 0xA1000000, 1024*128) == 0)
      break;

#endif
  bsp_flash_erase(0x100000,BLOCK);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  bsp_flash_erase(0x110000,BLOCK);

  dummy0();
  for(i= 0; i < 1024; i++) 
  bsp_flash_write(128, 0x100000+i*128, 0x80100000+i*128);

  do_loop(100);
#ifdef CMP
  if(memcmp((void *) 0x80100000, (void *) 0xA1100000, 1024*128) != 0)
		  stage++;
  }while(0);
#endif
#endif

#ifdef PART2
  //erase block 1 => total 64K
  bsp_flash_erase(0x120000,BLOCK);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  bsp_flash_erase(0x130000,BLOCK);

  dummy1();
  for(i= 0; i < 1024; i++) 
  bsp_flash_write(128, 0x120000+i*128, 0x80100000+i*128);

  do_loop(100);
#ifdef CMP
  if(memcmp((void *) 0x80100000, (void *) 0xA1120000, 1024*128) != 0)
		  stage++;
#endif

#endif

#ifdef PART3
  //erase block 1 => total 64K
  bsp_flash_erase(0x140000,BLOCK);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  bsp_flash_erase(0x150000,BLOCK);

  dummy2();
  for(i= 0; i < 1024; i++) 
  bsp_flash_write(128, 0x140000+i*128, 0x80100000+i*128);

  do_loop(100);
#ifdef CMP
  if(memcmp((void *) 0x80100000, (void *) 0xA1140000, 1024*128) != 0)
		  stage++;
#endif  

#endif

#ifdef SMBIOS
  bsp_flash_erase(0x160000,BLOCK);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  bsp_flash_erase(0x170000,BLOCK);

  dummy3();
  for(i = 0 ; i < 1024 ; i++)
    bsp_flash_write(128, 0x160000+i*128, 0x80100000+i*128);
  
  do_loop(100);
#ifdef CMP
  if(memcmp((void *) 0x80100000, (void *) 0xA1160000, 1024*128) != 0)
	  stage++;
#endif  
#endif

#ifdef  ROMDATA 
  bsp_flash_erase(0x180000,BLOCK);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  bsp_flash_erase(0x190000,BLOCK);

  dummy4();
  for(i = 0 ; i < 1024; i++)
      bsp_flash_write(128, 0x180000+i*128, 0x80100000+i*128);

  do_loop(100);
#ifdef CMP
  if(memcmp((void *) 0x80100000, (void *) 0xA1180000, 1024*128) != 0)
		  stage++;
#endif  

#endif

#ifdef IMEM  
  //imem refill image 192K, including os, lib, codec
  //data used by rom is in 0x81070000
  //total 192K + 64K maximal
  bsp_flash_erase(0x100000,BLOCK);
  if(stage == 0)
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  bsp_flash_erase(0x110000,BLOCK);
  if(stage == 0)
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);

  dummy5();
  for(i = 0 ; i < 1024; i++)
      bsp_flash_write(128, 0x100000+i*128, 0x80100000+i*128);

  if(memcmp((void *) 0x80100000, (void *) 0xA1100000, 1024*128) != 0)
		  stage++;

  bsp_flash_erase(0x120000,BLOCK);
  if(stage == 0)
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
  bsp_flash_erase(0x130000,BLOCK);
  if(stage == 0)
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);

  dummy6();
  for(i = 0 ; i < 1024; i++)
      bsp_flash_write(128, 0x120000+i*128, 0x80100000+i*128);

  if(memcmp((void *) 0x80100000, (void *) (0xA1120000), 1024*128) != 0)
		  stage++;
#endif


  dummy7();
}
