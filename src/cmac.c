#include "bsp.h"
#include "cmac.h"
#include "wcom.h"


INT8U	hwtxptr_cmac = 0x00;
INT8U	hwrxptr_cmac = 0x00;
CMACRXdesc	*rxd_cmac = NULL;
CMACTXdesc 	*txd_cmac = NULL;
INT32U CMAC_TOTAL_SEND = 0;
INT32U CMAC_TOK = 0;
INT32U CMAC_TDU = 0;
INT32U CMAC_ROK = 0;
INT32U CMAC_RDU = 0;
INT32U CMAC_DataError = 0;
INT32U CMAC_LenError = 0;
INT32U CMAC_TX_Need_Disable = 0;
INT32U CMAC_RX_Need_Disable = 0;
volatile INT16U last_ISR = 0;
volatile INT16U pre_ISR = 0;

ROM_EXTERN unsigned char *CMACRxdescStartAddr;
ROM_EXTERN unsigned char *CMACTxdescStartAddr;
ROM_EXTERN unsigned char *CMACTXBUFFER;
ROM_EXTERN unsigned char *CMACRXBUFFER;

ROM_EXTERN OS_EVENT *DASHREQSEM;//Shared resource => Task Request Atomic
ROM_EXTERN OS_EVENT *DASHREQLOCK;//Shared resource => Prevent Dash request(0x81) or ack(0x82) at the smae time
ROM_EXTERN OS_EVENT *DASH_OS_Response_Event;//Event
ROM_EXTERN OS_EVENT *DASH_OS_Push_Event;//Event
ROM_EXTERN unsigned char* DASH_ib_write_buffer;
ROM_EXTERN unsigned char* DASH_OS_PSH_Buf;

INT32U CMAC_TEST_LEN = 0;
extern INT32U CMAC_STOP;
//#if defined(CONFIG_BUILDROM)
void CMAC_SW_init()
{
	CMACRxdescStartAddr = (unsigned char *)malloc(0x100);
	CMACTxdescStartAddr = (unsigned char *)malloc(0x100);
	bsp_bits_set(MAC_OOBREG, 1, BIT_FIRMWARERDY, 1);
}

void CMAC_SW_release()
{
	int i;
	
	for(i=0;i<CMACRxdescNumber;i++)
	{
		free(PA2VA(REG32(CMACRxdescStartAddr+i*16+8)));		
	}
	
	free(CMACRxdescStartAddr);
	free(CMACTxdescStartAddr);
}

void bsp_disable_cmac_rx(void)
{
	INT32S i;
	INT8U      *buf;
	INT16U tmp;
	INT8U tmp1;
	
	for(i=0;i<CMACRxdescNumber;i++)
	{
		free(PA2VA(REG32(CMACRxdescStartAddr+i*16+8)));		
	}

	//tmp=REG16(CMAC_IOBASE+CMAC_IMR);
	// REG16(CMAC_IOBASE+CMAC_IMR)=0xFFFC&tmp;
	//Disable RX
	tmp1= REG8(CMAC_IOBASE+CMAC_OCR0);
	tmp1&=0xFE;	
	REG8(CMAC_IOBASE+CMAC_OCR0)=tmp1;
	//	hwrxptr_cmac=0;

}

void bsp_enable_cmac_rx(void)
{
	INT32S i;
	INT8U      *buf;
	INT16U tmp=0;

	//Rx descriptor setup
	for(i=0;i<CMACRxdescNumber;i++)
	{
		if(i == (CMACRxdescNumber - 1))
			REG32(CMACRxdescStartAddr+i*16)=0xC0000000;
		else
			REG32(CMACRxdescStartAddr+i*16)=0x80000000;

		REG32(CMACRxdescStartAddr+i*16+4)=0x0;
		REG32(CMACRxdescStartAddr+i*16+8)=VA2PA(malloc(0x600));
		REG32(CMACRxdescStartAddr+i*16+12)=0x0;
	}

	REG32(CMAC_IOBASE+CMAC_RDSAR)=VA2PA(CMACRxdescStartAddr);
	//Interrupt Configuration
	tmp=REG16(CMAC_IOBASE+CMAC_IMR);
	REG16(CMAC_IOBASE+CMAC_IMR)=0xFFFC&tmp;
	tmp=REG16(CMAC_IOBASE+CMAC_ISR);
	REG16(CMAC_IOBASE+CMAC_ISR)=0x0003|tmp;//clear
	tmp=REG16(CMAC_IOBASE+CMAC_IMR);
	REG16(CMAC_IOBASE+CMAC_IMR)=0x0003|tmp;
	hwrxptr_cmac=0;
	//Enable RX
	REG8(CMAC_IOBASE+CMAC_OCR0)=0x01;
}

