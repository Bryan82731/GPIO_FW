#ifndef _8111FP_PCIE_HOST_H
#define _8111FP_PCIE_HOST_H

//#include "Rlx_types.h"
//#include "Bsp_cfg.h"

typedef unsigned char       	UCHAR;
typedef unsigned short      	USHORT;
typedef unsigned int			UINT;
typedef unsigned long       	ULONG;
typedef long					LONG;
typedef LONG *					LONG_PTR;


//typedef           void           VOID;
//typedef unsigned  char           BOOLEAN;
//typedef unsigned  char           INT8U;        /* Unsigned  8-bit quantity */
//typedef signed    char           INT8S;        /* Signed    8-bit quantity */
//typedef unsigned  short          INT16U;       /* Unsigned 16-bit quantity */
//typedef signed    short          INT16S;       /* Signed   16-bit quantity */
//typedef unsigned  int            INT32U;       /* Unsigned 32-bit quantity */
//typedef signed    int            INT32S;       /* Signed   32-bit quantity */

#define	BIT_0	0x1<<0
#define BIT_1	0x1<<1
#define BIT_2	0x1<<2
#define BIT_3	0x1<<3
#define BIT_4	0x1<<4
#define BIT_5	0x1<<5
#define BIT_6	0x1<<6
#define	BIT_7	0x1<<7
#define BIT_8	0x1<<8
#define BIT_9	0x1<<9
#define BIT_10	0x1<<10
#define BIT_11	0x1<<11
#define BIT_12	0x1<<12
#define BIT_13	0x1<<13
#define	BIT_14	0x1<<14
#define BIT_15	0x1<<15
#define BIT_16	0x1<<16
#define BIT_17	0x1<<17
#define BIT_18	0x1<<18
#define BIT_19	0x1<<19
#define BIT_20	0x1<<20
#define	BIT_21	0x1<<21
#define BIT_22	0x1<<22
#define BIT_23	0x1<<23
#define BIT_24	0x1<<24
#define BIT_25	0x1<<25
#define BIT_26	0x1<<26
#define BIT_27	0x1<<27
#define	BIT_28	0x1<<28
#define BIT_29	0x1<<29
#define BIT_30	0x1<<30
#define BIT_31	0x1<<31

#define PH_ERROR_TIMEOUT 0
#define PH_SUCCESS BIT_0
#define PH_ERROR_PAGEFULL BIT_1
#define PH_ERROR_WRONGVALUE BIT_2
#define PH_ERROR_PCIELINK_FAIL BIT_3

#define	Getbit(dat, i)		((dat&(i))?1:0)
#define Setbit(dat, i)		((dat)|=(i))
#define Clearbit(dat, i)	((dat)&=(~(i)))
#define PCIE_HOST 1


#if 0
#define dbg printf
#else
#define dbg
#endif


//#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *) (addr)) = (val)
//#define READ_MEM32(addr)         (*(volatile unsigned int *) (addr))
//#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
//#define READ_MEM16(addr)         (*(volatile unsigned short *) (addr))
//#define WRITE_MEM8(addr, val)    (*(volatile unsigned char *) (addr)) = (val)
//#define READ_MEM8(addr)          (*(volatile unsigned char *) (addr))

//for OOB mac 2 IB access channel
#define MAC_BASE_ADDR                 0xBAF70000
#define MAC_IB_ACC_DATA           0x00A0
#define MAC_IB_ACC_SET            0x00A4

#define IO_IB_ACC_DATA           0x00A0
#define IO_IB_ACC_SET            0x00A4
#define MAC_BASE_ADDR                         0xBAF70000 
#define MAC2_BASE_ADDR                      0xBAFC0000 
#define MAC2_BASE_ADDR                      0xBAFC0000 
#define CFG_SPCACE_SIZE			0x300

#define 	OOBMAC_BASE_ADDR				0xBAF70000 


//WIFI DASH memory mapping
#define 	WIFI_DASH_BASE_ADDR				0xBAFA0000	

