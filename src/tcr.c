#include <stdlib.h>
#include <string.h>
#include "bsp.h"
#include "telnet.h"
#include "cmac.h"
extern OS_EVENT *TCRQ;
ROM_EXTERN INT8U TCRRxIndex _ATTRIBUTE_ROM_BSS;
ROM_EXTERN INT8U TCR2RxIndex _ATTRIBUTE_ROM_BSS;
ROM_EXTERN TCRCB tcrcb _ATTRIBUTE_ROM_BSS;
ROM_EXTERN OS_EVENT *TCRQ _ATTRIBUTE_ROM_BSS;
ROM_EXTERN INT8U tcr2reset _ATTRIBUTE_ROM_BSS;
ROM_EXTERN void *TCRMsg[32] _ATTRIBUTE_ROM_BSS;
ROM_EXTERN INT8U *kcsrx , *kcstx _ATTRIBUTE_ROM_BSS;
ROM_EXTERN INT32U KCSRXOK, KCSTXOK _ATTRIBUTE_ROM_BSS;
ROM_EXTERN INT8U tcr2reset _ATTRIBUTE_ROM_BSS;

extern TCROPT *bootsel;
extern _IPInfo IPInfo[];
extern DPCONF *dpconf;
extern smbiosrmcpdata *smbiosrmcpdataptr;
extern INT8U *timestampdataptr;
extern INT8U *smbiosptr;
extern flash_data_t dirty[ENDTBL];
extern INT32U smbiosdatalen;

#ifdef CONFIG_LANWAKE_NOTIFY_PCIE
extern INT8U gIsLanWake ; 
#endif

void bsp_tcr_init(void)
{

    //Enable THR and RBR
    static INT8U initdone = 0;

#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    INT8U tmp, i;
    INT32U counter = 0;
#endif

    //should disable first, and enable  later
    //to let the thr pointer to be reset
    REG8(TCR_BASE_ADDR + TCR_CONF0) = 0x00;

    //for baud rate 115200
    ///REG8(TCR_BASE_ADDR + TCR_DLL) = 0xC1;
    ///REG8(TCR_BASE_ADDR + TCR_DLM) = 0x01;
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    tcr_thr_t *tcrthr = (tcr_thr_t *) (TCRdescStartAddr);

    do
    {
        tmp = REG8(TCR_BASE_ADDR + TCR_CONF0);
        counter++;

    } while((tmp & 0x01) != 0x00 && counter < 1000);
#endif

    //enable fifo
    REG8(TCR_BASE_ADDR + TCR_FCR) = 0x01;

    //Set THR Interrupt Timing mitigation
    //original TPT 20
    REG32(TCR_BASE_ADDR + TCR_TPT) = 0x0000100;
    REG32(TCR_BASE_ADDR + TCR_TIMT)  = 0x0002500;
    REG8(TCR_BASE_ADDR + TCR_TIMPC)  = 0x10;
    //at most 64 packets

    //write_reg(SERIAL_RBR_IFG, 0x00001010);
    REG32(TCR_BASE_ADDR + TCR_RBR_IFG) = 0x0000000C;

    //REG8(TCR_BASE_ADDR + TCR_RBR_DESC_START) = 0;
    REG16(TCR_BASE_ADDR + TCR_ISR) = 0xFFFF ;

    if (initdone == 0)
    {
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
        REG32(TCR_BASE_ADDR + TCR_DESCADDR) = VA2PA(TCRdescStartAddr);

        for(i = 0 ; i < TCRTHRdescNumber; i++)
        {
            //140+112 = 252 < 256
            tcrthr->buff_size = 112;
            tcrthr->own = 1;
			tcrthr->rsvd = 0;
            tcrthr->bufaddr = VA2PA(malloc(112));
            if(i == TCRTHRdescNumber -1)
                tcrthr->eor = 1;
            else
                tcrthr->eor = 0;
            tcrthr++;
        }
#endif

        TCRQ = OSQCreate(TCRMsg,10);
        rlx_irq_register(BSP_TCR_IRQ, bsp_tcr_handler);

        //when pci-e reset, IMR would default to 0x04
        //REG16(TCR_BASE_ADDR + TCR_IMR) = 0x0004;
        initdone = 1;
    }
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    else
    {
        //reset own bit and buffer size only
        for(i = 0 ; i < TCRTHRdescNumber; i++)
        {
            tcrthr->buff_size = 112;
            tcrthr->own = 1;
            tcrthr++;
        }

    }

    TCRRxIndex = 0;
#endif
    REG16(TCR_BASE_ADDR + TCR_IMR) = 0x0006;
    REG8(TCR_BASE_ADDR + TCR_CONF0) = 0xE8;
    //REG8(TCR_BASE_ADDR + TCR_IER) &= ~0x02;

}
void bsp_kcs_init_patch();
void bsp_tcr_handler()
{

    INT16U val16;
    static INT8U pciereset = 0;
    //tcr has 1k bytes buffers inside

    REG16(TCR_BASE_ADDR + TCR_IMR) = 0x0000;
    val16 = REG16(TCR_BASE_ADDR + TCR_ISR);
    REG16(TCR_BASE_ADDR + TCR_ISR) = val16;

    if ((val16 & 0x4) != 0)
    {
    #if CONFIG_LANWAKEUP_ENABLED
	bsp_bits_set(MAC_OOB_LANWAKE , 0 , BIT_OOBLANWAKE , 1);//set BIT_OOBLANWAKE to 0	
    #endif	
	
	#ifdef CONFIG_CPU_SLEEP_ENABLED
	EnableCPUSleep();
	#endif
	
        //no in-band can issue a write in pci-e reset
        //reset the flag to prevent uncondition hang
        REG8(IOREG_IOBASE + MAC_FLAG_IB) = 0;
        //clear the inband exist bit
        bsp_bits_set(MAC_IBREG, 0, BIT_DRIVERRDY, 1);
        bsp_bits_set(MAC_IBREG, 0, BIT_APRDY, 1);
//EP is through TCAM, which is set in tcpTimer Task
#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
        //added, since PCI-E reset means IB leaves
        IPInfo[eth0].DoReloadFilterTable = 1;
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
        if (bsp_bits_get(IO_CONFIG1, BIT_SINGLE_IP, 1) == 0)
        {
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP0, 1);
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP1, 1);
            bsp_bits_set(MAC_RxCR, 1, BIT_FILTERV4DIP2, 1);
        }
