#include <bsp.h>
#include "hwpf.h"
#include "lib.h"

extern DPCONF *dpconf;

static Rule_NO_et 	Rule_NO 				= 0;

extern void setIPAddress_withoutFlash(unsigned int ip, unsigned char intf);
ROM_EXTERN TCAM_Allocation TCAMMem[NumOfTCAMSet];
ROM_EXTERN TCAMRule TCAMRuleMem[NumOfPFRule];
ROM_EXTERN Ret_Code_et TCAM_OCP_Read(INT32U entry_number, TCAM_ReadType_et type, TCAM_Entry_Setting_st *pstTCAM_Table);

/*****************************************************************************
 * Author : Han Wang (HanWang@realtek.com)

 * Create date : 2011/05/12
 
 * DESCRIPTION: TCAM_OCP_Read_Native
 
 *****************************************************************************/
Ret_Code_et TCAM_OCP_Read_Native(INT32U entry_number, TCAM_ReadType_et type, TCAM_Entry_Setting_st *pstTCAM_Table)
{
	INT32U reg_data = 0;
	
	if(entry_number >TCAM_Entry_Number)
		return RET_FAIL;

	/*TCAM_ACC_FLAG*/
	reg_data |= DWBIT31;
	/*RW_SEL*/
	reg_data &= ~DWBIT30;
	/*TCAM_WM : read type select*/
	if(type == TCAM_data)
		reg_data |= DWBIT13;
	else
		reg_data &= ~DWBIT13;
	reg_data |= DWBIT12;	
	/*TCAM_ADDR*/
	reg_data |= (entry_number&0x000001FF);
	
	REG32(IOREG_IOBASE+IO_TCAM_PORT)=reg_data;
	while((DWBIT31&REG32(IOREG_IOBASE+IO_TCAM_PORT))== DWBIT31){};

	reg_data = 0;
	reg_data = REG32(IOREG_IOBASE+IO_TCAM_DOUT);
	
	if(type == TCAM_data)
		pstTCAM_Table->Value = 0x0000FFFF & reg_data;
	else
		pstTCAM_Table->DontCareBit = 0x0000FFFF & (reg_data>>16);

	//pstTCAM_Table->Valid = ((DWBIT16 & reg_data) >> 16);
	pstTCAM_Table->Valid = REG32(IOREG_IOBASE+IO_TCAM_VOUT)&0x01;

	return RET_OK;
}

/*****************************************************************************
 * Author : Han Wang (HanWang@realtek.com)

 * Create date : 2011/03/23
 
 * DESCRIPTION: TCAM_OCP_Write
 
 *****************************************************************************/
Ret_Code_et TCAM_OCP_Write(INT16U DataBit, INT16U DontCareBit, INT8U Valid, INT16U entry_number,
										ONOFF_Switch_et data, ONOFF_Switch_et care, ONOFF_Switch_et valid)
{
	INT32U reg_data = 0;
	TCAM_Entry_Setting_st stTCAM_Table={0};
	
	//patch solution
	TCAM_OCP_Read(entry_number, TCAM_data, &stTCAM_Table);
	TCAM_OCP_Read(entry_number, TCAM_care, &stTCAM_Table);
	
	reg_data |= (DontCareBit<<16)&0xFFFF0000;
	reg_data |= DataBit;
	REG32(IOREG_IOBASE+IO_TCAM_DATA) = reg_data;
	
	reg_data = 0;
	/*TCAM_ACC_FLAG*/
	reg_data |= DWBIT31;
	/*RW_SEL*/
	reg_data |= DWBIT30;
	/*VALID BIT*/
	reg_data |= (Valid&0x01)<<16;
	/*TCAM_WM : valid*/	
	if(data == Switch_OFF)
		reg_data |= DWBIT15;
	/*TCAM_WM : care*/	
	if(care == Switch_OFF)
		reg_data |= DWBIT14;
	/*TCAM_WM : data*/
	if(valid == Switch_OFF)
		reg_data |= DWBIT13;
	/*TCAM_ADDR*/
	reg_data |= (entry_number&0x000001FF);
	
	REG32(IOREG_IOBASE+IO_TCAM_PORT) = reg_data;
	while((DWBIT31&REG32(IOREG_IOBASE+IO_TCAM_PORT))== DWBIT31);

	return RET_OK;
}

/*****************************************************************************
 * Author : Han Wang (HanWang@realtek.com)

 * Create date : 2011/03/23
 
 * DESCRIPTION: TCAM_OCP_Read
 
 *****************************************************************************/
