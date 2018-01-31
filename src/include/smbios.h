#ifndef __SMBIOS_H__
#define __SMBIOS_H__

#include <sys/dpdef.h>

#define WEBPAGESIZETHD 1200

/*
#ifdef HDR_INCLUDED
#include
#else
extern
#endif

#define HDR_INCLUDED
*/


typedef struct _SMBIOS_Table_EP
{
	unsigned char anchorStr[4];
	char chkSum;
	char EPLen;
	char majorVer;
	char minorVer;
	unsigned short MaxStSize;
	char EPReVer;
	char formattedArea[5];
	char ianchorStr[5];
	char ichkSum;
	unsigned short StTableLen;
	unsigned int StTableAddress;
	unsigned short StNum;
	unsigned int SMBIOSNum;
}SMBIOS_Table_EP;

typedef struct {
    unsigned int l;
    unsigned int h;
} SBU64;

typedef struct _SMBIOSHeader
{
    unsigned char type;
    unsigned char len;
    unsigned short handle;
    unsigned char* data;
}SMBIOSHeader;

enum SMBIOSDataType
{
    SBit = 0x0,
    SByte,
    SWORD,
    SDWORD,
    SQWORD,
    SSTR
};

enum SMBIOSType
{
    BIOS_INFO = 0x0,
    SYSTEM_INFO,
    BASE_BOARD_INFO,
    CHASSIS,
    PROCESSOR,
    MEM_CTL, /*Type 5*/
    MEM_MODULE,
    CACHE,
    PORT_CONNECTOR,
    SLOTS,
    ON_BOARD_DEV = 0x0A, /*Type 10*/
    OEM_STR,
    SYS_CFG_OPT,
    BIOS_LANG,
    GROUP_ASSOC,
    SYS_EVENT_LOG,
    PHY_MEM_ARRAY = 0x10,/*Type 16*/
    MEM_DEV,
    MEM_ERROR_INFO,
    MEM_ARRAY_MAP_ADDR,
    MEM_DEV_MAP_ADDR,/*Type 20*/
    BPT_DEV,
    BATTERY,
    SYS_RESET = 0x17,
    HW_SECURITY = 0x18,/*Type 24*/
    SYS_PWR_CTL,
    VOLTAGE_PROBE,
    COOL_DEV,
    TEMP_PROBE = 0x1C,/*Type 28*/
    EC_PROBE,
    OOBRA,
    SYS_BOOT_STATUS = 0x20,/*Type 32*/
    SMBIOS_Inactive = 0x7E/*Type 126*/
};

enum BIOSElement
{
    BIOS_VENDOR = 0x04,
    BIOS_VER = 0x05,
    BIOS_START_ADDR = 0x6,
    BIOS_RELEASE_DATE = 0x08,
    BIOS_ROM_SIZE = 0x09,
    BIOS_CHARS = 0x0A,
    BIOS_CHARS_EXT_BYTE = 0x12,
    BIOS_MAJOR = 0x14,
    BIOS_MINOR = 0x15,
    BIOS_FIRMWARE_MAJOR = 0x16,
    BIOS__FIRMWARE_MINOR = 0x17
};

enum SYSINFOElement
{
    SYSINFO_MAN = 0x4,
    SYSINFO_PN = 0x5,
    SYSINFO_VER = 0x6,
    SYSINFO_SN = 0x7,
    SYSINFO_UUID = 0x8,
    SYSINFO_WAKEUP_TYPE = 0x18,
    SYSINFO_SKU = 0x19,
    SYSINFO_FAMILY = 0x1A,
};

enum BASEBOARDINFOElement
{
    BBINFO_MAN = 0x4,
    BBINFO_PN = 0x5,
    BBINFO_VER = 0x6,
    BBINFO_SN = 0x7,
    BBINFO_ASSET_TAG = 0x8,
    BBINFO_FEATURE_TAG = 0x9,
    BBINFO_LOCATION_IN_CHASSIS = 0x0A,
    BBINFO_CHASSIS_HANDLE = 0x0B,
    BBINFO_BOARD_TYPE = 0x0D,
    BBINFO_OBJ_NUM = 0x0E,
    BBINFO_OBJ_HANDLE = 0x0F
};

