#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <bsp_cfg.h>

/*#include <rlx/rlx_types.h>
#include <bsp_cfg.h>
#include <rlx/rlx_cpu_regs.h>
#include "bsp.h"
#include <string.h>
#include "uart.h"*/

#include "rkvm.h"
#include "OOBMAC.h"
//#include "OOBMAC_COMMON.h"



extern volatile INT8U	Cmac2Cont;
INT8U	FT_EQC_test_RKVM;
INT32U FT_RKVM_testNum=0;
INT8U  FT_RKVM_txFinish=0;

extern volatile INT8U	Cmac2Disable;

volatile INT32U Req_bufdata=0,Req_bufheader=0,return_bufdata=0,return_bufheader=0;
INT8U * RxdescStartAddr_CMAC2 = NULL;
INT8U * TxdescStartAddr_CMAC2 = NULL;

extern void bsp_wait(INT32U usec);
extern void bsp_KCS_RKVM_NCSI_shareHandler(void);

extern char char_temp[];

volatile INT8U	hwtxptr_cmac2 = 0x00;
volatile INT8U	hwrxptr_cmac2 = 0x00;
volatile INT8U	cmac2_DMAstatus = header0_fetch;
volatile INT8U	cmac2_DMAstatus_old;
volatile INT8U	cmac2_DMAData = 0;

#ifdef RKVM_REQUEST_ZERO_SIZE_TEST
volatile INT8U	cmac2_OMR = 0x0f;   //tok&tdu&rdu&rok
#else
volatile INT8U	cmac2_OMR = 0x0b;   //tdu&rdu&rok
#endif
//volatile INT8U	cmac2_OMR = 0x0f;
//volatile INT8U	cmac2_OMR = 0x03;
volatile INT8U	Cmac2IBRst=0;
volatile INT32U 	CMAC2_TOTAL_SEND = 0;
volatile INT32U 	CMAC2_TOK = 0;
volatile INT32U 	CMAC2_TDU = 0;
volatile INT32U 	CMAC2_ROK = 0;
volatile INT32U 	CMAC2_RDU = 0;
volatile INT32U 	CMAC2_ROK_INT = 0;

volatile INT32U 	debug_CMAC2_INT = 0;
volatile INT32U 	debug_Rxloop = 0;

volatile INT32U 	CMAC2_RXHeaderCnt = 0;
volatile INT32U 	CMAC2_DataError = 0;
volatile INT32U 	CMAC2_LenError = 0;
volatile INT32U 	CMAC2_Length0 = 0;
volatile INT32U 	CMAC2_Length1 = 0;
volatile INT32U 	CMAC2_Offset0 = 0;
volatile INT32U 	CMAC2_Offset1 = 0;
//INT32U 	CMAC2_Offset = 0;
volatile INT32U 	CMAC2_Zero = 0;
volatile INT32U 	CMAC2_TXDataCnt = 0;
volatile INT32U 	CMAC2_RXDataCnt = 0;
volatile RXdesc	*rxd_cmac2 = NULL;
volatile TXdesc 	*txd_cmac2 = NULL;
INT32U	cmac2_header_addr[2] = {CMAC2_HSAR1,CMAC2_HSAR2};
INT32U	cmac2_data_addr[2] = {CMAC2_DSAR1,CMAC2_DSAR2};
volatile INT32U 	CMAC2_WBPass = 1;
INT32U 	CMAC2_DelayCnt = 100;
INT32U 	CMAC2_HDFetchCnt = 10;
volatile INT32U debug_CMAC2_HDFetchCnt = 0;
//add for debug
volatile INT32U header_length_unmatch = 0;

void bsp_cmac2_FullRst(void)
{
	REG8(0xb2000150) |= 0x80;
	REG8(0xb2000150) &= ~0x80;
	bsp_wait(1000);
	REG8(CMAC2_IOBASE+CMAC2_OSR0)=REG8(CMAC2_IOBASE+CMAC2_OSR0);
}


void memset_self(INT8U *start,INT8U data,INT32U len)
{
	while(len--)
		*start++ = data;
}
void memcpy_self(INT8U *des,INT8U *src,INT32U len)
{
	while(len--)
		*des++ = *src++;
}

