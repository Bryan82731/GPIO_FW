#include "bsp.h"
#include "lib.h"
#include "arp.h"
#include "tcp.h"
#include "wcom.h"
#include "cmac.h"
#include "uart.h"
#include "testio.h"

extern _IPInfo IPInfo;
extern DebugMsg *debugmsg;
extern DPCONF *dpconf;
extern asf_config_t *asfconfig;
INT32U CMAC_STOP = 0;;

void bsp_gmac_reset(INT8U txreset) _ATTRIBUTE_SRAM;

ROM_EXTERN INT8U RxDescIndex;
ROM_EXTERN INT8U TxDescIndex;
ROM_EXTERN OS_EVENT *RxQ;
ROM_EXTERN INT8U gmac_reset;
ROM_EXTERN int OSPushType;

#if CONFIG_VERSION  < IC_VERSION_DP_RevF
static INT8U *lbuffer;
#endif

extern const INT8U macpattern[60];

#if CONFIG_VERSION > IC_VERSION_DP_RevA
ROM_EXTERN OS_EVENT *DASHREQSEM;//Shared resource => Task Request Atomic
ROM_EXTERN OS_EVENT *DASHREQLOCK;//Shared resource => Prevent Dash request(0x81) or ack(0x82) at the smae time
ROM_EXTERN OS_EVENT *DASH_OS_Response_Event;//Event
ROM_EXTERN OS_EVENT *DASH_OS_Push_Event;//Event

ROM_EXTERN unsigned char* DASH_ib_write_buffer;
ROM_EXTERN unsigned char* DASH_OS_PSH_Buf;
#endif

ROM_EXTERN DASHDesc* DASH_ob_request;
ROM_EXTERN DASHDesc* DASH_ib_write;
INT8U PCI_BRIDGE_WAIT = 0;
//static void bsp_gmac_handler(void);

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION <= IC_VERSION_EP_RevB)
void gmac_software_hook(void)
{
    DEBUGMSG(DRIVER_DEBUG, "Get value %x\n", REG8(IOREG_IOBASE + IO_DMEMSTA));
    INT8U swisr;

    swisr = REG8(MAC_BASE_ADDR + MAC_SWISR);

    if (swisr == 0x00)
    {
#if CONFIG_VERSION == IC_VERSION_DP_RevA

        bsp_gmac_reset(0);
#else
        gmac_reset = 1;
#endif
    }
#if 0
    else if (REG8(IOREG_IOBASE + IO_DMEMSTA) == 0x01)
    {   //disable lan
        disable_master_engine();
        REG32(IOREG_IOBASE+MAC_IDR0) = 0x00000000;
        REG32(IOREG_IOBASE+MAC_IDR4) = 0x00000000;
        REG32(IOREG_IOBASE+IO_DIPV4) = 0x00000000;
#if defined(CONFIG_USB_ENABLED) && !defined(CONFIG_BUILDROM)
        usbq = malloc(sizeof(QHdr));
        memset(usbq, 0, sizeof(QHdr));
        usbq->cmd = SRV_STOP;
        OSQPost(USBQ, usbq);
        bsp_usb_disabled();
#endif
        bsp_bits_set(IO_CONFIG1, 0, BIT_DASHEN, 1);
    }
    else if (REG8(IOREG_IOBASE + IO_DMEMSTA) == 0x02 )
    {   //enable lan
#if defined(CONFIG_USB_ENABLED) && !defined(CONFIG_BUILDROM)
        bsp_usb_enabled();
#endif
    }
#endif
    //inband inform OOB that driver is ready to service
    else if (swisr == 0x05 )
    {
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
        bsp_bits_set_mutex(MAC_IBREG, 1, BIT_DRIVERRDY, 1);
#else
        bsp_bits_set(MAC_IBREG, 1, BIT_DRIVERRDY, 1);
#endif

        ///OSTaskResume(TASK_OOBResponse_PRIO);
        IPInfo.DoReloadFilterTable = 1;
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
        if (bsp_bits_get(IO_CONFIG1, BIT_SINGLE_IP, 1) == 0)
        {
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP0, 1);
            bsp_bits_set(MAC_RxCR, 0, BIT_FILTERV4DIP1, 1);
            bsp_bits_set(MAC_RxCR, 0, BIT_FILTERV4DIP2, 1);
        }
#endif
#if CONFIG_mDNS_OFFLOAD_ENABLED
        if (IPInfo.tmpIP)
        {
            setIPAddress(IPInfo.tmpIP);
            IPInfo.tmpIP = 0;
            SetMacAddr(IPInfo.OOBMAR);
        }
#endif
#ifdef CONFIG_8021X_ENABLED
        if (bsp_bits_get(MAC_OOBREG, BIT_SINGLE_IP, 1))
        {
            dpconf->eap = 0;
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
            bsp_bits_set(IO_HWFunCtr, 0, BIT_AAB, 1);
#endif
        }
#endif
    }
    //inband inform OOB that driver is about to leave
    else if (swisr == 0x06 )
    {
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
        bsp_bits_set_mutex(MAC_IBREG, 0, BIT_DRIVERRDY, 1);
#else
        bsp_bits_set(MAC_IBREG, 0, BIT_DRIVERRDY, 1);
#endif
#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
        IPInfo.DoReloadFilterTable = 1;
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
        if (bsp_bits_get(IO_CONFIG1, BIT_SINGLE_IP, 1) == 0)
        {
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP0, 1);
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP1, 1);
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP2, 1);
        }
#endif
#ifdef CONFIG_8021X_ENABLED
        if (bsp_bits_get(MAC_OOBREG, BIT_SINGLE_IP, 1))
        {
            dpconf->eap = 1;
            ///OSTaskResume(TASK_8021X_PRIO);
        }
#endif
#endif
    }
    else if(REG8(IOREG_IOBASE + IO_DMEMSTA) == 0x7)
    {
        PCI_BRIDGE_WAIT = 0;
    }
#if CONFIG_VERSION  == IC_VERSION_DP_RevA
    else if (swisr == 0x10 )
    {
        //PXE want to issue phy parameter
        REG8(TCR_BASE_ADDR + TCR_CONF0) = 0x00;
    }
    else if (swisr == 0x11 )
    {
        //PXE stop writing phy parameter
        REG8(TCR_BASE_ADDR + TCR_CONF0) = 0xE8;
    }
#endif
#if CONFIG_VERSION  > IC_VERSION_DP_RevF
    else if (swisr == 0x25 )
    {
        //CMAC stop
        CMAC_STOP = 1;
        REG32(KVM_BASE + CMAC_SYN_REG) = 0x25;
        REG32(MAC_BASE_ADDR + MAC_OOB2IB_INT) = 1;

    }
    else if (swisr == 0x26 )
    {
        //CMAC init
        CMAC_SW_release();
        bsp_cmac_init();
        CMAC_STOP = 0;
        REG32(KVM_BASE + CMAC_SYN_REG) = 0x26;
        REG32(MAC_BASE_ADDR + MAC_OOB2IB_INT) = 1;
    }
#endif
    //for PG usage
    else if (swisr == 0x50 )
    {
        //PXE want to issue phy parameter
        //REG8(TCR_BASE_ADDR + TCR_CONF0) = 0x00;
#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
        bsp_bits_set(IO_CONFIG1, 0, BIT_DASHEN, 1);
#else
        bsp_bits_set(MAC_OOBREG, 1, BIT_DASHEN, 1);
#endif
        tcrread();
    }
    else if (swisr == 0x51 )
    {
        //PXE stop writing phy parameter
        REG8(TCR_BASE_ADDR + TCR_CONF0) = 0xE8;
    }
    else if (swisr == 0x20 )
    {
        //Change Host Name ISR
        setHostName(0);
    }
    else if (swisr == 0x21 )
    {
        GetMacAddr_F((void *) dpconf->srcMacAddr[eth0], eth0);
    }
#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    else if (swisr == 0x30)
    {
        key_reload();
    }
#endif

