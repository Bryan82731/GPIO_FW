#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <bsp_cfg.h>

#include "OOBMAC.h"
#include "OOBMAC_COMMON.h"
#include "vga.h"

extern struct DMA_INFO dma_info0;
extern void bsp_wait(INT32U usec);
extern void dma_large_RKVM(INT32U dma_size,INT32U src, INT32U des, struct DMA_INFO *dma_p);
extern void bsp_pcie_sw_handler();
extern void bsp_cmac2_Reinit(void);


extern volatile INT8U dma_flag;
extern volatile INT32U VGAdiff_flag;

INT8U * RxdescStartAddr_OOBMAC = NULL;
INT8U * TxdescStartAddr_OOBMAC = NULL;
INT8U * TallyCounterAddr = NULL;
INT8U * RXBUFFER = NULL;
INT8U * TXBUFFER = NULL;



INT32U	TxTotal=0;
INT32U	RxTotal=0;
INT32U	TxTotal_TCP4;
INT32U	TxTotal_UDP4;
INT32U	TxTotal_IP4;
INT32U	TxTotal_TCP6;
INT32U	TxTotal_UDP6;
INT32U	TxTotal_NRM;
INT32U	RxTotal_TCP4;
INT32U	RxTotal_UDP4;
INT32U	RxTotal_IP4;
INT32U	RxTotal_TCP6;
INT32U	RxTotal_UDP6;
INT32U	RxTotal_NRM;
INT32U	RxTotal_IPv6;
INT32U  RxTotal_TAG_VLAN;
INT32U  RxTotal_DETAG_VLAN;

volatile TallyCnt *TallyCntPtr_OOBMAC1 = NULL;

#ifdef MAC_RX_DATA_CHECK
INT32U  RxTotal_DataError=0;
#endif

#ifdef MAC_RX_CHECKSUM
RxCheck rxCheck= {0};
#endif

#ifdef MAC_DEBUG_COUNT
INT32U RERCnt=0;
INT32U RFOVCnt=0;
INT32U RDUCnt=0;
#endif

#ifdef GPIO_INT_TEST
INT32U Gpio_int_num=0;
#endif




#ifdef MAC_reg_InterruptMitigation
INT32U TxCnt;
#endif

#ifdef MAC_FLOW_CONTROL_TEST
INT32U RxTotal_PAUSEFRAME;
INT32U RxTotal_PAUSEFRAME_release;
#endif
INT32U  ipv4UDPsum = 0;
INT32U  ipv4TCPsum = 0;
INT32U  ipv6UDPsum = 0;
INT32U  ipv6TCPsum = 0;

INT16U	length=TXLengthMin;

#if defined(MAC_reg_InterruptMitigation) || defined(MAC_TX_SENDSLOW) || defined(MAC_LOOPBACK)
INT16U  gmac_tx_delayTime = 10;
#endif

volatile INT8U	hwtxptr;
volatile INT8U	hwrxptr;

//INT8U	TxCont=1;   //for debug



INT16U 	fail_entry[512]= {0,};
INT8U	CmacCont;
//INT8U	CmacCont=1;  //for debug
INT8U	Cmac2Cont = 0;
INT8U	Cmac2_Data0_B;
INT8U	Cmac2Disable;
INT8U 	CMAC_Reinit=0;
INT8U 	VGA_Reinit=0;

INT8U  FT_EQC_test_OOBMAC=0;
INT32U FT_OOBMAC_testNum=0x10;
INT8U  FT_OOBMAC_txFinish=0;

//extern INT8U  Switch_PCIEHOST_BypassMode;
//extern INT8U  CombineTest_PCIEHOST;
extern INT8U  FT_EQC_test_RKVM;
extern INT32U FT_RKVM_testNum;
extern volatile INT32U 	CMAC2_TXDataCnt;
extern volatile INT32U 	CMAC2_RXDataCnt;
extern volatile INT32U 	CMAC2_DataError;
extern volatile INT32U 	CMAC2_LenError;

volatile INT32U	TxCont=0;
volatile INT32U	TxCont_CS=0;
volatile INT32U	TxCont_UDP4=0;
volatile INT32U	TxCont_TCP4=0;
volatile INT32U	TxCont_TCP6=0;
volatile INT32U	TxCont_UDP6=0;
volatile INT8U	TxMixed=0;
volatile INT8U	TxStartFlag=0;
volatile INT8U	SingleTxFlag=0;
INT8U	TestFlag=0;


INT8U   isForeverLoop=1;
#ifdef MAC_SLOW_TX
INT8U	isSlowTX;
#endif
#ifdef MAC_SLOW_RX
INT32U rxdelayDuration = 20;
#endif

RXdesc	*rxd;
TXdesc 	*txd;
volatile TXdesc_COM *txd_rkvm2oobmac;


#if 1//From Han
INT32U	RxPFCount[16]= {0,};
INT8U	Gamming_error=0;

#define OOB_COUNT_ACTION		0x00000180
#define OOB_COUNT_RuleCount		0x00000184
#define OOB_COUNT_OP_RST_ALL	1
#define OOB_COUNT_OP_GET_RULE	2

typedef struct
{
    unsigned int OP_Code:4;
    unsigned int finish_Flag:1;
    unsigned int rule_No:4;
    unsigned int rev1:7;
// ---------------------------
    unsigned int rev2:16;
} IBOOB_Channel;

/*static const TCAM_Property_st TCAM_Property = {
	{
		TCAM_MAC_Start_In_TCAM,				TCAM_VLAN_Start_In_TCAM,
		TCAM_TYPE_Start_In_TCAM,			TCAM_PTLv4_Start_In_TCAM,
		TCAM_PTLv6_Start_In_TCAM,			TCAM_SIPv4_Start_In_TCAM,
		TCAM_DIPv4_Start_In_TCAM, 			TCAM_SIPv6_Start_In_TCAM,
		TCAM_DIPv6_Start_In_TCAM,			TCAM_SPORT_Start_In_TCAM,
		TCAM_DPORT_Start_In_TCAM,			TCAM_Teredo_SPORT_Start_In_TCAM,
		TCAM_Teredo_DPORT_Start_In_TCAM,	TCAM_UDP_ESP_SPORT_Start_In_TCAM,
		TCAM_UDP_ESP_DPORT_Start_In_TCAM,	TCAM_OFFSET_Start_In_TCAM,
	},
	{
		TCAM_MAC_Number_Of_Set,				TCAM_VLAN_Number_Of_Set,
		TCAM_TYPE_Number_Of_Set,			TCAM_PTLv4_Number_Of_Set,
		TCAM_PTLv6_Number_Of_Set,			TCAM_SIPv4_Number_Of_Set,
		TCAM_DIPv4_Number_Of_Set, 			TCAM_SIPv6_Number_Of_Set,
		TCAM_DIPv6_Number_Of_Set,			TCAM_SPORT_Number_Of_Set,
		TCAM_DPORT_Number_Of_Set,			TCAM_Teredo_SPORT_Number_Of_Set,
		TCAM_Teredo_DPORT_Number_Of_Set,	TCAM_UDP_ESP_SPORT_Number_Of_Set,
		TCAM_UDP_ESP_DPORT_Number_Of_Set,	TCAM_OFFSET_Number_Of_Set,
	},
	{
		TCAM_MAC_Entry_Per_Set,				TCAM_VLAN_Entry_Per_Set,
		TCAM_TYPE_Entry_Per_Set,			TCAM_PTLv4_Entry_Per_Set,
		TCAM_PTLv6_Entry_Per_Set,			TCAM_SIPv4_Entry_Per_Set,
		TCAM_DIPv4_Entry_Per_Set, 			TCAM_SIPv6_Entry_Per_Set,
		TCAM_DIPv6_Entry_Per_Set,			TCAM_SPORT_Entry_Per_Set,
		TCAM_DPORT_Entry_Per_Set,			TCAM_Teredo_SPORT_Entry_Per_Set,
		TCAM_Teredo_DPORT_Entry_Per_Set,	TCAM_UDP_ESP_SPORT_Entry_Per_Set,
		TCAM_UDP_ESP_DPORT_Entry_Per_Set,	TCAM_OFFSET_Entry_Per_Set,
	},
};
*/
#endif
//--------------------
// --------------------
void bsp_4everloop(INT16U from)
{

#ifdef MAC_PULLGPIOWHENERROR
    REG8(OOBMAC_IOBASE+MAC_GPIOCTL+1) = (REG8(OOBMAC_IOBASE+MAC_GPIOCTL+1) & (0xf0 | (1<<0)|(1<<1)));
#endif

    while (isForeverLoop && from)
    {
#if 1
        /* Watchdog timer counts from 0x7fffffff to zero.
         * We kick it again if it is smaller than 0xfffffff
         */
        /*if( ((unsigned long)REG32(0xb8000008)) <= 0xfffffff){
        	REG8(0xb800000c) = 0x76;
        }
        */
        //lisa modify for FP   watch dog
        if( ((unsigned long)REG32(0xbb500008)) <= 0xfffffff)
        {
            REG8(0xbb50000c) = 0x76;
        }
#endif
    }
}

void gmac_dump_tally_counter(void)
{
    REG32(OOBMAC_IOBASE+MAC_DTCCR)=VA2PA(TallyCounterAddr)|DWBIT03;
    while(REG32(OOBMAC_IOBASE+MAC_DTCCR)&DWBIT03);
    TallyCntPtr_OOBMAC1 = (TallyCnt *)TallyCounterAddr;
}

void gmac_reset_counter(void)
{
    TxTotal=0;
    RxTotal=0;
    TxTotal_TCP4=0;
    TxTotal_UDP4=0;
    TxTotal_IP4=0;
    TxTotal_TCP6=0;
    TxTotal_UDP6=0;
    TxTotal_NRM=0;
    RxTotal_NRM=0;
    RxTotal_TCP4=0;
    RxTotal_UDP4=0;
    RxTotal_IP4=0;
    RxTotal_TCP6=0;
    RxTotal_UDP6=0;
    RxTotal_IPv6=0;
    RxTotal_TAG_VLAN=0;
    RxTotal_DETAG_VLAN=0;
    RERCnt=0;
    RFOVCnt=0;
    RDUCnt=0;
#ifdef MAC_RX_DATA_CHECK
    RxTotal_DataError=0;
#endif
#ifdef MAC_RX_CHECKSUM
    rxCheck.RxTotal_IPv4_CheckSumErr=0;
    rxCheck.RxTotal_Normal_lenthErr=0;
    rxCheck.RxTotal_TCPIPv4_CheckSumErr=0;
    rxCheck.RxTotal_TCPIPv6_CheckSumErr=0;
    rxCheck.RxTotal_UDPIPv4_CheckSumErr=0;
    rxCheck.RxTotal_UDPIPv6_CheckSumErr=0;
#endif

}

void oobmac_sw_int_event_deal_new(void)
{
    volatile INT16U val_swint;

    REG16(OOBMAC_IOBASE + MAC_SWINT_IMR) = 0x0000;
    val_swint= REG16(OOBMAC_IOBASE + MAC_SWINT_ISR);
    REG16(OOBMAC_IOBASE + MAC_SWINT_ISR) = val_swint;




    REG16(OOBMAC_IOBASE + MAC_SWINT_IMR) = DEFAULT_SWINT_IMR;
}

void oobmac_sw_int_event_deal_old(void)
{
#ifdef PKTFilter_Test
    //INT16U			pkt_count=0;
    INT32U			temp = 0;
    IBOOB_Channel st_IBOOB_Channel = {0,};
#endif

    /*start add by junjiang for pcie host*/
#ifdef CONFIG_PCIE_DEV_ENABLED
    //bsp_pcie_sw_handler();
#endif
    /*end add by junjiang for pcie host*/

#ifdef PKTFilter_Test
    temp = REG32(OOBMAC_IOBASE+OOB_COUNT_ACTION);
    memcpy(&st_IBOOB_Channel ,&temp, sizeof(st_IBOOB_Channel));
    if(st_IBOOB_Channel.OP_Code == OOB_COUNT_OP_RST_ALL)
    {
        for(i=0; i<16; i++)
            RxPFCount[i] = 0;
        st_IBOOB_Channel.finish_Flag = 1;
        memcpy(&temp ,&st_IBOOB_Channel, sizeof(st_IBOOB_Channel));
        REG32(OOBMAC_IOBASE+OOB_COUNT_ACTION) = temp;
    }
    else if(st_IBOOB_Channel.OP_Code == OOB_COUNT_OP_GET_RULE)
    {
        REG32(OOBMAC_IOBASE+OOB_COUNT_RuleCount) = RxPFCount[st_IBOOB_Channel.rule_No];
        st_IBOOB_Channel.finish_Flag = 1;
        memcpy(&temp ,&st_IBOOB_Channel, sizeof(st_IBOOB_Channel));
        REG32(OOBMAC_IOBASE+OOB_COUNT_ACTION) = temp;
    }
#else
    //----------
    //Initialize CMAC OOB Send
    if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_CMAC_TX)
    {
        CmacCont = 1;
    }
    //Stop CMAC OOB Send
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_CMAC_STOPTX)
    {
        CmacCont = 0;
    }
    //Initialize CMAC2 OOB Send
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_CMAC2_TX)
    {
        Cmac2Cont = 1;


//		bsp_cmac2_Reinit();

    }
    //Stop CMAC2 OOB Send
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_CMAC2_STOPTX)
    {
        Cmac2Cont = 0;
    }
    //Disable CMAC2
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_CMAC2_DISABLE)
    {
        Cmac2Disable = 1;
    }
    //Initialize OOB Mixed TX Test
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_GMAC_TX)
    {
        TxStartFlag = 1;
        TxMixed = 1;
        TxCont = 1;
        TxCont_TCP4 = 1;
        TxCont_UDP4 = 1;
        TxCont_CS = 1;
        TxCont_TCP6 = 1;
        TxCont_UDP6 = 1;

    }
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_GMAC_SINGLE_TX)
    {
        SingleTxFlag = 1;

    }
    //Stop OOB Mixed TX Test
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_GMAC_STOPTX)
    {
        TxStartFlag = 0;
        TxMixed = 0;
        TxCont = 0;
        TxCont_TCP4 = 0;
        TxCont_UDP4 = 0;
        TxCont_CS = 0;
        TxCont_TCP6 = 0;
        TxCont_UDP6 = 0;
    }
