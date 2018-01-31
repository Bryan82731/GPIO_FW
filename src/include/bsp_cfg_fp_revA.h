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
#define NODASH               0x3C088100 ;
#define NORMAL_SIG           0x52544B00
#define NORMAL2              0x0100F809
#define DASH_DISABLED        0x44415348
#define FWUPDATE             0x46555044
#define FWUPDATEROM          0x55504452
#define FWUPDATENEW          0x52445550
#define PATCH_SIG            0x50415443
#define VALID_KEY_SIG        0x004B5452
#define VALID_CONF_SIG       0x434B5452
#define DUAL_IP_MODE         0x4455414C

#if CONFIG_VERSION  == IC_VERSION_EP_RevA
#define PKT_ALIGN_SIZE       0
#else
#define PKT_ALIGN_SIZE       2
#endif

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
#define DPCONF_ADDR          0xA2020000
#define DPCONF_ADDR_CACHED   0x82020000

#define WDT_CONFIG           DPCONF_ADDR_CACHED + 0x1000 + 0x40

/***********************************************/
//for DCO calibration

#define FREQ_CAL_REG0                       0x08
#define FREQ_CAL_REG1                       0x0C
	
#define EN_DCO_500M                         8
#define REF_DCO_500M                        9
#define REF_DCO_500M_VALID                  15
#define FRE_REF_COUNT                       16
#define FREQ_CAL_EN                         1
#define CPU2_IOBASE		                    0xbaf00000	// CPU2 

/***********************************************/


#define CPU_BASE             0xBB000000
#define CPU1_IOBASE			 CPU_BASE
#define CPU2_IOBASE			 0xbaf00000

#define UPHY_REG			 (CPU1_IOBASE + 0x0)
    #define UPHY_VSTATUS         0x00
    #define UPHY_SLB             0x01
    #define VCONTROL             0x02
    #define VSTATUS              0x03
	#define uphy_connect		0x00800000
	#define uphy_slb_hs			0x00400000
	#define ponrst_n_umac_d2	0x00004000
#define CPU_REG              (CPU1_IOBASE + 0x4)
#define CPU_DUMMY0_REG       (CPU1_IOBASE + 0x14)


#define CPU2_IOBASE			 0xbaf00000
#define RISC_DATA_REG        		0X0
#define RISC_CMD_REG         		0X4
#define CPU2_DMY0_REG        		0X8
	#define ponrst_n_uphy       0x00000004

/* IMEM & DMEM Configuration */
#define IMEM_BASE            0x00000000
#define IMEM_TOP             0x0003FFFF
#define IMEM_V_BASE          0x80000000

#define DMEM_BASE            0x00040000
#define DMEM_TOP             0x0007FFFF
#define DMEM_ON              0x00000400
#define DMEM_V_BASE          0x80040000

#define L2MEM_ROM_V_BASE     0x80080000
#define L2MEM_ROM_V_TOP      0x800BFFFF

//DMEM reserved for global descriptors
#define DESC_SIZE            0x1000

#define REFILL_BASE          0x02100000
#define REFILL_TOP           0x0213FFFF

#define DDR_IOBASE           0xbba00000 
#define UART_IOBASE          0xBA000000
#define MAX_PKT_SIZE         1536

//Descriptors for GMAC/SMBUS/TLS
//From 0x80100000 - 0x801007FF
#define DMEMStartAddr        0x80040000

#define CTLOUTdescStartAddr  (DMEMStartAddr + 0x000)
#define CTLINdescStartAddr   (DMEMStartAddr + 0x040)
#define OUTdescStartAddr     (DMEMStartAddr + 0x080)
#define INdescStartAddr      (DMEMStartAddr + 0x0C0)

//TCR & TCR2 Descriptors
#define TCRdescStartAddr     (DMEMStartAddr + 0x200)
#define TCR2descStartAddr    (DMEMStartAddr + 0xE00)

//GMAC Tx/Rx Descriptors
#define RxdescStartAddr      (DMEMStartAddr + 0xF00)
#define TxdescStartAddr      (DMEMStartAddr + 0x100)

#define TLSdescStartAddr     (DMEMStartAddr + 0x280) // old value is  "+0x260"  2015.11.02.briankuo

//SMBus Descriptor 
#define SMBTxdescStartAddr   (DMEMStartAddr + 0x290)
#define SMBMTxdescStartAddr  (DMEMStartAddr + 0x2C0)
#define SMBRxdescStartAddr   (DMEMStartAddr + 0x300)
#define SMBRxbuffStartAddr   (DMEMStartAddr + 0x500)

