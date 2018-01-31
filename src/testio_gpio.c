
#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <bsp_cfg.h>

#include "OOBMAC_COMMON.h"
#include "OOBMAC.h"
#include "mdiobitbang.h"
#include "usb.h"
#include "parammd.h"


enum _RW_CMD{
	CMD_WRITE,
	CMD_READ 
} ;

enum _SUBTYPE{
	SUBTYPE_OCP,
	SUBTYPE_GPHY,
	SUBTYPE_EFUSE,
	SUBTYPE_EPHY
} ;


enum _mode{
	MODE_ALIVE,
	MODE_STALL

};

enum _dev{
	DEV_PCI,
	DEV_USBPLA,
	DEV_USBDMA

}; 

enum _wordnum {
 NUM_WORD,
 NUM_DWORD
};

	
typedef union _info {
	INT32U info_raw ;
	
	struct _INFO_TESTIO_Normal {
		INT32U mode :	1;
		INT32U opera_rw :	1;
		INT32U word_num :	1;
		INT32U lbits : 5;
		INT32U hbits : 5;
		INT32U channel : 3 ;
		INT32U dev	:	2;
	}INFO_TESTIO_Normal;
	

	
}__attribute__ ((__packed__)) info_union ;




typedef struct _TYPE_TESTIO {	
	INT8U setnum ;
	INT16U	clk	;

}__attribute__ ((__packed__)) TYPE_TESTIO,*pTYPE_TESTIO;

typedef struct __TESTIO_CMD {
	info_union info ; 
	INT16U addr;
	INT32U data;
	INT32U *data_addr;
}__attribute__ ((__packed__))TESTIO_CMD,*pTESTIO_CMD;



typedef struct _CONFIG_SCRIPT
{
	INT32U		Hbit;
	INT32U		Lbit;

	//Named for MAC
	INT32U		Offset;
	INT32U		Size;
	INT32U		Value;

	//BONNIE
	//Named for ByteEn , only for USBNIC
	INT8U		ByteEn;

	INT8U Dev ;

}	CFG_SCRIPT, *PCFG_SCRIPT;





INT32U Freq_TESTIO ;



void Testio_Gpio_Init (void ){
		//GPIO init 	
	OOBMAC_gpio_init(MDC_PIN,OUTPUT,1) ;
	OOBMAC_gpio_init(MDIO_PIN,INPUT,0) ;
	OOBMAC_gpio_init(MCS_PIN,OUTPUT,1) ;
	

}







	

	
INT8U  TESTIO_Direct_Write( INT16U adr , INT16U val){
	
	//mcs keep hign
	paraport_setmcs_HIGH () ;
	mdiobb_write( 0x02, adr, val ); 
	paraport_setmcs_LOW () ;
	return 0 ; 
	

}


INT8U TESTIO_Direct_Read(INT16U adr , INT16U * data ){
	
	//phy adr must  be 5'b00010 for TestIO,
	paraport_setmcs_HIGH () ;
	*data = mdiobb_read(0x02 , adr);
	paraport_setmcs_LOW () ;
	return 0;
	
}