#ifdef MAC_SLOW_TX
    else if(REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_GMAC_TXSLOWENABLE)
    {
        isSlowTX = 1;
    }
    else if(REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_GMAC_TXSLOWDISABLE)
    {
        isSlowTX = 0;
    }
#endif
    //IB will write CMAC reset when start
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_CMAC_RESET)
    {
        CMAC_Reinit=1;
    }
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_CombineTest)
    {
        //OOB_CombineTest=1;
        ;
    }
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == 0x4F)
    {
        //VGA_Reinit=1;
        ;
    }
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_FT_EQC_test_OOBMAC)
    {
        FT_EQC_test_OOBMAC=1;
        //FT_OOBMAC_testNum = REG32(OOBMAC_IOBASE+MAC_DUMMY_INFORM_IB_DATA);
        FT_OOBMAC_testNum=0x10;
        gmac_reset_counter();

    }
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_FT_EQC_test_stop_OOBMAC)
    {
        FT_EQC_test_OOBMAC=0;
    }
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_FT_EQC_test_RKVM)
    {
        FT_EQC_test_RKVM=1;
        //FT_RKVM_testNum = REG32(OOBMAC_IOBASE+MAC_DUMMY_INFORM_IB_DATA);
        bsp_cmac2_Reinit();
    }
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_FT_EQC_test_stop_RKVM)
    {
        FT_EQC_test_RKVM=0;
    }
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_SWITCH_PCIEHOST_BYPASSMODE)
    {
        //Switch_PCIEHOST_BypassMode=1;
        ;
    }
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_CombineTest_PCIEHOST)
    {
        //CombineTest_PCIEHOST=1;
        ;
    }
    else if (REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_CombineTest_STOP_PCIEHOST)
    {
        //CombineTest_PCIEHOST=0;
        ;
    }



#ifdef MAC_DEBUG_COUNT
    else if(REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_GMAC_DUMPRER)
    {
        REG32(OOBMAC_IOBASE + MAC_DMEMSTA) = RERCnt;
    }
    else if(REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_GMAC_DUMPRFO)
    {
        REG32(OOBMAC_IOBASE + MAC_DMEMSTA) = RFOVCnt;
    }
    else if(REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_GMAC_DUMPRX)
    {
        REG32(OOBMAC_IOBASE + MAC_DMEMSTA) = RxTotal;
    }
    else if(REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_GMAC_DUMPTX)
    {
        REG32(OOBMAC_IOBASE + MAC_DMEMSTA) = TxTotal;
    }
    else if(REG8(OOBMAC_IOBASE + MAC_DMEMSTA) == OOB_GMAC_RESETCNT)
    {
        TxTotal=0;
        RxTotal=0;
        TxTotal_TCP4=0;
        TxTotal_UDP4=0;
        TxTotal_IP4=0;
        TxTotal_TCP6=0;
        TxTotal_UDP6=0;
        RxTotal_TCP4=0;
        RxTotal_UDP4=0;
        RxTotal_IP4=0;
        RxTotal_TCP6=0;
        RxTotal_UDP6=0;
        RxTotal_IPv6=0;
        RxTotal_NRM=0;
        TxTotal_NRM=0;
        RxTotal_TAG_VLAN=0;
        RxTotal_DETAG_VLAN=0;
        RERCnt=0;
        RFOVCnt=0;
        RDUCnt=0;
    }
#endif
    else
    {
    }
#endif
}


void oobmac_normal_event_deal(void)
{
    //--------------------
    volatile INT16U	length=0;
    volatile INT16U val;
    INT8U i;
    INT8U * rddataptr;
    //----------
#ifdef MAC_RX_DATA_CHECK
    INT32U u = 0;
    INT16U iphdrlen = 0;
    INT16U tcphdrlen = 0;
#endif
    //--------------------
#ifdef PKTFilter_Test
    //INT16U			pkt_count=0;
    INT32U			temp = 0;
    IBOOB_Channel st_IBOOB_Channel = {0,};
#endif

    //--------------------
    REG16(OOBMAC_IOBASE + MAC_IMR) = 0x0000;
    //--------------------
    val = REG16(OOBMAC_IOBASE + MAC_ISR);
    REG16(OOBMAC_IOBASE + MAC_ISR) = val;
    //--------------------
    do
    {
        //--------------------
        //software interrupt
        if ((val & OCP_SW_INT) != 0)
        {
            oobmac_sw_int_event_deal_old();
        }

#ifdef MAC_DEBUG_COUNT
        if(val & OCP_RER)
        {
            RERCnt++;
        }
        if(val & OCP_FOVW)
        {
            RFOVCnt++;
        }
        if(val & OCP_RDU)
        {
            RDUCnt++;
        }
#endif

#ifdef MAC_reg_InterruptMitigation
        if(val & OCP_TOK)
        {
            TxCnt++;
        }
#endif

        if ((val & OCP_ROK) || (val & OCP_RDU))
        {
            for (i = 0; i < RxdescNumber_OOBMAC; i++)
            {
                rxd = (RXdesc *) (RxdescStartAddr_OOBMAC) + hwrxptr;
                if (rxd->OWN == 1)
                {
                    break;
                }

#ifdef PKTFilter_Test
                if(rxd->PKTFNO%2 == 0)
                {
                    if(rxd->GAME != 1)
                    {
                        //while(1){}
                        Gamming_error=1;
                        Gamming_error=1;
                        Gamming_error=1;
                    }
                }
                else
                {
                    if(rxd->GAME != 0)
                    {
                        //while(1){}
                        Gamming_error=1;
                        Gamming_error=1;
                        Gamming_error=1;
                    }
                }
                RxPFCount[rxd->PKTFNO]++;
                rxd->Length = 0x600;
                //Release RX descriptor
                rxd->OWN = 1;
                //Update descriptor pointer
                hwrxptr = (hwrxptr + 1) % RxdescNumber_OOBMAC;
                break;
#endif

                rddataptr = (INT8U *) PA2VA(rxd->BufferAddress);
                /*TAVA is available while RxConf ENRX_VLAN is on*/
                if(rxd->TAVA)
                {
                    RxTotal_DETAG_VLAN++;
                }

                if (rxd->V4F)   //IPv4 Packet
                {
                    if (rxd->TCPT)  //IPv4 TCP Packet
                    {
                        RxTotal_TCP4++;
                        RxTotal++;
#ifdef MAC_RX_CHECKSUM
                        if ((rxd->TCPF == 0) && (rxd->IPF == 0) && (rxd->UDPF))
                        {
#ifdef MAC_RX_DATA_CHECK
                            iphdrlen = (rddataptr[14] - 0x40) * 4;
                            tcphdrlen = (rddataptr[iphdrlen + 14 + 12] >> 4) * 4;
                            for (u = (14 + iphdrlen + tcphdrlen + 1); u < (rxd->Length - 4); u++)
                            {
                                if (*(rddataptr + u) != (INT8U)(*(rddataptr + u - 1) + 1))
                                {
                                    //	bsp_4everloop(1);
                                    RxTotal_DataError++;
                                }
                            }
#endif
                        }
                        else
                        {
                            //	bsp_4everloop(2);
                            rxCheck.RxTotal_TCPIPv4_CheckSumErr++;
                        }
#endif
                    }
                    else if (rxd->UDPT)    //IPv4 UDP / IP Packet
                    {
                        RxTotal_UDP4++;
                        RxTotal++;
#ifdef MAC_RX_CHECKSUM
                        if ((rxd->TCPF) && (rxd->IPF == 0) && (rxd->UDPF == 0))
                        {
#ifdef MAC_RX_DATA_CHECK
                            iphdrlen = (rddataptr[14] - 0x40) * 4;
                            for (u = (14 + iphdrlen + 8 + 1); u < (rxd->Length - 4); u++)
                            {
                                if (*(rddataptr + u) != (INT8U)(*(rddataptr + u - 1) + 1))
                                {
                                    //	bsp_4everloop(3);
                                    RxTotal_DataError++;
                                }
                            }
#endif
                        }
                        else
                        {
                            //	bsp_4everloop(4);
                            rxCheck.RxTotal_UDPIPv4_CheckSumErr++;
                        }
#endif
                    }
                    else    //IPv4 IP - Only Packet
                    {
                        RxTotal_IP4++;
                        RxTotal++;
#ifdef MAC_RX_CHECKSUM
                        if ((rxd->TCPF) && (rxd->IPF == 0) && (rxd->UDPF))
                        {
#ifdef MAC_RX_DATA_CHECK
                            //-----------mask by lisa-----------------------
                            //for (u = (14 + 12 + 1); u < (rxd->Length - 4); u++) {
                            //--------------add by lisa ---------------------
                            for (u = (14 + 20 + 1); u < (rxd->Length - 4); u++)
                            {
                                //--------------------------------------------------------
                                if (*(rddataptr + u) != (INT8U)(*(rddataptr + u - 1) + 1))
                                {
                                    //	bsp_4everloop(5);
                                    RxTotal_DataError++;
                                }
                            }
#endif
                        }
                        else
                        {
                            //	bsp_4everloop(6);
                            rxCheck.RxTotal_IPv4_CheckSumErr++;
                        }
#endif
                    }
                }
                //-----
                else if (rxd->V6F)
                    //IPv6 Packet
                {
                    if (rxd->TCPT)
                        //IPv6 TCP Packet
                    {
                        RxTotal_TCP6++;
                        RxTotal++;
#ifdef MAC_RX_CHECKSUM
                        if ((rxd->TCPF == 0) && (rxd->UDPF))
                        {
#ifdef MAC_RX_DATA_CHECK
                            iphdrlen = 40;
                            if (*(rddataptr + 20) != 0x06)
                            {
                                do
                                {
                                    i = *(rddataptr + 14 + iphdrlen);
                                    iphdrlen = iphdrlen + 8 *(*(rddataptr + 14 + iphdrlen + 1) + 1);
                                }
                                while (i != 0x06);
                            }
                            tcphdrlen = 20;
                            for (u = (14 + iphdrlen + tcphdrlen + 1); u < (rxd->Length - 4); u++)
                            {
                                if (*(rddataptr + u) != (INT8U)(*(rddataptr + u - 1) + 1))
                                {
                                    //	bsp_4everloop(7);
                                    RxTotal_DataError++;
                                }
                            }
#endif
                        }
                        else
                        {
                            //	bsp_4everloop(8);
                            rxCheck.RxTotal_TCPIPv6_CheckSumErr++;
                        }
#endif
                    }
                    else if (rxd->UDPT)
                        //IPv6 UDP Packet
                    {
                        RxTotal_UDP6++;
                        RxTotal++;

#ifdef MAC_RX_CHECKSUM
                        if ((rxd->TCPF) && (rxd->UDPF == 0))
                        {
#ifdef MAC_RX_DATA_CHECK
                            iphdrlen = 40;
                            if (*(rddataptr + 20) != 0x11)
                            {
                                do
                                {
                                    i = *(rddataptr + 14 + iphdrlen);
                                    iphdrlen = iphdrlen + 8 * (*(rddataptr + 14 + iphdrlen + 1) + 1);
                                }
                                while (i != 0x11);
                            }
                            tcphdrlen = 8;
                            for (u = (14 + iphdrlen + tcphdrlen + 1);
                                    u < (rxd->Length - 4); u++)
                            {
                                if (*(rddataptr + u) != (INT8U)(*(rddataptr + u - 1) + 1))
                                {
                                    //	bsp_4everloop(9);
                                    RxTotal_DataError++;
                                }
                            }
#endif
                        }
                        else
                        {
                            //	bsp_4everloop(10);
                            rxCheck.RxTotal_UDPIPv6_CheckSumErr++;
                        }
#endif
                    }
                    else
                        //IPv6 with Extension Header NOT supportted
                    {
                        RxTotal_IPv6++;
                        RxTotal++;
#ifdef MAC_RX_DATA_CHECK
                        iphdrlen = 40;
                        if (*(rddataptr + 20) != 0x06 && *(rddataptr + 20) != 0x11)
                        {
                            do
                            {
                                i = *(rddataptr + 14 + iphdrlen);
                                iphdrlen = iphdrlen + 8 * (*(rddataptr + 14 + iphdrlen + 1) + 1);
                            }
                            while ((i != 0x06) && (i != 0x11));
                        }
                        else
                        {
                            i = *(rddataptr + 20);
                        }
                        if (i == 0x06)
                            tcphdrlen = 20;
                        else
                            tcphdrlen = 8;
                        for (u = (14 + iphdrlen + tcphdrlen + 1); u < (rxd->Length - 4); u++)
                        {
                            if (*(rddataptr + u) != (INT8U)(*(rddataptr + u - 1) + 1))
                            {
                                //	bsp_4everloop(11);
                                RxTotal_DataError++;
                            }
                        }
#endif
                    }
                }
#ifdef MAC_TEST_VLAN_RX
                else if(!rxd->TAVA && rddataptr[12] == 0x81 && rddataptr[13] == 0x0)
                {
                    if(rddataptr[14] == (VLANDESC & 0xff) && rddataptr[15] == (VLANDESC & 0xff00)>>8)
                    {
                        RxTotal_TAG_VLAN++;
                        RxTotal++;
                    }
                    else
                    {
                        //	bsp_4everloop(12);
                        rxCheck.RxTotal_Vlan_Unmatched++;
                    }
                }
#endif
#ifdef MAC_FLOW_CONTROL_TEST
                else if((rddataptr[0]==0x01 && rddataptr[1]==0x80 && rddataptr[2]==0xc2 && rddataptr[3]==0x00 && rddataptr[4]==0x00 && rddataptr[5]==0x01) &&
                        rddataptr[12] == 0x88 && rddataptr[13] == 0x08 && rddataptr[14]==0x00 && rddataptr[15]==0x01)
                {
                    /*This is pause frame*/
                    if(rddataptr[16]==0 && rddataptr[17]==0)
                    {
                        RxTotal_PAUSEFRAME_release++;
                    }
                    else
                    {
                        RxTotal_PAUSEFRAME++;
                    }
                    RxTotal++;
                }
#endif
                else
                    //Normal Packet
                {
                    RxTotal_NRM++;
                    RxTotal++;
                    //Calculate Length
                    //------------------add by lisa--2015.3.13-----------------
#ifdef MAC_RX_CHECKSUM
                    length=(*(rddataptr + 12))*256+(*(rddataptr + 13));   //payload_length
                    if (((rxd->Length - 4-14) == length)
                            || (length < 60 && (rxd->Length == 64))
                            || length == 0xFFFF)
                    {
#ifdef MAC_RX_DATA_CHECK
                        for (u = 14 + 1; u < (rxd->Length - 4); u++)
                        {
                            if (*(rddataptr + u) != (INT8U)(*(rddataptr + u - 1) + 1))
                            {
                                //		bsp_4everloop(13);
                                RxTotal_DataError++;    //serial data check
                            }
                        }
                        REG8(CPU1_IOBASE + 0x2b) = RxTotal;
#endif
                    }
                    else
                    {
                        //	bsp_4everloop(14);
                        rxCheck.RxTotal_Normal_lenthErr++;
                    }
#endif
                }
#ifdef MAC_SLOW_RX
                bsp_wait(rxdelayDuration);
#endif
                rxd->Length = 0x600;
                //Release RX descriptor
                rxd->OWN = 1;
                //Update descriptor pointer
                hwrxptr = (hwrxptr + 1) % RxdescNumber_OOBMAC;
            }
        }
        val = REG16(OOBMAC_IOBASE + MAC_ISR);
        REG16(OOBMAC_IOBASE + MAC_ISR) = val;
        val = val & DEFAULT_IMR;
    }
    while (val != 0);
    REG16(OOBMAC_IOBASE + MAC_IMR) = DEFAULT_IMR;
}