//EHCI
#if defined(CONFIG_EHCI_INTEP) && (CONFIG_VERSION >= IC_VERSION_FP_RevA)
//for interrupt 1 in descriptor address neil
#define INTIN1descStartAddr		(DMEMStartAddr + 0x700)
//for interrupt 1 in descriptor address neil
#define INTIN2descStartAddr		(DMEMStartAddr + 0x708)
#endif

#ifdef CONFIG_USB_FLASH//tomadd 20140821
#if CONFIG_VERSION >= IC_VERSION_FP_RevA 
#define USBFlashStartAddr    (0xA2100000)
#else
#define USBFlashStartAddr    (0xA1080000)
#endif
#endif

//Maximal 16
#define RxdescNumber                 16
#define TxdescNumber                 16

#define SMBTxdescNumber               1

#define TCRTHRdescNumber             16 
#define OUTdescNumber                 4
#define INdescNumber                  4 
#define CTLOUTdescNumber              4
#define CTLINdescNumber               1
#if defined(CONFIG_EHCI_INTEP) && (CONFIG_VERSION >= IC_VERSION_FP_RevA)
#define INTIN1desNumber					1	//Maximal INT IN 1descriptor number
#define INTIN2desNumber					1	//Maximal INT IN 2 descriptor number
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

#define DPCONF_ROM_START_ADDR_NOCACHE    (DPCONF_ADDR + 0x1000)
#define DPCONF_BACKUP_DEFAULT_ADDR (DPCONF_ADDR + 0x2000)

//can not use with role at the same time
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

//TLS certificate
#define TLS_FLASH_DATA_ADDRESS   (DPCONF_ADDR_CACHED + 0xF000)

//fun0(GMAC)
#define FUNCTION0_BASE_ADDR		0xbaf10000	// GMAC Configuration
#define BMC_ISR 	FUNCTION0_BASE_ADDR+0x0038
#define BMC_IMR 	FUNCTION0_BASE_ADDR+0x003A
#define VPD_STS     0x0010
#define PERSTB_R_STS 	0x0020
#define PERSTB_F_STS	0x0040

#define OOBMAC_IOBASE			0xBAF70000  //for FP OOB MAC1 slave
#define NCSI_IOBASE				0xBAFC0000 
#define MAC_DUMMY_INFORM_IB     0x018C

//TCR configuration registers
#define TCR_BASE_ADDR            0xBAF50000
#define TCR2_BASE_ADDR           0xB2002000	//FP no TCR2
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
#define TCR_DESCADDR             0x1C
//word access
#define TCR_THR_DATA_START       0x20
//0x30 - 0x130
#define TCR_RBR_DATA_START       0x40
#define TCR_TPT                  0x60
#define TCR_RBR_IFG              0x68
#define TCR_IMR                  0x6C
#define TCR_ISR                  0x6E
#define SERIAL_RBR_TIMER         0x74

//#define HEAP_START_ADDR          0x80101000
//#define MEM_MAP_ADDR             0x81070000
#define PROVISION_ADDR           (DPCONF_ADDR_CACHED + 0x113C)


/*
SMBus Engine Register
Time: 2008.03.03 1300PM
*/
//physical address:0x1200_1000~0x1200_17FF
#define SMBUS_BASE_ADDR      0xBAF30000
#define SMBUS0_IOBASE            SMBUS_BASE_ADDR //SMBUS Slave0
#define SEIO_SMEn                0x0000
	#define		SMEN_SMBUS_EN			0x0001
	#define 	SMEN_SMBUS_M_PEC_EN		0x0002
	#define		SMEN_SMBUS_S_PEC_EN		0x0004
	#define		SMEN_SMBUSALERT_N		0x0008
	#define		SMEN_LISTEN_EN			0x0010
	#define		SMEN_SMB_SL_WS_EN		0x0020
	#define 	SMEN_SLAVE_EN			0x0040
	#define 	SMEN_MASTER_EN			0x0080
	#define 	SMEN_SMB_MSG_WIN		0x0100
	#define 	SMEN_SMB_S0_ST 			0x0200
	#define 	SMEN_SMB_MS_ACT 		0x0400
	#define 	SMEN_MS_FAIL_STS		0x0800
	#define 	SMEN_MS_MCMD_FAIL		0x1000
	#define 	SMEN_SMB_SL_ADDR_COL	0x2000
	#define 	SMEN_LSN0_ST			0x4000	