//there is no DIPv4 regiser for IB to access in EP
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
    //linux inform DP to set MAC/IP Address
    else if (swisr == 0x41)
    {
        GetMacAddr_F((void *) dpconf->srcMacAddr[intf]);
        setIPAddress(REG32(IOREG_IOBASE+IO_DIPV4));
        DEBUGMSG(MESSAGE_DEBUG, "IP:%x MAC:%02x%02x%02x%02x%02x%02x\n", REG32(IOREG_IOBASE+IO_DIPV4),dpconf->srcMacAddr[intf][0],dpconf->srcMacAddr[intf][1],dpconf->srcMacAddr[intf][2], dpconf->srcMacAddr[intf][3],dpconf->srcMacAddr[intf][4],dpconf->srcMacAddr[intf][5] );
    }
#endif
    else if (swisr == 0x42)
    {
        _boot();		
    }
	
	if(pci_sw_isr)
		pci_sw_isr();
}
#endif

#if (CONFIG_VERSION  <= IC_VERSION_DP_RevF) && (CONFIG_VERSION  > IC_VERSION_DP_RevA)
void gmac_pcie_hook(void)
{
    //INT8U dhcpstate;
    //static INT8U provstate;
    //INT32U ipaddr;
    //INT8U count = 0;
    OSOOBHdr *hdr;
    //indicate fw is busy handling pci-e request from inband or pxe
    //REG16(IOREG_IOBASE + IO_CONFIG) &= ~0x1000;

    OSPushType = REG8(IOREG_IOBASE + IO_HREQ);

    if (OSPushType == 0x85) //host request write, pxe write for dpconf
    {
        bsp_bits_set(IO_CONFIG1, 0, BIT_DASHEN, 1);
        hdr = (OSOOBHdr *) DASH_OS_PSH_Buf;
        hdr->type = OOB_SET_DPCONF;
        hdr->flag = PCIE_NO_ACK;
        DASH_ib_write->len = DASHOSPSHLEN;

        DASH_ib_write->lowAddr = (char *)VA2PA(DASH_OS_PSH_Buf) + sizeof(OSOOBHdr);
        DASH_ib_write->own = 1;
        REG8(IOREG_IOBASE+MAC_TPPoll) = 0x40;

        //dhcpstate   = dpconf->DHCPv4Enable;
        //ipaddr = dpconf->HostIP[intf].addr;
        //bsp_bits_set(IO_CONFIG1, 0, BIT_DASHEN, 1);

        //pxe could at most transfer 0x5A size
        ///memset(dpconf->admin.name, 0, 16);
        ///memset(dpconf->admin.passwd, 0, 16);
#if 0
        DASH_ib_write->len = sizeof(*dpconf);
        DASH_ib_write->lowAddr = (INT8U *) VA2PA(dpconf);
        DASH_ib_write->own = 1;
        REG8(IOREG_IOBASE+MAC_TPPoll) = 0x40;


        while(DASH_ib_write->own && count++ < 100)
            bsp_wait(2);

        //time out
        if(count >= 100)
            return;
#endif

#if 0
        if (!isprint(dpconf->admin.name[0]))
        {
            memcpy((void *) dpconf->admin.name, "Administrator", 13);
            memcpy((void *) dpconf->admin.passwd, "Realtek", 7);
        }
        dpconf->admin.opt   = 1;
        dpconf->admin.role  = 0x7F;
        dpconf->admin.crc16 = inet_chksum((void *) &dpconf->admin,34);
#endif

        //inituser has already done it
        //copy to the crc filed, others remains
        //memcpy(getuserinfo(), &dpconf->admin, 36);

#if 0
        if (ipaddr != dpconf->HostIP[intf].addr)
        {
            setIPAddress_F(htonl(dpconf->HostIP[intf].addr));
            setSubnetMask_F(htonl(dpconf->SubnetMask.addr));
        }


        if (dhcpstate == 0  && dpconf->DHCPv4Enable)
        {
            dpconf->DHCPv4State = InitState;
            setIPAddress_F(0);
            OSTaskResume(TASK_DHCPv4_PRIO);
        }
        setHostName((void *) dpconf->HostName);

        if (fwsig->SIG == NORMAL_SIG && fwsig->ProvISRHook == NEED_PATCH)
            fwsig->ProvISRHook_F(provstate);
#endif
        /*
        #ifdef CONFIG_PROVISIONING
                    dpconf->PKIEnabled = 1;
                    dpconf->PSKEnabled = 1;
                    memset(&soapcb, 0, sizeof(SOAPCB));
                    if (dpconf->ProvisioningMode == SMALL_BUSINESS)
                    {
                        dpconf->ProvisioningState = PROVISIONED;
                        dpconf->asfon = 1;
                        dpconf->httpService = 1;
                        dpconf->httpsService = 1;
                        dpconf->EchoService = 1;
                    }
                    else if (dpconf->ProvisioningMode == ENTERPRISE)
                    {
                        if (dpconf->ProvisioningState == FULL_UNPROVISIONED
                                && provstate == FULL_UNPROVISIONED)
                        {
                            setreset(DPCONFIGTBL);
                        }
                        else if (dpconf->ProvisioningState == PARTIAL_UNPROVISIONED
                                 && provstate == PARTIAL_UNPROVISIONED)
                        {
                            setreset(DPCONFIGTBL);
                        }
                        else if (dpconf->ProvisioningState == FULL_UNPROVISIONED )
                        {
                            full_unprovision();
                            setdirty(DPCONFIGTBL);
                            setdirty(USERTBL);
                            setdirty(EVENTTBL);
                            setreset(EVENTTBL);
                        }
                        else if (dpconf->ProvisioningState == PARTIAL_UNPROVISIONED)
                        {
                            partial_unprovision();
                            setreset(DPCONFIGTBL);
                        }
                    }
        #endif
        */
        //setwait(DPCONFIGTBL);
        //setdirty(DPCONFIGTBL);
        ///OSTimeDlyResume(TASK_FLASH_PRIO);
    }
    /*
        else if (REG8(IOREG_IOBASE + IO_HREQ) == 0x93) //acpi header
        {
        }
        else if (REG8(IOREG_IOBASE + IO_HREQ) == 0x92) //acpi data
        {
        }
    */

//original using PXE to get SMBIOS data
#if CONFIG_VERSION  > IC_VERSION_DP_RevA
#ifdef  CONFIG_PXE_SMBIOS
    else if (OSPushType == 0x83) //smbios header
    {
#ifndef CONFIG_VENDOR_FSC
        bsp_set_sstate(S_S0);
#endif
        DASH_ib_write->len = 32;
        DASH_ib_write->lowAddr = (INT8U *) VA2PA(smbiosptr);
        DASH_ib_write->own = 1;
        REG8(IOREG_IOBASE+MAC_TPPoll) = 0x40;
    }
    else if (OSPushType == 0x82) //smbios data
    {
#ifndef CONFIG_VENDOR_FSC
        bsp_set_sstate(S_S0);
#endif
        DASH_ib_write->len = *(INT16U *) (smbiosptr + 0x16);
        DASH_ib_write->lowAddr = (INT8U *) VA2PA(smbiosptr+32);
        DASH_ib_write->own = 1;
        REG8(IOREG_IOBASE+MAC_TPPoll) = 0x40;
        dirty[SMBIOSTBL].length = *(INT16U *) (smbiosptr + 0x16) + SMBIOS_DATA_OFFSET + SMBIOS_HEADER_SIZE;
        //clear the buffer to prevent pollution
        memset(timestampdataptr + dirty[SMBIOSTBL].length, 0 , MAX_SMBIOS_SIZE-dirty[SMBIOSTBL].length);
        setdirty(SMBIOSTBL);
    }
    else if (OSPushType == 0x7F) //request debug information
    {
        DEBUGMSG(DRIVER_DEBUG, "InBand Request RxTotal:%d TxTotal:%d DrvDrop:%d DrvBusy:%d TDU:%d RDU:%d\n", debugmsg->RxTotal, debugmsg->TxTotal, debugmsg->DrvDrop, debugmsg->DrvBusy, debugmsg->TDU, debugmsg->RDU);
        DASH_ib_write->len = 512;
        DASH_ib_write->lowAddr = (INT8U *) VA2PA(debugmsg);
        DASH_ib_write->own = 1;
        REG8(IOREG_IOBASE+MAC_TPPoll) = 0x40;
    }
#endif
    else if (OSPushType == 0x02) //pxe read
    {
        //provstate = dpconf->ProvisioningState;
        DASH_ib_write->len = sizeof(*dpconf);
        DASH_ib_write->lowAddr = (INT8U *) VA2PA(dpconf);
        DASH_ib_write->own = 1;
        REG8(IOREG_IOBASE+MAC_TPPoll) = 0x40;
    }
    //OS Write
    else if (OSPushType == 0x91)
    {
        DASH_ib_write->own = 1;
        DASH_ib_write->len = 1024;
        if (DASH_ib_write_buffer != 0)
        {
            DASH_ib_write->lowAddr = (char *)VA2PA(DASH_ib_write_buffer);
        }
        else
        {
        }
        //already initialized in gmac_init
        //REG32(IOREG_IOBASE + IO_DSDSAR) = VA2PA(DASH_ib_write);
        REG8(IOREG_IOBASE+MAC_TPPoll) = 0x40;
    }
    else if (OSPushType == 0x92)
    {
        DASH_ib_write->own = 1;
        DASH_ib_write->len = DASHOSPSHLEN;
        DASH_ib_write->lowAddr = (char *)VA2PA(DASH_OS_PSH_Buf);
        REG8(IOREG_IOBASE+MAC_TPPoll) = 0x40;
    }
    else if (OSPushType == 0x06) //pxe read asf! table
    {
        DASH_ib_write->len = sizeof(*asfconfig);
        DASH_ib_write->lowAddr = (INT8U *) VA2PA(asfconfig);
        DASH_ib_write->own = 1;
        REG8(IOREG_IOBASE+MAC_TPPoll) = 0x40;
    }
    else if (OSPushType == 0x86) //host request write, pxe write for dpconf
    {
        hdr = (OSOOBHdr *) DASH_OS_PSH_Buf;
        hdr->type = OOB_SET_ASFTBL;
        hdr->flag = PCIE_NO_ACK;
        DASH_ib_write->len = DASHOSPSHLEN;

        DASH_ib_write->lowAddr = (char *)VA2PA(DASH_OS_PSH_Buf) + sizeof(OSOOBHdr);
        DASH_ib_write->own = 1;
        REG8(IOREG_IOBASE+MAC_TPPoll) = 0x40;
    }
    //OS Write
    //OS Write
#endif
}
#endif

