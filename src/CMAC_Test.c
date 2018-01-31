#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <bsp_cfg.h>

#include "Cmac.h"
#include "OOBMAC_COMMON.h"
#include "OOBMAC.h"

//#include "dma.h"


extern volatile INT8U 	dma_flag;
extern void bsp_wait(INT32U);
volatile INT8U cmac_test_flag = 0;
//use dma for Rx data copy to Tx
extern struct DMA_INFO dma_info0;

volatile INT8U	hwtxptr_cmac_test = 0x00;
volatile INT8U	hwrxptr_cmac_test = 0x00;
volatile CMACRXdesc	*rxd_cmac_test = NULL;
volatile CMACTXdesc *txd_cmac_test = NULL;
volatile INT32U CMACTest_TOTAL_SEND = 0;
volatile INT32U CMACTest_TOK = 0;
volatile INT32U CMACTest_TDU = 0;
volatile INT32U CMACTest_ROK = 0;
volatile INT32U CMACTest_RDU = 0;
volatile INT32U CMACTest_DataError = 0;
volatile INT32U CMACTest_LenError = 0;
volatile INT8U CMACTest_TX_Need_Disable = 0;
volatile INT8U CMAC_ALL_Need_Disable = 0;

INT32U CMAC_DelayCnt = 10;
INT32U CMAC_PKT_length = 64;

INT32U CMAC_Reset_Count = 0;

volatile INT16U last_ISR_test = 0;
volatile INT16U pre_ISR_test = 0;
INT32U PKT_length = 64;
INT8U  RX_DISABLE = 0;
INT32U RX_DISABLE_Count = 0;
INT32U RX_DISABLE_Count_MAX = 2000;//1000000;
INT32U reg_test_success = 0;
INT32U reg_test_fail = 0;

extern char char_temp[];
extern volatile INT8U	CmacCont;
extern volatile INT8U 	CMAC_Reinit;

INT8U* CMACTestRxdescStartAddr=NULL;
INT8U* CMACTestTxdescStartAddr=NULL;


void CMAC_testing_func_test(void);
void bsp_cmac_init_test(void);


void bsp_null_handler()
{

}

void bsp_cmac_FullRst_test(void)
{
    INT16U temp;
    REG8(OOBMAC_IOBASE + 0x150) |= 0x20;
    bsp_wait(300);

    REG16(CMAC_IOBASE+CMAC_IMR)=0;
    temp = REG16(CMAC_IOBASE+CMAC_ISR);
    REG16(CMAC_IOBASE+CMAC_ISR) = temp;

}