void bsp_oobmac_handler(void)
{
    volatile INT16U oobmac_temp=0,swint_temp=0;

#ifdef MAC_reg_SWInterrupts   //newly added in 8111FPvB
    swint_temp = REG16(OOBMAC_IOBASE + MAC_SWINT_ISR) & REG16(OOBMAC_IOBASE + MAC_SWINT_IMR);
#endif

    oobmac_temp = REG16(OOBMAC_IOBASE + MAC_ISR) & REG16(OOBMAC_IOBASE + MAC_IMR);

#ifdef MAC_reg_SWInterrupts   //newly added in 8111FPvB
    if(swint_temp)
    {
        oobmac_sw_int_event_deal_new();
    }
#endif

    if(oobmac_temp)
    {
        oobmac_normal_event_deal();
    }
}

void test_free_oobmac(void)
{
    TXdesc *txdesc;
    RXdesc *rxdesc;
    INT16U i;

    if(RxdescStartAddr_OOBMAC!= NULL)
    {
        free(RxdescStartAddr_OOBMAC);
    }
    if(TxdescStartAddr_OOBMAC!= NULL)
    {
        free(TxdescStartAddr_OOBMAC);
    }
    if(TallyCounterAddr != NULL)
    {
        free(TallyCounterAddr);
    }
    for(i=0; i<TxdescNumber_OOBMAC; i++)
    {
        txdesc = ((TXdesc *)TxdescStartAddr_OOBMAC)+i;
        if(PA2VA(txdesc->BufferAddress) != NULL)
        {
            free(PA2VA(txdesc->BufferAddress));
        }
    }
    for(i=0; i<RxdescNumber_OOBMAC; i++)
    {
        rxdesc = ((RXdesc *)RxdescStartAddr_OOBMAC)+i;
        if(PA2VA(rxdesc->BufferAddress) != NULL)
        {
            free(PA2VA(rxdesc->BufferAddress));
        }
    }

}

void bsp_oobmac_init(void)
{
    INT32U value;
    INT16U i;
    INT8U RCRvalue;
    INT16U entry_number;
    INT16U test_num;
    INT16U DataBit=0;
    INT8U Valid=0;
    INT8U loop=1;
    TCAM_Entry_Setting_st stTCAM_Table= {0,};
    TCAM_Entry_Setting_st *pstTCAM_Table=NULL;

    /*Disable tx rx*/
    REG8(OOBMAC_IOBASE+MAC_CMD)=0x0;
    /*DASH Enable*/
    REG8(OOBMAC_IOBASE+MAC_OOBREG)=REG8(OOBMAC_IOBASE+MAC_OOBREG)|0x01;           //mask at moment    dash not use

    bsp_oobmac_init_packet();
//	srand((unsigned)TXBUFFER);

    REG32(OOBMAC_IOBASE+MAC_MAR0)=0x0;
    REG32(OOBMAC_IOBASE+MAC_MAR4)=0x0;
    /* Assign Tx and Rx descriptor address */
    REG32(OOBMAC_IOBASE+MAC_RDSAR)=VA2PA(RxdescStartAddr_OOBMAC);
    REG32(OOBMAC_IOBASE+MAC_TNPDS)=VA2PA(TxdescStartAddr_OOBMAC);

#if 1
    /* enable hw checksum & set interrupt timer unit */
    REG16(OOBMAC_IOBASE+MAC_CPCR)=0x0021;
#else
    /*No checksum offload. Rx descriptor will not record real packet type*/
    REG16(OOBMAC_IOBASE+MAC_CPCR)=0x0001;
#endif

    REG8(OOBMAC_IOBASE+MAC_RxCR) = REG8(OOBMAC_IOBASE+MAC_RxCR) & 0x0f;
    RCRvalue= REG8(OOBMAC_IOBASE+MAC_RxCR);
#ifdef MAC_TEST_VLAN_RXDETAGING
    RCRvalue |= RCR_VLANDETAGGING;
    /*Accept Bro,Multi, and physical match packets. Reject flow control, error, runt packets*/
    REG8(OOBMAC_IOBASE+MAC_RxCR)   = RCRvalue;
#endif

#ifdef MAC_ACCEPT_FLOW_CTL
    RCRvalue |= RCR_AFL;
    REG8(OOBMAC_IOBASE+MAC_RxCR)   = RCRvalue;
#endif


#ifdef MAC_FLOW_CONTROL_TEST
    //REG8(OOBMAC_IOBASE+MAC_RxCR) |= (1<<6);
    /*Set RxConf Rx_FIFO_Cond_SEL(bit 10, 9) to (0, 1)*/
    REG8(OOBMAC_IOBASE+MAC_RxCR+1) = 0x2; //OOB rx fifo nearly full/empty condition for flow control
#endif

#if 1//Setup RMS
    REG16(OOBMAC_IOBASE+MAC_RxCR+2) = 0x600;
#endif

    //Interrupt Configuration
    REG16(OOBMAC_IOBASE+MAC_IMR)=0x0000;
    REG16(OOBMAC_IOBASE+MAC_ISR)=0xFFFF;
    REG16(OOBMAC_IOBASE+MAC_IMR)=DEFAULT_IMR;
    REG16(OOBMAC_IOBASE+MAC_SWINT_IMR)=0x0000;
    REG16(OOBMAC_IOBASE+MAC_SWINT_ISR)=0xFFFF;
    REG16(OOBMAC_IOBASE+MAC_SWINT_IMR)=DEFAULT_SWINT_IMR;
#ifdef TXUNICAST
    REG32(OOBMAC_IOBASE+MAC_IDR0)=0x00054ce0;
    REG32(OOBMAC_IOBASE+MAC_IDR4)=0x00000001;
#else
    /*Write MAC Addr*/
    REG32(OOBMAC_IOBASE+MAC_IDR0)=0x00000067;  //0x00000067
    REG32(OOBMAC_IOBASE+MAC_IDR4)=0x00000000;
#endif
#if 1//From Han
    PacketFillDefault();
#endif

#ifdef TXUNICAST
    /*Write TCAM macid1-0*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x000000e0;
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010003;
#else
    /*Write TCAM macid1-0*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x00006700;   //0x00000067
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010003;
#endif
    while(((value=REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)) & 0x80000000) == 0x80000000);

#ifdef TXUNICAST
    /*Write TCAM macid1-1*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x00004c05;
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010004;
#else
    /*Write TCAM macid1-1*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x00000000;
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010004;
#endif
    while(((value=REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)) & 0x80000000) == 0x80000000);

#ifdef TXUNICAST
    /*Write TCAM macid1-2*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x00000001;
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010005;
#else
    /*Write TCAM macid1-2*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x00000000;
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010005;
#endif
    while(((value=REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)) & 0x80000000) == 0x80000000);

    /*Write Packet Rule 1*/
    REG32(OOBMAC_IOBASE+MAC_PKT_RULE1)=0x00000002;
    /*Write Rule Act*/
//	REG32(OOBMAC_IOBASE+MAC_PKT_RULE_ACT0)=0x00000201;
    REG32(OOBMAC_IOBASE+MAC_PKT_RULE_ACT0)=0x00000301;  //IB drop the matched packet & OOB receive it
    /*Write packet rule enable*/
    REG32(OOBMAC_IOBASE+MAC_PKT_RULE_EN)=0x80000002;
    REG32(OOBMAC_IOBASE+0x254)=0x00000000;

    hwtxptr = 0x00;
    hwrxptr = 0x00;
    //Enable TX/RX
#if 1//For phase 2
    i = 0;
    /*Check SYNC_RXEN and SYNC_TXEN for 255 times*/
    while((REG8(OOBMAC_IOBASE+MAC_CMD+1) & 0x3) && i++ < 0xff)
    {
        bsp_wait(1);
    }
    if(i < 0xff)
    {
        REG8(OOBMAC_IOBASE+MAC_CMD)=0x0C;
    }
    else
    {
        //	bsp_4everloop(15);
    }
#else
    REG8(OOBMAC_IOBASE+MAC_CMD)=0x0C;
#endif

#ifdef PKTFilter_Test
#if 0
    //tcam bist
    REG8(OOBMAC_IOBASE+0x402)=0x20;
    REG8(OOBMAC_IOBASE+0x402)=0x28;
    while((REG8(OOBMAC_IOBASE+0x402)&0x01)!=0x01) {}
    if((REG8(OOBMAC_IOBASE+0x410)&0x10)==0x10)
    {
        //tcam fault
        while(loop) {}
    }
    REG8(OOBMAC_IOBASE+0x402)=0;
#endif
#if 0
    REG8(OOBMAC_IOBASE+0x402)=0x40;
    REG8(OOBMAC_IOBASE+0x402)=0x50;
    while((REG8(OOBMAC_IOBASE+0x402)&DWBIT02)!=DWBIT02) {}
    bsp_wait(1000*1000);
    REG8(OOBMAC_IOBASE+0x402)=0xD0;
    while((REG8(OOBMAC_IOBASE+0x402)&DWBIT02)!=DWBIT02) {}
    bsp_wait(1000*1000);
    REG8(OOBMAC_IOBASE+0x402)=0xD0;
    while((REG8(OOBMAC_IOBASE+0x402)&DWBIT01)!=DWBIT01) {}
    if((REG8(OOBMAC_IOBASE+0x410)&DWBIT05) == DWBIT05)
    {
        while(loop) {}
    }

    REG8(OOBMAC_IOBASE+0x403)|=0x01;
    REG8(OOBMAC_IOBASE+0x402)=0x40;
    REG8(OOBMAC_IOBASE+0x402)=0x50;
    while((REG8(OOBMAC_IOBASE+0x402)&DWBIT02)!=DWBIT02) {}
    bsp_wait(1000*1000);
    REG8(OOBMAC_IOBASE+0x402)=0xD0;
    while((REG8(OOBMAC_IOBASE+0x402)&DWBIT02)!=DWBIT02) {}
    bsp_wait(1000*1000);
    REG8(OOBMAC_IOBASE+0x402)=0xD0;
    while((REG8(OOBMAC_IOBASE+0x402)&DWBIT01)!=DWBIT01) {}
    if((REG8(OOBMAC_IOBASE+0x410)&DWBIT05) == DWBIT05)
    {
        while(loop) {}
    }
    REG8(OOBMAC_IOBASE+0x402)=0;
#endif
#if 0
    while(loop)
    {
        //init write data=0x5a5a and valid=1
        for(entry_number=0; entry_number<512; entry_number++)
        {
            TCAM_OCP_Write(0x5a5a, 0, 1, entry_number, Switch_ON, Switch_ON, Switch_ON);
        }
        //write data=0xa5a5 and valid=0
        //for(entry_number=511; entry_number>=0; entry_number--)
        entry_number=512;
        while(entry_number)
        {
            entry_number--;
            pstTCAM_Table = &stTCAM_Table;
            memset(pstTCAM_Table, 0, sizeof(TCAM_Entry_Setting_st));
            __TCAM_OCP_Read(entry_number, TCAM_data, pstTCAM_Table);
            __TCAM_OCP_Read(entry_number, TCAM_care, pstTCAM_Table);
            if(stTCAM_Table.Value!=0x5a5a || stTCAM_Table.DontCareBit!=0xa5a5 || stTCAM_Table.Valid != 1)
            {
                fail_entry[entry_number]=1;
            }
            TCAM_OCP_Write(0xa5a5, 0, 0, entry_number, Switch_ON, Switch_ON, Switch_ON);
        }
    }
#endif
#if 0
    for(entry_number=0; entry_number<512; entry_number++)
    {
        TCAM_OCP_Write(0x5a5a, 0, 1, entry_number, Switch_ON, Switch_ON, Switch_ON);
    }
    entry_number=0x32;
    TCAM_OCP_Write(0x1234, 0, 0, entry_number, Switch_ON, Switch_ON, Switch_ON);
#endif
#if 0
    for(test_num=0; test_num<512; test_num++)
    {
        for(entry_number=0; entry_number<512; entry_number++)
        {
            TCAM_OCP_Write(0x5a5a, 0, 1, entry_number, Switch_ON, Switch_ON, Switch_ON);
            TCAM_OCP_Write(0x1234, 0, 0, test_num, Switch_ON, Switch_ON, Switch_ON);
            pstTCAM_Table = &stTCAM_Table;
            __TCAM_OCP_Read(entry_number, TCAM_care, pstTCAM_Table);
            if(pstTCAM_Table->Valid != 0x01)
            {
                if(entry_number == test_num)
                    continue;
                fail_entry[entry_number]=1;
            }
        }
    }
#endif
#if  0
    for(entry_number=0; entry_number<512; entry_number++)
    {
        TCAM_OCP_Write(0x5a5a, 0, 1, entry_number, Switch_ON, Switch_ON, Switch_ON);
        //TCAM_OCP_Write(0x5a5a, 0, 1, 0x40, Switch_ON, Switch_ON, Switch_ON);
        TCAM_OCP_Write(0x1234, 0, 0, 0x32, Switch_ON, Switch_ON, Switch_ON);
        pstTCAM_Table = &stTCAM_Table;
        __TCAM_OCP_Read(entry_number, TCAM_care, pstTCAM_Table);
        if(pstTCAM_Table->Valid != 0x01)
        {
            fail_entry[entry_number]=1;
        }
    }
#endif
#endif
    gmac_reset_counter();
}