void bsp_disable_cmac_tx(void)
{
	INT32S i;
	INT8U      *buf;
	INT16U tmp;
	INT8U tmp1;

	//tmp=REG16(CMAC_IOBASE+CMAC_IMR);
	//REG16(CMAC_IOBASE+CMAC_IMR)=0xFFF3&tmp;
	//Disable TX
	tmp1= REG8(CMAC_IOBASE+CMAC_OCR2);
	tmp1&=0xFE;
	REG8(CMAC_IOBASE+CMAC_OCR2)=tmp1;
	// hwtxptr_cmac=0;
}

void bsp_enable_cmac_tx(void)
{
	INT32S i;
	INT8U      *buf;
	INT16U tmp=0;

	//Tx desciptor setup
	for(i=0;i<CMACTxdescNumber;i++)
	{
		if(i ==(CMACTxdescNumber-1))
			REG32(CMACTxdescStartAddr+i*16)=0x70000000;
		else
			REG32(CMACTxdescStartAddr+i*16)=0x30000000;

		REG32(CMACTxdescStartAddr+i*16+4)=0x0;		
		//by hao
		//REG32(CMACTxdescStartAddr+i*16+8)=VA2PA(CMACTXBUFFER+i*0x600);
		//zero copy, no need to set tx buffers
		REG32(CMACTxdescStartAddr+i*16+12)=0x0;
	}

	REG32(CMAC_IOBASE+CMAC_TNPDS)=VA2PA(CMACTxdescStartAddr);
	//Interrupt Configuration
	tmp=REG16(CMAC_IOBASE+CMAC_IMR);
	REG16(CMAC_IOBASE+CMAC_IMR)=0xFFF3&tmp;
	tmp=REG16(CMAC_IOBASE+CMAC_ISR);
	REG16(CMAC_IOBASE+CMAC_ISR)=0x000c|tmp;//clear
	tmp=REG16(CMAC_IOBASE+CMAC_IMR);
	REG16(CMAC_IOBASE+CMAC_IMR)=0x000c|tmp;
	hwtxptr_cmac=0;

	while(!(REG16(CMAC_IOBASE+CMAC_ISR)&DWBIT05))
	{
	}
	//Enable TX
	REG8(CMAC_IOBASE+CMAC_OCR2)=0x01;
}


//Power state change from S0 to  sx, call enable when Sx->S0 
void bsp_cmac_disable()
{
	bsp_wait(100);	
	bsp_disable_cmac_tx();
	bsp_disable_cmac_rx();
	bsp_enable_cmac_rx();				
}

//Sx->S0 
void bsp_cmac_enable()
{
	while((REG8(CMAC_IOBASE+CMAC_ISR)&BIT5) != BIT5)
	{
		//CMAC wait TX disable		
	}

	bsp_enable_cmac_tx();
}


//InBand Driver disable CMAC rx, wii trigger OOB CMAC ISR bit6
void bsp_ibcmac_disable()
{				
	bsp_wait(100);
	bsp_disable_cmac_tx();
	while((REG8(CMAC_IOBASE+CMAC_ISR)&BIT5) != BIT5)
	{								
	}	
	bsp_cmac_enable();												
}
//#endif


