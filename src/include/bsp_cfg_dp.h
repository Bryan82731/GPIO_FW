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
#include "sys/autoconf.h"
#include "sys/dpdef.h"

#define SOLINTERVAL  3 

//signature for handling ROM code and Flash code
#define NODASH               0x3C088100
#if CONFIG_VERSION == 1
#define NORMAL_SIG           0x3C088101
#else
#define NORMAL_SIG           0x52544B00
#endif
#define NORMAL2              0x0100F809
#define DASH_DISABLED        0x44415348
#define FWUPDATE             0x46555044
#define FWUPDATEROM          0x55504452
#define PATCH_SIG            0x50415443
#define VALID_KEY_SIG        0x004B5452
#define VALID_CONF_SIG       0x434B5452
#define DUAL_IP_MODE         0x4455414C

#define PKT_ALIGN_SIZE       2
#define TCR_MAX_PKT_SIZE     250 

#define MAX_SENSOR_NUMS      16

#define MAX_USERS            16

//assume PLDM would complete after 20 secs when booting
#define WAIT_PLDM_SECS       20

#define PLDM_STR_TBL_SIZE    1536 
#define PLDM_ATT_TBL_SIZE    512

//memory partition
#define PAR_SIZE             16
/*
 *******************************************************************************
 * Address mapping of BSP registers
 *******************************************************************************
 */
#define DPCONF_ADDR          0xA1000000
#define DPCONF_ADDR_CACHED   0x81000000
 
#define WDT_CONFIG           0x8100108C
#define CPU_REG              0xB9000004
#define CPU_BASE             0xB9000000

#define UPHY_VSTATUS         0x00
#define UPHY_SLB             0x01
#define VCONTROL             0x02
#define VSTATUS              0x03

/* IMEM & DMEM Configuration */
#define IMEM_BASE            0x00000000
#define IMEM_TOP             0x0003FFFF

#define DMEM_BASE            0x00100000
#define DMEM_TOP             0x0011FFFF
#define DMEM_ON              0x00000400
#define DMEM_V_BASE          0x80100000

//#ifdef CONFIG_FPGA_TEST_ROM
#if 1
#define REFILL_BASE          0x01080000
#define REFILL_TOP           0x010BFFFF

#define DUMMY_BASE           0xFF000000
#define DUMMY_TOP            0x00000000
#else
#define DUMMY_BASE           0xFF000000
#define DUMMY_TOP            0xFF0003FF
#endif

#define MAX_PKT_SIZE         1536

//Descriptors for GMAC/SMBUS/TLS
//From 0x80100000 - 0x801007FF
#define DMEMStartAddr        0x80100000
//#define RxdescStartAddr      (DMEMStartAddr + 0x0)
#define RxdescStartAddr      (DMEMStartAddr + 0xF00)
#define TxdescStartAddr      (DMEMStartAddr + 0x100)
#define DMDescStartAddr      (DMEMStartAddr + 0x200)
#define DSDescStartAddr      (DMEMStartAddr + 0x230)
#define TLSdescStartAddr     (DMEMStartAddr + 0x260)

//SMBus Descriptor 
#define SMBTxdescStartAddr   (DMEMStartAddr + 0x290)
#define SMBMTxdescStartAddr  (DMEMStartAddr + 0x2C0)
#define SMBRxdescStartAddr   (DMEMStartAddr + 0x300)
//Rx/Tx Buffer for SMBUS/PLDM
#define SMBRxbuffStartAddr   (DMEMStartAddr + 0x500)
#define SMBTxbuffStartAddr   (DMEMStartAddr + 0xE00)
#define SMBMTxbuffStartAddr  (DMEMStartAddr + 0xE80)

//Desc for USB
//#define USBDescStartAddr     (DMEMStartAddr + 0xF00)
#define USBFlashStartAddr    (0xA1080000)

//Maximal 16
#define RxdescNumber                 16
#define TxdescNumber                 16

#define SMBTxdescNumber               1

#ifdef CONFIG_BLOCK_READ 
#define SMBRxdescNumber              64 
#define SMBRxSIZE                    36
#else
#define SMBRxdescNumber              16 
#define SMBRxSIZE                    76
#endif

