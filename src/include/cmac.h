typedef struct _CMACRxDesc{
INT32U		Length:14;
INT32U		TCPF:1;
INT32U		UDPF:1;
INT32U		IPF:1;
INT32U		PKTFNO:4;
INT32U		TCPT:1;
INT32U		UDPT:1;
INT32U		V4F:1;
INT32U		V6F:1;
INT32U		BRO:1;
INT32U		PHY:1;
INT32U		MAR:1;
INT32U		LS:1;
INT32U		FS:1;
INT32U		EOR:1;
INT32U		OWN:1;
//----------
INT32U		VLanTag:16;
INT32U		TAVA:1;
INT32U		RSVD:14;
INT32U		GAME:1;
//----------
INT32U		BufferAddress;
INT32U		BufferAddressHigh;
}volatile CMACRXdesc;//volatile
//--------------------
typedef struct _CMACTxDesc{
INT32U		Length:16;
INT32U		RSVD0:9;
INT32U		GTSENV6:1;
INT32U		GTSENV4:1;
INT32U		RSVD1:1;
INT32U		LS:1;
INT32U		FS:1;
INT32U		EOR:1;
INT32U		OWN:1;
//----------
INT32U		VLanTag:16;
INT32U		LGSEN:1;
INT32U		TAGC:1;
INT32U		TCPHO:10;
INT32U		V6F:1;
INT32U		IPV4CS:1;
INT32U		TCPCS:1;
INT32U		UDPCS:1;
//----------
INT32U		BufferAddress;
INT32U		BufferAddressHigh;
}volatile CMACTXdesc;//volatile

#define DWBIT00 	0x00000001
#define DWBIT01 	0x00000002
#define DWBIT02 	0x00000004
#define DWBIT03 	0x00000008
#define DWBIT04 	0x00000010
#define DWBIT05 	0x00000020
#define DWBIT06 	0x00000040
#define DWBIT07 	0x00000080
#define DWBIT08 	0x00000100
#define DWBIT09 	0x00000200
#define DWBIT10 	0x00000400
#define DWBIT11 	0x00000800
#define DWBIT12 	0x00001000
#define DWBIT13 	0x00002000
#define DWBIT14 	0x00004000
#define DWBIT15 	0x00008000
#define DWBIT16 	0x00010000
#define DWBIT17 	0x00020000
#define DWBIT18 	0x00040000
#define DWBIT19 	0x00080000
#define DWBIT20 	0x00100000
#define DWBIT21 	0x00200000
#define DWBIT22 	0x00400000
#define DWBIT23 	0x00800000
#define DWBIT24 	0x01000000
#define DWBIT25 	0x02000000
#define DWBIT26 	0x04000000
#define DWBIT27 	0x08000000
#define DWBIT28 	0x10000000
#define DWBIT29 	0x20000000
#define DWBIT30 	0x40000000
#define DWBIT31 	0x80000000

/* CMAC controller */
#define CMAC_IOBASE			0xBAF20000
#define CMAC_OCR0			0x0000
#define CMAC_OCR1			0x0001
#define CMAC_OCR2			0x0002
#define CMAC_OCR3			0x0003
#define CMAC_ISR			0x000E
#define CMAC_IMR			0x000C
#define CMAC_IMR1			0x000D
#define CMAC_RDSAR			0x0004
#define CMAC_TNPDS			0x0008

#define CMACRxdescNumber		4 //Max.=16
#define CMACTxdescNumber		4 //Max.=16

#define CMAC_simulation_case1	1//CMAC RX check incremental data
#define CMAC_simulation_case2	0//CMAC TX send incremental data
#define CMAC_simulation_case3	0//CMAC TX send back data from CMAC RX

#define handshaking_chk		0

//test setting
#define CMAC_TEST 1
#define CMAC_CKECK 0
#define CMAC_RX2TX 1

#define CMAC_SERIAL
//#define CMAC_DISABLE_ENABLE

//for test
void bsp_null_handler();
//for test

void bsp_cmac_handler(void);
void bsp_cmac_init(void);
void CMAC_SW_release();
void bsp_cmac_send(INT8U *data, INT32U size);
void bsp_cmac_test_SerierData(INT32U size);
void bsp_cma_TestRegisterRW(void);
void bsp_cmac_handler_sw_patch();
void bsp_cmac_reset();


void bsp_cmac_handler_test(void);
void bsp_cmac_init_test(void);
void bsp_cmac_send_test(INT8U *data, INT32U size);
void CMAC_Test_Task(void);
void bsp_cmac_SerierData_test(INT8U data, INT32U size);



