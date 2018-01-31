#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <bsp_cfg.h>

#include "TCRF.h"
#include "OOBMAC_COMMON.h"



extern void bsp_wait(INT32U usec);


INT8U * TCRDescAddr = NULL;
INT8U * TCRBuffAddr = NULL;

volatile INT8U	TCRDescPtr=0;
volatile INT32U TCR_TOK;
volatile INT32U TCR_Tx_total;
volatile INT32U TCR_ROK;
volatile INT32U TCR_Reset;

volatile INT32U TCR_INT_CNT=0;
volatile INT32U TCR_INT_CNT_Rx=0;
volatile INT8U  TCR_thr_int_ready=0;



void TCR_Test_Task(void)
{
    INT32U data_temp=0;

#if 0
    //1.register interrupt
    rlx_irq_register(BSP_TCR_IRQ,bsp_tcrfp_handler); 		//register OOBMAC interrupt
//	rlx_irq_init();  // it is called in startTask at beginning;restore all intvet and enable interrupt
    //2.initial testItem
    bsp_tcrfp_init();
#endif

    //3.test loop
    while(1)
    {
#if 0
        if(TCR_thr_int_ready)   //TOK interrupt flag
        {
            if(tcr_read_thr())  //polling valid own bit
                TCR_thr_int_ready = 0;
        }
#endif

        OSTimeDly(OS_TICKS_PER_SEC/10);//10
    }
}


void bsp_tcrfp_handler()
{
    volatile INT16U 	val16;
//    INT8U j=0;
//    TCRTxDesc *txdesc;

    REG16(TCR_IOBASE+TCR_IMR)=0x0;
    val16=REG16(TCR_IOBASE+TCR_ISR);
    REG16(TCR_IOBASE+TCR_ISR) = val16;

//   TCR_INT_CNT++;  //add for debug by melody
    if((val16&0x4)!=0)   //pciereset
    {
        bsp_tcrfp_init();
    }

    if((val16&0x2)!=0)  //tok and loopback
    {
#if 0
        //TCR_INT_CNT_Rx++;   //add for debug by melody
        TCR_thr_int_ready = 1;
        //tcr_read_thr();
        if(TCR_thr_int_ready)   //TOK interrupt flag
        {
            if(tcr_read_thr())  //polling valid own bit
                TCR_thr_int_ready = 0;
        }
#endif

       

        
        while(!tcr_read_thr());
    }

    REG16(TCR_IOBASE+TCR_IMR)=0x000e;
}

INT8U tcr_read_thr(void)
{
    INT8U j=0;
    TCRTxDesc *txdesc;
    for(j=0; j<THR_DESC_NUM; j++)
    {
        txdesc = (TCRTxDesc *)(TCRDescAddr + TCRDescPtr*8);
        if((txdesc->OWN) == 0)
        {
            TCR_ROK++;
            bsp_tcr_send(txdesc->Length,  (INT8U *)PA2VA(txdesc->Bufferaddr));
            txdesc->Length=0x80;
            txdesc->Bufferaddr=VA2PA(TCRBuffAddr+TCRDescPtr*0x80);
            //TCRTxDesc->Bufferaddr=VA2PA(TCRBuffAddr+TCRDescPtr*0x80 + (rand()%4));//for any byte test
            txdesc->OWN=0x01;
            TCR_TOK++;

            TCRDescPtr=(TCRDescPtr+1)%THR_DESC_NUM;
        }
        else
        {
            break;// TCR THR TX data still not ready
        }
    }

    //if j==0,means own bit is not cleared by HW even though interrupt is ok
    //issue: cleared own bit is later than interrupt
    return j;
}

