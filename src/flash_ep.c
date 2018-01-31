/******************************modify note**********************************/
/*because we don't know whether this chip is support 4bytes mode ,so if be compatial with*/ 
/* 3bytes mode ,check chip's capability is added,and old function 's modify is limited in not*/ 
/* affecting early 3bytes mode.													*/
/**************************************************************************/



#include <rlx_types.h>
#include "bsp.h"
//#include <rlx_cpu_regs.h>
#include "flash.h"
#include <stdlib.h>


#define SECT_4K 	0x01		/* OPCODE_BE_4K works uniformly */
#define SST_WRITE	0x04		/* use SST byte programming */


extern char char_temp[20];
SPI_FLASH_STRUCT flash_struct;




struct spi_device_id {
	INT32U	jedec_id;
	INT32U  sector_size;
	INT16U  n_sectors;
	INT8U   flags;
} ;


/* NOTE: double check command sets and memory organization when you add
 * more flash chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 */
static const struct spi_device_id m25p_ids[] = {
	/* Atmel -- some are (confusingly) marketed as "DataFlash" */
	{ 0x1f6601,  32 * 1024,   4, SECT_4K },  //"at25fs010"
	{ 0x1f6604,  64 * 1024,   8, SECT_4K },	 //"at25fs040"  

	{ 0x1f4401,  64 * 1024,   8, SECT_4K },	 //"at25df041a" 
	{ 0x1f4701,  64 * 1024,  64, SECT_4K }, //"at25df321a",
	{ 0x1f4800,  64 * 1024, 128, SECT_4K },  //"at25df641",  

	{ 0x1f0400,  64 * 1024,  8, SECT_4K },   //"at26f004",   
	{ 0x1f4501,  64 * 1024, 16, SECT_4K },   //"at26df081a", 
	{ 0x1f4601,  64 * 1024, 32, SECT_4K },   //"at26df161a", 
	{ 0x1f4700,  64 * 1024, 64, SECT_4K },   //"at26df321",  

	{ 0x1f2500,  64 * 1024, 16, SECT_4K },	 //"at45db081d", 

	/* EON -- en25xxx */
	{ 0x1c3116,  64 * 1024,  64, SECT_4K },  //"en25f32", 
	{ 0x1c2016,  64 * 1024,  64, 0 },		 //"en25p32", 
	{ 0x1c3016,  64 * 1024,  64, 0 },		 //"en25q32b", 
	{ 0x1c2017,  64 * 1024, 128, 0 },		 //"en25p64", 
	{ 0x1c3017,  64 * 1024, 128, SECT_4K },  //"en25q64", 
	{ 0x1c7019,  64 * 1024, 512, 0 },		 //"en25qh256", 

	/* Everspin */
	{ 32 * 1024, 1, 256, 2 },		   //"mr25h256", 

	/* GigaDevice */
	{ 0xc84016,  64 * 1024,  64, SECT_4K },//"gd25q32", 
	{ 0xc84017,  64 * 1024, 128, SECT_4K }, //"gd25q64",

	/* Intel/Numonyx -- xxxs33b */
	{ 0x898911,  64 * 1024,  32, 0 },	 //"160s33b", 
	{ 0x898912,  64 * 1024,  64, 0 },	 //"320s33b", 
	{ 0x898913, 64 * 1024, 128, 0 },	//"640s33b", 

	/* Macronix */
	{ 0xc22012,  64 * 1024,   4, SECT_4K },//"mx25l2005a",
	{ 0xc22013,  64 * 1024,   8, SECT_4K }, //"mx25l4005a", 
	{ 0xc22014,  64 * 1024,  16, 0 },		//"mx25l8005",
	{ 0xc22015,  64 * 1024,  32, SECT_4K },//"mx25l1606e", 
	{ 0xc22016,  64 * 1024,  64, 0 },	     //"mx25l3205d",  
	{ 0xc22017,  64 * 1024, 128, 0 },		//"mx25l6405d", 
	{ 0xc22018,  64 * 1024, 256, 0 },		//"mx25l12805d", 
	{ 0xc22618,  64 * 1024, 256, 0 },		//"mx25l12855e"
	{ 0xc22019,  64 * 1024, 512, 0 },		//"mx25l25635e", 
	{ 0xc22619,  64 * 1024, 512, 0 },     //"mx25l25655e", 
	{ 0xc2201a,  64 * 1024, 1024, 0 },	//"mx66l51235l",
	{ 0xc22415,  64 * 1024, 32, SECT_4K },// "mx25l1633e", 

	/* Micron */
	{ 0x20ba17,  64 * 1024, 128, 0 },	// "n25q064",
	{ 0x20bb18,  64 * 1024, 256, 0 },  //"n25q128a11",  
	{ 0x20ba18,  64 * 1024, 256, 0 },	//"n25q128a13",  
	{ 0x20ba19,  64 * 1024, 512, SECT_4K },//"n25q256a",

	/* Spansion -- single (large) sector size only, at least
	 * for the chips listed here (without boot sectors).
	 */
	{ 0x010215,   64 * 1024,  64, 0 }, //"s25sl032p", 
	{ 0x010216,   64 * 1024, 128, 0 }, //"s25sl064p",
	{ 0x010219,  256 * 1024, 128, 0 }, //"s25fl256s0",
	{ 0x010219,   64 * 1024, 512, 0 }, //"s25fl256s1",
	{ 0x010220,  256 * 1024, 256, 0 }, //"s25fl512s",  
	{ 0x010221,  256 * 1024, 256, 0 }, //"s70fl01gs",  
	{ 0x012018,  256 * 1024,  64, 0 }, //"s25sl12800",
	{ 0x012018,   64 * 1024, 256, 0 }, //"s25sl12801", 
	{ 0x012018,  256 * 1024,  64, 0 }, //"s25fl129p0", 
	{ 0x012018,   64 * 1024, 256, 0 }, // "s25fl129p1", 
	{ 0x010212,   64 * 1024,   8, 0 }, //"s25sl004a",
	{ 0x010213,   64 * 1024,  16, 0 }, //"s25sl008a", 
	{ 0x010214,   64 * 1024,  32, 0 }, //"s25sl016a", 
	{ 0x010215,   64 * 1024,  64, 0 }, //"s25sl032a",
	{ 0x010216,   64 * 1024, 128, 0 }, //"s25sl064a", 
	{ 0xef4015,   64 * 1024,  32, SECT_4K },//"s25fl016k", 
	{ 0xef4017,   64 * 1024, 128, SECT_4K },//"s25fl064k",

	/* SST -- large erase sizes are "overlays", "sectors" are 4K */
	{ 0xbf258d,  64 * 1024,  8, SECT_4K | SST_WRITE },//"sst25vf040b", 
	{ 0xbf258e,  64 * 1024, 16, SECT_4K | SST_WRITE },//"sst25vf080b",
	{ 0xbf2541,  64 * 1024, 32, SECT_4K | SST_WRITE },//"sst25vf016b",
	{ 0xbf254a,  64 * 1024, 64, SECT_4K | SST_WRITE },//"sst25vf032b", 
	{ 0xbf254b,  64 * 1024, 128, SECT_4K },			  //"sst25vf064c",
	{ 0xbf2501,  64 * 1024,  1, SECT_4K | SST_WRITE },//"sst25wf512",  
	{ 0xbf2502,  64 * 1024,  2, SECT_4K | SST_WRITE },// "sst25wf010",  
	{ 0xbf2503,  64 * 1024,  4, SECT_4K | SST_WRITE },//"sst25wf020",
	{ 0xbf2504,  64 * 1024,  8, SECT_4K | SST_WRITE },// "sst25wf040",  

	/* ST Microelectronics -- newer production may have feature updates */
	{ 0x202010,  32 * 1024,   2, 0 }, //"m25p05", 
	{ 0x202011,  32 * 1024,   4, 0 }, //"m25p10",  
	{ 0x202012,  64 * 1024,   4, 0 }, //"m25p20",
	{ 0x202013,  64 * 1024,   8, 0 }, //"m25p40",
	{ 0x202014,  64 * 1024,  16, 0 }, //"m25p80",  
	{ 0x202015,  64 * 1024,  32, 0 }, //"m25p16",  
	{ 0x202016,  64 * 1024,  64, 0 }, //"m25p32", 
	{ 0x202017,  64 * 1024, 128, 0 }, //"m25p64",  
	{ 0x202018,  256 * 1024, 64, 0 }, //"m25p128", 
	{ 0x20ba16,  64 * 1024,  64, 0 }, // "n25q032",

/*
	{  INFO(0,  32 * 1024,   2, 0) },		//"m25p05-nonjedec",  
	{  INFO(0,  32 * 1024,   4, 0) },		//"m25p10-nonjedec",
	{  INFO(0,  64 * 1024,   4, 0) },		//"m25p20-nonjedec",  
	{  INFO(0,  64 * 1024,   8, 0) },		//"m25p40-nonjedec", 
	{  INFO(0,  64 * 1024,  16, 0) },		//"m25p80-nonjedec", 
	{  INFO(0,  64 * 1024,  32, 0) },		//"m25p16-nonjedec", 
	{  INFO(0,  64 * 1024,  64, 0) },		//"m25p32-nonjedec",
	{  INFO(0,  64 * 1024, 128, 0) },		//"m25p64-nonjedec",  
	{  INFO(0, 256 * 1024,  64, 0) },		//"m25p128-nonjedec", 
*/
	{  0x204011,  64 * 1024,    2, 0 },//"m45pe10", 
	{  0x204014,  64 * 1024,   16, 0 },//"m45pe80",
	{  0x204015,  64 * 1024,   32, 0 },//"m45pe16",

	{  0x208012,  64 * 1024,  4,       0 },//"m25pe20",
	{  0x208014,  64 * 1024, 16,       0 },// "m25pe80",
	{  0x208015,  64 * 1024, 32, SECT_4K },//"m25pe16",

	{  0x207116,  64 * 1024, 64, SECT_4K },//"m25px32",  
	{  0x207316,  64 * 1024, 64, SECT_4K },//"m25px32-s0",
	{  0x206316,  64 * 1024, 64, SECT_4K },//"m25px32-s1", 
	{  0x207117,  64 * 1024, 128, 0 },//"m25px64",   

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
	{ 0xef3011, 64 * 1024,  2,  SECT_4K },//"w25x10", 
	{ 0xef3012, 64 * 1024,  4,  SECT_4K },//"w25x20",
	{ 0xef3013, 64 * 1024,  8,  SECT_4K },//"w25x40", 
	{ 0xef3014, 64 * 1024,  16, SECT_4K },//"w25x80", 
	{ 0xef3015, 64 * 1024,  32, SECT_4K },//"w25x16", 
	{ 0xef3016, 64 * 1024,  64, SECT_4K },//"w25x32", 
	{ 0xef4016, 64 * 1024,  64, SECT_4K },//"w25q32", 
	{ 0xef6016, 64 * 1024,  64, SECT_4K },//"w25q32dw",
	{ 0xef3017, 64 * 1024, 128, SECT_4K },//"w25x64",
	{ 0xef4017, 64 * 1024, 128, SECT_4K },//"w25q64",
	{ 0xef5014, 64 * 1024,  16, SECT_4K },//"w25q80",
	{ 0xef4014, 64 * 1024,  16, SECT_4K },//"w25q80bl", 
	{ 0xef4018, 64 * 1024, 256, SECT_4K },//"w25q128",
	{ 0xef4019, 64 * 1024, 512, SECT_4K },//"w25q256",

	/* Catalyst / On Semiconductor -- non-JEDEC */
	//{ "cat25c11", CAT25_INFO(  16, 8, 16, 1) },
	//{ "cat25c03", CAT25_INFO(  32, 8, 16, 2) },
	//{ "cat25c09", CAT25_INFO( 128, 8, 32, 2) },
	//{ "cat25c17", CAT25_INFO( 256, 8, 32, 2) },
	//{ "cat25128", CAT25_INFO(2048, 8, 64, 2) },
	//{ },
	
};