/*
 *******************************************************************************
 * bsp_gmac_handler()
 *
 * Description: GMAC interrupt handler.
 *
 * Arguments  : None
 *
 * Returns    : None
 *******************************************************************************
 */
//static void bsp_gmac_handler(void)
#if  (CONFIG_VERSION  < IC_VERSION_EP_RevB ) || defined(CONFIG_BUILDROM)
void bsp_gmac_handler(void)
{
    volatile INT16U val, extisr;
    INT32U state;
#if defined(CONFIG_USB_ENABLED) && !defined(CONFIG_BUILDROM)
    QHdr *usbq;
#endif
    //INT8U httpservice;

    rxdesc_t *rxdesc;
    INT8U i;



#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
    REG16(IOREG_IOBASE+MAC_IMR) &= ~0xE06F;
    val = REG16(IOREG_IOBASE+MAC_ISR);
    REG16(IOREG_IOBASE+MAC_ISR) = (val & ~0x02);
#else
    REG16(MAC_BASE_ADDR + MAC_IMR) = 0;
    val = REG16(MAC_BASE_ADDR + MAC_ISR);
    REG16(MAC_BASE_ADDR + MAC_ISR) = val;

    extisr = REG16(MAC_BASE_ADDR + MAC_EXT_INT);

    //isolate int
    if((extisr & 0x01))
    {
        state = REG32(MAC_BASE_ADDR + MAC_EXT_INT) & 0x00010000;
        if(state)
            bsp_set_sstate(S_S0);
        else
            bsp_set_sstate(S_S5);

    }
#endif


    //software interrupt
    if ((val & 0x40) != 0)
        gmac_software_hook_F();

    if ((val & 0x01) != 0)
#if (CONFIG_VERSION != IC_VERSION_EP_RevA)
        rx_recycle_F();
#else

        rx_recycle_patched();
#endif

#if  (CONFIG_VERSION >  IC_VERSION_DP_RevA) && (CONFIG_VERSION  <= IC_VERSION_DP_RevF)
    if (( val & REQDMODE ) !=0 )
    {
        gmac_pcie_hook_F();
    }

    else if (( val & DASHOK ) !=0 )
    {
        //fw again is ready to accept pci-e request
        OSSemPost(DASHREQLOCK);
    }

    else if (( val & HOSTOK ) !=0)
    {
        //only post for inband write
        if(OSPushType == 0x91)
            OSSemPost(DASH_OS_Response_Event);
        else if(OSPushType == 0x92 || OSPushType == 0x85 || OSPushType == 0x86)
            OSSemPost(DASH_OS_Push_Event);
    }

#endif

    if ((val & 0x04) != 0)
        debugmsg->TxTotal++;
    if ((val & 0x08) != 0)
        debugmsg->TDU++;
    if ((val & 0x20) != 0)
    {
#if CONFIG_mDNS_OFFLOAD_ENABLED
        //Skip sleep link chage
        if (!bsp_AP_exist())
#endif
            IPInfo.LinkChange = 1;
        //bsp_gphy_set();
    }
    if ((val & 0x10) != 0)
        DEBUGMSG(DRIVER_DEBUG, "FIFO overflow\n");
    if ((val & 0x02) != 0)
    {
        for (i=0 ; i < RxdescNumber; i++)
        {
            rxdesc = (rxdesc_t *) RxdescStartAddr + i;
            rxdesc->offset0.cmd.own = 1;
            rxdesc->offset0.cmd.buffer_size = BUFF_SIZE;
        }
        debugmsg->RDU++;

        REG16(IOREG_IOBASE+MAC_ISR) |= 0x02;
        DEBUGMSG(DRIVER_DEBUG, "RDU happens\n");
    }
	
	#if 0
	if((val & ACCRISC_bit) != 0)
	{
		IB_Access_OOB_Handler();
	}
	#endif

#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
    REG16(IOREG_IOBASE+MAC_IMR) = 0xE06F;
#else
    REG16(MAC_BASE_ADDR + MAC_IMR) = DEFAULT_IMR;
    //clear state at last to avoid transient change
    REG16(MAC_BASE_ADDR + MAC_EXT_INT) = extisr;
#endif


}
#endif

#if CONFIG_VERSION  < IC_VERSION_DP_RevF
void bsp_lbtest()
{
    if (dpconf->HostIP[intf].addr == 0)
        return;

    //when IB exist, no need to do loop back test
    if (bsp_bits_get(IO_CONFIG1, BIT_DRIVERRDY, 1))
        return;

    rxdesc_t * rxdesc = (rxdesc_t *) RxdescStartAddr ;
    INT8U *ptr = (INT8U *) PA2VA(rxdesc->rx_buf_addr_low);
    INT16U count = 0;
    //INT32U RxCRVal;

    REG32(IOREG_IOBASE+MAC_TxCR)   = 0x00000B00;
    //RxCRVal = REG32(IOREG_IOBASE + MAC_RxCR);
    //REG32(IOREG_IOBASE+MAC_RxCR)   &= ~0x4000000;

    while (1)
    {
        rxdesc = (rxdesc_t *) RxdescStartAddr ;

        send(lbuffer, 60,  OPT_TCPCS,  0 );

        //wait Rx complete
        count = 0;
        while (rxdesc->offset0.st.own && count < 1000)
        {
            bsp_wait(10);
            count++;
            //wait 10 mesc at most
        }

        if (count != 1000 && memcmp(ptr+54, lbuffer+54, sizeof(macpattern-54)) == 0)
        {
            rxdesc->offset0.st.own = 1;
            rxdesc->offset0.cmd.buffer_size = BUFF_SIZE;
            RxDescIndex = 1;
            TxDescIndex = 1;
            break;
        }
        else
            bsp_gmac_reset(1);

    }

    //REG32(IOREG_IOBASE + MAC_RxCR) = RxCRVal;
    REG32(IOREG_IOBASE+MAC_TxCR)   &= ~0x00000800;
}
#endif

/*
 *******************************************************************************
 * bsp_gmac_init()
 *
 * Description: GMAC initializer
 * Arguments  : None
 * Returns    : None
 *******************************************************************************
 */