//12/21 Read value from OCP Address
INT16U RdOCPAddr(INT8U dev , INT32U address)
{
	INT16U writeword,ReadData,writeaddr;
	INT32U tmpcouter=0;
	int i,j;
	//select dev 
	if (dev == DEV_USBDMA) {  // usb dma mcu 
		TESTIO_Direct_Write(0x00,0x0080);  
	}else {
		// usb pla mcu  or pci
		TESTIO_Direct_Write(0x00,0x0000);  	
	}
	
	
	//Step 2. OP = 0x01, PHYAD = 0x02, REGAD = 0x02, DATA = 0x0123
	writeaddr = (INT16U)address;
	TESTIO_Direct_Write(0x04,writeaddr); // OCP address register,offset 0x04
   
   //--[2]Into Debug Mode	 
	//printf("2. Debug Mode End \n"); 
	//Step 3. OP = 0x01, PHYAD = 0x02, REGAD = 0x00,
	//DATA = 0x4001(Bit14-13 = 2'b01 Memory Read,Bit0 = 0[normal mode] 1[Debug mode])
	//mcu_stall_flag=1;
	if (dev == DEV_USBDMA) {  // usb dma mcu 
		TESTIO_Direct_Write(0x00,0x4081); 
	}else {
		TESTIO_Direct_Write(0x00,0x4001); // OCP control register
	}
	
   //--[3]Check Offset 0x00 Data Bit14 == 0
	//Step 4. OP = 0x10, PHYAD = 0x02, REGAD = 0x00, Check Data Bit14 = 0
	do
	{
	  tmpcouter++;
	  TESTIO_Direct_Read(0x00,&ReadData);
	  i = (ReadData & 0x4000) >> 14;
	  
	}while( (i!=0) && (tmpcouter<10000) );

	if(tmpcouter==10000)
	{
		//cprintf("\n\n\t OCPADDR Flag bit not changed....\n");
		//GetKeyboardHit();
		return 0xFFFF;
	}
	//Step 5. OP = 0x10, PHYAD = 0x02, REGAD = 0x02, Check Data = 0x5A5A
	TESTIO_Direct_Read(0x02,&ReadData);
	//printf("3. Read data from OCP address offset 0x0123 = %4X\n",ReadData);	  
	
	return ReadData;		
}//end of access ocp address function code




INT16U ParaOCPMCUSTALLRead(INT8U dev ,INT32U RegNum)
{
	INT16U ValRead;

	ValRead = RdOCPAddr (dev , RegNum ) ;

	return ValRead;
}


INT16U ReadOCPMCUSTALLRegTestIO(PCFG_SCRIPT Script)
{
	INT16U 		RetVal;	
	
	RetVal = ParaOCPMCUSTALLRead(Script->Dev ,Script->Offset);
	
	return RetVal; 

}


//-------------------------------------------------------------------------------


INT16U WtOCPAddr(INT8U dev ,INT32U address, INT32U  data)
{
    INT16U writeword,ReadData,writeaddr;
    INT32U  tmpcouter=0;
    int i;

	if (dev == DEV_USBDMA) {  // usb dma mcu 
		 TESTIO_Direct_Write(0x00,0x0080);  
	}else {
			// usb pla mcu  or pci
		 TESTIO_Direct_Write(0x00,0x0000);  	
	}
	
   //--[1]From Normal Mode
    //printf("1. From S0 normal mode to S1 debug mode :\n");    
  
    //Step 1. OP = 0x01, PHYAD = 0x02, REGAD = 0x01, DATA = 0x5A5A
    writeword = (INT16U)data;
    TESTIO_Direct_Write(0x02,writeword); // OCP data register,offset 0x02
    
    //Step 2. OP = 0x01, PHYAD = 0x02, REGAD = 0x02, DATA = 0x0123
    writeaddr = (INT16U)address;
    TESTIO_Direct_Write(0x04,writeaddr); // OCP address register,offset 0x04
   
   //--[2]Into Debug Mode    
    //printf("2. Debug Mode End \n"); 
    //Step 3. OP = 0x01, PHYAD = 0x02, REGAD = 0x00, 
    //DATA = 0x6001(Bit14-13 = 2'b11 Memory Write,Bit0 = 0[normal mode] 1[Debug mode])
    //mcu_stall_flag=1;
    TESTIO_Direct_Write(0x00,0x6001); // OCP control register
   
   //--[3]Check Offset 0x00 Data Bit14 == 0
    //Step 4. OP = 0x10, PHYAD = 0x02, REGAD = 0x00, Check Data Bit14 = 0
    do
    {
      tmpcouter++;
      TESTIO_Direct_Read(0x00,&ReadData);
      i = (ReadData & 0x4000) >> 14;
      
    }while( (i!=0) && (tmpcouter<10000) );

    if(tmpcouter==10000)
    {
		//cprintf("\n\n\t OCPADDR Flag bit not changed....\n");
		//GetKeyboardHit();
		return 0xFFFF;
    }
    return 0;
     	
}//end of access ocp address function code




void ParaOCPMCUSTALLWrite(INT8U dev, INT32U RegNum, INT32U RegVal)
{

	WtOCPAddr( dev , RegNum, RegVal);
}






