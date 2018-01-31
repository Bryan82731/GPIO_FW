#include "lib.h"
#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <bsp_cfg.h>

#include "OOBMAC_COMMON.h"
#include "mdiobitbang.h"
#include "usb.h"


enum _RW_CMD{
	CMD_WRITE,
	CMD_READ ,
	CMD_OTHER
} ;

enum _MODE_CMD{
	MODE_ALIVE,
	MODE_STALL
};


typedef struct _TYPE_MMD {
	INT8U setnum ;
	INT16U	clk	;

}__attribute__ ((__packed__)) TYPE_MMD,*pTYPE_MMD;

typedef union _info {
	INT16U info_raw ;
	
	struct _INFO_MMD_Normal {
		INT16U mode	:	1;
		INT16U opera_rw :	2;
		INT16U lbits	:	4;
		INT16U hbits	:	4;
		INT16U subtype	:	5;
	}INFO_MMD_Normal;

	struct _INFO_MMD_POST {
		INT16U mode	:	1;
		INT16U opera_rw :	2;
		INT16U post_num	:	8;
		INT16U subtype	:	5;
	}INFO_MMD_POST;
	

	struct _INFO_MMD_GPHY {
		INT16U mode	:	1;
		INT16U opera_rw :	2;
		INT16U lbits	:	4;
		INT16U hbits	:	4;
		INT16U subtype	:	5;
	}INFO_MMD_GPHY;

	struct _INFO_MMD_EPHY {
		INT16U mode	:	1;
		INT16U opera_rw :	2;
		INT16U lbits	:	4;
		INT16U hbits	:	4;
		INT16U subtype	:	5;
	}INFO_MMD_EPHY;

	
	struct _INFO_MMD_EFUSE {
			INT16U mode :	1;
			INT16U opera_rw :	2;
			INT16U lbits	:	4;
			INT16U hbits	:	4;
			INT16U subtype	:	5;
		}INFO_MMD_EFUSE;
	
}__attribute__ ((__packed__)) info_union ;


typedef struct __MMD_CMD {
	info_union info ;
	INT8U PRTAD;
	INT8U DEVAD;
	INT16U addr;
	INT16U data;
	INT16U *data_addr;
}__attribute__ ((__packed__))MMD_CMD,*pMMD_CMD;





extern  OS_CPU_SR  cpu_sr;


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
	INT8U Port ; 

}	CFG_SCRIPT, *PCFG_SCRIPT;




INT8U  OCD_Write(INT8U dev , INT16U adr , INT16U val , INT8U byte_enable){

	INT16U timeout=0;

	/*USB dev need to set a other reg than pcie dev*/
	// usb pla mcu 
	if (dev == 1){
		mdiobb_write( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x8000>>1), 0x0000);	
	}else if (dev == 2) {  // usb dma mcu 
		mdiobb_write( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x8000>>1), 0x0080);	
	}
	
	
	//set testio_ocp_data0
	
	mdiobb_write( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x8008>>1), val);	

	//set testio_ocp_addr
	mdiobb_write(0x01, MII_ADDR_C45|(0x1F<<16)|(0x800C>>1), adr );

	//set testio_ocp_cmd
	mdiobb_write(0x01, MII_ADDR_C45|(0x1F<<16)|(0x800E>>1), 0x8000 | byte_enable);

	//check testio_ocp_cmd's test_acc_en
	while(mdiobb_read( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x800E>>1)) & 0x8000){
		timeout++;
		if(timeout==10){
			printk("OCD write hang in polling acc_en!\n");
			return 0xff;
		}
	}

	//  Maybe next time, mmd will enter in MCU stall mode, and pla_usb_sw will disturb the signal dbg_sw_pla_usb, 
	//so it's better to back the default value,when finishing the MCU alive mode
	if (dev == 2){
		mdiobb_write( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x8000>>1), 0x0000);	
	}

	return 0;
	
		

}


