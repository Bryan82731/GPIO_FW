/* file for DMA, CPU sleep, CPU&BUS frequency ratio change */
/* 8111FP Project */
/* created by leon, Realsil, 2015-5-6 */
/* leon_zou@realsil.com.cn */

#include <rlx/rlx_cpu_regs.h>	
#include <bsp_cfg.h>
#include <bsp.h>
#include <string.h>
#include <stdlib.h>
#include <rlx/rlx_types.h>
#include "dma.h"
#include "uart.h"

#include "OOBMAC.h"
#include "OOBMAC_COMMON.h"
#include "vga.h"
#include "vnc.h"


volatile INT8U 	dma_flag;
INT8U  	clk_divide_val;
volatile INT32U check_timer_value;

extern char char_temp[];//char buffer for UART putout

volatile int dma_int_cnt=0,call_dmac_cnt=0;

#if DMA
volatile struct DMA_INFO dma_info0;
volatile struct DMA_LLI  dma_lli0;
struct DMA_MULTI_INFO dma_mul_info[4]={{2000,100,3},{2003,7,2},{8192,17,152},{50*1024,16,176}};
#endif

extern struct VgaInfo VGAInfo;


extern void rlx_dcache_wbinvalidate_all(void);


/* function set DMAC parameters */

void set_dma_info(struct DMA_INFO *dma_p, \
						INT32U src, \
						INT32U des, \
						enum DMA_TYPE dma_type, \
						INT32U len, \
						INT32U count, \
						INT32U interval)
{
	dma_p->p_src 	= (INT8U *)src;
	dma_p->p_des 	= (INT8U *)des;
	dma_p->dma_type 	= dma_type;
	dma_p->len 		= len;
	dma_p->count 	= count;
	dma_p->interval 	= interval;
}


/* DMA function, for channel0 
*   parameters:
*	INT32U src: 
*			source dma address, should be physical address only
*	INT32U des: 
*			destination dma address, should be physical address only
*	enum DMA_TYPE dma_type: dma type, can be one of 
*							DMA_TYPE_BYTE,//dma one byte one time
*							DMA_TYPE_WORD,//dma two bytes one time
*							DMA_TYPE_DWORD,//dma four bytes one time
*							DMA_TYPE_DOUBLE_DWORD//dma eight bytes one time
*
*	INT32U len:  dma lenth, should be less than 4096
*/
INT32U dma_channel0(struct DMA_INFO *dma_p)
{
	INT32U temp = 0;
	//if(((src&(0x3))!=0)&&((des&(0x3))!=0))// four bytes alignment
	//	return ADDRESS_INV_ERR;
	if((dma_p->len)>4095)
		return LEN_INV_ERR;// lenth check
	
	REG32(DMA_DmaCfgReg) = 1;//enable DMA then REGs can be write into
	REG32(DMA_ChEnReg)   = 0xff00;//bit8 is Channel0 Write enable, bit0 is Channel0 enable, here disable Channel0
									//disable all channel
	REG32(DMA_ClearSrcTran) = 1;
	REG32(DMA_ClearTfr) = 1;
	REG32(DMA_ClearBlock) = 1;
	REG32(DMA_ClearDstTran) = 1;
	REG32(DMA_ClearErr) = 1;		//CLEAR ALL INTERRUPTS


	switch(dma_p->dma_type)
	{
		case DMA_TYPE_BYTE:
			/* set source Transfer width 8bit, destination trasnfer Width 8bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_BYTE<<1)|((INT8U)DMA_TYPE_BYTE<<4)|(0x1<<0);
			break;
		case DMA_TYPE_WORD:
			/* set source Transfer width 16bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_WORD<<1)|((INT8U)DMA_TYPE_WORD<<4)|(0x1<<0);
			break;
		case DMA_TYPE_DWORD:
			/* set source Transfer width 16bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_DWORD<<1)|((INT8U)DMA_TYPE_DWORD<<4)|(0x1<<0);
			break;
		case DMA_TYPE_DOUBLE_DWORD:
			/* set source Transfer width 16bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_DOUBLE_DWORD<<1)|((INT8U)DMA_TYPE_DOUBLE_DWORD<<4)|(0x1<<0);
			break;
		default:
			/* DMA type invalid */
			REG32(DMA_DmaCfgReg) = 0;//disable DMA
			return TYPE_INV_ERR;
			break;
	}

	/* SMS AHB Master 1 , select DMS AHB Master 2, SRC_msize = 4 byte, DES_msize = 4 byte, mem2mem DMA */
	temp |= (AHB_MASTER1<<SMS_POS)|(AHB_MASTER2<<DMS_POS)|(DMA_BUS_1<<SRC_MSIZE_POS)|(DMA_BUS_1<<DES_MSIZE_POS)|(0<<TT_FC_POS);

	REG32(DMA_CTL0+4) 	 = dma_p->len;// set trasnfer lenth in one block
	REG32(DMA_SAR0) 	 = VA2PA((INT32U)dma_p->p_src)&0x1fffffff;//set source address
	REG32(DMA_DAR0) 	 = VA2PA((INT32U)dma_p->p_des)&0x1fffffff;//set destination address

	REG32(DMA_CTL0) 	 = temp;// set trasnfer lenth in one block
	REG32(DMA_MaskTfr)   = 0x0101;//unmask channel0 for Transfer
	REG32(DMA_MaskBlock) = 0x0100;//mask channel0 for Block Transfer
	
	REG32(DMA_DmaCfgReg) = 1;//enable DMA
	REG32(DMA_ChEnReg)   = 0x0101;//enable Channel0

	return 0;
}

/* DMA scatter function, for channel0 
*   parameters:
*	INT32U src: 
*			source dma address, should be physical address only
*	INT32U des: 
*			destination dma address, should be physical address only
*	enum DMA_TYPE dma_type: dma type, can be one of 
*							DMA_TYPE_BYTE,//dma one byte one time
*							DMA_TYPE_WORD,//dma two bytes one time
*							DMA_TYPE_DWORD,//dma four bytes one time
*							DMA_TYPE_DOUBLE_DWORD//dma eight bytes one time
*
*	INT32U len:  dma lenth, should be less than 4096
*     INT32U count : dma count in scatter mdoe one time 
*     INT32U interval :dma interval in scatter mdoe
*/

INT32U dma_channel0_scatter(struct DMA_INFO *dma_p)
{
	INT32U temp = 0;
	//if(((src&(0x3))!=0)&&((des&(0x3))!=0))// four bytes alignment
	//	return ADDRESS_INV_ERR;
	if((dma_p->len)>4095)
		return LEN_INV_ERR;// lenth check
	
	REG32(DMA_DmaCfgReg) = 1;//enable DMA then REGs can be write into
	REG32(DMA_ChEnReg)   = 0xff00;//bit8 is Channel0 Write enable, bit0 is Channel0 enable, here disable Channel0
									//disable all channel
	REG32(DMA_ClearSrcTran) = 1;
	REG32(DMA_ClearTfr) = 1;
	REG32(DMA_ClearBlock) = 1;
	REG32(DMA_ClearDstTran) = 1;
	REG32(DMA_ClearErr) = 1;		//CLEAR ALL INTERRUPTS
	
	switch(dma_p->dma_type)
	{
		case DMA_TYPE_BYTE:
			/* set source Transfer width 8bit, destination trasnfer Width 8bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_BYTE<<1)|((INT8U)DMA_TYPE_BYTE<<4)|(0x1<<0);
			break;
		case DMA_TYPE_WORD:
			/* set source Transfer width 16bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_WORD<<1)|((INT8U)DMA_TYPE_WORD<<4)|(0x1<<0);
			break;
		case DMA_TYPE_DWORD:
			/* set source Transfer width 16bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_DWORD<<1)|((INT8U)DMA_TYPE_DWORD<<4)|(0x1<<0);
			break;
		case DMA_TYPE_DOUBLE_DWORD:
			/* set source Transfer width 16bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_DOUBLE_DWORD<<1)|((INT8U)DMA_TYPE_DOUBLE_DWORD<<4)|(0x1<<0);
			break;
		default:
			/* DMA type invalid */
			REG32(DMA_DmaCfgReg) = 0;//disable DMA
			return TYPE_INV_ERR;
			break;
	}	

	/* SMS AHB Master 1 , select DMS AHB Master 2, SRC_msize = 1 byte, DES_msize = 1 byte, mem2mem DMA */
	temp |= (AHB_MASTER1<<SMS_POS)|(AHB_MASTER2<<DMS_POS)|(DMA_BUS_4<<SRC_MSIZE_POS)|(DMA_BUS_4<<DES_MSIZE_POS)|(0<<TT_FC_POS);
	temp |= (1<<DST_SCATTER_EN_POS);
	REG32(DMA_CTL0+4) 	 = dma_p->len;// set trasnfer lenth in one block
	REG32(DMA_SAR0) 	 = VA2PA((INT32U)dma_p->p_src)&0x1fffffff;//set source address
	REG32(DMA_DAR0) 	 = VA2PA((INT32U)dma_p->p_des)&0x1fffffff;//set destination address

	
	//enable Dstination Scatter, Scatter transfer type
	REG32(DMA_DSR0)      = ((dma_p->count)<<DST_SCATTER_DSC_POS)|((dma_p->interval)<<DST_SCATTER_DSI_POS);
	REG32(DMA_CTL0)      = temp;
	
	
	REG32(DMA_MaskTfr)   = 0x0101;//unmask channel0 for Transfer
	REG32(DMA_MaskBlock) = 0x0100;//mask channel0 for Block Transfer
	
	REG32(DMA_DmaCfgReg) = 1;//enable DMA
	REG32(DMA_ChEnReg)   = 0x0101;//enable Channel0

	return 0;
}

