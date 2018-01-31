
/* Registers for GMAC settings                  */

#define MAC_BASE_ADDR        0xB2000000
#define MAC_MAR0                 0x0008
#define MAC_MAR4                 0x000C

//0x128
#define BIT_POWER_ON_RESET   0x1D
#define BIT_SINGLE_IP        0x1C

#if CONFIG_VERSION >= IC_VERSION_EP_RevA
#define BIT_LANWAKE_IB_DIS   0x01
#define BIT_OOB_LANWAKE      0x00

#define BIT_ISOLATE          0x00
#define BIT_AUTOLOAD         0x01
#define BIT_LINKOK           0x02
#define BIT_TP10             0x03
#define BIT_TP100            0x04
#define BIT_TP1000           0x05

//For MAC_STATUS 0x10E
#define BIT_FLASH_LOCK       0x00

//For IBREG 0x124
#define BIT_DRIVERRDY        0x00
#define BIT_APRDY            0x01
#define BIT_FWMODE           0x0A

//For OOBREG 0x128
#define BIT_DASHEN           0x00
#define BIT_TLSEN            0x01
#define BIT_FIRMWARERDY      0x02
#define BIT_OOBRESET         0x03

//For BIOSREG 0x12C
#define BIT_SYSSH            0x00

//FOR ACPI 0x160
#define BIT_ACPI             0x00

//For SYNC1 0x170
#define BIT_RMTCTL           0x00

//For GPIO control 0x500
#define BIT_GPO_EN           0x04
#define BIT_GPO_OE           0x05
#define BIT_GPO_I            0x06
#define BIT_GPO_C            0x07


//For 0x700
#define BIT_AES_KEY          0x00

#define OWN_BIT              0x80000000
#define BUFF_SIZE            0x00000600
#define EOR_BIT              0x40000000

#define MAC_TNPDS                 0x0024
#define MAC_RDSAR                 0x0028
#define MAC_IMR                   0x002C
#define MAC_ISR                   0x002E

#define MAC_TPPoll                0x0030
#define MAC_CMD                   0x0036
#define MAC_RxCR                  0x0044
#define MAC_CPCR                  0x0048

#define MAC_IB_ACC_DATA           0x00A0
#define MAC_IB_ACC_SET            0x00A4

#define MAC_EXT_INT               0x0100
#define MAC_MAC_STATUS            0x0104
#define MAC_FLASH_LOCK            0x010E
#define MAC_FLAG_IB               0x0110
#define MAC_FLAG_OOB              0x0114
//alias, would be removed
#define IO_FLAG_IB               0x0110
#define IO_FLAG_OOB              0x0114

#define MAC_MUTEX_TURN            0x0118

#define MAC_FVID                  0x0120
#define MAC_IBREG                 0x0124
#define MAC_OOBREG                0x0128
#define MAC_BIOSREG               0x012C
#define MAC_HOSTNAME              0x0140
#define MAC_LAN_WAKE              0x0154
#define MAC_ACPI                  0x0160
#define MAC_SYNC1                 0x0170
#define MAC_SWISR                 0x0180
#define MAC_NO_CLEAR              0x01DC

#define MAC_GPIO                  0x0500

//start address of key loading
#define MAC_AES_KEY               0x0700
#define MAC_AES_KEY_SIG           0x0710
#define MAC_AES_KEY_NUM           0x073E

//#define IO_FLAG_IB                0x0006
//#define IO_FLAG_OOB               0x0007
#define I0_OOB_PORT0              0x0010
#define I0_OOB_PORT1              0x0012
#define I0_OOB_PORT2              0x0014
#define I0_OOB_PORT3              0x0016

#else
#define MAC_SWISR          0x0038
//LANE WAKE
#define BIT_LANWAKE_IB_DIS   0x1F
#define BIT_OOB_LANWAKE      0x1E

#define BIT_RMTCTL           0x19
#define BIT_SYSSH            0x18
#define BIT_ACPI             0x10

#define BIT_DASHEN           0x0F
#define BIT_OOBRESET         0x0E
#define BIT_APRDY            0x0D
#define BIT_FIRMWARERDY      0x0C
#define BIT_DRIVERRDY        0x0B
#define BIT_FWMODE           0x0A
#define BIT_OOB_WDT          0x09
#define BIT_DRV_WAIT_OOB     0x08
#define BIT_TLSEN            0x07

