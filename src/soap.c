#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include "sys/autoconf.h"
#include "sys/dpdef.h"
#include "b64.h"
#include "app_cfg.h"
#include "soap.h"
#include "tcp.h"
#include "ip.h"
#include "tls.h"
#include "lib.h"
#include "smbios.h"
#include "asn1.h"

//#define SOAPLOOPTEST
//#define SOAPLOOPTEST_DHCP
// #define PROVISIONING_TEST
#define SOAP_RTK
//#define SOAP_DEBUG
#define HOSTNAME_BASED_CERT_NAME

ROM_EXTERN UserInfo *userhead;
ROM_EXTERN UserInfo *usertail;
extern const unsigned char *csr;
extern OS_STK TaskWSMANTLSStk[ TASK_WSMAN_TLS_STK_SIZE ];
extern DPCONF *dpconf;
extern flash_data_t dirty[ENDTBL];

OS_EVENT *ProvisioningSem;
DPCONF *dpconfbuf;
SOAPCB soapcb = { 0 };
unsigned char useradd;

#ifdef SOAP_DEBUG
unsigned char soapdbg[2048];
#endif


const static unsigned char auth_ok[] =
    "HTTP/1.1 200 OK\r\nContent-Type: text/xml; charset=UTF-8\r\nDate: Mon, 30 Jan 2006 00:00:43 GMT\r\nCache-Control: no-cache\r\nExpires: Thu, 26 Oct 1995 00:00:00 GMT\r\nTransfer-Encoding: chunked\r\nServer: Intel(R) Active Management Technology 3.0.5\r\n\r\n";

const static unsigned char head[] =
    "xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" \
    xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" \
    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \
    xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" ";

#ifdef SOAP_RTK
const static Namespace NS_R[] =
{
    {"xmlns:ns1=\"http://schemas.realtek.com.tw/Dash\""},

    {0}
};

#else
const static Namespace NS_I[] =
{
    {"xmlns:cmn=\"http://schemas.intel.com/platform/client/Common/2006/01\""},

    {"xmlns:cstr=\"http://schemas.intel.com/platform/client/CertStore/2006/01\""},
    {"xmlns:xcfg=\"http://schemas.intel.com/platform/client/XProfiles/2006/01\""},
    {"xmlns:adl=\"http://schemas.intel.com/platform/client/AuditLog/2008/01\""},
    {"xmlns:apl=\"http://schemas.intel.com/2004/12/management/PC/AgentWatchdogLocal\""},
    {"xmlns:apr=\"http://schemas.intel.com/2004/12/management/PC/AgentWatchdogRemote\""},
    {"xmlns:cb=\"http://schemas.intel.com/2004/12/management/PC/CircuitBreaker\""},
    {"xmlns:eaca=\"http://schemas.intel.com/platform/client/EACAdmin/2006/01\""},
    {"xmlns:eac=\"http://schemas.intel.com/platform/client/EAC/2006/01\""},
    {"xmlns:emi=\"http://schemas.intel.com/platform/client/EventManager/2004/01\""},
    {"xmlns:hwa=\"http://schemas.intel.com/platform/client/HardwareAsset/2004/01\""},
    {"xmlns:idr=\"http://schemas.intel.com/platform/client/Redirection/2004/01\""},
    {"xmlns:inf=\"http://schemas.intel.com/platform/client/GeneralInfo/2004/01\""},
    {"xmlns:net=\"http://schemas.intel.com/platform/client/NetworkAdministration/2004/01\""},
    {"xmlns:rai=\"http://schemas.intel.com/platform/client/RemoteAccessAdmin/2008/01\""},
    {"xmlns:rci=\"http://schemas.intel.com/platform/client/RemoteControl/2004/01\""},
    {"xmlns:sai=\"http://schemas.intel.com/platform/client/SecurityAdministration/2004/01\""},
    {"xmlns:str=\"http://schemas.intel.com/platform/client/Storage/2004/01\""},
    {"xmlns:stra=\"http://schemas.intel.com/platform/client/StorageAdministration/2004/01\""},
    {"xmlns:tim=\"http://schemas.intel.com/platform/client/NetworkTime/2004/01\""},
    {"xmlns:uac=\"http://schemas.intel.com/platform/client/UserAccessControl/2008/01\""},
    {"xmlns:wcxs=\"http://schemas.intel.com/platform/client/WirelessConfiguration/2006/01\""},
    {0}
};

#endif

#if CONFIG_VERSION < 6
int RtkRand( void* rndparams )
{
    unsigned char hashvalue[ 20 ];
    sha1( ( unsigned char* ) rndparams, 20, hashvalue );
    memcpy( rndparams, hashvalue, 20 );
    return ( int ) hashvalue[ 10 ];
}
#endif

void full_unprovision(unsigned char intf)
{
    INT8U i = 0;
    INT8U j = 0;
    dpconf->asfon = 0;
    dpconf->httpService = 0;
    dpconf->httpsService = 0;
    dpconf->EchoService = 0;
    //ht 20120112 : marked for provision process correct
    //dpconf->HostIP[intf].addr = 0;
    dpconf->SubnetMask[intf].addr = 0;
    dpconf->GateWayIP[intf].addr = 0;
    dpconf->DNSIP[intf].addr = 0;
    dpconf->DHCPv4Enable |= 1<<intf;
    strcpy((void *) dpconf->HostName, "DASHOOBHOST");
    strcpy((void *) dpconf->DomainName, "");
    dpconf->ProvisionServerIP.addr = 0;
    dpconf->ProvisionServerPort = 0;
    //ht 20120112 : marked for provision process correct
    /*
    for (i = 0; i < 8; i++)
        dpconf->pskdata.PID[i] = 0x00;
    for (i = 0; i < 32; i++)
        dpconf->pskdata.PPS[i] = 0x00;
    for (i = 0; i < 6; i++)
        for (j = 0;j < 20;j++)
            dpconf->pkidata.certhash[i][j] = 0x00;
    strcpy((void *) dpconf->pkidata.dnssuf, "");
    strcpy((void *) dpconf->pkidata.fqdn, "");
    */
    dirty[EVENTTBL].length = 0;
    dirty[USERTBL].length = 0;
}
void partial_unprovision(unsigned char intf)
{
    dpconf->asfon = 0;
    dpconf->httpService = 0;
    dpconf->httpsService = 0;
    dpconf->EchoService = 0;
    dpconf->HostIP[intf].addr = 0;
    dpconf->SubnetMask[intf].addr = 0;
    dpconf->GateWayIP[intf].addr = 0;
    dpconf->DNSIP[intf].addr = 0;
    dpconf->DHCPv4Enable = 1<<intf;
}
void doprovision(INT8U provstate)
{
    dpconf->PKIEnabled = 1;
    dpconf->PSKEnabled = 1;
    memset(&soapcb, 0, sizeof(SOAPCB));
    if (dpconf->ProvisioningMode == SMALL_BUSINESS)
    {
        dpconf->ProvisioningState = PROVISIONED;
        dpconf->asfon = 1;
        dpconf->httpService = 1;
        dpconf->httpsService = 1;
        dpconf->EchoService = 1;
    }
    else if (dpconf->ProvisioningMode == ENTERPRISE)
    {
        if (dpconf->ProvisioningState == FULL_UNPROVISIONED
                && provstate == FULL_UNPROVISIONED)
        {
            setreset(DPCONFIGTBL);
        }
        else if (dpconf->ProvisioningState == PARTIAL_UNPROVISIONED
                 && provstate == PARTIAL_UNPROVISIONED)
        {
            setreset(DPCONFIGTBL);
        }
        else if (dpconf->ProvisioningState == FULL_UNPROVISIONED )
        {
            full_unprovision();
            setdirty(DPCONFIGTBL);
            setdirty(USERTBL);
            setdirty(EVENTTBL);
            setreset(EVENTTBL);
        }
        else if (dpconf->ProvisioningState == PARTIAL_UNPROVISIONED)
        {
            partial_unprovision();
            setreset(DPCONFIGTBL);
        }
    }
}
void removeProvPCB(TCPPCB *pcb)
{
    if (pcb->ssl->peer_cert != NULL)
    {
        x509_free( pcb->ssl->peer_cert  );
        memset( pcb->ssl->peer_cert, 0, sizeof( x509_cert ) );
        free(pcb->ssl->peer_cert);
        pcb->ssl->peer_cert = NULL;
    }
    if (pcb->ssl->own_cert != NULL)
    {
        x509_free( pcb->ssl->own_cert  );
        memset( pcb->ssl->own_cert, 0, sizeof( x509_cert ) );
        free(pcb->ssl->own_cert);
        pcb->ssl->own_cert = NULL;
    }
    if (pcb->ssl->ca_crl != NULL)
    {
        x509_crl_free( pcb->ssl->ca_crl  );
        free(pcb->ssl->ca_crl);
        pcb->ssl->ca_crl = NULL;
    }
}
void getMacAddrProv( unsigned char *macbuf )
{
    int i = 0;
    unsigned char tmpchar;
    unsigned char *tmp = macbuf;

    while ( i < 6 )
    {
        tmpchar = dpconfbuf->srcMacAddr[intf][ i ] >> 4;

        if ( tmpchar <= 0x9 )
        {
            tmpchar += 0x30;
        }
        else
        {
            tmpchar += 0x37;
        }

        tmp[ i ] = tmpchar;

        tmpchar = dpconfbuf->srcMacAddr[intf][ i ] & 0x0f;

        if ( tmpchar <= 0x9 )
        {
            tmpchar += 0x30;
        }
        else
        {
            tmpchar += 0x37;
        }

        tmp[ i + 1 ] = tmpchar;

        if ( i < 5 )
            tmp[ i + 2 ] = ':';

        tmp += 2;

        i++;
    }
}


void stopSOAPTask( RTSkt * cs, INT8U end )
{
    if ( cs )
    {
        printf( "stopSOAPTASK\n" );
        unsigned char err = 0;

        OSSemDel( cs->rxSem, OS_DEL_ALWAYS, &err );

        if ( cs->txSem != 0 )
            OSSemDel( cs->txSem, OS_DEL_ALWAYS, &err );

        cs->rxSem = cs->txSem = 0;

        if ( cs )
            free( cs );
    }

#ifdef SOAPLOOPTEST_DHCP
    dpconf->ProvisionServerIP.addr = 0x00000000;

#endif
    if ( end == 0 )
        OSSemPost( ProvisioningSem );

    printf( "stop soap task\n" );

    OSTaskDel( OS_PRIO_SELF );
}

