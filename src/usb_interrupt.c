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
#endif

//test
extern OS_EVENT *USBQ;
extern USBCB usbcb;
//unsigned char clearfeature = 0;
///volatile INT8U *bufaddr;
extern INT16U usb_sector_size[3];
extern DPCONF *dpconf;
extern char kb_ready;
extern char mouse_ready ;

struct command_block_wrapper *CBW;
struct command_status_wrapper *CSW;

#ifdef EHCISTALL
//tomadd 2011.09.18
unsigned int ep_stall;
#endif
unsigned int ep_count = 0;	
unsigned int inLen ;




#if 1//CONFIG_OTG_ENABLED//patch for usb otg, tomadd@20131212
void otg_usb_enabled(void)
{

    REG32(USB_OTG_BASE + DCTL) &= ~0x02;//SftDiscon = 0 ,normal operation

    rlx_irq_set_mask(BSP_USB_IRQ, OS_FALSE);

    otg_usb_init();

    REG32(USB_OTG_BASE + GUSBCFG) |=  0x40000000;//patch for usb otg device mode (base on firmware note v1.1)
    //patch usb device reset not issued by host
    //HP motherboard
    //usb_dev_reset();
    //device_int_enum_done();
}
#endif

#if (CONFIG_VERSION <= IC_VERSION_DP_RevF)
void otg_usb_enabled(void)
{
    REG32(0xB5000804) &= ~0x02;

    rlx_irq_set_mask(BSP_USB_IRQ, OS_FALSE);

    otg_usb_init();

    //patch usb device reset not issued by host
    //HP motherboard
    //usb_dev_reset();
    //device_int_enum_done();
}
#endif

#if 1//CONFIG_USB_FLASH
void otg_usb_disabled(void)
{

    rlx_irq_set_mask(BSP_USB_IRQ, OS_TRUE);
    REG32(USB_OTG_BASE + GAHBCFG) &= ~0x00000001;

    REG32(USB_OTG_BASE + DCTL ) |= 0x02;
    //at most 3ms + 2.5 us
    bsp_wait(10000);
}

void bsp_usb_handler(void) {

    volatile INT32U gintmsk = REG32(USB_OTG_BASE + GINTMSK);
    volatile INT32U gintsts = REG32(USB_OTG_BASE + GINTSTS);
    volatile INT32U gint_status = gintmsk & gintsts;
    //volatile INT32U diepmsk = REG32(USB_OTG_BASE + DIEPMSK);
    //INT32U diepint0 = REG32(USB_OTG_BASE + DIEPINTn);

    REG32(USB_OTG_BASE + DIEPMSK);
    REG32(USB_OTG_BASE +  GINTSTS) = gintsts;


    if ( !gint_status ) {
        return;
    }

    if ( gint_status & 0x00001000 ) {	/* usb reset */
        usb_dev_reset();
        //add lanwake after set_address command for Toshiba@20140821
#if CONFIG_LANWAKEUP_ENABLED 	
        lan_wake();
#endif        
    }

    if ( gint_status & 0x00002000 ) {	/* enumeration done */
        device_int_enum_done();
    }
    if ( gint_status & 0x00080000 ) {	/* out endpoints interrupt mask */
        //Clear Nak Status..//
        device_int_out_ep();
    }
    if ( gint_status & 0x00040000 ) {
        REG32(USB_OTG_BASE + GINTMSK) = 0;
        device_int_in_ep();
        REG32(USB_OTG_BASE + GINTMSK) = gintmsk;
    }
    if ( gint_status & 0x00000010 ) {
        REG32(USB_OTG_BASE + DOEPCTL0) |= 0x80000000;
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
        if (++count > 100000)
            return ;
    }
    while ( !(greset & 0x80000000) );	/* ahbidle */

    count = 0;
    greset |= 0x1;	/* core soft reset */
    REG32(USB_OTG_BASE + GRSTCTL) =  greset ;
    do
    {
        greset = REG32(USB_OTG_BASE + GRSTCTL);
        //soft reset fail
        if (++count > 10000)
            return;
    }
    while ( greset & 0x1 );	/* csftrst */

    bsp_wait(100000);
    //wait 3 phy clocks, 100 ms

}