//RxCR for VC only
#define BIT_FILTERV6DIP1     0x1f
#define BIT_FILTERV4DIP0     0x1e
#define BIT_FILTERV4DIP1     0x1d
#define BIT_FILTERV4DIP2     0x1c

#define BIT_FILTERV6SIP0     0x19
#define BIT_FILTERV4SIP0     0x15

//VC only
#endif
#else
#define MAC_SWISR          0x0038
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
#endif

#define OWN_BIT              0x80000000
#define BUFF_SIZE            0x00000600
#define EOR_BIT              0x40000000

#define IOREG_IOBASE         0xB2000000
#define MAC_BASE_ADDR        0xB2000000

#define MAC_IDR0                 0x0000
#define MAC_IDR4                 0x0004


#define IO_IDR0                  0x0000
#define IO_IDR4                  0x0004
#define IO_MAR0                  0x0008
#define IO_MAR4                  0x000C

#if CONFIG_VERSION  == IC_VERSION_DP_RevF
#define IO_CONFIG                0x00B8
#define IO_CONFIG1               0x00B8
#define IO_CONFIG2               0x00B8
#define MAC_BIOSREG              0x00B8
#define IO_SSTATE                0x00B8
#define IO_FLAG_OOB              0x0007
#define IO_FLAG_IB               0x0006
#define I0_OOB_PORT0             0x0010
#define I0_OOB_PORT1             0x0012
#define I0_OOB_PORT2             0x0014
#define I0_OOB_PORT3             0x0016
#define IO_FWVER                 0x0088
#elif CONFIG_VERSION < IC_VERSION_DP_RevF
#define MAC_OOBREG               0x0011
#define MAC_IBREG                0x0011
#define MAC_ACPI                 0x0014
#define MAC_SYNC1                0x0015
#define MAC_LAN_WAKE             0x0010

#define IO_CONFIG                0x0010
#define IO_CONFIG0               0x0010
#define IO_CONFIG1               0x0011
#define IO_FWVER1                0x0012
#define IO_FWVER2                0x0013
#define IO_SSTATE                0x0014
#define IO_CONFIG2               0x0015
#define MAC_BIOSREG              0x0015
#define IO_FLAG_OOB              0x0016
#define IO_FLAG_IB               0x0017
#endif

//general for DP/EP
#define MAC_TPPoll               0x0030
#define MAC_TNPDS                0x0024
#define MAC_RDSAR                0x0028
#define MAC_CPCR                 0x0048

#define IO_DMDSAR                0x001C //DMEM Master Descriptor Start Address
#define IO_DSDSAR                0x0020 //DMEM Slave Descriptor Start Address
#define IO_TNPDS                 0x0024
#define IO_RDSAR                 0x0028
#define IO_IMR                   0x002C
#define IO_ISR                   0x002E
#define IO_TPPoll                0x0030
#define IO_HREQ                  0x0034
#define IO_CMD                   0x0036
#define IO_DMEMSTA               0x0038
#define IO_DMEMENDA              0x003C
#define IO_TxCR                  0x0040
#define IO_RxCR                  0x0044
#define IO_CPCR                  0x0048
#define IO_MITI                  0x004A
#define IO_CONF3                 0x0054
#define IO_PHY                   0x0060
#define IO_SIPV6_0               0x0070

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF

#define IO_FWVER                 0x0088
#define IO_DIP2V4                0x008C
#define IO_DIPFILTER             0x0044
#else
#define IO_DIPFILTER             0x0047
#endif

#define IO_PROFILE1              0x0094
#define IO_PROFILE2              0x0098
#define IO_TURN                  0x009C
#define IO_HOSTNAME              0x00A0
#define IO_SIPV4_0               0x00B0

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
#if CONFIG_VERSION	== IC_VERSION_DP_RevF
#define MAC_OOBREG               0x00B8
#define MAC_IBREG                0x00B8
#define MAC_SYNC1                0x00B8
#define MAC_ACPI                 0x00B8

#define IO_DIP1V4                0x00B4
#define IO_CONFIG                0x00B8
#define IO_CONFIG1               0x00B8
#define IO_CONFIG2               0x00B8
#define MAC_BIOSREG              0x00B8
#endif
//#define IO_SSTATE                0x00B8
#define I0_OOB_PORT4             0x00BC
#define IO_HWFunCtr              0x00BC
#define IO_PortCtr               0x00BC
#else
#define IO_HWFunCtr              0x0015
#define IO_SIPV4_1               0x00B4
#define IO_SIPV4_2               0x00B8
#define IO_SIPV4_3               0x00BC
#endif

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