//maximal 256
#define SMBMTxdescNumber              1

//timestamp and smbios table entry
//the first 0x10 are timestamp
//FSC also put smbios header information which is 32 bytes long
#define SMBIOS_DATA_OFFSET       0x10
#define SMBIOS_HEADER_SIZE       0x20
#define MAX_SMBIOS_SIZE          8192
#define DPCONF_ROM_START_ADDR    (DPCONF_ADDR_CACHED + 0x1000)
#define PATTERN_START_ADDR       (DPCONF_ADDR_CACHED + 0x2000)
#define SMBIOS_ROM_START_ADDR    (DPCONF_ADDR_CACHED + 0x6000)
//#define SMBIOS_TABLE_ENTRY       128
#define SYSTEM_ASF_TABLE         (DPCONF_ADDR_CACHED + 0x8000)
#define SYSTEM_EVENT_LOG_ROM     (DPCONF_ADDR_CACHED + 0x9000)
#define SYSTEM_USER_INFO         (DPCONF_ADDR_CACHED + 0xA000)
#define SYSTEM_PLDM_DATA         (DPCONF_ADDR_CACHED + 0xB000)
#define PLDM_STR_TBL             (DPCONF_ADDR_CACHED + 0xB100)
#define PLDM_ATR_TBL             (DPCONF_ADDR_CACHED + 0xB800)
#define PLDM_VAL_TBL             (DPCONF_ADDR_CACHED + 0xBC00)
#define SYSTEM_SW_INFO           (DPCONF_ADDR_CACHED + 0xC000)
#define SYSTEM_SUB_INFO          (DPCONF_ADDR_CACHED + 0xD000)
#define SYSTEM_ROLE_INFO         (DPCONF_ADDR_CACHED + 0xE000)
#define SYSTEM_CERT_DATA         (DPCONF_ADDR_CACHED + 0xF000)

#define FLASH_WRITE_SIZE         64
#define MAX_EVENT_ENTRY          32
#define ROM_DATA_START_ADDR      0x00008000

//TLS certificate
#define TLS_FLASH_DATA_ADDRESS   (DPCONF_ADDR_CACHED + 0xF000)

//TCR configuration registers
#define TCR_BASE_ADDR            0xB2001800
#define TCR2_BASE_ADDR           0xB2002000
//byte access
#define TCR_DLL                  0x02
#define TCR_DLM                  0x03
#define TCR_IER                  0x04
#define TCR_FCR                  0x06
#define TCR_CONF0                0x10
#define TCR_THR_DESC_START       0x11
#define TCR_THR_PKT_RD           0x12
//0x11 - 0x20
#define TCR_RBR_DESC_START       0x13
#define TCR_TIMT                 0x14
#define TCR_TIMPC                0x18
//word access
#define TCR_THR_DATA_START       0x20
//0x30 - 0x130
#define TCR_RBR_DATA_START       0x40
#define TCR_TPT                  0x60
#define TCR_RBR_IFG              0x68
#define TCR_IMR                  0x6C
#define TCR_ISR                  0x6E
#define SERIAL_RBR_TIMER         0x74

#define HEAP_START_ADDR          0x80101000
#define MEM_MAP_ADDR             0x81070000
#define PROVISION_ADDR           (DPCONF_ADDR_CACHED + 0x113C)


/*
SMBus Engine Register
Time: 2008.03.03 1300PM
*/
//physical address:0x1200_1000~0x1200_17FF
#define SMBUS_BASE_ADDR      0xB2001000
#define SEIO_SMEn                0x0000
#define SEIO_SMPollEn            0x0002
#define SEIO_ISR                 0x0008
#define SEIO_IMR                 0x0009
#define SEIO_MIS_ERR             0x000B
#define SEIO_SlaveAddr           0x000C
#define SEIO_RxDesc              0x001C
#define SEIO_RxCurDesc           0x0020
#define SEIO_TxDesc              0x0024

