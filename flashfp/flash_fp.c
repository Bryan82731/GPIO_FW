/******************************modify note**********************************/
/*because we don't know whether this chip is support 4bytes mode ,so if be compatial with*/ 
/* 3bytes mode ,check chip's capability is added,and old function 's modify is limited in not*/ 
/* affecting early 3bytes mode.													*/
/**************************************************************************/
//#include <rlx_types.h>
#include "bsp.h"
//#include <rlx_cpu_regs.h>
#include "flash.h"
#include <stdlib.h>

extern char char_temp[20];
SPI_FLASH_STRUCT flash_struct;

void Check_SPIC_Busy(void)
{
	INT32U   spic_busy;

	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
	}while(spic_busy!=0x06);
}


void Flash_RDSR(void )
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

//erase block 
void Flash_erase_block(INT32U Address)
{
	INT32U   DWtmp;
	INT8U 	 flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=3;//SPI Address byte in user mode
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
	DWtmp = (DWtmp << 8 )+ FLASH_BE_COM;
	REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address

	
	Check_SPIC_Busy();	

	do{
		Flash_RDSR();	
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);

	
}

//erase sector 
void Flash_erase_sector(INT32U Address)
{
	INT32U    DWtmp;
	INT8U     flash_busy;
	
	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=3;//SPI Address byte in user mode
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
	DWtmp = (DWtmp << 8 )+ FLASH_CHIP_SEC;
	REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address
	
	
	Check_SPIC_Busy();	

	do{
		Flash_RDSR();
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);


}

//set  write auto mode 
void Set_SPIC_Write_one_channel(void)//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//TX mode
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read data back
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x03;//SPI Address byte =3 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//one channel address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC		
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
void Flash_write_one_channel_User(INT32U NDF, INT32U Address, INT32U *DReadBuffer)
{	
	INT32U    DWtmp, i;
	INT8U     flash_busy;
	INT8U    *BReadBuffer;
    int b  = (((INT32U)DReadBuffer)%4);	
			
	Set_SPIC_Write_one_channel();
	
	BReadBuffer=DReadBuffer;

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;//Write Enable
	Check_SPIC_Busy();

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC	

	DWtmp = Address >> 16;
	DWtmp = DWtmp + (Address & 0x0000ff00);
	DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
	DWtmp = (DWtmp << 8 )+ FLASH_PP_COM;
	REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address

	
	for(i=0;i<NDF/4;i++)
	{
		if(b)
		{
			REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4);
			REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4+1);
			REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4+2);
			REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4+3);	
		}
		else
		{
			REG32(FLASH_BASE_ADDR+FLASH_DR)=*(DReadBuffer+i);		
		}
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


static void Flash_Exit4byte_Addrmode (void)
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC

	REG8(FLASH_BASE_ADDR+FLASH_DR)=OPCODE_EX4B;//enter 4byte command 	
	Check_SPIC_Busy();

}


INT32U Flash_RDID(void )
{
	INT32U   flash_ID,spic_busy;
	INT8U	 i;
	INT32U   size;
	INT16U   ext_id;

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x300;//EEPROM mode
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Enable spi_slave0
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x03;//read five byte back
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_RDID_COM;//RDID

	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
		spic_busy=spic_busy & 0x00000001;
	}while(spic_busy!=0x00);
	
	flash_ID =(REG8(FLASH_BASE_ADDR+FLASH_DR)<<16)+(REG8(FLASH_BASE_ADDR+FLASH_DR)<<8)+REG8(FLASH_BASE_ADDR+FLASH_DR);

	//winbond 256Mb flash default address mode is 4byte address !!!
	if (((flash_ID>>16) == 0xef)&&((flash_ID&0x0000ff)==0x19)){
		Flash_Exit4byte_Addrmode();
	}
	
	
	return flash_ID;
}


/*Define spi_sclk divider value.The frequency of spi_sclk is derived from:
Frequency of spi_sclk = Frequency of bus_clk / (2*baudr).*/
void Set_SPIC_Clock(INT8U baudr)
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1

#ifdef FPGA
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=baudr;//BAUD Rate = 1/2 ssclk
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
#endif

//if ASIC ,use SPIC default value 

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
}


void Set_SPIC_Read_one_channel_62M()//set BAUD Rate = 1/2 ssclk
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x1;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30000;//SPI Address byte = 3, read dummy cycle =0
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
}

void Set_SPIC_Read_one_channel_15M()////set BAUD Rate = 1/8 ssclk
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x4;//BAUD Rate = 1/8 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30000;//SPI Address byte = 3, read dummy cycle =0
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
}

void Set_SPIC_Read_two_channel()//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x1;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1
	//REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30009;//SPI Address byte = 3, read dummy cycle =8
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30008;
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x204;//dual address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
}

void Set_SPIC_Read_two_channel_auto()//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x1;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1
	//REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30009;//SPI Address byte = 3, read dummy cycle =8
	//REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30008;
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30010;
	//REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x204;//dual address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x206;//
	REG32(FLASH_BASE_ADDR+FLASH_READ_DUAL_ADDR_DATA)=0x3B;
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
}
void Set_SPIC_Read_four_channel()//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
	REG32(FLASH_BASE_ADDR+FLASH_BAUDR)=0x1;//BAUD Rate = 1/2 ssclk
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x3000d;//SPI Address byte = 3, read dummy cycle =d, 0x3000d for simulation,0x3000e for fpga
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x210;//quad address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC		
}

void Set_SPIC_Write_one_channel()//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//TX mode
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read data back
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30000;//SPI Address byte = 3, read dummy cycle =0
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x03;//SPI Address byte =3 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//one channel address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC		
}

void Set_SPIC_Write_four_channel()//Kaiwen
{
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//TX mode
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read data back
	REG32(FLASH_BASE_ADDR+FLASH_AUTO_LENGTH)=0x30000;//SPI Address byte = 3, read dummy cycle =0
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x03;//SPI Address byte =3 in user mode
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x300;//quad address/data, blocking write
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC		
}

// unprotect the FLASH
//when write /erase falsh ,must call this function firstly 
//NOTE: because different  rand flash has different manner to declare/set quad enable ,
//so just use one

void WRSR_Flash_one_two_channel(void )
{
	INT8U   flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel, blocking write
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





void Flash_Init(void){
	INT32U FLASH_ID;
	
	Set_SPIC_Clock(1);//Frequency of spi_sclk = Frequency of bus_clk / (2*baudr)
	FLASH_ID=Flash_RDID();

	WRSR_Flash_one_two_channel(); // unprotect all 

	
}

//WRSR_Flash_one_two_channel will unprotect all
void Flash_Unprotect(INT32U Address)
{
	#if 0
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
	#endif
}

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