#if  (CONFIG_VERSION  < IC_VERSION_EP_RevB ) || defined(CONFIG_BUILDROM)
void bsp_gmac_init(void)
{
    INT32S i;
    INT32U macaddrval;

    //do_a_patch();
#if 0
//#ifdef VERSIONB
    INT32S temp;
    INT8U loop_count = 0;
    //modify voltage
    REG32(IOREG_IOBASE + IO_PHY)  = 0x801f0002;//switch page 2
    bsp_wait(20000);
    REG32(IOREG_IOBASE + IO_PHY)  = 0x00000000;
    bsp_wait(20000);
    temp=REG32(IOREG_IOBASE + IO_PHY);
    temp=temp&0x0000ffff;
#if 0
    // To recognize 8111DP Hardware Version ID,
    // 8111DP Version A ,DP_HVID = 0x00
    // 8111DP Version B ,DP_HVID = 0x01
    // 8111DP Version C ,DP_HVID = 0x02......
    DP_HVID = REG32(IOREG_IOBASE + IO_DTCCR) & 0x7F;
#endif
    // To avoid gphy not ready, jerome suggest!!
    // Gphy register page 2 offet 0x00 default value = 0x86A0.
    // [Reason]CPU function work more early than gphy function.
    while (temp != 0x000086a0 && loop_count<2)
    {
        REG32(IOREG_IOBASE + IO_PHY)  = 0x00000000;
        bsp_wait(20000);
        temp=REG32(IOREG_IOBASE + IO_PHY);
        temp=temp&0x0000ffff;
        loop_count++;
    }

    if (temp==0x000086a0)
    {
        REG32(IOREG_IOBASE + IO_PHY)  = 0x800085A0;
        bsp_wait(30000);
        REG32(IOREG_IOBASE + IO_PHY)  = 0x800084A0;
        bsp_wait(30000);
        REG32(IOREG_IOBASE + IO_PHY)  = 0x800083A0;
        bsp_wait(30000);
        REG32(IOREG_IOBASE + IO_PHY)  = 0x800082A0;
        bsp_wait(30000);
        REG32(IOREG_IOBASE + IO_PHY)  = 0x800081A0;
        bsp_wait(30000);
        REG32(IOREG_IOBASE + IO_PHY)  = 0x800080A0;
        bsp_wait(30000);
        REG32(IOREG_IOBASE + IO_PHY)  = 0x8001c270;
        bsp_wait(30000);
        REG32(IOREG_IOBASE + IO_PHY)  = 0x801f0005;//switch page 5
        bsp_wait(30000);
        REG32(IOREG_IOBASE + IO_PHY)  = 0x80010340;//disable micro c interrupt
        bsp_wait(30000);
        REG32(IOREG_IOBASE + IO_PHY)  = 0x801f0000;
        bsp_wait(30000);
        REG8(0xb9000000 + 0x04)  = 0x00;//speed up cpu
        bsp_wait(30000);
        REG16(IOREG_IOBASE + IO_CONFIG) |= (0x0200);// ocp register offset 0x11 bit2
        REG16(IOREG_IOBASE + IO_CONFIG)&= ~(0x0200);// ocp register offset 0x11 bit2, patch in band and oob access gphy bug
        //REG8(IOREG_IOBASE + 0x11)  = 0x02;
        //REG8(IOREG_IOBASE + 0x11)  = 0x00;
    }
#endif


    //move to bsp_setup
    //bsp_bits_set(IO_CONFIG1, 1, BIT_DASHEN, 1);

#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
    if (dpconf->ipmode != DUAL_IP_MODE)
    {
        macaddrval = ReadIBReg(0xC000);
        memcpy(dpconf->srcMacAddr[eth0], &macaddrval, 4);
        macaddrval = ReadIBReg(0xC004);
        memcpy(dpconf->srcMacAddr[eth0]+4, &macaddrval, 2);
        SetMacAddr(dpconf->srcMacAddr[eth0]);
    }
#endif


#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
    REG8(MAC_BASE_ADDR + MAC_CMD)=0x0;
#endif

    for (i=0 ; i < TxdescNumber; i++)
    {
        //Tx desciptor setup
        if (i == (TxdescNumber-1))
            REG32(TxdescStartAddr+i*16) = 0x70000000;
        else
            REG32(TxdescStartAddr+i*16) = 0x30000000;
        REG32(TxdescStartAddr+i*16+4) = 0x0;
        //zero copy, no need to set tx buffers
        REG32(TxdescStartAddr+i*16+12) = 0x0;
    }

    for (i=0 ; i < RxdescNumber; i++)
    {
        //Rx descriptor setup

        if (i == (RxdescNumber - 1))
            REG32(RxdescStartAddr+i*16) = 0xC0000600;
        else
            REG32(RxdescStartAddr+i*16) = 0x80000600;
        REG32(RxdescStartAddr+i*16+4) = 0x0;

        REG32(RxdescStartAddr+i*16+8) = VA2PA(malloc(MAX_PKT_SIZE)) + PKT_ALIGN_SIZE ;
        REG32(RxdescStartAddr+i*16+12) = 0x0;
    }


#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    if (dpconf->ipmode == DUAL_IP_MODE)
    {
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
        bsp_bits_set(MAC_OOBREG, 0, BIT_SINGLE_IP, 1);
#else
        bsp_bits_set(IO_CONFIG, 0, BIT_SINGLE_IP, 1);
#endif
        if (check_mac_addr((void *) dpconf->srcMacAddr[eth0]))
        {
            i = REG32(MAC_BASE_ADDR + MAC_IDR0);
            i = i | 0x00000010;
            REG32(MAC_BASE_ADDR + MAC_IDR0) = i;
            memcpy(dpconf->srcMacAddr[eth0], &i, 4);
            i = REG32(MAC_BASE_ADDR + MAC_IDR4);
            memcpy(dpconf->srcMacAddr[eth0] + 4, &i, 2);
        }
        else
        {
            SetMacAddr(dpconf->srcMacAddr[eth0]);
        }
    }
    else
    {
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
        bsp_bits_set(MAC_OOBREG, 1, BIT_SINGLE_IP, 1);
#else
        bsp_bits_set(IO_CONFIG, 1, BIT_SINGLE_IP, 1);
        GetMacAddr_F((void *) dpconf->srcMacAddr[eth0], eth0);
#endif

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
        //disable normal lanwake function in single IP mode
        //bsp_bits_set(IO_CONFIG, 1, BIT_LANWAKE_IB_DIS, 1);
        bsp_bits_set(MAC_LAN_WAKE, 1, BIT_LANWAKE_IB_DIS, 1);
#endif
    }

#else
    //Assign MAC ID for dual IP/MAC case, for revision < RTL8111DP-VC
    SetMacAddr(dpconf->srcMacAddr[eth0]);
#endif

    sha1((void *)dpconf->srcMacAddr[eth0], sizeof(dpconf->srcMacAddr[eth0]), (void *) dpconf->UUID);

    REG32(IOREG_IOBASE+IO_DIPV4) = htonl(dpconf->HostIP[eth0].addr);

    REG32(MAC_BASE_ADDR + MAC_MAR0)   = 0x0;
    REG32(MAC_BASE_ADDR + MAC_MAR4)   = 0x0;

    //Assign Tx and Rx descriptor address
    REG32(MAC_BASE_ADDR + MAC_RDSAR)  = VA2PA(RxdescStartAddr);
    REG32(MAC_BASE_ADDR + MAC_TNPDS)  = VA2PA(TxdescStartAddr);

#if (CONFIG_VERSION > IC_VERSION_DP_RevA) && (CONFIG_VERSION <= IC_VERSION_DP_RevF)
    //DMEM Master & Slave descriptor set
    REG32(IOREG_IOBASE + IO_DSDSAR) = VA2PA(DSDescStartAddr);
    DASH_ib_write = (DASHDesc* ) DSDescStartAddr;
    REG32(IOREG_IOBASE + IO_DMDSAR) = VA2PA(DMDescStartAddr);
    DASH_ob_request = (DASHDesc* ) DMDescStartAddr;
    ///REG32(DMEMdescStartAddr + 0x08) = VA2PA(smbiosdataptr);
#endif

    //enable hw checksum
    REG16(MAC_BASE_ADDR + MAC_CPCR)   = 0x0021;
    //set interrupt timer unit

#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
#if CONFIG_VERSION == IC_VERSION_DP_RevA
    REG8(IOREG_IOBASE+MAC_CMD)     &= 0x80;
    REG8(IOREG_IOBASE+MAC_CMD)     |= 0x10;
#else
    REG8(IOREG_IOBASE+MAC_CMD)     = 0x10;
#endif

    REG16(IOREG_IOBASE+MAC_IMR)   &= ~0xE06F;
#else
    /* Accept Bro,Multi, and physical match packets. Reject flow control, error, runt packets */
    REG32(MAC_BASE_ADDR + MAC_RxCR) |= 0x0E;
    //set RMS
    ///REG32(MAC_BASE_ADDR + MAC_RxCR) |= 0x06000000;
    REG16(MAC_BASE_ADDR + MAC_IMR)    = 0x0000;
#endif

    REG16(IOREG_IOBASE+MAC_ISR)    = 0xFFFF;

#ifdef GMAC_LOOP_BACK
    REG32(IOREG_IOBASE+MAC_TxCR)   = 0x00000B00;
#endif

    //Each Tx/Rx would issue a interrupt
    //REG16(IOREG_IOBASE+IO_MITI)   = 0x5050;

#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
    if (dpconf->IPv6Enable)
    {
        REG32(IOREG_IOBASE+MAC_RxCR)   = 0x80000300; //default is 0x30E
    }

    if (dpconf->IPv4Enable)
    {
        REG32(IOREG_IOBASE+MAC_RxCR)   = 0x40000300; //default is 0x30E
    }

#if CONFIG_VERSION == IC_VERSION_DP_RevA
    REG8(IOREG_IOBASE+MAC_CMD)     &= 0x80;
    REG8(IOREG_IOBASE+MAC_CMD)     |= 0x0E;
#else
    if (dpconf->ipmode == DUAL_IP_MODE)
        REG8(IOREG_IOBASE+MAC_CMD)     = 0x0E;
    else
        REG8(IOREG_IOBASE+MAC_CMD)     = 0x0C;
#endif

    REG32(IOREG_IOBASE+MAC_RxCR)   |= 0x0000000E; //default is 0x30E
#endif

    //check EP case?
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
    //in-band exist, but firmware started later
    if (bsp_bits_get(MAC_IMR, BIT_IN_BAND, 1))
    {
        bsp_bits_set(IO_CONFIG1, 1, BIT_DRIVERRDY, 1);
        IPInfo.DoReloadFilterTable = 1;
        if (bsp_bits_get(IO_CONFIG1, BIT_SINGLE_IP, 1) == 0)
        {
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP0, 1);
            bsp_bits_set(MAC_RxCR, 0, BIT_FILTERV4DIP1, 1);
            bsp_bits_set(MAC_RxCR, 0, BIT_FILTERV4DIP2, 1);
        }
    }
    else
    {
        bsp_bits_set(IO_CONFIG1, 0, BIT_DRIVERRDY, 1);
        IPInfo.DoReloadFilterTable = 1;
        if (bsp_bits_get(IO_CONFIG1, BIT_SINGLE_IP, 1) == 0)
        {
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP0, 1);
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP1, 1);
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP2, 1);
        }

    }

