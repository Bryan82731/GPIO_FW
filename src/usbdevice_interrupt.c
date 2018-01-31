#include <stdlib.h>
#include <string.h>
#include "usb.h"
#include "bsp.h"
#include "lib.h"
#include "telnet.h"


#ifdef CONFIG_USBLOG_DEBUG
INT8U  usbx[USB_ENTRIES][USB_UNIT];
INT8U  usbidx = 0;
INT8U  *dbgptr;
INT32U dbglen;
INT32U global_error=0;
#endif

extern OS_EVENT *USBQ;
extern USBCB usbcb;
extern INT16U usb_sector_size[3];
extern DPCONF *dpconf;
extern char kb_ready;
extern char mouse_ready ;

void otg_usb_enabled(void)
{
	REG32(USB_OTG_BASE + GAHBCFG) |= (GAHBCFG_GlblIntrMsk);

	rlx_irq_set_mask(BSP_USB_IRQ, OS_FALSE);

	REG32(USB_OTG_BASE + DCTL) &= ~(DCTL_SftDiscon);//SftDiscon = 0 ,normal operation
}

void otg_usb_disabled(void)
{
	REG32(USB_OTG_BASE + DCTL) |= DCTL_SftDiscon; //SftDiscon = 1

	rlx_irq_set_mask(BSP_USB_IRQ, OS_TRUE);
	
	REG32(USB_OTG_BASE + GAHBCFG) &= ~(GAHBCFG_GlblIntrMsk);

	//at most 3ms + 2.5 us
	OSTimeDly(4);
}

extern void CPU_Sleep(void);

void device_int_suspend()
{
	static INT32U suspend_cnt=0, test_suspend;
	suspend_cnt++;
	test_suspend++;
}

void device_int_resume()
{
	static INT32U resume_cnt=0;
	resume_cnt++;
}

static INT32U sof_cnt = 0, rxnonempty_cnt = 0, txempty_cnt = 0;

void bsp_usb_handler(void)
{
	volatile INT32U gintmsk = REG32(USB_OTG_BASE + GINTMSK);
	volatile INT32U gintsts = REG32(USB_OTG_BASE + GINTSTS);
	volatile INT32U gint_status = gintmsk & gintsts;
	volatile INT32U gotgint_sts;

	REG32(USB_OTG_BASE +  GINTSTS) = gintsts;

	if(gint_status == 0)
	{
		return;
	}
	
	if(gint_status & GINTSTS_OTG_INT)  	/* ogt interrupt */
	{
		gotgint_sts=REG32(USB_OTG_BASE + GOTGINT);
		//clear the interrupt status
		REG32(USB_OTG_BASE + GOTGINT) |= gotgint_sts;
		
		//check device is removed from USB Host
		if( gotgint_sts & GOTGINT_SesEndDet)
		{			
			/*ToDo*/
		}
	}
	if(gint_status & GINTSTS_USB_DISCONNECT)
	{

	}
	if(gint_status & GINTSTS_USB_SESSREQ)
	{

	}

	if(gint_status & GINTSTS_USB_RESET)  	/* usb reset */
	{
		usb_dev_reset();
	}

	if(gint_status & GINTSTS_ENUM_DONE)  	/* enumeration done */
	{
		device_int_enum_done();
	}

	if(gint_status & GINTSTS_OUT_EP_INT)  	/* out endpoints interrupt mask */
	{
		//Clear Nak Status..//
		device_int_out_ep();
	}

	if(gint_status & GINTSTS_IN_EP_INT)
	{
		device_int_in_ep();
	}

	if(gint_status & GINTSTS_USB_SUSPEND)
	{
		device_int_suspend();
	}

	if(gint_status & GINTSTS_USB_RESUME)
	{
		device_int_resume();
	}

	if(gint_status & GINTSTS_SOF)
	{
		//recieve every 1ms in HS
		sof_cnt++;
		sof_cnt %= 1000;
	}

	if(gint_status & GINTSTS_RXFIFO_NONEMPTY)
	{
		//bossino, should not enter here in DMA mode!?
		//check remain RX fifo & flush
		rxnonempty_cnt++;
	}
	if(gint_status & GINTSTS_NP_TXFIFO_EMPTY)
	{
		//bossino, should not enter here in DMA mode!?
		txempty_cnt++;
	}
}

void core_reset()
{
	INT32U greset;
	INT32U count = 0;
	do
	{
		bsp_wait(10);
		greset =  REG32(USB_OTG_BASE + GRSTCTL) ;

		//AHB master is not idle,
		if(++count > 100000)
		{
			#ifdef CONFIG_USBLOG_DEBUG
			global_error = 3;//AHB idle error
			#endif
			//return ;
			while(1);
		}
	}
	while(!(greset & GRSTCTL_AHBIdle));	/* ahbidle */

	count = 0;
	greset |= GRSTCTL_CSftRst;	/* core soft reset */
	REG32(USB_OTG_BASE + GRSTCTL) =  greset ;
	do
	{
		bsp_wait(10);
		greset = REG32(USB_OTG_BASE + GRSTCTL);
		//soft reset fail
		if(++count > 10000)
		{
			#ifdef CONFIG_USBLOG_DEBUG
			global_error = 4;//GRSTCTL error
			#endif
			return;
		}
	}
	while(greset & GRSTCTL_CSftRst);	/* csftrst */

	wait_AHB_idle();
	bsp_wait(1000000);
	//wait 3 phy clocks, 100 ms
}