#define SEIO_SMPollEn            0x0002
	#define		SMEN_SMB_STRANS_PS		0x0001
	#define 	SMEN_SMB_STRANS_CPS		0x0002
	#define		SMEN_SMB_MC_POLL		0x0010
	#define		SMEN_SMB_LSN_RST		0x0020
	#define 	SMEN_SMB_RAND_RST		0x0040
	#define 	SMEN_SMB_FRST			0x0080
#define SEIO_ISR                 0x0008
	#define		ISR_RX_NS_INT			0x01
	#define		ISR_RX_RB_INT			0x02
	#define		ISR_SR_PEC_ERR_INT		0x04
	#define		ISR_S_ABNORMAL_INT		0x08
	#define		ISR_RX_RDU_INT			0x10
	#define		ISR_TX_INT				0x20
	#define		ISR_MR_PEC_ERR_INT		0x40
	#define		ISR_M_ABNORMAL_INT		0x80
#define SEIO_IMR                 0x0009
#define SEIO_MIS_ERR             0x000B
#define SEIO_SlaveAddr           0x000C
#define SEIO_SlaveAddr1          0x0010
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

#define SEIO_RxDescOnListenM			0x005C
#define SEIO_RxCurDescOnListenM			0x0060//read only
#define SEIO_PLDMdata1 			0x0070 //32-bytes
#define SEIO_PLDMdata1_b1			0x0070 
#define SEIO_PLDMdata1_b2			0x0074 
#define SEIO_PLDMdata1_b3			0x0078 
#define SEIO_PLDMdata1_b4			0x007C 
#define SEIO_PLDMdata1_b5			0x0080 
#define SEIO_PLDMdata1_b6			0x0084 
#define SEIO_PLDMdata1_b7			0x0088 
#define SEIO_PLDMdata1_b8			0x008C 

#define SEIO_PLDMdata2     		0x0090 //32-bytes
#define SEIO_PLDMdata2_b1			0x0090 
#define SEIO_PLDMdata2_b2			0x0094 
#define SEIO_PLDMdata2_b3			0x0098 
#define SEIO_PLDMdata2_b4			0x009C 
#define SEIO_PLDMdata2_b5			0x00A0 
#define SEIO_PLDMdata2_b6			0x00A4 
#define SEIO_PLDMdata2_b7			0x00A8 
#define SEIO_PLDMdata2_b8			0x00AC 

#define SEIO_ASF_bootOptData		0x00B0 // 13 bytes
#define SEIO_ASF_bootOptData_b1		0x00B0 // 4 bytes
#define SEIO_ASF_bootOptData_b2		0x00B4 // 4 bytes
#define SEIO_ASF_bootOptData_b3		0x00B8 // 4 bytes
#define SEIO_ASF_bootOptData_b4		0x00BC // 1 bytes

#define SEIO_PLDMdata1_ByteCnt 		0x00BD
#define SEIO_PLDMdata2_ByteCnt 		0x00BE
#define SEIO_ASF_bootOpt_ByteCnt		0x00BF 
#define SEIO_SMB_BRautoReply_EN		0x00C0 
#define SEIO_SMB_BR_DataRdy			0x00C1 
#define SEIO_SMB_norm_TxD			0x00C2 

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
//SMBus Engine -2 for internal test
#define SMBUS1_IOBASE            SMBUS_BASE_ADDR + 0x10000	// SMBUS Slave1 

/* TIMER address mapping  */
#define TIMER_IOBASE         0xBA800000

#define TIMER_LC             0x00
#define TIMER_CV             0x04
#define TIMER_CR             0x08
#define TIMER_EOI            0x0C

#define HZ                   100


/*   Flash Controller            */
#define FLASH_DATA_ADDR      0xA2000000
#define FLASH_BASE_ADDR      0xBC000000

#define IO_DASH_REQUEST      0x35
#define OCP_SIPV4_0          0xB0

/*
  GMAC

*/
//genral accross version
#define IOREG_IOBASE         0xBAF70000
#define MAC_BASE_ADDR        0xBAF70000
#define MAC_IDR0                 0x0000
#define MAC_IDR4                 0x0004
#define MAC_MAR0                 0x0008
#define MAC_MAR4                 0x000C

//VA/VB, bsp_bits_set is 8 bits access
//VC is 32 bits access

#define BIT_IN_BAND          0x0C
#define BIT_RSVD             0x0B
//HwFunCtr
#define BIT_AAB              0x17

