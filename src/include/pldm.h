#ifndef __PLDM_H__
#define __PLDM_H__
#include <sys/autoconf.h>
#include <rlx/rlx_types.h>

typedef struct _BiosAttr
{
   struct _BiosAttr *next;	
   INT16U  *attr_possible_handle;
   INT8U   *attr_data;
   INT8U   *attr_val_index;
   INT8U   *attr_default_index;
   INT8U   *attr_val_handle;
   INT16U  attr_handle;
   INT16U  attr_type;
   INT16U  attr_name_handle;
   INT16U  attr_possible_num;
   INT8U   attr_val_possible_num;
   INT8U   attr_default_num;
   INT16U  option; 
}BiosAttr;

typedef struct
{
    INT8U  *ptr[3];
    INT32U len[3];
    INT32U tag[3];
    INT8U  TBL1:1, TBL2:1, TBL3:1, pldmerror:1, RSVD: 4;
    INT8U  index;
    INT8U  valid;
    INT8U  dirty;
    INT8U  *xferptr[3];
    INT32U numwrite[4];
    INT32U xferlen[3];
    INT32U xfertag[3];
    INT32U chksum[3];
}pldm_t;

typedef struct 
{
    INT8U slaveaddr;
    INT8U cmd;
    INT8U length;
    INT8U sslaveaddr;
    INT8U hdrversion:1, rsvd: 5, mctpstop:1, mctpstart:1;
    INT8U dstept;
    INT8U srcept;
    INT8U msgtag:3, to:1, pktseq:2, msgstop:1, msgstart:1;
    INT8U msgtype:7, ic: 1;
    INT8U iid: 5, rsvd2: 1, datagram: 1, rd: 1;
    INT8U pldmtype:6, hdrver: 2;
    INT8U pldmcmd;
    INT8U pldmcode;
    INT8U val[57];
    INT8U val2[2];
}pldm_res_t;

typedef enum _pldm_base_code
{
PLDM_SUCCESS,
PLDM_ERROR,
PLDM_ERROR_INVALID_DATA,
PLDM_ERROR_INVALID_LENGTH,
PLDM_ERROR_NOT_READY,
PLDM_ERROR_UNSUPPORTED_PLDM_CMD,
PLDM_INVALID_PLDM_TYPE = 0x20,
}PLDM_BASE_CODE;

//PLDM DSP 0240
enum{
SetTID = 1,
GetTID,
GetPLDMVersion,
GetPLDMTypes,
GetPLDMCommands,
};

//PLDM DSP 0246
enum {
GetSMBIOSStructureTableMetadata = 1,
SetSMBIOSStructureTableMetadata,
GetSMBIOSStructureTable,
SetSMBIOSStructureTable,
GetSMBIOSStructureByType,
GetSMBIOSStructureByHandle,
};

enum {
NO_SMBIOS_STRUCTURE_TABLE_METADATA = 0x83,
};

//PLDM DSP 0247
enum {
GetBIOSTable = 1,
SetBIOSTable,
UpdateBIOSTable,
GetBIOSTableTags,
SetBIOSTableTags,
AcceptBIOSAttributesPendingValues,
SetBIOSAttributeCurrentValue,
GetBIOSAttributeCurrentValueByHandle,
GetBIOSAttributePendingValueByHandle,
GetBIOSAttributeCurrentValueByType,
GetBIOSAttributePendingValueByType,
GetDateTime,
SetDateTime,
GetBIOSStringTableStringType,
SetBIOSStringTableStringType,
BIOS_TABLE_TAG_UNAVAILABLE = 0x86,
};

enum{
PLDM_BIOS_TABLE_UNAVAILABLE = 0x83,
PLDM_BIOS_TABLE_TAG_UNAVAILABLE = 0x86,
PLDM_UNKNOWN = 0xFF,
};

enum {
    BIOSEnumeration = 0x00,
    BIOSString = 0x01,
    BIOSPassword = 0x02,
    BIOSBootConfigSetting = 0x04,
};

INT32U CreateAttrVal(INT8U *buf,INT8U *seq);
BiosAttr *InitialPLDM();
INT8U getPLDMElement(BiosAttr *addr,unsigned char ptype,unsigned char val,unsigned char *out);
#endif
