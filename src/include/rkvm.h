#ifndef __CMAC2_H__
#define __CMAC2_H__

/* 
definition of RKVM test

*/
#define RKVM 1
#define RKVM_REBOOT_TEST
#define RKVM_DISABLE_ENABLE_TEST

//#define RKVM_RX_DATA_CHECK
//#define RKVM_REQUEST_ZERO_SIZE_TEST
//#define RKVM_RX_ANY_BYTE_TEST
//4.CPU dynamic down speed test Engine DMA of RKVM
//#define DYNAMIC_CPU_SLOW_RKVM

#ifdef DYNAMIC_CPU_SLOW_RKVM
#define DYNAMIC_CPU_SLOW
#define RKVM_RX_DATA_CHECK
#endif

#ifdef RKVM_OOBMAC
//#define RKVM 				1
//#define KCS_TEST            1
//#define GMAC_TEST           1
#define DMA 				1  //define dma_info0

#endif


/* CMAC2 controller */
#define CMAC2_IOBASE	0xBAF80400
#define CMAC2_HSAR1		0x00
#define CMAC2_HSAR2		0x08
#define CMAC2_DSAR1		0x10
#define CMAC2_DSAR2		0x18
#define CMAC2_DUMMY1	0x20
#define CMAC2_DUMMY2	0x24
#define CMAC2_TDSAR		0x80
#define CMAC2_RDSAR		0x88
#define CMAC2_ICR0		0x90
#define CMAC2_IMR0		0x92
#define CMAC2_ISR0		0x93
#define CMAC2_OCR0		0xA0
#define CMAC2_OMR0		0xA2
#define CMAC2_OSR0		0xA3
#define CMAC2_MWBDATA	0xB0
#define CMAC2_MWBEN		0xC0
#define CMAC2_MWBC		0xC2

#define RxdescNumber_CMAC2		4 //Max.=16
#define TxdescNumber_CMAC2		4 //Max.=16
#define CMAC2BufferSize			0xc18   //max length:3096? why not 3k=3072

typedef enum{
	header0_fetch = 0,
	header0_complete,	
	FrameBuf0_fetch,
	header1_fetch,
	header1_complete,	
	FrameBuf1_fetch,		
}CMAC2_DMA_STATUS_et;

void bsp_cmac2_init(void);
void bsp_cmac2_IBTXinit(void);
void bsp_cmac2_OOBRXinit(void);
void bsp_cmac2_handler();
void bsp_cmac2_req_data(INT32U size);
void bsp_cmac2_EnablePCIERSTB(void);
void bsp_cmac2_DisablePCIERSTB(void);
void bsp_cmac2_close_header(INT32U number,  INT16U bit_mask, INT8U *header_data);
void bsp_cmac2_header_data_handshaking(void);
extern void RKVM_OOBMAC_send(INT8U *payload_ptr,volatile INT16U length_payload);
#endif

