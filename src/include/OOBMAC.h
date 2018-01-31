#ifndef __GMAC_H__
#define __GMAC_H__
#include <rlx/Rlx_types.h>
#include <rlx/rlx_cpu_regs.h>


/*MAC test Compiler flag*/
/*OOBMAC engine test*/
//1.VLAN test
//#define MAC_TEST_VLAN_TX
//#define MAC_TEST_VLAN_RX
//#define MAC_TEST_VLAN_RXDETAGING
//2.Tx/Rx disable enable test
//#define MAC_RESET_TEST   //Tx/Rx enable/disable test check whether DMA is OK
//3.Rx Any byte test
//#define MAC_RX_ANY_BYTE
//4.CPU dynamic down speed test
//#define DYNAMIC_CPU_SLOW
//#define MAC_RX_CHECKSUM
//#define MAC_RX_DATA_CHECK
//5.CPU sleep
//#define CPU_SLEEP
//#define CPU_SLEEP_OOBMAC_WAKEUP
//#define MAC_RX_CHECKSUM
//#define MAC_RX_DATA_CHECK
//#define MAC_DEBUG_COUNT
//6.Checksum Tx/Rx test
//#define MAC_RX_CHECKSUM  //rx checksum
//7.Flow Control
//#define MAC_FLOW_CONTROL_TEST   //test  Tx direction:setting: receive pause packet; when recieve pause packet,it can slow down it's speed
//#define MAC_ACCEPT_FLOW_CTL  //test Rx direction:when it suffers from RDU,it can send pause packet
//#define MAC_SLOW_RX           //to create RDU condition
//8.Reboot test
//#define MAC_REBOOT_TEST   
//9.IB+OOB+OOB Data check test
//#define IB_OOB_OOBDATACHECK
//#define TXUNICAST
//#define MAC_RX_CHECKSUM
//#define MAC_RX_DATA_CHECK
//10.large/giant send test
//#define MAC_LARGE_GIANT_SEND

/*OOBMAC reg check*/
//1.EXTR_INT-isolate interrupt
//#define ISOLATE_INTERRUPT_TEST
//2.EXTR_INT-GPIO interrupt
//#define GPIO_INT_TEST
//#define TIMER_ENABLE
//3.Interrupt Mitigation
//#define MAC_reg_InterruptMitigation
//4.OOBLanwakeCtrl
//#define MAC_reg_oobLanwakeCtrl
//5.OOB_TO_IB_INT
//#define MAC_reg_OOB2IBInterrupt
//6.newly added 16 sw interrupts
#define MAC_reg_SWInterrupts

//for fp88pin debug
//#define DEBUG_LTSSM
//#define OOBMAC_DMA_DEBUG



//usually opened in debug/test condition
#define MAC_DEBUG_COUNT
#define MAC_RX_CHECKSUM
#define MAC_RX_DATA_CHECK
#define MAC_LOOPBACK

//#define MAC_SLOW_RX
//#define MAC_TX_SENDSLOW   //TX slow controlled by FW
//#define TXUNICAST
//#define MAC_PULLGPIOWHENERROR
//#define PKTFilter_Test
//#define MAC_SLOW_TX      //TX slow controlled by dos tool


#if defined(MAC_TEST_VLAN_TX) || defined(MAC_TEST_VLAN_RX)
#define VLANDESC 0x1234
#endif

//GMAC IO Register setting
//#define IOREG_IOBASE              0xB2000000
//#define OOBMAC_IOBASE			0xB2000000
#define OOBMAC_IOBASE			0xBAF70000  //for FP OOB MAC1 slave
#define NCSI_IOBASE				0xBAFC0000 


#define MAC_IDR0				0x0000
#define MAC_IDR4				0x0004
#define MAC_FLAG0				0x0006
#define MAC_FLAG1				0x0007
#define MAC_MAR0				0x0008
#define MAC_MAR4				0x000C
#define MAC_OOB_PORT0			0x0010
#define MAC_OOB_PORT1			0x0012
#define MAC_OOB_PORT2			0x0014
#define MAC_OOB_PORT3			0x0016
#define MAC_DMDSAR				0x001C //DMEM Master Descriptor Start Address
#define MAC_DSDSAR				0x0020 //DMEM Slave Descriptor Start Address
#define MAC_TNPDS				0x0024
#define MAC_RDSAR				0x0028
#define MAC_IMR					0x002C
#define MAC_ISR					0x002E
#define MAC_TPPoll				0x0030
#define MAC_HREQ				0x0034
#define MAC_DASH_REQUEST		0x0035
#define MAC_CMD					0x0036
#define MAC_TxCR				0x0040
#define MAC_RxCR				0x0044
#define MAC_CPCR				0x0048
#define MAC_MITI				0x004A
#define MAC_DTCCR				0x0050
#define MAC_CONF3				0x0054
#define MAC_CPUSLOW				0x0060
#define IO_IB_ACC_DATA          0x00A0
#define IO_IB_ACC_SET           0x00A4
#define MAC_DBG_SEL				0x00F0