INT8U  OCD_Read(INT8U dev , INT16U adr , INT16U * data ){

	INT16U timeout=0;

	

	/*USB dev need to set a other reg than pcie dev*/
	// usb pla mcu 
	if (dev == 1){
		mdiobb_write( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x8000>>1), 0x0000);	
	}else if (dev == 2) {  // usb dma mcu 
		mdiobb_write( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x8000>>1), 0x0800);	
	}
	
	
	//set testio_ocp_addr
	mdiobb_write( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x800C>>1), adr );

	//set testio_ocp_cmd
	mdiobb_write( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x800E>>1), 0x8000);

	//check testio_ocp_cmd's test_acc_en
	while(mdiobb_read( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x800E>>1)) & 0x8000){
		timeout++;
		if(timeout==10){
			printk("OCD Read hang in polling acc_en!\n");
			return 0xff;
		}
	}

	//return testio_ocp_data
 	*data = mdiobb_read(0x01, MII_ADDR_C45|(0x1F<<16)|(0x8008>>1));

	//printk("MMD add: 0x%x  , data :0x%x  \r\n",adr , *data);

	
	//	Maybe next time, mmd will enter in MCU stall mode, and pla_usb_sw will disturb the signal dbg_sw_pla_usb, 
	//so it's better to back the default value,when finishing the MCU alive mode
	if (dev == 2){
		mdiobb_write( 0x01, MII_ADDR_C45|(0x1F<<16)|(0x8000>>1), 0x0000);	
	}

	return 0;

		
}
		



//1、對訪問ocp space register的address，需要先除2，再送進去
//2、訪問非ocp space的address，則直接送出
//fix me 
INT8U  Direct_Write(INT8U  portadr, INT8U devad, INT16U adr , INT16U val){
	
	
	mdiobb_write( portadr, MII_ADDR_C45|(devad<<16)|adr, val );	
	return 0 ; 
	

}


INT8U Direct_Read(INT8U portadr, INT8U devad, INT16U adr , INT16U * data ){
	

	*data = mdiobb_read(portadr , MII_ADDR_C45|(devad<<16)|adr);
	//printk("MMD add: 0x%x  , data :0x%x  \r\n",adr , *data); // LED share with uart
	return 0;
	
}


INT8U Direct_post_Read(INT8U  portadr, INT8U devad, INT16U adr, INT16U * desadr,INT16U step ){

	return mdiobb_post_read(portadr, MII_ADDR_C45|(devad<<16)|adr,desadr,step);

}


INT8U  EFUSE_read (PCFG_SCRIPT script , INT16U * read_data){

	INT32U tmp , tmpdelay=0;
	INT8U retval =0 ;

	// enable efuse eeprom
	Direct_Write (script->Port, script->Dev ,\
		0xe85a >> 1 , 0xc400 ) ;  // enable efuse pg & dis -hide 
		
	Direct_Write (script->Port , script->Dev,\
		0xdd04 >> 1 , 0x8000 | (script->Offset>>2) ) ;  // efuse address need Dword alignment 
	do {
		retval = Direct_Read(script->Port, \
			script->Dev, 0xdd04 >> 1 , &tmp);
		tmpdelay ++ ; 
	}while ((tmp & 0x8000) && (tmpdelay<1000)) ;


	if(tmpdelay==1000)
	{
	//ShowDebugNum(0x60);
	//cprintf("\n\n\tWrite Efuse Flag bit not changed....\n");
	//fail !!
	retval = 1  ; //error
	
	}

	
	Direct_Read (script->Port, script->Dev,\
		(0xdd00+(script->Offset%4)) >> 1 , read_data);

	return retval ; 
	
}

void EFUSE_write (PCFG_SCRIPT script)
{
	INT16U rd_data ;
	INT16U wr_data ;
	INT16U Mask ;

	// enable efuse eeprom
	Direct_Write (script->Port, script->Dev ,\
		0xe85a >> 1 , 0xc400 ) ;  // enable efuse pg & dis -hide 

	// before write , read efuse value ,
	EFUSE_read ( script , &rd_data) ;


	// modify the word value based write efuse value & address 
	Mask = (0xFFFF << script->Lbit) & (0xFFFF >> (15 - script->Hbit));

	if(Mask != 0xFFFF)
	{
		
		wr_data = ( rd_data & (0xFFFF ^ Mask)) | (script->Value & Mask);
	}
	else
	{
		wr_data = script->Value;
	}
	
	//write back  data 
	Direct_Write (script->Port, script->Dev ,\
		(0xdd00+(script->Offset%4)) >> 1 , wr_data ) ;  

	// cmd write 
	Direct_Write (script->Port , script->Dev,\
		0xdd04 >> 1 , 0x4000 | (script->Offset>>2) ) ;
	
}