void bsp_cmac2_IBTXinit(void)
{
	INT32S	i;
	INT8U *tmp;
	//INT8U	*buf;
	//INT16U	tmp=0;

	TxdescStartAddr_CMAC2 = malloc(TxdescNumber_CMAC2*sizeof(struct _TxDesc));
	if(TxdescStartAddr_CMAC2==NULL){
		bsp_4everloop(0);
	}
	tmp = (INT8U*) TxdescStartAddr_CMAC2;
	memset(tmp, 0, sizeof(struct _TxDesc)*TxdescNumber_CMAC2);

	//Tx desciptor setup
	for(i=0;i<TxdescNumber_CMAC2;i++)
  	{
  		if(i ==(TxdescNumber_CMAC2-1))
  			REG32(TxdescStartAddr_CMAC2+i*16)=0x70000000;
  		else
  			REG32(TxdescStartAddr_CMAC2+i*16)=0x30000000;

		REG32(TxdescStartAddr_CMAC2+i*16+4)=0x0;
		REG32(TxdescStartAddr_CMAC2+i*16+8)=REG32(CMAC2_IOBASE+cmac2_data_addr[i%2]);
  		//zero copy, no need to set tx buffers
  		REG32(TxdescStartAddr_CMAC2+i*16+12)=REG32(CMAC2_IOBASE+cmac2_data_addr[i%2]+4);
  	}	
	REG32(CMAC2_IOBASE+CMAC2_TDSAR)=VA2PA(TxdescStartAddr_CMAC2);
	REG32(CMAC2_IOBASE+CMAC2_TDSAR+4)=0;
	
	//Interrupt Configuration
	REG8(CMAC2_IOBASE+CMAC2_IMR0)=0;
	REG8(CMAC2_IOBASE+CMAC2_ISR0)=REG8(CMAC2_IOBASE+CMAC2_ISR0);
	//REG8(CMAC2_IOBASE+CMAC2_IMR0)=0x0C;
	
	hwtxptr_cmac2=0;

	//max tx dma
	//REG8(CMAC2_IOBASE+CMAC2_ICR0)|=0x14;   
	REG8(CMAC2_IOBASE+CMAC2_ICR0)&=0xe3;   //add for tx DMA hang by melody 20160623  set 0x90_bit4:2=3'b001   set MRd length max=32byte
 //	REG8(CMAC2_IOBASE+CMAC2_ICR0)|=0x18;   //set MRd max = 1K  default  result: hang
 //	REG8(CMAC2_IOBASE+CMAC2_ICR0)|=0x14;   //set MRd max = 512 BYTE  result: hang
 	REG8(CMAC2_IOBASE+CMAC2_ICR0)|=0x10;   //set MRd max = 256 BYTE result: pass  set 0x90_bit4:2=3'b100   set MRd length max=256byte
 	//descriptor flag disable
	//REG8(CMAC2_IOBASE+CMAC2_ICR0)|=0x80;
	//Enable TX
	REG8(CMAC2_IOBASE+CMAC2_ICR0)|=0x01;
}

