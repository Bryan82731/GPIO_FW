#ifndef _FLASH_H
#define _FLASH_H

typedef struct {
	/* JEDEC id zero means "no ID" (most older chips); otherwise it has
	 * a high byte of zero plus three data bytes: the manufacturer id,
	 * then a two byte device id.
	 */
	INT32U	flash_ID;
	/* The size listed here is what works with OPCODE_SE, which isn't
	 * necessarily called a "sector" by the vendor which be called block .
	 */
	INT32U	sector_size;
	INT16U	n_sectors;	
	INT16U	addr_width;
	INT16U	flags;
} SPI_FLASH_STRUCT ;

extern SPI_FLASH_STRUCT flash_struct;
extern INT32U FLASH_ID;

#define spi_blk_erase(a,b) Flash_erase_block(b)
#define spi_se_erase(a,b)  Flash_erase_sector(b)
#define spi_read_id(a) Flash_RDID(&flash_struct)
#define spi_se_unprotect(a,b) Flash_Unprotect(b)
#define spi_write(ssimap1, Address, DReadBuffer, NDF) Flash_write_one_channel_User(NDF, Address, DReadBuffer)

//Flash Controller Register setting
//For 8111EP
#define FLASH_DATA_ADDR            	0xA2000000
#define FLASH_BASE_ADDR            	0xBC000000
//flash control register
#define FLASH_CTRLR0             	0x00
#define FLASH_CTRLR1                0x04
#define FLASH_SSIENR                0x08
#define FLASH_SER                   0x10
#define FLASH_BAUDR                 0x14
#define FLASH_RXFTLR                0x1C
#define FLASH_SR                    0x28
#define FLASH_IMR                   0x2C
#define FLASH_ISR                   0x30
#define FLASH_DR                    0x60
#define FLASH_READ_DUAL_ADDR_DATA	0xE8
#define FLASH_ADDR_CTRLR2         	0x110
#define FLASH_ADDR_LENGTH         	0x118
#define FLASH_AUTO_LENGTH         	0x11C
#define FLASH_VALID_CMD				0x120
#define FLASH_SIZE_CONTRL			0x124

//flash control register interrupt
#define FLASH_INT_TXEIS             0x01
#define FLASH_INT_TXOIS             0x02
#define FLASH_INT_RXUIS             0x04
#define FLASH_INT_RXOIS             0x08
#define FLASH_INT_RXFIS             0x10
#define FLASH_INT_MSTIS             0x20

//flash command
#define FLASH_READ_COM              0x03
#define FLASH_FAST_READ_COM     	0x0B
#define FLASH_SE_COM                0x20
#define FLASH_BE_COM                0xD8
#define FLASH_CE_COM                0xC7
#define FLASH_WREN_COM              0x06
#define FLASH_WRDI_COM              0x04
#define FLASH_RDSR_COM              0x05
#define FLASH_WRSR_COM              0x01
#define FLASH_RDID_COM              0x9F
#define FLASH_REMS_COM              0x90
#define FLASH_RES_COM               0xAB
#define FLASH_PP_COM                0x02
#define FLASH_DP_COM                0xB9
#define FLASH_SE_PROTECT            0x36
#define FLASH_SE_UNPROTECT          0x39
#define FLASH_CHIP_ERA              0x60
#define FLASH_CHIP_BLK              0x52//erase 32K block command

/* Used for Macronix flashes only. */
#define	OPCODE_EN4B		0xB7	/* Enter 4-byte mode */
#define	OPCODE_EX4B		0xE9	/* Exit 4-byte mode */
/* Used for Spansion flashes only. */
#define	OPCODE_BRWR		0x17	/* Bank register write */


#define FLASH_RDCR_COM				0x15
#define FLASH_WREAR_COM				0xC5
#define FLASH_RDEAR_COM				0xC8
#define FLASH_RSTEN_COM				0x66
#define FLASH_RST_COM				0x99

#define FLASH_SIZE					4*1024*1024/8
//#define FLASH_SIZE					32*1024*1024
#define FLASH_CHIP_SEC       0x20


#define FLASH_DEBUG	
//#define FPGA
void 		Flash_erase_all(void);
void 		Flash_erase_sector(INT32U Address);
void 		Flash_erase_block(INT32U Address);
void		Flash_write_one_channel_User(INT32U NDF, INT32U Address, INT32U *ReadBuffer);
void 		Flash_Init(void);
int 		Compare_Flash(INT32U *source, INT32U *destination, int size);
#endif 