#endif
#endif

        dpconf->restart = 0;

        //for Version before F, using PCI-E signal
        if (bsp_get_sstate() == S_S0)
            bsp_set_sstate(S_S5);

        smbiosrmcpdataptr->PollType = POLL_STOP;
        smbiosrmcpdataptr->Boottime = 0;

        bsp_tcr_init();

#if CONFIG_VERSION >= IC_VERSION_EP_RevA
        //When power on, CMAC will receive pcie reset.
        //Enable pcie reset to reset CMAC descriptor
        REG16(CMAC_IOBASE+CMAC_IMR)=0xCf;
#endif

//#if defined(CONFIG_KCS_ENABLED) && (CONFIG_VERSION <= IC_VERSION_DP_RevD)
#if CONFIG_KCS_ENABLED 
	#if (CONFIG_VERSION <= IC_VERSION_DP_RevD)
		bsp_kcs_init();
	#else
		#if defined(CONFIG_KCS_AMD_PATCH_ENABLED) &&( (CONFIG_CHIPSET_AMD)||(CONFIG_CHIPSET_AMDSOC))
        		//bsp_kcs_init();		
        		bsp_kcs_init_patch();
		#else			
			bsp_kcs_init_patch();
		#endif
	#endif
#endif

        //only FSC use tcr2
        if (dpconf->vendor == FSC || dpconf->vendor == SAMSUNG)
            bsp_tcr2_init();

        tcrcb.bootopt = 0;
        pciereset = 1;
        tcr2reset = 1;
        //REG16(TCR_BASE_ADDR + TCR_IMR) = 0x0006;//tcrimr;



#if !defined(CONFIG_mDNS_OFFLOAD_ENABLED) && (CONFIG_VERSION <= IC_VERSION_DP_RevF)
        //MacPro do not have auxiliary power
        //handle dirty shutdown case
        bsp_gmac_reset(1);
#endif

#if CONFIG_VERSION  < IC_VERSION_DP_RevF
#ifdef CONFIG_GPIO_DEBUG
        GPIOSet(0);
#endif
        bsp_lbtest();
#ifdef CONFIG_GPIO_DEBUG
        GPIOSet(1);
