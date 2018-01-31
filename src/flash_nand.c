/************************************************************************
 *
 *      FLASH_SPI.C
 *
 *      The 'FLASH_SPI' module implements the FLASH device driver
 *      interface to be used via 'IO' device driver services:
 *
 *        1) init  device:  configure and initialize FLASH driver
 *        2) open  device:  not used
 *        3) close device:  not used
 *        4) read  device:  not used
 *        5) write device:  write FLASH device
 *        6) ctrl  device:  a) ERASE_SYSTEMFLASH
 *                          b) ERASE_FILEFLASH
 *
 *
 * ######################################################################
 *
 * mips_start_of_legal_notice
 * 
 * Copyright (c) 2003 MIPS Technologies, Inc. All rights reserved.
 *
 *
 * Unpublished rights (if any) reserved under the copyright laws of the
 * United States of America and other countries.
 *
 * This code is proprietary to MIPS Technologies, Inc. ("MIPS
 * Technologies"). Any copying, reproducing, modifying or use of this code
 * (in whole or in part) that is not expressly permitted in writing by MIPS
 * Technologies or an authorized third party is strictly prohibited. At a
 * minimum, this code is protected under unfair competition and copyright
 * laws. Violations thereof may result in criminal penalties and fines.
 *
 * MIPS Technologies reserves the right to change this code to improve
 * function, design or otherwise. MIPS Technologies does not assume any
 * liability arising out of the application or use of this code, or of any
 * error or omission in such code. Any warranties, whether express,
 * statutory, implied or otherwise, including but not limited to the implied
 * warranties of merchantability or fitness for a particular purpose, are
 * excluded. Except as expressly provided in any written license agreement
 * from MIPS Technologies or an authorized third party, the furnishing of
 * this code does not give recipient any license to any intellectual
 * property rights, including any patent rights, that cover this code.
 *
 * This code shall not be exported or transferred for the purpose of
 * reexporting in violation of any U.S. or non-U.S. regulation, treaty,
 * Executive Order, law, statute, amendment or supplement thereto.
 *
 * This code constitutes one or more of the following: commercial computer
 * software, commercial computer software documentation or other commercial
 * items. If the user of this code, or any related documentation of any
 * kind, including related technical data or manuals, is an agency,
 * department, or other entity of the United States government
 * ("Government"), the use, duplication, reproduction, release,
 * modification, disclosure, or transfer of this code, or any related
 * documentation of any kind, is restricted in accordance with Federal
 * Acquisition Regulation 12.212 for civilian agencies and Defense Federal
 * Acquisition Regulation Supplement 227.7202 for military agencies. The use
 * of this code by the Government is further restricted in accordance with
 * the terms of the license agreement(s) and/or applicable contract terms
 * and conditions covering this code from MIPS Technologies or an authorized
 * third party.
 *
 * 
 * mips_end_of_legal_notice
 * 
 *
 ************************************************************************/

//note :
//something need to be correct to adapt to our system
// #define PAGE_TMP_ADDR		0xa3000000
//if ( n_device[idx].id == chipid ) id is 16bit & chipid is 32bit 
//read_id_len need to correct :5 or 6 
//static n_device_type n_device[DEV_SIZE]  table need to correct 
//NAND_BOOTCODE_AREA_12MB and so on 
//if (FLASH_NAND_get_block_state(p_param->adr, &state)) //!!!!!!!!,THIS FOR LOOP IS INVALID 


/************************************************************************
 *      Include files
 ************************************************************************/

//#include <sysdefs.h>
//#include <syserror.h>
//#include <sysdev.h>
//#include <mips.h>
//#include <io_api.h>
//#include <syscon_api.h>
//#include "Flash_nand_function.h"
#include "flash_api.h"
#include "flash_nand_reg.h"
#include "nf_reg_qy.h"
#include <rlx_types.h>
#include "bsp.h"


typedef unsigned long long		   INT64U;
#define Mem_VPTrans                    0x80000000
#define OK   0



//#include <project_config.h>


/************************************************************************
 *      Definitions
 ************************************************************************/

enum trans_mode{
	TRANS_CMD  = 0x80,
	TRANS_ADDR = 0x81,
	TRANS_SBR  = 0x82,	
	TRANS_MBW  = 0x83,	//Multi Byte Write
	TRANS_MBR  = 0x84	//Multi Byte Read
};



/* Identification tags for memory devices */
typedef enum flash_device_id
{
	FLASH_SYSTEMFLASH_DEVICE = 0,
	FLASH_MONITORFLASH_DEVICE,
	FLASH_FILEFLASH_DEVICE,
	FLASH_BOOT_DEVICE,
	FLASH_UNKNOWN_DEVICE
} t_flash_device_id ;



#define TOSHIBA_512Mbit 		0x9876 
#define SAMSUNG_1Gbit   		0xECD3
#define SAMSUNG_256Mbit_SBK		0xEC75
#define SAMSUNG_512Mbit_SBK		0xEC76
#define HYNIX_2Gbit				0xADDA
#define HYNIX_8Gbit				0xADDC
#define HYNIX_512Mbit_SBK		0xAD76
#define HYNIX_8Gbit_MLC_57nm	0xADD3



/************************************************************************
 *  Macro Definitions
*************************************************************************/
	
/* set chip select: 0xbc00130[4:0] ,low enable */
#define NF_CHIP_SEL(x)	( (~(1<<(x))) & 0x1f )

#define  WRITE_REG_INT32U(reg_address,reg_to_set)	*((volatile INT32U *)reg_address)=reg_to_set

#define  READ_REG_INT32U(reg_address)  *((volatile INT32U*) reg_address)



//because these is no malloc function ,use array to subtuite
#define MAX_PAGESIZE		2048
#define MAX_BLOCK_PERFLASH  4096   //512MB flash 
#define MAX_BLKSTATE_LEN	MAX_BLOCK_PERFLASH  //total 1024 block   =  blocks_per_flash!!!!!!!
#define MAX_BBT_LENGTH      MAX_BLOCK_PERFLASH*RBA_PERCENTAGE/100

/*
*	Chip ID list
*
*	Name. ID code, pagesize, chipsize in MegaByte, eraseblock size,
*	options
*
*	Pagesize; 0, 256, 512
*	0	get this information from the extended chip ID
+	256	256 Byte page size
*	512	512 Byte page size
*/
const struct nand_flash_id nand_flash_ids[] = {
	
	{0x36, 512, 64, 0x4000},//"NAND 64MiB 1,8V 8-bit",	
	{0x76, 512, 64, 0x4000},//"NAND 64MiB 3,3V 8-bit",	

	{0x78, 512, 128, 0x4000},//"NAND 128MiB 1,8V 8-bit",	
	{0x39, 512, 128, 0x4000},//"NAND 128MiB 1,8V 8-bit",	
	{0x79, 512, 128, 0x4000},//"NAND 128MiB 3,3V 8-bit",	

	{0x71, 512, 256, 0x4000},//"NAND 256MiB 3,3V 8-bit",

	/*
	 * These are the new chips with large page size. The pagesize and the
	 * erasesize is determined from the extended id bytes
	 */

	/*512 Megabit */
	{0xA2, 0,  64, 0},//"NAND 64MiB 1,8V 8-bit",	
	{0xF2, 0,  64, 0},//"NAND 64MiB 3,3V 8-bit",	

	/* 1 Gigabit */
	{0xA1, 0, 128, 0},//"NAND 128MiB 1,8V 8-bit",	
	{0xF1, 0, 128, 0},//NAND 128MiB 3,3V 8-bit",	

	/* 2 Gigabit */
	{0xAA, 0, 256, 0},//"NAND 256MiB 1,8V 8-bit"
	{0xDA, 0, 256, 0},//"NAND 256MiB 3,3V 8-bit",	

	/* 4 Gigabit */
	{0xAC, 0, 512, 0},//"NAND 512MiB 1,8V 8-bit",	
	{0xDC, 0, 512, 0},//"NAND 512MiB 3,3V 8-bit",	

	/* 8 Gigabit */
	{0xA3, 0, 1024, 0},//"NAND 1GiB 1,8V 8-bit",	
	{0xD3, 0, 1024, 0},//"NAND 1GiB 3,3V 8-bit",	

	/* 16 Gigabit */
	{0xA5, 0, 2048, 0},//"NAND 2GiB 1,8V 8-bit",	
	{0xD5, 0, 2048, 0},//"NAND 2GiB 3,3V 8-bit",

	{NULL},
};

/************************************************************************
 *      Public variables
 ************************************************************************/
//char char_temp[20];//char buffer for UART putout


/************************************************************************
 *      Static variables
 ************************************************************************/

static INT32U nf_edo_state;		// nf controller edo setting

#define DATA_ALL_ONE	1		// read one page and all bit is '1'

n_device_type devicetype;
INT32U bytes_per_page;
INT32U pages_per_block;
INT32U blocks_per_flash;
INT32U pages_per_chip;
INT32U total_chip_num;
INT32U die_size;

blk_st_t *blk_state;			// bootcode block state array
INT32U blk_state_len=MAX_BLKSTATE_LEN;	// length of block state array .test by lyc 
BB_t *bbt = NULL;				// pointer to start of bad block table
INT32U BBT_LENGTH;
INT8U bbt_space[MAX_BBT_LENGTH*sizeof(BB_t)];


/************************************************************************
 *      Static function prototypes
 ************************************************************************/
INT32S nf_read_page(n_device_type *device, INT8U chip_sel, INT32U page_no, INT8U *buf);
static INT32S nf_write_page( n_device_type *device, INT8U chip_sel, INT32U page_no, INT8U *buf);
static INT8S nf_oneblk_state(n_device_type *device, INT32U start_block, INT32U offset,INT8U *state);
//static INT8S  nf_read_to_PP(n_device_type *device, INT32U page_no);
static INT8S  nf_erase_block(n_device_type *device, INT32U block);
static INT8S  nf_read(n_device_type *device, INT32U start_page, INT8U *buf, INT32U size);
 INT8S  nf_write(n_device_type *device, INT32U start_page, INT8U *buf, INT32U size);
static INT8U nf_get_spare(INT8S mem_region, INT32U *spare, INT32U offset);
static INT8U nf_set_spare(INT32U spare, INT32U offset);
 void  nf_read_id(INT32U DDR_BASE, INT8U size);