#define SEIO_SCTimeOut           0x0028
#define SEIO_SampleTimingCtrl    0x0040
#define SEIO_PutDataTimingCtrl   0x0042
#define SEIO_SMBFallingSetting   0x0044
#define SEIO_SMBRisingSetting    0x0046
//2008.04.01 tomadd
#define SEIO_Status              0x0001
#define SEIO_ModeCmd             0x0004
#define SEIO_MTxDesc             0x0054
//2008.04.06 tomadd
#define SEIO_BusFree             0x002C
#define SEIO_HoldTimeRStart_1    0x002E
#define SEIO_RS_SetupTime_1      0x0030
#define SEIO_StopSetupTime_1     0x0032
#define SEIO_DataHoldTime        0x0034
#define SEIO_MasterClkLow        0x0038
#define SEIO_MasterClkHigh       0x003A
#define SEIO_MasterRBControl     0x003C
#define SEIO_MasterTDControl     0x003E
#define SEIO_SlaveTimingCtrl     0x0040
#define SEIO_SlaveLDTimingCtrl   0x0042
#define SEIO_SlaveSMBCFallTime   0x0044
#define SEIO_SlaveSMBCRiseTime   0x0046
#define SEIO_SlaveSMBDFallTime   0x0048
#define SEIO_SlaveSMBDRiseTime   0x004A
#define SEIO_HoldTimeRStart_2    0x004C
#define SEIO_RS_SetupTime_2      0x004E
#define SEIO_StopSetupTime_2     0x0050
#define SEIO_SMBCFallTime        0x0058
#define SEIO_SMBCRiseTime        0x0059
#define SEIO_SMBDFallTime        0x005A
#define SEIO_SMBDRiseTime        0x005B

//SMBus Engine -2
#define SEIO_SMEn2               0x0080
#define SEIO_Status2             0x0081
#define SEIO_SMPollEn2           0x0082
#define SEIO_ModeCmd2            0x0084
#define SEIO_ISR2                0x0088
#define SEIO_IMR2                0x0089
#define SEIO_SlaveAddr2          0x008C
#define SEIO_SlaveAddr3          0x0090
#define SEIO_SampleTimingCtrl2   0x00C0
#define SEIO_PutDataTimingCtrl2  0x00C2
#define SEIO_SMBFallingSetting2  0x00C4
#define SEIO_SMBRisingSetting2   0x00C6
#define SEIO_RxDesc2             0x009C
#define SEIO_RxCurDesc2          0x00A0
#define SEIO_BusFree2            0x00AC
#define SEIO_HoldTimeRStart_1S   0x00AE
#define SEIO_RS_SetupTime_1S     0x00B0
#define SEIO_StopSetupTime_1S    0x00B2
#define SEIO_DataHoldTime_S      0x00B4
#define SEIO_MasterClkLow_S      0x00B8
#define SEIO_MasterClkHigh_S     0x00BA
#define SEIO_MasterRBControl_S   0x00BC
#define SEIO_MasterTDControl_S   0x00BE
#define SEIO_SlaveTimingCtrl_S   0x00C0
#define SEIO_SlaveLDTimingCtrl_S 0x00C2
#define SEIO_SlaveSMBCFallTime_S 0x00C4
#define SEIO_SlaveSMBCRiseTime_S 0x00C6
#define SEIO_SlaveSMBDFallTime_S 0x00C8
#define SEIO_SlaveSMBDRiseTime_S 0x00CA
#define SEIO_HoldTimeRStart_2S   0x00CC
#define SEIO_RS_SetupTime_2S     0x00CE
#define SEIO_StopSetupTime_2S    0x00D0
//SMBus Engine -2

/* TIMER address mapping  */
#define TIMER_IOBASE         0xB1000000

#define TIMER_LC             0x00
#define TIMER_CV             0x04
#define TIMER_CR             0x08
#define TIMER_EOI            0x0C

#define BSP_TIMER_FREQ       125000000
#define HZ                   100

/*   Flash Controller            */
/*   Flash Controller            */
#define FLASH_BASE_ADDR      0xB4000000
#define FLASH_CTRLR0         0x00
#define FLASH_NDF            0x04
#define FLASH_SSIENR         0x08
#define FLASH_RXFTLR         0x1C
#define FLASH_SR             0x28
#define FLASH_IMR            0x2C
#define FLASH_ISR            0x30
#define FLASH_DR             0x60
#define FLASH_INT_TXEIS      0x01
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

