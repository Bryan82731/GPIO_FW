#ifndef FLASH_API_H
#define FLASH_API_H

/************************************************************************
 *
 *      FLASH_api.h
 *
 *      The 'FLASH_api' module defines the FLASH device driver
 *      interface to be used via 'FLASH' device driver services:
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




/************************************************************************
 *    Include files
 ************************************************************************/
#include "rlx_types.h"

//#include "sysdefs.h"
//#include "syserror.h"

/************************************************************************
 *   FLASH device driver, ERROR completion codes
*************************************************************************/

#define ERROR_FLASH                  0x00009000 /* for compile time check    */
#define ERROR_FLASH_PROGRAM_ERROR        0x9000 /* Flash device failure      */
#define ERROR_FLASH_INVALID_ADDRESS      0x9001 /* Physical address not impl.*/
#define ERROR_FLASH_INVALID_COMMAND      0x9002 /* Ctrl. Command not supported*/
#define ERROR_FLASH_TIME_OUT             0x9003 /* FLASH operation timed out */
#define ERROR_FLASH_VERIFY_ERROR         0x9004 /* Program error detected during verify */
#define ERROR_FLASH_LOCKED               0x9005 /* Some sector(s) locked     */
#define ERROR_FLASH_ERASE_ERROR          0x9006 /* Some sector(s) have erase error */
#define ERROR_FLASH_LOW_VOLTAGE          0x9007 /* Low programming voltage detected */
#define ERROR_FLASH_WRITE_PROTECTED      0x9008 /* Flash is write protected  */
#define ERROR_FLASH_FILE_FLASH_PROT      0x9009 /* Environment FLASH is write protected */
#define ERROR_FLASH_FILE_FLASH_LOCK      0x900a /* Environment FLASH is lock-bit protected */
#define ERROR_FLASH_MONITOR_FLASH_LOCK   0x900b /* MONITOR FLASH sector(s) locked */
#define ERROR_FLASH_QRY_NOT_FOUND        0x900c /* CFI Query-id string not found */
#define ERROR_FLASH_BOOT_WRITE_PROTECTED 0x900d /* Boot is write protected   */


/************************************************************************
 *   boot FLASH type (bit[1:0] in register 0xb8000304)
*************************************************************************/
#ifdef FPGA

#ifdef FPGA_BOOT_FROM_NAND
#define BOOT_FLASH_TYPE				BOOT_NAND
#else
#define BOOT_FLASH_TYPE				BOOT_NOR_SERIAL
#endif

#else

#define BOOT_FLASH_TYPE				((REG32(0xb801bd28) >> 12) & 0x1)

#endif /* #ifdef FPGA */

//#define BOOT_NOR_PARALLEL			0x0
#define BOOT_NOR_SERIAL				0x0
#define BOOT_NAND				0x1
//#define BOOT_PCI				0x3

////env area on DRAM
#define ENV_DDR_BASE				0xa0010000
#define ENV_SIZE				0x10000

/************************************************************************
 *   NAND FLASH specific
*************************************************************************/
#define NAND_BOOTCODE_AREA_12MB			0xc00000	// bootcode blocks in NAND flash (first 12MB)
#define NAND_BOOTCODE_AREA_30MB		   0x1e00000
#define NAND_BOOTCODE_AREA_50MB		   0x3200000
#define DEFAULT_NAND_BOOTCODE_SIZE    NAND_BOOTCODE_AREA_12MB

#define EXT_PARA_DDR_BASE			  0xa0a00000	// ext_para_ptr base address
#define FW_DSCPT_TABLE_BASE			  0xa2000000	// Firmware Description Table base address

#define NAND_FACTORY_SIZE				0x400000	// factory area size (right behind bootcode blocks) !!!!!!!


