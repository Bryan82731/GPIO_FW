/*
 * Realtek Semiconductor Corp.
 *
 * Board Support Package header file
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Dec. 07, 2007
 */


#ifndef  _BSP_H_
#define  _BSP_H_
#include <unistd.h>
#include <stddef.h>
#include <sys/dpdef.h>
#include <sys/ucos_ii.h>    /* prototype, OS and CPU API             */
#include <sys/autoconf.h>
#include <sys/inet_types.h>
#include <rlx/rlx_cpu.h>
#include <app_cfg.h>
#include "bsp_cfg.h"    /* address mapping and BSP configuration */
#include "smbus.h"
#include "x509.h"
#include "debug.h"

#define ROM_EXTERN 

//#define ASF_BOOT_ONLY
//#define PROFILING

#define IC_VERSION_DP_RevA 1
#define IC_VERSION_DP_RevB 2
#define IC_VERSION_DP_RevD 4

//DP Revf
//Hw Feature:Port Filter, Support Single/Dual IP mode but do not fixed DHCP issue
//Rom Code: lib.c arp.c ip.c tcp.c rtskt.c
#define IC_VERSION_DP_RevF 6

#define IC_VERSION_EP_RevA 10

#ifdef CONFIG_USB_DEBUG
#define USB_ENTRIES 200
#define USB_UNIT     36
#endif

#define SMBUS_ENTRIES 	64

/*
#ifdef CONFIG_MEMDEBUG
#define malloc(x) dbgmalloc((x), __FILE__, __LINE__)
#define free(x)   dbgfree((x), __FILE__, __LINE__)
void *dbgmalloc(int size, char *fn, int line);
void dbgfree(void *ptr, char *fn, int line);
#else
#ifndef CONFIG_BUILDROM
#define malloc(x) mallocx(x)
#define free(x)   freex(x)
#endif
#endif
*/

#define MD5_SHA1 						0
#define AES128_CBC_ENCRYPT 	1
#define AES128_CBC_DECRYPT 	2
#define SHA1_HMAC_20				3
#define MD5_HMAC_16					4
#define SHA1_HMAC_24				5
#define MD5_HMAC_24					6
#define HASH_KEY_SIZE 36


enum {S_S0 = 0, S_S1, S_S2, S_S3, S_S4, S_S5, S_BIOS, S_UNKNOWN = 0x0E};

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
enum {DPCONFIGTBL = 0, USERTBL, WAKETBL, ENDTBL, EVENTTBL, SMBIOSTBL, PLDMTBL,SUBTBL, ROLETBL,SWTBL};
#else
enum {DPCONFIGTBL = 0, USERTBL, EVENTTBL, SMBIOSTBL, PLDMTBL, SUBTBL,ROLETBL,SWTBL, ASFTBL, ENDTBL};
#endif

enum {PLDM_unit8 = 0, PLDM_sint8, PLDM_uint16, PLDM_sint16, PLDM_uint32, PLDM_sint32};

enum {
    INVALID = 0,
    ARP_PKT,
    EAP_PKT,
    V4_PKT,
    V6_PKT,
};

enum {
    SMBIOS,
    EVENTLOG,
    USERINFO,
    PLDM,
};

typedef struct
{
   PRTSkt tcrcs;
   PRTSkt tcrSrv;
   PPKT   pkt;
   INT8U  datardy;
   INT8U  state;
   INT8U  bootopt;
   INT8U  type;
   INT8U contype;
}TCRCB;

typedef struct
{
    INT8U *ptr;
    INT32U length;
}RxQMsg;

typedef struct
{
    INT8U   *addr;
    INT8U   *flashaddr;
    INT16U  length;
    INT16U  reset:1, dirty:1, wait:1, rsvd: 13;
}flash_data_t;

typedef struct
{
    INT8U   own;
    INT8U   opt;
    INT16U  len;
    INT8U  *bufaddr;
}volatile tcr_buf_t;

