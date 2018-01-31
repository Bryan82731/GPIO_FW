#ifndef __SOAP_H__
#define __SOAP_H__

#include "rtskt.h"

enum {FULL_UNPROVISIONED, PARTIAL_UNPROVISIONED, PROVISIONED};

enum {SMALL_BUSINESS, ENTERPRISE};

enum {AGENT_INITIATED, BARE_METAL};

enum {UNKNOWN, FQDN, DHCP_OPTION, PROVISIONSERVER};

enum {SOAP_INIT, SOAP_HELLO_DONE, SOAP_ACCEPTED, SOAP_SUCCESS, SOAP_FAILURE};

enum {FWUpdateStatusInvalid, FWUpdateStatusInitiating, FWUpdateStatusUpdating, FWUpdateStatusSuccess};
enum{ShowUsersStatusInvalid, ShowUsersStatusDisplaying, ShowUsersStatusComplete};
typedef struct _pkiHelloPkt
{
	INT16U acs;
	INT16U iv;
	INT32U rc;
	INT8U guid[16];
	INT8U numCerHash;
}pkiPkt;

typedef struct _pskHelloPkt
{
	INT16U acs;
	INT16U iv;
	INT32U rc;
	INT8U guid[16];
	INT8U pid[8];
}pskPkt;

typedef struct _Namespace
{
	INT8U *ns;
}Namespace;

typedef struct _SOAPCB
{
	INT8U hello_done:1, validuser:1, dummy:6;
	INT8S state;
	INT16U protimestamp;	
	INT16U rc;
	char *soapActionNS;
	char *soapAction;	
	char *sendbuf;
	char *rxbuf;
}SOAPCB;

typedef struct _CrlType
{
	struct _CrlType *next;
	INT8U* crlurl;
	INT8U ser_num[20];
}CrlType;

typedef struct _TrustedFqdnCN
{
	struct _TrustedFqdnCN *next;
	INT8U fqdnsuf[MAX_FQDN_LEN];
	//maximum of 4 elements in a list where each element is limited to a maximum of 64 characters
}TrustedFqdnCN;

void provisioningSrv(void *data);
INT8S soapSend( RTSkt * s );
void SOAPTask( void *data );
void soapInput( RTSkt * s, int len);
void soapContent( RTSkt * s );
void soapSendHello();
void provLoginAccess(int* t, unsigned char *tbuf );
void provGetFWVersion( int* t, unsigned char *tbuf );
void provUpdateFWVersion(int* t, unsigned char *tbuf );
void provGetIPMode(int* t, unsigned char *tbuf );
void provGetHWInfo(int* t, unsigned char *tbuf );
void provSetTCPIP(int* t, unsigned char *tbuf );
void provGetTCPIP(int* t, unsigned char *tbuf );
void provSetHostName(int* t, unsigned char *tbuf );
void provGetHostName(int* t, unsigned char *tbuf );
void provSetDomainName(int* t, unsigned char *tbuf );
void provGetDomainName(int* t, unsigned char *tbuf );
void provGetAdminUser(int* t, unsigned char *tbuf );
void provModUser(int* t, unsigned char *tbuf );
void provAddMachine(int* t, unsigned char *tbuf );
void provAddUser(int* t, unsigned char *tbuf );
void provRemoveUser(int* t, unsigned char *tbuf );
void provGetUserNum(int* t, unsigned char *tbuf );
void provShowUsers(int* t, unsigned char *tbuf );
void provSetEnabledFunctions(int* t, unsigned char *tbuf );
void provGetServerCertReq(int* t, unsigned char *tbuf );
void provSetServerCert(int* t, unsigned char *tbuf );
void provSetFullUnprovision();
void provSetPartialUnprovision();
void removeProvPCB(TCPPCB *pcb);
void doprovision(INT8U provstate);
#endif