#endif
#endif

    }

    if ((val16 & 0x2) != 0)
    {
        if(pciereset == 1)
        {
            tcrcb.bootopt = 1;
            pciereset = 0;
        }
        tcrrx();
    }

    REG16(TCR_BASE_ADDR + TCR_IMR) = 0x0006;

}

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
void tcrrx()
{

    static PKT  *txPkt = NULL;
    static INT8U *ptr;
    INT16U count = 0;
    INT8U length = 0;
    INT8U err;
    INT8U i;
    static INT8U sendflag = 0;
    OS_Q_DATA qdata;

    volatile static INT8U  *tcrdesc =   (INT8U *) (TCR_BASE_ADDR + TCR_THR_DESC_START);
    volatile static INT8U *tcrdata =   (INT8U *) (TCR_BASE_ADDR + TCR_THR_DATA_START);


    if(tcrcb.state == CONNECTED)
    {

        if(txPkt != NULL)
        {
            OSQQuery(TCRQ, &qdata);

            //means telnet TASK already get the Q
            if(qdata.OSNMsgs == 0 || txPkt->len > (TCR_MAX_PKT_SIZE-16))
                txPkt = NULL;
        }

        if(txPkt == NULL)
        {
            if(tcrcb.contype == SSH_TCR)
                txPkt = (PKT*)allocSSHPkt_F(tcrcb.tcrcs, TCR_MAX_PKT_SIZE, 1);
            else
                txPkt = (PKT*)allocPkt_F(TCR_MAX_PKT_SIZE);

            ptr = txPkt->wp;

            if(tcrcb.contype == SSH_TCR)
                ptr += 4;
            //txPkt->wp += 4 ?

            txPkt->len = 0;
            sendflag = 1;
        }
    }

    while (1)
    {
        REG8(TCR_BASE_ADDR + TCR_THR_PKT_RD) = 1;
        //wait for tcr ready (complete data transfer from sram)
        while (REG8(TCR_BASE_ADDR + TCR_THR_PKT_RD))
        {
            bsp_wait(1);
            //maximal wait 1 ms
            if (++count == 1000)
            {
                bsp_tcr_init();
                return ;
            }
        }

        if ((*tcrdesc & 0x80) == 0x00)
        {
            if(tcrcb.state == CONNECTED)
            {
                length = *tcrdesc & 0x1f;
                txPkt->len += length;

                for (i = 0; i < length; i++)
                    *ptr++ = *(tcrdata+i);

                *tcrdesc = 0x80;
            }
            else
                *tcrdesc = 0x80;

        }
        else
        {
            if((tcrcb.state == CONNECTED) && sendflag)
            {
                //post the buffer at the very beginning
                if(txPkt->len)
                    err = OSQPost(TCRQ, txPkt);
                else
                    err = OS_ERR_Q_FULL;

                //if queue if full or length is 0, reuse the buffer
                if (err == OS_ERR_Q_FULL)
                {
                    freePkt(txPkt);//txPkt->len = 0;
                    txPkt= NULL;
                }
                else
                    sendflag = 0;
            }
            break;
        }
    }
}
#else

void tcrrx()
{

    static PKT  *txPkt = NULL;
    static INT8U *ptr = NULL;
    INT16U count = 0;
    INT8U length = 0;
    INT8U err;
    INT8U i;
    static INT8U sendflag = 0;

    tcr_thr_t *tcrthr = (tcr_thr_t *) (TCRdescStartAddr) + TCRRxIndex;

    OS_Q_DATA qdata;


    while(!tcrthr->own && tcrthr->buff_size)
    {
        if(tcrcb.state == CONNECTED)
        {
            OSQQuery(TCRQ, &qdata);

            if(!qdata.OSNMsgs  || !txPkt)
            {
                if(tcrcb.contype == SSH_TCR)
                    txPkt = (PKT*)allocSSHPkt_F(tcrcb.tcrcs,TCR_MAX_PKT_SIZE,1);
                else
                    txPkt = (PKT*)allocPkt_F(TCR_MAX_PKT_SIZE);

                if(txPkt)
                {
                    ptr = txPkt->wp;

                    if(tcrcb.contype == SSH_TCR)
                        ptr += 4;
                    txPkt->len = 0;

                    sendflag = 1;
                }
            }

            if(txPkt)
            {
                memcpy(ptr, (INT8U *)PA2VA(tcrthr->bufaddr), tcrthr->buff_size);
                txPkt->len += tcrthr->buff_size;
                ptr += tcrthr->buff_size;

                //post the buffer at the very beginning
                if(sendflag)
                {
                    if(txPkt->len)
                        err = OSQPost(TCRQ, txPkt);
                    else
                        err = OS_ERR_Q_FULL;

                    //if queue if full, reuse the buffer
                    if (err == OS_ERR_Q_FULL)
                    {
                        freePkt(txPkt);
                        txPkt = NULL;
                    }
                    else
                        sendflag = 0;
                }
                if(txPkt){
                if(txPkt->len  > (TCR_MAX_PKT_SIZE - 16) )
                    txPkt = NULL;
            }
        }
        }

        tcrthr->own = 1;
        tcrthr->buff_size = 112;

        TCRRxIndex = ( TCRRxIndex + 1 ) % TCRTHRdescNumber;

        tcrthr = (tcr_thr_t *) (TCRdescStartAddr) + TCRRxIndex;
    }

}
#endif

//#if CONFIG_VERSION <= IC_VERSION_DP_RevF
//#if (CONFIG_VERSION  <= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)

