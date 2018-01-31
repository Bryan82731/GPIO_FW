#ifndef __SMBus_H__
#define __SMBus_H__

#include <sys/dpdef.h>
#include <rlx/rlx_types.h>

//number of SMBUS CHIPSETs supported
//#define NUM_SMBUS_CHIPSET  6 
#define ASF_TABLE_OFFSET   36

enum {STATE_UNKNOWN = 0, STATE_NORMAL, STATE_WARNING, STATE_CRITICAL, STATE_FATAL, STATE_LOWERWARNING, STATE_LOWERCRITICAL, STATE_LOWERFATAL, STATE_UPPERWARNING, STATE_UPPERCRITICAL, STATE_UPPERFATAL};

enum {ASF_INFO = 0, ASF_ALRT, ASF_RCTL, ASF_RMCP, ASF_ADDR};

enum {ASF_RESET = 0, ASF_POWER_OFF, ASF_POWER_ON, ASF_POWER_CR};

enum {LOG_INFO = 0, LOG_HW, LOG_AUTH, LOG_WARNING, LOG_ERROR, LOG_ALL, LOG_END};

enum {EVENT_NONE = 0,  EVENT_ASSERT, EVENT_DEASSERT};

enum {BIOSPWD, CASEINTRU};

enum {SNR_UNKNOWN = 0, SNR_OTHER, SNR_TEMPERATURE, SNR_VOLTAGE, SNR_CURRENT, SNR_TACHOMETER, SNR_COUNTER };

enum {SNR_INIT = 0, SNR_EXIST, SNR_READ, SNR_INVALID, SNR_FAULT};

typedef struct _snr_name_tbl
{
    INT8U  name[32];
    INT8U  address;
}snr_name_tbl_t;

typedef struct _asf_ctldata
{
    INT8U function;
    INT8U slaveaddr;
    INT8U cmd;
    INT8U data;
}asf_ctldata_t;

typedef struct _asf_alert
{
    INT8U   assert;
    INT8U   deassert;
    INT8U   exactmatch;
    INT8U   logtype:4, status:4;
    INT8U   address;
    INT8U   command;
    INT8U   datamsk;
    INT8U   cmpvalue;
    INT8U   evtsnrtype;
    INT8U   evttype;
    INT8U   evtoffset;
    INT8U   evtsrctype;
    INT8U   evtseverity;
    INT8U   snrnum;
    INT8U   entity;
    INT8U   instance;
}asf_alert_t;

typedef struct _legacy_snr
{
    INT8U  addr;
    INT8U  command;
    INT8U  addrplus;
    INT8U  type;
    INT8U  sensorindex;
}legacy_snr_t;


typedef struct _asf_config
{
    INT8U           maxwdt;
    INT8U           minsnrpoll;
    INT16U          systemid;
    INT8U           IANA[4];
    INT32U          pec;
    asf_ctldata_t   asf_rctl[4];
    INT8U           RMCPCap[7];
    INT8U           RMCPCmpCode;
    INT8U           RMCPIANA[4];
    INT8U           RMCPSpecialCmd;
    INT8U           RMCPSpecialCmdArgs[2];
    INT8U           RMCPBootOptions[2];
    INT8U           RMCPOEMArgs[2];
    INT8U           numofsnr;
    legacy_snr_t    legacysnr[16];
    INT8U           numofalerts;
    asf_alert_t     asfalerts[16];
}asf_config_t;

typedef struct _asf_header
{
    INT8U  sig[4];
    INT32U length;
    INT8U  revision;
    INT8U  chksum;
    INT8U  oemid[6];
    INT8U  oemtableid[8];
    INT32U oemrevision;
    INT32U creatorid;
    INT32U creatorrev;
}asf_header_t;

typedef struct _asf_record
{
    INT8U   type;
    INT8U   resvered;
    INT16U  length;
}asf_record_t;

typedef enum _msendtype{
   REMOTE_CONTROL = 0,
   ASF_SENSOR_POLL,
   LS_POLL,
   BMC_POLL,
   PLDM_RESPONSE,
   PLDM_REQUEST,
}msendtype;

enum{
    SMBIOS_NOT_READY = 0,
    SMBIOS_READY,
};

enum{
    INTEL = 0,
    AMD,
    NVIDIA,
    HP,
    LENOVO,
    AMDSOC
};

enum { PLDM_BLOCK_READ = 0,
       PLDM_BLOCK_WRITE,
};

enum {
       AMIBIOS = 0,
       PHOENIXBIOS,
       DELLBIOS,
       LENOVOBIOS,
};

enum {
       LEGACY = 0,
       UEFI 
};