static void  nf_map_phys_page(n_device_type *device, INT32U page_no, INT32U *chip_sel, INT32U *phys_addr);
static INT8U nf_spare_ddr_enabled(void);
static INT32S init_bbt(n_device_type *device);
static void remapped_bbt(INT32U *chip_select, INT32U *block_no);
static void check_build_bbt(void *dev) ;
static INT32S load_bbt(n_device_type *device);
static void write_bbt(void *dev);
static int nf_BB_handle( n_device_type *device, INT32U current_block_no);


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          FLASH_NAND_init
 *  Description :
 *  -------------
 * This service initializes the FLASH driver to handle all
 * FLASH devices on this board environment.
 *  
 *
 *  Parameters :
 *  -----------
 * n_device_type* type 
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
INT32S FLASH_NAND_init(n_device_type* type )  ;



INT32S FLASH_NAND_read( t_FLASH_read_descriptor *p_param ) ;  /* IN: write data       */


/************************************************************************
 *
 *                          FLASH_NAND_write
 *  Description :
 *  -------------
 *  This service writes data into a specified address location, which
 *  can be in either SYSTEM-FLASH or FILE-FLASH space. Default is RAM.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'p_param',   IN,    variable of type, t_FLASH_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         data has been stored
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *  ERROR_FLASH_INVALID_ADDRESS,        Physical address not impl.    
 *
 *
 ************************************************************************/

INT32S FLASH_NAND_write(t_FLASH_write_descriptor *p_param ) ; /* IN: write data       */



INT32S FLASH_NAND_erase( t_FLASH_erase_descriptor *p_param );





/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


void WAIT_DONE(volatile INT32U *addr, INT32U mask, INT32U value){
	int n=0;

	
	while ( ((*addr)& mask) != value ){
  	if(n++>0x10000){
			//printk("Time out \n");
			#ifdef UART_DEBUG
				uart_write_str("Time out \n");
			#endif
							
			return;      
     }   
			asm("nop");
	}
}	


/*reset controller &NAND flash IC*/
void RESET(void){
	 //soft rest NF_controller
	WRITE_REG_INT32U(NF_SOFT_RST_reg,NF_SOFT_RST_reg_rst_ena(1)); 
	WAIT_DONE((volatile INT32U *)NF_SOFT_RST_reg,0x01,0x0);

	WRITE_REG_INT32U(NF_PD_reg, 0x1f);			//disable flash CS0
	WRITE_REG_INT32U(NF_BLANK_CHK_reg, NF_BLANK_CHK_blank_ena(1));	

	//set timing ...
	WRITE_REG_INT32U(NF_TIME_PARA3_reg,  NF_TIME_PARA3_T3(0x0));			//Set flash timming T3
	WRITE_REG_INT32U(NF_TIME_PARA2_reg,  NF_TIME_PARA2_T2(0x0));			//Set flash timming T2
	WRITE_REG_INT32U(NF_TIME_PARA1_reg,  NF_TIME_PARA1_T1(0x0));			//Set flash timming T1

	//Set ECC
	WRITE_REG_INT32U(NF_MULTI_CHNL_MODE_reg, NF_MULTI_CHNL_MODE_ecc_no_check(0) | NF_MULTI_CHNL_MODE_ecc_pass(0) 
	| NF_MULTI_CHNL_MODE_no_wait_busy(0) | NF_MULTI_CHNL_MODE_edo(0));				//Set HW no check ECC, no_wait_busy

	
	WRITE_REG_INT32U(NF_PD_reg, 0x1e);								//Enable flash CS0

	//WRITE_REG_INT32U(NF_DELAY_CTL_reg, 0x80);						//check ready/busy pin delay 
	
	
	WRITE_REG_INT32U(NF_ND_CMD_reg,  NF_ND_CMD_cmd(0xFF));				        //Set read data CMD1 (0x00)
	WRITE_REG_INT32U(NF_ND_CTL_reg, NF_ND_CTL_xfer(1) | NF_ND_CTL_tran_mode(0x0));	//Trigger CMD xfer
	WAIT_DONE((volatile INT32U *)NF_ND_CTL_reg, 0x80, 0x0);					//Wait xfer done	
	WAIT_DONE((volatile INT32U *)NF_ND_CTL_reg, 0xC0, 0x40);				//Wait die Ready	
}



/************************************************************************
 *
 *                          FLASH_NAND_init
 *  Description :
 *  -------------
 *  This service initializes the FLASH_SPI driver.
 *
 * processor :1) read ID & compare to vendor's device table 
 *		      2)...
 *			3) load/bulid bbt & every block 's state 
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'p_param',   INOUT, not used
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/

INT32S FLASH_NAND_init(n_device_type* type )    
{

	
	INT8U   i;
	INT8U 	ecc_mode_select ;
	INT8U	flash_id[5];
	
	RESET();

	
		// set ECC bit level
	/*ecc mode select from Pin,through read  */
	REG32(REG_Chip_En) = 0x1f; //disable ce
	//XFER_READ_ID(&ecc_mode_select,1);
	nf_read_id(&ecc_mode_select,1);
	REG32(REG_Chip_En) = 0x1e; //enable  ce
	
	//REG32(REG_CTL)=0x82;
	//while( REG32(REG_CTL) & 0x80 )
	//	;
	//switch ((REG32(REG_DATA)>>4)&0x03){
	switch((ecc_mode_select>>4)&0x03){
		case 0:type->ecc_bit=6;
				REG32(REG_ECC_SEL)=0x0;
				break;
		case 1:type->ecc_bit=12;
				REG32(REG_ECC_SEL)=0x01;
				break;
		case 2:type->ecc_bit=24;
				REG32(REG_ECC_SEL)=0x02;
				break;
		default: type->ecc_bit=6;
				REG32(REG_ECC_SEL)=0x0;
				break;
	}		
	

	//XFER_READ_ID(&flash_id[0],5);
	nf_read_id(&flash_id[0], 5);   //EF F1 80 95 00

	#ifdef UART_DEBUG
		uart_write_str("Flash ID = ");
	
		for(i=0;i<5;i++){
			itoa_self(flash_id[i],char_temp,16);
			uart_write_str(char_temp);
			uart_write_str("  ");
		}

		uart_write_str("\r\n");
	#endif

	/* Read device IDs */
	//dev_id = flash_id[1];



	
	
	/* Lookup the flash id */
	for (i = 0; nand_flash_ids[i].id != NULL; i++) {
		if (flash_id[1] == nand_flash_ids[i].id) {
			type->id =flash_id[1];
			type->PageSize=nand_flash_ids[i].pagesize;
		    type->size= (INT64U)nand_flash_ids[i].chipsize<< 20;
			type->BlockSize=nand_flash_ids[i].erasesize;

			//address mode set 
			switch(nand_flash_ids[i].chipsize){
				case 64:	//512Mbit
				case 128:	//1Gbit
					type->addr_mode_rw=3; //2byte cl,2byte row
					type->addr_mode_erase=6; //2byte row 
					break;
				case 256:	//2Gbit
				case 512:	//4Gbit
				case 1024:	//8Gbit
					type->addr_mode_rw=1; //2byte cl,3byte row
					type->addr_mode_erase=4;//3byte row
				break;
				default : break;
				
				
			}
			break;
		}
	}
	
	
	
	/* Newer devices have all the information in additional id bytes */
	if (!type->PageSize) {

		/* The 3rd id byte holds MLC / multichip data*/
		type->num_chips = 1<<(flash_id[2]&0x03);
			
		/* Calc pagesize */
		type->PageSize = 1024 << (flash_id[3] & 0x3);
		
		/* Calc blocksize. Blocksize is multiples of 64KiB */
		type->BlockSize= (64 * 1024) << ((flash_id[3]>>4)& 0x03);

		
		
	} 
	
	// calculate block number per flash and page size per block
	bytes_per_page = type->PageSize;
	pages_per_block  = type->BlockSize / type->PageSize;
	die_size = type->size / type->num_chips;
	type->size=die_size;	//only one die we can handle 
	pages_per_chip   = die_size / bytes_per_page;
	blocks_per_flash = die_size / type->BlockSize;
	total_chip_num=1;

	

	// enable spare DMA to DDR (8B unit)
	//REG32(REG_SPR_DDR_CTL) = 0x20000000 | (((INT32U)nf_spare_dma_base) >> 3); //!
	//	REG8((nf_spare_dma_base|0x80000000)+i)=0x5a;
	//}

	//REG32(REG_SPR_DDR_CTL) = 0x0;	


	//nf_read_to_PP(type,0);
	//nf_read_to_PP(type,0);
	//nf_erase_block(type,0);
	//READ_PP(0, 0, VA2PA(PAGE_READ_TMP_ADDR)>>3, 2048/512, 6, 0);
	//nf_read_page(type, 0, 0, PAGE_READ_TMP_ADDR);

	//nf_read(type, 0,PAGE_READ_TMP_ADDR, 2048);
	
	//for(i=0;i<32;i++){
	//	REG8((nf_spare_dma_base|0x80000000)+i)=0x5a;
	//}
	//nf_write(type, 0,PAGE_TMP_ADDR , 2048);
	
	//for(i=0;i<32;i++){
	//	REG8((nf_spare_dma_base|0x80000000)+i)=0x12;
	//}
	//nf_read(type, 0,PAGE_READ_TMP_ADDR, 2048);

	//nf_read_to_PP(type,0);

	//nf_read(type, 0,PAGE_READ_TMP_ADDR, 2048);

	// enable spare DMA to DDR (8B unit)
	REG32(REG_SPR_DDR_CTL) = 0x20000000 | ((((INT32U)nf_spare_dma_base) & (~0x80000000)) >> 3); //!
	
#ifndef ROM_COMPLIE	
    /* get bad block table info */
    check_build_bbt(type);
    /* check integrity of bootcode related blocks */
#else
	load_bbt(type);
#endif

    return( OK ) ;
}


/************************************************************************
 *
 *                          FLASH_NAND_read
 *  Description :
 *  -------------
 *  This service reads data from a specified address location, which
 *  can be in either SYSTEM-FLASH or FILE-FLASH space. 
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'p_param',   IN,    variable of type, t_FLASH_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         data has been stored
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *  ERROR_FLASH_INVALID_ADDRESS,        Physical address not impl.    
 *
 *
 ************************************************************************/