#if ( CONFIG_VERSION <= IC_VERSION_DP_RevF) ||  ((CONFIG_KCS_AMD_PATCH_ENABLED) && (CONFIG_CHIPSET_AMD))||  ((CONFIG_KCS_AMD_PATCH_ENABLED) && (CONFIG_CHIPSET_AMDSOC)) || defined(CONFIG_BUILDROM)
//#if ( CONFIG_VERSION <= IC_VERSION_DP_RevF) || ((CONFIG_KCS_AMD_PATCH_ENABLED) && defined(CONFIG_CHIPSET_AMD))
void bsp_kcs_init()
{
    //Fill in Rx desc and Tx Desc
    if (kcsrx == NULL)
    {
        kcsrx = malloc(256);
        kcstx = malloc(256);
        rlx_irq_register(BSP_KCS_IRQ, bsp_kcs_handler);
    }

    REG32(KCS_BASE + KCS_RX_DMA_ADDR) = VA2PA(kcsrx);
    REG8(KCS_BASE + KCS_RX_DESC) = 0x80;
    REG32(KCS_BASE + KCS_TX_DMA_ADDR) = VA2PA(kcstx);
    REG8(KCS_BASE + KCS_TX_DESC) = 0x00;

    REG8(KCS_BASE + KCS_CONF)  = 0x16; //tx_enable, rx_enable
    REG8(KCS_BASE + KCS_ERROR_CODE) = 0x00;
    REG8(KCS_BASE + KCS_CTRL) = 0x01;

    REG16(KCS_BASE  + KCS_ISR) = 0x07FF;
    REG16(KCS_BASE  + KCS_IMR) = 0x0000;

    //clear kcs status to avoid windows cpu utilization high
    REG8(KCS_BASE + KCS_STATUS2) = 0x00;

}

void bsp_kcs_handler(void)
{
    INT16U val16;

    REG16(KCS_BASE + KCS_IMR) = 0x0000;
    val16 = REG16(KCS_BASE + KCS_ISR);

    if ((val16 & 0x1) != 0)	//rxok
        kcs_rx_recycle();
    if ((val16 & 0x2) != 0)	//txok
        KCSTXOK++;

    if ((val16 & 0x4) != 0)	//ABORT
    {
        REG8(KCS_BASE + KCS_ERROR_CODE) = 0xC0;
        REG8(KCS_BASE + KCS_CTRL) = 0x00;
    }

    if ((val16 & 0x8) != 0)	//Get Status
        bsp_kcs_init();

    if((val16 & 0x40))
    {
        if(REG8(KCS_BASE + KCS_STATUS2) & 0x02)
            REG8(KCS_BASE + KCS_STATUS2) = 0x01;
    }

    //if((val16 & 0x10) != 0)	//Invalid cmd

    //if((val16 & 0x20) != 0)	//wr_cmd_sts

    //if((val16 & 0x40) != 0)	//wr_data_in_sts

    if ((val16 & 0x80) != 0)	//PCI reset
    {
//from Version D, kcs pcie-reset is posedge
#if CONFIG_VERSION >= IC_VERSION_DP_RevD

        //FSC would inform the power state to DP
        //also means the time for enabling polling
        if (dpconf->vendor != FSC)
            bsp_set_sstate(S_S0);
        smbiosrmcpdataptr->PollType = POLL_NONE;
#endif
    }


    REG16(KCS_BASE + KCS_ISR) = val16;
    REG16(KCS_BASE + KCS_IMR) = 0x0000;
    //PCI-E reset is non-maskable

}

void kcs_rx_recycle()
{

    INT32U length;
    INT32U result=0;
    INT32U i;
    INT8U  val8;

    //Read Rx descriptor
    do
    {
        length = REG8(KCS_BASE + KCS_RX_DESC);

    } while ((length & 0x80) == 0x80);

    length = (length << 8) + REG8(KCS_RX_DESC_LEN);

#if 0
    if (length>256)
        while (1);
#endif


    val8 = *(kcsrx);
    for (i=1; i<length-1; i++)
    {
        if (*(kcsrx+i)!=++val8)
        {
            result=1;
            break;
        }
    }

    if (result==0)
        KCSRXOK++;

    REG8(KCS_BASE + KCS_RX_DESC) = 0x80;

    //set val8 to the rxbuf value
    //kcs_ptr = (int8*)(KCS_RxBufStartAddr);
    //kcs_ptr = kcs_ptr+0x80100000;
    val8 = *(kcsrx);

    for (i=0; i<length; i++)
        *(kcstx+i)=val8++;

    if (length!=256)
    {
        REG8(KCS_BASE + KCS_TX_DESC_LEN) = length;
        REG8(KCS_BASE + KCS_TX_DESC) = 0x80;
    }
    else
    {
        REG8(KCS_BASE + KCS_TX_DESC_LEN) = 0x00;
        REG8(KCS_BASE + KCS_TX_DESC) = 0x81;
    }

}