#endif

#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
    IPInfo.DoReloadFilterTable = 1;
#endif

    //GPHY auto neogoiation
    //Open 1000 Full Capability
#if 0
    bsp_gphy_write(0x801F0000);
    bsp_gphy_write(0x80090300);
    //go back to page 0
    bsp_gphy_write(0x801F0000);

    //GPHY auto neogoiation
    bsp_gphy_write(0x80009200);

#endif
    //wait for phy to ready

//may cause bios to hang
#if 0
#ifndef CONFIG_VENDOR_FSC
    bsp_wait(2000000);
#endif
#endif

    /**Dash sw init**/
#if (CONFIG_VERSION > IC_VERSION_DP_RevA)
//#if (CONFIG_VERSION > IC_VERSION_DP_RevA) && (CONFIG_VERSION <= IC_VERSION_DP_RevF)
    DASHREQSEM = OSSemCreate(1);
    DASHREQLOCK = OSSemCreate(1);
    DASH_OS_Response_Event = OSSemCreate(0);
    DASH_OS_Push_Event = OSSemCreate(0);
    DASH_OS_PSH_Buf = (unsigned char*) malloc(DASHOSPSHLEN);
#endif

    //DASH_ob_request = (DASHDesc*) malloc(sizeof(DASHDesc));
    ///DASH_ib_write = (DASHDesc*) malloc(sizeof(DASHDesc));

    //since only 1 session is allowed
    //DASH_ib_write is assigned to a fixed address: DSDescStartAddr
    //DASH_ob_request is assigned to a fixed address: DMDescStartAddr


//#if CONFIG_VERSION > IC_VERSION_DP_RevA
#if (CONFIG_VERSION > IC_VERSION_DP_RevA) && (CONFIG_VERSION <= IC_VERSION_DP_RevF)

#if CONFIG_VERSION  == IC_VERSION_DP_RevF
    //enable dmem bridge
    bsp_bits_set(IO_PortCtr, 1, BIT_PCIEBDGEN, 1);
    //set the phy clear bit to 1
    bsp_bits_set(IO_PortCtr, 1, BIT_PHYCLR, 1);
    //Disable OOB Filter
    bsp_bits_set(IO_PortCtr, 1, BIT_OOBFILTERDISABLE, 1);

    reloadFilterTable_F();
#else
    bsp_bits_set(IO_CONFIG1, 1, BIT_PCIEBDGEN, 1);
    //set the phy clear bit to 1
    bsp_bits_set(IO_CONFIG1, 1, BIT_PHYCLR, 1);
#endif

#endif
    //indicate fw is ready to accept pci-e bridge
    bsp_bits_set(MAC_OOBREG, 1, BIT_FIRMWARERDY, 1);

    //if (dpconf->vendor == EVERLIGHT)
#ifdef CONFIG_GPIO_DEBUG
    {
        GPIOSet(1);
        GPIOEnable();
    }
#endif

#if CONFIG_VERSION  < IC_VERSION_DP_RevF
    bsp_lbtest();
#endif

    //for version D, to accept All broadcast packets
    //handling NetBIOS protocol
#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
    //accept all

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
    bsp_bits_set(IO_HWFunCtr, 1, BIT_AAB, 1);
#endif

    if (bsp_bits_get(IO_CONFIG, BIT_SINGLE_IP, 1))
    {
        bsp_bits_set(IO_HWFunCtr, 1, BIT_FILTERMACIP, 1);
    }
    else
        //enable OOB port filter 0->Enable, 1 -> Do not care
        bsp_bits_set(IO_HWFunCtr, 0, BIT_FILTERMACIP, 1);

    //enable RMCP UDP
    //bsp_bits_set(IO_HWFunCtr, 1, BIT_FILTERRMCP, BITS_SET);


#else
    bsp_bits_set(IO_CONFIG2, 1, BIT_AAB, 1);
#endif
#endif

    gmac_reset = 0;
    rlx_irq_register(BSP_GMAC_IRQ, bsp_gmac_handler);

#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
    REG16(IOREG_IOBASE+MAC_IMR)    = 0xE06F;
#else
    REG16(MAC_BASE_ADDR + MAC_IMR)    = DEFAULT_IMR;


    //Check SYNC RXEN and SYNC TXEN
    while(REG16(MAC_BASE_ADDR + MAC_CMD) & 0x300)
        ;

    /* Enable TX/RX */
    REG8(MAC_BASE_ADDR+MAC_CMD)=0x0C;

#endif

}
#endif


#if (CONFIG_VERSION >= IC_VERSION_EP_RevA)
int bsp_inband_push_ack()
{
    unsigned char err = 0;
    unsigned char *data = malloc(DASHOSPSHLEN);
    int status = PCI_DMA_OK;
    OSOOBHdr* hdr =(OSOOBHdr*) data;
    int i = 0;

    //Driver ready(phy token) and AP must ready(pci bridge token 1)
    if (!bsp_AP_exist())
    {
        free(data);
        return AP_NOT_READY;
    }

    while(CMAC_STOP)
    {
        i++;
        OSTimeDly(1);
    }

    if(i >= 10)
    {
        return AP_NOT_READY;;
    }

    i = 0;

    OSSemPend(DASHREQLOCK, 0, &err);

    hdr->len = 0;
    hdr->type = OSPUSHDATA;
    hdr->hostReqV = 0x92;

    bsp_cmac_send(data, DASHOSPSHLEN);

    OSSemPend(DASHREQLOCK, OOBMWT, &err);
    if (err == OS_ERR_TIMEOUT )
    {
        DEBUGMSG(WCOM_DEBUG,"bsp_inband_push_ack time out\n");
        status = PCI_BRIDGE_FAIL;
        OOBHwRest();
    }
    OSSemPost(DASHREQLOCK);
    free(data);
    return status;

}