#define NAND_BLOCK_BAD               		0x00
#define NAND_BLOCK_HWSETTING         		0X23
#define NAND_BLOCK_BOOTCODE          		0x79
#define NAND_BLOCK_DATA              		0x80		// t_extern_param + logo + rescue
#define NAND_BLOCK_ENVPARAM_MAGICNO  		0x81
#define NAND_BLOCK_FACTORY_SETTING          0x82    	// factory setting tar file
#define NAND_BLOCK_OTHER_DATA				0xd0		// unknown data (user program into flash)
#define NAND_BLOCK_BBT						0xbb		// bad block table
#define NAND_BLOCK_CLEAN					0xff		// block is empty
#define NAND_BLOCK_UNDETERMINE				0x55		// block state is undetermined 
// (NOTE: BLOCK_UNDETERMINE is a fake state, all blocks in the flash SHALL NOT have this state or bootcode will get confused)

#define NAND_DEFAULT_SEARCH_DEPTH		   	 0x3
#define RBA_PERCENTAGE 						   5		// percentage of blocks reserved for bad block remapping
#define BB_INIT 						  0xFFFE		// initial bad_block value in bad block table
#define	RB_INIT							  0xFFFD		// initial remap_block value in bad block table
#define BB_DIE_INIT						  0xEEEE		// initial bad_block chip value in bad block table
#define RB_DIE_INIT					 BB_DIE_INIT		// initial remap_block chip value in bad block table



#define PAGE_RW_TMP_ADDR	0x80040000	// NAND temp page read/write address:need size 4096Byte 
#define nf_spare_dma_base	0x80041000  // spare to memory  address  need size :128Byte

#define PAGE_WRITE_ADDR		0x80050000  //64KB
#define PAGE_READ_ADDR		0x80060000

/* bad block table structure */
typedef struct  {
    unsigned short BB_die;		// chip number where bad block reside in
    unsigned short bad_block;		// block no. marked as bad
    unsigned short RB_die;		// chip number where remapping block reside in
    unsigned short remap_block;		// remapping block no.
} __attribute__ ((__packed__)) BB_t;

/* bootcode related block state structure */
typedef struct  {
    unsigned char  block_type;		// block type
    unsigned char  version_no;		// env parameter version no
    unsigned short page_idx;		// page index of data block
}__attribute__ ((__packed__)) blk_st_t;

#define DEFAULT_HWSETTING_BLOCK_COUNT		0X2	// number of HW setting block on NAND flash
#define DEFAULT_BOOTCODE_BLOCK_COUNT		0X2	// number of bootcode blocks on NAND flash
#define DEFAULT_DATA_BLOCK_COUNT		0X2	// number of "t_extern_param + logo + rescue" blocks on NAND flash


/************************************************************************
 *   FLASH 'ctrl' service, command codes
*************************************************************************/

typedef struct 
{
    unsigned int 	id ;
    unsigned short 	PageSize;
    unsigned int 	BlockSize;
    unsigned char   *string;
    unsigned int  	size;
	unsigned char 	read_id_len;
	unsigned char   id5;
	unsigned char   id6;
	unsigned char 	num_chips;
	unsigned char 	t3;
	unsigned char 	t2;
	unsigned char 	t1;
	unsigned char 	ecc_bit;        
	unsigned char 	addr_mode_rw;  //column address & page address cycle ,based on column size & page size 
	unsigned char   addr_mode_erase; //erase address mode  
} n_device_type;



typedef enum FLASH_ctrl_command_ids
{
    FLASH_CTRL_ERASE_SYSTEMFLASH = 0,      /* FLASH 'ERASE' SYSTEMFLASH */
    FLASH_CTRL_ERASE_FILEFLASH,            /* FLASH 'ERASE' FILEFLASH   */
    FLASH_CTRL_ERASE_FLASH_AREA,           /* FLASH 'ERASE' (partial) FLASH AREA */
    FLASH_CTRL_INQUIRE_FLASH_DEVICEID,     /* FLASH 'INQUIRE' committed */
    FLASH_CTRL_TEST_SYSTEMFLASH,           /* FLASH 'TEST' SYSTEMFLASH  */
    FLASH_CTRL_WRITE_FILEFLASH,            /* FLASH 'WRITE' FILEFLASH   */
    FLASH_CTRL_GET_ENV,		   	   		   /* FLASH 'GET' latest env variable */
    FLASH_CTRL_SAVE_ENV,		   		   /* FLASH 'SAVE' env variable */
    FLASH_CTRL_GET_FACTORY,		           /* FLASH 'GET' factory setting */
    FLASH_CTRL_SAVE_FACTORY,		       /* FLASH 'SAVE' factory setting */
    FLASH_CTRL_CLEAR_FACTORY,		       /* FLASH 'CLEAR' factory setting */
    FLASH_CTRL_TEST_MONITORFLASH,          /* FLASH 'TEST' MONITORFLASH */
    FLASH_CTRL_EXIT                        /* FLASH 'EXIT' */
} t_FLASH_ctrl_command_ids;