/* DMA gather function, for channel0 
*   parameters:
*	INT32U src: 
*			source dma address, should be physical address only
*	INT32U des: 
*			destination dma address, should be physical address only
*	enum DMA_TYPE dma_type: dma type, can be one of 
*							DMA_TYPE_BYTE,//dma one byte one time
*							DMA_TYPE_WORD,//dma two bytes one time
*							DMA_TYPE_DWORD,//dma four bytes one time
*							DMA_TYPE_DOUBLE_DWORD//dma eight bytes one time
*
*	INT32U len:  dma lenth, should be less than 4096
*     INT32U count : dma count in scatter mdoe one time 
*     INT32U interval :dma interval in scatter mdoe
*/

INT32U dma_channel0_gather(struct DMA_INFO *dma_p)
{
	INT32U temp = 0;
	//if(((src&(0x3))!=0)&&((des&(0x3))!=0))// four bytes alignment
	//	return ADDRESS_INV_ERR;
	if((dma_p->len)>4095)
		return LEN_INV_ERR;// lenth check
	
	REG32(DMA_DmaCfgReg) = 1;//enable DMA then REGs can be write into
	REG32(DMA_ChEnReg)   = 0xff00;//bit8 is Channel0 Write enable, bit0 is Channel0 enable, here disable Channel0
									//disable all channel
	REG32(DMA_ClearSrcTran) = 1;
	REG32(DMA_ClearTfr) = 1;
	REG32(DMA_ClearBlock) = 1;
	REG32(DMA_ClearDstTran) = 1;
	REG32(DMA_ClearErr) = 1;		//CLEAR ALL INTERRUPTS

	
	switch(dma_p->dma_type)
	{
		case DMA_TYPE_BYTE:
			/* set source Transfer width 8bit, destination trasnfer Width 8bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_BYTE<<1)|((INT8U)DMA_TYPE_BYTE<<4)|(0x1<<0);
			break;
		case DMA_TYPE_WORD:
			/* set source Transfer width 16bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_WORD<<1)|((INT8U)DMA_TYPE_WORD<<4)|(0x1<<0);
			break;
		case DMA_TYPE_DWORD:
			/* set source Transfer width 16bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_DWORD<<1)|((INT8U)DMA_TYPE_DWORD<<4)|(0x1<<0);
			break;
		case DMA_TYPE_DOUBLE_DWORD:
			/* set source Transfer width 16bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_DOUBLE_DWORD<<1)|((INT8U)DMA_TYPE_DOUBLE_DWORD<<4)|(0x1<<0);
			break;
		default:
			/* DMA type invalid */
			REG32(DMA_DmaCfgReg) = 0;//disable DMA
			return TYPE_INV_ERR;
			break;
	}	
	
	/* SMS AHB Master 1 , select DMS AHB Master 2, SRC_msize = 4 byte, DES_msize = 4byte, mem2mem DMA */
	temp |= (AHB_MASTER1<<SMS_POS)|(AHB_MASTER2<<DMS_POS)|(DMA_BUS_4<<SRC_MSIZE_POS)|(DMA_BUS_4<<DES_MSIZE_POS)|(0<<TT_FC_POS);
	temp |= (1<<SRC_GATHER_EN_POS);
	//enable Source gather, Counter and interval  are unit in transfer width
	REG32(DMA_SGR0)      = ((dma_p->count)<<SRC_GATHER_SGC_POS)|((dma_p->interval)<<SRC_GATHER_SGI_POS);
	REG32(DMA_CTL0+4) 	 = dma_p->len;// set trasnfer lenth in one block
	REG32(DMA_SAR0) 	 = VA2PA((INT32U)dma_p->p_src)&0x1fffffff;//set source address
	REG32(DMA_DAR0) 	 = VA2PA((INT32U)dma_p->p_des)&0x1fffffff;//set destination address
	REG32(DMA_CTL0)	     = temp;  // set CTL0

	
	REG32(DMA_MaskTfr)   = 0x0101;//unmask channel0 for Transfer
	REG32(DMA_MaskBlock) = 0x0100;//mask channel0 for Block Transfer
	
	REG32(DMA_DmaCfgReg) = 1;//enable DMA
	REG32(DMA_ChEnReg)   = 0x0101;//enable Channel0

	return 0;
}



/* DMA Link list Item function, for channel0 
*   parameters:
*	INT32U src: 
*			source dma address, should be physical address only
*	INT32U des: 
*			destination dma address, should be physical address only
*	enum DMA_TYPE dma_type: dma type, can be one of 
*							DMA_TYPE_BYTE,//dma one byte one time
*							DMA_TYPE_WORD,//dma two bytes one time
*							DMA_TYPE_DWORD,//dma four bytes one time
*							DMA_TYPE_DOUBLE_DWORD//dma eight bytes one time
*
*	INT32U len:  dma lenth, should be less than 4096
*/


INT32U dma_channel0_lli(struct DMA_INFO *dma_p)
{
	INT32U temp = 0;
	//if(((src&(0x3))!=0)&&((des&(0x3))!=0))// four bytes alignment
	//	return ADDRESS_INV_ERR;
	if((dma_p->len)>4095)
		return LEN_INV_ERR;// lenth check
	
	REG32(DMA_DmaCfgReg) = 1;//enable DMA then REGs can be write into
	REG32(DMA_ChEnReg)   = 0xff00;//bit8 is Channel0 Write enable, bit0 is Channel0 enable, here disable Channel0
									//disable all channel
	REG32(DMA_ClearSrcTran) = 0;
	REG32(DMA_ClearTfr) = 0;
	REG32(DMA_ClearBlock) = 0;
	REG32(DMA_ClearDstTran) = 0;
	REG32(DMA_ClearErr) = 0;		//CLEAR ALL INTERRUPTS

  	
	switch(dma_p->dma_type)
	{
		case DMA_TYPE_BYTE:
			/* set source Transfer width 8bit, destination trasnfer Width 8bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_BYTE<<1)|((INT8U)DMA_TYPE_BYTE<<4)|(0x1<<0);
			break;
		case DMA_TYPE_WORD:
			/* set source Transfer width 16bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_WORD<<1)|((INT8U)DMA_TYPE_WORD<<4)|(0x1<<0);
			break;
		case DMA_TYPE_DWORD:
			/* set source Transfer width 32bit, destination trasnfer Width 16bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_DWORD<<1)|((INT8U)DMA_TYPE_DWORD<<4)|(0x1<<0);
			break;
		case DMA_TYPE_DOUBLE_DWORD:
			/* set source Transfer width 64bit, destination trasnfer Width 64bit, enable trasnfer interrupt */
			temp = ((INT8U)DMA_TYPE_DOUBLE_DWORD<<1)|((INT8U)DMA_TYPE_DOUBLE_DWORD<<4)|(0x1<<0);
			break;
		default:
			/* DMA type invalid */
			REG32(DMA_DmaCfgReg) = 0;//disable DMA
			return TYPE_INV_ERR;
			break;
	}	
	
	
	REG32(DMA_CTL0+4) 	 = dma_p->len;// set trasnfer lenth in one block
	REG32(DMA_SAR0) 	 = VA2PA((INT32U)dma_p->p_src)&0x1fffffff;//set source address
	REG32(DMA_DAR0) 	 = VA2PA((INT32U)dma_p->p_des)&0x1fffffff;//set destination address


	/* SMS AHB Master 1 , select DMS AHB Master 2, SRC_msize = 4 byte, DES_msize = 4 byte, mem2mem DMA */
	temp |= (AHB_MASTER1<<SMS_POS)|(AHB_MASTER2<<DMS_POS)|(DMA_BUS_4<<SRC_MSIZE_POS)|(DMA_BUS_4<<DES_MSIZE_POS)|(0<<TT_FC_POS);
	/* select LLP SRC enable,  LLP DES enable*/
	temp |= (1<<LLP_SRC_POS)|(1<<LLP_DES_POS);
	/* set LLP descripter physical address, select AHB bus master1*/
	REG32(DMA_LLP_LOC)   = (LLI_LOC_PMEM_BASE&0xfffffffc)|(AHB_MASTER1);
	
	REG32(DMA_CTL0)      = temp;
	REG32(DMA_MaskTfr)   = 0x0101;//unmask channel0 for Transfer
	REG32(DMA_MaskBlock) = 0x0100;//unmask channel0 for Block Transfer
	
	REG32(DMA_DmaCfgReg) = 1;//enable DMA
	REG32(DMA_ChEnReg)   = 0x0101;//enable Channel0

	return 0;
}


