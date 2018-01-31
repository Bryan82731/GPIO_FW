#ifndef __SNMP_H__
#define __SNMP_H__

#include <rlx/rlx_cpu.h>
#include <sys/inet_types.h>
#include "lib.h"
#include "ip.h"
#include "udp.h"
#include "arp.h"
#include "smbus.h"

typedef struct
{
    INT8U  header;
    INT8U  length;
    INT8U  version[3];
    INT8U  community_header;
    INT8U  string_length;
    INT8U  community[6];
    INT8U  PDU[2];
    INT8U  EOT;
    INT8U  EOL;
    INT8U  SubID[9];
    INT8U  agent_addr_type;
    INT8U  agent_addr_length;
    //INT8U  agent_address[4];
    INT32U agent_address;
    INT8U  trap_type_type;
    INT8U  trap_type_length;
    INT8U  trap_type;
    INT8U  strap_type_type;
    INT8U  strap_length;
    INT8U  stf_reserved;
    INT8U  stf_event_sensor;
    INT8U  stf_event;
    INT8U  stf_event_offset;

    //specific_trap_field
    INT8U  time_stamp_type;
    INT8U  time_stamp_length;
    INT8U  time_stamp[4];
    INT8U  variable_binding_type;
    INT8U  variable_binding_length;
    INT8U  variable_binding_type2;
    INT8U  variable_binding_length2;
    INT8U  object_type;
    INT8U  object_length;
    INT8U  SID2[10];
    INT8U  value_type;
    INT8U  value_length;
    INT8U  guid[16];
    INT8U  sequence_number[2];
    INT8U  local_timestamp[4];
    INT8U  UTC_offset[2];
    INT8U  trap_source;
    INT8U  event_source;
    INT8U  event_severity;
    INT8U  sensor_device;
    INT8U  sensor_number;
    INT8U  Entity;
    INT8U  Entity_Instance;
    INT8U  Event_Data[8];
    INT8U  Language_code;
    INT8U  Manufacturer_ID[4];
    INT8U  System_ID[2];
    INT8U  OEM_Custom;
}SNMP;

typedef struct
{
    EthHdr ethHdr;
    IPHdr  ipHdr;
    UDPHdr udpHdr;
    SNMP   SNMPData;
}SNMPHdr;


#define Realtek_IANA            0x00006a92

#define SNMPPort                    0x00A2
#define SNMP_hdr                      0x30
#define SNMP_Version              0x020100
#define PDU_Type                    0xa461
#define Enterprise_Object_Type        0x06
#define Enterprise_Object_Length      0x09
#define Agent_Address_Type            0x40
#define Agent_Address_Length          0x04
#define Trap_Type_Type                0x02
#define Trap_Type_Length              0x01
#define Trap_Type                     0x06
#define Specific_Trap_Type_Type       0x02
#define Specific_Trap_Type_Length     0x04
#define Time_Stamp_Type               0x43
#define Time_Stamp_Length             0x04
#define Variable_Binding_Type         0x30
#define Object_Type                   0x06
#define Object_Length                 0x0a
#define Value_Type                    0x04

void SNMPpktGen(SNMP *buf, eventdata *pevent);
int snmpSend(PRTSkt s, eventdata *pevent);
#ifdef CONFIG_SOFTWARE_KVM_ENABLED
int snmpSend_kvm(PRTSkt s, eventdata *pevent);
#endif
void SNMPTask(void *p_arg);
#endif