/*
	1.Send data to inband
	2.Wait for TX OK
	3.Wait for IB send response data.
*/
int bsp_get_inband_data(char* data, int len)
{
    unsigned char err = 0;
    int status = PCI_DMA_OK;
    int i = 0;
    OSOOBHdr* hdr =(OSOOBHdr*) data;

#if 0
//SD3 test
    bsp_cmac_send(data, len);
    return;
#endif
    if (!bsp_AP_exist())
        return AP_NOT_READY;

    while(CMAC_STOP)
    {
        i++;
        OSTimeDly(1);
    }

    if(i >= 10)
    {
        return AP_NOT_READY;;
    }

    i = 0;

    OSSemPend(DASHREQSEM, 0, &err);
    OSSemPend(DASHREQLOCK, 0, &err);
    DASH_ib_write_buffer = data;

    hdr->hostReqV = 0x91;
    bsp_cmac_send(DASH_ib_write_buffer, len);

    OSSemPend(DASHREQLOCK, OOBMWT, &err);
    if (err == OS_ERR_TIMEOUT )
    {
        status = PCI_BRIDGE_FAIL;
        OOBHwRest();
        DEBUGMSG(WCOM_DEBUG, "bsp_get_inband_data TIMEOUT 1\n");
    }

    OSSemPost(DASHREQLOCK);

    if (status == PCI_DMA_OK)
    {
        OSSemPend(DASH_OS_Response_Event, OOBMWT, &err);

        if (err == OS_ERR_TIMEOUT )
        {
            DEBUGMSG(WCOM_DEBUG, "bsp_get_inband_data TIMEOUT 2\n");
            status = AP_NOT_RESPONSE;
        }
    }

    DASH_ib_write_buffer = 0;
    OSSemPost(DASHREQSEM);

    return status;
}


#elif (CONFIG_VERSION > IC_VERSION_DP_RevA)
int bsp_inband_push_ack()
{
    unsigned char err = 0;
    unsigned char *data = malloc(DASHOSPSHLEN);
    int status = PCI_DMA_OK;
    OSOOBHdr* hdr =(OSOOBHdr*) data;
    int i = 0;

    //Driver ready(phy token) and AP must ready(pci bridge token 1)
    if (!bsp_AP_exist())
    {
        free(data);
        return AP_NOT_READY;
    }

    i = 0;
    while(PCI_BRIDGE_WAIT && i < 10)
    {
        OSTimeDly(OS_TICKS_PER_SEC * 0.1);
        i++;
    }
    i = 0;
    PCI_BRIDGE_WAIT = 1;

    OSSemPend(DASHREQLOCK, 0, &err);

    while (((DASH_ob_request->own == 1) || (REG8(IOREG_IOBASE + IO_DASH_REQUEST) != 0)) && i < 10)
    {
        OSTimeDly(OS_TICKS_PER_SEC * 0.5);
        i++;
    }

    if (i == 10)
        DEBUGMSG(WCOM_DEBUG, "bsp_inband_push_ack error\n");

    hdr->len = 0;
    hdr->type = OSPUSHDATA;
    DASH_ob_request->own = 1;
    DASH_ob_request->len = DASHOSPSHLEN;
    DASH_ob_request->lowAddr = (char*)VA2PA(data);

    //alread initialized in gamc_init
    ///REG32(IOREG_IOBASE + IO_DMDSAR) = VA2PA(DASH_ob_request);
    REG8(IOREG_IOBASE + IO_DASH_REQUEST) = 0x92;
    OSSemPend(DASHREQLOCK, OOBMWT, &err);
    if (err == OS_ERR_TIMEOUT )
    {
        DEBUGMSG(WCOM_DEBUG,"bsp_inband_push_ack time out\n");
        status = PCI_BRIDGE_FAIL;
        OOBHwRest();
    }
    OSSemPost(DASHREQLOCK);
    free(data);
    return status;
}

int bsp_get_inband_data(char* data, int len)
{
    unsigned char err = 0;
    int status = PCI_DMA_OK;
    int i = 0;

    if (!bsp_AP_exist())
        return AP_NOT_READY;

    OSSemPend(DASHREQSEM, 0, &err);
    OSSemPend(DASHREQLOCK, 0, &err);
    DASH_ib_write_buffer = data;

    while (((DASH_ob_request->own == 1) || (REG8(IOREG_IOBASE + IO_DASH_REQUEST) != 0)) && i < 10)
    {
        OSTimeDly(OS_TICKS_PER_SEC * 0.5);
        i++;
    }

    if (i == 10)
        DEBUGMSG(WCOM_DEBUG, "bsp_get_inband_data error\n");

    DASH_ob_request->own = 1;
    DASH_ob_request->len = len;
    DASH_ob_request->lowAddr = (char*)VA2PA(data);

    REG8(IOREG_IOBASE + IO_DASH_REQUEST) = 0x91;

    OSSemPend(DASHREQLOCK, OOBMWT, &err);
    if (err == OS_ERR_TIMEOUT )
    {
        status = PCI_BRIDGE_FAIL;
        OOBHwRest();
        DEBUGMSG(WCOM_DEBUG, "bsp_get_inband_data TIMEOUT 1\n");
    }

    OSSemPost(DASHREQLOCK);

    if (status == PCI_DMA_OK)
    {
        OSSemPend(DASH_OS_Response_Event, OOBMWT, &err);

        if (err == OS_ERR_TIMEOUT )
        {
            DEBUGMSG(WCOM_DEBUG, "bsp_get_inband_data TIMEOUT 2\n");
            status = AP_NOT_RESPONSE;
        }
    }

    DASH_ib_write_buffer = 0;
    OSSemPost(DASHREQSEM);

    return status;
}
#endif


#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION <= IC_VERSION_DP_RevF)
//Return 0 means out of resource; Return 1 means transmit success.
#if defined(CONFIG_BUILDROM)
INT32U gmacsend(INT8U *bufaddr, INT32U length, INT8U options, void *freeptr)
#else
INT32U send(INT8U *bufaddr, INT32U length, INT8U options, void *freeptr)
#endif
{
    txdesc_t *txdesc, *tmptx;
    INT8U type = (options & 0x0f);
    INT8U v6flag = (options & OPT_IPV6) ;
    INT8U freed = (options & OPT_FREE);
    INT16U count = 0;

    OS_CPU_SR  cpu_sr = 0;


    OS_ENTER_CRITICAL();

#if 0
    if (freeflag[TxDescIndex])
    {
        free(freeflag[TxDescIndex]);
        freeflag[TxDescIndex] = 0;
    }
    if (freed)
        freeflag[TxDescIndex] = freeptr;
#endif

    txdesc = (txdesc_t *) (TxdescStartAddr) + TxDescIndex;
    TxDescIndex = ( TxDescIndex + 1 ) % TxdescNumber;

    while (txdesc->offset0.cmd.own == 1 && count < 20000)
    {
        bsp_wait(10);
        count++;
        //wait at most 200 m secs
    }

    if (count == 20000)
    {
        for (count = 0; count < TxdescNumber; count++)
        {
            tmptx = (txdesc_t *) TxdescStartAddr + count;
            tmptx->offset0.cmd.own = 0;
        }
        bsp_gmac_reset(1);
    }

    txdesc->tx_buf_addr_low = (INT8U *) VA2PA(bufaddr);
    if (type != OPT_NOCS )
    {
        if (type == OPT_UDPCS)
        {
            txdesc->offset1.cmd.udpcs = 1;
            txdesc->offset1.cmd.tcpcs = 0;
        }
        else if (type == OPT_TCPCS)
        {
            txdesc->offset1.cmd.tcpcs = 1;
            txdesc->offset1.cmd.udpcs = 0;
        }
        else {
            txdesc->offset1.cmd.udpcs =  0;
            txdesc->offset1.cmd.tcpcs = 0;
        }
        if (v6flag)
            txdesc->offset1.cmd.ipv4cs = 0;
        else
            txdesc->offset1.cmd.ipv4cs = 1;
    }
    else
    {
        txdesc->offset1.cmd.udpcs = 0;
        txdesc->offset1.cmd.tcpcs = 0;
        txdesc->offset1.cmd.ipv4cs = 0;
    }

    if (v6flag == OPT_IPV6)
    {
        txdesc->offset1.cmd.v6f = 1;
        txdesc->offset1.cmd.tcpho = 54;
        //Ethernet header len(14) + IPv6 header len(40)
    }
    else
        txdesc->offset1.cmd.v6f = 0;

    txdesc->offset0.cmd.fs  = 1;
    txdesc->offset0.cmd.ls  = 1;
    txdesc->offset0.cmd.length = length;
    txdesc->offset0.cmd.own = 1;


    //TxPolling
    REG8(MAC_BASE_ADDR + MAC_TPPoll) = 0x80;
#ifdef CONFIG_PROFILING
    mgtcycles[5] = rlx_cp3_get_counter_lo(0);
    rlx_cp3_stop();
#endif
    OS_EXIT_CRITICAL();

    if (freed)
    {
        //wait until DMA complete, for non-TCP packets
        while (txdesc->offset0.cmd.own)
            bsp_wait(1);

        //may only need to wait, but reuse the buffer without free
        if (freeptr)
            free(freeptr);
    }

    return 0;
}
#endif

