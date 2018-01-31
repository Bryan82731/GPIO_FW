#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include "rtskt.h"
#include "app_cfg.h"
#include "lib.h"
#include "wsman-names.h"
#include "dash.h"
#include "bsp.h"
#include "smbios.h"
#include "smbus.h"
#include "eventlog.h"
#include "http.h"
#include "pldm.h"
#include "telnet.h"
#include "usb.h"
#include "wcom.h"

#ifndef CONFIG_USB_ENABLED
unsigned char tip[4] = {192,168,0,5};
char fn[80] = "win98_r.bin";
USBCB usbcb;
#endif


#define EPR 1
#define ObjectAndEPR 2
#define Optimize 4

#define Account_pri		0x1
#define Role_pri                0x2

#define Text_pri      		0x4
#define USB_pri                 0x8
#define ComputerSystem_pri	0x10
#define PowerManagement_pri	0x20
#define Processor_pri		0x40
#define Boot_pri		0x80
#define Memory_pri		0x100
#define Sensor_pri		0x200
#define Fan_pri			0x400
#define PowerSupply_pri		0x800
#define PhysicalAsset_pri       0x1000
#define Indication_pri		0x2000
#define DHCP_pri 		0x4000
#define IP_pri			0x8000
#define Software_pri		0x10000
#define OperatingSystem_pri	0x20000
#define BIOS_pri                0x40000
#define Record_pri              0x80000
#define Opaque_pri              0x100000
#define R_Account_pri           0x200000
#define R_Role_pri              0x400000

#define WSM_CQL_FILTER_DIALECT    "http://schemas.dmtf.org/wbem/cql/1/dsp0202.pdf"
extern const INT8U snrtbl[][3];
extern const INT8U snrstatestr[][15];
extern flash_data_t dirty[ENDTBL];
extern flash_data_t dirty_ext[10];
//extern OS_EVENT *PLDMSNRSem;
extern OS_STK  TaskUSBStk[TASK_USB_STK_SIZE] _ATTRIBUTE_STK;

#ifdef CONFIG_TCR_ENABLED
OS_STK  TaskSOLStk[TASK_SOL_STK_SIZE] _ATTRIBUTE_STK;
#endif

#define ZeroTime    0x1
#define OneTime     0x2
#define XMLFragment 0x4
#define SQLQuery    0x8
#define NumFilterCollection 5
#define OpmMax     2048
#define OpaqueNum   4
void USBClientTask(void *data);
void USBSrvTask(void *data);
void SOLTask(void *data);

extern TCRCB tcrcb;
extern const INT8U biosname[2][10];
extern USBCB usbcb;
extern sensor_t sensor[MAX_SENSOR_NUMS];

extern const INT8U pwrtbl[16][2];
extern smbiosrmcpdata *smbiosrmcpdataptr;
extern pldm_t *pldmdataptr;
extern pldm_t pldmdata;
extern DPCONF *dpconf;
extern unsigned char tip[4];
extern char fn[80];
extern UserInfo *userhead;
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
SubEvent *subhead=NULL,*subtail=NULL;
#else
extern SubEvent *subhead,*subtail;
#endif
extern RoleInfo *rolehead,*roletail;
m_list *memhead=NULL,*memtail=NULL,*tmp_memhead=NULL,*tmp_memtail=NULL;
static const INT8U whitestr[] = {0x20,0x0a,0x0d,0x0,0x9};
extern TCROPT *bootsel;
extern TCROPT bootopts[14];

const INT8U hello[]     ="HTTP/1.1 200\r\nServer:DASH Service\r\nContent-Type: application/soap+xml;charset=UTF-8\r\nContent-Length:xxxxx\r\n\r\n";
const INT8U hello_err[] ="HTTP/1.1 400 Error\r\nContent-Type: application/soap+xml;charset=UTF-8\r\nServer: Openwsman\r\nContent-Length:xxxxx\r\n\r\n";
const INT8U hello_nego[]="HTTP/1.1 200 \r\nServer:DASH Service\r\nContent-Type: application/soap+xml;charset=UTF-8\r\nContent-Length:xxxxx\r\nWWW-Authenticate: Negotiate ";
char local_ip[16]="192.168.0.10",local_netmask[16]="255.255.255.0",local_gateway[16]="192.168.0.254";
char new_ip[16]="192.168.0.10",new_netmask[16]="255.255.255.0",new_gateway[16]="192.168.0.254";
char xuuid[32];
extern char *idata;
int sk;

int AllClass = 0;

char *to;

char access_info[128]="";

char ResourceURI[128];
char Role[128];
char ResultRole[128];
char p_selector[255]="";
char current_state[16];
char health_state[16];
char state[2]="3";
char power_state[2]="2";
char enabled_state[2] = "2";
char dhcp_state[2]="3";
char client_state[2]="0";
char request_state[2]="11";
char StartingBoot=0;
char PowerChangeCapabilities[2]="3";
char Method[255]="";
char SubClassProperties=0;
char SelectorEnabled=0;
//char FoundSelector=0;
char NextInstance[2]="1";
char SetDHCP=0,AssociatedCooling=-1;

int LogEnd = 0;
char change_sub=0;
int memuse = 0;
INT8U scode[255] = "s:Sender";
INT8U boot_seq[16];

plist *m_softId = NULL;
plist *m_swIdInstance[25];
plist *m_biosAttr = NULL;
plist *m_biosElement = NULL;
plist *m_biosAttrInstance[10];
plist *m_biosEleInstance[5];
plist *m_processorCIM = NULL;
plist *m_processorInstance[5];
plist *m_memoryCIM = NULL;
plist *m_memInstance[5];
plist *m_OperationSysCIM = NULL;
INT16S   m_dashPtype;

plist *m_fanCIM = NULL;
plist *m_fanInstance[5];

plist *m_SensorCIM = NULL;
plist *m_sensorInstance[5];


plist *m_CardCIM = NULL;
plist *m_CardInstance[5];

plist *m_ChassisCIM = NULL;
plist *m_ChassisInstance[5];

plist *m_SlotCIM = NULL;
plist *m_SlotInstance[10];

plist *m_ChipCIM = NULL;
plist *m_ChipInstance[5];

plist *m_PhyMemCIM = NULL;
plist *m_PhyMemInstance[10];

ROM_EXTERN RTSkt *dashcs _ATTRIBUTE_ROM_BSS;
ROM_EXTERN RTSkt *dashtlscs _ATTRIBUTE_ROM_BSS;
unsigned short currentPort;
int bind_port,inband_status=-1,AttrValOff=0;
INT16U new_power;
INT32U privilege[8];
INT8U OpaqueMask=0,*OpaqueBuf,*AttrValBuf=NULL;
BiosAttr *PLDMptr;
extern unsigned char* DASH_OS_PSH_Buf;
extern OS_EVENT *DASH_OS_Push_Event;
extern DHCPv4Config DHCPv4Conf[];

enum {
    ID_RequestStateChange,
    ID_RequestPowerStateChange,
    ID_CreateBootConfigSetting,
    ID_ApplyBootConfigSetting,
    ID_ChangeBootOrder,
    ID_CreateAccount,
    ID_AssignRoles,
    ID_CreateRole,
    ID_ModifyRole,
    ID_DeleteRole,
    ID_FirmwareUpdate,
    ID_CreateOpaque,
    ID_ReadOpaque,
    ID_WriteOpaque,
    ID_ReassignOwnershipOpaque,
    ID_SetBIOSAttribute,
    ID_RestoreBIOSDefaults,
    ID_SetBootConfigRole,

    Cdhcp,
    Ctextredirection,
    Cusbredirection_Connect,
    Cusbredirection_Listen,
    Cfirmwareupdate,
    Cpowercontrol,
    Csubscribe,
    Cunsubscribe,
    NotReady
};

static const struct _fault fault[] = {
    {"wsa:DestinationUnreachable","InvalidResourceURI",WSA_ACTION_FAULT},
    {"wsa:MessageInformationHeaderRequired","InvalidResourceURI",WSA_ACTION_FAULT},
    {"wsa:ActionNotSupported","InvalidResourceURI",WSA_ACTION_FAULT},
    {"wsman:EncodingLimit","MinimumEnvelopeLimit",WSMAN_ACTION_FAULT},
    {"wsman:InvalidSelectors","DuplicateSelectors",WSMAN_ACTION_FAULT},
    {"wsman:InvalidSelectors","UnexpectedSelectors",WSMAN_ACTION_FAULT},
    {"wsman:InvalidSelectors","InsufficientSelectors",WSMAN_ACTION_FAULT},
    {"wsman:UnsupportedFeature","OperationTimeout",WSMAN_ACTION_FAULT},
    {"wsa:InvalidMessageInformationHeader","OperationTimeout",WSA_ACTION_FAULT},
    {"wsman:UnsupportedFeature","ExpirationTime",WSMAN_ACTION_FAULT},
    {"wsen:FilterDialectRequestedUnavailable","InvalidResourceURI",WSENUM_ACTION_FAULT},
    {"wsen:CannotProcessFilter","CannotProcessFilter",WSENUM_ACTION_FAULT},
    {"wsen:InvalidEnumerationContext","InvalidEnumerationContext",WSENUM_ACTION_FAULT},
    {"wsa:InvalidMessageInformationHeader","InvalidMessageInformationHeader",WSA_ACTION_FAULT},
    {"wsa:InvalidMessageInformationHeader","DuplicateMessageID",WSA_ACTION_FAULT},
    {"wsmb:PolymorphismModeNotSupported","PolymorphismModeNotSupported",WSMB_ACTION_FAULT},
    {"wse:EventSourceUnableToProcess","EventSourceUnableToProcess",EVT_ACTION_FAULT},
    {"wse:InvalidMessage","EventInvalidMessage",EVT_ACTION_FAULT},
    {":InvalidRepresentation","InvalidNamespace",WSXF_ACTION_FAULT},
    {"s:MustUnderstand","NotUnderstood",WSA_ACTION_FAULT}

};

static struct _wselement element[] =
{
    { WSMID_IDENTIFY,NULL },
    { WSM_RESOURCE_URI, NULL},
    { WSA_ACTION, NULL},
    { WSM_REQUEST_TOTAL, NULL},
    { WSA_REPLY_TO,NULL},
    { WSA_TO,NULL},
    { WSA_MESSAGE_ID,NULL},
    { WSMB_ASSOCIATED_INSTANCES,NULL},
    { WSMB_ASSOCIATION_INSTANCES,NULL},
    { WSA_EPR,NULL},
    { WSM_SELECTOR_SET,NULL},
    { WSM_SELECTOR,NULL},
    { WSM_MAX_ENVELOPE_SIZE,NULL},
    { WSM_OPERATION_TIMEOUT, NULL},
    { WSM_FRAGMENT_TRANSFER,NULL},
    { "Identifier",NULL},
    {WSM_OPTION_SET,NULL},
    { 0, 0 }
};
static const struct _wselement method[] =
{
    {"RequestStateChange",NULL},
    {"RequestPowerStateChange",NULL},
    {"CreateBootConfigSetting",NULL},
    {"ApplyBootConfigSetting",NULL},
    {"ChangeBootOrder",NULL},
    {"CreateAccount",NULL},
    {"AssignRoles",NULL},
    {"CreateRole",NULL},
    {"ModifyRole",NULL},
    {"DeleteRole",NULL},
    {"InstallFromURI",NULL},
    {"Create",NULL},
    {"Read",NULL},
    {"Write",NULL},
    {"ReassignOwnership",NULL},
    {"SetBIOSAttribute",NULL},
    {"RestoreBIOSDefaults",NULL},
    {"SetBootConfigRole",NULL},
    { 0, 0 }
};
struct _wselement bodyelement[] =
{
    { WSMID_IDENTIFY,   NULL},
    { WSA_REFERENCE_PARAMETERS,NULL},
    { WSENUM_ENUMERATE, NULL},
    { WSENUM_RELEASE,   NULL},
    { WSENUM_PULL, NULL},
    { "_INPUT", NULL},
    { WSENUM_SUBSCRIBE, NULL},
    { WSENUM_GETSTATUS, NULL},
    { 0, 0 }
};

WsXmlNsData     Ns[] =
{
    {XML_NS_SOAP_1_2, "",0},
    {XML_NS_ADDRESSING, "",0},
    {XML_NS_EVENTING, "",0},
    {XML_NS_ENUMERATION, "",0},
    {XML_NS_SCHEMA_INSTANCE, "",0},
    {XML_NS_CIM_SCHEMA, "",0},
    {XML_NS_WS_MAN_CAT, "",0},
    {XML_NS_WSMAN_ID, "",0},
    {XML_NS_XML_SCHEMA, "",0},
    {XML_NS_WS_MAN, "",0},
    {XML_NS_CIM_BINDING, "",0},
    {XML_NS_OPENWSMAN, "",0},
    {XML_NS_TRANSFER, "",0},
    {XML_NS_CIM_CLASS, "",0},
    {0, "",0}
};

XMLElt _ElementCapabilities[]=
{
    {"Capabilities","CIM_BootServiceCapabilities",1},
    {"ManagedElement","CIM_BootService",1},
    {"1"},
    //  {"Capabilities","CIM_EnabledLogicalElementCapabilities",1},
    //  {"ManagedElement","CIM_ComputerSystem",1},
    //  {"1"},
    {"Capabilities","CIM_PowerManagementCapabilities",1},
    {"ManagedElement","CIM_PowerManagementService",1},
    {"1"},
    {"Capabilities","CIM_AccountManagementCapabilities",1},
    {"ManagedElement","CIM_AccountManagementService",1},
    {"1"},
    {"Capabilities","CIM_RoleBasedManagementCapabilities",1},
    {"ManagedElement","CIM_RoleBasedAuthorizationService",1},
    {"1"},
    {"Capabilities","CIM_PhysicalAssetCapabilities",1},
    {"ManagedElement","CIM_Chassis",1},
    {"1"},
    {"Capabilities","CIM_DHCPCapabilities",1},
    {"ManagedElement","CIM_DHCPProtocolEndpoint",1},
    {0,0}
};

XMLElt _EnabledLogicalElementCapabilities[]=
{
    {"ElementName","Capabilities"},
    {"ElementNameEditSupported","true"},
    {"InstanceID","Capabilities",1},
    {"MaxElementNameLen","256"},
    {"RequestedStatesSupported","2"},
    {"RequestedStatesSupported","3"},
    {"RequestedStatesSupported","11"},
    {0,0}
};

XMLElt _RegisteredProfile[]=
{
    {"AdvertiseTypes","2"},
    {"ElementName","Base Desktop and Mobile Profile"},
    {"InstanceID","Register",1},
    {"RegisteredName","Base Desktop and Mobile Profile"},
    {"RegisteredOrganization","2"},
    {"RegisteredVersion","1.0.0"},
    {0,0}
};
XMLElt _ManagedElement[]=
{
    {"Caption","xyz"},
    {"Description","xyz"},
    {0,0}
};
XMLElt _ComputerSystem[]=
{
    {"Caption","ComputerSystem"},
    {"CreationClassName","CIM_ComputerSystem",1},
    {"Dedicated","32"},
    {"Description","xyz"},
    {"ElementName","CS"},
    {"EnabledDefault","2"},
    {"EnabledState",enabled_state},
    {"HealthState","5"},
    {"IdentifyingDescriptions","CIM:GUID"},
    {"InstallDate","null"},
    {"Name",local_ip,1},
    {"NameFormat","IP"},
    {"OperationalStatus","0"},
    {"OtherEnabledState","null"},
    {"OtherIdentifyingInfo","",0,0,SYSINFO_UUID},
    {"PrimaryOwnerContact","ManagedSystem"},
    {"PrimaryOwnerName","SYSTEM"},
    {"RequestedState","5"},
    {"ResetCapability","2"},
    {"Roles","Managed System"},
    {"Status","Unknown"},
    {"TimeOfLastStateChange","null"},
    {0,0}
};

XMLElt _Processor[]=
{
    {"AddressWidth", "32"},
    {"CPUStatus","",0,0,PROCESSOR_STATUS},
    {"CreationClassName","CIM_Processor",1},
    {"CurrentClockSpeed","3000",0,0,PROCESSOR_CURRENT_SPEED},
    {"DataWidth",  "32"},
    {"DeviceID", "Processor",1},
    {"ElementName","Processor"},
    {"EnabledState","2"},
    {"ExternalBusClockSpeed","200",0,0,PROCESSOR_ECLK},
    {"Family","1",0,0,PROCESSOR_FAMILY,1},
    {"HealthState","5"},
    {"LoadPercentage","0"},
    {"MaxClockSpeed","3800",0,0,PROCESSOR_MAX_SPEED},
    {"OperationalStatus","2"},
    {"OtherFamilyDescription","7"},
    {"RequestedState","2"},
    {"Stepping", "Intel",0,0,PROCESSOR_VER},
    {"SystemCreationClassName","CIM_ComputerSystem",1},
    {"SystemName",local_ip,1},
    {"UniqueID","CPU0",0,0,PROCESSOR_ID},
    {"UpgradeMethod","Other",0,0,PROCESSOR_UPGRADE,1},
    {0,0}
};
XMLElt _AssociatedCacheMemory[]=
{
    {"Antecedent","CIM_Memory",1},
    {"Associativity","3"},
    {"CacheType","3"},
    {"Dependent","CIM_Processor",1},
    {"Level","3"},
    {"OtherCacheTypeDescription","1"},
    {"ReadPolicy","0"},
    {"WritePolicy","2"},
    {0,0}
};

XMLElt _Memory[]=
{
    {"Access","3"},
    {"BlockSize","1"},
    {"ConsumableBlocks","0",0,0,MEMDEV_SIZE},
    {"CreationClassName","CIM_Memory",1},
    {"DeviceID","Memory",1,0,1},
    {"ElementName","Total System Memory"},
    {"EnabledState","2"},
    {"HealthState","5"},
    {"NumberOfBlocks","0",0,0,MEMDEV_SIZE},
    {"OperationalStatus","2"},
    {"RequestedState","12"},
    {"SystemCreationClassName","CIM_ComputerSystem",1},
    {"SystemName",local_ip,1},
    {"Volatile","true"},
    {0,0}
};
XMLElt _CacheMemory[]=
{
    {"Access","3"},
    {"BlockSize","1"},
    {"ConsumableBlocks","0",0,0,CACHE_INSTALL_SIZE,2},
    {"CreationClassName","CIM_Memory",1},
    {"DeviceID","CacheMemory",1},
    {"ElementName","Cache",0,0,CACHE_SKT_DSEIGN},
    {"EnabledState","2"},
    {"HealthState","5"},
    {"NumberOfBlocks","0",0,0,CACHE_INSTALL_SIZE,2},
    {"OperationalStatus","2"},
    {"RequestedState","12"},
    {"SystemCreationClassName","CIM_ComputerSystem",1},
    {"SystemName",local_ip,1},
    {"Volatile","true"},
    {0,0}
};

XMLElt _BootService[]=
{
    {"CreationClassName","CIM_BootService"},
    {"ElementName","BootService"},
    {"Name","BootService",1},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName","ManagedSystem"},
    {0,0}
};
XMLElt _BootConfigSetting[]=
{
    {"ElementName","Bootcfgsetting"},
    {"InstanceID","Bootcfgsetting",1},
    /*
    {"1",r_uri},
    {"ElementName",local_ip},
    {"InstanceID","BootCS",1},
    */
    {0,0}
};

XMLElt _BootSourceSetting[]=
{
    {"ElementName","Bootsrcsetting",0,0,1},
    {"FailThroughSupported","1"},
    {"InstanceID","",1},
    {"StructuredBootString","",0,0,1},
    {0,0}
};

XMLElt _BootServiceCapabilities[]=
{
    {"BootConfigCapabilities","2"},
    {"BootConfigCapabilities","3"},
    {"ElementName","BSC"},
    {"ElementNameEditSupported","true"},
    {"InstanceID","BSC",1},
    {0,0}
};

XMLElt _PowerManagementService[]=
{
    {"CreationClassName","CIM_PowerManagementService",1},
    {"ElementName","Power Management Service"},
    {"Name","ManagedSystem",1},
    {"RequestedPowerState",request_state},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {0,0}
};
const XMLElt _PowerManagementCapabilities[]=
{
    {"ElementName","Capabilities of the Power Management Service"},
    {"InstanceID","PowerManagement",1},

    {"PowerChangeCapabilities","3"},
    {"PowerChangeCapabilities","4"},
    {"PowerChangeCapabilities","6"},
    {"PowerChangeCapabilities","7"},
    {"PowerChangeCapabilities","8"},

    {"PowerStatesSupported","2"},
    {"PowerStatesSupported","4"},
    {"PowerStatesSupported","5"},
    {"PowerStatesSupported","7"},
    {"PowerStatesSupported","8"},
    {"PowerStatesSupported","9"},
    {"PowerStatesSupported","10"},
    {"PowerStatesSupported","11"},
    {"PowerStatesSupported","12"},
    {"PowerStatesSupported","13"},
    {"PowerStatesSupported","14"},
    {"PowerStatesSupported","15"},
    {"PowerStatesSupported","16"},

    {0,0}
};

XMLElt _HostedService[]=
{
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_BootService",1},
    {"1"},
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_PowerManagementService",1},
    {"1"},
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_AccountManagementService",1},
    {"1"},
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_RoleBasedAuthorizationService",1},
    {"1"},
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_IndicationService",1},
    {"1"},
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_TextRedirectionService",1},
    {"1"},
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_USBRedirectionService",1},
    {0,0}
};
XMLElt _ElementConformsToProfile[]=
{
    {"ConformantStandard","CIM_RegisteredProfile",1},
    {"ManagedElement","CIM_ComputerSystem",1},
    {0,0}
};
XMLElt _ReferencedProfile[]=
{
    {"Antecedent","CIM_RegisteredProfile",1},
    {"Dependent","CIM_RegisteredProfile",1},
    {0,0}
};
XMLElt _ElementSettingData[]=
{
    {"IsCurrent","2"},
    {"IsDefault","2"},
    {"IsNext","1"},
    {"ManagedElement","CIM_ComputerSystem",1},
    {"SettingData","CIM_BootConfigSetting",1},
    /*
      {"1"},
      {"IsCurrent","1"},
      {"IsDefault","1"},
      {"IsNext","1"},
      {"ManagedElement","CIM_IndicationService",1},
      {"SettingData","CIM_IndicationServiceSettingData",1},
    */
    /*
    {"1",r_uri},
    {"IsCurrent","2"},
    {"IsDefault","2"},
    {"IsNext","2"},
    {"ManagedElement","CIM_IPProtocolEndpoint"},
    {"SettingData","CIM_StaticIPAssignmentSettingData"},
    */
    {0,0}
};
XMLElt _OrderedComponent[]=
{
    {"AssignedSequence","1"},
    {"GroupComponent","CIM_BootConfigSetting",1},
    {"PartComponent","CIM_BootSourceSetting",1},
    {0,0}
};
XMLElt _ServiceAffectsElement[]=
{
    {"AffectedElement","CIM_ComputerSystem",1},
    {"AffectingElement","CIM_BootService",1},
    {"ElementEffects","5"},
    {"1"},
    {"AffectedElement","CIM_Role",1},
    {"AffectingElement","CIM_RoleBasedAuthorizationService",1},
    {"ElementEffects","5"},
    {"1"},
    {"AffectedElement","CIM_ListenerDestinationWSManagement",1},
    {"AffectingElement","CIM_IndicationService",1},
    {"1"},
    {"AffectedElement","CIM_TextRedirectionSAP",1},
    {"AffectingElement","CIM_TextRedirectionService",1},
    {"1"},
    {"AffectedElement","CIM_USBRedirectionSAP",1},
    {"AffectingElement","CIM_USBRedirectionService",1},
    {"1"},
    {"AffectedElement","CIM_CIM_USBDevice",1},
    {"AffectingElement","CIM_USBRedirectionService",1},
    {0,0}
};
XMLElt _SystemDevice[]=
{
    {"GroupComponent","CIM_ComputerSystem",1},
    {"PartComponent","CIM_Processor",1},
    {"1"},
    {"GroupComponent","CIM_ComputerSystem",1},
    {"PartComponent","CIM_Memory",1},
    {"1"},
    {"GroupComponent","CIM_ComputerSystem",1},
    //  {"PartComponent","CIM_Sensor",1},
    {"PartComponent","CIM_NumericSensor",1},
    {"1"},
    {"GroupComponent","CIM_ComputerSystem",1},
    {"PartComponent","CIM_Fan",1},
    {"1"},
    {"GroupComponent","CIM_ComputerSystem",1},
    {"PartComponent","CIM_PowerSupply",1},
    {0,0}
};
XMLElt _ComputerSystemPackage[]=
{

    {"Antecedent","CIM_Chassis",1},
    {"Dependent","CIM_ComputerSystem",1},
    {"PlatformGUID","00000000000000000000000000000000"},

    {0,0}
};
XMLElt _SystemPackaging[]=
{
    {"Antecedent","CIM_Chassis",1},
    {"Dependent","CIM_ComputerSystem",1},
    {0,0}
};
XMLElt _AssociatedPowerManagementService[]=
{
    {"AvailableRequestedPowerStates","2"},
    // {"AvailableRequestedPowerStates","3"},
    {"AvailableRequestedPowerStates","4"},
    {"AvailableRequestedPowerStates","7"},
    {"AvailableRequestedPowerStates","8"},
    {"AvailableRequestedPowerStates","10"},
    {"PowerOnTime","PT10S"},
    {"PowerState",power_state},
    {"RequestedPowerState",request_state},
    {"ServiceProvided","CIM_PowerManagementService",1},
    {"UserOfService","CIM_ComputerSystem",1},
    {0,0}
};
XMLElt _IndicationService[]=
{
    {"CreationClassName","CIM_IndicationService",1},
    {"DeliveryRetryAttempts","3"},
    {"DeliveryRetryInterval","3"},
    {"ElementName","IndicationService"},
    // {"FilterCreationEnabled","true"},
    {"FilterCreationEnabled","false"},
    {"Name","IndicationService"},
    {"SubscriptionRemovalAction","2"},
    {"SubscriptionRemovalTimeInterval","3"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip,1},
    {0,0}
};
XMLElt _IndicationServiceSettingData[]=
{
    {"DeliveryRetryAttempts","3"},
    {"DeliveryRetryInterval","3"},
    {"FilterCreationEnabled","true"},
    {"InstanceID","indication1",1},
    {"SubscriptionRemovalAction","2"},
    {"SubscriptionRemovalTimeInterval","3"},
    {0,0}
};
XMLElt _IndicationFilter[]=
{
    {"CreationClassName","CIM_IndicationFilter",1},
    {"IndividualSubscriptionSupported","true"},
    {"Name","IndicationFilter",1},
    {"Query","SELECT * FROM CIM_AlertIndication"},
    {"QueryLanguage","WQL"},
    {"SourceNamespace","WQL"},
    {"SystemCreationClassName","CIM_IndicationFilter"},
    {"SystemName","ManagedSystem"},
    {0,0}
};
XMLElt _FilterCollection[]=
{
    {"CollectionName","Events"},
    {"ElementName","RTK:AllEvents"},
    {"InstanceID","RTK",1},
    {0,0}
};
XMLElt _ListenerDestinationWSManagement[]=
{
    {"CreationClassName","CIM_ListenerDestinationWSManagement",1},
    {"Destination","http://127.0.0.1:8080/eventsink"},
    {"ElementName","Listener"},
    {"Name","ListenerDestinationWSManagement:1",1},
    {"PersistenceType","2"},
    {"SystemCreationClassName","CIM_ComputerSystem",1},
    {"SystemName",local_ip,1},
    {0,0}
};
XMLElt _MemberOfCollection[]=
{
    /*
    {"Collection","CIM_FilterCollection"},
    {"Member","CIM_LintenerDestination"},
    {"1",r_uri},
    */
    {"Collection","CIM_Role",1},
    {"Member","CIM_Identity",1},

    {"1"},
    {"Collection","CIM_Role",1},
    {"Member","CIM_Privilege",1},

    {0,0}
};
XMLElt _FilterCollectionSubscription[]=
{
    {"FailureTriggerTimeInterval","0"},
    {"Filter","CIM_FilterCollection",1},
    {"Handler","CIM_ListenerDestinationWSManagement",1},
    {"OnFatalErrorPolicy","4"},
    {"RepeatNotificationPolicy","2"},
    {"SubscriptionState","2"},
    {0,0}
};
XMLElt _IndicationSubscription[]=
{
    //  {"FailureTriggerTimeInterval","1"},
    {"Filter","CIM_IndicationFilter",1},
    {"Handler","CIM_ListenerDestinationWSManagement",1},
//   {"OnFatalErrorPolicy","4"},
//   {"OtherSubscriptionState","1"},
//   {"RepeatNotificationPolicy","2"},
//   {"SubscriptionState","1"},
    {0,0}
};
XMLElt _OwningCollectionElement[]=
{
    {"OwnedElement","CIM_FilterCollection",1},
    {"OwningElement","CIM_IndicationService",1},
    {"1"},
    {"OwnedElement","CIM_Role",1},
    {"OwningElement","CIM_ComputerSystem",1},
    {0,0}
};
XMLElt _SoftwareIdentity[]=
{
    {"BuildNumber","1"},
    {"Classifications","10"},
    {"ElementName","Software"},
    {"IdentityInfoType","Dash Firmware"},
    {"IdentityInfoValue","1"},
    {"InstanceID","SW",1},
    {"IsEntity","true"},
    {"MajorVersion","1"},
    {"MinorVersion","0"},
    {"RevisionNumber","1"},
    {"TargetOSTypes","1"},
    {"VersionString","1.0.0"},
    {0,0}
};

XMLElt _NumericSensor[]=
{
    {"BaseUnits","2"},
    {"CommunicationStatus","4"},
    {"CreationClassName","CIM_NumericSensor"},
    {"CurrentReading","0"},
    {"CurrentState","Normal"},
    {"DeviceID","Sensor",1},
    {"ElementName","GPIO"},
    {"EnabledState",state},
    {"HealthState","5"},
#ifndef CONFIG_REDUCED_SENSOR_INSTANCE
    {"LowerThresholdCritical","0"},
    {"LowerThresholdFatal","0"},
    {"LowerThresholdNonCritical","0"},
#endif
    {"OperationalStatus","2"},
#ifndef CONFIG_REDUCED_SENSOR_INSTANCE
    {"PossibleStates","Non-Critical"},
    {"PossibleStates","Lower Non-Critical"},
    {"PossibleStates","Upper Non-Critical"},
    {"PossibleStates","Critical"},
    {"PossibleStates","Lower Critical"},
    {"PossibleStates","Upper Critical"},
    {"PossibleStates","Fatal"},
    {"PossibleStates","Lower Fatal"},
    {"PossibleStates","Upper Fatal"},
#endif
    {"PossibleStates","Normal"},
    {"PossibleStates","Unknown"},
    {"RateUnits","0"},
    {"RequestedState","5"},
    {"SensorType","2"},
    //{"OtherSensorTypeDescription","Legacy Sensor"},
    {"SettableThresholds",""},
    {"SupportedThresholds",""},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {"UnitModifier","0"},
#ifndef CONFIG_REDUCED_SENSOR_INSTANCE
    {"UpperThresholdCritical","0"},
    {"UpperThresholdFatal","0"},
    {"UpperThresholdNonCritical","0"},
#endif
    {0,0}
};
XMLElt _Fan[]=
{
    {"ActiveCooling","true"},
    {"CreationClassName","CIM_Fan"},
    {"CurrentReading","1"},
//   {"DesiredSpeed","0"},
    {"DeviceID","Fan",1},
    {"ElementName","Fan"},
    {"EnabledState",state},
    {"HealthState","5"},
    {"OperationalStatus","2"},
    //{"RequestedState",request_state},
    {"RequestedState","12"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName","IP"},
    {"VariableSpeed","false"},
    {0,0}
};
XMLElt _AssociatedSensor[]=
{
    {"Antecedent","CIM_NumericSensor",1},
    {"Dependent","CIM_Fan",1},
    {0,0}
};

XMLElt _PowerSupply[]=
{
    {"CreationClassName","CIM_PowerSupply"},
    {"CurrentReading","1"},
    {"DeviceID","PowerSupply",1},
    {"ElementName","PowerSupply"},
    {"EnabledState","2"},
    {"HealthState","5"},
    {"OperationalStatus","2"},
    //{"RequestedState",request_state},
    {"RequestedState","12"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName","IP"},
    {"TotalOutputPower","0"},
    {0,0}
};
XMLElt _PhysicalComponent[]=
{
    {"CIM_Chip"},
    {"CIM_PhysicalMemory"},
    {0,0}
};
XMLElt _PhysicalConnector[]=
{
    {"CIM_Slot"},
    {0,0}
};
XMLElt _PhysicalFrame[]=
{
    {"CIM_Chassis"},
    {0,0}
};

XMLElt _PhysicalElement[]=
{
    {"CIM_Card"},
    {"CIM_Chassis"},
    {"CIM_Chip"},
    {"CIM_PhysicalMemory"},
    {"CIM_Slot"},
    {0,0}
};
XMLElt _PhysicalPackage[]=
{
//   {"CIM_Card"},
    {"CIM_Chassis"},
    {0,0}
};

XMLElt _PhysicalMemory[]=
{
    {"BankLabel", "bank X",0,0,MEMDEV_BANK_LOCATOR},
    {"CanBeFRUed","false"},
    {"Capacity", "512",0,0,MEMDEV_SIZE,2},
    {"CreationClassName","CIM_PhysicalMemory",1},
    //  {"DataWidth", "32",0,0,MEMDEV_DATA_WIDTH},
    //  {"DeviceID","Memory"},
    {"ElementName","Memory"},
    {"FormFactor","8"},
    //  {"InterleavePosition", "1"},
    {"Manufacturer","HW Co.",0,0,MEMDEV_MAN},
    {"MemoryType", "24"},
    {"PartNumber","",0,0,MEMDEV_PN},
    // {"PositionInRow", "1"},
    {"SerialNumber","",0,0,MEMDEV_SN},
    {"Speed", "0",0,0,MEMDEV_SPEED},
    {"Tag","Mem",1},
    {"UserTracking","NA"},
    //  {"TotalWidth", "32768",0,0,MEMDEV_WIDTH},
    {0,0}
};



XMLElt _Card[]=
{
    {"CanBeFRUed","true"},
    {"CreationClassName","CIM_Card"},
    {"ElementName","BaseBoard"},
    {"HostingBoard","true"},
    {"Manufacturer","",0,0,BBINFO_MAN},
    {"Model","",0,0,BBINFO_PN} ,
    {"PackageType","",0,0,BBINFO_BOARD_TYPE},
    {"PartNumber","",0,0,BBINFO_VER},
    {"SerialNumber","",0,0,BBINFO_SN},
    {"Tag","Card",1},
    {0,0}
};
XMLElt _Container[]=
{
    {"GroupComponent","CIM_Chassis",1},
    {"PartComponent","CIM_PhysicalMemory",1},
    {0,0}
};

XMLElt _AccountManagementService[]=
{
    {"CreationClassName","CIM_AccountManagementService",1},
    {"ElementName","AccountManagement"},
    {"EnabledState","2"},
    {"Name","AccountManagement"},
    {"RequestedState","12"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName","IP"},
    {0,0}
};
XMLElt _AccountManagementCapabilities[]=
{
    {"ElementName","AccountManagementCapabilities"},
    {"ElementNameEditSupported","true"},
    {"InstanceID","AMC",1},
    {"MaxElementNameLen","256"},
    {"OperationsSupported","2"},
    {"RequestedStatesSupported","2"},
    {0,0}
};
XMLElt _RoleBasedAuthorizationService[]=
{
    {"CreationClassName","CIM_RoleBasedAuthorizationService",1},
    {"ElementName","RoleBased"},
    {"Name","RBAS"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName","IP"},
    {0,0}
};
XMLElt _RoleBasedManagementCapabilities[]=
{
    {"ElementName","RBAC"},
    {"InstanceID","RBAC",1},
    {"SharedPrivilegeSupported","true"},
    {"SupportedMethods","0"},
    {"SupportedMethods","4"},
    {"SupportedMethods","5"},
    {"SupportedMethods","9"},
    {0,0}
};
XMLElt _Role[]=
{
    {"CommonName","Role"},
    {"CreationClassName","CIM_Role"},
    {"ElementName","Role"},
    {"Name","Role",1},
    {"RoleCharacteristics","2"},
    /*
    {"1"},
    {"CommonName","Role"},
    {"CreationClassName","CIM_Role"},
    {"ElementName","User"},
    {"Name","User",1},
    {"RoleCharacteristics","2"},
    {"1"},
    {"CommonName","Role"},
    {"CreationClassName","CIM_Role"},
    {"ElementName","Guest"},
    {"Name","Guest",1},
    {"RoleCharacteristics","2"},
    */
    {0,0}
};
XMLElt _RoleLimitedToTarget[]=
{
    {"DefiningRole","CIM_Role",1},
    {"TargetElement","CIM_ComputerSystem",1},
    {0,0}
};
XMLElt _Chassis[]=
{
    {"CanBeFRUed","false"},
    {"ChassisPackageType","3"},
    {"CreationClassName","CIM_Chassis",1},
    {"ElementName","Chassis"},
    {"Manufacturer","",0,0, CHASSIS_MAN},
    {"Model","",0,0, SYSINFO_PN},
    {"PackageType","3"},
    {"SKU","",0,0,SYSINFO_SKU},
    {"SerialNumber","",0,0,CHASSIS_SN},
    {"Tag","Chassis",1},
    {"UserTracking",""},
    {"Version","",0,0,CHASSIS_VER},
    {0,0}
};

XMLElt _PhysicalAssetCapabilities[]=
{
    {"ElementName","PAC"},
    {"FRUInfoSupported","true"},
    {"InstanceID","PAC",1},
    {0,0}
};
XMLElt _Realizes[]=
{
    {"Antecedent","CIM_PhysicalMemory",1},
    {"Dependent","CIM_Memory",1},
    {0,0}
};
XMLElt _ServiceAvailableToElement[]=
{
    {"ServiceProvided","CIM_PowerManagementService",1},
    {"UserOfService","CIM_ComputerSystem",1},
    {0,0}
};
XMLElt  _Account[]=
{
    {"Caption","Account for admin"},
    {"CreationClassName","CIM_Account"},
    {"ElementName","Account"},
    {"EnabledState","2"},
    {"Name","admin",1},
    {"RequestedState","12"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {"UserID","admin"},
    {"UserPassword","***"},
    {0,0}
};
XMLElt _Privilege[]=
{
    {"Activities","7"},
    {"ActivityQualifiers","Admin"},
    {"ElementName","Admin Execute"},
    {"InstanceID","priv",1},
    {"PrivilegeGranted","true"},
    {"QualifierFormats","9"},
    {"RepresentsAuthorizationRights","false"},
    {0,0}
};

XMLElt _Identity[]=
{
    {"InstanceID","ID",1},
    {"ElementName","Admin"},
    {0,0}
};
XMLElt _UserContact[]=
{
    {"UserID","admin"},
    {"Name","admin",1},
    {"CreationClassName","CIM_UserContact"},
    {"ElementName","UserContact"},
    {0,0}
};
XMLElt _AssignedIdentity[]=
{
    {"IdentityInfo","CIM_Identity",1},
    {"ManagedElement","CIM_UserContact",1},
    {0,0}
};
XMLElt _AlertIndication[]=
{
    {"MessageID","PET111"},
    {0,0}
};
XMLElt _IPProtocolEndpoint[]=
{
    {"AddressOrigin","3"},
    {"CreationClassName","CIM_IPProtocolEndpoint",1},
    {"ElementName","ip1"},
    {"EnabledState","2"},
    {"GatewayIPv4Address",local_gateway,2},
    {"IPv4Address",local_ip,2},
    {"Name","IPProtocol",1},
    {"NameFormat","2"},
    {"ProtocolIFType","4096"},
    {"RequestedState","12"},
    {"SubnetMask",local_netmask,2},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName","IP"},
    {0,0}
};
XMLElt _StaticIPAssignmentSettingData[]=
{
    {"InstanceID","StaticIP",1},
    {"AddressOrigin","3"},
    {"ElementName","ip1"},
    {"IPv4Address",local_ip,2},
    {"SubnetMask",local_netmask,2},
    {"GatewayIPv4Address",local_gateway,2},

    {0,0}
};
XMLElt _RecordLog[]=
{
    {"CurrentNumberOfRecords","3"},
    {"ElementName","INFO"},
    {"EnabledState","2"},
    {"InstanceID","id",1},
    {"MaxNumberOfRecords","256"},
    {"Name","record"},
    {"OverwritePolicy","2"},
    {0,0}
};

XMLElt _LogManagesRecord[]=
{
    {"Log","CIM_RecordLog"},
    {"Record","CIM_LogEntry"},
    {0,0}
};

XMLElt _LogEntry[]=
{
    {"InstanceID","entry1",1},
    {"ElementName","info"},
    {"LogInstanceID","id"},
    {"CreationTimeStamp","20081002152300.000000-000"},
    {"LogName","info"},
    {"RecordID","256"},
    {"RecordData","*10.1.2*5*BMC Ambient Temp temperature sensor returned to normal*22*C",0,0,1},
    {"RecordFormat","RecordType*string"},
    {0,0}
};
XMLElt _TextRedirectionService[]=
{
    {"CreationClassName","CIM_TextRedirectiontService"},
    {"ElementName","Text Redirection Service"},
    {"EnabledState",state},
    {"MaxCurrentEnabledSAPs","1"},
    {"Name","TextRedirectService",1},
    {"RedirectionServiceType","2"},
    {"RequestedState",state},
    {"SharingMode","3"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {0,0}
};
XMLElt _TextRedirectionSAP[]=
{
    {"CreationClassName","CIM_TextRedirectionSAP"},
    {"ElementName","Text Redirection SAP for the Telnet Service"},
    {"EnabledState",state},
    {"Name","TextRedirectionSAP",1},
    //  {"RequestedState",request_state},
    {"RequestedState",state},
    {"SessionTerminateSequence","", 2},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {"TextFlowType","3"},

    {0,0}
};
XMLElt _BIOSService[]=
{
    {"CreationClassName","CIM_BIOSService",1},
    {"Name","bios",1},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {0,0}
};
XMLElt _BIOSServiceCapabilities[]=
{
    {"InstanceID","Bios",1},
    {"MethodsSupported","0"},
    {"MethodsSupported","1"},
    {"MethodsSupported","2"},
    {"PasswordEncoding","0"},
    {0,0}
};
XMLElt _BIOSElement[]=
{
    {"Manufacturer","ManagedSystem",0,0,BIOS_VENDOR},
    {"Name","bios",1},
    {"SoftwareElementID","100"},
    {"SoftwareElementState","2"},
    {"TargetOperatingSystem","1"},
    {"Version","1.0.0",0,0,BIOS_VER},
    {0,0}
};
XMLElt _BIOSAttribute[]=
{
//   {"AttributeName","CIM:name:1",0,0,BIOS_CHARS},
    {"AttributeName","",0,0,BIOS_CHARS},
    {"CurrentValue","",0,0,BIOS_CHARS_EXT_BYTE},
    {"InstanceID","Attribute",1},
    {"IsReadOnly","true"},
    {"PossibleValues",""},
    {0,0}
};
XMLElt _USBRedirectionCapabilities[]=
{
    {"ClassesSupported","8"},
    {"ConnectionModesSupported","3"},
    {"EelementName","Capabilities of the USB Redirection Service"},
    {"ElementNameEditSupported","false"},
    {"InfoFormatsSupported","200"},
    {"InstanceID","URC"},
    {"MaxDevicesPerSAP","1"},
    {"RequestedStatesSupported","2"},
    {"RequestedStatesSupported","3"},
    {"SAPCapabilitiesSupported","2"},
    {"SingleClassPerSAP","true"},
    {"USBVersionsSupported","512"},
    {0,0}
};
XMLElt _USBRedirectionService[]=
{
    {"CreationClassName","CIM_USBRedirectiontService"},
    {"ElementName","USBRedirectionService"},
    {"EnabledState","2"},
    {"MaxCurrentEnabledSAPs","2"},
    {"Name","USBRedirectionService",1},
    {"RedirectionServiceType","4"},
    {"RequestedState",request_state},
    {"SharingMode","2"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {0,0}
};

XMLElt _USBRedirectionSAP[]=
{
    {"ConnectionMode","3"},
    {"CreationClassName","CIM_USBRedirectiontSAP"},
    {"ElementName","USBRedirectionSAP"},
    {"EnabledState",state},
    {"Name","USBRedirectionSAP",1},
    {"RequestedState",state},
    {"ResetTimeout","PT1S"},
    {"SessionTimeout","PT1S"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {0,0}
};


XMLElt _OperatingSystem[]=
{
    {"CreationClassName","CIM_OperatingSystem",1},
    {"Name","Unknown OS"},
    {"OSType","0"},
    {"EnabledState",state},
    {"RequestedState","5"},
    {0,0}
};
XMLElt _DNSProtocolEndpoint[]=
{
    {"CreationClassName","CIM_DNSProtocolEndPoint",1},
    {"Name","DNS",1},
    {"AcessContext","3"},
    {"AccessInfo",""},
    {"InfoFormat","3"},
    {"ElementName","DNS"},
    {0,0}
};
XMLElt _Slot[]=
{
    {"ConnectorLayout","7"},
    {"ConnectorType","1"},
    {"CreationClassName","CIM_Slot"},
    {"ElementName","Slot"},
    {"Model","NA",0,0,SLOT_DESIGN},
    {"Number","",0,0,SLOT_ID}, 
    {"Tag","Slot",1},
    {0,0}
};
XMLElt _Chip[]=
{
    {"CanBeFRUed","false"},
    {"CreationClassName","CIM_Chip"},
    {"ElementName","Processor"},
    {"Manufacturer","",0,0, PROCESSOR_MAN},
    {"Model","",0,0,PROCESSOR_VER },
    {"PartNumber","",0,0,PROCESSOR_PN},
    {"SerialNumber","",0,0,PROCESSOR_SN},
    {"Tag","Processor",1},
    {"UserTracking","NA"},
    {0,0}
};

XMLElt _TCPProtocolEndpoint[]=
{
    {"CreationClassName","CIM_TCPProtocolEndPoint",1},
    {"SystemCreationClassName","CIM_EthernetPort",1},
    {"Name","TCRTCP",1},
    {"ElementName","TCP"},
    {"PortNumber","87"},
    // {"PortNumber","57"},
    {"ProtocolIFType","4400"},
    {"SystemName",local_ip},
    {0,0}
};

XMLElt _USBTCPProtocolEndpoint[]=
{
    {"CreationClassName","CIM_TCPProtocolEndPoint",1},
    {"SystemCreationClassName","CIM_EthernetPort",1},
    {"Name","USBTCP",1},
    {"ElementName","TCP"},
    {"PortNumber","59"},
    {"ProtocolIFType","4400"},
    {"SystemName",local_ip},
    {0,0}
};

XMLElt _DHCPProtocolEndpoint[]=
{
    {"ClientState",client_state},
    {"CreationClassName","CIM_DHCPProtocolEndpoint"},
    {"ElementName","dhcp"},
    {"EnabledState",dhcp_state},
    {"Name","dhcp",1},
    {"NameFormat","2"},
    {"OtherTypeDescription","dhcp"},
    {"ProtocolIFType","1"},
    {"RequestedState","12"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {0,0}
};
/*
XMLElt _NetworkPort[]=
{
    {"CreationClassName","CIM_NetworkPort"},
    {"ElementName","NetworkPort"},
    {"EnabledState","2"},
    {"LinkTechnology","2"},
    {"MaxSpeed","1000000000"},
    {"Name","NetworkPort",1},
    {"PermanentAddress","00e04c000001"},
    {"PortNumber","1"},
    {"RequestedState","12"},
    {"Speed","1000000000"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {0,0}
};
*/
XMLElt _AssociatedCooling[]=
{
    /*
      {"Antecedent","CIM_Fan",0},
    {"Dependent","CIM_Chassis"},
      {"1",r_uri},
      */
    {"Antecedent","CIM_Fan",1},
    {"Dependent","CIM_Processor",1},
    {0,0}
};
XMLElt _ServiceAccessBySAP[]=
{
    {"Antecedent","CIM_TextRedirectionService",1},
    {"Dependent","CIM_TextRedirectionSAP",1},
    {"1"},
    {"Antecedent","CIM_USBRedirectionService",1},
    {"Dependent","CIM_USBRedirectionSAP",1},
    {0,0}
};
XMLElt _HostedAccessPoint[]=
{
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_TextRedirectionSAP",1},
    {"1"},
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_USBRedirectionSAP",1},
    {"1"},
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_IPProtocolEndpoint",1},
    {"1"},
    {"Antecedent","CIM_ComputerSystem",1},
    {"Dependent","CIM_DHCPProtocolEndpoint",1},
    {0,0}
};
XMLElt _USBDevice[]=
{
    {"ClassCode","1"},
    {"CommandTimeout","PT10S"},
    {"CreationClassName","CIM_USBDevice",1},
    {"DeviceID","USBDevice",1},
    {"SystemCreationClassName","CIM_ComputerSystem",1},
    {"SystemName",local_ip,1},
    {0,0}
};
XMLElt _DHCPCapabilities[]=
{
    {"ElementName","DHCPCap"},
    {"ElementNameEditSupported","true"},
    {"InstanceID","DHCPCap",1},
    {"MaxElementNameLen","256"},
    {"OptionsSupported","0"},
    {0,0}
};
XMLElt _SAPSAPDependency[]=
{
    {"Antecedent","CIM_IPProtocolEndpoint",1},
    {"Dependent","CIM_DHCPProtocolEndpoint",1},
    {0,0}
};
XMLElt _ConcreteDependency[]=
{
    {"Antecedent","CIM_RegisteredProfile",1},
    {"Dependent","CIM_FilterCollection",1},
    {0,0}
};
XMLElt _SystemBIOS[]=
{
    {"GroupComponent","CIM_ComputerSystem",1},
    {"PartComponent","CIM_BIOSElement",1},
    {0,0}
};
XMLElt _ConcreteComponent[]=
{
    {"GroupComponent","CIM_BIOSElement",1},
    {"PartComponent","CIM_BIOSAttribute",1},
    {"1"},
    {"GroupComponent","CIM_BIOSElement",1},
    {"PartComponent","CIM_BIOSPassword",1},
    {0,0}
};
XMLElt _BIOSPassword[]=
{
    {"CurrentValue",""},
    {"IsSet","true"},
    {"MaxLength","32"},
    {"MinLength","0"},
    {"PendingValue",""},
    {0,0}
};
XMLElt _SoftwareInstallationService[]=
{
    {"CreationClassName","CIM_SoftwareInstallationService"},
    {"Name","SoftwareInstallation",1},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {0,0}
};
XMLElt _BindsTo[]=
{
    {"Antecedent","CIM_TCPProtocolEndpoint",1},
    {"Dependent","CIM_TextRedirectionSAP",1},
    {0,0}
};
XMLElt _RemoteAccessAvailableToElement[]=
{
    {"Antecedent","CIM_RemoteServiceAccessPoint",1},
    {"Dependent","CIM_USBRedirectionSAP",1},
    {0,0}
};
XMLElt _RemoteServiceAccessPoint[]=
{
    {"CreationClassName","CIM_RemoteServiceAccessPoint",1},
    {"ElementName","USB Redirection Remote Access Point"},
    {"Name","RemoteServiceAccessPoint",1},
    {"AcessContext","1"},
    //  {"AccessInfo",access_info},
    {"AccessInfo",""},
    // {"InfoFormat","3"},
    {"InfoFormat","200"},
    {"otherAccessContext","USB Redirection Destination"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {0,0}
};
XMLElt _OpaqueManagementData[]=
{
    {"CreationClassName","CIM_OpaqueManagementData"},
    {"DeviceID","Block",1},
    {"ElementName","OpaqueManagementData"},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {"DataFormat",""},
    {"DataSize",""},
    {"LastAccessed",""},
    {"MaxSize","1024"},
    {"WriteLimited","2"},
    {"Access","3"},
    {"DataOrganization","2"},
    {"BlockSize","4096"},
    {"NumberOfBlocks","4"},
    {"ConsumableBlocks","4"},
    {0,0}
};
XMLElt _OpaqueManagementDataService[]=
{
    {"CreationClassName","CIM_OpaqueManagementDataService"},
    {"Name","OpaqueManagementData",1},
    {"SystemCreationClassName","CIM_ComputerSystem"},
    {"SystemName",local_ip},
    {0,0}
};

XMLElt _ManagedSystemElement[]=
{
    {0,0}
};

const Profile  _Profile[]=
{
    {"CIM_ComputerSystem",&_ComputerSystem,-1,SYSTEM_INFO},
    {"CIM_RegisteredProfile",&_RegisteredProfile,-1},
    {"CIM_EnabledLogicalElementCapabilities",&_EnabledLogicalElementCapabilities,-1},
    {"CIM_PhysicalPackage",&_PhysicalPackage,PhysicalAsset_pri},
    {"CIM_Processor",&_Processor,Processor_pri,PROCESSOR},
    {"CIM_PhysicalMemory",&_PhysicalMemory,PhysicalAsset_pri,MEM_DEV},
    //  {"CIM_PhysicalMemory",&_PhysicalMemory,3},
    {"CIM_Chassis",&_Chassis,PhysicalAsset_pri,CHASSIS},
    {"CIM_AssociatedCacheMemory",&_AssociatedCacheMemory,Processor_pri,-1},
    {"CIM_BootService",&_BootService,Boot_pri},
    {"CIM_BootConfigSetting",&_BootConfigSetting,Boot_pri},
    {"CIM_BootSourceSetting",&_BootSourceSetting,Boot_pri},
    {"CIM_BootServiceCapabilities",&_BootServiceCapabilities,Boot_pri},
    {"CIM_PowerManagementService",&_PowerManagementService,PowerManagement_pri},
    {"CIM_PowerManagementCapabilities",&_PowerManagementCapabilities,PowerManagement_pri},
    {"CIM_HostedService",&_HostedService,-1,-1},
    {"CIM_ElementConformsToProfile",&_ElementConformsToProfile,-1,-1},
    {"CIM_ReferencedProfile",&_ReferencedProfile,-1,-1},
    {"CIM_ElementSettingData",&_ElementSettingData,-1,-1},
    {"CIM_OrderedComponent",&_OrderedComponent,Boot_pri,-1},
    {"CIM_ServiceAffectsElement",&_ServiceAffectsElement,-1,-1},
    {"CIM_SystemDevice",&_SystemDevice,-1,-1},
    {"CIM_ComputerSystemPackage",&_ComputerSystemPackage,ComputerSystem_pri,-1},
    {"CIM_ElementCapabilities",&_ElementCapabilities,-1,-1},
    {"CIM_AssociatedPowerManagementService",&_AssociatedPowerManagementService,PowerManagement_pri,-1},
    {"CIM_SoftwareIdentity",&_SoftwareIdentity,Software_pri},
    {"CIM_Sensor",&_NumericSensor,Sensor_pri},
    {"CIM_NumericSensor",&_NumericSensor,Sensor_pri},
    {"CIM_Fan",&_Fan,Fan_pri},
    {"CIM_AssociatedSensor",&_AssociatedSensor,-1,-1},
    {"CIM_PowerSupply",&_PowerSupply,PowerSupply_pri,39},
    {"CIM_Memory",&_Memory,Memory_pri,MEM_DEV},
    {"CIM_PhysicalElement",&_PhysicalElement,PhysicalAsset_pri},
    {"CIM_PhysicalComponent",&_PhysicalComponent,PhysicalAsset_pri},
    {"CIM_PhysicalConnector",&_PhysicalConnector,PhysicalAsset_pri},
    {"CIM_PhysicalFrame",&_PhysicalFrame,PhysicalAsset_pri},
    {"CIM_AccountManagementService",&_AccountManagementService,Account_pri},
    {"CIM_AccountManagementCapabilities",&_AccountManagementCapabilities,Account_pri},
    {"CIM_RoleBasedAuthorizationService",&_RoleBasedAuthorizationService,Role_pri},
    {"CIM_RoleBasedManagementCapabilities",&_RoleBasedManagementCapabilities,Role_pri},
    {"CIM_Role",&_Role,R_Role_pri},
    {"CIM_OwningCollectionElement",&_OwningCollectionElement,-1,-1},
    {"CIM_RoleLimitedToTarget",&_RoleLimitedToTarget,-1,-1},
    {"CIM_FilterCollection",&_FilterCollection,-1},
    {"CIM_IndicationServiceSettingData",&_IndicationServiceSettingData,Indication_pri},
    {"CIM_IndicationService",&_IndicationService,Indication_pri},
    {"CIM_ListenerDestinationWSManagement",&_ListenerDestinationWSManagement,Indication_pri},
    {"CIM_IndicationFilter",&_IndicationFilter,Indication_pri},
    {"CIM_IndicationSubscription",&_IndicationSubscription,Indication_pri,-1},
  //  {"CIM_AbstractIndicationSubscription",&_IndicationSubscription,Indication_pri,-1},
    {"CIM_AbstractIndicationSubscription",&_FilterCollectionSubscription,Indication_pri,-1},
    {"CIM_FilterCollectionSubscription",&_FilterCollectionSubscription,Indication_pri,-1},
    {"CIM_PhysicalAssetCapabilities",&_PhysicalAssetCapabilities,PhysicalAsset_pri},
    {"CIM_SystemPackaging",&_SystemPackaging,-1,-1},
    {"CIM_Realizes",&_Realizes,-1,-1},
    {"CIM_Container",&_Container,-1,-1},
    {"CIM_MemberOfCollection",&_MemberOfCollection,-1,-1},
    {"CIM_Account",&_Account,R_Account_pri},
    {"CIM_Privilege",&_Privilege,R_Account_pri},
    {"CIM_Identity",&_Identity,R_Account_pri},
    {"CIM_AssignedIdentity",&_AssignedIdentity,Account_pri,-1},
    {"CIM_UserContact",&_UserContact,Account_pri},
    {"CIM_IPProtocolEndpoint",&_IPProtocolEndpoint,IP_pri},
    {"CIM_StaticIPAssignmentSettingData",&_StaticIPAssignmentSettingData,IP_pri},
    {"CIM_TextRedirectionService",&_TextRedirectionService,Text_pri},
    {"CIM_TextRedirectionSAP",&_TextRedirectionSAP,Text_pri},
    {"CIM_RecordLog",&_RecordLog,-1},
    {"CIM_LogEntry",&_LogEntry,-1},
    {"CIM_LogManagesRecord",&_LogManagesRecord,-1,-1},
    {"CIM_BIOSService",&_BIOSService,BIOS_pri,99},
    {"CIM_BIOSElement",&_BIOSElement,BIOS_pri,99},
    {"CIM_BIOSAttribute",&_BIOSAttribute,BIOS_pri,99},
    {"CIM_BIOSServiceCapabilities",&_BIOSServiceCapabilities,BIOS_pri,99},
    {"CIM_USBRedirectionSAP",&_USBRedirectionSAP,USB_pri},
    {"CIM_OperatingSystem",&_OperatingSystem,OperatingSystem_pri},
    {"CIM_DNSProtocolEndpoint",&_DNSProtocolEndpoint,-1},
    {"CIM_Slot",&_Slot,PhysicalAsset_pri,SLOTS},
    {"CIM_Chip",&_Chip,PhysicalAsset_pri,PROCESSOR},
    {"CIM_Card",&_Card,PhysicalAsset_pri,BASE_BOARD_INFO},
    {"CIM_AssociatedCooling",&_AssociatedCooling,-1,-1},
    // {"CIM_TCRTCPProtocolEndpoint",&_TCRTCPProtocolEndpoint,3},
    {"CIM_USBTCPProtocolEndpoint",&_USBTCPProtocolEndpoint,USB_pri},
    {"CIM_DHCPProtocolEndpoint",&_DHCPProtocolEndpoint,DHCP_pri},
    {"CIM_ServiceAccessBySAP",&_ServiceAccessBySAP,-1,-1},
    {"CIM_HostedAccessPoint",&_HostedAccessPoint,-1,-1},
    {"CIM_USBRedirectionService",&_USBRedirectionService,USB_pri},
    {"CIM_USBDevice",&_USBDevice,USB_pri},
    {"CIM_USBRedirectionCapabilities",&_USBRedirectionCapabilities,USB_pri},
    {"CIM_DHCPCapabilities",&_DHCPCapabilities,DHCP_pri},
    {"CIM_SAPSAPDependency",&_SAPSAPDependency,-1,-1},
    {"CIM_ConcreteDependency",&_ConcreteDependency,-1,-1},
    {"CIM_SystemBIOS",&_SystemBIOS,BIOS_pri,-1},
    {"CIM_ConcreteComponent",&_ConcreteComponent,-1,-1},
    {"CIM_BIOSPassword",&_BIOSPassword,BIOS_pri},
    {"CIM_SoftwareInstallationService",&_SoftwareInstallationService,Software_pri},
    {"CIM_BindsTo",&_BindsTo,-1,-1},
    {"CIM_TCPProtocolEndpoint",&_TCPProtocolEndpoint,IP_pri | Text_pri| USB_pri},
    {"CIM_ProtocolEndpoint",&_TCPProtocolEndpoint,IP_pri | Text_pri| USB_pri},
    {"CIM_RemoteAccessAvailableToElement",&_RemoteAccessAvailableToElement,-1,-1},
    {"CIM_RemoteServiceAccessPoint",&_RemoteServiceAccessPoint,-1},
    {"CIM_OpaqueManagementData",&_OpaqueManagementData,Opaque_pri},
    {"CIM_OpaqueManagementDataService",&_OpaqueManagementDataService,Opaque_pri},
    {"CIM_ManagedSystemElement",&_ManagedSystemElement,ComputerSystem_pri},
//    {"CIM_NetworkPort",&_NetworkPort,ComputerSystem_pri},
    {0,0}
};

//CONFIG_CIM_IndicationService
//CONFIG_CIM_ComputerSystem
//CONFIG_CIM_PowerManagementService
//CONFIG_CIM_TextRedirectionService
//CONFIG_CIM_BIOSElement
//CONFIG_CIM_PhysicalElement
//CONFIG_CIM_IPProtocolEndpoint
//CONFIG_CIM_OperatingSystem
//CONFIG_CIM_Memory
//CONFIG_CIM_RecordLog
//CONFIG_CIM_SoftwareIdentity
//CONFIG_CIM_AccountManagementService
//CONFIG_CIM_DHCPProtocolEndpoint
//CONFIG_CIM_RoleBasedAuthorizationService
//CONFIG_CIM_Processor   

#if 1
const RegProfile _RegProfile[]=
{

#if 0
    //{"CIM_OpaqueManagementDataService","Opaque Management Data",Opaque_pri},
    #ifdef CONFIG_CIM_ComputerSystem
    {"CIM_ComputerSystem","Base Desktop And Mobile","BaseDesktopAndMobile","1.0.2",ComputerSystem_pri},
    #else
    {"CIM_ComputerSystem","Base Desktop And Mobile","BaseDesktopAndMobile","1.0.2",0},	
    #endif

     #ifdef CONFIG_CIM_Processor
    {"CIM_Processor","CPU","CPU","1.0.1",Processor_pri},
    #else
    {"CIM_Processor","CPU","CPU","1.0.1",0},	
    #endif	

    #ifdef	 CONFIG_CIM_BootService
    {"CIM_BootService","Boot Control","BootControl","1.0.2",Boot_pri},
    #else
    {"CIM_BootService","Boot Control","BootControl","1.0.2",0},
    #endif

    #ifdef CONFIG_CIM_PowerManagementService	
    {"CIM_PowerManagementService","Power State Management","PowerStateManagement","1.0.2",PowerManagement_pri},
    #else
	{"CIM_PowerManagementService","Power State Management","PowerStateManagement","1.0.2",0},
    #endif

    #ifdef	 CONFIG_CIM_IndicationService
    {"CIM_IndicationService","Indications","Indications","1.2.1",Indication_pri},
    #else
    {"CIM_IndicationService","Indications","Indications","1.2.1",0},	
    #endif	

    #ifdef	CONFIG_CIM_Memory
    {"CIM_Memory","System Memory","SystemMemory","1.0.1",Memory_pri },
    #else
    {"CIM_Memory","System Memory","SystemMemory","1.0.1",0 },	
    #endif

    #ifdef CONFIG_CIM_SoftwareIdentity	
    {"CIM_SoftwareIdentity","Software Inventory","SoftwareInventory","1.0.1",Software_pri},
    #else
    {"CIM_SoftwareIdentity","Software Inventory","SoftwareInventory","1.0.1",0},		
    #endif

    #ifdef CONFIG_CIM_NumericSensor
    {"CIM_NumericSensor","Sensors","Sensors","1.0.2",Sensor_pri},
    #else
    {"CIM_NumericSensor","Sensors","Sensors","1.0.2",0},
    #endif

    #ifdef CONFIG_CIM_Fan	
    {"CIM_Fan","Fan","Fan","1.0.1",Fan_pri},
    #else
    {"CIM_Fan","Fan","Fan","1.0.1",0},
    #endif

    #ifdef CONFIG_CIM_PowerSupply
    {"CIM_PowerSupply","Power Supply","PowerSupply","1.1.0",PowerSupply_pri},
    #else
    {"CIM_PowerSupply","Power Supply","PowerSupply","1.1.0",0},
    #endif

    #ifdef CONFIG_CIM_PhysicalElement
    {"CIM_PhysicalElement","Physical Asset","PhysicalAsset","1.0.2",PhysicalAsset_pri},
    #else
    #endif

    #ifdef CONFIG_CIM_AccountManagementService    
    {"CIM_AccountManagementService","Simple Identity Management","SimpleIdentityManagement","1.1.0",Account_pri},
    #else
    #endif

    #ifdef CONFIG_CIM_RoleBasedAuthorizationService
    {"CIM_RoleBasedAuthorizationService","Role Based Authorization","RoleBasedAuthorization","1.0.0",Role_pri},
    #else
    #endif

    #ifdef CONFIG_CIM_TextRedirectionService
    {"CIM_TextRedirectionService","Text Console Redirection","TextConsoleRedirection","1.0.2",Text_pri},
    #else
    #endif

    #ifdef CONFIG_CIM_USBRedirectionService
    {"CIM_USBRedirectionService","USB Redirection","USBRedirection","1.0.0",USB_pri},
    #else
    {"CIM_USBRedirectionService","USB Redirection","USBRedirection","1.0.0",0},
    #endif

    #ifdef CONFIG_CIM_DHCPProtocolEndpoint
    {"CIM_DHCPProtocolEndpoint","DHCP Client","DHCPClient","1.0.3",DHCP_pri},
    #else
    {"CIM_DHCPProtocolEndpoint","DHCP Client","DHCPClient","1.0.3",0},	
    #endif
	
    #ifdef CONFIG_CIM_IPProtocolEndpoint
    {"CIM_IPProtocolEndpoint","IP Interface","IPInterface","1.1.0",IP_pri},
    #else
    {"CIM_IPProtocolEndpoint","IP Interface","IPInterface","1.1.0",0},	
    #endif
	
    #ifdef CONFIG_CIM_BIOSElement
    {"CIM_BIOSElement","BIOS Management","BIOSManagement","1.0.1",BIOS_pri},
    #else
	{"CIM_BIOSElement","BIOS Management","BIOSManagement","1.0.1",0},
    #endif
	
    #ifdef CONFIG_CIM_OperatingSystem
    {"CIM_OperatingSystem","Operating System","OperatingSystem","1.0.0",OperatingSystem_pri},
    #else
	{"CIM_OperatingSystem","Operating System","OperatingSystem","1.0.0",0},
    #endif
	
    #ifdef CONFIG_CIM_RecordLog
    {"CIM_RecordLog", "Record Log","RecordLog","1.0.1",Record_pri},
    #else
	{"CIM_RecordLog", "Record Log","RecordLog","1.0.1",0},
    #endif
	
    #ifdef CONFIG_CIM_OpaqueManagementDataService
    {"CIM_OpaqueManagementDataService","Opaque Management Data","OpaqueManagementData","1.0.1",Opaque_pri},
    #else
    {"CIM_OpaqueManagementDataService","Opaque Management Data","OpaqueManagementData","1.0.1",0},    
    #endif    
    {0,0}
#else	
   {"CIM_ComputerSystem","Base Desktop And Mobile","BaseDesktopAndMobile","1.0.2",ComputerSystem_pri},	
   {"CIM_Processor","CPU","CPU","1.0.1",Processor_pri},  
   {"CIM_BootService","Boot Control","BootControl","1.0.2",Boot_pri},   
   {"CIM_PowerManagementService","Power State Management","PowerStateManagement","1.0.2",PowerManagement_pri},  
   {"CIM_IndicationService","Indications","Indications","1.2.1",Indication_pri},   
   {"CIM_Memory","System Memory","SystemMemory","1.0.1",Memory_pri },   
   {"CIM_SoftwareIdentity","Software Inventory","SoftwareInventory","1.0.1",Software_pri},  
   {"CIM_NumericSensor","Sensors","Sensors","1.0.2",Sensor_pri},  
   {"CIM_Fan","Fan","Fan","1.0.1",Fan_pri},  
   {"CIM_PowerSupply","Power Supply","PowerSupply","1.1.0",PowerSupply_pri}, 
   {"CIM_PhysicalElement","Physical Asset","PhysicalAsset","1.0.2",PhysicalAsset_pri},  
   {"CIM_AccountManagementService","Simple Identity Management","SimpleIdentityManagement","1.1.0",Account_pri},   
   {"CIM_RoleBasedAuthorizationService","Role Based Authorization","RoleBasedAuthorization","1.0.0",Role_pri}, 
   {"CIM_TextRedirectionService","Text Console Redirection","TextConsoleRedirection","1.0.2",Text_pri}, 
   {"CIM_USBRedirectionService","USB Redirection","USBRedirection","1.0.0",USB_pri},   
   {"CIM_DHCPProtocolEndpoint","DHCP Client","DHCPClient","1.0.3",DHCP_pri},
   {"CIM_IPProtocolEndpoint","IP Interface","IPInterface","1.1.0",IP_pri},
   {"CIM_BIOSElement","BIOS Management","BIOSManagement","1.0.1",BIOS_pri},   
   {"CIM_OperatingSystem","Operating System","OperatingSystem","1.0.0",OperatingSystem_pri},   
   {"CIM_RecordLog", "Record Log","RecordLog","1.0.1",Record_pri},   
   {"CIM_OpaqueManagementDataService","Opaque Management Data","OpaqueManagementData","1.0.1",Opaque_pri}, 
   {0,0}
#endif	
};

#else

const RegProfile _RegProfile[]=
{
    //{"CIM_OpaqueManagementDataService","Opaque Management Data",Opaque_pri},
    {"CIM_ComputerSystem","Base Desktop And Mobile","BaseDesktopAndMobile","1.0.2",ComputerSystem_pri},
    {"CIM_Processor","CPU","CPU","1.0.1",Processor_pri},
    {"CIM_BootService","Boot Control","BootControl","1.0.2",Boot_pri},
    {"CIM_PowerManagementService","Power State Management","PowerStateManagement","1.0.2",PowerManagement_pri},
    {"CIM_IndicationService","Indications","Indications","1.2.1",Indication_pri},
    {"CIM_Memory","System Memory","SystemMemory","1.0.1",Memory_pri },
    {"CIM_SoftwareIdentity","Software Inventory","SoftwareInventory","1.0.1",Software_pri},
    {"CIM_NumericSensor","Sensors","Sensors","1.0.2",Sensor_pri},
    {"CIM_Fan","Fan","Fan","1.0.1",Fan_pri},
    {"CIM_PowerSupply","Power Supply","PowerSupply","1.1.0",PowerSupply_pri},
    {"CIM_PhysicalElement","Physical Asset","PhysicalAsset","1.0.2",PhysicalAsset_pri},
    {"CIM_AccountManagementService","Simple Identity Management","SimpleIdentityManagement","1.1.0",Account_pri},
    {"CIM_RoleBasedAuthorizationService","Role Based Authorization","RoleBasedAuthorization","1.0.0",Role_pri},
    {"CIM_TextRedirectionService","Text Console Redirection","TextConsoleRedirection","1.0.2",Text_pri},
    {"CIM_USBRedirectionService","USB Redirection","USBRedirection","1.0.0",USB_pri},
    {"CIM_DHCPProtocolEndpoint","DHCP Client","DHCPClient","1.0.3",DHCP_pri},
    {"CIM_IPProtocolEndpoint","IP Interface","IPInterface","1.1.0",IP_pri},
    {"CIM_BIOSElement","BIOS Management","BIOSManagement","1.0.1",BIOS_pri},
    {"CIM_OperatingSystem","Operating System","OperatingSystem","1.0.0",OperatingSystem_pri},
    {"CIM_RecordLog", "Record Log","RecordLog","1.0.1",Record_pri},
    {"CIM_OpaqueManagementDataService","Opaque Management Data","OpaqueManagementData","1.0.1",Opaque_pri},
    {0,0}
};
#endif
#if 1
#define compSys_Dedicated_value_MAX 39
unsigned char *compSys_Dedicated_value[]={//computerSys_Dedicated_value
	"Not Dedicated","Unknown","Other","Storage","Router","Switch","Layer 3 Switch","Central Office Switch","Hub","Access Server",
		"Firewall","Print","I/O","Web Caching","Management","Block Server","File Server","Mobile User Device","Repeater","Bridge/Extender",
		"Gateway","Storage Virtualizer","Media Library",	"ExtenderNode","NAS Head","Self-contained NAS","UPS","IP Phone","Management Controller","Chassis Manager",
		"Host-based RAID controller","Storage Device Enclosure","Desktop","Laptop",	"Virtual Tape Library","Virtual Library System","Network PC/Thin Client","FC Switch","Ethernet Switch",
		//"DMTF Reserved"//,"Vendor Reserved"
};
#define sys_enState_value_MAX 11
unsigned char *sys_enState_value[]={//computerSys_Dedicated_value
	"Unknown","Other","Enabled","Disabled","Shutting Down",
		"Not Applicable","Enabled but Offline","In Test"," Deferred","Quiesce","Starting"//,"DMTF Reserved","Vendor Reserved"
};

#define sys_reqState_value_MAX 13
unsigned char *sys_reqState_value[]={//computerSys_Dedicated_value
	"Unknown","","Enabled","Disabled","Shut Down",
	"No Change","Offline","Test","Deferred","Quiesce",
	"Reboot","Reset","Not Applicable"//,"DMTF Reserved"//,"Vendor Reserved"
};

#define power_state_value_MAX 18
unsigned char *power_state_value[]={//computerSys_Dedicated_value
	"","Other","On","Sleep - Light","Sleep -Deep","Power Cycle (Off - Soft)",
		"Off - Hard","Hibernate (Off - Soft)","Off - Soft","Power Cycle (Off-Hard)","Master Bus Reset",
		"Diagnostic Interrupt (NMI)","Off - Soft Graceful","Off - Hard Graceful","Master Bus Reset Graceful","Power Cycle (Off - Soft Graceful)",
		"Power Cycle (Off - Hard Graceful)","Diagnostic Interrupt (INIT)"//,"DMTF Reserved"//,"Vendor Specific"
};
#define powMangCapProp_powChg_MAX 9
unsigned char *powMangCapProp_powChg[] = {
	"Unknown","Other","Power Saving Modes Entered Automatically","Power State Settable",
	"Power Cycling Supported","Timed Power On Supported","Off Hard Power Cycling Supported",
	"HW Reset Supported","Graceful Shutdown Supported"
};

#define softidProp_class_MAX 14
unsigned char *softidProp_class[] = {
	"Unknown", "Other", "Driver", "Configuration Software", "Application Software",
	"Instrumentation", "Firmware/BIOS", "Diagnostic Software", "Operating System", "Middleware",
	"Firmware", "BIOS/FCode", "Support/Service Pack", "Software Bundle" //,DMTF Reserved, Vendor Reserved
};

#define SoftwareElementState_MAX 4
unsigned char *SoftwareElementState_value[] = {
	"Deployable", "Installable", "Executable", "Running" //,DMTF Reserved, Vendor Reserved
};	

#define OperationalStatus_MAX 20
unsigned char *OperationalStatus_value[] = {
	"Unknown","Other", "OK", "Degraded", "Stressed",
	"Predictive Failure", "Error", "Non-Recoverable Error", "Starting", "Stopping",
	"Stopped", "In Service", "No Contact", "Lost Communication", "Aborted",
	"Dormant", "Supporting Entity in Error", "Completed", "Power Mode", "Relocating"// DMTF Reserved, Vendor Reserved
};

#define RequestedState_MAX 13
unsigned char *RequestedState_value[] = {
	"Unknown","NA","Enabled","Disabled", "Shut Down",
	"No Change","Offline","Test","Deferred","Quiesce",
	"Reboot","Reset","Not Applicable"//, DMTF Reserved, Vendor Reserved
};

#define MemoryType_MAX 26
unsigned char *MemoryType_value[] = {
	"UNKNOWN","Other","DRAM","Synchronous DRAM","Cache DRAM",
	"EDO","EDRAM","VRAM","SRAM","RAM",
	"ROM","Flash","EEPROM","FEPROM","EPROM",
	"CDRAM","3DRAM","SDRAM","SGRAM","RDRAM",
	"DDR","DDR-2","BRAM","FB-DIMM","DDR3",
	"FBD2"//,"DMTF Reserved","Vendor Reserved
};

#else

#define compSys_Dedicated_value_MAX 0
unsigned char *compSys_Dedicated_value[]={
};
#define sys_enState_value_MAX 0
unsigned char *sys_enState_value[]={
};

#define sys_reqState_value_MAX 0
unsigned char *sys_reqState_value[]={
};

#define power_state_value_MAX 0
unsigned char *power_state_value[]={//computerSys_Dedicated_value	
};

#endif	

#if 1
plist *DefaultInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata);
plist *DefaultInbandInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata);
plist *DefaultBiosInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata);
unsigned char *NewMem();
plist* ReleaseProfile(plist *ptr);	
plist *DefaultSensorInstance(DASHCB *dcb,XMLElt *p, INT8U *pdata);
void UpdateComputerSystemCIM(){


	strcpy(power_state, pwrtbl[bsp_get_sstate()]);
        if (strcmp(power_state, "2"))
            strcpy(enabled_state, "3");
        else
            strcpy(enabled_state, "2");
		
}
//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11..32767, 32768..65535
//Unknown, Other, Enabled, Disabled, Shutting Down, Not Applicable, Enabled but Offline, In Test, Deferred, Quiesce, Starting, DMTF Reserved, Vendor Reserved
unsigned char *getEnableState(int idx){
	if(idx < 11 ){
		switch(idx){
			case 0:
				return "Unknown";
			break;
			case 1:
				return "Other";
			break;
			case 2:
				return "Enabled";
			break;
			case 3:
				return "Disabled";
			break;
			case 4:
				return "Shutting Down";
			break;
			case 5:
				return "Not Applicable";
			break;
			case 6:
				return "Enabled but Offline";
			break;
			case 7:
				return "In Test";
			break;
			case 8:
				return "Deferred";
			break;
			case 9:
				return "Quiesce";
			break;
			case 10:
				return "Starting";
			break;
			default:
				return "NA";
			break;	
			
		}
	}else if(idx <=32767 ){
		return "DMTF Reserved";
	}else{
		return "Vendor Reserved";
	}
}

unsigned char *getRequestedState(unsigned char *data){
	int idx = atoi(data);
	
	if(idx < RequestedState_MAX ){
		return RequestedState_value[idx];
	}else if(idx < 32768){
		return "DMTF Reserved";
	}else{
		return "Vendor Reserved";
	}
}



unsigned char *getOperationalStatus(unsigned char *data){
	int idx =0;
	idx = atoi(data);
	if((idx < OperationalStatus_MAX)&&(idx >=0)){
		return OperationalStatus_value[idx];			
	}else if(idx< 32767){
		return "DMTF Reserved";
	}else{
		return "Vendor Reserved";
	}
}



unsigned char *getMemoryType(unsigned char *data){	
	
	int idx =0;
	idx = atoi(data);
	if((idx < MemoryType_MAX)&&(idx >=0)){
		return MemoryType_value[idx];			
	}else if(idx< 32767){
		return "DMTF Reserved";
	}else{
		return "Vendor Reserved";
	}
}

unsigned char *getAccess(unsigned char *data){
	int idx = atoi(data);
	
	switch(idx){
		case 0:
			return "Unknown";
		break;
		case 1:
			return "Readable";
		break;
		case 2:
			return "Writeable";
		break;
		case 3:
			return "Read/Write Supported";
		break;
		case 4:
			return "Write Once";
		break;
		default:
			return "NA";
		break;
	}
}


unsigned char *getHealthState(unsigned char *data){

	int idx = atoi(data);
	
	if(idx < 11 ){
		switch(idx){
			case 0:
				return "Unknown";
			break;
			case 5:
				return "OK";
			break;
			case 10:
				return "Degraded/Warning";
			break;
			case 15:
				return "Minor failure";
			break;	
			case 20:
				return "Major failure";
			break;
			case 25:
				return "Critical failure";
			break;	
			case 30:
				return "Non-recoverable error";
			break;
			
			default:
				return data;
		}
	}else if(idx <=32767 ){
		return "DMTF Reserved";
	}else{
		return "Vendor Reserved";
	}
	
}

void UpdateOperationSysCIM(){//this will be called by CIM_OperationSystem from htm.c
  
	if(m_OperationSysCIM != NULL){
		ReleaseProfile(m_OperationSysCIM);
		m_OperationSysCIM = NULL;
	}
	
	m_OperationSysCIM = DefaultInbandInstance(NULL,_OperatingSystem,NewMem());
}

int getSwIdInstanceCount(){
	
	plist *curr=NULL;	
	int idx = 0;
	curr = m_softId;

	for(idx=0;idx<25;idx++){
		m_swIdInstance[idx] = NULL;
	}
	idx = 0;
	while((curr) && (curr->eltname)&&(idx<25)){
		if(strcmp( curr->eltname ,_SoftwareIdentity[0].eltname) == 0){	
			m_swIdInstance[idx++] =  curr;
		}
		curr = curr->next;
	}
	//printf("[RTK]There are %d instance\n",idx);	
	return idx;
}


int getbiosAttrInstanceCount(){
	
	plist *curr=NULL;	
	int idx = 0;
	
	

	
	for(idx=0;idx<10;idx++){
		m_biosAttrInstance[idx] = NULL;
	}	

	idx = 0;
	curr = m_biosAttr;
	while((curr) && (curr->eltname)&&(idx<10)){
		if(strcmp( curr->eltname ,_BIOSAttribute[0].eltname) == 0){	
			m_biosAttrInstance[idx++] =  curr;
		}
		curr = curr->next;
	}
	//printf("[RTK]There are %d element instance\n",idx);	
	return idx;
}

int getMemInstanceCount(){
	
	plist *curr=NULL;	
	int idx = 0;
	
	

	
	for(idx=0;idx<5;idx++){
		m_memInstance[idx] = NULL;
	}	

	idx = 0;
	curr = m_memoryCIM;
	while((curr) && (curr->eltname)&&(idx<5)){
		if(strcmp( curr->eltname ,_Memory[0].eltname) == 0){	
			m_memInstance[idx++] =  curr;
		}
		curr = curr->next;
	}
	//printf("[RTK]There are %d element instance\n",idx);	
	//+briank.rtk: just for debug-----------------------------	
	/*
	curr = m_memInstance[0] ;
	do{
		printf("[RTK]%s:(%s)\n",curr->eltname,curr->data);	
		curr = curr->next;
	}while( strcmp(curr->eltname,_Memory[0].eltname) != 0 );
	*/
	//+----------------------------------------------------
	
	return idx;
}

int getProcessorInstanceCount(){
	
	plist *curr=NULL;	
	int idx = 0;
	
	for(idx=0;idx<5;idx++){
		m_processorInstance[idx] = NULL;
	}	

	idx = 0;
	curr = m_processorCIM;
	while((curr) && (curr->eltname)&&(idx<5)){
		if(strcmp( curr->eltname ,_Processor[0].eltname) == 0){	
			m_processorInstance[idx++] =  curr;
		}
		curr = curr->next;
	}
	//printf("[RTK]There are %d element instance\n",idx);	
	return idx;
}


int getbiosEleInstanceCount(){
	
	plist *curr=NULL;	
	int idx = 0;
	
	

	for(idx=0;idx<5;idx++){
		m_biosEleInstance[idx] = NULL;
	}
	
	
	idx = 0;
	curr = m_biosElement;
	while((curr) && (curr->eltname)){
		if(strcmp( curr->eltname ,_BIOSElement[0].eltname) == 0){	
			m_biosEleInstance[idx++] =  curr;
		}
		curr = curr->next;
	}
	//printf("[RTK]There are %d element instance\n",idx);		
	return idx;
}


int getFanInstanceCount(){
	
	plist *curr=NULL;	
	int idx = 0;
	
	

	for(idx=0;idx<5;idx++){
		m_fanInstance[idx] = NULL;
	}
	
	
	idx = 0;
	curr = m_fanCIM;
	while((curr) && (curr->eltname)){
		if(strcmp( curr->eltname ,_Fan[0].eltname) == 0){	
			m_fanInstance[idx++] =  curr;
		}
		curr = curr->next;
	}
	//printf("[RTK]There are %d element instance\n",idx);
	return idx;
}


int getSensorInstanceCount(){
	
	plist *curr=NULL;	
	int idx = 0;
	
	

	for(idx=0;idx<5;idx++){
		m_sensorInstance[idx] = NULL;
	}
	
	
	idx = 0;
	curr = m_SensorCIM;
	while((curr) && (curr->eltname)){
		if(strcmp( curr->eltname ,_NumericSensor[0].eltname) == 0){	
			m_sensorInstance[idx++] =  curr;
		}
		curr = curr->next;
	}
	//printf("[RTK]There are %d element instance\n",idx);	
	return idx;
}


int getCommonInstanceCount(char *cim_name){
	plist **instanceList = NULL;
	plist *pCim = NULL;
	int max_count = 0;
	XMLElt *_xmlEltData;
	int idx = 0;
	plist *curr;
	
	if((strcmp(cim_name,"CIM_Card") == 0 )&&(strlen(cim_name)==strlen("CIM_Card"))){
		instanceList = m_CardInstance;
		pCim  = m_CardCIM;
		max_count = 5;
		_xmlEltData = _Card;
	}else if((strcmp(cim_name,"CIM_Chassis") == 0 )&&(strlen(cim_name)==strlen("CIM_Chassis"))){
		instanceList = m_ChassisInstance;
		pCim  = m_ChassisCIM;
		max_count = 5;
		_xmlEltData = _Chassis;
	}else if((strcmp(cim_name,"CIM_Chip") == 0 )&&(strlen(cim_name)==strlen("CIM_Chip"))){
		instanceList = m_ChipInstance;
		pCim  = m_ChipCIM;
		max_count = 5;
		_xmlEltData = _Chip;
	}else if((strcmp(cim_name,"CIM_PhysicalMemory") == 0 )&&(strlen(cim_name)==strlen("CIM_PhysicalMemory"))){
		instanceList = m_PhyMemInstance;
		pCim  = m_PhyMemCIM;
		max_count = 5;
		_xmlEltData = _PhysicalMemory;
	}else if((strcmp(cim_name,"CIM_Slot") == 0 )&&(strlen(cim_name)==strlen("CIM_Slot"))){
		instanceList = m_SlotInstance;
		pCim  = m_SlotCIM;
		max_count = 10;
		_xmlEltData = _Slot;
	}
	else{
		return 0;
	}


	for(idx=0;idx<5;idx++){
		instanceList[idx] = NULL;
	}
	
	
	idx = 0;
	curr = pCim;
	while((curr) && (curr->eltname)){
		if(strcmp( curr->eltname ,_xmlEltData[0].eltname) == 0){	
			instanceList[idx++] =  curr;
		}
		curr = curr->next;
	}
	//printf("[RTK]There are %d (%s) instance\n",idx,cim_name);		
	return idx;
}
void UpdateSoftwareIdCIM(){

	if(m_softId!=NULL){
		ReleaseProfile(m_softId);
	}

	m_softId = DefaultInbandInstance(NULL,_SoftwareIdentity,NewMem()); 
	 // Must call ReleaseProfile later , for free the memory space from NewMem
}

void releaseSoftwareIdCIM(){
	int idx;
	
	if(m_softId != NULL){
		ReleaseProfile(m_softId);
		m_softId = NULL;
	}
	for(idx=0;idx<25;idx++){
		m_swIdInstance[idx] = NULL;
	}
	ReleaseBuf();


	//printFreeMem(2);
}
void UpdateProcessorCIM(){
	plist *curr=NULL;	
	
	if(m_processorCIM != NULL){
		ReleaseProfile(m_processorCIM);
		m_processorCIM = NULL;
	}
	m_dashPtype = 4;
	m_processorCIM = DefaultInstance(NULL,_Processor,NewMem()); 

	//printout for debug--------------------------
	/*
	curr = m_processorCIM;
	 while(curr){
	 	if(curr->eltname){
				printf("[RTK](processor)[%s:%s]\n",curr->eltname,curr->data);
	 	}
		curr = curr->next ;
	 }*/
	 //-------------------------------------------
	 
}

void UpdateMemoryCIM(){
	plist *curr=NULL;	
	
	if(m_memoryCIM != NULL){
		ReleaseProfile(m_memoryCIM);
		m_memoryCIM = NULL;
	}
	m_dashPtype = MEM_DEV;
	m_memoryCIM = DefaultInstance(NULL,_Memory,NewMem()); 

	//printout for debug--------------------------
	/*
	curr = m_memoryCIM;
	 while(curr){
	 	if(curr->eltname){
				printf("[RTK](mem)[%s:%s]\n",curr->eltname,curr->data);
	 	}
		curr = curr->next ;
	 }
	 */
	 //-------------------------------------------
	 
}
void UpdateFanCIM(){
	
	plist *curr=NULL;	
	
	if(m_fanCIM != NULL){
		ReleaseProfile(m_fanCIM);
		m_fanCIM = NULL;
	}
	m_dashPtype = MEM_DEV;

	m_fanCIM = DefaultSensorInstance(NULL,_Fan,NewMem());
	//printout for debug--------------------------
	/*
	if(m_fanCIM){
		curr = m_fanCIM;
		 while(curr){
		 	if(curr->eltname){
					printf("[RTK](fan)[%s:%s]\n",curr->eltname,curr->data);
		 	}
			curr = curr->next ;
		 }
	}else{
		printf("[RTK] m_fanCIM is null\n");
	}*/
	 //-------------------------------------------
	 
}

void UpdateSensorCIM(){
	
	plist *curr=NULL;	
	
	if(m_SensorCIM != NULL){
		ReleaseProfile(m_SensorCIM);
		m_SensorCIM = NULL;
	}
	//m_dashPtype = MEM_DEV;	
	m_SensorCIM = DefaultSensorInstance(NULL,_NumericSensor,NewMem());

	//printout for debug--------------------------
	/*
	if(m_SensorCIM){
		
		curr = m_SensorCIM;
		 while(curr){
	 	if(curr->eltname){
				printf("[RTK](Sensor)[%s:%s]\n",curr->eltname,curr->data);
	 	}
		curr = curr->next ;
		 } 
	}else{
		printf("[RTK] m_sensorCIM is null\n");
	}
	*/	
	 //-------------------------------------------
	 
}

int UpdateCommonCIM(char *cim_name){	
	plist *pCim = NULL;
	int max_count = 0;
	XMLElt *_xmlEltData;
	int idx = 0;
	plist *curr;
	int count;

//printf("[RTK] + UpdateCommonCIM\n");		
	if((strcmp(cim_name,"CIM_Card") == 0 )&&(strlen(cim_name)==strlen("CIM_Card"))){
		m_dashPtype = 0x02;
		if(m_CardCIM != NULL){
			ReleaseProfile(m_CardCIM);
			m_CardCIM = NULL;
		}
		max_count = 5;
		_xmlEltData = _Card;
	}else if((strcmp(cim_name,"CIM_Chassis") == 0 )&&(strlen(cim_name)==strlen("CIM_Chassis"))){		
		m_dashPtype = 0x03;
		if(m_ChassisCIM != NULL){
			ReleaseProfile(m_ChassisCIM);
			m_ChassisCIM = NULL;
		}
		max_count = 5;
		_xmlEltData = _Chassis;
	}else if((strcmp(cim_name,"CIM_Chip") == 0 )&&(strlen(cim_name)==strlen("CIM_Chip"))){
	m_dashPtype = 0x04;
		if(m_ChipCIM != NULL){
			ReleaseProfile(m_ChipCIM);
			m_ChipCIM= NULL;
		}
		max_count = 5;
		_xmlEltData = _Chip;
	}else if((strcmp(cim_name,"CIM_PhysicalMemory") == 0 )&&(strlen(cim_name)==strlen("CIM_PhysicalMemory"))){		
	m_dashPtype = 0x11;
		if(m_PhyMemCIM != NULL){
			ReleaseProfile(m_PhyMemCIM);
			m_PhyMemCIM= NULL;
		}
		max_count = 10;
		_xmlEltData = _PhysicalMemory;
	}else if((strcmp(cim_name,"CIM_Slot") == 0 )&&(strlen(cim_name)==strlen("CIM_Slot"))){		
	m_dashPtype = 0x09;
		if(m_SlotCIM != NULL){
			ReleaseProfile(m_SlotCIM);
			m_SlotCIM= NULL;
		}
		max_count = 10;
		_xmlEltData = _Slot;
	}else{
		return 0;
	}
	//----------------------------------------------------------------------------
	
	//m_dashPtype = MEM_DEV;
	//printf("[RTK] repare to DefaultInstance\n");	
	pCim = DefaultInstance(NULL,_xmlEltData,NewMem());
	//printf("[RTK] leave DefaultInstance\n");	
	//+ printout for debug--------------------------
	/*
	if(strcmp(cim_name,"CIM_PhysicalMemory") == 0){
		curr = pCim;
	 	while(curr){
		 	if(curr->eltname){
					printf("[RTK](%s)[%s:%s]\n",cim_name,curr->eltname,curr->data);
		 	}
			curr = curr->next ;
	 	}
	}else{
		printf("[RTK] m_CardCIM is null\n");
	}*/
	// * printout for debug--------------------------
	
	if(strcmp(cim_name,"CIM_Card") == 0 ){
		m_CardCIM = pCim;
	}else if(strcmp(cim_name,"CIM_Chassis") == 0 ){		
		m_ChassisCIM = pCim;
	}else if(strcmp(cim_name,"CIM_Chip") == 0 ){
		m_ChipCIM = pCim;
	}else if(strcmp(cim_name,"CIM_PhysicalMemory") == 0 ){		
		m_PhyMemCIM = pCim;
	}else if(strcmp(cim_name,"CIM_Slot") == 0 ){		
		m_SlotCIM = pCim;
	}
	return getCommonInstanceCount(cim_name);


}
void UpdateBIOSElementCIM(){
	
	if(m_biosElement!=NULL){
		ReleaseProfile(m_biosElement);
	}	
	m_dashPtype = 0;
	m_biosElement = DefaultInstance(NULL,_BIOSElement,NewMem()); 
	
}

void UpdateBIOSAttrCIM(){	
	
	if(m_biosAttr!=NULL){
		ReleaseProfile(m_biosAttr);
	}	
	m_biosAttr = DefaultBiosInstance(NULL,_BIOSAttribute,NewMem()); 	 	
}
void releaseBIOSElementCIM(){
	int idx;

	if(m_biosElement != NULL){
		ReleaseProfile(m_biosElement);
		m_biosElement = NULL;
	}		
	for(idx=0;idx<5;idx++){
		m_biosEleInstance[idx] = NULL;
	}	
	ReleaseBuf();


	//printFreeMem(2);
}
void releaseBIOSAttrCIM(){
	int idx;

	
	if(m_biosAttr != NULL){
		ReleaseProfile(m_biosAttr);
		m_biosAttr = NULL;
	}	
	for(idx=0;idx<10;idx++){
		m_biosAttrInstance[idx] = NULL;
	}
	ReleaseBuf();


	//printFreeMem(2);
}
void releaseProcessorCIM(){
	int idx;
	if(m_processorCIM != NULL){
		ReleaseProfile(m_processorCIM);
		m_processorCIM = NULL;
	}
	for(idx=0;idx<5;idx++){
		m_processorInstance[idx] = NULL;
	}
	ReleaseBuf();
}

void releaseMemoryCIM(){
	int idx;
	if(m_memoryCIM != NULL){
		ReleaseProfile(m_memoryCIM);
		m_memoryCIM = NULL;
	}
	for(idx=0;idx<5;idx++){
		m_memInstance[idx] = NULL;
	}
	ReleaseBuf();
}
void releaseSensorCIM(){
	int idx;
	if(m_SensorCIM != NULL){
		ReleaseProfile(m_SensorCIM);
		m_SensorCIM = NULL;
	}
	for(idx=0;idx<5;idx++){
		m_sensorInstance[idx] = NULL;
	}
	ReleaseBuf();
}
void releaseFanCIM(){
	int idx;
	if(m_fanCIM != NULL){
		ReleaseProfile(m_fanCIM);
		m_fanCIM = NULL;
	}
	for(idx=0;idx<5;idx++){
		m_fanInstance[idx] = NULL;
	}
	ReleaseBuf();
}


void releaseCommonCIM(char *cim_name){
	plist **instanceList = NULL;	
	int idx = 0;	
	int count = 0;
	if((strcmp(cim_name,"CIM_Card") == 0 )&&(strlen(cim_name)==strlen("CIM_Card"))){
		instanceList = m_CardInstance;		
		if(m_CardCIM != NULL){
			ReleaseProfile(m_CardCIM);
			m_CardCIM= NULL;
		}
		count= 5;
	}else if((strcmp(cim_name,"CIM_Chassis") == 0 )&&(strlen(cim_name)==strlen("CIM_Chassis"))){
		instanceList = m_ChassisInstance;		
		if(m_ChassisCIM != NULL){
			ReleaseProfile(m_ChassisCIM);
			m_ChassisCIM= NULL;
		}
		count=5;
	}else if((strcmp(cim_name,"CIM_Chip") == 0 )&&(strlen(cim_name)==strlen("CIM_Chip"))){
		instanceList = m_ChipInstance;		
		if(m_ChipCIM != NULL){
			ReleaseProfile(m_ChipCIM);
			m_ChipCIM= NULL;
		}
		count=5;
	}else if((strcmp(cim_name,"CIM_PhysicalMemory") == 0 )&&(strlen(cim_name)==strlen("CIM_PhysicalMemory"))){
		instanceList = m_PhyMemInstance;		
		if(m_PhyMemCIM != NULL){
			ReleaseProfile(m_PhyMemCIM);
			m_PhyMemCIM= NULL;
		}
		count=5;
	}else if((strcmp(cim_name,"CIM_Slot") == 0 )&&(strlen(cim_name)==strlen("CIM_Slot"))){
		instanceList = m_SlotInstance;		
		if(m_SlotCIM != NULL){
			ReleaseProfile(m_SlotCIM);
			m_SlotCIM= NULL;
		}
		count=5;
	}else{
		return ;
	}
	//----------------------------------------------------------------------------
	
	for(idx=0;idx<count;idx++){
		instanceList[idx] = NULL;
	}
	ReleaseBuf();	
}

void releaseOperationSysCIM(){
	int idx;
	if(m_OperationSysCIM != NULL){
		ReleaseProfile(m_OperationSysCIM);
		m_OperationSysCIM = NULL;
	}	
	ReleaseBuf();
}

void releaseCardCIM(){
	int idx;
	if(m_CardCIM != NULL){
		ReleaseProfile(m_CardCIM);
		m_CardCIM = NULL;
	}
	for(idx=0;idx<5;idx++){
		m_CardInstance[idx] = NULL;
	}
	ReleaseBuf();
}

void releaseChassisCIM(){
	int idx;
	if(m_ChassisCIM != NULL){
		ReleaseProfile(m_ChassisCIM);
		m_ChassisCIM = NULL;
	}
	for(idx=0;idx<5;idx++){
		m_ChassisInstance[idx] = NULL;
	}
	ReleaseBuf();
}
//CIM_BIOSElement.TargetOperatingSystem 
unsigned char *returnOperationSys(unsigned char *defaultv){
	int idx;

	idx = atoi(defaultv);	
	
	switch(idx){
						case 0:
							return "Unknown";
						case 1:
							return "Other";
						case 2:
							return "MACOS";
						case 14:
							return "MSDOS";	
						case 67:
							return "Windows XP";
						case 69:
							return "Microsoft Windows Server 2003";
						case 70:
							return "Microsoft Windows Server 2003 64-Bit";
						case 71:
							return "Windows XP 64-Bit";	
						case 72:
							return "Windows XP Embedded";	
						case 73:
							return "Windows Vista";	
						case 74:
							return "Windows Vista 64-Bit";	
						case 76:
							return "Microsoft Windows Server 2008";		
						case 77:
							return "Microsoft Windows Server 2008 64-Bit";		
						case 79:
							return "RedHat Enterprise Linux";		
						case 80:
							return "RedHat Enterprise Linux 64-Bit";	
						case 98:
							return "Linux 2.4.x";
						case 103:
							return "Microsoft Windows Server 2008 R2";		
						case 105:
							return "Microsoft Windows 7";
						default:
							return defaultv;
		}
}

unsigned char *getBIOSElemProperty(char *eltname , int eleidx){
	plist *curr=NULL;	
	int EltIdx=0;
	int idx=0;
	

	curr = m_biosEleInstance[eleidx];

	while((curr)&&(curr->eltname)){
	//while(curr->eltname){
		if(strcmp( curr->eltname ,eltname) == 0){	
			if(strcmp( "SoftwareElementState" ,eltname) == 0){
				idx = atoi(curr->data);
				if((idx < SoftwareElementState_MAX)&&(idx >=0)){
					return SoftwareElementState_value[idx]; 
				}
			}else if(strcmp( "TargetOperatingSystem" ,eltname) == 0){
				return returnOperationSys(curr->data);
			}else{		
				return curr->data;
			}
		}
		curr=curr->next;
	}	
	return "NA";
	
}

unsigned char *getBIOSAttrProperty(char *eltname , int attridx){
	plist *curr=NULL;	
	int EltIdx=0;
	int idx=0;

	
	curr = m_biosAttrInstance[attridx];
	
	while((curr)&&(curr->eltname)){
	//while(curr->eltname){
		
		if(strcmp( curr->eltname ,eltname) == 0){	
			return curr->data;
		}
		curr=curr->next;
	}
	
	return "NA";
	
}

unsigned char *getCommonProperty(char *eltname , int commonidx, char *cim_name){
	plist *curr=NULL;	
	int idx;
	
	if((strcmp(cim_name,"CIM_Card") == 0 )&&(strlen(cim_name)==strlen("CIM_Card"))){
		curr = m_CardInstance[commonidx];	
	}else if((strcmp(cim_name,"CIM_PROCESSOR") == 0 )&&(strlen(cim_name)==strlen("CIM_PROCESSOR"))){
		curr = m_processorInstance[commonidx];	
	}else if((strcmp(cim_name,"CIM_SoftwareIdentity") == 0 )&&(strlen(cim_name)==strlen("CIM_SoftwareIdentity"))){
		curr = m_swIdInstance[commonidx];
	}else if((strcmp(cim_name,"CIM_FAN") == 0 )&&(strlen(cim_name)==strlen("CIM_FAN"))){
		curr = m_fanInstance[commonidx];
	}else if((strcmp(cim_name,"CIM_SENSOR") == 0 )&&(strlen(cim_name)==strlen("CIM_SENSOR"))){
		curr = m_sensorInstance[commonidx];
	}else if((strcmp(cim_name,"CIM_MEMORY") == 0 )&&(strlen(cim_name)==strlen("CIM_MEMORY"))){
		curr = m_memInstance[commonidx];	
	}else if((strcmp(cim_name,"CIM_Chassis") == 0 )&&(strlen(cim_name)==strlen("CIM_Chassis"))){
		curr = m_ChassisInstance[commonidx];	
	}else if((strcmp(cim_name,"CIM_Chip") == 0 )&&(strlen(cim_name)==strlen("CIM_Chip"))){
		curr = m_ChipInstance[commonidx];	
	}else if((strcmp(cim_name,"CIM_PhysicalMemory") == 0 )&&(strlen(cim_name)==strlen("CIM_PhysicalMemory"))){
		curr = m_PhyMemInstance[commonidx];	
	}else if((strcmp(cim_name,"CIM_Slot") == 0 )&&(strlen(cim_name)==strlen("CIM_Slot"))){
		curr = m_SlotInstance[commonidx];	
	}
	else{
		return "NA";
	}


	//--------------------------------------------------------------------------
	while((curr) && (curr->eltname)){		
		if(strcmp( curr->eltname ,eltname) == 0){

			if(strcmp("HealthState" ,eltname) == 0){				
				return getHealthState(curr->data);
			}else if(strcmp("EnabledState" ,eltname) == 0){
				idx = atoi(curr->data);
				return getEnableState(idx);
			}else if(strcmp("RequestedState" ,eltname) == 0){				
				return getRequestedState(curr->data);
			}else if(strcmp("OperationalStatus" ,eltname) == 0){				
				return getOperationalStatus(curr->data);
			}else if(strcmp( "Classifications" ,eltname) == 0){
					idx = atoi(curr->data);	
					if(idx < softidProp_class_MAX){
						return softidProp_class[idx];
					}else if(idx<=32768){
						return "DMTF Reserved";
					}else{
						return "Vendor Reserved";
					}
			}else if(strcmp("Access" ,eltname) == 0){				
				return getAccess(curr->data);
			}else if(strcmp("MemoryType" ,eltname) == 0){
				return getMemoryType(curr->data);
			}
			else if(strcmp("CPUStatus" ,eltname) == 0){
				idx = atoi(curr->data);
				switch(idx){
					case 0:
						return "Unknown";
					break;
					case 1:
						return "CPU Enabled";	
					break;
					case 2:
						return "CPU Disabled by User";
					break;
					case 3:
						return "CPU Disabled By BIOS (POST Error)";
					break;
					case 4:
						return "CPU Is Idle";
					break;
					case 7:
						return "Other";
					break;
					default:
						return "NA";
					break;
				
				}
			}			
			else{
				return curr->data;
			}
		}
		curr= curr->next;
	}
	return "NA";
}
unsigned char *getPowerMangSrvProperty(char *eltname){
	XMLElt *pElt=NULL;
	int EltIdx=0;
	int idx=0;

	pElt = _AssociatedPowerManagementService;

	while(pElt[EltIdx].eltname){				
		
		if(strcmp( pElt[EltIdx].eltname ,eltname) == 0){						

			if(strcmp( "PowerState" ,eltname) == 0){			
				idx = atoi(pElt[EltIdx].data);			
				if((idx < power_state_value_MAX)&&(idx >=0)){
					return power_state_value[idx];			
				}else if(idx< 32767){
					return "DMTF Reserved";
				}else{
					return "Vendor Reserved";
				}
			}				
				
		}else{
			EltIdx++;
		}
	}
	return "NA";

	
}

//+briank.rtk for Web management page

unsigned char *getTestCIMProperty_debug(int idx,int tag){
	XMLElt *pElt=NULL;	

	pElt = _BIOSElement;

	printf("[RTK]<%d> (%s)::(%s)\n",tag,pElt[idx].eltname , pElt[idx].data);

	//idata
}

unsigned char *getOsPropertyFromIdata(char *eltname){

	unsigned char *pch;

	
	return "unknown";
}
	
unsigned char *getOperationSysProperty(char *eltname){

	XMLElt *pElt=NULL;
	int EltIdx=0;
	int idx=0;
	
//printf("[RTK]idata=(%s)\n",idata);
	
	pElt = _OperatingSystem;


	
	if(strcmp( "Version" ,eltname) ==0){
		if(idata == NULL){
			return pElt[1].data;
		}else{
			return getOsPropertyFromIdata("Version");
		}
	}	
	else{
		while(pElt[EltIdx].eltname){
			//EnabledState , RequestedState,AvailableRequestedStates,TransitioningToState
			if(strcmp( pElt[EltIdx].eltname ,eltname) == 0){

				if(strcmp( "OSType" ,eltname) == 0){								
					return returnOperationSys(pElt[EltIdx].data);
					
				}else if(strcmp( "EnabledState" ,eltname) == 0){			
					idx = atoi(pElt[EltIdx].data);			
					if(idx < sys_enState_value_MAX){
						return sys_enState_value[idx];	
					}else if(idx<=32768){
						return "DMTF Reserved";
					}else{
						return "Vendor Reserved";
					}
				}else if(strcmp( "RequestedState" ,eltname) == 0){
					idx = atoi(pElt[EltIdx].data);	
					if(idx < sys_reqState_value_MAX){
						return sys_reqState_value[idx];
					}else if(idx<=32768){
						return "DMTF Reserved";
					}else{
						return "Vendor Reserved";
					}
				}
				else{					
					
					return pElt[EltIdx].data;	
				}
			}			
			EltIdx++;
		}
	}
	
	return "NA";//"Please Call FAE!!!"	
}

 



unsigned char *getComputerSysProperty(char *eltname){
	XMLElt *pElt=NULL;
	int EltIdx=0;
	int idx=0;

	pElt = _ComputerSystem;

	while(pElt[EltIdx].eltname){


		
		if(strcmp( pElt[EltIdx].eltname ,eltname) == 0){
			
			if(strcmp( "Dedicated" ,eltname) == 0){			
				idx = atoi(pElt[EltIdx].data);			
				if(idx < compSys_Dedicated_value_MAX){
					return compSys_Dedicated_value[idx];			
				}else if(idx< 32568){
					return "DMTF Reserved";
				}else{
					return "Vendor Reserved";
				}
			}else if(strcmp( "EnabledState" ,eltname) == 0){			
				idx = atoi(pElt[EltIdx].data);			
				if(idx < sys_enState_value_MAX){
					return sys_enState_value[idx];	
				}else if(idx<=32768){
					return "DMTF Reserved";
				}else{
					return "Vendor Reserved";
				}
			}else if(strcmp( "RequestedState" ,eltname) == 0){			
				idx = atoi(pElt[EltIdx].data);			
				if(idx < sys_reqState_value_MAX){
					return sys_reqState_value[idx];
				}else if(idx<=32768){
					return "DMTF Reserved";
				}else{
					return "Vendor Reserved";
				}
			}else{
			
				return pElt[EltIdx].data;
			}
		}else{
			EltIdx++;
		}
	}
	return "NA";//"Please Call FAE!!!"
	
}


//Description : 'count' is the start index when searching "_PowerManagementCapabilities"
unsigned char *getPowerMangCapProperty(char *eltname,int *count){
	XMLElt *pElt=NULL;
	int EltIdx=0;
	int idx=0;

	pElt = _PowerManagementCapabilities;

	EltIdx = *count;

	while(pElt[EltIdx].eltname){				
		
		if(strcmp( pElt[EltIdx].eltname ,eltname) == 0){						

			if(strcmp( "PowerChangeCapabilities" ,eltname) == 0){			
				idx = atoi(pElt[EltIdx].data);			
				if((idx < power_state_value_MAX)&&(idx >=0)){
					*count = EltIdx ;
					return powMangCapProp_powChg[idx];			
				}else{
					*count = -1 ;
					return "undefine";
				}
			}				
				
		}else{
			EltIdx++;
		}
	}
	*count = -1 ;
	return "";

	
}

#endif


unsigned char *NewMem()
{
    m_list *mptr;
    mptr=malloc(sizeof(m_list));
    mptr->mbuf=malloc(1024);
    memset(mptr->mbuf,0,1024);
    if (!memhead)
        memhead=mptr;
    mptr->next = NULL;
    if (memtail)
        memtail->next =mptr;

    memtail = mptr;
    return mptr->mbuf;
}
void ReleaseBuf()
{
    m_list *mfreeptr,*mptr;

    mptr=memhead;
    while (mptr)
    {
        mfreeptr = mptr;
        mptr = mptr->next;
        free(mfreeptr->mbuf);
        free(mfreeptr);
    }
    memhead=NULL;
    memtail=NULL;

}
unsigned char *TmpNewMem()
{
    m_list *mptr;
    mptr=malloc(sizeof(m_list));
    mptr->mbuf=malloc(1024);
    memset(mptr->mbuf,0,1024);
    if (!tmp_memhead)
        tmp_memhead=mptr;
    mptr->next = NULL;
    if (tmp_memtail)
        tmp_memtail->next =mptr;

    tmp_memtail = mptr;
    return mptr->mbuf;
}
void TmpReleaseBuf()
{
    m_list *mfreeptr,*mptr;

    mptr=tmp_memhead;
    while (mptr)
    {
        mfreeptr = mptr;
        mptr = mptr->next;
        free(mfreeptr->mbuf);
        free(mfreeptr);
    }
    tmp_memhead=NULL;
    tmp_memtail=NULL;

}
int generate_uuid(char *ptr)
{

    static int clock_sequence = 1;
    unsigned long long longTimeVal;
    int timeLow = 0;
    int timeMid = 0;
    int timeHigh = 0;
    //unsigned char uuid[16];
    INT8U *uuid = malloc(16);
    int i;
//   char buf[32]="";
    //  char* ptr = buf;
    struct timeval tv;
    //int max_length = SIZE_OF_UUID_STRING;

//    if ( !no_prefix ) max_length += 5;      // space for "uuid:"
//    if ( size < max_length )
//        return 0;

//    if ( buf == NULL )
//        return 0;

    // get time data
    gettimeofday( &tv, NULL );
    longTimeVal = (unsigned long long)1000000 * (unsigned long long)tv.tv_sec + (unsigned long long)tv.tv_usec;
    timeLow = longTimeVal & 0xffffffff;     // lower 32 bits
    timeMid = (longTimeVal >> 32) & 0xffff; // middle 16 bits
    timeHigh = (longTimeVal >> 32) & 0xfff; // upper 12 bits

    // update clock sequence number
    clock_sequence++;

    // get mac address

//   {
    for ( i = 0; i < 6; i++ )
        uuid[i] = dpconf->srcMacAddr[eth0][i];                       // mac address
//    } else {
//        for( i = 0; i < 6; i++ )
//            uuid[i] = clock_sequence & 0xff;        // just in case we do not have a mac
//   }

    uuid[6] = clock_sequence & 0xff;                // clock seq. low
    uuid[7] = 0x80 | ((clock_sequence >> 8) & 0x3f);// clock seq. high and variant
    uuid[8] = timeHigh & 0xff;                      // time high/version low bits
    uuid[9] = timeHigh >> 8 | 0x10;                 // time high/version high bits
    *(short int*)(uuid+10) = (short int)timeMid;
    *(int*)(uuid+12) = timeLow;
    /*
        if ( !no_prefix )
        {
            sprintf( ptr, "uuid:" );
            ptr += 5;
        }
    */
    sprintf( ptr, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             uuid[15], uuid[14], uuid[13], uuid[12],
             uuid[11], uuid[10], uuid[9], uuid[8],
             uuid[7], uuid[6], uuid[0], uuid[1],
             uuid[2], uuid[3], uuid[4], uuid[5] );

    free(uuid);
    DEBUGMSG(DASH_DEBUG, "uuid=%s\n",ptr);

    return 1;

}
char *strback(char *str,char ch)
{
    while (*--str != ch)
        ;

    return str;
}
INT8U str2hex (INT8U * s)
{
    INT8U x = 0;
    INT8U c;

    c = *s;
    if (c >= '0' && c <= '9')
        c = c - '0';
    else
    {
        if (c >= 'a' && c <= 'f')
            c = c - 'a' + 0x0a;
        else if (c >= 'A' && c <= 'F')
            c = c - 'A' + 0x0a;
        else
            c = 0;                    // wrong char
    }

    x = c << 4;

    s++;
    c = *s;
    if (c >= '0' && c <= '9')
        c = c - '0';
    else
    {
        if (c >= 'a' && c <= 'f')
            c = c - 'a' + 0x0a;
        else if (c >= 'A' && c <= 'F')
            c = c - 'A' + 0x0a;
        else
            c = 0;                    // wrong char
    }

    x |= c;

    return x;
}
INT8U subscribe(DASHCB *dcb,SubEvent *NewSub)
{
    INT8U count = 0;
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    SubEvent *tmp=subhead;
    SubEvent *now=NULL;
    while (tmp)
    {
        now=tmp;
        tmp=tmp->next;
        count++;
    }
    NewSub->next=NULL;
    if (now==NULL)
        subhead=NewSub;
    else if (count < 15)
        now->next = NewSub;
    else
    {
        free(NewSub);
        return Fault_EventSourceUnableToProcess;
    }

    //  change_sub=1;
    dcb->control=Csubscribe;
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    dirty_ext[SUBTBL].length=dirty_ext[SUBTBL].length+sizeof(SubEvent);
    dirty_ext[SUBTBL].dirty=1;
#else
    setdirtylen(SUBTBL, sizeof(SubEvent) , DIRTY_ADD);
    setdirty(SUBTBL);
#endif
    OS_EXIT_CRITICAL();
    return 0;

}

void unsubscribe(DASHCB *dcb,INT8U *id)
{
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    SubEvent *tmp=subhead;
    SubEvent *now = NULL;
    while (tmp)
    {
        if (strcmp(tmp->subid,id)==0)
        {
            if (now)
                now->next =  tmp->next;
            else
                subhead = tmp->next;

            free(tmp);
            dcb->control=Cunsubscribe;
            break;
        }
        now = tmp;
        tmp = tmp->next;
    }

    OS_EXIT_CRITICAL();

    if(dcb->control==Cunsubscribe)
    {
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
        dirty_ext[SUBTBL].length=dirty_ext[SUBTBL].length-sizeof(SubEvent);
        dirty_ext[SUBTBL].dirty=1;
#else
        setdirtylen(SUBTBL, sizeof(SubEvent) , DIRTY_SUB);
        setdirty(SUBTBL);
#endif
    }
}

void sendpacket(DASHCB *dcb)
{
    send_list *freehead;
    PKT* txPkt = NULL;

    while (dcb->sendlist)
    {
        if (!dcb->cs->pcb)
        {
            freehead = dcb->sendlist;
            dcb->sendlist = dcb->sendlist->next;
            free(freehead->addr);
            free(freehead);
            //   dcb->sendlist = dcb->sendlist->next;
        }

        else
        {
            if (dcb->cs->pcb->TLSEnable)
            {
                sendTLSWebPage(dcb->sendlist->addr+PKT_HDR_SIZE,dcb->sendlist->len , dcb->cs->pcb);
                freehead = dcb->sendlist;
                dcb->sendlist = dcb->sendlist->next;
                free(freehead->addr);
                free(freehead);
            }
            else
            {
                txPkt = (PKT*)setPkt(dcb->sendlist->addr,dcb->sendlist->len);

                freehead = dcb->sendlist;
                dcb->sendlist = dcb->sendlist->next;
                free(freehead);
                rtSktSend_F(dcb->cs, txPkt, 0);
            }
        }

    }

}
plist* ReleaseProfile(plist *ptr)
{
    plist *freeptr;
    while (ptr)
    {
        freeptr = ptr;
        ptr = ptr->next;
        free(freeptr);
    }
    return NULL;
}
INT16U ClearSendBuf(DASHCB *dcb)
{
    send_list *freehead;
    INT16U len=0;
    while (dcb->sendlist)
    {
        freehead = dcb->sendlist;
        dcb->sendlist = dcb->sendlist->next;
        len=len+freehead->len;
        free(freehead->addr);
        free(freehead);
    }
    return len;

}

send_list* new_send_list()
{
    send_list *ptr = malloc(sizeof(send_list));
    memset(ptr, 0 , sizeof(send_list));
    ptr->addr = malloc(ETH_PKT_LEN);
    // memset(ptr->addr, 0 , PKT_HDR_SIZE+1);
    memset(ptr->addr, 0 , ETH_PKT_LEN);
    return ptr;
}

void insertlen(DASHCB *dcb,send_list *sendlist, INT16U headerlen)
{
    send_list *current = sendlist;
    INT16U total = 0;
    INT8U *ptr;

    while (current)
    {
        //assert(sendlist->len < ETH_PAYLOAD_LEN);
        total += current->len;
        current = current->next;
    }
    if(dcb->ad_nego==1)
    {
    	ptr = strstr(sendlist->addr + PKT_HDR_SIZE,"\r\n\r\n");
    	headerlen=ptr-(sendlist->addr + PKT_HDR_SIZE)+4;
    }

    total -= headerlen;

    ptr = strstr(sendlist->addr + PKT_HDR_SIZE,"xxxxx");
    sprintf(ptr,"%5d",total);

    *(ptr+5) = '\r';
    //remove the null character
}

XMLElt *ProfileEntry(DASHCB *dcb,char *r_uri)
{
    INT16U i=0;
    Profile *p= _Profile,*ptr=NULL;
    int change=0;
    dcb->Ptype=-2;

    for (;;)
    {

        if (!p[i].eltname)
            break;

        if (strcmp(p[i].eltname,r_uri)==0 && strlen(p[i].eltname)==strlen(r_uri))
        {
            if (p[i].type==99)
                dcb->Ptype=0;
            else if ( p[i].type >0)
                dcb->Ptype=p[i].type;
            else
                dcb->Ptype=-2;
            dcb->profile_priv=p[i].priv;

            if( p[i].type==-1)
                dcb->association=-1;
            ptr=p[i].addr;
            break;
        }

        i++;
    }

    if (!(dcb->privilege & dcb->profile_priv))
        return NULL;
    //  if (ptr==_TCPProtocolEndpoint && dcb->tcp==1)
    //      ptr=_USBTCPProtocolEndpoint;

	if(ptr == _StaticIPAssignmentSettingData){		
		if(bsp_in_band_exist()){
			//not allow network setting at in-band 
			return NULL;
		}
	}


    if ((ptr==_IndicationFilter || ptr==_ListenerDestinationWSManagement || ptr==_IndicationSubscription) && subhead==NULL)
        return NULL;
    else if (ptr!=NULL)
        return ptr;
    else return NULL;
}

plist * SearchEnd(plist *p,INT32S InstanceCount,char *Seq)
{
    INT8U *eltname,*data;
    INT8U count=0;
    plist *ptr;

    if (Seq!=NULL)
        *Seq=1;

    ptr=p;

    while (p)
    {
        eltname=p->eltname;
        data=p->data;

        if (strcmp(eltname,"1")==0 && count==InstanceCount)
        {
            if (Seq!=NULL)
                *Seq=0;
            break;
        }
        if (strcmp(eltname,"1")==0)
        {
            count++;
            ptr=p->next;
        }
        p=p->next;
    }

    return ptr;

}
INT32S GetInstanceCount(DASHCB *dcb,plist *p,INT8U *sbuf)
{
    int count=0,match=0;
    INT8U *pname,*pdata;
    if (strcmp(sbuf,"")==0)
        return -1;

    while (p)
    {

        if (strcmp(p->eltname,"1")==0)
        {
            if (match >0)
                break;
            count++;
            match=0;
            p=p->next;
            continue;
        }
        if (p->key==1)
        {

            match=0;
            pname=sbuf;
            pdata=pname+strlen(pname)+1;
            while (strlen(pname))
            {
                if (strcmp("CreationClassName",pname) == 0)
                {
                    if (!strstr(pdata,"CIM_") && strcmp(pdata,"")!=0)
                    {
                        dcb->fault_id=Fault_InvalidFilter;
                        break;
                    }
                }
                if (strcmp(p->eltname,pname)==0 && strcmp(p->data,pdata)==0)
                    match++;

                pname=pdata+strlen(pdata)+1;
                pdata=pname+strlen(pname)+1;

            }

            if (match >1)
            {
                dcb->fault_id=Fault_DuplicateSelectorFault;
                break;
            }

            if (match==0)
            {
                p=p->next;
                while (p)
                {
                    if (strcmp(p->eltname,"1")==0)
                        break;
                    p=p->next;
                }
                continue;
            }

        }

        p=p->next;
    }
    if (match ==1)
        return count;
    else if ( dcb->fault_id > 0)
        return -1;
    else
    {
        dcb->fault_id=Fault_InvalidNameFault;
        return -1;
    }
}

plist *GetElement(plist *p,INT8U *name,INT8U icount)
{
    int count=0;
    if (p)
    {
        if (strcmp(p->eltname,"1")==0)
            p=p->next;
    }

    while (p)
    {
        //  if(!p->eltname)
        //    break;

        if (strcmp(p->eltname,"1")==0)
        {
            count++;
            p=p->next;
            continue;
        }
        if (!name)
        {
            if (p->key==1 && count==icount)
                return p;
        }
        else
        {
            if (strcmp(p->eltname,name)==0 && count==icount)
                return p;
        }

        p=p->next;
    }

    return NULL;
}

int SetElement(plist *p,INT8U *name,INT8U *data,INT8U *out)
{

    while (p)
    {
        if (!p->eltname)
            break;

        if (strcmp(p->eltname,name)==0)
        {
            p->data=out;
            strcpy(p->data,data);
            return 1;
        }

        p=p->next;
    }


    return 0;
}


send_list * SelectElement (send_list *current,INT8U *buf, plist *p,char index)
{
    INT8U *eltname,*data,count=0,key,set=0;

    buf = current->addr + PKT_HDR_SIZE;

    while (p)
    {
        eltname=p->eltname;
        data=p->data;
        key=p->key;
        if (!eltname)
            break;

        if (strcmp(eltname,"1")==0)
        {
            count++;
            p=p->next;

            continue;
        }

        if (count > index)
            break;

        if (key==1 && index==count)
        {
            if (strlen(buf) >= PAYLOAD_SIZE_THD)
            {

                current->len = strlen(buf);
                current->next = new_send_list();

                current = current->next;
                buf = current->addr + PKT_HDR_SIZE;

            }
            if (set==0)
            {
                set=1;
                sprintf(buf+strlen(buf),"<%s:SelectorSet>",Ns[IDX_NS_WS_MAN].prefix);
                sprintf(buf+strlen(buf),"<%s:Selector Name=\"__cimnamespace\">root/interop</%s:Selector>",Ns[IDX_NS_WS_MAN].prefix,Ns[IDX_NS_WS_MAN].prefix);
            }
            sprintf(buf+strlen(buf),"<%s:Selector Name=\"%s\">%s</%s:Selector>",Ns[IDX_NS_WS_MAN].prefix,eltname,data,Ns[IDX_NS_WS_MAN].prefix);

        }

        p=p->next;
    }
    if (set==1)
        sprintf(buf+strlen(buf),"</%s:SelectorSet>",Ns[IDX_NS_WS_MAN].prefix);
    return current;
}


int SearchIndex (plist *p,INT8U *data1,INT8U *data2)
{
    INT8U *eltname,*data;
    int count=0;

    for (;;)
    {
        eltname=p->eltname;
        data=p->data;
        //DEBUGMSG(DASH_DEBUG, "ZZ=%s %s %s %s\n",eltname,data,data1,data2);
        if (!eltname)
            break;
        if (strcmp(eltname,"1")==0)
        {
            p=p->next;
            count++;
            continue;
        }
        if (strcmp(eltname,data1)==0 && strcmp(data,data2)==0)
        {
            return count;
        }

        p=p->next;
    }

    return -1;
}

plist *DefaultRoleInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata)
{
    INT16U i=0,j=0,index=0;
    plist *head=NULL,*current=NULL,*prev=NULL;
    unsigned char *out = pdata;

    UserInfo *acc_ptr=getuserinfo();
    RoleInfo *role_p;
    INT8U mask=0xff,role=0xff,idx=0,next=0;
    if ((dcb->actiontype==WS_DISP_TYPE_PULL || AllClass==1) && dcb->ref_count[0]!=0xff)
    {
        while (acc_ptr)
        {
            if (idx==dcb->ref_count[0])
            {
                role=acc_ptr->role;
                break;
            }
            idx++;
            acc_ptr=acc_ptr->next;
        }
    }
    role_p=rolehead;
    while (role_p)
    {
        if(!(role_p->mask & role))
        {
            role_p=role_p->next;
            continue;
        }
        if (next==1)
        {
            index++;
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
        }
        i=0;
        while (p[i].eltname)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;
            current->data=p[i].data;
            if (strcmp(current->eltname,"Name")==0 || strcmp(current->eltname,"ElementName")==0)
                strcpy(out,role_p->name);

            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j>PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }

            current->key=p[i].key;
            current->used=p[i].used;
            current->idx=p[i].idx;

            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;

            i++;
        }
        next=1;
        role_p=role_p->next;
    }

    return head;
}

#ifdef ASF_BOOT_ONLY
plist *DefaultBootOrderInstance(XMLElt *p,INT8U *pdata)
{
    INT16U i=0,j=0,k=0;

    plist *head=NULL,*current=NULL,*prev=NULL;
    INT8U *BootOrder;
    unsigned char *out=pdata;
    BootOrder=getBootOrder();
    do
    {
        while (p[i].eltname)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;
            current->data=p[i].data;

            if (p[i].idx >0 && *BootOrder!= 0)
            {

                strcpy(out, getBootstr(*BootOrder));
                if (strlen(out))
                {
                    current->data=out;
                    j=j+strlen(out)+1;
                    if (j> PAYLOAD_SIZE_THD)
                    {
                        pdata=NewMem();
                        j=0;
                    }
                    out=pdata+j;
                    strcpy(out,"");
                }

            }
            /*
            if (strcmp(current->eltname,"PartComponent")==0 && p==_OrderedComponent)
            {
                current->key=k;
                k++;
            }
            else
            */
            current->key=p[i].key;

            current->used=p[i].used;
            current->idx=p[i].idx;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;
            i++;
        }

        i=0;
        BootOrder++;
        if (*BootOrder!=0)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname= NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
        }
    } while (*BootOrder!= 0);
    dcb->InstanceCount=index+1;
    return head;

}
#endif

int getUserState(INT8U *name,INT8U count);

plist *DefaultAccountInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata)
{
    INT16U i=0,j=0,index=0;
    unsigned char *out = pdata;
    plist *head=NULL,*current=NULL,*prev=NULL;

    UserInfo *ptr=getuserinfo();

    do
    {
        while (p[i].eltname)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;
            current->data=p[i].data;
            
            if (strcmp(current->eltname,"Name")==0 || strcmp(current->eltname,"UserID")==0)
                strncpy(out, ptr->name,sizeof(ptr->name));
          
            if (strcmp(current->eltname,"Caption")==0)
                strncpy(out, ptr->caption,sizeof(ptr->caption));

            if (strcmp(current->eltname,"InstanceID")==0)
                sprintf(out,"%s:%d",p[i].data,index);

            if (strcmp(current->eltname,"EnabledState")==0){
              	  sprintf(out,"%d",getUserState(ptr->name , 0xFF));
	     }

            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j> PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }

            current->key=p[i].key;
            current->used=p[i].used;
            current->idx=p[i].idx;

            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;
            i++;
        }

        i=0;
        ptr=ptr->next;
        if (ptr)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname= NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
            index++;
        }
    } while(ptr);
    dcb->InstanceCount=index+1;
    return head;

}
plist *DefaultLogInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata) // Event Log
{
    INT16U i=0,j=0,id=0,tmp=0,offset = 0;
    plist *head=NULL,*current=NULL,*prev=NULL;
    ///struct _elist *ptr;
    ///eventdata *ed;
    eventdata *ptr=NULL,*ptr1=NULL;
    unsigned char *out=pdata;
    INT8U flag=0;
    LogEnd=1;
    ptr= eventGet();
    i=-1;
    int count;

    count = dcb->ref_count[0];
    if (count == 5)
        count = 0xff;
    while (ptr)
    {

        if (ptr->logtype==count || count == 0xff)
            i++;
        if (flag==1 && (ptr->logtype==count || count==0xff))
        {
            LogEnd=0;
            id=tmp;
            break;

        }
        if ((i==dcb->count && ptr->logtype==count)||(id==dcb->count && count==0xff))
        {
            flag=1;
            ptr1=ptr;
            tmp=id;
        }

        ptr=ptr->next;
        if (ptr==NULL)
            break;
        id++;

    }
    if (LogEnd==1 && flag==1)
        id=tmp;


    ptr=ptr1;
    if (!ptr)
        return NULL;
    i=0;
    //ed = ptr->addr;
    while (p[i].eltname)
    {
        current=malloc(sizeof(plist));
        memset(current, 0 , sizeof(plist));
        current->eltname=p[i].eltname;
        current->data=p[i].data;

        if (strcmp(current->eltname,"InstanceID")==0)
            sprintf(out,"%d",id);
        else if (strcmp(current->eltname,"RecordData")==0)
        {
         #if CONFIG_FWLOG_ENABLED
        	if(ptr->Event_Sensor_Type == 0x7F && ptr->Event_Type == 0x1F ){
				
      		getFwLogParser(ptr, out);

	}else	
	#endif
            offset = getEventLogParser(ptr, out);
        }
        else if (strcmp(current->eltname,"CreationTimeStamp") == 0)
        {
            offset = strftime(out, 255, "%Y%m%e%H%M%S.000000-000",
                              localtime((time_t *) &(ptr->timestamp)));
        }

        if (strlen(out))
        {
            current->data = out;
            j=j+strlen(out)+1;
            if (j> PAYLOAD_SIZE_THD)
            {
                pdata=NewMem();
                j=0;
            }
            out=pdata+j;
            strcpy(out,"");
        }

        current->key=p[i].key;
        current->used=p[i].used;
        current->idx=p[i].idx;

        if (head == NULL)
            head = current;
        else
            prev->next = current;

        prev = current;
        i++;
    }
    return head;

}

plist *DefaultIndicationInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata)
{
    INT16U i=0,j=0,index=0;
    int filter;
    plist *head=NULL,*current=NULL,*prev=NULL;
    unsigned char *out=pdata;
    SubEvent *ptr=subhead;

    if (!ptr)
        return NULL;

    do
    {
        while (p[i].eltname)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;
            current->data=p[i].data;

            if (strcmp(current->eltname,"Name")==0)
                strcpy(out, ptr->subid);

            if (strcmp(current->eltname,"Query")==0)
                strcpy(out, ptr->query);
            if (strcmp(current->eltname,"Destination")==0)
                strcpy(out, ptr->notifyto);


            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j> PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }

            current->key=p[i].key;
            current->used=p[i].used;

            if (p==_FilterCollectionSubscription || p == _IndicationSubscription)
            {
                if ((strcmp(current->eltname,"Handler")==0) || (strcmp(current->eltname,"Filter")==0))
                {
                    if ((strstr(ptr->query,"FilterCollection")) && (strcmp(current->data,"CIM_FilterCollection") == 0))
                    {
                        sscanf(ptr->query,"FilterCollection:%d",&filter);
                        current->idx=filter;
                    }
                    else
                        current->idx = index;
                }
            }
            else
                current->idx=p[i].idx;

            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;
            i++;
        }

        i=0;
        ptr=ptr->next;
        if (ptr)
        {
            index++;
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
        }
    } while (ptr);
    dcb->InstanceCount=index+1;
    return head;

}
plist *DefaultPldmInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata)
{
    INT8U i=0,index=0;
    INT16U j = 0,k=0;
    plist *head=NULL,*current=NULL,*prev=NULL;
    INT8U curLoc;
    INT8U *out=pdata;
    BiosAttr *ptr ;
    //INT8U tmpbuf[128];
    INT8U *tmpbuf;
    INT8U attr_val_num = 0;
    INT8U *attr_val_index;
    OS_CPU_SR cpu_sr;


    OS_ENTER_CRITICAL();
    memcpy(&pldmdata, pldmdataptr, sizeof(pldmdata));
    OS_EXIT_CRITICAL();
    if(pldmdata.TBL1 | pldmdata.TBL2 | pldmdata.TBL3)
    {
        pldmdataptr->TBL1 = 0;
        pldmdataptr->TBL2 = 0;
        pldmdataptr->TBL3 = 0;

        boot_seq[0]=0xff;
        if (dpconf->bios == AMIBIOS && dpconf->biostype == LEGACY)
        {
            for (i=0; i<16; i++)
                boot_seq[i]=i;
        }
        OS_ENTER_CRITICAL();
        PLDMptr = InitialPLDM();
        OS_EXIT_CRITICAL();
    }
    ptr=PLDMptr;

    while (ptr)
    {
        if (ptr->attr_type==dcb->Ptype)
            break;
        ptr=ptr->next;
    }

    if (ptr)
    {
        attr_val_num   = ptr->attr_val_possible_num;
        if (boot_seq[0]!=0xff)
            attr_val_index = boot_seq;
        else
        {
            attr_val_index = ptr->attr_val_index;
            i=0;
            while(i <16)
            {
                boot_seq[i]=*(attr_val_index+i);
                i++;
                if(i==attr_val_num)
                    break;
            }
        }
    }
    else
        return NULL;

    if (attr_val_num == 0)
        return NULL;

    tmpbuf = malloc(128);
    i=0;
    do
    {
        curLoc=0;
        while (p[i].eltname)
        {

            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));

            current->eltname=p[i].eltname;

            current->data=p[i].data;

            // if (p[i].idx >0)
            // {
            if (strcmp(current->eltname,"InstanceID")==0)
                sprintf(out,"%d",*attr_val_index);

            else if ( strcmp(current->eltname,"AssignedSequence")==0)
                sprintf(out,"%d",index);

            else if (p[i].idx >0)
            {
                if(ptr->attr_val_index != NULL)
                    getPLDMElement(ptr,1,*attr_val_index,out);
            }


            if ( strcmp(current->eltname,"ElementName")==0)
            {
                strcpy(tmpbuf,out);
                sprintf(out,"Boot Setting Configuration (%s)",tmpbuf);
            }

            if ( strcmp(current->eltname,"StructuredBootString")==0)
            {
                strcpy(tmpbuf,out);
                sprintf(out,"CIM:%s:1",tmpbuf);
            }
            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j> PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }

            // }
            current->key=p[i].key;

            current->used=p[i].used;
            current->idx=p[i].idx;
            current->next=NULL;

            if (strcmp(current->eltname,"PartComponent")==0 && p==_OrderedComponent)
            {
                current->idx=k;
                k++;
            }
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;

            i++;

        }
        i=0;
        //ptr->attr_val_possible_num--;
        attr_val_num--;

        if (attr_val_num > 0)
        {
            // ptr->attr_possible_handle++;
            index++;
            //ptr->attr_val_index++;
            attr_val_index++;
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
        }
    } while (attr_val_num >0);


    free(tmpbuf);
    dcb->InstanceCount=index+1;
    return head;
}
plist *DefaultInbandInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata)
{
    INT16U i=0,j=0,index=0;//len=0;
    plist *head=NULL,*current=NULL,*prev=NULL;
    unsigned char *out = pdata;

    char *data=NULL,*data_end=NULL,*start_ptr=NULL,*end_ptr,tmp_ch;
    int major,minor,revision,build,current_class=0;
    unsigned char *pclass,*addr;
    static INT8U *ostype=NULL, *osname = NULL;

    SMBIOSHeader sh = {0};
    unsigned char val=0;

    if (ostype == NULL)
        ostype = malloc(8);
    if (osname == NULL)
        osname = malloc(64);

//debug_printFreeMemory_F();

    //if (bsp_AP_exist())
    if (bsp_IODrvAP_exist() || bsp_AP_exist())
        strcpy(state,"2");
    else
        strcpy(state,"3");

#if CONFIG_VERSION > 1
    if (p==_OperatingSystem && strstr(idata,"CIM_OperatingSystem="))
        data=strstr(idata,"CIM_OperatingSystem=")+strlen("CIM_OperatingSystem=");
    if (p==_SoftwareIdentity && strstr(idata,"CIM_SoftwareIdentity="))
        data=strstr(idata,"CIM_SoftwareIdentity=")+ strlen("CIM_SoftwareIdentity=");
    if (data)
    {
        data=strchr(data,'#');
    }

#endif

    while (1)
    {
        i=0;
        pclass=NULL;
        while (p[i].eltname)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;

            current->data=p[i].data;

            if (i==0 && data!=NULL)
            {
                start_ptr=strstr(data,"VersionString=");
                if (start_ptr)
                {
                    start_ptr=start_ptr+14;
                    end_ptr=strchr(start_ptr+1,',');
                    if (end_ptr)
                    {
                        tmp_ch=*end_ptr;
                        *end_ptr='\0';
                        build = 0;
                        sscanf(start_ptr,"%d.%d.%d.%d",&major,&minor,&revision,&build);
                        *end_ptr=tmp_ch;
                    }
                }
            }
            if (index==0 && p==_SoftwareIdentity)
            {
                if (strcmp(current->eltname,"VersionString")==0)
                {
                    addr = getSMBIOSTypeAddr(0);
                    if(addr)
                    {
                        toSMBIOSHeader(&sh, addr);
                        getSMBIOSElement(addr, &sh, 0, BIOS_VER, out);
                    }
                }
                if (strcmp(current->eltname,"IdentityInfoType")==0)
                    strcpy(out,"Bios Firmware");

            }
            else if (index==1 && p==_SoftwareIdentity)
            {
                if (strcmp(current->eltname,"VersionString")==0)
                    sprintf(out,"%d.%d.%d.%d",dpconf->fwMajorVer, dpconf->fwMinorVer, dpconf->fwExtraVer, dpconf->fwBuildVer);

                if (strcmp(current->eltname,"BuildNumber")==0)
                    sprintf(out,"%d",dpconf->fwBuildVer);
                if (strcmp(current->eltname,"MajorVersion")==0)
                    sprintf(out,"%d",dpconf->fwMajorVer);
                if (strcmp(current->eltname,"MinorVersion")==0)
                    sprintf(out,"%d",dpconf->fwMinorVer);
                if (strcmp(current->eltname,"RevisionNumber")==0)
                    sprintf(out,"%d",dpconf->fwExtraVer);
            }
            else
            {
                if (strcmp(current->eltname,"MajorVersion")==0)
                    sprintf(out,"%d",major);
                if (strcmp(current->eltname,"MinorVersion")==0)
                    sprintf(out,"%d",minor);
                if (strcmp(current->eltname,"RevisionNumber")==0)
                    sprintf(out,"%d",revision);
                if (strcmp(current->eltname,"BuildNumber")==0)
                    sprintf(out,"%d",build);
            }
            if (strcmp(current->eltname,"ElementName")==0)
                sprintf(out,"%s:%d",p[i].data,index);
            if (strcmp(current->eltname,"InstanceID")==0)
                sprintf(out,"%s:%d",p[i].data,index);
            if ((p==_OperatingSystem || index >1) && data!=NULL )
                start_ptr=strstr(data,current->eltname);
            else
                start_ptr=NULL;
            if(start_ptr)
            {
                start_ptr=start_ptr+strlen(current->eltname);
                if (*start_ptr=='=')
                {
                    end_ptr=strchr(start_ptr+1,',');
                    if (end_ptr)
                    {
                        tmp_ch=*end_ptr;
                        *end_ptr='\0';
                        strcpy(out,start_ptr+1);
                        if (strcmp(current->eltname, "OSType")==0)
                        {
                            p[i].data = ostype;
                            strcpy(ostype, start_ptr+1);
                        }
                        if (strcmp(current->eltname, "Name") == 0 && p !=_SoftwareIdentity)
                        {
                            p[i].data = osname;
                            strcpy(osname, start_ptr+1);
                        }
                        *end_ptr=tmp_ch;
                    }
                    else
                        strcpy(out,start_ptr+1);
                }
            }
            if (strcmp(current->eltname,"Classifications")==0)
            {
                strcpy(out,"XX");
                pclass=out;
            }
            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j> PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }
            if (strcmp(current->eltname,"IdentityInfoType")==0)
            {
                if (strstr(current->data,"Dash Firmware"))
                    current_class=10;
                else if (strstr(current->data,"Bios Firmware"))
                    current_class=6;
                else if (strstr(current->data,".sys"))
                    current_class=2;
                else
                    current_class=4;
            }

            current->key=p[i].key;

            current->used=p[i].used;
            current->idx=p[i].idx;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;

            i++;
        }
        if (pclass!=NULL)
        {
            /*
            if (current_class==10)
                memcpy(pclass,"10",2);
            else if (current_class==2)
                memcpy(pclass,"2",2);
            else
                memcpy(pclass,"4",2);
                */
            ltostr(current_class,pclass);
        }
        if (data_end)
            *data_end='#';
        val=0;
        if (index==0 && p==_SoftwareIdentity)
            val=1;
        else if (data==NULL|| p==_OperatingSystem)
            break;

        else if (index==1 && p==_SoftwareIdentity)
            data_end=strchr(data,'#');
        else
        {
            data_end=strchr(data+1,'#');
            if (data_end)
            {
                *data_end='\0';
                data=data_end+1;
            }
        }

        if (data_end)
            val=1;
        if (index==25)
            val=0;  

        if (val)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname= NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
            index++;
            continue;
        }
        break;
    }
	
	
	
if(dcb != NULL ){	//+briank.rtk
    dcb->InstanceCount=index+1;
}
    return head;
}

unsigned int GetTotalValue(INT8U type, INT8U idx,INT8U *out)
{
    unsigned char *addr,*curLoc;
    unsigned long long uVal=0,TotalValue=0;
    unsigned char val=0;
    SMBIOSHeader sh = {0};
    addr = getSMBIOSTypeAddr(type);
    toSMBIOSHeader(&sh, addr);
    do
    {
        getSMBIOSElement(addr, &sh, type, idx, out);
        uVal=0;
        sscanf(out,"%llu MB",&uVal);
        TotalValue=TotalValue+uVal;
        curLoc = addr;
        val=getSMBIOSTypeNext(curLoc, type, &addr);
    } while (val >0);

    return TotalValue;

}
plist *DefaultSensorInstance(DASHCB *dcb,XMLElt *p, INT8U *pdata)
{
    INT16U i=0,j=0,index=0;
    INT8U sensor_num=0;
    plist *head=NULL,*current=NULL,*prev=NULL;
    unsigned char *out = pdata;
    //int exist=0,next_flag=0;
    int next_flag = 0;

    if (bsp_get_sstate() == S_S5)// || exist == 0)
        strcpy(state,"3");
    else
        strcpy(state,"2");

    DEBUGMSG(MESSAGE_DEBUG, "ACPI is %d\n", REG8(0xB2000014));
    DEBUGMSG(MESSAGE_DEBUG, "Str is %s\n", state);

    for (sensor_num = 0 ; sensor_num < dpconf->numofsnr ; sensor_num++)
    {

        //only enumerate the existing fans with actual value

        if (sensor[sensor_num].exist != SNR_READ &&  sensor[sensor_num].exist != SNR_EXIST)
            continue;

	if(dcb){
	        if((strcmp(dcb->ref_uri,"CIM_Fan")==0 || strcmp(dcb->r_uri,"CIM_Fan")==0) && sensor[sensor_num].type != SNR_TACHOMETER)
	            continue;
	}else{
		if(((p==_Fan)&& sensor[sensor_num].type != SNR_TACHOMETER)){ // This is for Web Manager Interface 
			continue;
		}
	}
//        gethealthstr(sensor[sensor_num].fault, health_state);
//       getstatestr(sensor[sensor_num].state, current_state);

        if (next_flag)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
            index++;
        }


        i=0;
        while (p[i].eltname)
        {
            	
            if(sensor[sensor_num].type & 0x80)
            {
              if(strcmp(p[i].eltname,"CurrentReading")==0 || strcmp(p[i].eltname,"BaseUnits")==0
              || strcmp(p[i].eltname,"LowerThresholdNonCritical") == 0
              || strcmp(p[i].eltname,"UpperThresholdNonCritical") == 0
              || strcmp(p[i].eltname,"LowerThresholdCritical") == 0
              || strcmp(p[i].eltname,"UpperThresholdCritical") == 0
              || strcmp(p[i].eltname,"LowerThresholdFatal") == 0
              || strcmp(p[i].eltname,"UpperThresholdFatal") == 0)                
              {
                i++;
                continue;
              }  
            }    
             
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;
            current->data=p[i].data;
            if (p[i].key==1)
            {
                if (strcmp(current->eltname,"DeviceID")==0)
                    sprintf(out,"%s:%d",current->data,index);
            }
            if(strcmp(current->eltname,"LowerThresholdNonCritical") == 0)
                sprintf(out,"%u",sensor[sensor_num].LNC);
            if(strcmp(current->eltname,"UpperThresholdNonCritical") == 0)
                sprintf(out,"%u",sensor[sensor_num].UNC);
            if(strcmp(current->eltname,"LowerThresholdCritical") == 0)
                sprintf(out,"%u",sensor[sensor_num].LC);
            if(strcmp(current->eltname,"UpperThresholdCritical") == 0)
                sprintf(out,"%u",sensor[sensor_num].UC);
            if(strcmp(current->eltname,"LowerThresholdFatal") == 0)
                sprintf(out,"%u",sensor[sensor_num].LF);
            if(strcmp(current->eltname,"UpperThresholdFatal") == 0)
                sprintf(out,"%u",sensor[sensor_num].UF);

	        if(strcmp(current->eltname,"UnitModifier") == 0){

			#if (defined CONFIG_BIOS_DELL) && (defined CONFIG_VENDOR_DELL)
				sprintf(out,"%d",0);
			#else	
		        	sprintf(out,"%d",sensor[sensor_num].offset[1]);
			#endif

				
	        }

            if(strcmp(current->eltname,"HealthState") ==0)
                sprintf(out,"%d", sensor[sensor_num].fault);
            if(strcmp(current->eltname,"CurrentState") ==0)
                strcpy(out, snrstatestr[sensor[sensor_num].state]);
            //strcpy(out,"Normal");

            //     if (p == _Fan && strcmp(current->eltname,"DesiredSpeed")==0)
            //         sprintf(out,"%u",sensor[sensor_num].value);

            if(p == _NumericSensor && strcmp(current->eltname,"CurrentReading")==0)
            {
                if (bsp_get_sstate() == S_S5)// || exist == 0)
                    sprintf(out,"%d",0);
                else
                {

                    if(sensor[sensor_num].signvalue)
                        sprintf(out,"%d",sensor[sensor_num].value);
                    else
                        sprintf(out,"%u",sensor[sensor_num].value);
                }
            }
            if(strcmp(current->eltname,"ElementName")==0)
                strcpy(out, sensor[sensor_num].name);

            if(strcmp(current->eltname,"SensorType")==0)
                sprintf(out,"%d",sensor[sensor_num].type & 0x7F);
            if(strcmp(current->eltname,"BaseUnits")==0)
                strcpy(out, snrtbl[sensor[sensor_num].type & 0x7F]);
            /*
                        {
                          if(sensor[sensor_num].type == SNR_FAN)
                            strcpy(out,"19");
                          if(sensor[sensor_num].type == SNR_TEMPERATURE)
                            strcpy(out,"2");
                        }
            */
            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j>PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }

            current->key=p[i].key;

            current->used=p[i].used;
            current->idx=p[i].idx;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;

            i++;
        }
        next_flag=1;
    }

    //fix if only fan has polled, but sensor has not
    //sensor profile would be empty

    if (head == NULL)
    {
        i=0;
        strcpy(state,"3");
        while (p[i].eltname)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;
            current->data=p[i].data;
            if (p[i].key==1)
            {
                if (strcmp(current->eltname,"DeviceID")==0)
                    sprintf(out,"%s:%d",current->data,index);
            }

            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j>PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }
            current->key=p[i].key;

            current->used=p[i].used;
            current->idx=p[i].idx;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;

            i++;
        }

    }

    if(dcb){
    	dcb->InstanceCount=index+1;
    }
    return head;
}
plist *DefaultPrivilegeInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata)
{
    INT16U i=0,j=0,index=0,idx=0,count=0;
    RoleInfo *role_p=rolehead;
    INT8U next=0;
    INT32U mask=0xffffffff;
    RegProfile *pf=_RegProfile;
    plist *head=NULL,*current=NULL,*prev=NULL;
    unsigned char *out = pdata;

	

    idx=dcb->ref_count[0];
    if (idx!=0xff)
    {
        while(role_p)
        {
            if(idx==count)
            {
                mask=role_p->privilege;
                break;
            }
            count++;
            role_p=role_p->next;
        }
    }
    idx=0;

	//printf(" + DefaultPrivilegeInstance , role_p->privilege=0x%x\n",role_p->privilege);
	
    while (pf[idx].eltname)
    {
        if(!(pf[idx].priv & mask))
        {
            idx++;
            continue;
        }
        if (next==1)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
        }


        i=0;
        while (p[i].eltname)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;
            current->data=p[i].data;
            if (strcmp(current->eltname,"InstanceID")==0)
                sprintf(out,"%s:%d",current->data,idx);
            if (strcmp(current->eltname,"ActivityQualifiers")==0)
                strcpy(out,pf[idx].priv_name);
            if (strcmp(current->eltname,"ElementName")==0)
                sprintf(out,"%s Execute",pf[idx].priv_name);

            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j>PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }

            current->key=p[i].key;
            current->used=p[i].used;
            current->idx=p[i].idx;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;

            i++;
        }

        idx++;
        next=1;
    }
    dcb->InstanceCount=index+1;
    return head;
}
plist *DefaultElementConformsInstance(DASHCB *dcb,INT8U *str, INT8U *pdata)
{
    INT16U i=0,j=0,k=0,index=0;
    plist *head=NULL,*current=NULL,*prev=NULL;
    unsigned char *out = pdata;
    XMLElt *p=_ElementConformsToProfile;
    strcpy(access_info,"");
    k=dcb->InstanceCount;
	
    while(1)
    {
        i=0;
        while (p[i].eltname)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;
            current->data=p[i].data;

            if (strcmp(current->eltname,"ManagedElement")==0)
            {
                // strcpy(out,pf[dcb->RegClass].eltname);
                strcpy(out,str);
                current->idx=index;
            }

            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j>PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }

            current->key=p[i].key;
            current->used=p[i].used;
            if (strcmp(current->eltname,"ConformantStandard")==0)
                current->idx=dcb->RegClass;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;

            i++;
        }
        k--;
        if (k)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
            index++;
        }
        else
            break;
    }
    dcb->InstanceCount=index+1;
    return head;
}
plist *DefaultRegisterInstance(XMLElt *p,INT8U *pdata)
{
    INT16U i=0,j=0,k=0,index=0;
    plist *head=NULL,*current=NULL,*prev=NULL;
    unsigned char *out = pdata;
    RegProfile *pf=_RegProfile;
    do
    {
        i=0;
        while (p[i].eltname)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;
            current->data=p[i].data;
            if (strcmp(current->eltname,"InstanceID")==0)
                sprintf(out,"%s:%d",current->data,index);

            if (strcmp(current->eltname,"ElementName")==0)
                sprintf(out,"%s Profile",pf[k].addr);
            if (strcmp(current->eltname,"RegisteredName")==0)
                strcpy(out,pf[k].addr);
            if (strcmp(current->eltname,"RegisteredVersion")==0)
                if (pf[k].info) strcpy(out,pf[k].info);

            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j>PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }

            current->key=p[i].key;
            current->used=p[i].used;
            current->idx=p[i].idx;
            if (strcmp(current->eltname,"Antecedent")==0 && p==_ReferencedProfile)
                current->idx=index;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;

            i++;
        }

        k++;
        while (!pf[k].priv)
        {
            if (!pf[k].eltname)
                break;
            k++;
        }
        if (pf[k].eltname)
        {
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
            index++;
        }
    } while (pf[k].eltname);

    return head;
}

plist *DefaultBiosInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata)
{
    INT8U i=0,index=0;
    INT16U j =0,k=0,len=0;
    plist *head=NULL,*current=NULL,*prev=NULL;
    INT8U *out=pdata;
    BiosAttr *ptr ;
    INT8U attr_val_num = 0;
    OS_CPU_SR cpu_sr;

    OS_ENTER_CRITICAL();
    memcpy(&pldmdata, pldmdataptr, sizeof(pldmdata));
    OS_EXIT_CRITICAL();

    if(pldmdata.TBL1 | pldmdata.TBL2 | pldmdata.TBL3 | !PLDMptr)
    {
        pldmdataptr->TBL1 = 0;
        pldmdataptr->TBL2 = 0;
        pldmdataptr->TBL3 = 0;

        boot_seq[0]=0xff;
        if (dpconf->bios == AMIBIOS && dpconf->biostype == LEGACY)
        {
            for (i=0; i<16; i++)
                boot_seq[i]=i;
        }
        //   OSTimeDly(3*OS_TICKS_PER_SEC);
        OS_ENTER_CRITICAL();
        PLDMptr = InitialPLDM();
        OS_EXIT_CRITICAL();
    }

    ptr=PLDMptr;
    i=0;
    while(ptr)
    {
        attr_val_num=ptr->attr_possible_num;
        while (p[i].eltname)
        {
            //      if(ptr->attr_type!=0)
            //        break;
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));

            current->eltname=p[i].eltname;

            current->data=p[i].data;

            // if (p[i].idx >0)
            // {

            if (strcmp(current->eltname,"InstanceID")==0)
                sprintf(out,"Attribute:%d",index);

            if(strcmp(current->eltname,"AttributeName")==0)
                getPLDMElement(ptr,0,index,out);
            else if(strcmp(current->eltname,"CurrentValue")==0  && (ptr->attr_type==1 || ptr->attr_type==2))
            {
            	if(ptr->attr_type == BIOSPassword)
            	  strcpy(out,"******");
            	else  
                {
                  len=*(ptr->attr_val_handle+3)+(*(ptr->attr_val_handle+4) <<8);
                  strncpy(out,ptr->attr_val_handle+5,len);
                }                
            }
            else if(strcmp(current->eltname,"CurrentValue")==0)
            {
                if(ptr->attr_val_index != NULL)
                    getPLDMElement(ptr,1,*ptr->attr_val_index,out);
            }

            else if(strcmp(current->eltname,"DefaultValue")==0)
            {
                if(ptr->attr_val_index != NULL)
                    getPLDMElement(ptr,1,*ptr->attr_default_index,out);
            }

            else if (strcmp(current->eltname,"PossibleValues")==0)
            {
                if(attr_val_num >0)
                {
                    getPLDMElement(ptr,1,k,out);
                    attr_val_num--;
                    k++;
                }
                if(attr_val_num==0)
                    k=0;

            }
            out= trimspace(out);
            if (strlen(out))
            {
                current->data=out;
                j=j+strlen(out)+1;
                if (j> PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem(dcb);
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }

            // }
            current->key=p[i].key;

            current->used=p[i].used;
            current->idx=p[i].idx;
            current->next=NULL;


            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;
            if(k ==0)
                i++;

        }
        i=0;

        //ptr->attr_val_possible_num--;
        do
        {
            ptr=ptr->next;
            if(!ptr)
                break;
        } while(!ptr->attr_val_handle);


        if (ptr)
        {
            if(ptr->attr_type==Pldm_Attr_Bootconfig)
                break;
            index++;

            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=NextInstance;
            current->next=NULL;
            if (head == NULL)
                head = current;
            else
                prev->next = current;
            prev = current;
        }
    }

    return head;
}
plist *DefaultInstance(DASHCB *dcb,XMLElt *p,INT8U *pdata)
{
    INT16U i=0,j=0,k=0,index=0;
    plist *head=NULL,*current=NULL,*prev=NULL;
    unsigned char *addr=NULL,*curLoc,*out = pdata;
    unsigned char val=0;
    SBU64 u64;
    //out=malloc(1024);
    unsigned long long uVal=0;
    //INT8U tmp;
    INT8U *tmpbuf;
    INT16U speed;
    int port,opaque_count = 0;
    if (p==NULL)
        return NULL;
    SMBIOSHeader sh = {0};	
	
//    if (p==_ElementConformsToProfile)
//        return DefaultElementConformsInstance(dcb,p,pdata);
    if(dcb){
    	dcb->InstanceCount=0;
    }
    if (p==_RegisteredProfile || p==_ReferencedProfile)
        return DefaultRegisterInstance(p,pdata);
    if (p==_Role)
        return DefaultRoleInstance(dcb,p,pdata);

    if (p==_Privilege)
        return DefaultPrivilegeInstance(dcb,p,pdata);
    if (p==_OperatingSystem || p==_SoftwareIdentity)
        return  DefaultInbandInstance(dcb,p,pdata);

    if ((pldmdataptr->ptr[0] && p==_BootSourceSetting)
            || (pldmdataptr->ptr[0] && p==_OrderedComponent))
    {
        dcb->Ptype=Pldm_Attr_Bootconfig;
        return DefaultPldmInstance(dcb,p,pdata);
    }
    if(p==_BIOSAttribute)
        return DefaultBiosInstance(dcb,p,pdata);
    //if(p==_BootSourceSetting || p==_OrderedComponent)
    //   return DefaultBootOrderInstance(p,pdata);

    if (p==_NumericSensor || p==_Fan)
        // if (p==_NumericSensor)
    {
        /*
        GPIOGet(&tmp);
        if (tmp==1)
            strcpy(current_state,"Open");
        else
            strcpy(current_state,"Closed");
        */
        return DefaultSensorInstance(dcb,p,pdata);
    }
    if (p == _Account || p==_Identity)
        return DefaultAccountInstance(dcb,p,pdata);
    if (p == _LogEntry)
        return DefaultLogInstance(dcb,p,pdata);
    // if ((p == _IndicationFilter || p==_ListenerDestination) && subhead!=NULL)
    if (p == _IndicationFilter || p==_ListenerDestinationWSManagement || p==_FilterCollectionSubscription || p == _IndicationSubscription)
        return DefaultIndicationInstance(dcb,p,pdata);
    if (p== _AssociatedPowerManagementService)
    {
        strcpy(power_state, pwrtbl[bsp_get_sstate()]);
        if (strcmp(power_state, "2"))
            strcpy(enabled_state, "3");
        else
            strcpy(enabled_state, "2");
    }
    if (p== _TextRedirectionSAP || p==_TextRedirectionService)
    {
        if (tcrcb.state == CONNECTED)
            strcpy(state, "6");
        else
            strcpy(state, "3");
    }
    if (p== _USBRedirectionSAP || p==_USBRedirectionService)
    {
        if (usbcb.usbstate == CONNECTED)
            strcpy(state, "6");
        else
            strcpy(state, "3");
    }
    if (p==_AssociatedCacheMemory)
        dcb->Ptype=CACHE;

	
    if ((dcb)&&(dcb->Ptype >= 0))
    {
        addr = getSMBIOSTypeAddr(dcb->Ptype);
        toSMBIOSHeader(&sh, addr);
    }else if(dcb==NULL){
    	 addr = getSMBIOSTypeAddr(m_dashPtype);
        toSMBIOSHeader(&sh, addr);
    }
    tmpbuf = malloc(128);
    if (p==_OpaqueManagementData)
    {
        for(opaque_count=0; opaque_count<OpaqueNum; opaque_count++)
        {
            memset(tmpbuf,0,128);
            read_opaque(opaque_count,0,128,tmpbuf);
            if((((OPAQDATA*)tmpbuf))->Status==1)
                break;
        }
        if (opaque_count==OpaqueNum)
        {
            free(tmpbuf);
            return NULL;
        }

    }
    do
    {    	
        while (p[i].eltname)
        {
            if(p==_ComputerSystem)
            {
            	if(strcmp(p[i].eltname,"HealthState")==0)
            	{
            	  strcpy(out,"0");	
            	  if(bsp_get_sstate()!=S_UNKNOWN)
            	    strcpy(out,"5");
            	}    
            	  
            }	   	

            if(p==_PhysicalMemory)
            {
                getBIOSElemnetWord(addr, MEMDEV_SIZE, out);
                if(strstr(out,"0 "))
                    break;
            }
            if(p==_Chassis)
            {
#if 1
                if(dcb){
		if(strcmp(p[i].eltname,"Model")==0 || strcmp(p[i].eltname,"SKU")==0){
	                    dcb->Ptype=SYSTEM_INFO;
	          }
	          else{
	          	dcb->Ptype=CHASSIS;
	          }
		 addr = getSMBIOSTypeAddr(dcb->Ptype);
               	 toSMBIOSHeader(&sh, addr);	  
                }else{
                	if(strcmp(p[i].eltname,"Model")==0 || strcmp(p[i].eltname,"SKU")==0){
			m_dashPtype = SYSTEM_INFO;		
                	}else{
                		m_dashPtype=CHASSIS;
                	}
		addr = getSMBIOSTypeAddr(m_dashPtype);
               	 toSMBIOSHeader(&sh, addr);
                }
#else
if(strcmp(p[i].eltname,"Model")==0 || strcmp(p[i].eltname,"SKU")==0)
                    dcb->Ptype=SYSTEM_INFO;
                else
                    dcb->Ptype=CHASSIS;

                addr = getSMBIOSTypeAddr(dcb->Ptype);
                toSMBIOSHeader(&sh, addr);
#endif	     
            }            

            strcpy(out,"");
            if (strcmp(p[i].eltname,"1")==0)
                index++;
            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=p[i].eltname;

            current->data=p[i].data;

            if (p[i].idx >0 && addr !=NULL)
            {
                if (p[i].attr==1)
                    getBIOSElemnetByte(addr, p[i].idx, out);
                else if (p[i].attr==2)
                    getBIOSElemnetWord(addr, p[i].idx, out);
                else{
			if(dcb){				
                    	getSMBIOSElement(addr, &sh, dcb->Ptype, p[i].idx, out);
			}else{
				getSMBIOSElement(addr, &sh, m_dashPtype , p[i].idx, out);
			}
                }


                if ((dcb) && dcb->Ptype==BIOS_INFO && p[i].idx==BIOS_CHARS)
                {
                    k=0;
                    sscanf((char*)out, "%d %d", &u64.l, &u64.h);
                    getBIOSChar(u64, &sh, out);
                    k=k+strlen(out);
                    while (getBIOSChar(u64, &sh, out+k))
                        k=k+strlen(out);
                }
				
                if ((dcb) && dcb->Ptype==MEM_DEV && p[i].idx==MEMDEV_SIZE)
                {                	 
                    if (p==_Memory)
                    {
                        if (!uVal)
                            uVal=GetTotalValue(MEM_DEV,MEMDEV_SIZE,out);
                    }
                    else
                        sscanf(out,"%llu MB",&uVal);
                    if (uVal)
                        ltostr(uVal*1024*1024, out);
                    else
                        strcpy(out,"0");
                }
		   //+briank.rtk------------------------------------		
		   else if((!(dcb))&& m_dashPtype ==MEM_DEV&& p[i].idx==MEMDEV_SIZE){
                	if (p==_Memory)
                    {
                        if (!uVal)
                            uVal=GetTotalValue(MEM_DEV,MEMDEV_SIZE,out);
                    }
                    else
                        sscanf(out,"%llu MB",&uVal);
                    if (uVal)
                        ltostr(uVal*1024*1024, out);
                    else
                        strcpy(out,"0");
                }
		   //*briank.rtk------------------------------------
                if((dcb) && dcb->Ptype==CACHE && p[i].idx==CACHE_INSTALL_SIZE)
                {
                    sscanf(out,"%llu",&uVal);
                    ltostr(uVal*1024, out);
                }
		   //+briank.rtk------------------------------------			
		   else if((!(dcb))&& m_dashPtype ==CACHE&& p[i].idx==CACHE_INSTALL_SIZE){		   	
                	 sscanf(out,"%llu",&uVal);
                    ltostr(uVal*1024, out);
                }
		    //*briank.rtk------------------------------------
            }
		
            if (strcmp(current->eltname,"ElementName")==0){

			#if 1	
               	if((dcb) && dcb->Ptype == CACHE && p[i].idx==CACHE_SKT_DSEIGN){
		  		//printf("[RTK] %s \n",out);			
			}else if((!dcb) && m_dashPtype == CACHE && p[i].idx==CACHE_SKT_DSEIGN){
		  		//printf("[RTK] %s \n",out);
			}else{
				sprintf(out,"%s:%d",current->data,index);
			}
			#else
			sprintf(out,"%s:%d",current->data,index);
			#endif

            }

            if (p==_RecordLog || p==_FilterCollection)
            {
                if (strcmp(current->eltname,"ElementName")==0 || strcmp(current->eltname,"CollectionName")==0)
                {
                    if (index==0)
                        strcpy(out,"INFO");
                    else if (index==1)
                        strcpy(out,"HW");
                    else if (index==2)
                        strcpy(out,"USER");
                    else if (index==3)
                        strcpy(out,"WARNING");
                    else if (index==4)
                        strcpy(out,"ERROR");
                    else
                        strcpy(out,"ALL");
                }

            }

            if (p[i].key==1 && p!=_ComputerSystem)
            {
                if (strcmp(current->eltname,"InstanceID")==0 || strcmp(current->eltname,"DeviceID")==0
                        || strcmp(current->eltname,"Tag")==0 || strcmp(current->eltname,"Name")==0)
                    sprintf(out,"%s:%d",current->data,index);
            }
            if(p==_OpaqueManagementData)
            {
                if (strcmp(current->eltname,"ElementName")==0)
                    strcpy(out,((OPAQDATA*)tmpbuf)->ElementName);
                if (strcmp(current->eltname,"DeviceID")==0)
                    sprintf(out,"%s:%d",current->data,opaque_count);
            }
            //for the MS SCCM SOL
            if (p==_TextRedirectionSAP && strcmp(current->eltname,"Name")==0)
                sprintf(out,"%s:%d",current->data,index+1);
            if (p==_USBRedirectionSAP && strcmp(current->eltname,"ConnectionMode")==0)
            {
                if(strcmp(access_info,"")!=0)
                    strcpy(out,"3");
                else if(index==0)
                    strcpy(out,"3");
                else
                    strcpy(out,"2");
            }

            if (p==_USBRedirectionSAP && (strcmp(current->eltname,"EnabledState")==0 || strcmp(current->eltname,"RequestedState")==0))
                //  if (p==_USBRedirectionSAP && strcmp(current->eltname,"EnabledState")==0)
            {
                if (usbcb.usbstate == CONNECTED)
                    strcpy(state, "6");
                else
                    strcpy(state, "3");
            }

            if (p==_TCPProtocolEndpoint && strcmp(current->eltname,"PortNumber")==0)
            {
                if(index==0)
                    port=87;
                else if(index==1)
                    port=57;
                else
                    port=59;
                sprintf(out,"%d",port);
            }
            if (p==_TCPProtocolEndpoint && strcmp(current->eltname,"ProtocolIFType")==0)
            {
                if(index==1)
                    strcpy(out,"4401");
            }

            if (strlen(out))
            {
                if (strcmp(current->eltname,"AttributeName")==0)
                {
                    strcpy(tmpbuf,out);
                    sprintf(out,"CIM:%s:1",tmpbuf);
                }
                current->data=out;
                j=j+strlen(out)+1;
                if (j>PAYLOAD_SIZE_THD)
                {
                    pdata=NewMem();
                    j=0;
                }
                out=pdata+j;
                strcpy(out,"");
            }

            current->key=p[i].key;

            current->used=p[i].used;
            current->idx=p[i].idx;
            current->next=NULL;

            if (p==_BindsTo)
                current->idx=index;
            if (strcmp(current->eltname,"Antecedent")==0 && p==_AssociatedCacheMemory)
                current->idx=index+1;

            if (head == NULL)
                head = current;
            else
                prev->next = current;

            prev = current;

            i++;
        }
        i=0;
        val=0;
        if(p==_BindsTo && index==0)
            val=1;
        if(p==_USBRedirectionSAP && index < 3)
            val=1;
        if(p==_TCPProtocolEndpoint && index < 2)
            val=1;
        if(p==_TextRedirectionSAP && index==0)
            val=1;
        if (p==_OpaqueManagementData)
        {
            opaque_count++;
            for(; opaque_count<OpaqueNum; opaque_count++)
            {
                memset(tmpbuf,0,128);
                read_opaque(opaque_count,0,128,tmpbuf);
                if((((OPAQDATA*)tmpbuf))->Status==1)
                {
                    val=1;
                    break;
                }
            }
        }


        if ((p==_Memory) &&(dcb) &&(strcmp(dcb->ref_uri,"CIM_ComputerSystem")==0)) 
        //if (p==_Memory && strcmp(dcb->ref_uri,"CIM_ComputerSystem")==0)
            break;
        if ((p==_RecordLog || p==_FilterCollection) && index==NumFilterCollection)
            break;
        if (p==_RecordLog || p==_FilterCollection)
            val=1;
        else if (p==_Memory)
        {
            p=_CacheMemory;
	     if(dcb){
            	dcb->Ptype=CACHE;
		addr = getSMBIOSTypeAddr(dcb->Ptype);
	     }
	     else{
		m_dashPtype = CACHE;
		addr = getSMBIOSTypeAddr(m_dashPtype);
	     }
            toSMBIOSHeader(&sh, addr);
            val=1;

        }
        else if (dcb && dcb->Ptype >=0 && addr !=NULL)
        {
            curLoc = addr;
            val=getSMBIOSTypeNext(curLoc, dcb->Ptype, &addr);
        }else if((dcb == NULL)&&(m_dashPtype>=0)){
        	if(addr !=NULL){
			curLoc = addr;
            		val=getSMBIOSTypeNext(curLoc, m_dashPtype , &addr);
        	}
        }

        if(p==_PhysicalMemory)
        {
            while(val >0)
            {
                getBIOSElemnetWord(addr, MEMDEV_SIZE, out);
                if(!strstr(out,"0 "))
                    break;
                else
                {
                    curLoc = addr;
			
			if(dcb){
                    	val=getSMBIOSTypeNext(curLoc, dcb->Ptype, &addr);
			}else{
				val=getSMBIOSTypeNext(curLoc, m_dashPtype, &addr);
			}
                }
            }
        }

        if (val >0)
        {
            if(head==NULL)
                continue;

            current=malloc(sizeof(plist));
            memset(current, 0 , sizeof(plist));
            current->eltname=NextInstance;
            current->next=NULL;
            // if (head == NULL)
            //     head = current;
            // else
            prev->next = current;
            prev = current;
            index++;
        }

    } while (val >0);
    if(dcb){
    	dcb->InstanceCount=index+1;
    }
    free(tmpbuf);
    return head;
}
void InitialRegister()
{
    // Profile *pf=_RegProfile;
    // unsigned char *addr,i=0;

    privilege[2]=0x700003;
    privilege[1]=0x70000c;
    privilege[0]=0x3ffff0;
    /*
    while (pf[i].eltname)
    {
        if (pf[i].type)
        {
            addr = getSMBIOSTypeAddr(pf[i].type);
            if (!addr)
                pf[i].priv=0;
            else if (!*strltrim(addr+*(addr+1)))
                pf[i].priv=0;
            else
                pf[i].priv=1;
        }

        i++;
    }
    */

}

send_list* genXML(DASHCB *dcb,send_list *sendlist,plist *p)
//genXML (send_list *sendlist,XMLElt *p)
{
    INT8U *fragment_transfer = element[ID_FRAGMENT_TRANSFER].elementPtr;
    char *eltname,*data;
    int i,done=0;
    send_list *current;
    plist *currentx;
    INT8U *buf;
    int set_flag=0;

    current = new_send_list();
    sendlist-> next = current;

    buf = current->addr + PKT_HDR_SIZE;

    if (strcmp(p->eltname,"1")==0)
        p=p->next;
    dcb->StartOff=p;


    //DEBUGMSG(DASH_DEBUG, "genXML\n");
    while (p)
    {
        eltname=p->eltname;
        data=p->data;

//       if(strlen(buf) >= PAYLOAD_SIZE_THD)
        if (strlen(buf) >= PAYLOAD_SIZE_THD)
        {

            current->len = strlen(buf);
            current->next = new_send_list();

            current = current->next;
            buf = current->addr + PKT_HDR_SIZE;

        }

        if (fragment_transfer!=NULL)
        {
            fragment_transfer=strltrim(fragment_transfer);

            if (strncmp(fragment_transfer,eltname,strlen(eltname))==0)
            {
                sprintf(buf+strlen(buf),"<%s:XmlFragment><p:%s>%s</p:%s></%s:XmlFragment>",Ns[IDX_NS_WS_MAN].prefix,eltname,data,eltname,Ns[IDX_NS_WS_MAN].prefix);
                break;
            }
            p=p->next;
            continue;
        }

        if (dcb->association==-1 && p->key==1)
        {
            if ((dcb->epr & (EPR |ObjectAndEPR))>0 && set_flag==0)
            {
              sprintf(buf+strlen(buf),"<%s:SelectorSet>",Ns[IDX_NS_WS_MAN].prefix);
              set_flag=1;
            }  
            
            if ((dcb->epr & (EPR |ObjectAndEPR))>0)
                sprintf(buf+strlen(buf),"<%s:Selector Name=\"%s\"><%s:EndpointReference><%s:Address>%s</%s:Address>",Ns[IDX_NS_WS_MAN].prefix,eltname,Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_ADDRESSING].prefix,WSA_TO_ANONYMOUS,Ns[IDX_NS_ADDRESSING].prefix);
            else
                sprintf(buf+strlen(buf),"<p:%s><%s:Address>%s</%s:Address>",eltname,Ns[IDX_NS_ADDRESSING].prefix,WSA_TO_ANONYMOUS,Ns[IDX_NS_ADDRESSING].prefix);
            sprintf(buf+strlen(buf),"<%s:ReferenceParameters><%s:ResourceURI>%s/%s</%s:ResourceURI>",Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_WS_MAN].prefix,XML_NS_CIM_CLASS,data,Ns[IDX_NS_WS_MAN].prefix);

            currentx=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb, data),TmpNewMem());
            current=SelectElement(current,buf,currentx,p->idx);
            buf = current->addr + PKT_HDR_SIZE;
            TmpReleaseBuf();
            currentx = ReleaseProfile(currentx);
            if ((dcb->epr & (EPR | ObjectAndEPR))>0)
                sprintf(buf+strlen(buf),"</%s:ReferenceParameters></%s:EndpointReference></%s:Selector>",Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_WS_MAN].prefix);
            else
                sprintf(buf+strlen(buf),"</%s:ReferenceParameters></p:%s>",Ns[IDX_NS_ADDRESSING].prefix,eltname);
            p=p->next;
            dcb->StartOff=p;
            continue;
        }
        if((dcb->epr & (EPR | ObjectAndEPR)) >0 && dcb->association!=-1 && done==0)
        {
            current=SelectElement(current,buf,dcb->StartOff,0);
            buf = current->addr + PKT_HDR_SIZE;
            done=1;
        }

        // if (strcmp(eltname,"1")==0 && dcb->actiontype!=WS_DISP_TYPE_ENUMERATE)
        if (strcmp(eltname,"1")==0)
            break;
        // if ((Ptype==-1 && dcb->epr!=1) || (dcb->Ptype!=-1))
        if ((dcb->epr & (EPR | ObjectAndEPR))==0 || dcb->association!=-1)
        {

            if (SubClassProperties==1)
                if (strcmp(dcb->r_uri,"CIM_System")==0 && strcmp(eltname,"Dedicated")==0)
                {
                    SubClassProperties=0;
                    i++;
                    continue;
                }

            sprintf(buf+strlen(buf),"<p:");
            sprintf(buf+strlen(buf),eltname);
            sprintf(buf+strlen(buf),">");
            sprintf(buf+strlen(buf),data);
            sprintf(buf+strlen(buf),"</p:");
            sprintf(buf+strlen(buf),eltname);
            sprintf(buf+strlen(buf),">");

        }
        p=p->next;
        dcb->StartOff=p;
        i++;
    }
    if ((dcb->epr & (EPR |ObjectAndEPR))>0 && set_flag==1)
       sprintf(buf+strlen(buf),"</%s:SelectorSet>",Ns[IDX_NS_WS_MAN].prefix);
   
    current->len = strlen(buf);
    return current;

}
INT8U* ParserNsData(DASHCB *dcb,INT8U* str)
{

    INT32S i,len;
    INT8U *prefix_start,*prefix_end;
    INT8U *str_end;
    INT8U token[] = ":Header";
    INT8U nsheader[] = "xmlns:";
    INT8U found=0;
    for(i=0; i<14; i++)
        strcpy(Ns[i].prefix,"");
    str_end = strstr(str, token);
    //Ns data appears only before the header tag

    assert(str_end);
    //in case if header error

    while (str < str_end)
    {
        found=0;
        str = strstr(str,nsheader);

        if (str == NULL || str > str_end)
            //return (str_end + strlen(token) + 1);
            return (str_end + strlen(token) -1 );

        prefix_start = str + strlen(nsheader);
        prefix_end = strchr(str, '=');

        assert(prefix_end);

        str = prefix_end;

        while (isspace(*str++) || *str == '"' )
            ;
        //ignore any space until find a match ' " ' character

        i = 0 ;
        while (Ns[i].uri)
        {
            len = strlen(Ns[i].uri);

            if (strncmp(str, Ns[i].uri, len) == 0)
            {
                Ns[i].used = 1;
                prefix_start = strltrim(prefix_start);
                prefix_end = strrtrim(prefix_end);
                len = prefix_end - prefix_start;
                strncpy(Ns[i].prefix, prefix_start,len);
                found=1;
                break;
            }
            i++;
        }
        //    if(found==0)
        //      dcb->fault_id=Fault_InvalidNamespace;



    }
    //return (str_end + strlen(token) + 1);
    return (str_end + strlen(token) - 1);

}


struct _WSaction
{
    char *wsaction;
    int wstype;
}
e_action[]=
{
    {TRANSFER_ACTION_GET,WS_DISP_TYPE_GET},
    {TRANSFER_ACTION_PUT,WS_DISP_TYPE_PUT},
    {TRANSFER_ACTION_CREATE,WS_DISP_TYPE_CREATE},
    {TRANSFER_ACTION_DELETE,WS_DISP_TYPE_DELETE},
    {ENUM_ACTION_ENUMERATE,WS_DISP_TYPE_ENUMERATE},
    {ENUM_ACTION_RELEASE,WS_DISP_TYPE_RELEASE},
    {ENUM_ACTION_PULL,WS_DISP_TYPE_PULL},
    {ENUM_ACTION_RENEW,WS_DISP_TYPE_UPDATE},
    {XML_NS_CIM_CLASS,WS_DISP_TYPE_INVOKE},
    {EVT_ACTION_SUBSCRIBE,WS_DISP_TYPE_SUBSCRIBE},
    {EVT_ACTION_UNSUBSCRIBE,WS_DISP_TYPE_UNSUBSCRIBE},
    {EVT_ACTION_GETSTATUS,WS_DISP_TYPE_EVT_GETSTATUS},
    {EVT_ACTION_RENEW,WS_DISP_TYPE_EVT_RENEW},
    {0,0}
};
INT8U PutBodyElement(DASHCB *dcb,INT8U *str)
{

    INT8U token[] = ":Body>";

    INT8U wsheader[] = "<";
    INT8U wstailer[] = ">";
    INT8U *str_end, *cmp_end, *ptr,*out;
    //INT8U tmpbuf[64];
    INT8U *tmpbuf;
    INT16U tmp;
    INT8U val;
    plist *current,*current1;
    int len=0,ret=0,i=0,j=0;

    dcb->fault_id = Fault_NoFault;
    str_end = strstr(str,token);
    if (!str_end)
        return 0;
    assert(str_end);
    *str_end = NULLCHR;

    tmpbuf = malloc(64);

    out=NewMem();
    while (str)
    {
        str = strstr(str,wsheader);  //find a leading '<'

        if (str == NULL || str >= str_end)
            break;

        ptr = strstr(str,wstailer);  //find a following '>'
        if (!ptr)
            break;
        *ptr = NULLCHR;
        ptr=strltrim(ptr+1);
        if (*ptr=='<')
        {
            str=ptr;
            continue;
        }
        cmp_end  = ptr;


        str = strchr(str, ':') + 1;  //wxxx:

        ptr = strchr(cmp_end,'<');  //find a leading '<'
        *ptr=NULLCHR;
        ret=SetElement(dcb->StartOff,str,cmp_end,out+len);
        len=len+strlen(cmp_end)+1;
        str=ptr+1;

    }
    if (strcmp(dcb->r_uri,"CIM_TextRedirectionSAP")==0)
    {
        current= GetElement(dcb->StartOff,"SessionTerminateSequence",0);
        bootsel = NULL;
        tcrcb.bootopt = 0;
        for (i= 0; i < sizeof(bootopts)/sizeof(bootopts[0]); i++)
        {
            if (strcmp(current->data, bootopts[i].name) == 0)
            {
                bootsel = &bootopts[i];
                break;
            }
        }
    }

    if (strcmp(dcb->r_uri,"CIM_StaticIPAssignmentSettingData")==0)
    {
    	if(bsp_in_band_exist()){
			
		dcb->fault_id = Fault_ActionFault;
		printf("[RTK] return Fault_ActionFault\n");
		
	}else{
        current= GetElement(dcb->StartOff,"IPv4Address",0);
        strcpy(local_ip,current->data);
        strcpy(new_ip,current->data);
        current= GetElement(dcb->StartOff,"SubnetMask",0);
        strcpy(local_netmask,current->data);
        strcpy(new_netmask,current->data);
        current= GetElement(dcb->StartOff,"GatewayIPv4Address",0);
        strcpy(local_gateway,current->data);
        strcpy(new_gateway,current->data);
        dcb->control=NotReady;
    	}
    }

    if (strcmp(dcb->r_uri,"CIM_Account")==0)
    {
        current= GetElement(dcb->StartOff,"UserID",0);
        current1=GetElement(dcb->StartOff,"UserPassword",0);
        if ((dcb->userinfo->role==1 || dcb->userinfo->role==0x7f) && strcmp(current1->data,"***")!=0)
        {
            if (strstr(current1->data,"0x"))
            {
                memset(tmpbuf,0,64);
                tmp=str2hex((INT8U *)(current1->data+8));
                for (i=0,j=0; j<(tmp-4); j++)
                {
                    tmpbuf[j]=str2hex(current1->data+10+i);
                    i=i+2;
                }
                moduser(current->data,tmpbuf);
            }
            else
                moduser(current->data,current1->data);
        }
    }

    if (strcmp(dcb->r_uri,"CIM_Sensor")==0)
    {
        if (GPIOGet(&val))
        {
            current= GetElement(dcb->StartOff,"PossibleStates",0);
            if (strcmp(current->data,"Open")==0)
                GPIOSet(1);
            else
                GPIOSet(0);
        }

    }
    if (strcmp(dcb->r_uri,"CIM_RemoteServiceAccessPoint")==0)
    {
        current= GetElement(dcb->StartOff,"AccessInfo",0);
        strcpy(access_info,current->data);
    }

    free(tmpbuf);
    return dcb->fault_id;
}

void gen_nsdata(DASHCB *dcb,INT8U *buf)
{

    INT32S i = 0;

    if (dcb->fault_id)
        strcpy(buf,hello_err);
#ifdef CONFIG_MSAD_ENABLED        
    else if(dcb->ad_nego==1)
    {
      strcpy(buf,hello_nego);
      addADauth(buf+strlen(buf), &dcb->cs->pcb->msad.auth_buffer);
    } 
#endif         
    else
        strcpy(buf,hello);

    strcat(buf+strlen(buf),"<s:Envelope");


    while (Ns[i].uri)
    {
        if (Ns[i].used)
        {
            if (i==IDX_NS_SOAP_1_2)
                sprintf(buf,"%s xmlns:s=\"%s\"",buf,Ns[i].uri);
            else if (i==IDX_NS_CIM_CLASS)
                sprintf(buf,"%s xmlns:%s=\"%s/%s\"",buf,Ns[i].prefix,Ns[i].uri,dcb->r_uri);
            else
                sprintf(buf,"%s xmlns:%s=\"%s\"",buf,Ns[i].prefix,Ns[i].uri);
        }
        i++;
    }

}

void gen_action(DASHCB *dcb,INT8U *buf)
{
    INT8U *action     = element[ID_ACTION].elementPtr;
    INT8U *message_id = element[ID_MESSAGE_ID].elementPtr;
    INT8U *fragment_transfer = element[ID_FRAGMENT_TRANSFER].elementPtr;
    sprintf(buf+strlen(buf),"><s:Header>");
    sprintf(buf+strlen(buf),"<%s:Action>",Ns[IDX_NS_ADDRESSING].prefix);


    if (!dcb->fault_id && action != NULL)
        strcat(buf,action);

    if (!dcb->fault_id)
        sprintf(buf+strlen(buf),"%s",WSFW_RESPONSE_STR);
    else
        sprintf(buf+strlen(buf),"%s",fault[dcb->fault_id-1].fault_action);

    sprintf(buf+strlen(buf),"</%s:Action>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"<%s:To>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"%s",WSA_TO_ANONYMOUS);
    sprintf(buf+strlen(buf),"</%s:To>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"<%s:RelatesTo>",Ns[IDX_NS_ADDRESSING].prefix);

    if (message_id != NULL)
        strcat(buf,message_id);

    sprintf(buf+strlen(buf),"</%s:RelatesTo>",Ns[IDX_NS_ADDRESSING].prefix);

    if (fragment_transfer!=NULL)
        sprintf(buf+strlen(buf),"<%s:FragmentTransfer>%s</%s:FragmentTransfer>",Ns[IDX_NS_WS_MAN].prefix,fragment_transfer,Ns[IDX_NS_WS_MAN].prefix);
    sprintf(buf+strlen(buf),"<%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
}
void wsman_identify_stub(DASHCB *dcb)
{

    static const char str1[]="http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd";
    static const char str2[]="Dash Project";
    static const char str3[]="1.0.0";
    static const char str4[]="1.0.0";
    static const char str5[]="http://schemas.dmtf.org/wbem/wsman/1/wsman/secprofile/http/digest";
    static const char str6[]="http://schemas.dmtf.org/wbem/wsman/1/wsman/secprofile/https/digest";
    static const char str7[]="http://schemas.dmtf.org/wbem/wsman/1/wsman/secprofile/https/basic";
    INT8U *buf;
    send_list *current;
    INT8U ns[32]="";
    current = new_send_list();
    dcb->sendlist = current;
    buf = current->addr + PKT_HDR_SIZE;

    gen_nsdata(dcb,buf);
    if (Ns[IDX_NS_WSMAN_ID].used)
    {
        strcpy(ns,Ns[IDX_NS_WSMAN_ID].prefix);
        sprintf(buf+strlen(buf)," xmlns:dash=\"http://schemas.dmtf.org/wbem/dash/1/dash.xsd\">");
    }
    else
    {
        strcpy(ns,"wsmid");
        sprintf(buf+strlen(buf)," xmlns:wsmid=\"http://schemas.dmtf.org/wbem/wsman/identity/1/wsmanidentity.xsd\" xmlns:dash=\"http://schemas.dmtf.org/wbem/dash/1/dash.xsd\">");
    }
    current->len = strlen(buf);
    current = new_send_list();
    dcb->sendlist-> next = current;
    buf = current->addr + PKT_HDR_SIZE;

    sprintf(buf+strlen(buf),"<s:Header/><s:Body><%s:IdentifyResponse>",ns);
    sprintf(buf+strlen(buf),"<%s:ProtocolVersion>%s</%s:ProtocolVersion>",ns,str1,ns);
    sprintf(buf+strlen(buf),"<%s:ProductVendor>%s</%s:ProductVendor>",ns,str2,ns);
    sprintf(buf+strlen(buf),"<%s:ProductVersion>%s</%s:ProductVersion>",ns,str3,ns);
    sprintf(buf+strlen(buf),"<dash:DASHVersion>%s</dash:DASHVersion>",str4);
    sprintf(buf+strlen(buf),"<%s:SecurityProfiles><%s:SecurityProfileName>%s</%s:SecurityProfileName><%s:SecurityProfileName>%s</%s:SecurityProfileName><%s:SecurityProfileName>%s</%s:SecurityProfileName></%s:SecurityProfiles>",ns,ns,str5,ns,ns,str6,ns,ns,str7,ns,ns,ns);
    sprintf(buf+strlen(buf),"</%s:IdentifyResponse></s:Body></s:Envelope>",ns);

    current->len = strlen(buf);
    insertlen(dcb,dcb->sendlist, strlen(hello));
    dcb->ProfilePtr=NULL;
    dcb->StartOff=NULL;
    dcb->actiontype = -1;
    dcb->cs->pcb->httpAuthPass = UNPASS;

}

void create_release_response_envelope(DASHCB *dcb)
{
    INT8U *buf;
    //send_list *sendlist;
    //char * reply_to=element[ID_REPLY_TO].elementPtr;

    dcb->sendlist = new_send_list();
    buf = dcb->sendlist->addr + PKT_HDR_SIZE;

    gen_nsdata(dcb,buf);
    gen_action(dcb,buf+strlen(buf));


    sprintf(buf+strlen(buf),"%s","00000002-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"</s:Header><s:Body/></s:Envelope>");

    dcb->sendlist->len = strlen(buf);
    insertlen(dcb,dcb->sendlist, strlen(hello));


    ///return sendlist;

}
void create_delete_response_envelope(DASHCB *dcb)
{
    INT8U *buf,i,j=-1,*tmpbuf;
    //send_list *sendlist;
    plist *currentx;
    currentx= GetElement(dcb->ProfilePtr,NULL,dcb->count);
    if (strcmp(dcb->r_uri,"CIM_Account")==0 && (dcb->userinfo->role==1 || dcb->userinfo->role==0x7f))
    {
        dcb->status=deluser(currentx->data);
        if (dcb->status==2)
        {
            dcb->fault_id=Fault_ActionFault;
            return;
        }

    }
    if (strcmp(dcb->r_uri,"CIM_OpaqueManagementData")==0)
    {
        tmpbuf=malloc(128);
        for(i=0; i<OpaqueNum; i++)
        {
            memset(tmpbuf,0,128);
            read_opaque(i,0,128,tmpbuf);
            if(((OPAQDATA*)tmpbuf)->Status==1)
                j++;
            if(j==dcb->count)
                break;
        }
        if(strcmp(dcb->userinfo->name,((OPAQDATA*)tmpbuf)->Owner)==0)
            delete_opaque(i);
        else
            dcb->fault_id=Fault_ActionFault;
        free(tmpbuf);
    }

    dcb->sendlist = new_send_list();
    buf = dcb->sendlist->addr + PKT_HDR_SIZE;

    gen_nsdata(dcb,buf);
    gen_action(dcb,buf+strlen(buf));


    sprintf(buf+strlen(buf),"%s","00000002-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"</s:Header><s:Body/></s:Envelope>");

    dcb->sendlist->len = strlen(buf);
    insertlen(dcb,dcb->sendlist, strlen(hello));

    dcb->StartOff=NULL;

}
void create_create_response_envelope(DASHCB *dcb)
{
    INT8U *buf, *ptr;
    send_list *sendlist;
    INT8U *action     = element[ID_ACTION].elementPtr;
    INT8U *message_id = element[ID_MESSAGE_ID].elementPtr;
    //char * reply_to=element[ID_REPLY_TO].elementPtr;

    sendlist = new_send_list();
    buf = sendlist->addr + PKT_HDR_SIZE;


    gen_nsdata(dcb,buf);

    sprintf(buf+strlen(buf)," xmlns:wxf=\"http://schemas.xmlsoap.org/ws/2004/09/transfer\"><%s:Header>",Ns[IDX_NS_SOAP_1_2].prefix);
//  sprintf(buf+strlen(buf),"><%s:Header>",Ns[IDX_NS_SOAP_1_2].prefix);
    sprintf(buf+strlen(buf),"<%s:Action s:mustUnderstand=\"true\">",Ns[IDX_NS_ADDRESSING].prefix);

    if (action!=NULL)
    {
        ptr = strchr(action,'<');
        strncat(buf, action, (ptr - action));
    }

    sprintf(buf+strlen(buf),"%s",WSFW_RESPONSE_STR);
    sprintf(buf+strlen(buf),"</%s:Action>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"<%s:To>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"%s",WSA_TO_ANONYMOUS);

    sprintf(buf+strlen(buf),"</%s:To>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"<%s:RelatesTo>",Ns[IDX_NS_ADDRESSING].prefix);
    if (message_id!=NULL)
    {
        ptr = strchr(message_id,'<');
        strncat(buf, message_id, (ptr-message_id));
    }

    sprintf(buf+strlen(buf),"</%s:RelatesTo>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"<%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);


    sprintf(buf+strlen(buf),"%s","00000002-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"</s:Header><s:Body><wxf:ResourceCreated><wsa:Address>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:Address><wsa:ReferenceParameters>");
    sprintf(buf+strlen(buf),"<p:CIM_ComputerSystem xmlns:p=\"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"><p:CreationClassName>ABC</p:CreationClassName><p:Name>SimFire</p:Name></p:CIM_ComputerSystem>");
    //genXML(buf,_ComputerSystem);
    // sprintf(buf+strlen(buf),"<wsman:SelectorSet><wsman:Selector Name=\"CreationClassName\">ABC</wsman:Selector><wsman:Selector Name=\"Name\">SimFire</wsman:Selector></wsman:SelectorSet>");
    sprintf(buf+strlen(buf),"</wsa:ReferenceParameters></wxf:ResourceCreated></s:Body></s:Envelope>");

    sendlist->len = strlen(buf);
    insertlen(dcb,sendlist, strlen(hello));

    dcb->sendlist = sendlist;
    //return sendlist;

}

void create_enumerate_response_envelope(DASHCB *dcb)
{

    INT8U *buf;
    send_list *current;//,*head;

    current = new_send_list();
    //head = current;
    dcb->sendlist = current;
    buf = current->addr + PKT_HDR_SIZE;
//getTestCIMProperty_debug(6,5);
    gen_nsdata(dcb,buf);
    gen_action(dcb,buf+strlen(buf));

    sprintf(buf+strlen(buf),"%s","00000002-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"</s:Header><s:Body><%s:EnumerateResponse>",Ns[IDX_NS_ENUMERATION].prefix);


    // if (!(dcb->epr & Optimize))
    sprintf(buf+strlen(buf), "<%s:EnumerationContext>00000001-0000-0000-0000-000000000000</%s:EnumerationContext>",Ns[IDX_NS_ENUMERATION].prefix,Ns[IDX_NS_ENUMERATION].prefix);

    sprintf(buf+strlen(buf), "</%s:EnumerateResponse></s:Body></s:Envelope>",Ns[IDX_NS_ENUMERATION].prefix);
    //  DEBUGMSG(DASH_DEBUG, "enumerate_buf=%s\n",buf);
    current->len = strlen(buf);

    insertlen(dcb,dcb->sendlist, strlen(hello));

    dcb->StartOff=NULL;
    dcb->SubClass=0;
    dcb->RegClass=0;
//    if(strcmp(dcb->r_uri,"CIM_PhysicalElement")==0 || strcmp(dcb->r_uri,"CIM_PhysicalPackage")==0
//      || strcmp(dcb->r_uri,"CIM_PhysicalComponent")==0 ||strcmp(dcb->r_uri,"CIM_PhysicalConnector")==0
//      || strcmp(dcb->r_uri,"CIM_PhysicalFrame")==0)
//     dcb->SubClass=1;

    if (dcb->PullOption!=OneTime)
        dcb->count=0;

    if (strcmp(dcb->q_uri,"")==0)
        strcpy(dcb->q_uri,dcb->r_uri);
    dcb->PreURI=dcb->URI;
//getTestCIMProperty_debug(6,2);

    ///return head;

}
void create_pull_response_envelope(DASHCB *dcb)
{
    INT8U *buf,*tmpbuf;
    send_list *current;//,*head;
    char EndSeq;
    XMLElt *ptr,*XMLPtr;
    int i,j=-1,tmp_epr=0;
    UserInfo *acc_ptr;
    RegProfile *pf=_RegProfile;
    if(strcmp(dcb->r_uri,"CIM_NumericSensor")==0 && dcb->ref_count[0]!=0xff)
        dcb->PullOption=OneTime;
    if (dcb->PullOption & OneTime)
        dcb->count=dcb->ref_count[0];
    if (strcmp(dcb->r_uri,"CIM_Identity")==0 && strcmp(dcb->ref_uri,"CIM_OpaqueManagementData")==0)
    {
        tmpbuf=malloc(128);
        for(i=0; i<OpaqueNum; i++)
        {
            memset(tmpbuf,0,128);
            read_opaque(i,0,128,tmpbuf);
            if(((OPAQDATA*)tmpbuf)->Status==1)
                j++;
            if(j==dcb->ref_count[0])
                break;
        }
        acc_ptr=getuserinfo();
        dcb->count=0;
        while (acc_ptr)
        {
            if (strcmp(((OPAQDATA*)tmpbuf)->Owner, acc_ptr->name)==0)
                break;
            dcb->count++;
            acc_ptr=acc_ptr->next;
        }
        free(tmpbuf);
        dcb->PullOption=OneTime;
    }
    if (strcmp(dcb->r_uri,"CIM_ProtocolEndpoint")==0 && strcmp(dcb->ref_uri,"CIM_USBRedirectionSAP")==0)
        dcb->count=2;

    ptr=ProfileEntry(dcb,dcb->r_uri);

    if (dcb->PullOption & ZeroTime)
        ptr=NULL;

    current = new_send_list();
    //head = current;
    dcb->sendlist = current;
    buf = current->addr + PKT_HDR_SIZE;

    gen_nsdata(dcb,buf);

    gen_action(dcb,buf+strlen(buf));



    sprintf(buf+strlen(buf),"00000000-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    if (dcb->epr & ObjectAndEPR)
      dcb->epr= ObjectAndEPR;
    dcb->StartOff=SearchEnd(dcb->ProfilePtr,dcb->count,&EndSeq);
    if (dcb->PullOption & OneTime)
        EndSeq=1;

    if (strcmp(dcb->r_uri,"CIM_LogEntry")==0 )
    {
        if (LogEnd==1)
            EndSeq=1;
        else
            EndSeq=0;
        dcb->StartOff=dcb->ProfilePtr;
    }
    if(dcb->URI==_PhysicalElement || dcb->URI==_PhysicalPackage || dcb->URI==_PhysicalComponent || dcb->URI==_PhysicalConnector || dcb->URI==_PhysicalFrame)
        // if (dcb->SubClass > 0)
    {
        XMLPtr=dcb->URI;
        if(XMLPtr[dcb->SubClass+1].eltname !=NULL && EndSeq==1)
            EndSeq=2;
    }

    if(dcb->URI== _ElementConformsToProfile || dcb->URI==_ManagedSystemElement)
    {
        if(strcmp(pf[dcb->RegClass].eltname,"CIM_PhysicalElement")==0)
        {
            XMLPtr=_PhysicalElement;
            if(XMLPtr[dcb->SubClass+1].eltname !=NULL && EndSeq==1)
                EndSeq=2;
            if(XMLPtr[dcb->SubClass+1].eltname ==NULL && EndSeq==1)
                EndSeq=3;
        }
        else if(pf[dcb->RegClass+1].eltname !=NULL && EndSeq==1)
            EndSeq=3;
    }

    if (ptr==NULL || dcb->StartOff==NULL)
    {
        sprintf(buf+strlen(buf),"</s:Header><s:Body><%s:PullResponse>",Ns[IDX_NS_ENUMERATION].prefix);
        sprintf(buf+strlen(buf),"<%s:EndOfSequence/></%s:PullResponse></s:Body></s:Envelope>",Ns[IDX_NS_ENUMERATION].prefix,Ns[IDX_NS_ENUMERATION].prefix);
        dcb->epr=0;
        dcb->StartOff=NULL;
        dcb->count=0;
        dcb->SubClass=0;
        dcb->PullOption=0;
        dcb->association=0;
        strcpy(dcb->q_uri,"");
        memset(dcb->ref_count,-1,16);
        current->len = strlen(buf);
        insertlen(dcb,dcb->sendlist, strlen(hello));
        return;
        ///return head;
    }
    if (EndSeq==1)
        sprintf(buf+strlen(buf),"</s:Header><s:Body><%s:PullResponse><%s:Items>",Ns[IDX_NS_ENUMERATION].prefix,Ns[IDX_NS_ENUMERATION].prefix);
    else
    {
        sprintf(buf+strlen(buf),"</s:Header><s:Body><%s:PullResponse>",Ns[IDX_NS_ENUMERATION].prefix);
        sprintf(buf+strlen(buf),"<%s:EnumerationContext>00000001-0000-0000-0000-000000000000</%s:EnumerationContext><%s:Items>",Ns[IDX_NS_ENUMERATION].prefix,Ns[IDX_NS_ENUMERATION].prefix,Ns[IDX_NS_ENUMERATION].prefix);
    }
    //  if ((dcb->epr & ObjectAndEPR)==ObjectAndEPR && dcb->association !=-1)
    if ((dcb->epr & ObjectAndEPR)==ObjectAndEPR)
        sprintf(buf+strlen(buf),"<%s:Item>",Ns[IDX_NS_WS_MAN].prefix);

    if (dcb->PullOption & XMLFragment)
        sprintf(buf+strlen(buf), "<%s:XmlFragment>",Ns[IDX_NS_WS_MAN].prefix);

    tmp_epr=dcb->epr;
    if(dcb->epr & ObjectAndEPR)
      dcb->epr=0;
    if (!(dcb->epr & (EPR | ObjectAndEPR )))
    {
        sprintf(buf+strlen(buf),"<p:%s xmlns:p=\"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/%s\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">",dcb->r_uri,dcb->r_uri);
        current->len = strlen(buf);
        current= genXML(dcb,current,dcb->StartOff);
        buf = current->addr + PKT_HDR_SIZE;
        sprintf(buf+strlen(buf),"</p:%s>",dcb->r_uri);

    }
    dcb->epr=tmp_epr;
    if(dcb->association !=-1 && (element[ID_ASSOCIATION_INSTANCES].elementPtr!=NULL || element[ID_ASSOCIATED_INSTANCES].elementPtr!=NULL || dcb->epr!=0 || (dcb->PullOption & SQLQuery)))
    {
        element[ID_ASSOCIATION_INSTANCES].elementPtr=NULL;
        element[ID_ASSOCIATED_INSTANCES].elementPtr=NULL;

        sprintf(buf+strlen(buf),"<%s:EndpointReference><%s:Address>%s</%s:Address><%s:ReferenceParameters><%s:ResourceURI>",Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_ADDRESSING].prefix,WSA_TO_ANONYMOUS,Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_WS_MAN].prefix);
        sprintf(buf+strlen(buf),"%s/%s</%s:ResourceURI>",XML_NS_CIM_CLASS,dcb->r_uri,Ns[IDX_NS_WS_MAN].prefix);
        if (dcb->count >=0)
        {
            if (strcmp(dcb->r_uri,"CIM_LogEntry")==0)
                current=SelectElement(current,buf,dcb->ProfilePtr,0);
            else
                current=SelectElement(current,buf,dcb->ProfilePtr,dcb->count);
            buf = current->addr + PKT_HDR_SIZE;
        }
        sprintf(buf+strlen(buf),"</%s:ReferenceParameters></%s:EndpointReference>",Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_ADDRESSING].prefix);

    }

    if (dcb->association==-1 && (dcb->epr & (EPR | ObjectAndEPR))> 0)
    {
    	dcb->StartOff=SearchEnd(dcb->ProfilePtr,dcb->count,&EndSeq);
        sprintf(buf+strlen(buf),"<%s:EndpointReference><%s:Address>%s</%s:Address><%s:ReferenceParameters><%s:ResourceURI>",Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_ADDRESSING].prefix,WSA_TO_ANONYMOUS,Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_WS_MAN].prefix);
        sprintf(buf+strlen(buf),"%s/%s</%s:ResourceURI>",XML_NS_CIM_CLASS,dcb->r_uri,Ns[IDX_NS_WS_MAN].prefix);
        current->len = strlen(buf);
        current= genXML(dcb,current,dcb->StartOff);
        buf = current->addr + PKT_HDR_SIZE;
        sprintf(buf+strlen(buf),"</%s:ReferenceParameters></%s:EndpointReference>",Ns[IDX_NS_ADDRESSING].prefix,Ns[IDX_NS_ADDRESSING].prefix);

    }

    if (dcb->PullOption & XMLFragment)
        sprintf(buf+strlen(buf), "</%s:XmlFragment>",Ns[IDX_NS_WS_MAN].prefix);

    (dcb->count)++;

    if (EndSeq==2)
    {
        EndSeq=0;
        dcb->StartOff=NULL;
        dcb->count=0;
        (dcb->SubClass)++;
    }
    if(EndSeq==3)
    {
        EndSeq=0;
        dcb->StartOff=NULL;
        dcb->count=0;
        (dcb->RegClass)++;
    }

    if ((dcb->epr & ObjectAndEPR)==ObjectAndEPR)
        //  if ((dcb->epr & ObjectAndEPR)==ObjectAndEPR && dcb->association !=-1)
        sprintf(buf+strlen(buf),"</%s:Item>",Ns[IDX_NS_WS_MAN].prefix);
    if (EndSeq==1)
    {
        sprintf(buf+strlen(buf),"</%s:Items><%s:EndOfSequence/></%s:PullResponse></s:Body></s:Envelope>",Ns[IDX_NS_ENUMERATION].prefix,Ns[IDX_NS_ENUMERATION].prefix,Ns[IDX_NS_ENUMERATION].prefix);
        dcb->epr=0;
        dcb->StartOff=NULL;
        dcb->count=0;
        dcb->SubClass=0;
        dcb->PullOption=0;
        dcb->association=0;
        strcpy(dcb->q_uri,"");
        memset(dcb->ref_count,-1,16);

    }
    else
        sprintf(buf+strlen(buf),"</%s:Items></%s:PullResponse></s:Body></s:Envelope>",Ns[IDX_NS_ENUMERATION].prefix,Ns[IDX_NS_ENUMERATION].prefix);


    current->len = strlen(buf);

    insertlen(dcb,dcb->sendlist, strlen(hello));

}
void create_get_response_envelope(DASHCB *dcb)
{
    INT8U *buf;
    send_list *current; //,*head;


	//getTestCIMProperty_debug(5,0);
	
    current = new_send_list();
    ///head = current;
    dcb->sendlist = current;
    buf = current->addr + PKT_HDR_SIZE;
    gen_nsdata(dcb,buf);
    gen_action(dcb,buf+strlen(buf));

    sprintf(buf+strlen(buf),"00000000-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"</s:Header><s:Body><p:%s xmlns:p=\"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/%s\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">",dcb->r_uri,dcb->r_uri);

    current->len = strlen(buf);

    current=genXML(dcb,current,dcb->StartOff);
    buf = current->addr + PKT_HDR_SIZE;

    sprintf(buf+strlen(buf),"</p:%s></s:Body></s:Envelope>",dcb->r_uri);//+briank.rtk dcb->r_uri is ex: CIM_ComputerSystem 

    current->len = strlen(buf);

    insertlen(dcb,dcb->sendlist, strlen(hello));

    dcb->StartOff=NULL;

    ///dcb->sendlist = head;
}




void create_put_response_envelope(DASHCB *dcb,INT8U *body)
{
    INT8U *buf;
    send_list *current;///,*head;
    PutBodyElement(dcb,body);

    current = new_send_list();
    ///head = current;
    dcb->sendlist = current;
    buf = current->addr + PKT_HDR_SIZE;


    gen_nsdata(dcb,buf);
    gen_action(dcb,buf+strlen(buf));


    sprintf(buf+strlen(buf),"00000000-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    // sprintf(buf+strlen(buf),"</s:Header><s:Body><p:%s xmlns:p=\"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/%s\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">",dcb->r_uri,dcb->r_uri);
    sprintf(buf+strlen(buf),"</s:Header><s:Body><p:%s xmlns:p=\"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/%s\">",dcb->r_uri,dcb->r_uri);
    //  strcpy(name,"SimFire");

    current->len = strlen(buf);
    current=genXML(dcb,current,dcb->StartOff);
    // current = genXML(dcb,current,ProfilePtr);
    buf = current->addr + PKT_HDR_SIZE;


    sprintf(buf+strlen(buf),"</p:%s>",dcb->r_uri);
    sprintf(buf+strlen(buf),"</s:Body></s:Envelope>");

    current->len = strlen(buf);

    insertlen(dcb,dcb->sendlist, strlen(hello));

    ///dcb->sendlist =  head;



}

void create_subscribe_response_envelope(DASHCB *dcb)
{
    INT8U *buf;
    plist *currentx;
    send_list *current;//,*head;

    current = new_send_list();
    ///head = current;
    dcb->sendlist = current;
    buf = current->addr + PKT_HDR_SIZE;

    gen_nsdata(dcb,buf);
    sprintf(buf+strlen(buf)," xmlns:wsen=\"http://schemas.xmlsoap.org/ws/2004/09/enumeration\"");
    current->len = strlen(buf);
    current->next = new_send_list();

    current = current->next;
    buf = current->addr + PKT_HDR_SIZE;
    gen_action(dcb,buf+strlen(buf));

    sprintf(buf+strlen(buf),"00000000-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"</s:Header><s:Body>");
    if (strcmp(dcb->r_uri,"CIM_FilterCollection")==0)
    {
        sprintf(buf+strlen(buf),"<wse:SubscribeResponse><wse:SubscriptionManager>");
        sprintf(buf+strlen(buf),"<wsa:Address>%s</wsa:Address>",WSA_TO_ANONYMOUS);
        sprintf(buf+strlen(buf),"<wsa:ReferenceParameters><wsman:ResourceURI>%s/CIM_FilterCollectionSubscription</wsman:ResourceURI><wsman:SelectorSet>",XML_NS_CIM_CLASS);
        sprintf(buf+strlen(buf),"<%s:Selector Name=\"__cimnamespace\">root/interop</%s:Selector>",Ns[IDX_NS_WS_MAN].prefix,Ns[IDX_NS_WS_MAN].prefix);
        strcpy(dcb->r_uri,"CIM_FilterCollectionSubscription");
        currentx=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,dcb->r_uri),NewMem());
        dcb->epr=EPR;
        dcb->association=-1;
        current->len = strlen(buf);
        current= genXML(dcb,current,SearchEnd(currentx,dcb->count,NULL));
        // current= genXML(dcb,current,currentx);
        buf = current->addr+ PKT_HDR_SIZE;
        ReleaseProfile(currentx);
        dcb->epr=0;
        dcb->association=0;
        // sprintf(buf+strlen(buf),"</wse:SubscriptionManager>");
        sprintf(buf+strlen(buf),"</wsman:SelectorSet></wsa:ReferenceParameters></wse:SubscriptionManager>");
    }
    else
        sprintf(buf+strlen(buf),"<wse:SubscribeResponse><wse:SubscriptionManager><wsa:Address>http://www.example.org/oceanwatch/SubscriptionManager</wsa:Address><wsa:ReferenceParameters><wse:Identifier>uuid:%s</wse:Identifier></wsa:ReferenceParameters></wse:SubscriptionManager>",xuuid);
    //  sprintf(buf+strlen(buf), "<wsen:EnumerationContext>00000001-0000-0000-0000-000000000000</wsen:EnumerationContext>");
    sprintf(buf+strlen(buf),"</wse:SubscribeResponse>");
    current->len = strlen(buf);

    //  current=genXML(current,ProfileEntry(r_uri));
    buf = current->addr + PKT_HDR_SIZE;
//  buf = current->addr;
    sprintf(buf+strlen(buf),"</s:Body></s:Envelope>");

    current->len = strlen(buf);

    insertlen(dcb,dcb->sendlist, strlen(hello));

}

void create_unsubscribe_response_envelope(DASHCB *dcb)
{
    INT8U *buf;
    send_list *current;//,*head;
    plist *cur_p,*cur_px;

    cur_p=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,dcb->ref_uri),TmpNewMem());
    if (cur_p)
    {
        cur_px= GetElement(cur_p,"Name",dcb->count);
        unsubscribe(dcb,cur_px->data);
        ReleaseProfile(cur_p);
    }
    TmpReleaseBuf();
    current = new_send_list();
    ///head = current;
    dcb->sendlist = current;
    buf = current->addr + PKT_HDR_SIZE;

    gen_nsdata(dcb,buf);
    gen_action(dcb,buf+strlen(buf));

    sprintf(buf+strlen(buf),"00000000-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"</s:Header>");
    //  sprintf(buf+strlen(buf),"<wse:SubscribeResponse><wse:SubscriptionManager><wsa:Address>http://www.example.org/oceanwatch/SubscriptionManager</wsa:Address><wsa:ReferenceParameters><wse:Identifier>uuid:22e8a584-0d18-4228-b2a8-3716fa2097fa</wse:Identifier></wsa:ReferenceParameters></wse:SubscriptionManager><wse:Expires>2004-07-01T00:00:00.000-00:00</wse:Expires></wse:SubscribeResponse>");

    //sprintf(buf+strlen(buf),"<wse:UnSubscribeResponse><wse:SubscriptionManager><wsa:Address>http://www.example.org/oceanwatch/SubscriptionManager</wsa:Address><wsa:ReferenceParameters><wse:Identifier>uuid:%s</wse:Identifier></wsa:ReferenceParameters></wse:SubscriptionManager></wse:SubscribeResponse>",Identifier);

    current->len = strlen(buf);

    //  current=genXML(current,ProfileEntry(r_uri));
    buf = current->addr + PKT_HDR_SIZE;
//  buf = current->addr;
    sprintf(buf+strlen(buf),"<s:Body/></s:Envelope>");

    current->len = strlen(buf);

    insertlen(dcb,dcb->sendlist, strlen(hello));


    ///return head;
}
void create_evt_getstatus_response_envelope(DASHCB *dcb)
{
    INT8U *buf;
    send_list *current;///,*head;

    current = new_send_list();
    dcb->sendlist = current;
    ///head = current;
    buf = current->addr + PKT_HDR_SIZE;

    gen_nsdata(dcb,buf);
    gen_action(dcb,buf+strlen(buf));

    sprintf(buf+strlen(buf),"00000000-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"</s:Header><s:Body>");

    sprintf(buf+strlen(buf),"<wse:GetStatusResponse><wse:Expires>2004-06-26T12:00:00.000-00:00</wse:Expires></wse:GetStatusResponse>");

    current->len = strlen(buf);

    buf = current->addr + PKT_HDR_SIZE;

    sprintf(buf+strlen(buf),"</s:Body></s:Envelope>");

    current->len = strlen(buf);

    insertlen(dcb,dcb->sendlist, strlen(hello));
    ///return head;
}
void create_evt_renew_response_envelope(DASHCB *dcb)
{
    INT8U *buf;
    send_list *current;//,*head;

    current = new_send_list();
    ///head = current;
    dcb->sendlist = current;
    buf = current->addr + PKT_HDR_SIZE;

    gen_nsdata(dcb,buf);
    gen_action(dcb,buf+strlen(buf));

    sprintf(buf+strlen(buf),"00000000-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"</s:Header><s:Body>");

    sprintf(buf+strlen(buf),"<wse:RenewResponse><wse:Expires>2004-06-26T12:00:00.000-00:00</wse:Expires></wse:RenewResponse>");

    current->len = strlen(buf);

    buf = current->addr + PKT_HDR_SIZE;

    sprintf(buf+strlen(buf),"</s:Body></s:Envelope>");

    current->len = strlen(buf);

    insertlen(dcb,dcb->sendlist, strlen(hello));


    ///return head;
}

void auth_anon(DASHCB *dcb)
{
    INT8U *buf;
    ///send_list *sendlist;

    dcb->sendlist = new_send_list();
    buf = dcb->sendlist->addr + PKT_HDR_SIZE;

    strcpy(buf,hello);

    dcb->sendlist->len = strlen(buf);

    insertlen(dcb,dcb->sendlist, strlen(hello));

    ///return sendlist;
}
extern int setAuthCredential(unsigned char *nonce  , unsigned char*opaque);
void auth_error(DASHCB *dcb,int val)
{
    INT8U *buf;
    ///send_list *sendlist;
    ////INT8U hello_auth_err[]="HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Digest realm=\"OPENWSMAN\", qop=\"auth\", nonce=\"12345\", opaque =\"6789\"\r\nContent-Length:xxxx\r\n\r\n401 Authorization required";
//----------------------
    unsigned int nonceA = 0, nonceB = 0,nonceC = 0;
    int len = 0;
    unsigned char *authFailStr = NULL;
    char *nonceBuf = NULL, *opaqueBuf = NULL;
 //----------------------
//HTTP/1.1 401 Unauthorized\r\nContent-Length: 26\r\nWWW-Authenticate: Negotiate\r\nWWW-Authenticate: Digest realm=\"OPENWSMAN\", qop=\"auth\",
//nonce=\"%x%x\", opaque =\"%x\"\r\n\r\n401 Authorization required
	//----------------------
        	  srand(REG32(TIMER_IOBASE + TIMER_CV));
        	  
        	  nonceA = rand() ;	  
	  nonceB = rand() ;	  
	  nonceC = rand() ;	
	  authFailStr = malloc(256);
	  
	//-----------------------------------------------------------------
	nonceBuf=malloc(32);
	if(nonceBuf != NULL){
		memset(nonceBuf , 0x00 ,32);
		sprintf(nonceBuf , "%x%x" ,nonceA,nonceB );
	}
	
	opaqueBuf=malloc(32);
	if(opaqueBuf!=NULL){
		memset(opaqueBuf , 0x00 ,32);
		sprintf(opaqueBuf , "%x" ,nonceC );
		setAuthCredential(nonceBuf , opaqueBuf  );
	}
	

	DEBUGMSG(HTTP_DEBUG,"setAuthCredential [<%s> , <%s> ]\n",nonceBuf , opaqueBuf  );

	if(nonceBuf){
		free(nonceBuf);
	}
	if(opaqueBuf){
		free(opaqueBuf);
	}
	//-----------------------------------------------------------------
	
	  
	  if(authFailStr){
	  	memset(authFailStr,0x00,256);
	  	sprintf(authFailStr,"HTTP/1.1 401 Unauthorized\r\nContent-Length: 26\r\nWWW-Authenticate: Negotiate\r\nWWW-Authenticate: Digest realm=\"OPENWSMAN\", qop=\"auth\",nonce=\"%x%x\", opaque =\"%x\"\r\n\r\n401 Authorization required",nonceA,nonceB,nonceC);
	  }
	 //---------------------- 	
	
	    dcb->sendlist = new_send_list();
	    buf = dcb->sendlist->addr + PKT_HDR_SIZE;
	    if (val==BASIC_AUTH)
	        strcpy(buf,"HTTP/1.1 401 Unauthorized\r\nContent-Length: 26\r\nWWW-Authenticate: Basic realm=\"OPENWSMAN\"\r\n\r\n401 Authorization required");		  
	    else{
	        if(authFailStr){
		 len = strlen(authFailStr);	
		 if(len > 256){
		 	len = 256;
		}
	        	strncpy(buf , authFailStr , len );
		free(authFailStr);		
	 	authFailStr = NULL;
	        }else{
	        	strcpy(buf,"HTTP/1.1 401 Unauthorized\r\nContent-Length: 26\r\nWWW-Authenticate: Negotiate\r\nWWW-Authenticate: Digest realm=\"OPENWSMAN\", qop=\"auth\",nonce=\"1CE5486fC54DAB\", opaque =\"1DEAE79B\"\r\n\r\n401 Authorization required");
	        }
	    }
	   dcb->sendlist->len = strlen(buf);    	
    ///return sendlist;
}
#ifdef CONFIG_MSAD_ENABLED
void auth_kerberos_ok(DASHCB *dcb)
{
    INT8U *buf;

    dcb->sendlist = new_send_list();
    buf = dcb->sendlist->addr + PKT_HDR_SIZE;
    strcpy(buf,"HTTP/1.1 200 \r\nContent-Length: 0\r\nWWW-Authenticate: Negotiate ");
 
    addADauth(buf+strlen(buf), &dcb->cs->pcb->msad.auth_buffer);
    
    dcb->sendlist->len = strlen(buf);

    ///return sendlist;
}
#endif
void response_fault(DASHCB *dcb)
{
    INT8U *buf;

    dcb->sendlist = new_send_list();
    buf = dcb->sendlist->addr + PKT_HDR_SIZE;

    gen_nsdata(dcb,buf);
    gen_action(dcb,buf+strlen(buf));

    if (dcb->fault_id == Fault_InvalidEnumContext)
        strcpy(scode,  "s:Receiver");
    else if (dcb->fault_id== Fault_NotUnderstood)
        strcpy(scode,  "s:MustUnderstand");
    else
        strcpy(scode,  "s:Sender");


    sprintf(buf+strlen(buf),"00000000-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    sprintf(buf+strlen(buf),"</s:Header><s:Body><s:Fault><s:Code><s:Value>%s</s:Value><s:Subcode><s:Value>%s</s:Value></s:Subcode></s:Code><s:Reason><s:Text xml:lang=\"en\">The action is not supported by the service.</s:Text></s:Reason><s:Detail><wsman:FaultDetail>%s/%s</wsman:FaultDetail></s:Detail></s:Fault></s:Body></s:Envelope>",scode,fault[dcb->fault_id-1].fault_str,XML_NS_WSMAN_FAULT_DETAIL,fault[dcb->fault_id-1].fault_detail);

    dcb->sendlist->len = strlen(buf);
    insertlen(dcb,dcb->sendlist, strlen(hello_err));


    ///return sendlist;
}

void create_invoke_response_envelope(DASHCB *dcb)
{
    INT8U *buf, ns[10],*op_buf;
    send_list *current;//,*head;
    plist *currentx;
    UserInfo *acc_ptr;

    current = new_send_list();

    ///head = current;
    dcb->sendlist = current;
    buf = current->addr + PKT_HDR_SIZE;

    gen_nsdata(dcb,buf);
    if (!Ns[IDX_NS_CIM_CLASS].used)
        sprintf(buf+strlen(buf)," xmlns:n1=\"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/%s\"",dcb->r_uri);
    gen_action(dcb,buf+strlen(buf));
    if (Ns[IDX_NS_CIM_CLASS].used)
        strcpy(ns,Ns[IDX_NS_CIM_CLASS].prefix);
    else
        strcpy(ns,"n1");

    sprintf(buf+strlen(buf),"%s","00000002-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</%s:MessageID>",Ns[IDX_NS_ADDRESSING].prefix);
    //  StartingBoot=1;

    ///invoke_ret = 0;
//   sprintf(buf+strlen(buf),"</s:Header><s:Body><%s:%s_OUTPUT><%s:ReturnValue>%d</%s:ReturnValue></%s:%s_OUTPUT></s:Body></s:Envelope>",ns,Method,ns,invoke_ret,ns,ns,Method);
    sprintf(buf+strlen(buf),"</s:Header><s:Body><%s:%s_OUTPUT><%s:ReturnValue>%d</%s:ReturnValue>",ns,Method,ns,dcb->status,ns);

    if (dcb->control==ID_CreateAccount && !dcb->status)
    {
        sprintf(buf+strlen(buf),"<Account><wsa:Address>%s</wsa:Address>",WSA_TO_ANONYMOUS);
        sprintf(buf+strlen(buf),"<wsa:ReferenceParameters><wsman:ResourceURI>%s/CIM_Account</wsman:ResourceURI>",XML_NS_CIM_CLASS);

        acc_ptr=getuserinfo();
        while (acc_ptr)
        {
            (dcb->count)++;
            acc_ptr=acc_ptr->next;
        }
        currentx=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,"CIM_Account"),TmpNewMem());
        current=SelectElement(current,buf,currentx,(dcb->count)-1);
        buf = current->addr + PKT_HDR_SIZE;
        TmpReleaseBuf();
        currentx = ReleaseProfile(currentx);
        sprintf(buf+strlen(buf),"</wsa:ReferenceParameters></Account>");
    }

    if (dcb->status == 0 && dcb->control==ID_ReadOpaque)
    {
        strcpy(buf+strlen(buf),"<n1:Data>");
        current->len = strlen(buf);
        op_buf=OpaqueBuf;
        while(strlen(op_buf) >0)
        {
            if(strlen(op_buf)+current->len > PAYLOAD_SIZE_THD)
            {
                strncpy(buf+strlen(buf),op_buf,PAYLOAD_SIZE_THD-current->len);
                op_buf=op_buf+PAYLOAD_SIZE_THD-current->len;
            }
            else
            {
                strcpy(buf+strlen(buf),op_buf);
                op_buf=op_buf+strlen(op_buf);
            }

            current->len = strlen(buf);
            if(current->len >= PAYLOAD_SIZE_THD)
            {
                current->next = new_send_list();
                current = current->next;
                buf = current->addr + PKT_HDR_SIZE;
            }

        }
        strcpy(buf+strlen(buf),"</n1:Data>");
        if (OpaqueBuf)
            free(OpaqueBuf);
        OpaqueBuf=NULL;
    }
    sprintf(buf+strlen(buf),"</%s:%s_OUTPUT></s:Body></s:Envelope>",ns,Method);

    current->len = strlen(buf);
    insertlen(dcb,dcb->sendlist, strlen(hello));


    ///return head;



}




void ParserAction(DASHCB *dcb,INT8U * action)
{

    INT8U *ptr;
    INT32S i = 0;

    while (e_action[i].wsaction)
    {
        if (strncmp(action, e_action[i].wsaction,strlen(e_action[i].wsaction)) == 0)
        {
            if (e_action[i].wstype == WS_DISP_TYPE_INVOKE)
            {
                ptr = strrchr(action, '/');
                //element[ID_ACTION].elementPtr = ptr + 1;
                strcpy(Method,trimspace(ptr + 1));
            }
            //return e_action[i].wstype;
            dcb->actiontype = e_action[i].wstype;
			
            return ;
        }
        i++;
    }

    dcb->actiontype = -1;
}
void register_endpoint(DASHCB *dcb,INT8U *body)
{
    // if(element[ID_IDENTIFY].elementPtr!=NULL)
    //	  i=WS_DISP_TYPE_IDENTIFY;
    switch (dcb->actiontype)
    {
    case WS_DISP_TYPE_IDENTIFY:
        wsman_identify_stub(dcb);
        break;

    case WS_DISP_TYPE_GET:
        create_get_response_envelope(dcb);
        break;

    case WS_DISP_TYPE_PUT:
        create_put_response_envelope(dcb,body);
        break;

    case WS_DISP_TYPE_CREATE:
        create_create_response_envelope(dcb);
        break;

    case WS_DISP_TYPE_DELETE:
        create_delete_response_envelope(dcb);
        break;

    case WS_DISP_TYPE_ENUMERATE:
        if (strcmp(p_selector,"")!=0)
            SelectorEnabled=1;
        create_enumerate_response_envelope(dcb);
        break;

    case WS_DISP_TYPE_RELEASE:
        create_release_response_envelope(dcb);
        break;

    case WS_DISP_TYPE_PULL:
        if (SelectorEnabled==1)
            SelectorEnabled=2;
        create_pull_response_envelope(dcb);
        break;

    case WS_DISP_TYPE_INVOKE:
        create_invoke_response_envelope(dcb);
        break;

    case WS_DISP_TYPE_SUBSCRIBE:
        create_subscribe_response_envelope(dcb);
        break;

    case WS_DISP_TYPE_UNSUBSCRIBE:
        create_unsubscribe_response_envelope(dcb);
        break;

    case WS_DISP_TYPE_EVT_GETSTATUS:
        create_evt_getstatus_response_envelope(dcb);
        break;

    case WS_DISP_TYPE_EVT_RENEW:
        create_evt_renew_response_envelope(dcb);
        break;

    default:
        dcb->sendlist = NULL;
        break;
    }
}
void ResetUsed (XMLElt *p)
{
    char *eltname;
    int i=0;

    for (;;)
    {
        eltname=p[i].eltname;

        //if (!eltname || SQLQuery)
        if (!eltname)
            break;
        p[i].used=0;

        i++;
    }
    i=0;
    while (Ns[i].uri)
    {
        Ns[i].used=0;
        i++;
    }

    return;


}
void SearchKey(DASHCB *dcb,INT8U *str,plist *current)
{
    INT8U *ptr = str, *ptr_end,*ptr1,*ptr1_end,ch,ch1,renew=0;
    INT8U token[] = "Selector Name";
    INT8U *sbuf,*pbuf,*sptr;
    INT32S ret = 0;
    plist *tmp_p;
    tmp_p=current;
    //  sptr=strstr(str, "</wsman:SelectorSet>");
    sptr=strstr(str, ":SelectorSet>");
    if (sptr)
        *sptr = NULLCHR;
    sbuf=malloc(256);
    memset(sbuf,0,256);

    pbuf=sbuf;

    while (ptr)
    {

        ptr = strchr(ptr, '"');

        if (ptr)
            ptr_end = strchr(ptr+1, '"');
        ch=*ptr_end;
        *ptr_end = NULLCHR;

        if (strcmp(ptr+1,"__cimnamespace")==0)
        {
            *ptr_end=ch;
            ptr = strstr(ptr+1+strlen("__cimnamespace"),token);
            continue;
        }

        sprintf(pbuf,"%s",ptr+1);
        pbuf=pbuf+strlen(ptr+1)+1;

        ptr1= strchr(ptr_end+1, '>');

        if (ptr1)
        {
            ptr1_end = strchr(ptr1+1, '<');
            if(ptr1_end)
            {
                ch1=*ptr1_end;
                *ptr1_end = NULLCHR;
            }
            if(strlen(ptr1+1)==0)
            {
                ptr = strstr(ptr1_end+1,token);
                *ptr_end=ch;
                *ptr1_end=ch1;
                continue;
            }
            sprintf(pbuf,"%s",ptr1+1);
            pbuf=pbuf+strlen(ptr1+1)+1;
        }

        if (strcmp("CreationClassName",ptr+1) == 0)
        {
            if (!strstr(ptr1+1,"CIM_"))
                dcb->fault_id=Fault_InvalidFilter;
        }

        if (strcmp(dcb->r_uri,"CIM_LogEntry")==0 && strcmp("InstanceID",ptr+1)==0)
        {
            dcb->count=atoi(ptr1+1);
            renew=1;
        }
        *ptr_end=ch;
        *ptr1_end=ch1;
        ptr = strstr(ptr1_end,token);
    }
    if(sptr)
        *sptr=':';
    if (renew==1)
    {
        ReleaseBuf();
        dcb->ProfilePtr = ReleaseProfile(dcb->ProfilePtr);
        dcb->ProfilePtr=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,dcb->r_uri),NewMem());
        current=dcb->ProfilePtr;
    }

    if (strcmp(sbuf,"")==0 && dcb->actiontype==WS_DISP_TYPE_GET)
        dcb->fault_id=Fault_MissingSelectorFault;
    else
        ret= GetInstanceCount(dcb,current,sbuf);

    if (ret >= 0)
        dcb->count = ret;


    free(sbuf);
    //return;
}
void SearchReference(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr, *ptr_end,*ptr_end1;

    INT8U uri[64];
    INT8U name[64];
    INT8S idx=-1;
    plist *currentx;

    idx=0;
    sprintf(name,":%s>", WSA_REFERENCE_PARAMETERS);
    str=strstr(str,name);
    if (!str)
        return;
    ptr_end=strstr(str+strlen(name),name);
    if (ptr_end)
        *ptr_end=NULLCHR;
    while (1)
    {
        ptr=strstr(str,":ResourceURI>");
        if (ptr)
        {
            if (strstr(ptr,":SelectorSet/>"))
                break;
            ptr_end1=strchr(ptr+strlen(":ResourceURI>"),'<');
            if(ptr_end1)
                *ptr_end1=NULLCHR;
            if (strstr(ptr,"http://"))
            {
                ptr = strrchr(ptr, '/') + 1;
                strcpy(uri,ptr);
            }
            if (ptr_end1)
                *ptr_end1='<';
            ptr=strstr(ptr_end1+1, ":SelectorSet>");
            ptr=ptr+strlen(":SelectorSet>");
            currentx=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,uri),TmpNewMem());
            SearchKey(dcb,ptr,currentx);
            dcb->ref_count[idx]=dcb->count;
            TmpReleaseBuf();
            ReleaseProfile(currentx);

        }
        idx++;
        if(ptr_end)
            *ptr_end=':';
        str=strstr(ptr_end+1+strlen(name), name);
        if (str)
            ptr_end=strstr(str+1+strlen(name), name);
        else
            break;
    }
}

void InitElement(DASHCB *dcb)
{
    int i=0;

    while (element[i].elementName)
    {
        element[i].elementPtr = NULL;
        i++;
    }
    i=0;
    while (bodyelement[i].elementName)
    {
        bodyelement[i].elementPtr = NULL;
        i++;
    }

    i=0;
    while (Ns[i].uri)
    {
        Ns[i].used=0;
        i++;
    }

    return;
}

int getUserState(INT8U *name,INT8U count)
{

    UserInfo *ptr = userhead;
    INT8U i=0;

    while (ptr)
    {
    	if(name!=NULL)
    	{
           if(strcmp(ptr->name,name)==0)
           {
             //ptr->opt=1;//Enable
             if(ptr->opt == 1 ){
			return 2;
             	}else{
             		return 3;
             	}
             //break;
           }	   
	}	
    	else if(i==count)
       {
           if(ptr->opt == 1 ){
			return 2;
             	}else{
             		return 3;
             	}
       }
       i++;	
       ptr=ptr->next;
    }    
    
}

void EnableUser(INT8U *name,INT8U count,INT8U val)
{

    UserInfo *ptr = userhead;
    INT8U i=0;

    while (ptr)
    {
    	if(name!=NULL)
    	{
           if(strcmp(ptr->name,name)==0)
           {
             ptr->opt=1;
             break;
           }	   
	}	
    	else if(i==count)
        {
           if(val==2)	
             ptr->opt=1;
           else
             ptr->opt=0; 
           break;
        }
        i++;
	 ptr->crc16 = inet_chksum(ptr, 34);
        ptr=ptr->next;
    }    
    setdirty(USERTBL);
}
void md_RequestStateChange(DASHCB *dcb, INT8U *str)
{
    INT8U *ptr,*ptr_end,*ptr1;
    INT16U value;
    plist *xptr,*current,*currentx;
    INT32U address;
    int ip0,ip1,ip2,ip3,ret;
    //INT8U info[128];
    dcb->status = 0;
    xptr=dcb->StartOff;

    ptr=strstr(str,":RequestedState>");
    if (ptr)
    {
        ptr=ptr+strlen(":RequestedState>");
        ptr1=ptr;

        ptr_end = strstr(ptr1, ":RequestedState>");
        ptr_end=strback(ptr_end,'<');
        if (ptr_end)
        {
            *ptr_end = NULLCHR;
            //   str = trimspace(ptr + 1);
            //str = strltrim(ptr + 1);

            if (strcmp(request_state,ptr)!=0)
                strcpy(request_state,ptr);
            value = atoi(request_state);

            if (strcmp(dcb->r_uri,"CIM_TextRedirectionSAP")==0)
            {
                dcb->qhdr = malloc(sizeof(QHdr));
                memset(dcb->qhdr, 0, sizeof(QHdr));
                if (value==2 || value==6)
                {
                    dcb->qhdr->cmd = SRV_RESTART;
                    dcb->qhdr->port = 87;
                    current=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,"CIM_TCPProtocolEndpoint"),TmpNewMem());
                    currentx= GetElement(current,"PortNumber",dcb->count);
                    sscanf(currentx->data,"%d",&bind_port);
                    dcb->qhdr->port = bind_port;
                    TmpReleaseBuf();
                    ReleaseProfile(current);
                }
                else
                {
                    dcb->qhdr->cmd = SRV_STOP;
                }

                dcb->control = Ctextredirection;
            }

            if (strcmp(dcb->r_uri,"CIM_USBRedirectionSAP")==0)
            {
                dcb->qhdr = malloc(sizeof(QHdr));
                memset(dcb->qhdr, 0, sizeof(QHdr));
                if(strcmp(access_info,"")==0 && dcb->count!=0)
                {
                    dcb->qhdr->option=dcb->count-1;
                    dcb->control = Cusbredirection_Listen;
                }
                else
                    dcb->control = Cusbredirection_Connect;


                if (value==2 || value==6)
                {
                    if (dcb->control == Cusbredirection_Connect)
                    {
                        bind_port=80;
                        ptr=strstr(access_info,"http://");
                        if (ptr)
                        {
                            ptr=ptr+strlen("http://");
                            ptr_end=strchr(ptr,'/');
                            if (ptr_end)
                            {
                                *ptr_end='\0';
                                ret=sscanf(ptr,"%d.%d.%d.%d",&ip0,&ip1,&ip2,&ip3);
                                if (ret==0)
                                {
                                    rtGethostbyname_F(IPv4,ptr,&address);
                                    tip[0]=address&0xff;
                                    tip[1]=(address>>8) & 0xff;
                                    tip[2]=(address>>16)&0xff;
                                    tip[3]=address>>24;
                                }
                                else
                                {
                                    tip[0]=ip0;
                                    tip[1]=ip1;
                                    tip[2]=ip2;
                                    tip[3]=ip3;
                                }
                                strcpy(fn,ptr_end+1);
                                ptr=strchr(ptr,':');
                                if (ptr)
                                    sscanf(ptr+1,"%d",&bind_port);
                                *ptr_end='/';

                            }
                        }
                    }

                    if (dcb->control == Cusbredirection_Listen)
                    {
                        current=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,"CIM_TCPProtocolEndpoint"),TmpNewMem());
                        currentx= GetElement(current,"PortNumber",2);
                        sscanf(currentx->data,"%d",&bind_port);
                        TmpReleaseBuf();
                        ReleaseProfile(current);
                    }
                    dcb->qhdr->cmd = SRV_RESTART;
                    dcb->qhdr->port = bind_port;
                }
                else
                {
                    strcpy(access_info,"");	
                    dcb->qhdr->cmd = SRV_STOP;
                }

            }

            if (strcmp(dcb->r_uri,"CIM_DHCPProtocolEndpoint")==0)
            {
                if (value==2 || value==6)
                {
					onoffIntfDHCPv4(eth0, 1);
					DHCPv4Conf[eth0].DHCPv4State = InitState;
                }
                if (value==3)
					onoffIntfDHCPv4(eth0, 0);                    
				
                dcb->control=Cdhcp;
            }
            if (strcmp(dcb->r_uri,"CIM_IPProtocolEndpoint")==0)
            {
                dpconf->DHCPv4Enable= 0;
                dcb->control=Cdhcp;
            }
            if (strcmp(dcb->r_uri,"CIM_Sensor")==0)
            {
                if (value==2)
                    GPIOEnable();
                else
                    GPIODisable();
            }
            if (strcmp(dcb->r_uri,"CIM_Account")==0)
              if(dcb->count!=0)
                EnableUser(NULL,dcb->count,value);
              else
                dcb->status=2;           
        }
    }
    return;
}

void md_RequestPowerStateChange(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr,*ptr_end,time_flag=0,pstate;
    INT16U value;

#if 0
    ptr=strstr(str,":TimeoutPeriod>");
    if (ptr)
    {
        DEBUGMSG(DASH_DEBUG,"Find TimeoutPeriod\n");
        //invoke_ret=2;
        dcb->status = 2;
        return;
    }
#endif

    ptr=strstr(str,":Time>");
    if (ptr)
    {
        time_flag=1;
    }
    pstate=bsp_get_sstate();

    ptr=strstr(str,"PowerState>");
    if (ptr)
    {
        ptr += strlen("PowerState>");
        ptr_end = strchr(ptr, '<');
        *ptr_end = NULLCHR;
        value = atoi(ptr);

        if ((value==10 || value == 14) && pstate==S_S0)
            value=0x10;
        else if (value==2 && pstate!=S_S0)
        {
            strcpy(power_state,"2");
            value=0x11;
        }
        else if ((value == 12 || value == 6 || value==8) && pstate==S_S0)
        {
            strcpy(power_state,"8");
            value=0x12;
        }
        else if (value == 7 && pstate == S_S0) //hibernate
            value = 0x14;
        else if ((value == 3 || value == 4) && pstate == S_S0)//standby
            value = 0x15;
        else if(value ==5)
            value=0x13;
        else
            value=0;    

        if(value)
        {
          dcb->control=Cpowercontrol;
          new_power=value;
          dcb->status =0;
        }  
    }


    //  if(time_flag==1 && request_state!=2 && request_state!=5 && request_state!=6)
    //   invoke_ret=2;

    return;
}
void md_ApplyBootConfigSetting(INT8U *str)
{
    return;
}
void md_CreateBootConfigSetting(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr,*ptr_end,*ptr1;
    ptr=strstr(str,"StartingBootConfig>");
    if (ptr)
    {
        ptr=ptr+strlen("StartingBootConfig>");
        ptr1=ptr;
        //ptr = strchr(str,'>');
        ptr_end = strstr(ptr1, "StartingBootConfig>");
        ptr_end=strback(ptr_end,'<');
        if (ptr_end)
        {
            *ptr_end = NULLCHR;
            //   str = trimspace(ptr + 1);
            str = strltrim(ptr + 1);
            if (strlen(str))
            {
                StartingBoot=1;
                //invoke_ret=0;
                dcb->status = 0;
                return;
            }
            DEBUGMSG(DASH_DEBUG,"invoke=%s***\n",str);
        }
    }
    //invoke_ret=2;
    dcb->status = 2;
    return;
}

#ifdef ASF_BOOT_ONLY
void md_ChangeBootOrder(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr,*ptr_end,*ptr1,*ptr1_end;
    INT8U token[] = "Selector Name";
    INT8U idx=0;
    INT8U *tmp;

    tmp = getBootOrder();
    if (strstr(str, "<wsman:SelectorSet>"))
    {
        ptr = strstr(str,token);
        while (ptr)
        {
            ptr = strchr(ptr, '"');

            if (ptr)
                ptr_end = strchr(ptr+1, '"');
            *ptr_end = NULLCHR;
            // DEBUGMSG(DASH_DEBUG,">>%s\n",ptr+1);
            ptr1= strchr(ptr_end+1, '>');

            if (ptr1)
                ptr1_end = strchr(ptr1+1, '<');
            *ptr1_end = NULLCHR;
            //  DEBUGMSG(DASH_DEBUG, ">>%s %s\n",ptr+1,ptr1+1);
            //  idx=SearchIndex(ProfileEntry("CIM_BootSourceSetting"),ptr+1,ptr1+1);
            *(tmp+idx) = getBootID(ptr1+1);
#if 0
            if (strcmp(ptr1+1,"hd")==0)
                *(tmp + idx) = 2;
            else if (strcmp(ptr1+1,"cd")==0)
                *(tmp + idx) = 5;
            else if (strcmp(ptr1+1,"net")==0)
                *(tmp + idx) = 1;
            else if (strcmp(ptr1+1,"net")==0)
                *(tmp + idx) = 4;
            else
                *(tmp + idx) = 0;
            //setBootOrder(idx,tmp);
#endif
            //*(tmp+idx) =
            idx++;
            ptr = strstr(ptr1_end+1+strlen(ptr1+1),token);

        }
    }

    //invoke_ret=0;
    dcb->status = 0;
    return;
}
#endif

void md_ChangeBootOrderPldm(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr,*ptr_end;
    INT8U token[] = "Selector Name";
    INT8U idx=0,i=0;
    INT8U *buf,*seq;
    INT8U len;

    seq=boot_seq;
    // if (strstr(str, ":SelectorSet>"))
    if (strstr(str, ":SelectorSet>") || strstr(str, "<SelectorSet") )
    {

        ptr = strstr(str,token);

        while (ptr)
        {
            ptr = strchr(ptr, '"');
            if (ptr)
            {
                ptr_end = strchr(ptr+1, '"');
                *ptr_end = NULLCHR;
            }
            ptr=strstr(ptr+1,"InstanceID");
            if (!ptr)
            {
                *ptr_end='"';
                ptr=ptr_end+1;
                ptr = strstr(ptr,token);
                continue;
            }
            *ptr_end='"';
            ptr= strchr(ptr_end+1, '>');

            if (ptr)
            {
                ptr_end = strchr(ptr+1, '<');
                *ptr_end = NULLCHR;
            }

            //sscanf(ptr+1,"%d",&idx);
            idx = atoi(ptr+1);
            *(seq+i)=idx;
            i++;
            *ptr_end='<';
            ptr=ptr_end+1;
            ptr = strstr(ptr_end+1,token);

        }
        buf=malloc(64);
        memset(buf,0,64);
        len=CreateAttrVal(buf,seq);
        bsp_pldm_set(buf,len);
        free(buf);

    }

    //invoke_ret=0;
    dcb->status = 0;
    return;
}
void md_SetBootConfigRole(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr,*ptr_end;

    INT8U *buf,*seq,len=0;
    int val;
    dcb->status = 2;
    ptr=strstr(str,":Role>");
    if(ptr)
    {
        ptr=ptr+strlen(":Role>");
        ptr_end=strstr(ptr,"<");
        if (ptr_end)
            *ptr_end=NULLCHR;
        buf=malloc(64);
        memset(buf,0,64);

        seq=boot_seq;
        len=CreateAttrVal(buf,seq);
        if(len)
        {
            val=atoi(ptr);
            if(val==0)
                buf[3]=2;
            if(val==1)
                buf[3]=4;
            if(val==2)
                buf[3]=3;
            bsp_pldm_set(buf,len);
            dcb->status = 0;
        }
        free(buf);
    }
    return;
}
/*
void md_ChangeBootOrderPldm(DASHCB *dcb,INT8U *str)
{
    INT8U *buf,len;

    len=CreateAttrVal(&buf,dcb->ref_count);
    bsp_pldm_set(buf,len);
    dcb->status = 0;
    return;
}
*/

void md_CreateAccount(DASHCB *dcb,INT8U *str)
{
    UserInfo *info;
    INT8U *name, *end, *passwd;
    //INT8U tmpbuf[64];
    INT8U *tmpbuf;
    INT16U tmp;
    INT8U i=0,j=0;
    if (!(dcb->privilege & Account_pri))
    {
        dcb->status = 2;
        return;
    }

    str=strstr(str,":AccountTemplate");
    if (!str)
        return;
    end=strstr(str+strlen(":AccountTemplate"),":AccountTemplate");
    *end=NULLCHR;

    tmpbuf = malloc(64);

    if (str)
    {
        name = strstr(str, ":Name>");
        if (name)
        {
            name =name+strlen(":Name>");
            end =strchr(name+1,'<');
            *end= NULLCHR;
            passwd=end+1;
            passwd = strstr(passwd, ":UserPassword>");
            passwd = passwd+strlen(":UserPassword>");
            end = strchr(passwd+1,'<');
            *end = NULLCHR;
            if (strstr(passwd,"0x"))
            {
                memset(tmpbuf,0,64);
                tmp=str2hex(passwd+8);
                for (i=0,j=0; j<(tmp-4); j++)
                {
                    tmpbuf[j]=str2hex(passwd+10+i);
                    i=i+2;
                }
                //  tmpbuf[j]='\0';
                passwd=tmpbuf;
            }
        }
        else
        {
            name = strstr(str, ";Name=");
            if (name)
            {
                name = strchr(name+strlen(";Name="),'"');
                end =strchr(name+1,'"');
                *end= NULLCHR;
                passwd = end + 1;
                passwd = strstr(passwd, ";UserPassword=");
                passwd = strchr(passwd+strlen(";UserPassword="),'"');
                end = strchr(passwd+1,'"');
                *end = NULLCHR;
                name++;
                passwd++;
            }
        }

        //DEBUGMSG(DASH_DEBUG,"before created %s is %s\n", name, passwd);

        if ((info = adduser(name, passwd)) == NULL)
            dcb->status = 2;
        else
          {
            EnableUser(name,0,0); 
            dcb->status = 0;
          }  


    }

    free(tmpbuf);

    return;
}
void md_AssignRoles(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr_end,*str_org;
    UserInfo *acc_ptr;
    RoleInfo *role_p=rolehead;
    INT8U i=0,role=0,count=0;
    if (!(dcb->privilege & Role_pri))
    {
        dcb->status = 2;
        return;
    }

    str_org=str;
    str=strstr(str,":Identity");
    if (!str)
        return;
    ptr_end=strstr(str+strlen(":Identity"),":Identity");
    if(ptr_end)
        *ptr_end=NULLCHR;
    memset(dcb->ref_count,-1,16);
    SearchReference(dcb,str);
    if (ptr_end)
        *ptr_end=':';
    acc_ptr=getuserinfo();
    while (acc_ptr)
    {
        if (i==dcb->ref_count[0])
            break;
        i++;
        acc_ptr=acc_ptr->next;
    }

    str=str_org;
    str=strstr(str,":Roles");
    if(str)
    {
        str_org=str;
        while(str)
        {
            ptr_end=strstr(str+strlen(":Roles"),":Roles");
            if(ptr_end)
                str=strstr(ptr_end+strlen(":Roles"),":Roles");
        }
        if (ptr_end)
            *ptr_end=NULLCHR;
        memset(dcb->ref_count,-1,16);
        SearchReference(dcb,str_org);
        if (ptr_end)
            *ptr_end=':';
    }

    i=0;
    while (i<16)
    {
        if (dcb->ref_count[i]==0xff)
            break;
        count=0;
        role_p=rolehead;
        while(role_p)
        {
            if(count==dcb->ref_count[i])
            {
                role=role | role_p->mask;
                break;
            }
            role_p=role_p->next;
            count++;
        }
        i++;
    }
    setrole(acc_ptr->name,role);
    dcb->status = 0;

    return;
}
void md_ModifyRole(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr_end,*ptr_end1,*str_org,idx;
    INT8U i=0,count=0;
    RoleInfo *role_p=rolehead;
    RegProfile *pf=_RegProfile;
    INT32U priv=0;
    if (!(dcb->privilege & Role_pri))
    {
        dcb->status = 2;
        return;
    }
    str_org=str;
    str=strstr(str,":Role");
    if (!str)
        return;
    ptr_end=strstr(str+strlen(":Role"),":Role");
    if (ptr_end)
        *ptr_end=NULLCHR;
    memset(dcb->ref_count,-1,16);
    SearchReference(dcb,str);
    if(!dcb->ref_count[0])
    {
        dcb->status=2;
        return;
    }
    idx=dcb->ref_count[0];

    *ptr_end=':';
    str=str_org;
    str=strstr(str,":Privileges");
    while (str)
    {
        ptr_end=strstr(str+strlen(":Privileges"),":Privileges");
        if (ptr_end)
            ptr_end=ptr_end+strlen(":Privileges");
        str=strstr(str,":ActivityQualifiers>");
        str=str+strlen(":ActivityQualifiers>");
        ptr_end1=strstr(str,"<");
        if (ptr_end1)
            *ptr_end1=NULLCHR;
        i=0;
        while (pf[i].eltname)
        {
            if (strcmp(pf[i].priv_name,str)==0)
            {
                priv=priv | pf[i].priv;
                break;
            }
            i++;
        }
        str=strstr(ptr_end,":Privileges");
    }
    role_p=rolehead;
    count=0;
    while(role_p)
    {
        if(idx==count)
            role_p->privilege=priv;
        count++;
        role_p=role_p->next;
    }

    dcb->status = 0;
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    dirty_ext[ROLETBL].dirty=1;
#else
    setdirty(ROLETBL);
#endif
    return;
}
void md_CreateRole(DASHCB *dcb,INT8U *str)
{
    INT8U *name, *end,*end1,*str_next;
    INT8U RoleMask=0,i=0,SetBit=1,ret=0;

    RegProfile *pf=_RegProfile;
    RoleInfo *role_p;

    role_p=rolehead;
    while(role_p)
    {
        RoleMask=RoleMask | role_p->mask;
        role_p=role_p->next;
    }

    str=strstr(str,":Privileges>");
    str_next=strstr(str,":RoleTemplate>");
    if (!str_next)
        return;
    role_p = malloc(sizeof(RoleInfo));
    role_p->mask=0;
    role_p->privilege=0;
    while (str)
    {
        end=strstr(str+strlen(":Privileges>"),":Privileges>");
        if (end)
        {
            end=end+strlen(":Privileges>");
            *end=NULLCHR;
        }
        str=strstr(str,":ActivityQualifiers>");
        str=str+strlen(":ActivityQualifiers>");
        end1=strstr(str,"<");
        if (end1)
            *end1=NULLCHR;
        i=0;
        while (pf[i].eltname)
        {
            if (strcmp(pf[i].priv_name,str)==0)
            {
                role_p->privilege=role_p->privilege | pf[i].priv;
                break;
            }
            i++;
        }
        str=strstr(end+1,":Privileges>");
    }
    str=str_next;
    end=strstr(str+strlen(":RoleTemplate>"),":RoleTemplate>");
    *end=NULLCHR;

    if (str)
    {
        name = strstr(str, ":Name>");
        if (name)
        {
            name =name+strlen(":Name>");
            end =strchr(name+1,'<');
            *end= NULLCHR;
        }
    }

    strcpy(role_p->name,name);

    while(SetBit)
    {
        if(!(RoleMask & SetBit))
        {
            role_p->mask=SetBit;
            //   	RoleMask=RoleMask | SetBit;
            ret=AddRole(role_p);
            break;
        }
        SetBit=SetBit << 1;

    }
    if(ret)
        dcb->status = 2;
    else
        dcb->status=0;
    return;
}
void md_DeleteRole(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr_end;

    if (!(dcb->privilege & Role_pri))
    {
        dcb->status = 2;
        return;
    }
    str=strstr(str,":Role");
    if (!str)
        return;
    ptr_end=strstr(str+strlen(":Role"),":Role");
    if (ptr_end)
        *ptr_end=NULLCHR;
    memset(dcb->ref_count,-1,16);
    SearchReference(dcb,str);
    if(!dcb->ref_count[0])
    {
        dcb->status=2;
        return;
    }
    DeleteRole(dcb->ref_count[0]);
    //  setdirty(USERTBL);
    dcb->status = 0;
    return;
}

void md_FirmwareUpdate(DASHCB *dcb,INT8U *str)
{
    if (dcb->userinfo->role==0x7f)
    {
        dcb->control=Cfirmwareupdate;
        dcb->status=0;
    }
    else
        dcb->status=2;
    return;
}
void md_CreateOpaque(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr,*ptr_end;
    INT8U i;
    INT8U *opaquedata;
    UserInfo *acc_ptr;

    if (!strstr(str,":Owner"))
        return;
    ptr_end=strstr(strstr(str,":Owner")+strlen(":Owner"),":Owner");
    if (ptr_end)
        *ptr_end=NULLCHR;
    memset(dcb->ref_count,-1,16);
    SearchReference(dcb,str);
    acc_ptr=getuserinfo();
    i=0;
    while (acc_ptr)
    {
        if (i==dcb->ref_count[0])
            break;
        i++;
        acc_ptr=acc_ptr->next;
    }
    opaquedata=malloc(128);
    memset(opaquedata,0,128);
    for(i=0; i<OpaqueNum; i++)
    {
        read_opaque(i,0,128,opaquedata);
        if((((OPAQDATA*)opaquedata))->Status==0)
            break;
    }

    if(i<OpaqueNum)
    {
        *ptr_end=':';
        ptr=strstr(str,":ElementName>");
        ptr=ptr+strlen(":ElementName>");
        ptr_end=strchr(ptr,'<');
        if (ptr_end)
            *ptr_end=NULLCHR;

        create_opaque(i, 2048, NULL, ptr, acc_ptr->name);
        dcb->status = 0;
    }
    free(opaquedata);
    return;
}

void md_ReadWriteOpaque(DASHCB *dcb,INT8U *str)
{
    INT8U *opaquedata,*ptr,*ptr_end,ch,id,i,j=-1;
    int offset,length,lengthx=OpmMax,idx;
    ptr=strstr(str,":OpaqueManagementData");
    if (!ptr)
        return;
    ptr_end=strstr(ptr+strlen(":OpaqueManagementData"),":OpaqueManagementData");
    if (ptr_end)
    {
        ch=*ptr_end;
        *ptr_end=NULLCHR;
        memset(dcb->ref_count,-1,16);
        SearchReference(dcb,ptr);
        *ptr_end=ch;
        opaquedata=malloc(128);
        for(i=0; i<OpaqueNum; i++)
        {
            memset(opaquedata,0,128);
            read_opaque(i,0,128,opaquedata);
            if(((OPAQDATA*)opaquedata)->Status==1)
                j++;
            if(j==dcb->ref_count[0])
                break;
        }
        id=i;
        if(strcmp(dcb->userinfo->name,((OPAQDATA*)opaquedata)->Owner)!=0)
        {
            free(opaquedata);
            return;
        }
        else
            free(opaquedata);

    }
    if (i==OpaqueNum)
        return;

    ptr=strstr(str,":Offset>");
    if(ptr)
    {
        ptr=ptr+strlen(":Offset>");
        ptr_end=strchr(ptr,'<');
        *ptr_end=NULLCHR;
        offset=atoi(ptr)+128;
        *ptr_end='<';
    }
    ptr=strstr(str,":Length>");
    if(ptr)
    {
        ptr=ptr+strlen(":Length>");
        ptr_end=strchr(ptr,'<');
        *ptr_end=NULLCHR;
        length=atoi(ptr);
        *ptr_end='<';
    }
    if(length > OpmMax/2)
        return;


    if (dcb->control==ID_ReadOpaque)
    {
        OpaqueBuf=malloc(OpmMax);
        memset(OpaqueBuf,0,OpmMax);
        opaquedata=malloc(OpmMax);
        memset(opaquedata,0,OpmMax);
        read_opaque(id,offset,length+1,opaquedata);
        idx=0;
#ifdef UNICODE_FOR_OPAQUE
        while(idx <length)
        {
            if(*(opaquedata+idx)==0)
                *(opaquedata+idx)=0x20;
            idx++;
        }
#endif
        b64_encode(OpaqueBuf,&lengthx,opaquedata,length);
        free(opaquedata);
    }
    else
    {
        opaquedata=malloc(OpmMax);
        memset(opaquedata,0,OpmMax);
        ptr=strstr(str,":Data>");
        if (ptr)
        {
            ptr=ptr+strlen(":Data>");
            ptr_end=strchr(ptr,'<');
            *ptr_end=NULLCHR;
            b64_decode(ptr,opaquedata,length);
            write_opaque(id,offset,length,opaquedata);
        }
        free(opaquedata);
    }

    dcb->status = 0;
    return;
}
void md_ReassignOwnershipOpaque(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr,*ptr_end,*opaquedata,ch;
    INT8U id,i=0,j=-1;
    UserInfo *acc_ptr;

    ptr=strstr(str,":OpaqueManagementData");
    if(!ptr)
        return;
    opaquedata=malloc(128);
    ptr_end=strstr(ptr+strlen(":OpaqueManagementData"),":OpaqueManagementData");
    if(ptr_end)
    {
        ch=*ptr_end;
        *ptr_end=NULLCHR;
        memset(dcb->ref_count,-1,16);
        SearchReference(dcb,ptr);
        *ptr_end=ch;
        for(i=0; i<OpaqueNum; i++)
        {
            memset(opaquedata,0,128);
            read_opaque(i,0,128,opaquedata);
            if(((OPAQDATA*)opaquedata)->Status==1)
                j++;
            if(j==dcb->ref_count[0])
                break;
        }
        if(strcmp(dcb->userinfo->name,((OPAQDATA*)opaquedata)->Owner)!=0)
        {
            free(opaquedata);
            return;
        }
        id=i;
    }
    ptr=strstr(str,":NewOwner");
    if(ptr)
    {
        ptr_end=strstr(ptr+strlen(":NewOwner"),":NewOwner");
        if(ptr_end)
            *ptr_end=NULLCHR;

        memset(dcb->ref_count,-1,16);
        SearchReference(dcb,ptr);
        acc_ptr=getuserinfo();
        i=0;
        while (acc_ptr)
        {
            if(i==dcb->ref_count[0])
                break;
            i++;
            acc_ptr=acc_ptr->next;
        }
        memcpy(((OPAQDATA*)opaquedata)->Owner, acc_ptr->name, 16);
        write_opaque(id, 0, 128, (INT8U*)opaquedata);
        dcb->status = 0;
    }

    free(opaquedata);
    return;
}
void md_SetBIOSAttribute(DASHCB *dcb,INT8U *str)
{
    INT8U *ptr,*ptr_end,*ptr1,*ptr1_end;
    if(!AttrValBuf)
    {
      AttrValBuf=malloc(PLDM_ATT_TBL_SIZE);
      memset(AttrValBuf,0,PLDM_ATT_TBL_SIZE);
    }  
    ptr=strstr(str,":AttributeName");
    if(!ptr)
        return;
    ptr=strchr(ptr+strlen(":AttributeName"),'>');
    if(ptr)
    {
        ptr_end=strchr(ptr+1,'<');
        *ptr_end=NULLCHR;
    }
    ptr1=strstr(ptr_end+1,":AttributeValue");
    if(!ptr1)
        return;
    ptr1=strchr(ptr1+strlen(":AttributeValue"),'>');
    if(ptr1)
    {
        ptr1_end=strchr(ptr1+1,'<');
        *ptr1_end=NULLCHR;
    }
    SetValue(ptr+1,ptr1+1);
    dcb->status=0;
    return;
}
void md_RestoreBIOSDefaults(DASHCB *dcb,INT8U *str)
{
    bsp_pldm_set(AttrValBuf,AttrValOff);
    free(AttrValBuf);
    AttrValBuf=NULL; 
    AttrValOff=0;    
    dcb->status=0;
    return;
}
int CheckSensor()
{
    int i;
    
#if CONFIG_SENSOR_BY_AGENT 
    OSOOBHdr *hdr;    
    hdr = (OSOOBHdr *) DASH_OS_PSH_Buf;
    hdr->type = Get_HM_Sensor;
    OSSemPost(DASH_OS_Push_Event);
#endif
    
    for (i = 0 ; i < MAX_SENSOR_NUMS ; i++)
    {
        if (sensor[i].exist == SNR_READ)
            return 1;
    }
    return 0;
}
INT8U ParserBody(DASHCB *dcb, INT8U *str)
{
    INT8U *ptr, *ptr_end,*ptr_end1,*str_end,ch;
    INT8U *str_org = str;
    INT8U uri[64];
    INT8U *name;
    INT8U i,j;
    INT8S idx=-1;
    //  static const INT8U token[] = "Selector Name";
    plist *currentx;
    SubEvent *subinfo;
    int ip0,ip1,ip2,ip3,ret;
    INT8U ip_str[32];
    INT32U address;
    int err=0;
    dcb->fault_id = Fault_NoFault;
    i=0;
    j=0;


    name = malloc(256);

    while (bodyelement[i].elementName)
    {
        if (i==ID_IDENTIFY){
            sprintf(name,":%s", bodyelement[i].elementName);
        }
        //     else if (Ns[IDX_NS_CIM_CLASS].used && i == ID_INVOKE)
        else if (i == ID_INVOKE){
            sprintf(name,":%s%s", Method,bodyelement[i].elementName);
        }
        else{
            sprintf(name,":%s", bodyelement[i].elementName);
        }

	
	
        ptr = strstr(str, name);
        if (ptr)
            bodyelement[i].elementPtr = ptr + strlen(name);

        i++;
    }
    if (bodyelement[ID_IDENTIFY].elementPtr)
    {
        free(name);
        return 0;
    }

    i = 0 ;
    while (bodyelement[i].elementName)
    {
        str=str_org;
        if (!bodyelement[i].elementPtr)
        {
            i++;
            continue;
        }


        //   if (Ns[IDX_NS_CIM_CLASS].used && i == ID_INVOKE)
        if (i == ID_INVOKE)
        {
            sprintf(name,":%s%s>", Method,bodyelement[i].elementName);
            ptr_end = strstr(bodyelement[i].elementPtr+strlen(name), name);
        }
        else
        {
            sprintf(name,":%s>", bodyelement[i].elementName);
            ptr_end = strstr(bodyelement[i].elementPtr, name);
        }
        if (ptr_end)
            *ptr_end = NULLCHR;

        else
        {
            i++;
            continue;
        }

        str = bodyelement[i].elementPtr;
        if (str == NULL)
            continue;

        switch (i)
        {
        case ID_ENUMERATE:
            strcpy(dcb->q_uri,"");
            if (strstr(str, ":Expires>"))
            {
                dcb->fault_id = Fault_ExpirationTime;
                break;
            }
            dcb->PullOption=0;
            //both for assoication and associated
            if (strstr(str,":IncludeResultProperty"))
                dcb->PullOption =XMLFragment;


            if (strstr(str, ":ResourceURI>"))
            {

                ptr=strstr(str, XML_NS_CIM_CLASS);

                if (ptr)
                    ptr=ptr+strlen(XML_NS_CIM_CLASS)+1;
                ptr_end = strchr(ptr, '<');
                ch=*ptr_end;
                *ptr_end = NULLCHR;
                strcpy(ResourceURI,ptr);
                strcpy(dcb->r_uri,ptr);
                strcpy(dcb->ref_uri,ptr);
                *ptr_end=ch;
                if (AllClass==1)
                {
                    ReleaseBuf();
                    dcb->ProfilePtr = ReleaseProfile(dcb->ProfilePtr);
                    dcb->ProfilePtr=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,dcb->r_uri),NewMem());
                }

            }
            if(strstr(str, ":Filter Dialect=") && AllClass==0)
            {
                free(name);
                return Fault_FilterDialect;
            }
            if(strstr(str, "wsen:Filter") && strstr(str,"wsman:Filter"))
            {
                free(name);
                return Fault_InvalidFilter;
            }


            // if (strstr(str, "OptimizeEnumeration"))
            //     dcb->epr |= Optimize;
            if (strstr(str, "EnumerateEPR"))
                dcb->epr |= EPR;
            if (strstr(str, "EnumerateObjectAndEPR"))
                dcb->epr |= ObjectAndEPR;

            ptr = strstr(str, ":PolymorphismMode>");
            if (ptr)
            {
                ptr=ptr+strlen(":PolymorphismMode>");
                ptr_end = strchr(ptr, '<');
                ch=*ptr_end;
                *ptr_end = NULLCHR;


                if (strstr(ptr,"IncludeSubClassProperties"))
                {
                    *ptr_end=ch;
                    free(name);
                    return  Fault_PolyNotSupported;
                }
                if (strstr(ptr,"ExcludeSubClassProperties"))
                {
                    *ptr_end=ch;
                    free(name);
                    return  Fault_PolyNotSupported;
                }
                //  SubClassProperties=1;


            }

            str= str_org;
            ptr = strstr(str, ":Role>");
            if (ptr)
            {
                ptr=ptr+strlen(":Role>");
                ptr_end = strchr(ptr, '<');
                ch=*ptr_end;
                *ptr_end = NULLCHR;
                strcpy(Role,ptr);

                *ptr_end=ch;

            }
            else
                strcpy(Role,"");
            ptr = strstr(str, ":ResultRole>");
            if (ptr)
            {
                ptr=ptr+strlen(":ResultRole>");
                ptr_end = strchr(ptr, '<');
                ch=*ptr_end;
                *ptr_end = NULLCHR;
                strcpy(ResultRole,ptr);

                *ptr_end=ch;

            }
            ptr = strstr(str, ":AssociationClassName>");
            if (ptr)
            {
                ptr=ptr+strlen(":AssociationClassName>");
                ptr_end = strchr(ptr, '<');
                ch=*ptr_end;
                *ptr_end = NULLCHR;

                if (strcmp(Role,"")!=0)
                {
                    if (ProfileEntry(dcb, ptr))
                    {

                        currentx=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,ptr),TmpNewMem());
                        if (!GetElement(currentx,Role,0))
                        {
                            *ptr_end=ch;
                            TmpReleaseBuf();
                            ReleaseProfile(currentx);
                            free(name);
                            return Fault_InvalidFilter;
                        }
                        *ptr_end=ch;
                        TmpReleaseBuf();
                        ReleaseProfile(currentx);
                    }
                }

                if (strstr(ptr,"CIM_AssignedIdentity") || strstr(ptr,"CIM_BindsTo"))
                    dcb->PullOption=OneTime;
                *ptr_end=ch;

            }


            ptr = strstr(str, ":ResultClassName>");
            if (ptr)
            {
                ptr=ptr+strlen(":ResultClassName>");
                ptr_end = strchr(ptr, '<');
                ch=*ptr_end;
                *ptr_end = NULLCHR;
                strcpy(dcb->q_uri,ptr);
                if(AllClass==1)
                    strcpy(dcb->r_uri,ptr);
                if(strcmp(dcb->r_uri,"CIM_PhysicalElement")==0 && strcmp(dcb->ref_uri,"CIM_Memory")==0)
                {
                    if( dcb->ref_count[0]==0)
                        strcpy(dcb->r_uri,"CIM_PhysicalMemory");
                    else
                        strcpy(dcb->r_uri,"CIM_XXX");
                    strcpy(dcb->q_uri,"");
                }
                if(strcmp(dcb->r_uri,"CIM_NumericSensor")==0 && CheckSensor() >0)
                {
                    //OSSemPost(PLDMSNRSem);
                    OSTaskResume(TASK_SNR_READ_ID);
                    OSTimeDly(1);
                }
                if (strstr(str,":AssociationInstances") && strcmp(Role,"")!=0)
                {
                    currentx=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,ptr),TmpNewMem());
                    if (currentx)
                    {
                        if (!GetElement(currentx,Role,0))
                        {
                            *ptr_end=ch;
                            TmpReleaseBuf();
                            ReleaseProfile(currentx);
                            free(name);
                            return Fault_InvalidFilter;
                        }
                        TmpReleaseBuf();
                        ReleaseProfile(currentx);
                    }
                }

                *ptr_end=ch;


            }

            //CQL Parsering
            ptr = strstr(str, "http://schemas.dmtf.org/wbem/cql/1/dsp0202.pdf");
            if (ptr)
            {
                dcb->PullOption=SQLQuery;
                ptr = strstr(ptr, "FROM");
                if (ptr)
                {
                    ptr=ptr+strlen("FROM");
                    ptr_end = strstr(ptr, "<");
                    if (ptr_end)
                        *ptr_end = NULLCHR;
                    strcpy(dcb->r_uri, trimspace(ptr));
                    if (ptr_end)
                        *ptr_end='<';
                }
            }

            break;

        case ID_RELEASE:
            ptr = strstr(str, "<wsen:EnumerationContext>");
            if (ptr)
            {
                ptr += strlen("<wsen:EnumerationContext>");
                ptr_end = strstr(str, "</wsen:EnumerationContext>");
                *ptr_end = NULLCHR;

                if (!strcmp(ptr, "0"))
                {
                    free(name);
                    return Fault_InvalidEnumContext;
                }
                if (!strstr(ptr, "-"))
                {
                    free(name);
                    return Fault_InvalidEnumContext;
                }
            }
            break;

        case ID_PULL:
            if (strstr(str, "<wsen:EnumerationContext/>"))
            {
                free(name);
                return Fault_InvalidEnumContext;
            }

            ptr = strstr(str, "<wsen:EnumerationContext>");
            if (ptr) //must be
            {
                ptr += strlen("<wsen:EnumerationContext>");
                ptr_end = strstr(str,"</wsen:EnumerationContext>");
                *ptr_end = NULLCHR;
                if (!strchr(ptr, '-'))
                {
                    free(name);
                    return Fault_InvalidEnumContext;
                }
            }

            break;
        case ID_INVOKE:
            //invoke_ret=2;
            idx=-1;
            dcb->status = 2;
            ptr=strback(str,':');
            ptr++;
            //ptr=strstr(ptr,"CreateBootConfigSetting");

            ptr_end = strchr(ptr,'_');
            *ptr_end=NULLCHR;
            DEBUGMSG(DASH_DEBUG,"invoke0=%s\n",ptr);
            j=0;
            while (method[j].elementName)
            {
                if (strcmp(method[j].elementName,ptr)==0)
                {
                    idx=j;
                    dcb->control=idx;
                    break;
                }
                j++;
            }

            if (idx >=0)
            {
                *ptr_end='_';
                ptr=ptr_end+1;
                if (ptr)
                {

                    DEBUGMSG(DASH_DEBUG,"######idx=%d\n",idx);

                    if (idx==ID_RequestStateChange)
                        md_RequestStateChange(dcb, ptr);
                    else if (idx==ID_RequestPowerStateChange)
                        md_RequestPowerStateChange(dcb,ptr);
                    else if (idx==ID_CreateBootConfigSetting)
                        md_CreateBootConfigSetting(dcb,ptr);
                    else if (idx==ID_ApplyBootConfigSetting)
                        md_ApplyBootConfigSetting(ptr);
                    else if (idx==ID_CreateAccount)
                        md_CreateAccount(dcb,ptr);
                    else if (idx==ID_AssignRoles)
                        md_AssignRoles(dcb,ptr);
                    else if (idx==ID_CreateRole)
                        md_CreateRole(dcb,ptr);
                    else if (idx==ID_ModifyRole)
                        md_ModifyRole(dcb,ptr);
                    else if (idx==ID_DeleteRole)
                        md_DeleteRole(dcb,ptr);
                    else if (idx==ID_ChangeBootOrder && pldmdata.ptr[0]!=NULL)
                        md_ChangeBootOrderPldm(dcb,ptr);
                    else if (idx==ID_FirmwareUpdate)
                        md_FirmwareUpdate(dcb,ptr);
                    else if (idx==ID_CreateOpaque)
                        md_CreateOpaque(dcb,ptr);
                    else if (idx==ID_ReadOpaque || idx==ID_WriteOpaque)
                        md_ReadWriteOpaque(dcb,ptr);
                    else if (idx==ID_ReassignOwnershipOpaque)
                        md_ReassignOwnershipOpaque(dcb,ptr);
                    else if (idx== ID_SetBIOSAttribute)
                        md_SetBIOSAttribute(dcb,ptr);
                    else if (idx== ID_RestoreBIOSDefaults)
                        md_RestoreBIOSDefaults(dcb,ptr);                            
                    else if (idx== ID_SetBootConfigRole)
                        md_SetBootConfigRole(dcb,ptr);

#ifdef ASF_BOOT_ONLY
                    else
                        md_ChangeBootOrder(dcb,ptr);
#endif

                }
            }

            break;
        case ID_SUBSCRIBE:

            subinfo = malloc(sizeof(SubEvent));
            memset(subinfo, 0, sizeof(SubEvent));

            ptr=strstr(str,":Filter");
            if (ptr)
            {
                generate_uuid(xuuid);
                strcpy(subinfo->subid,xuuid);

                ptr=ptr+strlen(":Filter");
                str_end=strstr(ptr,":Filter>");
                if (str_end)
                {
                    *str_end = NULLCHR;
                    if (!strstr(ptr,"Dialect="))
                    {
                        *str_end=':';
                        free(subinfo);
                        free(name);
                        return Fault_EventSourceUnableToProcess;
                    }
                    if (strstr(ptr,WSM_WQL_FILTER_DIALECT) || strstr(ptr,XML_NS_CIM_ALL_CLASS) || strstr(ptr,WSM_CQL_FILTER_DIALECT))
                    {
                        ptr=strchr(ptr,'>')+1;

                        ptr_end = strchr(ptr,'<');
                        *ptr_end = NULLCHR;

                        //subinfo->query=malloc(strlen(ptr)+1);
                        if (strlen(ptr) >max_data_size)
                            err=Fault_EventSourceUnableToProcess;
                        else
                            strcpy(subinfo->query,ptr);
                        *ptr_end='<';
                    }
                    *str_end=':';
                }
            }
            else if (strcmp(dcb->r_uri,"CIM_FilterCollection")==0)
                sprintf(subinfo->query,"FilterCollection:%d",dcb->count);

            else
                strcpy(subinfo->query,"SELECT * FROM CIM_AlertIndication");
            if (strstr(str,":NotifyTo>"))
            {
                ptr = strstr(str, ":Address>");
                ptr += strlen(":Address>");
                ptr_end = strchr(ptr,'<');
                if (ptr_end)
                    *ptr_end = NULLCHR;

                ptr=strstr(ptr,"http://");
                if (ptr)
                {
                    ptr=ptr+strlen("http://");
                    ptr_end1=strchr(ptr,':');
                    if (ptr_end1)
                        *ptr_end1 = NULLCHR;
                    ret=sscanf(ptr,"%d.%d.%d.%d",&ip0,&ip1,&ip2,&ip3);
                    if (ret==0)
                    {
                        rtGethostbyname_F(IPv4,ptr,(INT8U *) &address);
                        sprintf(ip_str,"%d.%d.%d.%d",address&0xff,(address>>8) & 0xff,(address>>16)&0xff,address>>24);
                        if (ptr_end1)
                            *ptr_end1=':';
                        sprintf(subinfo->notifyto,"http://%s%s",ip_str,ptr_end1);
                    }
                    else
                    {
                        if (ptr_end1)
                            *ptr_end1=':';
                        if (strlen(ptr) >max_data_size)
                            err=Fault_EventSourceUnableToProcess;
                        else
                            sprintf(subinfo->notifyto,"http://%s",ptr);
                    }

                    if (ptr_end)
                        *ptr_end='<';
                }

            }
            if (strcmp(subinfo->notifyto,"")==0)
                err= Fault_EventSourceUnableToProcess;

            if (err!=0)
            {
                free(subinfo);
                free(name);
                return Fault_EventSourceUnableToProcess;
                //strcpy(subinfo->notifyto,"http://127.0.0.1:8080");
            }

            generate_uuid(xuuid);
            strcpy(subinfo->subid,xuuid);
            dcb->fault_id = subscribe(dcb,subinfo);
            break;

        case ID_REFERENCE:
            memset(dcb->ref_count,-1,16);
            idx=0;
            sprintf(name,":%s>", WSA_REFERENCE_PARAMETERS);
            while (1)
            {
                ptr=strstr(str,":ResourceURI>");
                if (ptr)
                {
                    if (strstr(ptr,":SelectorSet/>"))
                    {
                        *ptr_end=':';
                        break;
                    }
                    ptr_end1=strchr(ptr+strlen(":ResourceURI>"),'<');
                    *ptr_end1=NULLCHR;
                    if (strstr(ptr,"http://"))
                    {
                        ptr = strrchr(ptr, '/') + 1;
                        strcpy(uri,ptr);
                    }
                    if (ptr_end1)
                        *ptr_end1='<';
                    ptr=strstr(ptr_end1+1, ":SelectorSet>");
                    ptr=ptr+strlen(":SelectorSet>");

                    currentx=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,uri),TmpNewMem());

                    SearchKey(dcb,ptr,currentx);

                    dcb->ref_count[idx]=dcb->count;
                    TmpReleaseBuf();
                    ReleaseProfile(currentx);
                }
                idx++;
                *ptr_end=':';
                str=strstr(ptr_end+1+strlen(name), name);
                if (str)
                    ptr_end=strstr(str+1+strlen(name), name);
                else
                    break;
            }

            break;

        }
        i++;
    }

    free(name);
    return dcb->fault_id;
}
INT8U check_envelope(DASHCB *dcb,INT8U index)
{

    INT8U *ptr,*ptr1_end;
    INT8U *envelope = element[index].elementPtr,*env_tmp;

    static INT8U token[] = "Selector Name";
    plist  *current;
    XMLElt *XMLPtr;
    //static INT8U uuid[255] = {0x00};
    static INT8U *uuid = NULL;
    INT8U tmp_str[64]= {0};
    RegProfile *pf=_RegProfile;
    dcb->fault_id = Fault_NoFault;


    if (uuid == NULL)
    {
        uuid = malloc(255);
        memset(uuid, 0, 255);
    }
    switch (index)
    {
    case ID_RESOURCE_URI:

        //     if (SQLQuery)
        //         break;

        if (strstr(envelope,"http://"))
        {
            if (strstr(envelope, "schemas."))
                ptr = strrchr(envelope, '/') + 1;
            else
                dcb->fault_id = Fault_DestUnReachable;
        }
        else
        {
            //not a http://xxx/yyy/zzz format
            //ptr = envelope + 14;   //for skipping "classnamespace"
            ptr = envelope;
        }

        // if (fault_id  == Fault_NoFault && EPR == 0)

        if (dcb->ref==1)
        {
            strcpy(dcb->ref_uri,ptr);
            break;
        }
        if (dcb->fault_id  == Fault_NoFault)
        {
            if (strcmp(ptr,"*") == 0 || (dcb->PullOption & SQLQuery))
            {
                AllClass = 1;
                if(strcmp(dcb->q_uri,"")==0)
                    strcpy(dcb->q_uri,"CIM_ComputerSystem");
                strcpy(dcb->r_uri,dcb->q_uri);

            }
            else
            {
                AllClass = 0;
                strcpy(dcb->r_uri, ptr);
            }
        }

		
        if(strcmp(dcb->r_uri,"CIM_Sensor")==0)
            strcpy(dcb->r_uri,"CIM_NumericSensor");
        if (strcmp(dcb->r_uri,"CIM_System")==0 || strcmp(dcb->r_uri,"CIM_ManagedElement")==0)
            strcpy(dcb->r_uri,"CIM_ComputerSystem");
        if (strcmp(dcb->r_uri,"CIM_ListenerDestination")==0)
            strcpy(dcb->r_uri,"CIM_ListenerDestinationWSManagement");
        if (strcmp(dcb->r_uri,"")!=0)
        {
            XMLPtr=ProfileEntry(dcb,dcb->r_uri);
            dcb->URI=XMLPtr;
            if(XMLPtr==_ManagedSystemElement)
            {
                strcpy(dcb->r_uri,pf[dcb->RegClass].eltname);
                XMLPtr=ProfileEntry(dcb,dcb->r_uri);
            }
            if(XMLPtr==_PhysicalElement || XMLPtr==_PhysicalPackage || XMLPtr==_PhysicalComponent || XMLPtr==_PhysicalConnector || XMLPtr==_PhysicalFrame)
            {			
                strcpy(dcb->r_uri,XMLPtr[dcb->SubClass].eltname);		
                XMLPtr=ProfileEntry(dcb,dcb->r_uri);		
            }
            if(XMLPtr==_ElementConformsToProfile)
            {
                while(pf[dcb->RegClass].eltname)
                {					
                    if(strcmp(pf[dcb->RegClass].eltname,"CIM_PhysicalElement")==0)
                    {
                        XMLPtr=_PhysicalElement;
                        strcpy(tmp_str,XMLPtr[dcb->SubClass].eltname);
                        XMLPtr=ProfileEntry(dcb,XMLPtr[dcb->SubClass].eltname);
                        current=DefaultInstance(dcb,XMLPtr,TmpNewMem());
                    }
                    else
                    {	
                        current=DefaultInstance(dcb,ProfileEntry(dcb,pf[dcb->RegClass].eltname),TmpNewMem());
                        strcpy(tmp_str,pf[dcb->RegClass].eltname);
                    }
                    TmpReleaseBuf();
                    current = ReleaseProfile(current);
                    if(dcb->InstanceCount)
                    {
                        dcb->ProfilePtr=DefaultElementConformsInstance(dcb,&tmp_str,NewMem());
                        break;
                    }
                    (dcb->RegClass)++;
                }
            }
            else{		  
			dcb->ProfilePtr=DefaultInstance(dcb,XMLPtr,NewMem());			
		}
        }

        break;


    case ID_ACTION:
        if (element[ID_ACTION].elementPtr != NULL)
            ParserAction(dcb,element[ID_ACTION].elementPtr);

        if(dcb->actiontype==WS_DISP_TYPE_RELEASE)
            dcb->actiontype=WS_DISP_TYPE_RELEASE;

        break;

    case ID_REPLY_TO:
        break;

    case ID_TO:

        if (element[ID_TO].elementPtr)
        {
            // if(!strstr(envelope, local_ip) && !strstr(envelope,dpconf->HostName) && !strstr(envelope,local_HostIPv6))
            sprintf(tmp_str,":%d",dpconf->DASHPort);
            if (!strstr(envelope, tmp_str))
            {
                sprintf(tmp_str,":%d",dpconf->DASHTLSPort);
                if (!strstr(envelope, tmp_str))
                    dcb->fault_id = Fault_DestUnReachable;
            }
        }
        break;

    case ID_MESSAGE_ID:
        if (!strstr(envelope, "uuid:"))
            dcb->fault_id = Fault_InvalidMessage;
        //   if (!strcmp(envelope, uuid))
        //       fault_id = Fault_DuplicateMessage;
        strcpy(uuid, envelope);

        break;
    case ID_SELECTOR_SET:
        // FoundSelector=0;

        //   p = dcb->ProfilePtr;

        env_tmp = strstr(envelope,token);

        if (envelope == NULL)
            return Fault_MissingSelectorFault;
        current=dcb->ProfilePtr;
        if (dcb->ref==1)
        {
            current=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,dcb->ref_uri),TmpNewMem());
            SearchKey(dcb,env_tmp,current);
            TmpReleaseBuf();
            ReleaseProfile(current);
            dcb->ref=0;
        }
        else
            SearchKey(dcb,env_tmp,dcb->ProfilePtr);

        if(strcmp(dcb->r_uri,"CIM_FilterCollection")==0 && dcb->fault_id==Fault_InvalidNameFault)
            dcb->fault_id=0;

        break;


    case ID_MAX_ENVELOPE_SIZE:
        if (*envelope == '0')
            //should check this before send the packet
            dcb->fault_id = Fault_MaxEnvelopeFault;
        break;

    case ID_OPERATION_TIMEOUT:

        ptr = strstr(envelope, "P");
        if (!ptr)
            dcb->fault_id =  Fault_InvalidTimeOut;
        ptr = strstr(envelope, "PT0S");
        if (ptr) //time 0 seconds
            dcb->fault_id = Fault_OperationTimeout;
        break;

    case ID_OPTIONSET:

        ptr1_end= strback(envelope, '"');
        ptr=strback(ptr1_end-1,'"');
        if (ptr1_end)
        {
            *ptr1_end=NULLCHR;
            if (strcmp(ptr+1,"true")==0)
                dcb->fault_id= Fault_NotUnderstood;
            *ptr1_end='"';
        }
        break;
    }
    return dcb->fault_id;


}
INT8U ParserElement(DASHCB *dcb,INT8U *str)
{

    INT8U i;
    INT8U token[] = ":Header>";
    INT8U wsheader[] = "<";
    INT8U wstailer[] = ">";
    INT8U *str_end, *cmp, *cmp_end, *ptr;
    // INT8U fault_id = Fault_NoFault;
    INT8U tmp_name[255];
    INT8U *name;
    INT8U reference=0;
//   dcb->association=0;
    dcb->ref=0;
    str_end = strstr(str,token);
    if (!str_end)
        return 0;
    //assert(str_end);
    str_end=strback(str_end,'<');
    *str_end = NULLCHR;

    name = malloc(255);

    while (str)
    {
        str = strstr(str,wsheader);  //find a leading '<'

        if (str == NULL || str >= str_end)
            //  return fault_id;
            break;
        ptr = strstr(str,wstailer);  //find a following '>'
        if (*(str+1)=='/')
        {
            str=ptr+1;
            continue;
        }
        *ptr = NULLCHR;
        cmp_end = strpbrk(str, whitestr);
        if (cmp_end)
        {
            *cmp_end = NULLCHR;
            *ptr = '>';
        }

        if (*(ptr-1) == '/') //found a <xxx:yyy /> pattern
        {
            //   *(ptr-1) = NULLCHR;
            //   name[0]  = NULLCHR;
            str=ptr+1;
            continue;
        }
        else
            sprintf(name, "</%s>", str+1);

        cmp_end  = ptr + 1;
        str = strchr(str, ':');  //wxxx:
        //str points to the start address of element
        //cmp points to the start address of the content associated with element
        if (name != NULL)
        {
            cmp_end = strstr(ptr+1,name);  //find a leading "</xxx>"

            cmp_end = strrtrim(cmp_end);
            *cmp_end = NULLCHR;
        }

        i = 0;
        while (element[i].elementName)
        {
            sprintf(tmp_name, ":%s", element[i].elementName);
            //if(strcmp(element[i].elementName, str) == 0)
            if (strstr(str,tmp_name))
            {
                cmp = ptr + 1;
                element[i].elementPtr = cmp;
                /*
                                if (name != NULL)
                                    cmp_end = strstr(cmp,name);  //find a leading "</xxx>"

                                cmp_end = strrtrim(cmp_end);
                                *cmp_end = NULLCHR;
                */

                if (i==ID_SELECTOR_SET)
                {
                    ptr=strstr(str+strlen(":SelectorSet>"),WSA_REFERENCE_PARAMETERS);
                    if (ptr)
                    {
                        dcb->ref=1;
                        reference=1;
                        *cmp_end='<';
                        //  str=strback(ptr,'<');
                        ptr=strstr(ptr,":ResourceURI>");
                        str=strback(ptr,'<');
                        break;
                    }

                }
                if (i==ID_SELECTOR)
                {
                    ptr=strstr(str+strlen(":Selector>"),WSA_REFERENCE_PARAMETERS);
                    if (ptr)
                    {
                        dcb->ref=1;
                        reference=1;
                        *cmp_end='<';
                        //    str=strback(ptr,'<');
                        ptr=strstr(ptr,":ResourceURI>");
                        str=strback(ptr,'<');
                        break;
                    }

                }


                //find the first fault, still need to process the whole header
                if (dcb->fault_id == Fault_NoFault)
                    dcb->fault_id = check_envelope(dcb,i);

                break;
            }
            i++;
        }

        if (reference==0)
            str = cmp_end + strlen(name);
        reference=0;

    }
    dcb->StartOff=SearchEnd(dcb->ProfilePtr,dcb->count,NULL);
    //  if(dcb->actiontype==WS_DISP_TYPE_ENUMERATE && AllClass==0)
    //     strcpy(dcb->p_uri,dcb->r_uri);

    free(name);
    if(dcb->actiontype==WS_DISP_TYPE_ENUMERATE && strcmp(dcb->r_uri,"CIM_NumericSensor")==0 && CheckSensor()>0)
    {
        //OSSemPost(PLDMSNRSem);
        OSTaskResume(TASK_SNR_READ_ID);
        OSTimeDly(1);
    }

    if ((dcb->actiontype==WS_DISP_TYPE_PULL || dcb->actiontype==WS_DISP_TYPE_RELEASE) && dcb->PreURI!=dcb->URI && AllClass==0)
        dcb->fault_id=Fault_MsgInfoHdrRequired;
    return dcb->fault_id;
}


/* parseelt parse the xml stream and
 * call the callback functions when needed... */
INT8U parseelt(DASHCB *dcb)
{

    INT8U *str;
    INT8U ret;


    str = ParserNsData(dcb,dcb->rxbuf);
    ret = ParserElement(dcb,str);

    return ret;

}

INT8U check_missing(DASHCB *dcb)
{

    if (!element[ID_RESOURCE_URI].elementPtr && dcb->actiontype!=WS_DISP_TYPE_UNSUBSCRIBE && dcb->actiontype != WS_DISP_TYPE_IDENTIFY)
        return Fault_DestUnReachable;
    else if (!element[ID_ACTION].elementPtr)
        return Fault_ActionFault;
    else if (!element[ID_REPLY_TO].elementPtr)
        return Fault_MsgInfoHdrRequired;
    else if (!element[ID_TO].elementPtr)
        return Fault_DestUnReachable;
    else if (!element[ID_MESSAGE_ID].elementPtr)
        return Fault_InvalidMessage;

    return Fault_NoFault;

}

//+briank.rtk : just for debug -----------------------------------
void debug_printfIP(int idx){

	printf("[RTK](%d) Host Ip=0x%x\n",idx,dpconf->HostIP[eth0].addr);	
}

//*briank.rtk : just for debug -----------------------------------
void ConnectControl(DASHCB *dcb, unsigned char intf)
{
    INT8U *tmpptr,index;
    unsigned int address;
    char error;
    OS_CPU_SR cpu_sr = 0;
#if CONFIG_VERSION > 2
    //INT8U cpu;
    //struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
#endif
    if(dcb->control == NotReady)
        return;
    switch (dcb->control)
    {
    case Cdhcp:
        if (dpconf->DHCPv4Enable&(1<<intf))
        {
            //if(bsp_bits_get(IO_CONFIG1, BIT_DRIVERRDY, 1))
            if(bsp_in_band_exist())
                dcb->status = DASH_FAIL;
            else{			
                error = OSTaskResume(TASK_DHCPv4_PRIO);
            	}
        }
        else if (dpconf->DHCPv6Enable &(1<<intf))
        {
            //if(bsp_bits_get(IO_CONFIG1, BIT_DRIVERRDY, 1))
            if(bsp_in_band_exist())
                dcb->status = DASH_FAIL;
            else
                error = OSTaskResume(TASK_DHCPv6_PRIO);
        }
        else
        {
            address = 0;
            tmpptr = new_ip;
            for (index = 0; index < 4; index++)
            {
                address += atoi(tmpptr) << ((3-index)*8);
                tmpptr = strchr(tmpptr, '.') + 1;
            }
            setIPAddress_F(address, intf);
            address = 0;
            tmpptr = new_netmask;
            for (index = 0; index < 4; index++)
            {
                address += atoi(tmpptr) << ((3-index)*8);
                tmpptr = strchr(tmpptr, '.') + 1;
            }
            setSubnetMask_F(address, intf);
            address = 0;
            tmpptr = new_gateway;
            for (index = 0; index < 4; index++)
            {
                address += atoi(tmpptr) << ((3-index)*8);
                tmpptr = strchr(tmpptr, '.') + 1;
            }
            setGateWayIP_F(address, intf);

        }

        break;

    case Ctextredirection:
#ifdef CONFIG_TCR_ENABLED
        dcb->qhdr->contype = 0xff;
        if (strcmp(dcb->r_uri,"CIM_TextRedirectionSAP")==0)
        {
            if(dcb->count==0)
                dcb->qhdr->contype = 1;
            //tcrcb.contype =1;
            else
                dcb->qhdr->contype = 0;
        }

        //tcrcb.contype =0;
        OS_ENTER_CRITICAL();
        if (tcrcb.state >= ACCEPTED)
            tcrcb.state = DISCONNECTED;
        if (dcb->qhdr->cmd != SRV_STOP)
            tcrcb.type = dcb->qhdr->option;
        OS_EXIT_CRITICAL();
        if (dcb->qhdr->cmd == SRV_STOP)
            free(dcb->qhdr);
        else if (tcrcb.state != LISTENED && dcb->qhdr->cmd == SRV_RESTART && dcb->qhdr->contype !=0xff)
        {
            //wait a pre-created TCR to be deleted
            while (tcrcb.state != DISABLED )
                OSTimeDly(5);

            OSTaskCreateExt (SOLTask,
                             (void *) dcb->qhdr,
                             (OS_STK *) & TaskSOLStk[TASK_SOL_STK_SIZE - 1],
                             TASK_SOL_PRIO,
                             TASK_SOL_ID,
                             (OS_STK *) & TaskSOLStk[0],
                             TASK_SOL_STK_SIZE,
                             (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
        }
#endif
        break;

    case Cusbredirection_Connect:
#ifdef CONFIG_USB_ENABLED

	#ifdef CONFIG_CPU_SLEEP_ENABLED		
	
	if(usbcb.usbstate == DISCONNECTED && dcb->qhdr->cmd ==SRV_STOP){
		//USB disconnect 
		//printf("[RTK] EnableCPUSleep from USB(Listen Mode)\n");
		 EnableCPUSleep();	
	}else if((dcb->qhdr->cmd ==SRV_RESTART)||(dcb->qhdr->cmd ==SRV_START)){
		//printf("[RTK] DisableCPUSleep from USB(Listen Mode)\n");
		DisableCPUSleep();	
	}
	#endif

        if (usbcb.usbstate != DISABLED)
        {
            usbcb.delrequest = 1;
            stopUSB();
            usbcb.delrequest = 0;
        }

        if (dcb->qhdr->cmd == SRV_STOP)
            free(dcb->qhdr);
        else if (dcb->qhdr->cmd == SRV_RESTART)
        {

            OSTaskCreateExt (USBClientTask,
                             (void *) dcb->qhdr,
                             (OS_STK *) & TaskUSBStk[TASK_USB_STK_SIZE - 1],
                             TASK_USB_PRIO,
                             TASK_USB_ID,
                             (OS_STK *) & TaskUSBStk[0],
                             TASK_USB_STK_SIZE,
                             (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

        }

#endif
        break;
    case Cusbredirection_Listen:

	//printf("[RTK]  Connection Conrol: Cusbredirection_Listen\n");
		
#ifdef CONFIG_USB_ENABLED

	#ifdef CONFIG_CPU_SLEEP_ENABLED		
	
	if(usbcb.usbstate == DISABLED && dcb->qhdr->cmd ==SRV_STOP){
		//USB disconnect 
		//printf("[RTK] EnableCPUSleep from USB(Listen Mode)\n");
		 EnableCPUSleep();	
	}else if((dcb->qhdr->cmd ==SRV_RESTART)||(dcb->qhdr->cmd ==SRV_START)){
		//printf("[RTK] DisableCPUSleep from USB(Listen Mode)\n");
		DisableCPUSleep();	
	}
	#endif

        if (usbcb.usbstate != DISABLED)
        {
            usbcb.delrequest = 1;
            stopUSB();
            usbcb.delrequest = 0;
        }

        if (dcb->qhdr->cmd == SRV_STOP)
            free(dcb->qhdr);
        else if (dcb->qhdr->cmd == SRV_RESTART)
        {
            OSTaskCreateExt (USBSrvTask,
                             (void *) dcb->qhdr,
                             (OS_STK *) & TaskUSBStk[TASK_USB_STK_SIZE - 1],
                             TASK_USB_PRIO,
                             TASK_USB_ID,
                             (OS_STK *) & TaskUSBStk[0],
                             TASK_USB_STK_SIZE,
                             (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
        }

#endif
        break;
    case Cfirmwareupdate:
        //if ((bsp_AP_exist() & dpconf->restart) || !bsp_bits_get(IO_CONFIG1, BIT_DRIVERRDY, 1))
	//debug_printfIP(0);

	//printf("[RTK]  Connection Conrol: Cfirmwareupdate\n");
		

	if(dpconf->DHCPv4Enable){//+briank.rtk : change to static mode automatically.		
		dpconf->DHCPv4Enable = 0x00;
		dpconf->isDHCPv4Enable = 0x01;
		OSTimeDly(OS_TICKS_PER_SEC*0.5);
	}
	else{
		dpconf->isDHCPv4Enable = 0x00;
	}
		
		
        dcb->status=0;

//firmware update only supported after Version B
#if CONFIG_VERSION > 2
#if 0
        spi_se_erase(ssi_map, 0);
        bsp_wdt_kick();
        cpu = REG8(0xB9000004);
        REG8(0xB9000004) = (cpu | 0x08);
#endif
        if(dcb->status==0)
        {
            writesig(dcb->rxbuf, FWUPDATE, 0);
			//debug_printfIP(1);
            setreset(DPCONFIGTBL);
			//debug_printfIP(2);
            setdirty(DPCONFIGTBL);
			//debug_printfIP(3);
            OSTaskChangePrio( TASK_FLASH_PRIO, TASK_HIGH_PRIO);
            OSTaskResume( TASK_FLASH_PRIO );
			//debug_printfIP(4);
        }
        else
            dcb->status = DASH_FAIL;
#else
        dcb->status = DASH_FAIL;
#endif
        break;
    case Cpowercontrol:
        SMBus_Prepare_RmtCtrl(new_power, 0);
        break;
    case Csubscribe:
        change_sub=1;
        break;
    case Cunsubscribe:
        change_sub=1;
        break;
    default:
        break;

    }
    /*
        if (tcrcb.state == DISABLED)
        {
            qhdr = malloc(sizeof(QHdr));
            memset(qhdr, 0, sizeof(QHdr));
            qhdr->cmd = SRV_RESTART;
            current=DefaultInstance(dcb,(XMLElt *) ProfileEntry(dcb,"CIM_TCRTCPProtocolEndpoint"),NewMem());
            currentx= GetElement(current,"PortNumber",0);
            sscanf(currentx->data,"%d",&bind_port);
            qhdr->port = bind_port;
            ReleaseProfile(current);

            OSTaskCreateExt (SOLTask,
                             (void *) qhdr,
                             (OS_STK *) & TaskSOLStk[TASK_SOL_STK_SIZE - 1],
                             TASK_SOL_PRIO,
                             TASK_SOL_ID,
                             (OS_STK *) & TaskSOLStk[0],
                             TASK_SOL_STK_SIZE,
                             (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
        }
    */
    dcb->control=0;
    return;

}

void wsinit()
{
    RTSkt* sNetBIOSPtl;
   
    userhead->opt=1;

    userhead->crc16 = inet_chksum( userhead, 34);
	 
    init_role();
    init_subscribe();
    setHostName(dpconf->HostName);
    sNetBIOSPtl = enableNetBIOSPtl();
    //if (dpconf->chipset == AMD || dpconf->chipset == HP || dpconf->chipset == LENOVO)
    if(dpconf->chipset == LENOVO)
        _BIOSElement[1].data = "LENOVO";
    else if (dpconf->chipset == HP)
        _BIOSElement[1].data = "HP";
    else
    {
        if(dpconf->bios == AMIBIOS)
            _BIOSElement[1].data = "AMI";
        else if(dpconf->bios == PHOENIXBIOS || dpconf->bios == LENOVOBIOS)
            _BIOSElement[1].data = "PHOENIX";
        else
            _BIOSElement[1].data = "BIOS";

    }

    DEBUGMSG(PLDM_DEBUG, "Entering ParserSensor [%s:%d]: %s\n", __FILE__,__LINE__,__func__);
    ParserSensor();
    /*
    _ComputerSystem[10].data = dpconf->HostName;
    _Processor[18].data  = dpconf->HostName;
    _Memory[12].data  = dpconf->HostName;
    _CacheMemory[12].data = dpconf->HostName;
    _BootConfigSetting[0].data = dpconf->HostName;
    _IndicationService[8].data = dpconf->HostName;
    _USBRedirectionSAP[9].data = dpconf->HostName;
    _USBRedirectionSAP[20].data = dpconf->HostName;
    _USBRedirectionSAP[31].data = dpconf->HostName;
    _TCRTCPProtocolEndpoint[6].data = dpconf->HostName;
    _USBTCPProtocolEndpoint[6].data = dpconf->HostName;
    _DHCPProtocolEndpoint[10].data = dpconf->HostName;
    _USBDevice[5].data = dpconf->HostName;
    _SoftwareInstallationService[3].data = dpconf->HostName;
    */
}

//void wsman(void *data)
void wsman(DASHCB *dcb,RTSkt *cs)
{
    INT32U address;
    int datasize,payload_off=0,org_off=0,payload_size=0;
    INT8U *str_end, *ptr,*ptr1,*body,ch,reset=0,ANON=0;
    INT16U mob=0;
    HTTPHdrInfo hinfo;
    int status = 0,init=0;
    PKT* rxPkt = 0;
    INT32U htmlLen;//+briank
    INT8U *len_start;//+briank
		
    RoleInfo *role_p=rolehead;
    datasize=0;

	//debug_printFreeMemory_F();
	
    if(dcb->rxbuf)
    {
        ReleaseBuf();
        dcb->ProfilePtr = ReleaseProfile(dcb->ProfilePtr);
        free(dcb->rxbuf);
        dcb->rxbuf=NULL;
        stopconn(dcb->cs);
        dcb->closesend=0;
    }
    if(!dcb->rxbuf){
        dcb->rxbuf = malloc(4096);
	 
    }

	//if(dcb->rxbuf){
		//debug_printFreeMemory_F();
		//printf("Allocate Buffer Fail\n");		
	//}

	
    dcb->cs=cs;
    while (dcb->cs)
    {
        if (!dcb->closesend)
        {
            while ((rxPkt = rtSktRx_F(dcb->cs,600, &status)) == 0)
            {
                //  if (status == SKTDISCONNECT || (status == OS_ERR_TIMEOUT && dashcb->rstflag == 1) || dashcb->closesend ==1 )
                if (status == SKTDISCONNECT ||status == OS_ERR_TIMEOUT)
                {
                    reset=1;
                    break;
                }

            }

//getTestCIMProperty_debug(6,9);
			

        }
        if (reset==1 || dcb->closesend==1)
        {
            if(dcb->cs->pcb && dcb->cs->pcb->httpAuthMethod == Kerb) 
               dcb->privilege =0;        	
            ReleaseBuf();
            dcb->ProfilePtr = ReleaseProfile(dcb->ProfilePtr);
            free(dcb->rxbuf);
            dcb->rxbuf=NULL;
            stopconn(dcb->cs);
            dcb->closesend=0;
            reset=0;
            break;
        }

        if (datasize==0)
        {
            memset(&hinfo,0,sizeof(HTTPHdrInfo));
            memset(dcb->rxbuf,0,4096);
        }

        memcpy(dcb->rxbuf+datasize, rxPkt->wp, rxPkt->len);
        datasize += rxPkt->len;	
        if (init==0)
        {
		
		
		
            ptr=strstr(dcb->rxbuf,"\r\n\r\n");

            if(strstr(dcb->rxbuf,"Content-Type:"))
            {
                if(!(strstr(dcb->rxbuf,"application/soap+xml") || strstr(dcb->rxbuf,"text/xml")))
                {				
                    datasize=0;
                    dcb->closesend=1;
                    continue;
                }else{
                
			//+----------------------------------
			htmlLen = 0;
			len_start = strstr(dcb->rxbuf,"Content-Length:");			
			if(len_start != NULL ){
				len_start += strlen("Content-Length:");
				sscanf(len_start,"%d",&htmlLen);
			}else{
			}
			//*----------------------------------
                }
            }

            if (ptr )
            {
                init=1;
                ch=*ptr;
                *ptr = NULLCHR;
                payload_size=datasize-strlen(dcb->rxbuf)-4;
                ptr1=strstr(dcb->rxbuf,"Content-Length:");
                if (ptr1)
                {
                    *ptr=ch;
                    getHttpHeaderInfo(dcb->rxbuf, strlen(dcb->rxbuf), dcb->cs->pcb, &hinfo);
                    free(dcb->cs->pcb->httpReqData);
                    dcb->cs->pcb->httpReqData=0;
                    ptr1=ptr1+strlen("Content-Length:");
                    ptr1=strltrim(ptr1);
                    hinfo.httpContentLen=0;
                    str_end=strstr(ptr1,"\r\n");
                    while (ptr1!=str_end && *ptr1!=0)
                    {
                        hinfo.httpContentLen=(*ptr1-'0')+(hinfo.httpContentLen)*10;
                        ptr1++;
                    }
                    if (strstr(dcb->rxbuf,"WSMANIDENTIFY: unauthenticated"))
                        hinfo.httpAuthPass=1;

                }

                if (strstr(dcb->rxbuf,"UTF-16"))
                    payload_off=strlen(dcb->rxbuf);

                *ptr=ch;
            }
        }
        else
            payload_size=payload_size+rxPkt->len;


        *(dcb->rxbuf+datasize) = NULLCHR;
        freePkt(rxPkt);
        rxPkt=0;

        if (payload_size < hinfo.httpContentLen || init==0)
            continue;
        init=0;
        
#ifdef CONFIG_SOFTWARE_KVM_ENABLED
	     if(memcmp(&dpconf->ConsoleIP, &dcb->cs->pcb->destIPAddr, IPv4_ADR_LEN) != 0)
	     {
		      memcpy(&dpconf->ConsoleIP, &dcb->cs->pcb->destIPAddr, IPv4_ADR_LEN);
	     }
#endif
        
#ifdef CONFIG_MSAD_ENABLED        
        dcb->ad_nego=0;   
        chkAuthDash(dcb->rxbuf, payload_size, dcb->cs->pcb);
        if(dcb->cs->pcb->httpAuthPass == PASS && dcb->cs->pcb->httpAuthMethod == Kerb && dcb->privilege ==0)
        {
          dcb->privilege=0xffffffff;
          dcb->ad_nego=1;
          hinfo.httpAuthPass=1;
          if(hinfo.httpContentLen==0)
          {
            auth_kerberos_ok(dcb);
            sendpacket(dcb);	
            datasize=0;
            continue;
          }              
        }  
#endif
        if (payload_off >0)
        {

            payload_size=0;
            ptr=strstr(dcb->rxbuf,"\r\n\r\n");
            mob=*(ptr+4) + (*(ptr+5) << 8);
            ptr=ptr+6;
            payload_off=0;
            org_off=0;
            while (org_off < hinfo.httpContentLen)
            {
                if(*(ptr+org_off))
                {
                    *(ptr+payload_off)=*(ptr+org_off);
                    payload_off++;
                }
                org_off++;
            }
            *(ptr+payload_off) = NULLCHR;
            payload_off=0;
        }

        payload_size=0;
        dcb->fault_id = Fault_NoFault;
        if(dcb->control !=NotReady)
        {
            address=dpconf->HostIP[cs->pcb->intf].addr;
            sprintf(local_ip,"%d.%d.%d.%d",address&0xff,(address>>8) & 0xff,(address>>16)&0xff,address>>24);
            address=dpconf->SubnetMask[cs->pcb->intf].addr;
            sprintf(local_netmask,"%d.%d.%d.%d",address&0xff,(address>>8) & 0xff,(address>>16)&0xff,address>>24);
            address=dpconf->GateWayIP[cs->pcb->intf].addr;
            sprintf(local_gateway,"%d.%d.%d.%d",address&0xff,(address>>8) & 0xff,(address>>16)&0xff,address>>24);
        }
        if(bsp_in_band_exist())
        {
            strcpy(dhcp_state,"5");
            strcpy(client_state,"0");
        }
        else if (dpconf->DHCPv4Enable==1)
        {
            strcpy(dhcp_state,"2");
            strcpy(client_state,"8");
        }
        else
        {
            strcpy(dhcp_state,"3");
            strcpy(client_state,"0");
        }
        /*
        if (strstr(dcb->rxbuf, "Connection: close")) //used for connection close option
            dcb->closesend = 1;
        else
            dcb->closesend = 0;
        */
        str_end = strstr(dcb->rxbuf,":Envelope>");
        if (!str_end)
        {
            datasize=0;
            dcb->closesend=1;
            continue;
        }
        *str_end = NULLCHR;
        body = strstr(dcb->rxbuf, ":Body");
        if (body)
            body=body+strlen(":Body");

        InitElement(dcb);

        //    ptr=strstr(dcb->rxbuf,":Identify/");
        //    if(!ptr)
        //      ptr=strstr(dcb->rxbuf,":Identify>");
        //    if (hinfo.httpAuthPass == 0 && ptr==NULL)

        ptr=strstr(dcb->rxbuf,"\r\n\r\n");
        if(ptr)
        {
            *ptr=NULLCHR;
            if (strstr(dcb->rxbuf, "Connection: close")) //used for connection close option
                dcb->closesend = 1;
            else
                dcb->closesend = 0;
            if(strstr(body,":Identify/") || strstr(body,":Identify "))
                dcb->actiontype= WS_DISP_TYPE_IDENTIFY;

            if(strstr(dcb->rxbuf,"/wsman-anon"))
              ANON=1;
            else
              ANON=0;    

            //   if(strncmp(dcb->rxbuf, "/wsman-anon", strlen("/wsman-anon")) == 0)
            //     ANON=1;
            *ptr='\r';
        }

        if (hinfo.httpAuthPass == 0 && ANON==0 &&  dcb->actiontype!= WS_DISP_TYPE_IDENTIFY)
        {
            dcb->fault_id=1;
            auth_error(dcb,dpconf->Security);
            sendpacket(dcb);
            datasize = 0;
            continue;

        }

        //after login, assign the userinfo to dash control block
        if (hinfo.uInfo)
        {
            dcb->userinfo = hinfo.uInfo;
            if(dcb->userinfo->opt!=0)
            {
              ch=dcb->userinfo->role;                      
              dcb->privilege=R_Account_pri | R_Role_pri;                       
              while (role_p)
              {
                if (ch & role_p->mask)
                    dcb->privilege|=role_p->privilege;
                role_p=role_p->next;
              }
			  
	   role_p=rolehead;  //restart again 
			  
            }
            else
              dcb->privilege=0;  
        }
        if(dcb->actiontype != WS_DISP_TYPE_IDENTIFY)
            dcb->actiontype = -1;
        dcb->association=0;

        dcb->fault_id = parseelt(dcb);
        if(ANON==1 && dcb->actiontype != WS_DISP_TYPE_IDENTIFY  && dcb->URI!=_ComputerSystem)
            dcb->fault_id=1;
        if (dcb->fault_id == 0)
        {
            dcb->fault_id = ParserBody(dcb, body);
            if (bodyelement[ID_IDENTIFY].elementPtr)
                dcb->actiontype = WS_DISP_TYPE_IDENTIFY;
        }
        if (dcb->fault_id == 0 && dcb->actiontype != WS_DISP_TYPE_IDENTIFY )
            dcb->fault_id = check_missing(dcb);


        if (dcb->fault_id == 0)
        {

            if (dcb->actiontype >= 0)
            {
                register_endpoint(dcb,body);
                if (dcb->fault_id ==0)
                {
                	//printf("[RTK] wsman : sendpacket\n");
                    sendpacket(dcb);
                    ReleaseBuf();
                    dcb->ProfilePtr = ReleaseProfile(dcb->ProfilePtr);
                    if (dcb->control>0 || tcrcb.state == DISABLED)
                        ConnectControl(dcb, cs->pcb->intf);
                    datasize = 0;
                    if (dcb->actiontype!=WS_DISP_TYPE_PULL && dcb->actiontype!=WS_DISP_TYPE_ENUMERATE)
                        dcb->ref_count[0]=0xff;

                    continue;
                }
            }
            else
                dcb->fault_id=Fault_ActionFault;
        }

        if (dcb->fault_id > 0)
        {

            response_fault(dcb);
            sendpacket(dcb);

            ReleaseBuf();
            dcb->ProfilePtr = ReleaseProfile(dcb->ProfilePtr);

            datasize = 0;
            continue;

        }

        ReleaseBuf();
        dcb->ProfilePtr = ReleaseProfile(dcb->ProfilePtr);

        datasize = 0;

    }

}
void wsmanSrv(void *data)
{
    DASHCB *dashcb;
    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_TCP),*cs;
    wsinit();
#ifndef  CONFIG_WEB_SKT_STYLE
    if (dpconf->httpService){
		printf("[RTK] set httpRX\n");
        s->pcb->apRx = httpRx;
    }
#ifdef CONFIG_MSAD_ENABLED
    if(dpconf->Security == KERB_AUTH)
        rtk_server_acquire_creds("host", &s->pcb->msad.server_creds);
#endif
#endif

#if CONFIG_PortMatch_Test
    dpconf->DASHPort = 80;
#endif

#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F(s);
#endif
    rtSktBind_F(s, dpconf->DASHPort);
    rtSktListen_F(s, 6);
    InitialRegister();
    dashcb = malloc(sizeof(DASHCB));
    memset(dashcb, 0 , sizeof(DASHCB));
    while (OS_TRUE)
    {
        cs = rtSktAccept_F(s);

	//printf("[RTK] after rtSktAccept_F  + wsman\n");
		
        wsman(dashcb,cs);
    }

}

void wsmanSrvIPv6(void *data)
{
    DASHCB *dashcb;
    RTSkt* s = rtSkt_F(IPv6, IP_PROTO_TCP),*cs;
//   wsinit();
    if (dpconf->httpService)
        s->pcb->apRx = httpRx;
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F(s);
#endif        
    rtSktBind_F(s, dpconf->DASHPort);
    rtSktListen_F(s, 6);
    InitialRegister();
    dashcb = malloc(sizeof(DASHCB));
    memset(dashcb, 0 , sizeof(DASHCB));
    while (OS_TRUE)
    {
        cs = rtSktAccept_F(s);
        wsman(dashcb,cs);
    }
}

void wsmanSrvTLS(void *data)
{
    DASHCB *dashcb;
    RTSkt* s,*cs;
//    wsinit();
    dpconf->httpsService=1;
    s= rtSkt_F(IPv4, IP_PROTO_TCP);
    s->pcb->TLSEnable = 1;
    if (dpconf->httpsService)
        s->pcb->apRx = httpRx;
#ifdef CONFIG_MSAD_ENABLED
    if(dpconf->Security == KERB_AUTH)
        rtk_server_acquire_creds("host", &s->pcb->msad.server_creds);
#endif
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F(s);
#endif
    rtSktBind_F(s, dpconf->DASHTLSPort);
    rtSktListen_F(s, 6);
    dashcb = malloc(sizeof(DASHCB));
    memset(dashcb, 0 , sizeof(DASHCB));

    while (OS_TRUE)
    {
        cs = rtSktAccept_F(s);
        wsman(dashcb,cs);
    }

}

void wsmanSrvTLSIPv6(void *data)
{
    DASHCB *dashcb;
    RTSkt* s,*cs;
//   wsinit();
    dpconf->httpsService=1;
    s= rtSkt_F(IPv6, IP_PROTO_TCP);
    s->pcb->TLSEnable = 1;
    if (dpconf->httpsService)
        s->pcb->apRx = httpRx;
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F(s);
#endif        
    rtSktBind_F(s, dpconf->DASHTLSPort);
    rtSktListen_F(s, 6);
    dashcb = malloc(sizeof(DASHCB));
    memset(dashcb, 0 , sizeof(DASHCB));
    while (OS_TRUE)
    {
        dashcb->cs = rtSktAccept_F(s);
        wsman(dashcb,cs);
    }

}