void bsp_tcr2_handler(void)
{
    INT16U val16;
    static INT8U pciereset = 0;
    static INT8U *tmp = NULL;
    //tcr has 1k bytes buffers inside
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    tcr_thr_t *tcrthr = (tcr_thr_t *) (TCR2descStartAddr) + TCR2RxIndex;
#else
    INT8U length = 0;
    INT16U count = 0;
    volatile INT8U  *tcrdesc =   (INT8U *) (TCR2_BASE_ADDR + TCR_THR_DESC_START);
    volatile INT8U *tcrdata =   (INT8U *) (TCR2_BASE_ADDR + TCR_THR_DATA_START);

#endif

    REG16(TCR2_BASE_ADDR + TCR_IMR) = 0x0000;
    val16 = REG16(TCR2_BASE_ADDR + TCR_ISR);
    REG16(TCR2_BASE_ADDR + TCR_ISR) = val16;

    if (tcr2reset)
    {
        tmp = timestampdataptr + SMBIOS_DATA_OFFSET;
        smbiosptr = (INT8U *) (SMBIOS_ROM_START_ADDR + SMBIOS_DATA_OFFSET);
        //use the rom version before update completion
        tcr2reset = 0;
        smbiosdatalen = 0;
    }

    if ((val16 & 0x2) != 0)
    {
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
        while(!tcrthr->own)
        {
            memcpy(tmp, (INT8U *)PA2VA(tcrthr->bufaddr), tcrthr->buff_size);
            tmp += tcrthr->buff_size;
            smbiosdatalen += tcrthr->buff_size;

            if (smbiosdatalen > 23 && smbiosdatalen >= (*((INT16U *) (timestampdataptr + SMBIOS_DATA_OFFSET + 0x16)) + 0x20))
            {
                if(memcmp((INT8U *) (timestampdataptr+SMBIOS_DATA_OFFSET), "_SM_",4) == 0)
                {
                    smbiosptr = (INT8U *) (timestampdataptr + SMBIOS_DATA_OFFSET);
                    dirty[SMBIOSTBL].length = *(INT16U *) (smbiosptr + 0x16);
                    memset(smbiosptr + dirty[SMBIOSTBL].length + SMBIOS_HEADER_SIZE, 0, MAX_SMBIOS_SIZE-SMBIOS_HEADER_SIZE-SMBIOS_DATA_OFFSET-dirty[SMBIOSTBL].length);
                    DEBUGMSG(MESSAGE_DEBUG, "Change to RAM Version length is %d, smbios len is %d\n", dirty[SMBIOSTBL].length, smbiosdatalen);
                    setdirty(SMBIOSTBL);
                }
            }

            tcrthr->own = 1;
            tcrthr->buff_size = 128;

            TCR2RxIndex = ( TCR2RxIndex + 1 ) % TCRTHRdescNumber;

            tcrthr = (tcr_thr_t *) (TCR2descStartAddr) + TCR2RxIndex;
        }
#else
        while (1) {

            REG8(TCR2_BASE_ADDR + TCR_THR_PKT_RD) = 1;

            while (REG8(TCR2_BASE_ADDR + TCR_THR_PKT_RD))
            {
                bsp_wait(1);
                if (++count == 1000)
                {
                    bsp_tcr2_init();
                    return;
                }
            }
            if ((*tcrdesc & 0x80) == 0x00)
            {
                length = *tcrdesc & 0x1f;

                if (length == 0)
                    break;

                memcpy(tmp, (void *) tcrdata, length);
                tmp += length;
                smbiosdatalen += length;

                //has got the length field
                if (smbiosdatalen > 23 && smbiosdatalen >= (*((INT16U *) (timestampdataptr + SMBIOS_DATA_OFFSET + 0x16)) + 0x20))
                {
                    if(memcmp((INT8U *) (timestampdataptr+SMBIOS_DATA_OFFSET), "_SM_",4) == 0)
                    {
                        smbiosptr = (INT8U *) (timestampdataptr + SMBIOS_DATA_OFFSET);
                        dirty[SMBIOSTBL].length = *(INT16U *) (smbiosptr + 0x16);
                        memset(smbiosptr + dirty[SMBIOSTBL].length + SMBIOS_HEADER_SIZE, 0, MAX_SMBIOS_SIZE-SMBIOS_HEADER_SIZE-SMBIOS_DATA_OFFSET-dirty[SMBIOSTBL].length);
                        DEBUGMSG(MESSAGE_DEBUG, "Change to RAM Version length is %d, smbios len is %d\n", dirty[SMBIOSTBL].length, smbiosdatalen);
                        setdirty(SMBIOSTBL);
                    }
                }
                *tcrdesc = 0x80;
            }
            else
                break;
        }
#endif
    }

    REG16(TCR2_BASE_ADDR + TCR_IMR) = 0x0006;

}