#define TEST_LEN ((dma_p->len)<<((INT8U)dma_info0.dma_type))
#define TEST_COUNT (>>((INT8U)dma_info0.dma_type))


//2K buffer to store Link list struct information
INT8U lli_struct_info[2048];

/* functon do the any bytes DMA, for size large tha 4095 bytes, it will do the LLI action
*   else a normal one block DMA will be performed with DMA transfer type BYTE
*   the LLI action will calculate the num of upper size, which beyond 4K bytes, and 
*   set DMA transfer type Double DWORD for this part.
*   Parameters :
*		dma_size:  total DMA transfer bytes
*		src:           DMA source address
*		des:          DMA destination address
*		dma_p:      DMA_INFO struct
*   Returns:
*            LEN_INV_ERR: dma lenth error mseeage
*/
INT32U set_dma_large_info(INT32U dma_size,INT32U src, INT32U des, struct DMA_INFO *dma_p)
{
	INT32U count,less,upp,upp_4k,less_4k,temp;
	less = dma_size & 0xfff;//size below 4096 bytes
	upp  = dma_size & (~0xfff);//size large than 4096 bytes
	upp_4k   = (upp>>3)/4095;//numbers of 4095*8 Bytes
	less_4k  = (upp>>3)%4095;//less numbers of 4095*8 Bytes
	
	if(upp == 0)// means DMA len is less than 4096 bytes, use normal DMA, no LLI
	{
		if(less == 0) // MEANS dma lenth = 0 , error
			return LEN_INV_ERR;
		dma_p->dma_type = DMA_TYPE_BYTE;//one block transfer is OK, DMA type is BYTE type
		dma_p->len = less;
		dma_p->p_src = (INT8U *)src;
		dma_p->p_des = (INT8U *)des;
		dma_p->dma_proc_type = ONE_BLOCK_DMA;
	}
	else if((upp_4k == 0)&&(less == 0))// for 1k,2k,3k,4k,5k,6k,7k bytes, no need for multiblock transfer
	{
		dma_p->dma_type = DMA_TYPE_DOUBLE_DWORD;//one block transfer is OK, DMA type is BYTE type
		dma_p->len = less_4k;
		dma_p->p_src = (INT8U *)src;
		dma_p->p_des = (INT8U *)des;
		dma_p->dma_proc_type = ONE_BLOCK_DMA;
	}
	else if((less_4k == 0)&&(less == 0)&&(upp_4k == 1))// for n*4095*4K bytes, no need for multiblock transfer
	{
		dma_p->dma_type = DMA_TYPE_DOUBLE_DWORD;//one block transfer is OK, DMA type is BYTE type
		dma_p->len = 4095;
		dma_p->p_src = (INT8U *)src;
		dma_p->p_des = (INT8U *)des;
		dma_p->dma_proc_type = ONE_BLOCK_DMA;
	}
	else// link list
	{
		
		dma_p->dma_type = DMA_TYPE_DOUBLE_DWORD;
		if(upp_4k)
			dma_p->len = 4095;
		else
			dma_p->len = less_4k;
		dma_p->p_src = (INT8U *)src;
		dma_p->p_des = (INT8U *)des;
		dma_p->dma_proc_type = MULTI_BLOCK_DMA;

		/* set source Transfer width 64bit, destination trasnfer Width 64bit, enable trasnfer interrupt */
		temp = ((INT8U)DMA_TYPE_DOUBLE_DWORD<<1)|((INT8U)DMA_TYPE_DOUBLE_DWORD<<4)|(0x1<<0);
		/* SMS AHB Master 1 , select DMS AHB Master 2, SRC_msize = 1 byte, DES_msize = 1 byte, mem2mem DMA */
		temp |= (AHB_MASTER1<<SMS_POS)|(AHB_MASTER2<<DMS_POS)|(DMA_BUS_4<<SRC_MSIZE_POS)|(DMA_BUS_4<<DES_MSIZE_POS)|(0<<TT_FC_POS);
		/* select LLP SRC enable,  LLP DES enable*/
		temp |= (1<<LLP_SRC_POS)|(1<<LLP_DES_POS);
		
		dma_lli0.src_p        = VA2PA((INT32U)dma_p->p_src)&0x1fffffff;//config src addr for LLI DMA, should be Physical addr
		dma_lli0.des_p        = VA2PA((INT32U)dma_p->p_des)&0x1fffffff;//config des addr for LLI DMA, should be Physical addr
		dma_lli0.llp          = (INT32U)LLI_LOC_PMEM_BASE;
		dma_lli0.llp_addr     = (INT32U)LLI_LOC_VMEM_BASE;
		dma_lli0.ctl_l        = temp;
		dma_lli0.ctl_h        = dma_p->len;
		if((less_4k == 0)&&(less == 0)&&(upp_4k > 1))// for n*4095*4K bytes, no need for multiblock transfer
		{
			for(count=0;count<upp_4k;count++)
		    {
		      if(count == upp_4k-1)
		      	{
			  	  dma_lli0.llp=0;
		          dma_lli0.ctl_l = dma_lli0.ctl_l & 0xe7ffffff;// last DMA block, close LLI,set SRC dES TR width =1byte
		      	}
			  else
			  	{
			  	  dma_lli0.ctl_h  = 4095;
			  	  dma_lli0.llp = dma_lli0.llp + 20;
			  	}
			  *(INT32U *)dma_lli0.llp_addr 		  = dma_lli0.src_p;
			  *(INT32U *)(dma_lli0.llp_addr + 4)  = dma_lli0.des_p;
			  *(INT32U *)(dma_lli0.llp_addr + 8)  = dma_lli0.llp;
			  *(INT32U *)(dma_lli0.llp_addr + 12) = dma_lli0.ctl_l;
			  *(INT32U *)(dma_lli0.llp_addr + 16) = dma_lli0.ctl_h;
			  dma_lli0.llp_addr += 20;
			  dma_lli0.src_p  = dma_lli0.src_p  + dma_lli0.ctl_h*8;
		      dma_lli0.des_p  = dma_lli0.des_p +  dma_lli0.ctl_h*8;
		    }
		}
		else if((less == 0) && (less_4k != 0) && (upp_4k != 0))
		{
			for(count=0;count<upp_4k+1;count++)
		    {
		      if(count == upp_4k)
		      	{
			  	  dma_lli0.llp=0;
				  dma_lli0.ctl_h  = less_4k;
		          dma_lli0.ctl_l = dma_lli0.ctl_l & 0xe7ffffff;// last DMA block, close LLI,set SRC dES TR width =1byte
		      	}
			  else
			  	{
			  	  dma_lli0.ctl_h  = 4095;
			  	  dma_lli0.llp = dma_lli0.llp + 20;
			  	}
			  *(INT32U *)dma_lli0.llp_addr 		  = dma_lli0.src_p;
			  *(INT32U *)(dma_lli0.llp_addr + 4)  = dma_lli0.des_p;
			  *(INT32U *)(dma_lli0.llp_addr + 8)  = dma_lli0.llp;
			  *(INT32U *)(dma_lli0.llp_addr + 12) = dma_lli0.ctl_l;
			  *(INT32U *)(dma_lli0.llp_addr + 16) = dma_lli0.ctl_h;
			  dma_lli0.llp_addr += 20;
			  dma_lli0.src_p  = dma_lli0.src_p  + dma_lli0.ctl_h*8;
		      dma_lli0.des_p  = dma_lli0.des_p +  dma_lli0.ctl_h*8;
		    }
		}
		else if((less != 0) && (less_4k == 0) && (upp_4k != 0))
		{
			for(count=0;count<upp_4k+1;count++)
		    {
		      if(count == upp_4k)
		      	{
			  	  dma_lli0.llp=0;
				  dma_lli0.ctl_h  = less;
		          dma_lli0.ctl_l = dma_lli0.ctl_l & 0xe7ffff81;// last DMA block, close LLI,set SRC dES TR width =1byte
		      	}
			  else
			  	{
			  	  dma_lli0.ctl_h  = 4095;
			  	  dma_lli0.llp = dma_lli0.llp + 20;
			  	}
			  *(INT32U *)dma_lli0.llp_addr 		  = dma_lli0.src_p;
			  *(INT32U *)(dma_lli0.llp_addr + 4)  = dma_lli0.des_p;
			  *(INT32U *)(dma_lli0.llp_addr + 8)  = dma_lli0.llp;
			  *(INT32U *)(dma_lli0.llp_addr + 12) = dma_lli0.ctl_l;
			  *(INT32U *)(dma_lli0.llp_addr + 16) = dma_lli0.ctl_h;
			  dma_lli0.llp_addr += 20;
			  dma_lli0.src_p  = dma_lli0.src_p  + dma_lli0.ctl_h*8;
		      dma_lli0.des_p  = dma_lli0.des_p +  dma_lli0.ctl_h*8;
		    }
		}
		else if((less != 0) && (less_4k != 0))
		{	
			for(count=0;count<upp_4k+2;count++)
		    {
		      if(count == upp_4k)
		        {
		          dma_lli0.llp 	  = dma_lli0.llp + 20;
		          dma_lli0.ctl_h  = less_4k;
				  
		        }
		      else if(count == upp_4k+1)
		      	{
			  	  dma_lli0.llp=0;
				  dma_lli0.ctl_h  = less;
		          dma_lli0.ctl_l = dma_lli0.ctl_l & 0xe7ffff81;
		      	}
			  else
			  	{
			  	  dma_lli0.llp = dma_lli0.llp + 20;
		      	}
			  *(INT32U *)dma_lli0.llp_addr 		  = dma_lli0.src_p;
			  *(INT32U *)(dma_lli0.llp_addr + 4)  = dma_lli0.des_p;
			  *(INT32U *)(dma_lli0.llp_addr + 8)  = dma_lli0.llp;
			  *(INT32U *)(dma_lli0.llp_addr + 12) = dma_lli0.ctl_l;
			  *(INT32U *)(dma_lli0.llp_addr + 16) = dma_lli0.ctl_h;
			  dma_lli0.llp_addr += 20;
			  if(count == upp_4k)
		      	{

				  dma_lli0.src_p  = dma_lli0.src_p  + less_4k*8;
		      	  dma_lli0.des_p  = dma_lli0.des_p  + less_4k*8;
		      	}
			  else
			  	{
				  dma_lli0.src_p  = dma_lli0.src_p  + 4095*8;
		      	  dma_lli0.des_p  = dma_lli0.des_p +  4095*8;
		      	}
		    }
		}
	}
	return 0;
}