#define MAC_EXTR_INT			0x0100
#define MAC_ISOLATE_INT			0x0100

#define MAC_STATUS				0x0104
#define MAC_OOB2IB_INT			0x0108
#define MAC_MUTEXIB				0x0110
#define MAC_MUTEXOOB			0x0114
#define MAC_MUTEX_C				0x0118
#define MAC_MP					0x011c
#define MAC_FID					0x0120
#define MAC_SYNC 				0x0123
#define MAC_OOBREG				0x0128
#define MAC_DF0					0x0130
#define MAC_HN0					0x0140
#define MAC_BLK_RST				0x0150
#define MAC_OOBLANWAKE_CTRL		0x0154
#define MAC_DMEMSTA				0x0180
#define MAC_DMEMENDA            0x0184
#define MAC_DUMMY_CPUSLEEP      0x0188
#define MAC_DUMMY_INFORM_IB     0x018C

//in EQC test
#define MAC_DUMMY_INFORM_IB_DATA      0x0190  //it is different between in EQC test and in combine test
#define MAC_DUMMY_INFORM_IB_EVENT     0x0194
#define MAC_DUMMY_INFORM_IB_SWINT     0x0198
//in combine test
#define OOB_IB_OOBMAC_RXCNT      0x0190  //it is different between in EQC test and in combine test
#define OOB_IB_PCIEHOST_TXCNT      0x0194
#define OOB_IB_PCIEHOST_RXCNT      0x0198

#define OOB_IB_DUMMY      0x019C

#define MAC_TCAM_DATA           0x02B0
#define MAC_TCAM_PORT           0x02B4
#define MAC_TCAM_DOUT           0x02B8
#define MAC_TCAM_VOUT           0x02BC
#define MAC_PKT_RULE_ACT0       0x0200
#define MAC_PKT_RULE0           0x0300
#define MAC_PKT_RULE1           0x0310
#define MAC_PKT_RULE_EN         0x02F0
#define MAC_GPIOCTL             0x0500
#define MAC_GPIOCTL2 			0x0504
#define MAC_GPIOCTL3            0x0508
#define MAC_GPIOCTL4 			0x050C
#define MAC_SWINT_IMR 			0x060C
#define MAC_SWINT_ISR 			0x060E
#define MAC_SWINT_SET 			0x0612

#if defined(MAC_LARGE_GIANT_SEND)
#define	LargeTXLengthMax		65536
#define	GiantTXLengthMax		81920
#define	TXLengthMin				1515   //for debug 
//#define DMEMLen					245760
#define TXLength				81920   //80K
#else
#define	TXLengthMax		    1510
#define	RXLengthMax		    1510
#define	TXLengthMin			60

#define	TXBUFFERSTEP        1514
#define	RXBUFFERSTEP        1514

#define DMEMLen		        4096
#define TXLength	        1510
#endif

#if defined(MAC_LARGE_GIANT_SEND) || defined(MAC2_LARGE_GIANT_SEND)
#define TxdescNumber_OOBMAC 3
#define RxdescNumber_OOBMAC	3

/*#elif defined RKVM_OOBMAC
#define TxdescNumber_OOBMAC 4
#define RxdescNumber_OOBMAC	4//Max.=16*/

#else 
//#define TxdescNumber_OOBMAC 5    //Max.=16
//#define RxdescNumber_OOBMAC	8//Max.=16
#define TxdescNumber_OOBMAC 4    //Max.=16
#define RxdescNumber_OOBMAC	4    //Max.=16
#endif

/*#ifdef VGA_TEST_ENABLED
#define TxdescNumber_VGA 16    //Max.=16
#define RxdescNumber_VGA 16
#endif*/