void WriteOCPMCUSTALLRegTestIO(PCFG_SCRIPT Script)
{
	INT32U		Mask;
	INT32U 		RetVal,SetVal;

	if (Script->Size == NUM_DWORD){
		Mask = (0xFFFFFFFF << Script->Lbit) & (0xFFFFFFFF >> (31 - Script->Hbit));
		
		if(Mask != 0xFFFFFFFF)
		{
			RetVal = ParaOCPMCUSTALLRead(Script->Dev ,Script->Offset); 
			RetVal = (ParaOCPMCUSTALLRead(Script->Dev ,Script->Offset+2)<<16)+ RetVal; 
			SetVal = (RetVal & (0xFFFFFFFF ^ Mask)) | (Script->Value & Mask);
		}
		else
		{
			SetVal = Script->Value;
		}
		
		ParaOCPMCUSTALLWrite(Script->Dev , Script->Offset, SetVal&0x0000ffff);
		ParaOCPMCUSTALLWrite(Script->Dev , Script->Offset+2, (SetVal>>16));
		
	}else{
		Mask = (0xFFFF << Script->Lbit) & (0xFFFF >> (15 - Script->Hbit));

		if(Mask != 0xFFFF)
		{
			RetVal = ParaOCPMCUSTALLRead(Script->Dev ,Script->Offset);
			SetVal = (RetVal & (0xFFFF ^ Mask)) | (Script->Value & Mask);
		}
		else
		{
			SetVal = Script->Value;
		}

	 	ParaOCPMCUSTALLWrite(Script->Dev , Script->Offset, SetVal);
	}

	


	
}



//------------------------------------------------------------------------

INT32U  RdOCPAddr_Alive(INT8U dev , INT32U address , INT32U size )
{
    INT16U   ReadData_temp, ReadData_temp2;
    INT16U  writeaddr;
	INT32U 	ReadData;
    INT32U tmpcouter=0;
    INT16U i;//,j;

    
    //Switch MCU to normal mode
    ////OCD Control Register Bit0 = 1 debug mode, Bit0 = 0 normal mode
    /*USB dev need to set a other reg than pcie dev*/
	
 	if (dev == DEV_USBDMA) {  // usb dma mcu 
		 TESTIO_Direct_Write(0x00,0x0080);  
	}else {
			// usb pla mcu  or pci
		 TESTIO_Direct_Write(0x00,0x0000);  	
	}
    //Step 1. OP = 0x01, PHYAD = 0x02, REGAD = 0x08, DATA = 0x0
    //TESTIO_Direct_Write(0x08,0); 
    //Step 2. OP = 0x01, PHYAD = 0x02, REGAD = 0x0A, DATA = 0x0
    //TESTIO_Direct_Write(0x0A,0);    
    //Step 3. OP = 0x01, PHYAD = 0x02, REGAD = 0x0C, DATA = 0xc840
    TESTIO_Direct_Write(0x0C,(INT16U)address); 
    //Step 4. OP = 0x01, PHYAD = 0x02, REGAD = 0x0E, DATA = 0x8000
    TESTIO_Direct_Write(0x0E,0x8000); 
   
   //--[2]Check Offset 0x0E Data Bit15 == 0
    //Step 5. OP = 0x10, PHYAD = 0x02, REGAD = 0x00, Check Data Bit14 = 0       
    do
    {
      tmpcouter++;
	  TESTIO_Direct_Read (0x0E , &ReadData) ;
      i = (ReadData & 0x8000) >> 15;
      
    }while( (i!=0) && (tmpcouter<50000) );
  
    if(tmpcouter==50000)
    {
	//cprintf("\n\n\t OCPADDR Flag bit not changed[MCU Alive]....\n");
	//cprintf("Clear Current Access\n");
	TESTIO_Direct_Write(0x0E,0x0000); 
	//GetKeyboardHit();
	return 0;
    }

	//Step 6. OP = 0x10, PHYAD = 0x02, REGAD = 0x08, Check Data[15:0]
   	TESTIO_Direct_Read(0x08,&ReadData);
    //delay(1);
    //Step 7. OP = 0x10, PHYAD = 0x02, REGAD = 0x0A, Check Data[31:16]
	if (size == NUM_DWORD){
		TESTIO_Direct_Read(0x0A,&ReadData_temp2);
    	//delay(1);
    	ReadData=(((INT32U) (ReadData_temp2&0x0000FFFF))<<16) | (ReadData&0x0000FFFF);
    	//printf("Read from OCP address offset 0x%4X = %8.8lx\n",address,ReadData);     
	}
	
    return ReadData;    	
}//end of access ocp address function code