void soapSendHello()
{
    PKT * pkt;
    INT8U i = 0,j = 0,k = 0;
    INT8U pro_type = UNKNOWN;

    if ( dpconf->PKIEnabled )
    {
        INT8U found = 0;
        k = 0;
        for ( i = 0; i < 6; i++ )
        {
            for ( j = 0; j < 20; j++ )
            {
                if ( dpconf->pkidata.certhash[ i ][ j ] != 0x00 )
                {
                    k++;
                    found = 1;
                    break;
                }
            }
        }
        if ( !found )
        {
            dpconf->PKIEnabled = 0;
        }
    }

    if ( dpconf->PSKEnabled )
    {
        INT8U found = 0;
        for ( i = 0; i < PID_LEN; i++ )
        {
            if ( dpconf->pskdata.PID[ i ] != 0x00 )
            {
                found = 1;
                break;
            }
        }
        if ( !found )
        {
            dpconf->PSKEnabled = 0;
        }
    }
    if ( !dpconf->PKIEnabled && !dpconf->PSKEnabled )
    {
        return ;
    }
    while (!dpconf->HostIP[intf].addr)
        OSTimeDly(1);

    // DNS lookup
    while ( dpconf->ProvisionServerIP.addr == 0 && strcmp( dpconf->ProvisionServerName, "" ) != 0 )
    {
        INT8U * fqdn = NULL;

        if ( pro_type <= FQDN && strcmp( dpconf->pkidata.fqdn, "" ) != 0 )
        {
            fqdn = dpconf->pkidata.fqdn;
            pro_type = FQDN;
        }

        else if ( pro_type <= DHCP_OPTION && strcmp( dpconf->DomainName, "" ) != 0 )
        {
            fqdn = malloc( ( strlen( dpconf->ProvisionServerName ) + strlen( dpconf->DomainName ) + 1 + 1 ) * sizeof( unsigned char ) );
            sprintf( fqdn, "%s.%s", dpconf->ProvisionServerName, dpconf->DomainName );
            pro_type = DHCP_OPTION;
        }
        else if ( pro_type <= PROVISIONSERVER )
        {
            fqdn = malloc( ( strlen( dpconf->ProvisionServerName ) + 1 ) * sizeof( unsigned char ) );
            sprintf( fqdn, "%s", dpconf->ProvisionServerName );
            pro_type = PROVISIONSERVER;
        }

        if ( rtGethostbyname_F(IPv4, fqdn, ( unsigned char * ) & dpconf->ProvisionServerIP.addr ) == 0 )
        {
            if ( pro_type == FQDN || pro_type == DHCP_OPTION )
                pro_type++;
            else if ( pro_type == PROVISIONSERVER )
                pro_type = UNKNOWN;

            soapcb.state = SOAP_INIT;

            soapcb.hello_done = 0;
        }

        if ( !strcmp( dpconf->pkidata.fqdn, "" ) )
            free( fqdn );

        if ( dpconf->ProvisionServerIP.addr != 0 )
            break;
    }

    if ( dpconf->ProvisionServerIP.addr == 0 )
        return ;

    RTSkt *s = rtSkt_F( IPv4, IP_PROTO_TCP );

#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F( s );
#endif
    soapcb.rc++;
    if (dpconf->ProvisionServerPort == 0)
        dpconf->ProvisionServerPort = 9972;
    while ( rtSktConnect_F( s, ( INT8U * ) & dpconf->ProvisionServerIP.addr, dpconf->ProvisionServerPort ) == -1 )
    {
        soapcb.state = SOAP_INIT;
        soapcb.hello_done = 0;
        s = stopconn_F( s );
        OSSemPost( ProvisioningSem );
        return ;
    }

    if ( dpconf->PKIEnabled == 1 )
    {
        pkt = ( PKT * ) allocPkt_F( sizeof( pkiPkt ) + k * 22 );

        pkiPkt *p = ( pkiPkt * ) pkt->wp;
        memset( p, 0, sizeof( pkiPkt ) );
        memcpy( p->guid, dpconf->UUID, 16 );
        p->acs = 0x0001;
        p->iv = 0x0003;
        p->rc = soapcb.rc;
        p->numCerHash = k;

        for ( i = 0; i < p->numCerHash; i++ )
        {
            *( ( INT8U * ) p + 25 + 22 * i ) = 0x01;//sha1
            *( ( INT8U * ) p + 26 + 22 * i ) = 0x14;
            memcpy( ( INT8U * ) p + 27 + 22 * i, ( INT8U * ) dpconf->pkidata.certhash[ i ], 20 );
        }

        rtSktSend_F( s, pkt, sizeof( pkiPkt ) + p->numCerHash * 22 );
    }
    else if ( dpconf->PSKEnabled == 1 )
    {
        pkt = ( PKT * ) allocPkt_F( sizeof( pskPkt ) );
        pskPkt *p = ( pskPkt * ) pkt->wp;
        memset( p, 0, sizeof( pskPkt ) );
        memcpy( p->guid, dpconf->UUID, 16 );
        p->acs = 0x0001;
        p->iv = 0x0002;
        p->rc = soapcb.rc;
        memcpy( p->pid, dpconf->pskdata.PID, 8 );
        rtSktSend_F( s, pkt, sizeof( pskPkt ) );
    }

    s = stopconn_F( s );

    soapcb.state = SOAP_HELLO_DONE;
    soapcb.hello_done = 1;
    return ;
}

int
matchNS( unsigned char *sans )
{
#ifdef SOAP_RTK
    const static Namespace * tmp = NS_R;
#else
    const static Namespace *tmp = NS_I;
#endif
    int i = 0;

    while ( tmp[ i ].ns )
    {
        if ( !strncmp( tmp[ i ].ns + 6, sans, 3 ) || !strncmp( tmp[ i ].ns + 6, sans, 4 ) )
        {
            return i;
        }

        i++;
    }

    return -1;
}

int
minl( unsigned char *a, unsigned char *b )
{
    if ( strlen( a ) > strlen( b ) )
        return strlen( b );
    else if ( strlen( a ) < strlen( b ) )
        return strlen( a );
    else
        return strlen( a );
}

#ifdef CONFIG_DASH_ENABLED
void provisioningSrv( void *data )
{
    unsigned char err;
    OS_TCB pdata;
    RTSkt * s = NULL;
#if CONFIG_VERSION >= 6
    init_cert();
#endif
    OSTimeDly(OS_TICKS_PER_SEC*3);
    s = rtSkt_F( IPv4, IP_PROTO_TCP );
    rtSktBind_F( s, 664 );
    rtSktListen_F( s, 1 );
    s->pcb->TLSEnable = 1;
    memset( &soapcb, 0, sizeof( SOAPCB ) );
    ProvisioningSem = OSSemCreate( 1 );
    while ( 1 )
    {
        OSSemPend( ProvisioningSem, 0, &err );

        if ( soapcb.protimestamp )
        {
            if ( soapcb.protimestamp <= 5 )       // 1 min
            {
                printf( "time dly 1\n" );
                OSTimeDly( OS_TICKS_PER_SEC * 12 );       // 12s
                printf( "time dly 2\n" );
                soapcb.protimestamp++;
            }
            else if ( soapcb.protimestamp <= 10 )      // 10 min
            {
                printf( "time dly 3\n" );
                OSTimeDly( OS_TICKS_PER_SEC * 120 );      // 120s
                printf( "time dly 4\n" );
                soapcb.protimestamp++;
            }
            else if ( soapcb.protimestamp <= 15 )      // 1 hour
            {
                printf( "time dly 5\n" );
                // OSTimeDly(OS_TICKS_PER_SEC * 1200u);
                // //1200s
                OSTimeDlyHMSM( 0, 20, 0, 0 );     // 1200s
                soapcb.protimestamp++;
                printf( "time dly 6\n" );
            }
            else if (soapcb.protimestamp <= 40)
            {
                OSTimeDlyHMSM( 0, 36, 0, 0 );     // 3600s
                soapcb.protimestamp++;
            }
            else
            {
                soapcb.protimestamp = 0;
                s = stopconn_F( s );
                OSSemDel( ProvisioningSem, OS_DEL_ALWAYS, &err );
                printf( "Out of time, provisioning stop\n" );
                OSTaskDel( OS_PRIO_SELF );
            }
        }
        else
            soapcb.protimestamp = 1;

        printf( "Provisioning Sem\n" );

        while ( OSTaskQuery( TASK_SOAP_PRIO, &pdata ) == OS_ERR_NONE )
        {
            printf( "Task exist\n" );
            OSTaskDel( TASK_SOAP_PRIO );
        }

        if ( ( dpconf->ProvisioningState == FULL_UNPROVISIONED || dpconf->ProvisioningState == PARTIAL_UNPROVISIONED ) )
        {
#ifdef SOAPLOOPTEST
            int rctmp = soapcb.rc;
            memset( &soapcb, 0, sizeof( SOAPCB ) );
            soapcb.rc = rctmp;
#endif
            if ( !soapcb.hello_done )
            {
#if CONFIG_VERSION >= 6
                EnableDrvWaitOOB_F();
#endif
                soapSendHello();
#if CONFIG_VERSION >= 6
                DisableDrvWaitOOB_F();
#endif
            }

            if ( soapcb.hello_done )
            {
#if CONFIG_VERSION >= 6
                EnableDrvWaitOOB_F();
#endif
                OSTaskCreateExt ( SOAPTask,
                                  ( void * ) s,
                                  ( OS_STK * ) & TaskWSMANTLSStk[ TASK_WSMAN_TLS_STK_SIZE - 1 ],
                                  TASK_SOAP_PRIO,
                                  TASK_SOAP_ID,
                                  ( OS_STK * ) & TaskWSMANTLSStk[ 0 ],
                                  TASK_WSMAN_TLS_STK_SIZE,
                                  ( void * ) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR );

#ifndef SOAPLOOPTEST
                OSTimeDly( OS_TICKS_PER_SEC * 5 );

                if ( soapcb.state < SOAP_ACCEPTED )
                {
                    soapcb.hello_done = 0;
                    soapcb.state = SOAP_INIT;
#if CONFIG_VERSION >= 6
                    DisableDrvWaitOOB_F();
#endif
                    OSSemPost( ProvisioningSem );
                }
#endif
            }
        }
        else
        {
            s = stopconn_F( s );
#if CONFIG_VERSION >= 6
            DisableDrvWaitOOB_F();
#endif
            printf( "Provisioning Complete\n" );
            // setreset(DPCONFIGTBL);
            // cpu_reset();
            bsp_irq_disable();
            _boot();
        }
    }
    OSTimeDly( 1 );
}
#endif

void write_user()
{
    OS_CPU_SR  cpu_sr = 0;
    INT32U flashaddr;
    INT32U i = 0;
    aes_context *ctx = malloc(sizeof(aes_context));
    unsigned char *cbcptr, *ptr = NULL, *tmpptr = NULL;
    struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
    int numwrite = dirty[USERTBL].length;
    cbcptr =  malloc(64);
    ptr = cbcptr;
    tmpptr = (INT8U *) (getuserinfo()->next);
    //flashaddr = (INT32U ) (dirty[USERTBL].flashaddr - DPCONF_ADDR_CACHED);
    flashaddr = ((INT32U ) dirty[USERTBL].flashaddr) & 0x00FFFFFF;
    OS_ENTER_CRITICAL();
    spi_se_erase(ssi_map, (INT32U) flashaddr);
    OS_EXIT_CRITICAL();   
    memset(ctx, 0, sizeof(aes_context));

#if (CONFIG_VERSION  <= IC_VERSION_DP_RevF)
    aes_crypt(ctx, tmpptr, ptr, AES_ENCRYPT);
#else
    aes_crypt_sw(ctx, tmpptr, ptr, AES_ENCRYPT);
#endif

    for (i = 0;i<numwrite;i++)
    {
        OS_ENTER_CRITICAL();
        spi_write(ssi_map, flashaddr+i*64, ptr , 64);
        ptr = cbcptr;
        tmpptr = (INT8U *) ((UserInfo *) tmpptr)->next;
        memset(ctx, 0, sizeof(aes_context));
#if (CONFIG_VERSION  <= IC_VERSION_DP_RevF)
        aes_crypt(ctx, tmpptr, ptr, AES_ENCRYPT);
#else
        aes_crypt_sw(ctx, tmpptr, ptr, AES_ENCRYPT);
#endif	

        //flashaddr += wrsize;
        OS_EXIT_CRITICAL();
    }

    free(cbcptr);
    free(ctx);
}