INT32S FLASH_NAND_read(t_FLASH_read_descriptor *p_param )
{
	INT32U page_addr, number_of_pages, i, blk_no;
	INT32S res, flag = OK;
	INT8U *ptr = p_param->buffer;
	INT32U rd_len;
	INT8U state = 0;
	INT8U ver;
	INT32U temp;
	INT16U idx;
	
	// validate parameters
	if ( (p_param == NULL) || (p_param->buffer == NULL) 
		|| (p_param->adr > blocks_per_flash))
	{
		return (!OK);
	}
	
	// no need to read
	if (p_param->length == 0)
		return (OK);
	
	blk_no = p_param->adr;

	rd_len = p_param->length;
	// align read length to page boundary
	if (rd_len & (devicetype.PageSize - 1))
	{
		rd_len = (rd_len / devicetype.PageSize + 1) * devicetype.PageSize;
	}
	
	res = nf_read(&devicetype, (blk_no * pages_per_block), p_param->buffer, rd_len);

	return (res == 0 ? (OK) : (!OK));
}


/************************************************************************
 *
 *                          FLASH_NAND_write
 *  Description :
 *  -------------
 *  This service writes data into a specified address location, which
 *  can be in either SYSTEM-FLASH or FILE-FLASH space. Default is RAM.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'p_param',   IN,    variable of type, t_FLASH_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         			data has been stored
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *  ERROR_FLASH_INVALID_ADDRESS,        Physical address not impl.    
 *
 *
 ************************************************************************/
#ifndef ROM_COMPLIE

INT32S FLASH_NAND_write(t_FLASH_write_descriptor *p_param )
{
  	INT32S res;
	INT32U i, start_blk, end_blk;
	INT8U state;
	INT32U chip_sel, page_offset;
	
	// validate parameters
	if ((p_param == NULL) || (p_param->buffer == NULL))
		return (!OK);
	
	// no need to write
	if (p_param->length == 0)
		return (OK);

	start_blk = p_param->adr;
	end_blk = start_blk + p_param->length / devicetype.BlockSize;
	if (p_param->length % devicetype.BlockSize)
		end_blk++;

	// cannot write beyond flash
	if ((start_blk >= blocks_per_flash) || (end_blk >blocks_per_flash))
		return (ERROR_FLASH_INVALID_ADDRESS);
		
	// check block state is clean to write
	//modify by lyc 12/29
	for (i = start_blk; i < end_blk; i++)		//!!!!!!!!,THIS FOR LOOP IS INVALID 
	{
		// calculate chip select & page physical address & remap the bad block 
		nf_map_phys_page(&devicetype, i * pages_per_block, &chip_sel, &page_offset);
//		if (FLASH_NAND_get_block_state(page_offset / pages_per_block, &state))
		nf_oneblk_state(&devicetype,page_offset / pages_per_block,0,&state);
		switch (state)
		{
			//there is no clean block to remap bad block ,so we will meet this state 
			case NAND_BLOCK_BAD:
				return (ERROR_FLASH_ERASE_ERROR);

			//these block are not erase before program ,so we will return this error 
			case NAND_BLOCK_HWSETTING:
			case NAND_BLOCK_BOOTCODE:
			case NAND_BLOCK_DATA:
			case NAND_BLOCK_ENVPARAM_MAGICNO:
			case NAND_BLOCK_OTHER_DATA:			//add by lyc 
				return (ERROR_FLASH_WRITE_PROTECTED);

			case NAND_BLOCK_CLEAN:
				break;
			
			default:
				return (ERROR_FLASH_VERIFY_ERROR);
		}
		
		
	}

		
	// set spare byte
	nf_set_spare(NAND_BLOCK_OTHER_DATA, 0);	//this spare byte is always valid until  next call  nf_set_spare 
	res = nf_write(&devicetype, start_blk * pages_per_block, p_param->buffer, p_param->length);
	if (res)
		return (!OK);
	
	// update block state table
	//modify by lyc 12/29 remap the block & updata state in remaped block 
//	for (i = start_blk; i < end_blk; i++)
//	{
//		nf_map_phys_page(&devicetype, i * pages_per_block, &chip_sel, &page_offset);
//		if (FLASH_NAND_set_block_state(page_offset/pages_per_block, NAND_BLOCK_OTHER_DATA) != OK)
//			return (ERROR_FLASH_INVALID_ADDRESS);

//	}
	
	return (OK);	
}

#endif

/************************************************************************
 *      Local helper functions
 ************************************************************************/



/************************************************************************
 *
 *                          FLASH_NAND_erase
 *  Description :
 *  -------------
 *  Erase flash area; i.e. the driver erases the flash blocks inside 
 *  the specified memory area.
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'p_param',   IN,    variable of type, t_FLASH_ctrl_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         			 FLASH erased succesfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *  ERROR_FLASH_INVALID_ADDRESS         Address area not inside FLASH devices             
 *
 *
 ************************************************************************/
#ifndef ROM_COMPLIE
INT32S FLASH_NAND_erase( t_FLASH_erase_descriptor *p_param )
{
	INT32U i, end;
	INT32S res1, res2;
	INT8U state;
	INT32U chip_sel;
	INT32U page_offset;
	
	if (p_param == NULL)
		return (ERROR_FLASH_PROGRAM_ERROR);
	
	// do not allow erase past end of flash
	end = p_param->adr + p_param->bank_num;
	if (end > blocks_per_flash)
		return (ERROR_FLASH_INVALID_ADDRESS);
	
	// similar with program ,hide the bad block to up-layer
	//modify by lyc ,12/29
	for (i = p_param->adr; i < end; i++)
	{
		//through remap , only erase good block ,and reserve the BB-mark
		nf_map_phys_page(&devicetype, i * pages_per_block, &chip_sel, &page_offset); //remap 
		
		//if  erase error happened ,remap it to a clean block 
		//this new clean block no need to erase again??? 
		res2 = nf_erase_block(&devicetype, page_offset / pages_per_block);
		if (res2 != 0){
			if(nf_BB_handle( &devicetype, page_offset/pages_per_block)) //add by lyc 12/29
				return (ERROR_FLASH_ERASE_ERROR);	//no free bank to remap  
			
		}

		// update block state table
//		FLASH_NAND_set_block_state(page_offset/pages_per_block, NAND_BLOCK_CLEAN);
	}
	
	return (OK);
}
#endif




/************************************************************************
 *
 *                          nf_get_spare
 *  Description :
 *  -------------
 *  get bytes from spare area
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'mem_region',  IN,       mapping region. (only 0x0 ~ 0xe) => 0x2 is through PP, while 0xe is through table SRAM
 *  'spare',   	   INOUT,    spare area bytes.
 *  'offset',	   IN,       offset bytes in spare area.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:  
 *
 *
 ************************************************************************/
static INT8U nf_get_spare(INT8S mem_region, INT32U *spare, INT32U offset)
{
	if (nf_spare_ddr_enabled()) {
		*spare = REG32(nf_spare_dma_base+ offset);
	}
	else {
		// configure spare area data in PP (16 byte: 6 byte for user-defined, 10 byte for ECC)
		REG32(REG_PP_RDY) = 0; // disable read_by_pp
		REG32(REG_SRAM_CTL) = 0x30 | mem_region; // enable direct access to PP or table SRAM
		
		*spare = REG32(REG_BASE_ADDR + offset); // set spare area of first PP
		
		REG32(REG_SRAM_CTL) = 0x0; // disable direct access
	}

	return 0;
}


/************************************************************************
 *
 *                          nf_set_spare
 *  Description :
 *  -------------
 *  set bytes info spare area
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'spare',   	IN,       spare area bytes.
 *  'offset',	IN,       offset bytes in spare area.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:  
 *
 *
 ************************************************************************/
static INT8U nf_set_spare(INT32U spare, INT32U offset)
{
	INT8U i, count;
	
	if (nf_spare_ddr_enabled()) {
		// spare/data: 6B/512B based
		count = devicetype.PageSize / 512;  //if pagesize is 2K,then count is 4,total spare data write to spare_dma_base ,8byte align
		for (i = 0; i < count; i++) {
			// (only 6 byte is for software)
			REG32((nf_spare_dma_base) + i*8) = spare;
			REG8((nf_spare_dma_base) + i*8 + 4) = 0xff;
			REG8((nf_spare_dma_base) + i*8 + 5) = 0xff;
		}
	}
	else {
		// configure spare area data in PP (16 byte: 6 byte for user-defined, 10 byte for ECC)
		REG32(REG_PP_RDY) = 0; // disable read_by_pp
		REG32(REG_SRAM_CTL) = 0x30 | 0x4; // enable direct access to PP #5
		
		// 64B spare and ECC data area ,total data is writen to PP
		for (i=0; i<0x40; i+=0x10) { 
			// (only first 6 byte is user-defined)
			REG32(REG_BASE_ADDR + offset + i) = spare; 
			REG8(REG_BASE_ADDR+4 + i) = 0Xff;
			REG8(REG_BASE_ADDR+5 + i) = 0Xff;
		}
		
		REG32(REG_SRAM_CTL) = 0x0; // disable direct access
	}

	return 0;
}


