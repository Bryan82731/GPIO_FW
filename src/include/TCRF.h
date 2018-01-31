#ifndef __TCRF_H__
#define __TCRF_H__
#include <rlx/Rlx_types.h>
#include <rlx/rlx_cpu_regs.h>

#define THR_DESC_NUM 		2  //Max.=16
#define THR_DESC_LEN		8
#define THR_BUF_LEN			128

//TCR Register setting
//byte access
#define TCR_DLL					0x02
#define TCR_DLM					0x03
#define TCR_IER					0x04
#define TCR_FCR					0x06
#define TCR_MSR					0x0A
#define TCR_CONF0				0x10
#define TCR_THR_DESC_START		0x11
#define TCR_THR_PKT_RD			0x12
//0x11 - 0x20
#define TCR_RBR_DESC_START		0x13
#define TCR_TIMT				0x14
#define TCR_TIMPC				0x18
#define TCR_DESCADR				0x1C
//word access
#define TCR_THR_DATA_START		0x20
//0x30 - 0x130
#define TCR_RBR_DATA_START		0x40
#define TCR_TPT					0x60
#define TCR_RBR_IFG				0x68
#define TCR_IMR					0x6C
#define TCR_ISR					0x6E
#define SERIAL_RBR_TIMER		0x74

//TCR
//#define TCRDescAddr			(0x80040000 + 0X25c00)//1desc length=0x08
//#define TCRBuffAddr			(0x80040000 + 0x25d00)//1buffer length=0x80, any byte test



void bsp_tcrfp_handler(void);
void bsp_tcrfp_init(void);
void bsp_tcr_send(INT16S length, INT8U *ptr);
void bsp_tcr_disable();
INT8U tcr_read_thr(void);



typedef struct _TCRTxDesc
{
    INT32U	Length:11;
    INT32U	RESV:19;
    INT32U 	EOR:1;
    INT32U	OWN:1;
	
    INT32U 	Bufferaddr;
} TCRTxDesc;







#define TCR_IOBASE		0xbaf50000	// TCR0  Configuration


















#endif