void bsp_cmac_handler_test()
{

    volatile INT16U val = 0;
    INT8U i = 0;
    INT8U temp;
    INT8U *rddataptr;
    volatile INT16U		IMR_CMAC;
    INT32U u;
    INT32U CMAClength;

    //INT32U		*pcie_reset;
    IMR_CMAC = REG16(CMAC_IOBASE+CMAC_IMR);
    REG16(CMAC_IOBASE+CMAC_IMR)=0x0000;
    val=REG16(CMAC_IOBASE+CMAC_ISR);
    REG16(CMAC_IOBASE+CMAC_ISR)=val;//write 1 to any bit in the ISR will reset that bit

    do
    {
        last_ISR_test=val;
        if((val&(DWBIT00|DWBIT01))!=0)//ROK & RDU
        {
            cmac_test_flag =1;
            if(val&DWBIT01)//RDU
                CMACTest_RDU++;

            for(i=0; i<CMACRxdescNumber; i++)
            {
                rxd_cmac_test=(CMACRXdesc *)(CMACTestRxdescStartAddr)+hwrxptr_cmac_test;

                if(rxd_cmac_test->OWN==1)
                {
                    break;
                }
                else
                {
                    CMACTest_ROK++;

#if CMAC_CKECK//check data
                    rddataptr=(INT8U *)(PA2VA(rxd_cmac_test->BufferAddress));//optional unchacheable address
                    //check packet length
                    if(*rddataptr!= ((rxd_cmac_test->Length&0xFF00)>>8) ||
                            *(rddataptr+1)!= (rxd_cmac_test->Length&0x00FF))
                    {
                        REG8(CMAC_IOBASE+CMAC_IMR1) = 0xAA;
                        /*
                        uart_write_str("CMAC rx length error! \r\n");
                        itoa_self(*rddataptr,char_temp,16);
                        uart_write_str(char_temp);
                        uart_write_str("  ");
                        itoa_self(*(rddataptr+1),char_temp,16);
                        uart_write_str(char_temp);
                        uart_write_str("  ");
                        itoa_self(rxd_cmac->Length,char_temp,16);
                        uart_write_str(char_temp);
                        uart_write_str("  ");
                        */
                        //while(1);
                    }
                    //check serial data
                    temp = *(rddataptr+2);
                    for(u=2; u<rxd_cmac_test->Length; u++)
                    {
                        if((temp++) != *(rddataptr+u))
                        {
                            REG8(CMAC_IOBASE+CMAC_IMR1) = 0xAA;
                            CMACTest_DataError++;
                            /*
                            uart_write_str("CMAC rx data error! \r\n");
                            itoa_self(*(rddataptr+u-1),char_temp,16);
                            uart_write_str(char_temp);
                            uart_write_str("  ");
                            itoa_self(temp,char_temp,16);
                            uart_write_str(char_temp);
                            uart_write_str("  ");
                            itoa_self(u,char_temp,10);
                            uart_write_str(char_temp);
                            uart_write_str("  \r\n");
                            */
                            //while(1);

                        }

                    }
#endif
                    //send back to IB
#if CMAC_RX2TX
                    rddataptr=(INT8U *)(PA2VA(rxd_cmac_test->BufferAddress));//optional unchacheable address
                    bsp_cmac_send_test(rddataptr, rxd_cmac_test->Length);
#endif

                }
                //Release RX descriptor
                rxd_cmac_test->OWN=1;
                //Update descriptor pointer
                hwrxptr_cmac_test=(hwrxptr_cmac_test+1)%CMACRxdescNumber;
            }
        }

        if(val & DWBIT02)//TOK not that exactly; compare dostool tx number with FW rx number
        {
            CMACTest_TOK++;
        }

        if(val & DWBIT03)//TDU
        {
            CMACTest_TDU++;
        }

        if(val & DWBIT06)
        {
            CMACTest_TX_Need_Disable=1;
        }

        if(val & DWBIT07)
        {
            CMAC_ALL_Need_Disable=1;
        }

        if(val & DWBIT08)
        {
            CMAC_ALL_Need_Disable=1;
        }

        val=REG16(CMAC_IOBASE+CMAC_ISR);
        REG16(CMAC_IOBASE+CMAC_ISR)=val;//write 1 to any bit in the ISR will reset that bit
    }
    //while((val&0x01CF)!=0);
    while((val&0x014F)!=0);   //modified by melody@20161027: pcierst only do once
    //while((val&0xCF)!=0);

    REG16(CMAC_IOBASE+CMAC_IMR)=IMR_CMAC & (~DWBIT07);  //close pcierst
    pre_ISR_test=last_ISR_test;

}



void bsp_disable_cmac_rx_test(void)
{
    INT8U tmp;
    //tmp=REG16(CMAC_IOBASE+CMAC_IMR);
    // REG16(CMAC_IOBASE+CMAC_IMR)=0xFFFC&tmp;
    //Disable RX
    tmp= REG8(CMAC_IOBASE+CMAC_OCR0);
    tmp&=0xFE;
    REG8(CMAC_IOBASE+CMAC_OCR0)=tmp;
//	hwrxptr_cmac=0;

}