/************************************************************************
 *
 *                          nf_read_page
 *  Description :
 *  -------------
 *  read one page from NAND flash
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'device',   IN,    variable of type, n_device_type.
 *  'page_no',  IN,    source NAND page address.
 *  'buf',   	INOUT, destination DDR address.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00          : read page success  
 *  "-1"                 : means input argument has error
 *  "8"                  : means ECC is uncorrectable
 *  'DATA_ALL_ONE' = 0x1 : means all bits in the page is 1 (including spare area)
 *
 *
 ************************************************************************/
 INT32S nf_read_page(n_device_type *device, INT8U chip_sel, INT32U page_no, INT8U *buf)
{
	if ((device == NULL) || (buf == NULL)
		|| (page_no >= pages_per_block*blocks_per_flash))
		return (-1);

       	// flush dcache before DMA read
    rlx_dcache_flush( (INT32U *)buf, bytes_per_page);

	// Set chip select
	REG32(REG_Chip_En) = NF_CHIP_SEL(chip_sel);

	//Set data transfer count, data transfer count must be 0x200 at auto mode
	//Set SRAM path and access mode	
	REG32(REG_DATA_CNT1) = 0;	
	REG32(REG_DATA_CNT2) = 0x80|0x2;	// transfer mode, 0x200 bytes	
	//WRITE_REG_INT32U(NF_DATA_TL0_reg, 0x0);				//Set data transfer count[7:0]
	//WRITE_REG_INT32U(NF_DATA_TL1_reg, NF_DATA_TL1_access_mode(1) | NF_DATA_TL1_length1(2));	//Set data transfer count[13:8], unit is 512,SRAM path and access mode
	
	//Set page length at auto mode
	REG32(REG_PAGE_LEN) = device->PageSize >> 9;	//Set page length, unit = ( 512B + 16B)
	//WRITE_REG_INT32U(NF_PAGE_LEN_reg, device->PageSize >> 9);

	REG32(REG_BLANK_CHK) = 0x1;		// enable blank check

	//Set PP
	REG32(REG_PP_RDY) = 0x80;	//NAND --> PP --> DRAM
	REG32(REG_PP_CTL0) = 0x00; //this place must be 0
	REG32(REG_PP_CTL1) = 0;		
	//WRITE_REG_INT32U(NF_READ_BY_PP_reg, NF_READ_BY_PP_read_by_pp(1));	//Data read to DRAM from NAND through PP 		
	//WRITE_REG_INT32U(NF_PP_CTL0_reg, NF_PP_CTL0_pp_start_addr(0x0) | NF_PP_CTL0_pp_reset(0) | NF_PP_CTL0_pp_enable(0));				//Set PP starting address[9:8], PP reset and PP enable
	//WRITE_REG_INT32U(NF_PP_CTL1_reg, 0x0);				//Set PP starting assdress[7:0]

	//Set command
	REG32(REG_CMD1) = 0x00;	
	REG32(REG_CMD2) = 0x30;	
	REG32(REG_CMD3) = 0x70;		
		//Set command
	//WRITE_REG_INT32U(NF_ND_CMD_reg, 0x0);				//Set CMD1
	//WRITE_REG_INT32U(NF_CMD2_reg, 0x30);				//Set CMD2
	//WRITE_REG_INT32U(NF_CMD3_reg, 0x70);				//Set CMD3
	
	//Set address
	REG32(REG_PAGE_ADR0) =  (page_no & 0xff);
	REG32(REG_PAGE_ADR1) =  (page_no >> 8 ) & 0xff;
	REG32(REG_PAGE_ADR2) = ((page_no >>16 ) & 0x1f) | (device->addr_mode_rw << 5);
	REG32(REG_PAGE_ADR3) = ((page_no >> 21) & 0x7) << 5;
	REG32(REG_COL_ADR0)  = 0;
	REG32(REG_COL_ADR1)  = 0;
	//WRITE_REG_INT32U(NF_ND_PA0_reg, (( 0xff ) & ( 0>> 0 )));				//Set PA[7:0]
	//WRITE_REG_INT32U(NF_ND_PA1_reg, (( 0xff ) & ( 0>> 8 )));				//Set PA[15:8]
	//WRITE_REG_INT32U(NF_ND_PA2_reg, NF_ND_PA2_addr_mode(3) | NF_ND_PA2_page_addr2((( 0x1f ) & ( 0>> 16 ))));				//Set address mode & PA[20:16]
	//WRITE_REG_INT32U(NF_ND_PA3_reg, NF_ND_PA3_page_addr3((( 0x07 ) & ( 0>> 21 ))));				//Set PA[23:21]
	//WRITE_REG_INT32U(NF_ND_CA0_reg, (( 0xff ) & ( 0>> 0 )));				//Set CA[7:0]
	//WRITE_REG_INT32U(NF_ND_CA1_reg, (( 0xff ) & ( 0>> 8 )));				//Set CA[15:8]		
	
	//Set ECC
	REG32(REG_MultiChMod) = 0x0 | nf_edo_state;
	REG32(REG_ECC_STOP) = 0x80;	
	//WRITE_REG_INT32U(NF_MULTI_CHNL_MODE_reg, NF_MULTI_CHNL_MODE_ecc_no_check(0) 
	//	| NF_MULTI_CHNL_MODE_ecc_pass(0) | NF_MULTI_CHNL_MODE_edo(0));				//Set HW no check ECC
	//WRITE_REG_INT32U(NF_ECC_STOP_reg, NF_ECC_STOP_ecc_n_stop(1));				//Set ECC no stop

	//WRITE_REG_INT32U(NF_ECC_SEL_reg, 0x0);

		
	//Set spare
	 //Set spare 
	//REG32(REG_SPR_DDR_CTL) = 0x20000000 | (((INT32U)nf_spare_dma_base) >> 3);
	
	//Set DMA
	REG32(REG_DMA_ADR) = (((INT32U) (buf)) & ~Mem_VPTrans) >> 3;	// 8 bytes unit
	REG32(REG_DMA_LEN) = device->PageSize >> 9;	// 512 bytes unit
	REG32(REG_DMA_CONF) = 0x02|0x01;
	//WRITE_REG_INT32U(NF_DMA_CTL1_reg, 0x00040000>>3);				//Set DRAM start address
	//WRITE_REG_INT32U(NF_DMA_CTL2_reg, 4);				//Transfer length (Unit = 512B)
	//WRITE_REG_INT32U(NF_DMA_CTL3_reg, NF_DMA_CTL3_ddr_wr(1) | NF_DMA_CTL3_dma_xfer(1));	

	

	//Enable Auto mode
	REG32(REG_AUTO_TRIG) = 0x80|(device->PageSize == 512 ? 0x3 : 0x2);	//0x2: read in 2K page size
	while( REG32(REG_AUTO_TRIG) & 0x80 )
		;
	//WRITE_REG_INT32U(NF_AUTO_TRIG_reg, NF_AUTO_TRIG_auto_trig(1) | NF_AUTO_TRIG_spec_auto_case(0x0) | NF_AUTO_TRIG_auto_case(2));	//Set and enable auto mode 			
	//WAIT_DONE((volatile INT32U *)NF_AUTO_TRIG_reg, 0x80, 0x00);					//Wait auto mode done
	
	//Wait DMA done
	while( REG32(REG_DMA_CONF) & 0x1 );
	//WAIT_DONE((volatile INT32U *)NF_DMA_CTL3_reg, 0x01, 0x00);					//Wait auto mode done
	
	// return OK if all data bit is 1 (page is not written yet)
	if (REG32(REG_BLANK_CHK) & 0x2)
		return (DATA_ALL_ONE);

	// ECC error occurs but still correctable
	if (REG32(REG_ECC_STATE) & 0x4) 
		#ifdef UART_DEBUG
			uart_write_str("ECC error occurs but still correctable\r\n");
		#endif

	// poll ECC state (err_not_clr & ecc_err)
	if (REG32(REG_ECC_STATE) & 0x8) 
	{
		#ifdef UART_DEBUG
			uart_write_str("Uncorrectable HW ECC error, page number =0x");
			itoa_self(page_no,char_temp,16);
			uart_write_str(char_temp);
			uart_write_str("\r\n");
		#endif
		// debug
		return (REG32(REG_ECC_STATE) & 0x8);
	}
		
	return 0;
}


/************************************************************************
 *
 *                          nf_write_page
 *  Description :
 *  -------------
 *  write one page into NAND flash
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'device',   IN,    variable of type, n_device_type.
 *  'page_no',  IN,    destination NAND page address.
 *  'buf',   	IN,    source DDR address.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:  
 *
 *
 ************************************************************************/
#ifndef ROM_COMPLIE

static INT32S nf_write_page( n_device_type *device, INT8U chip_sel, INT32U page_no, INT8U *buf)
{
	INT32U temp;

	if ((device == NULL) || (buf == NULL)
		|| (page_no >= pages_per_block*blocks_per_flash))
		return (-1);
	
    // flush dcache before DMA write
    rlx_dcache_flush( buf, bytes_per_page);

	// Set chip select
	REG32(REG_Chip_En) = NF_CHIP_SEL(chip_sel);

	//Set data transfer count, data transfer count must be 0x200 at auto mode
	//Set SRAM path and access mode	
	REG32(REG_DATA_CNT1) = 0;	
	REG32(REG_DATA_CNT2) = 0x00|0x2;	// transfer mode, 0x200 bytes		
	
	//Set page length at auto mode
	REG32(REG_PAGE_LEN) = device->PageSize / 0x200;	//Set page length, unit = ( 512B + 16B)
	
	//Set PP
	REG32(REG_PP_RDY) = 0x0;	//NAND --> PP --> DRAM
	REG32(REG_PP_CTL0) = 0x0;
	REG32(REG_PP_CTL1) = 0;		
	
	
	//Set command
	REG32(REG_CMD1) = 0x80;	
	REG32(REG_CMD2) = 0x10;	
	REG32(REG_CMD3) = 0x70;	
	
	//Set address	
	REG32(REG_PAGE_ADR0) =  page_no & 0xff;
	REG32(REG_PAGE_ADR1) =  page_no >> 8 ;
	REG32(REG_PAGE_ADR2) = ((page_no >>16 ) & 0x1f) | (device->addr_mode_rw << 5);
	REG32(REG_PAGE_ADR3) = ((page_no >> 21) & 0x7) << 5;
	REG32(REG_COL_ADR0)  = 0;
	REG32(REG_COL_ADR1)  = 0;

	//Set ECC
	REG32(REG_MultiChMod) = 0x0 | nf_edo_state;
	REG32(REG_ECC_STOP) = 0x80;	
	
	//Set DMA
	REG32(REG_DMA_ADR) = (((INT32U) (buf)) & ~Mem_VPTrans) >> 3;	// 8 bytes unit
	REG32(REG_DMA_LEN) = device->PageSize / 0x200;	// 8*512 bytes
	REG32(REG_DMA_CONF) = 0x01;
	
	//Enable Auto mode
	REG32(REG_AUTO_TRIG) = 0x80|(device->PageSize == 512 ? 0x0 : 0x1);	//0x1: write in 2K page size
	while( REG32(REG_AUTO_TRIG) & 0x80 )
		;
	
	//Wait DMA done
	while( REG32(REG_DMA_CONF) & 0x1 );

	
	// execute command3 register and wait for executed completion
	REG32(REG_POLL_STATUS) = (0x6<<1) | 0x1; 
	while ( REG32(REG_POLL_STATUS) & 0x1)
		;

	temp = REG32(REG_DATA) & 0x1;
	
	if (temp == 0) {
		return 0;
	}

	
	//REG32(0xb801b200) = 0x58;	// (alan.tsai) print "X"
	return (-1);
}

