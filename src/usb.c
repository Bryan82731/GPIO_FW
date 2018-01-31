#include "bsp.h"
#include "lib.h"
#include "usb.h"
#include "rtskt.h"
#include "telnet.h"
#include <string.h>
#include <stdlib.h>


OS_EVENT *USBQ;
OS_EVENT *USBTxSem;

ROM_EXTERN void *USBMsg[16] _ATTRIBUTE_ROM_BSS;
ROM_EXTERN2 RTSkt *usbcs _ATTRIBUTE_ROM_BSS;
#if CONFIG_VERSION  >= IC_VERSION_EP_RevB
RTSkt *usbSrv;
#else
RTSkt *usbSrv;
#endif


extern const INT16U usb_sector_size[3];

const char RtHttpHdr[] = {"GET /%s HTTP/1.1\r\nAccept: */*\r\nHost: %d.%d.%d.%d\r\nRange: bytes=%u-%u\r\nConnection: Keep-Alive\r\n\r\n"};
char fn[80] = "win98_r.bin";
unsigned char tip[4] = {192,168,0,5};


extern void rtkehci_ep_start_transfer(INT32U len, INT8U *addr, INT8U is_in);
extern void rtkehci_ep0_start_transfer(INT16U len, INT8U *addr, INT8U is_in);
extern void rtkehci_init(void);
extern void ehci_usb_enabled(void);
extern void ehci_usb_disabled(void);
extern void dwc_otg_ep_start_transfer(INT32U len, INT8U *addr, INT8U is_in);
extern void dwc_otg_ep0_start_transfer(INT16U len, INT8U *addr, INT8U is_in);

extern DPCONF *dpconf;
extern USBCB usbcb;

void usb_config_init()
{
    if(dpconf->usbotg)
    {
        usbcb.irqnum                 = BSP_USB_IRQ;
		usbcb.bsp_usb_init			 = otg_usb_init;
		usbcb.bsp_usb_enabled		 = otg_usb_enabled;
		usbcb.bsp_usb_disabled		 = otg_usb_disabled;
		usbcb.usb_ep_start_transfer  = dwc_otg_ep_start_transfer;
		usbcb.usb_ep0_start_transfer = dwc_otg_ep0_start_transfer;
    }
#if defined(CONFIG_EHCI_INTEP)
    else
    {
        usbcb.irqnum                 = BSP_EHCI_IRQ;
        usbcb.bsp_usb_init           = rtkehci_init;
        usbcb.bsp_usb_enabled        = ehci_usb_enabled;
        usbcb.bsp_usb_disabled       = ehci_usb_disabled;
        usbcb.usb_ep_start_transfer  = rtkehci_ep_start_transfer;
        usbcb.usb_ep0_start_transfer = rtkehci_ep0_start_transfer;
        USBTxSem = OSSemCreate(0);
    }
#endif

    USBQ = OSQCreate(USBMsg,16);
}

int USBDataWebGet(RTSkt* s, INT32U start, INT32U end)
{
    char* httpHdr = (char*)malloc(256);
    unsigned char* tip = (unsigned char*)&(s->pcb->destIPAddr.addr);
    PKT *txPkt;

    memset(httpHdr, 0, 256);
    sprintf(httpHdr, RtHttpHdr, fn, tip[0], tip[1], tip[2], tip[3], start, end);
    txPkt = (PKT*)allocPkt_F(strlen(httpHdr));
    memcpy(txPkt->wp, httpHdr, txPkt->len);

    free(httpHdr);
    return rtSktSend_F(s, txPkt, 0);
}

INT8U getDiskSize(PKT *rxPkt)
{
    INT8U *tmp;

    tmp = strstr((const char *)rxPkt->wp, "Content-Range: bytes");

    if(tmp == NULL)
        return 1;

    tmp = strchr((const char*)tmp, '/');

    if(tmp == NULL)
        return 1;

    tmp++;
    while (((*tmp & 0xf0) == 0x30) && ((*tmp&0x0f) <= 0x09))
    {
        usbcb.disksize = usbcb.disksize *10 + (*tmp - 0x30);
        tmp++;
    }

    return 0;
}

