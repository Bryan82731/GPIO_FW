#ifndef __HWPF_H__
#define __HWPF_H__

#include <rlx/rlx_types.h>

#define UNICAST_MAR	1
#define MULCAST_MAR	2
#define BROADCAST_MAR	4
 
#define MAC_ADDR_LEN    6
 
#define IPv4 4
#define IPv6 6

#define IP_PROTO_ICMP 0x01
#define IP_PROTO_ICMPv6 0x3A
#define IP_PROTO_UDP 0x11
#define IP_PROTO_TCP 0x06
 
#define IPv4_ADR_LEN 4
#define IPv6_ADR_LEN 16
#define PORT_ADR_LEN 2
#define TYPE_ADR_LEN 2
#define PTL_ADR_LEN 1

#define TCAM_Entry_Invalid 0xFFFFFFFF
#define TCAM_Entry_Available		0xFF
#define TCAM_Entry_Default			0xFE
#define TCAM_PTL_NoExist			0xFF

#define NOSPEC	0xFF

typedef struct
{
 unsigned char ruleNO;
 unsigned char isSet:1, FRIBNotDrop:1, FROOB:1,IBMAR:1, OOBMAR:1;
 unsigned short RSV1;

 unsigned short MARType;
 unsigned short MACIDBitMap; //MACID0: IB, MACID1: IB 
 unsigned short VLANBitMap;
 unsigned short TypeBitMap; //Type0: IPv4, Type1: IPv6, Type2:ARP, Type3:VLAN
 unsigned short IPv4PTLBitMap; //IPv4PTL:TCP, IPv4PTL1:UDP, IPv4PTL2:ICMP, IPv4PTL3:AH, IPv4PTL4:ESP, IPv4PTL5:IPV4-IPv6
 unsigned short IPv6PTLBitMap;
 unsigned short DIPv4BitMap;
 unsigned short DIPv6BitMap;
 unsigned int	DPortBitMap;
}TCAMRule;


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

typedef struct
{
	INT16U Start_In_TCAM	[MAX_TCAM_Entry_Type];
	INT16U Number_Of_Set	[MAX_TCAM_Entry_Type];
	INT16U Entry_Per_Set	[MAX_TCAM_Entry_Type];
}TCAM_Property_st;

typedef struct
{	
   TCAM_Entry_Type_et type;
   unsigned int num;   
   unsigned char* data;
}TCAM_Allocation;


/*TCAM Rule Action*/
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
	TCAM_Read=0,
	TCAM_Write,
	TCAM_RW_MAX,
}TCAM_RW_et;

typedef enum{
	RET_FAIL,
	RET_OK,
}Ret_Code_et;

typedef enum{
	Switch_OFF,
	Switch_ON,
}ONOFF_Switch_et;