//--------------------
enum
{
OPT_NOCS = 0,
OPT_IPCS = 1,
OPT_TCPCS = 2,
OPT_UDPCS = 4,
OPT_FREE  = 16,
OPT_IPV6  = 64,
};
//--------------------
enum
{
OCP_ACCIB = 1<<10,
OCP_TER = 1<<9,
OCP_RER = 1<<8,
OCP_SW_INT = 1<<6,
OCP_FOVW = 1<<4,
OCP_TDU = 1<<3,
OCP_TOK = 1<<2,
OCP_RDU = 1<<1,
OCP_ROK = 1<<0,
};
enum
{
EXTR_INT_ISOLATE = 1<<0,
EXTR_INT_WAKEUP = 1<<1,
EXTR_INT_GPI = 1<<3,
EXTR_INT_GPO = 1<<4,
EXTR_INT_GPIO1 = 1<<5,
EXTR_INT_GPIO2 = 1<<6,
EXTR_INT_GPIO3 = 1<<7,
EXTR_INT_GPIO4 = 1<<8,
EXTR_INT_GPIO5 = 1<<9,
EXTR_INT_GPIO6 = 1<<10,
EXTR_INT_GPIO7 = 1<<11,
};

enum
{
RCR_AR = 1<<4,
RCR_AER = 1<<5,
RCR_AFL = 1<<6,
RCR_VLANDETAGGING=1<<7,
RCR_FLOW_RDU_EN=1<<8,
};

#if    defined(MAC_DEBUG_COUNT)
	   #define DEFAULT_IMR (OCP_RER|OCP_SW_INT|OCP_RDU|OCP_ROK|OCP_TOK|OCP_TER)
#elif  defined(MAC_reg_InterruptMitigation)
	   #define DEFAULT_IMR (OCP_TOK|OCP_TER|OCP_ROK|OCP_RER|OCP_SW_INT)
#else
	   #define DEFAULT_IMR (OCP_SW_INT|OCP_RDU|OCP_ROK)
#endif

#if    defined(ISOLATE_INTERRUPT_TEST)
	   #define DEFAULT_EXTR_INT (EXTR_INT_ISOLATE)
#elif  defined(GPIO_INT_TEST)
	   #define DEFAULT_EXTR_INT (EXTR_INT_GPI|EXTR_INT_GPIO1|EXTR_INT_GPIO2|EXTR_INT_GPIO3|EXTR_INT_GPIO4|EXTR_INT_GPIO5|EXTR_INT_GPIO6|EXTR_INT_GPIO7|EXTR_INT_GPO)	   
#elif  defined(CPU_SLEEP)
	   #define DEFAULT_EXTR_INT (EXTR_INT_WAKEUP)
#else
	   #define DEFAULT_EXTR_INT (EXTR_INT_ISOLATE|EXTR_INT_WAKEUP)
#endif

#define DEFAULT_SWINT_IMR 0xFFFF

//--------------------
typedef struct _RxDesc{
INT32U		Length:14;
INT32U		TCPF:1;
INT32U		UDPF:1;
INT32U		IPF:1;
INT32U		PKTFNO:4;
INT32U		TCPT:1;
INT32U		UDPT:1;
INT32U		V4F:1;
INT32U		V6F:1;
INT32U		BRO:1;
INT32U		PHY:1;
INT32U		MAR:1;
INT32U		LS:1;
INT32U		FS:1;
INT32U		EOR:1;
INT32U		OWN:1;
//----------
INT32U		VLanTag:16;
INT32U		TAVA:1;
INT32U		RSVD:14;
INT32U		GAME:1;
//----------
INT32U		BufferAddress;
INT32U		BufferAddressHigh;
}RXdesc;
//--------------------
/*typedef struct _TCPHO_V6F{
	INT32U		TCPHO:10;
	INT32U		V6F:1;

}TCPHO_V6F;*/

typedef struct _TxDesc_COM{
INT32U		Length:16;
INT32U		RSVD0:9;
INT32U		GTSENV6:1;
INT32U		GTSENV4:1;
INT32U		LGSEN:1;
INT32U		LS:1;
INT32U		FS:1;
INT32U		EOR:1;
INT32U		OWN:1;
//----------
INT32U		VLanTag:16;
INT32U		RSVD1:1;
INT32U		TAGC:1;
/*union {
	struct TCPHO_V6F *normal;
	INT32U		     MSS:11;
}MSS_V6F;*/
INT32U			 MSS:11;
INT32U		IPV4CS:1;
INT32U		TCPCS:1;
INT32U		UDPCS:1;
//----------
INT32U		BufferAddress;
INT32U		BufferAddressHigh;
} TXdesc_COM;//using union combine normal desc and large desc