INT8U getWebData(PKT *rxPkt)
{
    char *tmp;
    static INT32S httpContentLen = 0;


    if(!usbcb.disksize)
        getDiskSize(rxPkt);

    //parsing http data
    if (httpContentLen <=0)
    {
        tmp = strstr((const char*)rxPkt->wp, "Content-Length:");

        if (tmp == NULL)
        {
            DEBUGMSG(HTTP_DEBUG, "Error in Content-Length Token\n");
            return 1;
        }

        tmp = strchr((const char *)tmp,' ');

        if (tmp == NULL)
        {
            DEBUGMSG(HTTP_DEBUG, "Error in Content-Length Format\n");
            return 1;
        }

        tmp++;
        httpContentLen = 0;
        while (((*tmp & 0xf0) == 0x30) && ((*tmp&0x0f) <= 0x09))
        {
            httpContentLen = httpContentLen *10 + (*tmp - 0x30);
            tmp++;
        }

        tmp = strstr((const char *)tmp, "\r\n\r\n");

        if (tmp == NULL)
        {
            // break;
            return 1;
        }

        tmp+=4;

        rxPkt->len = rxPkt->len - (tmp - ((char*)rxPkt->wp));
        httpContentLen = httpContentLen - rxPkt->len;
        rxPkt->wp = tmp;
    }
    else
    {
        httpContentLen = httpContentLen - rxPkt->len;
    }

    return 0;
}
INT8U getType(PKT *rxPkt)
{
    INT8U cdtag[7] = {0x01,0x43,0x44,0x30,0x30,0x31,0x01};
    if((memcmp(rxPkt->wp, cdtag, 7) == 0))
    {
        usbcb.usbtype = USB_CDROM;
        usbcb.sectorsize = usb_sector_size[usbcb.usbtype];
        usbcb.usbstate = ENABLED;
        usbcb.waitRx = 0;
        usbcb.bsp_usb_enabled();
        usbcb.usbstate = ENABLED;//+Tom 20141215 add for no device @ S5 boot on
        usbcb.usblen = 7;
        return 0;
    }
    else
    {

        while (USBDataWebGet(usbcs, 0, 511) == SKTDISCONNECT)
        {
            usbcb.usbSktconnected = 0;
            stopconn_F(usbcs);
            usbcs = rtSkt_F(IPv4, IP_PROTO_TCP);
            rtSktEnableTimeWait_F(usbcs);
            while (rtSktConnect_F(usbcs, tip, usbcb.port) == -1)
                OSTimeDly(1);
            usbcb.usbSktconnected = 1;
        }
        return 1;
    }

}

void getMBR(PKT *rxPkt)
{
    static INT16U nowsize = 0;
    INT32U temp;

    if (nowsize < 512) {
        memcpy(usbcb.MBR + nowsize, rxPkt->wp, rxPkt->len);
        nowsize += rxPkt->len;
    }

    if (nowsize == 512)
    {
        usbcb.usbstate = ACCEPTED;
        memcpy(&usbcb.mediatype, usbcb.MBR + 0x1BE, 1);
        //80 means a valid partition table,
        if (usbcb.mediatype != 0x80)
        {
            memcpy(&usbcb.sectors, usbcb.MBR+0x13, 2);
            //if zero, check 0x20

            if (!temp)
                memcpy(&usbcb.sectors, usbcb.MBR + 0x20, 4);
            usbcb.usbtype = USB_FLOPPY;

        }
        else
        {
            memcpy(&temp, (INT8U *) (usbcb.MBR + 0x1C6), 4);
            memcpy(&usbcb.sectors, usbcb.MBR + 0x1CA, 4);
            usbcb.sectors += temp;
            usbcb.usbtype = USB_MSD;

        }
        usbcb.sectorsize = usb_sector_size[usbcb.usbtype];
        usbcb.usbstate = ENABLED;
        usbcb.waitRx = 0;
        usbcb.bsp_usb_enabled();
        usbcb.usbstate = ENABLED;//+Tom 20141215 add for no device @ S5 boot on
        usbcb.usblen = 512;
        nowsize = 0;
    }

}