void SOAPTask( void *data )
{
    int status = 0;
    //int i = 0;
    int datasize = 0;
    //int found = 0;
    //unsigned char *end;
    //unsigned char *p;
    //int tmps = 0;
    PKT * pkt = NULL;
    RTSkt * soapcs = NULL;
    useradd = 0;

#if CONFIG_VERSION >=  6
    INT8U * sigbuf;
#endif
    soapcs = rtSktAccept_F( ( RTSkt * ) data );
    soapcb.state = SOAP_ACCEPTED;
    dpconfbuf = ( DPCONF * ) malloc( sizeof( DPCONF ) );

    if ( !dpconfbuf )
    {
        stopSOAPTask( soapcs, 0 );
    }

    memcpy( dpconfbuf, dpconf, sizeof( DPCONF ) );
    soapcb.rxbuf = malloc( sizeof( unsigned char ) * 4096 );

    if ( !soapcb.rxbuf )
    {
        free( dpconfbuf );
        stopSOAPTask( soapcs, 0 );
    }

    soapcb.sendbuf = malloc( sizeof( unsigned char ) * 2048 );

    if ( !soapcb.sendbuf )
    {
        free( dpconfbuf );
        stopSOAPTask( soapcs, 0 );
    }

    while ( 1 )
    {
        while ( ( pkt = rtSktRx_F( soapcs, 1000, &status ) ) == 0 )
        {
            if ( status == SKTDISCONNECT || status == OS_ERR_TIMEOUT )
            {
                soapcb.validuser = 0;
                rtSktClose_F( soapcs );
                free( soapcb.sendbuf );
                free( dpconfbuf );
                stopSOAPTask( soapcs, 0 );
                //DEBUG( CLIENT_DEBUG, ( "Connect Fail" ) );
                DEBUGMSG(PROVISION_DEBUG, "Connect Fail");
            }
        }
#ifdef SOAP_DEBUG
        memset(soapdbg, 2048, 0);
        memcpy(soapdbg, pkt->wp, pkt->len);
#endif
        if ( datasize == 0 )
        {
            memset( soapcb.rxbuf, 0, 4096 );
        }

        if ( pkt->len > 0 )
        {
            memcpy( soapcb.rxbuf + datasize, pkt->wp, pkt->len );
            datasize += pkt->len ;
            *( soapcb.rxbuf + datasize ) = '\0';
        }
        freePkt_F( pkt );
        if (strstr(soapcb.rxbuf , "</SOAP-ENV:Envelope>") == 0)
            continue;

        soapInput( soapcs, datasize);

        datasize = 0;

        if ( soapcb.state == SOAP_SUCCESS )
        {
#ifndef SOAPLOOPTEST
            rtSktClose_F( soapcs );
            free( soapcb.sendbuf );
            memcpy( ((INT8U*)dpconf) + sizeof(UserInfo), ((INT8U*)dpconfbuf) + sizeof(UserInfo),
                    sizeof( DPCONF ) - sizeof(UserInfo));
            dpconf->PKIEnabled = 0;
            dpconf->PSKEnabled = 0;
            dpconf->ProvisioningState = PROVISIONED;
            // /write_back(DPCONFIGTBL);
            free( dpconfbuf );
            free( soapcb.rxbuf );
#if CONFIG_VERSION >= 6
            sigbuf = malloc( 4096 );
            writesig( sigbuf, NORMAL_SIG, 0x00 );
#endif
            setdirty( DPCONFIGTBL );
            if (useradd)
            {
                write_user(USERTBL);
                dpconf->Security = KERB_AUTH;
            }
            setreset( DPCONFIGTBL );
            OSTimeDlyResume( TASK_FLASH_PRIO );
            stopSOAPTask( soapcs, 1 );
#else
#if 0
            rtSktClose_F( soapcs );
            free( soapcb.sendbuf );
            free( dpconfbuf );
            free( soapcb.rxbuf );
            printf( "Provisioning Successful\n" );
            stopSOAPTask( soapcs, 0 );
#endif
            rtSktClose_F( soapcs );
            free( soapcb.sendbuf );
            memcpy( dpconf, dpconfbuf, sizeof( DPCONF ) );
            dpconf->PKIEnabled = 1;
            dpconf->PSKEnabled = 1;
            dpconf->ProvisioningState = PARTIAL_UNPROVISIONED;

            free( dpconfbuf );
            free( soapcb.rxbuf );
#if CONFIG_VERSION >= 6
            sigbuf = malloc( 4096 );
            writesig( sigbuf, NORMAL_SIG, 0x00 );
#endif
            setdirty( DPCONFIGTBL );
            setreset( DPCONFIGTBL );
            OSTimeDlyResume( TASK_FLASH_PRIO );
            stopSOAPTask( soapcs, 1 );
#endif

        }
        else if ( soapcb.state == SOAP_FAILURE )
        {
            if ( dpconf->PKIEnabled == 1 )
                dpconf->PKIEnabled = 0;    // psk

            // turn
            rtSktClose_F( soapcs );

            free( soapcb.sendbuf );

            free( dpconfbuf );

            free( soapcb.rxbuf );

            printf( "Provisioning Fail\n" );

            stopSOAPTask( soapcs, 0 );

            return ;
        }
    }
}

INT8S soapSend( RTSkt * s )
{
    PKT * tlsPkt;
    INT16U tmp = 0;
    INT8U * p_sendbuf;
    printf( "soapSend\n" );
    soapContent( s );

    if ( soapcb.state == SOAP_FAILURE )
    {
        sendTLSAlert_F( s->pcb );
        return -1;
    }

    p_sendbuf = soapcb.sendbuf;
    tmp = strlen( p_sendbuf );

    while ( tmp >= ETH_PAYLOAD_LEN )
    {
        tlsPkt = allocTLSPkt( s->pcb, ETH_PAYLOAD_LEN );
        memcpy( tlsPkt->wp, p_sendbuf, ETH_PAYLOAD_LEN );
        tmp -= ETH_PAYLOAD_LEN;
        p_sendbuf += ETH_PAYLOAD_LEN;
        tlsEncodeData_F( tlsPkt, ETH_PAYLOAD_LEN, s->pcb,RECORD_APP_PROTOCOL_DATA );
    }

    tlsPkt = allocTLSPkt( s->pcb, tmp );
    memcpy( tlsPkt->wp, p_sendbuf, tmp );
    tlsEncodeData_F( tlsPkt, tmp, s->pcb,RECORD_APP_PROTOCOL_DATA );
    return 1;
}

void soapInput( RTSkt * s, int len)
{
    unsigned char * soapBodyLoc;
    unsigned char *soapActionBeginLoc;
    unsigned char *soapActionEndLoc;
    unsigned char *soapActionNSLoc;
    unsigned char *soapBodyEndLoc, *p = soapcb.rxbuf;
    int i = 0, found = 0;
    printf( "soapInput 1\n" );

    for ( ; i < 1024;i++ )
    {
        if ( p[ i ] == 0x48 && p[ i + 1 ] == 0x54&& p[ i + 2 ] == 0x54 && p[ i + 3 ] == 0x50 )
        {
            p = p + i;
            found = 1;
            break;
        }
    }

    if ( !found )
    {
        soapcb.state = SOAP_FAILURE;

        return ;
    }

    if ( ( soapBodyEndLoc = strstr( p, "</SOAP-ENV:Body>" ) ) )
    {
        *( soapBodyEndLoc ) = '\0';

        if ( ( soapBodyLoc = strstr( p, "SOAP-ENV:Body>" ) ) )
        {
            if ( strstr( soapBodyLoc + 14 + 1, "SOAP-ENV:Body>" ) ) soapBodyLoc = strstr( soapBodyLoc + 14 + 1, "SOAP-ENV:Body>" );

            soapActionBeginLoc = soapBodyLoc + 14 + 1;

            if ( ( soapActionNSLoc = strstr( soapActionBeginLoc, ":" ) ) )
            {
                soapcb.soapActionNS = malloc( sizeof( unsigned char ) * ( soapActionNSLoc - soapActionBeginLoc + 1 ) );
                memcpy( soapcb.soapActionNS, soapActionBeginLoc, soapActionNSLoc - soapActionBeginLoc );
                *( soapcb.soapActionNS + ( soapActionNSLoc - soapActionBeginLoc ) ) = '\0';

                if ( ( soapActionEndLoc = strstr( soapActionNSLoc + 1, ">" ) ) )
                {
                    soapcb.soapAction = malloc( sizeof( unsigned char ) * ( ( soapActionEndLoc - soapActionNSLoc - 1 ) + 1 ) );
                    memcpy( soapcb.soapAction, soapActionNSLoc + 1, soapActionEndLoc - soapActionNSLoc - 1 );
                    *( soapcb.soapAction + ( soapActionEndLoc - soapActionNSLoc - 1 ) ) = '\0';

                    if ( -1 == soapSend( s ) )
                    {
                        soapcb.state = SOAP_FAILURE;

                    }

                    free( soapcb.soapAction );
                }
                else
                {
                    soapcb.state = SOAP_FAILURE;

                }

                free( soapcb.soapActionNS );
            }
            else
            {
                soapcb.state = SOAP_FAILURE;

            }
        }
        else
        {
            soapcb.state = SOAP_FAILURE;

        }
    }
    else
        soapcb.state = SOAP_FAILURE;

    return ;
}

void soapContent( RTSkt * s )
{
    int t = 0;
    unsigned char *tbuf = NULL/*, *tmp = NULL*/;
    /*    unsigned char tmpchar;*/

    /*    struct _x509_name *p_x509name;*/
    unsigned char /**macbuf, *ipbuf, *maskbuf, *dnsbuf, */*sendbuf = soapcb.sendbuf,
            /* *gatewaybuf, *domainname, *hostname, */*soapAction = soapcb.soapAction,
                    /* *tmpbuf, *fqdnbuf, *username, *passwd, *p = soapcb.rxbuf,*/
                    *soapActionNS = soapcb.soapActionNS;

    int ind = matchNS( soapcb.soapActionNS );

    if ( ind >= 0 )
    {
        printf( "soapAction:%s\n", soapcb.soapAction );

        tbuf = malloc( sizeof( unsigned char ) * 4 );
#ifdef SOAP_RTK
        const static Namespace *NS = NS_R;
        t = strlen( "<SOAP-ENV:Envelope " ) ;
        t+=strlen( head ) + strlen( NS[ ind ].ns ) ;
        t+=strlen( "><SOAP-ENV:Body><" ) + strlen( soapcb.soapActionNS ) ;
        t+=strlen( ":" ) + strlen( soapcb.soapAction ) + strlen( "Response><" ) ;
        t+=strlen( soapcb.soapActionNS ) + strlen( ":ErrCode>0</" ) ;
        t+=strlen( soapcb.soapActionNS ) + strlen( ":ErrCode></" ) ;
        t+=strlen( soapcb.soapActionNS ) + strlen( ":" ) + strlen( soapcb.soapAction ) ;
        t+=strlen( "Response></SOAP-ENV:Body></SOAP-ENV:Envelope>" );

        sprintf( sendbuf, "%s%x\r\n%s%s%s%s%s:%s%s", auth_ok, t, "<SOAP-ENV:Envelope ", head, NS[ ind ].ns,"><SOAP-ENV:Body><", soapcb.soapActionNS, soapcb.soapAction,"Response>" );
#if 0
        sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s",
                 "<", soapActionNS, ":ErrCode>1</", soapActionNS, ":ErrCode>" );
#endif
        /***************************RTK SOAP API*************************************/

        if ( !strncmp ( "loginAccess", soapAction, minl( "loginAccess", soapAction ) ) )
        {
            provLoginAccess( &t, tbuf );

            if ( soapcb.validuser == 0 )
            {
                soapcb.state = SOAP_FAILURE;

            }
            goto wrapup;
        }
        if ( soapcb.validuser == 0 )
        {
            soapcb.state = SOAP_FAILURE;

            goto wrapup;
        }
        if ( !strncmp ( "getFWVersion", soapAction, minl( "getFWVersion", soapAction ) ) )
        {
            provGetFWVersion( &t, tbuf );
        }
        else if ( !strncmp( "updateFWVersion", soapAction, minl( "updateFWVersion", soapAction ) ) )
        {
            provUpdateFWVersion( &t, tbuf);
        }
        else if ( !strncmp ( "getIPMode", soapAction, minl( "getIPMode", soapAction ) ) )
        {
            provGetIPMode(&t, tbuf);
        }
        else if ( !strncmp( "getHWInfo", soapAction, minl( "getHWInfo", soapAction ) ) )
        {
            provGetHWInfo(&t, tbuf);
        }
        else if ( !strncmp ( "setTCPIP", soapAction, minl( "setTCPIP", soapAction ) ) )
        {
            provSetTCPIP(&t, tbuf);
        }
        else if ( !strncmp( "getTCPIP", soapAction, minl( "getTCPIP", soapAction ) ) )
        {
            provGetTCPIP(&t, tbuf);
        }
        else if ( !strncmp( "setHostName", soapAction, minl( "setHostName", soapAction ) ) )
        {
            provSetHostName(&t, tbuf);
        }
        else if ( !strncmp ( "getHostName", soapAction, minl( "getHostName", soapAction ) ) )
        {
            provGetHostName(&t, tbuf);
        }
        else if ( !strncmp ( "setDomainName", soapAction, minl( "setDomainName", soapAction ) ) )
        {
            provSetDomainName(&t, tbuf);
        }
        else if ( !strncmp( "getDomainName", soapAction, minl( "getDomainName", soapAction ) ) )
        {
            provGetDomainName(&t, tbuf);
        }
        else if ( !strncmp( "getAdminUser", soapAction, minl( "getAdminUser", soapAction ) ) )
        {
            provGetAdminUser(&t, tbuf);
        }
        else if ( !strncmp( "modUser", soapAction, minl( "modUser", soapAction ) )
                  || !strncmp( "modAdminUser", soapAction, minl( "modAdminUser", soapAction ) ) )
        {
            provModUser(&t, tbuf);
        }
        else if ( !strncmp( "setKerberosOptions", soapAction, minl( "setKerberosOptions", soapAction ) ) )
        {
            provAddMachine(&t, tbuf);
        }
        else if ( !strncmp( "addUser", soapAction, minl( "addUser", soapAction ) ) )
        {
            provAddUser(&t, tbuf);
        }
        else if ( !strncmp ( "removeUser", soapAction, minl( "removeUser", soapAction ) ) )
        {
            provRemoveUser(&t, tbuf);
        }
        else if ( !strncmp ( "getUserNum", soapAction, minl( "getUserNum", soapAction ) ) )
        {
            provGetUserNum(&t, tbuf);
        }
#if 0
        else if ( !strncmp ( "showUsers", soapAction, minl( "showUsers", soapAction ) ) )
        {
            provShowUsers(&t, tbuf);
        }
#endif
        else if ( !strncmp( "setEnabledFunctions", soapAction, minl( "setEnabledFunctions", soapAction ) ) )
        {
            provSetEnabledFunctions(&t, tbuf);
        }
        else if ( !strncmp ( "getServerCertReq", soapAction, minl( "getServerCertReq", soapAction ) ) )
        {
            provGetServerCertReq(&t, tbuf);
        }
        else if ( !strncmp( "setServerCert", soapAction, minl( "setServerCert", soapAction ) ) )
        {
            provSetServerCert(&t, tbuf);
        }
#if 0
        else if ( !strncmp( "getServerCert", soapAction, minl( "getServerCert", soapAction ) ) )
        {
            soapcb.state = SOAP_FAILURE;

        }

        else if ( !strncmp ( "RemoveServerCert", soapAction, minl( "RemoveServerCert", soapAction ) ) )
        {
            soapcb.state = SOAP_FAILURE;

        }
#endif
        else if ( !strncmp ( "provComplete", soapAction, minl( "provComplete", soapAction ) ) )
        {
            soapcb.state = SOAP_SUCCESS;
        }
        else if ( !strncmp( "fullUnprovision", soapAction, minl("fullUnprovision", soapAction)))
        {
            provSetFullUnprovision();
        }
        else if ( !strncmp( "partialUnprovision", soapAction, minl("partialUnprovision", soapAction)))
        {
            provSetPartialUnprovision();
        }
        else
        {
            soapcb.state = SOAP_FAILURE;

        }

        /***************************INTEL SOAP API*************************************/
#endif
wrapup:
        free( tbuf );

        if ( soapcb.state == SOAP_FAILURE )
        {
            sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s",
                     "<", soapActionNS, ":ErrCode>1</", soapActionNS, ":ErrCode>" );
        }
        else
        {
            sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s",
                     "<", soapActionNS, ":ErrCode>0</", soapActionNS, ":ErrCode>" );
        }

        sprintf( sendbuf + strlen( sendbuf ),
                 "%s%s:%s%s", "</", soapActionNS,
                 soapAction,
                 "Response></SOAP-ENV:Body></SOAP-ENV:Envelope>\r\n0\r\n\r\n" );
    }

    printf( "send length:%d\n", strlen( sendbuf ) );
    return ;
}

