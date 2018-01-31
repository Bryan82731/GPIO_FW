#ifndef __WCOM_H__
#define __WCOM_H__

#include "soap.h"

#define WSMANREQ 1
#define OSPUSHDATA 2
#define DASHOSPSHLEN ETH_PAYLOAD_LEN

#define OOB_SET_IP 11
#define OOB_SET_SRV 12

#define OOB_GET_IP 15
#define OOB_GET_SRV 16
#define OOB_GET_IB_IP 17
#define OOB_GET_FW_VERSION 18

#define OOB_SEND_TEST_START 21
#define OOB_SEND_TEST_STOP 22
#define OOB_SEND_TEST 23
#define IB_SEND_TEST 24

#define OOB_REQ_CLIPORT 30
#define OOB_RELEASE_CLIPORT 31
#define OOB_REQ_ALLOW 32
#define OOB_REQ_DENY 33
#define OOB_REQ_PROV 34
#define OOB_SET_PROV 35
#define OOB_REQ_OPAQ_S 36
#define OOB_REQ_OPAQ_E 37
#define OOB_SET_BLO_NUM 38
#define OOB_SET_OPAQ_S 39
#define OOB_SET_OPAQ_E 40
#define OOB_REQ_OPAQ_BLO 41
#define OOB_OPAQ_CRE_BLO 42
#define OOB_OPAQ_DEL_BLO 43
#define OOB_SET_DPCONF   44
#define OOB_SET_ASFTBL   45

#define SENSOR_INBAND 95
#define Get_HM_Sensor 96
#define Set_HM_Sensor 97
#define SMBIOS_CHECK 98
#define SMBIOS_WMI 99
#define OOB_WMI 100

#define IB_mDNSOFFLOAD_CMD 			210
#define IB_mDNSOFFLOAD_RR 			215
#define IB_mDNSOFFLOAD_UDP_PORT 216
#define IB_mDNSOFFLOAD_TCP_PORT 217
#define IB_mDNSOFFLOAD_MAC_ADDR 218

#define OOBMWT 3*OS_TICKS_PER_SEC

#define PCI_DMA_OK 0
#define PCI_BRIDGE_FAIL -1
#define AP_NOT_READY -2
#define AP_NOT_RESPONSE -3

#define PCIE_NO_ACK  88

typedef struct _OSOOBHdr{
	unsigned int len;
	unsigned char type;
	unsigned char flag;
	unsigned char hostReqV;
	unsigned char res;	
}OSOOBHdr;

typedef struct _ProvRecord
{
	INT8U DomainName[MAX_DN_LEN];
	INT16U ProvisioningSetup:1, ProvisioningMode:1, 
					ProvisioningState: 2, PKIEnabled:1, PSKEnabled:1;
	INT16U ProvisionServerPort;
	IPAddress ProvisionServerIP;
	INT8U ProvisionServerName[MAX_PS_LEN];
    struct _pkidata pkidata;
    struct _pskdata pskdata;	
    INT8U OTP[MAX_OTP_LEN];	
}ProvRecord;
typedef struct _OpaqueData
{		
	unsigned char end;
	unsigned char block;
	unsigned short offset;	
	unsigned short len;
	unsigned char buf[1024];
}OpaqueData; 

typedef struct _IPv4ChgInfo
{
	char IPver;
	char isDHCP;
	char IPv4addr[IPv4_ADR_LEN];
	char IPv4mask[IPv4_ADR_LEN];
	char IPv4Gateway[IPv4_ADR_LEN];
	char IPv4DNS[IPv4_ADR_LEN];
}IPv4ChgInfo;

typedef struct _IPv6ChgInfo
{
	char IPver;
	char isDHCP;
	char PrefixLen;
	char unUsed;
	char IPv6addr[16];	
	char IPv6Gateway[16];
	char IPv6DNS[16];
}IPv6ChgInfo;
	
typedef struct _OOBSrvSetting{
	unsigned char EchoService:1, httpService:1, httpsService:1, wsmanService:1;
}OOBSrvSetting;

void ComWinTask(void *p_arg);
void OOBResponseTask(void *p_arg);

#endif