//--------------------------------------------------------------------------------




INT32U WtOCPAddr_Alive(INT8U dev, INT32U address, INT32U data, INT8U size , INT8U byte_enable)
{
    INT16U writeword,ReadData,writeaddr;
    INT32U tmpcouter=0;
    INT32U i;//,j;


    //Switch MCU to normal mode
    ////OCD Control Register Bit0 = 1 debug mode, Bit0 = 0 normal mode
    /*USB dev need to set a other reg than pcie dev*/
	
 	if (dev == DEV_USBDMA) {  // usb dma mcu 
		 TESTIO_Direct_Write(0x00,0x0080);  
	}else {
			// usb pla mcu  or pci
		 TESTIO_Direct_Write(0x00,0x0000);  	
	}
	
      
    //Step 1. OP = 0x01, PHYAD = 0x02, REGAD = 0x08, DATA = 0x5A5A
    writeword = (INT16U)(data&0x0000FFFF);
    TESTIO_Direct_Write(0x08,writeword); // OCP data register,offset 0x08
    
	if (size == NUM_DWORD){
		//Step 2. OP = 0x01, PHYAD = 0x02, REGAD = 0x0A, DATA = 0x6B6B
		writeword = (INT16U)(data>>16)&0x0000FFFF;
    	TESTIO_Direct_Write(0x0A,writeword); // OCP address register,offset 0x0A
	}
	
    //Step 3. OP = 0x01, PHYAD = 0x02, REGAD = 0x0C, DATA = 0xc840
    writeaddr = (INT16U)address;
    TESTIO_Direct_Write(0x0C,writeaddr); // OCP address register,offset 0x0C

	//Step 4. OP = 0x01, PHYAD = 0x02, REGAD = 0x0E, DATA = 0x800F (BYTE Enable = 4'b1111)
    TESTIO_Direct_Write(0x0E,0x8000 | byte_enable ); // OCP address register,offset 0x0E
   
   //--[2]Check Offset 0x0E Data Bit15 == 0
    //Step 5. OP = 0x10, PHYAD = 0x02, REGAD = 0x00, Check Data Bit14 = 0
    do
    {
      tmpcouter++;
   	  TESTIO_Direct_Read(0x0E,&ReadData);
      i = (ReadData & 0x8000) >> 15;
      
    }while( (i!=0) && (tmpcouter<50000) );
   
    //sleep(1);
    if(tmpcouter==50000)
    {
	//cprintf("\n\n\t OCPADDR Flag bit not changed[MCU Alive]....\n");
	//cprintf("Clear Current Access\n");
	TESTIO_Direct_Write(0x0E,0x0000); 
	//GetKeyboardHit();
	return 0;
    }
    return 0;
     	
}//end of access ocp address function code


// pla, dev=1 ; usb ,dev =2
INT32U ParaOCPRead(INT8U dev , INT32U  RegNum , INT32U size )
{
	INT32U ValRead;

	ValRead = RdOCPAddr_Alive(dev , RegNum ,size );

	return ValRead;
}


void ParaOCPWrite(INT8U dev , INT32U RegNum, INT32U RegVal , INT8U size , INT8U byte_enable)
{

	WtOCPAddr_Alive(dev , RegNum, RegVal,size,byte_enable);
}



VOID ParaGPHYWrite(INT8U dev ,INT32U RegNum, INT32U RegVal)
{
	INT32U ValSet;
	INT32U ValRead;

	//Write
	ValSet=(1<<31)|(RegNum<<16)|(RegVal);
	ParaOCPWrite(dev,0xDE00, ValSet,NUM_DWORD,0x0f);

	do
	{
		ValRead = ParaOCPRead(dev ,0xDE00,NUM_DWORD);
		if(!(ValRead & 0x80000000)) break;
	}
	while(1);

}