void provLoginAccess( int* t, unsigned char *tbuf)
{
    unsigned char * newUser, *username, *passwd, *tmpbuf;
    unsigned char *p = soapcb.rxbuf;

    if ( ( newUser = strstr( p, "<ns1:User>" ) ) )
    {
        if ( ( username = strstr( newUser, "<ns1:Username>" ) ) )
        {
            if ( ( tmpbuf = strstr( username + strlen( "<ns1:Username>" ), "</ns1:Username>" ) ) )
            {
                *tmpbuf = '\0';
                username += strlen( "<ns1:Username>" );

                if ( strcmp( dpconfbuf->admin.name, username ) )
                {
                    soapcb.state = SOAP_FAILURE;

                    return ;
                }

                *tmpbuf = '<';
            }

        }
    }

    if ( ( passwd = strstr( newUser, "<ns1:Password>" ) ) )
    {
        if ( ( tmpbuf = strstr( passwd + strlen( "<ns1:Password>" ), "</ns1:Password>" ) ) )
        {
            *tmpbuf = '\0';
            passwd += strlen( "<ns1:Password>" );
            unsigned char *tmpPW = malloc( strlen( passwd ) );
            memset( tmpPW, 0, strlen( passwd ) );
            b64_decode( passwd, tmpPW, strlen( passwd ) );

            if ( strcmp( dpconfbuf->admin.passwd, tmpPW ) )
            {
                soapcb.state = SOAP_FAILURE;

                return;
            }

            soapcb.validuser = 1;
            free( tmpPW );

            *tmpbuf = '<';
        }
    }

}

void provGetFWVersion( int* t, unsigned char *tbuf )
{
    unsigned char *soapActionNS = soapcb.soapActionNS;
    unsigned char *sendbuf = soapcb.sendbuf;

    *t += +strlen( "<" ) + strlen( soapActionNS )
          +strlen( ":FWVersion><" )
          +strlen( soapActionNS )
          +strlen( ":MajorVersion>" )
          +DigitLength( dpconf->fwMajorVer ) + strlen( "</" )
          +strlen( soapActionNS ) + strlen( ":MajorVersion><" )
          +strlen( soapActionNS ) + strlen( ":MinorVersion>" )
          +DigitLength( dpconf->fwMinorVer ) + strlen( "</" )
          +strlen( soapActionNS ) + strlen( ":MinorVersion><" )
          +strlen( soapActionNS ) + strlen( ":ExtraVersion>" )
          +DigitLength( dpconf->fwExtraVer ) + strlen( "</" )
          +strlen( soapActionNS ) + strlen( ":ExtraVersion></" )
          +strlen( soapActionNS ) + strlen( ":FWVersion>" );

    sprintf( tbuf, "%x", *t );
    memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
    sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s%d","<", soapActionNS, ":FWVersion><",
             soapActionNS, ":MajorVersion>", dpconf->fwMajorVer );
    sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s%d%s%s%s%s%s%d","</", soapActionNS, ":MajorVersion><",
             soapActionNS,":MinorVersion>", dpconf->fwMinorVer, "</", soapActionNS, ":MinorVersion><",
             soapActionNS,":ExtraVersion>", dpconf->fwExtraVer );
    sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s", "</",   soapActionNS, ":ExtraVersion></",
             soapActionNS,":FWVersion>" );

#ifdef PROVISIONING_TEST
    soapcb.state = 1;
#endif

}

void provUpdateFWVersion( int* t, unsigned char *tbuf )
{
    unsigned char * fwUpdateState, *tmpbuf, *fwupdatelen, *fwbuf;
    unsigned char tmpchar;
    unsigned int tmpint;
    unsigned char *p = soapcb.rxbuf;

    if ( ( fwUpdateState = strstr( p, "FWUpdateState>" ) ) )
    {
        if ( ( tmpbuf = strstr( fwUpdateState + strlen( "FWUpdateState" ), "</ns1:FWUpdateState>" ) ) )
        {
            *tmpbuf = '\0';
            tmpchar = atoi( fwUpdateState + strlen( "FWUpdateState>" ) );
            *tmpbuf = '<';

            if ( ( fwupdatelen = strstr( p, "FWUpdateLength>" ) ) )
            {
                if ( ( tmpbuf = strstr( fwupdatelen + strlen( "FWUpdateLength" ),
                                        "</ns1:FWUpdateLength>" ) ) )
                {
                    *tmpbuf = '\0';
                    tmpint = atoi( fwupdatelen + strlen( "FWUpdateLength>" ) );
                    *tmpbuf = '<';

                    if ( ( fwbuf = strstr( p, "FWUpdateBuffer>" ) ) )
                    {
                        if ( ( tmpbuf = strstr( fwUpdateState + strlen( "FWUpdateBuffer" ), "</ns1:FWUpdateBuffer>" ) ) )
                        {
                            *tmpbuf = '\0';

                            if ( tmpchar == FWUpdateStatusInitiating )
                            {
#if 0
                                fp = fopen( "68DPSPI.bin", "wb" );

                                if ( !fp )
                                    printf( "open file error\n" );

                                fwrite( fwbuf + strlen( "FWUpdateBuffer>" ), sizeof( unsigned char ), tmpint * 2, fp );

                                fclose( fp );

#endif

                            }
                            else if ( tmpchar == FWUpdateStatusUpdating )
                            {
#if 0
                                fp = fopen( "68DPSPI.bin", "ab+" );
                                fwrite( fwbuf + strlen( "FWUpdateBuffer>" ), sizeof( unsigned char ), tmpint * 2, fp );
                                fclose( fp );
#endif

                            }
                            else if ( tmpchar == FWUpdateStatusSuccess )
                            {
#if 0
                                fp = fopen( "68DPSPI.bin", "ab+" );
                                fwrite( fwbuf + strlen( "FWUpdateBuffer>" ), sizeof( unsigned char ), tmpint * 2, fp );
                                fclose( fp );
#endif

                            }

                            *tmpbuf = '<';
                        }
                    }
                }
            }
        }
    }

}

void provGetIPMode( int* t, unsigned char *tbuf )
{
    unsigned char *soapActionNS = soapcb.soapActionNS;
    unsigned char *sendbuf = soapcb.sendbuf;

    *t += strlen( "<" ) + strlen( soapActionNS ) + strlen( ":IPMode>" );
    + strlen( dpconfbuf->ipmode == 0 ? "DualIP" : "SingleIP" );
    + strlen( "</" ) + strlen( soapActionNS ) + strlen( ":IPMode>" );

    sprintf( tbuf, "%x", *t );
    memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
    sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s%s%s", "<", soapActionNS, ":IPMode>",dpconfbuf->ipmode == 0 ? "DualIP" : "SingleIP", "</",soapActionNS, ":IPMode>" );
}

void provGetHWInfo( int* t, unsigned char *tbuf )
{
    unsigned char *soapActionNS = soapcb.soapActionNS;
    unsigned char *sendbuf = soapcb.sendbuf;

    unsigned char *macbuf;
    macbuf = malloc( 32 * sizeof( unsigned char ) );
    memset( macbuf, 0, 32 );
    getMacAddrProv( macbuf );
    printf( "%s\n", macbuf );

    *t += strlen( "<" ) + strlen( soapActionNS ) + strlen( ":MAC>" )
          +  strlen( macbuf ) + strlen( "</" ) + strlen( soapActionNS )
          +  strlen( ":MAC>" );

    sprintf( tbuf, "%x", *t );
    memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
    sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s%s%s", "<",soapActionNS, ":MAC>", macbuf, "</", soapActionNS, ":MAC>" );

    free( macbuf );
}