#define IO_DASH_REQUEST      0x35
#define OCP_SIPV4_0          0xB0

/*
  GMAC

*/
//genral accross version
#define IOREG_IOBASE         0xB2000000
#define MAC_BASE_ADDR        0xB2000000
#define MAC_IDR0                 0x0000
#define MAC_IDR4                 0x0004
#define MAC_MAR0                 0x0008
#define MAC_MAR4                 0x000C

//VA/VB, bsp_bits_set is 8 bits access
//VC is 32 bits access
//Offset 0x11
#define BIT_ACPI             0x00
#define BIT_DASHEN           0x07
#define BIT_OOBRESET         0x06
#define BIT_APRDY            0x05
#define BIT_FIRMWARERDY      0x04
#define BIT_DRIVERRDY        0x03
#define BIT_PCIEBDGRESET     0x02
#define BIT_PHYCLR           0x01
#define BIT_PCIEBDGEN        0x00
//Offset 0x10
#define BIT_TLSEN            0x07

//Offset 0x15
#define BIT_AAB              0x07
#define BIT_RMTCTL           0x01
#define BIT_SYSSH            0x00

#define BIT_FILTERV6DIP1     0x07
#define BIT_FILTERV4DIP0     0x06

//VB only

#define OWN_BIT              0x80000000
#define BUFF_SIZE            0x00000600
#define EOR_BIT              0x40000000

#define MAC_TNPDS                 0x0024
#define MAC_RDSAR                 0x0028
#define MAC_IMR                   0x002C
#define MAC_ISR                   0x002E

#define MAC_TPPoll                0x0030
#define MAC_CMD                   0x0036
#define MAC_TxCR                  0x0040
#define MAC_RxCR                  0x0044
#define MAC_CPCR                  0x0048

#define MAC_FLAG_OOB              0x0016
#define MAC_FLAG_IB               0x0017

#define MAC_IBREG                0x0011
#define MAC_OOBREG               0x0010
#define MAC_BIOSREG              0x0015

#define MAC_SYNC1                0x0015
#define MAC_SWISR                0x0038

#define IO_CONFIG                0x0010
#define IO_CONFIG0               0x0010
#define IO_CONFIG1               0x0011
#define IO_FWVER1                0x0012
#define IO_FWVER2                0x0013
#define IO_SSTATE                0x0014
#define IO_CONFIG2               0x0015

#define IO_DMDSAR                0x001C //DMEM Master Descriptor Start Address
#define IO_DSDSAR                0x0020 //DMEM Slave Descriptor Start Address
#define IO_HREQ                  0x0034
#define IO_DMEMSTA               0x0038
#define IO_DMEMENDA              0x003C
#define IO_MITI                  0x004A
#define IO_CONF3                 0x0054
#define IO_PHY                   0x0060
#define IO_SIPV6_0               0x0070

#define IO_DIPFILTER             0x0047

#define IO_PROFILE1              0x0094
#define IO_PROFILE2              0x0098
#define IO_TURN                  0x009C
#define IO_HOSTNAME              0x00A0
#define IO_SIPV4_0               0x00B0

#define IO_HWFunCtr              0x0015
#define IO_SIPV4_1               0x00B4
#define IO_SIPV4_2               0x00B8
#define IO_SIPV4_3               0x00BC

#define IO_DIPV6                 0x00C0
#define IO_DIPV4                 0x00D0

#define IO_KEY                   0x0070
#define IO_KEY_SIG               0x0080

/* TLS Configuration */
#define TLS_BASE_ADDR        0xB2000800
#define TLS_DESC                   0x00
#define TLS_POLL                   0x04
#define TLS_IMR                    0x05
#define TLS_STATUS                 0x06

/*KCS Configuration*/
#define KCS_BASE             0xB2002800
#define KCS_CONF                   0x00
#define KCS_CTRL                   0x01
#define KCS_STATUS1                0x02
//bit 0 => to clear OBF
#define KCS_STATUS2                0x03
#define KCS_ERROR_CODE             0x04
#define KCS_COMMAND                0x05
#define KCS_DATA_OUT               0x06
#define KCS_DATA_IN                0x07
#define KCS_RX_DESC                0x11
#define KCS_RX_DESC_LEN            0x10
#define KCS_RX_DMA_ADDR            0x14
#define KCS_TX_DESC                0x21
#define KCS_TX_DESC_LEN            0x20
#define KCS_TX_DMA_ADDR            0x24
#define KCS_IMR                    0x30
#define KCS_ISR                    0x32