/*erase chip*/
void Flash_erase_all()//Kaiwen
{
	INT8U flash_busy;
	
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 1 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;//Write Enable
	Check_SPIC_Busy();

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_CHIP_ERA;//Chip erase	
	Check_SPIC_Busy();
	do{
		Flash_RDSR();	
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);
 }


/*write the flash men data , AUTO MODE*/
void Flash_write_one_channel(SPI_FLASH_STRUCT* pspi_struct,INT32U NDF, INT32U Address, INT32U *ReadBuffer)
{
	
	INT8U   flash_busy;
	INT32U   i;

	Set_SPIC_Write_one_channel();
	
	//Setup Single IO Auto Write 
	for(i=0;i<NDF;i=i+4)
	{
		//write 4byte data
		REG32(FLASH_DATA_ADDR+Address+i)=*(ReadBuffer+i/4);
		//REG32(FLASH_DATA_ADDR+0)=0x12345678;
		Check_SPIC_Busy(); 

		//Read Flash Status 
		do{
			Flash_RDSR();		
			//check flash busy?	
			flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
			flash_busy=flash_busy & 0x03;
		}while(flash_busy==0x03);
	}
	
}

/*********************************************************
*description:
	user mode to write data to spi flash 
*parameter:
	NDF:byte length
	Address:SPI address(relately 24/32-bit address),
	DReadBuffer: write buffer's point
*Note:ex,when write flash men address 0x82000000,then ,address is 0x00;
	the max  NDF is 128 bytes !!!
**********************************************************/
void Flash_write_one_channel_User(SPI_FLASH_STRUCT* pspi_struct,INT32U NDF, INT32U Address, INT32U *DReadBuffer)
{
	
	INT32U    DWtmp, i;
	INT8U     flash_busy;
	INT8U    *BReadBuffer;	

	Set_SPIC_Write_one_channel();

	BReadBuffer=DReadBuffer;

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;//Write Enable
	Check_SPIC_Busy();

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC	
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=((pspi_struct->addr_width==4)?0:3);//SPI Address byte in user mode

	if(pspi_struct->addr_width==4){
		REG8(FLASH_BASE_ADDR+FLASH_DR) = FLASH_PP_COM;
		DWtmp = Address >>24 ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address >>16)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address >>8)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
	}else{
		DWtmp = Address >> 16;
		DWtmp = DWtmp + (Address & 0x0000ff00);
		DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
		DWtmp = (DWtmp << 8 )+ FLASH_PP_COM;
		REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address
	}

	for(i=0;i<NDF/4;i++)
	{
		//REG32(FLASH_BASE_ADDR+FLASH_DR)=*(DReadBuffer+i);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4+1);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4+2);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4+3);
	}
	if((NDF%4)==1)
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
	else if((NDF%4)==2)
	{
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-2);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
	}
	else if((NDF%4)==3)
	{
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-3);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-2);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
	}
	else
	{
	}

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=1;//enable SPIC			
	Check_SPIC_Busy();
	
	//Read Flash Status 
	do{
		Flash_RDSR();		
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);

}