typedef struct
{
    union
    {
        INT32U word;

        struct
        {
            INT32U buffer_size : 14;
            INT32U rsvd	       : 16;
            INT32U eor	       : 1;
            INT32U own	       : 1;
        }cmd;

        struct
        {
            INT32U length  : 14;
            INT32U tcpf    : 1;
            INT32U udpf    : 1;
            INT32U ipf     : 1;
            INT32U tcpt    : 1;
            INT32U udpt    : 1;
            INT32U dash    : 1;
            INT32U rmcp    : 1;
            INT32U phy     : 1;
            INT32U mar     : 1;
            INT32U dip     : 1;
            INT32U sip     : 1;
            INT32U mobf    : 1;
            INT32U v4f     : 1;
            INT32U v6f     : 1;
            INT32U ls      : 1;
            INT32U fs      : 1;
            INT32U eor     : 1;
            INT32U own     : 1;
        }st;

    }offset0;

    union
    {
        INT32U word;
        struct
        {
INT32U vlan_tag    :
            16;
INT32U tava        :
            1;
INT32U ripv4       :
            1;
INT32U ripv6       :
            1;
INT32U rtcp        :
            1;
INT32U rsvd        :
            8;
INT32U rmcp        :
            1;
INT32U mobf        :
            1;
INT32U v4f         :
            1;
INT32U v6f         :
            1;
        }st;
    }offset1;

    INT8U *rx_buf_addr_low;
    INT8U *rx_buf_addr_high;

}volatile rxdesc_t;

typedef struct
{
    union
    {
        INT32U word;

        struct
        {
INT32U length      :
            16;
INT32U rsvd	   :
            9;
INT32U gtsenv6     :
            1;
INT32U gtsenv4     :
            1;
INT32U lgsen       :
            1;
INT32U ls          :
            1;
INT32U fs          :
            1;
INT32U eor	   :
            1;
INT32U own	   :
            1;
        }cmd;

        struct
        {
INT32U rsvd    :
            28;
INT32U ls      :
            1;
INT32U fs      :
            1;
INT32U eor     :
            1;
INT32U own     :
            1;
        }st;

    }offset0;

    union
    {
        INT32U word;
        struct
        {
INT32U vlan_tag    :
            16;
INT32U rsvd        :
            1;
INT32U tagc        :
            1;
INT32U tcpho       :
            10;
INT32U v6f         :
            1;
INT32U ipv4cs      :
            1;
INT32U tcpcs       :
            1;
INT32U udpcs       :
            1;
        }cmd;
    }offset1;

    INT8U  *tx_buf_addr_low;
    INT8U  *tx_buf_addr_high;

}volatile txdesc_t;


typedef struct
{
    INT8U  *bufaddr;
    void (*fn)(INT8U *bufaddr);
}event_t;

typedef struct
{
    union
    {
        INT32U word;
        struct
        {
INT32U length      :
            11;
INT32U rsvd        :
            15;
INT32U alsel       :
            3;

INT32U valid       :
            1;
INT32U eor         :
            1;
INT32U own         :
            1;
        }cmd;
    }offset1;

    INT32U RSVD;
    INT8U  *key_addr;
    INT8U  *payload_addr;

}volatile tlsdesc_t;

typedef struct _HwKeyFmt
{
    unsigned char SHA1[20];
    unsigned char MD5[20];
    unsigned char AES[16];
    unsigned char IV[16];
}HwKeyFmt;

void bsp_tls_init(void);
void tls_set(INT8U *keyaddr, INT8U *payloadaddr, INT8U alsel, INT16U length);
int tls_valid();
int tls_poll();
void tls_pass(); 
void tls_fail();
void tls_data_pass();
void tls_data_fail();
void tls_stop() ;

typedef struct _qhdr
{
    INT32U cmd;
    INT32U length; 
    INT8U  tip[4];
    INT16U option;
    INT16U port;
	INT8U contype;
}QHdr;

typedef struct _FWSIG
{
    INT32U  SIG;
    INT32U  EXT;
    INT32U  RSVD[2];
    INT32U  CRC32;
    INT32U  CRC32Start;
    INT32U  CRC32Size;
}FWSIG;

typedef struct _debug_msg
{
    INT32U  RxTotal;
    INT32U  TxTotal;
    INT32U  DrvDrop;
    INT32U  DrvBusy;
    INT32U  TDU;
    INT32U  RDU;
}DebugMsg;

typedef struct {
    const char *name;
    int (*open_d )(const void *path, int flags, int mode);
    int (*close_d )(int fd );
    _ssize_t (*write_d ) (int fd, const void *ptr, size_t len );
    _ssize_t (*read_d )(int fd, void *ptr, size_t len );
} devop_t;