#define		EP_INDIRECT_CH_OFFSET			0x0
#define		RC_INDIRECT_CH_OFFSET			0x100
#define		EP_DBI_CH_OFFSET				0x400
#define		RC_DBI_CH_OFFSET				0x500
#define		ELBI_CH_OFFSET					0x600
#define		CDM_MBOX_OFFSET					0x700
#define		DASH_MSIX_OFFSET				0x800
#define		WIFI_DASH_CFG_OFFSET			0xA00
#define		WIFI_DASH_ISR_OFFSET			0xB00
#define		ELBI_TRAN_IO_OFFSET				0x600
#define		WIFI_DASH_ELBI_TRAN_ISR_OFFSET	0x60C
#define		WIFI_DASH_ELBI_TRAN_IMR_OFFSET	0x60E


#define PCIE1_EP_MEM 0x00000000 //temp define for makefile
#define PCIE_HOST_MEM_START_ADDR_DMRAM 0x80040000
#define PCIE_HOST_MEM_START_ADDR_L2MROM 0x80080000
#define PCIE_HOST_MEM_START_ADDR_L2MRAM 0x800C0000
#define PCIE_HOST_MEM_START_ADDR_IMRAM 	0x80000000


//PCI define
#define		RTL8168_DID		0x816810EC
#define		RTL8129_DID		0x812910EC
#define		RTL8136_DID		0x813610EC

#define     PCI_VID			0x00
#define     PCI_DID			0x02
#define     PCI_COMMAND		0x04
#define     PCI_STATUS		0x06
#define     PCI_CLASS		0x08
#define     PCI_MISC		0x0C
#define     PCI_IOBASE		0x10
#define     PCI_MEMBASE		0x18
#define     PCI_ROMBASE		0x30
#define     PCI_IRQLINE		0x3C
#define     PCI_PMID		0x40
#define     PCI_PMC			0x42
#define     PCI_PMCSR		0x44
#define     PCI_VPDID		0x60
#define     PCI_VPD_ADD		0x62
#define     PCI_VPD_DATA	0x64


#define     MAX_BUS_NUM		255
#define     MAX_DEV_NUM		32
#define     MAX_FUN_NUM		8


//Host region
#define PCIE0_RC_CFG_BASE (0xb8b00000)
#define PCIE0_RC_EXT_BASE (0xb8b01000)

#define PCIE0_EP_CFG_BASE (0xb8b10000)
#define PCIE0_EP_IO_BASE  (0xb8c00000)
#define PCIE0_EP_MEM_BASE (0xb9000000)


#define PCIE0_RC_CAP_BASE (PCIE0_RC_CFG_BASE + 0x40)
#define PCIE0_RC_ECFG_BASE (PCIE0_RC_CFG_BASE + 0x100)
#define PCIE0_RC_PL_BASE (PCIE0_RC_CFG_BASE + 0x700)

#define PCIE0_MDIO	(PCIE0_RC_EXT_BASE+0x00)
#define PCIE0_ISR	(PCIE0_RC_EXT_BASE+0x04)
#define PCIE0_PWRCR	(PCIE0_RC_EXT_BASE+0x08)
#define PCIE0_IPCFG	(PCIE0_RC_EXT_BASE+0x0c)
#define PCIE0_BISTFAIL	(PCIE0_RC_EXT_BASE+0x10)

//TLP header
#define IO_TYPE		0b00010
#define	MEM_TYPE	0b00000
#define CFG_TYPE	0b00100

#define R_CMD		0b01
#define W_CMD		0b10


#define IO_R_FMT	0b00
#define	IO_W_FMT	0b10
#define	MEM_R_32B_FMT	0b00
#define	MEM_R_64B_FMT	0b01
#define	MEM_W_32B_FMT	0b10
#define	MEM_W_64B_FMT	0b11
#define	CFG_R_FMT	0b00
#define	CFG_W_FMT	0b10