Ret_Code_et TCAM_OCP_Read(INT32U entry_number, TCAM_ReadType_et type, TCAM_Entry_Setting_st *pstTCAM_Table)
{
	volatile INT32U reg_data = 0;
	
	if(entry_number >TCAM_Entry_Number)
		return RET_FAIL;

	/*TCAM_ACC_FLAG*/
	reg_data |= DWBIT31;
	/*RW_SEL*/
	reg_data &= ~DWBIT30;
	/*TCAM_WM : read type select*/
	if(type == TCAM_data)
		reg_data |= DWBIT13;
	else
		reg_data &= ~DWBIT13;
	reg_data |= DWBIT12;	
	/*TCAM_ADDR*/
	reg_data |= (entry_number&0x000001FF);
	
	REG32(IOREG_IOBASE+IO_TCAM_PORT)=reg_data;
	while((DWBIT31&REG32(IOREG_IOBASE+IO_TCAM_PORT))== DWBIT31){};

	reg_data = 0;
	reg_data = REG32(IOREG_IOBASE+IO_TCAM_DOUT);
	
	if(type == TCAM_data)
		pstTCAM_Table->Value = 0x0000FFFF & reg_data;
	else
		pstTCAM_Table->DontCareBit = 0x0000FFFF & (reg_data>>16);

	//pstTCAM_Table->Valid = ((DWBIT16 & reg_data) >> 16);
	pstTCAM_Table->Valid = REG32(IOREG_IOBASE+IO_TCAM_VOUT)&0x01;

	return RET_OK;
	#if 0
	volatile INT32U reg_data = 0;
	if(entry_number >TCAM_Entry_Number)
		return RET_FAIL;

	/*TCAM_ACC_FLAG*/
	reg_data |= DWBIT31;
	/*RW_SEL*/
	reg_data &= ~DWBIT30;
	/*TCAM_WM : read type select*/
	//if(type == TCAM_data)
	//	reg_data |= DWBIT13;
	//else
	//	reg_data &= ~DWBIT13;
	/*TCAM_ADDR*/
	reg_data |= entry_number&0x000001FF;
	
	REG32(IOREG_IOBASE+IO_TCAM_PORT)=reg_data;
	while((DWBIT31&REG32(IOREG_IOBASE+IO_TCAM_PORT))== DWBIT31){};

	reg_data = 0;
	reg_data = REG32(IOREG_IOBASE+IO_TCAM_DOUT);
	
	//if(type == TCAM_data)
		pstTCAM_Table->Value = 0x0000FFFF & reg_data;
	//else
		//phase2
		pstTCAM_Table->DontCareBit = (INT32U)(0xFFFF0000 & reg_data)>>16;
		//pstTCAM_Table->DontCareBit = 0x0000FFFF & reg_data;

	//phase2
	pstTCAM_Table->Valid = REG32(IOREG_IOBASE+IO_TCAM_VOUT);
	//pstTCAM_Table->Valid = ((DWBIT16 & reg_data) >> 16);

	return RET_OK;
	#endif
}


Ret_Code_et TCAM_AccessEntry(TCAM_Entry_Type_et Type, INT16U Number, INT16U Set, TCAM_RW_et RW, TCAM_Entry_Setting_st *value)
{
	INT32U entry_number = 0;
	Ret_Code_et ret = RET_OK;
	
	if(Number >= TCAM_Property.Entry_Per_Set[Type])
		return RET_FAIL;
	if(Set >= TCAM_Property.Number_Of_Set[Type])
		return RET_FAIL;
	if(RW >= TCAM_RW_MAX)
		return RET_FAIL;
	if(value == NULL)
		return RET_FAIL;

	entry_number = TCAM_Property.Start_In_TCAM[Type] + (Set*TCAM_Property.Entry_Per_Set[Type])+Number;

	if(RW == TCAM_Read)
	{
		ret = TCAM_OCP_Read(entry_number, TCAM_data, value);
		//ret = TCAM_OCP_Read(entry_number, TCAM_care, value);
	}
	else
		ret = TCAM_OCP_Write(value->Value, value->DontCareBit, value->Valid, entry_number, Switch_ON, Switch_ON, Switch_ON);

	return ret;
}


void PacketFilterClearEntryValid(TCAM_Entry_Type_et Enrty_Type, INT8U Set, ONOFF_Switch_et Valid)
{
	TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};
	INT8U i = 0;

	for(i=0; i<TCAM_Property.Entry_Per_Set[Enrty_Type]; i++)
	{
		st_TCAM_Entry_Setting.Value = 0;
		st_TCAM_Entry_Setting.DontCareBit = 0;
		st_TCAM_Entry_Setting.Valid = 0;
		TCAM_AccessEntry(Enrty_Type, i, Set, TCAM_Write, &st_TCAM_Entry_Setting);
	}
}

void PacketFilterSettingMAC(INT8U MAC_Set, INT8U *MAC_Address, ONOFF_Switch_et Valid)
{
	TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};
	INT8U i = 0;

	for(i=0; i<TCAM_Property.Entry_Per_Set[TCAM_MAC]; i++)
	{
		st_TCAM_Entry_Setting.Value = MAC_Address[i*2]<<8|MAC_Address[i*2+1];
		st_TCAM_Entry_Setting.DontCareBit = 0;
		st_TCAM_Entry_Setting.Valid = Valid;
		TCAM_AccessEntry(TCAM_MAC, i, MAC_Set, TCAM_Write, &st_TCAM_Entry_Setting);
	}
}

