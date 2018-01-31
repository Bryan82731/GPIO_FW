#ifndef __VGA_H__
#define __VGA_H__
#include "bsp.h"

#define VGA 1
#define FT_EQC_TEST 1
#ifdef VGA_OOBMAC

#define DMA 				1  //define dma_info0
#endif

#define HerPixel 64					//Horizontal  pixel of each block
#define Line 64					//each block have 64 lines
#define pck_Dlen 0x578			//packet data len=1400
#define FBsize 0x1000			//uCOS dmem max size  4K


//VGA slave
#define VGA_IOBASE		0xbafb0000	// VGA Slave
#define WIFI_DASH_IOBASE 0xbafa0000
#define VGA_IMR			0x52C	// VGA Slave IMR
#define VGA_ISR			0x530	// VGA Slave ISR
#define STD_reg_ISR0		0x540     //VGA STD reg ISR0 0xFFFFFFFF
#define STD_reg_ISR1		0x544     //VGA STD reg ISR1 0xFFFFFFFF
#define STD_reg_ISR2		0x548     //VGA STD reg ISR2 0x0000FFFF
#define STD_reg_IMR0		0x534     //VGA STD reg IMR0 0xFFFFFFFF
#define STD_reg_IMR1		0x538     //VGA STD reg IMR1 0xFFFFFFFF
#define STD_reg_IMR2		0x53C     //VGA STD reg IMR2 0x0000FFFF
#define ROM_Base_Addr	0x568

//frame buffer maxima
#define VBlock_maxnum 	0x11
#define HBlock_maxnum 	0x1E
#define InBlock_maxnum 	0x40
#define VGA_ROM_SPI_base	0x02000000 //SW modify


//frame buffer register
//FB0 : legacy frame buffer
#define FB0_Base_Addr		0x558
#define FB0_resol		0x56C // bit[11:0] : hori-resolution , bit[23:12] : veri-resol,  bit[31:24] : byte per pixel
#define FB0_CTRL 		0x11C
#define FB0_VaddrBase 	0x120
#define FB0_HaddrBase 	0x164
#define FB0_LaddrBase	 0x1DC
#define FB0_BGN			0x2DC
#define FB0_BFN_base	0x2E0
#define FB0_BFN_timer	0x320
#define FB0_HBN_BPP		0x100
#define FB0_DDR_base	0x08000000 //SW modify
#define FB0_DDR_P_Addrbase	0x08000000
#define FB0_DMEM_P_Addrbase	0x00071A60
#define FB0_DDR_V_Addrbase	0x88000000

//FB1 : BAR frame buffer
#define FB1_Base_Addr		0x55c
#define FB1_resol		0x570 // bit[11:0] : hori-resolution , bit[23:12] : veri-resol,  bit[31:24] : byte per pixel
#define FB1_CTRL 		0x324
#define FB1_VaddrBase 	0x328
#define FB1_HaddrBase 	0x36C
#define FB1_LaddrBase	0x3E4
#define FB1_BGN			0x4E4
#define FB1_BFN_base	0x4E8
#define FB1_BFN_timer	0x528
#define FB1_HBN_BPP 	0x108
#define FB1_DDR_base	0x08000000 //SW modify
#define FB1_DDR_P_Addrbase	0x08000000
#define FB1_DMEM_P_Addrbase	0x00071A60
#define FB1_DDR_V_Addrbase	0x88000000
#define DDR_diff_Tx_flag	0x7F0000

#define VGA_dummy2 0x574
/*
VGA_dummy2 : 
bit[0] : set to test VGA 
bit[1] : VGA_ini ok flag
bit[2] : diff flag ckeck test flag
bit[19:16] : fb0/fb1block timer 
bit[31] : IB OK bit 

VGA_dummy2 : 
bit[0] :  VGA pre init OK
bit[1] : VGA_ini ok flag
bit[2] : diff flag check test flag(packet 4444 has been sent)
bit[19:16] : fb0/fb1block timer 
bit[30] : FT test enable bit 
bit[31]:dostool prepare frame buffer OK
*/
#define VGA_dummy3 0x578    //used for reporting OOBMAC RxTotal result to UGMAC when in combine test

volatile INT32U STD_counter;
volatile INT8U	VGA_flag;
volatile INT32U diff_timer_counter;
volatile INT32U black_screen_counter;

#if 0
//fun0 register
#define FUN0   0xBAF10000		//PCIE FUN0 
#define BMC_ISR  0x38
#define BMC_IMR 0x3A
#define BIT21   0x200000
#define BIT22   0x400000
#endif

void bsp_VGA_handler();
void bsp_VGA_PCIE_handler();
void VGA_initial();
void VGA_enable(void);
void VGA_pre_Initial();
//void VGA_test_ini();
void update_frame_buffer_fb1();
void vga_dma(void);
void VGA_Test_Task(void);
void test_flow_VGA(void);
void bsp_gmac_VGA_handler(void) ;
#if 0
void bsp_oobmac_init_VGA(void);
void bsp_ostimer_handler_VGA(void);
void bsp_timer_init_VGA(void);
void bsp_setup_VGA(void);
#endif
extern void bsp_oobmac_send_VGA(INT8U pktType,const INT32U start_addr,const INT8U pktnum,INT32U blocknum,INT8U BPP);

#endif