void otg_usb_init()
{
	REG32(USB_OTG_BASE + GAHBCFG) &= ~(GAHBCFG_HBstLen);
	
	if(usbcb.usbinit == 0)
		core_reset();

	REG32(USB_OTG_BASE + GAHBCFG) |= GAHBCFG_DMAEn | HBstLen_256W;
	/* config gahbcfg(dma, ahb burst length) */

	REG32(USB_OTG_BASE + GUSBCFG) = GUSBCFG_ForceDevMode | USBTrdTim8B | GUSBCFG_PHYSel | GUSBCFG_PHYIf16B;
	/* default, disable hnp and srp and select utmi..in gusbcfg */

	//REG32(USB_OTG_BASE + GAHBCFG) |= GAHBCFG_GlblIntrMsk;  /* enable global interrupt */

	usb_dev_init(!usbcb.usbinit);

	if(usbcb.usbinit == 0)
	{
		rlx_irq_register(BSP_USB_IRQ, bsp_usb_handler);
		usbcb.usbinit = 1;
	}
}


int Rd_UPHY(INT8U phyaddr)
{
	INT8U  temp_count;
	INT8U  offset;
	INT8U  Rd_Data;

	offset = phyaddr & 0x0f ;

	REG8(CPU_BASE + VCONTROL) = 0x90 | offset;
	// vloadm# need pull low ~ 99ns
	for(temp_count = 0; temp_count < 100; temp_count++)
		REG8(CPU_BASE + VCONTROL) = 0x80 | offset;

	REG8(CPU_BASE + VCONTROL) = 0x90 | offset;

	offset = (phyaddr & 0xf0) >> 4;
	REG8(CPU_BASE + VCONTROL) = 0x90 | offset;
	// vloadm# need pull low ~ 99ns
	for(temp_count = 0; temp_count < 100; temp_count++)
		REG8(CPU_BASE + VCONTROL) = 0x80 | offset;

	REG8(CPU_BASE + VCONTROL) = 0x90 | offset;

	//Read Register Value
	Rd_Data = REG8(CPU_BASE + UPHY_VSTATUS);

	return Rd_Data;
}
//usb phy register 0xE0~0xE7 (write)
//usb phy register 0xF0~0xF3 (write)
void Wt_UPHY(INT8U phyaddr, INT8U Wt_Data)
{
	INT8U offset;

	// Input Data to usb phy register //
	REG8(CPU_BASE + VSTATUS) = Wt_Data;

	offset = phyaddr & 0x0f ;

	REG8(CPU_BASE + VCONTROL) = 0x90 | offset;
	// vloadm# need pull low ~ 99ns
	//for (temp_count=0; temp_count<100; temp_count++)
	REG8(CPU_BASE + VCONTROL) = 0x80 | offset;

	bsp_wait(1);

	REG8(CPU_BASE + VCONTROL) = 0x90 | offset;

	offset = (phyaddr & 0xf0) >> 4;
	REG8(CPU_BASE + VCONTROL) = 0x90 | offset;
	// vloadm# need pull low ~ 99ns
	//for (temp_count=0; temp_count<100; temp_count++)
	REG8(CPU_BASE + VCONTROL) = 0x80 | offset;

	bsp_wait(1);

	REG8(CPU_BASE + VCONTROL) = 0x90 | offset;

	bsp_wait(100);
}


void dwc_otg_flush_tx_fifo(INT32U epnum)
{
	INT32U greset = GRSTCTL_TxFFlsh;
	INT32U count = 0;

	greset |= ((epnum << 6) & GRSTCTL_TxFNum);

	REG32(USB_OTG_BASE + GRSTCTL) =  greset ;
	do
	{
		greset = REG32(USB_OTG_BASE + GRSTCTL) ;
		bsp_wait(1);
		if(++count > 10000)
		{
			#ifdef CONFIG_USBLOG_DEBUG
			global_error = 1;//dwc_otg_flush_tx_fifo error
			#endif
			break;
		}
	}
	while(greset & GRSTCTL_TxFFlsh);
}

void dwc_otg_flush_rx_fifo(INT32U epnum)
{
	INT32U greset = GRSTCTL_RxFFlsh;
	INT32U count = 0;

	greset |= ((epnum << 6) & GRSTCTL_TxFNum);

	REG32(USB_OTG_BASE + GRSTCTL) =  greset ;
	do
	{
		greset = REG32(USB_OTG_BASE + GRSTCTL) ;
		if(++count > 10000)
		{
			#ifdef CONFIG_USBLOG_DEBUG
			global_error = 2;//dwc_otg_flush_rx_fifo error
			#endif
			break;
		}
	}
	while(greset & GRSTCTL_RxFFlsh);
}