void __inline__ updateTimer(void)
{
#if 1
    /* Watchdog timer counts from 0x7fffffff to zero.
     * We kick it again if it is smaller than 0xfffffff
     */
    /*if(((unsigned long)REG32(0xb8000008)) <= 0xfffffff){
    	REG8(0xb800000c) = 0x76;
    }*/
    //lisa modify for FP Watch dog   2015-06-09
    if(((unsigned long)REG32(0xbb500008)) <= 0xfffffff)
    {
        REG8(0xbb50000c) = 0x76;
    }

#endif
}

void bsp_oobmac_disable(void)
{

    //-----------lisa add-------------------------------
    /*modify Rule1 Act*/
    //Even the Rule1 is matched,OOB will drop the packet(stop oob rx).The packet can also be sent to IB MAC.
    REG32(OOBMAC_IOBASE+MAC_PKT_RULE_ACT0)=0x00000001;
    //-------------------------------------------------
    /*Check TPPoll.*/

    INT8U TPPoll;
    INT8U MAC_CMD1;
    TPPoll=REG8(OOBMAC_IOBASE+MAC_TPPoll);
    while((REG8(OOBMAC_IOBASE+MAC_TPPoll) & 0x80))
    {
        /*Wait TX finish*/

        TPPoll=REG8(OOBMAC_IOBASE+MAC_TPPoll);

        updateTimer();
    }
    MAC_CMD1=REG8(OOBMAC_IOBASE+MAC_CMD+1);
    while((REG8(OOBMAC_IOBASE+MAC_CMD+1) & 0x0c))
    {
        /*Wait engine goes down*/

        MAC_CMD1=REG8(OOBMAC_IOBASE+MAC_CMD+1);
        updateTimer();
    }
    REG8(OOBMAC_IOBASE+MAC_CMD) = 0x00;
    while((REG8(OOBMAC_IOBASE+MAC_CMD+1) & 0x3))
    {
        /*Wait engine goes down*/
        updateTimer();
    }
}
void bsp_oobmac_enable(void)
{
    volatile RXdesc *rxdesc;
    volatile TXdesc *txdesc;
    INT8U i;

    hwrxptr=0;
    hwtxptr=0;
    for(i=0; i<RxdescNumber_OOBMAC; i++)
    {
        rxdesc = ((RXdesc *)RxdescStartAddr_OOBMAC)+i;
        rxdesc->OWN = 1;
        rxdesc->Length = 0x600;
    }
    for(i=0; i <TxdescNumber_OOBMAC; i++)
    {
        txdesc=((TXdesc *)TxdescStartAddr_OOBMAC)+i;
        txdesc->OWN=0;
    }
    REG16(OOBMAC_IOBASE+MAC_ISR) = 0xFFFF;
    REG16(OOBMAC_IOBASE+MAC_IMR) = DEFAULT_IMR;
    REG16(OOBMAC_IOBASE+MAC_SWINT_ISR) = 0xFFFF;
    REG16(OOBMAC_IOBASE+MAC_SWINT_IMR) = DEFAULT_SWINT_IMR;
    /*Each time we need to turn on tx/rx, we have to check that hardware is down*/
    while((REG8(OOBMAC_IOBASE+MAC_CMD+1) & 0x3))
    {
        /*Wait engine goes down*/
        updateTimer();
    }

    //-----------lisa add-------------------------------
    /*resume Rule1 Act*/
    //when the Rule1 is matched,the packet will be sent to OOB MAC.The packet can also be sent to IB MAC.
    REG32(OOBMAC_IOBASE+MAC_PKT_RULE_ACT0)=0x00000201;
    //-------------------------------------------------

    REG8(OOBMAC_IOBASE +MAC_CMD) = 0x0C;


}
void bsp_oobmac_reset(void)
{

#ifdef MAC_RESET_TEST
    bsp_oobmac_disable();
#endif

    /*	#ifdef MAC_CPU_DOWNSPEED
    	//REG32(0xb9000004) = (REG32(0xb9000004)?0:2);
    	REG32(0xbb000004) = (REG32(0xbb000004)?0:2);   //lisa modify for FP CPU config1
    	#endif*/

#ifdef MAC_RESET_TEST
    bsp_oobmac_enable();
#endif
}

//--------------------
#if 0//From HAN
Ret_Code_et TCAM_OCP_Write(INT16U DataBit, INT16U DontCareBit, INT8U Valid, INT16U entry_number,
                           ONOFF_Switch_et data, ONOFF_Switch_et care, ONOFF_Switch_et valid)
{
    INT32U reg_data = 0;

    reg_data |= (DontCareBit<<16)&0xFFFF0000;
    reg_data |= DataBit;
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA) = reg_data;

    reg_data = 0;
    /*TCAM_ACC_FLAG*/
    reg_data |= DWBIT31;
    /*RW_SEL*/
    reg_data |= DWBIT30;
    /*VALID BIT*/
    reg_data |= (Valid&0x01)<<16;
    //---mask by lisa for FP-------- bit 13,14,15 not use when tcam_wr,default 0---------------------
    /*TCAM_WM : valid*/
    //if(data == Switch_OFF)
    //reg_data |= DWBIT15;
    /*TCAM_WM : care*/
    //if(care == Switch_OFF)
    //reg_data |= DWBIT14;
    /*TCAM_WM : data*/
    //if(valid == Switch_OFF)
    //reg_data |= DWBIT13;
    //--------------------------------------
    /*TCAM_ADDR*/
    reg_data |= (entry_number&0x000001FF);

    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT) = reg_data;
    while((DWBIT31&REG32(OOBMAC_IOBASE+MAC_TCAM_PORT))== DWBIT31);

    return RET_OK;
}
Ret_Code_et TCAM_OCP_Read(INT32U entry_number, TCAM_ReadType_et type, TCAM_Entry_Setting_st *pstTCAM_Table)
{
    volatile INT32U reg_data = 0;
    if(entry_number >TCAM_Entry_Number)
        return RET_FAIL;

    /*TCAM_ACC_FLAG*/
    reg_data |= DWBIT31;
    /*RW_SEL*/
    reg_data &= ~DWBIT30;
    /*TCAM_WM : read type select*/
    if(type == TCAM_data)
        reg_data |= DWBIT13;
    else
        reg_data &= ~DWBIT13;
    /*TCAM_ADDR*/
    reg_data |= entry_number&0x000001FF;

    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=reg_data;
    while((DWBIT31&REG32(OOBMAC_IOBASE+MAC_TCAM_PORT))== DWBIT31) {};

    reg_data = 0;
    reg_data = REG32(OOBMAC_IOBASE+MAC_TCAM_DOUT);

    if(type == TCAM_data)
        pstTCAM_Table->Value = 0x0000FFFF & reg_data;
    else
        pstTCAM_Table->DontCareBit = 0x0000FFFF & reg_data;

    pstTCAM_Table->Valid = ((DWBIT16 & reg_data) >> 16);

    return RET_OK;
}

Ret_Code_et __TCAM_OCP_Read(INT32U entry_number, TCAM_ReadType_et type, TCAM_Entry_Setting_st *pstTCAM_Table)
{
    INT32U reg_data = 0;

    if(entry_number >TCAM_Entry_Number)
        return RET_FAIL;

    /*TCAM_ACC_FLAG*/
    reg_data |= DWBIT31;
    /*RW_SEL*/
    reg_data &= ~DWBIT30;
    /*TCAM_WM : read type select*/
    if(type == TCAM_data)
        reg_data |= DWBIT13;
    else
        reg_data &= ~DWBIT13;
    reg_data |= DWBIT12;
    /*TCAM_ADDR*/
    reg_data |= (entry_number&0x000001FF);

    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=reg_data;
    while((DWBIT31&REG32(OOBMAC_IOBASE+MAC_TCAM_PORT))== DWBIT31) {};

    reg_data = 0;
    reg_data = REG32(OOBMAC_IOBASE+MAC_TCAM_DOUT);

    if(type == TCAM_data)
        pstTCAM_Table->Value = 0x0000FFFF & reg_data;
    else
        pstTCAM_Table->DontCareBit = 0x0000FFFF & (reg_data>>16);

    //pstTCAM_Table->Valid = ((DWBIT16 & reg_data) >> 16);
    pstTCAM_Table->Valid = REG32(OOBMAC_IOBASE+MAC_TCAM_VOUT)&0x01;

    return RET_OK;
}

Ret_Code_et TCAM_WriteRule(Rule_NO_et number, RuleFormat_et bit, ONOFF_Switch_et OnOff)
{
    INT8U *rule = 0;

    if(number > RULE_NO_MAX || bit >MAX_RULE_NUMBER)
        return RET_FAIL;

    rule = (INT8U *)(OOBMAC_IOBASE + MAC_PKT_RULE0);
    rule = rule + (sizeof(INT32U)*4*number);
    rule = rule + (sizeof(INT32U)*(bit>>5));

    if(OnOff == Switch_ON)
    {
        REG32(rule) |= (0x01<<(bit%32));
    }
    else
    {
        REG32(rule) &= (0x01<<(bit%32));
    }

    return RET_OK;
}