void PacketFilterSettingIPv4(INT8U IPv4_Set, INT8U *DIPv4, ONOFF_Switch_et Valid)
{
	TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};
	INT8U i = 0;

	for(i=0; i<TCAM_Property.Entry_Per_Set[TCAM_DIPv4]; i++)
	{
		st_TCAM_Entry_Setting.Value = DIPv4[i*2]<<8|DIPv4[i*2+1];
		st_TCAM_Entry_Setting.DontCareBit = 0;
		st_TCAM_Entry_Setting.Valid = Valid;
		TCAM_AccessEntry(TCAM_DIPv4, i, IPv4_Set, TCAM_Write, &st_TCAM_Entry_Setting);
	}
}

void PacketFilterSettingIPv6(INT8U IPv6_Set, INT8U *DIPv6, ONOFF_Switch_et Valid)
{
	TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};
	INT8U i = 0;

	for(i=0; i<TCAM_Property.Entry_Per_Set[TCAM_DIPv6]; i++)
	{
		st_TCAM_Entry_Setting.Value = DIPv6[i*2]<<8|DIPv6[i*2+1];
		st_TCAM_Entry_Setting.DontCareBit = 0;
		st_TCAM_Entry_Setting.Valid = Valid;
		TCAM_AccessEntry(TCAM_DIPv6, i, IPv6_Set, TCAM_Write, &st_TCAM_Entry_Setting);
	}
}

void PacketFilterSettingPort(INT8U Port_Set, INT16U PortValue, ONOFF_Switch_et Valid)
{
	TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};

	st_TCAM_Entry_Setting.Value = PortValue;
	st_TCAM_Entry_Setting.DontCareBit = 0;
	st_TCAM_Entry_Setting.Valid = Valid;
	TCAM_AccessEntry(TCAM_DPORT, 0, Port_Set, TCAM_Write, &st_TCAM_Entry_Setting);
}

void PacketFilterSettingType(INT8U Type_Set, INT16U TypeValue, ONOFF_Switch_et Valid)
{
	TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};

	st_TCAM_Entry_Setting.Value = TypeValue;
	st_TCAM_Entry_Setting.DontCareBit = 0;
	st_TCAM_Entry_Setting.Valid = Valid;
	TCAM_AccessEntry(TCAM_TYPE, 0, Type_Set, TCAM_Write, &st_TCAM_Entry_Setting);
}

void PacketFilterSettingPTLv4(INT8U PTL_Set, INT8U PTLv4, ONOFF_Switch_et Valid)
{
	TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};
	
	st_TCAM_Entry_Setting.Value = PTLv4;
	st_TCAM_Entry_Setting.DontCareBit = 0xFF00;
	st_TCAM_Entry_Setting.Valid = Valid;
	TCAM_AccessEntry(TCAM_PTLv4, 0, PTL_Set, TCAM_Write, &st_TCAM_Entry_Setting);	
}

void PacketFilterSettingPTLv6(INT8U PTL_Set, INT8U PTLv6, ONOFF_Switch_et Valid)
{
	TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};

	st_TCAM_Entry_Setting.Value = PTLv6<<8;
	st_TCAM_Entry_Setting.DontCareBit = 0x00FF;
	st_TCAM_Entry_Setting.Valid = Valid;
	TCAM_AccessEntry(TCAM_PTLv6, 0, PTL_Set, TCAM_Write, &st_TCAM_Entry_Setting);
}

Ret_Code_et PacketFilterRuleEn(Rule_NO_et number, ONOFF_Switch_et OnOff)
{
	INT32U data = 0;
	
	if(number > RULE_NO_MAX)
	{
		return RET_FAIL;
	}

	if(OnOff == Switch_ON)
	{
		data = REG32(IOREG_IOBASE+IO_PKT_RULE_EN);
		data |= (INT32U)0x00000001 << number;
		data |= DWBIT31;
		REG32(IOREG_IOBASE+IO_PKT_RULE_EN)=data;
		while((DWBIT31&REG32(IOREG_IOBASE+IO_PKT_RULE_EN))== DWBIT31){};
	}
	else
	{
		data = REG32(IOREG_IOBASE+IO_PKT_RULE_EN);
		data &= ~((INT32U)0x00000001 << number);
		data |= DWBIT31;
		REG32(IOREG_IOBASE+IO_PKT_RULE_EN)=data;
		while((DWBIT31&REG32(IOREG_IOBASE+IO_PKT_RULE_EN))== DWBIT31){};
	}

}

void TCAM_RuleActionSet(TCAM_RuleActionSet_st* act)
{
	INT32U value = 0;
	INT32U offset = 0;
	INT32U ActValue = 0;

	ActValue = (act->IB | act->OOB<<1 | act->Drop<<2 | act->Meter_No<<3 | act->Meter_En<<6 | act->Gamming<<7);
	offset = (act->number/4)*4;
	value = REG32(IOREG_IOBASE+IO_PKT_RULE_ACT0+offset);
	value &= ~((INT32U)0x000000FF<<((act->number%4)*8));
	value |= ActValue<<((act->number%4)*8);
	REG32(IOREG_IOBASE+IO_PKT_RULE_ACT0+offset) = value;
}