#define BIT_FILTERMACIP      0x16
#define BIT_AATCP_UDPB       0x15
#define BIT_OOBFILTERDISABLE 0x14
#define BIT_FILTERRMCP       0x13

#define BIT_PCIEBDGRESET     0x12
#define BIT_PHYCLR           0x11
#define BIT_PCIEBDGEN        0x10

//Cfg
#define BIT_CPU_SPEEDDOWN4   0x1F
#define BIT_CPU_SPEEDDOWN2   0x1E
#define BIT_POWER_ON_RESET   0x1D
#define BIT_SINGLE_IP        0x1C

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

//For OOBREG 0x13c
#define BIT_OOBLANWAKE         0x00

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
#define MAC_CONFIG0               0x0051

#define MAC_IB_ACC_DATA           0x00A0
#define MAC_IB_ACC_SET            0x00A4

#define MAC_EXT_INT               0x0100
#define MAC_MAC_STATUS            0x0104
#define MAC_OOB2IB_INT            0x0108
#define MAC_FLASH_LOCK            0x010E
#define MAC_FLAG_IB               0x0110
#define MAC_FLAG_OOB              0x0114

#define MAC_MUTEX_TURN            0x0118

#define MAC_FVID                  0x0120
#define MAC_IBREG                 0x0124
#define MAC_OOBREG                0x0128
#define MAC_BIOSREG               0x012C
#define MAC_OOB_LANWAKE	0x013C
#define MAC_HOSTNAME              0x0140
#define MAC_LAN_WAKE              0x0154
#define MAC_ACPI                  0x0160
#define MAC_SYNC1                 0x0170
#define MAC_SWISR                 0x0180
#define MAC_ROMVER                0x019C
#define MAC_NO_CLEAR              0x01DC

#define MAC_GPIO                  0x0500

//start address of key loading
#define MAC_AES_KEY               0x0700
#define MAC_AES_KEY_SIG           0x0710
#define MAC_AES_KEY_NUM           0x073F

//#define IO_FLAG_IB                0x0006
//#define IO_FLAG_OOB               0x0007
#define I0_OOB_PORT0              0x0010
#define I0_OOB_PORT1              0x0012
#define I0_OOB_PORT2              0x0014
#define I0_OOB_PORT3              0x0016

//general for DP/EP
#define IO_DMDSAR                0x001C //DMEM Master Descriptor Start Address
#define IO_DSDSAR                0x0020 //DMEM Slave Descriptor Start Address
#define IO_HREQ                  0x0034
#define IO_DMEMSTA               0x0038
#define IO_DMEMENDA              0x003C
#define IO_MITI                  0x004A
#define IO_CONF3                 0x0054
#define IO_PHY                   0x0060
#define IO_SIPV6_0               0x0070

#define IO_FWVER                 0x0088
#define IO_DIP2V4                0x008C
#define IO_DIPFILTER             0x0044

#define IO_PROFILE1              0x0094
#define IO_PROFILE2              0x0098
#define IO_TURN                  0x009C
#define IO_HOSTNAME              0x00A0
#define IO_SIPV4_0               0x00B0

//#define IO_SSTATE                0x00B8
#define I0_OOB_PORT4             0x00BC
#define IO_HWFunCtr              0x00BC
#define IO_PortCtr               0x00BC

#define IO_DIPV6                 0x00C0
#define IO_DIPV4                 0x00D0

#define IO_KEY                   0x0070
#define IO_KEY_SIG               0x0080

/* TLS Configuration */
#define TLS_BASE_ADDR        0xBB800000
#define TLS_DESC                   0x00
#define TLS_POLL                   0x04
#define TLS_IMR                    0x05
#define TLS_STATUS                 0x06

/*TRNG Configuration*/
#define TRNG_BASE             0xBB900000
#define TRNG_ANALOG			 	0x14
#define TRNG_RETURN3			 	0x2C
#define TRNG_RETURN4			 	0x30
#define TRNG_POW_BIT				0x01
#define TRNG_RDY_BIT				0x01

/*KCS Configuration*/
#define KCS_BASE             0xBAF80000
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

/*KCS Configuration*/
#define KVM_BASE             0xBAF80400
#define CMAC_SYN_REG		0x20
//EHCI
/** RTK EHCI Engine Register Definition **/
#define EHCI_BASE_ADDR          (0xBAF60000) 
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