//PCIE cfg0 space offset
#define VID_DID 0x0
#define CMD_STATUS_REG 0x4
#define BADDR0	0x10
#define BADDR1	0x14
#define BADDR2	0x18
#define BADDR3	0x1c
#define BADDR4	0x20
#define BADDR5	0x24
#define CARDBUS_CIS	0x28
#define SVID_SDID 0x2c
#define EXP_ROM_ADDR 0x30
#define CAP_PTR	0x34
#define INT_REG 0x3c

//other information
#define BUS_NUM_T	1 //total bus num,defalt define 1
#define DEV_NUM_T	1 //total device num,defalt define 1
#define FUN_NUM_T	2 //total function num,defalt defin 2
#define IO_BASE_ADDR_FUN(i)	0x0000D000+0x00000100*i
#define IO_BASE_ADDR 0x0000D000
#define MEM_BASE_ADDR	0x00040000
#define TIME_OUT 1000000

#define NIC_INIT 1
#define WIFI_INIT 0

//RC indirect channel
typedef struct Pg_status{
volatile INT32U free_page_num:3;
volatile INT32U pg_wr_ptr:2;
volatile INT32U pg_rd_ptr:2;
volatile INT32U pg_full:1;
volatile INT32U :24;
}Pg_status;

typedef struct RcPage{
volatile INT32U	TLP_TYPE:5; //Tlp's type,IO,cfg,mem [4:0]
volatile INT32U	:1;  		//reserved  [5];
volatile INT32U	EP:1; 		//TLP's EP bit [6]
volatile INT32U	:1;  		//reserved [7]
volatile INT32U	NS:1; 		//TLP's NS bit [8]
volatile INT32U	RO:1; 		//TLP's RO bit [9]
volatile INT32U	TC:3;		//TLP's TC bit [12:10]
volatile INT32U	MSG:8;		//TLP's MSG code [20:13]
volatile INT32U	DBI:1;		//TLP's TC bit [21]
volatile INT32U   :10;		//resrverd [31:22]

volatile INT32U   LEN:5;  	//pcie len 
volatile INT32U   CMD:2;  	//r/w cmd
volatile INT32U   :1;			//reserved	
volatile INT32U	FIRST_BE:4;	//first_byte
volatile INT32U	LAST_BE:4;	//last_byte
volatile INT32U	:12;		//reserved
volatile INT32U DONE:1;
volatile INT32U	ERR:1;		//ERR BIT read only
volatile INT32U	INTEN:1;	//'1' mean need  send interrupt after page compeltion
volatile INT32U	OWN:1;		//'1' valid for HW, 0

volatile INT32U	ADDRL:32;	
volatile INT32U	ADDRH:32;
}RcPage;

typedef struct CFG_ADDR{
volatile INT32U :2;
volatile INT32U reg_num:10;
//volatile INT32U ext_reg_num:4;
volatile INT32U :4;
volatile INT32U fun:3;
volatile INT32U dev:5;
volatile INT32U bus:8;
}CFG_ADDR;
// EP DBI 
typedef struct Ep_DBI{
volatile INT32U :2;
volatile INT32U addr:10;
volatile INT32U :4;
volatile INT32U func_num:3;
volatile INT32U :1;
volatile INT32U cs2:1;
volatile INT32U :11;
volatile INT32U wdata:32;
volatile INT32U rdata:32;
volatile INT32U Ctrl_execute:1;
volatile INT32U Ctrl_cmd:2;   	//2'b01: Read,2'b10: Write
volatile INT32U Ctrl_byteen:4;
volatile INT32U :1;
volatile INT32U Ctrl_err:1;
volatile INT32U :23;
}Ep_DBI;


