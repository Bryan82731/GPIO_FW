/*
 * Realtek Semiconductor Corp.
 *
 * Board Support Package header file
 *
 * Viller Hsiao (villerhsiao@realtek.com.tw)
 * Jan. 02, 2008
 */

#ifndef  _BSP_CFG_H_ 
#define  _BSP_CFG_H_

//Use DP or EP
//#define RTL8111EP

/*
 *******************************************************************************
 * Address mapping of BSP registers
 *******************************************************************************
 */
#define WRSIZE    128
#define NUMLOOP   1024
#define DMEMSIZE  131072 
//WRSIZE * NUMLOOP = 128 Kbytes

#define BSP_DCACHE_LINE_SIZE         4


/* IMEM & DMEM Configuration */
#define DMEM_BASE            0x00040000
#define DMEM_TOP             0x0007FFFF
#define DMEM_ON              0x00000400
#define DMEM_V_BASE          0x80040000
#define DMEM_V_TOP           0x8007FFFF

#define FLASH_CACHED_ADDR    0x82000000
#define FLASH_ADDR           0xA2000000
/*   Flash Controller            */
#define FLASH_BASE_ADDR      0xBC000000


#define FLASH_CTRLR0	     0x00
#define FLASH_NDF	     0x04
#define FLASH_SSIENR	     0x08
#define FLASH_RXFTLR	     0x1C
#define FLASH_SR    	     0x28
#define FLASH_IMR	     0x2C
#define FLASH_ISR	     0x30
#define FLASH_DR  	     0x60
#define FLASH_INT_TXEIS	     0x01
#define FLASH_INT_TXOIS      0x02
#define FLASH_INT_RXUIS      0x04
#define FLASH_INT_RXOIS      0x08
#define FLASH_INT_RXFIS      0x10
#define FLASH_INT_MSTIS      0x20
#define FLASH_READ_COM       0x03
#define FLASH_FAST_READ_COM  0x0B
#define FLASH_SE_COM         0x20
#define FLASH_BE_COM         0xD8
#define FLASH_CE_COM         0xC7
#define FLASH_WREN_COM       0x06
#define FLASH_WRDI_COM       0x04
#define FLASH_RDSR_COM       0x05
#define FLASH_WRSR_COM       0x01
#define FLASH_RDID_COM       0x9F
#define FLASH_REMS_COM       0x90
#define FLASH_RES_COM        0xAB
#define FLASH_PP_COM         0x02
#define FLASH_DP_COM         0xB9
#define FLASH_SE_PROTECT     0x36
#define FLASH_SE_UNPROTECT   0x39

#define MAC_BASE_ADDR        0xBAF70000
#define MAC_TPPoll           0x0030
#define MAC_CMD              0x0036

#define SECTOR         FLASH_SEC_ERA_COM
#define BLOCK          FLASH_BLK_ERA_COM
#endif
