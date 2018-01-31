/***************************************/
/*  Date: 2010.09.10                                      */
/*  Editor: Tom                                              */
/*  Reason: RTK EHCI Design                         */
/*  Firmware version: Draft                            */
/***************************************/
#include "usb.h"
#include "bsp.h"
#include "telnet.h"
#include <stdlib.h>
#include <string.h>

extern USBCB usbcb;
extern OS_EVENT *USBQ;
extern OS_EVENT *USBTxSem;

char kb_ready = 0;
char mouse_ready = 0;

int EHCI_RST();
INT32U Rd_IBIO(INT8U phyaddr);
INT8U CTRLDescOUTIndex,CTRLDescINIndex,BulkDescOUTIndex, BulkDescINIndex;
#if defined(CONFIG_EHCI_INTEP) && (CONFIG_VERSION >= IC_VERSION_FP_RevA)
INT8U INT1DescINIndex,INT2DescINIndex;

void ehci_init_hid()
{
    INT8U i;
    INT1DescINIndex=0;
    INT2DescINIndex=0;
    outindesc_r *intin1desc = (outindesc_r *)(INTIN1descStartAddr);
     for (i = 0; i < INTIN1desNumber; i++)
     {
        //prevent hardware overwrite each other
        if (i == INTIN1desNumber-1)
            intin1desc->eor  = 1;//last descriptor
        else
            intin1desc->eor  = 0;
        intin1desc->own  = 0;
        intin1desc++;
     } 
     REG32(INTINDESCADDR1) = VA2PA(INTIN1descStartAddr);     
     outindesc_r *intin2desc = (outindesc_r *)(INTIN2descStartAddr);
     for (i = 0; i < INTIN2desNumber; i++)
     {
        if (i == INTIN2desNumber-1)
            intin2desc->eor  = 1;//last descriptor
        else
            intin2desc->eor  = 0;
        
        intin2desc->own  = 0;
        intin2desc++;
     }
     REG32(INTINDESCADDR2) = VA2PA(INTIN2descStartAddr);

     REG32(TIMEOUTCFG)   = 0x40000000;//timeoutcfg, config offset0x2B: bit6 in polling not change(patch)
     REG32(ENDPOINT_REG) = 0x00000054;//offset 0x44 :keyboard ep4 and mouse ep5
     Wt_IBIO(HCSPARAMS,0x00000003);//there are 3 downstream port, for 3 port modify ,neil 
     	
}

void ehci_intep_enabled(INT8U portnum)
{
    ///INT32U tmp;
    //EHCI_RST();
    usb_dev_init(!usbcb.usbinit);
    rtkehci_init();

    if(portnum == 1){
    	Wt_IBIO(PORTSC+0x04, 0x1001);//port1
    }
    if(portnum == 2){
    	Wt_IBIO(PORTSC+0x08, 0x1001);//port2

    }
#ifdef CONFIG_DOORBELL_CHECK_ENABLED
		bsp_timer_reinit();
#endif
usbcb.portnum = portnum;	
REG8(DEVICE_ADDRESS + 1) = 0 ;
REG8(DEVICE_ADDRESS + 2) = 0 ;
}

void ehci_intep_disabled(INT8U portnum)
{
    if(portnum == 1)
    	Wt_IBIO(PORTSC+0x04, 0x1000);//port1
    if(portnum == 2)
    	Wt_IBIO(PORTSC+0x08, 0x1000);//port2

//REG8(DEVICE_ADDRESS + 1) = 0 ;//avoid to repeat wqtd
//REG8(DEVICE_ADDRESS + 2) = 0 ;//avoid to repeat wqtd    
//reset ehci to avoid cmdsts always 0x80 @usb disconnect
    //EHCI_RST();

#ifdef CONFIG_DOORBELL_CHECK_ENABLED
		bsp_timer_reinit();
#endif	    
    bsp_wait(1000);
}