#if (CONFIG_VERSION >= IC_VERSION_FP_RevA)//for FP usage
#define TIMEOUTCFG              (0x028 + EHCI_BASE_ADDR)
/* Timeout config registe*/
#define INTINDESCADDR1					(0x030 + EHCI_BASE_ADDR)		
/*Interrupt IN Descriptor Start Address Register 1 for port 2 device(keyboard) */
#define INTINDESCADDR2					(0x034 + EHCI_BASE_ADDR)		
/*Interrupt IN Descriptor Start Address Register 2 for port 3 device(mouse)*/	
#define DEVICE_ADDRESS					(0x040 + EHCI_BASE_ADDR)		
/*Bit[22:16]:device address for mouse device, Bit[14:8]: device address for keyboard device, Bit[6:0]: device address for msd device*/
#define ENDPOINT_REG  					(0x044 + EHCI_BASE_ADDR)		
/*BIT[7:4]: keyboard EP;BIT[3:0]: Mouse EP*/
#endif

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
#define USB_OTG_BASE         0xBB400000
#define GOTGCTL          0x000      /* OTG Control and Satus Register*/
#define GOTGINT          0x004      /* OTG Interrupt Register*/
        #define GOTGINT_SesEndDet                  (0x1<<2)

#define GAHBCFG          0x008      /* Core AHB Configuration Reg */
	#define GAHBCFG_InvDescEndian	(1<<24)
	#define GAHBCFG_AHBSingle		(1<<23)
	#define GAHBCFG_NotiAllDmaWrit	(1<<22)
	#define GAHBCFG_RemMemSupp		(1<<21)
	#define GAHBCFG_NPTxFEmpLvl		(1<<7)
	#define GAHBCFG_DMAEn			(1<<5)
	#define GAHBCFG_HBstLen			(0xf<<1)
		#define HBstLen_1W				(0<<1)
		#define HBstLen_4W				(1<<1)
		#define HBstLen_8W				(2<<1)
		#define HBstLen_16W				(3<<1)
		#define HBstLen_32W				(4<<1)
		#define HBstLen_64W				(5<<1)
		#define HBstLen_128W				(6<<1)
		#define HBstLen_256W				(7<<1)
	#define GAHBCFG_GlblIntrMsk		(1<<0)
#define GUSBCFG          0x00C      /* Core USB Configuration Reg*/
	#define GUSBCFG_ForceDevMode	(1<<30)
	#define GUSBCFG_USBTrdTim		(0xf<<10)
		#define USBTrdTim16B			(0x5<<10)
		#define USBTrdTim8B				(0x9<<10)
	#define GUSBCFG_PHYSel			(1<<6)
	#define GUSBCFG_PHYIf16B		(1<<3)
#define GRSTCTL          0x010      /* Core Reset Register */
	#define GRSTCTL_AHBIdle		(1<<31)
	#define GRSTCTL_DMAReq		(1<<30)
	#define GRSTCTL_TxFNum		(0x1f<<6)
	#define GRSTCTL_TxFFlsh		(1<<5)
	#define GRSTCTL_RxFFlsh		(1<<4)
	#define GRSTCTL_INTknQFlsh	(1<<3)
	#define GRSTCTL_FrmCntrRst	(1<<2)
	#define GRSTCTL_CSftRst		(1<<0)
#define GINTSTS          0x014      /* Core Interrupt Reg */
	#define GINTSTS_CurMod 		(1<<0)
	#define CurMod_Host				0x00000001
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
	#define DCFG_DescDMA 		(0x1<<23)
	#define DCFG_EPMisCnt		(0x1f<<18)
	#define DCFG_EnDevOutNak	(0x1<<13)
	#define DCFG_PerFrInt		(0x3<<11)
	#define DCFG_DevAddr		(0x7f<<4)
	#define DCFG_Ena32KHzSusp	(0x1<<3)
	#define DCFG_NZStsOUTHShk	(0x1<<2)
	#define	DCFG_DevSpd			(0x3<<0)
		#define DevSpd_HIGH				0
		#define DevSpd_FULL				1
		#define DevSpd_LOW				2
		#define DevSpd_FULL_48M			3
