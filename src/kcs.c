#include "OOBMAC.h"
#include <bsp.h>
#include <bsp_cfg.h>
#include "kcs.h"
#include "bsp_cfg_fp_revA.h"

volatile INT32U 	KCSTestRXOK=0;
volatile INT32U 	KCSTestTXOK=0;
volatile INT8U      kcs_flag = 0;

volatile INT32U KCSRXBUFFER ;
volatile INT32U KCSTXBUFFER ;

extern void bsp_wait(INT32U);
//extern void bsp_cmac2_handler(void);


void kcstest_rx_recycle(void)
{
	//--------------------
	volatile INT32U 	length;
	volatile INT32U 	i;
	volatile INT8U  	*Rxptr, *Txptr;
	//--------------------
	//Read Rx descriptor
//	uart_write_str("Rx");
	do
	{
		length=REG8(KCS_BASE+KCS_RX_DESC);
	}while((length & 0x80)==0x80);
	//----------
	length=(length<<8)+REG8(KCS_BASE+KCS_RX_DESC_LEN);	
	//----------
	Rxptr=(INT8U *)KCSRXBUFFER;
	Txptr=(INT8U *)KCSTXBUFFER;
	
	KCSTestRXOK++;
	//--------------------
	REG8(KCS_BASE + KCS_RX_DESC) = 0x80;
	//--------------------
	//set val8 to the rxbuf value
	//kcs_ptr=(int8*)(KCS_RxBufStartAddr);
	//kcs_ptr=kcs_ptr+0x80100000;
	//val8 =*(KCSRXBUFFER);
	//for(i=0;i<length;i++)
	//    *(KCSTXBUFFER+i)=val8++;
	//--------------------
	for(i=0;i<length;i++)
	{
		*(Txptr+i)=*(Rxptr+i);
	}
	//--------------------
	if (length!=256)
	{
		REG8(KCS_BASE+KCS_TX_DESC_LEN)=length;
		REG8(KCS_BASE+KCS_TX_DESC)=0x80;
	}
	else
	{
		REG8(KCS_BASE+KCS_TX_DESC_LEN)=0x00;
		REG8(KCS_BASE+KCS_TX_DESC)=0x81;
	}
	//--------------------
}

void kcstest_tx_count(void)
{
//	uart_write_str("Tx");
	KCSTestTXOK++;
}

void kcstest_abort(void)
{
//	uart_write_str("Ab");
	REG8(KCS_BASE+KCS_ERROR_CODE)=0xC0;
	REG8(KCS_BASE+KCS_CTRL)=0x01;
}
extern char char_temp[];//char buffer for UART putout

void bsp_kcsTest_handler(void)
{
	volatile INT16U val16;
	//--------------------
	REG8(KCS_BASE + KCS_IMR)=0x00;
	val16 = REG16(KCS_BASE+KCS_ISR);
	//--------------------
	if((val16 & 0x1)!=0)	//rxok
	  	kcstest_rx_recycle();
	if((val16 & 0x2)!=0)	//txok
	        kcstest_tx_count();
	if((val16 & 0x4)!=0)	//ABORT
	        kcstest_abort();
	if((val16 & 0x8)!=0)	//Get Status
		{
			//uart_write_str("Get");
	        bsp_kcsTest_init();
		}
	//--------------------
	//if((val16 & 0x10)!=0)//Invalid cmd

	//if((val16 & 0x20)!=0)//wr_cmd_sts

	//if((val16 & 0x40)!=0)//wr_data_in_sts
	//--------------------
	if((val16 & 0x80)!=0)//PCI reset
	       {
			//uart_write_str("Pci reset!\r\n");
	        bsp_kcsTest_init();
		}
	//--------------------
	REG16(KCS_BASE+KCS_ISR)=val16;
	REG8(KCS_BASE+KCS_IMR)=0x9f;
	kcs_flag = 1;

	//--------------------
//	bsp_cmac2_handler();
}

void bsp_kcsTest_init(void)
{
	if(KCSRXBUFFER!= NULL)
	{
		free(KCSRXBUFFER);
	}
	KCSRXBUFFER=malloc(0x100);

	if(KCSTXBUFFER!= NULL)
	{
		free(KCSTXBUFFER);
	}
	KCSTXBUFFER=malloc(0x100);
		
	// set Tx,Rx buffer address and OWN bit
	REG32(KCS_BASE+KCS_RX_DMA_ADDR)=VA2PA(KCSRXBUFFER);
	REG8(KCS_BASE+KCS_RX_DESC)=0x80;//OWN bit
	REG32(KCS_BASE+KCS_TX_DMA_ADDR)=VA2PA(KCSTXBUFFER);
	REG8(KCS_BASE+KCS_TX_DESC)=0x00;//OWN bit

	REG8(KCS_BASE+KCS_CONF)=0x06; //tx_enable, rx_enable
	REG8(KCS_BASE+KCS_ERROR_CODE)=0x00;
	REG8(KCS_BASE+KCS_CTRL)=0x01;
	//--------------------
	REG16(KCS_BASE + KCS_ISR)=0xffff;
	REG8(KCS_BASE  + KCS_IMR)=0x9f;
//--------------------
}


void bsp_kcs_disable(void)
{
//--------------------
REG8(KCS_BASE+KCS_STATUS2)=REG8(KCS_BASE+KCS_STATUS2)|0xC2;//Disbale IBF = 1, State = error
bsp_wait(1000);
REG8(KCS_BASE+KCS_CONF)=0x00; //tx/rx disable
bsp_wait(1000);
//--------------------
}

void kcs_send(INT32U len,INT8U data)
{
	volatile INT8U *Txptr;
	INT32U i;
	Txptr=(INT8U *)KCSTXBUFFER;
	for(i=0;i<len;i++)
	{
		*Txptr++ = data++;
	}
	len = len & 0x1ff;
	if (len!=256)
	{
		REG8(KCS_BASE+KCS_TX_DESC_LEN)=len;
		REG8(KCS_BASE+KCS_TX_DESC)=0x80;// set tx OWN bit
	}
	else
	{
		REG8(KCS_BASE+KCS_TX_DESC_LEN)=0x00;
		REG8(KCS_BASE+KCS_TX_DESC)=0x81;// set tx OWN bit
	}
}

/* function for KCS sent software interupt */
void kcs_send_sw_int(void)
{
	INT8U temp;
	temp = REG8(KCS_BASE)|0x01;//set software interrupt
	REG8(KCS_BASE) = temp;// send software interupt
}



void KCS_Test_Task(void)
{
	
	
	//1.register interrupt
//	rlx_irq_register(BSP_GMAC_IRQ,bsp_oobmac_handler); 		//register OOBMAC interrupt 
//	rlx_irq_register(BSP_KCS_IRQ, bsp_kcsTest_handler);
//	rlx_irq_init();  // it is called in startTask at beginning;restore all intvet and enable interrupt
	//2.initial testItem
//	bsp_oobmac_init();  
//	bsp_kcsTest_init();
	//3.test loop	
   while(1)
   {
	   OSTimeDly(10);	 //100*10ms=1s	set timer interrupt 10ms
   }
}