INT8U  GPHY_read (PCFG_SCRIPT script , INT16U * read_data){

	INT16U tmp , tmpdelay=0;
	INT8U retval =0 ;
		
	Direct_Write (script->Port , script->Dev,\
		0xde02 >> 1 , 0x0000 | (script->Offset>>1) ) ;  // //gphy channel need word address  
	do {
		retval = Direct_Read(script->Port, \
			script->Dev, 0xde02 >> 1 , &tmp);
		tmpdelay ++ ; 
	}while ((!(tmp & 0x8000)) && (tmpdelay<1000)) ;


	if(tmpdelay==1000)
	{
	//ShowDebugNum(0x60);
	//cprintf("\n\n\tWrite Efuse Flag bit not changed....\n");
	//fail !!
	retval = 1  ; //error
	
	}

	
	Direct_Read (script->Port, script->Dev,\
		0xde00 >> 1 , read_data);

	return retval ; 
	
}

void GPHY_write (PCFG_SCRIPT script)
{
	INT16U rd_data ;
	INT16U wr_data ;
	INT16U Mask ;
	INT8U retval =0 ;
	INT16U tmp ,tmpdelay=0;;


	// modify the word value based write efuse value & address 
	Mask = (0xFFFF << script->Lbit) & (0xFFFF >> (15 - script->Hbit));

	if(Mask != 0xFFFF)
	{
		// before write , read efuse value ,
		GPHY_read ( script , &rd_data) ;
		
		wr_data = ( rd_data & (0xFFFF ^ Mask)) | (script->Value & Mask);
	}
	else
	{
		wr_data = script->Value;
	}
	
	//write back  data 
	Direct_Write (script->Port, script->Dev ,\
		0xde00 >> 1 , wr_data ) ;  

	// cmd write 
	Direct_Write (script->Port , script->Dev,\
		0xde02 >> 1 , 0x8000 | (script->Offset>>1) ) ;

	//wait complete
	
	do {
		retval = Direct_Read(script->Port, \
			script->Dev, 0xde02 >> 1 , &tmp);
		tmpdelay ++ ; 
	}while (((tmp & 0x8000)) && (tmpdelay<1000)) ;
	
}



INT8U  EPHY_read (PCFG_SCRIPT script , INT16U * read_data){

	INT16U tmp , tmpdelay=0;
	INT8U retval =0 ;
		
	Direct_Write (script->Port , script->Dev,\
		0xde22 >> 1 , 0x0000 | (script->Offset>>1) ) ;  // //gphy channel need word address  
	do {
		retval = Direct_Read(script->Port, \
			script->Dev, 0xde22 >> 1 , &tmp);
		tmpdelay ++ ; 
	}while ((!(tmp & 0x8000)) && (tmpdelay<1000)) ;


	if(tmpdelay==1000)
	{
	//ShowDebugNum(0x60);
	//cprintf("\n\n\tWrite Efuse Flag bit not changed....\n");
	//fail !!
	retval = 1  ; //error
	
	}

	
	Direct_Read (script->Port, script->Dev,\
		0xde20 >> 1 , read_data);

	return retval ; 
	
}