void bsp_enable_cmac_rx_test(void)
{
    INT32U i;
    INT16U tmp=0;
    //Rx descriptor setup
    for(i=0; i<CMACRxdescNumber; i++)
    {
        if(i == (CMACRxdescNumber - 1))
            REG32(CMACTestRxdescStartAddr+i*16)=0xC0000000;
        else
            REG32(CMACTestRxdescStartAddr+i*16)=0x80000000;

        REG32(CMACTestRxdescStartAddr+i*16+4)=0x0;
        //REG32(CMACRxdescStartAddr+i*16+8)=VA2PA(i+CMACRXBUFFER+i*0x600);  //mask for debug
        //REG32(CMACRxdescStartAddr+i*16+8)=(VA2PA(CMACRXBUFFER+i*0x600)&0x0fffffff);     //not any byte assignment  mask for DDR
        //REG32(CMACRxdescStartAddr+i*16+8)=(CMACRXBUFFER+i*0x600)|0x20000000;
        REG32(CMACTestRxdescStartAddr+i*16+8)=((RXdesc *)(CMACTestRxdescStartAddr)+i)->BufferAddress;
        REG32(CMACTestRxdescStartAddr+i*16+12)=0x0;
    }
    //REG32(CMAC_IOBASE+CMAC_RDSAR)=(VA2PA(CMACRxdescStartAddr)&0x0fffffff); //mask for DDR
    //REG32(CMAC_IOBASE+CMAC_RDSAR)=(CMACRxdescStartAddr)|0x20000000;
    REG32(CMAC_IOBASE+CMAC_RDSAR)=VA2PA(CMACTestRxdescStartAddr);
    //Interrupt Configuration
    tmp=REG16(CMAC_IOBASE+CMAC_IMR);
    REG16(CMAC_IOBASE+CMAC_IMR)=0xFFFC&tmp;
    tmp=REG16(CMAC_IOBASE+CMAC_ISR);
    REG16(CMAC_IOBASE+CMAC_ISR)=0x0003|tmp;//clear
    tmp=REG16(CMAC_IOBASE+CMAC_IMR);
    REG16(CMAC_IOBASE+CMAC_IMR)=0x0003|tmp;
    hwrxptr_cmac_test=0;

    //multi-bust length any-byte alignment
    REG8(CMAC_IOBASE+0xA0) &= ~0x18;
    //Enable RX
    REG8(CMAC_IOBASE+CMAC_OCR0)=0x01;
}


void bsp_disable_cmac_tx_test(void)
{
    INT8U tmp;
    //tmp=REG16(CMAC_IOBASE+CMAC_IMR);
    //REG16(CMAC_IOBASE+CMAC_IMR)=0xFFF3&tmp;
    //Disable TX
    tmp = REG8(CMAC_IOBASE+CMAC_OCR2);
    tmp &= 0xFE;
    REG8(CMAC_IOBASE+CMAC_OCR2) = tmp;
    // hwtxptr_cmac=0;
}


void bsp_enable_cmac_tx_test(void)
{
    INT32U i;
    INT16U tmp=0;
    //Tx desciptor setup
    for(i=0; i<CMACTxdescNumber; i++)
    {
        if(i ==(CMACTxdescNumber-1))
            REG32(CMACTestTxdescStartAddr+i*16)=0x70000000;
        else
            REG32(CMACTestTxdescStartAddr+i*16)=0x30000000;

        REG32(CMACTestTxdescStartAddr+i*16+4)=0x0;
        REG32(CMACTestTxdescStartAddr+i*16+8)=((TXdesc *)(CMACTestTxdescStartAddr)+i)->BufferAddress;
        //REG32(CMACTxdescStartAddr+i*16+8)=VA2PA(CMACTXBUFFER+i*0x600);
        //REG32(CMACTxdescStartAddr+i*16+8)=VA2PA(CMACTXBUFFER+i*0x600);  //not any byte assignment for debug
        //zero copy, no need to set tx buffers
        REG32(CMACTestTxdescStartAddr+i*16+12)=0x0;
    }

    REG32(CMAC_IOBASE+CMAC_TNPDS)=VA2PA(CMACTestTxdescStartAddr);
    //Interrupt Configuration
    tmp=REG16(CMAC_IOBASE+CMAC_IMR);
    REG16(CMAC_IOBASE+CMAC_IMR)=0xFFF3&tmp;
    tmp=REG16(CMAC_IOBASE+CMAC_ISR);
    REG16(CMAC_IOBASE+CMAC_ISR)=0x000c|tmp;//clear
    tmp=REG16(CMAC_IOBASE+CMAC_IMR);
    REG16(CMAC_IOBASE+CMAC_IMR)=0x000c|tmp;
    hwtxptr_cmac_test=0;
    //Enable TX
    REG8(CMAC_IOBASE+CMAC_OCR2)=0x01;
}