void bsp_cmac2_OOBRXinit(void)
{
	INT32S	i;
	INT8U *tmp;
	//INT8U	*buf;
	//INT16U	tmp=0;
	INT8U * temp;

	RxdescStartAddr_CMAC2 = malloc(RxdescNumber_CMAC2*sizeof(struct _RxDesc));
	if(RxdescStartAddr_CMAC2==NULL){
		bsp_4everloop(0);
	}
	tmp = (INT8U*) RxdescStartAddr_CMAC2;
	memset(tmp, 0, sizeof(struct _RxDesc)*RxdescNumber_CMAC2);
		
	//Rx descriptor setup
	for(i=0;i<RxdescNumber_CMAC2;i++)
  	{
  		if(i == (RxdescNumber_CMAC2 - 1))
  			REG32(RxdescStartAddr_CMAC2+i*16)=0xC0000000;
  		else
  			REG32(RxdescStartAddr_CMAC2+i*16)=0x80000000;
		
  		REG32(RxdescStartAddr_CMAC2+i*16+4)=0x0;
  		//REG32(RxdescStartAddr_CMAC2+i*16+8)=VA2PA(CMAC2RXBUFFER+(i*CMAC2BufferSize));
		//for buffer in ddr, use expression below
		temp = malloc(CMAC2BufferSize);
		if(temp==NULL){
			bsp_4everloop(0);
		}
		#ifdef RKVM_RX_ANY_BYTE_TEST
		REG32(RxdescStartAddr_CMAC2+i*16+8)=(VA2PA((INT32U)temp+rand()%4))&0x1fffffff;
		#else
		REG32(RxdescStartAddr_CMAC2+i*16+8)=(VA2PA((INT32U)temp))&0x1fffffff;
        #endif
  		REG32(RxdescStartAddr_CMAC2+i*16+12)=0x0;
		
  	}
	REG32(CMAC2_IOBASE+CMAC2_RDSAR)=VA2PA(RxdescStartAddr_CMAC2);
	REG32(CMAC2_IOBASE+CMAC2_RDSAR+4)=0;

	REG8(CMAC2_IOBASE+CMAC2_OMR0) = 0;
	REG8(CMAC2_IOBASE+CMAC2_OSR0) = REG8(CMAC2_IOBASE+CMAC2_OSR0);   //clear OSR
	REG8(CMAC2_IOBASE+CMAC2_OMR0) = cmac2_OMR;//rdu & rok & TDU
	hwrxptr_cmac2=0;
	
/*	#ifdef RKVM_REBOOT_TEST
	bsp_cmac2_EnablePCIERSTB();    //after here,it may be interrupted by interrupt again
	#endif*/ //move it to fun0's pciehost rising edge interrupt
	
	//max rx dma
	//REG8(CMAC2_IOBASE+CMAC2_OCR0)|=0x14;
	//multi-burst length any-byte alignment read select  || default 1: chusybp 0:pengkc
	REG8(CMAC2_IOBASE+CMAC2_MWBC) &= ~0x20;
	//Enable RX
	REG8(CMAC2_IOBASE+CMAC2_OCR0)|=0x01;
}

void bsp_cmac2_DisableTX(void)
{
	REG8(CMAC2_IOBASE+CMAC2_ICR0) &= ~0x01;
}

void bsp_cmac2_DisableRX(void)
{
	REG8(CMAC2_IOBASE+CMAC2_OCR0) &= ~0x01;
}

void bsp_cmac2_EnablePCIERSTB(void)
{
	cmac2_OMR |= 0x80;
	REG8(CMAC2_IOBASE+CMAC2_OMR0) = cmac2_OMR;
}

void bsp_cmac2_DisablePCIERSTB(void)
{
	REG8(CMAC2_IOBASE+CMAC2_OMR0) &= ~0x80;
}

void test_free_cmac2(void)
{
	TXdesc *txdesc;
	RXdesc *rxdesc;
	INT16U i;

	if(RxdescStartAddr_CMAC2!= NULL){
		free(RxdescStartAddr_CMAC2);
	}
	if(TxdescStartAddr_CMAC2!= NULL){
		free(TxdescStartAddr_CMAC2);
	}

	for(i=0;i<RxdescNumber_CMAC2;i++) 
	{
		rxdesc = ((RXdesc *)RxdescStartAddr_CMAC2)+i;
		if(PA2VA(rxdesc->BufferAddress) != NULL){
			free(PA2VA(rxdesc->BufferAddress));
		}
	}
	
}

void bsp_cmac2_Reinit(void)
{
	test_free_cmac2();
	
	#ifndef CONFIG_KCS_TEST_ENABLED    //default pciereset IMR is 1
	REG16(KCS_BASE + KCS_IMR)=0x0000;
	REG16(KCS_BASE + KCS_ISR)=REG16(KCS_BASE + KCS_ISR);
	#endif

	#ifndef CONFIG_TCR_TEST_ENABLED 	//default pciereset IMR is 1
	REG16(TCR_BASE_ADDR + TCR_IMR)=0x0000;
	REG16(TCR_BASE_ADDR + TCR_ISR)=REG16(TCR_BASE_ADDR + TCR_ISR);
	#endif
			
	#ifndef CONFIG_NCSI_TEST_ENABLED
	REG16(NCSI_IOBASE + MAC_IMR)=0x0000;
	REG16(NCSI_IOBASE + MAC_ISR)=REG16(NCSI_IOBASE + MAC_ISR);
	#endif
	
	bsp_cmac2_init();

	CMAC2_TOTAL_SEND = 0;
	CMAC2_TOK = 0;
	CMAC2_TDU = 0;
	CMAC2_ROK = 0;
	CMAC2_RDU = 0;
	CMAC2_DataError = 0;
	CMAC2_LenError = 0;
	CMAC2_TXDataCnt = 0;
	CMAC2_RXDataCnt = 0;	

	cmac2_DMAstatus = header0_fetch;
	cmac2_DMAData=0;

	CMAC2_WBPass=1;

}

