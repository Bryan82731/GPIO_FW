#include <stdlib.h>
#include <string.h>
#include "usb.h"
#include "bsp.h"
#include "tcp.h"
#include "telnet.h"
#include "flash.h"

extern USBCB usbcb;
RTSkt *usbcs;
extern DPCONF *dpconf;
extern unsigned char tip[4];
extern OS_EVENT *USBTxSem;


INT8U lc;
INT8U lcErr;
extern INT8U const inquiry_data[3][36];
extern INT8U const disc_conf[140];
extern INT8U const disc_sense[40];
extern INT8U const disc_info[34];
extern INT8U const mode_sense[18];
extern INT8U const iso_toc[24];
extern INT8U const iso_conf[16];


typedef struct _TYPE_FWUPDATE {
	INT16U PAGE_NUM ;
	INT8U  length 	;

}__attribute__ ((__packed__)) TYPE_FWUPDATE,*pTYPE_FWUPDATE;



INT8U FWupdate_Parsing (struct command_block_wrapper  *CMD , INT8U * data_addr , INT16U *pdata_num){
	INT16U i=0;
	INT8U offset = sizeof(TYPE_FWUPDATE) ; //mmd cmd offset 
	pTYPE_FWUPDATE p  = (pTYPE_FWUPDATE)(CMD->data);
	TYPE_FWUPDATE FWUPDATE_cmd ;
	INT8U retval=0;
	*pdata_num =0 ;
	static INT16U page_old = 0 ;
	INT16U length ;

	FWUPDATE_cmd.PAGE_NUM = (CMD->data [1]<<8 ) + CMD->data [0];
	// if the next fw update , clear this static record 
	if (FWUPDATE_cmd.PAGE_NUM == 0){
		page_old = 0; 	
	}
	if ((FWUPDATE_cmd.PAGE_NUM != page_old )){
		// miss page 
		retval = 1 ; 
		return retval ; 
	}else {
		page_old ++ ; 
	}
	
	FWUPDATE_cmd.length = CMD->data [2] ;
	length = (FWUPDATE_cmd.length ==0 )? 256:  FWUPDATE_cmd.length ;

	//one block = 256 page 
	if ( !(FWUPDATE_cmd.PAGE_NUM % 256 )  ){	
		Flash_erase_block(0x30000 + (FWUPDATE_cmd.PAGE_NUM)*256);
	}

	if (length >128){
		Flash_write_one_channel_User(128,0x30000+(FWUPDATE_cmd.PAGE_NUM)*256,&(CMD->data[3]));
		Flash_write_one_channel_User((length==256)?128:(length%128)  ,0x30000+(FWUPDATE_cmd.PAGE_NUM)*256+128,&(CMD->data[3+128]));
	}else{
		Flash_write_one_channel_User(length,0x30000+(FWUPDATE_cmd.PAGE_NUM)*256,&(CMD->data[3]));
	}
	

	return retval ;
}

extern  OS_CPU_SR  cpu_sr;


INT8U  CMD_Parsing(struct command_block_wrapper  *CMD , struct command_status_wrapper *CSW){
	INT8U retval= 0 ;
	INT16U data_num ;

	CSW->SEQ= CMD->SEQ;
    CSW->Type= CMD->Type;


	OS_ENTER_CRITICAL();
	

	switch ( CMD->Type ){
		//GPIO	
		case 0 :
			GPIO_IIC_Init() ;
			retval = GPIO_Parsing (CMD ,CSW->data,&data_num);
			CSW->Length = data_num + 8 ; //total length
			CSW->bCSWStatus = retval ; 
		break;

		//old testio
		case 1:

			Testio_Gpio_Init () ;
			retval = Testio_Parsing (CMD ,CSW->data,&data_num);
			CSW->Length = data_num + 8 ; //total length
			CSW->bCSWStatus = retval ; 
			
		break;

		//MMD
		case 2:
			MMD_Init () ;
			retval = MMD_Parsing (CMD ,CSW->data,&data_num);
			//rlx_icache_invalidate_all() ;
			CSW->Length = data_num + 8 ; //total length
			CSW->bCSWStatus = retval ; 
			
		break;

		//FW update 
		case 8 :
			retval = FWupdate_Parsing(CMD ,CSW->data,&data_num);
			CSW->Length = data_num + 8 ; //total length
			CSW->bCSWStatus = retval ; 
		break ;
		
		//NOT support
		default :
			;
	}

	OS_EXIT_CRITICAL();
		
	return retval ;
}


void ata_cbw()
{

     CMD_Parsing ( usbcb.CBW , usbcb.CSW ) ; //parsing 

	 //send CSW 
		
	usbcb.usb_ep_start_transfer(usbcb.CSW->Length, usbcb.CSW, is_IN);
    rlx_irq_set_mask(usbcb.irqnum, OS_FALSE);
    

}

void ata_csw()
{
    if(dpconf->ehcipatch && !dpconf->usbotg && usbcb.stall)
    {
        return;
    }

    usbcb.epstate = EP_CSW;
    usbcb.usb_ep_start_transfer( 13, (INT8U *) usbcb.CSW, is_IN );
    if(!dpconf->usbotg)
    {
        usbcb.epstate = EP_CBW;
        usbcb.usb_ep_start_transfer( 512, usbcb.CBW, is_OUT );
    }
}


INT8U IsValidCBW(INT32U epsize)
{
/*
	if ((usbcb.usbmps - epsize) != CBWLength)
        return 0;

    if (usbcb.CBW->dCBWSignature != CBWSIG)
        return 0;
*/
    return 1;

}