void EPHY_write (PCFG_SCRIPT script)
{
	INT16U rd_data ;
	INT16U wr_data ;
	INT16U Mask ;
	INT8U retval =0 ;
	INT16U tmp ,tmpdelay=0;;


	// modify the word value based write efuse value & address 
	Mask = (0xFFFF << script->Lbit) & (0xFFFF >> (15 - script->Hbit));

	if(Mask != 0xFFFF)
	{
		// before write , read efuse value ,
		EPHY_read ( script , &rd_data) ;
		
		wr_data = ( rd_data & (0xFFFF ^ Mask)) | (script->Value & Mask);
	}
	else
	{
		wr_data = script->Value;
	}
	
	//write back  data 
	Direct_Write (script->Port, script->Dev ,\
		0xde20 >> 1 , wr_data ) ;  

	// cmd write 
	Direct_Write (script->Port , script->Dev,\
		0xde22 >> 1 , 0x8000 | (script->Offset>>1) ) ;

	//wait complete
	
	do {
		retval = Direct_Read(script->Port, \
			script->Dev, 0xde22 >> 1 , &tmp);
		tmpdelay ++ ; 
	}while (((tmp & 0x8000)) && (tmpdelay<1000)) ;
	
}




INT32U Freq_MMD ;


 

INT8U MMD_Processing ( pTYPE_MMD ptype_mmd ,pMMD_CMD pmmd_cmd  ){

	INT8U retval=0;


	if (ptype_mmd->clk ){
		Freq_MMD = 1000000/ptype_mmd->clk ; //unit : ns
		if (Freq_MMD == 0)	Freq_MMD = 500  ;
	}else {
		//use default clock 
		Freq_MMD = 500 ; 
	}
	

	//Freq_MMD == 0 ;
	
	INT16U i=0  ;
	INT16U tmpdelay =0  ;
	INT32U tmp ;
	CFG_SCRIPT script ; 
	INT16U Mask ;
	INT16U readval ,setval ;
	
	
	
	
	for ( i=0 ;i < ptype_mmd->setnum;i++){
	
		switch ((((&pmmd_cmd[i])->info.info_raw >> 14) &0x03)){
			//PCI MMD
			case 0x00:
			// USB PLA mcu
			case 0x01 : 
			//USB DMA mcu	
			case 0x02 :	
				switch (((&pmmd_cmd[i])->info.info_raw >> 11) & 0x07){
					//normal MMD
					case 0x0 :
						if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_ALIVE) &&\
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_READ) ){
							//the addr is orignal ,  and H/W ignore the lsb of address 
							retval = OCD_Read((((&pmmd_cmd[i])->info.info_raw >> 14) &0x03), (&pmmd_cmd[i])->addr , &readval) ;
							// mask 
							Mask = (0xFFFF << ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits)) & \
								(0xFFFF >> (15 - ((&pmmd_cmd[i])->info.INFO_MMD_Normal.hbits)));
							if(Mask != 0xFFFF) 
								*((&pmmd_cmd[i])->data_addr) =  (readval & Mask )>> ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits) ;
							else 
								*((&pmmd_cmd[i])->data_addr) = readval ;
									
						}else if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_ALIVE) &&\ 
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_WRITE) ){
							//mask
							Mask = (0xFFFF << ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits)) & \
								(0xFFFF >> (15 - ((&pmmd_cmd[i])->info.INFO_MMD_Normal.hbits)));
							if(Mask != 0xFFFF)
							{
								retval = OCD_Read((((&pmmd_cmd[i])->info.info_raw >> 14) &0x03), (&pmmd_cmd[i])->addr ,&readval) ;
								setval  = ( readval & (0xFFFF ^ Mask)) | ((&pmmd_cmd[i])->data & Mask);
							}
							else
							{
								setval = (&pmmd_cmd[i])->data ;
							}
							
							//the addr is orignal ,and H/W ignore the lsb of address ; current not support byte enable 
							retval = OCD_Write((((&pmmd_cmd[i])->info.info_raw >> 14) &0x03),(&pmmd_cmd[i])->addr,setval , 0x03);  
						
						}else if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_STALL) &&\ 
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_READ) ){
							//the addr is word alignment  , 
							retval = Direct_Read((&pmmd_cmd[i])->PRTAD, \
								(&pmmd_cmd[i])->DEVAD, (&pmmd_cmd[i])->addr >> 1 ,  &readval);

							// mask 
							Mask = (0xFFFF << ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits)) & \
								(0xFFFF >> (15 - ((&pmmd_cmd[i])->info.INFO_MMD_Normal.hbits)));
							if(Mask != 0xFFFF) 
								*((&pmmd_cmd[i])->data_addr) =  (readval & Mask )>> ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits) ;
							else {
								*((&pmmd_cmd[i])->data_addr) =  readval  ;
							}
						}else if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_STALL) &&\ 
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_WRITE) ){
							//mask
							Mask = (0xFFFF << ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits)) & \
								(0xFFFF >> (15 - ((&pmmd_cmd[i])->info.INFO_MMD_Normal.hbits)));
							if(Mask != 0xFFFF)
							{
								retval = Direct_Read((&pmmd_cmd[i])->PRTAD, \
								(&pmmd_cmd[i])->DEVAD, (&pmmd_cmd[i])->addr >> 1 , &readval);
								setval  = ( readval & (0xFFFF ^ Mask)) | ((&pmmd_cmd[i])->data & Mask);
							}
							else
							{
								setval = (&pmmd_cmd[i])->data ;
							}
							
							retval = Direct_Write((&pmmd_cmd[i])->PRTAD , (&pmmd_cmd[i])->DEVAD ,\
								(&pmmd_cmd[i])->addr >> 1 , setval) ;
						}else {
							//the addr is word alignment  ,
							retval = Direct_post_Read((&pmmd_cmd[i])->PRTAD, (&pmmd_cmd[i])->DEVAD, \
								(&pmmd_cmd[i])->addr >> 1 , ((&pmmd_cmd[i])->data_addr),\
								(&pmmd_cmd[i])->info.INFO_MMD_POST.post_num );
		
						}
						break;
					//GPHY
					case 0x1 :
						script.Port = (&pmmd_cmd[i])->PRTAD ;
						script.Dev = (&pmmd_cmd[i])->DEVAD ;
						script.Offset = (&pmmd_cmd[i])->addr ;
						script.Hbit = (&pmmd_cmd[i])->info.INFO_MMD_GPHY.hbits ;
						script.Lbit = (&pmmd_cmd[i])->info.INFO_MMD_GPHY.lbits ;
						script.Value = (&pmmd_cmd[i])->data ; 
						
						if (((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_READ)){
														
							GPHY_read (&script , &readval);
								// mask 
							Mask = (0xFFFF << ((&pmmd_cmd[i])->info.INFO_MMD_GPHY.lbits)) & \
								(0xFFFF >> (15 - ((&pmmd_cmd[i])->info.INFO_MMD_GPHY.hbits)));
							if(Mask != 0xFFFF) 
								*((&pmmd_cmd[i])->data_addr) =  (readval & Mask )>> ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits) ;
							else {
								*((&pmmd_cmd[i])->data_addr) =  readval  ;
							}

						}else{

							GPHY_write (&script) ;
						}

					

						
						break ;
					//EFUSE
					case 0x02 :
						script.Port = (&pmmd_cmd[i])->PRTAD ;
						script.Dev = (&pmmd_cmd[i])->DEVAD ;
						script.Offset = (&pmmd_cmd[i])->addr ;
						script.Hbit = (&pmmd_cmd[i])->info.INFO_MMD_EFUSE.hbits ;
						script.Lbit = (&pmmd_cmd[i])->info.INFO_MMD_EFUSE.lbits ;
						script.Value = (&pmmd_cmd[i])->data ; 
						
							
						if (((&pmmd_cmd[i])->info.INFO_MMD_EFUSE.opera_rw == CMD_READ)){
								
							EFUSE_read (&script , &readval);

							// mask 
							Mask = (0xFFFF << ((&pmmd_cmd[i])->info.INFO_MMD_EFUSE.lbits)) & \
								(0xFFFF >> (15 - ((&pmmd_cmd[i])->info.INFO_MMD_EFUSE.hbits)));
							if(Mask != 0xFFFF) 
								*((&pmmd_cmd[i])->data_addr) =  (readval & Mask )>> ((&pmmd_cmd[i])->info.INFO_MMD_EFUSE.lbits) ;
							else {
								*((&pmmd_cmd[i])->data_addr) =  readval  ;
							}

						}else{

							EFUSE_write (&script) ;
						}

	
						break ;
					//EPHY
					case 0x3 :
						script.Port = (&pmmd_cmd[i])->PRTAD ;
						script.Dev = (&pmmd_cmd[i])->DEVAD ;
						script.Offset = (&pmmd_cmd[i])->addr ;
						script.Hbit = (&pmmd_cmd[i])->info.INFO_MMD_EPHY.hbits ;
						script.Lbit = (&pmmd_cmd[i])->info.INFO_MMD_EPHY.lbits ;
						script.Value = (&pmmd_cmd[i])->data ; 
						
						if (((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_READ)){
														
							EPHY_read (&script , &readval);

							// mask 
							Mask = (0xFFFF << ((&pmmd_cmd[i])->info.INFO_MMD_EPHY.lbits)) & \
								(0xFFFF >> (15 - ((&pmmd_cmd[i])->info.INFO_MMD_EPHY.hbits)));
							if(Mask != 0xFFFF) 
								*((&pmmd_cmd[i])->data_addr) =  (readval & Mask )>> ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits) ;
							else {
								*((&pmmd_cmd[i])->data_addr) =  readval  ;
							}
						}else{

							EPHY_write (&script) ;
						}

						
						break ;
					default :
						;
						
				
				}
				break ;