//user mode ,4byte address
void Flash_Read_one_channel_User_4Byte(SPI_FLASH_STRUCT* pspi_struct,INT32U NDF, INT32U Address, INT32U *DReadBuffer)
{
	
	INT32U    DWtmp, i;
	INT8U     flash_busy;
	INT8U    *BReadBuffer;
	int 	 retval;
	INT32U   spic_busy;

	BReadBuffer=DReadBuffer;
	
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x300;//read mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Enable spi_slave0
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=NDF;//read NDF  byte back
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256!!!, SO pin at pin 1, spi_sout[1]!!!!!!!!
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x00;//SPI Address byte = 4 in user mode
	
	REG8(FLASH_BASE_ADDR+FLASH_DR) = FLASH_READ_COM;
	DWtmp = Address >>24 ;
	REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
	DWtmp = (Address >>16)&(0x000000ff) ;
	REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
	DWtmp = (Address >>8)&(0x000000ff) ;
	REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
	DWtmp = (Address)&(0x000000ff) ;
	REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC

	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
		spic_busy=spic_busy & 0x00000001;
	}while(spic_busy!=0x00);

	
	for(i=0;i<NDF/4;i++)
	{
		*(DReadBuffer+i)=REG32(FLASH_BASE_ADDR+FLASH_DR);			
	}
	if((NDF%4)==1)
		*(BReadBuffer+NDF-1)=REG8(FLASH_BASE_ADDR+FLASH_DR);
	else if((NDF%4)==2)
	{
		*(BReadBuffer+NDF-2)=REG8(FLASH_BASE_ADDR+FLASH_DR);
		*(BReadBuffer+NDF-1)=REG8(FLASH_BASE_ADDR+FLASH_DR);
	}
	else if((NDF%4)==3)
	{
		*(BReadBuffer+NDF-3)=REG8(FLASH_BASE_ADDR+FLASH_DR);
		*(BReadBuffer+NDF-2)=REG8(FLASH_BASE_ADDR+FLASH_DR);
		*(BReadBuffer+NDF-1)=REG8(FLASH_BASE_ADDR+FLASH_DR);
	}
	else
	{
	}		
	
}