#endif



/************************************************************************
 *
 *                          nf_oneblk_state 
 *  Description :
 *  -------------
 *  scan NAND flash to get block state from spare data 
 *  
 * V2.0 modify from  nf_find_blk, only get one block's state ,clean or bad.
 *  Parameters :
 *  ------------
 *  'start_block',  	IN,    start block to search
 *  'offset',       	IN,    page offset in the block
 *
 *  Return values :
 *  ---------------
 *  '-1': device is NULL or start_block beyond flash or not found
 *
 ************************************************************************/
static INT8S nf_oneblk_state(n_device_type *device, INT32U start_block, INT32U offset,INT8U *state)
{
	INT32U blk, limit, page_no;
	INT32U spare;
	INT8S res;
	INT8U ver;
	INT16U idx;
//	INT32U spr_ddr_ctrl;
	
	// validate arguments
	if ((device == NULL) || (start_block >= blocks_per_flash) || (offset >= pages_per_block))
		return (-1);
	
	//scan deeply the block  for the spare data 
	//if check data is allone ,spare=0xffffffff;
	//if ecc is ok, spare=read data from spare area.
	//if ecc is error,spare=bad blcok 
		// read first page of the block to table sram
		//res = nf_read_to_table(device, page_no, device->PageSize);	// read page to table SRAM	
		//res = nf_read_to_PP(device, start_block*pages_per_block+offset);   //该函数可读出厂时的bad block 标志
		res = nf_read_page(device,0,start_block*pages_per_block+offset,PAGE_RW_TMP_ADDR);
		switch (res)
		{
			case DATA_ALL_ONE:	// page is empty
				spare = 0xffffffff;
				break;
			
			case 0:		// read  success
				//nf_get_spare(0xe, &spare, 0);
				//because above nf_read_to_pp,spare data is in PP_Buffer
				//in this nf_find_blk function ,nf_spare_ddr_en has beed disable 
				nf_get_spare(0x4, &spare, 0);//0x04:pp buffer #5(2049~2176),
				break;
			
			default:	// read to table has error,include ECC error,
				spare = 0x00000000;
//				FLASH_NAND_set_block_state(blk, NAND_BLOCK_BAD);
//				continue;	// next block
		}
		


	
		
		// update with new magic no. in spare area
		//set_block_state(blk, (INT8U)(spare & 0xff));
		*state = spare & 0xff;
//		ver = (spare >> 8) & 0xff;
//		idx = (spare >> 16) & 0xffff;
//		FLASH_NAND_set_detail_block_state(blk, state, ver, idx);
	
	
	return (0);
}



/************************************************************************
 *
 *                          nf_read_to_PP
 *  Description :
 *  -------------
 *  read the 1st 2048B of page data from NAND flash into PP buffer
 *  (primary for get spare area byte)
 *  auto mode without DMA
 *
 *  vision: 2.0 ,change reading 1024B to 2048byte
 *
 *  Parameters :
 *  ------------
 * 
 *  'device',       	IN,    variable of type, n_device_type.
 *  'page_no',  	IN,    page address
 *
 *
 *  Return values :
 *  ---------------
 *
 *  "0"                  : means read page success (ECC is ok)
 *  "-1"                 : means device is NULL
 *  "DATA_ALL_ONE" = 0x1 : means all bits in the page is 1 (including spare area)
 *  "8"                  : means ECC is uncorrectable
 *
 *
 ************************************************************************/
/*
static INT8S nf_read_to_PP(n_device_type *device, INT32U page_no)
{
	INT32U chip_sel;
	INT8S res = 0;
	
	if (device == NULL)
		return (-1);
		
	nf_map_phys_page(device, page_no, &chip_sel, &page_no);

	// Set chip select
	REG32(REG_Chip_En) = NF_CHIP_SEL(chip_sel);

	//Set data transfer count, data transfer count must be 0x200 at auto mode
	//Set SRAM path and access mode	
	REG32(REG_DATA_CNT1) = 0;
	REG32(REG_DATA_CNT2) = 0x82;		// to PP; data tx count = 0x200
	
	//set page length at auto mode (512B unit)
	//REG32(REG_PAGE_LEN) = 2048 >> 9;
	REG32(REG_PAGE_LEN) = device->PageSize >> 9;
	
	REG32(REG_PP_RDY) = 0x80;		// set read_by_pp
	//REG32(REG_PP_CTL0) = 0;			// enable PP
	REG32(REG_PP_CTL1) = 0;			// PP starting address at 0
	

	
	REG32(REG_BLANK_CHK) = 0x1;		// enable blank check
	
	//set command
	REG32(REG_CMD1) = 0;
	REG32(REG_CMD2) = 0x30;
	REG32(REG_CMD3) = 0x70;
	
	//Set address	
	REG32(REG_PAGE_ADR0) =  page_no & 0xff;
	REG32(REG_PAGE_ADR1) =  page_no >> 8 ;
	REG32(REG_PAGE_ADR2) = ((page_no >>16 ) & 0x1f) | (device->addr_mode_rw << 5);
	REG32(REG_PAGE_ADR3) = ((page_no >> 21) & 0x7) << 5;
	REG32(REG_COL_ADR0)  = 0;
	REG32(REG_COL_ADR1)  = 0;
	
	//set ECC
	REG32(REG_MultiChMod) = 0;
	REG32(REG_ECC_STOP) = 0x80;		// non-stop
	
	//no DMA (data remains in PP)
	REG32(REG_DMA_CONF) = 0;

	//disable DMA spare data to DDR
	//REG32(REG_SPR_DDR_CTL) = 0x0;

	//Enable Auto mode
	REG32(REG_AUTO_TRIG) = 0x80 | (device->PageSize == 512 ? 0x3 : 0x2);	//0x2: read in 2K page size
	while( REG32(REG_AUTO_TRIG) & 0x80 )
		;
	
	// check if all data bit is 1 (data is not written yet)
	if (REG32(REG_BLANK_CHK) & 0x2)
		res = DATA_ALL_ONE;
	else 
		res = REG32(REG_ECC_STATE) & 0x8;	// get ECC result (ecc_not_clr)
	
	//Reset PP
	REG32(REG_PP_CTL0) = 0x2;
	
	return res;
}
*/

/************************************************************************
 *
 *                          nf_erase_block
 *  Description :
 *  -------------
 *  erase NAND flash block
 *  remap the bad block & erase the nand through set the NF_Controller's reg
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'device',       	IN,    variable of type, n_device_type.
 *  'block',	  	IN,    block no.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  "-1": means device is NULL
 *
 *
 ************************************************************************/

#ifndef ROM_COMPLIE

static INT8S nf_erase_block(n_device_type *device, INT32U block)
{
	INT32U page_addr, temp, chip_sel;

	if (device == NULL)
		return (-1);

	nf_map_phys_page(device, block * pages_per_block, &chip_sel, &page_addr);

	// Set chip select
	REG32(REG_Chip_En) = NF_CHIP_SEL(chip_sel);

	//Set command
	REG32(REG_CMD1) = 0x60;		//Set CMD1
	REG32(REG_CMD2) = 0xd0;		//Set CMD2
	REG32(REG_CMD3) = 0x70;		//Set CMD3		

	//Set address
	//note. page_addr[5:0] is ignored to be truncated as block
	REG32(REG_PAGE_ADR0) =  page_addr & 0xff;
	REG32(REG_PAGE_ADR1) =  page_addr >> 8;
	REG32(REG_PAGE_ADR2) = ((page_addr >>16 ) & 0x1f) | (device->addr_mode_erase << 5);
	REG32(REG_PAGE_ADR3) = ((page_addr >> 21) & 0x7) << 5;
	REG32(REG_COL_ADR0)  = 0;
	REG32(REG_COL_ADR1)  = 0;


	//Set ECC: Set HW no check ECC, no_wait_busy
	REG32(REG_MultiChMod) = 0x1 << 4 | nf_edo_state;
		
	//Enable Auto mode: Set and enable auto mode
	// and wait until auto mode done
	REG32(REG_AUTO_TRIG) = 0x8a;
	while ( REG32(REG_AUTO_TRIG) & 0x80)
		;
	
	//execute command3 register and wait for executed completion
	REG32(REG_POLL_STATUS) = (0x6<<1) | 0x1; 
	while ( REG32(REG_POLL_STATUS) & 0x1)
		;
	//Enable Auto mode read status 
	//REG32(REG_AUTO_TRIG) =0x82;				//Set and enable auto mode 			
	//while ( REG32(REG_AUTO_TRIG) & 0x80)	//Wait auto mode done
	//	;					
	

	temp = REG32(REG_DATA) & 0x1;
	
	if (temp == 0) {
		return 0;
	}else{
		#ifdef UART_DEBUG
			uart_write_str("Erase fail, page number =0x");
			itoa_self(page_addr,char_temp,16);
			uart_write_str(char_temp);
			uart_write_str("\r\n");
		#endif
	}
	
	return (-1);
}

#endif

/************************************************************************
 *
 *                          nf_read 
 *  Description :
 *  -------------
 *  read NAND flash
 *
 *  Parameters :
 *  ------------
 *  'device',       	IN,    variable of type, n_device_type.
 *  'start_page',  	IN,    start page address to read
 *  'buf', 	        IN,    pointer for buffer of data to read
 *  'size',       	IN,    number of bytes to read
 *
 *  Return values :
 *  ---------------
 *  '-1': device is NULL or read beyond flash or read failed
 *
 ************************************************************************/