/* funtion for anybyte lenth DMA transfer */
void dma_large(INT32U dma_size,INT32U src, INT32U des, struct DMA_INFO *dma_p)
{
	struct DMA_INFO *dma_info_p = dma_p;
	set_dma_large_info(dma_size,src,des,dma_info_p);
	if(dma_info_p->dma_proc_type == ONE_BLOCK_DMA)
	{
		rlx_dcache_wbinvalidate_all();// Write back cache to MEM
		dma_channel0(dma_info_p);
	}
	else if(dma_info_p->dma_proc_type == MULTI_BLOCK_DMA)
	{
		rlx_dcache_wbinvalidate_all();// Write back cache to MEM
		dma_channel0_lli(dma_info_p);
	}
	//start_channel0();
}

/* funtion for anybyte lenth DMA transfer
function: DMA DMEM of RKVM to DMEM of OOBMAC
note: 1.DMEM to DMEM need not through cache
      2.RKVM length is less than 4K,so only use ONE_BLOCK_DMA in reality
      3.RISC don't support embedded interrupt.if RKVM handler use DMA,it should only use polling mode instead of interrupt mode.
*/
void dma_large_RKVM(INT32U dma_size,INT32U src, INT32U des, struct DMA_INFO *dma_p)
{
	struct DMA_INFO *dma_info_p = dma_p;
	set_dma_large_info(dma_size,src,des,dma_info_p);
	if(dma_info_p->dma_proc_type == ONE_BLOCK_DMA)  //less than 4K
	{
	//	rlx_dcache_wbinvalidate_all();// Write back cache to MEM
		dma_channel0(dma_info_p);
		while(!(REG8(DMA_RawTfr)&0x01));  //raw interrupt state of Transfer, wait for DMA done
		REG8(DMA_ClearTfr)   = 0x1;//clear channel0 interrupt state
	}
	else if(dma_info_p->dma_proc_type == MULTI_BLOCK_DMA)
	{
	//	rlx_dcache_wbinvalidate_all();// Write back cache to MEM
		dma_channel0_lli(dma_info_p);
		while(!(REG8(DMA_RawBlock)&0x01));  //raw interrupt state of Transfer, wait for DMA done
		REG8(DMA_ClearBlock)   = 0x1;//clear channel0 interrupt state
	}
	
}

void dma_large_VGA(INT32U dma_size,INT32U src, INT32U des, struct DMA_INFO *dma_p)
{
	struct DMA_INFO *dma_info_p = dma_p;
	set_dma_large_info(dma_size,src,des,dma_info_p);
	if(dma_info_p->dma_proc_type == ONE_BLOCK_DMA)  //less than 4K
	{
	//	rlx_dcache_wbinvalidate_all();// Write back cache to MEM
		dma_channel0(dma_info_p);
		while(!(REG8(DMA_RawTfr)&0x01));  //raw interrupt state of Transfer, wait for DMA done
		REG8(DMA_ClearTfr)   = 0x1;//clear channel0 interrupt state
	}
	else if(dma_info_p->dma_proc_type == MULTI_BLOCK_DMA)
	{
	//	rlx_dcache_wbinvalidate_all();// Write back cache to MEM
		dma_channel0_lli(dma_info_p);
		while(!(REG8(DMA_RawBlock)&0x01));  //raw interrupt state of Transfer, wait for DMA done
		REG8(DMA_ClearBlock)   = 0x1;//clear channel0 interrupt state
	}
	
}


void start_channel0(void)
{
	REG32(DMA_ChEnReg)   = 0x0101;//enable Channel0
	
	call_dmac_cnt++;
}

/* dma */
void dma_multi_block(INT32U src, INT32U des,struct DMA_MULTI_INFO *dma_mul_p, struct DMA_INFO *dma_p)
{
	struct DMA_INFO *dma_info_p = dma_p;
	struct DMA_MULTI_INFO *dma_mul_info_p = dma_mul_p;
	INT32U i;
	for(i=0;i<4;i++)
	{
		dma_large(dma_mul_info_p->len,src,des,dma_info_p);
		start_channel0();
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
		src += dma_mul_info_p->len + dma_mul_info_p->inter_src;
		des += dma_mul_info_p->len + dma_mul_info_p->inter_des;
		dma_mul_info_p++;
	}
}




void bsp_dma_handler(void)
{
	dma_int_cnt++;
	REG32(DMA_MaskTfr)	 = 0x0100;//mask channel0 for Transfer   added by melody 20170113
	REG32(DMA_MaskBlock) = 0x0100;//mask channel0 for Block Transfer  added by melody 20170113

	REG32(DMA_ClearTfr)   = 0x1;//clear channel0 interrupt
	REG32(DMA_ClearBlock) = 0x1;//clear channel0 Block interrupt
	//stop_test_timer();
	//check_timer_value = check_test_timer();
	dma_flag = 1;
	bsp_wait(50 * 3); //about 50 us, added for issue: interrupt once again by melody@20160927 
}

#if 0
void bsp_dma_handler(void)
{
	REG32(DMA_ClearTfr)   = 0x1;//clear channel0 interrupt
	//REG32(DMA_ClearBlock) = 0x1;//clear channel0 Block interrupt
	REG32(DMA_ChEnReg)   = 0xff00;//bit8 is Channel0 Write enable, bit0 is Channel0 enable, here disable Channel0
									//disable all channel
	stop_test_timer();
	check_timer_value = check_test_timer();
	dma_flag = 1;
}
#endif


/* 	function for DMAC test 
*	input parameter val is char from UART
*     val :
*		d: DMA normal test
*		s: DMA scatter test
*		g: DMA gather test
*		l:  DMA Link list test
*/