enum ChassisElement
{
    CHASSIS_MAN = 0x4,
    CHASSIS_TYPE = 0x5,
    CHASSIS_VER = 0x6,
    CHASSIS_SN = 0x7,
    CHASSIS_ASSET_TAG = 0x8,
    CHASSIS_BOOTUP = 0x9,
    CHASSIS_POWER = 0x0A,
    CHASSIS_THERMAL = 0x0B,
    CHASSIS_SECURITY = 0x0C,
    CHASSIS_OEM = 0x0D,
    CHASSIS_HEIGHT = 0x011,
    CHASSIS_POWER_NUM = 0x12,
    CHASSIS_ELEMENT_COUNT = 0x13,
    CHASSIS_ELEMENT_LEN = 0x14,
    CHASSIS_ELEMENT = 0x15
};

enum ProcessorElement
{
    PROCESSOR_SKT_DESIGN = 0x4,
    PROCESSOR_TYPE = 0x5,
    PROCESSOR_FAMILY = 0x6,
    PROCESSOR_MAN = 0x7,
    PROCESSOR_ID = 0x8,
    PROCESSOR_VER = 0x10,
    PROCESSOR_VOLTAGE = 0x11,
    PROCESSOR_ECLK = 0x12,
    PROCESSOR_MAX_SPEED = 0x14,
    PROCESSOR_CURRENT_SPEED = 0x16,
    PROCESSOR_STATUS = 0x018,
    PROCESSOR_UPGRADE = 0x19,
    PROCESSOR_L1_CACHE = 0x1A,
    PROCESSOR_L2_CACHE = 0x1C,
    PROCESSOR_L3_CACHE = 0x1E,
    PROCESSOR_SN = 0x020,
    PROCESSOR_ASSET_TAG = 0x21,
    PROCESSOR_PN = 0x22,
    PROCESSOR_CORE_COUNT = 0x23,
    PROCESSOR_CORE_ENABLED = 0x24,
    PROCESSOR_THREAD_COUNT = 0x25,
    PROCESSOR_CHARS = 0x26,
    PROCESSOR_FAMILY2 = 0x28
};

enum MemoryCtlElement
{
    MCTL_ERR_DETECT_METHOD = 0x4,
    MCTL_ERR_CORRECT_CAP = 0x5,
    MCTL_INTERLEAVE = 0x6,
    MCTL_CUR_INTERLEAVE = 0x7,
    MCTL_MAX_SIZE = 0x8,
    MCTL_SPEED = 0x9,
    MCTL_TYPE = 0x0B,
    MCTL_VOLTAGE = 0x0D,
    MCTL_SLOTS = 0x0E,
    MCTL_HANDLE = 0x0F,
    MCTL_ERR_CAP_ENABLED = 0x10
};

enum MemoryModuleElement
{
    MM_SKT_DSEIGN = 0x4,
    MM_BANK = 0x5,
    MM_SPEED = 0x6,
    MM_TYPE = 0x07,
    MM_INSTALLED_SIZE = 0x9,
    MM_ENABLED_SIZE = 0xA,
    MM_ERR_STATUS = 0xB
};

enum CacheElement
{
    CACHE_SKT_DSEIGN = 0x04,
    CACHE_CFG = 0x05,
    CACHE_MAX_SIZE = 0x07,
    CACHE_INSTALL_SIZE = 0x09,
    CACHE_SUPPORT_SRAM_TYPE = 0x0B,
    CACHE_CURRENT_SRAM_TYPE = 0x0D,
    CACHE_SPEED = 0x0F,
    CACHE_ERR_CORRECT_TYPE = 0x10,
    CACHE_SYS_TYPE = 0x11,
    CACHE_SUPPORT_ASSOC = 0x12
};

enum PortConectorElement
{
    PCON_INTER_REF_DESIGN = 0x04,
    PCON_INTER_TYPE = 0x05,
    PCON_EXTER_REF_DESIGN = 0x06,
    PCON_EXTER_TYPE = 0x07,
    PCON_PORT_TYPE = 0x08
};

enum SlotElement
{
    SLOT_DESIGN =  0x04,
    SLOT_TYPE = 0x05,
    SLOT_BUS_WIDTH = 0x06,
    SLOT_USAGE = 0x07,
    SLOT_LEN = 0x08,
    SLOT_ID = 0x9,
    SLOT_CHAR1 = 0x0B,
    SLOT_CHAR2 = 0x0C,
    SLOT_SEG_GN = 0x0D,
    SLOT_BUS_NUM = 0x0F,
    SLOT_DEV_NUM = 0x10
};


enum OEMStringElement
{
    OEMSTR_COUNT =  0x04,
};

enum SysCfgOptElement
{
    SYSCFGOPT_COUNT =  0x04,
};

enum BIOSLanguageElement
{
    BIOSLANG_INSTALL_LANG =  0x04,
    BIOSLANG_FLAGS =  0x05,
    BIOSLANG_CURRENT_LANG =  0x15
};