void bsp_cmac2_init(void)
{
	//bsp_cmac2_FullRst();
	#if 0//for CMAC2 simulation
	//while((REG8(CMAC2_IOBASE+0x93)&0x20) != 0x20){}
	//while((REG8(CMAC2_IOBASE+0x91)&0x80) != 0x80){}
	REG32(CMAC2_IOBASE+cmac2_header_addr[0])=0x10000000;
	REG32(CMAC2_IOBASE+cmac2_header_addr[0]+4)=0;
	REG32(CMAC2_IOBASE+cmac2_header_addr[1])=0x18000000;
	REG32(CMAC2_IOBASE+cmac2_header_addr[1]+4)=0;

	REG32(CMAC2_IOBASE+cmac2_data_addr[0])=0x10000010;
	REG32(CMAC2_IOBASE+cmac2_data_addr[0]+4)=0;
	REG32(CMAC2_IOBASE+cmac2_data_addr[1])=0x18000010;
	REG32(CMAC2_IOBASE+cmac2_data_addr[1]+4)=0;
	#endif

	#if 0
	REG32(CMAC2_IOBASE+cmac2_header_addr[0])=0x00049b24;
	REG32(CMAC2_IOBASE+cmac2_header_addr[0]+4)=0;
	REG32(CMAC2_IOBASE+cmac2_header_addr[1])=0x0004a324;
	REG32(CMAC2_IOBASE+cmac2_header_addr[1]+4)=0;

	REG32(CMAC2_IOBASE+cmac2_data_addr[0])=0x00049344;
	REG32(CMAC2_IOBASE+cmac2_data_addr[0]+4)=0;
	REG32(CMAC2_IOBASE+cmac2_data_addr[1])=0x00049b44;
	REG32(CMAC2_IOBASE+cmac2_data_addr[1]+4)=0;
	#endif

	bsp_cmac2_DisableTX();
	bsp_cmac2_DisableRX();
	bsp_wait(1000);
	bsp_cmac2_IBTXinit();
	bsp_cmac2_OOBRXinit();
}