#if 0
void dma_test(char val, struct DMA_INFO *dma_p)
{
	INT32U i,j,temp,pos;
	volatile INT8U *p_data_s;
	volatile INT8U *p_data_e;
	if((val=='d'))
	{
		set_dma_info(	dma_p, \
						(DMA_SRC), \
						(DMA_DES), \
						DMA_TYPE_DOUBLE_DWORD, \
						DMA_SIZE,  \
						DMA_COUNT, \
						DMA_INTERVAL);
		//memset((void *)0x80040000,0,0x40000);
		p_data_s = (INT8U *)(DMA_SRC|0x20000000);
		p_data_e = (INT8U *)(DMA_DES|0x20000000);
		for(i=1;i<=DMA_SIZE;i++)
		{
			*p_data_s++ = (INT8U)i;
		}
		
		for(i=1;i<=DMA_SIZE;i++)
		{
			//*p_data_e++ = 0xa5;
		}
		//rlx_dcache_wbinvalidate_all();// Write back cache to MEM
		uart_write_str("Start DMA:\r\n");
		if(dma_channel0(dma_p)==0)
		{
			clear_test_timer();
			start_channel0();
			start_test_timer();
			while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
			dma_flag = 0;
			uart_write_str("DMA: done!\r\n");
			uart_write_str("SRC: 0x");
			itoa_self(DMA_SRC,char_temp,16);//get src address
			uart_write_str(char_temp);
			uart_write_str(" DES: 0x");
			itoa_self(DMA_DES,char_temp,16);//get des address
			uart_write_str(char_temp);
			uart_write_str(" lenth:  ");
			itoa_self(dma_p->len,char_temp,10);//get length
			uart_write_str(char_temp);
			uart_write_str(" type: byte");
			uart_write_str("\r\n");
			// check value multipile 16 and mulitipile clk divide mean ns
			// divide 1000 get us
			clk_divide_val = REG8(CPU1_IOBASE+4)&0x07;//get clock divide first
			check_timer_value = (check_timer_value<<(4+clk_divide_val))/1000;
			itoa_self(check_timer_value,char_temp,10);//display unit is us
			uart_write_str(char_temp);
			uart_write_str(" us");
			uart_write_str("\r\n");
		}
		p_data_s = (INT8U *)(DMA_SRC|0x20000000);
		p_data_e = (INT8U *)(DMA_DES|0x20000000);
		for(i=1;i<=DMA_SIZE;i++)
		{
			if((*p_data_e++) != (INT8U)i)
				break;
		}
		if(i!=DMA_SIZE+1)
		{
			uart_write_str("DMA data check failed!\r\n");
			itoa_self(i,char_temp,16);//display unit is us
			uart_write_str("failed postion : 0X");
			uart_write_str(char_temp);
			uart_write_str("\r\n");
		}
		else
			uart_write_str("DMA data check succeed!\r\n");
	}


	if((val=='s'))//for DMA scatter test
	{
		set_dma_info(	dma_p, \
						DMA_SRC_S, \
						DMA_DES_S, \
						DMA_TYPE_BYTE, \
						DMA_SIZE,  \
						DMA_COUNT, \
						DMA_INTERVAL);
		for(i=1,pos=1;i<=TEST_LEN;i++,pos++)// set SRC and DES patten
		{
			dma_p->p_src[i-1] = (INT8U)i;
			dma_p->p_des[pos-1] = 0xa5;
			if(((i%dma_p->count)==0))
				pos = pos+ dma_p->interval + 1;
		}
		rlx_dcache_wbinvalidate_all();// Write back cache to MEM
		uart_write_str("Start DMA Scatter:\r\n");
		if(dma_channel0_scatter(dma_p)==0)
		{
			clear_test_timer();
			start_channel0();
			start_test_timer();
			while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
			dma_flag = 0;
			uart_write_str("DMA scatter: done!\r\n");
			uart_write_str("SRC: 0x");
			itoa_self(DMA_SRC_S,char_temp,16);//get src address
			uart_write_str(char_temp);
			uart_write_str(" DES: 0x");
			itoa_self(DMA_DES_S,char_temp,16);//get des address
			uart_write_str(char_temp);
			uart_write_str(" lenth:  ");
			itoa_self(dma_p->len,char_temp,10);//get length
			uart_write_str(char_temp);
			uart_write_str(" type: BYTE");
			uart_write_str("\r\n");
			// check value multipile 16 and mulitipile clk divide mean ns
			// divide 1000 get us
			clk_divide_val = REG8(CPU1_IOBASE+4)&0x07;//get clock divide first
			check_timer_value = (check_timer_value<<(4+clk_divide_val))/1000;
			itoa_self(check_timer_value,char_temp,10);//display unit is us
			uart_write_str(char_temp);
			uart_write_str(" us");
			uart_write_str("\r\n");
		}
		for(i=1;i<=dma_p->len;i++)
		{
			temp = *dma_p->p_des;
			if(((i%dma_p->count)==0))
				dma_p->p_des = dma_p->p_des+ dma_p->interval + 1;
			else
				dma_p->p_des++;
			
			if(temp != (INT8U)i)
				break;
		}
		if(i!=(dma_p->len+1))
		{
			uart_write_str("DMA data check failed!\r\n");
			itoa_self(i,char_temp,16);//display unit is us
			uart_write_str("failed postion : 0X");
			uart_write_str(char_temp);
			uart_write_str("\r\n");
		}
		else
			uart_write_str("DMA scatter data check succeed!\r\n");
	}

	if((val=='g'))//for DMA gather test
	{
		set_dma_info(	dma_p, \
						(DMA_SRC_G|0x20000000), \
						DMA_DES_G, \
						DMA_TYPE_DWORD, \
						DMA_SIZE,  \
						DMA_COUNT, \
						DMA_INTERVAL);
		for(i=1,pos=1;i<=dma_p->len;i++,pos++)// set SRC and DES patten
		{
			//dma_p->p_des[i-1] = 0xa5;
			dma_p->p_src[pos-1] = (INT8U)i;
			if(((i%dma_p->count)==0))
				pos = pos+ dma_p->interval;
				
		}
		//rlx_dcache_wbinvalidate_all();// Write back cache to MEM
		uart_write_str("Start DMA Gather:\r\n");
		if(dma_channel0_gather(dma_p)==0)
		{
			clear_test_timer();
			start_channel0();
			start_test_timer();
			while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
			dma_flag = 0;
			uart_write_str("DMA Gather: done!\r\n");
			uart_write_str("SRC: 0x");
			itoa_self(DMA_SRC_G,char_temp,16);//get src address
			uart_write_str(char_temp);
			uart_write_str(" DES: 0x");
			itoa_self(DMA_DES_G,char_temp,16);//get des address
			uart_write_str(char_temp);
			uart_write_str(" lenth:  ");
			itoa_self(dma_p->len,char_temp,10);//get length
			uart_write_str(char_temp);
			uart_write_str(" type: BYTE");
			uart_write_str("\r\n");
			// check value multipile 16 and mulitipile clk divide mean ns
			// divide 1000 get us
			clk_divide_val = REG8(CPU1_IOBASE+4)&0x07;//get clock divide first
			check_timer_value = (check_timer_value<<(4+clk_divide_val))/1000;
			itoa_self(check_timer_value,char_temp,10);//display unit is us
			uart_write_str(char_temp);
			uart_write_str(" us");
			uart_write_str("\r\n");
		}
		for(i=1;i<=dma_p->len;i++)
		{				
			if(dma_p->p_des[i-1] != (INT8U)i)
				break;
		}
		if(i!=(dma_p->len+1))
		{
			uart_write_str("DMA data check failed!\r\n");
			itoa_self(i,char_temp,16);//display unit is us
			uart_write_str("failed postion : 0X");
			uart_write_str(char_temp);
			uart_write_str("\r\n");
		}
		else
			uart_write_str("DMA Gather data check succeed!\r\n");
	}

	if((val=='h'))//for DMA large test
	{
		p_data_s = (INT8U *)(DMA_SRC_H|0x20000000);
		p_data_e = (INT8U *)(DMA_DES_H|0x20000000);
		for(j=1;j<=DMA_SIZE;j++)
		{	
			//*p_data_e++ = 0xa5;
			*p_data_s++ = (INT8U)j;
		}
		//rlx_dcache_wbinvalidate_all();// Write back cache to MEM
		uart_write_str("Start DMA large:\r\n");
		dma_large(DMA_SIZE,DMA_SRC_H,DMA_DES_H,dma_p);
		clear_test_timer();
		start_channel0();
		start_test_timer();
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
		uart_write_str("DMA large: done!\r\n");
		uart_write_str("SRC: 0x");
		itoa_self(DMA_SRC_H,char_temp,16);//get src address
		uart_write_str(char_temp);
		uart_write_str(" DES: 0x");
		itoa_self(DMA_DES_H,char_temp,16);//get des address
		uart_write_str(char_temp);
		uart_write_str(" lenth:  ");
		itoa_self(DMA_SIZE,char_temp,16);//get length
		uart_write_str(char_temp);
		uart_write_str(" type: BYTE");
		uart_write_str("\r\n");
		// check value multipile 16 and mulitipile clk divide mean ns
		// divide 1000 get us
		clk_divide_val = REG8(CPU1_IOBASE+4)&0x07;//get clock divide first
		check_timer_value = (check_timer_value<<(4+clk_divide_val))/1000;
		itoa_self(check_timer_value,char_temp,10);//display unit is us
		uart_write_str(char_temp);
		uart_write_str(" us");
		uart_write_str("\r\n");
		p_data_s = (INT8U *)(DMA_SRC_H|0x20000000);
		p_data_e = (INT8U *)(DMA_DES_H|0x20000000);
		for(j=1;j<=DMA_SIZE;j++)
		{	
			if((*p_data_e++) != (INT8U)j)
				break;
		}
		if((j!=DMA_SIZE+ 1))
		{
			uart_write_str("DMA data check failed!\r\n");
			itoa_self(j,char_temp,16);//display unit is us
			uart_write_str("failed postion : 0X");
			uart_write_str(char_temp);
			uart_write_str("\r\n");
		}
		else
			uart_write_str("DMA large data check succeed!\r\n");
	}
	
	
	
	if((val=='m'))//for DMA multi block test
	{
		p_data_s = (INT8U *)(DMA_SRC_M|0x20000000);
		p_data_e = (INT8U *)(DMA_DES_M|0x20000000);
		for(i=0;i<4;i++)
		{
			for(j=1;j<=dma_mul_info[i].len;j++)
			{	
				*p_data_s++ = (INT8U)j;
				*p_data_e++ = 0xa5;					
			}
			p_data_s += dma_mul_info[i].inter_src;
			p_data_e += dma_mul_info[i].inter_des;
		}
		uart_write_str("DMA multi start!\r\n");
		dma_multi_block(DMA_SRC_M,DMA_DES_M,dma_mul_info,dma_p);
		
		p_data_e = (INT8U *)(DMA_DES_M|0x20000000);
		for(i=0;i<4;i++)
		{
			for(j=1;j<=dma_mul_info[i].len;j++)
			{	
				if((*p_data_e++) != (INT8U)j)
				break;
			}
			if(j != dma_mul_info[i].len + 1)
				break;
			p_data_e += dma_mul_info[i].inter_des;
		}
		if((j!=dma_mul_info[3].len + 1) ||(i!=4))
		{
			uart_write_str("DMA data check failed!\r\n");
			itoa_self(j,char_temp,16);//display unit is us
			uart_write_str("failed postion X: 0X");
			uart_write_str(char_temp);
			itoa_self(i,char_temp,16);//display unit is us
			uart_write_str(" Y: 0X");
			uart_write_str(char_temp);
			uart_write_str("\r\n");
		}
		else
			uart_write_str("DMA large data check succeed!\r\n");
		/*p_data_s = (INT8U *)(DMA_SRC_M|0x20000000);
		p_data_e = (INT8U *)(DMA_DES_M|0x20000000);
		for(i=0;i<3;i++)
		{
			for(j=1;j<=dma_mul_info[i].len;j++)
			{	
				*p_data_s++ = 0xff;
				*p_data_e++ = 0xff;					
			}
			p_data_s += dma_mul_info[i].inter_src;
			p_data_e += dma_mul_info[i].inter_des;
		}*/
	}
}
#endif