void rtkehci_INTIN_start_transfer(INT8U len, INT8U *addr, INT8U is_in,INT8U portnum)//(INT8U portnum ,unsigned short len, unsigned int*addr, unsigned char is_in)
{
    INT8U *intinaddr;
    outindesc_r *intindesc ; 
	INT32U tcounter=0;

	if (portnum == 1)
	{
		intindesc = (outindesc_r *)(INTIN1descStartAddr)+INT1DescINIndex;
		INT1DescINIndex = ( INT1DescINIndex + 1 ) % INTIN1desNumber;
	}
	else if (portnum == 2)
	{
		intindesc = (outindesc_r *)(INTIN2descStartAddr)+INT2DescINIndex;
		INT2DescINIndex = ( INT2DescINIndex + 1 ) % INTIN2desNumber;
	}
#if 0	
	//descriptor is full  and not be moved to IB(not used) ,return !
	if((intindesc->own == 1) || ((REG32(CMDSTS)&(0x00000001<<portnum))!=0)){
		return -1;
	}
#endif
  
    if (is_in)
    {
          while(((REG32(CMDSTS)&(0x00000001<<portnum))!=0))
          	;
          
          intindesc->length = len;
          intindesc->ls = 1;
          intindesc->fs  = 0;//first segement
          
          if(usbcb.stall)
          {
            intindesc->stall = 1;//set stall
          }
          else
          {
            intindesc->bufaddr = (INT8U *) VA2PA(addr);
            intindesc->stall = 0;//clear stall
          }

          intindesc->own= 1; 
    //printk("[rtkehci_INTIN_start_transfer] intindesc= 0x%x , addr = 0x%x ===>\n\r", intindesc, intindesc->bufaddr);
          REG32(CMDSTS) |= (0x00000001<<portnum);

    }
#if 0
	return 0;
#endif
}
#endif

void ehci_usb_enabled()
{
    ///INT32U tmp;
    //EHCI_RST();
    usb_dev_init(!usbcb.usbinit);
    rtkehci_init();
    ///tmp = Rd_IBIO(PORTSC);
    ///Wt_IBIO(PORTSC, tmp | 0x00000001);
    Wt_IBIO(PORTSC, 0x1001);
#ifdef CONFIG_DOORBELL_CHECK_ENABLED
		bsp_timer_reinit();
#endif
usbcb.portnum = 0;	
REG8(DEVICE_ADDRESS) = 0; 
}

//#ifdef CONFIG_BUILDROM
void ehci_usb_disabled()
{
    ///INT32U tmp;
    //usbcb.portnum = 0;
    Wt_IBIO(PORTSC, 0x1000);
    ///tmp = Rd_IBIO(PORTSC);
    ///Wt_IBIO(PORTSC, tmp & 0xFFFFFFFE);
    //REG8(DEVICE_ADDRESS) = 0;  //avoid to repeat wqtd  
//reset ehci to avoid cmdsts always 0x80 @usb disconnect
    //EHCI_RST();

#ifdef CONFIG_DOORBELL_CHECK_ENABLED
		bsp_timer_reinit();
#endif	    
    bsp_wait(1000);
}
//#endif
//tomadd 2011.05.02
//EHCI Bulk IN transfer
void rtkehci_ep_start_transfer(INT32U len, INT8U *addr, INT8U is_in)
{
    outindesc_r *indesc = (outindesc_r *)(INdescStartAddr)+BulkDescINIndex;

    outindesc_r *outdesc= (outindesc_r *) (OUTdescStartAddr)+BulkDescOUTIndex;

#if 0
    tmp=len%OUTBUFLEN;
    if(tmp==0)
        j=len/OUTBUFLEN;
    else
        j=(len/OUTBUFLEN)+1;
#endif
#if 0
    if(len<=0x4000)
        j=1;
    else
        j=2;
#endif
    ///j=1;
    if (is_in)
    {
        //if already stalled, just return

        while((REG32(CMDSTS) & 0x00000080))
            ;

        indesc->length = len; // buffer size = 4kbytes
        indesc->ls  = 1;
        indesc->fs  = 0;//first segement

        if(usbcb.stall)
        {
            indesc->stall = 1;
        }
        else
        {
            indesc->bufaddr = (INT8U *) VA2PA(addr);
            indesc->stall = 0;
        }

        indesc->own = 1;
#if 0
        for (i = 0; i < j; i++)
        {
            //prevent hardware overwrite each other
            if (i == (j-1))
            {
                indesc->length = len-(i*OUTBUFLEN);
                indesc->length = len-(i*0x4600);
                indesc->ls  = 1;//last segment
                indesc->fs  = 0;
                indesc->bufaddr = (INT8U *) VA2PA(addr)+(0x4600*i);
            }
            else
            {
                indesc->length = len-0x0A00; // buffer size = 4kbytes
                indesc->ls  = 0;
                indesc->fs  = 1;//first segement
                indesc->bufaddr = (INT8U *) VA2PA(addr);
            }
            //indesc->stoi.length = 0x1000; // buffer size = 4kbytes
            //indesc->outin_buf_lowaddr = (INT8U *) VA2PA(inaddr) + i*OUTBUFLEN;
            //indesc->outin_buf_lowaddr = (INT8U *) VA2PA(inaddr);
            indesc->own = 1;
            indesc++;

        }
#endif
        //BulkDescINIndex= ( BulkDescINIndex+ 1 )% (INdescNumber) ;
        //outindesc_r *indesc = (outindesc_r *)(INdescStartAddr)+BulkDescINIndex;
        //indesc->stoi.length = len;
        //indesc->stoi.own= 1;
        //command status register
        //bit7: bulk in descriptor polling queue
        //tcounter=0;
#if 0
        while(((REG32(CMDSTS)&0x00000080)!=0)&& tcounter<1000)
        {
            tcounter++;
        }
#endif
        //change to next buffer for ping-pong
        usbcb.bufindex = (usbcb.bufindex + 1) % 2;
        usbcb.rbuf = usbcb.usbbuf[usbcb.bufindex];

        REG32(CMDSTS) |= 0x00000080;

        BulkDescINIndex = ( BulkDescINIndex + 1 ) % INdescNumber;


    }
    else
    {

        //while(!outdesc->own)
        //    OSTimeDly(1);
        outdesc->length = len;
        outdesc->bufaddr = (INT8U *) VA2PA(addr);
        outdesc->own = 1;
        REG32(EHCI_ISR) = 0x00000008;
        //BulkDescOUTCur= ( BulkDescOUTCur+ 1 )% (OUTdescNumber) ;
        //recycle_outdesc(outdesc);
    }
}