void bsp_cmac2_handler()
{
	volatile INT8U	val = 0;
	volatile INT8U	IMR_CMAC= 0;
	volatile INT8U	*rddataptr;
	volatile INT8U  start_data;
	volatile INT8U	i = 0;
	volatile INT32U	u = 0;
	volatile INT8U header_data[16];
	
	IMR_CMAC = REG8(CMAC2_IOBASE+CMAC2_OMR0);
	REG8(CMAC2_IOBASE+CMAC2_OMR0)=0;
	val=REG8(CMAC2_IOBASE+CMAC2_OSR0);	
	REG8(CMAC2_IOBASE+CMAC2_OSR0)=val;//write 1 to any bit in the ISR will reset that bit

//	debug_CMAC2_INT++;
	
	do
	{
		if((val&(DWBIT00|DWBIT01))!=0)//ROK & RDU
		{
			if(val&DWBIT01)//RDU
				CMAC2_RDU++;
			if(val&DWBIT00) //RxOK
			{
			//	CMAC2_ROK_INT++;
			
			//	debug_Rxloop=0;
				
				bsp_wait(20 * 3);
			
				for(i=0;i<RxdescNumber_CMAC2;i++)
				{
					rxd_cmac2=(RXdesc *)(RxdescStartAddr_CMAC2)+hwrxptr_cmac2;			
					
					if(rxd_cmac2->OWN==1)
					{
		      			break;
		      		}
					else
					{
						CMAC2_ROK++;
						rddataptr=(INT8U *)(PA2VA(rxd_cmac2->BufferAddress));
						//rddataptr=(INT8U *)(PA2VA(rxd_cmac2->BufferAddress)|0xA0000000);

						/*data check*/
						//if(rxd_cmac2->Length != 16 /*&&  rxd_cmac2->Length != 0*/)
						if(rxd_cmac2->PHY == 1)
						{
							if(rxd_cmac2->Length == 0)
							{
								REG8(CMAC2_IOBASE+CMAC2_IMR0) |= 0x01;
								CMAC2_Zero++;
								cmac2_DMAData = *rddataptr;
								while(1){}
								goto close_header;
							}
							
							CMAC2_RXDataCnt++;
							
						//	return_bufdata++;
							#ifdef RKVM_RX_DATA_CHECK
							
							/*check length*/  //it need to be modified melody20160623
							if((*rddataptr == 0? CMAC2_Length0: CMAC2_Length1) != rxd_cmac2->Length)
							{
								CMAC2_LenError++;
								while(1){}
							}

							/*check data*/
							/*
							0 -> frame data index
							1 -> frame length(low8bit)
							2 -> frame length()high8bit
							3 -> offset number
							4 -> start of incremental 
							*/
							//if( (INT8U)(*(rddataptr+1))+1 == (INT8U)(*(rddataptr+2)))
							start_data = *(rddataptr + 3) + 4;
							for(u=4;u<rxd_cmac2->Length;u++)
							{
								if(*(rddataptr+u)!= start_data++ )
								{	
										CMAC2_DataError++;
										
										REG16(OOBMAC_IOBASE + MAC_OOB2IB_INT) |= 0x1;  
									/*	uart_write_str("rkvm fail pos: \r\n");
										uart_write_str("0x");
										itoa_self(rddataptr,char_temp,16);//get src address
										uart_write_str(char_temp);
										uart_write_str("  pos  0x");
										itoa_self(u,char_temp,16);//get src address
										uart_write_str(char_temp);*/
										while(1){}
								}
							}
							#endif
						close_header:
						/*close header*/
							for(u=0;u<16;u++)
								header_data[u]=u;
							header_data[15] &= ~0x80;
							bsp_cmac2_close_header(*rddataptr,  0xFFFF, header_data);
							#ifdef RKVM_OOBMAC
							RKVM_OOBMAC_send(rddataptr,rxd_cmac2->Length);
							#endif
						}
						/*copy header*/
						//else if(rxd_cmac2->Length == 16)
						//header: byte0-1:length   byte2:Frame flag  byte15_bit8:valid flag
						else if(rxd_cmac2->PHY == 0)
						{
							//CMAC2_RXHeaderCnt++;
							memcpy(header_data, rddataptr, 16);
							if((header_data[15] & 0x80) == 0x80)
							{
								if(cmac2_DMAstatus == header0_fetch)
								{
									if(header_data[2] == 0)  //frame0
									{
										cmac2_DMAstatus = header0_complete;
										CMAC2_Length0 = *((INT16U*)header_data);							
									}
									else
									{	
									//	header_length_unmatch++;
									}
								}
								else if(cmac2_DMAstatus == header1_fetch)
								{
								//	cmac2_DMAstatus = header1_complete;
								//	CMAC2_Length1 = *((INT16U*)header_data);
									if(header_data[2] == 1)  //frame1 
									{
										cmac2_DMAstatus = header1_complete;
										CMAC2_Length1 = *((INT16U*)header_data);							
									}
									else
									{	
									//	header_length_unmatch++;  //it already happened
									}
								}
							//	return_bufheader++;
							}
						}
						/*else if(rxd_cmac2->Length == 0)
						{
							CMAC2_Zero++;
						}*/
						else
						{
							while(1){}						
						}
						/*clear buffer content*/
						//memset(rddataptr&0xfffffffc, 0, 0x600);
						memset((void *)((INT32U)rddataptr&0xfffffffc), 0, rxd_cmac2->Length);
						//memset_self(rddataptr,0,CMAC2BufferSize);
					}
					
				    /*Release RX descriptor*/
				    rxd_cmac2->OWN=1;
				    /*Update descriptor pointer*/
				    hwrxptr_cmac2=(hwrxptr_cmac2+1)%RxdescNumber_CMAC2;
				}
				
				debug_Rxloop = i;
			}
		}

		//PCIE ResetB
		
	/*	#ifdef RKVM_REBOOT_TEST
		if((val&DWBIT07)!=0)
		{
			IMR_CMAC = 0;
			Cmac2IBRst = 1;
			Cmac2Cont= 0;
			FT_EQC_test_RKVM=0;
		}
		#endif*/
		
		#ifdef RKVM_REQUEST_ZERO_SIZE_TEST
		if((val&DWBIT02)!=0)
		{
			CMAC2_TOK++;
		}
		#endif
		
		val=REG8(CMAC2_IOBASE+CMAC2_OSR0);
		REG8(CMAC2_IOBASE+CMAC2_OSR0)=val;//write 1 to any bit in the ISR will reset that bit
	}	
	//while((val&cmac2_OMR)!=0);
	while((val&(DWBIT00|DWBIT01))!=0);  //modified by melody:pciereset interrupt only do once 
	
//	REG8(CMAC2_IOBASE+CMAC2_OMR0)=IMR_CMAC;
	REG8(CMAC2_IOBASE+CMAC2_OMR0)=IMR_CMAC & (~DWBIT07);  //level sensitive;mask pcierst interrupt until pcierst high

}