/*
			// USB PLA mcu
			case 0x01 : 
				switch (((&pmmd_cmd[i])->info.info_raw >> 11) & 0x07){
					//normal MMD
					case 0x0 :
						if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_ALIVE) &&\
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_READ) ){
							//the addr is orignal ,  and H/W ignore the lsb of address 
							retval = OCD_Read(1,(&pmmd_cmd[i])->addr ,(&pmmd_cmd[i])->data_addr) ;
										
						}else if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_ALIVE) &&\ 
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_WRITE) ){
							//the addr is orignal ,and H/W ignore the lsb of address ; current not support byte enable 
							retval = OCD_Write(1,(&pmmd_cmd[i])->addr,(&pmmd_cmd[i])->data , 0x03);  
						
						}else if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_STALL) &&\ 
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_READ) ){
							//the addr is word alignment  , 
							retval = Direct_Read((&pmmd_cmd[i])->PRTAD, \
								(&pmmd_cmd[i])->DEVAD, (&pmmd_cmd[i])->addr >> 1 , ((&pmmd_cmd[i])->data_addr) );
						}else if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_STALL) &&\ 
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_WRITE) ){
							retval = Direct_Write((&pmmd_cmd[i])->PRTAD , (&pmmd_cmd[i])->DEVAD ,\
								(&pmmd_cmd[i])->addr >> 1 , (&pmmd_cmd[i])->data) ;
						}else {
							//the addr is word alignment  ,
							retval = Direct_post_Read((&pmmd_cmd[i])->PRTAD, (&pmmd_cmd[i])->DEVAD, \
								(&pmmd_cmd[i])->addr >> 1 , ((&pmmd_cmd[i])->data_addr),\
								(&pmmd_cmd[i])->info.INFO_MMD_Normal.post_num );
		
						}
						break;
					//GPHY
					case 0x1 :
						break ;
					case 0x02 :
						script.Port = (&pmmd_cmd[i])->PRTAD ;
						script.Dev = (&pmmd_cmd[i])->DEVAD ;
						script.Offset = (&pmmd_cmd[i])->addr ;
						script.Hbit = (&pmmd_cmd[i])->info.INFO_MMD_EFUSE.hbits ;
						script.Lbit = (&pmmd_cmd[i])->info.INFO_MMD_EFUSE.lbits ;
						script.Value = (&pmmd_cmd[i])->data ; 
						
							
						if (((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_READ)){
								
							EFUSE_read (&script , ((&pmmd_cmd[i])->data_addr));

						}else{

							EFUSE_write (&script) ;
						}

	
						break ;
					//EPHY
					//case 0x02 :
					//	break ;
					default :
						;
						
				
				}
				break ;
				
			//USB DMA mcu	
			case 0x02 :
				switch (((&pmmd_cmd[i])->info.info_raw >> 11) & 0x07){
					//normal MMD
					case 0x0 :
						if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_ALIVE) &&\
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_READ) ){
							//the addr is orignal ,  and H/W ignore the lsb of address 
							retval = OCD_Read(2,(&pmmd_cmd[i])->addr ,(&pmmd_cmd[i])->data_addr) ;
										
						}else if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_ALIVE) &&\ 
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_WRITE) ){
							//the addr is orignal ,and H/W ignore the lsb of address ; current not support byte enable 
							retval = OCD_Write(2,(&pmmd_cmd[i])->addr,(&pmmd_cmd[i])->data , 0x03);  
						
						}else if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_STALL) &&\ 
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_READ) ){
							//the addr is word alignment  , 
							retval = Direct_Read((&pmmd_cmd[i])->PRTAD, \
								(&pmmd_cmd[i])->DEVAD, (&pmmd_cmd[i])->addr >> 1 , ((&pmmd_cmd[i])->data_addr) );
						}else if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.mode == MODE_STALL) &&\ 
							((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_WRITE) ){
							retval = Direct_Write((&pmmd_cmd[i])->PRTAD , (&pmmd_cmd[i])->DEVAD ,\
								(&pmmd_cmd[i])->addr >> 1 , (&pmmd_cmd[i])->data) ;
						}else {
							//the addr is word alignment  ,
							retval = Direct_post_Read((&pmmd_cmd[i])->PRTAD, (&pmmd_cmd[i])->DEVAD, \
								(&pmmd_cmd[i])->addr >> 1 , ((&pmmd_cmd[i])->data_addr),\
								(&pmmd_cmd[i])->info.INFO_MMD_Normal.post_num );
		
						}
						break;
					}
				break ;
*/				
			//other DEV 
				//the OCP  addr is orignal  , no need change  
			case 0x03 :
				if (  ((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_READ) ){
					
					retval = Direct_Read((&pmmd_cmd[i])->PRTAD, \
						(&pmmd_cmd[i])->DEVAD, (&pmmd_cmd[i])->addr , &readval );
					// mask 
					Mask = (0xFFFF << ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits)) & \
						(0xFFFF >> (15 - ((&pmmd_cmd[i])->info.INFO_MMD_Normal.hbits)));
					if(Mask != 0xFFFF) 
						*((&pmmd_cmd[i])->data_addr) =  (readval & Mask )>> ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits) ;
					else {
						*((&pmmd_cmd[i])->data_addr) =  readval  ;
					}
				}else if ( ((&pmmd_cmd[i])->info.INFO_MMD_Normal.opera_rw == CMD_WRITE) ){
					//mask
					Mask = (0xFFFF << ((&pmmd_cmd[i])->info.INFO_MMD_Normal.lbits)) & \
						(0xFFFF >> (15 - ((&pmmd_cmd[i])->info.INFO_MMD_Normal.hbits)));
					if(Mask != 0xFFFF)
					{
						retval = Direct_Read((&pmmd_cmd[i])->PRTAD, \
						(&pmmd_cmd[i])->DEVAD, (&pmmd_cmd[i])->addr  , &readval);
						setval  = ( readval & (0xFFFF ^ Mask)) | ((&pmmd_cmd[i])->data & Mask);
					}
					else
					{
						setval = (&pmmd_cmd[i])->data ;
					}
					
					retval = Direct_Write((&pmmd_cmd[i])->PRTAD , (&pmmd_cmd[i])->DEVAD ,\
						(&pmmd_cmd[i])->addr , setval) ;
				}else {
					retval = Direct_post_Read((&pmmd_cmd[i])->PRTAD, (&pmmd_cmd[i])->DEVAD, \
						(&pmmd_cmd[i])->addr , ((&pmmd_cmd[i])->data_addr),\
						(&pmmd_cmd[i])->info.INFO_MMD_POST.post_num );

				}
		
				break ;

			}
	}

	return  retval ;

}