INT32U ParaGPHYRead(INT8U dev ,INT32U RegNum)
{
	INT32U ValRead;
	INT32U ValSet;

	ValSet=RegNum<<16;
	ParaOCPWrite(dev ,0xDE00, ValSet,NUM_DWORD ,0xf);

	do
	{
		ValRead = ParaOCPRead(dev ,0xDE00,NUM_DWORD);
		if(ValRead & 0x80000000) break;
	}
	while(1);

	

	return ValRead&0x0000ffff;
}

INT32U ReadPhyRegNoPageTestIO(PCFG_SCRIPT Script)
{
	INT32U			RegVal;


	RegVal = 0;
	RegVal = ParaGPHYRead(Script->Dev, Script->Offset);

}

void WritePhyRegNoPageTestIO(PCFG_SCRIPT Script)
{
	INT16U		Mask;
	INT32U		RetVal,SetVal;

	Mask = (0xFFFF << Script->Lbit) & (0xFFFF >> (15 - Script->Hbit));

	if(Mask != 0xFFFF)
	{
		RetVal = ParaGPHYRead(Script->Dev, Script->Offset);
		SetVal = ((INT16U)RetVal & (0xFFFF ^ Mask)) | ((INT16U)Script->Value & Mask);
	}
	else
	{
		SetVal = (INT16U)Script->Value;
	}
	
	ParaGPHYWrite(Script->Dev, Script->Offset,SetVal);

}



INT32U ParaEFUSERead(INT8U dev ,INT32U RegNum)
{
	INT32U ValRead;
	INT32U ValSet;

	ValSet=(1<<31)|RegNum<<16;
	ParaOCPWrite(dev, 0xDD00, ValSet,NUM_DWORD,0x0f);

	do
	{
		ValRead = ParaOCPRead(dev ,0xDD00, NUM_DWORD);
		if(!(ValRead & 0x80000000)) break;
	}
	while(1);

	return ValRead;
}


//include dummy bit , will be called by ReadEFUSETestIO16
INT32U ReadEFUSETestIO(PCFG_SCRIPT Script)
{
 
	INT32U		RegVal,RegVal17;

 

	RegVal = ParaEFUSERead(Script->Dev,Script->Offset);

	RegVal17=((RegVal>>7) & 0x00010000) |(RegVal & 0x0000FFFF);

	//fprintf(g_OutputHandle,"%05x\n", (RegVal17 & Mask));
	

}

// not include dummy bit , use it in normal   
INT16U ReadEFUSETestIO16(PCFG_SCRIPT Script)
{
	
	INT32U		RegVal,RegVal17;
	INT16U		pos;
	INT32U		tmp;
	INT16U		s,a;
	INT16U		dummypos;

	pos=Script->Offset;

	RegVal = ParaEFUSERead(Script->Dev,Script->Offset);

	RegVal17=((RegVal>>7) & 0x00010000) |(RegVal & 0x0000FFFF); // fix me 

	tmp=RegVal17;
	s=pos%32;
	a=s%16;
	if (s/16) 
		dummypos=16-a;
	else 
		dummypos=a;
	tmp=((tmp >> (dummypos+1))<< dummypos) | (tmp & (~(0xffffffff << dummypos)));
	
	RegVal17=(INT16U)(tmp);
	
	pos++;
	
	//fprintf(g_OutputHandle,"%05x\n", (RegVal17 & Mask));

}



VOID ParaEFUSEWrite(INT8U dev,INT32U RegNum, INT32U RegVal)
{
	INT32U ValRead;
	INT32U ValSet;

	ValSet=(1<<30)|(RegNum<<16)|(RegVal);
	ParaOCPWrite(dev, 0xDD00, ValSet,NUM_DWORD,0x0f);

	do
	{
		ValRead = ParaOCPRead(dev,0xDD00,NUM_DWORD );
		if(!(ValRead & 0x40000000)) break;
	}
	while(1);
}


void WriteEFUSETestIO(PCFG_SCRIPT Script)
{
	INT32U		Mask;
	INT32U		RetVal,SetVal;

	Mask = (0xFFFF << Script->Lbit) & (0xFFFF >> (15 - Script->Hbit));

	if(Mask != 0xFFFF)
	{
		RetVal = ParaEFUSERead(Script->Dev , Script->Offset);
		SetVal = (RetVal & (0xFFFF ^ Mask)) | (Script->Value & Mask);
	}
	else
	{
		SetVal = Script->Value;
	}
	
	ParaEFUSEWrite(Script->Dev,Script->Offset,SetVal);
}