enum {
       FSC = 0,
       PEGATRON,
       EVERLIGHT,
       GENERIC,
       SAMSUNG,
       DELL
};

enum {
    POLL_NONE = 0,
    POLL_LS,
    POLL_ASF_STATUS,
    POLL_ASF_PEND,
    POLL_ASF_DATA,
    POLL_SENSOR_DETECT,
    POLL_SENSOR,
    POLL_STOP
};

typedef struct _sensor
{
    INT8U  name[32];
    INT8U  offset[2];
    INT8U  pollindex: 4, exist:4;
    INT8U  event: 2, fault:6;
    INT32S value;
    INT8U  state;
    INT8U  prestate; 
    INT8U  type:7, signvalue:1;
    INT8U  index;
    INT32U LNC;
    INT32U UNC;
    INT32U LC;
    INT32U UC;
    INT32U LF;
    INT32U UF;
}sensor_t;

typedef struct
{
    INT8U  pollflag;
    INT8U  expired;
    INT8U  bootopt[11];
    INT8U  MsgType;
    INT8U  LSPFlag;
    INT8U  ASFFlag;
    INT8U  PollTimeOut;
    INT8U  PollType;
    INT8U  Status[28];
    INT8U  Flag[56];
    INT8U  Boottime;
    INT8U  SensorIndex;
    INT8U  LSensor[8];
    //at most support 8 legacy sensors
    sensor_t *sensor;
    INT8U  IBRmtl;
    INT8U  IBRmtlCmd;
}smbiosrmcpdata;

//total 32 bytes
typedef struct _eventdata
{
    INT32U  timestamp;
    INT8U   Event_Sensor_Type;
    INT8U   Event_Type;
    INT8U   Event_Offset;
    INT8U   Event_Source_Type;
    INT8U   Event_Severity;
    INT8U   Sensor_Device;
    INT8U   Sensor_Number;
    INT8U   Entity;
    INT8U   Entity_Instance;
    INT8U   EventData[8];
    INT8U   alertnum :6, sent: 2;
    INT16U  timeout;
    INT16U  seqnum;
    INT8U   interval;
    INT8U   watchdog :2, logtype : 6;
    struct _eventdata *next;
}eventdata;

#if 0
typedef struct _elist {
    struct _elist *next;
    eventdata  *addr;
}event_list;
#endif

typedef struct
{
    INT8U rtaddr;
    INT8U cmd;
    INT8U bytecount;
    INT8U subcmd;
    INT8U version;
    INT8U val;
}smbioshdr;

typedef struct
{
    INT8U rtslaveaddr;
    INT8U cmd;
    INT8U bytecount;
    INT8U rval;
    INT8U version;
    INT8U sstate;

}smbiosrmcp;

typedef struct
{

    INT8U  *addr;
    INT8U  status;
    INT16U length;

}smbiostable_t;

typedef struct
{
    struct
    {
        INT32U length  : 7;
        INT32U index   : 8;
        INT32U eor     : 1;
        INT32U rsvd    : 8;
    	INT32U rsvd2   : 7;
        //INT32U mctp    :  1;
        INT32U own     : 1;
    }st;
    INT8U* rx_buf_addr;

}volatile smbrxdesc_t;

typedef struct _smbus_config
{
    INT8U           module_inited;
    INT8U           freq_div;
    INT8U           freq_slave_div;
    INT8U           desc_num;
    INT8U           desc_rxsize;
    INT8U           slave_addr;
    INT8U           master_pec_enable;
    INT8U           slave_pec_enable;
    smbrxdesc_t *rxdesc;//slave rx
    smbrxdesc_t *txdesc;//slave tx
    smbrxdesc_t *mtxdesc;//master tx
    INT8U 	RxDescIndex;
}smbus_config_t;

typedef struct
{
    //1. Return Boot Options Response
    //2. No Boot Options Response
    INT8U BCount;
    INT8U SubCmd;
    INT8U VersionNumber;
    //End No Boot Options Response
    INT8U RC_IANA[4];
    INT8U SpecialCmd;
    INT8U SpCmdParameter[2];
    INT8U BootOptionsBitMask[2];
    INT8U OEMParameter[2];

}BootOptions;

#define BIT0    0x1
#define BIT1    0x2
#define BIT2    0x4
#define BIT3    0x8
#define BIT4    0x10
#define BIT5    0x20
#define BIT6    0x40
#define BIT7    0x80
#define BIT8    0x100
#define BIT9    0x200
#define BIT10   0x400
#define BIT11   0x800
#define BIT12   0x1000
#define BIT13   0x2000
#define BIT14   0x4000
#define BIT15   0x8000
#define BIT16   0x10000
#define BIT17   0x20000
#define BIT18   0x40000
#define BIT19   0x80000
#define BIT20   0x100000
#define BIT21   0x200000
#define BIT22   0x400000
#define BIT23   0x800000
#define BIT24   0x1000000
#define BIT25   0x2000000
#define BIT26   0x4000000
#define BIT27   0x8000000
#define BIT28   0x10000000
#define BIT29   0x20000000
#define BIT30   0x40000000
#define BIT31   0x80000000