typedef struct _TxDesc{
INT32U		Length:16;
INT32U		RSVD0:9;
INT32U		GTSENV6:1;
INT32U		GTSENV4:1;

INT32U		RSVD1:1;
INT32U		LS:1;
INT32U		FS:1;
INT32U		EOR:1;
INT32U		OWN:1;
//----------
INT32U		VLanTag:16;
INT32U		LGSEN:1;
INT32U		TAGC:1;
INT32U		TCPHO:10;
INT32U		V6F:1;
INT32U		IPV4CS:1;
INT32U		TCPCS:1;
INT32U		UDPCS:1;
//----------
INT32U		BufferAddress;
INT32U		BufferAddressHigh;
} TXdesc;

typedef struct _LargeTxDesc{
INT32U		Length:16;
INT32U		RSVD0:9;
INT32U		GTSENV6:1;
INT32U		GTSENV4:1;

INT32U		LGSEN:1;
INT32U		LS:1;
INT32U		FS:1;
INT32U		EOR:1;
INT32U		OWN:1;
//----------
INT32U		VLanTag:16;
INT32U		RSVD1:1;
INT32U		TAGC:1;
INT32U		MSS:11;
INT32U		RSVD2:3;
//----------
INT32U		BufferAddress;
INT32U		BufferAddressHigh;
} LargeTxDesc;

typedef struct _GiantTxDesc{
INT32U		Length:18;
INT32U		TCPHO:7;
INT32U		GTSENV6:1;
INT32U		GTSENV4:1;
INT32U		LGSEN:1;
INT32U		LS:1;
INT32U		FS:1;
INT32U		EOR:1;
INT32U		OWN:1;
//----------
INT32U		VLanTag:16;
INT32U		RSVD1:1;
INT32U		TAGC:1;
INT32U		MSS:11;
INT32U		RSVD2:3;
//----------
INT32U		BufferAddress;
INT32U		BufferAddressHigh;
} GiantTxDesc;

typedef struct _TallyCnt{
    INT32U	TxOkLow;
    INT32U  TxOkHigh;
    INT32U	RxOkLow;
    INT32U	RxOkHigh;
    INT32U	TxErrLow;
    INT32U	TxErrHigh;
    INT32U	RxErrLow;
    INT16U	MissPkt;
    INT16U	FAE;
    INT32U	Tx1Col;
    INT32U	TxMCol;
    INT32U	RxOkPhyLow;
    INT32U	RxOkPhyHigh;
    INT32U	RxOkBrdLow;
    INT32U	RxOkBrdHigh;
    INT32U	RxOkMul;
    INT16U	TxAbt;
    INT16U	TxUndrn;
}TallyCnt;


typedef struct _RxCheck
{
	INT32U	RxTotal_TCPIPv4_CheckSumErr;
	INT32U	RxTotal_UDPIPv4_CheckSumErr;
	INT32U	RxTotal_IPv4_CheckSumErr;
	
	INT32U	RxTotal_TCPIPv6_CheckSumErr;
	INT32U	RxTotal_UDPIPv6_CheckSumErr;
	
	#ifdef MAC_TEST_VLAN_RX
	INT32U  RxTotal_Vlan_Unmatched;
	#endif
	INT32U  RxTotal_Normal_lenthErr;

}RxCheck;

enum pktType{
	NORMALPKT,
	IPV4,
	IPV4TCP,
	IPV4UDP,
	IPV6TCP,
	IPV6UDP,
	DEBUGPKT,
	LARGE,
	GIANTIPV4,
	GIANTIPV6,
	ERRORPKT,
};

#define DWBIT00 	0x00000001
#define DWBIT01 	0x00000002
#define DWBIT02 	0x00000004
#define DWBIT03 	0x00000008
#define DWBIT04 	0x00000010
#define DWBIT05 	0x00000020
#define DWBIT06 	0x00000040
#define DWBIT07 	0x00000080
#define DWBIT08 	0x00000100
#define DWBIT09 	0x00000200
#define DWBIT10 	0x00000400
#define DWBIT11 	0x00000800
#define DWBIT12 	0x00001000
#define DWBIT13 	0x00002000
#define DWBIT14 	0x00004000
#define DWBIT15 	0x00008000
#define DWBIT16 	0x00010000
#define DWBIT17 	0x00020000
#define DWBIT18 	0x00040000
#define DWBIT19 	0x00080000
#define DWBIT20 	0x00100000
#define DWBIT21 	0x00200000
#define DWBIT22 	0x00400000
#define DWBIT23 	0x00800000
#define DWBIT24 	0x01000000
#define DWBIT25 	0x02000000
#define DWBIT26 	0x04000000
#define DWBIT27 	0x08000000
#define DWBIT28 	0x10000000
#define DWBIT29 	0x20000000
#define DWBIT30 	0x40000000
#define DWBIT31 	0x80000000