void bsp_tcr2_init(void)
{

    //Enable THR and RBR
    static INT8U initdone = 0;
    INT8U tmp, i;
    INT32U counter = 0;

    //should disable first, and enable  later
    //to let the thr pointer to be reset
    REG8(TCR2_BASE_ADDR + TCR_CONF0) = 0x00;

#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    tcr_thr_t *tcrthr = (tcr_thr_t *) (TCR2descStartAddr);

    do
    {
        tmp = REG8(TCR2_BASE_ADDR + TCR_CONF0);
        counter++;

    } while((tmp & 0x01) != 0x00 && counter < 1000);
#endif

    //enable fifo
    REG8(TCR2_BASE_ADDR + TCR_FCR) = 0x01;

    //Set THR Interrupt Timing mitigation
    //original TPT 20
    REG32(TCR2_BASE_ADDR + TCR_TPT) = 0x0000100;
    REG32(TCR2_BASE_ADDR + TCR_TIMT)  = 0x0002500;
    REG8(TCR2_BASE_ADDR + TCR_TIMPC)  = 0x10;
    //at most 64 packets

    //write_reg(SERIAL_RBR_IFG, 0x00001010);
    REG32(TCR2_BASE_ADDR + TCR_RBR_IFG) = 0x0000000C;

    //REG8(TCR_BASE_ADDR + TCR_RBR_DESC_START) = 0;
    REG16(TCR2_BASE_ADDR + TCR_ISR) = 0xFFFF ;

    if (initdone == 0)
    {
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
        REG32(TCR2_BASE_ADDR + TCR_DESCADDR) = VA2PA(TCR2descStartAddr);

        for(i = 0 ; i < TCRTHRdescNumber; i++)
        {
            tcrthr->buff_size = 128;
            tcrthr->own = 1;
            tcrthr->bufaddr = VA2PA(malloc(128));
            if(i == TCRTHRdescNumber -1)
                tcrthr->eor = 1;
            else
                tcrthr->eor = 0;
            tcrthr++;
        }
#endif

        rlx_irq_register(BSP_TCR2_IRQ, bsp_tcr2_handler);

        //when pci-e reset, IMR would default to 0x04
        //REG16(TCR_BASE_ADDR + TCR_IMR) = 0x0004;
        initdone = 1;
        tcr2reset = 1;
    }
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    else
    {
        //reset own bit and buffer size only
        for(i = 0 ; i < TCRTHRdescNumber; i++)
        {
            tcrthr->buff_size = 128;
            tcrthr->own = 1;
            tcrthr++;
        }

    }

    TCR2RxIndex = 0;
#endif
    REG16(TCR2_BASE_ADDR + TCR_IMR) = 0x0006;
    REG8(TCR2_BASE_ADDR + TCR_CONF0) = 0xE8;
    //REG8(TCR_BASE_ADDR + TCR_IER) &= ~0x02;


}

void tcr_received(INT8U *buf, INT8U len)
{
    volatile INT8U *rxdesc = (INT8U *) (TCR_BASE_ADDR + TCR_RBR_DESC_START);
    INT8U *ptr = (INT8U *) (TCR_BASE_ADDR + TCR_RBR_DATA_START);
    INT8U sendlen;
#ifdef FN_PATCH_AMI
    INT8U alt[4] = {27, '^', 'A', 0};
#endif
    INT32U count =0;

    while (len)
    {
        sendlen = (len > 16) ? (16) :  (len);
        while ((*rxdesc & 0x80) == 0x80 && count < 1000)
        {
            //OSTimeDly(1);
            count++;
            bsp_wait(10);
        }

        REG8(TCR_BASE_ADDR + TCR_FCR) = 0x01;
#ifdef FN_PATCH_AMI
        if (buf[0] == 0x1b && sendlen == 2 && isalpha(buf[1]))
            //if(0)
        {

            strcpy(ptr, alt);
            memcpy(ptr+3, buf+1, sendlen-1);
            *rxdesc = (0x80 | (sendlen+2));
        }
        else if (buf[0] == 0x1b && buf[1] =='[' && buf[3] == '~' && sendlen == 4)
        {
            if (buf[2] == '6')
                buf[1] = '/';
            else if (buf[2] == '5')
                buf[1] = '?';
            else if (buf[2] == '1')
                buf[1] = 'h';
            else if (buf[2] == '4')
                buf[1] = 'k';

            strncpy(ptr, buf, 2);
            *rxdesc = (0x80 | 0x02);
        }
        else
#endif
        {
            memcpy(ptr, buf, sendlen);
            *rxdesc = (0x80 | sendlen);
        }
        buf += sendlen;
        len -= sendlen;
    }
}
#endif
//Patch bsp_kcs_handler
#ifdef CONFIG_POWER_SAVING_ENABLED
	#if CONFIG_VERSION >= IC_VERSION_EP_RevB
