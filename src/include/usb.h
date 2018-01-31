#include <rlx/rlx_types.h>
#include <sys/autoconf.h>
#include <sys/inet_types.h>


#define ep0_mps		64
//#define FLASH_ADDR 	0x81080000
//#define FLASH_ADDR 0x81000000

#define  CBWSIG 	0x43425355

#define  BULK_SIZE      2048 



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
enum {
    USB_HS = 0,
    USB_FS = 1
};

enum {
    USB_CBI = 0,
    USB_BULK,
    USB_INTERRUPT,
};

enum {
    USB_DISABLED = -1,
    USB_MSD = 0,
    USB_CDROM = 1,
    USB_FLOPPY = 2,
//    USB_HID = 3,
};

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

#if CONFIG_VERSION >= IC_VERSION_FP_RevA
typedef struct
{
    INT32U length: 16, rsvd: 4, devaddr:7, stall:1, ls:1, fs:1, eor:1, own:1;
    INT8U* bufaddr;
}volatile outindesc_r;
#else
typedef struct
{
    INT32U length: 16, rsvd:11, stall:1, ls:1, fs:1, eor:1, own:1;
    INT8U* bufaddr;
}volatile outindesc_r;
#endif

typedef struct _USBCB {
    struct usb_ctrlrequest *setup_pkt;
    struct command_block_wrapper  *CBW;
    struct command_status_wrapper *CSW;
    INT8U *usbbuf[2];
    INT8U *rbuf;
    PPKT  pkt;
    INT8U *qhdr;
    struct usb_dt_response dtresponse[14];//add hid dtresponse
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

struct usb_ctrlrequest {
    INT8U  Recipient : 5, Type: 2, XferDir: 1; 
    //unsigned char bRequestType;
    INT8U  bRequest;
    INT16U wValue;
    INT16U wIndex;
    INT16U wLength;
};

/*
 * USB TYPE
 */

#define USB_TYPE_STANDARD	0
#define USB_TYPE_CLASS		1
#define USB_TYPE_VENDOR		2

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
    EP_CBW ,
    EP_DATA_IN,
    EP_DATA_OUT,
    EP_STALL,
    EP_SHORT,
    EP_CSW
} ep_state_e;

/* CBW data structure */
struct command_block_wrapper {
    INT16U   SEQ;
    INT16U   Length;
    INT8U    Type;
    INT8U	 data[507];
} ;

/* CSW data structure */

struct command_status_wrapper {
    INT16U SEQ;
    INT16U Length;
    INT8U  Type;
    INT8U  bCSWStatus;
	INT16U RSV;
	INT8U  data[504];
	
} ;

#define CBWLength				512

#define INQUIRY					0x12
#define READ_FORMAT_CAPACITIES 			0x23
#define READ10					0x28
#define	WRITE10					0x2A
#define MODE_SENSE6 				0x1A
#define REQUEST_SENSE 				0x03
#define MODE_SELECT6				0x15
#define MODE_SELECT10				0x55
#define MODE_SENSE10				0x5A
#define READ_CAPACITY				0x25
#define START_STOP_UNIT				0x1B
#define TEST_UNIT_READY				0x00
#define READ_TOC                    		0x43
#define READ_IO_BLOCK               		0x66
#define PREVENT_ALLOW_MEDIUM_REMOVAL 		0x1E
#define GET_EVENT_STATUS_NOTIFICATION           0x4A
#define READ_DISC_INFORMATION			0x51
#define GET_CONFIGURATION                       0x46
#define ERASE12                                 0xAC
#define READ_DVD_STRUCTURE                      0xAD
#define REPORT_KEY                              0xA4
#define SYNC_CACHE                              0x35
#define READ_TRACK_INFORMATION                  0x52 
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
#define ASC_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE  0x21
#define ASCQ_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE 0x00
#define ASC_MEDIUM_NOT_PRESENT                  0x3a
#define ASCQ_MEDIUM_NOT_PRESENT                 0x00
#define ASC_PERIPHERAL_DEVICE_WRITE_FAULT       0x03
#define ASCQ_PERIPHERAL_DEVICE_WRITE_FAULT      0x00
#define ASC_UNRECOVERED_READ_ERROR              0x11
#define ASCQ_UNRECOVERED_READ_ERROR             0x00
#define ASC_WRITE_PROTECTED                     0x27
#define ASCQ_WRITE_PROTECTED                    0x00
#define ep0_mps					64

void dwc_otg_ep_start_transfer(INT32U len, INT8U *addr, INT8U is_in);
void dwc_otg_ep0_start_transfer(INT16U len,INT8U *addr, INT8U is_in);
void usb_otg_init();
void device_int_setup();
void dwc_otg_flush_tx_fifo();
void dwc_otg_flush_rx_fifo();
void usb_dev_reset();
void usb_dev_init(INT8U hwreset);
void device_int_enum_done();
void device_int_out_ep();
void device_int_in_ep();
void ata_cbw();
void ata_csw();
void usb_desc_init();
INT8U IsValidCBW(INT32U epsize);
INT8U IsMeaningfulCBW();
void handle_ep_complete();
void handle_ep0();
int USBDataWebGet(PRTSkt s, INT32U start, INT32U end);
void Wt_IBIO(INT8U phyaddr,INT32U Wt_Data);
void rtkehci_init(void);
void rtkehci_ep_start_transfer(INT32U len, INT8U *addr, INT8U is_in);
void set_usb_hs();
void set_usb_fs();
void setup_phase(struct usb_ctrlrequest *request);
void usb_config_init();
void rtkehci_ep_start_transfer(INT32U len, INT8U *addr, INT8U is_in);
void rtkehci_ep0_start_transfer(INT16U len, INT8U *addr, INT8U is_in);
void rtkehci_init(void);
void rtkehci_int_handler();
void ehci_usb_enabled(void);
void ehci_usb_disabled(void);
void otg_usb_enabled(void);
void otg_usb_disabled(void);
void otg_usb_init();
void ehci_patch(void);
void ehci_intep_enabled(INT8U portnum);
void ehci_intep_disabled(INT8U portnum);
void rtkehci_INTIN_start_transfer(INT8U len, INT8U *addr, INT8U is_in,INT8U portnum);
void ehci_init_hid();
void disable_vnc_usb_dev();
void enable_vnc_usb_dev();