/*****************************************************************************
 * Author : Han Wang (HanWang@realtek.com)

 * Create date : 2011/03/25
 
 * DESCRIPTION: TCAM_RuleSetting
 
 *****************************************************************************/
Ret_Code_et TCAM_WriteRule(Rule_NO_et number, RuleFormat_et bit, ONOFF_Switch_et OnOff)
{
	INT8U *rule = 0;

	if(number > RULE_NO_MAX || bit >MAX_RULE_NUMBER)
		return RET_FAIL;

	rule = (INT8U *)(IOREG_IOBASE + IO_PKT_RULE0);
	rule = rule + (sizeof(INT32U)*4*number);
	rule = rule + (sizeof(INT32U)*(bit>>5));

	if(OnOff == Switch_ON)
		{REG32(rule) |= (0x01<<(bit%32));}
	else
		{REG32(rule) &= (0x01<<(bit%32));}

	return RET_OK;
}

RuleFormat_et TCAM_GetRuleBit (TCAM_Entry_Type_et Type, INT16U Set)
{
	switch(Type)
	{
		case TCAM_MAC:
				return MAC_0+Set;
		case TCAM_MARIB:
				return MARI;
		case TCAM_MAROOB:
				return MARO;
		case TCAM_VLAN:
				if(Set<=3)
					return VLAN_0+Set;
				else if(Set<=5)
					return VLAN_4to5;
				else if(Set<=10)
					return VLAN_6to10;
				else if(Set<=15)
					return VLAN_11to15;
				else
					return OUT_OF_RANGE;
		case TCAM_TYPE:
				if(Set<=7)
					return TYPE_0+Set;
				else if(Set<=11)
					return TYPE_8to11;
				else if(Set<=15)
					return TYPE_12to15;
				else
					return OUT_OF_RANGE;
		case TCAM_PTLv4:
		case TCAM_PTLv6:
				return PTL_0+Set;
		case TCAM_SIPv4:
		case TCAM_SIPv6:
				return SIP_0+Set;
		case TCAM_DIPv4:
		case TCAM_DIPv6:
				return DIP_0+Set;			
		case TCAM_SPORT:
				if(Set<=4)
					return SPORT_0to4;
				else if(Set<=9)
					return SPORT_5to9;
				else if(Set<=14)
					return SPORT_10to14;
				else if(Set<=19)
					return SPORT_15to19;
				else if(Set<=24)
					return SPORT_20to24;
				else if(Set<=29)
					return SPORT_25to29;
				else if(Set<=39)
					return SPORT_30to39;
				else if(Set<=49)
					return SPORT_40to49;
				else if(Set<=59)
					return SPORT_50to59;
				else if(Set<=69)
					return SPORT_60to69;
				else
					return OUT_OF_RANGE;
		case TCAM_DPORT:
				if(Set<=9)
					return DPORT_0to9;
				else if(Set<=19)
					return DPORT_10to19;
				else if(Set<=29)
					return DPORT_20to29;
				else if(Set<=39)
					return DPORT_30to39;
				else if(Set<=49)
					return DPORT_40to49;
				else if(Set<=59)
					return DPORT_50to59;
				else if(Set<=69)
					return DPORT_60to69;
				else if(Set<=79)
					return DPORT_70to79;
				else if(Set<=89)
					return DPORT_80to89;
				else if(Set<=99)
					return DPORT_90to99;
				else if(Set<=109)
					return DPORT_100to109;
				else if(Set<=127)
					return DPORT_110to127;
				else
					return OUT_OF_RANGE;
		case TCAM_Teredo_SPORT:			
		case TCAM_Teredo_DPORT:
		case TCAM_UDP_ESP_SPORT:
		case TCAM_UDP_ESP_DPORT:
			return 100;
		case TCAM_OFFSET:
				if(Set<=3)
					return OFFSET_0to3;
				else if(Set<=7)
					return OFFSET_4to7;
				else if(Set<=11)
					return OFFSET_8to11;
				else if(Set<=15)
					return OFFSET_12to15;
				else if(Set<=19)
					return OFFSET_16to19;
				else if(Set<=23)
					return OFFSET_20to23;
				else if(Set<=27)
					return OFFSET_24to27;
				else if(Set<=31)
					return OFFSET_28to31;
				else if(Set<=35)
					return OFFSET_32to35;
				else if(Set<=39)
					return OFFSET_36to39;
				else if(Set<=43)
					return OFFSET_40to43;
				else if(Set<=47)
					return OFFSET_44to47;
				else if(Set<=51)
					return OFFSET_48to51;
				else if(Set<=55)
					return OFFSET_52to55;
				else if(Set<=59)
					return OFFSET_56to59;
				else if(Set<=63)
					return OFFSET_60to63;				
				else
					return OUT_OF_RANGE;
		default:
			return OUT_OF_RANGE;
	}
}