#define DCTL             0x804        /* Device Control Register */
	#define DCTL_DepSleepBESLReject	(1<<18)
	#define DCTL_EnContOnBNA		(1<<17)
	#define DCTL_NakOnBble			(1<<16)
	#define DCTL_IgnrFrmNum			(1<<15)
	#define DCTL_GMC				(0x3<<13)
	#define DCTL_PWROnPrgDone		(1<<11)
	#define DCTL_CGOUTNak			(1<<10)
	#define DCTL_SGOUTNak			(1<<9)
	#define DCTL_CGNPInNak			(1<<8)
	#define DCTL_SGNPInNak			(1<<7)
	#define DCTL_TstCtl				(0x7<<4)
			#define TstCtl_J			(1<<4)
			#define TstCtl_K			(2<<4)
			#define TstCtl_SE0_NAK		(3<<4)
			#define TstCtl_Packet		(4<<4)
			#define TstCtl_ForceEn		(5<<4)
	#define DCTL_GOUTNakSts			(1<<3)
	#define DCTL_GNPINNakSts		(1<<2)
	#define DCTL_SftDiscon			(1<<1)
	#define DCTL_RmtWkUpSig			(1<<0)
#define DSTS             0x808
	#define DSTS_EnumSpd		(0x3<<1)
		#define EnumSpd_High		(0x0<<1)
		#define EnumSpd_Full		(0x1<<1)
		#define EnumSpd_Low			(0x2<<1)
		#define EnumSpd_Full_48M	(0x3<<1)
#define DSTSR            (0x808 + USB_OTG_BASE)       /* Device Status Register (RO)*/
#define DIEPMSK          0x810
/* Device IN Endpoint Common Interrupt Mask Register*/
#define DOEPMSK          0x814
/* Device OUT Endpoint Common Interrupt Mask Register*/
#define DAINT            0x818
/* Device All Endpoint Interrupt Register*/
#define DAINTMSK         0x81C
	#define	DAINTMSK_InEpMsk		0xFFFF
		#define	InEpMsk_EP(ep_num)			0x0001<<(ep_num)
	#define	DAINTMSK_OutEpMsk 	0xFFFF<<16
		#define OutEpMsk_EP(ep_num)			(0x0001<<(ep_num))<<16

/* Device All Endpoint Interrupt Mask Register*/

#define DIEPCTL0         0x900 /* Logical IN Endpoint-Specific Registers */
	#define DXEPCTL_EPEna		(1<<31)
	#define DXEPCTL_EPDis		(1<<30)
	#define DXEPCTL_SetD1PID		(1<<29)
	#define DXEPCTL_SetD0PID		(1<<28)
	#define DXEPCTL_SNAK			(1<<27)
	#define DXEPCTL_CNAK			(1<<26)
	#define DXEPCTL_TxFNum		(0xf<<22)
	#define DXEPCTL_Stall		(1<<21)
	#define DXEPCTL_Snp			(1<<20)
	#define DXEPCTL_EPType		(0x3<<18)
		#define EPType_Control		(0<<18)
		#define EPType_Iso			(1<<18)
		#define EPType_Bulk			(2<<18)
		#define EPType_Int			(3<<18)
	#define DXEPCTL_NAKSts		(1<<17)
	#define DXEPCTL_DPID			(1<<16)
	#define DXEPCTL_USBActEP		(1<<15)
	#define DXEPCTL_NextEp		(0xf<<11)//non-periodic IN endpoints only.(valid only for Shared FIFO operations)
	#define DXEPCTL_MPS			(0x3)
		#define MPS_64B				(0<<0)
		#define MPS_32B 				(1<<0)
		#define MPS_16B				(2<<0)
		#define MPS_8B				(3<<0)
#define DIEPINT0         0x908 /* IN Endpoint 0 Input Register */
	#define DXEPINT_StupPktRcvd		(1<<15)
	#define DxEPINT_NYETIntrpt		(1<<14)
	#define DXEPINT_NAKIntrpt		(1<<13)
	#define DXEPINT_BbleErrIntrpt	(1<<12)
	#define DXEPINT_PktDrpSts		(1<<11)
	#define DXEPINT_BNAIntr			(1<<9)
	#define DXEPINT_TxfifoUndrn		(1<<8)
	#define DXEPINT_TxFEmp			(1<<7)
	#define DXEPINT_INEPNakEff		(1<<6)
	#define DXEPINT_Back2BackSETupMsk	DXEPINT_INEPNakEff//Back2BackSETupMsk control out endp only
	#define DXEPINT_INTknEPMis		(1<<5)
	#define DXEPINT_INTknTXFEmp		(1<<4)
	#define DXEPINT_TimeOUT			(1<<3)
	#define DXEPINT_SetUPMsk			DXEPINT_TimeOUT //SetUPMsk control out endps only
	#define DXEPINT_AHBErr			(1<<2)
	#define DXEPINT_EPDisbld		(1<<1)
	#define DXEPINT_XferCompl		(1<<0)