/*USB Configurations */
#define USB_OTG_BASE         0xB5000000
#define GOTGCTL          0x000      /* OTG Control and Satus Register*/
#define GOTGINT          0x004      /* OTG Interrupt Register*/
#define GAHBCFG          0x008      /* Core AHB Configuration Reg */
#define GUSBCFG          0x00C      /* Core USB Configuration Reg*/
#define GRSTCTL          0x010      /* Core Reset Register */
#define GINTSTS          0x014      /* Core Interrupt Reg */
#define GINTMSK          0x018      /* Core Interrupt Mask Reg*/

#define GRXFSIZ          0x024      /* Receive FIFO Size Reg*/
#define GNPTXFSIZ        0x028      /* Non-periodic Transmit FIFO Size Reg */
#define GNPTXSTS         0x02C      /* Non-periodic Transmit FIFO Status Reg */
#define GHWCFG1          0x044      /* User HW Config1 Reg */
#define GHWCFG2          0x048      /* User HW Config2 Reg */
#define GHWCFG3          0x04C      /* User HW Config3 Reg */
#define GHWCFG4          0x050      /* User HW Config4 Reg*/

#define DIEPTXFn         (0x104 + USB_OTG_BASE)       /* Device IN Endpoint Transmit Fifo Size Register*/

#define DCFG             0x800        /* Device Configuration Register */
#define DCTL             0x804        /* Device Control Register */
#define DSTS             0x808
#define DSTSR            (0x808 + USB_OTG_BASE)       /* Device Status Register (RO)*/
#define DIEPMSK          0x810
/* Device IN Endpoint Common Interrupt Mask Register*/
#define DOEPMSK          0x814
/* Device OUT Endpoint Common Interrupt Mask Register*/
#define DAINT            0x818
/* Device All Endpoint Interrupt Register*/
#define DAINTMSK         0x81C
/* Device All Endpoint Interrupt Mask Register*/

#define DIEPCTL0         0x900 /* Logical IN Endpoint-Specific Registers */
#define DIEPINT0         0x908 /* IN Endpoint 0 Input Register */
#define DIEPTSIZ0        0x910 /* IN Endpoint 0 Transfer Size Register */
#define DIEPDMA0         0x914 /* IN Endpoint 0 DMA Address Register    */
#define DIEPCTL1         0x920 /* Logical IN Endpoint-Specific Registers */
#define DIEPINT1         0x928 /* IN Endpoint 1 Input Register */
#define DIEPTSIZ1        0x930 /* IN Endpoint 1 Transfer Size Register */
#define DIEPDMA1         0x934 /* IN Endpoint 1 DMA Address Register    */

#define DOEPCTL0         0xB00 /* Control OUT Endpoint 0 Control Register*/
#define DOEPINT0         0xB08
#define DOEPTSIZ0        0xB10 /* OUT Endpoint 0 Transfer Size Register */
#define DOEPDMA0         0xB14 /* OUT Endpoint n DMA Address Register   */
#define DOEPCTL2         0xB40 /* Control OUT Endpoint 0 Control Register*/
#define DOEPTSIZ2        0xB50 /* OUT Endpoint 0 Transfer Size Register */
#define DOEPDMA2         0xB54 /* OUT Endpoint n DMA Address Register   */

#define GICR_BASE                           (0xB5000000) /* 0xB5000000 */
#define GIMR                                (0x000 + GICR_BASE)       /* Global interrupt mask */
#define GISR                                (0x004 + GICR_BASE)       /* Global interrupt status */
#define IRR                                 (0x008 + GICR_BASE)       /* Interrupt routing */
#define IRR0                                (0x008 + GICR_BASE)       /* Interrupt routing */
#define IRR1                                (0x00C + GICR_BASE)       /* Interrupt routing */
#define IRR2                                (0x010 + GICR_BASE)       /* Interrupt routing */
#define IRR3                                (0x014 + GICR_BASE)       /* Interrupt routing */