enum GroupAssociationElement
{
    GA_NAME =  0x04,
    GA_ITEM_TYPE =  0x05,
    GA_ITEM_HANDLE =  0x6
};

enum SYSEventLogElement
{
    SYSEVTLOG_LEN =  0x04,
    SYSEVTLOG_HEADER_OFFSET =  0x06,
    SYSEVTLOG_DATA_OFFSET =  0x08,
    SYSEVTLOG_ACCESS_METHOD =  0x0A,
    SYSEVTLOG_STATUS =  0x0B,
    SYSEVTLOG_TOKEN =  0x0C,
    SYSEVTLOG_METHOD_ADDRESS =  0x10,
    SYSEVTLOG_HEADER_FORMAT =  0x14,
    SYSEVTLOG_DESCRIPTOR_NUM =  0x15,
    SYSEVTLOG_DESCRIPTOR_TYPE =  0x16,
    SYSEVTLOG_DESCRIPTOR_LIST =  0x17
};

enum PhyMemArrayElement
{
    PHYMEMARY_LOCATION =  0x04,
    PHYMEMARY_USE =  0x05,
    PHYMEMARY_ERR_CORRECT =  0x06,
    PHYMEMARY_MAX_CAP =  0x07,
    PHYMEMARY_ERR_INFO_HANDLE =  0x0B,
    PHYMEMARY_DEV_NUM =  0x0D
};

enum MemDevElement
{
    MEMDEV_LOCATION =  0x04,
    MEMDEV_ERR_INFO_HANDLE =  0x06,
    MEMDEV_WIDTH =  0x08,
    MEMDEV_DATA_WIDTH =  0x0A,
    MEMDEV_SIZE =  0xC,
    MEMDEV_FACTOR =  0x0E,
    MEMDEV_SET =  0x0F,
    MEMDEV_DEV_LOCATOR =  0x10,
    MEMDEV_BANK_LOCATOR =  0x11,
    MEMDEV_TYPE =  0x12,
    MEMDEV_DEV_DETAIL =  0x13,
    MEMDEV_SPEED =  0x015,
    MEMDEV_MAN =  0x17,
    MEMDEV_SN =  0x18,
    MEMDEV_TAG =  0x19,
    MEMDEV_PN =  0x1A,
    MEMDEV_ATTRIBUTE =  0x1B
};

enum MemErrorInfoElement
{
    MEMEINFO_TYPE =  0x04,
    MEMEINFO_GRANULARITY =  0x05,
    MEMEINFO_OP =  0x06,
    MEMEINFO_VENDOR_SYNDROME =  0x07,
    MEMEINFO_ARR_ERROR_ADDRESS =  0x0B,
    MEMEINFO_DEV_ERROR_ADDRESS =  0x0F,
    MEMEINFO_ERROR_RES =  0x13
};

enum MemMapAryAddrElement
{
    MEMARYMAP_START_ADR =  0x04,
    MEMARYMAP_END_ADR =  0x08,
    MEMARYMAP_HANDLE =  0x0C,
    MEMARYMAP_PWIDTH =  0x0E
};

enum MemMapDevAddrElement
{
    MEMDEVMAP_START_ADR =  0x04,
    MEMDEVMAP_END_ADR =  0x08,
    MEMDEVMAP_HANDLE =  0x0C,
    MEMDEVMAP_ARY_HANDLE =  0x0E,
    MEMDEVMAP_ROW_POS =  0x10,
    MEMDEVMAP_INTERLEAVE_POS =  0x11,
    MEMDEVMAP_INTERLEAVE_DEPTH =  0x12
};

enum BuiltInPointDevElement
{
    BPTDEV_TYPE =  0x04,
    BPTDEV_IF =  0x05,
    BPTDEV_BTN_NUM =  0x6
};

enum BatteryElement
{
    BATTERY_LOC =  0x04,
    BATTERY_MAN =  0x05,
    BATTERY_DATE =  0x06,
    BATTERY_SN =  0x07,
    BATTERY_DN =  0x08,
    BATTERY_DEV_CHEMISTRY =  0x09,
    BATTERY_DESIGN_CAP =  0x0A,
    BATTERY_DEGIGN_VOLTAGE =  0x0C,
    BATTERY_SBDS_VER =  0x0E,
    BATTERY_MAX_ERR =  0x0F,
    BATTERY_SBDS_SN =  0x10,
    BATTERY_SBDS_DATE =  0x12,
    BATTERY_SBDS_DEV_CHEMISTRY =  0x14,
    BATTERY_CAP_MUL =  0x15,
    BATTERY_OEM_SPEC = 0x16
};