void bsp_cmac_handler_sw_patch()
{
	volatile INT16U val = 0;
	INT8U i = 0;
	INT32U u = 0;
	INT8U *rddataptr;
	INT32U		len;
	volatile INT16U		IMR_CMAC;
	OSOOBHdr *hdr;
	char dbg[64];
	

	IMR_CMAC = REG16(CMAC_IOBASE+CMAC_IMR);
	REG16(CMAC_IOBASE+CMAC_IMR)=0x0000;
	val=REG16(CMAC_IOBASE+CMAC_ISR);	
	REG16(CMAC_IOBASE+CMAC_ISR)=val;//write 1 to any bit in the ISR will reset that bit
	do
	{
		last_ISR=val;

		memset(dbg, 0, 64);
		//printk("bsp_cmac_handler_sw %x \n", val);
		//socket_write(0, dbg, strlen(dbg));

		if((val&(DWBIT00|DWBIT01))!=0)//ROK & RDU
		{
			if(val&DWBIT01)//RDU
			{

			}

			for(i=0;i<CMACRxdescNumber;i++)
			{
				rxd_cmac=(CMACRXdesc *)(CMACRxdescStartAddr)+hwrxptr_cmac;
				if(rxd_cmac->OWN==1)
				{
					break;
				}
				else
				{
					rddataptr=(INT8U *)PA2VA(rxd_cmac->BufferAddress);

#if 0
					//SD3 Test

					memcpy(DASH_OS_PSH_Buf, rddataptr, rxd_cmac->Length);
					CMAC_TEST_LEN = rxd_cmac->Length;

					OSSemPost(DASH_OS_Push_Event);
#else
					hdr = (OSOOBHdr *)rddataptr;

					if(hdr->hostReqV == 0x91)
					{
						if(DASH_ib_write_buffer!=0)
						{
							memcpy(DASH_ib_write_buffer, rddataptr, rxd_cmac->Length);
						}
						OSSemPost(DASH_OS_Response_Event);
					}
					else if((hdr->hostReqV == 0x92) || (hdr->hostReqV == 0x85) || (hdr->hostReqV == 0x02)|| (hdr->hostReqV == 0x06))
					{
						memcpy(DASH_OS_PSH_Buf, rddataptr, rxd_cmac->Length);
						OSSemPost(DASH_OS_Push_Event);
					}

					memset(dbg, 0, 64);
					//sprintf(dbg, "hdr->hostReqV %x \n", hdr->hostReqV);
					//socket_write(0, dbg, strlen(dbg));
#endif				
					//
					//swfun(rddataptr, rxd_cmac->Length);
				}
				//Release RX descriptor
				rxd_cmac->OWN=1;
				//Update descriptor pointer
				hwrxptr_cmac=(hwrxptr_cmac+1)%CMACRxdescNumber;
			}
		}


		if(val & DWBIT02)//TOK
		{
			OSSemPost(DASHREQLOCK);
		}

		if(val & DWBIT03)//TDU
		{
		}
		
		//printf("CMAC ISR:%d\n", val);

		if(val & DWBIT06)
		{
			CMAC_TX_Need_Disable=1;
		}

		if(val & DWBIT07)
		{
			
			CMAC_RX_Need_Disable=1;
		}

		val=REG16(CMAC_IOBASE+CMAC_ISR);
		REG16(CMAC_IOBASE+CMAC_ISR)=val;//write 1 to any bit in the ISR will reset that bit
	}while((val&0x4F)!=0);
	
	bsp_cmac_reset();

	REG16(CMAC_IOBASE+CMAC_IMR)=0x4f;
}