//tomadd 2011.04.29
//EHCI control IN transfer
void rtkehci_ep0_start_transfer(INT16U len, INT8U *addr, INT8U is_in)
{
    //INT8U *ctloutaddr,*ctlinaddr;
    outindesc_r *ctlindesc = (outindesc_r *)(CTLINdescStartAddr)+CTRLDescINIndex;
    CTRLDescINIndex = ( CTRLDescINIndex + 1 ) % CTLINdescNumber;
    ///INT32U tcounter=0;
    //ctlinaddr = VA2PA(addr);//(unsigned int)addr & ~Uncache_Mask;
    ctlindesc->bufaddr=(INT8U *) VA2PA(addr);

//printk("[rtkehci_ep0_start_transfer] addr = 0x%x, len = 0x%x ===>\r\n", ctlindesc->bufaddr, len); 
    if (is_in)
    {
#if 0
        while(((REG32(CMDSTS)&0x00000001)!=0) && tcounter<1000)
        {
            tcounter++;
        }
#endif

        while(((REG32(CMDSTS)&0x00000001)!=0))
            ;

        if(usbcb.stall == 1)
        {
            ctlindesc->stall = 1;//set stall
            usbcb.stall = 0;
        }
        else
            ctlindesc->stall = 0;//clear stall

        ctlindesc->ls = 1;//last segment
        ctlindesc->length = len;
        ctlindesc->own= 1;

        //command status register
        //bit0: control in descriptor polling queue

        REG32(CMDSTS) |= 0x00000001;

        while(((REG32(CMDSTS)&0x00000001)!=0))
            ;
        //since ehci would automatically response the ack,reset the status to default
        usbcb.ep0state = EP0_STATUS;
    
    }
    else{
        
        CTRLDescOUTIndex = ( CTRLDescOUTIndex + 1 ) % CTLOUTdescNumber; 
        outindesc_r *ctloutdesc = (outindesc_r *)(CTLOUTdescStartAddr)+CTRLDescOUTIndex;               
        ctloutdesc->length = 0x40;
        ctloutdesc->own = 1;
        REG32(EHCI_ISR) = 0x00000002;  
    	
    }

    //since ehci would automatically response the ack,reset the status to default
    //usbcb.ep0state = EP0_STATUS;
}

//tomadd 2011.05.02
//recycle control out descriptor
void recycle_ctloutdesc(outindesc_r *ctloutdesc)
{
    INT8U i = 0;
    if (!ctloutdesc->own)
    {

        while (i++ < CTLOUTdescNumber)
        {
            //recycle control out descriptor
            ctloutdesc->length = 0x40;
            ctloutdesc->own = 1;
            CTRLDescOUTIndex= ( CTRLDescOUTIndex+ 1 )% (CTLOUTdescNumber) ;
            ctloutdesc = (outindesc_r *) CTLOUTdescStartAddr + CTRLDescOUTIndex;
        }
        CTRLDescOUTIndex= ( CTRLDescOUTIndex+ 1 )% (CTLOUTdescNumber) ;
        ctloutdesc = (outindesc_r *) CTLOUTdescStartAddr + CTRLDescOUTIndex;
    }

}

//tomadd 2011.05.02
//recycle bulk out descriptor