INT8U MMD_Parsing (struct command_block_wrapper  *CMD , INT8U * data_addr , INT16U *pdata_num){
	
	INT16U i=0;
	INT8U offset = sizeof(TYPE_MMD) ; //mmd cmd offset 
	pTYPE_MMD p  = (pTYPE_MMD)(CMD->data);
	pMMD_CMD pmmd_cmd ;
	INT8U retval=0;
	*pdata_num =0 ; 
	
	pmmd_cmd = (pMMD_CMD)malloc (sizeof(MMD_CMD)*(p->setnum));

	for (i=0; i< p->setnum;i++){
		(&pmmd_cmd[i])->info.info_raw = ( CMD->data[i*8+offset+1]<<8 ) + CMD->data[i*8+offset+0] ;
		(&pmmd_cmd[i])->PRTAD = CMD->data[i*8+offset+2] ;
		(&pmmd_cmd[i])->DEVAD = CMD->data[i*8+offset+3] ; ;
		(&pmmd_cmd[i])->addr = ( CMD->data[i*8+offset+5]<<8 )+ CMD->data[i*8+offset+4];
		(&pmmd_cmd[i])->data = ( CMD->data[i*8+offset+7]<<8 )+ CMD->data[i*8+offset+6];
		(&pmmd_cmd[i])->data_addr = data_addr +  (*pdata_num)   ;
		if ((((&pmmd_cmd[i])->info.info_raw>>1)&0x03 ) == CMD_READ){
			*pdata_num += 2 ; // one read num -> WORD
		}else if ((((&pmmd_cmd[i])->info.info_raw>>1)&0x03 ) == CMD_OTHER){
			*pdata_num +=  (((&pmmd_cmd[i])->info.info_raw>>3)&0xff)<<1; //post read num 
		}
	}			
			

	//call MMD function to processing CMD	
	retval = MMD_Processing(p,pmmd_cmd);
	free (pmmd_cmd);

	return retval ;

}