enum { SRV_NONE = 0, 
       SRV_START, 
       SRV_STOP, 
       SRV_RESTART, 
       SRV_TX, 
       SRV_RX,
       USB_SETUP_PKT,
       USB_EP0_IN_COMPLETE,
       USB_EP0_OUT_COMPLETE,
       USB_EP_IN_COMPLETE,
       USB_EP_OUT_COMPLETE,
       USB_CBW,
       USB_DATA_IN,
       USB_DATA_OUT,
       USB_CSW,
       USB_STATUS_ERROR,
       USB_BULK_TEST,
};


/*
********************************************************************************
* BSP definition
********************************************************************************
*/
/* Register Macro */
//for access bit fields, different register lengths between E and F
#define REGX(reg)		(*(volatile INT8U *)(reg))

#define REG32(reg)		(*(volatile INT32U *)(reg))
#define REG16(reg)		(*(volatile INT16U *)(reg))
#define REG8(reg)		(*(volatile INT8U  *)(reg))


#define PA2VA(vaddr)		((INT32U) (vaddr) | DMEM_V_BASE)
#define VA2PA(vaddr)		((INT32U) (vaddr) & ~DMEM_V_BASE)


/*
*****************************************************************************************
* FUNCTION  PROTOTYPES
*****************************************************************************************
*/
/* Initialization */
void     bsp_setup (void) _ATTRIBUTE_TRAP ;
void	 bsp_irq_disable(void);

void rx_recycle(void);
void gmac_software_hook(void) _ATTRIBUTE_SRAM;
void gmac_pcie_hook(void);

/* GMAC functions */
INT32U send(INT8U *txbufferaddress, INT32U length,INT8U options, void *free) _ATTRIBUTE_SRAM;
//INT32U send(INT8U *txbufferaddress, INT32U length,INT8U options, void *free) ;
void GetMacAddr(INT8U* addr);
void SetMacAddr(INT8U* addr);
void tcr_received(INT8U *ptr, INT8U len) _ATTRIBUTE_SRAM;
void do_loop(INT32U count);
void SOLTxTask(void *pdata);
void SOLRxTask(void *pdata);
void USBTxTask(void *pdata);
void FlashTask(void *pdata);
void bsp_tcr_thr_enable();
void bsp_tcr_thr_disable();
void *reassign(void **p, size_t byte);
void rlx_irq_register(RlxIrqIdType irq_id, RlxIrqHandlerType handler);
inline void bsp_wdt_kick();
void bsp_tcr_imr_enable() _ATTRIBUTE_SRAM;
void bsp_tcr_imr_disable() _ATTRIBUTE_SRAM;
void spi_write(void *ssi_map, INT32U dst, void *src, INT8U size);
void spi_write_word(void *ssi_map, INT32U dst ,INT32U word);
void spi_se_erase(void *ssi_map, INT32U dst);
INT32U spi_read_id(void *ssi_map);
INT32U spi_se_unprotect(void *ssi_map, INT32U sector);
void bsp_usb_init();
//PCI-Bridge functions
int bsp_AP_exist();
int bsp_IODrvAP_exist();
int bsp_get_inband_data(char* data, int len);
int bsp_inband_push_ack();
void bsp_usb_enabled();
void bsp_usb_disabled();
void *mallocx(INT32U size);
void freex(void *ptr);
void do_a_patch();
unsigned int parfind(void *buf1);
void bsp_wait(INT32U usec) _ATTRIBUTE_SRAM;
void bsp_set_sstate(INT8U state) _ATTRIBUTE_SRAM;
//void bsp_set_sstate(INT8U state) ;
void bsp_pldm_set(INT8U *addr, INT8U len) _ATTRIBUTE_SRAM;
void SMBUSFree(smbrxdesc_t *rxdesc) _ATTRIBUTE_SRAM;
void event_log_load() _ATTRIBUTE_INIT;
void setdirty(INT8U type);
void setreset(INT8U type);
void setwait(INT8U type);
INT8U bsp_get_sstate(void);
void set_remote_dbg(INT16U port);
inline INT32U bsp_in_band_exist();
void bsp_bits_set(INT8U offset, INT32U value, INT8U bits, INT8U width) _ATTRIBUTE_SRAM;
int socket_open(const void *path, int flags, int mode);
int socket_close(int fd);
_ssize_t socket_write(int fd, const void *ptr, size_t len);
_ssize_t socket_read(int fd,  void *ptr, size_t len);
void wsmanSrv(void *data);
void wsmanSrvTLS(void *data);
void fill_rmttbl(INT8U state) ;
void setHostName(char* str) _ATTRIBUTE_IROM;
void cpu_reset();
void full_unprovision();
void partial_unprovision();
void bsp_setup_p();
INT32U bsp_bits_get(INT8U offset, INT8U field, INT8U width) _ATTRIBUTE_SRAM;
void init_cert();
INT8U check_mac_addr(INT8U *src);
void writesig(INT8U *buffer, INT32U sig, INT8U status) ;
INT32U GetKeyStatus();
void updateCert(INT8U *key, INT8U len);
void stopUSB();
void stopTCR();
PRTSkt enableNetBIOSPtl();
void disable_master_engine() _ATTRIBUTE_INIT __attribute__((far_call));
void _boot() __attribute__((far_call));
INT8U bsp_get_link_status();
void ASFsnrpolling();
void setPLDMEndPoint(INT8U srcept, INT8U dstept);
void PLDM_BW_Patch();
void bsp_iso_handler(void);
void bsp_ison_handler(void);