Ret_Code_et TCAM_AccessEntry(TCAM_Entry_Type_et Type, INT16U Number, INT16U Set, TCAM_RW_et RW, TCAM_Entry_Setting_st *value)
{
    INT32U entry_number = 0;
    Ret_Code_et ret = RET_OK;

    if(Number >= TCAM_Property.Entry_Per_Set[Type])
        return RET_FAIL;
    if(Set >= TCAM_Property.Number_Of_Set[Type])
        return RET_FAIL;
    if(RW >= TCAM_RW_MAX)
        return RET_FAIL;
    if(value == NULL)
        return RET_FAIL;

    entry_number = TCAM_Property.Start_In_TCAM[Type] + (Set*TCAM_Property.Entry_Per_Set[Type])+Number;

    if(RW == TCAM_Read)
    {
        ret = TCAM_OCP_Read(entry_number, TCAM_data, value);
        ret = TCAM_OCP_Read(entry_number, TCAM_care, value);
    }
    else
        ret = TCAM_OCP_Write(value->Value, value->DontCareBit, value->Valid, entry_number, Switch_ON, Switch_ON, Switch_ON);

    return ret;
}

Ret_Code_et TCAM_SetTypeField(TCAM_Entry_Type_et Type, INT16U Set,
                              TCAM_RW_et RW, TCAM_Entry_Setting_st *value)
{
    TCAM_Entry_Setting_st *p_value = NULL;
    Ret_Code_et ret = RET_OK;
    INT8U i = 0;

    p_value = value;
    if(p_value == NULL)
    {
        return RET_FAIL;
    }

    switch(Type)
    {
        case TCAM_MAC:
            for(i=0; i<3; i++)
            {
                ret = TCAM_AccessEntry(Type,i,Set,RW,p_value+i);
                if(ret == RET_FAIL)
                    return ret;
            }
            break;

        case TCAM_SIPv4:
        case TCAM_DIPv4:
            for(i=0; i<2; i++)
            {
                ret = TCAM_AccessEntry(Type,i,Set,RW,p_value+i);
                if(ret == RET_FAIL)
                    return ret;
            }
            break;

        case TCAM_SIPv6:
        case TCAM_DIPv6:
            for(i=0; i<8; i++)
            {
                ret = TCAM_AccessEntry(Type,i,Set,RW,p_value+i);
                if(ret == RET_FAIL)
                    return ret;
            }
            break;

        default:
            ret = TCAM_AccessEntry(Type,0,Set,RW,p_value+i);
            if(ret == RET_FAIL)
                return ret;
            break;
    }

    return ret;
}

RuleFormat_et TCAM_GetRuleBit (TCAM_Entry_Type_et Type, INT16U Set)
{
    switch(Type)
    {
        case TCAM_MAC:
            return MAC_0+Set;
        case TCAM_MARIB:
            return MARI;
        case TCAM_MAROOB:
            return MARO;
        case TCAM_VLAN:
            if(Set<=3)
                return VLAN_0+Set;
            else if(Set<=5)
                return VLAN_4to5;
            else if(Set<=10)
                return VLAN_6to10;
            else if(Set<=15)
                return VLAN_11to15;
            else
                return OUT_OF_RANGE;
        case TCAM_TYPE:
            if(Set<=7)
                return TYPE_0+Set;
            else if(Set<=11)
                return TYPE_8to11;
            else if(Set<=15)
                return TYPE_12to15;
            else
                return OUT_OF_RANGE;
        case TCAM_PTLv4:
        case TCAM_PTLv6:
            return PTL_0+Set;
        case TCAM_SIPv4:
        case TCAM_SIPv6:
            return SIP_0+Set;
        case TCAM_DIPv4:
        case TCAM_DIPv6:
            return DIP_0+Set;
        case TCAM_SPORT:
            if(Set<=4)
                return SPORT_0to4;
            else if(Set<=9)
                return SPORT_5to9;
            else if(Set<=14)
                return SPORT_10to14;
            else if(Set<=19)
                return SPORT_15to19;
            else if(Set<=24)
                return SPORT_20to24;
            else if(Set<=29)
                return SPORT_25to29;
            else if(Set<=39)
                return SPORT_30to39;
            else if(Set<=49)
                return SPORT_40to49;
            else if(Set<=59)
                return SPORT_50to59;
            else if(Set<=69)
                return SPORT_60to69;
            else
                return OUT_OF_RANGE;
        case TCAM_DPORT:
            if(Set<=9)
                return DPORT_0to9;
            else if(Set<=19)
                return DPORT_10to19;
            else if(Set<=29)
                return DPORT_20to29;
            else if(Set<=39)
                return DPORT_30to39;
            else if(Set<=49)
                return DPORT_40to49;
            else if(Set<=59)
                return DPORT_50to59;
            else if(Set<=69)
                return DPORT_60to69;
            else if(Set<=79)
                return DPORT_70to79;
            else if(Set<=89)
                return DPORT_80to89;
            else if(Set<=99)
                return DPORT_90to99;
            else if(Set<=109)
                return DPORT_100to109;
            else if(Set<=127)
                return DPORT_110to127;
            else
                return OUT_OF_RANGE;
        case TCAM_Teredo_SPORT:
        case TCAM_Teredo_DPORT:
        case TCAM_UDP_ESP_SPORT:
        case TCAM_UDP_ESP_DPORT:
            return 100;
        case TCAM_OFFSET:
            if(Set<=3)
                return OFFSET_0to3;
            else if(Set<=7)
                return OFFSET_4to7;
            else if(Set<=11)
                return OFFSET_8to11;
            else if(Set<=15)
                return OFFSET_12to15;
            else if(Set<=19)
                return OFFSET_16to19;
            else if(Set<=23)
                return OFFSET_20to23;
            else if(Set<=27)
                return OFFSET_24to27;
            else if(Set<=31)
                return OFFSET_28to31;
            else if(Set<=35)
                return OFFSET_32to35;
            else if(Set<=39)
                return OFFSET_36to39;
            else if(Set<=43)
                return OFFSET_40to43;
            else if(Set<=47)
                return OFFSET_44to47;
            else if(Set<=51)
                return OFFSET_48to51;
            else if(Set<=55)
                return OFFSET_52to55;
            else if(Set<=59)
                return OFFSET_56to59;
            else if(Set<=63)
                return OFFSET_60to63;
            else
                return OUT_OF_RANGE;
        default:
            return OUT_OF_RANGE;
    }
}
void PacketFillDefault(void)
{
    INT8U MAC_Address[6] = {0x00, 0xE0, 0x4C, 0x05, 0x00, 0x01};

    PacketFilterSettingMAC(0, MAC_Address);

    PacketFilterSettingType(0, 0x0800);
    PacketFilterSettingType(1, 0x86DD);
    PacketFilterSettingType(2, 0x0806);
    PacketFilterSettingType(3, 0x8100);

    PacketFilterSettingPTLv4(0, 0x06);
    PacketFilterSettingPTLv4(1, 0x11);
    PacketFilterSettingPTLv4(2, 0x01);
    PacketFilterSettingPTLv4(3, 0x32);
    PacketFilterSettingPTLv4(4, 0x33);
    PacketFilterSettingPTLv4(5, 0x29);

    PacketFilterSettingPTLv6(0, 0x06);
    PacketFilterSettingPTLv6(1, 0x11);
    PacketFilterSettingPTLv6(2, 0x3A);
    PacketFilterSettingPTLv6(3, 0x32);
    PacketFilterSettingPTLv6(4, 0x33);
    PacketFilterSettingPTLv6(5, 0x00);
    PacketFilterSettingPTLv6(6, 0x2B);
    PacketFilterSettingPTLv6(7, 0x3C);
    PacketFilterSettingPTLv6(8, 0x2C);

}
void PacketFilterSettingMAC(INT8U MAC_Set, INT8U *MAC_Address)
{
    TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};
    INT8U i = 0;

    for(i=0; i<TCAM_Property.Number_Of_Set[TCAM_MAC]; i++)
    {
        st_TCAM_Entry_Setting.Value = MAC_Address[i*2]<<8|MAC_Address[i*2+1];
        st_TCAM_Entry_Setting.DontCareBit = 0;
        st_TCAM_Entry_Setting.Valid = 1;
        TCAM_AccessEntry(TCAM_MAC, i, MAC_Set, TCAM_Write, &st_TCAM_Entry_Setting);
    }
}


void PacketFilterSettingIPv4(INT8U IPv4_Set, INT8U *DIPv4)
{
    TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};
    INT8U i = 0;

    for(i=0; i<TCAM_Property.Number_Of_Set[TCAM_DIPv4]; i++)
    {
        st_TCAM_Entry_Setting.Value = DIPv4[i*2]<<8|DIPv4[i*2+1];
        st_TCAM_Entry_Setting.DontCareBit = 0;
        st_TCAM_Entry_Setting.Valid = 1;
        TCAM_AccessEntry(TCAM_DIPv4, i, IPv4_Set, TCAM_Write, &st_TCAM_Entry_Setting);
    }
}

void PacketFilterSettingIPv6(INT8U IPv6_Set, INT8U *DIPv6)
{
    TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};
    INT8U i = 0;

    for(i=0; i<TCAM_Property.Number_Of_Set[TCAM_DIPv6]; i++)
    {
        st_TCAM_Entry_Setting.Value = DIPv6[i*2]<<8|DIPv6[i*2+1];
        st_TCAM_Entry_Setting.DontCareBit = 0;
        st_TCAM_Entry_Setting.Valid = 1;
        TCAM_AccessEntry(TCAM_DIPv6, i, IPv6_Set, TCAM_Write, &st_TCAM_Entry_Setting);
    }
}

void PacketFilterSettingPort(INT8U Port_Set, INT16U PortValue)
{
    TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};

    st_TCAM_Entry_Setting.Value = PortValue;
    st_TCAM_Entry_Setting.DontCareBit = 0;
    st_TCAM_Entry_Setting.Valid = 1;
    TCAM_AccessEntry(TCAM_DPORT, 0, Port_Set, TCAM_Write, &st_TCAM_Entry_Setting);
}

void PacketFilterSettingType(INT8U Type_Set, INT16U TypeValue)
{
    TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};

    st_TCAM_Entry_Setting.Value = TypeValue;
    st_TCAM_Entry_Setting.DontCareBit = 0;
    st_TCAM_Entry_Setting.Valid = 1;
    TCAM_AccessEntry(TCAM_TYPE, 0, Type_Set, TCAM_Write, &st_TCAM_Entry_Setting);
}


void PacketFilterSettingPTLv4(INT8U PTL_Set, INT8U PTLv4)
{
    TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};

    st_TCAM_Entry_Setting.Value = PTLv4;
    st_TCAM_Entry_Setting.DontCareBit = 0xFF00;
    st_TCAM_Entry_Setting.Valid = 1;
    TCAM_AccessEntry(TCAM_PTLv4, 0, PTL_Set, TCAM_Write, &st_TCAM_Entry_Setting);
}

void PacketFilterSettingPTLv6(INT8U PTL_Set, INT8U PTLv6)
{
    TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};

    st_TCAM_Entry_Setting.Value = PTLv6<<8;
    st_TCAM_Entry_Setting.DontCareBit = 0x00FF;
    st_TCAM_Entry_Setting.Valid = 1;
    TCAM_AccessEntry(TCAM_PTLv6, 0, PTL_Set, TCAM_Write, &st_TCAM_Entry_Setting);
}
Ret_Code_et PacketFilterRuleEn(Rule_NO_et number, ONOFF_Switch_et OnOff)
{
    INT32U data = 0;

    if(number > RULE_NO_MAX)
    {
        return RET_FAIL;
    }

    if(OnOff == Switch_ON)
    {
        data = REG32(OOBMAC_IOBASE+MAC_PKT_RULE_EN);
        data |= (INT32U)0x00000001 << number;
        data |= DWBIT31;
        REG32(OOBMAC_IOBASE+MAC_PKT_RULE_EN)=data;
        while((DWBIT31&REG32(OOBMAC_IOBASE+MAC_PKT_RULE_EN))== DWBIT31) {};
    }
    else
    {
        data = REG32(OOBMAC_IOBASE+MAC_PKT_RULE_EN);
        data &= ~((INT32U)0x00000001 << number);
        data |= DWBIT31;
        REG32(OOBMAC_IOBASE+MAC_PKT_RULE_EN)=data;
        while((DWBIT31&REG32(OOBMAC_IOBASE+MAC_PKT_RULE_EN))== DWBIT31) {};
    }
    return RET_OK;
}

#endif