void bsp_cmac_send_test(INT8U *data, INT32U size)
{
    INT32U i = 0;
    INT8U *buf = NULL;

    txd_cmac_test=(CMACTXdesc *)(CMACTestTxdescStartAddr)+hwtxptr_cmac_test;
    if(txd_cmac_test->OWN==1)
    {
        return;
    }
    else
    {
        buf = (INT8U *)PA2VA(txd_cmac_test->BufferAddress);

        //dma_large(size,(INT32U)data,(INT32U)buf,&dma_info0);
        //start_channel0();
        //while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
        //dma_flag = 0;

        for(i=0; i<size; i++)
            *(buf+i) = *(data+i);
        txd_cmac_test->TAGC=0;
        txd_cmac_test->Length=size;
        txd_cmac_test->FS=1;
        txd_cmac_test->LS=1;
        txd_cmac_test->BufferAddress=VA2PA(buf);
        txd_cmac_test->OWN=1;
        REG8(CMAC_IOBASE+CMAC_OCR2)=(REG8(CMAC_IOBASE+CMAC_OCR2)|0x02);
        REG8(CMAC_IOBASE+CMAC_OCR2)=(REG8(CMAC_IOBASE+CMAC_OCR2)|0x02);
        //while (txd_cmac->OWN==1);

        hwtxptr_cmac_test=(hwtxptr_cmac_test+1)%CMACTxdescNumber;
    }

}

void bsp_cmac_memory_init(void)
{
    INT8U * tmp;
    INT8U i;
    CMACTXdesc *txdesc;
    CMACRXdesc *rxdesc;

    CMACTestTxdescStartAddr = malloc(CMACTxdescNumber*sizeof(struct _CMACTxDesc));
    if(CMACTestTxdescStartAddr==NULL)
    {
        bsp_4everloop(0);
    }

    CMACTestRxdescStartAddr= malloc(CMACRxdescNumber*sizeof(struct _CMACRxDesc));
    if(CMACTestRxdescStartAddr==NULL)
    {
        bsp_4everloop(0);
    }

    tmp = (INT8U*) CMACTestTxdescStartAddr;
    memset(tmp, 0, sizeof(TXdesc)*CMACTxdescNumber);

    // Tx desciptor initial
    for(i=0; i<CMACTxdescNumber; i++)
    {
        txdesc = ((TXdesc *)CMACTestTxdescStartAddr)+i;
        txdesc->FS = 1;
        txdesc->LS = 1;
        txdesc->BufferAddress = VA2PA((INT32U)malloc(0x600));
        if(i ==(CMACTxdescNumber-1))
        {
            txdesc->EOR = 1;
        }
    }

    tmp = (INT8U*) CMACTestRxdescStartAddr;
    memset(tmp, 0, sizeof(RXdesc)*CMACRxdescNumber);

    // Rx descriptor initial
    for(i=0; i<CMACRxdescNumber; i++)
    {
        rxdesc = ((RXdesc *)CMACTestRxdescStartAddr)+i;
        rxdesc->Length = 0x600;
        rxdesc->BufferAddress = VA2PA((INT32U)malloc(0x600));
        if(i == (CMACRxdescNumber - 1))
        {
            rxdesc->EOR = 1;
        }
        rxdesc->OWN = 1;
    }

}