/*-----------------------------
|	  						|
|		TCAM Entry No.		|
|							|
-----------------------------*/
#if 0
#define TCAM_Entry_Number						512
/*MAC*/
#define TCAM_MAC_Start_In_TCAM					0
#define TCAM_MAC_Number_Of_Set					10
#define TCAM_MAC_Entry_Per_Set					3
#define TCAM_MAC_Total_Entry					TCAM_MAC_Number_Of_Set*TCAM_MAC_Entry_Per_Set
/*VLAN*/
#define TCAM_VLAN_Start_In_TCAM					TCAM_MAC_Start_In_TCAM+TCAM_MAC_Total_Entry
#define TCAM_VLAN_Number_Of_Set					16
#define TCAM_VLAN_Entry_Per_Set					1
#define TCAM_VLAN_Total_Entry					TCAM_VLAN_Number_Of_Set*TCAM_VLAN_Entry_Per_Set
/*TYPE*/
#define TCAM_TYPE_Start_In_TCAM					TCAM_VLAN_Start_In_TCAM+TCAM_VLAN_Total_Entry
#define TCAM_TYPE_Number_Of_Set					16
#define TCAM_TYPE_Entry_Per_Set					1
#define TCAM_TYPE_Total_Entry					TCAM_TYPE_Number_Of_Set*TCAM_TYPE_Entry_Per_Set
/*PTL IPv4*/
#define TCAM_PTLv4_Start_In_TCAM				TCAM_TYPE_Start_In_TCAM+TCAM_TYPE_Total_Entry
#define TCAM_PTLv4_Number_Of_Set				12
#define TCAM_PTLv4_Entry_Per_Set				1
#define TCAM_PTLv4_Total_Entry					TCAM_PTLv4_Number_Of_Set*TCAM_PTLv4_Entry_Per_Set
/*PTL IPv6*/
#define TCAM_PTLv6_Start_In_TCAM				TCAM_PTLv4_Start_In_TCAM+TCAM_PTLv4_Total_Entry
#define TCAM_PTLv6_Number_Of_Set				12
#define TCAM_PTLv6_Entry_Per_Set				1
#define TCAM_PTLv6_Total_Entry					TCAM_PTLv6_Number_Of_Set*TCAM_PTLv6_Entry_Per_Set
/*SIPv4*/
#define TCAM_SIPv4_Start_In_TCAM				TCAM_PTLv6_Start_In_TCAM+TCAM_PTLv6_Total_Entry
#define TCAM_SIPv4_Number_Of_Set				4
#define TCAM_SIPv4_Entry_Per_Set				2
#define TCAM_SIPv4_Total_Entry					TCAM_SIPv4_Number_Of_Set*TCAM_SIPv4_Entry_Per_Set
/*DIPv4*/
#define TCAM_DIPv4_Start_In_TCAM				TCAM_SIPv4_Start_In_TCAM+TCAM_SIPv4_Total_Entry
#define TCAM_DIPv4_Number_Of_Set				12
#define TCAM_DIPv4_Entry_Per_Set				2
#define TCAM_DIPv4_Total_Entry					TCAM_DIPv4_Number_Of_Set*TCAM_DIPv4_Entry_Per_Set
/*SIPv6*/
#define TCAM_SIPv6_Start_In_TCAM				TCAM_DIPv4_Start_In_TCAM+TCAM_DIPv4_Total_Entry
#define TCAM_SIPv6_Number_Of_Set				4
#define TCAM_SIPv6_Entry_Per_Set				8
#define TCAM_SIPv6_Total_Entry					TCAM_SIPv6_Number_Of_Set*TCAM_SIPv6_Entry_Per_Set
/*DIPv6*/
#define TCAM_DIPv6_Start_In_TCAM				TCAM_SIPv6_Start_In_TCAM+TCAM_SIPv6_Total_Entry
#define TCAM_DIPv6_Number_Of_Set				12
#define TCAM_DIPv6_Entry_Per_Set				8
#define TCAM_DIPv6_Total_Entry					TCAM_DIPv6_Number_Of_Set*TCAM_DIPv6_Entry_Per_Set
/*SPORT*/
#define TCAM_SPORT_Start_In_TCAM				TCAM_DIPv6_Start_In_TCAM+TCAM_DIPv6_Total_Entry
#define TCAM_SPORT_Number_Of_Set				70//30
#define TCAM_SPORT_Entry_Per_Set				1
#define TCAM_SPORT_Total_Entry					TCAM_SPORT_Number_Of_Set*TCAM_SPORT_Entry_Per_Set
/*DPORT*/
#define TCAM_DPORT_Start_In_TCAM				TCAM_SPORT_Start_In_TCAM+TCAM_SPORT_Total_Entry
#define TCAM_DPORT_Number_Of_Set				128//72
#define TCAM_DPORT_Entry_Per_Set				1
#define TCAM_DPORT_Total_Entry					TCAM_DPORT_Number_Of_Set*TCAM_DPORT_Entry_Per_Set
/*Teredo SPORT*/
#define TCAM_Teredo_SPORT_Start_In_TCAM			TCAM_DPORT_Start_In_TCAM+TCAM_DPORT_Total_Entry
#define TCAM_Teredo_SPORT_Number_Of_Set			1
#define TCAM_Teredo_SPORT_Entry_Per_Set			1
#define TCAM_Teredo_SPORT_Total_Entry			TCAM_Teredo_SPORT_Number_Of_Set*TCAM_Teredo_SPORT_Entry_Per_Set
/*Teredo DPORT*/
#define TCAM_Teredo_DPORT_Start_In_TCAM			TCAM_Teredo_SPORT_Start_In_TCAM+TCAM_Teredo_SPORT_Total_Entry
#define TCAM_Teredo_DPORT_Number_Of_Set			1
#define TCAM_Teredo_DPORT_Entry_Per_Set			1
#define TCAM_Teredo_DPORT_Total_Entry			TCAM_Teredo_DPORT_Number_Of_Set*TCAM_Teredo_DPORT_Entry_Per_Set
/*UDP_ESP SPORT*/
#define TCAM_UDP_ESP_SPORT_Start_In_TCAM		TCAM_Teredo_DPORT_Start_In_TCAM+TCAM_Teredo_DPORT_Total_Entry
#define TCAM_UDP_ESP_SPORT_Number_Of_Set		1
#define TCAM_UDP_ESP_SPORT_Entry_Per_Set		1
#define TCAM_UDP_ESP_SPORT_Total_Entry			TCAM_UDP_ESP_SPORT_Number_Of_Set*TCAM_UDP_ESP_SPORT_Entry_Per_Set
/*UDP_ESP DPORT*/
#define TCAM_UDP_ESP_DPORT_Start_In_TCAM		TCAM_UDP_ESP_SPORT_Start_In_TCAM+TCAM_UDP_ESP_SPORT_Total_Entry
#define TCAM_UDP_ESP_DPORT_Number_Of_Set		1
#define TCAM_UDP_ESP_DPORT_Entry_Per_Set		1
#define TCAM_UDP_ESP_DPORT_Total_Entry			TCAM_UDP_ESP_DPORT_Number_Of_Set*TCAM_UDP_ESP_DPORT_Entry_Per_Set
/*OFFSET*/
#define TCAM_OFFSET_Start_In_TCAM				TCAM_UDP_ESP_DPORT_Start_In_TCAM+TCAM_UDP_ESP_DPORT_Total_Entry
#define TCAM_OFFSET_Number_Of_Set				64
#define TCAM_OFFSET_Entry_Per_Set				1
#define TCAM_OFFSET_Total_Entry					TCAM_OFFSET_Number_Of_Set*TCAM_OFFSET_Entry_Per_Set
typedef enum{
	RET_FAIL,
	RET_OK,
}Ret_Code_et;