void usb_dev_init(INT8U hwreset)
{
	void *p ;
	if(hwreset)
	{

		p = malloc(BULK_SIZE);
		if (p!=NULL)	usbcb.usbbuf[0] = p ;
		else  goto Error1 ;
		p =  malloc(BULK_SIZE);
		if (p!=NULL)	usbcb.usbbuf[1] = p ;
		else  goto Error2 ;
	
		//need (4*n)+6 Double words
		p = malloc(sizeof(*usbcb.setup_pkt) * 9);
		if (p!=NULL)	usbcb.setup_pkt = p ;
		else  goto Error3 ;
		memset((void *) usbcb.setup_pkt, 0, sizeof(*usbcb.setup_pkt) * 9);

		
		p = malloc(512 * 4); // the max length of CMD is 512 , *4 ? neil 
		if (p!=NULL)	usbcb.CBW = p ;
		else  goto Error4 ;
		memset((void *) usbcb.CBW, 0, 512 * 4);

		p = malloc(512 ); // // the max length of STATUS is 512 , neil 
		if (p!=NULL)	usbcb.CSW = p ;
		else  goto Error5 ; 
		memset((void *) usbcb.CSW, 0, 512);

		//*((INT8U *) (USBDescStartAddr+0xD0)) = 0; //LUN
		usb_desc_init();

		//for keeping
		p = malloc(512 ); // // the max length of STATUS is 512 , neil 
		if (p!=NULL)	usbcb.MBR = p ;
		else  goto Error6 ; 

		p = malloc(64 ); // // the max length of STATUS is 512 , neil 
		if (p!=NULL)	usbcb.cbi = p ;
		else  goto Error7; 

#if defined(CONFIG_EHCI_INTEP)
		//&& defined(CONFIG_USB_HID)
		//for HID
		usbcb.portnum = 0;
		usbcb.tmpbuf = malloc(16);
		memset(usbcb.tmpbuf, 0, 16);
#endif
	}

	usbcb.bufindex = 0;
	usbcb.rbuf = usbcb.usbbuf[usbcb.bufindex];
	usbcb.ep0state = EP0_DISCONNECT;
	usbcb.epstate = EP_CBW;
	usbcb.usbstate = DISCONNECTED;

#ifdef CONFIG_USB_FLASH
	usbcb.usbtype = USB_MSD;
	usbcb.sectorsize = usb_sector_size[usbcb.usbtype];
#endif
	//reset variables
	usbcb.clearfeature = 0;
	usbcb.protocol = USB_BULK;

	if(dpconf->usbotg)
	{
		REG32(USB_OTG_BASE + DCFG) &= ~(DCFG_DevSpd);/* high speed */

		//2.0 full speed
		//REG32(USB_OTG_BASE + DCFG) |= 0x01;

		dwc_otg_flush_tx_fifo(0);
		dwc_otg_flush_rx_fifo(0);

		/* Clear all pending Device Interrupts */
		REG32(USB_OTG_BASE + DIEPMSK) = 0;
		REG32(USB_OTG_BASE + DOEPMSK) = 0;
		REG32(USB_OTG_BASE + DAINT) = DAINTMSK_OutEpMsk | DAINTMSK_InEpMsk;
		REG32(USB_OTG_BASE + DAINTMSK) = InEpMsk_EP(0)|OutEpMsk_EP(0);

		/* clear any pending interrupts */
		REG32(USB_OTG_BASE + GINTSTS) = 0xFFFFFFFF;
		REG32(USB_OTG_BASE + GOTGINT) = 0xFFFFFFFF;

		//unmask in/out
		///REG32(USB_OTG_BASE + DIEPMSK) = 0xFFFFFFFF;
		///REG32(USB_OTG_BASE + DOEPMSK) = 0xFFFFFFFF;

		/* enable common interrupt */
		#if 1
		REG32(USB_OTG_BASE + GINTMSK) |= GINTSTS_USB_RESET | GINTSTS_ENUM_DONE | \
		                                 GINTSTS_EARLY_SUSPEND | GINTSTS_USB_SUSPEND | \
		                                 GINTSTS_SOF | GINTSTS_USB_RESUME | \
		                                 GINTSTS_IN_EP_INT | GINTSTS_OUT_EP_INT;
		#else
		REG32(USB_OTG_BASE + GINTMSK) |=  GINTSTS_USB_RESUME|GINTSTS_USB_SESSREQ|GINTSTS_USB_DISCONNECT|\
	                                    GINTSTS_OUT_EP_INT|GINTSTS_IN_EP_INT | \
	                                    GINTSTS_ENUM_DONE|GINTSTS_USB_RESET | \
	                                    GINTSTS_USB_SUSPEND|GINTSTS_EARLY_SUSPEND | \
	                                    GINTSTS_SOF|GINTSTS_OTG_INT;
		#endif
	}

	return ;

	Error7:
		free (usbcb.MBR) ;
	Error6:
		free (usbcb.CSW) ;
	Error5:
		free (usbcb.CBW) ;
	Error4:
		free (usbcb.setup_pkt) ;
	Error3: 
		free (usbcb.usbbuf[1]) ;
	Error2:
		free (usbcb.usbbuf[0]) ;
	Error1:
		printk("malloc fail  !! in usb_dev_init" );
		
}

void ep0_out_start()
{
	INT32U transfer_size = 0x40; //64byte

	usbcb.ep0state  = EP0_IDLE;

	REG32(USB_OTG_BASE + DOEPTSIZ0) = SUPCnt_3p | InPktCnt_1p | transfer_size;

	REG32(USB_OTG_BASE + DOEPDMA0) = VA2PA(usbcb.setup_pkt);

	REG32(USB_OTG_BASE + DOEPCTL0) |= DXEPCTL_EPEna | DXEPCTL_CNAK | DXEPCTL_USBActEP;
	//enable ep0 to receive setup packets
}
void usb_dev_reset()
{
	INT32U i, tmp, transfer_size;

	//tomadd 0820
	//Set NAK for all OUT EPs
	for(i = 0; i < 16; i++)
		REG32(USB_OTG_BASE + DOEPCTL0 + 0x20 * i) |= DXEPCTL_SNAK;

	//Disable IN EPs if enable bit is set
	for(i = 0;  i < 16; i++)
	{
		tmp = REG32(USB_OTG_BASE + DIEPCTL0 + 0x20 * i);
		if(tmp & DXEPCTL_EPEna)
			REG32(USB_OTG_BASE + DIEPCTL0 + 0x20 * i) |= DXEPCTL_EPDis | DXEPCTL_SNAK;
	}
	// Tx FIFO Flush
	dwc_otg_flush_tx_fifo(0);
	dwc_otg_flush_rx_fifo(0);//beware of the time must not too long, otherwise the first OUT packet will be flushed

	// Reset Device Address
	REG32(USB_OTG_BASE + DCFG) &= ~(DCFG_DevAddr);
	//tomadd 0820

	REG32(USB_OTG_BASE + DAINTMSK) = InEpMsk_EP(0)|OutEpMsk_EP(0);
	//unmask in and out ep0 in DAINTMSK

	REG32(USB_OTG_BASE + DOEPMSK) =  DXEPINT_SetUPMsk | DXEPINT_XferCompl;
	//unmask setup, xfercompl in DOEPMSK

	REG32(USB_OTG_BASE + DIEPMSK) =  DXEPINT_XferCompl;
	//unmask timeout, xfercompl in DIEPMSK

	//set data fifo ram if needed, now is default value
	transfer_size = 0x40;
	REG32(USB_OTG_BASE + DOEPTSIZ0) =  SUPCnt_3p | InPktCnt_1p | transfer_size;
	//set setup packet count to 3, transfer size to 64

	REG32(USB_OTG_BASE + DOEPDMA0) = VA2PA(usbcb.setup_pkt);
	//set dma memory address for setup packets
}