/************************************************************************
 *  Parameter definitions
*************************************************************************/


/*  FLASH write descriptor, to be used with FLASH-'write' service */
typedef struct FLASH_write_descriptor
{
    INT32U adr ;           /*bank num       */ 
    INT32U length ;        /* number of bytes to write                 */
    INT8U  *buffer ;       /* pointer for buffer of data to be written */
} t_FLASH_write_descriptor ;


/*  FLASH read descriptor, to be used with FLASH-'read' service */
typedef struct FLASH_read_descriptor
{
    INT32U adr ;         		 /* block number of nand flash;*/ 
    INT32U length ;        		/* number of bytes to read                 */
    INT8U  *buffer ;       		/* pointer for buffer of data to be read */
} t_FLASH_read_descriptor ;


/* FLASH ctrl descriptor, to be used with specific FLASH services */
typedef struct FLASH_erase_descriptor
{
    INT32U adr ;		  /* block adr   */ 
    INT32U bank_num ;   /*  block num */
} t_FLASH_erase_descriptor ;

//typedef struct {
//
//}t_sys_error_string;

//sys_dcache_flush_all();

//note this function !!!!!!!!!!!!!!!!!!!!!!!
//sys_sync


n_device_type devicetype;


#define MIPS_RESET_VECTOR		  0x1FC0000 

#define ROM_COMPLIE

/************************************************************************
 *   Public functions
 ************************************************************************/
//INT32S FLASH_PARELLEL_install( void ) ;
//INT32S FLASH_SPI_install( void );
//INT32S FLASH_Spansion_install( void );
//INT32S FLASH_install( void );
//INT32S FLASH_NAND_install( void );



//add by lyc 
INT32S FLASH_NAND_init(n_device_type* type )  ;



INT32S FLASH_NAND_read( t_FLASH_read_descriptor *p_param ) ; 


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
 * 'OK' = 0x00:                         			 data has been stored
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *  ERROR_FLASH_INVALID_ADDRESS,         Physical address not impl.    
 *
 *
 ************************************************************************/
INT32S FLASH_NAND_write(t_FLASH_write_descriptor *p_param ) ; 







/************************************************************************
 *
 *                          FLASH_NAND_ctrl
 *  Description :
 *  -------------
 *  This service comprise following specific FLASH services:
 * //   1) 'ERASE_SYSTEMFLASH'
 * //   2) 'ERASE_FILEFLASH'
 *    3) 'FLASH_CTRL_ERASE_FLASH_AREA'
 * //   4) 'FLASH_CTRL_INQUIRE_FLASH_AREA'
 * //   5) 'FLASH_CTRL_TEST_SYSTEMFLASH'
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'p_param',   IN,    variable of type, t_FLASH_ctrl_descriptor.
 *  	1) p_param->command: FLASH_CTRL_ERASE_FLASH_AREA--erase command;
 *    2) p_param->user_phyadr:erase block start no.;
 *    3) p_param->user_length:erase block total no.
 *
 *
 *  Return values :
 *  ---------------
 * 'OK' = 0x00:                         			 FLASH service completed successfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *  ERROR_FLASH_INVALID_ADDRESS          erase address is unvaluable 

 *
 ************************************************************************/

INT32S FLASH_NAND_erase(t_FLASH_erase_descriptor *p_param ) ; 


INT8S flash_nand_test(INT32U * DDR_Base_w,INT32U * DDR_Base_r,INT32U flash_addr, INT32U rw_length);


void nf_read_id(INT32U DDR_BASE, INT8U size);
INT8S nf_write(n_device_type *device, INT32U start_page, INT8U *buf, INT32U size);


extern char char_temp[20];//char buffer for UART putout





#endif /* #ifndef FLASH_API_H */