void ParaEFUSEWrite16(INT8U dev , INT32U RegNum, INT32U RegVal)
{
	INT32U ValRead;
	INT32U ValSet,tmp17,tmp16;
	tmp17=(RegVal & 0x10000)<<7;
	tmp16=(RegVal&0x0FFFF);
	ValSet=(1<<30)|(RegNum<<16)|(tmp16)|(tmp17);
	ParaOCPWrite(dev ,0xDD00, ValSet, NUM_DWORD , 0x0f);

	do
	{
		ValRead = ParaOCPRead(dev ,0xDD00, NUM_DWORD);
		if(!(ValRead & 0x40000000)) break;
	}
	while(1);
}

void WriteEFUSETestIO16(PCFG_SCRIPT Script)
{
	INT32U		Mask;
	INT32U		RetVal,SetVal;

	// enable efuse eeprom,enable efuse pg & dis -hide 
	ParaOCPWrite (Script->Dev , 0xe85a ,0xc400 ,NUM_WORD,0x03) ;

	Mask = (0x1FFFF << Script->Lbit) & (0x1FFFF >> (16 - Script->Hbit));

	if(Mask != 0x1FFFF)
	{
		RetVal = ParaEFUSERead(Script->Dev , Script->Offset);
		SetVal = (RetVal & (0x1FFFF ^ Mask)) | (Script->Value & Mask);
	}
	else
	{
		SetVal = Script->Value;
	}
	
	ParaEFUSEWrite16(Script->Dev,Script->Offset,SetVal);
}



//-----------------------------------------------------------------------


INT32U ParaEPHYRead(INT32U dev ,INT32U RegNum)
{
	INT32U ValRead;
	INT32U ValSet;

	ValSet=RegNum<<16;
	ParaOCPWrite(dev,0xDE20, ValSet,NUM_DWORD , 0x0f);

	do
	{
		ValRead = ParaOCPRead(dev, 0xDE20,NUM_DWORD);
		if(ValRead & 0x80000000) break;
	}
	while(1);

	return ValRead;
}


INT16U 
ReadEPhyRegTestIO(PCFG_SCRIPT Script)
{
	INT16U		Mask;
	INT32U		RegVal;


	RegVal =(INT16U ) ParaEPHYRead(Script->Dev , Script->Offset);


}


void ParaEPHYWrite(INT32U dev ,INT32U RegNum, INT32U RegVal)
{
	INT32U ValRead;
	INT32U ValSet;

	ValSet=(1<<31)|(RegNum<<16)|(RegVal);
	ParaOCPWrite(dev, 0xDE20, ValSet , NUM_DWORD , 0x0f);

	do
	{
		ValRead = ParaOCPRead(dev, 0xDE20, NUM_DWORD);
		if(!(ValRead & 0x80000000)) break;
	}
	while(1);
}


void
WriteEPhyRegTestIO(PCFG_SCRIPT Script)
{
	INT32U		Mask;
	INT32U		RetVal,SetVal;

	Mask = (0xFFFF << Script->Lbit) & (0xFFFF >> (15 - Script->Hbit));

	if(Mask != 0xFFFF)
	{
		RetVal = ParaEPHYRead(Script->Dev ,Script->Offset);
		SetVal = ((INT16U)RetVal & (0xFFFF ^ Mask)) | ((INT16U)Script->Value & Mask);
	}
	else
	{
		SetVal = (INT16U)Script->Value;
	}
	
	ParaEPHYWrite(Script->Dev, Script->Offset , SetVal);

}


//-----------------------------------------------------------------------------

INT32U ReadOCPRegTestIO(PCFG_SCRIPT Script)
{
	
	INT32U		RetVal;
	

	RetVal = ParaOCPRead(Script->Dev ,Script->Offset,Script->Size);
	
}