extern INT8U pcie_up_timer;
	#endif
#endif	

void bsp_kcs_handler_patch(void);
void bsp_kcs_init_patch()
{
    //Fill in Rx desc and Tx Desc


	
    if (kcsrx == NULL)
    {
        kcsrx = malloc(256);
        kcstx = malloc(256);
        rlx_irq_register(BSP_KCS_IRQ, bsp_kcs_handler_patch);
    }

    REG32(KCS_BASE + KCS_RX_DMA_ADDR) = VA2PA(kcsrx);
    REG8(KCS_BASE + KCS_RX_DESC) = 0x80;
    REG32(KCS_BASE + KCS_TX_DMA_ADDR) = VA2PA(kcstx);
    REG8(KCS_BASE + KCS_TX_DESC) = 0x00;

    REG8(KCS_BASE + KCS_CONF)  = 0x16; //tx_enable, rx_enable
    REG8(KCS_BASE + KCS_ERROR_CODE) = 0x00;
    REG8(KCS_BASE + KCS_CTRL) = 0x01;

    REG16(KCS_BASE  + KCS_ISR) = 0x07FF;
    REG16(KCS_BASE  + KCS_IMR) = 0x0000;

    //clear kcs status to avoid windows cpu utilization high
    REG8(KCS_BASE + KCS_STATUS2) = 0x00;

}

void bsp_kcs_handler_patch(void)
{
    INT16U val16;
    INT32U reg;
	

    REG16(KCS_BASE + KCS_IMR) = 0x0000;
    val16 = REG16(KCS_BASE + KCS_ISR);

	//printf("[RTK] Disable power saving\n");
	//DisableGigaPowerSavingParameter();
	#ifdef CONFIG_POWER_SAVING_ENABLED
	#if CONFIG_VERSION >= IC_VERSION_EP_RevB
	//pcie_up_timer = 12;
	#endif
	#endif


	#ifdef CONFIG_CPU_SLEEP_ENABLED
	DisableCPUSleep();
	SetLucidTimerInSec(90);
	#endif

	#ifdef CONFIG_LANWAKE_NOTIFY_PCIE
	if(gIsLanWake == 1){
		//Wakeup by LanWake pin from Firmware
		gIsLanWake = 0;
		reg  = readCSIChannel(0,0x814);
		reg=reg|0x01;
		writeCSIChannel(0,0x814, reg);
	}else{
		
		//clear notify bit
		reg  = readCSIChannel(0,0x814);
		reg=reg &( ~(INT32U)(0x01));
		writeCSIChannel(0,0x814, reg);
		
	}
	#endif
	

    if ((val16 & 0x1) != 0)	//rxok
        kcs_rx_recycle();
    if ((val16 & 0x2) != 0)	//txok
        KCSTXOK++;

    if ((val16 & 0x4) != 0)	//ABORT
    {
        REG8(KCS_BASE + KCS_ERROR_CODE) = 0xC0;
        REG8(KCS_BASE + KCS_CTRL) = 0x00;
    }

    if ((val16 & 0x8) != 0)	//Get Status
        bsp_kcs_init_patch();

    if((val16 & 0x40))
    {
        if(REG8(KCS_BASE + KCS_STATUS2) & 0x02)
            REG8(KCS_BASE + KCS_STATUS2) = 0x01;
    }

    //if((val16 & 0x10) != 0)	//Invalid cmd

    //if((val16 & 0x20) != 0)	//wr_cmd_sts

    //if((val16 & 0x40) != 0)	//wr_data_in_sts

    if ((val16 & 0x80) != 0)	//PCI reset
    {
//from Version D, kcs pcie-reset is posedge
#if CONFIG_VERSION >= IC_VERSION_DP_RevD

        //FSC would inform the power state to DP
        //also means the time for enabling polling
        if (dpconf->vendor != FSC)
            bsp_set_sstate(S_S0);
        smbiosrmcpdataptr->PollType = POLL_NONE;
        #ifdef CONFIG_VENDOR_FSC  
        if(tcrcb.state == DISABLED){
        	reg  = readCSIChannel(0, 0x814);
        	reg=reg|0x02;//bit1=1
        	//printf("[RTK] Set fun0 PCI CS Offset0x814 = 2 @PCIE_POSITIVE\n");
        	writeCSIChannel(0, 0x814, reg);
        }
        else{
        	reg  = readCSIChannel(0, 0x814);
        	reg=reg &( ~(INT32U)(0x02));//bit1=0
        	//printf("[RTK] Set fun0 PCI CS Offset0x814 = 0 @PCIE_POSITIVE\n");
        	writeCSIChannel(0, 0x814, reg);
        }       
        #endif
#endif
    }


    REG16(KCS_BASE + KCS_ISR) = val16;
    REG16(KCS_BASE + KCS_IMR) = 0x0000;
    //PCI-E reset is non-maskable

}