void PacketFillDefault(void)
{
	INT8U MAC_Address[6];
	
	GetMacAddr_F(MAC_Address, eth0);
	
	PacketFilterSettingMAC(1, &MAC_Address, Switch_ON);
	
	PacketFilterSettingType(0, 0x0800, Switch_ON);
	PacketFilterSettingType(1, 0x86DD, Switch_ON);
	PacketFilterSettingType(2, 0x0806, Switch_ON);
	PacketFilterSettingType(3, 0x8100, Switch_ON);

	PacketFilterSettingPTLv4(0, 0x06, Switch_ON);
	PacketFilterSettingPTLv4(1, 0x11, Switch_ON);
	PacketFilterSettingPTLv4(2, 0x01, Switch_ON);
	PacketFilterSettingPTLv4(3, 0x32, Switch_ON);
	PacketFilterSettingPTLv4(4, 0x33, Switch_ON);
	PacketFilterSettingPTLv4(5, 0x29, Switch_ON);

	PacketFilterSettingPTLv6(0, 0x06, Switch_ON);
	PacketFilterSettingPTLv6(1, 0x11, Switch_ON);
	PacketFilterSettingPTLv6(2, 0x3A, Switch_ON);
	PacketFilterSettingPTLv6(3, 0x32, Switch_ON);
	PacketFilterSettingPTLv6(4, 0x33, Switch_ON);
	PacketFilterSettingPTLv6(5, 0x00, Switch_ON);
	PacketFilterSettingPTLv6(6, 0x2B, Switch_ON);
	PacketFilterSettingPTLv6(7, 0x3C, Switch_ON);
	PacketFilterSettingPTLv6(8, 0x2C, Switch_ON);
	
}

void PacketFilterInit(void)
{
	INT32U i = 0, j = 0;

	//TCAM vaild bit clear and all Leaky Bucket clear
	REG32(IOREG_IOBASE+IO_PKT_CLR)=0x800000FF;
	bsp_wait(100);

	//Fill default value
	PacketFillDefault();
	
	//disable all rule to every rule
	for(i=0; i<RULE_NO_MAX; i++)
	{
		for(j=0; j<MAX_RULE_NUMBER; j++)
		{
			TCAM_WriteRule(i, j, Switch_OFF);
		}
	}

	//Rule 0 use IB RCR setting and disable all rule
	REG32(IOREG_IOBASE+IO_PKT_RULE_EN) = 0x10000000;
	while(DWBIT31 == (DWBIT31 & REG32(IOREG_IOBASE+IO_PKT_RULE_EN))){};

	//enable rule 0
	//PacketFilterRuleEn(RULE_NO_0, Switch_ON);
	//PacketEntryRuleTabRst();
}

INT32U PacketFilterChkPTLv4Exist(INT8U protocol)
{
	INT32U i;
	TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};
	
	for(i=0; i<6; i++)
	{
		TCAM_AccessEntry(TCAM_PTLv4, 0, i, TCAM_Read, &st_TCAM_Entry_Setting);
		if(st_TCAM_Entry_Setting.Value&0x00FF == protocol)
			return i;
	}

	return TCAM_PTL_NoExist;
}

INT32U PacketFilterChkPTLv6Exist(INT8U protocol)
{
	INT32U i;
	TCAM_Entry_Setting_st  st_TCAM_Entry_Setting = {0,};
	
	for(i=0; i<9; i++)
	{
		TCAM_AccessEntry(TCAM_PTLv6, 0, i, TCAM_Read, &st_TCAM_Entry_Setting);
		if((st_TCAM_Entry_Setting.Value&0xFF00)>>8 == protocol)
			return i;
	}

	return TCAM_PTL_NoExist;
}