typedef enum{
	RX_Command = 0,
		RX_Init,
		RX_Read_Entry,
		RX_Write_Entry,
		RX_TmpMem2OOB,
		RX_OOB2TmpMem,
		RX_RuleSetting,
		RX_RuleSwitch,
		RX_SetIBDefault,
		RX_ResetCount,
		RX_GetCount,
		RX_OffsetSetting,
		RX_MARGroupSetting,
		RX_IBWriteMACIO,
	TX_Command,
		TX_CheckOK,
		TX_RetCount,
		
	TCAM_Command_Max
}TCAM_Command_et;

typedef enum{
	Switch_OFF,
	Switch_ON,
}ONOFF_Switch_et;

/*rule field*/
typedef enum{
	MAC_0,//0
	MAC_1,
	MAC_2,
	MAC_3,
	MAC_4,
	MAC_5,
	MAC_6,
	MAC_7,
	MAC_8,
	MAC_9,//9

	MARI,
	MARO,
	BRD,//12

	VLAN_0,
	VLAN_1,
	VLAN_2,//15
	VLAN_3,
	VLAN_4to5,
	VLAN_6to10,
	VLAN_11to15,

	TYPE_0,//20
	TYPE_1,
	TYPE_2,
	TYPE_3,
	TYPE_4,
	TYPE_5,
	TYPE_6,
	TYPE_7,
	TYPE_8to11,
	TYPE_12to15,
	
	PTL_0,//30
	PTL_1,
	PTL_2,
	PTL_3,
	PTL_4,
	PTL_5,
	PTL_6,
	PTL_7,
	PTL_8,
	PTL_9,
	PTL_10,//40
	PTL_11,
	
	SIP_0,
	SIP_1,
	SIP_2,
	SIP_3,
	
	DIP_0,
	DIP_1,
	DIP_2,
	DIP_3,
	DIP_4,//50
	DIP_5,
	DIP_6,
	DIP_7,
	DIP_8,
	DIP_9,
	DIP_10,
	DIP_11,
	
	SPORT_0to4,
	SPORT_5to9,
	SPORT_10to14,//60
	SPORT_15to19,
	SPORT_20to24,
	SPORT_25to29,
	SPORT_30to39,
	SPORT_40to49,
	SPORT_50to59,
	SPORT_60to69,
	
	DPORT_0to9,
	DPORT_10to19,
	DPORT_20to29,//70
	DPORT_30to39,
	DPORT_40to49,
	DPORT_50to59,
	DPORT_60to69,
	DPORT_70to79,
	DPORT_80to89,
	DPORT_90to99,
	DPORT_100to109,
	DPORT_110to127,
	
	OFFSET_0to3,//80
	OFFSET_4to7,
	OFFSET_8to11,
	OFFSET_12to15,
	OFFSET_16to19,
	OFFSET_20to23,
	OFFSET_24to27,
	OFFSET_28to31,
	OFFSET_32to35,
	OFFSET_36to39,
	OFFSET_40to43,//90
	OFFSET_44to47,
	OFFSET_48to51,
	OFFSET_52to55,
	OFFSET_56to59,
	OFFSET_60to63,//95
	
	MAX_RULE_NUMBER,
	OUT_OF_RANGE,
}RuleFormat_et;