/* test Timer for time count */
/* use MIPS internal timer */
void clear_test_timer(void)
{
	INT32U cause_temp = 0;
	//disable inner Timer first
	asm volatile("mfc0   %0, $13"   : "=r"(cause_temp));
	cause_temp |=(1<<27);
	asm volatile("mtc0   %0, $13"   :: "r"(cause_temp));
	cause_temp = 0;
	//set count and compare to 0
	asm volatile("mtc0   %0, $9"   :: "r"(cause_temp));
	asm volatile("mtc0   %0, $11"   :: "r"(cause_temp));
}

void start_test_timer(void)
{
	INT32U cause_temp = 0;
	//enableable inner Timer first
	asm volatile("mfc0   %0, $13"   : "=r"(cause_temp));
	cause_temp &=~(1<<27);
	asm volatile("mtc0   %0, $13"   :: "r"(cause_temp));
}
void stop_test_timer(void)
{
	INT32U cause_temp = 0;
	//disable inner Timer first
	asm volatile("mfc0   %0, $13"   : "=r"(cause_temp));
	cause_temp |=(1<<27);
	asm volatile("mtc0   %0, $13"   :: "r"(cause_temp));
	
}
INT32U check_test_timer(void)
{
	INT32U temp ;
	asm volatile("mfc0   %0, $9"   : "=r"(temp));
	return temp;
	
}

INT32U  get_timer(INT32U base)
{
	//printf("%s = %x\n", __FUNCTION__, mips_count_get() );
	return check_test_timer() - base;
}


// only for testing, random function based on timer
INT32U rand_gen(INT32U bottom, INT32U top)
{
	INT32U temp;
	if(top<bottom)
		while(1);
	do{
		temp = check_test_timer();
		temp = (temp%top) ;
		}while(temp<bottom);
	return temp;
}

/* for OOB MAC access IB REG channel*/

#if 0
INT32U OOB_READ_IB(INT16U addr)/* addr should be OCP addr */
{
	REG32(OOBMAC_IOBASE + IO_IB_ACC_SET) = 0x800f0000 | addr;
	while(REG32(OOBMAC_IOBASE + IO_IB_ACC_SET)&0x80000000);
	return(REG32(OOBMAC_IOBASE + IO_IB_ACC_DATA));
}
void OOB_WRITE_IB(INT16U addr,INT32U data)/* addr should be OCP addr */
{
	REG32(OOBMAC_IOBASE + IO_IB_ACC_DATA) = data;
	REG32(OOBMAC_IOBASE + IO_IB_ACC_SET) = 0x808f0000 | addr;
	while(REG32(OOBMAC_IOBASE + IO_IB_ACC_SET)&0x80000000);
}
#endif

/* for 8111FP, only PKG172 have UART module, and UART RX pin is shared with NCSI
*   default UART rx pin is not input 
*   so, here we enable UART rx pin input for debug
*/
#if 0
void uart_rx_en_not_pkg172(void)
{
	INT32U value;
	value = OOB_READ_IB(0xdc0c);//read PICO 0xDC0e
	uart_write_str("en UART Rx\r\n");
	uart_write_str("ERI 0xDC0E: 0x");
	itoa_self(value,char_temp,16);//get src address
	uart_write_str(char_temp);
	uart_write_str("\r\n");
	uart_write_str("set ERI 0xDC0E: 0x");
	itoa_self(value|(0x00060000),char_temp,16);//get src address
	uart_write_str(char_temp);
	uart_write_str("\r\n");
	OOB_WRITE_IB(0xdc0c,value|(0x00060000));//set UART Rx pin input
}
#endif

/* calculate and return  x^y */
INT32U pow_self(INT32U x,INT32U y)
{
	INT8U i;
	if (y==0)
		return 1;
	if (y==1)
		return x;
	for(i=1;i<y;i++)
	{
		x = x*x;
	}
	return x;
}
//for zhuqin , VGA DMA from DDR to DMEM
void dmac_test_gather(struct DMA_INFO *dma_p)
{
	volatile INT32U i;

	for(i=0;i<16*1024;i++)
		{
		*(INT8U *)(0xa8040000+i) = (INT8U)i;
		}
	
	set_dma_info(dma_p,\
				0x88040000|0x20000000,\
				0x80040000,\
				DMA_TYPE_DWORD,\
				1*64*16,\
				16,\
				16);
	
	
	dma_channel0_gather(dma_p);
	start_channel0();
	while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
	dma_flag = 0;

}


#define ALIGN_DATA(x) (((x)>>2)<<2)

void dma_test_asic(struct DMA_INFO *dma_p)
{
	volatile INT32U i;
	INT32U src,des,len;
	static INT32U round1 = 0,round2 = 0,round3 = 0,round4 = 0;
	//dmem to dmem
	round1++;
	srand(check_test_timer());
	do{
		src = 0x80040000 + rand()%0x40000;
		des = 0x80040000 + rand()%0x40000;
		len = (rand()%4095)+1;
	}while(((src + len + 4) > 0x80080000) || ((des + len + 4) > 0x80080000) || \
			(src == des) || \
			((src<des)&&((src + len + 4)>des)) || \
			((des<src)&&((des + len + 4)>src)));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE_BYTE, \
					len,  \
					0, \
					0);
	src = src|0x20000000;
	des = des|0x20000000;
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(src + i-1) = (INT8U)(i-1);
	}
	
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(des + i -1) = 0xa5;
	}
	
	if(dma_channel0(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len;i++)
	{
		if(*(INT8U *)(des+i-1) != (INT8U)(i-1))
			{
			while(1);
			}
	}

	//dmem to ddr
	round2++;
	do{
		src = 0x80050000 + rand()%0x20000;
		des = 0xa8800000 + rand()%4;
		len = (rand()%4095)+1;
	}while(((src + len + 4) > 0x80080000) || (des + len + 4) > 0xa9000000);
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE_BYTE, \
					len,  \
					0, \
					0);
	src = src|0x20000000;
	des = des|0x20000000;
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(src + i -1 ) = (INT8U)(i-1);
	}
	
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(des + i - 1) = 0xa5;
	}
	
	if(dma_channel0(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len;i++)
	{
		if(*(INT8U *)(des+i-1) != (INT8U)(i-1))
			{
		 	*(INT32U *)0xbaf70180 = 2;
			while(1);
			}
	}

	//ddr to dmem
	round3++;
	do{
		src = 0xa8800000 + + rand()%4;
		des = 0x80040000 + rand()%0x40000;
		len = (rand()%4095)+1;
	}while(((src + len + 4) > 0xa9000000) || (des + len + 4) > 0x80080000);
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE_BYTE, \
					len,  \
					0, \
					0);
	src = src|0x20000000;
	des = des|0x20000000;
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(src + i -1 ) = (INT8U)(i-1);
	}
	
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(des + i - 1) = 0xa5;
	}
	
	if(dma_channel0(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len;i++)
	{
		if(*(INT8U *)(des+i-1) != (INT8U)(i-1))
			{
			while(1);
			}
	}

	//ddr to ddr
	round4++;
	do{
		src = 0xa8800000 + rand()%0x800000;
		des = 0xa8800000 + rand()%0x800000;
		len = (rand()%4095)+1;
	}while(((src + len + 4) > 0xa9000000) || ((des + len + 4) > 0xa9000000) || \
			(src == des) || \
			((src<des)&&((src + len + 4)>des)) || \
			((des<src)&&((des + len + 4)>src)));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE_BYTE, \
					len,  \
					0, \
					0);
	src = src|0x20000000;
	des = des|0x20000000;
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(src + i -1 ) = (INT8U)(i-1);
	}
	
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(des + i - 1) = 0xa5;
	}
	
	if(dma_channel0(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len;i++)
	{
		if(*(INT8U *)(des+i-1) != (INT8U)(i-1))
			{
				while(1);
			}
	}
}