#if defined(CONFIG_BUILDROM)
void bsp_gmac_enable()
{
    rxdesc_t *rxdesc;
    txdesc_t *txdesc;

    INT8U i;

    //reset own bit and length of descritpors
    for (i=0 ; i < RxdescNumber; i++)
    {
        rxdesc = (rxdesc_t *) RxdescStartAddr + i;
        rxdesc->offset0.cmd.own = 1;
        rxdesc->offset0.cmd.buffer_size = BUFF_SIZE;
    }

    for (i=0 ; i < TxdescNumber; i++)
    {
        txdesc = (txdesc_t *) TxdescStartAddr + i;
        txdesc->offset0.cmd.own = 0;
    }

    REG16(MAC_BASE_ADDR + MAC_ISR) = 0xFFFF;

    while(REG16(MAC_BASE_ADDR + MAC_CMD) & 0x300)
        ;

    REG8(MAC_BASE_ADDR+MAC_CMD)=0x0C;

}

void bsp_gmac_disable()
{
    //Check TPPoll to wait TX Finish
    while(REG8(MAC_BASE_ADDR + MAC_TPPoll) & 0x80)
        ;

    //TX FIFO is not empty
    while(REG16(MAC_BASE_ADDR + MAC_CMD) & 0x0C00)
        ;

    REG8(MAC_BASE_ADDR + MAC_CMD) = 0x00;

    //wait engine goest down
    while(REG16(MAC_BASE_ADDR + MAC_CMD) & 0x0300)
        ;
}

void bsp_gmac_reset(INT8U txreset)
{

    bsp_gmac_disable();
    bsp_gmac_enable();
    RxDescIndex  = 0;
    TxDescIndex  = 0;
    bsp_bits_set(MAC_OOBREG, 1, BIT_OOBRESET, 1);
    gmac_reset = 1;
}
#endif

#if CONFIG_VERSION <= IC_VERSION_DP_RevF

void bsp_gmac_reset(INT8U txreset)
{
    rxdesc_t *rxdesc;
    txdesc_t *txdesc = (txdesc_t *) (TxdescStartAddr) + (TxDescIndex + TxdescNumber  - 1) % TxdescNumber;
    INT8U i;
    INT16U count = 0;


    //let Rx RDU
    for (i=0 ; i < RxdescNumber; i++)
    {
        rxdesc = (rxdesc_t *) RxdescStartAddr + i;
        rxdesc->offset0.cmd.own = 0;
        rxdesc->offset0.cmd.buffer_size = BUFF_SIZE;
    }

    REG32(IOREG_IOBASE+MAC_RxCR)   &= 0xFFFFFF00; //default is 0x30E
    //wait a packet time for Rx DMA complete, at least 10 u secs
    bsp_wait(20);

    //wait the last Tx to be completed
    if (txreset)
    {
        while (txdesc->offset0.cmd.own == 1 && count < 20000)
        {
            count++;
            bsp_wait(10);
            //wait 200 mesc at most
        }
    }

//bit 15 is used for Revision A
//bit 8 is used for GPIO, avaliable in Revision E only
#if CONFIG_VERSION == IC_VERSION_DP_RevA
    if (txreset)
        REG8(IOREG_IOBASE+MAC_CMD)     &= 0x80;
    else
        REG8(IOREG_IOBASE+MAC_CMD)     &= 0x84;
#else
    if (txreset)
        REG8(IOREG_IOBASE+MAC_CMD)     &= 0x10;
    else
        REG8(IOREG_IOBASE+MAC_CMD)     &= 0x14;
#endif

    RxDescIndex  = 0;
    if (txreset)
        TxDescIndex  = 0;

    for (i=0 ; i < RxdescNumber; i++)
    {
        rxdesc = (rxdesc_t *) RxdescStartAddr + i;
        rxdesc->offset0.cmd.own = 1;
        rxdesc->offset0.cmd.buffer_size = BUFF_SIZE;
    }

    if (txreset && count == 20000)
    {
        for (i=0 ; i < TxdescNumber; i++)
        {
            txdesc = (txdesc_t *) TxdescStartAddr + i;
            txdesc->offset0.cmd.own = 0;
        }
    }


    if (txreset)
    {
        if (dpconf->ipmode == DUAL_IP_MODE)
            REG8(IOREG_IOBASE+MAC_CMD)     |= 0x0E;
        else
            REG8(IOREG_IOBASE+MAC_CMD)     |= 0x0C;
    }
    else
        REG8(IOREG_IOBASE+MAC_CMD)     |= 0x0A;

    //RxCR should be the latest one to open

    REG32(IOREG_IOBASE + MAC_RxCR) |= 0x0E;

    bsp_bits_set(IO_CONFIG1, 1, BIT_OOBRESET, 1);

    gmac_reset = 1;

}
#endif