#if 0
void recycle_outdesc(outindesc_r *outdesc)
{
    INT8U i = 0;
    if (!outdesc->own)
    {

#if 0
        while (i++ < OUTdescNumber)
        {
            //recycle out descriptor
            outdesc->length = OUTBUFLEN;
            outdesc->ls = 0;
            outdesc->fs = 0;
            outdesc->own = 1;
            //#ifdef bulk_in_test
            //addr=(INT8U *)(BulkOUTADDR);
            //#endif
            //outdesc->outin_buf_lowaddr = (INT8U *) VA2PA(addr) +i* (outdesc->stoi.length);
            BulkDescOUTIndex= ( BulkDescOUTIndex+ 1 )% (OUTdescNumber) ;
            outdesc = (outindesc_r *) OUTdescStartAddr + BulkDescOUTIndex;
        }
#else
        outdesc->length = 512;
        //outdesc->ls = 0;
        //outdesc->fs = 0;
        //default use CBW
        outdesc->bufaddr = (INT8U *) VA2PA(usbcb.CBW);
        outdesc->own = 1;
#endif
        BulkDescOUTIndex= ( BulkDescOUTIndex+ 1 )% (OUTdescNumber) ;
        outdesc = (outindesc_r *) OUTdescStartAddr + BulkDescOUTIndex;

        //clear the next descriptor since it may be out data
#if 0
        if(usbcb.epstate == EP_DATA_OUT)
            outdesc->own = 0;
        else
        {
            outdesc->bufaddr = (INT8U *) VA2PA(usbcb.CBW);
            outdesc->own = 1;
        }
#endif

        //BulkDescOUTCur= ( BulkDescOUTCur+ 1 )% (OUTdescNumber) ;
    }

}
#endif

//***************************************************//
//  FW ACCESS IB Channel IO or Configuration Space   //
//  Build Date: 2011.04.28                           //
//  Editor: Tom                                      //
//***************************************************//

// tomadd 2011.04.28
// OOB reads IB IO

INT32U Rd_IBIO(INT8U phyaddr)
{
    INT32U  temp_count;
    INT8U  offset;
    //default error code
    INT32U  Rd_Data = 0xDEADBEAF;
    offset = phyaddr;

    REG32(OOBACTADDR) = 0x0000F000|offset;
    temp_count=0;
    while((REG32(OOBACTADDR)>>31)==0 && temp_count<1000)
    {
        Rd_Data=REG32(OOBACTDATA);
        temp_count++;

    };

    return REG32(OOBACTDATA);
}
//tomadd 2011.04.28
//OOB writes IB IO
void Wt_IBIO(INT8U phyaddr,INT32U Wt_Data)
{

    INT32U  temp_count;
    INT8U offset;
    offset = phyaddr;
    REG32(OOBACTDATA)=Wt_Data;
    REG32(OOBACTADDR)=0x8000F000|offset;
    temp_count=0;
    while((REG32(OOBACTADDR)>>31)==1 && temp_count<1000)
    {
        temp_count++;
    };

}
// tomadd 2011.04.28
// OOB reads IB PCIE configuration space
#if 0
int Rd_PCIECS(INT8U phyaddr)
{
    INT8U  temp_count;
    INT8U  offset;
    INT32U  Rd_Data;
    offset = phyaddr;

    REG32(DBIACTADDR) = 0x0000F000|offset;
    temp_count=0;
    while((REG32(DBIACTADDR)>>31)==0&&temp_count<1000)
    {
        Rd_Data=REG32(DBIACTDATA);
        temp_count++;

    };

    return Rd_Data;
}
#endif
//tomadd 2011.04.28
//OOB writes IB PCIE configuration space
void Wt_PCIECS(INT8U phyaddr,INT32U Wt_Data)
{

    INT8U  temp_count;
    INT8U offset;
    offset = phyaddr;
    REG32(DBIACTDATA)=Wt_Data;
    REG32(DBIACTADDR)=0x8000F000|offset;
    temp_count=0;
    while((REG32(DBIACTADDR)>>31)==1&&temp_count<1000)
    {
        temp_count++;
    };

}