typedef struct	PH_INT{
volatile INT32U sii_rc_inta:1;		//bit0
volatile INT32U sii_rc_intb:1;		//bit1
volatile INT32U sii_rc_intc:1;		//bit2
volatile INT32U sii_rc_intd:1;		//bit3	
volatile INT32U sii_rc_pme_msg:1;	//bit4
volatile INT32U sii_rc_err_msg:1;	//bit5
volatile INT32U sii_rc_ven_msg:1;	//bit6
volatile INT32U sii_ep_ven_msg:1;	//bit7
volatile INT32U sii_rc_ltr_msg:1;	//bit8
volatile INT32U sii_ep_obff_msg:1;	//bit9
volatile INT32U sii_ep_unlock_msg:1;//bit10
volatile INT32U elbi_rw_sts:1;		//bit11
volatile INT32U elbi_axi_err:1;		//bit12
volatile INT32U elbi_timeout:1;		//bit13
volatile INT32U cdm_rw:1;			//bit14
volatile INT32U cdm_axi_err:1;		//bit15
volatile INT32U cdm_timeout:1;		//bit16
volatile INT32U rc_indr_int:1;		//bit17
volatile INT32U lc_wr_timeout:1;	//bit18
volatile INT32U lc_rd_timeout:1;	//bit19
volatile INT32U rg_up_dma_wdu:1;	//bit20
volatile INT32U rg_up_dma_ok:1;		//bit21
volatile INT32U hst_wr_timeout:1;	//bit22
volatile INT32U hst_rd_timeout:1;	//bit23
volatile INT32U rg_dn_dma_rdu:1;	//bit24
volatile INT32U rg_dn_dma_ok:1;		//bit25
volatile INT32U :6;
}PH_INT;

typedef struct ELBI_TRAN_INT{
	volatile INT16U rg_io_rw:1;
	volatile INT16U rg_io_axi_err:1;
	volatile INT16U rg_io_timeout:1;
	volatile INT32U :13;
}ELBI_TRAN_INT;
typedef struct ELBI_TRAN_IO{
volatile INT32U rg_io_data;		//io data;
volatile INT32U rg_io_addr;		//io addr;
volatile INT32U :16;			
volatile INT32U rg_io_rd:4;		//io wirte byte enalbe;
volatile INT32U rg_io_wr:4;		//io read byte enalbe;
volatile INT32U :5;
volatile INT32U	rg_func_num:2;	//notify which pcie function's io operation

//1'b1 : set by ELBI_TRANS,  notify HOST an io operation to be proccessed ,
//1'b0 : clear by HOST (4281) , notify ELBI_TRANS io operation completed
volatile INT32U	rg_io_rw_flag:1;
}ELBI_TRAN_IO;

typedef struct CDM_MBOX_INT{
	volatile INT16U rg_cdm_rw:1;
	volatile INT16U rg_cdm_axi_err:1;
	volatile INT16U rg_cdm_timeout:1;
	volatile INT32U :13;
}CDM_MBOX_INT;
typedef struct CDM_MBOX_CFG{
volatile INT32U rg_cdm_data;		//io data;
volatile INT32U rg_cdm_addr;		//io addr;
volatile INT32U :16;			
volatile INT32U rg_cdm_rd:4;		//io wirte byte enalbe;
volatile INT32U rg_cdm_wr:4;		//io read byte enalbe;
volatile INT32U :5;
volatile INT32U	rg_func_num:2;	//notify which pcie function's io operation

//1'b1 : set by ELBI_TRANS,  notify HOST an io operation to be proccessed ,
//1'b0 : clear by HOST (4281) , notify ELBI_TRANS io operation completed
volatile INT32U	rg_cdm_rw_flag:1;
}CDM_MBOX_CFG;

//fun0 interrupt type
typedef struct	FUN0_INT{
volatile INT16U rx_pkt_avail_sts:1;		//bit0
volatile INT16U rx_ok_sts:1;			//bit1
volatile INT16U rdu_sts:1;				//bit2
volatile INT16U tx_ok_sts:1;			//bit3	
volatile INT16U vpd_sts:1;				//bit4
volatile INT16U perstb_r_sts:1;			//bit5
volatile INT16U perstb_f_sts:1;			//bit6
volatile INT16U lanwake_rc_f_sts:1;		//bit7
volatile INT16U sii_rc_ltr_msg:8;		//bit8
}FUN0_INT;