#define DIEPTSIZ0        0x910 /* IN Endpoint 0 Transfer Size Register */
	#define DXEPTSIZ_SUPCnt			(3<<29)
		#define SUPCnt_1p				(1<<29)
		#define SUPCnt_2p				(2<<29)
		#define SUPCnt_3p				(3<<29)
	#define DXEPTSIZ_InPktCnt		(0x3<<19)
		#define InPktCnt_1p				(1<<19)
	#define DXEPTSIZ_OutPktCnt		(0x1<<19)	
	#define DXEPTSIZ_XferSize		(0x7f<<0)
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

#define GICR_BASE                           (0xBB400000) /* 0xB5000000 */
#define GIMR                                (0x000 + GICR_BASE)       /* Global interrupt mask */
#define GISR                                (0x004 + GICR_BASE)       /* Global interrupt status */
#define IRR                                 (0x008 + GICR_BASE)       /* Interrupt routing */
#define IRR0                                (0x008 + GICR_BASE)       /* Interrupt routing */
#define IRR1                                (0x00C + GICR_BASE)       /* Interrupt routing */
#define IRR2                                (0x010 + GICR_BASE)       /* Interrupt routing */
#define IRR3                                (0x014 + GICR_BASE)       /* Interrupt routing */

#if CONFIG_VERSION >= IC_VERSION_FP_RevA //tomadd 20151002
#define GUID             0x03C       /* User ID Register */ 
#define GINTSTS_MODMISMATCH_INT		(0x00000002) //bit01 ModeMisMsk
#define GINTSTS_OTG_INT				(0x00000004) //bit02 OTGIntMsk
#define GINTSTS_SOF					(0x00000008) //bit03 Sof
#define GINTSTS_RXFIFO_NONEMPTY		(0x00000010) //bit04 RxFLvl
#define GINTSTS_NP_TXFIFO_EMPTY		(0x00000020) //bit05 NPTxFEmp
#define GINTSTS_EARLY_SUSPEND		(0x00000400) //bit10 ErlySusp
#define GINTSTS_USB_SUSPEND			(0x00000800) //bit11 USBSusp
#define GINTSTS_USB_RESET			(0x00001000) //bit12 USBRst
#define GINTSTS_ENUM_DONE			(0x00002000) //bit13 EnumDone
#define GINTSTS_IN_EP_INT			(0x00040000) //bit18 IEPInt
#define GINTSTS_OUT_EP_INT			(0x00080000) //bit19 OEPInt
#define GINTSTS_RESETDET			(0x00800000) //bit23 RESETDET
#define GINTSTS_USB_DISCONNECT		(0x20000000) //bit29 DisconnInt
#define GINTSTS_USB_SESSREQ			(0x40000000) //bit30 SessReqInt
#define GINTSTS_USB_RESUME			(0x80000000) //bit31 WkUpInt

//Bonnie modify 
// USB use this range DMEM_V_BASE~ DMEM_V_BASE+0x10000, total 64k
#define USB_DESC		(DMEM_V_BASE+0x00000)     //descriptor content
#define USB_IEP0_BUF		(DMEM_V_BASE+0x01000) //control 4k
#define USB_SETUP_BUF		(DMEM_V_BASE+0x02000) //Setup 64 BYTES
#define USB_OEP0_BUF		(DMEM_V_BASE+0x02040) //control out
#define USB_EP1_BUF			(DMEM_V_BASE+0x03000) //bulk 16k
#define USB_EP2_BUF			(DMEM_V_BASE+0x07000) //bulk 16k
#define USB_EP3_BUF			(DMEM_V_BASE+0x0B000) //interrupt 4k
#define USB_EP4_BUF			(DMEM_V_BASE+0x0C000) //interrupt 4k
#endif

/* Watch Dog Timer */
#define WDT_BASE_ADDR        0xBB500000   /* 0xB8000000 - 0xB80000FF */
#define WDT_CR               0x00
#define WDT_TORR             0x04
#define WDT_CCVR             0x08
#define WDT_CRR              0x0C
#define WDT_STAT             0x10
#define WDT_EOI              0x14

/*InBand*/
#define PCR					0xDC00
#define FPGA_INDIC			0xEA1C
#define DDR_CAP				0xDC40

#define TESTIO_ACT			0x04