static INT8S nf_read(n_device_type *device, INT32U start_page, INT8U *buf, INT32U size)
{
	INT32U stop_page;
	INT32S res;
	INT32U chip_sel, page_offset;
	INT32U page_counter;
	
	// validate arguments (size should be aligned to page size boundary)
	if ( (device == NULL) || (buf == NULL)
		|| (start_page > pages_per_block * blocks_per_flash)
		|| (size & (device->PageSize - 1))
		|| (size == 0) )
		return (-1);

	// do not allow read past end of flash
	stop_page = start_page + size / device->PageSize;
	if (stop_page > pages_per_block * blocks_per_flash)
		return (-1);
	
	// if start page is not block-alignment, remap it  
	if(start_page & (pages_per_block-1)){
		nf_map_phys_page(device, start_page, &chip_sel, &page_offset);
		page_counter=0;
	}
	
	while (start_page < stop_page)
	{
		// calculate chip select & page physical address,remap the bad block
		//remap only when current page is at the first page of block
		if((start_page & (pages_per_block-1))==0){
			nf_map_phys_page(device, start_page, &chip_sel, &page_offset);
			page_counter=0;
		}
		//printftest("page_no:%x, chip_sel:%x, page_offset:%x\n", start_page, (INT8U)chip_sel, page_offset);
		res = nf_read_page(device, (INT8U)chip_sel, page_offset+page_counter, buf);
		switch (res)
		{
			case DATA_ALL_ONE:	// page is clean
			case 0:
				break;
			
			default:
				return (-1);
		}
		
		buf += device->PageSize;
		start_page++;
		page_counter++;
	}
	return 0;
}


/************************************************************************
 *
 *                          nf_write 
 *  Description :
 *  -------------
 *  write  NAND flash 
 *  write one page by one page ,if meet bad blcok ,remap the block and write to remaped unused block
 *  if produce a bad block when writing,register this bad block to bbt and write to remapped unused block 
 *
 *  Parameters :
 *  ------------
 *  'device',       	IN,    variable of type, n_device_type.
 *  'start_page',  	IN,    start page address to write
 *  'buf', 	        IN,    pointer for buffer of data to be written
 *  'size',       	IN,    number of bytes to write
 *
 *  Return values :
 *  ---------------
 * ' 0': write OK 
 * '-1': device is NULL or write beyond flash or write failed
 *
 ************************************************************************/

#ifndef ROM_COMPLIE
 INT8S nf_write(n_device_type *device, INT32U start_page, INT8U *buf, INT32U size)
{
	INT32U stop_page;
	INT32S res;
	INT32U chip_sel, page_offset;
	INT32U page_counter;
	INT32U page_block_header;
	INT32U buf_block_header;
	
	// validate arguments (size should be aligned to page size boundary)
	if ( (device == NULL) || (buf == NULL)
		|| (start_page > pages_per_block * blocks_per_flash)
		|| (size & (device->PageSize - 1))
		|| (size == 0) )
		return (-1);

	// do not allow write past end of flash
	stop_page = start_page + size / device->PageSize;
	if (stop_page > pages_per_block * blocks_per_flash)
		return (-1);

	//if program from not-align-block address,try once remap
	//modify by lyc ,12/29
	if((start_page & (pages_per_block-1))){
		// calculate chip select & page physical address & remap the bad block 
		nf_map_phys_page(device, start_page, &chip_sel, &page_offset);
		page_counter=0;
		page_block_header= start_page;
		buf_block_header = buf;
	}
	//write one page by one page ,if meet bad block ,remap the block and write to remaped unused block
	//if produce a bad block when writing,register this bad block to bbt and write to remapped unused block 
	while (start_page < stop_page)
	{
		//only when current page is at new block ,calculate chip select & page physical address & remap the bad block 
		//record the current page and buf address ,they will be used when program error.
		//modify by lyc ,12/29
		if((start_page & (pages_per_block-1))==0){
			nf_map_phys_page(device, start_page, &chip_sel, &page_offset);
			page_counter=0;
			page_block_header= start_page;
			buf_block_header = buf;
		}
		
		res = nf_write_page(device, (INT8U)chip_sel, page_offset+page_counter, buf);
		//when program error,we should mark & remap this bad block & copy this block's data to new block 
		if (res) {
			// write 'BAD_BLOCK' signature to spare cell
			nf_BB_handle( device, start_page/pages_per_block);		
			buf = buf_block_header;
			start_page = page_block_header; 

			//not-align-block address solve
			if((start_page & (pages_per_block-1))){
				// calculate chip select & page physical address & remap the bad block 
				nf_map_phys_page(device, start_page, &chip_sel, &page_offset);
				page_counter=0;
			}
			
			continue;
			//return (-1);
		}	
		buf += device->PageSize;
		start_page++;
		page_counter++;
	}
	return 0;
}

#endif

/************************************************************************
 *
 *                          nf_read_id 
 *  Description :
 *  -------------
 * read id with manual mode 
 *
 *  Parameters :
 *  ------------
 *  DDR_BASE: the address to load the ID of nand flash 
 *  size :read data  byte size 
 *
 *  Return values :
 *  ---------------
 *  void 
 *
 ************************************************************************/

void nf_read_id(INT32U DDR_BASE, INT8U size)
{
	INT8U i;
				
	//Set data transfer count, data transfer count must be 0x200 at auto mode
	//Set SRAM path and access mode		
	REG32(REG_DATA_CNT1) = size;	// will read 5 bytes
	REG32(REG_DATA_CNT2) = 0x80;	// transfer mode
	
	//Set PP
	REG32(REG_PP_RDY) = 0;		// NOt through PP at manual mode 
	REG32(REG_PP_CTL0) = 0x01;	//PP enable ,just used for buffer 
	REG32(REG_PP_CTL1) = 0;		//PP start address to put the data 
	
	//Set command
	REG32(REG_CMD1) = 0x90;    //read ID command 
	//if( !trans_code(TRANS_CMD) )	while(1);
	REG32(REG_CTL) = 0x80;
	while (REG32(REG_CTL) & 0x80)
		;
	
	//Set address	 & address mode (only one address cycle )
	REG32(REG_PAGE_ADR0) = 0;
	REG32(REG_PAGE_ADR2) = 0x7<<5;
	//if( !trans_code(TRANS_ADDR) )	while(1);
	REG32(REG_CTL) = 0x81;
	while (REG32(REG_CTL) & 0x80)
		;
	
	//Enable MultiByteRead XFER mode
	//if( !trans_code(TRANS_MBR) )	while(1);
	REG32(REG_CTL) = 0x84;
	while (REG32(REG_CTL) & 0x80)
		;
	
	//Reset PP
	REG32(REG_PP_CTL0) = 0x2;
	
	//Move data to DRAM from SRAM,
	REG32(REG_SRAM_CTL) = 0x30;
	for ( i = 0 ; i < size ; i++){
		//temp = DDR_BASE * 8 + i * 4;
		REG8(DDR_BASE+i) = REG8(REG_PAGE_ADR0 + i);	//5 bytes value : EC D3 10 A6 64
	}	
	REG32(REG_SRAM_CTL) = 0x00;	//# no omitted
}



/************************************************************************
 *
 *                        nf_BB_handle
 *  Description :
 *  -------------
 * built for bad block handle  ，
 * 1) when this is the first remaped block ,then register in BBT,updata 1&2 block ,
 *	then next nf_map_phys_page(),remap the good block 
 * 2) when remaped block  also can't use register in BBT and write to 
 *	nand 's first &send block ,and write  related spare data,  
 *
 *  Parameters :
 *  ------------
 *  device:  n_device_type  
 *  current_block_no:the current write block which need to be remap.
 *
 *  Return values :
 *  ---------------
 *  0: handle OK
 *  -1:error,no free remap blk
 *
 ************************************************************************/

#ifndef ROM_COMPLIE

static int nf_BB_handle( n_device_type *device, INT32U current_block_no)
{
	INT32U page_no;
	INT32U cs;
	int i;

	page_no = current_block_no * pages_per_block;
		
	nf_map_phys_page(device, page_no, &cs, &page_no); //remaped block 
	
	/* there should be existed bbt */
	for (i = 0 ; i<BBT_LENGTH; i++)
	{
		if(bbt[i].bad_block == BB_INIT && bbt[i].remap_block !=RB_INIT)
		{				
			bbt[i].BB_die = cs;
			bbt[i].bad_block = page_no/pages_per_block;

			#ifdef UART_DEBUG
				uart_write_str("nf_BB_handle ok");
			#endif
			//printftest("i =0x%x BB_die=0x%x ", i, bbt[i].BB_die);
		    //printftest("bad_block=0x%x ", bbt[i].bad_block);
			//printftest("RB_die=0x%x ", bbt[i].RB_die);
			//printftest("remap_block=0x%x \n", bbt[i].remap_block);
			break;
			
		}
	}
	if (i == BBT_LENGTH) {
		#ifdef UART_DEBUG
			uart_write_str("BB_handle:no free remap blk\n");
		#endif
		return -1;	
	}	
	

	
	
	/* update BBT in nand flash */
	write_bbt(device);
	
	
	/* mark the block as BB */
	nf_set_spare(NAND_BLOCK_BAD, 0);
	//because this is a bad block ,so except spare area, other data in page can be any data 
	nf_write_page(device, cs, page_no, (INT8U *)PAGE_RW_TMP_ADDR);
	
	
	return 0;	
		
		
}

#endif


/************************************************************************
 *
 *                          bbt_exist
 *  Description :
 *  -------------
 * search backforward the unused block to fill the bbt's remap block preparely 
 *
 *  Parameters :
 *  ------------
 *  *start_blk: inout ,search the unused block from start_blk & return to the unused block no.   
 *
 *  Return values :
 *  ---------------
 * 0: remap OK
 * -1 :remap error  
 *
 ************************************************************************/