#if CONFIG_VERSION <= IC_VERSION_DP_RevF
void tcrread()
{
    INT16U val16;
    INT32U count;

    volatile static INT8U  *tcrdesc =   (INT8U *) (TCR_BASE_ADDR + TCR_THR_DESC_START);
    volatile static INT8U *tcrdata =   (INT8U *) (TCR_BASE_ADDR + TCR_THR_DATA_START);

    while(1)
    {
        count = 0;
        val16 = REG16(TCR_BASE_ADDR + TCR_ISR);
        REG16(TCR_BASE_ADDR + TCR_ISR) = val16;

        if ((val16 & 0x2) != 0)
        {
            REG8(TCR_BASE_ADDR + TCR_THR_PKT_RD) = 1;
            //wait for tcr ready (complete data transfer from sram)
            while (REG8(TCR_BASE_ADDR + TCR_THR_PKT_RD))
            {
                if (++count == 600000)
                    break;
            }
            if ((*tcrdesc & 0x80) == 0x00)
                *tcrdesc = 0x80;
        }

        REG32(WDT_BASE_ADDR + WDT_CRR) = 0x76;
    }


}
#else
void tcrread()
{
    tcr_thr_t *tcrthr = (tcr_thr_t *) (TCRdescStartAddr) + TCRRxIndex;

    while(!tcrthr->own && tcrthr->buff_size)
    {
        tcrthr->own = 1;
        tcrthr->buff_size = 112;

        TCRRxIndex = ( TCRRxIndex + 1 ) % TCRTHRdescNumber;

        tcrthr = (tcr_thr_t *) (TCRdescStartAddr) + TCRRxIndex;

        REG32(WDT_BASE_ADDR + WDT_CRR) = 0x76;
    }

    REG32(WDT_BASE_ADDR + WDT_CRR) = 0x76;
}

#endif



#if 0
void bsp_tcr_imr_enable(void)
{
    tcrimr |= 0x0002;
    REG16(TCR_BASE_ADDR + TCR_IMR) = tcrimr;
}
void bsp_tcr_imr_disable(void)
{
    tcrimr &= ~0x0002;
    REG16(TCR_BASE_ADDR + TCR_IMR) =  tcrimr;
}
void  tcrgetdata(PKT *pkt)
{

    INT16U count = 0;
    INT8U length = 0;
    INT8U *ptr = pkt->wp + pkt->len;
    INT8U i;
    volatile static INT8U  *tcrdesc =   (INT8U *) (TCR_BASE_ADDR + TCR_THR_DESC_START);
    volatile static INT8U *tcrdata =   (INT8U *) (TCR_BASE_ADDR + TCR_THR_DATA_START);

    while (1)
    {
        REG8(TCR_BASE_ADDR + TCR_THR_PKT_RD) = 1;
        //wait for tcr ready (complete data transfer from sram)
        while (REG8(TCR_BASE_ADDR + TCR_THR_PKT_RD))
        {
            bsp_wait(1);
            //maximal wait 1 ms
            if (++count == 1000)
            {
                bsp_tcr_init();
                return ;
            }
        }

        if ((*tcrdesc & 0x80) == 0x00)
        {
            length = *tcrdesc & 0x1f;
            pkt->len += length;

            for (i = 0; i < length; i++)
                *ptr++ = *(tcrdata+i);

            *tcrdesc = 0x80;
            if (pkt->len > (TCR_MAX_PKT_SIZE - 16))
                break;
        }
        else
            break;
    }

    //return total;
}
#endif
void stopTCR()
{
    PKT  *txPkt   = NULL;
    INT8U err;
   
    ///bsp_tcr_imr_disable();

    if (tcrcb.tcrcs)
        tcrcb.tcrcs = stopconn_F(tcrcb.tcrcs);

    if (tcrcb.tcrSrv)
        tcrcb.tcrSrv = stopconn_F(tcrcb.tcrSrv);

    //if (tcrcb.pkt)
    //    freePkt(tcrcb.pkt);

    tcrcb.state = DISABLED;
    
    txPkt = OSQAccept(TCRQ, &err);
    if(txPkt)
       freePkt_F(txPkt);

    OSTaskDel(OS_PRIO_SELF);
}