typedef enum{
	TCAM_data,
	TCAM_care,
}TCAM_ReadType_et;

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
	RULE_SETTING_FAIL,
}Rule_NO_et;
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

	TYPE_0,//20  /*IPv4*/
	TYPE_1,		/*IPv6*/
	TYPE_2,		/*ARP*/
	TYPE_3,		/*VLAN*/
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
#define TCAM_Entry_Number						512
/*MAC*/
#define TCAM_MAC_Start_In_TCAM				0
#define TCAM_MAC_Number_Of_Set				10
#define TCAM_MAC_Entry_Per_Set				3
#define TCAM_MAC_Total_Entry					TCAM_MAC_Number_Of_Set*TCAM_MAC_Entry_Per_Set
/*VLAN*/
#define TCAM_VLAN_Start_In_TCAM				TCAM_MAC_Start_In_TCAM+TCAM_MAC_Total_Entry
#define TCAM_VLAN_Number_Of_Set				16
#define TCAM_VLAN_Entry_Per_Set				1
#define TCAM_VLAN_Total_Entry					TCAM_VLAN_Number_Of_Set*TCAM_VLAN_Entry_Per_Set
/*TYPE*/
#define TCAM_TYPE_Start_In_TCAM				TCAM_VLAN_Start_In_TCAM+TCAM_VLAN_Total_Entry
#define TCAM_TYPE_Number_Of_Set				16
#define TCAM_TYPE_Entry_Per_Set				1
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
#define TCAM_SPORT_Number_Of_Set			70//30
#define TCAM_SPORT_Entry_Per_Set				1
#define TCAM_SPORT_Total_Entry				TCAM_SPORT_Number_Of_Set*TCAM_SPORT_Entry_Per_Set
/*DPORT*/
#define TCAM_DPORT_Start_In_TCAM				TCAM_SPORT_Start_In_TCAM+TCAM_SPORT_Total_Entry
#define TCAM_DPORT_Number_Of_Set			128//72
#define TCAM_DPORT_Entry_Per_Set				1
#define TCAM_DPORT_Total_Entry				TCAM_DPORT_Number_Of_Set*TCAM_DPORT_Entry_Per_Set
/*Teredo SPORT*/
#define TCAM_Teredo_SPORT_Start_In_TCAM		TCAM_DPORT_Start_In_TCAM+TCAM_DPORT_Total_Entry
#define TCAM_Teredo_SPORT_Number_Of_Set		1
#define TCAM_Teredo_SPORT_Entry_Per_Set		1
#define TCAM_Teredo_SPORT_Total_Entry			TCAM_Teredo_SPORT_Number_Of_Set*TCAM_Teredo_SPORT_Entry_Per_Set
/*Teredo DPORT*/
#define TCAM_Teredo_DPORT_Start_In_TCAM		TCAM_Teredo_SPORT_Start_In_TCAM+TCAM_Teredo_SPORT_Total_Entry
#define TCAM_Teredo_DPORT_Number_Of_Set		1
#define TCAM_Teredo_DPORT_Entry_Per_Set		1
#define TCAM_Teredo_DPORT_Total_Entry			TCAM_Teredo_DPORT_Number_Of_Set*TCAM_Teredo_DPORT_Entry_Per_Set
/*UDP_ESP SPORT*/
#define TCAM_UDP_ESP_SPORT_Start_In_TCAM	TCAM_Teredo_DPORT_Start_In_TCAM+TCAM_Teredo_DPORT_Total_Entry
#define TCAM_UDP_ESP_SPORT_Number_Of_Set	1
#define TCAM_UDP_ESP_SPORT_Entry_Per_Set		1
#define TCAM_UDP_ESP_SPORT_Total_Entry		TCAM_UDP_ESP_SPORT_Number_Of_Set*TCAM_UDP_ESP_SPORT_Entry_Per_Set
/*UDP_ESP DPORT*/
#define TCAM_UDP_ESP_DPORT_Start_In_TCAM	TCAM_UDP_ESP_SPORT_Start_In_TCAM+TCAM_UDP_ESP_SPORT_Total_Entry
#define TCAM_UDP_ESP_DPORT_Number_Of_Set	1
#define TCAM_UDP_ESP_DPORT_Entry_Per_Set		1
#define TCAM_UDP_ESP_DPORT_Total_Entry		TCAM_UDP_ESP_DPORT_Number_Of_Set*TCAM_UDP_ESP_DPORT_Entry_Per_Set
/*OFFSET*/
#define TCAM_OFFSET_Start_In_TCAM			TCAM_UDP_ESP_DPORT_Start_In_TCAM+TCAM_UDP_ESP_DPORT_Total_Entry
#define TCAM_OFFSET_Number_Of_Set			64
#define TCAM_OFFSET_Entry_Per_Set				1
#define TCAM_OFFSET_Total_Entry				TCAM_OFFSET_Number_Of_Set*TCAM_OFFSET_Entry_Per_Set

