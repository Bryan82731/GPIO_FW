#ifndef DASH_H
#define DASH_H
#include "smbus.h"
//#include <os_cpu.h>


#define HTTP_HEADER_OFFSET      56
#define NULLCHR                 '\0'
//#define PAYLOAD_SIZE_THD        1516
#define PAYLOAD_SIZE_THD        1000 

#define LastAssetClass 4
#define WS_DISP_TYPE_MASK               0xffff

#define WS_DISP_TYPE_RAW_DOC            0
#define WS_DISP_TYPE_GET                1
#define WS_DISP_TYPE_PUT                2
#define WS_DISP_TYPE_CREATE         3
#define WS_DISP_TYPE_DELETE         4

#define WS_DISP_TYPE_ENUMERATE      5
#define WS_DISP_TYPE_PULL               6
#define WS_DISP_TYPE_RELEASE            7
#define WS_DISP_TYPE_UPDATE         8
#define WS_DISP_TYPE_GETSTATUS      9
#define WS_DISP_TYPE_COUNT              11
#define WS_DISP_TYPE_DIRECT_PULL         12
#define WS_DISP_TYPE_DIRECT_GET            13
#define WS_DISP_TYPE_GET_NAMESPACE  14
#define WS_DISP_TYPE_CUSTOM_METHOD  15
#define WS_DISP_TYPE_DIRECT_PUT            16
#define WS_DISP_TYPE_IDENTIFY           17
#define WS_DISP_TYPE_DIRECT_CREATE           18
#define WS_DISP_TYPE_DIRECT_DELETE           19
#define WS_DISP_TYPE_ENUM_REFINSTS           20
#define WS_DISP_TYPE_INVOKE           21
#define WS_DISP_TYPE_SUBSCRIBE          22
#define WS_DISP_TYPE_UNSUBSCRIBE        23
#define WS_DISP_TYPE_EVT_GETSTATUS      24
#define WS_DISP_TYPE_EVT_RENEW       	25
#define WS_DISP_TYPE_PRIVATE            0xFFFE

#define Pldm_Attr_Enum 0
#define Pldm_Attr_Bootconfig 4


//static char XML_END1[3] =  {0x0d,0x0a,0x00};
//static char XML_END2[5] =  {0x0d,0x0a,0x0d,0x0a,0x00};

//static u8_t whitestr[] = {0x20,0x0a,0x0d,0x0};
typedef struct _slist {
    struct _slist *next;
    u8_t  *addr;
    u16_t len;	
}send_list;
typedef struct {
    char *eltname;
    char *data;
    char key;
    char used;
    char idx;
    char attr;
}XMLElt;

typedef struct {
    char *eltname;
    void *addr;
    INT32U priv;
    INT16S type;
}Profile;

typedef struct {
    char *eltname;
    char *addr;
    char *priv_name; 
    char *info;
    INT32U priv;   
}RegProfile;

typedef struct _plist {
    struct _plist *next;	
    char *eltname;
    char *data;
    char key;
    char used;
    char idx;
}plist;

typedef struct _DASHCB {
    UserInfo *userinfo;
    OS_EVENT *event;
    QHdr     *qhdr;  
    INT8U    *rxbuf;
    send_list *sendlist;
    RTSkt    *cs;
    plist    *ProfilePtr;
    plist    *StartOff;
    XMLElt   *URI; 
    XMLElt   *PreURI;
    INT32U   privilege;
    INT32U   profile_priv;
    INT32S   actiontype;
    INT32S   count;
    INT16S   Ptype;
    INT16S   association;
    INT8U    control;
    INT8U    status;
    INT8U    fault_id;
    INT8U    closesend;
    INT8U    epr;
    INT8U    PullOption;
    INT8S    SubClass;
    INT8S    RegClass;
    INT8U    InstanceCount; 
    INT8U    ref;
    INT8U    ad_nego;
    INT8U    ref_count[16];
    INT8U    r_uri[64];
    INT8U    q_uri[64];
    INT8U    ref_uri[64];
}DASHCB;

#if 0
typedef struct {
    char id;
    char name[10];
}BootOrder;
typedef struct {
     char *name;
     char *password;
     char role;
}uAccount;
#endif
/*
typedef struct _SubEvent {
    struct _SubEvent *next;	
    char *subid;
    char *notifyto;
    char *query;
}SubEvent;
*/
typedef struct _mbuf {
    struct _mbuf *next;
    u8_t  *mbuf;
}m_list;
		

enum {
    IDX_NS_SOAP_1_2=0,
    IDX_NS_ADDRESSING,
    IDX_NS_EVENTING,
    IDX_NS_ENUMERATION,
    IDX_NS_SCHEMA_INSTANCE,
    IDX_NS_CIM_SCHEMA,
    IDX_NS_WS_MAN_CAT,
    IDX_NS_WSMAN_ID,
    IDX_NS_XML_SCHEMA,
    IDX_NS_WS_MAN,
    IDX_NS_CIM_BINDING,
    IDX_NS_OPENWSMAN,
    IDX_NS_TRANSFER,
    IDX_NS_CIM_CLASS,
};

enum
{
    ID_IDENTIFY=0,
    ID_RESOURCE_URI,
    ID_ACTION,
    ID_REQUEST_TOTAL,
    ID_REPLY_TO,
    ID_TO,
    ID_MESSAGE_ID,
    ID_ASSOCIATED_INSTANCES,
    ID_ASSOCIATION_INSTANCES,
    ID_EPR,
    ID_SELECTOR_SET,
    ID_SELECTOR,
    ID_MAX_ENVELOPE_SIZE,
    ID_OPERATION_TIMEOUT,
    ID_FRAGMENT_TRANSFER,
    ID_IDENTIFIER,
    ID_OPTIONSET,
};


enum{
    ID_REFERENCE=1,
    ID_ENUMERATE,
    ID_RELEASE,
    ID_PULL,
    ID_INVOKE,
    ID_SUBSCRIBE,
    ID_GETSTATUS,
};

//Fault action and default message
enum {
	Fault_NoFault = 0,
	Fault_DestUnReachable,
	Fault_MsgInfoHdrRequired,
        Fault_ActionFault,
        Fault_MaxEnvelopeFault,
    	Fault_DuplicateSelectorFault,
    	Fault_InvalidNameFault,
       	Fault_MissingSelectorFault,
    	Fault_OperationTimeout,
    	Fault_InvalidTimeOut,
	Fault_ExpirationTime,
	Fault_FilterDialect,
	Fault_InvalidFilter,
	Fault_InvalidEnumContext,
	Fault_InvalidMessage,
	Fault_DuplicateMessage,
	Fault_PolyNotSupported,
	Fault_EventSourceUnableToProcess,
	Fault_EventInvalidMessage,
	Fault_InvalidNamespace,
        Fault_NotUnderstood		
};

struct _wselement
{
    char *elementName;
    char *elementPtr;
};

struct _fault
{
char *fault_str;
char *fault_detail;
char *fault_action;
};

struct __WsXmlNsData
{
    char* uri;
    char prefix[10];
    int  used;
};

typedef struct __WsXmlNsData WsXmlNsData;

//u8_t check_envelope(DASHCB *dcb,u8_t index);
void ReleaseBuf();
#endif
