#ifndef __USB_H__
#define __USB_H__
#include "bsp_cfg.h"
//tomadd 2011.10.04
//EP for 48M or 25M crystal select
//#define CLKSEL
//tomadd 2011.09.18
//#define EHCISTALL
//tomadd 2011.04.28
//#define RTKEHCI
//tomadd 2011.11.11
//for EP phase II EHCI suspend resume test
//#define EHCISUS
//tomadd 2011.10.24
//#define WINTEST
//tomadd 2011.11.08
//for EP speed change test
//#define CPUSPDCHG
//tomadd 2011.08.16
//#define Latency
//#define _DYNAMIC_FIFO_
//#define _TEST_UNIT_
//#define _ORIGINAL_CODE_
#define bulk_in_test
//#define TEST_IN
//#define USB_FLASH_MSC	1
//#define USB_DISK_DMEM	1
#define CHG_UPHY_RST		1
#define __HIGH_SPEED__
//#define REG32(reg)	(*(volatile unsigned int *)(reg))
//tomadd 2010.08.18
//#define REG16(reg)		(*(volatile INT16U *)(reg))
//#define REG8(reg)		  (*(volatile INT8U  *)(reg))
#define cpu_to_le32(x)  (x)//((x>>24) | ((x>>8)&0xff00) | ((x<<8)&0xff0000) | (x<<24))
#define cpu_to_le16(x)	(x)//((x>>8) | ((x&0x00ff)<<8))
#define Uncache_Mask	0x80000000
#if 0
#define dbg printf
#else
#define dbg
#endif
// tom add 2011.02.16
// Dynamic FIFO
/** Number of 4-byte words in the Rx FIFO in host mode when dynamic 
	 * FIFO sizing is enabled.
	 * 16 to 32768 (default 1024)  
	 */
#define dwc_param_host_rx_fifo_size_default 1024
		/** Number of 4-byte words in the non-periodic Tx FIFO in host mode 
	 * when Dynamic FIFO sizing is enabled in the core. 
	 * 16 to 32768 (default 1024)
	 */
#define dwc_param_host_nperio_tx_fifo_size_default 1024
/** Number of 4-byte words in the host periodic Tx FIFO when dynamic 
	 * FIFO sizing is enabled. 
	 * 16 to 32768 (default 1024)
	 */
#define dwc_param_host_perio_tx_fifo_size_default 1024
	/** The number of host channel registers to use.  
	 * 1 to 16 (default 12) 
	 * Note: The FPGA configuration supports a maximum of 12 host channels.
	 */
#define dwc_param_host_channels_default 12


#define DIEPTXFn         (0x104 + USB_OTG_BASE)       /* Device IN Endpoint Transmit Fifo Size Register*/


#define DSTSR            (0x808 + USB_OTG_BASE)       /* Device Status Register (RO)*/

#define DIEPCTLn         (0x900 + USB_OTG_BASE)       /* Device Logical IN Endpoint-Specific Registers */
#define DIEPINTn	       (0x908 + USB_OTG_BASE)       /* Device IN Endpoint 0 Input Register */
#define DIEPTSIZn        (0x910 + USB_OTG_BASE)       /* Device IN Endpoint 0 Transfer Size Register */
#define DOEPCTLn         (0xB00 + USB_OTG_BASE)       /* Device Control OUT Endpoint 0 Control Register*/


/* Global interrupt mask register field definitions 
*/
#define USB_D_IM				(1<<11)		/*USB OTG */
#define USB_H_IM				(1<<10)		/*USB HOST*/


/* Global interrupt status register field definitions 
*/
#define USB_D_IS				(1<<11)		/*USB OTG */
#define USB_H_IS				(1<<10)		/*USB HOST*/

/* Interrupt routing register 1 field definitions 
*/
#define USB_D_IPS				 12		/*USB OTG */
#define USB_H_IPS				 8		/*USB HOST*/

/* cathy */
#define USB_D_ILEV			6	
#define USB_H_ILEV			6

/* IRQ */
//#define OTG_IRQ					20
#ifdef CONFIG_USB_BULK_TEST
#define  BULK_SIZE      16384 
#else
#define  BULK_SIZE      2048 
#endif

enum {
    USB_CBI = 0,
    USB_BULK,
    USB_INTERRUPT,
};