typedef enum{
	TCAM_Read=0,
	TCAM_Write,
	TCAM_RW_MAX,
}TCAM_RW_et;

typedef enum{
	TCAM_MAC,
	TCAM_VLAN,
	TCAM_TYPE,
	TCAM_PTLv4,
	TCAM_PTLv6,
	TCAM_SIPv4,
	TCAM_DIPv4,
	TCAM_SIPv6,
	TCAM_DIPv6,
	TCAM_SPORT,
	TCAM_DPORT,
	TCAM_Teredo_SPORT,
	TCAM_Teredo_DPORT,
	TCAM_UDP_ESP_SPORT,
	TCAM_UDP_ESP_DPORT,
	TCAM_OFFSET,
	MAX_TCAM_Entry_Type,
	//----------------------
	
	TCAM_MARIB,
	TCAM_MAROOB,
}TCAM_Entry_Type_et;

/*TCAM Property*/
typedef struct
{
	INT8U number;
	INT8U Gamming:1;
	INT8U Meter_En:1;
	INT8U Meter_No:3;
	INT8U Drop:1;
	INT8U OOB:1;
	INT8U IB:1;
}TCAM_RuleActionSet_st;


typedef enum{
	TCAM_data,
	TCAM_care,
}TCAM_ReadType_et;