void set_usb_hs()
{

	INT8U *ptr;
	usbcb.usbmps = 512;
	//for IN endpoint 1, Configuration descriptor
	ptr = usbcb.dtresponse[USB_DT_CONFIG].ptr;
	ptr[22] = 0x00;
	ptr[23] = 0x02;

	//for OUT endpoint 2, Configuration descriptor
	ptr[29] = 0x00;
	ptr[30] = 0x02;

	//for IN endpoint 1, Other speed descriptor
	ptr = usbcb.dtresponse[USB_DT_OTHERSPEED].ptr;
	ptr[22] = 0x40;
	ptr[23] = 0x00;

	//for OUT endpoint 2, Other speed descriptor
	ptr[29] = 0x40;
	ptr[30] = 0x00;
}

void set_usb_fs()
{
	INT8U *ptr;
	usbcb.usbmps = 64;
	//for IN endpoint 1
	ptr = usbcb.dtresponse[USB_DT_CONFIG].ptr;
	ptr[22] = 0x40;
	ptr[23] = 0x00;

	//for OUT endpoint 2
	ptr[29] = 0x40;
	ptr[30] = 0x00;

	//for IN endpoint 1, Other speed descriptor
	ptr = usbcb.dtresponse[USB_DT_OTHERSPEED].ptr;
	ptr[22] = 0x00;
	ptr[23] = 0x02;

	//for OUT endpoint 2, Other speed descriptor
	ptr[29] = 0x00;
	ptr[30] = 0x02;
}

void device_int_enum_done()
{
	INT32U cfgtmp, transfer_size = 0x40;

	cfgtmp = REG32(USB_OTG_BASE + GUSBCFG);

	usbcb.speedmode = (REG32(USB_OTG_BASE + DSTS) & DSTS_EnumSpd) >> 1 ;

	if(usbcb.speedmode == USB_HS)
	{
		set_usb_hs();

		REG32(USB_OTG_BASE + DIEPCTL0) &= ~(DXEPCTL_MPS);
		//High Speed
		REG32(USB_OTG_BASE + GUSBCFG) = cfgtmp & ~(GUSBCFG_PHYSel);
	}
	else
	{
		set_usb_fs();
		REG32(USB_OTG_BASE + DIEPCTL0) &= ~(DXEPCTL_MPS);	//set mps = 64
		//Full Speed
		REG32(USB_OTG_BASE + GUSBCFG) = cfgtmp | GUSBCFG_PHYSel;
	}
	REG32(USB_OTG_BASE + DIEPTSIZ0) = InPktCnt_1p | transfer_size;

	REG32(USB_OTG_BASE + DOEPCTL0) |= DXEPCTL_EPEna ;

	usbcb.ep0state = EP0_IDLE;
	//enable ep0 to receive setup packets
}

void dwc_otg_ep0_start_transfer(INT16U len, INT8U *addr, INT8U is_in)
{

#ifdef CONFIG_USBLOG_DEBUG
	usbx[usbidx][USB_UNIT - 1] = is_in ? 'I' : 'O';
	usbx[usbidx][USB_UNIT - 4] = len;
	usbx[usbidx][USB_UNIT - 2] = 0x30;
	memcpy(usbx[usbidx], (INT8U *) addr, 32);
	usbidx = (usbidx + 1) % USB_ENTRIES;
#endif
	//endpoint 0 in and out is offset by 0x200
	//turn conditioanl hazard into data dependency

	REG32(USB_OTG_BASE + DOEPTSIZ0 - is_in * 0x200)  = (InPktCnt_1p| len);
	//set packet count to 1 and set the transfer size

	REG32(USB_OTG_BASE + DOEPDMA0  - is_in * 0x200)  = VA2PA(addr);
	REG32(USB_OTG_BASE + DOEPCTL0  - is_in * 0x200) |=  DXEPCTL_EPEna | DXEPCTL_CNAK ;
}