int usb_otg_init();
int device_int_setup();

struct usb_ctrlrequest {
    INT8U  Recipient : 5, Type: 2, XferDir: 1; 
    //unsigned char bRequestType;
    INT8U  bRequest;
    INT16U wValue;
    INT16U wIndex;
    INT16U wLength;
};


struct usb_ctrlrequest_test {
	
	unsigned char bRequestType;
	unsigned char bRequest;
	unsigned short wValue;
	unsigned short wIndex;
	unsigned short wLength;

	
};


/*
 * USB recipients
 */
#define USB_RECIP_MASK			0x1f
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03


/*
 * USB directions
 */
#define USB_DIR_OUT			0
#define USB_DIR_IN			0x80

typedef enum 
{ 
	EHCI_USBD,
	NORMAL_USBD,
	Max_USBD_TPYE
} USBdeviceType_e; 

/**
 * States of EP0.
 */


typedef enum ep0_state
{
    EP0_DISCONNECT,		/* no host */
    EP0_IDLE,
    EP0_IN_DATA_PHASE,
    EP0_OUT_DATA_PHASE,
    EP0_STATUS,
    EP0_STALL,
    EP0_CBI,
} ep0state_e;


typedef struct usb_dt_response{
    INT8U    *ptr;
    INT16U   len;
}USB_RESPONSE;


typedef struct _USBCB {
    struct usb_ctrlrequest *setup_pkt;
    struct command_block_wrapper  *CBW;
    struct command_status_wrapper *CSW;
    INT8U *usbbuf[2];
    INT8U *rbuf;
    PPKT  pkt;
    INT8U *qhdr;
    struct usb_dt_response dtresponse[10];
    struct usb_dt_response *get_status;
    struct usb_dt_response *clear_feature;
    struct usb_dt_response *reserved1;
    struct usb_dt_response *set_feature;
    struct usb_dt_response *resvered2;
    struct usb_dt_response *set_address;
    struct usb_dt_response *get_descriptor;
    struct usb_dt_response *set_descriptor;
    struct usb_dt_response *get_configuration;
    struct usb_dt_response *set_configuration;
    struct usb_dt_response *get_interface;
    struct usb_dt_response *set_interface;
    struct usb_dt_response *sync_frame;
    INT8U   *status;
    INT8U   speedmode;
    INT8U   ep0state;
    INT8U   epstate;
    INT8U   fcnum;
    INT16U  usbmps;
    INT16S  usbtype;
    INT32U  sendlen;
    INT32U  usblen;
    INT32U  sectorsize;
    INT8U   stall;
    INT8U   usbstate;
    INT8U   waitRx;
    INT8U   errcode;
    INT8U   *config;
    INT8U   *MBR;
    INT32U  sectors;
    INT8U   mediatype;
    INT8U   bufindex;
    INT8U   usbmode; //client or server mode
    INT8U   usbSktconnected;
    INT8U   *cbi;
    INT16U  port;
    INT8U   delrequest;
    INT8U   protocol;
    INT32U  disksize;
    INT32U  irqnum;
    INT32U  usbinit;
    //INT32U  waitpacket;
    void (*usb_ep_start_transfer)(INT32U len, INT8U *addr, INT8U is_in);
    void (*usb_ep0_start_transfer)(INT16U len,INT8U *addr, INT8U is_in);
    void (*bsp_usb_disabled)(void);
    void (*bsp_usb_enabled)(void);
    void (*bsp_usb_init)(void);
    INT8U clearfeature;

	INT8U portnum;
    INT8U  *tmpbuf;
}USBCB;


/*
 * Standard requests
 */
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE		0x03
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C
#define USB_DT_CONF_HID                 0x0C
#define USB_GET_IDLE_REQUEST      0x02

/*
 * Descriptor types
 */
#define USB_DT_BULK                     0x00
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_QUALIFIER    		0x06
#define USB_DT_OTHERSPEED   		0x07
#define USB_DT_INTERFACE_POWER          0x08
#define USB_DT_CBI                      0x09
//Descriptor for HID
#define USB_DT_CONF_HID                 0x0C
#define USB_DT_CONF_KEYBOARD            0x0D
#define USB_DT_CONF_MOUSE               0x0E
#define DEVICE_REPORT_TYPE              0x22