void otg_usb_init()
{
    REG32(USB_OTG_BASE + GAHBCFG) &= ~0x00000001; /* disable global interrupt */

#if 0

#if CONFIG_VERSION == IC_VERSION_DP_RevA
    //Version A for USB LOGO Test!!
    Wt_UPHY(0xE0, 0x99);
    Wt_UPHY(0xE1, 0xAD);
    //Wt_UPHY(0xE2, 0x70); //for 8111DP demoboard!!
    Wt_UPHY(0xE2, 0x32);   //for 8111DP QA board
    Wt_UPHY(0xE3, 0xD8);
    Wt_UPHY(0xE4, 0x00);
    Wt_UPHY(0xE5, 0x85);
    Wt_UPHY(0xE6, 0x98);
#elif CONFIG_VERSION == IC_VERSION_DP_RevD
    //For RL6060E usb phy calibration issue
    Wt_UPHY(0xE5, 0x81);//slew rate
    Wt_UPHY(0xE2, 0x82);//sensitivity current, original default value.
    //mannul calibration for sensitivity
    Wt_UPHY(0xE3, 0x98);
    bsp_wait(50000);//delay 50 ms
    Wt_UPHY(0xE3, 0xD8);
    Wt_UPHY(0xE2, 0x32);
#endif

#endif
    /*
    *buf++ = Rd_UPHY(0xC0);
    *buf++ = Rd_UPHY(0xC1);
    *buf++ = Rd_UPHY(0xC2);
    *buf++ = Rd_UPHY(0xC3);
    *buf++ = Rd_UPHY(0xC4);
    *buf++ = Rd_UPHY(0xC5);
    *buf++ = Rd_UPHY(0xC6);
    *buf++ = Rd_UPHY(0xC5);
    *buf++ = Rd_UPHY(0xC7);
    *buf++ = Rd_UPHY(0xD0);
    *buf++ = Rd_UPHY(0xD1);
    *buf++ = Rd_UPHY(0xD2);
    *buf++ = Rd_UPHY(0xD3);

    */


    if (usbcb.usbinit == 0)
        core_reset();

    REG32(USB_OTG_BASE + GAHBCFG) |= 0x0000002E;
    /* config gahbcfg(dma, ahb burst length) */

    REG32(USB_OTG_BASE + GUSBCFG) =  0x40001408;
    /* default, disable hnp and srp and select utmi..in gusbcfg */

    //may need to  core reset again, after change phy parameters
    //according to the original driver for linux

    /* clear pending interrupt */
    REG32(USB_OTG_BASE + GOTGINT) = 0xFFFFFFFF;

    REG32(USB_OTG_BASE + GINTSTS) = 0xFFFFFFFF;

    REG32(USB_OTG_BASE + GINTMSK) |=  0x00000006;
    /* otg interrupt and mode mismatch interrupt */

    REG32(USB_OTG_BASE + GAHBCFG) |= 0x00000001;  /* enable global interrupt */

    usb_dev_init(!usbcb.usbinit);

    if (usbcb.usbinit == 0)
    {
        //USBQ = OSQCreate(USBMsg,16);
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

    REG8(CPU_BASE + VCONTROL) = 0x90|offset;
    // vloadm# need pull low ~ 99ns
    for (temp_count=0; temp_count<100; temp_count++)
        REG8(CPU_BASE + VCONTROL) = 0x80|offset;

    REG8(CPU_BASE + VCONTROL) = 0x90|offset;

    offset = (phyaddr & 0xf0)>>4;
    REG8(CPU_BASE + VCONTROL) = 0x90|offset;
    // vloadm# need pull low ~ 99ns
    for (temp_count=0; temp_count<100; temp_count++)
        REG8(CPU_BASE + VCONTROL) = 0x80|offset;

    REG8(CPU_BASE + VCONTROL) = 0x90|offset;

    //Read Register Value
    Rd_Data = REG8(CPU_BASE + UPHY_VSTATUS);

    return Rd_Data;
}
//usb phy register 0xE0~0xE7 (write)
//usb phy register 0xF0~0xF3 (write)
void Wt_UPHY(INT8U phyaddr,INT8U Wt_Data)
{
    //return;
    //it seems that FPGA would fail
    INT8U offset;

    // Input Data to usb phy register //
    REG8(CPU_BASE + VSTATUS) = Wt_Data;

    offset = phyaddr & 0x0f ;

    REG8(CPU_BASE + VCONTROL) = 0x90|offset;
    // vloadm# need pull low ~ 99ns
    //for (temp_count=0; temp_count<100; temp_count++)
    REG8(CPU_BASE + VCONTROL) = 0x80|offset;

    bsp_wait(1);

    REG8(CPU_BASE + VCONTROL) = 0x90|offset;

    offset = (phyaddr & 0xf0)>>4;
    REG8(CPU_BASE + VCONTROL) = 0x90|offset;
    // vloadm# need pull low ~ 99ns
    //for (temp_count=0; temp_count<100; temp_count++)
    REG8(CPU_BASE + VCONTROL) = 0x80|offset;

    bsp_wait(1);

    REG8(CPU_BASE + VCONTROL) = 0x90|offset;

    bsp_wait(100);
}


void dwc_otg_flush_tx_fifo()
{
    INT32U greset = 0x00000020;
    INT32U count = 0;

    REG32(USB_OTG_BASE + GRSTCTL) =  greset ;
    do
    {
        greset = REG32(USB_OTG_BASE + GRSTCTL) ;
        if (++count > 10000)
            break;
    }
    while ( greset & 0x00000020 );

    /* Wait for 3 PHY Clocks*/
    bsp_wait(1);
}

void dwc_otg_flush_rx_fifo()
{
    INT32U greset = 0x00000010;
    INT32U count = 0;

    REG32(USB_OTG_BASE + GRSTCTL) =  greset ;
    do
    {
        greset = REG32(USB_OTG_BASE + GRSTCTL) ;
        if (++count > 10000)
            break;
    }
    while ( greset & 0x00000010 );

    /* Wait for 3 PHY Clocks*/
    bsp_wait(1);
}

void usb_dev_init(INT8U hwreset)
{

    if (hwreset)
    {

        usbcb.usbbuf[0] = malloc(BULK_SIZE);
        usbcb.usbbuf[1] = malloc(BULK_SIZE);
        //need (4*n)+6 Double words
        usbcb.setup_pkt = malloc(sizeof(*usbcb.setup_pkt) * 9);	//9*8
        memset((void *) usbcb.setup_pkt, 0, sizeof(*usbcb.setup_pkt) * 9);
        ///org_setup = setup_pkt;

        usbcb.CBW = malloc(sizeof(*usbcb.CBW)*4);
        memset((void *) usbcb.CBW, 0, sizeof(*usbcb.CBW)*4);

        usbcb.CSW = malloc(sizeof(*usbcb.CSW));
        memset((void *) usbcb.CSW, 0, sizeof(*usbcb.CSW));
        usbcb.CSW->dCSWSignature = 0x53425355;

        //*((INT8U *) (USBDescStartAddr+0xD0)) = 0; //LUN
        usb_desc_init();

        //for keeping
        usbcb.MBR = malloc(512);
        usbcb.cbi = malloc(64);
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

    //disable iSerial number
    //if(usbcb.usbtype == USB_FLOPPY)
    //    usbcb.dtresponse[USB_DT_DEVICE].ptr[16] = 0;
    if(dpconf->usbotg)
    {

        REG32(USB_OTG_BASE + DCFG) &= ~0x00000003;	/* high speed */

        //2.0 full speed
        //REG32(USB_OTG_BASE + DCFG) |= 0x01;

        dwc_otg_flush_tx_fifo();
        dwc_otg_flush_rx_fifo();

        /* Clear all pending Device Interrupts */
        REG32(USB_OTG_BASE + DIEPMSK) = 0;
        REG32(USB_OTG_BASE + DOEPMSK) = 0;
        REG32(USB_OTG_BASE + DAINT) = 0xFFFFFFFF;
        REG32(USB_OTG_BASE + DAINTMSK) = 0;

        /* clear any pending interrupts */
        REG32(USB_OTG_BASE + GINTSTS) = 0xFFFFFFFF;
        REG32(USB_OTG_BASE + GOTGINT) = 0xFFFFFFFF;

        //unmask in/out
        ///REG32(USB_OTG_BASE + DIEPMSK) = 0xFFFFFFFF;
        ///REG32(USB_OTG_BASE + DOEPMSK) = 0xFFFFFFFF;

#ifdef _DYNAMIC_FIFO_
        //Disable Global OUT NAK
        REG32(USB_OTG_BASE + DCTL) |= 0x0200;
        //Disable ALL IN/OUT Endpoint and Enable IN Endpoint NAK
        for (i=0; i<15; i++)
        {
            REG32(USB_OTG_BASE+0xB20+i*20) |= 0x40000000;
            REG32(USB_OTG_BASE+0x920+i*20) |= 0x48000000;
        }
        //Rx and Tx FIFO Flush
        dwc_otg_flush_tx_fifo();
        dwc_otg_flush_rx_fifo();
        //dynamic rx fifo size
        REG32(USB_OTG_BASE + GRXFSIZ) = 0x0000082A;
        REG32(GNPTXFSIZ) = 0x0100082A;
        REG32(DIEPTXFn+0x00) = 0x0000092A;
        REG32(DIEPTXFn+0x04) = 0x00000A2A;
        REG32(DIEPTXFn+0x08) = 0x00000B2A;
        REG32(DIEPTXFn+0x0C) = 0x00000C2A;
        REG32(DIEPTXFn+0x10) = 0x00000D2A;
        REG32(DIEPTXFn+0x14) = 0x00000E2A;
        REG32(DIEPTXFn+0x18) = 0x00000F2A;
        REG32(DIEPTXFn+0x1C) = 0x00000F4A;
        REG32(DIEPTXFn+0x20) = 0x00000F6A;
        REG32(DIEPTXFn+0x24) = 0x00000F8A;
        REG32(DIEPTXFn+0x28) = 0x00000FAA;
        REG32(DIEPTXFn+0x2C) = 0x00000FCA;
        REG32(DIEPTXFn+0x30) = 0x00000FEA;
        REG32(DIEPTXFn+0x34) = 0x0000100A;
        REG32(DIEPTXFn+0x38) = 0x0000102A;
#endif

        /* enable common interrupt */
        REG32(USB_OTG_BASE + GINTMSK) |= 0x000C3C00 ;
        ///REG32(USB_OTG_BASE + GINTMSK) |= 0x00003C08 ;
        //disable rxflv1 msk, add SOF mask, disable in/out ep interrupt
        /* in/out ep interrupt, enumeration done, USB reset, suspend,
           early suspend, rxflvl msk */
    }

}

void ep0_out_start()
{
    usbcb.ep0state  = EP0_IDLE;

    REG32(USB_OTG_BASE + DOEPTSIZ0) = 0x60080040;

    REG32(USB_OTG_BASE + DOEPDMA0) = VA2PA(usbcb.setup_pkt);

    REG32(USB_OTG_BASE + DOEPCTL0) |= 0x84008000;
    //enable ep0 to receive setup packets
}
void usb_dev_reset()
{
    INT32U i, tmp;

    //tomadd 0820
    //Set NAK for all OUT EPs
    for (i = 0; i < 16; i++)
        REG32(USB_OTG_BASE + DOEPCTL0 + 0x20*i) |= 0x08000000;

    //Disable IN EPs if enable bit is set
    for (i = 0;  i< 16; i++)
    {
        tmp = REG32(USB_OTG_BASE + DIEPCTL0 + 0x20*i) >> 31;
        if (tmp)
            REG32(USB_OTG_BASE + DIEPCTL0 + 0x20*i) |= 0x48000000;
        //Disable Endpoint and Set NAK, only if Enable bit is set
        /*
            if ( (i != 0) && (tmp == 0) )
        	continue;
            if ((tmp >> 31) == 1)
            {
                REG32(USB_OTG_BASE + DIEPCTL0 + 0x20*i) |= 0x48000000;
            //Disable Endpoint and Set NAK
            }
        */
    }
    // Tx FIFO Flush
    dwc_otg_flush_tx_fifo();

    // Reset Device Address
    REG32(USB_OTG_BASE + DCFG) &= 0xFFFFF80F;
    //tomadd 0820

    REG32(USB_OTG_BASE + DAINTMSK) |=  0x00010001;
    //unmask in and out ep0 in DAINTMSK

    REG32(USB_OTG_BASE + DOEPMSK) |=  0x00000009 ;
    //REG32(USB_OTG_BASE + DOEPMSK) |=  0x00000049 ;
    //unmask setup, xfercompl, back-to-back setup packets in DOEPMSK

    REG32(USB_OTG_BASE + DIEPMSK) |= 0x00000009 ;
    //unmask timeout, xfercompl in DIEPMSK

    //set data fifo ram if needed, now is default value

    REG32(USB_OTG_BASE + DOEPTSIZ0) =  0x60080040 ;
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
    INT32U cfgtmp;
    INT8U *ptr;

    usbcb.protocol = USB_BULK;

    cfgtmp = REG32(USB_OTG_BASE + GUSBCFG);

    usbcb.speedmode = (REG32(USB_OTG_BASE + DSTS) & 0x00000006) >> 1 ;

    if ( usbcb.speedmode == USB_HS ) {
        set_usb_hs();

        REG32(USB_OTG_BASE + DIEPCTL0) &= ~0x00000003;
        //High Speed
        REG32(USB_OTG_BASE + GUSBCFG) = cfgtmp & ~(0x00000040);
        //REG32(USB_OTG_BASE + DCFG) &= ~0x00000003;	/* high speed */
        //REG32(USB_OTG_BASE+0x900) |= cpu_to_le32(0x80000000);
    }
    else {

        set_usb_fs();
        REG32(USB_OTG_BASE + DIEPCTL0) &= ~0x00000003;	//set mps = 64
        //Full Speed
        REG32(USB_OTG_BASE + GUSBCFG) = cfgtmp | 0x00000040;
        //REG32(USB_OTG_BASE + DCFG) |=  0x00000001;	/* full speed */
        //REG32(USB_OTG_BASE+0x900) |= cpu_to_le32(0x00000003);	//set mps = 8
        //REG32(USB_OTG_BASE+0x00C) |= cpu_to_le32(0x00000001);	//set FS timeout cal
    }
    REG32(USB_OTG_BASE + DIEPTSIZ0) = 0x00080040 ;

    REG32(USB_OTG_BASE + DOEPCTL0) |= 0x80000000 ;

    usbcb.ep0state = EP0_IDLE;
    //enable ep0 to receive setup packets
}

void dwc_otg_ep0_start_transfer(INT16U len, INT8U *addr, INT8U is_in)
{

#ifdef CONFIG_USBLOG_DEBUG
    usbx[usbidx][USB_UNIT-1] = is_in ? 'I' : 'O';
    usbx[usbidx][USB_UNIT-4] = len;
    usbx[usbidx][USB_UNIT-2] = 0x30;
    memcpy(usbx[usbidx], (INT8U *) addr, 32);
    usbidx = (usbidx + 1) % USB_ENTRIES;
#endif


//may only need to be checked using shared FIFO
#if 0
    INT32U gtxstatus;
    if (is_in)
    {
        do {
            gtxstatus = REG32(USB_OTG_BASE + GNPTXSTS) ;
        } while (!(gtxstatus & 0x00FF0000));
    }
#endif

    //endpoint 0 in and out is offset by 0x200
    //turn conditioanl hazard into data dependency

    REG32(USB_OTG_BASE + DOEPTSIZ0 - is_in * 0x200)  = (0x00080000 | len);
    //set packet count to 1 and set the transfer size

    REG32(USB_OTG_BASE + DOEPDMA0  - is_in * 0x200)  = VA2PA(addr);
    REG32(USB_OTG_BASE + DOEPCTL0  - is_in * 0x200) |=  0x84000000 ;
    //if (is_in)
    //    REG32(USB_OTG_BASE + DOEPCTL0  - is_in * 0x200) &=  ~0x00000800 ;

}

void dwc_otg_ep_start_transfer(INT32U len, INT8U *addr, INT8U is_in)
{
    //unsigned int depctl, deptsiz, pkt_count, tmp, gnptxsts;
    INT32U deptsiz, pkt_count;// gnptxsts;
    //INT32U diepint = REG32(USB_OTG_BASE+ DIEPINT1) ;
    INT32U count = 0;
    //INT32U state;

#ifdef CONFIG_USBLOG_DEBUG
    if (is_in)
    {
        memset(usbx[usbidx], 0, USB_UNIT);
        usbx[usbidx][USB_UNIT-1] = 'I';
        usbx[usbidx][USB_UNIT-4] = len;
        memcpy(usbx[usbidx], (INT8U *) addr, ((len > 32) ? 32 : len));
        usbidx = (usbidx + 1) % USB_ENTRIES;
    }
#endif

    if ( is_in ) {
//may only need to be checked in shared FIFO mode
#if 0
        do {
            gnptxsts = REG32(USB_OTG_BASE + GNPTXSTS) ;
        } while (!(gnptxsts & 0x00FF0000));
#endif

        pkt_count = ifloor(len, usbcb.usbmps);
        deptsiz = len;
        deptsiz |= ( pkt_count << 19 );

        while (REG32(USB_OTG_BASE + DIEPCTL1) & 0x80000000)
        {
            if (count < 100000)
            {
                bsp_wait(1);
                count++;
            }
            else
            {
                OSTimeDly(1);
                count += 10000;
            }
            if (count >= 10*1000*1000) //10 secs
            {
                otg_usb_disabled();
                return;
            }
        }

        REG32(USB_OTG_BASE + DIEPTSIZ1) = deptsiz ;
        REG32(USB_OTG_BASE + DIEPDMA1 ) = VA2PA(addr);

        REG32(USB_OTG_BASE + DIEPCTL1) |= 0x84000000;
        //clear nak, epenable
        REG32(USB_OTG_BASE + DIEPCTL0) |= 0x00000800;
        //set next ep to 1 on diepctl0

        //change to next buffer for ping-pong
        usbcb.bufindex = (usbcb.bufindex + 1) % 2;
        usbcb.rbuf = usbcb.usbbuf[usbcb.bufindex];

    }
    else {	//ep2, bulk out
        pkt_count = ifloor(len , usbcb.usbmps);
        deptsiz = pkt_count * usbcb.usbmps;

        deptsiz |= ( pkt_count << 19 );

        REG32(USB_OTG_BASE + DOEPTSIZ2) = deptsiz ;
        REG32(USB_OTG_BASE + DOEPDMA2)  = VA2PA(addr);
        REG32(USB_OTG_BASE + DOEPCTL2) |= 0x84000000;
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
    INT32U txfifonum;

    if ( epnum == 1 ) {	//bulk in
        offset = 0x920;
        daintmsk |= 0x00000002;
        txfifonum = 0x00400000;
    }
    else if ( epnum == 2 ) {	//bulk out
        offset = 0xB40;
        daintmsk |= 0x00040000;
        txfifonum = 0x00000000;
    }
    else if ( epnum == 3 ) {	//interrupt in
        offset = 0x960;
        daintmsk |= 0x00000008;
        txfifonum = 0x00840000;
    }
    else {
        //only support EPIN1 and EPOUT2
        return;
    }

    //check if activated already
    if ( !(REG32(USB_OTG_BASE + offset) & 0x00008000))
    {
        REG32(USB_OTG_BASE + offset) &= ~0x000007FF;
        REG32(USB_OTG_BASE + offset) |= (0x10088000 | usbcb.usbmps | txfifonum);
    }

    REG32(USB_OTG_BASE + DAINTMSK) = daintmsk ;
}

void ep_set_stall( unsigned short ep, unsigned char is_in )
{
    unsigned int depctl;
    if ( is_in ) {
        depctl = REG32(USB_OTG_BASE+ DIEPCTL0 + ep*0x20);
        if ( depctl & 0x80000000 ) {
            depctl |= 0x40000000;	//disable ep
        }
        depctl |= 0x00200000;	//stall
        REG32(USB_OTG_BASE+ DIEPCTL0 + ep*0x20) = depctl ;
//		dbg("reg 0x920 = 0x%08x\n", cpu_to_le32(REG32(USB_OTG_BASE+0x900+ep*0x20)));
    }
    else {
        depctl = REG32(USB_OTG_BASE + DOEPCTL0 + ep*0x20);
        depctl |= 0x00200000;	//stall
        REG32(USB_OTG_BASE+DOEPCTL0+ep*0x20) = depctl ;
    }

    usbcb.status[0] = 1;

}

void ep_clear_stall( unsigned short ep, unsigned char is_in )
{
    unsigned int depctl;
    if ( is_in ) {
        depctl = REG32(USB_OTG_BASE + DIEPCTL0 + ep*0x20);
        depctl &= ~(0x00200000);	//clear stall
        depctl |= 0x10000000;		//set data0 pid
        REG32(USB_OTG_BASE + DIEPCTL0 + ep*0x20) =  depctl ;
    }
    else {
        depctl = REG32(USB_OTG_BASE + DOEPCTL0 +ep*0x20);
        depctl &= ~(0x00200000);	//clear stall
        depctl |= 0x10000000;		//set data0 pid

        REG32(USB_OTG_BASE + DOEPCTL0 + ep*0x20) = depctl ;
    }
}
#endif

void setup_phase(struct usb_ctrlrequest *request)
{
    unsigned ep;
    ///static INT8U count = 0;
    ///INT8U  Config;
    ///struct usb_ctrlrequest *request;
    INT8U *sendptr;
    INT8U sendlen;
    INT8U desctype, descindex;
    INT32U D_tmp;
    INT8U  B_tmp = 0;
#ifdef CONFIG_USBLOG_DEBUG
    INT8U *dbg = (INT8U *) request;

    memcpy(usbx[usbidx], (INT8U *) request, sizeof(*request));
    usbidx = (usbidx + 1) % USB_ENTRIES;

    DEBUGMSG(USB_DEBUG, "Setup packet %02x %02x %02x %02x %02x %02x %02x %02x\n", *dbg, *(dbg+1), *(dbg+2), *(dbg+3), *(dbg+4), *(dbg+5), *(dbg+6), *(dbg+7));
#endif

    if (request->XferDir)
        usbcb.ep0state = EP0_IN_DATA_PHASE;
    else
        usbcb.ep0state = EP0_OUT_DATA_PHASE;
//INT8U *dbg = (INT8U *) request;
//printk("Setup packet %02x %02x %02x %02x %02x %02x %02x %02x\r\n", *dbg, *(dbg+1), *(dbg+2), *(dbg+3), *(dbg+4), *(dbg+5), *(dbg+6), *(dbg+7));
    switch ( request->bRequest ) {
    case USB_REQ_GET_DESCRIPTOR:

        desctype = request->wValue >> 8;
        descindex = (request->wValue & 0xFF);
        //self powered
        usbcb.status[0] = 0x01;


        //set configration for bulk or CBI whenever a host asks
        if (desctype == 2)
        {
#ifndef CONFIG_VENDOR_DELL
            //if( usbcb.usbtype == USB_HID ) //rtk ehci port1 for hid
            if( usbcb.portnum == 1 )
            {
                usbcb.dtresponse[USB_DT_CONFIG].ptr = usbcb.dtresponse[USB_DT_CONF_HID].ptr;
                usbcb.dtresponse[USB_DT_CONFIG].len = usbcb.dtresponse[USB_DT_CONF_HID].len;            	
            }
            else
#else            
//#ifdef CONFIG_VENDOR_DELL

            if (!bsp_in_band_exist() && (usbcb.usbtype == USB_FLOPPY))
            {
                usbcb.dtresponse[USB_DT_CONFIG].ptr = usbcb.dtresponse[USB_DT_CBI].ptr;
                usbcb.dtresponse[USB_DT_CONFIG].len = usbcb.dtresponse[USB_DT_CBI].len;
#ifdef CONFIG_LATITUDE
                usbcb.dtresponse[USB_DT_CONFIG].ptr[16] = 1;
#endif
            }
            else
#endif
            {
                usbcb.dtresponse[USB_DT_CONFIG].ptr = usbcb.dtresponse[USB_DT_BULK].ptr;
                usbcb.dtresponse[USB_DT_CONFIG].len = usbcb.dtresponse[USB_DT_BULK].len;
            }
        }

        if (desctype == USB_DT_STRING)
        {
            sendptr = usbcb.dtresponse[desctype].ptr + descindex * 32;
            //handle iserial number for MSD
            if (descindex == 3 )
                sendptr += usbcb.usbtype * 32;
	
            #if (CONFIG_VERSION  == IC_VERSION_EP_RevB)   //patch usb reconnection issue@AMD Larne platform
            if(descindex == 0 && !dpconf->usbotg)
            	*sendptr = 0x08;
            #endif
            sendlen = sendptr[0];
        }

        if( desctype == DEVICE_REPORT_TYPE )//rtk ehci port1 for hid 
        {
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
        }
        else
        {
            sendptr = usbcb.dtresponse[desctype].ptr;
            sendlen = usbcb.dtresponse[desctype].len;
            if(usbcb.portnum == 1 && desctype == USB_DT_DEVICE){
            	*(sendptr + 16) = 0x00;
            	*(sendptr + 10) = 0x69;
            }

        }

        if (request->wLength > sendlen)
            request->wLength = sendlen;

        usbcb.usb_ep0_start_transfer(request->wLength , sendptr, is_IN);

        break;

    case USB_REQ_SET_ADDRESS:
        //bit 4-10 indicate the device address
        REG32(USB_OTG_BASE+ DCFG) |= request->wValue << 4 ;
        usbcb.ep0state = EP0_STATUS;

        usbcb.usb_ep0_start_transfer(0, usbcb.status , is_IN);
#ifdef CONFIG_EHCI_INTEP        
        //usbcb.portnum++;
        if(usbcb.portnum == 0){
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
        	printf("[1] DEVICE_ADDRESS = 0x%x\n", REG8(DEVICE_ADDRESS+1));
        }
#endif        	
        break;

    case USB_REQ_GET_STATUS:
        //bufaddr = (INT8U *)(USBDescStartAddr + 0xD4);
        usbcb.epstate = EP_CBW;
        //usbcb.usb_ep_start_transfer(CBWLength, (INT8U *) usbcb.CBW, is_OUT);

        //CBI should send 12 bytes
        //usbcb.ep0state = EP0_OUT_DATA_PHASE;
        if (request->wLength < 12)
        {
            //usbcb.epstate = EP_CBW;
            if(dpconf->usbotg)
                usbcb.usb_ep_start_transfer(CBWLength, (INT8U *) usbcb.CBW, is_OUT);

            //request->wLength = 2;

            usbcb.usb_ep0_start_transfer(2, usbcb.status, is_IN);
        }
        else
        {
            //get CBW from control endpoint
            usbcb.CBW->bCBWCBLength = request->wLength;
            usbcb.ep0state = EP0_CBI;
            usbcb.usb_ep0_start_transfer(request->wLength, usbcb.cbi, is_OUT);
            //need to ensure that get the CBI for CBW
            usbcb.protocol = USB_CBI;
        }



        break;

    case USB_REQ_GET_CONFIGURATION:
        request->wLength = 1;
        usbcb.usb_ep0_start_transfer(request->wLength, usbcb.config, is_IN);
        break;

    case USB_REQ_GET_INTERFACE:
        request->wLength = 1;
        ///bufaddr = (INT8U *)(USBDescStartAddr + 0xD4);
#if 0
        bufaddr = &LUN;
#endif
#if 0
        usbcb.usb_ep0_start_transfer(len, &LUN, is_IN);
#endif
        //usbcb.usb_ep0_start_transfer(request->wLength, usbcb.status, is_IN);
        if(request->Type == 0)//standard
        	usbcb.usb_ep0_start_transfer(request->wLength, usbcb.status, is_IN);

        else{//HID class code: SET_IDLE
        	usbcb.ep0state = EP0_STATUS;
        	usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
        }	

        break;
    case USB_REQ_SET_INTERFACE:
        ///*((INT8U *) (USBDescStartAddr + 0xD4)) = 0;
        if(request->Type == 0)//standard
        {
        	usbcb.status[0] = 0;
        	usbcb.ep0state = EP0_STATUS;
        	usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
        }
        else{//HID class code: SET_PROTOCOL
        	usbcb.ep0state = EP0_STATUS;
        	usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
        }

        break;

    case USB_REQ_SET_CONFIGURATION:
        *usbcb.config = (INT8U)(request->wValue);
        //usbcb.ep0state = EP0_STATUS;
        //usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
        if(request->Type == 0)//standard
        {
        	usbcb.ep0state = EP0_STATUS;
        	usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);

        }
        else{//HID class code: SET_REPORT
        	usbcb.ep0state = EP0_OUT_DATA_PHASE;
        	usbcb.usb_ep0_start_transfer(request->wLength, usbcb.tmpbuf , is_OUT);
        }
        
        if(dpconf->usbotg)
        {
            dwc_otg_ep_activate( 1 );
            dwc_otg_ep_activate( 2 );
            //MSC Test patch!!
            //Activate endpoint set data0 pid fail~
            REG32(USB_OTG_BASE+0xB40) |= (0x10000000);
            REG32(USB_OTG_BASE+0x920) |= (0x10000000);
            usbcb.usb_ep_start_transfer(CBWLength, (INT8U *) usbcb.CBW, is_OUT);

        }
        break;

    case USB_REQ_CLEAR_FEATURE:
        if(request->Type == 0)//standard
        {
        	switch (request->Recipient) {
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
                if ( ep == 0 )
                {
                    //ep_set_stall( 0, is_IN);
                    ep_clear_stall( 0, is_OUT);
                    ep_clear_stall( 0, is_IN);
                    //usbcb.ep0state = EP0_IDLE;
                    //ep0_out_start();
                    //return;
                }
                else if ( ep == 1 && usbcb.stall != 1) {
                    ep_clear_stall( 1, is_IN );
                    dwc_otg_ep_activate( ep );
                    //ata_csw();
                }
                else {
                    ep_clear_stall( 2, is_OUT );
                    //dwc_otg_ep_activate( ep );
                    usbcb.clearfeature = 1;
                }
            }
            usbcb.stall = 0;

#if 0
            if(dpconf->ehcipatch && !dpconf->usbotg && ep == 1)
            {
#if CONFIG_VERSION == IC_VERSION_EP_RevA
                bsp_wait(100);
#endif
            }
#endif

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
        else{//HID class code: GET_REPORT
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
        if(request->Type == 0)//standard
        {
        	switch (request->wValue) {
            case USB_DEVICE_REMOTE_WAKEUP:
            //*((INT8U *) (USBDescStartAddr + 0xD4)) = 0x01;
            ///totalLen = 0;
              usbcb.ep0state = EP0_STATUS;
              usbcb.status[0] = 0x01;
              usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
            break;

            case USB_DEVICE_TEST_MODE:
              ep = (request->wIndex & 0x0f00)>>8;
              usbcb.ep0state = EP0_STATUS;
              switch (ep) {
              case 1: // TEST_J
                //dctl.b.tstctl = 1;
                usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
                B_tmp = 0x10;
                //REG32(USB_OTG_BASE + DCTL)= D_tmp | 0x00000010 ;
                break;

              case 2: // TEST_K
                //dctl.b.tstctl = 2;
                usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
                B_tmp = 0x20;
                //REG32(USB_OTG_BASE + DCTL)= D_tmp | 0x00000020 ;
                break;

              case 3: // TEST_SE0_NAK
                //dctl.b.tstctl = 3;
                usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
                B_tmp = 0x30;
                //REG32(USB_OTG_BASE + DCTL)= D_tmp | 0x00000030 ;
                break;

              case 4: // TEST_PACKET
                //dctl.b.tstctl = 4;
                usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
                B_tmp = 0x40;
                //REG32(USB_OTG_BASE + DCTL)= D_tmp | 0x00000040 ;
                break;

              case 5: // TEST_FORCE_ENABLE
                //dctl.b.tstctl = 5;
                usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
                B_tmp = 0x50;
                //REG32(USB_OTG_BASE + DCTL)= D_tmp | 0x00000050 ;
                break;
              }

              D_tmp = REG32(USB_OTG_BASE + DCTL);
               bsp_wait(50000);
              REG32(USB_OTG_BASE + DCTL)= D_tmp | B_tmp ;
              break;

            default:
            //*((INT8U *) (USBDescStartAddr + 0xD4)) = 0x01;
              usbcb.status[0] = 0x01;
              usbcb.ep0state = EP0_STATUS;
              usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);

            break;
        	}
        }
        else{//HID class code: GET_PROTOCOL
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
        if (request->wIndex != 0 || request->wValue != 0 || request->wLength != 0)
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
//			dbg("other setup..\n");
        break;
    }
    DEBUGMSG(USB_DEBUG, "complete\n");
//	dbg("new pointer = 0x%08x\n", (unsigned int)setup_pkt);
}



void device_int_out_ep()
{
    INT32U daint = REG32(USB_OTG_BASE + DAINT) ;
    INT32U daintmsk = REG32(USB_OTG_BASE + DAINTMSK) ;

    //all out endpoint interrupt bits
    INT32U epint = (daint & daintmsk & 0xFFFF0000) >> 16;

    INT32U epnum = 0, doepint, doepsiz;
    INT32U doepmsk = REG32(USB_OTG_BASE + DOEPMSK) ;
    QHdr *txq;
    struct usb_ctrlrequest pkt;
    //INT8U  index;
    //INT8U  *lastpkt;


    while ( epint ) {
        if ( epint & 0x01 ) {
            doepint = REG32(USB_OTG_BASE + DOEPINT0 + epnum*0x20) ;
            REG32(USB_OTG_BASE+ DOEPINT0 +epnum*0x20) |= doepint;
            doepint = doepint & doepmsk;
            if ( epnum == 0 ) {
                if ( doepint & 0x00000008 ) {

                    /*
                    if(doepint & 0x00000040) //back-to-back
                    {
                    lastpkt = PA2VA(REG32(USB_OTG_BASE + DOEPDMA0)) - 8;
                                memcpy(&pkt,lastpkt, sizeof(setup_pkt));
                    }
                    else
                    {
                    index = 2 - REG32(USB_OTG_BASE + DOEPTSIZ0);
                    lastpkt = setup_pkt + index;
                                memcpy(&pkt, lastpkt, sizeof(setup_pkt));
                    }

                    #ifdef CONFIG_USBLOG_DEBUG
                    memset(usbx[usbidx], 0, USB_UNIT);
                    usbx[usbidx][USB_UNIT-1] = 'S';
                    usbx[usbidx][0] = doepint;
                    usbx[usbidx][4] = REG32(USB_OTG_BASE + DOEPTSIZ0);
                    usbx[usbidx][8] = REG32(USB_OTG_BASE + DOEPDMA0);
                    usbx[usbidx][12] = setup_pkt;
                    usbidx = (usbidx + 1) % USB_ENTRIES;
                    memset(usbx[usbidx], 0, USB_UNIT);
                    memcpy(usbx[usbidx], setup_pkt, 32);
                    usbx[usbidx][USB_UNIT-1] = 'S';
                    usbidx = (usbidx + 1) % USB_ENTRIES;
                    #endif
                    */
                    memcpy(&pkt, usbcb.setup_pkt, sizeof(*usbcb.setup_pkt));
                    //ep0_out_start();
                    setup_phase(&pkt);

                }
                if ( doepint & 0x00000001 ) {	//transfer complete
                    handle_ep0();

                }
            }
            else {
                if ( doepint & 0x00000001 ) {	//transfer complete
#ifdef CONFIG_USBLOG_DEBUG
                    memset(usbx[usbidx], 0, USB_UNIT);
                    usbx[usbidx][USB_UNIT-1] = 'O';
                    usbx[usbidx][USB_UNIT-4] = dbglen;
                    memcpy(usbx[usbidx], dbgptr, ((dbglen > 32) ? 32 : dbglen));
                    usbidx = (usbidx + 1) % USB_ENTRIES;
#endif
                    if ( usbcb.epstate == EP_CBW ) {
                        doepsiz = REG32(USB_OTG_BASE+ DOEPTSIZ2) ;
#ifdef CONFIG_USB_BULK_TEST
                        if ( IsValidCBW(doepsiz) ) {
#else
                        if ( IsValidCBW(doepsiz) && IsMeaningfulCBW()) {
#endif
                            txq = malloc(sizeof(QHdr));
                            memset(txq, 0 , sizeof(QHdr));
                            txq->cmd  = USB_CBW;
                            txq->length = usbcb.CBW->dCBWDataTransferLength;
                            rlx_irq_set_mask(usbcb.irqnum, OS_TRUE);
                            OSQPost(USBQ, txq);
                        }
#ifndef CONFIG_USB_BULK_TEST
                        else if (!IsValidCBW(doepsiz))
                        {
                            usbcb.stall = 1;

                            if(dpconf->usbotg)
                                ep_set_stall(1, is_IN);
                            else
                                usbcb.usb_ep_start_transfer(0, usbcb.rbuf, is_IN);
                        }
#endif
                        else
                        {
                            usbcb.CSW->bCSWStatus = 1;
                            usbcb.CSW->dCSWTag = usbcb.CBW->dCBWTag;
                            txq = malloc(sizeof(QHdr));
                            memset(txq, 0 , sizeof(QHdr));
#ifdef CONFIG_USB_BULK_TEST
                            txq->cmd  = USB_BULK_TEST;
#else
                            txq->cmd  = USB_STATUS_ERROR;
#endif
                            rlx_irq_set_mask(usbcb.irqnum, OS_TRUE);
                            OSQPost(USBQ, txq);
                        }
                    }
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
                }
                if ( doepint & 0x00000010 ) {	//

#ifdef TEST_OUT
                    if (REG32(USB_OTG_BASE + DOEPSIZ2)==0 && REG32(USB_OTG_BASE+DOEPCTL2)>>31==0) {
                        REG32(USB_OTG_BASE + DOEPDMA2)  =  VA2PA(addr);
                        REG32(USB_OTG_BASE + DOEPSIZ2)  =  0x01004000;
                        REG32(USB_OTG_BASE + DOEPCTL2) |=  0x84000000;
                    }
#endif

                    if ( usbcb.epstate == EP_CSW ) {
                        usbcb.epstate = EP_CBW;
                        usbcb.usb_ep_start_transfer(CBWLength, (INT8U *) usbcb.CBW, is_OUT);
                    }
                    //////test
#if 0
                    doepsiz = cpu_to_le32( REG32(USB_OTG_BASE+0xB50) );
                    if ( ((ep_mps - doepsiz) == CBWLength) && (lc == WRITE10) ) {
                        DEBUGMSG(USB_DEBUG, "test cbw\n");
                        epstate = EP_CBW;
                        ata_cbw();

                    }
                    else {
                        DEBUGMSG(USB_DEBUG, "lc = 0x%x\n", lc);
                    }
#endif
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
    INT32U epint = daint & daintmsk & 0x0000FFFF;
    INT32U epnum = 0, diepint;
    INT32U diepmsk = REG32(USB_OTG_BASE + DIEPMSK) ;
    QHdr *txq;

    while ( epint ) {
        if ( epint & 0x01 ) {
            diepint = REG32(USB_OTG_BASE+ DIEPINT0 +epnum*0x20) ;
            REG32(USB_OTG_BASE + DIEPINT0 + epnum*0x20) |= diepint;
            diepint = diepint & diepmsk;
            if ( epnum == 0 ) {
                //ep0 in transfer complete
                if ( diepint & 0x00000001 ) {
                    handle_ep0();
                }
            }
            else {
                if ( diepint & 0x00000001 ) {
                    if (usbcb.protocol == USB_BULK)
                    {
                        txq = malloc(sizeof(QHdr));
                        memset(txq, 0 , sizeof(QHdr));
                        txq->cmd  = USB_EP_IN_COMPLETE;
                        rlx_irq_set_mask(usbcb.irqnum, OS_TRUE);
                        OSQPost(USBQ, txq);
                    }
                    else
                        usbcb.epstate = EP_CBW;
                    //ep1 in transfer complete

#ifdef TEST_IN
                    if (REG32(USB_OTG_BASE + DIEPSIZ1) == 0 && REG32(USB_OTG_BASE+DIEPCTL1)>>31 == 0)
                    {
                        REG32(USB_OTG_BASE + DIEPSIZ1)  = 0x01004000 ;
                        REG32(USB_OTG_BASE + DIEPDMA1)  = VA2PA(addr) ;
                        REG32(USB_OTG_BASE + DIEPCTL1) |= 0x84000000 ;
                        REG32(USB_OTG_BASE + DIEPCTL0) |= 0x00000800 ;	                            //set next ep to 1 on diepctl0
                    }
#endif
                }
                if ( diepint & 0x00000010 ) {
                    if ( usbcb.clearfeature ) {
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
void handle_cbi()
{
    QHdr *txq;

    usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
    usbcb.ep0state = EP0_STATUS;

    memcpy(usbcb.CBW->rbc, usbcb.cbi, usbcb.CBW->bCBWCBLength);
    usbcb.CBW->bmCBWFlags = 0x80;

    switch (usbcb.CBW->rbc[0])
    {
    case INQUIRY:
    case REQUEST_SENSE:
        usbcb.CBW->dCBWDataTransferLength = usbcb.CBW->rbc[4];
        break;

    case READ_CAPACITY:
        usbcb.CBW->dCBWDataTransferLength = 0x08;
        break;

    case TEST_UNIT_READY:
    case START_STOP_UNIT:
        return;

    case WRITE10:
        usbcb.CBW->dCBWDataTransferLength = ((usbcb.CBW->rbc[7] << 8) + usbcb.CBW->rbc[8]) * usbcb.sectorsize;
        usbcb.CBW->bmCBWFlags = 0x00;
        break;

    case WRITE12:
        usbcb.CBW->dCBWDataTransferLength = ((usbcb.CBW->rbc[6] << 24) + (usbcb.CBW->rbc[7] << 16) + (usbcb.CBW->rbc[8] << 8) + usbcb.CBW->rbc[9]) * usbcb.sectorsize;
        usbcb.CBW->bmCBWFlags = 0x00;
        break;

    case VERIFY:

        usbcb.CBW->dCBWDataTransferLength = ((usbcb.CBW->rbc[7] << 16) + (usbcb.CBW->rbc[8] << 8) + usbcb.CBW->rbc[9]);
        break;

    case READ10:
        usbcb.CBW->dCBWDataTransferLength = ((usbcb.CBW->rbc[7] << 8) + usbcb.CBW->rbc[8]) * usbcb.sectorsize;
        break;

    case READ12:
        usbcb.CBW->dCBWDataTransferLength = ((usbcb.CBW->rbc[6] << 24) + (usbcb.CBW->rbc[7] << 16) + (usbcb.CBW->rbc[8] << 8) + usbcb.CBW->rbc[9]) * usbcb.sectorsize;
        break;


    case MODE_SENSE10:
        usbcb.CBW->dCBWDataTransferLength = 0x12;
        break;

    }

    txq = malloc(sizeof(QHdr));
    memset(txq, 0 , sizeof(QHdr));
    txq->cmd  = USB_CBW;
    txq->length = usbcb.CBW->dCBWDataTransferLength;
    rlx_irq_set_mask(usbcb.irqnum, OS_TRUE);
    OSQPost(USBQ, txq);

}
void handle_ep0()
{
    QHdr *txq;
    if (usbcb.ep0state == EP0_IN_DATA_PHASE)
    {
        usbcb.usb_ep0_start_transfer(0, usbcb.status, is_OUT);
        usbcb.ep0state = EP0_STATUS;
    }
    else if (usbcb.ep0state == EP0_OUT_DATA_PHASE)
    {
        usbcb.usb_ep0_start_transfer(0, usbcb.status, is_IN);
        usbcb.ep0state = EP0_STATUS;
    }
    else if (usbcb.ep0state == EP0_CBI)
    {
        handle_cbi();

    }
    else if (usbcb.ep0state == EP0_STATUS)
    {
        ep0_out_start();
    }
}