void bsp_tcrfp_init(void)
{
//Enable THR and RBR should disable first, and enable  later to let the thr pointer to be reset
    INT8U	temp8,i;
    INT32U	counter=0;
    TCRTxDesc *txdesc;
    INT8U *tmp;
    TCRDescPtr=0;

    REG8(TCR_IOBASE+TCR_CONF0)=0x00;
//check if ¡¥THR_en_sync¡¦ is ¡¥low¡¦ already.
//It represents all internal states have been cleared when ¡§THR_en_sync¡¨ is low.
    do
    {
        temp8=REG8(TCR_IOBASE+TCR_CONF0);
        counter++;
    }
    while((temp8 & 0x01)!=0x00 && counter<1000);

//enable fifo
    REG8(TCR_IOBASE + TCR_FCR)=0x01;

    REG32(TCR_IOBASE + TCR_TPT)=0x00001000;
    REG32(TCR_IOBASE + TCR_TIMT)=0x00018000;//0x00018000;//125000000 = 16S

    REG8(TCR_IOBASE + TCR_TIMPC)=0x20;

//at most 64 packets
    REG32(TCR_IOBASE + TCR_RBR_IFG)=0x0000000C;
    REG16(TCR_IOBASE + TCR_ISR) = 0xFFFF ;

//---------------------------------------------------

    TCRDescAddr= malloc(THR_DESC_NUM*THR_DESC_LEN);
    TCRBuffAddr= malloc(THR_DESC_NUM*THR_BUF_LEN);




    tmp = (INT8U*) TCRDescAddr;
    memset(tmp, 0, THR_DESC_LEN*THR_DESC_NUM);

    tmp = (INT8U*) TCRBuffAddr;
    memset(tmp, 0, THR_BUF_LEN*THR_DESC_NUM);



//Descriptor Base Address
    REG32(TCR_IOBASE+TCR_DESCADR)=VA2PA(TCRDescAddr);

    /* Tx desciptor setup */
    for(i=0; i<THR_DESC_NUM; i++)
    {
        txdesc = ((TCRTxDesc *)TCRDescAddr)+i;

        txdesc->Bufferaddr= VA2PA(TCRBuffAddr);
        txdesc->OWN = 1;
        txdesc->Length = THR_BUF_LEN;
        if(i ==(THR_DESC_NUM-1))
        {
            txdesc->EOR = 1;
        }
    }



//---------------------------------------------------
#if 0

//Descriptor Base Address
    REG32(TCR_IOBASE+TCR_DESCADR)=VA2PA((INT32U)malloc(TCRDescAddr));
//Descriptor 1
    for(i=0; i<THR_DESC_NUM; i++)
    {
        if(i==THR_DESC_NUM-1)
            REG32(TCRDescAddr+i*8)=0xC0000000|THR_BUF_LEN;
        else
            REG32(TCRDescAddr+i*8)=0x80000000|THR_BUF_LEN;
        REG32(TCRDescAddr+i*8+4)=VA2PA(TCRBuffAddr+i*0x80);
    }
#endif



    REG16(TCR_IOBASE+TCR_IMR)=0x000E;// enable timer mitigation & THR interrupt
    REG8(TCR_IOBASE+TCR_CONF0)=0xE8;
}


void bsp_tcr_disable(void)
{
    REG8(TCR_IOBASE+TCR_MSR)=REG8(TCR_IOBASE+TCR_MSR)&0xEF;
    REG8(TCR_IOBASE+TCR_CONF0)=0x00;
    bsp_wait(1000);
}



void bsp_tcr_send(INT16S length, INT8U *ptr)
{

    volatile INT8U  	*xaddr;
    INT32U	i,counter=0;

    volatile static INT8U  *rbrdesc=(INT8U *)(TCR_IOBASE + TCR_RBR_DESC_START);

    xaddr=(INT8U *)(TCR_IOBASE+0x40);

    do
    {
        if(length>=16)
        {
            for(i=0; i<16; i++)
            {
                *(xaddr+i)=*(ptr+i);
            }
            *rbrdesc=0x90;
        }
        else
        {
            for(i=0; i<length; i++)
            {
                *(xaddr+i)=*(ptr+i);
            }
            *rbrdesc=(0x80|(INT8U)length);
        }

        do
        {
            counter++;
            bsp_wait(10);
        }
        while((*rbrdesc&0x80)==0x80 && counter<10000 );

        if(counter==10000)
        {
            TCR_Reset++;
            bsp_tcrfp_init();
        }

        ptr=ptr+16;
        length=length-16;//length must be signe
    }
    while(length>0);
}