void WriteOCPRegTestIO(PCFG_SCRIPT Script)
{
	INT32U		Mask;
	INT32U		RetVal,SetVal;
	if (Script->Size == NUM_DWORD){
		Mask = (0xFFFFFFFF << Script->Lbit) & (0xFFFFFFFF >> (31 - Script->Hbit));
		
		if(Mask != 0xFFFFFFFF)
		{
			RetVal = ParaOCPRead(Script->Dev ,Script->Offset,NUM_DWORD);
			SetVal = (RetVal & (0xFFFFFFFF ^ Mask)) | (Script->Value & Mask);
		}
		else
		{
			SetVal = Script->Value;
		}
	}else{
		Mask = (0xFFFF << Script->Lbit) & (0xFFFF >> (15 - Script->Hbit));
		
		if(Mask != 0xFFFF)
		{
			RetVal = ParaOCPRead(Script->Dev ,Script->Offset,NUM_WORD);
			SetVal = (RetVal & (0xFFFF ^ Mask)) | (Script->Value & Mask);
		}
		else
		{
			SetVal = Script->Value;
		}
	}

	ParaOCPWrite(Script->Dev , Script->Offset,SetVal , Script->Size , Script->ByteEn);
}

extern  INT32U Freq_MMD ;


INT8U Testio_Processing ( pTYPE_TESTIO ptype_testio ,pTESTIO_CMD ptestio_cmd  ){

	INT8U retval=0;


	if (ptype_testio->clk ){
		Freq_MMD = 1000000/ptype_testio->clk ; //unit : ns
		if (Freq_MMD == 0)	Freq_MMD = 1000 ;
	}else {
		//use default clock 
		Freq_MMD = 1000 ; 
	}


	CFG_SCRIPT cgf_script ; 
	

	INT16U i=0  ;
	INT32U read_date ;
	
	
	for ( i=0 ;i < ptype_testio->setnum;i++){
	
		
		cgf_script.Offset = (&ptestio_cmd[i])->addr ;
		cgf_script.Value= (&ptestio_cmd[i])->data ; 
		cgf_script.Hbit= (&ptestio_cmd[i])->info.INFO_TESTIO_Normal.hbits;
		cgf_script.Lbit = (&ptestio_cmd[i])->info.INFO_TESTIO_Normal.lbits ;
		cgf_script.Size = (&ptestio_cmd[i])->info.INFO_TESTIO_Normal.word_num ; 
		cgf_script.ByteEn =  ((0xF >> (3-(cgf_script.Hbit>>3))) & (0xF<< (cgf_script.Lbit>>3))) ;
		

		switch ((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.dev ){
			//PCI testio
			case DEV_PCI:
				cgf_script.Dev= DEV_PCI ;
				break ;
			case DEV_USBPLA: 
				cgf_script.Dev= DEV_USBPLA ;
				break ;
			case DEV_USBDMA: 
				cgf_script.Dev = DEV_USBDMA ;
				break ;
			default:
				;
		}

		switch ((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.channel ){
			case SUBTYPE_OCP:
				if ( ((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.mode == MODE_ALIVE) &&\
					((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.opera_rw == CMD_READ) ){						
						read_date = ReadOCPRegTestIO (&cgf_script);
						// must be split into int16U write , address is not dw align
						*((INT16U *)(&ptestio_cmd[i])->data_addr) = (INT16U) read_date  ; 
						*(((INT16U *)(&ptestio_cmd[i])->data_addr)+1) = (INT16U) ((read_date & 0xFFFF0000)>>16)  ;  
				}else if ( ((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.mode == MODE_ALIVE) &&\ 
					((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.opera_rw == CMD_WRITE) ){
						WriteOCPRegTestIO (&cgf_script); 
				}else if ( ((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.mode == MODE_STALL) &&\
					((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.opera_rw == CMD_READ) ){
					read_date = ReadOCPMCUSTALLRegTestIO (&cgf_script)  ;
					*((INT16U *)(&ptestio_cmd[i])->data_addr) = (INT16U) read_date  ; 
					if (cgf_script.Size){
						cgf_script.Offset += 2 ;
						read_date = ReadOCPMCUSTALLRegTestIO (&cgf_script)  ;
						*(((INT16U *)(&ptestio_cmd[i])->data_addr)+1) = (INT16U) read_date  ;  
					}
					
				}else {
					WriteOCPMCUSTALLRegTestIO(&cgf_script) ;
				}
				break;

			//GPHY . pci dev & usb pla use GPHY channel 
			case SUBTYPE_GPHY :
				//gphy channel need word address 
				cgf_script.Offset = (&ptestio_cmd[i])->addr >> 1  ;
				
				//current only support alive mode , word size 
				if (((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.opera_rw == CMD_WRITE)){ 
					WritePhyRegNoPageTestIO ( &cgf_script) ;
				}else  {
					read_date = ReadPhyRegNoPageTestIO ( &cgf_script ) ;
					*((INT16U *)(&ptestio_cmd[i])->data_addr) = (INT16U) read_date  ; 
				}
				break ;

			//EFUSE ,pci dev & usb pla use
			// pcie & usb EFUSE channel define is not the same , fix me later !!!!
			case SUBTYPE_EFUSE :
				//efuse channel need word address 
				cgf_script.Offset = (&ptestio_cmd[i])->addr >> 1  ;
				
				//current only support alive mode ,word size 
				if (((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.opera_rw == CMD_WRITE)){
					WriteEFUSETestIO16 (&cgf_script) ;
				}else {
					//cgf_script.Dev = DEV_USBPLA;
					//cgf_script.Offset= (&ptestio_cmd[i])->addr ;
					read_date = ReadEFUSETestIO16 (&cgf_script) ; ;
					*((INT16U *)(&ptestio_cmd[i])->data_addr) = (INT16U) read_date  ;
				}
				break ;
			case SUBTYPE_EPHY : 
				//ephy channel need word address 
				cgf_script.Offset = (&ptestio_cmd[i])->addr >> 1  ;
				
				//current only support alive mode ,word size 
				if (((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.opera_rw == CMD_WRITE)){
					WriteEPhyRegTestIO (&cgf_script) ;
				}else {
					//cgf_script.Dev = DEV_USBPLA;
					//cgf_script.Offset= (&ptestio_cmd[i])->addr ;
					read_date = ReadEPhyRegTestIO (&cgf_script) ; ;
					*((INT16U *)(&ptestio_cmd[i])->data_addr) = (INT16U) read_date  ;
				}
				break ;
			default :
				;

		}
		

	}
	
	
	return  0 ;

}



INT8U Testio_Parsing (struct command_block_wrapper  *CMD , INT8U * data_addr , INT16U *pdata_num){
	
	INT16U i=0;
	INT16U offset = sizeof(TYPE_TESTIO) ; //mmd cmd offset 
	pTYPE_TESTIO p  = (pTYPE_TESTIO)(CMD->data);
	pTESTIO_CMD ptestio_cmd ;
	INT8U retval=0;
	*pdata_num =0 ; 
	
	ptestio_cmd = (pTESTIO_CMD)malloc (sizeof(TESTIO_CMD)*(p->setnum));

	for (i=0; i< p->setnum;i++){
		(&ptestio_cmd[i])->info.info_raw = ( CMD->data[offset+3]<<24 ) + (CMD->data[offset+2]<<16)  +\
			( CMD->data[offset+1]<<8 ) + CMD->data[offset+0] ;
		(&ptestio_cmd[i])->addr = ( CMD->data[offset+5]<<8 )+ CMD->data[offset+4];
		(&ptestio_cmd[i])->data = ( (INT32U)((CMD->data[offset+9]))<<24 )+ ((INT32U)((CMD->data[offset+8]))<<16)+ \
			((INT32U) ((CMD->data[offset+7]))<<8 )+ (INT32U)((CMD->data[offset+6]));
		(&ptestio_cmd[i])->data_addr = data_addr +  (*pdata_num)  ;

		offset += 12 ; 
		if ((((&ptestio_cmd[i])->info.INFO_TESTIO_Normal.word_num) ) == NUM_DWORD ) {
			
			if ((((&ptestio_cmd[i])->info.info_raw>>1)&0x01 ) == CMD_READ)
				*pdata_num += 4 ; // one read num in alive mode -> DWORD
			
		}else {
			//offset += 8 ; 
			if ((((&ptestio_cmd[i])->info.info_raw>>1)&0x01 ) == CMD_READ)
				*pdata_num += 2 ; // one read num in satll mode -> WORD
			
		}

		
		
	}			
			

	//call MMD function to processing CMD	
	retval = Testio_Processing(p,ptestio_cmd);
	free (ptestio_cmd);

	return retval ;

}