void dma_test_asic_large(struct DMA_INFO *dma_p)
{
	volatile INT32U i;
	INT32U src,des,len;
	static INT32U la_round1 = 0,la_round2 = 0,la_round3 = 0,la_round4 = 0;
	//dmem to dmem
	la_round1++;
	srand(check_test_timer());
	
	do{
		src = 0x80040000 + rand()%0x40000;
		des = 0x80040000 + rand()%0x40000;
		len = (rand()%100000)+1;
	}while(((src + len + 4) > 0x80080000) || ((des + len + 4) > 0x80080000) || \
			(src == des) || \
			((src<des)&&((src + len + 4)>des)) || \
			((des<src)&&((des + len + 4)>src)));
	
	//src = src|0x20000000;
	//des = des|0x20000000;
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(src + i-1) = (INT8U)(i-1);
	}
	
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(des + i -1) = 0xa5;
	}
	
	dma_large(len,src,des,dma_p);
	while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
	dma_flag = 0;
	
	for(i=1;i<=len;i++)
	{
		if(*(INT8U *)(des+i-1) != (INT8U)(i-1))
			{
			while(1);
			}
	}

	//dmem to ddr
	la_round2++;
	do{
		src = 0x80040000 + rand()%0x40000;
		des = 0xa8800000 + rand()%0x800000;
		len = (rand()%200000)+1;
	}while(((src + len + 4) > 0x80080000) || (des + len + 4) > 0xa9000000);
	

	for(i=1;i<=len;i++)
	{
		*(INT8U *)(src + i -1 ) = (INT8U)(i-1);
	}
	
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(des + i - 1) = 0xa5;
	}
	
	dma_large(len,src,des,dma_p);
	while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
	dma_flag = 0;

	for(i=1;i<=len;i++)
	{
		if(*(INT8U *)(des+i-1) != (INT8U)(i-1))
			{
			while(1);
			}
	}

	//ddr to dmem
	la_round3++;
	do{
		src = 0xa8800000 + + rand()%0x800000;
		des = 0x80040000 + rand()%0x40000;
		len = (rand()%200000)+1;
	}while(((src + len + 4) > 0xa9000000) || (des + len + 4) > 0x80080000);
	
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(src + i -1 ) = (INT8U)(i-1);
	}
	
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(des + i - 1) = 0xa5;
	}
	
	dma_large(len,src,des,dma_p);
	while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
	dma_flag = 0;
	
	for(i=1;i<=len;i++)
	{
		if(*(INT8U *)(des+i-1) != (INT8U)(i-1))
			{
			while(1);
			}
	}

	//ddr to ddr
	la_round4++;
	do{
		src = 0xa8800000 + rand()%0x800000;
		des = 0xa8800000 + rand()%0x800000;
		len = (rand()%2000000)+1;
	}while(((src + len + 4) > 0xa9000000) || ((des + len + 4) > 0xa9000000) || \
			(src == des) || \
			((src<des)&&((src + len + 4)>des)) || \
			((des<src)&&((des + len + 4)>src)));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE_BYTE, \
					len,  \
					0, \
					0);

	for(i=1;i<=len;i++)
	{
		*(INT8U *)(src + i -1 ) = (INT8U)(i-1);
	}
	
	for(i=1;i<=len;i++)
	{
		*(INT8U *)(des + i - 1) = 0xa5;
	}
	
	dma_large(len,src,des,dma_p);
	while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
	dma_flag = 0;
	
	for(i=1;i<=len;i++)
	{
		if(*(INT8U *)(des+i-1) != (INT8U)(i-1))
			{
				while(1);
			}
	}
}

void dma_test_asic_scatter(struct DMA_INFO *dma_p)
{
	volatile INT32U i,temp,temp2;
	INT32U src,des,len,cnt,intval,mul;
	static INT32U sca_round1 = 0,sca_round2 = 0,sca_round3 = 0,sca_round4 = 0;
	//dmem to dmem
	srand(check_test_timer());
	
	sca_round1++;
	do{
		src = ((0x80040000 + rand()%0x40000)>>2)<<2;
		des = ((0x80040000 + rand()%0x40000)>>2)<<2;
		do{
			len = (rand()%4092)+4;
			len = ALIGN_DATA(len);
			cnt = (rand()%100)+4;
			cnt = ALIGN_DATA(cnt);
			mul = len/cnt;
		}while((mul)<2);
		len = mul*cnt;
		intval = rand()%100;
		intval = ALIGN_DATA(intval);
		
	}while(	(src == des) || \
			((src + len*DMA_UNIT_SIZE + 4) > 0x80080000) || \
			((des + (len + intval*mul)*DMA_UNIT_SIZE) > 0x80080000) || \
			(((src + len*DMA_UNIT_SIZE + 4)>des)&&(src<des)) || \
			((((des + (len + intval*mul)*DMA_UNIT_SIZE)>src))&&(src>des)));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE, \
					len,  \
					cnt, \
					intval);
	src = src|0x20000000;
	des = des|0x20000000;
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)src + i-1) = (INT8U)(i-1);
	}
	
	for(i=1;i<=(len + intval*mul)*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)des + i -1) = 0xa5;
	}
	rlx_dcache_wbinvalidate_all();
	if(dma_channel0_scatter(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		temp = (i-1)/cnt/DMA_UNIT_SIZE;
		temp2 = (i-1)%(cnt*DMA_UNIT_SIZE);
		if(*((INT8U *)des+temp*(cnt+intval)*DMA_UNIT_SIZE+temp2) != (INT8U)(i-1))
			{
			while(1);
			}
	}
	
	//dmem to ddr
	sca_round2++;
	do{
		src = ((0x80040000 + rand()%0x40000)>>2)<<2;
		des = ((0xa8800000 + rand()%0x800000)>>2)<<2;
		do{
			len = (rand()%4092)+4;
			len = ALIGN_DATA(len);
			cnt = (rand()%100)+4;
			cnt = ALIGN_DATA(cnt);
			mul = len/cnt;
		}while((mul)<2);
		len = mul*cnt;
		intval = rand()%10;
		intval = ALIGN_DATA(intval);
	}while(	((src + len*DMA_UNIT_SIZE + 4) > 0x80080000) || \
			((des + (len + intval*mul)*DMA_UNIT_SIZE + 4) > 0xa9000000));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE, \
					len,  \
					cnt, \
					intval);
	src = src|0x20000000;
	des = des|0x20000000;
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)src + i-1) = (INT8U)(i-1);
	}
	
	for(i=1;i<=len + intval*mul;i++)
	{
		*((INT8U *)des + i -1) = 0xa5;
	}
	rlx_dcache_wbinvalidate_all();
	if(dma_channel0_scatter(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		temp = (i-1)/cnt/DMA_UNIT_SIZE;
		temp2 = (i-1)%(cnt*DMA_UNIT_SIZE);
		if(*((INT8U *)des+temp*(cnt+intval)*DMA_UNIT_SIZE+temp2) != (INT8U)(i-1))
			{
			while(1);
			}
	}
	
	
	//ddr to dmem
	sca_round3++;
	do{
		src = ((0xa8800000 + rand()%0x800000)>>2)<<2;
		des = ((0x80040000 + rand()%0x40000)>>2)<<2;
		do{
			len = (rand()%4092)+4;
			len = ALIGN_DATA(len);
			cnt = (rand()%100)+4;
			cnt = ALIGN_DATA(cnt);
			mul = len/cnt;
		}while((mul)<2);
		len = mul*cnt;
		intval = rand()%10;
		intval = ALIGN_DATA(intval);
	}while(	((src + len*DMA_UNIT_SIZE + 4) > 0xa9000000) || \
			((des + (len + intval*mul)*DMA_UNIT_SIZE + 4) > 0x80080000));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE, \
					len,  \
					cnt, \
					intval);
	src = src|0x20000000;
	des = des|0x20000000;
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)src + i-1) = (INT8U)(i-1);
	}
	
	for(i=1;i<=(len + intval*mul)*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)des + i -1) = 0xa5;
	}
	
	if(dma_channel0_scatter(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		temp = (i-1)/cnt/DMA_UNIT_SIZE;
		temp2 = (i-1)%(cnt*DMA_UNIT_SIZE);
		if(*((INT8U *)des+temp*(cnt+intval)*DMA_UNIT_SIZE+temp2) != (INT8U)(i-1))
			{
			while(1);
			}
	}
	
	//l2mem to l2mem
	sca_round4++;
	do{
		src = ((0xa8800000 + rand()%0x800000)>>2)<<2;
		des = ((0xa8800000 + rand()%0x800000)>>2)<<2;
		do{
			len = (rand()%4092)+4;
			len = ALIGN_DATA(len);
			cnt = (rand()%100)+4;
			cnt = ALIGN_DATA(cnt);
			mul = len/cnt;
		}while((mul)<2);
		len = mul*cnt;
		intval = rand()%100;
		intval = ALIGN_DATA(intval);
		
	}while(		(src == des) || \
			((src + len*DMA_UNIT_SIZE + 4) > 0xa9000000) || \
			((des + (len + intval*mul)*DMA_UNIT_SIZE + 4) > 0xa9000000) || \
			(((src + len*DMA_UNIT_SIZE + 4)>des)&&(src<des)) || \
			((((des + (len + intval*mul)*DMA_UNIT_SIZE + 4)>src))&&(src>des)));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE, \
					len,  \
					cnt, \
					intval);
	src = src|0x20000000;
	des = des|0x20000000;
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)src + i-1) = (INT8U)(i-1);
	}
	
	for(i=1;i<=(len + intval*mul)*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)des + i -1) = 0xa5;
	}
	rlx_dcache_wbinvalidate_all();
	if(dma_channel0_scatter(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		temp = (i-1)/cnt/DMA_UNIT_SIZE;
		temp2 = (i-1)%(cnt*DMA_UNIT_SIZE);
		if(*((INT8U *)des+temp*(cnt+intval)*DMA_UNIT_SIZE+temp2) != (INT8U)(i-1))
			{
			while(1);
			}
	}
	
}