void dwc_otg_ep_start_transfer(INT32U len, INT8U *addr, INT8U is_in)
{
	INT32U deptsiz, pkt_count;// gnptxsts;
	INT32U count = 0;

#ifdef CONFIG_USBLOG_DEBUG
	if(is_in)
	{
		memset(usbx[usbidx], 0, USB_UNIT);
		usbx[usbidx][USB_UNIT - 1] = 'I';
		usbx[usbidx][USB_UNIT - 4] = len;
		memcpy(usbx[usbidx], (INT8U *) addr, ((len > 32) ? 32 : len));
		usbidx = (usbidx + 1) % USB_ENTRIES;
	}
#endif

	if(is_in)
	{
		pkt_count = ifloor(len, usbcb.usbmps);
		deptsiz = len;
		deptsiz |= (pkt_count << 19);

		while(REG32(USB_OTG_BASE + DIEPCTL1) & DXEPCTL_EPEna)
		{
			if(count < 100000)
			{
				bsp_wait(1);
				count++;
			}
			else
			{
				OSTimeDly(1);
				count += 10000;
			}
			if(count >= 10 * 1000 * 1000) //10 secs
			{
				otg_usb_disabled();
				return;
			}
		}

		REG32(USB_OTG_BASE + DIEPTSIZ1) = deptsiz ;
		REG32(USB_OTG_BASE + DIEPDMA1) = VA2PA(addr);

		REG32(USB_OTG_BASE + DIEPCTL1) |= DXEPCTL_EPEna | DXEPCTL_CNAK;
		//clear nak, epenable
		REG32(USB_OTG_BASE + DIEPCTL0) |= 0x00000800;
		//set next ep to 1 on diepctl0

		//change to next buffer for ping-pong
		usbcb.bufindex = (usbcb.bufindex + 1) % 2;
		usbcb.rbuf = usbcb.usbbuf[usbcb.bufindex];

	}
	else  	//ep2, bulk out
	{
		pkt_count = ifloor(len , usbcb.usbmps);
		deptsiz = pkt_count * usbcb.usbmps;

		deptsiz |= (pkt_count << 19);

		REG32(USB_OTG_BASE + DOEPTSIZ2) = deptsiz ;
		REG32(USB_OTG_BASE + DOEPDMA2)  = VA2PA(addr);
		REG32(USB_OTG_BASE + DOEPCTL2) |= DXEPCTL_EPEna | DXEPCTL_CNAK;
		//doepctl2, clear nak, epenable
#ifdef CONFIG_USBLOG_DEBUG
		dbgptr = addr;
		dbglen = len;
#endif
	}
}

void dwc_otg_ep_activate(unsigned int epnum)
{
	INT32U offset;
	INT32U daintmsk = REG32(USB_OTG_BASE + DAINTMSK);
	INT32U txfifonum, eptype;

	if(epnum == 1)  	//bulk in
	{
		offset = DIEPCTL0 + 0x20 * epnum;
		daintmsk |= 1 << epnum;
		txfifonum = 1 << 22; //#fifo =1
		eptype = EPType_Bulk;
	}
	else if(epnum == 2)  	//bulk out
	{
		offset = DOEPCTL0 + 0x20 * epnum;
		daintmsk |= (1 << epnum) << 16;
		txfifonum = 0 << 22; //#fifo =0
		eptype = EPType_Bulk;
	}
	else if(epnum == 3)  	//interrupt in
	{
		offset = DIEPCTL0 + 0x20 * epnum;
		daintmsk |= 1 << epnum;
		txfifonum = 2 << 22; //#fifo =2
		eptype = EPType_Int;
	}
	else
	{
		//only support EPIN1 and EPOUT2
		return;
	}

	//check if activated already
	if(!(REG32(USB_OTG_BASE + offset) & DXEPCTL_USBActEP))
	{
		REG32(USB_OTG_BASE + offset) &= ~(DXEPCTL_MPS);
		REG32(USB_OTG_BASE + offset) |= (DXEPCTL_SetD0PID | eptype | DXEPCTL_USBActEP | usbcb.usbmps | txfifonum);
	}

	REG32(USB_OTG_BASE + DAINTMSK) = daintmsk ;
}

void ep_set_stall(unsigned short ep, unsigned char is_in)
{
	unsigned int depctl;
	if(is_in)
	{
		depctl = REG32(USB_OTG_BASE + DIEPCTL0 + ep * 0x20);
		if(depctl & DXEPCTL_EPEna)
		{
			depctl |= DXEPCTL_EPDis;	//disable ep
		}
		depctl |= DXEPCTL_Stall;	//stall
		REG32(USB_OTG_BASE + DIEPCTL0 + ep * 0x20) = depctl ;
	}
	else
	{
		depctl = REG32(USB_OTG_BASE + DOEPCTL0 + ep * 0x20);
		depctl |= DXEPCTL_Stall;	//stall
		REG32(USB_OTG_BASE + DOEPCTL0 + ep * 0x20) = depctl ;
	}

	usbcb.status[0] = 1;
}

void ep_clear_stall(unsigned short ep, unsigned char is_in)
{
	unsigned int depctl;
	if(is_in)
	{
		depctl = REG32(USB_OTG_BASE + DIEPCTL0 + ep * 0x20);
		depctl &= ~(DXEPCTL_Stall);	//clear stall
		depctl |= DXEPCTL_SetD0PID;		//set data0 pid
		REG32(USB_OTG_BASE + DIEPCTL0 + ep * 0x20) =  depctl ;
	}
	else
	{
		depctl = REG32(USB_OTG_BASE + DOEPCTL0 + ep * 0x20);
		depctl &= ~(DXEPCTL_Stall);	//clear stall
		depctl |= DXEPCTL_SetD0PID;		//set data0 pid

		REG32(USB_OTG_BASE + DOEPCTL0 + ep * 0x20) = depctl ;
	}
}