//modify by melody
void bsp_oobmac_init_packet(void)
{
    TXdesc *txdesc;
    RXdesc *rxdesc;
    INT8U *tmp;
    INT8U *temp;
    INT16U i;
    INT32U sum;
    INT8U pktBuf_ipAddr[8]= {192,168,10,18,192,168,10,2};

    //1.allocate resource
    TxdescStartAddr_OOBMAC= malloc(TxdescNumber_OOBMAC*sizeof(struct _TxDesc));
    if(TxdescStartAddr_OOBMAC==NULL)
    {
        bsp_4everloop(0);
    }
    RxdescStartAddr_OOBMAC= malloc(RxdescNumber_OOBMAC*sizeof(struct _RxDesc));
    if(RxdescStartAddr_OOBMAC==NULL)
    {
        bsp_4everloop(0);
    }
    TallyCounterAddr = malloc(sizeof(struct _TallyCnt));
    if(TallyCounterAddr==NULL)
    {
        bsp_4everloop(0);
    }

    tmp = (INT8U*) TxdescStartAddr_OOBMAC;
    memset(tmp, 0, sizeof(TXdesc)*TxdescNumber_OOBMAC);
    tmp = (INT8U*) RxdescStartAddr_OOBMAC;
    memset(tmp, 0, sizeof(RXdesc)*RxdescNumber_OOBMAC);
    tmp = (INT8U*) TallyCounterAddr;
    memset(tmp, 0, sizeof(TallyCnt));
    gmac_dump_tally_counter();

    /* Tx descriptor setup */
    for(i=0; i<TxdescNumber_OOBMAC; i++)
    {
        txdesc = ((TXdesc *)TxdescStartAddr_OOBMAC)+i;
        txdesc->FS = 1;
        txdesc->LS = 1;
        temp = malloc(TXBUFFERSTEP);
        if(temp==NULL)
        {
            bsp_4everloop(0);
        }
        txdesc->BufferAddress = VA2PA((INT32U)temp);
        if(i ==(TxdescNumber_OOBMAC-1))
        {
            txdesc->EOR = 1;
        }
    }

    /* Rx descriptor setup */
    for(i=0; i<RxdescNumber_OOBMAC; i++)
    {
        rxdesc = ((RXdesc *)RxdescStartAddr_OOBMAC)+i;
        rxdesc->Length = 0x600;
        temp = malloc(RXBUFFERSTEP);
        if(temp==NULL)
        {
            bsp_4everloop(0);
        }
#ifdef MAC_RX_ANY_BYTE
        rxdesc->BufferAddress = VA2PA((INT32U)temp+rand()%4);
#else
        rxdesc->BufferAddress = VA2PA((INT32U)temp);
#endif
        if(i == (RxdescNumber_OOBMAC- 1))
        {
            rxdesc->EOR = 1;
        }
        rxdesc->OWN = 1;
    }

    //2.calculate ipv4 pseudo header checksum
    for(i=0; i<8; i+=2)
    {
        ipv4UDPsum += ((pktBuf_ipAddr[i]<<8)&0xFF00)+(pktBuf_ipAddr[i+1]&0xFF);
        while (ipv4UDPsum>>16)
        {
            ipv4UDPsum = (ipv4UDPsum & 0xFFFF)+(ipv4UDPsum >> 16);
        }
    }
    ipv4TCPsum = ipv4UDPsum;
    ipv4UDPsum+= 0x11;
    ipv4TCPsum+= 0x6;
    while (ipv4TCPsum>>16)
    {
        ipv4TCPsum = (ipv4TCPsum & 0xFFFF)+(ipv4TCPsum >> 16);
    }
    while (ipv4UDPsum>>16)
    {
        ipv4UDPsum = (ipv4UDPsum & 0xFFFF)+(ipv4UDPsum >> 16);
    }

}

#if 0
void bsp_oobmac_init_packet(void)
{
    INT8U *pktBuf, etherProto[]= {0x08, 0x00};
    int i;
    /*We do following thins:
      * 1. Fill packet within increasing number. From 0x0 to 0xff
      * 2. Set MAC header without specifing any protocol
      * 3. Pseudo Sum for IPv4 UDP, IPv4 TCP, IPv6 UDP, IPv6 TCP
    */

    pktBuf = (INT8U*)(TXBUFFER);
    for(i=0; i<TxdescNumber_OOBMAC*TXBUFFERSTEP; i++)
    {
        pktBuf[i] = (INT8U)(i%256);
    }
    for(i=0; i<2; i++)
    {
        pktBuf = (INT8U*)(TXBUFFER+i*TXBUFFERSTEP);
        setMACHeader(pktBuf, etherProto);
        setIPv4Header(&pktBuf[14], 5, 0x11);
    }

    /*Set pktBuf from IP header of the first packet buffer*/
    pktBuf = (INT8U*)(TXBUFFER+14);
    for(i=12; i<20; i+=2)
    {
        ipv4UDPsum += ((pktBuf[i]<<8)&0xFF00)+(pktBuf[i+1]&0xFF);
        while (ipv4UDPsum>>16)
        {
            ipv4UDPsum = (ipv4UDPsum & 0xFFFF)+(ipv4UDPsum >> 16);
        }
    }
    ipv4TCPsum = ipv4UDPsum;
    ipv4UDPsum+= 0x11;
    ipv4TCPsum+= 0x6;
    while (ipv4TCPsum>>16)
    {
        ipv4TCPsum = (ipv4TCPsum & 0xFFFF)+(ipv4TCPsum >> 16);
    }
    while (ipv4UDPsum>>16)
    {
        ipv4UDPsum = (ipv4UDPsum & 0xFFFF)+(ipv4UDPsum >> 16);
    }
    etherProto[0] = 0x86;
    etherProto[1] = 0xdd;
    /*For IPv6 Pseduo sum, we make a fake packet first.*/
    pktBuf = (INT8U*)(TXBUFFER+TXBUFFERSTEP);
    setMACHeader(pktBuf, etherProto);
    setIPv6Header(&pktBuf[14], IPV6TCP);
    /*Add from 12 to 53*/
    for(i=8; i<40; i+=2)
    {
        ipv6TCPsum += ((pktBuf[14+i]<<8)&0xFF00)+(pktBuf[14+i+1]&0xFF);
        while (ipv6TCPsum>>16)
        {
            ipv6TCPsum = (ipv6TCPsum & 0xFFFF)+(ipv6TCPsum >> 16);
        }
    }
    ipv6UDPsum = ipv6TCPsum;
    ipv6UDPsum+= 0x11;
    ipv6TCPsum+= 0x6;
    ipv6TCPsum+=(TXLength-14-40-8-24-16);
    while (ipv6TCPsum>>16)
    {
        ipv6TCPsum = (ipv6TCPsum & 0xFFFF)+(ipv6TCPsum >> 16);
    }
    ipv6UDPsum+=(TXLength-14-40-8-24-16);
    while (ipv6UDPsum>>16)
    {
        ipv6UDPsum = (ipv6UDPsum & 0xFFFF)+(ipv6UDPsum >> 16);
    }

}
#endif

void __inline__ bsp_oobmac_send(INT32U pktType,const INT32U DDR_start_addr,
                                const INT8U pktnum,INT32U blocknum,INT32U Block_V_offset,INT8U BPP)
{
    int i, sendCnt, nextProtocol,k;
    volatile INT8U *pktbuf;
    TXdesc *txdesc[4];
    INT8U Block_line;
    INT32U pktlen=0,pkttotalnum=0,dataoffset=0;


    Block_line=0;
//=======================================================================

    txd=(TXdesc *)(TxdescStartAddr_OOBMAC)+hwtxptr;
    if(txd->OWN==1)
    {
        return;
    }
    txd->V6F = 0;
    txd->TAGC = 0;
    txd->IPV4CS =0;
    txd->TCPCS = 0;
    txd->UDPCS = 0;
    txd->TCPHO = 0;

    txd->FS=1;
    txd->LS=1;
    pktbuf = (INT8U*)PA2VA(txd->BufferAddress);

    /*Dest MAC*/
    memset(pktbuf, 0xff, 6);
    /*Source MAC*/
    pktbuf[6] = 0x0;
    pktbuf[7] = 0xe0;
    pktbuf[8] = 0x4c;
    pktbuf[9] = 0x05;
    pktbuf[10] = 0x00;
    pktbuf[11] = 0x01;//oob packet
    pktbuf[12] = 0x08;
    pktbuf[13] = 0x00;
    /*Ether type*/
    if(pktType==1) // first pkt
    {
        pktbuf[14] = 0x11;
        pktbuf[15] = 0x11;
        for(k=16; k<60; k++)
        {
            pktbuf[k] = 0x00;
        }
        txd->Length=60;
    }
    else if(pktType==2)//payload pkt
    {
        pktbuf[14] = 0x22;
        pktbuf[15] = 0x22;
        pktbuf[16] = blocknum&0xFF;
        pktbuf[17] = (blocknum>>8)&0xFF;

        if((4096*BPP-pktnum*0x578)<0x578)
            txd->Length=4096*BPP-pktnum*0x578;
        else
            txd->Length=0x578;

        for(k=0; k<(txd->Length); k++)
        {
            dataoffset=k+((pktnum*0x578)/(64*BPP)+Block_line)*Block_V_offset+(pktnum
                       *0x578)%(64*BPP);
            pktbuf[18+k] = REG8(DDR_start_addr+dataoffset);
            if(k%(64*BPP)==((64*BPP)-1))
            {
                Block_line++;
            }
        }

    }
    else if(pktType==3)//last packet
    {
        pktbuf[14] = 0x33;
        pktbuf[15] = 0x33;
        for(k=0; k<60; k++)
        {
            pktbuf[16+k] = 0x00;
        }
        txd->Length=60;
    }
    //fillDummyData(pktbuf, 0, UNCHANGEHDROFFSET);



    txd->OWN=1;
    REG8(OOBMAC_IOBASE+MAC_TPPoll)=0x80;
    TxTotal++;

    /*if( length++ > TXLength) {
    	length = TXLengthMin;
    }	*/
    hwtxptr=(hwtxptr+1)%TxdescNumber_OOBMAC;
    if(TxMixed!=0)
    {
        return;
    }

    return;

}