//EHCI
/** RTK EHCI Engine Register Definition **/
#define EHCI_BASE_ADDR          (0xB2003000) 
#define EHCICONFIG              (0x000 + EHCI_BASE_ADDR)     
/* EHCI Configuration register */
#define CMDSTS                  (0x004 + EHCI_BASE_ADDR)
/* Command and  Status register */
#define EHCI_IMR                (0x008 + EHCI_BASE_ADDR)
/* Interrupt mask register */
#define EHCI_ISR                (0x00C + EHCI_BASE_ADDR)
/* Interrupt status register */
#define OUTDesc_Addr            (0x010 + EHCI_BASE_ADDR)
/* Bulk OUT descriptor address register*/
#define INDesc_Addr             (0x014 + EHCI_BASE_ADDR)
/* Bulk IN descriptor address register*/
#define CTLOUTDesc_Addr         (0x018 + EHCI_BASE_ADDR)
/* Control OUT descriptor address register */
#define CTLINDesc_Addr          (0x01C + EHCI_BASE_ADDR)
/* Control IN descriptor address register*/
#define OOBACTDATA              (0x020 + EHCI_BASE_ADDR)
/* OOB access IB IO channel : data register*/
#define OOBACTADDR              (0x024 + EHCI_BASE_ADDR)
/* OOB access IB IO channel : address register*/
#define DBIACTDATA              (0x028 + EHCI_BASE_ADDR)
/* OOB access PCIE Configuration Space channel : data registe*/
#define DBIACTADDR              (0x02C + EHCI_BASE_ADDR)
/* OOB access PCIE Configuration Space channel : address registe*/
/** Hardware state machine response register for debug use **/
#define HWSTATE                 (0x050 + EHCI_BASE_ADDR)
/* Hardware state machine Register */

//EHCI connection bit
#define PORTSC                   0x064
#define HCSPARAMS                0x004
//tomadd 2011.09.22
//EHCI IO register
#define CONFIGFLAG               0x060
//tomadd 2011.09.30
//debug for WIN7 remove virtual device issue
#define USBCMD                   0x020


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
#define DIEPINT0	 0x908 /* IN Endpoint 0 Input Register */
#define DIEPTSIZ0        0x910 /* IN Endpoint 0 Transfer Size Register */
#define DIEPDMA0         0x914 /* IN Endpoint 0 DMA Address Register    */
#define DIEPCTL1         0x920 /* Logical IN Endpoint-Specific Registers */
#define DIEPINT1	 0x928 /* IN Endpoint 1 Input Register */
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

#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
#define BSP_SMBUS_IRQ        RLX_IRQ_ID_14
#else
/* ISOLATE N (S0)     */
#define BSP_ISON_IRQ         RLX_IRQ_ID_14
#endif

/* KCS cinfigurations */
#define BSP_KCS_IRQ          RLX_IRQ_ID_13

/* USB cinfigurations */
#define BSP_USB_IRQ          RLX_IRQ_ID_12

/* TCR2 configurations */
#define BSP_TCR2_IRQ         RLX_IRQ_ID_11

/* TCR  configurations */
#define BSP_TCR_IRQ          RLX_IRQ_ID_10

/* Timer configurations */
#define BSP_TIMER1_IRQ       RLX_IRQ_ID_9

#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
/* EHCI configurations */
#define BSP_EHCI_IRQ         RLX_IRQ_ID_8
#endif

/* GMAC configurations */
#define BSP_GMAC_IRQ         RLX_IRQ_ID_6

#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
/* SMBUS configurations */
#define BSP_SMBUS_IRQ        RLX_IRQ_ID_4
#endif

/* Flash Controller     */
#define BSP_FLASH_IRQ        RLX_IRQ_ID_5

/* UART Connfiguration  */
#define BSP_UART_IRQ         RLX_IRQ_ID_7

/* CMAC Connfiguration	*/
#define BSP_CMAC_IRQ	     RLX_IRQ_ID_2


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
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
#define IRQ_PRIORITY_4   IP2	      /* CMAC IRQ  */	
#else
#define IRQ_PRIORITY_4   IP_NONE
#endif
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

#endif /* _BSP_CFG_H_ */