//*************************************************************//
//  Introduce RTKEHCI_INT_HANDLER(),                           //
//  1. OUT transaction done                                    //
//       [1] Parse USB2.0 Chapter 9 packet                     //
//            (1) Call IN transaction response data            //
//       [2] Parse CBW/UFI command                             //
//            (1) Call IN transaction response data or CSW     //
//            (2) Response CSW                                 //
//  2. OUT tranfer done                                        //
//  Build Date: 2010.09.10                                     //
//  Editor: Tom                                                //
//  Modified Date: 2010.11.05                                  //
//*************************************************************//
void rtkehci_int_handler()
{
    INT32U OUTSTS;
    INT32U packetlen;
    INT32U D_tmp;
    QHdr *txq;

    //bulk out descriptor address
    outindesc_r *outdesc;
    outdesc = (outindesc_r *) (OUTdescStartAddr)+BulkDescOUTIndex;
    //control out descriptor address
    outindesc_r *ctloutdesc;
    ctloutdesc = (outindesc_r *)(CTLOUTdescStartAddr)+CTRLDescOUTIndex;
    //bulk in descriptor address
    outindesc_r *indesc;
    indesc = (outindesc_r *) (INdescStartAddr);
    //control in descriptor address
    outindesc_r *ctlindesc;
    ctlindesc = (outindesc_r *)(CTLINdescStartAddr);

    //disable IMR
    REG32(EHCI_IMR) = 0x00000000;
    OUTSTS = REG32(EHCI_ISR);

    //clear ISR
    REG32(EHCI_ISR)= (OUTSTS & 0xFFFFFFF7);

    //tomadd 2011.04.28
    //host controller reset done interrupt
    if(OUTSTS & 0x00000080)
    {
        //tomadd 2011.08.23
        //reset setup phase address counter
        ///setup_count=0;
        //tomadd 2011.09.30
        //debug for remove virtual device issue
        //disable ASE
        ///D_tmp=Rd_IBIO(USBCMD);
        ///Wt_IBIO(PORTSC,D_tmp& 0xFFFFFFDF);

        //port disconnection
        //D_tmp=Rd_IBIO(PORTSC);
        //Wt_IBIO(PORTSC,D_tmp & 0xFFFFFFFE);

        if(usbcb.usbstate != DISABLED && usbcb.usbstate != DISCONNECTED)
            Wt_IBIO(PORTSC, 0x1000);

        //EHCI_RST();
        rtkehci_init();
        //usbcb.portnum = 0;
        //REG32(EHCI_ISR)=REG32(EHCI_ISR)&0x00000080;

        if(usbcb.usbstate != DISABLED && usbcb.usbstate != DISCONNECTED )
            Wt_IBIO(PORTSC, 0x1001);
        ///D_tmp=Rd_IBIO(PORTSC);
        // FW writes IB EHCI port connection bit=1
        //Wt_IBIO(PORTSC,D_tmp|0x00000001);
        //tomadd 2011.09.22
        //for BIOS test
        //Wt_IBIO(CONFIGFLAG,0x00000001);

    }

    //tomadd 2011.04.28
    //host port reset interrupt
    if(OUTSTS & 0x00000200)
    {
        //REG32(EHCI_ISR)=REG32(EHCI_ISR)&0x00000200;
        //D_tmp=REG32(EHCICONFIG);
        //tom mark 2011.07.19
        //patch ring counter return 0
        // FW writes EHCI OOB Enable bit=0
        //ryan test
        // REG32(EHCICONFIG)=D_tmp&0xFFFEFFFF;
        //tomadd 2011.09.22
        //for BIOS test
        //Wt_IBIO(CONFIGFLAG,0x00000001);
        //D_tmp=Rd_IBIO(PORTSC);
        //Wt_IBIO(PORTSC,D_tmp & 0xFFFFDFFF);

    }

    //tomadd 2011.04.28
    //host port reset done interrupt
    if(OUTSTS & 0x00000100)
    {
        //REG32(EHCI_ISR)=REG32(EHCI_ISR)&0x00000100;
        D_tmp=REG32(EHCICONFIG);
        // FW writes EHCI OOB Enable bit=1
        //REG32(EHCICONFIG)=D_tmp|0x00010000;
        //rtkehci_init();
    }

    // tomadd 2011.05.02
    //Bit0 =1, setup token
    if(OUTSTS & 0x00000001)
    {

//printk("\n[ehci_handler](1)PORTSC = 0x%x,  PORTSC+4 = 0x%x, own =0x%x \r\n", Rd_IBIO(PORTSC),Rd_IBIO(PORTSC+4), ctloutdesc->own);
        //REG32(EHCI_ISR)=REG32(EHCI_ISR)&0x00000001;
        // Firmware needs to check out descriptor own bit = 0 or 1
        if(!(ctloutdesc->own))
        {
//printk("[ehci_handler](2)DEVICE_ADDRESS = 0x%x, devaddr = 0x%x \r\n", REG32(DEVICE_ADDRESS), ctloutdesc->devaddr );
            #if 0
            if(REG32(DEVICE_ADDRESS) != 0)
            {
            	if(ctloutdesc->devaddr == REG8(DEVICE_ADDRESS+1))
            		usbcb.usbtype = USB_HID;
            	if(ctloutdesc->devaddr == REG8(DEVICE_ADDRESS))
            		usbcb.usbtype = USB_MSD;
            
            }
            #endif
            setup_phase((struct usb_ctrlrequest *)PA2VA(ctloutdesc->bufaddr));
            // recycle outdescriptor
            recycle_ctloutdesc(ctloutdesc);
        }
    }

    // tom modified 2010.12.14
    //Bit1 =1,  control out
#if 0
    if(OUTSTS & 0x00000002)
    {

        //REG32(EHCI_ISR)=REG32(EHCI_ISR)&0x00000002;
        // Firmware needs to check out descriptor own bit = 0 or 1
        if(!(ctloutdesc->own))
        {
            //packetlen = 0x40-ctloutdesc->stoi.length;
            // or call IN transaction function.
            //if(packetlen == 0x00)
            //	setup_phase();//parsing usb2.0 chap 9 pattern...

            // recycle outdescriptor
            //ctloutdesc->stoi.length = 0x40; // write back to total bytes to transfer
            //ctloutdesc->stoi.own = 1;             // wrtie back to ownbit
            handle_ep0();
            recycle_ctloutdesc(ctloutdesc);
        }
    }
#endif

    // tom modified 2010.12.14
    //Bit4 =1, bulk out transaction
    if(OUTSTS & 0x00000010)
    {

        //REG32(EHCI_ISR)=REG32(EHCI_ISR)&0x00000010;
        // Firmware needs to check out descriptor own bit = 0 or 1
        if(!(outdesc->own))
        {
            BulkDescOUTIndex= ( BulkDescOUTIndex+ 1 )% (OUTdescNumber) ;
            packetlen = outdesc->length;
            if ( usbcb.epstate == EP_CBW ) {

                REG32(EHCI_ISR) = 0x00000008;

                if ( IsValidCBW(packetlen) && IsMeaningfulCBW()) {
                    txq = malloc(sizeof(QHdr));
                    memset(txq, 0 , sizeof(QHdr));
                    txq->cmd  = USB_CBW;
                    txq->length = usbcb.CBW->dCBWDataTransferLength;
                    rlx_irq_set_mask(usbcb.irqnum, OS_TRUE);
                    OSQPost(USBQ, txq);
                }
#if 0
                else if (!IsValidCBW(packetlen))
                {
                    ep_set_stall(1, is_IN);
                    usbcb.stall = 1;
                }
                else
                {
                    usbcb.CSW->bCSWStatus = 1;
                    usbcb.CSW->dCSWTag = usbcb.CBW->dCBWTag;
                    txq = malloc(sizeof(QHdr));
                    memset(txq, 0 , sizeof(QHdr));
                    txq->cmd  = USB_STATUS_ERROR;
                    rlx_irq_set_mask(usbcb.irqnum, OS_TRUE);
                    OSQPost(USBQ, txq);
                }
#endif
                //BulkDescOUTCur= ( BulkDescOUTCur+ 1 )% (OUTdescNumber) ;
            }
            else if (usbcb.epstate == EP_DATA_OUT)
            {
                rlx_irq_set_mask(usbcb.irqnum, OS_TRUE);
                OSSemPost(USBTxSem);
            }
#if 0
            else if ( usbcb.epstate == EP_DATA_OUT ) {
                if (usbcb.protocol == USB_BULK)
                {
                    txq = malloc(sizeof(QHdr));
                    memset(txq, 0 , sizeof(QHdr));
                    txq->cmd  = USB_EP_OUT_COMPLETE;
                    rlx_irq_set_mask(usbcb.irqnum, OS_TRUE);
                    OSQPost(USBQ, txq);
                }
                else
                    usbcb.epstate = EP_CBW;


            }
            else if ( usbcb.epstate == EP_CSW ) {
                usbcb.epstate = EP_CBW;
                //dwc_otg_ep_start_transfer(CBWLength, (INT8U *) usbcb.CBW, is_OUT);
                rtkehci_ep_start_transfer(CBWLength,(INT8U*)usbcb.CBW,is_OUT);
            }
#endif

            //recycle_outdesc(outdesc);
        }

    }
    // tom modified 2010.12.15
    //Bit2: control out descriptor unavailable
    //Bit3: bulk out descriptor unavailable
    //Bit5: bulk in descriptor unavailable
    //Bit6: control in descriptor unavailable
    if(OUTSTS & 0x00000004)
    {

        //REG32(EHCI_ISR)=REG32(EHCI_ISR)&0x00000004;
        //process control out descriptor unavailable
    }
#if 0
    if(OUTSTS & 0x00000008)
    {
        //REG32(EHCI_ISR)=REG32(EHCI_ISR)&0x00000008;
        //process bulk out descriptor unavailable
        recycle_outdesc(outdesc);
    }
#endif
    if(OUTSTS & 0x00000020)
    {
        //REG32(EHCI_ISR)=REG32(EHCI_ISR)&0x00000020;
        //process bulk in descriptor unavailable
    }
    if(OUTSTS & 0x00000040)
    {
        //REG32(EHCI_ISR)=REG32(EHCI_ISR)&0x00000040;
        //process control in descriptor unavailable
    }
#ifdef CONFIG_EHCI_INTEP
//can not disable keyboard and mouse IMR
//must response
    if(OUTSTS & 0x00010000)//keyboard
    {
    	//interrupt tx fifo empty
    	//printk("[ehci_handler] keyboard %x usbcb.tmpbuf\r\n", &usbcb.tmpbuf);
    	//keyboard ready
    	#if 1
    	memset(usbcb.tmpbuf, 0, 16);
    	rtkehci_INTIN_start_transfer(8 , usbcb.tmpbuf, is_IN, 1);
    	//kb_ready = 1;
    	#endif   	
    }
    if(OUTSTS & 0x00020000)//mouse
    {
    	//interrupt tx fifo empty
    	//printk("[ehci_handler] mouse \r\n");
    	//mouse ready

	//mouse_ready =1 ;
    	#if 0
    	memset(usbcb.tmpbuf, 0, 16);
    	usbcb.tmpbuf[0] = 1;
    	usbcb.tmpbuf[2] = -1;
    	usbcb.tmpbuf[3] = -1;
    	
    	rtkehci_INTIN_start_transfer(5 , usbcb.tmpbuf, is_IN, 2);
    	#endif
    }
#endif
    //tom modified 2011.09.18
    //Bit31 IB: IN transfer timeout
    //clear ISR
    //REG32(EHCI_ISR)=OUTSTS;
    //enable IMR
#ifdef CONFIG_EHCI_INTEP
    REG32(EHCI_IMR) = 0xE00303F7;//0x600003FF;
#else
    REG32(EHCI_IMR) = 0x600003FF;
#endif
}