// Version/Revsion field
#define Rsvd              BIT7|BIT6
#define UDID_Ver          BIT5|BIT4|BIT3
#define Silicon_RevsionID BIT2|BIT1|BIT0

// Interface field
#define IPMI_Support      BIT6
#define ASF_Support       BIT5
#define OEM_Support       BIT4
// smbus host address C2h
#define ARP_ADDRESS     0xC2
#define SMBHost_ADDRESS 0x10

/* ARP Commands */
#define ARP_PREPARE      0x01
#define ARP_RESET_DEV    0x02
#define ARP_GET_UDID_GEN 0x03
#define ARP_ASSIGN_ADDR  0x04
#define SMBIOS_Command   0x0a

/* ASF SMBus Cmd */
#define SnsrSystemState   0x01
#define ManagementControl 0x02
#define ASFConfiguration  0x03
#define Message           0x04
/* ASF SMBus Sub-Cmd */
#define GetEventData      0x11
#define GetEventStatus    0x12
#define DeviceTypePoll    0x13
#define SetSystemState    0x18
#define StartWatchDog     0x13
#define StopWatchDog      0x14
#define ClearBootOptions  0x15
#define ReturnBootOptions 0x16
#define NoBootOptions     0x17
#define PushAlertMsgWR    0x15
#define PushAlertMsgWoR   0x16
#define SetAlertConfig    0x11
#define GetAlertConfig    0x12

#define RCA_PWR_RST       0 
#define RCA_PWR_ON        1 
#define RCA_PWR_OFF       2 
#define RCA_PWR_PCR       3 
#define RCA_CMD           4 
#define RCA_ADDR          5 
#define RCA_LS_ADDR0      6
#define RCA_LS_CMD0       7
#define RCA_LS_ADDR1      8
#define RCA_LS_CMD1       9

// AMI BIOS Team Cowork - SMBus Receive Byte Protocol
#define Receive_Byte_Cmd 0x0b
#define ReturnValue_Yes  0x59
#define ReturnValue_No   0x4e

/* UDID Fields */
#define ARP_CAPAB       0
#define ARP_VER         1
#define ARP_VEND        2
#define ARP_DEV         4
#define ARP_INT         6
#define ARP_SUBVEND     8
#define ARP_SUBDEV      10
#define ARP_SPECID      12

#define UDID_LENGTH     0x11

#define SMBUS_ADDRESS_SIZE      0x80
#define ARP_FREE        0
#define ARP_RESERVED    1
#define ARP_BUSY        2
//#define GetUDID_BC      0x11


#define ARP_MAX_DEVICES 8
/* smbus well-known address */
#if 0
static INT8U smbaddrrsvd[] =
    /* As defined by SMBus Spec. Appendix C */
    {0x10, 0x12, 0x14, 0x16, 0x18, 0x50, 0x58, 0x5a,
     0x6e, ARP_ADDRESS,
     /* As defined by SMBus Spec. Sect. 5.2 */
     0x02, 0x04, 0x06, 0x08, 0x0a,
     0x0c, 0x0e, 0xf0, 0xf2, 0xf4, 0xf6,
     0xf8, 0xfa, 0xfc, 0xfe,
     /* Common PC addresses (bad idea) */
     0x5a, 0x90, 0x92, /* sensors */
     0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae, /* eeproms */
     0xD2, /* clock chips */
     /* Must end in 0 which is also reserved */
     0x00
    };

#endif
//static INT8U RTUDID_Value[16] = {0};
// get udid
struct arp_rtdevice
{
    INT32S status;
    INT8U udid[UDID_LENGTH];
    INT8U dev_cap;
    INT8U dev_ver;
    INT16S dev_vid;
    INT16U dev_did;
    //INT32Serface
    INT16S dev_INT32S;
    INT16S dev_svid;
    INT16S dev_sdid;
    //vendor-specific ID
    INT32U dev_vsid;
    INT8U saddr;
};


/* Client has this additional data */
struct arp_data
{
    INT32S sysctl_id;
    //need semaphore algorithm~~~??
    //  struct semaphore update_lock;

    char valid;             /* !=0 if following fields are valid */
    unsigned long last_updated;     /* In jiffies */