//OOB MAC interrupt type
typedef struct	OOBMAC_INT{
volatile INT16U ROK:1;		
volatile INT16U RDU:1;			
volatile INT16U TOK:1;
volatile INT16U TDU:1;				
volatile INT16U FOVW:1;				
volatile INT16U LinkChg:1;				
volatile INT16U sw_int:1;		
volatile INT16U FLK:1;			
volatile INT16U RER:1;	
volatile INT16U TER:1;	
volatile INT16U ACCIB:1;
volatile INT16U ACCRISC:1;
volatile INT16U	:4;
}OOBMAC_INT;


// RC DBI
typedef struct Rc_DBI{
volatile INT32U :2;
volatile INT32U addr:10;
volatile INT32U cs2:1;
volatile INT32U :19;
volatile INT32U wdata:32;
volatile INT32U rdata:32;
volatile INT32U Ctrl_execute:1;
volatile INT32U Ctrl_cmd:2;		//2'b01: Read,2'b10: Write
volatile INT32U Ctrl_byteen:4;
volatile INT32U :1;
volatile INT32U Ctrl_err:1;
volatile INT32U :23;
}Rc_DBI;

// UP DMA Interface
typedef struct Up_DMA{
volatile INT32U rg_up_dsc_ba:32;
volatile INT32U rg_up_burst_len:3;
volatile INT32U	rg_up_dsc_rdy:1;
volatile INT32U	:28;
volatile INT32U	rg_up_dma_poll:1;
volatile INT32U	:15;
volatile INT32U	rg_up_enable:1;
volatile INT32U	:15;

volatile INT32U	rg_up_dma_ok:1;
volatile INT32U	rg_up_dma_wdu:1;
volatile INT32U	rg_up_axi_timeout:1;	
volatile INT32U	:13;
volatile INT32U	rg_up_dma_ok_msk:1;
volatile INT32U	rg_up_dma_wdu_msk:1;
volatile INT32U	rg_up_axi_timeout_msk:1;
volatile INT32U :13;
}Up_DMA;

//DOWN DMA Interface
typedef struct DN_DMA{
volatile INT32U rg_dn_dsc_ba:32;
volatile INT32U rg_dn_burst_len:3;
volatile INT32U	rg_dn_dsc_rdy:1;
volatile INT32U	:28;
volatile INT32U	rg_dn_dma_poll:1;
volatile INT32U	:15;
volatile INT32U	rg_dn_enable:1;
volatile INT32U	:15;

volatile INT32U	rg_dn_dma_ok:1;
volatile INT32U	rg_dn_dma_rdu:1;
volatile INT32U	rg_dn_axi_timeout:1;	
volatile INT32U	:13;
volatile INT32U	rg_dn_dma_ok_msk:1;
volatile INT32U	rg_dn_dma_rdu_msk:1;
volatile INT32U	rg_dn_axi_timeout_msk:1;
volatile INT32U :13;
}DN_DMA;

//EP DMA descriptor
typedef struct EP_DMA_DSC{
volatile INT32U len:14;
volatile INT32U	:14;
volatile INT32U bsy:1;
volatile INT32U msk:1;
volatile INT32U eor:1;
volatile INT32U own:1;
}EP_DMA_DSC;

typedef struct TxDesc
{
volatile INT32U Length:18;
volatile INT32U GEN_Set:9;
volatile INT32U   LGSEN:1; //Large send; TCP/IP Large send operation enable
volatile INT32U   LS:1; //Last segment descriptor of Tx pkt 
volatile INT32U   FS:1; //First segment descriptor of Tx pkt 
volatile INT32U   EOR:1; // End of descriptor Ring
volatile INT32U   OWN:1; // OWN bit =1 for NIC use, =0 for UGMAC use

volatile INT32U VLanTag:16;
volatile INT32U   RESV1:1;
volatile INT32U   TAGC:1; //Vlan tag control bit
volatile INT32U TCPHO:10; // TCP Header offset
volatile INT32U   IPV6CS:1;
volatile INT32U   IPV4CS:1;
volatile INT32U   TCPCS:1;
volatile INT32U   UDP4CS:1;

volatile INT32U	BufferAddress;
volatile INT32U	BufferAddressHigh;
}TxDesc;