//********************************************************//
//  Introduce RTKEHCI_INIT(),                             //
//  1. Tune buffer size : 128bytes ~ 20kbytes             //
//  2. Prepare out and in descriptor                      //
//  3. Enable Out In transaction state mechine            //
//  Build Date: 2010.09.10                                //
//  Editor: Tom                                           //
//********************************************************//
void rtkehci_init(void)
{
    INT8U i;
    //INT32U tmp;

    //Reset Index
    CTRLDescOUTIndex=0;
    CTRLDescINIndex=0;
    BulkDescOUTIndex=0;
    //BulkDescOUTCur = 0;
    BulkDescINIndex=0;

    //allocate buffer before usage
    if(usbcb.usbstate == DISABLED || usbcb.usbstate == DISCONNECTED)
        usb_dev_init(!usbcb.usbinit);

    outindesc_r *ctloutdesc = (outindesc_r *)(CTLOUTdescStartAddr);
    outindesc_r *ctlindesc = (outindesc_r *)(CTLINdescStartAddr);
    outindesc_r *outdesc = (outindesc_r *)(OUTdescStartAddr);
    outindesc_r *indesc = (outindesc_r *)(INdescStartAddr);

    //for FPGA test(patch hardware bug)
    Wt_IBIO(HCSPARAMS,0x00000001);
    //Wt_IBIO(PORTSC,D_tmp|0x00000001);
    REG32(EHCICONFIG)=0x00000000;
    /* step1. tune out and in buffer size */
    REG32(EHCICONFIG) = 0x0002001B;

    /* step2. */
    //(1)prepare control/bulk out/bulk in descriptors
    // tomadd 2010.11.18
    //  [1]  for control out descriptor, CTLOUTdescNumber=4
    for(i=0; i< CTLOUTdescNumber; i++)
    {
        ctloutdesc->length = 0x40;
        ctloutdesc->bufaddr = (INT8U *) VA2PA(usbcb.setup_pkt)+ i* sizeof(*usbcb.setup_pkt);
        if(i==CTLOUTdescNumber-1)
            ctloutdesc ->eor = 1;
        else
            ctloutdesc ->eor = 0;

        ctloutdesc->own = 1;
        ctloutdesc++;
    }

    // tomadd 2010.12.14
    //  [2]  for control in descriptor, CTLINdescNumber=1
    for(i=0; i< CTLINdescNumber; i++)
    {
        //only set the eor
        if(i==CTLINdescNumber-1)
            ctlindesc ->eor = 1;
        else
            ctlindesc ->eor = 0;
        ctlindesc++;
    }
    //  [3] for bulk out descriptors, OUTdescNumber=4
    for(i=0; i< OUTdescNumber; i++)
    {
        outdesc->length = 512;

        //default assign CBW as  bulk out buffer
        outdesc->bufaddr = (INT8U *) VA2PA(usbcb.CBW);
        if(i==OUTdescNumber-1)
            outdesc ->eor = 1;//last descriptor
        else
            outdesc ->eor = 0;

        if(i == 0)
            outdesc->own = 1;
        outdesc++;
    }
    //  [4] for bulk IN descriptors, INdescNumber=4
    for (i = 0; i < INdescNumber; i++)
    {
        //only set the eor
        if (i == INdescNumber-1)
            indesc->eor  = 1;
        else
            indesc->eor  = 0;
        indesc++;
    }
    //(2)assign out descriptor address to register
    REG32(OUTDesc_Addr) = VA2PA(OUTdescStartAddr);
    //(3)assign in descriptor address to register
    REG32(INDesc_Addr) = VA2PA(INdescStartAddr);
    //(4)assign control out descriptor address to register
    REG32(CTLOUTDesc_Addr) = VA2PA(CTLOUTdescStartAddr);
    //(5)assign control in descriptor address to register
    REG32(CTLINDesc_Addr) = VA2PA(CTLINdescStartAddr);

    //tmp = Rd_IBIO(0x08);
    ///Wt_IBIO(0x8, 0xE400);

    /* step3. */
    //enable OUT interrupt mask register
    //tom modified 2011.09.18
    //Bit31 IB: IN transfer timeout
#ifdef CONFIG_EHCI_INTEP
    ehci_init_hid();
    REG32(EHCI_IMR) = 0xE00303F7;//0x600003FF;
#else
    REG32(EHCI_IMR) = 0x600003FF;
#endif
    /* step4. */
    // tom modified 2010.12.24
    // enable OUT transaction state mechine
    REG32(EHCICONFIG) = REG32(EHCICONFIG)|0x00010000;

//    set_usb_hs();
usbcb.usbmps = 512;

    if (usbcb.usbinit == 0)
    {
        rlx_irq_register(BSP_EHCI_IRQ, rtkehci_int_handler);
        usbcb.usbinit = 1;
    }
    else
        rlx_irq_set_mask(BSP_EHCI_IRQ, OS_FALSE);
}