/*TCAM Entry Setting*/
typedef struct
{
	/*when read TCAM data to this variable must know if "DontCareBit" not equal to 0xFFFF, the data just one kind of combination*/
	INT16U Value;
	/*If don't care bit exist set "1" to this bit position*/
	/*i.e. 0xFFFF=0000 0000 0000 0000->no dont care bit; 0x0007=0000 0000 0000 0111->BIT1,BIT2,BIT3 is don't care bit*/
	INT16U DontCareBit;
	INT8U Valid;
}TCAM_Entry_Setting_st;

/*TCAM Property*/
typedef struct
{
	INT16U Start_In_TCAM[MAX_TCAM_Entry_Type];
	INT16U Number_Of_Set[MAX_TCAM_Entry_Type];
	INT16U Entry_Per_Set[MAX_TCAM_Entry_Type];
}TCAM_Property_st;




/*list of rule number*/
typedef enum
{
	RULE_NO_0,
	RULE_NO_1,
	RULE_NO_2,
	RULE_NO_3,
	RULE_NO_4,
	RULE_NO_5,
	RULE_NO_6,
	RULE_NO_7,
	RULE_NO_8,
	RULE_NO_9,
	RULE_NO_10,
	RULE_NO_11,
	RULE_NO_12,
	RULE_NO_13,
	RULE_NO_14,
	RULE_NO_15,
	RULE_NO_MAX,
}Rule_NO_et;

#endif
/*We change packet header case by case. However, we do not change anything from this offset*/
#define UNCHANGEHDROFFSET 135

void bsp_oobmac_handler(void);
void bsp_oobmac_init(void);
void __inline__ bsp_oobmac_send(INT32U pktType,const INT32U DDR_start_addr,const INT8U pktnum,INT32U blocknum,INT32U Block_V_offset,INT8U BPP);
void __inline__ bsp_oobmac_test(const int pktType, const INT8U isChecksum);
void __inline__ pseudoheader(INT8U *sotreHere,const int pktType,const INT8U iplen);
void __inline__ fillDummyData(INT8U *pktBuf, const INT8U startValue, const int len);
void __inline__ setMACHeader(INT8U *pktBuf, INT8U *ethernetPro);
void __inline__ setIPv4Header(INT8U *pktBuf, const INT8U len, const int nextProtocol);
void __inline__ setIPv6Header(INT8U *pktBuf, const int pktType);
void bsp_4everloop(INT16U from);
void gmac_dump_tally_counter(void);
void bsp_oobmac_init_packet(void);
void RKVM_OOBMAC_send(INT8U *payload_ptr, volatile INT16U length_payload);
void bsp_oobmac_send_VGA(INT8U pktType,const INT32U start_addr,const INT8U pktnum,INT32U blocknum,INT8U BPP);

//void OOBMAC_Test_Task(void);
void test_flow_OOBMAC(void);


#if 0//From Han
Ret_Code_et TCAM_OCP_Write(INT16U DataBit, INT16U DontCareBit, INT8U Valid, INT16U entry_number,ONOFF_Switch_et data, ONOFF_Switch_et care, ONOFF_Switch_et valid);
Ret_Code_et TCAM_WriteRule(Rule_NO_et number, RuleFormat_et bit, ONOFF_Switch_et OnOff);
Ret_Code_et TCAM_OCP_Read(INT32U entry_number, TCAM_ReadType_et type, TCAM_Entry_Setting_st *pstTCAM_Table);
Ret_Code_et __TCAM_OCP_Read(INT32U entry_number, TCAM_ReadType_et type, TCAM_Entry_Setting_st *pstTCAM_Table);
Ret_Code_et TCAM_AccessEntry(TCAM_Entry_Type_et Type, INT16U Number, INT16U Set, TCAM_RW_et RW, TCAM_Entry_Setting_st *value);
void PacketFillDefault(void);
void PacketFilterSettingMAC(INT8U MAC_Set, INT8U *MAC_Address);
void PacketFilterSettingIPv4(INT8U IPv4_Set, INT8U *DIPv4);
void PacketFilterSettingIPv6(INT8U IPv6_Set, INT8U *DIPv6);
void PacketFilterSettingPort(INT8U Port_Set, INT16U PortValue);
void PacketFilterSettingType(INT8U Type_Set, INT16U TypeValue);
void PacketFilterSettingPTLv4(INT8U PTL_Set, INT8U PTLv4);
void PacketFilterSettingPTLv6(INT8U PTL_Set, INT8U PTLv6);
Ret_Code_et PacketFilterRuleEn(Rule_NO_et number, ONOFF_Switch_et OnOff);
#endif

#endif