/*********************************************************
*description:
	user mode to write data to spi flash 
*parameter:
	NDF:byte length
	Address:SPI address(relately 24-bit address),
	DReadBuffer: write buffer's point
*Note:ex,when write flash men address 0x82000000,then ,address is 0x00;
	the max  NDF is 128 bytes !!!
**********************************************************/
void Flash_write_four_channel_User(SPI_FLASH_STRUCT* pspi_struct,INT32U NDF, INT32U Address, INT32U *DReadBuffer)
{
	
	INT32U    DWtmp, i;
	INT8U     flash_busy;
	INT8U    *BReadBuffer;
	int 	 retval;
	

	BReadBuffer=DReadBuffer;

	Set_SPIC_Write_four_channel();

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;//Write Enable
	Check_SPIC_Busy();

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC	
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=((pspi_struct->addr_width==4)?0:3);;//SPI Address byte =3 in user mode
	
	if(pspi_struct->addr_width==4){
		REG8(FLASH_BASE_ADDR+FLASH_DR) = FLASH_PP_COM;
		DWtmp = Address >>24 ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address >>16)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address >>8)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
	}else{
		DWtmp = Address >> 16;
		DWtmp = DWtmp + (Address & 0x0000ff00);
		DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
		DWtmp = (DWtmp << 8 )+ FLASH_PP_COM;
		REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address
	}
	
	for(i=0;i<NDF/4;i++)
	{
		REG32(FLASH_BASE_ADDR+FLASH_DR)=*(DReadBuffer+i);			
	}
	if((NDF%4)==1)
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
	else if((NDF%4)==2)
	{
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-2);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
	}
	else if((NDF%4)==3)
	{
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-3);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-2);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
	}
	else
	{
	}
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=1;//enable SPIC			
	Check_SPIC_Busy();
	
	//Read Flash Status 
	do{
		Flash_RDSR();		
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);

	
}