// tomadd 2010.11.15
// ehci engine reset function
int EHCI_RST()
{
    INT32U ehcirst,count;

    count=0;
    ehcirst = 0x00800000;
    REG32(EHCICONFIG) = ehcirst;
    do
    {
        REG32(EHCICONFIG)=ehcirst;
        ehcirst = REG32(EHCICONFIG);

        if (++count > 1000)
        {
            return 1;
        }
    }
    while ( !(ehcirst & 0x00800000) );	/* ehci firmware reset */

    return 0;
}

#if (CONFIG_VERSION  >= IC_VERSION_EP_RevB)
//patch tag conflict issue
void ehci_patch(void)
{
        INT32U bak_1 = 0;
        INT32U bak_2 = 0;
        INT32U bak_3 = 0;
        INT32U bak_4 = 0;
        INT32U bak_5 = 0;

  bak_1=Rd_IBIO(0x20);
  bak_5=Rd_IBIO(0x24);
  bak_2=Rd_IBIO(0x34);
  bak_3=Rd_IBIO(0x38);
  bak_4=Rd_IBIO(0x28);

  //reset
  Wt_IBIO(0x20, 0x02);

  //bsp_wait(100000);
  //Wt_IBIO_test(0x20, bak_1);
  Wt_IBIO(0x60, 0x01);

  Wt_IBIO(PORTSC, 0x1001);
  Wt_IBIO(PORTSC, 0x1003);
  Wt_IBIO(0x24, 0x04);
  Wt_IBIO(PORTSC, 0x1101);
  Wt_IBIO(PORTSC, 0x1001);

  Wt_IBIO(0x24, 0x04);

  Wt_IBIO(PORTSC+0x04, 0x1001);
  Wt_IBIO(PORTSC, 0x1003);
  Wt_IBIO(0x24, 0x04);
  Wt_IBIO(PORTSC+0x04, 0x1101);
  Wt_IBIO(PORTSC+0x04, 0x1001);

  Wt_IBIO(0x24, 0x04);

  Wt_IBIO(0x34, bak_2);
  Wt_IBIO(0x38, bak_3);
  Wt_IBIO(0x28, bak_4);
  Wt_IBIO(0x20, bak_1);  
}
#endif

void enable_vnc_usb_dev()
{
	usbcb.usbstate = LISTENED;
	//usbcs = rtSktAccept_F(usbSrv);
	usbcb.usbstate = ACCEPTED;
	
	//usbcb.usbtype = rxq->option;
	//usbcb.sectorsize = usb_sector_size[usbcb.usbtype];
	usbcb.waitRx = 0;
	usbcb.usbmode = SERVER_MODE;
	kb_ready = 0;
	mouse_ready = 0;

	//usbcb.bsp_usb_enabled();
	usbcb.usbstate = ENABLED;//+Tom 20131105
	#ifdef CONFIG_EHCI_INTEP
	ehci_intep_enabled(1);
	#endif
	printf("enabled\n");		
} 

void disable_vnc_usb_dev()
{
	#ifdef CONFIG_EHCI_INTEP
	ehci_intep_disabled(1);
	#endif
	//usbcb.usbstate = DISABLED;//avoid to fail disconnect
	printf("disabled\n");	
}