void MMD_Init (){

	//GPIO init 	
	OOBMAC_gpio_init(MDC_PIN,OUTPUT,0) ; // default must be '0' ,or it will be failed in first time . mdio will climb just like cap charge ,why ?
	OOBMAC_gpio_init(MDIO_PIN,INPUT,0) ;



}

	
extern USBCB usbcb;




void MMD_Test_Task(){

	void * p ;
	pTYPE_MMD pmmd_type  ;
	pMMD_CMD pmmd_cmd ;
	struct command_block_wrapper  *cbw_tmp ;
	struct command_status_wrapper  *csw_tmp ;
	p = malloc(sizeof (struct command_block_wrapper)) ;
	if ( p !=NULL )
		 cbw_tmp = p ;
	else 
		goto ERROR1;

	p = malloc(sizeof (struct command_status_wrapper)) ;
	if ( p !=NULL )
		csw_tmp = p ;
	else 
		goto ERROR2;
	
	while(OS_TRUE)
	{

			
			OS_ENTER_CRITICAL();

			
			
			//creat CBW info
			cbw_tmp->SEQ = 0;
			cbw_tmp->Length =  16  ;
			cbw_tmp->Type = 0x02;
			pmmd_type = (pTYPE_MMD)(&(cbw_tmp->data[0])) ;
			pmmd_type->setnum = 1;
			pmmd_type->clk = 3000;  //10MHz
			pmmd_cmd = (pMMD_CMD)(((INT8U *)pmmd_type) + sizeof (TYPE_MMD)) ;
			pmmd_cmd->info.info_raw= 0x03;
			pmmd_cmd->PRTAD=0x01;
			pmmd_cmd->DEVAD=0x1E;
			pmmd_cmd->addr = 0x00 ; 
			
			CMD_Parsing ( cbw_tmp , csw_tmp  ) ; //parsing
			

			OS_EXIT_CRITICAL();
		
			OSTimeDly(OS_TICKS_PER_SEC);

			
	}

	ERROR2:
		free (cbw_tmp) ;
	ERROR1: 
		OSTaskDel(OS_PRIO_SELF);
		
	
}