#define REG32(reg)			(*(volatile INT32U *)(reg))
#define REG16(reg)			(*(volatile INT16U *)(reg))
#define REG8(reg)		  	(*(volatile INT8U  *)(reg))

#define IO_TCAM_DATA			0x02B0
#define IO_TCAM_PORT			0x02B4
#define IO_TCAM_DOUT			0x02B8
#define IO_TCAM_VOUT			0x02BC
#define IO_PKT_RULE_ACT0		0x0200
#define IO_PKT_LKBT0_SET0	0x0210	
#define IO_PKT_CLR			0x0250
#define IO_PKT_RULE0			0x0300
#define IO_PKT_RULE1			0x0310
#define IO_PKT_RULE_EN		0x02F0

static const TCAM_Property_st TCAM_Property = {
	{
		TCAM_MAC_Start_In_TCAM,			TCAM_VLAN_Start_In_TCAM,
		TCAM_TYPE_Start_In_TCAM,			TCAM_PTLv4_Start_In_TCAM,
		TCAM_PTLv6_Start_In_TCAM,			TCAM_SIPv4_Start_In_TCAM,
		TCAM_DIPv4_Start_In_TCAM, 			TCAM_SIPv6_Start_In_TCAM,
		TCAM_DIPv6_Start_In_TCAM,			TCAM_SPORT_Start_In_TCAM,
		TCAM_DPORT_Start_In_TCAM,			TCAM_Teredo_SPORT_Start_In_TCAM,
		TCAM_Teredo_DPORT_Start_In_TCAM,	TCAM_UDP_ESP_SPORT_Start_In_TCAM,
		TCAM_UDP_ESP_DPORT_Start_In_TCAM,	TCAM_OFFSET_Start_In_TCAM,
	},	
	{
		TCAM_MAC_Number_Of_Set,			TCAM_VLAN_Number_Of_Set,
		TCAM_TYPE_Number_Of_Set,			TCAM_PTLv4_Number_Of_Set,
		TCAM_PTLv6_Number_Of_Set,			TCAM_SIPv4_Number_Of_Set,
		TCAM_DIPv4_Number_Of_Set, 			TCAM_SIPv6_Number_Of_Set,
		TCAM_DIPv6_Number_Of_Set,			TCAM_SPORT_Number_Of_Set,
		TCAM_DPORT_Number_Of_Set,		TCAM_Teredo_SPORT_Number_Of_Set,
		TCAM_Teredo_DPORT_Number_Of_Set,	TCAM_UDP_ESP_SPORT_Number_Of_Set,
		TCAM_UDP_ESP_DPORT_Number_Of_Set,	TCAM_OFFSET_Number_Of_Set,
	},
	{
		TCAM_MAC_Entry_Per_Set,			TCAM_VLAN_Entry_Per_Set,
		TCAM_TYPE_Entry_Per_Set,			TCAM_PTLv4_Entry_Per_Set,
		TCAM_PTLv6_Entry_Per_Set,			TCAM_SIPv4_Entry_Per_Set,
		TCAM_DIPv4_Entry_Per_Set, 			TCAM_SIPv6_Entry_Per_Set,
		TCAM_DIPv6_Entry_Per_Set,			TCAM_SPORT_Entry_Per_Set,
		TCAM_DPORT_Entry_Per_Set,			TCAM_Teredo_SPORT_Entry_Per_Set,
		TCAM_Teredo_DPORT_Entry_Per_Set,	TCAM_UDP_ESP_SPORT_Entry_Per_Set,
		TCAM_UDP_ESP_DPORT_Entry_Per_Set,	TCAM_OFFSET_Entry_Per_Set,
	},
};

#if 1//PKTFilter_Test
Ret_Code_et TCAM_OCP_Write(INT16U DataBit, INT16U DontCareBit, INT8U Valid, INT16U entry_number,
										ONOFF_Switch_et data, ONOFF_Switch_et care, ONOFF_Switch_et valid);