/* Watch Dog Timer */
#define WDT_BASE_ADDR        0xB8000000   /* 0xB8000000 - 0xB80000FF */
#define WDT_CR               0x00
#define WDT_TORR             0x04
#define WDT_CCVR             0x08
#define WDT_CRR              0x0C
#define WDT_STAT             0x10
#define WDT_EOI              0x14

/*
 *******************************************************************************
 * BSP configurations
 *******************************************************************************
 */
/* ISOLATE is atcive  */
#define BSP_ISO_IRQ          RLX_IRQ_ID_15

/* ISOLATE N (S0)     */
#define BSP_ISON_IRQ         RLX_IRQ_ID_14

/* KCS cinfigurations */
#define BSP_KCS_IRQ          RLX_IRQ_ID_13

/* USB cinfigurations */
#define BSP_USB_IRQ          RLX_IRQ_ID_12

/* TCR configurations */
#define BSP_TCR2_IRQ         RLX_IRQ_ID_11

/* TCR configurations */
#define BSP_TCR_IRQ          RLX_IRQ_ID_10

/* Timer configurations */
#define BSP_TIMER1_IRQ       RLX_IRQ_ID_9

/* GMAC configurations */
#define BSP_GMAC_IRQ         RLX_IRQ_ID_6

/* SMBUS configurations */
#define BSP_SMBUS_IRQ        RLX_IRQ_ID_4

/* Flash Controller     */
#define BSP_FLASH_IRQ        RLX_IRQ_ID_5

/* UART Connfiguration  */
#define BSP_UART_IRQ         RLX_IRQ_ID_7

/*
 *******************************************************************************
 * IRQ configuration
 *******************************************************************************
 */

/*
 * Define the priority of IP[7:0]. Put IPx on IRQ_PRIORITY_x. IRQ_PRIORITY_0 is the
 * highest priority and IRQ_PRIORITY_7 is the lowest one. IP[15:8] is hardware-prioritized,
 * so you don't need to declare here.
 */
#define IRQ_PRIORITY_0   IP4          /* SMBUS IRQ, The highest priority IRQ  */
#define IRQ_PRIORITY_1   IP6          /* GMAC  IRQ */
#define IRQ_PRIORITY_2   IP7          /* UART  IRQ */
#define IRQ_PRIORITY_3   IP5          /* FLASH IRQ */
#define IRQ_PRIORITY_4   IP_NONE
#define IRQ_PRIORITY_5   IP_NONE
#define IRQ_PRIORITY_6   IP_NONE
#define IRQ_PRIORITY_7   IP_NONE      /* The lowest priority IRQ  */

//unit in seconds
//#define HEARTBEAT_TIME   10
//#define ARPREQ_TIME      60
//#define ALERT_NUM         3
#define ALERT_INTERVAL   60

//unit in 10ms
//#define ASF_POLL_TIME     3

// OS Related Debug Information
#define DBG_OSPrioCur   0x8021e5b0

/*
 *****************************************************************************************
 * Cache configuration
 *****************************************************************************************
 */
#define BSP_DCACHE_LINE_SIZE         4

/* according to Demonbranch */
/*#define MAC_BASE_ADDR   IOREG_IOBASE
#define MAC_OOBREG      IO_CONFIG
#define MAC_IDR0        IO_IDR0
#define MAC_IDR4        IO_IDR4
#define MAC_MAR0        IO_MAR0
#define MAC_MAR4        IO_MAR4
#define MAC_RDSAR       IO_RDSAR
#define MAC_TNPDS       IO_TNPDS
#define MAC_CPCR        IO_CPCR
#define MAC_TPPoll      IO_TPPoll
#define MAC_BIOSREG     IO_CONFIG2
#define MAC_SYNC1       IO_CONFIG2*/

/* according to original bsp_cfg.h in RealManage */
/*#define MAC_IBREG       IO_CONFIG1
#define MAC_SWISR       0x0038

#define DPCONF_ADDR_CACHED   0x81000000
#define DPCONF_ADDR          0xA1000000*/

#endif /* _BSP_CFG_H_ */