typedef struct RxDesc{
volatile INT32U	Length:14;
volatile INT32U	TCPF:1;
volatile INT32U	UDPF:1;
volatile INT32U	IPF:1;
volatile INT32U	TCPT:1;
volatile INT32U	UDPT:1;
volatile INT32U	CRC:1;
volatile INT32U	RUNT:1;
volatile INT32U	RES:1;
volatile INT32U	RWT:1;
volatile INT32U	HDF:1;
volatile INT32U	ERF:1;
volatile INT32U	BAR:1;
volatile INT32U	PAM:1;
volatile INT32U	MAR:1;
volatile INT32U	LS:1;
volatile INT32U	FS:1;
volatile INT32U	EOR:1;
volatile INT32U	OWN:1;
    
volatile INT32U	VLanTag:16;
volatile INT32U	TAVA:1;
volatile INT32U RIPV4:1;
volatile INT32U	RIPV6:1;
volatile INT32U  RTCP:1;
volatile INT32U  RSVD:1;
volatile INT32U  VMQ:4;
volatile INT32U Resv:3;
volatile INT32U  RMCP:1;
volatile INT32U	MOBF:1;
volatile INT32U	V4F:1;
volatile INT32U	V6F:1;
  
volatile INT32U	BufferAddress;	 //should be reversed when work
volatile INT32U	BufferAddressHigh;	 // on real c+ chip

//************************************Header Part
volatile INT32U	Length_Header:14;
volatile INT32U	Resv1:2;
volatile INT32U	Eth_Ty:3;
volatile INT32U Por_Num:3;
volatile INT32U Resv2:9;
volatile INT32U HDPOS:1;

volatile INT32U RSS_r;

volatile INT32U	BufferAddress_Header;	 //should be reversed when work
volatile INT32U	BufferAddressHigh_Header;	 // on real c+ chip

};
//struct Pci_Host{
//}

typedef struct Pci_Dev{
	struct Pg_status *pg_status;
	struct ELBI_TRAN_IO *Elbi_io_info;
	struct CDM_MBOX_CFG *Cdm_cfg_info;

	volatile INT32U FunNo;
	/*0x0-0x1FC/4 is for cfg 0x0~0x1FC,0x200/4~0x300/4 is for 0x0x700~0x7FC*/
	volatile INT32U	Default_cfg[1][CFG_SPCACE_SIZE/4];
//	volatile INT32U	Virtual_cfg[1][CFG_SPCACE_SIZE/4];
	volatile INT32U	Vendor_cfg[1][CFG_SPCACE_SIZE/4];	
	volatile INT32U Rxcount;	
	volatile INT32U io_addr;
	volatile INT32U rxdesctmp;
	volatile INT32U	txdesctmp;
	volatile INT32U pci_bus_scan_status;
	volatile INT32U allocate_mem_base;
	volatile INT32U Txcount;
	volatile INT32U	WrongData;
	volatile INT32U	WrongDesc;
	volatile INT32U	WrongCount;
	volatile INT32U IMRvalue;
	volatile INT32U Vendor_cfg_bar[6][2];//6 means bar0~bar5,2 means type and size
	volatile INT32U S3_Vendor_cfg_bar[0x10];//back up S3 S4 wifi bar addr
	//volatile INT32U S3_Vendor_cfg_cmd;//back up S3 S4 wifi bar addr
	volatile INT32U PM_status_offset;
	volatile INT32U ASPM_status_offset;
	volatile INT8U	S3_S4_Flag;	
	volatile INT8U	S3_S4_Resume;	
	volatile INT8U	IOFlag;
	volatile INT8U	Bypass_mode_wocfg_flag;
	volatile INT8U	Bypass_Driver_Status;
	volatile INT8U	rc_indr_int;
	volatile INT8U goto4281mode;
	volatile INT8U driver_status;
	volatile INT8U EP_prst_status;
}Pci_Dev; 
// Register access macro
	