void Flash_write_four_channel(SPI_FLASH_STRUCT* pspi_struct,INT32U NDF, INT32U Address, INT32U *ReadBuffer)
{
	
	INT8U   flash_busy;
	INT32U   i;
	
	Set_SPIC_Write_four_channel();

	//Setup Single IO Auto Write 
	for(i=0;i<NDF;i=i+4)
	{
		//write 4byte data
		REG32(FLASH_DATA_ADDR+Address+i)=*(ReadBuffer+i/4);
		Check_SPIC_Busy();

		//Read Flash Status 
		do{
			Flash_RDSR();		
			//check flash busy?	
			flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
			flash_busy=flash_busy & 0x03;
		}while(flash_busy==0x03);
	}
	
}

void Check_SPIC_Busy()
{
	INT32U   spic_busy;

	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
	}while(spic_busy!=0x06);
}

void Flash_RDSR()
{
	INT8U   spic_busy;

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x300;//EEPROM mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Enable spi_slave0
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x01;//read one byte back
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_RDSR_COM;//RDSR
	
	//check spic busy?
	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
		spic_busy=spic_busy & 0x01;
	}while(spic_busy!=0x00);
}

void Flash_RDCR()
{
	INT8U   spic_busy;

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x300;//EEPROM mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Enable spi_slave0
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x01;//read one byte back
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_RDCR_COM;//RDCR
	
	//check spic busy?
	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
		spic_busy=spic_busy & 0x01;
	}while(spic_busy!=0x00);
}

void Flash_RDEAR()
{
	INT8U   spic_busy;

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x300;//EEPROM mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Enable spi_slave0

	
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x01;//read one byte back
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_RDEAR_COM;//RDCR
	
	//check spic busy?
	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
		spic_busy=spic_busy & 0x01;
	}while(spic_busy!=0x00);
}