void bsp_cmac_init_test(void)
{
    INT16U tmp=0;
    //bsp_cmac_FullRst_test();
    //Disable RX
    REG8(CMAC_IOBASE+CMAC_OCR0)=0x00;
    //Disable TX
    REG8(CMAC_IOBASE+CMAC_OCR2)=0x00;

    //reset and enable
    //bit6 : rx_disable_status
    //bit7 : pcie_reset  enabled in pcierst high interrupt in fun0
    //bit5 : oob_tx_disable
    tmp=REG16(CMAC_IOBASE+CMAC_IMR);
    REG16(CMAC_IOBASE+CMAC_IMR)=0xFF3F&tmp;
    tmp=REG16(CMAC_IOBASE+CMAC_ISR);
    REG16(CMAC_IOBASE+CMAC_ISR)=0x00C0|tmp;//clear
    tmp=REG16(CMAC_IOBASE+CMAC_IMR);
    REG16(CMAC_IOBASE+CMAC_IMR)=0x0040|tmp;

    bsp_cmac_memory_init();
    bsp_enable_cmac_tx_test();
    bsp_enable_cmac_rx_test();
    //reset simulation status bit
    REG8(CMAC_IOBASE+CMAC_OCR3)=0x60;

    CMAC_Reinit=0;
}

void bsp_cmac_EnablePCIERSTB(void)
{
    REG16(CMAC_IOBASE+CMAC_IMR) |= 0x0080;
}


void bsp_cmac_SerierData_test(INT8U data, INT32U size)
{
    INT32U i = 0;
    INT8U *buf = NULL;

    txd_cmac_test=(TXdesc *)(CMACTestTxdescStartAddr)+hwtxptr_cmac_test;

    if(txd_cmac_test->OWN==1)
    {
        REG8(CMAC_IOBASE+CMAC_OCR2)=(REG8(CMAC_IOBASE+CMAC_OCR2)|0x02);
        return;
    }
    else
    {
        buf = (INT8U *)(PA2VA(txd_cmac_test->BufferAddress));

        //fill packet length to packet content
        *buf = (size&0xFF00)>>8;
        *(buf+1) = size&0x00FF;

        txd_cmac_test->TAGC=0;
        txd_cmac_test->Length=size;
        txd_cmac_test->FS=1;
        txd_cmac_test->LS=1;

        //fill incremental data
        for(i=2; i<size; i++)
            *(buf+i) = ((data++)&0xff);
        txd_cmac_test->OWN=1;

        //REG8(CMAC_IOBASE+CMAC_OCR2)=0x03;
        REG8(CMAC_IOBASE+CMAC_OCR2)=(REG8(CMAC_IOBASE+CMAC_OCR2)|0x02);
        REG8(CMAC_IOBASE+CMAC_OCR2)=(REG8(CMAC_IOBASE+CMAC_OCR2)|0x02);
        //while (txd_cmac->OWN==1);

        hwtxptr_cmac_test=(hwtxptr_cmac_test+1)%CMACTxdescNumber;
    }
    CMACTest_TOTAL_SEND++;
}