/*#define REG32(reg) (*(volatile unsigned int *)(reg))
#define REG16(reg) (*(volatile unsigned short *)(reg))
#define REG8(reg) (*(volatile unsigned char *)(reg))*/
INT32U PH_allocate_mem(volatile INT32U size);
INT32U	Read_Vt_PCIDword(volatile USHORT addr,volatile USHORT fun_num,volatile INT32U *value);
INT32U	Write_Vt_PCIDword(volatile USHORT addr,volatile USHORT fun_num,volatile ULONG value);
INT32U	Read_RC_PCIDword(volatile USHORT addr,volatile INT32U *value);
INT32U	Write_RC_PCIDword(volatile USHORT addr,volatile ULONG value);
USHORT Addr2Byte_en(volatile USHORT addr,volatile USHORT byte_init);
ULONG Read_EP_PCIDword(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U *value);
ULONG Read_EP_PCIWord(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U *value);
ULONG 	Read_EP_PCIByte(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U *value);
ULONG Write_EP_PCIDword(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U value);
ULONG	Write_EP_PCIWord(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U value);
ULONG Write_EP_PCIByte(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U value);
ULONG Read_EP_IODword(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U *value);
ULONG Read_EP_IOWord(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U *value);
ULONG Read_EP_IOByte(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U *value);
ULONG Write_EP_IODword(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U value);
ULONG Write_EP_IOWord(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U value);
ULONG Write_EP_IOByte(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U value);
INT32U Sent_TLP_R(volatile INT32U addr,volatile INT32U *value);
void bsp_PHost_handler(void);
void bsp_Fun0_handler(void);
void bsp_Fun0_subHandler(INT16U isr_tmp);
void bsp_OOBMAC_handler(void);

INT32U EP_INDI_Initial();
INT32U EP_INDI_Initial();
INT32U CFG_Initial(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun);
INT32U Device_Initial();
ULONG  PCIE_MEM8_READ(volatile INT32U Haddr,volatile INT32U Laddr,volatile INT32U *value);
ULONG  PCIE_MEM8_WRITE(volatile INT32U Haddr,volatile INT32U Laddr,volatile INT32U value);
ULONG PCIE_MEM16_READ(volatile INT32U Haddr,volatile INT32U Laddr,volatile INT32U *value);
ULONG  PCIE_MEM16_WRITE(volatile INT32U Haddr,volatile INT32U Laddr,volatile INT32U value);
ULONG PCIE_MEM32_READ(volatile INT32U Haddr,volatile INT32U Laddr,volatile INT32U *value);
ULONG  PCIE_MEM32_WRITE(volatile INT32U Haddr,unsigned int Laddr, unsigned int value);
void pci_interrupt_handler_callback(void);
INT32U Inital_Adapter();
INT32U Inital_Adapter_cfg();

ULONG Send_TLP_Polling(
	volatile INT32U TLP_TPYE,		//TLP Type
	volatile INT32U RW,				//Read or Write
	volatile INT32U Base_addr_H,	//high Base addr
	volatile INT32U Base_addr_L,	//low base addr
	volatile INT32U offset,			//offset addr 
	volatile INT32U first_byte_en,	//first byte enable
	volatile INT32U last_byte_en,	//last byte enalbe
	volatile INT32U LEN,			//length
	volatile INT32U Timeout,		//polling time out num
	volatile INT32U *value			//Read or write data ptr
	);
INT32U _2_4281();
INT32U _2_4281_cfg();
INT32U _2_Bypass();
INT32U _2_Bypass_WO_cfg();

INT32U VTcfg_2_VDcfg(volatile USHORT vtfun,volatile USHORT bus,volatile USHORT dev,volatile USHORT vdfun);
INT32U	DefCfg_2_VTCfg();
INT32U BackUp_Vendor_Cfg(volatile USHORT bus,volatile USHORT dev,volatile USHORT fun,volatile INT32U * Memory);
INT32U VDtable_2_wificfg();
INT32U Default_table_2_wificfg();