    INT8U address_pool[SMBUS_ADDRESS_SIZE];
    struct arp_rtdevice dev[ARP_MAX_DEVICES];
};
/* SMBus Protocol Structure*/
struct SMBus_ARP
{
    struct arp_rtdevice *RTUDIDptr;
    struct arp_data *RTARPdatptr;

    INT8S arpcmd_prepare;
    INT8S arpcmd_reset_dev;
    INT8S arpcmd_getudid;
    INT8S arpcmd_assign_addr;
// Reserved for future use!!
    INT8S arpcmd_rsvd;

    INT32S	numSMBDesc;

    struct	SMBDesc	*SMBDescArray;
    INT32U	SMBDescBufAddr;
};

// ASF SMBus Message 1. Command value
struct ASF_SMBusMsg
{
    // (1)sensor device and system state
    INT8S Sensor_Sytemstate;
    // (2)management control
    INT8S Mag_Control;
    // (3)ASF configuration
    INT8S ASF_Config;
    // (4)ASF Messaging
    INT8S ASF_Msging;
    // **(5)Reserved for future: 05h~0fh
    INT8S ASF_Rsvd;
    // sub-cmd structure
    struct SubCmd *SubCmdptr;

};
// ASF SMBus Message 2. Sub-Command Value
struct SubCmd
{
    // <1> sensor device and system state
    INT8S GetEvent_Data;                   //nic is master.
    INT8S GetEvent_Status;                 //nic is master.
    INT8S Device_TypePoll;                 //nic is slave.
    INT8S Set_SystemState;                 //nic is slave.
    // <2> watchdog timer
    INT8S StartWdg_Timer;                  //nic is slave.
    INT8S StopWdg_Timer;                   //nic is slave.
    // <3> get boot options
    INT8S Clear_BootOptions;               //nic is slave.
    INT8S Return_BootOptions;              //nic is slave.
    INT8S No_BootOptions;                  //nic is slave.
    // <4> messaging
    INT8S PushMsg_withRetransmission;      //nic is slave.
    INT8S PushMsg_withoutRetransmission;   //nic is slave.
    INT8S SetAlert_Config;                 //nic is master.
    INT8S GetAlert_Config;                 //nic is master.

};

struct SMBDesc
{
    INT8U slave_address;
    INT8U command_code;
    INT8U bytecount;
    INT8U sub_command_code;

};
void smbus_send(INT8U *buf, INT8U byte) _ATTRIBUTE_IROM;
eventdata *event_log(INT8U *addr, INT8U byte) _ATTRIBUTE_IROM;
void SMBus_Prepare_Ack(INT8U cmdcode);
INT8U *SMBus_Prepare_BootOptions() _ATTRIBUTE_IROM;
eventdata *eventGet() _ATTRIBUTE_IROM;
int getEventLogParser(eventdata *ed, unsigned char* buf);
INT8U SMBus_Prepare_GetEventStatus();//INT8S* Buffer_address, INT32S length);
INT8U SMBus_Prepare_GetEventData(INT8U index);
void SMBus_DTP_Message();
INT8U SMBus_Prepare_RmtCtrl(INT8U MsgType, INT8U force);
INT8U SMBus_Prepare_LSPMsg();
INT8U Fan_Poll(INT8U fanindex, INT8U index);
void setlogtype(eventdata *eptr);
INT8U SNR_detect();
INT8U *SMBus_Get_UDID();
void set_pldm_snr_slave_address(INT8U addr);
void SetSnrState(INT8U evtoffset);
void Get_FSC_Sensor(INT8U snrindex, INT8U polledval);
void bsp_smbus_handler(void);
void bsp_smbus_isr(void);
void bsp_smbus_init(INT8U reset); 
INT8U master_send(msendtype type, INT8U* bufaddr, INT8U length, int cmd);
void handle_pldm_br_ami(INT8U *addr) ;
void handle_pldm_br_phoenix(INT8U *addr) ;
void handle_pldm_bw(INT8U *addr);
void smbus_hook();
void smbus_asfpoll(); 
void smbus_arp(INT8U *addr);
void smbus_process_polling(INT8U *ptr);
void smbus_process_watchdog(INT8U *ptr);
void smbus_process_alerts(INT8U *ptr);
void handle_pldm_tags(INT8U *addr);
void pldm_handle_setgettime(INT8U *addr);
void smbus_process_pldm_bw(INT8U *addr);
void smbus_process_pldm_br_ami(INT8U *addr);
void smbus_process_pldm_br_phoenix(INT8U *addr);
void smbus_engine_reset(INT8U smbus_port);
void slaveMode_PLDM_ASFdataFill(INT8U smbus_port);
#endif