void bsp_cmac2_txpolling(void)
{
	//clear TDU OSR before polling: TDU may affect DMA
/*	if(REG8(CMAC2_IOBASE+CMAC2_OSR0)&0x08) //TDU
	{
		REG8(CMAC2_IOBASE+CMAC2_OSR0) = 0x08;
	}*/
	REG8(CMAC2_IOBASE+CMAC2_ICR0)=(REG8(CMAC2_IOBASE+CMAC2_ICR0)|0x02);  //avoid polling being cleared which may stop DMA
	REG8(CMAC2_IOBASE+CMAC2_ICR0)=(REG8(CMAC2_IOBASE+CMAC2_ICR0)|0x02);  
//	while (txd_cmac2->OWN==1);
}

void bsp_cmac2_ReqIBData(INT32U size, INT32U BufferAddress, INT8U isFrameData)
{
	//INT32U i = 0;
	//INT8U *buf = NULL;
	
	txd_cmac2=(TXdesc *)(TxdescStartAddr_CMAC2)+hwtxptr_cmac2;

	if(txd_cmac2->OWN==1 && !isFrameData)   //for header
	{
		bsp_cmac2_txpolling();
	/*	delay_ms(1);
		REG16(OOBMAC_IOBASE + MAC_OOB2IB_INT) |= 0x1; //for trigger*/
		return;
	}
	if(isFrameData)  //for data
	{
		while(txd_cmac2->OWN==1)
		{
			bsp_cmac2_txpolling();
		//	OSTimeDly(1);
		/*	delay_ms(1);
			REG16(OOBMAC_IOBASE + MAC_OOB2IB_INT) |= 0x1; */
		}
	}
/*	if(txd_cmac2->OWN==1)
	{
		return;
	}*/
//	else
//	{
		txd_cmac2->BufferAddress = BufferAddress;	
		txd_cmac2->GTSENV4=isFrameData;
		txd_cmac2->TAGC=0;
		txd_cmac2->Length=size;
		txd_cmac2->FS=1;
		txd_cmac2->LS=1;
		txd_cmac2->OWN=1;

	/*	if(isFrameData)  //request for data
		{
			delay_ms(1);
		}*/
		
		bsp_cmac2_txpolling();  //every tx desc need more than one polling
	
		hwtxptr_cmac2=(hwtxptr_cmac2+1)%TxdescNumber_CMAC2;
/*		if(isFrameData)
		{
			Req_bufdata++;
		}
		else 
		{
			Req_bufheader++;
		}*/
		
//	}
	
	CMAC2_TOTAL_SEND++;
}

void bsp_cmac2_ReqHeader(INT32U size, INT8U num)
{
	bsp_cmac2_ReqIBData(size, REG32(CMAC2_IOBASE+cmac2_header_addr[num]), 0);
}

void bsp_cmac2_ReqFrameBuf(INT32U size, INT8U num)
{
	bsp_cmac2_ReqIBData(size, REG32(CMAC2_IOBASE+cmac2_data_addr[num]), 1);
}