Rule_NO_et PacketFilterSetRule(TCAMRule* rule, Rule_NO_et rule_no)
{
	INT32U i = 0;
	TCAM_RuleActionSet_st act = {0,};
	
	if(rule_no >= RULE_NO_MAX)
		return RULE_SETTING_FAIL;			
	
	if(rule->MARType & UNICAST_MAR)
	{
		for(i=0; i<16; i++)
		{
			if(rule->MACIDBitMap& (INT16U)0x01<<i)
				TCAM_WriteRule(rule_no, TCAM_GetRuleBit(TCAM_MAC, i), Switch_ON);
		}
	}
	
	if(rule->MARType & MULCAST_MAR)
	{
		if(rule->IBMAR)
			TCAM_WriteRule(rule_no, MARI, Switch_ON);
		if(rule->OOBMAR)
			TCAM_WriteRule(rule_no, MARO, Switch_ON);
	}

	if(rule->MARType & BROADCAST_MAR)
		TCAM_WriteRule(rule_no, BRD, Switch_ON);

	/*VLAN*/
	for(i=0; i<16; i++)
	{
		if(rule->VLANBitMap& (INT16U)0x01<<i)
			TCAM_WriteRule(rule_no, TCAM_GetRuleBit(TCAM_VLAN, i), Switch_ON);
	}
	/*TYPE*/
	for(i=0; i<16; i++)
	{
		if(rule->TypeBitMap& (INT16U)0x01<<i)
			TCAM_WriteRule(rule_no, TCAM_GetRuleBit(TCAM_TYPE, i), Switch_ON);
	}
	/*IPv4 Protocol*/
	for(i=0; i<16; i++)
	{
		if(rule->IPv4PTLBitMap& (INT16U)0x01<<i)
			TCAM_WriteRule(rule_no, TCAM_GetRuleBit(TCAM_PTLv4, i), Switch_ON);
	}
	/*IPv6 Protocol*/
	for(i=0; i<16; i++)
	{
		if(rule->IPv6PTLBitMap& (INT16U)0x01<<i)
			TCAM_WriteRule(rule_no, TCAM_GetRuleBit(TCAM_PTLv6, i), Switch_ON);
	}
	/*DIPv4*/
	for(i=0; i<16; i++)
	{
		if(rule->DIPv4BitMap& (INT16U)0x01<<i)
			TCAM_WriteRule(rule_no, TCAM_GetRuleBit(TCAM_DIPv4, i), Switch_ON);
	}
	/*DIPv6*/
	for(i=0; i<16; i++)
	{
		if(rule->DIPv6BitMap& (INT16U)0x01<<i)
			TCAM_WriteRule(rule_no, TCAM_GetRuleBit(TCAM_DIPv6, i), Switch_ON);
	}
	/*PORT*/
	for(i=0; i<32; i++)
	{
		if(rule->DPortBitMap& (INT16U)0x01<<i)
			TCAM_WriteRule(rule_no, TCAM_GetRuleBit(TCAM_DPORT, i), Switch_ON);
	}

	/*set action*/
	if(rule->FRIBNotDrop == 0)
		act.IB = 1;	
	if(rule->FROOB)
		act.OOB = 1;
	act.number = rule_no;
	TCAM_RuleActionSet(&act);

	/*enable rule*/
	PacketFilterRuleEn(rule_no, Switch_ON);
	
	return rule_no;
}
//========================= Sample function =========================
//Basic Flow clear -> set address -> set rule
//Clear all port filter rule
void clearAllPFRule()
{
	INT32U i = 0, j = 0;
	TCAM_RuleActionSet_st act ={0,};
	
	//TCAM vaild bit clear and all Leaky Bucket clear
	REG32(IOREG_IOBASE+IO_PKT_CLR)=0x800000FF;
	bsp_wait(100);

	//disable all rule to every rule
	for(i=0; i<RULE_NO_MAX; i++)
	{
		for(j=0; j<MAX_RULE_NUMBER; j++)
		{
			TCAM_WriteRule(i, j, Switch_OFF);
		}

		act.number = i;
		TCAM_RuleActionSet(&act);
		
		PacketFilterRuleEn(i, Switch_OFF);
	}
	
	Rule_NO = 0;
}

void setTCAMData(TCAM_Allocation* info)
{
	INT32U i;
	
	switch(info->type)
	{
		case TCAM_DIPv4:
			for(i=0; i<TCAM_Property.Number_Of_Set[info->type]; i++)
			{
				if(i<info->num)
					PacketFilterSettingIPv4(i, &(info->data[i*IPv4_ADR_LEN]), Switch_ON);
				else
					PacketFilterClearEntryValid(info->type, i, Switch_OFF);
			}
		break;

		case TCAM_DIPv6:
			for(i=0; i<TCAM_Property.Number_Of_Set[info->type]; i++)
			{
				if(i<info->num)
					PacketFilterSettingIPv6(i, &(info->data[i*IPv6_ADR_LEN]), Switch_ON);
				else
					PacketFilterClearEntryValid(info->type, i, Switch_OFF);
			}
		break;

		case TCAM_MAC:
			for(i=0; i<TCAM_Property.Number_Of_Set[info->type]; i++)
			{
				if(i<info->num)
					PacketFilterSettingMAC(i, &(info->data[i*MAC_ADDR_LEN]), Switch_ON);
				else
					PacketFilterClearEntryValid(info->type, i, Switch_OFF);
			}
		break;

		case TCAM_DPORT:
			for(i=0; i<TCAM_Property.Number_Of_Set[info->type]; i++)
			{
				if(i<info->num)
				{
				unsigned short *port = (unsigned short *)(info->data + (i*PORT_ADR_LEN));
					PacketFilterSettingPort(i, *port, Switch_ON);
				}
				else
					PacketFilterClearEntryValid(info->type, i, Switch_OFF);
			}
		break;

		case TCAM_TYPE:
			for(i=0; i<TCAM_Property.Number_Of_Set[info->type]; i++)
			{
				if(i<info->num)
				{
				unsigned short *type = (unsigned short *)(info->data + (i*TYPE_ADR_LEN));
					PacketFilterSettingType(i, *type, Switch_ON);
				}
				else
					PacketFilterClearEntryValid(info->type, i, Switch_OFF);
			}
		break;

		case TCAM_PTLv4:
			for(i=0; i<TCAM_Property.Number_Of_Set[info->type]; i++)
			{
				if(i<info->num)
					PacketFilterSettingPTLv4(i, info->data[i*PTL_ADR_LEN], Switch_ON);
				else
					PacketFilterClearEntryValid(info->type, i, Switch_OFF);
			}
		break;

		case TCAM_PTLv6:
			for(i=0; i<TCAM_Property.Number_Of_Set[info->type]; i++)
			{
				if(i<info->num)
					PacketFilterSettingPTLv6(i, info->data[i*PTL_ADR_LEN], Switch_ON);
				else
					PacketFilterClearEntryValid(info->type, i, Switch_OFF);
			}
		break;

		case TCAM_MARIB:
			for(i=0; i<8; i++)
				REG8(IOREG_IOBASE+0x0F-i) = info->data[i];
		break;

		case TCAM_MAROOB:
			for(i=0; i<8; i++)
				REG8(IOREG_IOBASE+0x1F-i) = info->data[i];
		break;
	}
	
}