#define rx_recycle_F()                rx_recycle()
#define gmac_software_hook_F()        gmac_software_hook()
#define gmac_pcie_hook_F()            gmac_pcie_hook()
#define smbus_send_F(x,y)             smbus_send(x,y)
#define BootOptions_F()               SMBus_Prepare_BootOptions()
#define send_F(w,x,y,z)               send(w,x,y,z)

    //rtSkt_F
#define EnableDrvWaitOOB_F()          EnableDrvWaitOOB()
#define DisableDrvWaitOOB_F()         DisableDrvWaitOOB()
#define getPortNumber_F(x)            getPortNumber(x)
#define getUDPAddressInfo_F(x,y)      getUDPAddressInfo(x,y)
#define rtClientSkt_F(x)              rtClientSkt(x)
#define rtGethostbyname_F(x,y,z)      rtGethostbyname(x,y,z)
#define rtSkt_F(x,y)                  rtSkt(x,y)
#define rtSktAccept_F(x)              rtSktAccept(x)
#define rtSktBind_F(x,y)              rtSktBind(x,y)
#define rtSktClose_F(x)               rtSktClose(x)
#define rtSktCloseSend_F(x,y,z)       rtSktCloseSend(x,y,z)
#define rtSktConnect_F(x,y,z)         rtSktConnect(x,y,z)
#define rtSktEnableTimeWait_F(x)      rtSktEnableTimeWait(x)    
#define rtSktListen_F(x,y)            rtSktListen(x,y)
#define rtSktReuse_F(x,y,z)           rtSktReuse(x,y,z)
#define rtSktRx_F(x,y,z)              rtSktRx(x,y,z)
#define rtSktSend_F(x,y,z)            rtSktSend(x,y,z)
#define rtSktSetUDPInfo_F(w,x,y,z)    rtSktSetUDPInfo(w,x,y,z)
#define rtSktShutdown_F(x,y)          rtSktShutdown(x,y)
#define rtSktUDPBind_F(x,y)           rtSktUDPBind(x,y)
#define rtSktUDPSendTo_F(x,y,z)       rtSktUDPSendTo(x,y,z)
           
    //TCP
#define addTcpHeader_F(x,y,z)         addTcpHeader(x,y,z)
#define postListenPort_F(x)           postListenPort(x)
#define removePCB_F(x)                removePCB(x)
#define removeSerivce_F(x)            removeSerivce(x)
#define resetIPPtl_F(x)               resetIPPtl(x)
#define srvReset_F(x)                 srvReset(x)
#define tcpClose_F(x,y)               tcpClose(x,y)
#define tcpConnectionIn_F(x,y)        tcpConnectionIn(x,y)
#define tcpDequeue_F(x,y)             tcpDequeue(x,y)
#define tcpEnqueue_F(x,y)             tcpEnqueue(x,y)
#define tcpInput_F(x)                 tcpInput(x)
#define tcpListen_F(x)                tcpListen(x)
#define tcpPCBNew_F()                 tcpPCBNew()
#define tcpPCBRegister_F(x)           tcpPCBRegister(x)
#define tcpPCBUnRegister_F(x)         tcpPCBUnRegister(x)
#define tcpPeriodChk_F(x)             tcpPeriodChk(x)
#define tcpProbe_F(x)                 tcpProbe(x)
#define tcpProcess_F(x,y)             tcpProcess(x,y)
#define tcpRx_F(x,y,z)                tcpRx(x,y,z)
#define tcpRxInput_F(x,y,z)           tcpRxInput(x,y,z)
#define tcpSend_F(x,y,z)              tcpSend(x,y,z)
#define tcpSendData_F(x,y)            tcpSendData(x,y)
#define v6pseudochksum_F(x,y)         v6pseudochksum(x,y)
    
    //Udp