/* Device descriptor */
struct usb_device_descriptor {
	unsigned char  bLength;
	unsigned char  bDescriptorType;
	unsigned short bcdUSB;
	unsigned char  bDeviceClass;
	unsigned char  bDeviceSubClass;
	unsigned char  bDeviceProtocol;
	unsigned char  bMaxPacketSize0;
	unsigned short idVendor;
	unsigned short idProduct;
	unsigned short bcdDevice;
	unsigned char  iManufacturer;
	unsigned char  iProduct;
	unsigned char  iSerialNumber;
	unsigned char  bNumConfigurations;
} ;

/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PHYSICAL		5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER		7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_CDC_DATA		0x0a
#define USB_CLASS_CSCID		0x0b /* chip+ smart card */
#define USB_CLASS_CONTENT_SEC		0x0d /* content security */
#define USB_CLASS_APP_SPEC		0xfe
#define USB_CLASS_VENDOR_SPEC		0xff

#define STRING_MANUFACTURER		1
#define STRING_PRODUCT			2
#define STRING_SERIAL			3

#define is_IN	1
#define is_OUT	0

#define IdeGetMaxLun 0xFE

/*
 * USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE, and
 * are read as a bit array returned by USB_REQ_GET_STATUS.  (So there
 * are at most sixteen features of each type.)
 */
#define USB_DEVICE_SELF_POWERED		0	/* (read only) */
#define USB_DEVICE_REMOTE_WAKEUP	1	/* dev may initiate wakeup */
#define USB_DEVICE_TEST_MODE		2	/* (wired high speed only) */
#define USB_DEVICE_BATTERY		2	/* (wireless) */
#define USB_DEVICE_B_HNP_ENABLE		3	/* (otg) dev may initiate HNP */
#define USB_DEVICE_WUSB_DEVICE		3	/* (wireless)*/
#define USB_DEVICE_A_HNP_SUPPORT	4	/* (otg) RH port supports HNP */
#define USB_DEVICE_A_ALT_HNP_SUPPORT	5	/* (otg) other RH port does */
#define USB_DEVICE_DEBUG_MODE		6	/* (special devices only) */

#define USB_ENDPOINT_HALT		0	/* IN/OUT will STALL */

typedef enum ep_state 
{ 
	EP_CBW,		
	EP_DATA_IN,
	EP_DATA_OUT,
	EP_CSW,
	//tomadd 2011.07.07, for bulk out in data compare
	EP_TEST
} ep_state_e; 

/* CBW data structure */
struct command_block_wrapper {
	unsigned int  dCBWSignature;
	unsigned int  dCBWTag;
	unsigned int dCBWDataTransferLength;
	unsigned char  bmCBWFlags;
	unsigned char  bCBWLUN;
	unsigned char  bCBWCBLength;
	unsigned char  rbc[16];
} ;

/* CSW data structure */

struct command_status_wrapper {
	unsigned int  dCSWSignature;
	unsigned int  dCSWTag;
	unsigned int dCSWDataResidue;
	unsigned char  bCSWStatus;
} ;

#define CBWLength	31

#define INQUIRY	0x12
#define READ_FORMAT_CAPACITIES 0x23
#define READ10	0x28
#define	WRITE10	0x2A
#define MODE_SENSE6 0x1A
#define REQUEST_SENSE 0x03
#define MODE_SELECT6				0x15
#define MODE_SELECT10				0x55
#define MODE_SENSE10				0x5A
#define READ_CAPACITY				0x25
#define START_STOP_UNIT				0x1B
#define TEST_UNIT_READY				0x00
#define SEC_PROTOCOL_IN       0xA2
#define PREVENT_MEDIA_REM     0x1E
// MSC Test
#define READ12					0xA8
#define WRITE12					0xAA
#define WRITEANDVERIFY 				0x2E
#define VERIFY  				0x2F

//Sense key
#define SK_NOT_READY				0x02
#define SK_MEDIUM_ERROR				0x03
#define SK_HARDWARE_ERROR			0x04
#define SK_ILLEGAL_REQUEST			0x05
#define SK_UNIT_ATTENTION			0x06
#define SK_DATA_PROTECT       			0x07
#endif

