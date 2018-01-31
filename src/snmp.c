#include <sys/ucos_ii.h>
#include <string.h>
#include <stdlib.h>
#include "arp.h"
#include "snmp.h"
#include "smbus.h"
#include "bsp.h"
#include "tcp.h"
#include "ip.h"
#include "lib.h"
#include "rtskt.h"


extern OS_EVENT *SNMPQ;
extern asf_config_t *asfconfig;

static const INT8U SNMPVersion[] = {0x02, 0x01, 0x00};

static const INT8U Language_code[]={ 0x19 };

static const INT8U Sub_identifier[]={
    0x2b,0x06,0x01,0x04,0x01,
    0x98,0x6f,0x01,0x01,0x01
};
static const INT8U community[]={
    0x70,0x75,0x62,0x6c,0x69,
    0x63,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00
};

static const INT8U Manufacturer_ID[]={
    0x00,0x00,0x6a,0x92
};
#if 0
static const INT8U System_ID[]={
    0x00,0x00
};
#endif
static const INT8U OEM_Custom[]={
    0xc1
};

static INT16U sequence_num = 0;
extern eventdata *event_ptr;
extern DPCONF *dpconf;

UDPAddrInfo addrInfo;
//alert and heartbeat share the same sequence patterns

//Prepare SNMP packet content
void SNMPPktGen(SNMP *SNMPPkt, eventdata *pevent)
{

    INT16U bseqnum;
    INT32U timestamp = htonl(OSTimeGet());
    //INT8U  GUID[16] = { 0 };

    SNMPPkt->header = SNMP_hdr;
    SNMPPkt->length = sizeof(SNMP) - 2;
    memcpy(SNMPPkt->version,SNMPVersion,3);
    SNMPPkt->community_header = 0x04;
    SNMPPkt->string_length = 0x06;
    memcpy(SNMPPkt->community,community,6);
    //sprintf(SNMPPkt->PDU,"%c%c",(PDU_Type>>8),PDU_Type);
    SNMPPkt->PDU[0] = ((PDU_Type >> 8) & 0xFF);
    SNMPPkt->PDU[1] = (PDU_Type & 0xFF);
    SNMPPkt->EOT = Enterprise_Object_Type;
    SNMPPkt->EOL = Enterprise_Object_Length;


    //heartbeat
    memcpy(SNMPPkt->SubID,Sub_identifier ,9);
    SNMPPkt->agent_addr_type = Agent_Address_Type;
    SNMPPkt->agent_addr_length = Agent_Address_Length;
    SNMPPkt->agent_address = getIPAddress_F(eth0);
    SNMPPkt->trap_type_type = Trap_Type_Type;
    SNMPPkt->trap_type_length = Trap_Type_Length;
    SNMPPkt->trap_type = Trap_Type;
    SNMPPkt->strap_type_type = Specific_Trap_Type_Type;
    SNMPPkt->strap_length = Specific_Trap_Type_Length;
    SNMPPkt->stf_reserved = 0x00;
    if (pevent != NULL)
    {
        SNMPPkt->stf_event_sensor = pevent->Event_Sensor_Type;
        SNMPPkt->stf_event = pevent->Event_Type;
        SNMPPkt->stf_event_offset = pevent->Event_Offset;
    }
    else
    {
        SNMPPkt->stf_event_sensor = 0x25; //heartbeat
        SNMPPkt->stf_event = 0x6f;
        SNMPPkt->stf_event_offset = 0x00;
    }
    SNMPPkt->time_stamp_type = Time_Stamp_Type;
    SNMPPkt->time_stamp_length = Time_Stamp_Length;
    memcpy(SNMPPkt->time_stamp,&timestamp,sizeof(timestamp));
    SNMPPkt->variable_binding_type = Variable_Binding_Type;
    SNMPPkt->variable_binding_length = 0x3f;
    SNMPPkt->variable_binding_type2 = Variable_Binding_Type;
    SNMPPkt->variable_binding_length2 = Variable_Binding_Type;
    SNMPPkt->variable_binding_length2 = 0x3d;
    SNMPPkt->object_type = Object_Type ;
    SNMPPkt->object_length = Object_Length;
    memcpy(SNMPPkt->SID2,Sub_identifier ,10);
    SNMPPkt->value_type = Value_Type;
    SNMPPkt->value_length = 0x2f;

    //getGUID(GUID);
    //memcpy(SNMPPkt->guid, GUID,16);
    getGUID(SNMPPkt->guid);
    
    sequence_num++;
    if(pevent)
    {
    	if(pevent->seqnum == 0)
        	pevent->seqnum = sequence_num;
    }

    bseqnum = htons(sequence_num);
    memcpy(SNMPPkt->sequence_number,&bseqnum,sizeof(sequence_num));
    memset(SNMPPkt->local_timestamp, 0 , 4);
    SNMPPkt->UTC_offset[0] = 0xff;
    SNMPPkt->UTC_offset[1] = 0xff;
    if (pevent != NULL)
    {
        memcpy(&SNMPPkt->event_source, &pevent->Event_Source_Type,12);
        memset(&SNMPPkt->Event_Data[6], 0 ,2);
    }
    else
    {
        SNMPPkt->event_source  = 0x68;
        SNMPPkt->event_severity= 0x01;
        SNMPPkt->sensor_device = dpconf->arpaddr;
        SNMPPkt->sensor_number = 0x01;
        SNMPPkt->Entity        = 0x17;
        SNMPPkt->Entity_Instance= 0x00;
        memset(SNMPPkt->Event_Data,0,8);
    }
    SNMPPkt->trap_source=0x50;
    SNMPPkt->Language_code = Language_code[0];
    memcpy(SNMPPkt->Manufacturer_ID,Manufacturer_ID,4);
    memcpy(SNMPPkt->System_ID,&asfconfig->systemid,2);
    SNMPPkt->OEM_Custom = OEM_Custom[0];

}
int snmpSend(RTSkt *s, eventdata *pevent)
{
    SNMP *outPkt;
    PKT* pkt;

    pkt = (PKT *) allocUDPPkt_F(sizeof(SNMP), IPv4);

    //memory allocation fail, or event sensor type == 0
    //memory full, delay the time
    if (pkt == NULL)
        return SKTHOSTNOTFOUND;

    outPkt = (SNMP *) pkt->wp;

    //if (!addrInfo.destIPAddr.addr)
    {
        memcpy(&addrInfo.destIPAddr.addr, &dpconf->ConsoleIP, IPv4_ADR_LEN);
        addrInfo.destPort = 162;
    }

    SNMPPktGen(outPkt, pevent);
    return rtSktUDPSendTo_F(s, pkt, &addrInfo);
}