//in fact, it is only about initial/reinital setting; it uses loopback method to do DMA which is in CMAC handler
void CMAC_testing_func_test(void)
{
#ifdef CMAC_SERIAL
    INT32U temp;
    if(CMAC_ALL_Need_Disable)   //interrupt
    {
        bsp_wait(100);
        bsp_disable_cmac_tx_test();
        bsp_disable_cmac_rx_test();

        bsp_enable_cmac_rx_test();
        while((REG8(CMAC_IOBASE+CMAC_ISR)&DWBIT05) != DWBIT05)
        {
            //CMAC wait TX disable
        }
        bsp_enable_cmac_tx_test();
        CMAC_ALL_Need_Disable=0;
    }
    else
    {
        //OOB TX
        if(CMACTest_TX_Need_Disable == 0)  //interrupt
        {
            PKT_length = (rand()%(1514-64))+64+1;
            //PKT_length = 65;
            //bsp_cmac_SerierData_test(2,PKT_length);
            bsp_wait(CMAC_DelayCnt);
        }
        else   //normal DMA
        {
            //bsp_wait(100);
            bsp_disable_cmac_tx_test();
#if 0
            while((REG8(CMAC_IOBASE+CMAC_ISR)&DWBIT05) != DWBIT05)
            {
                //CMAC wait TX disable
            }
            REG32(OOBMAC_IOBASE + MAC_DMEMENDA) = 0x55555555;//for dos tool handshake
            do
            {
                temp = REG32(OOBMAC_IOBASE + MAC_DMEMENDA);
            }
            while(temp != 0xaaaaaaaa);
            REG32(OOBMAC_IOBASE + MAC_DMEMENDA) = 0x0;//for dos tool handshake
#endif
            bsp_enable_cmac_tx_test();

            //first_send=1;
            CMACTest_TX_Need_Disable=0;
        }
    }
#endif

    //OOB RX enable/disable test
#ifdef CMAC_DISABLE_ENABLE
    //RX
    //RX_DISABLE_Count++;
    if(RX_DISABLE == 0)// && (RX_DISABLE_Count > RX_DISABLE_Count_MAX))
    {
        //disable RX
        bsp_disable_cmac_rx_test();
        while(REG32(OOBMAC_IOBASE + MAC_DMEMSTA) != 0x55555555 );//for dos tool handshake
        REG32(OOBMAC_IOBASE + MAC_DMEMSTA) = 0;
        RX_DISABLE = 1;
        RX_DISABLE_Count = 0;
    }
    if(RX_DISABLE == 1) //&& (RX_DISABLE_Count > RX_DISABLE_Count_MAX))
    {
        bsp_enable_cmac_rx_test();
        //REG32(OOBMAC_IOBASE + MAC_DMEMSTA) = 0xaaaaaaaa;//for dos tool handshake
        //while(REG32(OOBMAC_IOBASE + MAC_DMEMSTA) != 0xaaaaaaaa );//wait for PCIe Tx OK
        RX_DISABLE = 0;
        RX_DISABLE_Count = 0;
    }
#endif

}


void test_free_Cmac(void)
{
    CMACTXdesc *txdesc;
    CMACRXdesc *rxdesc;
    INT16U i;

    if(CMACTestTxdescStartAddr != NULL)
    {
        free(CMACTestTxdescStartAddr);
    }
    if(CMACTestRxdescStartAddr != NULL)
    {
        free(CMACTestRxdescStartAddr);
    }

    for(i=0; i<TxdescNumber; i++)
    {
        txdesc = ((TXdesc *)CMACTestTxdescStartAddr)+i;
        if(PA2VA(txdesc->BufferAddress) != NULL)
        {
            free(PA2VA(txdesc->BufferAddress));
        }
    }
    for(i=0; i<RxdescNumber; i++)
    {
        rxdesc = ((RXdesc *)CMACTestRxdescStartAddr)+i;
        if(PA2VA(rxdesc->BufferAddress) != NULL)
        {
            free(PA2VA(rxdesc->BufferAddress));
        }
    }

}

void test_flow_CMAC(void)
{
    //if(CmacCont){
    CMAC_testing_func_test();
    //}
    if(CMAC_Reinit!=0)
    {
        test_free_Cmac();
        bsp_cmac_init_test();
    }
}

void CMAC_Test_Task(void)
{

    //1.register interrupt
    //rlx_irq_register(BSP_CMAC_IRQ, bsp_cmac_handler_test);
    //2.initial testItem
    //bsp_cmac_init_test();
    //3.test loop
    while(1)
    {
        test_flow_CMAC();
        OSTimeDly(10);   //100*10ms=1s  set timer interrupt 10ms
    }

}