enum HWSecurityElement
{
    HWSEC_SETTING =  0x04,
};

enum SysPowerControlElement
{
    SYSPWRCTL_MONTH =  0x04,
    SYSPWRCTL_DAY =  0x05,
    SYSPWRCTL_HOUR =  0x06,
    SYSPWRCTL_MIN =  0x07,
    SYSPWRCTL_SEC =  0x08
};

enum VoltageProbeElement
{
    VOLPB_DESCRIPTION =  0x04,
    VOLPB_LOC_STATUS =  0x05,
    VOLPB_MAX =  0x06,
    VOLPB_MIN =  0x08,
    VOLPB_RES =  0x0A,
    VOLPB_TOLERANCE =  0x0C,
    VOLPB_ACCURACY =  0x0E,
    VOLPB_OEM_DEFINED =  0x10,
    VOLPB_NOMINAL =  0x14
};

enum CoolingDeviceElement
{
    COOLDEV_TEMP_PBOBE =  0x04,
    COOLDEV_TYPE_STATUS =  0x06,
    COOLDEV_UINT =  0x07,
    COOLDEV_OEM_DEF =  0x08,
    COOLDEV_NOMINAL_SPEED =  0x0C
};

enum TempatureProbeElement
{
    TMPPB_DESCRIPTION =  0x04,
    TMPPB_LOC_STATUS =  0x05,
    TMPPB_MAX =  0x06,
    TMPPB_MIN =  0x08,
    TMPPB_RES =  0x0A,
    TMPPB_TOLERANCE =  0x0C,
    TMPPB_ACCURACY =  0x0E,
    TMPPB_OEM_DEFINED =  0x10,
    TMPPB_NOMINAL =  0x14
};

enum ElectricalCurrentProbeElement
{
    ECPB_DESCRIPTION =  0x04,
    ECPB_LOC_STATUS =  0x05,
    ECPB_MAX =  0x06,
    ECPB_MIN =  0x08,
    ECPB_RES =  0x0A,
    ECPB_TOLERANCE =  0x0C,
    ECPB_ACCURACY =  0x0E,
    ECPB_OEM_DEFINED =  0x10,
    ECPB_NOMINAL =  0x14
};

enum OutOfBandRemoteAccessElement
{
    OOBRA_MAN = 0x4,
    OOBRA_CONN = 0x5,
};

enum SysBootStatusElement
{
    SYSBOOTSTATUS_STATUS =  0x0A,
};

enum SysResetElement
{
    SYSRES_CAP =  0x04,
    SYSRES_CONUT =  0x05,
    SYSRES_LIMIT =  0x07,
    SYSRES_INTERVAL =  0x09,
    SYSRES_TIMEOUT =  0x0B
};

/*-----------------------------Functions------------------------------------*/

void toSMBIOSHeader(SMBIOSHeader *h, unsigned char *data) ;
unsigned char* getSMBIOSTypeAddr(enum SMBIOSType type);
int getSMBIOSElement(unsigned char *p,SMBIOSHeader *sh, enum SMBIOSType type, int e,unsigned char* out);
char getSMBIOSTypeNext(unsigned char* p,enum SMBIOSType type, unsigned char** next);

int getBIOSChar(SBU64 code, SMBIOSHeader *sh, unsigned char* out);
int getBIOSCharE1(SMBIOSHeader *sh,int e,unsigned char code,unsigned char* out);
int getBIOSCharE2(SMBIOSHeader *sh,int e,unsigned char code,unsigned char* out);

int getOnBoardDevType(unsigned char code, unsigned char* out);
int getBIOSElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out);
void getBIOSElemnetDWord(unsigned char *p,int e,unsigned char* out) ;
void getBIOSElemnetQWord(unsigned char *p, int e,unsigned char* out) ;
void getBIOSElemnetString(unsigned char *p,int e,unsigned char* out) ;
void getBIOSElemnetByte(unsigned char *p,int e,unsigned char* out) ;
void getBIOSElemnetWord(unsigned char *p,int e,unsigned char* out) ;
int getSysUID(unsigned char *p, unsigned char* out) ;
int grtProcessorID(unsigned char *p,SMBIOSHeader *sh, int e, unsigned char* out) ;
int getProcessorCache(unsigned short code, unsigned char* out); 
int getMemCtlCfgHandle(unsigned char* p, unsigned char slots, unsigned char code, unsigned char* out) ;
int getMemoryModuleBank(unsigned char code, unsigned char* out);
int getSysEventLogDescriptor(unsigned char count, unsigned char len,
                             unsigned char*p, unsigned char* out);
int ParserSensor() ;
#endif