INT32U Flash_RDID(SPI_FLASH_STRUCT* pspi_struct)
{
	INT32U   flash_ID,spic_busy;
	INT8U	 i;
	INT32U   size;

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x300;//EEPROM mode
	//REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x4;//BAUD Rate = 1/8 ssclk add 
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Enable spi_slave0
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x03;//read one byte back
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_RDID_COM;//RDID

	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
		spic_busy=spic_busy & 0x00000001;
	}while(spic_busy!=0x00);
	
	//flash_ID =(REG8(FLASH_BASE_ADDR+FLASH_DR)<<16)+(REG8(FLASH_BASE_ADDR+FLASH_DR)<<8)+REG8(FLASH_BASE_ADDR+FLASH_DR);
	pspi_struct->flash_ID = (REG8(FLASH_BASE_ADDR+FLASH_DR)<<16)+(REG8(FLASH_BASE_ADDR+FLASH_DR)<<8)+REG8(FLASH_BASE_ADDR+FLASH_DR);
	flash_ID = pspi_struct->flash_ID;
	
	for(i=0;i<sizeof(m25p_ids)/sizeof(m25p_ids[0]);i++){
		if(flash_ID == m25p_ids[i].jedec_id){
			pspi_struct->sector_size=m25p_ids[i].sector_size;
			pspi_struct->n_sectors=m25p_ids[i].n_sectors;
			pspi_struct->flags=m25p_ids[i].flags;
			
			/* enable 4-byte addressing if the device exceeds 16MiB */
			size = m25p_ids[i].n_sectors * m25p_ids[i].sector_size;
			if (size > 0x1000000) {
				pspi_struct->addr_width = 4;
				Flash_Enter4byte_Addrmode(pspi_struct,1);
			} else{
				pspi_struct->addr_width = 3;
			}
			break;
		}
	}
	//not found IC ,use default value
	if(i==sizeof(m25p_ids)/sizeof(m25p_ids[0])){
		pspi_struct->addr_width = 3;
		pspi_struct->sector_size = 64 * 1024 ; //16MByte flash 's block size 
		pspi_struct->n_sectors = 256 ; 
		pspi_struct->flags=SECT_4K; 
	} 
	
	return flash_ID;
}

/*
INT32U Flash_RDDATA(INT32U NDF, INT32U Address, INT32U *ReadBuffer)
{
	INT32U   spic_busy;

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x300;//EEPROM mode
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x16;//BAUD Rate = 1/44 ssclk add 
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Enable spi_slave0
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=NDF;//read NDF byte back
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_PP_COM;//RDdata

	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
		spic_busy=spic_busy & 0x00000001;
	}while(spic_busy!=0x00);
	
	for (i=0;i<NDF/4;i++)
		*(ReadBuffer+i)=REG32(FLASH_BASE_ADDR+FLASH_DR);
	if((NDF%4)==1)
		((INT8U *)ReadBuffer)+NDF
	else if((NDF%4)==2)
	{
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-2);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
	}
	else if((NDF%4)==3)
	{
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-3);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-2);
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
	}
	else
	{
	}
	return flash_ID;
}
*/


/*not used*/
void Flash_Unprotect(INT32U Address)
{
	INT32U   DWtmp;
	INT8U	 flash_busy;
	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30000;//SPI Address byte = 3, read dummy cycle =0
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 1 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode 
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
	Check_SPIC_Busy();

	DWtmp = Address >> 16;
	DWtmp = DWtmp + (Address & 0x0000ff00);
	DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
	DWtmp = (DWtmp << 8 )+ FLASH_SE_UNPROTECT ;
	REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;
	
	Check_SPIC_Busy();	

	do{
		Flash_RDSR();	
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);
}


/*not used*/
void Flash_Protect(INT32U Address)
{
	INT32U   DWtmp;
	INT8U 	 flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30000;//SPI Address byte = 3, read dummy cycle =0
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 1 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode 
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
	Check_SPIC_Busy();

	DWtmp = Address >> 16;
	DWtmp = DWtmp + (Address & 0x0000ff00);
	DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
	DWtmp = (DWtmp << 8 )+ FLASH_SE_PROTECT ;
	REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;
	
	Check_SPIC_Busy();	

	do{
		Flash_RDSR();	
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);
}
	
//set  read auto mode  one channel , /2 baudrate 
void Set_SPIC_Clock(INT8U baudr)//set BAUD Rate = 1/2 ssclk
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=baudr;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1
	//note !!!
	//when BUS clock=31.25Mhz,baudr=1(1/2),the dummy reg is 2,the value must modify based timing
	switch (baudr){
	case 1:
		REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)&\
											 0xffff0000|0x0002;// read dummy cycle =2
		break;
	default :
		REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)&\
											 0xffff0000|0x0000;// read dummy cycle =0
		break;
	}
		
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
}



//set  read auto mode  two channel , 1/2 baudrate ,
void Set_SPIC_Read_two_channel()//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x1;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1
	//REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30009;//SPI Address byte = 3, read dummy cycle =8
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)&0xffff0000|0x000a;
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x204;//dual address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
}


//not used ,not  test 
/*
void Set_SPIC_Read_two_channel_auto()//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
	//REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x1;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1
	//REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30009;//SPI Address byte = 3, read dummy cycle =8
	//REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30008;
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30010;
	//REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x204;//dual address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x206;//
	REG32(FLASH_BASE_ADDR+FLASH_READ_DUAL_ADDR_DATA)=0x3B;
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
}
*/