void setup_phase(struct usb_ctrlrequest *request)
{
	unsigned ep;
	INT8U *sendptr;
	INT8U sendlen;
	INT8U desctype, descindex;
	INT32U D_tmp;
	INT8U  B_tmp = 0;
#ifdef CONFIG_USBLOG_DEBUG
	INT8U *dbg = (INT8U *) request;

	memcpy(usbx[usbidx], (INT8U *) request, sizeof(*request));
	usbidx = (usbidx + 1) % USB_ENTRIES;

	DEBUGMSG(USB_DEBUG, "Setup packet %02x %02x %02x %02x %02x %02x %02x %02x\n", *dbg, *(dbg + 1), *(dbg + 2), *(dbg + 3), *(dbg + 4), *(dbg + 5), *(dbg + 6), *(dbg + 7));
#endif

	if(request->XferDir)
		usbcb.ep0state = EP0_IN_DATA_PHASE;
	else
		usbcb.ep0state = EP0_OUT_DATA_PHASE;

	switch(request->bRequest)
	{
		case USB_REQ_GET_DESCRIPTOR:

			desctype = request->wValue >> 8;
			descindex = (request->wValue & 0xFF);
			//self powered
			usbcb.status[0] = 0x01;

			//set configration for bulk or CBI whenever a host asks
			if(desctype == USB_DT_CONFIG)
			{
#ifdef CONFIG_EHCI_INTEP
				//if( usbcb.usbtype == USB_HID ) //rtk ehci port1 for hid
				if(usbcb.portnum == 1)
				{
					usbcb.dtresponse[USB_DT_CONFIG].ptr = usbcb.dtresponse[USB_DT_CONF_HID].ptr;
					usbcb.dtresponse[USB_DT_CONFIG].len = usbcb.dtresponse[USB_DT_CONF_HID].len;
				}
				else
#endif
				{
					usbcb.dtresponse[USB_DT_CONFIG].ptr = usbcb.dtresponse[USB_DT_BULK].ptr;
					usbcb.dtresponse[USB_DT_CONFIG].len = usbcb.dtresponse[USB_DT_BULK].len;
				}
			}
			
			if(desctype == DEVICE_REPORT_TYPE)  //rtk ehci port1 for hid
			{
			#ifdef CONFIG_EHCI_INTEP
				switch(request->wIndex)
				{
					case 0x01://hid: mouse=int_num1
						sendptr = usbcb.dtresponse[USB_DT_CONF_MOUSE].ptr;
						sendlen = usbcb.dtresponse[USB_DT_CONF_MOUSE].len;
						//#if defined(CONFIG_EHCI_INTEP) && defined(CONFIG_USB_HID)
						//rtkehci_INTIN_start_transfer(5 , usbcb.tmpbuf, is_IN, 2);
						mouse_ready = 1;//ready to transfer
						//#endif
						printf("report1\n");
						break;

					default://hid: keyboard=int_num0
						sendptr = usbcb.dtresponse[USB_DT_CONF_KEYBOARD].ptr;
						sendlen = usbcb.dtresponse[USB_DT_CONF_KEYBOARD].len;
						//#if defined(CONFIG_EHCI_INTEP) && defined(CONFIG_USB_HID)
						//rtkehci_INTIN_start_transfer(8 , usbcb.tmpbuf, is_IN, 1);
						kb_ready = 1;//key enumeration done
						//#endif
						printf("report0\n");
						break;

				}
			#endif
			}
			else
			{
				sendptr = usbcb.dtresponse[desctype].ptr;
				sendlen = usbcb.dtresponse[desctype].len;
#ifdef CONFIG_EHCI_INTEP
				if(usbcb.portnum == 1 && desctype == USB_DT_DEVICE)
				{
					*(sendptr + 16) = 0x00;
					*(sendptr + 10) = 0x69;
				}
#endif
			}
			
			if(desctype == USB_DT_STRING)
			{
				sendptr = usbcb.dtresponse[desctype].ptr + descindex * 32;
				//handle iserial number for MSD
				if(descindex == 3)
					sendptr += usbcb.usbtype * 32;

				sendlen = sendptr[0];
			}

			if(request->wLength > sendlen)
				request->wLength = sendlen;

			usbcb.usb_ep0_start_transfer(request->wLength , sendptr, is_IN);
			break;

		case USB_REQ_SET_ADDRESS:
			//bit 4-10 indicate the device address
			REG32(USB_OTG_BASE + DCFG) |= request->wValue << 4 ;
			usbcb.ep0state = EP0_STATUS;

			usbcb.usb_ep0_start_transfer(0, usbcb.status , is_IN);
#ifdef CONFIG_EHCI_INTEP
			//usbcb.portnum++;
			if(usbcb.portnum == 0)
			{
				//usbcb.usbtype = USB_MSD;
				REG8(DEVICE_ADDRESS) = request->wValue ;//ctlout->dev_addr, set for msd address
				printf("[0] DEVICE_ADDRESS = 0x%x\n", REG8(DEVICE_ADDRESS));
			}
			//if(usbcb.portnum == 2)
			else
			{
				//usbcb.usbtype = USB_HID;
				REG8(DEVICE_ADDRESS + 1) = request->wValue ;//ctlout->dev_addr, set for msd address
				REG8(DEVICE_ADDRESS + 2) = request->wValue ;//ctlout->dev_addr, set for msd address
				printf("[1] DEVICE_ADDRESS = 0x%x\n", REG8(DEVICE_ADDRESS + 1));
			}
#endif
			break;


		case USB_REQ_GET_CONFIGURATION:
			request->wLength = 1;
			usbcb.usb_ep0_start_transfer(request->wLength, usbcb.config, is_IN);
			break;

		case USB_REQ_GET_INTERFACE:
			request->wLength = 1;

			//usbcb.usb_ep0_start_transfer(request->wLength, usbcb.status, is_IN);
			if(request->Type == USB_TYPE_STANDARD)//standard
			{
				usbcb.usb_ep0_start_transfer(request->wLength, usbcb.status, is_IN);
			}
			else //HID class code: SET_IDLE
			{
				usbcb.ep0state = EP0_STATUS;
				usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
			}
			break;

		case USB_REQ_SET_INTERFACE:
			if(request->Type == USB_TYPE_STANDARD)//standard
			{
				usbcb.status[0] = 0;
				usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
			}
			else //HID class code: SET_PROTOCOL
			{
				usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
			}
			usbcb.ep0state = EP0_STATUS;
			break;

		case USB_REQ_SET_CONFIGURATION:
			*usbcb.config = (INT8U)(request->wValue);
			//usbcb.ep0state = EP0_STATUS;
			//usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
			if(request->Type == USB_TYPE_STANDARD)//standard
			{
				usbcb.ep0state = EP0_STATUS;
				usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);

			}
			else //HID class code: SET_REPORT
			{
				usbcb.ep0state = EP0_OUT_DATA_PHASE;
				usbcb.usb_ep0_start_transfer(request->wLength, usbcb.tmpbuf , is_OUT);
			}

			if(dpconf->usbotg)
			{
				dwc_otg_ep_activate(1);
				dwc_otg_ep_activate(2);
				//MSC Test patch!!
				//Activate endpoint set data0 pid fail~
				REG32(USB_OTG_BASE + DOEPCTL0 + 0x20 * 2) |= (DXEPCTL_SetD0PID);
				REG32(USB_OTG_BASE + DIEPCTL0 + 0x20 * 1) |= (DXEPCTL_SetD0PID);
				usbcb.usb_ep_start_transfer(CBWLength, (INT8U *) usbcb.CBW, is_OUT);  //cfg CBW address , & wait bulk out interrupt for CBW 
			}
			break;

		case USB_REQ_CLEAR_FEATURE:
			if(request->Type == USB_TYPE_STANDARD)//standard
			{
				switch(request->Recipient)
				{
					case USB_RECIP_DEVICE:
						//tomadd 0624
						//*((INT8U *) (USBDescStartAddr + 0xD4)) = 0x00;
						///  totalLen = len;
						///usbcb.ep0state = EP0_STATUS;
						usbcb.status[0] = 0x00;
						usbcb.ep0state = EP0_STATUS;
						usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
						break;

					case USB_RECIP_ENDPOINT:
						ep = request->wIndex & 0x0f;
						if(dpconf->usbotg)
						{
							if(ep == 0)
							{
								//ep_set_stall( 0, is_IN);
								ep_clear_stall(0, is_OUT);
								ep_clear_stall(0, is_IN);
								//usbcb.ep0state = EP0_IDLE;
								//ep0_out_start();
								//return;
							}
							else if(ep == 1 && usbcb.stall != 1)
							{
								ep_clear_stall(1, is_IN);
								dwc_otg_ep_activate(ep);
								//ata_csw();
							}
							else
							{
								ep_clear_stall(2, is_OUT);
								//dwc_otg_ep_activate( ep );
								usbcb.clearfeature = 1;
							}
						}
						usbcb.stall = 0;

						usbcb.status[0] = 0x00;
						usbcb.ep0state = EP0_STATUS;
						usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);

						if(dpconf->ehcipatch && !dpconf->usbotg && ep == 1)
						{
							ata_csw();
						}
						break;
				}
			}
			else //HID class code: GET_REPORT
			{
				usbcb.ep0state = EP0_IN_DATA_PHASE;
				usbcb.usb_ep0_start_transfer(request->wLength, usbcb.status, is_IN);
			}
			break;

		case USB_GET_IDLE_REQUEST:
			//HID class code: GET_IDLE
			usbcb.ep0state = EP0_IN_DATA_PHASE;
			request->wLength = 1;
			usbcb.usb_ep0_start_transfer(request->wLength, usbcb.status, is_IN);
			break;

		case USB_REQ_SET_FEATURE:
			if(request->Type == USB_TYPE_STANDARD)//standard
			{
				switch(request->wValue)
				{
					case USB_DEVICE_REMOTE_WAKEUP:
						//*((INT8U *) (USBDescStartAddr + 0xD4)) = 0x01;
						///totalLen = 0;
						usbcb.ep0state = EP0_STATUS;
						usbcb.status[0] = 0x01;
						usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
						break;

					case USB_DEVICE_TEST_MODE:
						ep = (request->wIndex & 0x0f00) >> 8;
						usbcb.ep0state = EP0_STATUS;
						switch(ep)
						{
							case 1: // TEST_J
								//dctl.b.tstctl = 1;
								usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
								B_tmp = TstCtl_J;
								break;

							case 2: // TEST_K
								//dctl.b.tstctl = 2;
								usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
								B_tmp = TstCtl_K;
								break;

							case 3: // TEST_SE0_NAK
								//dctl.b.tstctl = 3;
								usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
								B_tmp = TstCtl_SE0_NAK;
								break;

							case 4: // TEST_PACKET
								//dctl.b.tstctl = 4;
								usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
								B_tmp = TstCtl_Packet;
								break;

							case 5: // TEST_FORCE_ENABLE
								//dctl.b.tstctl = 5;
								usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
								B_tmp = TstCtl_ForceEn;
								break;
						}
						D_tmp = REG32(USB_OTG_BASE + DCTL);
						bsp_wait(50000);
						REG32(USB_OTG_BASE + DCTL) = D_tmp | B_tmp ;
						break;
					default:
						//*((INT8U *) (USBDescStartAddr + 0xD4)) = 0x01;
						usbcb.status[0] = 0x01;
						usbcb.ep0state = EP0_STATUS;
						usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
						break;
				}
			}
			else//HID class code: GET_PROTOCOL
			{
				usbcb.ep0state = EP0_IN_DATA_PHASE;
				request->wLength = 1;
				usbcb.usb_ep0_start_transfer(request->wLength, usbcb.status, is_IN);
			}

			break;

		case IdeGetMaxLun:
			//ep_set_stall(0, is_IN);
			//ep0_out_start();
			usbcb.status[0] = 0x00;
			//usbcb.ep0state = EP0_STATUS;
			if(dpconf->ehcipatch && !dpconf->usbotg)
				usbcb.stall = 1;
			usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
			break;

		case USB_CLASS_VENDOR_SPEC:
			if(request->wIndex != 0 || request->wValue != 0 || request->wLength != 0)
			{
				ep_set_stall(0, is_IN);
				ep0_out_start();
			}
			else
			{
				usbcb.ep0state = EP0_STATUS;
				usbcb.usb_ep0_start_transfer(request->wLength, usbcb.status, is_IN);
				//reset recovery
				usbcb.stall = 0;
			}
			break;

		default:
			break;
	}
	//DEBUGMSG(USB_DEBUG, "complete\n");