void provSetTCPIP( int* t, unsigned char *tbuf)
{
    unsigned char * dhcpbuf, *tmpbuf, *maskbuf, *ipbuf, *dnsbuf, *gatewaybuf;
    unsigned char *p = soapcb.rxbuf;

    if ( ( dhcpbuf = strstr( p, "DhcpEnabled>" ) ) )
    {
        if ( ( tmpbuf = strstr( dhcpbuf + strlen( "DhcpEnabled>" ), "</ns1:DhcpEnabled>" ) ) )
        {
            *tmpbuf = '\0';
            dpconfbuf->DHCPv4Enable = atoi( dhcpbuf + strlen( "DhcpEnabled>" ) ) == 2 ? 1 : 0;
            *tmpbuf = '<';

            if ( ( ipbuf = strstr( p, "IpAddress>" ) ) )
            {
                if ( ( tmpbuf = strstr( ipbuf + strlen( "IpAddress>" ), "</ns1:IpAddress>" ) ) )
                {
                    *tmpbuf = '\0';
                    dpconfbuf->HostIP[intf].addr = ntohl( strtoul( ipbuf + strlen( "IpAddress>" ), NULL, 0 ) );
                    *tmpbuf = '<';

                    if ( ( maskbuf = strstr( p, "SubnetMask>" ) ) )
                    {
                        if ( ( tmpbuf = strstr( maskbuf + strlen( "SubnetMask>" ), "</ns1:SubnetMask>" ) ) )
                        {
                            *tmpbuf = '\0';
                            dpconfbuf->SubnetMask.addr = ntohl( strtoul ( maskbuf + strlen( "SubnetMask>" ), NULL, 0 ) );
                            *tmpbuf = '<';

                            if ( ( gatewaybuf =
                                        strstr( p, "GatewayAddress>" ) ) )
                            {
                                if ( ( tmpbuf = strstr( gatewaybuf + strlen( "GatewayAddress>" ), "</ns1:GatewayAddress>" ) ) )
                                {
                                    *tmpbuf = '\0';
                                    dpconfbuf->GateWayIP.addr = ntohl( strtoul( gatewaybuf + strlen( "GatewayAddress>" ), NULL, 0 ) );
                                    *tmpbuf = '<';

                                    if ( ( dnsbuf = strstr( p, "DnsAddress>" ) ) )
                                    {
                                        if ( ( tmpbuf = strstr( dnsbuf + strlen ( "DnsAddress>" ), "</ns1:DnsAddress>" ) ) )
                                        {
                                            *tmpbuf = '\0';
                                            dpconfbuf->DNSIP.addr = ntohl( strtoul( dnsbuf + strlen( "DnsAddress>" ), NULL, 0 ) );
                                            *tmpbuf = '<';
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void provGetTCPIP( int* t, unsigned char *tbuf, unsigned char intf)
{
    unsigned char *maskbuf, *ipbuf, *dnsbuf, *gatewaybuf;
    unsigned char *sendbuf = soapcb.sendbuf;

    ipbuf = ( unsigned char * ) malloc( 16 * sizeof( unsigned char ) );
    sprintf( ipbuf, "%lu", ( unsigned long ) htonl( dpconfbuf->HostIP[intf].addr ) );

    maskbuf = ( unsigned char * ) malloc( 16 * sizeof( unsigned char ) );
    sprintf( maskbuf, "%lu", ( unsigned long ) htonl( dpconfbuf->SubnetMask.addr ) );

    dnsbuf = ( unsigned char * ) malloc( 16 * sizeof( unsigned char ) );
    sprintf( dnsbuf, "%lu", ( unsigned long ) htonl( dpconfbuf->DNSIP[intf].addr ) );

    gatewaybuf = ( unsigned char * ) malloc( 16 * sizeof( unsigned char ) );
    sprintf( gatewaybuf, "%lu", ( unsigned long ) htonl( dpconfbuf->GateWayIP.addr ) );

    *t += strlen( "<ns1:IPv4Config><ns1:DhcpEnabled>" )
          + 1 /* 0:invalid, 1:disable, 2:enable */
          + strlen( "</ns1:DhcpEnabled><ns1:IpAddress>" )
          + strlen( ipbuf )
          + strlen( "</ns1:IpAddress><ns1:SubnetMask>" )
          + strlen( maskbuf )
          + strlen( "</ns1:SubnetMask><ns1:GatewayAddress>" )
          + strlen( dnsbuf )
          + strlen( "</ns1:GatewayAddress><ns1:DnsAddress>" )
          + strlen( gatewaybuf )
          + strlen( "</ns1:DnsAddress></ns1:IPv4Config>" );

    sprintf( tbuf, "%x", *t );

    memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );

    sprintf( sendbuf + strlen( sendbuf ), "%s%d%s%s%s%s%s%s%s%s%s","<ns1:IPv4Config><ns1:DhcpEnabled>",dpconfbuf->DHCPv4Enable,"</ns1:DhcpEnabled><ns1:IpAddress>", ipbuf,"</ns1:IpAddress><ns1:SubnetMask>", maskbuf,"</ns1:SubnetMask><ns1:GatewayAddress>", dnsbuf,"</ns1:GatewayAddress><ns1:DnsAddress>", gatewaybuf,"</ns1:DnsAddress></ns1:IPv4Config>" );

    free( ipbuf );
    free( maskbuf );
    free( dnsbuf );
    free( gatewaybuf );

}

void provSetHostName( int* t, unsigned char *tbuf )
{
    unsigned char *hostname, *tmpbuf;
    unsigned char *p = soapcb.rxbuf;

    if ( ( hostname = strstr( p, "<ns1:HostName>" ) ) )
    {
        if ( ( tmpbuf = strstr( hostname + strlen( "<ns1:HostName>" ), "</ns1:HostName>" ) ) )
        {
            *tmpbuf = '\0';

            if ( strlen( hostname ) - strlen( "<ns1:HostName>" ) <= MAX_HN_LEN )
                sprintf( dpconfbuf->HostName, "%s", hostname + strlen( "<ns1:HostName>" ) );
            else
            {
                soapcb.state = SOAP_FAILURE;

                return ;
            }

            *tmpbuf = '<';
        }
    }

}

void provGetHostName( int* t, unsigned char *tbuf )
{
    unsigned char *soapActionNS = soapcb.soapActionNS;
    unsigned char  *sendbuf = soapcb.sendbuf;

    *t += strlen( "<" ) + strlen( soapActionNS )
          + strlen( ":HostName>" ) + strlen( dpconfbuf->HostName )
          + strlen( "</" ) + strlen( soapActionNS ) + strlen( ":HostName>" );
    sprintf( tbuf, "%x", *t );
    memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
    sprintf( sendbuf + strlen( sendbuf ),"<%s:HostName>%s</%s:HostName>", soapActionNS, dpconfbuf->HostName, soapActionNS );
}

void provSetDomainName( int* t, unsigned char *tbuf )
{
    unsigned char *domainname, *tmpbuf;
    unsigned char *p = soapcb.rxbuf;
    if ( ( domainname = strstr( p, "<ns1:DomainName>" ) ) )
    {
        if ( ( tmpbuf = strstr( domainname + strlen( "<ns1:DomainName>" ), "</ns1:DomainName>" ) ) )
        {
            *tmpbuf = '\0';

            if ( strlen( domainname ) - strlen( "<ns1:DomainName>" ) <= MAX_DN_LEN )
                sprintf( dpconfbuf->DomainName, "%s", domainname + strlen( "<ns1:DomainName>" ) );
            else
            {
                soapcb.state = SOAP_FAILURE;

                return;
            }

            *tmpbuf = '<';
        }
    }

}

void provGetDomainName( int* t, unsigned char *tbuf )
{
    unsigned char *soapActionNS = soapcb.soapActionNS;
    unsigned char  *sendbuf = soapcb.sendbuf;

    *t += strlen( "<" ) + strlen( soapActionNS )
          + strlen( ":DomainName>" ) + strlen( dpconfbuf->DomainName )
          + strlen( "</" ) + strlen( soapActionNS )
          + strlen( ":DomainName>" );
    sprintf( tbuf, "%x", *t );
    memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
    sprintf( sendbuf + strlen( sendbuf ), "<%s:DomainName>%s</%s:DomainName>", soapActionNS,dpconfbuf->DomainName, soapActionNS );
}

void provGetAdminUser( int* t, unsigned char *tbuf )
{
    unsigned char *soapActionNS = soapcb.soapActionNS;
    unsigned char  *sendbuf = soapcb.sendbuf;

    unsigned char *tmp = malloc(sizeof(dpconfbuf->admin.passwd) * 2);
    int size = sizeof(dpconfbuf->admin.passwd) * 2;
    b64_encode( tmp, &size, dpconfbuf->admin.passwd, sizeof( dpconfbuf->admin.passwd ) );

    *t += strlen( "<" ) + strlen( soapActionNS ) + strlen( ":User>" ) +strlen( dpconfbuf->HostName ) + strlen( "</" ) +
          strlen( soapActionNS ) + strlen( ":User>" );
    sprintf( tbuf, "%x", *t );
    memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
    sprintf( sendbuf + strlen( sendbuf ),"<%s:User><%s:Username>%s</%s:Username><%s:Password>%s</%s:Password></%s:User>",
             soapActionNS, soapActionNS, dpconfbuf->admin.name,soapActionNS, soapActionNS, tmp, soapActionNS,soapActionNS );
    free( tmp );
}

void provModUser( int* t, unsigned char *tbuf )
{
    unsigned char *newUser, *tmpbuf, *username;
    char *passwd;
    unsigned char  *soapAction = soapcb.soapAction;
    unsigned char *p = soapcb.rxbuf;

    if ( ( newUser = strstr( p, "<ns1:User>" ) ) )
    {
        unsigned char *tmpUN = NULL;
        unsigned char *tmpPW = NULL;
        if ( ( username = strstr( newUser, "<ns1:Username>" ) ) )
        {
            if ( ( tmpbuf = strstr( username + strlen( "<ns1:Username>" ), "</ns1:Username>" ) ) )
            {
                *tmpbuf = '\0';
                username += strlen( "<ns1:Username>" );
                tmpUN = malloc( strlen( username ) + 1 );
                sprintf( tmpUN, "%s", username );
                // username
                *tmpbuf = '<';
            }
        }

        if ( ( passwd = strstr( newUser, "<ns1:Password>" ) ) )
        {
            if ( ( tmpbuf = strstr( passwd + strlen( "<ns1:Password>" ), "</ns1:Password>" ) ) )
            {
                *tmpbuf = '\0';
                passwd += strlen( "<ns1:Password>" );
                tmpPW = malloc( strlen( passwd ) );
                memset( tmpPW, 0, strlen( passwd ) );
                b64_decode( passwd, tmpPW, strlen( passwd ) );

                if ( !strncmp ( "modAdminUser", soapAction, minl( "modAdminUser", soapAction ) ) )
                {
                    memset( dpconfbuf->admin.name, 0, sizeof( dpconfbuf->admin.name ) );
                    strcpy( dpconfbuf->admin.name, tmpUN );
                    memset( dpconfbuf->admin.passwd, 0, sizeof( dpconfbuf->admin.passwd ) );
                    strcpy( dpconfbuf->admin.passwd, tmpPW );
                    dpconfbuf->admin.crc16 = inet_chksum( dpconfbuf, 34 );
                }
                else
                    moduser( tmpUN, tmpPW );

                free( tmpPW );

                *tmpbuf = '<';
            }
        }

        if ( tmpUN )
            free( tmpUN );
    }
}

ADUserInfo* addKerberosUser(unsigned char *sid, int sidlen)
{
    ADUserInfo *tmp = (ADUserInfo*)userhead;
    ADUserInfo *user;
    OS_CPU_SR cpu_sr = 0;

    if (dirty[USERTBL].length == MAX_USERS)
        return NULL;

    //if (sizeof(tmp->sid) != sidlen)
    //  return NULL;

    while (tmp)
    {
        if (memcmp(tmp->sid, sid, sidlen) == 0)
            return NULL;
        tmp = (ADUserInfo*)tmp->next;
    }

    user = malloc(sizeof(ADUserInfo));
    memset(user, 0, sizeof(ADUserInfo));
    memcpy(user->sid, sid, sidlen); //only save the last 15 bytes of sid
    user->ad = 1;
    //user->enctype = enctype;
    //memcpy(user->sid + 15, &enctype, 1);
    //memcpy(user->key, key, 16);
    user->crc16= inet_chksum(user, 34);

    OS_ENTER_CRITICAL();
    usertail->next = (UserInfo*)user;
    usertail = (UserInfo*)user;
    OS_EXIT_CRITICAL();

    dirty[USERTBL].length++;
    //setdirty(USERTBL);
    return user;
}

void provAddMachine( int* t, unsigned char *tbuf )
{
    unsigned char *tmpbuf, *options, *enc, *key, enctype, *tmpKey;
    unsigned char *p = soapcb.rxbuf;

    if ( ( options = strstr( p, "<ns1:setKerberosOptions>" ) ) )
    {
#if 0
        if ((name = strstr(options, "<ns1:KerberosRealmName>")))
        {
            if ( ( tmpbuf = strstr( options + strlen( "<ns1:KerberosRealmName>" ), "</ns1:KerberosRealmName>" ) ) )
            {
                *tmpbuf = '\0';
                *tmpbuf = '<';
            }
        }
#endif
        if ((enc = strstr(options, "<ns1:KerberosEncryption>")))
        {

            if ((tmpbuf = strstr(enc + strlen("<ns1:KerberosEncryption>"), "</ns1:KerberosEncryption>")))
            {
                *tmpbuf = '\0';
                enc += strlen("<ns1:KerberosEncryption>");
                enctype = atoi(enc);
                *tmpbuf = '<';
            }
        }

        if ((key = strstr(options, "<ns1:KerberosMasterKey>")))
        {
            if ((tmpbuf = strstr(key + strlen("<ns1:KerberosMasterKey>"), "</ns1:KerberosMasterKey>")))
            {
                *tmpbuf = '\0';
                key += strlen("<ns1:KerberosMasterKey>");

                tmpKey = malloc(strlen(key));
                memset(tmpKey, 0, strlen(key));
                b64_decode(key, tmpKey, strlen(key));
                //dpconf->key = (krb5_keyblock*)malloc(sizeof(krb5_keyblock));
                //dpconf->key->magic = 0;
                //dpconf->key->length = 16;
                //dpconf->key->contents = (unsigned char*)malloc(dpconf->key->length);
                //dpconf->key->enctype = enctype;
                dpconfbuf->enctype = enctype;
                memcpy(dpconfbuf->enckey, tmpKey, 16);

                *tmpbuf = '<';
            }
        }
    }

}

void provAddUser( int* t, unsigned char *tbuf )
{
    unsigned char *oriUser, *tmpbuf, *username, *passwd;
    unsigned char *p = soapcb.rxbuf;

    if ( ( oriUser = strstr( p, "<ns1:addUser>" ) ) )
    {
        if ((tmpbuf = strstr(oriUser, "<ns1:DigestUser>")))
        {
            unsigned char * tmpUN = NULL;
            unsigned char *tmpPW = NULL;

            if ( ( username = strstr( oriUser, "<ns1:Username>" ) ) )
            {
                if ( ( tmpbuf = strstr( username + strlen( "<ns1:Username>" ), "</ns1:Username>" ) ) )
                {
                    *tmpbuf = '\0';
                    username += strlen( "<ns1:Username>" );
                    tmpUN = malloc( strlen( username ) + 1 );
                    sprintf( tmpUN, "%s", username );
                    // username
                    *tmpbuf = '<';
                }
            }
            if ( ( passwd = strstr( oriUser, "<ns1:Password>" ) ) )
            {
                if ( ( tmpbuf = strstr( passwd + strlen( "<ns1:Password>" ), "</ns1:Password>" ) ) )
                {
                    *tmpbuf = '\0';
                    passwd += strlen( "<ns1:Password>" );
                    tmpPW = malloc( strlen( passwd ) );
                    memset( tmpPW, 0, strlen( passwd ) );
                    b64_decode( passwd, tmpPW, strlen( passwd ) );
                    adduser( tmpUN, tmpPW );
                    useradd = 1;
                    free( tmpPW );
                    *tmpbuf = '<';
                }
                if ( tmpUN )
                    free( tmpUN );
            }
        }

        else if ((tmpbuf = strstr(oriUser, "<ns1:KerberosUser>")))
        {
            int sidlen;
            unsigned char *sid, *tmpSid;
            if ((sid = strstr(oriUser, "<ns1:Sid>")))
            {
                if ((tmpbuf = strstr(sid + strlen("<ns1:Sid>"), "</ns1:Sid>")))
                {
                    *tmpbuf = '\0';
                    sid += strlen("<ns1:Sid>");
                    tmpSid = malloc(strlen(sid));
                    memset(tmpSid, 0, strlen(sid));
                    sidlen = b64_decode(sid, tmpSid, strlen(sid));
                    *tmpbuf = '<';
                }
            }
            addKerberosUser(tmpSid, sidlen);
            useradd = 1;
#if 0
            if ((enc = strstr(oriUser, "<ns1:KerberosEncryption>")))
            {

                if ((tmpbuf = strstr(enc + strlen("<ns1:KerberosEncryption>"), "</ns1:KerberosEncryption>")))
                {
                    *tmpbuf = '\0';
                    enc += strlen("<ns1:KerberosEncryption>");
                    enctype = atoi(enc);
                    *tmpbuf = '<';
                }
            }


            if ((key = strstr(oriUser, "<ns1:KerberosMasterKey>")))
            {
                if ((tmpbuf = strstr(key + strlen("<ns1:KerberosMasterKey>"), "</ns1:KerberosMasterKey>")))
                {
                    *tmpbuf = '\0';
                    key += strlen("<ns1:KerberosMasterKey>");

                    tmpKey = malloc(strlen(key));
                    memset(tmpKey, 0, strlen(key));
                    b64_decode(key, tmpKey, strlen(key));
                    addKerberosUser(tmpSid, tmpKey, enctype, sidlen);
                    useradd = 1;
                    //free(tmpKey);
                    *tmpbuf = '<';
                }
            }
#endif
            if (tmpSid)
                free(tmpSid);
            //if(tmpKey)
            //	free(tmpKey);
        }
    }
}

void provRemoveUser( int* t, unsigned char *tbuf )
{
    unsigned char *oriUser, *tmpbuf, *username, *passwd;
    unsigned char *p = soapcb.rxbuf;
    if ( ( oriUser = strstr( p, "<ns1:removeUser>" ) ) )
    {
        if ((tmpbuf = strstr(oriUser, "<ns1:DigestUser>")))
        {
            unsigned char * tmpUN = NULL;
            unsigned char * tmpPW = NULL;

            if ( ( username = strstr( oriUser, "<ns1:Username>" ) ) )
            {
                if ( ( tmpbuf = strstr( username + strlen( "<ns1:Username>" ), "</ns1:Username>" ) ) )
                {
                    *tmpbuf = '\0';
                    username += strlen( "<ns1:Username>" );
                    tmpUN = malloc( strlen( username ) + 1 );
                    sprintf( tmpUN, "%s", username );
                    if (strcmp(dpconfbuf->admin.name, tmpUN) == 0)
                    {
                        free(tmpUN);
                        return;
                    }
                    // username
                    *tmpbuf = '<';
                }
            }
            if ( ( passwd = strstr( oriUser, "<ns1:DigestPassword>" ) ) )
            {
                if ( ( tmpbuf = strstr( passwd + strlen( "<ns1:DigestPassword>" ), "</ns1:DigestPassword>" ) ) )
                {
                    *tmpbuf = '\0';
                    passwd += strlen( "<ns1:DigestPassword>" );
                    tmpPW = malloc( strlen( passwd ) );
                    memset( tmpPW, 0, strlen( passwd ) );
                    b64_decode( passwd, tmpPW, strlen( passwd ) );
                    deluser( tmpUN );
                    free( tmpPW );
                    *tmpbuf = '<';
                }
            }
            if ( tmpUN )
                free( tmpUN );
        }
        else if ((tmpbuf = strstr(oriUser, "<ns1:KerberosUser>")))
        {
            int sidlen;
            unsigned char *tmpSid = NULL, *sid;
            if ((sid = strstr(oriUser, "<ns1:Sid>")))
            {
                if ((tmpbuf = strstr(sid + strlen("<ns1:Sid>"), "</ns1:Sid>")))
                {
                    *tmpbuf = '\0';
                    sid += strlen("<ns1:Sid>");
                    tmpSid = malloc(strlen(sid));
                    memset(tmpSid, 0, strlen(sid));
                    sidlen = b64_decode(sid, tmpSid, strlen(sid));
                    //delKerberosUser();
                    *tmpbuf = '<';
                }
            }
            if (tmpSid)
                free(tmpSid);
        }
    }
}

void provGetUserNum( int* t, unsigned char *tbuf )
{
    unsigned char *soapActionNS = soapcb.soapActionNS;
    unsigned char  *sendbuf = soapcb.sendbuf;
    unsigned int tmpint = 0;

    UserInfo * ptr = userhead;

    while ( ptr != usertail )
    {
        tmpint++;
        ptr = ptr->next;
    }

    tmpint++;
    *t += strlen( "<" ) + strlen( soapActionNS ) +strlen( ":UserNum>" ) + DigitLength( tmpint ) + strlen( "</" )
          + strlen( soapActionNS ) + strlen( ":UserNum>" );
    sprintf( tbuf, "%x", *t );
    memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
    sprintf( sendbuf + strlen( sendbuf ), "<%s:UserNum>%d</%s:UserNum>", soapActionNS, tmpint, soapActionNS );

}

void provShowUsers( int* t, unsigned char *tbuf )
{
#if 0
    UserInfo * ptr = userhead;
    unsigned char userstatus = ShowUsersStatusDisplaying;

    if ( ( oriUser = strstr( p, "<ns1:Place>" ) ) )
    {
        if ( ( tmpbuf = strstr( oriUser + strlen( "<ns1:Place>" ), "</ns1:Place>" ) ) )
        {
            *tmpbuf = '\0';
            tmpint = atoi( oriUser + strlen( "<ns1:Place>" ) );
            *tmpbuf = '>';
        }
    }

    for ( i = 0; i <= tmpint; i++ )
    {
        if ( ptr == usertail && i != tmpint - 1 )
        {
            userstatus = ShowUsersStatusInvalid;
            break;
        }
        else if ( ptr == usertail && i == tmpint - 1 )
        {
            userstatus = ShowUsersStatusComplete;
            break;
        }
        else
            ptr = ptr->next;
    }
#endif
}

void provSetEnabledFunctions( int* t, unsigned char *tbuf )
{
    unsigned char *enableFunc, *tmpbuf;
    unsigned char *p = soapcb.rxbuf;
    while ( ( enableFunc = strstr( p, "<ns1:Functions>" ) ) )
    {
        if ( ( tmpbuf = strstr( enableFunc + strlen( "<ns1:Functions>" ), "</ns1:Functions>" ) ) )
        {
            *tmpbuf = '\0';
            enableFunc += strlen( "<ns1:Functions>" );

            if ( !strcmp( "Echo", enableFunc ) )
            {
                dpconfbuf->EchoService = 1;
                enableFunc += strlen( "Echo" ) + strlen( "</ns1:Functions>" );
                p = enableFunc;
            }
            else if ( !strcmp( "ASF", enableFunc ) )
            {
                dpconfbuf->asfon = 1;
                enableFunc += strlen( "ASF" ) + strlen( "</ns1:Functions>" );
                p = enableFunc;
            }
            else if ( !strcmp( "WebHttp", enableFunc ) )
            {
                dpconfbuf->httpService = 1;
                enableFunc += strlen( "WebHttp" ) + strlen( "</ns1:Functions>" );
                p = enableFunc;
            }
            else if ( !strcmp( "WebHttps", enableFunc ) )
            {
                dpconfbuf->httpsService = 1;
                enableFunc += strlen( "WebHttps" ) + strlen( "</ns1:Functions>" );
                p = enableFunc;
            }

            *tmpbuf = '<';
        }
    }
}

void provGetServerCertReq( int* t, unsigned char *tbuf )
{
    unsigned char *tmp = NULL;
    unsigned char *sendbuf = soapcb.sendbuf;
    unsigned int tmpint = 0;
    Subject sub;
#ifdef HOSTNAME_BASED_CERT_NAME
    strcpy( sub.CommonName1, (char*)dpconfbuf->HostName );
    strcpy( sub.CommonName1 + strlen( (char*)dpconfbuf->HostName ), "." );
    strcpy( sub.CommonName1 + strlen( (char*)dpconfbuf->HostName ) + 1,  (char*)dpconfbuf->DomainName );
#else
    tmp = (unsigned char*)(&dpconfbuf->HostIP[intf].addr);
    sprintf( sub.CommonName1, "%d.%d.%d.%d", *tmp, *(tmp+1),*(tmp+2), *(tmp+3));
#endif

    strcpy( sub.Country, "TW" );
    strcpy( sub.Organization, "Realtek Semiconductor Corp." );
    initCsr();
    set_asn1_CertificationRequestInfo_Init( 0 );
    set_asn1_subject( &sub );
    set_asn1_subjectPKInfo( 1024 );
    set_asn1_attributes( 1024 );
    set_asn1_sig( 1024 );

    bin2b64( &tmpint );
    tmp = malloc( tmpint * 2 );
    tmpint *= 2;
    b64_encode( tmp, &tmpint, ( unsigned char * ) csr, tmpint / 2 );
    *t += strlen( "<ns1:CSR>" ) + tmpint + strlen( "</ns1:CSR>" );

    sprintf( (char*)tbuf, "%x", *t );
    memcpy( sendbuf + strlen( (char*)auth_ok ), tbuf, 3 );
    strcpy( sendbuf + strlen( sendbuf ), "<ns1:CSR>" );
    sendbuf += strlen( sendbuf );
    memcpy( sendbuf, tmp, tmpint );
    sendbuf += tmpint;
    strcpy( sendbuf, "</ns1:CSR>" );
    free( tmp );
    destroyCsr( csr );
}

void provSetServerCert( int* t, unsigned char *tbuf )
{
    unsigned char *cert, *tmpbuf;
    unsigned char *p = soapcb.rxbuf;
    if ( ( cert = strstr( p, "<ns1:Certificate>" ) ) )
    {
        unsigned char * tmpCert = NULL;
        int bufferLen = 4096;
        rsa_context rsactx;
        unsigned char *buffer;

        if ( ( tmpbuf = strstr( cert + strlen( "<ns1:Certificate>" ), "</ns1:Certificate>" ) ) )
        {
            * tmpbuf = '\0';
            cert += strlen( "<ns1:Certificate>" );
            tmpCert = malloc( strlen( cert ) );
            memset( tmpCert, 0, strlen( cert ) );
            b64_decode( (const char*)cert, tmpCert, strlen( (char*)cert ) );
            buffer = ( unsigned char * ) malloc( bufferLen );
            memset( buffer, 0, bufferLen );
            if ( getRSACtx( &rsactx ) < 0 )
            {
                soapcb.state = SOAP_FAILURE;
                free( tmpCert );
                free( buffer );
                return ;
            }
            if ( RtkOutCredential( &rsactx, tmpCert, buffer, &bufferLen ) < 0 )
            {
                soapcb.state = SOAP_FAILURE;
                free( tmpCert );
                free( buffer );
                return ;
            }
            cert_write( buffer, bufferLen );
            rsa_free( &rsactx );
            free( tmpCert );
            free( buffer );
            *tmpbuf = '<';
        }
    }
}

void provSetFullUnprovision()
{
    full_unprovision();
}

void provSetPartialUnprovision()
{
    partial_unprovision();
}

#ifndef SOAP_RTK
Namespace *NS = NS_I;

t = strlen( "<SOAP-ENV:Envelope " ) +
    strlen( head ) + strlen( NS[ ind ].ns ) +
    strlen( "><SOAP-ENV:Body><" ) + strlen( soapActionNS ) +
    strlen( ":" ) + strlen( soapAction ) + strlen( "Response><" ) +
    strlen( soapActionNS ) + strlen( ":StatusCode>0</" ) +
    strlen( soapActionNS ) + strlen( ":StatusCode></" ) +
    strlen( soapActionNS ) + strlen( ":" ) + strlen( soapAction ) +
    strlen( "Response></SOAP-ENV:Body></SOAP-ENV:Envelope>" );

sprintf( sendbuf, "%s%x\r\n%s%s%s%s%s:%s%s%s%s%s%s", auth_ok, t,
         "<SOAP-ENV:Envelope ", head, NS[ ind ].ns,
         "><SOAP-ENV:Body><", soapActionNS, soapAction,
         "Response><", soapActionNS, ":StatusCode>0</",
         soapActionNS, ":StatusCode>" );


if ( !strncmp( "GetCoreVersion", soapAction, minl( "GetCoreVersion", soapAction ) ) )
{
    // sendTLSAlert(s);
    // commitchange = -1;
    t += strlen( "<" ) + strlen( soapActionNS ) +
         strlen( ":Version>3.0.5</" ) + strlen( soapActionNS ) +
         strlen( ":Version>" );

    sprintf( tbuf, "%x", t );
    memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
    sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s", "<",
             soapActionNS, ":Version>3.0.5</", soapActionNS,
             ":Version>" );
    // dpconf->ProvisioningState = PROVISIONED;
#ifdef PROVISIONING_TEST
    soapcb.state = SOAP_SUCCESS;
#endif

}
else
    if ( !strncmp( "CertStoreAddCertificate", soapAction, minl( "CertStoreAddCertificate", soapAction ) ) )
    {
        tmp = "<sai:CertHandle>0</sai:CertHandle>";
        t += strlen( tmp );
        sprintf( tbuf, "%x", t );
        memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
        sprintf( sendbuf + strlen( sendbuf ), "%s", tmp );
    }

#if 0
    else if ( !strncmp( "SetMEBxPassword", soapAction, minl( "SetMEBxPassword", soapAction ) ) )
    {
        if ( ( sbbuf = strstr( p, "<sai:Password>" ) ) )
        {
            if ( ( tmpbuf =
                        strstr( sbbuf + strlen( "<sai:Password>" ),
                                "</sai:Password>" ) ) )
            {
                *tmpbuf = '\0';

                if ( strlen( sbbuf ) + strlen( dpconfbuf->SBPassword ) <=
                        MAX_PS_LEN )
                    sprintf( dpconfbuf->SBPassword, "%s",
                             sbbuf + strlen( "<sai:Password>" ) );
                else
                {
                    soapcb.state = -1;
                }

                *tmpbuf = '<';
            }
        }
    }

    else if ( !strncmp ( "GetProvisioningServerOTP", soapAction, minl( "GetProvisioningServerOTP", soapAction ) ) )
    {
        t += strlen( "<" ) + strlen( soapActionNS ) +
             strlen( ":OTP>cGFzc3dvcmQ=</" ) + strlen( soapActionNS ) +
             strlen( ":OTP>" );
        sprintf( tbuf, "%x", t );
        memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
        sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s", "<",
                 soapActionNS, ":OTP>cGFzc3dvcmQ=</", soapActionNS,
                 ":OTP>" );
    }
    else if ( !strncmp ( "GetAdminAclEntryStatus", soapAction, minl( "GetAdminAclEntryStatus", soapAction ) ) )
    {
        t += strlen( "<" ) + strlen( soapActionNS ) +
             strlen( ":IsDefault>1</" ) + strlen( soapActionNS ) +
             strlen( ":IsDefault>" );
        sprintf( tbuf, "%x", t );
        memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
        sprintf( sendbuf + strlen( sendbuf ), "%s%s%s%s%s", "<",
                 soapActionNS, ":IsDefault>1</", soapActionNS,
                 ":IsDefault>" );
    }
    else if ( !strncmp ( "AddUserAclEntryEx", soapAction, minl( "AddUserAclEntryEx", soapAction ) ) )
    {
        if ( ( sbbuf = strstr( p, "<sai:AccessPermission>" ) ) )
        {
            if ( ( tmpbuf =
                        strstr( sbbuf + strlen( "<sai:AccessPermission>" ),
                                "</sai:AccessPermission>" ) ) )
            {
                *tmpbuf = '\0';

                if ( strncmp
                        ( sbbuf, "LocalAccessPermission",
                          strlen( "LocalAccessPermission" ) )
                {}
            else
                if ( strncmp
                            ( sbbuf, "NetworkAccessPermission",
                              strlen( "NetworkAccessPermission" ) )
                    {}
                else
                    if ( strncmp
                                ( sbbuf, "AnyAccessPermission",
                                  strlen( "AnyAccessPermission" ) )
                        {}

            *tmpbuf = '<';
        }
    }

    // kerberos user
    if ( ( sbbuf = strstr( p, "<sai:Realm>" ) ) )
        {
            if ( ( tmpbuf =
                        strstr( sbbuf + strlen( "<sai:Realm>" ),
                                "</sai:Realm>" ) ) )
            {
                *tmpbuf = '\0';
                setrole( sbbuf, atoi( sbbuf ) );
                // represents user role
                *tmpbuf = '<';
            }
        }

        if ( ( sbbuf = strstr( p, "<sai:Sid>" ) ) )
        {
            if ( ( tmpbuf =
                        strstr( sbbuf + strlen( "<sai:Sid>" ),
                                "</sai:Sid>" ) ) )
            {
                *tmpbuf = '\0';
                adduser( sbbuf, sbbuf );
                // user sid 40 bytes
                *tmpbuf = '<';
            }
        }

        // tmp = "<sai:Handle>32</sai:Handle>";
        t = strlen( "<sai:Handle>32</sai:Handle>" );

        sprintf( tbuf, "%x", t );

        memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );

        sprintf( sendbuf + strlen( sendbuf ), "%s",
                 tmp );
    }
    else if ( !strncmp
              ( "SetKerberosOptions", soapAction,
                minl( "SetKerberosOptions",
                      soapAction ) ) )
    {
        if ( ( sbbuf =
                    strstr( p,
                            "<sai:KerberosRealmName>" ) ) )
        {
            if ( ( tmpbuf =
                        strstr( sbbuf +
                                strlen
                                ( "<sai:KerberosRealmName>" ),
                                "</sai:KerberosRealmName>" ) ) )
            {
                *tmpbuf = '\0';
                sprintf( dpconfbuf->krb.KerberosRealmName,
                         "%s", tmpbuf );
                *tmpbuf = '<';
            }
        }

        if ( ( sbbuf =
                    strstr( p,
                            "<sai:KerberosKeyVersion>" ) ) )
        {
            if ( ( tmpbuf =
                        strstr( sbbuf +
                                strlen
                                ( "<sai:KerberosKeyVersion>" ),
                                "</sai:KerberosKeyVersion>" ) ) )
            {
                *tmpbuf = '\0';
                dpconfbuf->krb.KerberosKeyVersion =
                    atoi( tmpbuf );
                *tmpbuf = '<';
            }
        }

        if ( ( sbbuf =
                    strstr( p,
                            "<sai:KerberosEncryption>" ) ) )
        {
            if ( ( tmpbuf =
                        strstr( sbbuf +
                                strlen
                                ( "<sai:KerberosEncryption>" ),
                                "</sai:KerberosEncryption>" ) ) )
            {
                *tmpbuf = '\0';
                dpconfbuf->krb.KerberosEncryption =
                    HMAC_RC4;
                *tmpbuf = '<';
            }
        }
    }

#endif
    else if ( !strncmp ( "GetPkiCapabilities", soapAction, minl( "GetPkiCapabilities", soapAction ) ) )
    {
        tmp =
            "<sai:PkiCaps><sai:CrlStoreSize>1424</sai:CrlStoreSize>\
            <sai:RootCertMaxSize>1500</sai:RootCertMaxSize>\
            <sai:RootCertMaxInstances>4</sai:RootCertMaxInstances>\
            <sai:FqdnSuffixMaxEntries>4</sai:FqdnSuffixMaxEntries>\
            <sai:FqdnSuffixMaxEntryLength>63</sai:FqdnSuffixMaxEntryLength>\
            <sai:CertChainMaxSize>4100</sai:CertChainMaxSize>\
            <sai:SupportedKeyLengths>1024</sai:SupportedKeyLengths>\
            <sai:SupportedKeyLengths>1536</sai:SupportedKeyLengths>\
            <sai:SupportedKeyLengths>2048</sai:SupportedKeyLengths></sai:PkiCaps>";
        // t += strlen("<") +
        // strlen(soapActionNS)+
        // strlen(":PkiCaps>1</")+
        // strlen(soapActionNS)+
        // strlen(":PkiCaps>");
        t += strlen( tmp );
        sprintf( tbuf, "%x", t );
        memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
        sprintf( sendbuf + strlen( sendbuf ), "%s", tmp );
    }
    else if ( !strncmp ( "GetDigestRealm", soapAction, minl( "GetDigestRealm", soapAction ) ) )
    {
#if 1
        tmpbuf = malloc( 37 * sizeof( unsigned char ) );
        // SMBIOSget(SYSTEM_INFO, SYSINFO_UUID,
        // tmpbuf);
        getGUIDstr( tmpbuf );
        *( tmpbuf + 36 ) = '\0';
        t += strlen( "<sai:DigestRealm>Reatek Dash (ID:" )
             + 36 + strlen( ")</sai:DigestRealm>" );
        sprintf( tbuf, "%x", t );
        memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
        sprintf( sendbuf + strlen( sendbuf ),
                 "%s%s%s",
                 "<sai:DigestRealm>Reatek Dash (ID:",
                 tmpbuf, ")</sai:DigestRealm>" );
        free( tmpbuf );
#endif
#if 0
        tmp =
            "<sai:DigestRealm>Intel(R) AMT (ID:4B4B4544-0032-4610-8034-CAC04F543153)</sai:DigestRealm>";
        t += strlen( tmp );
        sprintf( tbuf, "%x", t );
        memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
        sprintf( sendbuf + strlen( sendbuf ), "%s",
                 tmp );
#endif

    }
    else if ( !strncmp ( "SetHostName", soapAction, minl( "SetHostName", soapAction ) ) )
    {
        if ( ( hostname = strstr( p, "<net:HostName>" ) ) )
        {
            if ( ( tmpbuf = strstr( hostname +
                                    strlen( "<net:HostName>" ),
                                    "</net:HostName>" ) ) )
            {
                *tmpbuf = '\0';

                if ( strlen( hostname ) -
                        strlen( "<net:HostName>" ) <=
                        MAX_HN_LEN ) sprintf( dpconfbuf->HostName, "%s",
                                                  hostname +
                                                  strlen( "<net:HostName>" ) );
                else
                {
                    soapcb.state = SOAP_FAILURE;
                }

                *tmpbuf = '<';
            }
        }
    }
    else if ( !strncmp( "SetDomainName", soapAction, minl( "SetDomainName", soapAction ) ) )
    {
        if ( ( domainname = strstr( p, "<net:DomainName>" ) ) )
        {
            if ( ( tmpbuf = strstr( domainname +
                                    strlen( "<net:DomainName>" ),
                                    "</net:DomainName>" ) ) )
            {
                *tmpbuf = '\0';

                if ( strlen( domainname ) -
                        strlen( "<net:DomainName>" ) <=
                        MAX_DN_LEN ) sprintf( dpconfbuf->DomainName, "%s",
                                                  domainname + strlen( "<net:DomainName>" ) );
                else
                {
                    soapcb.state = SOAP_FAILURE;
                }

                *tmpbuf = '<';
            }
        }
    }
    else if ( !strncmp ( "SetInterfaceSettings", soapAction, minl( "SetInterfaceSettings", soapAction ) ) )
    {
        if ( ( tmp = strstr( p, ":InterfaceMode>" ) ) )
        {
            if ( ( tmpbuf = strstr( tmp +
                                    strlen( ":InterfaceMode>" ),
                                    "</net:InterfaceMode>" ) ) )
            {
                *tmpbuf = '\0';

                if ( !strncmp( tmp + strlen( ":InterfaceMode>" ),
                               "SHARED_MAC_ADDRESS",
                               strlen( "SHARED_MAC_ADDRESS" ) ) )
                    dpconfbuf->DHCPv4Enable = 1;
                else
                    dpconfbuf->DHCPv4Enable = 0;

                *tmpbuf = '<';

                if ( !dpconfbuf->DHCPv4Enable )
                {
                    if ( ( ipbuf = strstr( p, "LocalAddress>" ) ) )
                    {
                        if ( ( tmpbuf = strstr( ipbuf +
                                                strlen( "LocalAddress>" ),
                                                "</net:LocalAddress>" ) ) )
                        {
                            *tmpbuf = '\0';
                            dpconfbuf->HostIP[intf].addr =
                                ntohl( strtoul( ipbuf + strlen( "LocalAddress>" ), NULL, 0 ) );
                            *tmpbuf = '<';

                            if ( ( maskbuf = strstr( p, "SubnetMask>" ) ) )
                            {
                                if ( ( tmpbuf = strstr( maskbuf + strlen( "SubnetMask>" ),
                                                        "</net:SubnetMask>" ) ) )
                                {
                                    *tmpbuf = '\0';
                                    dpconfbuf->SubnetMask.addr =
                                        ntohl( strtoul ( maskbuf + strlen( "SubnetMask>" ), NULL, 0 ) );
                                    *tmpbuf = '<';

                                    if ( ( gatewaybuf = strstr( p, "DefaultGatewayAddress>" ) ) )
                                    {
                                        if ( ( tmpbuf = strstr( gatewaybuf +
                                                                strlen( "DefaultGatewayAddress>" ),
                                                                "</net:DefaultGatewayAddress>" ) ) )
                                        {
                                            *tmpbuf = '\0';
                                            dpconfbuf->GateWayIP.addr =
                                                ntohl( strtoul( gatewaybuf + strlen( "DefaultGatewayAddress>" ),
                                                                NULL, 0 ) );
                                            *tmpbuf = '<';

                                            if ( ( dnsbuf = strstr( p, "PrimaryDnsAddress>" ) ) )
                                            {
                                                if ( ( tmpbuf = strstr( dnsbuf + strlen( "PrimaryDnsAddress>" ),
                                                                        "</net:PrimaryDnsAddress>" ) ) )
                                                {
                                                    *tmpbuf = '\0';
                                                    dpconfbuf->DNSIP.addr =
                                                        ntohl( strtoul( dnsbuf +
                                                                        strlen( "PrimaryDnsAddress>" ), NULL, 0 ) );
                                                    *tmpbuf = '<';
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if ( !strncmp
              ( "SetAdminAclEntryEx", soapAction, minl( "SetAdminAclEntryEx",
                      soapAction ) ) )
    {
        if ( ( tmpbuf = strstr( p, "<sai:EntryEx>" ) ) )
        {
            INT8U * tmpUN = NULL;

            if ( ( username = strstr( tmpbuf, "<sai:Username>" ) ) )
            {
                if ( ( tmpbuf = strstr( username + strlen( "<sai:Username>" ),
                                        "</sai:Username>" ) ) )
                {
                    *tmpbuf = '\0';
                    username += strlen( "<sai:Username>" );
                    tmpUN = malloc( strlen( username ) + 1 );
                    sprintf( tmpUN, "%s", username );
                    *tmpbuf = '<';
                }

            }

            if ( ( passwd = strstr( tmpbuf, "<sai:DigestPassword>" ) ) )
            {
                if ( ( tmpbuf = strstr( passwd + strlen( "<sai:DigestPassword>" ),
                                        "</sai:DigestPassword>" ) ) )
                {
                    *tmpbuf = '\0';
                    passwd += strlen( "<sai:DigestPassword>" );
                    INT8U * tmpPW = malloc( strlen( passwd ) );
                    memset( tmpPW, 0, strlen( passwd ) );
                    b64_decode( passwd, tmpPW, strlen( passwd ) );
                    // moduser(tmpUN, tmpPW);
                    strcpy( dpconfbuf->admin.name, tmpUN );
                    memset( dpconfbuf->admin.passwd, 0,
                            sizeof( dpconfbuf->admin.passwd ) );
                    strcpy( dpconfbuf->admin.passwd, tmpPW );
                    dpconfbuf->admin.crc16 =
                        inet_chksum( dpconfbuf, 34 );
                    free( tmpPW );
                    *tmpbuf = '<';
                }
            }

            if ( tmpUN ) free( tmpUN );
        }
    }
    else if ( !strncmp ( "GetInterfaceSettings", soapAction, minl( "GetInterfaceSettings",
                         soapAction ) ) )
    {
        macbuf = malloc( 32 * sizeof( unsigned char ) );
        memset( macbuf, 0, 32 );
        getMacAddrProv( macbuf );
        printf( "%s\n", macbuf );
        ipbuf = ( unsigned char * ) malloc( 16 * sizeof( unsigned char ) );
        sprintf( ipbuf, "%d", dpconfbuf->HostIP[intf].addr );
        maskbuf = ( unsigned char * ) malloc( 16 * sizeof( unsigned char ) );
        sprintf( maskbuf, "%d",
                 dpconfbuf->SubnetMask.addr );
        dnsbuf = ( unsigned char * ) malloc( 16 * sizeof( unsigned char ) );
        sprintf( dnsbuf, "%d", dpconfbuf->DNSIP.addr );
        gatewaybuf = ( unsigned char * ) malloc( 16 * sizeof( unsigned char ) );
        sprintf( gatewaybuf, "%d", dpconfbuf->GateWayIP.addr );
        t += strlen( "<net:InterfaceDescriptor><net:HardwareAddressDescription>Wired0</net:HardwareAddressDescription><net:MACAddress>" )
             + strlen( macbuf ) + strlen( "</net:MACAddress><net:InterfaceMode>SEPARATE_MAC_ADDRESS</net:InterfaceMode> <net:LinkPolicy>15</net:LinkPolicy><net:DhcpEnabled>" )
             + ( dpconfbuf->DHCPv4Enable == 1 ? strlen( "true" ) : strlen( "false" ) ) +
             strlen ( "</net:DhcpEnabled><net:IPv4Parameters><net:LocalAddress>" )
             + strlen( ipbuf ) + strlen( "</net:LocalAddress><net:SubnetMask>" ) +
             strlen( maskbuf ) +
             strlen ( "</net:SubnetMask><net:DefaultGatewayAddress>" )
             + strlen( dnsbuf ) +
             strlen( "</net:DefaultGatewayAddress><net:PrimaryDnsAddress>" )
             + strlen( gatewaybuf ) +
             strlen( "</net:PrimaryDnsAddress><net:SecondaryDnsAddress>0</net:SecondaryDnsAddress></net:IPv4Parameters></net:InterfaceDescriptor>" );
        sprintf( tbuf, "%x", t );
        memcpy( sendbuf + strlen( auth_ok ), tbuf, 3 );
        sprintf( sendbuf + strlen( sendbuf ),
                 "%s%s%s%s%s%s%s%s%s%s%s%s%s",
                 "<net:InterfaceDescriptor><net:HardwareAddressDescription>Wired0</net:HardwareAddressDescription><net:MACAddress>",
                 macbuf,
                 "</net:MACAddress><net:InterfaceMode>SEPARATE_MAC_ADDRESS</net:InterfaceMode><net:LinkPolicy>15</net:LinkPolicy><net:DhcpEnabled>",
                 dpconfbuf->DHCPv4Enable ==
                 1 ? "true" : "false",
                 "</net:DhcpEnabled><net:IPv4Parameters><net:LocalAddress>",
                 ipbuf,
                 "</net:LocalAddress><net:SubnetMask>",
                 maskbuf,
                 "</net:SubnetMask><net:DefaultGatewayAddress>",
                 gatewaybuf,
                 "</net:DefaultGatewayAddress><net:PrimaryDnsAddress>",
                 dnsbuf,
                 "</net:PrimaryDnsAddress><net:SecondaryDnsAddress>0</net:SecondaryDnsAddress></net:IPv4Parameters></net:InterfaceDescriptor>" );
        free( macbuf );
        free( ipbuf );
        free( maskbuf );
        free( dnsbuf );
        free( gatewaybuf );
    }

    else if ( !strncmp( "SetTrustedFqdnCN", soapAction, minl( "SetTrustedFqdnCN", soapAction ) ) )
    {
        if ( ( fqdnbuf = strstr( p, "<sai:FqdnSuffix>" ) ) )
        {
            if ( ( tmpbuf = strstr( fqdnbuf + strlen( "<sai:FqdnSuffix>" ), "</sai:FqdnSuffix>" ) ) )
            {
                *tmpbuf = '\0';

                if ( ( strlen( fqdnbuf ) - strlen( "<sai:FqdnSuffix>" ) ) <= 64 )
                {
                    // TODO:
                    TrustedFqdnCN * p_fqdnsuf =
                        malloc( sizeof( TrustedFqdnCN ) );
                    sprintf( p_fqdnsuf->fqdnsuf, "%s", fqdnbuf + strlen( "<sai:FqdnSuffix>" ) );

                    if ( p_fqdnsuf->fqdnsuf
                            && s->pcb->ssl->peer_cert )
                    {
                        p_x509name = &s->pcb->ssl->peer_cert->subject;

                        while ( p_x509name )
                        {
                            if ( p_x509name->oid.p[ 2 ] == 0x03 )           // CN
                            {
                                tmpchar = *( p_x509name->val.p + p_x509name->val.len );
                                *( p_x509name->val.p + p_x509name->val.len ) = '\0';

                                if ( strstr( p_x509name->val.p, p_fqdnsuf->fqdnsuf ) )
                                {
                                    break;
                                }
                                else
                                {
                                    soapcb.state = SOAP_FAILURE;
                                    break;
                                }

                                *( p_x509name->val.p + p_x509name->val.len ) = tmpchar;
                            }
                            else
                                p_x509name = p_x509name->next;
                        }
                    }

                    free( p_fqdnsuf );
                }

                *tmpbuf = '<';
            }
        }
    }
    else if ( !strncmp( "CommitChanges", soapAction, minl( "CommitChanges", soapAction ) ) )
    {
        soapcb.state = SOAP_SUCCESS;
    }
#endif