void __inline__ bsp_oobmac_test(const int pktType, const INT8U isChecksum)
{
    int i, sendCnt, nextProtocol;
    INT8U iplen = 5, tcplen = 5, etherProtocol[] = {0x08, 0x00};
    volatile INT8U *pktbuf;
    TXdesc *txdesc[4];

#ifdef OOBMAC_DMA_DEBUG
    if(pktType == DEBUGPKT)
    {
        for(i=0; i<TxdescNumber_OOBMAC; i++)
        {
            txd=(TXdesc *)(TxdescStartAddr_OOBMAC)+hwtxptr;
            if(txd->OWN==1)
            {
                break;
            }
            txd->V6F = 0;
            txd->TAGC = 0;
            txd->IPV4CS = isChecksum;
            txd->TCPCS = 0;
            txd->UDPCS = 0;
            txd->TCPHO = 0;
            txd->Length = 60;
            txd->FS=1;
            txd->LS=1;
            pktbuf = (INT8U*)PA2VA(txd->BufferAddress);
            fillDummyData(pktbuf, 0, UNCHANGEHDROFFSET);

            etherProtocol[0] = 0xff;
            etherProtocol[1] = 0xff;
            setMACHeader(pktbuf, etherProtocol);

            txd->OWN=1;
            REG8(OOBMAC_IOBASE+MAC_TPPoll)=0x80;
            TxTotal++;

            hwtxptr=(hwtxptr+1)%TxdescNumber_OOBMAC;
            if(TxMixed!=0)
            {
                break;
            }
            return;
        }
        return;
    }
#endif

    if(pktType != IPV4TCP && pktType != IPV4UDP)
    {
        for(i=0; i<TxdescNumber_OOBMAC; i++)
        {
            txd=(TXdesc *)(TxdescStartAddr_OOBMAC)+hwtxptr;
            if(txd->OWN==1)
            {
                break;
            }
            txd->V6F = 0;
#ifdef MAC_TEST_VLAN_TX
            txd->TAGC = 1;
            txd->VLanTag = VLANDESC;
#else
            txd->TAGC = 0;
#endif
            txd->IPV4CS = isChecksum;
            txd->TCPCS = 0;
            txd->UDPCS = 0;
            if(IPV6TCP == pktType || IPV6UDP == pktType)
            {
                txd->V6F = 1;
                //txd->TCPHO = 102;   //with extension IPv6 header
                txd->TCPHO = 54;   //without extension IPv6 header by melody
                txd->Length = TXLength;
                txd->IPV4CS = 0;
                if(IPV6TCP == pktType)
                {
                    txd->TCPCS = isChecksum;  //HW offload checksum
                }
                else
                {
                    txd->UDPCS = isChecksum;
                }
            }
            else
            {
                txd->TCPHO = 0;
                txd->Length = length;
            }
            txd->FS=1;
            txd->LS=1;
            pktbuf = (INT8U*)PA2VA(txd->BufferAddress);
            //	fillDummyData(pktbuf, 0, UNCHANGEHDROFFSET);
            //fillDummyData(pktbuf, 0, txd->Length);
            fillDummyData(&pktbuf[12], 1, txd->Length);

            if(NORMALPKT == pktType)
            {
                etherProtocol[0] = 0xff;
                etherProtocol[1] = 0xff;
                setMACHeader(pktbuf, etherProtocol);
            }
            else if(IPV4 == pktType)
            {
                etherProtocol[0] = 0x08;
                etherProtocol[1] = 0x00;
                setMACHeader(pktbuf, etherProtocol);
                setIPv4Header(&pktbuf[14], 5, 0xff);  //IPv4 headr length 20;
            }
            else if((IPV6TCP == pktType) || (IPV6UDP == pktType))
            {
                etherProtocol[0] = 0x86;
                etherProtocol[1] = 0xdd;
                setMACHeader(pktbuf, etherProtocol);
                setIPv6Header(&pktbuf[14], pktType);
                pseudoheader(pktbuf, pktType, txd->TCPHO-14);  //fill pseudo header checksum in TCP/UDP checksum field
            }

            txd->OWN=1;
            REG8(OOBMAC_IOBASE+MAC_TPPoll)=0x80;
            TxTotal++;

            if(IPV4 == pktType)
            {
                TxTotal_IP4++;
            }
            else if(IPV6UDP == pktType)
            {
                TxTotal_UDP6++;
            }
            else if(IPV6TCP == pktType)
            {
                TxTotal_TCP6++;
            }
            else if(NORMALPKT == pktType)
            {
                TxTotal_NRM++;
            }

            if( length++ > TXLength)
            {
                length = TXLengthMin;
            }
            hwtxptr=(hwtxptr+1)%TxdescNumber_OOBMAC;
            if(TxMixed!=0 || SingleTxFlag)
            {
                break;
            }
            if(FT_EQC_test_OOBMAC && (TxTotal == FT_OOBMAC_testNum))
            {
                FT_OOBMAC_txFinish=1;
                return;
            }
            if(SingleTxFlag)
            {
                return;
            }
        }
#if defined(MAC_reg_InterruptMitigation) || defined(MAC_TX_SENDSLOW)  //add cause:interrupt cannot be  genareted immediately
        bsp_wait(gmac_tx_delayTime);   //10us
#endif
        return;
    }

    //multiDesc a packet
    sendCnt = 0;
    while(sendCnt < TxdescNumber_OOBMAC)
    {
        //1. make sure at least 4 tx desc available
        for(i=0; i<4; i++)
        {
            txdesc[i]=((TXdesc *)(TxdescStartAddr_OOBMAC))+((hwtxptr + i)%TxdescNumber_OOBMAC);
            if(txdesc[i]->OWN)
            {
                return;
            }
        }
        if(length > 134)
        {
            iplen = (INT8U)(rand()%0xb + 5);
            if(pktType == IPV4TCP)
            {
                tcplen = (INT8U)(rand()%0xb + 5);
            }
        }
        if(pktType == IPV4TCP)
        {
            nextProtocol = 0x6;   //TCP
        }
        else
        {
            nextProtocol = 0x11;  //UDP
        }
        for(i=0; i<4; i++)
        {
            txdesc[i]->V6F = 0;
#ifdef MAC_TEST_VLAN_TX
            txdesc[i]->TAGC = 1;
            txdesc[i]->VLanTag = VLANDESC;
#else
            txdesc[i]->TAGC = 0;
#endif
            txdesc[i]->TCPHO = 0;
            txdesc[i]->IPV4CS = isChecksum;
            if(pktType == IPV4TCP)
            {
                txdesc[i]->TCPCS = isChecksum;
                txdesc[i]->UDPCS = 0;
            }
            else
            {
                txdesc[i]->UDPCS = isChecksum;
                txdesc[i]->TCPCS = 0;
            }
        }

        //desc0:MAC header
        txdesc[0]->FS = 1;
        txdesc[0]->LS = 0;
        txdesc[0]->Length = 14;
        setMACHeader((INT8U*)PA2VA(txdesc[0]->BufferAddress), etherProtocol);
        //desc1:IPv4 header
        txdesc[1]->FS=0;
        txdesc[1]->LS=0;
        txdesc[1]->Length=iplen*4;  //radom length  may be with extension ip header=>psuedo header checksum must
        setIPv4Header((INT8U*)PA2VA(txdesc[1]->BufferAddress), iplen, nextProtocol);
        //desc2:TCP/UDP header
        txdesc[2]->FS=0;
        txdesc[2]->LS=0;
        pktbuf = (INT8U*)PA2VA(txdesc[2]->BufferAddress);  //fill TCP/UDP header
        //desc3:data offload
        txdesc[3]->FS=0;
        txdesc[3]->LS=1;
        //	fillDummyData((INT8U*)PA2VA(txdesc[3]->BufferAddress), 0, UNCHANGEHDROFFSET);

        if(pktType == IPV4TCP)
        {
            pseudoheader_multiDesc(&((INT8U*)PA2VA(txdesc[2]->BufferAddress))[16], pktType, iplen*4); //HW has offload it
            pktbuf[12] = (tcplen<<4)|0x0;
            txdesc[2]->Length=tcplen*4;  //radom length  may be with  extension tcp header
            txdesc[3]->Length=(length - (14 + (iplen+tcplen)*4));
        }
        else if(pktType == IPV4UDP)
        {
            pseudoheader_multiDesc(&((INT8U*)PA2VA(txdesc[2]->BufferAddress))[6], pktType, iplen*4);
            txdesc[2]->Length=8;  //fixed udp header length
            memset(pktbuf, 0x88, 4);
            pktbuf[4] = (INT8U)((0xff00 & (length - iplen*4 - 14))>>8);
            pktbuf[5] = (INT8U)((0x00ff & (length - iplen*4 - 14)));
            txdesc[3]->Length=(length - (14 + iplen*4 + 8));
        }

        fillDummyData((INT8U*)PA2VA(txdesc[3]->BufferAddress), 0, txdesc[3]->Length);

        for(i=3; i>=0; i--)
        {
            txdesc[i]->OWN = 1;
        }
        REG8(OOBMAC_IOBASE+MAC_TPPoll)=0x80;
        TxTotal++;

        sendCnt++;
        if(pktType == IPV4TCP)
        {
            TxTotal_TCP4++;
        }
        else if(pktType == IPV4UDP)
        {
            TxTotal_UDP4++;
        }

        if( length++ > TXLength)
        {
            length = TXLengthMin;
        }
        hwtxptr=(hwtxptr+4)%TxdescNumber_OOBMAC;
        if(TxMixed!=0)
        {
            break;
        }
        if(FT_EQC_test_OOBMAC && (TxTotal == FT_OOBMAC_testNum))
        {
            FT_OOBMAC_txFinish=1;
            return;
        }
        if(SingleTxFlag)
        {
            return;
        }
    }
}

//added by melody 20161021
void __inline__ pseudoheader(INT8U *protocolhdr,const int pktType,const INT8U iplen)
{
    INT32U sum;
    INT16U Temp=0;
    int i;
    INT32U psedSum=0;

    if(!protocolhdr)
    {
        return;
    }

    //1.caculate Pseudo Header of Source and Destination address
    if(pktType == IPV4TCP || pktType == IPV4UDP)
    {
        for(i=12; i<20; i=i+2)
        {
            Temp=protocolhdr[14+i];
            Temp=Temp<<8;
            Temp+=protocolhdr[14+i+1];
            psedSum+=(INT32U)Temp;
            while(psedSum>>16)
            {
                psedSum=(psedSum&0x0FFFF)+(psedSum>>16);
            }
        }
    }
    else if(pktType == IPV6TCP || pktType == IPV6UDP)
    {
        for(i=8; i<40; i=i+2)
        {
            Temp=protocolhdr[14+i];
            Temp=Temp<<8;
            Temp+=protocolhdr[14+i+1];
            psedSum+=(INT32U)Temp;
            while(psedSum>>16)
            {
                psedSum=(psedSum&0x0FFFF)+(psedSum>>16);
            }
        }
    }
    //2. caculate Pseudo Header of upper layer protocol
    if(pktType == IPV6UDP || pktType == IPV4UDP) 		   //Next Header
    {
        Temp=0x11; //Next Header is UDP protocol
    }
    else if(pktType == IPV6TCP || pktType == IPV4TCP)
    {
        Temp=0x06; //Next Header is TCP protocol
    }
    psedSum+=(INT32U)Temp;
    while(psedSum>>16)
    {
        psedSum=(psedSum&0x0FFFF)+(psedSum>>16);
    }
    //3. caculate Pseudo Header of upper layer(TCP/UDP) length
    Temp=txd->Length-14-iplen;
    psedSum+=(INT32U)Temp;
    while(psedSum>>16)
    {
        psedSum=(psedSum&0x0FFFF)+(psedSum>>16);
    }
    //put pseudo header checksum value in the checksum field.
    if(pktType == IPV6TCP || pktType == IPV4TCP)
    {
        protocolhdr[14+iplen+16]=((psedSum)&0xFF00)>>8;
        protocolhdr[14+iplen+17]=(psedSum) & 0x0FF;
    }
    else if(pktType == IPV6UDP || pktType == IPV4UDP)
    {
        protocolhdr[14+iplen+6]=((psedSum)&0xFF00)>>8;
        protocolhdr[14+iplen+7]=(psedSum) & 0x0FF;
    }
}

void __inline__ pseudoheader_multiDesc(INT8U *protocolhdr,const int pktType,const INT8U iplen)
{
    INT32U sum;

    if(!protocolhdr)
    {
        return;
    }

    //3. cs of length
    if(pktType == IPV4TCP)
    {
        sum = ipv4TCPsum;
    }
    else if(pktType  == IPV4UDP)
    {
        sum = ipv4UDPsum;
    }
    else
    {
        return;
    }
    sum+=(length - 14 - iplen);
    while(sum>>16)
    {
        sum=(sum&0x0FFFF)+(sum>>16);
    }
    //4.fill pseudo header checksum in TCP/UDP checksum field
    protocolhdr[0]=(INT8U)((sum&0xFF00)>>8);
    protocolhdr[1]=(INT8U)(sum & 0x0FF);
}

void __inline__ fillDummyData(INT8U *pktBuf, const INT8U startValue, const int len)
{
    int i;
    if(!pktBuf)
    {
        return;
    }
#ifdef OOBMAC_DMA_DEBUG
    for(i=0; i<len; i++)
    {
        pktBuf[i] = 0x11;  //fixed data for debug
    }
#else
    pktBuf[0] = startValue;
    for(i=1; i<len; i++)
    {
        pktBuf[i] = (INT8U)(pktBuf[i-1] + 1);  //serial increase
    }
#endif
}
void __inline__ setMACHeader(INT8U *pktBuf, INT8U *ethernetPro)
{
    if(!pktBuf || !ethernetPro)
    {
        return;
    }
#ifdef TXUNICAST
    pktBuf[0] = 0x0;
    pktBuf[1] = 0xe0;
    pktBuf[2] = 0x4c;
    pktBuf[3] = 0x28;
    pktBuf[4] = 0x80;
    pktBuf[5] = 0x00;
#elif defined(OOBMAC_DMA_DEBUG) || defined(MAC_LOOPBACK)
    pktBuf[0] = 0x67;
    pktBuf[1] = 0x00;
    pktBuf[2] = 0x00;
    pktBuf[3] = 0x00;
    pktBuf[4] = 0x00;
    pktBuf[5] = 0x00;
#else
    /*Dest MAC*/
    memset(pktBuf, 0xff, 6);
#endif
    /*Source MAC*/
#ifdef TXUNICAST
    pktBuf[6] = 0x0;
    pktBuf[7] = 0xe0;
    pktBuf[8] = 0x4c;
    pktBuf[9] = 0x05;
    pktBuf[10] = 0x00;
    pktBuf[11] = 0x01;
#else
    pktBuf[6] = 0x0;
    pktBuf[7] = 0x22;
    pktBuf[8] = 0x33;
    pktBuf[9] = 0x44;
    pktBuf[10] = 0x55;
    pktBuf[11] = 0x66;
#endif
    pktBuf[12] = ethernetPro[0];
    pktBuf[13] = ethernetPro[1];
}

void __inline__ setIPv4Header(INT8U *pktBuf, const INT8U len, const int nextProtocol)
{
    if(!pktBuf)
    {
        return;
    }
    pktBuf[0] = 0x40|len;
    pktBuf[1] = 0xFF;
    //To coordinate with DOS tool
    pktBuf[2] = 0xFF;
    pktBuf[3] = 0xFF;
    //----------
    pktBuf[6] = 0x40;
    pktBuf[7] = 0;
    /*Protocol. This should change if we send TCP or UDP packet*/
    pktBuf[9] = nextProtocol;

    /*Source IP*/
    pktBuf[12] = 192;  //0xc0
    pktBuf[13] = 168;  //0xa8
    pktBuf[14] = 10;   //0x0a
    pktBuf[15] = 18;   //0x12

    /*Dst IP*/
    pktBuf[16] = 192;
    pktBuf[17] = 168;
    pktBuf[18] = 10;
    pktBuf[19] = 2;
}
void __inline__ setIPv6Header(INT8U *pktBuf, const int pktType)
{
#if 0   //melody mask and modify it
    memset(&pktBuf[8], 0x00, 32);
    pktBuf[0]=0x6f;
    pktBuf[2]=0x10;
    pktBuf[3]=0x11;
    pktBuf[4]=0x05;
    pktBuf[5]=0xB4;
    pktBuf[6]=0x0;
    if(IPV6TCP == pktType)
    {
        pktBuf[1]=0x01;
        pktBuf[40+12]=0x50;
        pktBuf[40+13]=0x00;
        pktBuf[72]=0x06;
    }
    else
    {
        pktBuf[1]=0x04;
        pktBuf[40+4]=0x50;
        pktBuf[40+5]=0xC8;
        pktBuf[72]=0x11;
    }

    pktBuf[40]=0x3C;
    pktBuf[41]=0x00;
    pktBuf[48]=0x2B;
    pktBuf[49]=0x02;
    pktBuf[50]=0x00;
    pktBuf[73]=0x01;
    pktBuf[74]=0x00;

    if(IPV6TCP == pktType)
    {
        pktBuf[88+12]=0x50;
        pktBuf[88+13]=0x00;
        pktBuf[88+16]=((ipv6TCPsum)&0xFF00)>>8;
        pktBuf[88+17]=(ipv6TCPsum)&0x0FF;

    }
    else
    {
        pktBuf[88+4]=0x05;
        pktBuf[88+5]=0x7C;
        pktBuf[88+6]=((ipv6UDPsum)&0xFF00)>>8;
        pktBuf[88+7]=(ipv6UDPsum)&0x0FF;

    }
#endif

    pktBuf[0]=0x60;
    pktBuf[1]=0x00;
    pktBuf[2]=0x00;
    pktBuf[3]=0x00;
    pktBuf[4]=((txd->Length-14-40)&0xFF00)>>8;
    pktBuf[5]=(txd->Length-14-40) & 0x0FF;

    if(IPV6TCP == pktType)
    {
        pktBuf[6]=0x06;    // TCP type  txd->TCPHO
        pktBuf[40+12]=0x50;  //TCP header length 5*4=20
        pktBuf[40+13]=0x00;
    }
    else
    {
        pktBuf[6]=0x11;    // UDP type
        pktBuf[40+4]=((txd->Length-txd->TCPHO)&0xFF00)>>8;;  //UDP length
        pktBuf[40+5]=(txd->Length-txd->TCPHO)&0xFF;
    }

}