//	dbg("new pointer = 0x%08x\n", (unsigned int)setup_pkt);
}



void device_int_out_ep()
{
	INT32U daint = REG32(USB_OTG_BASE + DAINT) ;
	INT32U daintmsk = REG32(USB_OTG_BASE + DAINTMSK) ;
	//all out endpoint interrupt bits
	INT32U epint = (daint & daintmsk & DAINTMSK_OutEpMsk) >> 16;
	INT32U epnum = 0, doepint, doepsiz;
	INT32U doepmsk = REG32(USB_OTG_BASE + DOEPMSK) ;
	QHdr *txq;
	struct usb_ctrlrequest pkt;

	while(epint)
	{
		if(epint & 0x01)
		{
			doepint = REG32(USB_OTG_BASE + DOEPINT0 + epnum * 0x20) ;
			REG32(USB_OTG_BASE + DOEPINT0 + epnum * 0x20) |= doepint;
			doepint = doepint & doepmsk;
			if(epnum == 0)
			{
				if(doepint & DXEPINT_SetUPMsk)
				{
					memcpy(&pkt, usbcb.setup_pkt, sizeof(*usbcb.setup_pkt));
					setup_phase(&pkt);
					//memset (usbcb.setup_pkt ,  0 , sizeof(*usbcb.setup_pkt)) ;
				}
				if(doepint & DXEPINT_XferCompl)  	//transfer complete
				{
					handle_ep0();
				}
			}
			else
			{
				/*
					For OUT endpoint this field indicates that the requested data from the 
					internal FIFO is moved to external system memory. This interrupt is 
					generated only when the corresponding endpoint descriptor is closed, 
					and the IOC bit for the corresponding descriptor is set.
				*/
				if(doepint & DXEPINT_XferCompl)  	//transfer complete
				{
#ifdef CONFIG_USBLOG_DEBUG
					memset(usbx[usbidx], 0, USB_UNIT);
					usbx[usbidx][USB_UNIT - 1] = 'O';
					usbx[usbidx][USB_UNIT - 4] = dbglen;
					memcpy(usbx[usbidx], dbgptr, ((dbglen > 32) ? 32 : dbglen));
					usbidx = (usbidx + 1) % USB_ENTRIES;
#endif
					if(usbcb.epstate == EP_CBW)
					{
						doepsiz = REG32(USB_OTG_BASE + DOEPTSIZ2) ;

						if(IsValidCBW(doepsiz))   //mark it ,neil 
						{
							//message
							txq = malloc(sizeof(QHdr));
							memset(txq, 0 , sizeof(QHdr));
							txq->cmd  = USB_CBW;

							//txq->length = usbcb.CBW->dCBWDataTransferLength;  //modify 
							rlx_irq_set_mask(usbcb.irqnum, OS_TRUE); // mask interrupt 
							OSQPost(USBQ, txq);
						}

					}
					
				}
				/*
				OUT Token Received When Endpoint Disabled (OUTTknEPdis)
				Indicates that an OUT token was received when the endpoint was not yet 
				enabled. This interrupt is asserted on the endpoint for which the OUT 
				token was received.
				*/
				if(doepint & DXEPINT_INTknTXFEmp)  	//
				{
					if(usbcb.epstate == EP_CSW)
					{
						usbcb.epstate = EP_CBW;
						usbcb.usb_ep_start_transfer(CBWLength, (INT8U *) usbcb.CBW, is_OUT); // wait next bulk out 
					}
				}
			}

		}
		epnum ++;
		epint >>= 1;
	}
}