void dma_test_asic_gather(struct DMA_INFO *dma_p)
{
	volatile INT32U i,temp,temp2;
	INT32U src,des,len,cnt,intval,mul;
	static INT32U ga_round1 = 0,ga_round2 = 0,ga_round3 = 0,ga_round4 = 0;
	//dmem to dmem
	srand(check_test_timer());
	
	ga_round1++;
	do{
		src = ((0x80040000 + rand()%0x40000)>>2)<<2;
		des = ((0x80040000 + rand()%0x40000)>>2)<<2;
		do{
			len = (rand()%4092)+4;
			len = ALIGN_DATA(len);
			cnt = (rand()%100)+4;
			cnt = ALIGN_DATA(cnt);
			mul = len/cnt;
		}while((mul)<2);
		len = mul*cnt;
		intval = rand()%100;
		intval = ALIGN_DATA(intval);
		
	}while(	(src == des) || \
			((des + len*DMA_UNIT_SIZE + 4) > 0x80080000) || \
			((src + (len + intval*mul)*DMA_UNIT_SIZE) > 0x80080000) || \
			(((des + len*DMA_UNIT_SIZE + 4)>src)&&(des<src)) || \
			((((src + (len + intval*mul)*DMA_UNIT_SIZE)>des))&&(des>src)));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE, \
					len,  \
					cnt, \
					intval);
	src = src|0x20000000;
	des = des|0x20000000;
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		temp = (i-1)/cnt/DMA_UNIT_SIZE;
		temp2 = (i-1)%(cnt*DMA_UNIT_SIZE);
		*((INT8U *)src+temp*(cnt+intval)*DMA_UNIT_SIZE+temp2) = (INT8U)(i-1);
	}

	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)des + i-1) = 0xa5;
	}
	
	
	rlx_dcache_wbinvalidate_all();
	if(dma_channel0_gather(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		if(*((INT8U *)des + i-1) != (INT8U)(i-1))
			{
				while(1);
			}
	}
	
	//dmem to ddr
	ga_round2++;
	do{
		src = ((0x80040000 + rand()%0x40000)>>2)<<2;
		des = ((0xa8800000 + rand()%0x800000)>>2)<<2;
		do{
			len = (rand()%4092)+4;
			len = ALIGN_DATA(len);
			cnt = (rand()%100)+4;
			cnt = ALIGN_DATA(cnt);
			mul = len/cnt;
		}while((mul)<2);
		len = mul*cnt;
		intval = rand()%10;
		intval = ALIGN_DATA(intval);
	}while(	((src + (len + intval*mul)*DMA_UNIT_SIZE + 4) > 0x80080000) || \
			((des + len*DMA_UNIT_SIZE + 4) > 0xa9000000));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE, \
					len,  \
					cnt, \
					intval);
	src = src|0x20000000;
	des = des|0x20000000;
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		temp = (i-1)/cnt/DMA_UNIT_SIZE;
		temp2 = (i-1)%(cnt*DMA_UNIT_SIZE);
		*((INT8U *)src+temp*(cnt+intval)*DMA_UNIT_SIZE+temp2) = (INT8U)(i-1);
	}

	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)des + i-1) = 0xa5;
	}
	
	
	rlx_dcache_wbinvalidate_all();
	if(dma_channel0_gather(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		if(*((INT8U *)des + i-1) != (INT8U)(i-1))
			{
				while(1);
			}
	}
	
	
	//ddr to dmem
	ga_round3++;
	do{
		src = ((0xa8800000 + rand()%0x800000)>>2)<<2;
		des = ((0x80040000 + rand()%0x40000)>>2)<<2;
		do{
			len = (rand()%4092)+4;
			len = ALIGN_DATA(len);
			cnt = (rand()%100)+4;
			cnt = ALIGN_DATA(cnt);
			mul = len/cnt;
		}while((mul)<2);
		len = mul*cnt;
		intval = rand()%10;
		intval = ALIGN_DATA(intval);
	}while(	((src + (len + intval*mul)*DMA_UNIT_SIZE + 4) > 0xa9000000) || \
			((des + len*DMA_UNIT_SIZE + 4) > 0x80080000));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE, \
					len,  \
					cnt, \
					intval);
	src = src|0x20000000;
	des = des|0x20000000;
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		temp = (i-1)/cnt/DMA_UNIT_SIZE;
		temp2 = (i-1)%(cnt*DMA_UNIT_SIZE);
		*((INT8U *)src+temp*(cnt+intval)*DMA_UNIT_SIZE+temp2) = (INT8U)(i-1);
	}

	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)des + i-1) = 0xa5;
	}
	
	
	rlx_dcache_wbinvalidate_all();
	if(dma_channel0_gather(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		if(*((INT8U *)des + i-1) != (INT8U)(i-1))
			{
				while(1);
			}
	}
	
	//l2mem to l2mem
	ga_round4++;
	do{
		src = ((0xa8800000 + rand()%0x800000)>>2)<<2;
		des = ((0xa8800000 + rand()%0x800000)>>2)<<2;
		do{
			len = (rand()%4092)+4;
			len = ALIGN_DATA(len);
			cnt = (rand()%100)+4;
			cnt = ALIGN_DATA(cnt);
			mul = len/cnt;
		}while((mul)<2);
		len = mul*cnt;
		intval = rand()%100;
		intval = ALIGN_DATA(intval);
		
	}while(		(src == des) || \
			((des + len*DMA_UNIT_SIZE + 4) > 0xa9000000) || \
			((src + (len + intval*mul)*DMA_UNIT_SIZE + 4) > 0xa9000000) || \
			(((des + len*DMA_UNIT_SIZE + 4)>src)&&(des<src)) || \
			((((src + (len + intval*mul)*DMA_UNIT_SIZE + 4)>des))&&(des>src)));
	
	set_dma_info(	dma_p, \
					(src), \
					(des), \
					DMA_TYPE, \
					len,  \
					cnt, \
					intval);
	src = src|0x20000000;
	des = des|0x20000000;
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		temp = (i-1)/cnt/DMA_UNIT_SIZE;
		temp2 = (i-1)%(cnt*DMA_UNIT_SIZE);
		*((INT8U *)src+temp*(cnt+intval)*DMA_UNIT_SIZE+temp2) = (INT8U)(i-1);
	}

	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		*((INT8U *)des + i-1) = 0xa5;
	}
	
	
	rlx_dcache_wbinvalidate_all();
	if(dma_channel0_gather(dma_p)==0)
	{
		while(dma_flag == 0);//Wait for DMA done. CAUTION: here may stuck CPU
		dma_flag = 0;
	}
	
	for(i=1;i<=len*DMA_UNIT_SIZE;i++)
	{
		if(*((INT8U *)des + i-1) != (INT8U)(i-1))
			{
				while(1);
			}
	}
	
}
#ifdef CONFIG_VGA_TEST_ENABLED
//VGA 1 Block DMA from DDR to DMEM
void dmac_test_gather_VGA(INT32U startaddr,INT32U destaddr,INT8U BPP,struct DMA_INFO *dma_p)
{
//	volatile INT32U i;
	set_dma_info(dma_p,\
	         startaddr|0x20000000,\
	         destaddr,\
	         DMA_TYPE_WORD,\
	         (BPP*HP)*(16>>DMA_TYPE_WORD),\
	        (BPP*HP)>>DMA_TYPE_WORD,\
	        ( (VGAInfo.FB1_Hresol-0x40)>>DMA_TYPE_WORD)*BPP);

	dma_channel0_gather(dma_p);
	while(!(REG8(DMA_RawTfr)&0x01));  //raw interrupt state of Transfer, wait for DMA done
	REG8(DMA_ClearTfr)   = 0x1;//clear channel0 interrupt state
         
}

void dmac_test_nomal(INT32U dma_size,INT32U src, INT32U des, struct DMA_INFO *dma_p)
{
    struct DMA_INFO *dma_info_p = dma_p;
		 
 	dma_large_VGA(dma_size,src, des, dma_info_p);
        
	bsp_oobmac_send_VGA(4,0,0,0,0);
}
#endif