//test throughput,using normal packet to test
/*
extend to support oobmac large send

*/
void RKVM_OOBMAC_send(INT8U *payload_ptr,volatile INT16U length_payload)
{
    volatile INT8U *pktBuf;
    INT8U header_length=14;  //normal protocol
    INT8U send_count=1,i=0;

    if(length_payload<1497)  //64-1514 packet
    {
        //1. wait for available tx desc
        txd_rkvm2oobmac =(TXdesc_COM*)(TxdescStartAddr_OOBMAC)+hwtxptr;
        while(txd_rkvm2oobmac->OWN==1);
        //2. prepare descriptor
        txd_rkvm2oobmac->Length = length_payload+14;
        txd_rkvm2oobmac->FS=1;
        txd_rkvm2oobmac->LS=1;
        txd_rkvm2oobmac->LGSEN = 0;
        //3. prepare data
        pktBuf = (INT8U*)PA2VA(txd_rkvm2oobmac->BufferAddress);
        //header normal protocol
        memset(pktBuf, 0xff, 6);  //DA
        pktBuf[6] = 0x0;		  //SA
        pktBuf[7] = 0x22;
        pktBuf[8] = 0x33;
        pktBuf[9] = 0x44;
        pktBuf[10] = 0x55;
        pktBuf[11] = 0x66;
        pktBuf[12] = 0xff;		  //protocol
        pktBuf[13] = 0xff;
        //payload
        //	memcpy(&pktBuf[14], payload_ptr, length_payload);  //using memory copy
        //using DMAC DMA RKVM to OOBMAC DMEM
        dma_large_RKVM((INT32U)length_payload, payload_ptr, &pktBuf[14], &dma_info0);
        //	while(!dma_flag);
        //	dma_flag=0;// wait DMA done

        //4. close desc
        txd_rkvm2oobmac->OWN=1;
        //5. polling
        REG8(OOBMAC_IOBASE+MAC_TPPoll)=0x80;
        //6. maintain desc
        hwtxptr=(hwtxptr+1)%TxdescNumber_OOBMAC;
        TxTotal++;


    }
    else     //large packet
    {
        //1. using disassemble multi packet to send
#if 1
        send_count=length_payload/(1510-header_length)+1;
        for(i=1; i<=send_count; i++)
        {
            //1. wait for available tx desc
            txd_rkvm2oobmac=(TXdesc_COM*)(TxdescStartAddr_OOBMAC)+hwtxptr;
            while(txd_rkvm2oobmac->OWN==1);
            //2. prepare descriptor
            if(i==send_count)
            {
                txd_rkvm2oobmac->Length = length_payload-1496*(send_count-1)+14;  //the last send
            }
            else
            {
                txd_rkvm2oobmac->Length = 1496+14;
            }
            txd_rkvm2oobmac->FS=1;
            txd_rkvm2oobmac->LS=1;
            txd_rkvm2oobmac->LGSEN = 0;
            //3. prepare data
            pktBuf = (INT8U*)PA2VA(txd_rkvm2oobmac->BufferAddress);
            //header normal protocol
            memset(pktBuf, 0xff, 6);  //DA
            pktBuf[6] = 0x0;		  //SA
            pktBuf[7] = 0x22;
            pktBuf[8] = 0x33;
            pktBuf[9] = 0x44;
            pktBuf[10] = 0x55;
            pktBuf[11] = 0x66;
            pktBuf[12] = 0xff;		  //protocol
            pktBuf[13] = 0xff;
            //payload
            //	memcpy(&pktBuf[14], payload_ptr, length_payload);  //using memory copy
            //using DMAC DMA RKVM to OOBMAC DMEM
            if(i==send_count)
            {
                dma_large_RKVM((INT32U)(length_payload-1496*(send_count-1)), payload_ptr+(send_count-1)*1496, &pktBuf[14], &dma_info0);
            }
            else
            {

                dma_large_RKVM((INT32U)1496, payload_ptr+(i-1)*1496, &pktBuf[14], &dma_info0);
            }
            //4. close desc
            txd_rkvm2oobmac->OWN=1;
            //5. polling
            REG8(OOBMAC_IOBASE+MAC_TPPoll)=0x80;
            //6. maintain desc
            hwtxptr=(hwtxptr+1)%TxdescNumber_OOBMAC;
            TxTotal++;
        }
#endif
        //2.using oobmac large send,but it should be TCP packet
#if 0
        //1.prepare IP/TCP header


#endif

    }

}

#ifdef CONFIG_VGA_TEST_ENABLED
void bsp_oobmac_send_VGA(INT8U pktType,const INT32U start_addr,const INT8U pktnum,INT32U blocknum,INT8U BPP)
{
    volatile int i, sendCnt, nextProtocol,k;
    volatile INT8U *pktbuf;
    volatile TXdesc *txdesc[4];
    volatile INT8U Block_line;
    volatile INT32U pktlen=0,pkttotalnum=0,dataoffset=0,j,Len;
    volatile volatile INT32U FB1resolu;


    Block_line=0;
//=======================================================================

    txd=(TXdesc *)(TxdescStartAddr_OOBMAC)+hwtxptr;
    if(txd->OWN==1)
    {
        return;
    }
    txd->V6F = 0;
    txd->TAGC = 0;
    txd->IPV4CS =0;
    txd->TCPCS = 0;
    txd->UDPCS = 0;
    txd->TCPHO = 0;

    txd->FS=1;
    txd->LS=1;
    pktbuf = (INT8U*)PA2VA(txd->BufferAddress);

    /*Dest MAC*/
    OS_CPU_SR  cpu_sr = 0;

#ifdef MAC_LOOPBACK
    pktbuf[0] = 0x67;
    pktbuf[1] = 0x00;
    pktbuf[2] = 0x00;
    pktbuf[3] = 0x00;
    pktbuf[4] = 0x00;
    pktbuf[5] = 0x00;
#else
    OS_ENTER_CRITICAL();
    memset(pktbuf, 0xff, 6);
    OS_EXIT_CRITICAL();
#endif

    /*Source MAC*/
    pktbuf[6] = 0x0;
    pktbuf[7] = 0xe0;
    pktbuf[8] = 0x4c;
    pktbuf[9] = 0x05;
    pktbuf[10] = 0x00;
    pktbuf[11] = 0x01;//oob packet
    pktbuf[12] = 0x08;
    pktbuf[13] = 0x00;
    /*Ether type*/
    if(pktType==1) // first pkt
    {
        pktbuf[14] = 0x11;
        pktbuf[15] = 0x11;
        for(k=16; k<60; k++)
        {
            pktbuf[k] = 0x00;
        }
        txd->Length=60;
    }
    else if(pktType==2)//payload pkt
    {
        pktbuf[14] = 0x22;
        pktbuf[15] = 0x22;
        pktbuf[16] = blocknum&0xFF;
        pktbuf[17] = (blocknum>>8)&0xFF;

        if((0x400*BPP-pktnum*0x578)<0x578)			//64*64*BPP block divide each len=<0x578
            Len=0x400*BPP-pktnum*0x578;

        else
            Len=0x578;

        for(j=0; j<(Len); j++)
        {
            pktbuf[18+j]=REG8(start_addr+j);
        }
        txd->Length=Len+18;

    }

    else if(pktType==3)//last packet
    {
        pktbuf[14] = 0x33;
        pktbuf[15] = 0x33;
        for(k=0; k<60; k++)
        {
            pktbuf[16+k] = 0x00;
        }
        txd->Length=76;
    }

    else if(pktType==4)//difference flag IB and resolution packet
    {

        FB1resolu=REG32(VGA_IOBASE+FB1_resol);

        pktbuf[14] = 0x44;
        pktbuf[15] = 0x44;


        //resolution
        pktbuf[16]=FB1resolu& 0xFF;
        pktbuf[17]=(FB1resolu>>8) & 0xFF;
        pktbuf[18]=(FB1resolu>>16) & 0xFF;
        pktbuf[19]=(FB1resolu>>24) & 0xFF;

        for(i=0; i<64; i++)
        {
            pktbuf[20+i]=REG8(VGAdiff_flag+i)& 0xFF;				//DMEM to save diff-flag-Tx

        }

        txd->Length=84;

    }
    //fillDummyData(pktbuf, 0, UNCHANGEHDROFFSET);

    txd->OWN=1;
    REG8(OOBMAC_IOBASE+MAC_TPPoll)=0x80;
    TxTotal++;
    //REG32(VGA_IOBASE+VGA_dummy3)=diff_timer_counter<<16;

    hwtxptr=(hwtxptr+1)%TxdescNumber_OOBMAC;
    if(TxMixed!=0)
    {
        return;
    }

    return;

}
#endif

void test_flow_OOBMAC(void)  //oobmac tx
{
    //cmd from dos tool
    if(FT_EQC_test_OOBMAC)
    {
        if(FT_OOBMAC_txFinish)   //report test result to dos tool
        {
            REG8(OOBMAC_IOBASE + MAC_DMEMENDA+0x02) = 0;
            //REG32(OOBMAC_IOBASE + MAC_DUMMY_INFORM_IB_EVENT) = 0;
            FT_EQC_test_OOBMAC=0;
            FT_OOBMAC_txFinish=0;
            OSTimeDly(1);  //wait for rx finish  about 10ms
            //REG32(OOBMAC_IOBASE + MAC_DUMMY_INFORM_IB_DATA) = RxTotal;
            REG32(OOBMAC_IOBASE + MAC_DMEMENDA) |= (DWBIT19 | DWBIT17);  //oobmac event & PASS
            if(RxTotal_DataError || (RxTotal!=FT_OOBMAC_testNum))
            {
                REG32(OOBMAC_IOBASE + MAC_DMEMENDA) &= (~DWBIT17);
				REG32(OOBMAC_IOBASE + MAC_DMEMENDA) |= DWBIT16;
            }
            if(rxCheck.RxTotal_IPv4_CheckSumErr || rxCheck.RxTotal_Normal_lenthErr || rxCheck.RxTotal_TCPIPv4_CheckSumErr || \
                    rxCheck.RxTotal_TCPIPv6_CheckSumErr || rxCheck.RxTotal_UDPIPv4_CheckSumErr || rxCheck.RxTotal_UDPIPv6_CheckSumErr)
            {
				REG32(OOBMAC_IOBASE + MAC_DMEMENDA) &= (~DWBIT17);
				REG32(OOBMAC_IOBASE + MAC_DMEMENDA) |= DWBIT16;
				//set checksum err bit
            }
            //REG16(OOBMAC_IOBASE + MAC_OOB2IB_INT) |= 0x1;          //OOBMAC to IBMAC interrupt(ISR_BIT15): report result to driver
            return;
        }
        else
        {
            bsp_oobmac_test(NORMALPKT, 0);
            //bsp_oobmac_test(IPV4TCP, 1);   //include multidesc test
            //bsp_oobmac_test(IPV4UDP, 1);   //multidesc
            //bsp_oobmac_test(IPV4, 1);
            //bsp_oobmac_test(IPV6TCP, 1);
            //bsp_oobmac_test(IPV6UDP, 1);
        }
    }
}


void OOBMAC_Test_Task(void)
{
    INT8U t=0;

    //1.register interrupt
//  rlx_irq_register(BSP_GMAC_IRQ,bsp_oobmac_handler); 	 //register OOBMAC interrupt
//  rlx_irq_init();  // it is called in startTask at beginning;restore all intvet and enable interrupt
    //2.initial testItem
//  bsp_oobmac_init();
    //3.test loop

    while(1)
    {
//#if !defined(RKVM_OOBMAC) && !defined(VGA_OOBMAC)
        test_flow_OOBMAC();
//#endif
        OSTimeDly(1);   //unit:10ms

    }
}