Ret_Code_et TCAM_WriteRule(Rule_NO_et number, RuleFormat_et bit, ONOFF_Switch_et OnOff);
void PacketFilterInit(void);

void clearAllPFRule();
void setTCAMData(TCAM_Allocation* info);
Rule_NO_et setPFRule(TCAMRule* rule);
void RstSharePFRuleMem(TCAMRule* rstRule, TCAMRule* copyRule);
void SetOOBBasicRule();
void EnableIBIPv6ICMPPFRule();
void DisableIBIPv6ICMPPFRule();
void EnableOOBDHCPv6PFRule();
void DisableOOBDHCPv6PFRule();
#endif

/*----------------------------Software Define-------------------------------------*/


enum PFRuleIdx
{
	ArpFRule =0,
	OOBUnicastPFRule = 1, //Packets match OOB Mac Address, IPv4, TCP/UCP/ICMP will be accepted
	OOBIPv6PFRule = 2, //Packets match OOB Mac Address/Multicast address, IPv6 will be accepted
	OOBPortPFRule = 3, //Filter multicast/broadcast packets(Do not care ip version?)
	IBIPv4TCPPortPFRule = 1,
	IBIPv4UDPPortPFRule = 2,
	IBIPv6TCPPortPFRule = 3,
	IBIPv6UDPPortPFRule = 4,		
	IBIPv6ICMPPFRule = 5,		
	NumOfPFRule,
};

enum TCAMSetIdx
{
	TCAMMacIDSet = 0,
	TCAMVLANTagSet,
	TCAMTypeSet,
	TCAMIPv4PTLSet,
	TCAMIPv6PTLSet,
	TCAMDIPv4Set,
	TCAMDIPv6Set,
	TCAMDDPortSet,		
	NumOfTCAMSet,
};

enum IPv4ListIdx
{
	UniIPv4Addr = 0,
	LBIPv4Addr,
	GBIPv4Addr,
	MIPv4Addr,	
	NumOfIPv4Addr,
};

enum IPv6ListIdx
{
	UniIPv6Addr = 0,
	LLIPv6Addr,
	NumOfIPv6Addr,
};


enum EthTypeBitMap
{
	IPv4TypeBitMap = 1<<0,
	IPv6TypeBitMap = 1<<1,
	ARPTypeBitMap = 1<<2,
	VLANTypeBitMap = 1<<3,	
};

enum MacIDBitMap
{
	IBMacBitMap = 1<<0,
	OOBMacBitMap = 1<<1,
	Mac8021xBitMap = 1<<2,
	PTPv1MacBitMap = 1<<3,	
	PTPv2MacBitMap = 1<<3,	
};


enum IPv4AddrBitMap
{
	EnableUniIPv4Addr = 1<<0,
	EnableLBIPv4Addr = 1<<1,
	EnableGBIPv4Addr = 1<<2,
	EnableMIPv4Addr = 1<<3,
};

enum IPv6AddrBitMap
{
	EnableUniIPv6Addr = 1<<0,
	EnableLLLIPv6Addr = 1<<1,
	EnableMIPv6Addr = 1<<2,
};

enum IPv4PTLBitMap
{
	IPv4PTLTCP = 	1<<0,
	IPv4PTLUDP = 	1<<1,
	IPv4PTLICMP = 	1<<2,
	IPv4PTLAH =	1<<3,
	IPv4PTLESP =	1<<4,
	IPv4PTLIPV4IPV6 =1<<5,	
};

enum IPv6PTLBitMap
{
	IPv6PTLTCP =	1<<0,
	IPv6PTLUDP =	1<<1,
	IPv6PTLICMP =	1<<2,
	IPv6PTLAH = 1<<3,
	IPv6PTLESP =	1<<4,
	IPv6PTLHopByHop =1<<5,	
	IPv6PTLRoute =1<<6,
	IPv6PTLDest =1<<7,		
	IPv6PTLFrag =1<<8,

};

void hwPFInit();
#endif