Ret_Code_et chgPFRule(Rule_NO_et ruleNo, TCAMRule* rule)
{
	INT32U j=0;
	TCAM_RuleActionSet_st act ={0,};

	PacketFilterRuleEn(ruleNo, Switch_OFF);
	for(j=0; j<MAX_RULE_NUMBER; j++)
	{
		TCAM_WriteRule(ruleNo, j, Switch_OFF);
	}

	PacketFilterSetRule(rule, ruleNo);
}

Rule_NO_et setPFRule(TCAMRule* rule)
{	
	if(rule->isSet)
	{
		chgPFRule(rule->ruleNO , rule);
		return rule->ruleNO;		
	}
	else
	{
		rule->isSet = 1;
		rule->ruleNO = PacketFilterSetRule(rule, Rule_NO++);
		return rule->ruleNO;
	}
}
/*----------------------------Software Function-------------------------------------*/
void RstSharePFRuleMem(TCAMRule* rstRule, TCAMRule* copyRule)
{
	unsigned char ruleNO = rstRule->ruleNO;
	unsigned char isSet = rstRule->isSet;

	memset(rstRule, 0, sizeof(TCAMRule));
	if(copyRule)
	{
		memcpy(rstRule, copyRule, sizeof(TCAMRule));
	}
	rstRule->ruleNO = ruleNO;
	rstRule->isSet = isSet;	
}




void PFMemInit()
{
	memset(TCAMMem, 0 ,sizeof(TCAMMem));
	memset(TCAMRuleMem, 0, sizeof(TCAMRuleMem));
	//Init TCAMMem type
	TCAMMem[TCAMMacIDSet].type = TCAM_MAC;
	TCAMMem[TCAMVLANTagSet].type = TCAM_VLAN;
	TCAMMem[TCAMTypeSet].type = TCAM_TYPE;
	TCAMMem[TCAMIPv4PTLSet].type = TCAM_PTLv4;
	TCAMMem[TCAMIPv6PTLSet].type = TCAM_PTLv6;
	TCAMMem[TCAMDIPv4Set].type = TCAM_DIPv4;
	TCAMMem[TCAMDIPv6Set].type = TCAM_DIPv6;
	TCAMMem[TCAMDDPortSet].type = TCAM_DPORT;

	TCAMMem[TCAMDIPv4Set].num = NumOfIPv4Addr;
	TCAMMem[TCAMDIPv4Set].data = (unsigned char*)malloc(NumOfIPv4Addr*IPv4_ADR_LEN);
	memset(TCAMMem[TCAMDIPv4Set].data, 0 ,sizeof(NumOfIPv4Addr*IPv4_ADR_LEN));
	
	TCAMMem[TCAMDIPv6Set].num = NumOfIPv6Addr;
	TCAMMem[TCAMDIPv6Set].data = (unsigned char*)malloc(NumOfIPv6Addr*IPv6_ADR_LEN);
	memset(TCAMMem[TCAMDIPv6Set].data, 0 ,sizeof(NumOfIPv6Addr*IPv6_ADR_LEN));	
	//dpconf->HostIP[intf].addr stroe with big endian
	dpconf->HostIP[eth0].addr = ntohl(dpconf->HostIP[eth0].addr);	
	//setIPAddress_F(dpconf->HostIP[intf].addr);
	setIPAddress_withoutFlash(dpconf->HostIP[eth0].addr, eth0);
}