INT32U BackUp_Virtual_Cfg(volatile USHORT fun,volatile INT32U * Memory);
INT32U	_Read_Vt_PCIDword(volatile USHORT addr,volatile USHORT fun_num,volatile INT32U *value,volatile USHORT cs2);
INT32U	_Write_Vt_PCIDword(volatile USHORT addr,volatile USHORT fun_num,volatile ULONG value,volatile INT32U first_byte_en,volatile USHORT cs2);
INT32U Cfg_Bar_VD2VT(volatile USHORT VDfun);
INT32U	SearchPID(volatile INT32U bus,volatile INT32U dev,volatile INT32U fun,volatile INT32U SPID,volatile INT32U offset);
void Show_Rc_cfg(INT16U size);
void Show_table_cfg();
void Before_shut_down();
void Fun0_Initial();
INT32U RC_INDI_Initial_cfg();
INT32U OOBMACReadIBReg( INT16U reg);
void OOBMACWriteIBReg(INT16U reg, INT8U highBit, INT8U lowBit, INT32U value);
void OOB2IB_W(INT16U reg,INT8U byte_en, INT32U value);
void prst_control(INT8U value);
void Iso_control(INT8U value);
void Wake_control(INT8U value);
void PH_HW_Reset(INT8U value);
volatile INT32U Rc_ephy_R(INT32U reg);
volatile INT32U Rc_ephy_W(INT32U reg,INT32U data);
INT32U Byte_Enalbe_2_FF(volatile INT8U byte_enalbe);
INT32U Wifi_driver_loading();
void Enable_wifidevcie_ISR();
INT32U Inital_Adapter_indriver();

INT32U WIFI_Reg32(INT32U reg);
#define REGX32(reg)	(*(volatile INT32U  *)(WIFI_Reg32(reg)))
#define REGX16(reg)	(*(volatile INT16U  *)(WIFI_Reg32(reg)))
#define REGX8(reg)	(*(volatile INT8U  *)(WIFI_Reg32(reg)))

/*typedef enum{
    PERSTB_ISOLATEB_LANWAKE_CLKREQB,   //table1
	LEDPIN0TO2,
	EJTAG_SMBALERT0,
	SMBCLK0_SMBDATA0,
	SPISCK_SPISI_SPISO0_2_3,
	NF_ALE_NF_DD1_3_5_7,
	NF_DD0_6_RDYWR_N_RD_N_NF_CLE,
	UARTTX_UARTRX,
	SMBCLK1_SMBDATA1,
	NCSI_GPIO,
	GPIPIN,
	
}SharePin_type;
     
typedef enum{
    NOT_USED_MODE,
	FUNCTION_MODE,
	DEBUG_MODE,
	TESTIO_MODE,
	GPIO_MODE,
	BOUNDARY_SCAN_MODE,
	
}SharePin_mode;

typedef enum{
    OOBMAC,
	NCSI,

}OOBMAC_type;

typedef enum{
	GPI=1,	
	GPO,
	GPIO1,
	GPIO2,
	GPIO3,
	GPIO4,  //6
	GPIO5,
	GPIO6,
	GPIO7,	//9
	

}OOBMAC_gpio_no;

typedef enum{
    INPUT,
	OUTPUT,

}OOBMAC_gpio_dir;

typedef enum{
    DISABLE,
	ENABLE,

}OOBMAC_gpio_en;

INT8U OOBMAC_sharepin_mode_set(SharePin_type sharepin,SharePin_mode mode);
void OOBMAC_gpio_init(OOBMAC_gpio_no gpio_num,OOBMAC_gpio_dir dir,INT8U val);//if dir is GPIO_INPUT, then val is useless,but you should enable interrupt
void OOBMAC_gpio_output(OOBMAC_gpio_no gpio_num,INT8U val);
INT8U OOBMAC_gpio_input(OOBMAC_gpio_no gpio_num);*/
void WIFIDASH_DLY(INT32U i);
void bsp_pcie_sw_handler();
#endif