#ifdef CONFIG_SOFTWARE_KVM_ENABLED
int snmpSend_kvm(RTSkt *s, eventdata *pevent)
{
    SNMP *outPkt;
    PKT* pkt;
	
    pkt = (PKT *) allocUDPPkt_F(sizeof(SNMP), IPv4);

    //memory allocation fail, or event sensor type == 0
    //memory full, delay the time
    if (pkt == NULL)
        return SKTHOSTNOTFOUND;

    outPkt = (SNMP *) pkt->wp;

    //if (!addrInfo.destIPAddr.addr)
    {
        memcpy(&addrInfo.destIPAddr.addr, &dpconf->ConsoleIP, IPv4_ADR_LEN);
	 //ht 20130613 : KVM need use 163 (default 162)
	 //addrInfo.destPort = 162;
	 addrInfo.destPort = 163;

	 printf("destIP = %x, consoleIP = %x\n", addrInfo.destIPAddr.addr, dpconf->ConsoleIP);
    }

    SNMPPktGen(outPkt, pevent);
    return rtSktUDPSendTo_F(s, pkt, &addrInfo);
}
#endif

void SNMPTask(void *p_arg)
{
    (void) p_arg;

    INT8U err;
    eventdata *pevent;
    RTSkt *s;
    INT8S state = 0;
    INT8U interval = dpconf->hbtime;

    //if dhcp is enabled, but IP is still 0
    //wait for IP address to be ready
    while (!dpconf->HostIP[eth0].addr)
        OSTimeDly(1);

    s = rtSkt_F(IPv4, IP_PROTO_UDP);
    rtSktShutdown_F(s, SKT_DISABLE_RX);
    rtSktUDPBind_F(s, 162);

#ifdef CONFIG_SOFTWARE_KVM_ENABLED
    RTSkt *s_kvm;
    s_kvm = rtSkt_F(IPv4, IP_PROTO_UDP);
    rtSktShutdown_F(s_kvm, SKT_DISABLE_RX);
    rtSktUDPBind_F(s_kvm, 163);
#endif

    while (OS_TRUE)
    {

        pevent = (eventdata *) OSQPend(SNMPQ,OS_TICKS_PER_SEC*interval,&err);
        if (dpconf->asfon && s && dpconf->HostIP[eth0].addr && dpconf->ConsoleIP[eth0].addr)
        {
#ifdef CONFIG_SOFTWARE_KVM_ENABLED
	 	        snmpSend_kvm(s_kvm,pevent);
#endif
	 	        state = snmpSend(s,pevent);
        }
        //at most would send the hearbeat every 3 minutes
        if (state == SKTHOSTNOTFOUND && interval < 180)
            interval += interval;
        else
            interval = dpconf->hbtime;
    }

}