#define udpInput_F(x)                 udpInput(x)
#define udpSend_F(x,y,z)              udpSend(x,y,z)   
    //IP
#define ipInput_F(x)                  ipInput(x)
#define addIPHeader_F(x,y)            addIPHeader(x,y)
    
    //arp
#define arpTableTimeOutReset_F()      arpTableTimeOutReset()
#define arpTableTimeOutUpdate_F()     arpTableTimeOutUpdate()
#define findEtherAddr_F(x,y)          findEtherAddr(x,y)
#define findIPv6EtherAddr_F(x,y)      findIPv6EtherAddr(x,y)
#define sendARPPkt_F(x,y,z)           sendARPPkt(x,y,z)
#define updateArpTable_F(x,y)         updateArpTable(x,y)
    
    //lib
#define addEthernetHeader_F(x,y)      addEthernetHeader(x,y)
#define addMulticasAddress_F(x)       addMulticasAddress(x)
    
#define allocIPPkt_F(x,y)             allocIPPkt(x,y)
#define allocPkt_F(x)                 allocPkt(x)
#define allocUDPPkt_F(x,y)            allocUDPPkt(x,y)
    
#define freePkt_F(x)                  freePkt(x)
#define sendPkt_F(x)                  sendPkt(x)
    
#define getDNSMAR_F()                 getDNSMAR()
#define getGateWayMAR_F(x)            getGateWayMAR(x)
#define getIPAddress_F()              getIPAddress()
#define getIPv6Address_F()            getIPv6Address()
    
#define resetMulticasAddress_F()      resetMulticasAddress()
    
#define sendTCPData_F(x)              sendTCPData(x)
#define setDNSIP_F(x)                 setDNSIP(x)
#define setGateWayIP_F(x)             setGateWayIP(x)
#define setIPAddress_F(x)             setIPAddress(x)
#define setIPv6Address_F(x)           setIPv6Address(x)
#define setIPv6DNS_F(x)               setIPv6DNS(x)
#define setIPv6GateWay_F(x)           setIPv6GateWay(x)
#define setLinkLocalAddress_F(x)      setLinkLocalAddress(x)
#define setMatchSubnetMask_F()        setMatchSubnetMask()
#define setMulticastList_F()          setMulticastList()
#define setSubnetMask_F(x)            setSubnetMask(x)
#define stopconn_F(x)                 stopconn(x)

/* Added */
#define ROM_EXTERN2 

/* from bsp.h in RealManage */
#define bsp_bits_set_mutex(w,x,y,z) bsp_bits_set(w,x,y,z)
#define gmacsend_F(w,x,y,z)           send(w,x,y,z)
#define tlsEncodeData_F(w,x,y,z)      tlsEncodeData(w,x,y,z)
#define deCodeTLSData_F(w,x,y,z,m)    deCodeTLSData(w,x,y,z,m)
#define doTLSAccept_F(x,y,z)          doTLSAccept(x,y,z)
#define sendTLSAlert_F(x)             sendTLSAlert(x)
           
    //TCP
#define SetIBPF_F()		      SetIBPF()
#define SetOOBPF_F()		      SetOOBPF()
#define setNewIBTCAMPortPF_F(x,y)     setNewIBTCAMPortPF(x,y)
    
    //Udp
#define allocSSHPkt_F(x,y,z)          allocSSHPkt(x,y,z)

#define gmacsend_F(w,x,y,z)           send(w,x,y,z)

//DHCP
#define dhcpSend_F(x,y) dhcpSend(x,y)
#define dhcpInput_F(x,y,z) dhcpInput(x,y,z)
#define dhcpOptions_F(w, x, y, z) dhcpOptions(w, x, y, z)
#define dhcpGetOptionsValue_F(w, x, y, z, m) dhcpGetOptionsValue(w, x, y, z, m)
#define dhcpReply_F(x, y, z) dhcpReply(x, y, z)
#define DHCPv4TimeOut_F() DHCPv4TimeOut()

//ICMP
#define icmpInput_F(x,y) icmpInput(x,y)

#endif /*  _BSP_H_ */