static int get_remap_block(INT32U *start_blk)
{
	INT8U state;
	INT32U ith,page_no;
	INT32U spare;
	INT8S res;
	//INT32U spr_ddr_ctrl;
	
	
	
	
	if (start_blk == NULL)
		return (-1);
	
	
	
	// cannot remap beyond flash
	if (*start_blk >= blocks_per_flash) {
		
		return (-1);
	}	

/*	
	if (nf_spare_ddr_enabled()) {
		spr_ddr_ctrl = REG32(REG_SPR_DDR_CTL);	// save original setting
		REG32(REG_SPR_DDR_CTL) &= ~0x20000000;	// turn off spare to DDR!
	}
	else
		spr_ddr_ctrl = 0;

*/

	// search backward , >=2 bbt blocks , I think
	for (ith = *start_blk; ith >= 2 ; ith--)
	{
		//FLASH_NAND_get_block_state(ith, &state);
		page_no = ith*pages_per_block;
				
		//res = nf_read_to_PP(&devicetype, page_no);
		res = nf_read_page(&devicetype,0,page_no,PAGE_RW_TMP_ADDR);
		
		
		switch (res)
		{
			case DATA_ALL_ONE:
				spare = 0xff;
				break;
			
			case 0:		// read to table success
				nf_get_spare(0x4, &spare, 0);
				spare &= 0xff;
				break;
					
			default:	// read to table has error
				#ifdef UART_DEBUG
					uart_write_str("// read to table has error");//hcy test
				#endif
				
				continue;	// next block
		}
		
				
		state = spare & 0xff; //get the block's state !!
		
		
		
		
		// found replaceable block
		if (state == NAND_BLOCK_CLEAN)
		{			
			*start_blk = ith;
			
//			if (spr_ddr_ctrl != 0)
//			REG32(REG_SPR_DDR_CTL) = spr_ddr_ctrl;	// restore to original setting
						
			
			return (0);
		}
	}

//	if (spr_ddr_ctrl != 0)
//		REG32(REG_SPR_DDR_CTL) = spr_ddr_ctrl;	// restore to original setting
						
	return (-1);
}



//transform the block number to phy block number
// do not need to calculate block offset in single chip case
static int map_phys_block(n_device_type *device, INT8U *chip_select, INT32U *phys_block)
{
	INT32U blocks_per_chip;
	INT8U i;
	
	// do not need to calculate block offset in single chip case
	if (total_chip_num == 1)
	{
		*chip_select = 0;
		return 0;
	}

	blocks_per_chip = pages_per_chip / pages_per_block;

	for (i = 0; i < total_chip_num; i++)
	{
		if (*phys_block < blocks_per_chip)
		{
			*chip_select = i;
			return 0;
		}
		
		*phys_block -= blocks_per_chip;
	}
	
	return -1;
}

/************************************************************************
 *
 *                          bbt_exist
 *  Description :
 *  -------------
 *  check if BBT exist in NAND FLASH block 1 & 2; 
 *
 *  Parameters :
 *  ------------
 *
 *  Return values :
 *  ---------------
 * 0: no bbt exist 
 * 1:bbt exit exist
 * -1 : no bbt exist & erase error  ,almostly not happen 
 *
 ************************************************************************/
#ifndef ROM_COMPLIE
static int bbt_exist(void *dev)
{
	INT32S res;
	INT8U state = 0;
	n_device_type *device = (n_device_type *)dev;

	
	// first check block BBT_1ST_BLK
	nf_oneblk_state(device,0,0,&state);
	//nf_oneblk_state(device,1,0,&state);
	
//	FLASH_NAND_get_block_state(BBT_1ST_BLK, &state);
	switch (state)
	{
		case NAND_BLOCK_CLEAN:	// block BBT_1ST_BLK is empty
			return (0);
		
		case NAND_BLOCK_BBT:	// block BBT_1ST_BLK has bad block table,bbt exist 
			return (1);
		
		case NAND_BLOCK_BAD:   //almostly not happen 
		default:
			// other block ,so erase it and return no  bbt exist 
			res = nf_erase_block(device, 0);
			if (res == 0)
				return (0);
			else
				#ifdef UART_DEBUG
					uart_write_str("erase block %d failed\n");
				#endif
			break;
	}
	
	// block BBT_1ST_BLK failed, check block BBT_2ND_BLK instead
//	FLASH_NAND_get_block_state(BBT_2ND_BLK, &state);
	nf_oneblk_state(device,1,0,&state);
	switch (state)
	{
		case NAND_BLOCK_CLEAN:	// block BBT_2ND_BLK is empty
			return (0);
		
		case NAND_BLOCK_BBT:	// block BBT_2ND_BLK has bad block table
			return (1);
		
		case NAND_BLOCK_BAD:
		default:
			res = nf_erase_block(device, 1);
			if (res == 0)
				return (0);
			else
				#ifdef UART_DEBUG
					uart_write_str("erase bbt block failed\n");
				#endif
			break;
	}

	return (-1);
}
#endif

/************************************************************************
 *
 *                          init_bbt
 *  Description :
 *  -------------
 *  initialize the bbt descrptions(get remap block for bbt struct ) in DRAM, mainly get the 
 *  unused remapable block 
 *
 *  Parameters :
 *  ------------
 *
 *  Return values :
 *  ---------------
 *
 ************************************************************************/
#ifndef ROM_COMPLIE
static int init_bbt(n_device_type *device)
{
	INT32U i, remap_idx;
	INT32U bbt_limit, bbt_size;
	INT8U cs;
	INT32U phys_offset;
	int rcode;
	
	bbt=(BB_t *)&bbt_space[0];
	
	// reset bad block table with initial value
	for (i = 0 ; i < BBT_LENGTH; i++)
	{
		bbt[i].BB_die = BB_DIE_INIT;
		bbt[i].bad_block = BB_INIT;
		bbt[i].RB_die = RB_DIE_INIT;
		bbt[i].remap_block = RB_INIT;
	}
	
	// fill the remap block field
	//the following is searching  backforward the unused block to fill the bbt's remap block preparely 
	remap_idx = blocks_per_flash - 1;
	bbt_limit = blocks_per_flash - BBT_LENGTH;
	for (i = 0; i < BBT_LENGTH; i++)
	{
		if (get_remap_block(&remap_idx)) {
			#ifdef UART_DEBUG
				uart_write_str("get_remap_block err \n");	
			#endif
			return (-1);		// cannot find remap block anymore
		}	
		//printftest("new remap_idx=0x%x \n", remap_idx);//hcy test
		
		// remapping block no. is limited to the last "BBT_LENGTH" blocks of flash
		if (remap_idx < bbt_limit)
			break;

		// calculate physical block offset and chip select
		phys_offset = remap_idx;
		if (map_phys_block(device, &cs, &phys_offset)) {
			#ifdef UART_DEBUG
				uart_write_str("map_phys_block \n");	
			#endif
			return (-1);
		}	
		//printftest("bbt[%d].RB_die=0x%x, bbt[%d].remap_block=0x%x \n",
		//	i, cs, i, phys_offset);//hcy test 
		bbt[i].RB_die = cs;
		bbt[i].remap_block = phys_offset;
		remap_idx--;
	}
	
	return 0;
}
#endif

/************************************************************************
 *
 *                          build_bbt
 *  Description :
 *  -------------
 *  scan bootcode area bad block (through check the block state)  ,then bulid into bbt in DRAM 
 *  processor: 1) check every block's state ;
 *			2) if meet bad error ,register to bbt with the bad block address ,
 *                     the remapable unused block in bbt is initialed in init_bbt();
 *
 *
 *  Parameters :
 *  ------------
 *
 *  Return values :
 *  ---------------
 *
 ************************************************************************/
#ifndef ROM_COMPLIE
static int build_bbt(void *dev)
{
	//int res;
	INT32U i, bbt_i;
	INT32U phys_offset;
	INT8U state = 0, cs;
	n_device_type *device = (n_device_type *)dev;
	
	// check for bad block from nand_bootcode_area_size ???
	//changed from 0 block
	for (i = 0, bbt_i = 0; 
		(i < blocks_per_flash) && (bbt_i < BBT_LENGTH); // stop when bad block table is full
		i++)
	{
		
//		if (FLASH_NAND_get_block_state(i, &state) !=OK)
//			break;
		nf_oneblk_state(device,i,0,&state);
	
		
		if (state == NAND_BLOCK_BAD)
		{
			// check remapping block is valid
			if (bbt[bbt_i].remap_block == RB_INIT)
				break;			// reach the end of valid table entry
			
			// calculate physical block offset and chip select
			phys_offset = i;
			if (map_phys_block(device, &cs, &phys_offset))
				return (-1);
			
			// register into bad block table,the bad block has be hook with bbt[] ,and can be remap to other block 
			bbt[bbt_i].BB_die = cs;
			bbt[bbt_i].bad_block = phys_offset;
			bbt_i++;

			#ifdef UART_DEBUG
				uart_write_str("this bad block is 0x  ");
				itoa_self(phys_offset,char_temp,16);
				uart_write_str(char_temp);
				uart_write_str("\r\n");
			#endif

			
		}
	}

	return (0);
}
#endif

/************************************************************************
 *
 *                          load_bbt
 *  Description :
 *  -------------
 *  load bbt to DRAM from NAND FLASH 
 *
 *  Parameters :
 *  ------------
 *
 *  Return values :
 *  ---------------
 *
 ************************************************************************/


static INT32S load_bbt(n_device_type *device)
{
	INT32U i, bbt_size;
	INT8U state[2] = {NAND_BLOCK_UNDETERMINE,NAND_BLOCK_UNDETERMINE};
	int rcode;

//    INT32U BBT_1ST_BLK= (parse_kernel_version())?1:0;
//    INT32U BBT_2ND_BLK= (parse_kernel_version())?2:1;
//	INT32U BBT_1ST_BLK = 0;
//	INT32U BBT_2ND_BLK = 1;
	
	for (i = 0; i < 2; i++)
	{
//		FLASH_NAND_get_block_state(i, &state);
		nf_oneblk_state(device,i,0,&state[i]);
		if (state[i] == NAND_BLOCK_BBT)
			break;
	}
	
	// bad block table not found (something wrong with building bad block table) 
	// or cannot get bad block table (cause read ECC failed)
	if (i == 2)
	{
		BBT_LENGTH = 0;
		bbt = NULL;
		return (-1);
	}
	
	BBT_LENGTH = blocks_per_flash * RBA_PERCENTAGE / 100;
	bbt_size = sizeof(BB_t) * BBT_LENGTH;
	
	// read bad block table to DRAM
	if (nf_read( device, i * pages_per_block, (INT8U *)PAGE_RW_TMP_ADDR, ((bbt_size / device->PageSize) + 1) * device->PageSize ))
	{
		return (-1);
	}
	
	/* allocate space for bad block table */
//	 bbt= (BB_t *)malloc(bbt_size);
	bbt=(BB_t *)&bbt_space[0];
/*
	mem.size     =  bbt_size;
	mem.boundary = 16;
	mem.memory   = (void*) &bbt;
	rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
	                         &mem,
	                         sizeof(t_sys_malloc) ) ;
	if (rcode != OK)
	{
	        return( rcode ) ;
	}
*/
	
	memset(bbt, 0xff, bbt_size);
	
	// copy bad block table from DRAM
	memcpy(bbt, (INT8U *)PAGE_RW_TMP_ADDR, bbt_size);
	
	return (0);
}
/************************************************************************
 *
 *                          write_bbt
 *  Description :
 *  -------------
 *  write bbt in DRAM to NAND FLASH  block 0 & 1
 *  processor:1) copy the bbt to some menory 
 *		      2)  erase the 1 & 2 block
 *		      3) program the data in memory to 1&2 block 
 *                 4)set above block state(BBT block )
 *  Parameters :
 *  ------------
 *
 *  Return values :
 *  ---------------
 *
 ************************************************************************/