/*DCO*/
#define FREQ_CAL_REG0			 0x08
#define FREQ_CAL_REG1			 0x0C
#define EN_DCO_500M			 	 8
#define REF_DCO_500M			 9
#define REF_DCO_500M_VALID		 15
#define FRE_REF_COUNT			 16
#define FREQ_CAL_EN				 1

/* DMAC */
#define OOB_MAC_IOBASE	MAC_BASE_ADDR
#define DMA_IOBASE		0xbbc00000	// DMA Engine Slave
#define DMA_SAR0		(DMA_IOBASE + 0)//DMA channel0 Source ADDRESS REG
#define DMA_DAR0		(DMA_IOBASE + 0x008)//DMA channel0 Destination ADDRESS REG
#define DMA_CTL0		(DMA_IOBASE + 0x018)//DMA channel0 Control REG
#define DMA_CFG0		(DMA_IOBASE + 0x040)//DMA channel0 CONFIG REG
#define DMA_SGR0		(DMA_IOBASE + 0x048)//DMA channel0 Source Gather Register
#define DMA_DSR0		(DMA_IOBASE + 0x050)//DMA channel0 Destination Scatter Register

#define DMA_LLP_LOC		(DMA_IOBASE + 0x010)//DMA channel0 Link list Item start address in mem
#define DMA_LLP_LMS		(DMA_IOBASE + 0x010)//DMA channel0 Link list Master select, bit0:bit1


#define DMA_DmaCfgReg   (DMA_IOBASE + 0x398)//DmaCfgReg bit 0 enable DMAC
#define DMA_ChEnReg		(DMA_IOBASE + 0x3a0)//DMA channel enable

#define DMA_RawTfr   (DMA_IOBASE + 0x2c0)//raw interrupt state of Transfer
#define DMA_RawBlock (DMA_IOBASE + 0x2c8)//raw interrupt state of Block Transfer

#define DMA_StatusTfr   (DMA_IOBASE + 0x2e8)//interrupt state of Transfer
#define DMA_StatusBlock (DMA_IOBASE + 0x2f0)//interrupt state of Block Transfer
#define DMA_StatusErr   (DMA_IOBASE + 0x308)//interrupt state of err
#define DMA_MaskTfr     (DMA_IOBASE + 0x310)//mask Transfer complete interrupt
#define DMA_MaskBlock   (DMA_IOBASE + 0x318)//mask Block Transfer complete interrupt
#define DMA_MaskErr     (DMA_IOBASE + 0x330)//mask Transfer error interrupt

#define DMA_ClearTfr    (DMA_IOBASE + 0x338)//clear transfer interrupt
#define DMA_ClearBlock  (DMA_IOBASE + 0x340)//clear block transfer interrupt
#define DMA_ClearErr    (DMA_IOBASE + 0x358)//clear error interrupt
#define DMA_ClearSrcTran (DMA_IOBASE + 0x348)
#define DMA_ClearDstTran (DMA_IOBASE + 0x350)
/*
 *******************************************************************************
 * BSP configurations
 *******************************************************************************
 */
/* ISOLATE is atcive  */
#define BSP_ISO_IRQ          RLX_IRQ_ID_15

#define BSP_SMBUS_IRQ        RLX_IRQ_ID_14

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

/* EHCI configurations */
#define BSP_EHCI_IRQ         RLX_IRQ_ID_8

/* GMAC configurations */
#define BSP_GMAC_IRQ         RLX_IRQ_ID_6


/* Flash Controller     */
#define BSP_FLASH_IRQ        RLX_IRQ_ID_5

/* UART Connfiguration  */
#define BSP_UART_IRQ         RLX_IRQ_ID_7

#define BSP_FUN0_IRQ		RLX_IRQ_ID_8
/* CMAC Connfiguration  */
#define BSP_CMAC_IRQ         RLX_IRQ_ID_2

/* PCIE host Connfiguration  */
#define BSP_PHOST_IRQ 		RLX_IRQ_ID_11
#define BSP_Device_IRQ      RLX_IRQ_ID_15

/* DMA Connfiguration  */
#define BSP_DMA_IRQ			RLX_IRQ_ID_4

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
#define IRQ_PRIORITY_4   IP2          /* CMAC IRQ  */
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
//#define DBG_OSPrioCur   0x8021e5b0

/*
 *****************************************************************************************
 * Cache configuration
 *****************************************************************************************
 */
#define BSP_DCACHE_LINE_SIZE         4

#endif /* _BSP_CFG_H_ */