void SetOOBBasicRule()
{
	RstSharePFRuleMem_F(&TCAMRuleMem[ArpFRule], 0);
	RstSharePFRuleMem_F(&TCAMRuleMem[OOBUnicastPFRule], 0);
	RstSharePFRuleMem_F(&TCAMRuleMem[OOBIPv6PFRule], 0);	
	//ARP rule
	TCAMRuleMem[ArpFRule].FROOB = 1;
	TCAMRuleMem[ArpFRule].FRIBNotDrop = 1;
	TCAMRuleMem[ArpFRule].MARType = UNICAST_MAR | BROADCAST_MAR;
	TCAMRuleMem[ArpFRule].MACIDBitMap = OOBMacBitMap;
	TCAMRuleMem[ArpFRule].TypeBitMap = ARPTypeBitMap;	
	TCAMRuleMem[ArpFRule].DIPv4BitMap = EnableUniIPv4Addr; 
	TCAMRuleMem[ArpFRule].ruleNO = setPFRule_F(&TCAMRuleMem[ArpFRule]);
	
	//Accept IPv4/IPv6 udp unicast packet
	TCAMRuleMem[OOBUnicastPFRule].FROOB = 1;
	TCAMRuleMem[OOBUnicastPFRule].FRIBNotDrop = 1;
	TCAMRuleMem[OOBUnicastPFRule].MARType = UNICAST_MAR;
	TCAMRuleMem[OOBUnicastPFRule].MACIDBitMap = OOBMacBitMap;	
	TCAMRuleMem[OOBUnicastPFRule].TypeBitMap = IPv4TypeBitMap | IPv6TypeBitMap;	
	TCAMRuleMem[OOBUnicastPFRule].DIPv4BitMap = EnableUniIPv4Addr | EnableUniIPv6Addr;
	TCAMRuleMem[OOBUnicastPFRule].IPv4PTLBitMap = IPv4PTLTCP | IPv4PTLUDP | IPv4PTLICMP | IPv6PTLUDP; 
	TCAMRuleMem[OOBUnicastPFRule].ruleNO = setPFRule_F(&TCAMRuleMem[OOBUnicastPFRule]);
	
	//Accept IPv6 tcp and icmp packet match OOB mac or multicast mac
	TCAMRuleMem[OOBIPv6PFRule].FROOB = 1;
	TCAMRuleMem[OOBIPv6PFRule].OOBMAR = 1;
	TCAMRuleMem[OOBIPv6PFRule].FRIBNotDrop = 1;
	TCAMRuleMem[OOBIPv6PFRule].MARType = UNICAST_MAR | MULCAST_MAR;
	TCAMRuleMem[OOBIPv6PFRule].MACIDBitMap = OOBMacBitMap;
	TCAMRuleMem[OOBIPv6PFRule].TypeBitMap = IPv6TypeBitMap; 
	TCAMRuleMem[OOBIPv6PFRule].IPv6PTLBitMap = IPv6PTLTCP | IPv6PTLICMP; 
	TCAMRuleMem[OOBIPv6PFRule].ruleNO = setPFRule_F(&TCAMRuleMem[OOBIPv6PFRule]); 

}

void EnableIBIPv6ICMPPFRule()
{
	RstSharePFRuleMem_F(&TCAMRuleMem[IBIPv6ICMPPFRule], 0);
	TCAMRuleMem[IBIPv6ICMPPFRule].FROOB = 1;
	TCAMRuleMem[IBIPv6ICMPPFRule].FRIBNotDrop = 1;
	TCAMRuleMem[IBIPv6ICMPPFRule].MARType = UNICAST_MAR | MULCAST_MAR;
	TCAMRuleMem[IBIPv6ICMPPFRule].MACIDBitMap = OOBMacBitMap;
	TCAMRuleMem[IBIPv6ICMPPFRule].TypeBitMap = IPv6TypeBitMap;
	TCAMRuleMem[IBIPv6ICMPPFRule].IPv6PTLBitMap = IPv6PTLICMP;
	TCAMRuleMem[IBIPv6ICMPPFRule].DIPv6BitMap = EnableUniIPv6Addr; 
	TCAMRuleMem[IBIPv6ICMPPFRule].ruleNO = setPFRule_F(&TCAMRuleMem[IBIPv6ICMPPFRule]);
}

void DisableIBIPv6ICMPPFRule()
{
	//Disable IBIPv6ICMPPFRule Rule
        PacketFilterRuleEn(TCAMRuleMem[IBIPv6ICMPPFRule].ruleNO, 0);
}

void EnableOOBDHCPv6PFRule()
{
	TCAMRuleMem[OOBIPv6PFRule].IPv6PTLBitMap = IPv6PTLTCP | IPv6PTLICMP | IPv6PTLUDP; 
	TCAMRuleMem[OOBIPv6PFRule].ruleNO = setPFRule_F(&TCAMRuleMem[OOBIPv6PFRule]); 
}

void DisableOOBDHCPv6PFRule()
{
	TCAMRuleMem[OOBIPv6PFRule].IPv6PTLBitMap = IPv6PTLTCP | IPv6PTLICMP; 
	TCAMRuleMem[OOBIPv6PFRule].ruleNO = setPFRule_F(&TCAMRuleMem[OOBIPv6PFRule]); 
}

void hwPFInit()
{
	clearAllPFRule();	
	PacketFilterInit();
	PFMemInit();
	SetOOBBasicRule_F();	
}