#ifndef ROM_COMPLIE
	static 
	void write_bbt(void *dev)
	{
		INT32U idx, len, bbt_size;
		int res;
	
		INT32U BBT_1ST_BLK= 0;
		INT32U BBT_2ND_BLK= 1; 
		
		n_device_type *device = (n_device_type *)dev;
	
		// prepare memory space for bad block table (aligned to page boundary)
		bbt_size = sizeof(BB_t) * BBT_LENGTH;
		len = bbt_size / device->PageSize; //page num 
		if (bbt_size & (device->PageSize - 1))
			len++;
			
		if (len >= pages_per_block)
		{
			uart_write_str("bbt exceed one block\n");
			return;
		}
		
		len *= device->PageSize; //all page size 
		memset((INT8U *)PAGE_RW_TMP_ADDR, 0xff, len);
		memcpy((INT8U *)PAGE_RW_TMP_ADDR, bbt, bbt_size);
		//uart_write_str("write bbt \n");//hcy test		
		// write bad block table into block 0 & 1
		nf_set_spare(NAND_BLOCK_BBT, 0);
		for (idx = BBT_1ST_BLK; idx < BBT_2ND_BLK+1; idx++)
		{
			res = nf_erase_block(device, idx);
	
			if (res == 0)
			{
				if (nf_write(device, idx * pages_per_block, (INT8U *)PAGE_RW_TMP_ADDR, len))
				 #ifdef UART_DEBUG
					uart_write_str("write bbt failed\n");
				 #else
				 	;
				 #endif
			}
			else
			{
				 #ifdef UART_DEBUG
					uart_write_str("erase bbt failed\n");
				 #endif
	//			FLASH_NAND_set_block_state(idx, NAND_BLOCK_BAD);
			}
			//nf_read(device, 0,PAGE_READ_TMP_ADDR, 2048);
			//READ_PP(0, 0, VA2PA(PAGE_READ_TMP_ADDR)>>3, 2048, 6, 0);
			//READ_PP(0, 64, VA2PA(PAGE_READ_TMP_ADDR)>>3, 2048, 6, 0);
		} 
	}
#endif

/************************************************************************
 *
 *                          check_build_bbt
 *  Description :
 *  -------------
 *  check & build BBT
 *  processor: 1) check bbt wheather is exist
 			2)if not exist ,init bbt(build bbt array and find remapable block)
 						find every block's state,if meet eeror ,bulid to bbt
 						write  bbt to nand 1 &2 block
 			3) if exist ,load the bbt from nand 
 *
 *  Parameters :
 *  ------------
 *  dev
 *  Return values :
 *  ---------------
 *
 ************************************************************************/
#ifndef ROM_COMPLIE
static 
void check_build_bbt(void *dev)
{
	INT32S res;
	INT32U idx;
	INT32U ini_bb_offset;
	n_device_type *device = (n_device_type *)dev;
	//INT32U spare;
	INT8U status;
	int rcode;

	
	if (bbt_exist(dev) == 0 )
	{
		#ifdef UART_DEBUG
			uart_write_str("build bbt...");
		#endif	
		// reserve 5% blocks for remapable block  
		BBT_LENGTH = blocks_per_flash * RBA_PERCENTAGE / 100;
		if(BBT_LENGTH > MAX_BBT_LENGTH){
			BBT_LENGTH=MAX_BBT_LENGTH;
		}

		if (init_bbt(dev))
		{
			#ifdef UART_DEBUG
				uart_write_str("init_bbt failed\n");
			#endif
			return;
		}
		
		/* fill block state table */
		// search with non-exist magic no (this guarantees we can visit to the end of the bootcode blocks)
//		res = nf_find_blk(&devicetype, 0, blocks_per_flash, 0);

		
		 
		build_bbt(dev);
		
		write_bbt(dev);
		
		#ifdef UART_DEBUG
			uart_write_str("build bbt...finished");
		#endif
	}
	else {
		#ifdef UART_DEBUG
			uart_write_str("load bbt...");
		#endif
		
		load_bbt(dev);
	}	
		
}

#endif

/************************************************************************
 *
 *                          remapped_bbt
 *  Description :
 *  -------------
 *  remap bad block
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'chip_sel',  	INOUT,    chip select value
 *  'block_no',		INOUT,    bad/remap block no.
 *
 *
 *  Return values :
 *  ---------------
 *
 *
 *
 ************************************************************************/
static void remapped_bbt(INT32U *chip_sel, INT32U *block_no)
{
	INT32U i;
	INT32U cs, blk;
	
	// no remapping info (remain original chip_select & block_no)
	if (bbt == NULL)
		return;

	cs = *chip_sel;
	blk = *block_no;
	for (i = 0; i < BBT_LENGTH; i++)
	{
		// check if bad block entry is valid
		if (bbt[i].BB_die == BB_DIE_INIT)
			break;
		
		// update remapping block if found (may have recursive mapping)
		//if remaped bloack also is bad block ,then search the bbt until the remap block
		if ((bbt[i].BB_die == cs) && (bbt[i].bad_block == blk))
		{
			cs = bbt[i].RB_die;
			blk = bbt[i].remap_block;
			//printftest("[%s:%d] remap to (%x, 0x%x)\n", __FUNCTION__, __LINE__, cs, blk);
			continue;
		}
	}
	// return final remapping result
	*chip_sel = cs;
	*block_no = blk;
	
	return;
}

/************************************************************************
 *
 *                          nf_map_phys_page
 *  Description :
 *  -------------
 *  convert logical page address(page number) to physical page address (and chip select)
 *  if bbt is not available,return the original address 
 *  else if meet bad block, through the remapped_bbt() to remap the bloack & return new address 
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'page_no',		IN,     logical page address
 *  'chip_sel',  	OUT,    chip select value
 *  'page_addr',	OUT,    physical page address
 *
 *
 *  Return values :
 *  ---------------
 *
 *
 *
 ************************************************************************/
static void nf_map_phys_page(n_device_type *device, INT32U page_no, INT32U *chip_sel, INT32U *phys_addr)
{
	INT32U block_offset, page_offset;
	INT32U temp;

	// calculate chip select
	if (total_chip_num == 1)
	{
		*chip_sel = 0;	// only one possible chip select
		block_offset = page_no / pages_per_block;
		page_offset = page_no % pages_per_block;		
	}
	else
	{
		*chip_sel = page_no / pages_per_chip;
		temp = page_no % pages_per_chip;
		block_offset = temp / pages_per_block;
		page_offset = temp % pages_per_block;
	}
	
	// remap bad block  ,return the remaped unused block 
	remapped_bbt(chip_sel, &block_offset);
	
	// calculate physical pages address
	*phys_addr = block_offset * pages_per_block + page_offset;
	
	return;
}


//reg defination has been change 
static INT8U nf_spare_ddr_enabled(void)
{
	return ((REG32(REG_SPR_DDR_CTL) >> 29) & 0x1) ? 1 : 0;
	
}


INT8S flash_nand_rom_read()
{
         //t_FLASH_erase_descriptor param_erase;
         //t_FLASH_write_descriptor param_write;
         t_FLASH_read_descriptor param_read;
         INT32U i;
         INT32U* DDR_Base_r = 0x800c0000;
         INT32U rw_length = 120*1024;//120KB (sram1:0x800D7C00(33k)/stk:0x800CF000(35k)
         INT32U flash_addr = 8;//nand flash boot code is at block 8-7 
         int rcode;
		 
		 FLASH_NAND_init(&devicetype);
        
         //read
         param_read.adr=flash_addr;
         param_read.buffer=DDR_Base_r;
         //function will automatily to align to page address.
         param_read.length=rw_length;
         rcode=FLASH_NAND_read(&param_read);
         if (rcode){
                   //uart_write_str("FLASH_NAND_read error ");
                   return rcode;
         }
}


#ifndef ROM_COMPLIE


INT8S flash_nand_test(INT32U* DDR_Base_w, INT32U* DDR_Base_r,INT32U flash_addr, INT32U rw_length){
	t_FLASH_erase_descriptor param_erase;
	t_FLASH_write_descriptor param_write;
	t_FLASH_read_descriptor param_read;
	INT32U i;
	static  INT32U series=0;

	int rcode;

	//erase
	param_erase.adr= flash_addr;
	param_erase.bank_num=rw_length/(128*1024)+1;	 //erase >=1 block
	rcode=FLASH_NAND_erase( &param_erase );
	if (rcode){
		#ifdef UART_DEBUG
			uart_write_str("FLASH_NAND_erase error ");
		#endif
		
		return rcode;
	}


	for(i=0;i<(rw_length/4);i++){
		REG32(DDR_Base_w+i)=series;
		series++;
		
	}

	//write 
	param_write.adr=flash_addr;
	param_write.buffer=DDR_Base_w;
	param_write.length=rw_length;
	rcode=FLASH_NAND_write(&param_write);
	if (rcode){	
		#ifdef UART_DEBUG
			uart_write_str("FLASH_NAND_write error ");
		#endif
		return rcode;
	}

	
	//read 
	param_read.adr=flash_addr;
	param_read.buffer=DDR_Base_r;
	param_read.length=rw_length;
	rcode=FLASH_NAND_read(&param_read);
	if (rcode){
		#ifdef UART_DEBUG
			uart_write_str("FLASH_NAND_read error ");
		#endif
		
		return rcode;
	}
	
	//compare
	for(i=0;i<rw_length/4;i++){
		if(REG32(DDR_Base_w+i)!=REG32(DDR_Base_r+i)){
			#ifdef UART_DEBUG
				uart_write_str("FLASH_NAND_compare error ");
			#endif
			
			return (-1);
		}
	}
	
	return (0)	;

}

#endif 