//set  read auto mode  quad, /2 baudrate 
void Set_SPIC_Read_four_channel()//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x1;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1
//	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x3000d;//SPI Address byte = 3, read dummy cycle =d, 0x3000d for simulation,0x3000e for fpga
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)&0xffff0000|0x000e;// read dummy cycle =d, 0x000d for simulation,0x000e for fpga
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x210;//quad address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC		
}

//set  write auto mode 
void Set_SPIC_Write_one_channel()//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//TX mode
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read data back
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x03;//SPI Address byte =3 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//one channel address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC		
}

//set  write auto mode  quad 
void Set_SPIC_Write_four_channel()//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//TX mode
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read data back
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x300;//quad address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC		
}


// unprotect the FLASH(defaut is protected) & set quad r/w
//when write /erase falsh ,must call this function firstly 
//NOTE: because different  rand flash has different manner to declare/set quad enable ,
//so just use one

void WRSR_Flash_one_two_channel(void )
{
	INT8U   flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x1;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)&0xffff0000|0x0002;
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 1 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//one channel address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	



	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
	Check_SPIC_Busy();

	//Write Status
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WRSR_COM;
	REG8(FLASH_BASE_ADDR+FLASH_DR)=0x00;//one/two channel Read mode Enable
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
	Check_SPIC_Busy();	
	do{
		Flash_RDSR();	
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);

}

/*

// unprotect the FLASH(defaut is protected) & set quad r/w
void WRSR_Flash_quad_channel()
{
	INT8U   flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x1;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)&0xffff0000|0x0002;// read dummy cycle =2
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 1 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode 
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
	Check_SPIC_Busy();

	//Write Status
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WRSR_COM;
	REG8(FLASH_BASE_ADDR+FLASH_DR)=0x40;//QUAD Read Enable
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
	Check_SPIC_Busy();	
	do{
		Flash_RDSR();	
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);
	
	do{
		Flash_RDSR();
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x40;
	}while(flash_busy!=0x40);//QUAD Read Enable OK?
}
*/


//erase sector 
void Flash_erase_sector(SPI_FLASH_STRUCT* pspi_struct,INT32U Address)
{
	INT32U    DWtmp;
	INT8U     flash_busy;
	
	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=((pspi_struct->addr_width==4)?0:3);//SPI Address byte in user mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode 
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
	Check_SPIC_Busy();

	if(pspi_struct->addr_width==4){
		REG8(FLASH_BASE_ADDR+FLASH_DR) = FLASH_CHIP_SEC;
		DWtmp = Address >>24 ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address >>16)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address >>8)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		
	}else{
		DWtmp = Address >> 16;
		DWtmp = DWtmp + (Address & 0x0000ff00);
		DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
		DWtmp = (DWtmp << 8 )+ FLASH_CHIP_SEC;
		REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address
	}

	
	Check_SPIC_Busy();	

	do{
		Flash_RDSR();
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);


}

//erase block 
void Flash_erase_block(SPI_FLASH_STRUCT* pspi_struct,INT32U Address)
{
	INT32U   DWtmp;
	INT8U 	 flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=((pspi_struct->addr_width==4)?0:3);//SPI Address byte in user mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode 
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
	Check_SPIC_Busy();

	if(pspi_struct->addr_width==4){
		REG8(FLASH_BASE_ADDR+FLASH_DR) = FLASH_BE_COM;
		DWtmp = Address >>24 ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address >>16)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address >>8)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		DWtmp = (Address)&(0x000000ff) ;
		REG8(FLASH_BASE_ADDR+FLASH_DR) = DWtmp;
		
	}else{
		DWtmp = Address >> 16;
		DWtmp = DWtmp + (Address & 0x0000ff00);
		DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
		DWtmp = (DWtmp << 8 )+ FLASH_BE_COM;
		REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address
	}
	
	Check_SPIC_Busy();	

	do{
		Flash_RDSR();	
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);

	
}