void sendUSBPkt(PKT *rxPkt)
{
    static INT16U nowsize = 0;
    static INT16U remainsize = 0;

    usbcb.usblen -= rxPkt->len;

    if((usbcb.usblen == 0) && (usbcb.sendlen > 32768) && (usbcb.usbmode != SERVER_MODE))
    {
        usbcb.usbSktconnected = 0;
        stopconn_F(usbcs);
        usbcs = rtSkt_F(IPv4, IP_PROTO_TCP);
        rtSktEnableTimeWait_F(usbcs);
        while (rtSktConnect_F(usbcs, tip, usbcb.port) == -1)
            OSTimeDly(1);
        usbcb.usbSktconnected = 1;
    }

    while (rxPkt->len)
    {

        if (nowsize + rxPkt->len >= 2048)
        {
            remainsize = 2048 - nowsize;
            memcpy(usbcb.rbuf+nowsize, rxPkt->wp, remainsize);
            usbcb.usb_ep_start_transfer(2048, usbcb.rbuf, is_IN );
            nowsize = 0;
            rxPkt->wp += remainsize;
            rxPkt->len -= remainsize;
        }
        else if (nowsize + rxPkt->len >= 1024)
        {
            remainsize = 1024 - nowsize;
            memcpy(usbcb.rbuf+nowsize, rxPkt->wp, remainsize);
            usbcb.usb_ep_start_transfer(1024, usbcb.rbuf, is_IN );
            nowsize = 0;
            rxPkt->wp += remainsize;
            rxPkt->len -= remainsize;
        }
        else if (nowsize + rxPkt->len >= 512)
        {
            remainsize = 512 - nowsize;
            memcpy(usbcb.rbuf+nowsize, rxPkt->wp, remainsize);
            usbcb.usb_ep_start_transfer(512, usbcb.rbuf, is_IN );
            nowsize = 0;
            rxPkt->wp += remainsize;
            rxPkt->len -= remainsize;
        }
        else
        {
            memcpy(usbcb.rbuf+nowsize, rxPkt->wp, rxPkt->len);
            nowsize += rxPkt->len;
            rxPkt->len = 0;
        }
    }
}

void handle_ep_complete()
{
    if (usbcb.epstate == EP_DATA_IN || usbcb.epstate == EP_DATA_OUT)
    {
        ata_csw();
    }
    else if (usbcb.epstate == EP_CSW)
    {
        usbcb.epstate = EP_CBW;
        usbcb.usb_ep_start_transfer(CBWLength, (INT8U *) usbcb.CBW, is_OUT);
    }
}

void USBQHandle(QHdr *rxq)
{

    switch (rxq->cmd)
    {

    case USB_CBW:
        usbcb.usbstate = CONNECTED;
        usbcb.usblen = rxq->length;
        ata_cbw();
        break;


    case USB_STATUS_ERROR:
        ata_csw();
        rlx_irq_set_mask(usbcb.irqnum, OS_FALSE);
        break;

    case USB_BULK_TEST:
        usbcb.usb_ep_start_transfer( BULK_SIZE, usbcb.rbuf, is_IN );
        rlx_irq_set_mask(usbcb.irqnum, OS_FALSE);
        break;
	case USB_CORE_RST:
		core_reset();
		break;

    default :
        break;
    }

    //only when the data is return locally, send the csw immediately
    if(!dpconf->usbotg  && !usbcb.waitRx && usbcb.epstate != EP_CBW)
    {
        ata_csw();
        rlx_irq_set_mask(usbcb.irqnum, OS_FALSE);
    }

}

void stopUSB()
{

    static INT8U enter = 0;
    OS_CPU_SR cpu_sr = 0;
    INT8U status = 0;

    if (enter == 1)
        return;

    OS_ENTER_CRITICAL();
    enter = 1;
    OS_EXIT_CRITICAL();

    usbcb.bsp_usb_disabled();

    if (usbcs)
    {
        usbcs = stopconn_F(usbcs);

        if (usbSrv)
            usbSrv = stopconn_F(usbSrv);
        status = 1;
    }

    if (usbcb.pkt)
        freePkt_F(usbcb.pkt);

    if (usbcb.qhdr)
        free(usbcb.qhdr);


    OS_ENTER_CRITICAL();
    usbcb.usbstate = DISABLED;
    enter = 0;
    OS_EXIT_CRITICAL();

    if (status == 1)
        OSTaskDel(TASK_USB_PRIO);
}

void wait_AHB_idle(void)
{
	INT32U count = 0;
	do
	{
		//AHB master is not idle,
		if(++count > 100000)
		{
			while(1);
			//return;
		}

		bsp_wait(10);
	}
	while(!(REG32(USB_OTG_BASE + GRSTCTL) & GRSTCTL_AHBIdle));	/* ahbidle */
}
