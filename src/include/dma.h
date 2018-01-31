#ifndef  _DMA_H_
#define  _DMA_H_

#include <rlx/rlx_types.h>
#include <bsp_cfg.h>
#include <rlx/rlx_cpu_regs.h>	


#define DMA 1
#define RISC_DIVIDE 1



#define SRC_GATHER_EN_POS   17
#define SRC_GATHER_SGI_POS  0
#define SRC_GATHER_SGC_POS  20


#define DST_SCATTER_EN_POS  18
#define DST_SCATTER_DSI_POS 0
#define DST_SCATTER_DSC_POS 20




/* Number of data items to be transferred of DMA_TR_WIDTH*/
enum DMA_BUS_TR_NUM {
	DMA_BUS_1,//one item
	DMA_BUS_4,// 4 items
	DMA_BUS_8,
	DMA_BUS_16,
	DMA_BUS_32,
	DMA_BUS_64,
	DMA_BUS_128,
	DMA_BUS_256,
};
/* AHB_MASTER */
enum DMA_AHB_MASTER {
	AHB_MASTER1,//AHB_MASTER1
	AHB_MASTER2,//AHB_MASTER2
	AHB_MASTER3,//AHB_MASTER3
	AHB_MASTER4 //AHB_MASTER4
};

// means DMA_TYPE_DWORD 
#define DMA_UNIT_SIZE	DMA_UNIT_DWORD
#define DMA_TYPE		DMA_TYPE_DWORD

enum DMA_UNIT
{
	DMA_UNIT_BYTE=1,//dma one byte one time
	DMA_UNIT_WORD=2,//dma two bytes one time
	DMA_UNIT_DWORD=4,//dma four bytes one time
	DMA_UNIT_DOUBLE_DWORD=8//dma eight bytes one time
};

enum DMA_TYPE {
	DMA_TYPE_BYTE=0,//dma one byte one time
	DMA_TYPE_WORD=1,//dma two bytes one time
	DMA_TYPE_DWORD=2,//dma four bytes one time
	DMA_TYPE_DOUBLE_DWORD=3//dma eight bytes one time
};

enum DMA_PROC {
	DMA_ONE_BLOCK,
	DMA_MULTI_BLOCKS
};

/* for dma controllor setting */
struct DMA_INFO
{
	INT8U *p_src;
	INT8U *p_des;
	enum DMA_TYPE dma_type;
	enum DMA_PROC dma_proc_type;
	INT32U count;
	INT32U interval;
	INT32U len;	
};

/* for multi block transfer */
struct DMA_MULTI_INFO
{
	INT32U len;// transfer lenth for one block
	INT32U inter_src;// source side transfer interval to next block
	INT32U inter_des;// destination side transfer interval to next block
};


struct DMA_LLI
{
	INT32U src_p;
	INT32U des_p;
	INT32U ctl_l;
	INT32U ctl_h;
	INT32U llp;
	INT32U llp_addr;
};

struct IMEM_R_DATA
{
	INT32U data_hi;
	INT32U data_low;
};

void set_dma_info(struct DMA_INFO *dma_p, \
						INT32U src, \
						INT32U des, \
						enum DMA_TYPE dma_type, \
						INT32U len, \
						INT32U count, \
						INT32U interval);

INT32U dma_channel0(struct DMA_INFO *dma_p);
INT32U dma_channel0_scatter(struct DMA_INFO *dma_p);
INT32U dma_channel0_gather(struct DMA_INFO *dma_p);
INT32U dma_channel0_lli(struct DMA_INFO *dma_p);
void dma_test(char val, struct DMA_INFO *dma_p);
void dma_large(INT32U dma_size,INT32U src, INT32U des, struct DMA_INFO *dma_p);
void dma_large_RKVM(INT32U dma_size,INT32U src, INT32U des, struct DMA_INFO *dma_p);

void dma_multi_block(INT32U src, INT32U des,struct DMA_MULTI_INFO *dma_mul_p, struct DMA_INFO *dma_p);

void dma_test_asic(struct DMA_INFO *dma_p);
void dma_test_asic_scatter(struct DMA_INFO *dma_p);
void dma_test_asic_gather(struct DMA_INFO *dma_p);
void dma_test_asic_large(struct DMA_INFO *dma_p);




void bsp_dma_handler(void);
void start_channel0(void);

void clear_test_timer(void);

void start_test_timer(void);
void stop_test_timer(void);

void dmac_test_gather(struct DMA_INFO *dma_p);
void dmac_test_gather_VGA(INT32U startaddr,INT32U destaddr,INT8U BPP,struct DMA_INFO *dma_p);


INT32U check_test_timer(void);
INT32U rand_gen(INT32U bottom, INT32U top);

#if 0
INT32U OOB_READ_IB(INT16U addr);
void OOB_WRITE_IB(INT16U addr,INT32U data);
#endif



#define ADDRESS_INV_ERR 1
#define LEN_INV_ERR		2
#define TYPE_INV_ERR    3


#define DMA_SRC 	0x80040001
#define DMA_DES   	0x800e0008
#define DMA_SRC_S 	0x80040001
#define DMA_DES_S 	0x80060003
#define DMA_SRC_G	0x88050000
#define DMA_DES_G 	0x80050000
#define DMA_SRC_L	0x80040000
#define DMA_DES_L 	0x80060000
#define DMA_SRC_H	0x80067a30
#define DMA_DES_H 	0x80066dd4
#define DMA_SRC_M	0x80040001
#define DMA_DES_M 	0x800e0002



#define DMA_SIZE		(16*1*4)
#define DMA_COUNT		16
#define DMA_INTERVAL	(0x10)


INT8U lli_struct_info[2048];

#define LLI_LOC_VMEM_BASE (&lli_struct_info) //store the DMA LLI descripter
#define LLI_LOC_PMEM_BASE VA2PA(LLI_LOC_VMEM_BASE) //store the DMA LLI descripter
#define LLI_INTERVAL	0x10


#define SMS_POS    		25
#define DMS_POS    		23
#define LLP_SRC_POS  	28
#define LLP_DES_POS 	27
#define SRC_MSIZE_POS 	14
#define DES_MSIZE_POS 	11
#define TT_FC_POS 		20

#define ONE_BLOCK_DMA   0
#define MULTI_BLOCK_DMA 1


#define IO_IB_ACC_DATA           0x00A0
#define IO_IB_ACC_SET            0x00A4

void uart_rx_en_not_pkg172(void);



#endif