void bsp_cmac_reset()
{
	#if 1
	if(CMAC_TX_Need_Disable)
	{
		bsp_disable_cmac_tx();
		while(!(REG16(CMAC_IOBASE+CMAC_ISR)&DWBIT05))
		{
		}
		bsp_enable_cmac_tx();
		CMAC_TX_Need_Disable = 0;	
	}
	
	if(CMAC_RX_Need_Disable)
	{
		bsp_disable_cmac_tx();
		while(!(REG16(CMAC_IOBASE+CMAC_ISR)&DWBIT05))
		{
		}
		
		bsp_disable_cmac_rx();
		bsp_enable_cmac_rx();
		bsp_enable_cmac_tx();
		CMAC_RX_Need_Disable = 0;
	}
	#endif
}
void bsp_cmac_init()
{
	INT32S i;
	INT8U      *buf;
	INT16U tmp=0;

	CMAC_SW_init();

	//Disable RX
	REG8(CMAC_IOBASE+CMAC_OCR0)=0x00;
	//Disable TX
	REG8(CMAC_IOBASE+CMAC_OCR2)=0x00;

	//reset and enable 
	//bit6 : rx_disable_status 
	//bit7 : pcie_reset
	tmp=REG16(CMAC_IOBASE+CMAC_IMR);
	REG16(CMAC_IOBASE+CMAC_IMR)=0xFF3F&tmp;
	tmp=REG16(CMAC_IOBASE+CMAC_ISR);
    	REG16(CMAC_IOBASE+CMAC_ISR)=0x00C0|tmp;//clear
	tmp=REG16(CMAC_IOBASE+CMAC_IMR);
    	REG16(CMAC_IOBASE+CMAC_IMR)=0x00C0|tmp;

	bsp_enable_cmac_tx();
#if 0
	//Tx desciptor setup
	for(i=0;i<CMACTxdescNumber;i++)
	{
		if(i ==(CMACTxdescNumber-1))
			REG32(CMACTxdescStartAddr+i*16)=0x70000000;
		else
			REG32(CMACTxdescStartAddr+i*16)=0x30000000;

		REG32(CMACTxdescStartAddr+i*16+4)=0x0;
		REG32(CMACTxdescStartAddr+i*16+8)=VA2PA(CMACTXBUFFER+i*0x600);
		//zero copy, no need to set tx buffers
		REG32(CMACTxdescStartAddr+i*16+12)=0x0;
	}
#endif

	//tmp=REG16(CMAC_IOBASE+CMAC_ISR);
    	//REG16(CMAC_IOBASE+CMAC_ISR)=0x0020|tmp;//clear
    	
#if 0//CMAC_simulation_case2
	buf = CMACTXBUFFER;
	for(i=2; i<0x600; i++)
		*(buf+i) = (i%0x100);
#endif

	bsp_enable_cmac_rx();
#if 0
	//Rx descriptor setup
	for(i=0;i<CMACRxdescNumber;i++)
	{
		if(i == (CMACRxdescNumber - 1))
			REG32(CMACRxdescStartAddr+i*16)=0xC0000000;
		else
			REG32(CMACRxdescStartAddr+i*16)=0x80000000;

		REG32(CMACRxdescStartAddr+i*16+4)=0x0;
		REG32(CMACRxdescStartAddr+i*16+8)=VA2PA(CMACRXBUFFER+i*0x600);
		REG32(CMACRxdescStartAddr+i*16+12)=0x0;
	}
#endif

#if 0
	//Assign Tx and Rx descriptor address
	REG32(CMAC_IOBASE+CMAC_RDSAR)=VA2PA(CMACRxdescStartAddr);
	REG32(CMAC_IOBASE+CMAC_TNPDS)=VA2PA(CMACTxdescStartAddr);

	//Interrupt Configuration
	REG16(CMAC_IOBASE+CMAC_IMR)=0x0000;
	REG16(CMAC_IOBASE+CMAC_ISR)=0x000F;//clear
	REG16(CMAC_IOBASE+CMAC_IMR)=0x000F;

	//Enable RX
	REG8(CMAC_IOBASE+CMAC_OCR0)=0x01;
	//Enable TX
	REG8(CMAC_IOBASE+CMAC_OCR2)=0x01;
#endif

	//DMEM bridge reset
	//REG8(IOREG_IOBASE+0x0150)|=0x04;
	//REG8(IOREG_IOBASE+0x0150)=0x00;

	//reset simulation status bit
	REG8(CMAC_IOBASE+CMAC_OCR3)=0;

	//handshaking polling bit
	REG32(IOREG_IOBASE+0x00000180)= 0x00000000;
	rlx_irq_register(BSP_CMAC_IRQ, bsp_cmac_handler_sw_patch);
	//rlx_irq_set_handler(BSP_CMAC_IRQ, bsp_cmac_handler_sw);
	//rlx_irq_set_mask(BSP_CMAC_IRQ, OS_FALSE);

}

void bsp_cmac_send(INT8U *data, INT32U size)
{
	INT32U i = 0;
	INT8U *buf = NULL;
	
	txd_cmac=(CMACTXdesc *)(CMACTxdescStartAddr)+hwtxptr_cmac;

	while(txd_cmac->OWN==1)
	{
		i++;
		if(i == 100)
		{
			printf("bsp_cmac_send own bit not turn\n");
		}
	}
	
	//printk("bsp_cmac_send %x\n", data[4]);

	//memcpy((INT8U *)VA2PA(CMACTXBUFFER), data, size);
	txd_cmac->TAGC=0;
	txd_cmac->Length=size;
	txd_cmac->FS=1;
	txd_cmac->LS=1;
	txd_cmac->BufferAddress=(INT8U *)VA2PA(data);
	txd_cmac->OWN=1;
	REG8(CMAC_IOBASE+CMAC_OCR2)= (0x02|REG8(CMAC_IOBASE+CMAC_OCR2));
	hwtxptr_cmac=(hwtxptr_cmac+1)%CMACTxdescNumber;
}