void device_int_in_ep()
{
	INT32U daint = REG32(USB_OTG_BASE + DAINT) ;
	INT32U daintmsk = REG32(USB_OTG_BASE + DAINTMSK) ;
	INT32U epint = daint & daintmsk & DAINTMSK_InEpMsk;
	INT32U epnum = 0, diepint;
	INT32U diepmsk = REG32(USB_OTG_BASE + DIEPMSK) ;
	QHdr *txq;

	while(epint)
	{
		if(epint & 0x01)
		{
			diepint = REG32(USB_OTG_BASE + DIEPINT0 + epnum * 0x20) ;
			REG32(USB_OTG_BASE + DIEPINT0 + epnum * 0x20) |= diepint;

			diepint &= diepmsk;
			if(epnum == 0)
			{
				//ep0 in transfer complete
				if(diepint & DXEPINT_XferCompl)
				{
					handle_ep0();
				}
			}
			else
			{
				/*
					For IN endpoint this field indicates that the requested data from the 
					descriptor is moved from external system memory to internal FIFO.
				*/
				if(diepint & DXEPINT_XferCompl)
				{
				
						usbcb.epstate = EP_CBW;
						usbcb.usb_ep_start_transfer(CBWLength, (INT8U *) usbcb.CBW, is_OUT); // wait next bulk out 
					//ep1 in transfer complete
				}
				if(diepint & DXEPINT_INTknTXFEmp)
				{
					if(usbcb.clearfeature)
					{
						ata_csw();
						usbcb.clearfeature = 0;
					}
				}
			}
		}
		epnum ++;
		epint >>= 1;
	}

}

void handle_ep0()
{
	QHdr *txq;
	if(usbcb.ep0state == EP0_IN_DATA_PHASE)
	{
		usbcb.usb_ep0_start_transfer(0, usbcb.status, is_OUT);
		usbcb.ep0state = EP0_STATUS;
	}
	else if(usbcb.ep0state == EP0_OUT_DATA_PHASE)
	{
		usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
		usbcb.ep0state = EP0_STATUS;
	}
	else if(usbcb.ep0state == EP0_STATUS)
	{
		ep0_out_start();
	}
}