void bsp_cmac2_close_header(INT32U number,  INT16U bit_mask, INT8U *header_data)
{
	INT32U i;

	/*copy header data*/
	for(i=0; i<16; i++)
		REG8(CMAC2_IOBASE+CMAC2_MWBDATA+i) = *(header_data+i);

	/*set bit mask*/
	REG16(CMAC2_IOBASE+CMAC2_MWBEN)=bit_mask;

	/*polling hw transfer complete*/
	if(number==0)
	{
		REG8(CMAC2_IOBASE+CMAC2_MWBC) |=0x80;
		while((REG8(CMAC2_IOBASE+CMAC2_MWBC)&0x80) == 0x80){}
	}
	else
	{
		REG8(CMAC2_IOBASE+CMAC2_MWBC) |=0x40;
		while((REG8(CMAC2_IOBASE+CMAC2_MWBC)&0x40) == 0x40){}
	}
	CMAC2_WBPass=1;
}

void bsp_cmac2_header_data_handshaking(void)
{

	/*for zero length test*/
	//bsp_cmac2_ReqFrameBuf(0, 0);
	//return;
	#ifdef RKVM_REQUEST_ZERO_SIZE_TEST
		bsp_cmac2_ReqIBData(0, REG32(CMAC2_IOBASE+CMAC2_HSAR1), 0);
		bsp_cmac2_ReqIBData(0, REG32(CMAC2_IOBASE+CMAC2_HSAR2), 0);
		bsp_cmac2_ReqIBData(0, REG32(CMAC2_IOBASE+CMAC2_DSAR1), 0);
		bsp_cmac2_ReqIBData(0, REG32(CMAC2_IOBASE+CMAC2_DSAR2), 0);
		return;
	#endif

/*	#ifdef RKVM_REBOOT_TEST
	OOBMAC_gpio_output(GPO,0);
	delay_ms(25*100);   //delay_ms(25)=1us
	OOBMAC_gpio_output(GPO,1);
	#endif
*/	
	#if defined(RKVM_REBOOT_TEST) || defined(RKVM_DISABLE_ENABLE_TEST)
	if(Cmac2IBRst == 1 || Cmac2Disable== 1)
	{	
		Cmac2IBRst=0;   //clear flag must come to the first
		Cmac2Disable=0;
		bsp_cmac2_Reinit();
		return;
	}
	#endif

	//bsp_wait(CMAC2_DelayCnt);
//	if(1)   //Cmac2Cont
	if(Cmac2Cont || FT_EQC_test_RKVM)
	{
		if(FT_RKVM_txFinish)
		{
			FT_EQC_test_RKVM=0;
			FT_RKVM_txFinish=0;
			bsp_wait(500 * 10);
			//REG32(OOBMAC_IOBASE + MAC_DUMMY_INFORM_IB_DATA) = CMAC2_RXDataCnt; 
			REG32(OOBMAC_IOBASE + MAC_DUMMY_INFORM_IB_EVENT) |= DWBIT01| DWBIT16;  //RKVM event & PASS
			if(CMAC2_DataError || (CMAC2_RXDataCnt!=FT_RKVM_testNum)){
				REG32(OOBMAC_IOBASE + MAC_DUMMY_INFORM_IB_EVENT) = REG32(OOBMAC_IOBASE + MAC_DUMMY_INFORM_IB_EVENT) & (~DWBIT16) | DWBIT17;  //set DATA err bit 
			}
			if(CMAC2_LenError){
				REG32(OOBMAC_IOBASE + MAC_DUMMY_INFORM_IB_EVENT) = REG32(OOBMAC_IOBASE + MAC_DUMMY_INFORM_IB_EVENT) & (~DWBIT16) | DWBIT19;  //set length err bit 
			}
			REG16(OOBMAC_IOBASE + MAC_OOB2IB_INT) |= 0x1;          //OOBMAC to IBMAC interrupt(ISR_BIT15): report result to driver
			return;
		}
		
		switch(cmac2_DMAstatus)
		{
			case header0_fetch:
				//bsp_cmac2_ReqHeader(16, 0);
				//cmac2_DMAstatus = header0_complete;
				//if(CMAC2_WBPass==0)
				//	return;
			//	if(CMAC2_HDFetchCnt < 10)
			//		CMAC2_HDFetchCnt++;
			//	else
			//	{
			//		CMAC2_HDFetchCnt=0;	
					OSTimeDly(1);
					bsp_cmac2_ReqHeader(16, 0);  //send header request periodly
					
					debug_CMAC2_HDFetchCnt++;
					
					#ifdef DYNAMIC_CPU_SLOW_RKVM
					bsp_wait(500 * 3);
					#endif
			//	}
				
				//CMAC2_HDFetchCnt=0;
				//bsp_cmac2_ReqHeader(16, 0);
				//bsp_wait(2000);
				//cmac2_DMAstatus=header0_complete;
				break;
				
			case header0_complete:
				cmac2_DMAstatus = FrameBuf0_fetch;
				
				break;

			case FrameBuf0_fetch:
				CMAC2_WBPass=0;
				cmac2_DMAData= 0;
				cmac2_DMAstatus_old = cmac2_DMAstatus;
				
				//if(CMAC2_Length0==0)
				//	while(1){}
				bsp_cmac2_ReqFrameBuf(CMAC2_Length0, 0);  //only send once
			//	delay_us(3*15);  //about 15us  cause DMA data need about 12us
				cmac2_DMAstatus = header1_fetch;
				CMAC2_TXDataCnt++;
				if(FT_EQC_test_RKVM && (CMAC2_TXDataCnt == FT_RKVM_testNum))
				{
					FT_RKVM_txFinish=1;
					return;
				}
				break;

			case header1_fetch:
				//if(CMAC2_WBPass==0)
				//	return;
			//	if(CMAC2_HDFetchCnt <10)
			//		CMAC2_HDFetchCnt++;
			//	else
			//	{
			//		CMAC2_HDFetchCnt=0;
					OSTimeDly(1);
					bsp_cmac2_ReqHeader(16, 1);
					
					debug_CMAC2_HDFetchCnt++;
					
					#ifdef DYNAMIC_CPU_SLOW_RKVM
					bsp_wait(500 * 3);
					#endif
			//	}
				//CMAC2_HDFetchCnt=0;
				//bsp_cmac2_ReqHeader(16, 1);
				//bsp_wait(2000);
				//cmac2_DMAstatus=header1_complete;
				break;

			case header1_complete:
				cmac2_DMAstatus = FrameBuf1_fetch;
				break;

			case FrameBuf1_fetch:
				CMAC2_WBPass=0;
				cmac2_DMAData= 1;
				cmac2_DMAstatus_old = cmac2_DMAstatus;
				
				//if(CMAC2_Length1==0)
				//	while(1){}
				bsp_cmac2_ReqFrameBuf(CMAC2_Length1, 1);
			//	delay_us(3*15);  //about 15us  cause DMA data need about 12us
				cmac2_DMAstatus = header0_fetch;
				CMAC2_TXDataCnt++;
				if(FT_EQC_test_RKVM && (CMAC2_TXDataCnt == FT_RKVM_testNum))
				{
					FT_RKVM_txFinish=1;
					return;
				}
				break;

			default:
				while(1){}
				break;		
		}

	}
	else{
		OSTimeDly(10);  //100ms
	}
	
}
void test_flow_CMAC2(void)
{
	bsp_cmac2_header_data_handshaking();
}

void RKVM_Test_Task(void)
{
	INT32U data_temp=0;

	//1.initial testItem
//	bsp_oobmac_init(); 
	//2.register interrupt
//	rlx_irq_register(BSP_GMAC_IRQ,bsp_oobmac_handler); 		//register OOBMAC interrupt 
//	rlx_irq_init();  // it is called in startTask at beginning;restore all intvet and enable interrupt
	
	//1.initial testItem
//	bsp_cmac2_init();  
	//2.register interrupt
//	rlx_irq_register(BSP_KCS_IRQ,bsp_KCS_RKVM_NCSI_shareHandler); 		//register OOBMAC interrupt 
//	rlx_irq_init();  // it is called in startTask at beginning;restore all intvet and enable interrupt

//	rlx_irq_init();	 //open interrupt IE
	
	//3.test loop	
    while(1)
	{
	//	test_flow_CMAC2();  
		bsp_cmac2_header_data_handshaking();
	//	OSTimeDly(100);   //100*10ms=1s  set timer interrupt 10ms
	}
}