#if defined(CONFIG_PortMatch_Test) || defined(CONFIG_8021X_ENABLED) || (CONFIG_VERSION  <= IC_VERSION_DP_RevF) || defined(CONFIG_WAKE_UP_PATTERN_ENABLED) || defined(CONFIG_BUILDROM)
void rx_recycle(void)
{

    INT8U err;
    //INT8U cnt = 0;
    rxdesc_t *rxdesc;
    INT8U *ptr, *newptr, *tmpptr;
    EthHdr *ethdr;
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
    RxQMsg *tmpmsg;
#endif


#ifdef CONFIG_PROFILING
    avg[0] += (mgtcycles[1] - mgtcycles[0]);
    avg[1] += (mgtcycles[2] - mgtcycles[1]);
    avg[2] += (mgtcycles[3] - mgtcycles[2]);
    avg[3] += (mgtcycles[4] - mgtcycles[3]);
    avg[4] += (mgtcycles[5] - mgtcycles[4]);
    rlx_cp3_init();
    rlx_cp3_start(CP3_PERFMODE1);
    mgtcycles[0] = rlx_cp3_get_counter_lo(0);
#endif
    rxdesc = (rxdesc_t *) RxdescStartAddr + RxDescIndex;
	
    while (!rxdesc->offset0.st.own)
    {
        ptr = (INT8U *) PA2VA(rxdesc->rx_buf_addr_low-PKT_ALIGN_SIZE);
        ethdr = (EthHdr *) ptr;

        debugmsg->RxTotal++;

        if (rxdesc->offset0.st.v4f && dpconf->IPv4Enable && !IPInfo.IPv4Reset)
            *ptr = V4_PKT;
        else if (rxdesc->offset0.st.v6f && dpconf->IPv6Enable && !IPInfo.IPv6Reset)
            *ptr = V6_PKT;
        else if (ethdr->ethType == htons(ARP_Protocol) && dpconf->IPv4Enable && !IPInfo.IPv4Reset)
            *ptr = ARP_PKT;
        else if (ethdr->ethType == htons(EAP8021x_Protocol))
            *ptr = EAP_PKT;
        else
        {
            debugmsg->DrvDrop++;
            *ptr = INVALID;			
        }

        *(ptr + rxdesc->offset0.st.length -2) = '\0';

        if (*ptr == INVALID)
            DEBUGMSG(DRIVER_DEBUG, "packet with %d invalid\n",rxdesc->offset0.st.length);

        //if invalid => drop the packet
        if (*ptr != INVALID)
        {

            ///newptr = reassign((void **)&ptr, rxdesc->offset0.st.length + PKT_ALIGN_SIZE);
            if(rxdesc->offset0.st.length <= 128)
            {
                newptr = malloc(rxdesc->offset0.st.length);
                memcpy(newptr, ptr, rxdesc->offset0.st.length);
                tmpptr = ptr;
                ptr = newptr;
                newptr = tmpptr;

            }
            else
            {
                newptr = malloc(MAX_PKT_SIZE);
            }

            if (newptr != NULL)
            {
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                tmpmsg = malloc(sizeof(RxQMsg));
                tmpmsg->ptr = ptr;
                tmpmsg->length = rxdesc->offset0.st.length;
#endif
                DEBUGMSG(DRIVER_DEBUG, "packet with %d received\n",rxdesc->offset0.st.length);
                rxdesc->rx_buf_addr_low = (INT8U *) VA2PA(newptr) + PKT_ALIGN_SIZE;
#ifdef CONFIG_PROFILING
                mgtcycles[1] = rlx_cp3_get_counter_lo(0) ;
#endif

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                err = OSQPost(RxQ, tmpmsg);
#else
		*(ptr + 1) = eth0;
                err = OSQPost(RxQ, ptr);
#endif

                //if queue is full
                if (err == OS_ERR_Q_FULL)
                {
                    DEBUGMSG(DRIVER_DEBUG, "packet droped %d with QFULL\n",rxdesc->offset0.st.length);
                    free(ptr);
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                    free(tmpmsg);
#endif
                }

            }
            else
            {
                debugmsg->DrvBusy++;
                DEBUGMSG(DRIVER_DEBUG,"packet droped %d with Memory empty\n",rxdesc->offset0.st.length);
            }
            //if full, drop the packet
        }
        rxdesc->offset0.cmd.buffer_size = BUFF_SIZE;
        rxdesc->offset0.cmd.own = 1;

        RxDescIndex = ( RxDescIndex + 1 ) % RxdescNumber;

        rxdesc = (rxdesc_t *) RxdescStartAddr + RxDescIndex;

    }

}
#endif

//exist in ROM only for Revision F, useless for EP
#if CONFIG_VERSION  < IC_VERSION_DP_RevF
void bsp_gmac_rx_stop()
{
    rxdesc_t *rxdesc;
    INT8U i;

    //let Rx RDU
    for (i=0 ; i < RxdescNumber; i++)
    {
        rxdesc = (rxdesc_t *) RxdescStartAddr + i;
        rxdesc->offset0.cmd.own = 0;
        rxdesc->offset0.cmd.buffer_size = BUFF_SIZE;
    }

    REG32(IOREG_IOBASE+MAC_RxCR)   &= 0xFFFFFF00; //default is 0x30E
    //wait a packet time for Rx DMA complete, at least 10 u secs
    bsp_wait(20);
}

#endif

#if CONFIG_VERSION  == IC_VERSION_EP_RevA
void rx_recycle_patched(void)
{

    INT8U err;
    rxdesc_t *rxdesc;
    INT8U *ptr, *newptr;
    EthHdr *ethdr;
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
    RxQMsg *tmpmsg;
#endif


    rxdesc = (rxdesc_t *) RxdescStartAddr + RxDescIndex;

    while (!rxdesc->offset0.st.own)
    {

        ptr = (INT8U *) PA2VA(rxdesc->rx_buf_addr_low-PKT_ALIGN_SIZE);

        newptr = malloc(rxdesc->offset0.st.length);

        memcpy(newptr+2, ptr, rxdesc->offset0.st.length-2);
        ethdr = (EthHdr *) newptr;

        if (rxdesc->offset0.st.v4f && dpconf->IPv4Enable && !IPInfo.IPv4Reset)
            *newptr = V4_PKT;
        else if (ethdr->ethType == htons(ARP_Protocol) && dpconf->IPv4Enable && !IPInfo.IPv4Reset)
            *newptr = ARP_PKT;
        else if (rxdesc->offset0.st.v6f && dpconf->IPv6Enable && !IPInfo.IPv6Reset)
            *newptr = V6_PKT;
        else
        {
            *newptr = INVALID;
        }

        *(newptr + rxdesc->offset0.st.length -2) = '\0';

        if (*newptr != INVALID)
        {

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
            tmpmsg = malloc(sizeof(RxQMsg));
            tmpmsg->ptr = newptr;
            tmpmsg->length = rxdesc->offset0.st.length;
            err = OSQPost(RxQ, tmpmsg);
#else
            err = OSQPost(RxQ, newptr);
#endif

            //if queue is full
            if (err == OS_ERR_Q_FULL)
            {
                free(newptr);
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                free(tmpmsg);
#endif
            }
        }

        rxdesc->offset0.cmd.buffer_size = BUFF_SIZE;
        rxdesc->offset0.cmd.own = 1;
        RxDescIndex = ( RxDescIndex + 1 ) % RxdescNumber;
        rxdesc = (rxdesc_t *) RxdescStartAddr + RxDescIndex;
    }
}

void patch_gmac_buffer_addr()
{
    INT8U i = 0;
    INT32U *ptr;

    for (i=0 ; i < RxdescNumber; i++)
    {
        ptr = (INT32U *) (PA2VA(REG32(RxdescStartAddr+i*16+8)) - 2);
        free(ptr);
        REG32(RxdescStartAddr+i*16+8) = VA2PA(malloc(MAX_PKT_SIZE)) + PKT_ALIGN_SIZE ;
    }

}
#endif


//#if  (CONFIG_VERSION  == IC_VERSION_EP_RevB) && defined(CONFIG_WAKE_UP_PATTERN_ENABLED)
#if 0
void rx_recycle_patchedB(void)
{

    INT8U err;
    rxdesc_t *rxdesc;
    INT8U *ptr, *newptr;
    EthHdr *ethdr;
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
    RxQMsg *tmpmsg;
#endif
    unsigned char *dbug;


    rxdesc = (rxdesc_t *) RxdescStartAddr + RxDescIndex;

    while (!rxdesc->offset0.st.own)
    {


        ptr = (INT8U *) PA2VA(rxdesc->rx_buf_addr_low-PKT_ALIGN_SIZE);

        newptr = malloc(rxdesc->offset0.st.length);

        memcpy(newptr, ptr, rxdesc->offset0.st.length);
        ethdr = (EthHdr *) newptr;



        if (rxdesc->offset0.st.v4f && dpconf->IPv4Enable && !IPInfo.IPv4Reset) {

            *newptr = V4_PKT;
            //printf("[RTK]rx_recycle_patched : got a V4_PKT\n");
        }
        else if (ethdr->ethType == htons(ARP_Protocol) && dpconf->IPv4Enable && !IPInfo.IPv4Reset) {

            *newptr = ARP_PKT;
            //printf("[RTK]rx_recycle_patched : got a ARP\n");
        }
        else if (rxdesc->offset0.st.v6f && dpconf->IPv6Enable && !IPInfo.IPv6Reset) {
            *newptr = V6_PKT;
        }
        else
        {
            *newptr = INVALID;
            //printf("[RTK]rx_recycle_patched : got a INVALID\n");
        }

        *(newptr + rxdesc->offset0.st.length -2) = '\0';

        if (*newptr != INVALID)
        {

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
            tmpmsg = malloc(sizeof(RxQMsg));
            tmpmsg->ptr = newptr;
            tmpmsg->length = rxdesc->offset0.st.length;
            err = OSQPost(RxQ, tmpmsg);
#else
            err = OSQPost(RxQ, newptr);
#endif

            //if queue is full
            if (err == OS_ERR_Q_FULL)
            {
                free(newptr);
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                free(tmpmsg);
#endif
            }

        }
        rxdesc->offset0.cmd.own = 1;
        rxdesc->offset0.cmd.buffer_size = BUFF_SIZE;

        RxDescIndex = ( RxDescIndex + 1 ) % RxdescNumber;

        rxdesc = (rxdesc_t *) RxdescStartAddr + RxDescIndex;

    }

}

#endif