//this function is for auto mode ,ex.run code into spi flash 
void Flash_Enter4byte_Addrmode (SPI_FLASH_STRUCT* pspi_struct,INT8U enable)
{
	INT8U flash_busy;
	INT8U mode;

	switch (pspi_struct->flash_ID >> 16) {
	case 0xC2:/*MACRONIX*/
	case 0xEF /* winbond */:
		mode=1;
		break;
	default:
		/* Spansion style */
		mode=0;
		break;
	}

	//setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=(((pspi_struct->addr_width==4)?0:3)<<16)|0x0000ffff;//SPI Address byte 
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte =1
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode
	//REG32(FLASH_BASE_ADDR+FLASH_SIZE_CONTRL)=0x0d;  //change the default flash size !
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
	
	if (mode == 1 ){
		REG8(FLASH_BASE_ADDR+FLASH_DR)=enable ? OPCODE_EN4B : OPCODE_EX4B;//enter 4byte command 	
		Check_SPIC_Busy();
	}else{
		//Write  Bank Address Register
		REG8(FLASH_BASE_ADDR+FLASH_DR)=OPCODE_BRWR;
		Check_SPIC_Busy();

		//bit 7 :EXTADD Extended Address 1: 4-byte (32-bits) addressing required from command 
		REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
		REG8(FLASH_BASE_ADDR+FLASH_DR)=enable << 7;
		REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
		Check_SPIC_Busy();	
	}


}

/*
int Flash_Exit4byte_Addrmode (SPI_FLASH_STRUCT* pspi_struct)
{
	INT8U flash_busy;

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	//REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=(REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)&0x0000ffff)|0x30000;
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 0 ,just one command in user mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC

	REG8(FLASH_BASE_ADDR+FLASH_DR)=OPCODE_EX4B;//enter 4byte command 	
	Check_SPIC_Busy();
	
	Flash_RDCR();		
	flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
	flash_busy=flash_busy & 0x20;
	if(flash_busy!=0x00){
		return -1 ;
	}

	pspi_struct->address_mode_cur = address_mode_3byte;

	return 0;

}
*/

void Flash_Init(void){
	INT32U FLASH_ID;
	
	//Flash_Reset();
	Set_SPIC_Clock(1);//Frequency of spi_sclk = Frequency of bus_clk / (2*baudr)
	FLASH_ID=Flash_RDID(&flash_struct);

#ifdef FLASH_DEBUG
	uart_write_str("flash ID =   ");
	itoa_self(FLASH_ID,char_temp,16);
	uart_write_str(char_temp);
	uart_write_str("\r\n");
	if(FLASH_ID == MX25L25645G){
		uart_write_str("This flash is MX25L25645G\r\n");
	}
#endif

	WRSR_Flash_one_two_channel(); // unprotect all 

	
}

//following function is only for micron spi flash 
/*
void Flash_TopHalf_Mode(SPI_FLASH_STRUCT* pspi_struct)
{
	INT8U   flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	//REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)&0x0000ffff|0x30000;//SPI Address byte = 3, read dummy cycle =0
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 1 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
	Check_SPIC_Busy();

	//Write Status
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREAR_COM;
	REG8(FLASH_BASE_ADDR+FLASH_DR)=0x01;//EAR[0]=1,top half
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
	Check_SPIC_Busy();	
	do{
		Flash_RDEAR();	
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x01;
	}while(flash_busy==0x00);

	pspi_struct->EAR_status = EAR_top_rw;
	
}

void Flash_BottomHalf_Mode (SPI_FLASH_STRUCT* pspi_struct)
{
	INT8U   flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	//REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)&0x0000ffff|0x30000;//SPI Address byte = 3, read dummy cycle =0
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 1 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
	Check_SPIC_Busy();

	//Write Status
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREAR_COM;
	REG8(FLASH_BASE_ADDR+FLASH_DR)=0x00;//EAR[0]=1,bottom half
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
	Check_SPIC_Busy();	
	do{
		Flash_RDEAR();	
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x01;
	}while(flash_busy==0x01);

	pspi_struct->EAR_status = EAR_bottom_rw;
	
}

void Flash_Reset(void){
	INT8U   flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x1;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)&0xffff0000|0x0002;// read dummy cycle =2
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 1 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode 
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_RSTEN_COM;
	Check_SPIC_Busy();

	//Write Status
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_RST_COM;
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
	Check_SPIC_Busy();	

}

*/

int Compare_Flash(INT32U *source, INT32U *destination, int size){
	int i = 0;
	INT32U *addr1;
	INT32U *addr2;
		
	for ( i = 0 ; i < size ; i++ ){
		addr1 = source + i; 
		addr2 = destination + i;
		if ( *addr1 != *addr2 ){
			return -1 ;	
		}	 	
	}

	return 0;
}




