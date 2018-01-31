#include <stdlib.h>
#include <string.h>
#include "app_cfg.h"
#include "bsp.h"
#include "aes.h"
#include "eap.h"
#include "bsp_cfg.h"
#include "tls.h"
#include "des.h"
#include "x509.h"
#include "md4.h"

void eapfree();

extern DPCONF *dpconf;
unsigned char TLS_VER_1_0[] = {0x03, 0x01};
MSCHV2_VARS *myvars = NULL;

#define RECORD_LAYER_LEN 5
#define HAND_PRO_HDR 4
#define CLIENT_HELLO_LEN 45
#define EAP_LEN 25
#define EAP_HDR_LEN 5 //EAP_LEN = eth header + 802.1x header + EAP_HDR_LEN
#define EAP_HDR_TLS_LEN 1+4
#define CLI_KEY_XHG_KEY_LEN 130
#define CLI_CERT_VER_LEN 130
#define CHG_CIPHER_SPEC 1
#define CLI_FINISH_LEN 32

/*** EAP Type IDs (Only for EAP types that are implemented!!) ***/

// Bogus invalid EAP method # that indicates that we are talking about
// static WEP.
#define EAP_TYPE_MD5        4
#define EAP_TYPE_OTP        5
#define EAP_TYPE_GTC        6
#define EAP_TYPE_TLS        13
#define EAP_TYPE_LEAP       17
#define EAP_TYPE_SIM        18
#define EAP_TYPE_TTLS       21
#define EAP_TYPE_AKA        23
#define EAP_TYPE_PEAP       25
#define EAP_TYPE_MSCHAPV2   26
#define EAP_TYPE_TNC        38	/* tentative assignment per TNC_IFT_v1_0_r3 */
/* (Section 7.1.3, page 27) */
#define EAP_TYPE_FAST       43
#define EAP_TYPE_PSK		47

char certreq = 0;
#define EAP_TYPE_MSCHAP 0x1a
#define EAP_TYPE_EXPANDED    0xfe

#define EAP_TLV_VENDOR_SPECIFIC_TLV 7

#define PEAP_EAP_EXTENSION    33

#define PEAP_TLV_TYPE_FLAGS  0xc000

#define PEAP_TLV_MANDATORY_FLAG 0x8000
#define PEAP_TLV_RESERVED_FLAG  0x4000

#define PEAP_TLV_CRYPTOBINDING  0x0c
#define PEAP_TLV_RESULT			0x03
#define PEAP_TLV_SOH_RESPONSE	0x03

#define PEAP_TLV_RESULT_RESERVED	0x00
#define PEAP_TLV_RESULT_SUCCESS		0x01
#define PEAP_TLV_RESULT_FAILURE		0x02

#define PEAP_CRYPTOBINDING_IPMK_SEED_STR      "Inner Methods Compound Keys"
#define PEAP_CRYPTOBINDING_IPMK_SEED_STR_LEN  27
#define PEAP_CRYPTOBINDING_IPMK_SEED_LEN      59

#define PEAP_CRYPTOBINDING_TLV_SIZE			  56

/* SSoH and SSoHR Attributes */
enum {
    SSOH_MS_MACHINE_INVENTORY = 1,
    SSOH_MS_QUARANTINE_STATE = 2,
    SSOH_MS_PACKET_INFO = 3,
    SSOH_MS_SYSTEMGENERATED_IDS = 4,
    SSOH_MS_MACHINENAME = 5,
    SSOH_MS_CORRELATIONID = 6,
    SSOH_MS_INSTALLED_SHVS = 7,
    SSOH_MS_MACHINE_INVENTORY_EX = 8
};

enum {
    SystemHealthId = 2,
    ComplianceResultCodes = 4,
    VendorSpecific = 7,
    FailureCategory = 14,

    /*Option TLV*/
    Reserved1 = 0,
    Reserved2 = 1,
    IPv4FixupServers = 3,
    TimeOfLastUpdate = 5,
    ClientId = 6,
    HealthClass = 8,
    SoftwareVersion = 9,
    ProductName = 10,
    HealthClassStatus = 11,
    SohGenerationTime = 12,
    ErrorCodes = 13,
    IPv6FixupServers = 15,
    /*16 ~ 255 Reserved*/
};

/* SMI Network Management Private Enterprise Code for vendor specific types */
enum {
    EAP_VENDOR_IETF = 0,
    EAP_VENDOR_MICROSOFT = 0x000137 /* Microsoft */,
    EAP_VENDOR_WFA = 0x00372A /* Wi-Fi Alliance */
};

enum
{
    EAP_BEGIN,
    EAP_INIT,
    EAP_EAPOL,
    EAP_IDENTITY
};

const unsigned char eapCert[]=
{
    0x00,0x05,0xd9,0x30,0x82,0x05,0xd5,0x30,0x82,0x04,0xbd,0xa0,0x03,0x02,0x01,0x02
    ,0x02,0x0a,0x61,0x74,0x8b,0x82,0x00,0x00,0x00,0x00,0x00,0x05,0x30,0x0d,0x06,0x09
    ,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00,0x30,0x45,0x31,0x13,0x30
    ,0x11,0x06,0x0a,0x09,0x92,0x26,0x89,0x93,0xf2,0x2c,0x64,0x01,0x19,0x16,0x03,0x63
    ,0x6f,0x6d,0x31,0x17,0x30,0x15,0x06,0x0a,0x09,0x92,0x26,0x89,0x93,0xf2,0x2c,0x64
    ,0x01,0x19,0x16,0x07,0x72,0x74,0x6b,0x2d,0x6e,0x61,0x63,0x31,0x15,0x30,0x13,0x06
    ,0x03,0x55,0x04,0x03,0x13,0x0c,0x77,0x69,0x6e,0x32,0x30,0x30,0x33,0x45,0x6e,0x74
    ,0x43,0x41,0x30,0x1e,0x17,0x0d,0x31,0x30,0x31,0x31,0x30,0x35,0x30,0x32,0x32,0x39
    ,0x31,0x37,0x5a,0x17,0x0d,0x31,0x31,0x31,0x31,0x30,0x35,0x30,0x32,0x32,0x39,0x31
    ,0x37,0x5a,0x30,0x52,0x31,0x13,0x30,0x11,0x06,0x0a,0x09,0x92,0x26,0x89,0x93,0xf2
    ,0x2c,0x64,0x01,0x19,0x16,0x03,0x63,0x6f,0x6d,0x31,0x17,0x30,0x15,0x06,0x0a,0x09
    ,0x92,0x26,0x89,0x93,0xf2,0x2c,0x64,0x01,0x19,0x16,0x07,0x72,0x74,0x6b,0x2d,0x6e
    ,0x61,0x63,0x31,0x0e,0x30,0x0c,0x06,0x03,0x55,0x04,0x03,0x13,0x05,0x55,0x73,0x65
    ,0x72,0x73,0x31,0x12,0x30,0x10,0x06,0x03,0x55,0x04,0x03,0x13,0x09,0x76,0x6f,0x6c
    ,0x64,0x65,0x6d,0x6f,0x72,0x74,0x30,0x81,0x9f,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48
    ,0x86,0xf7,0x0d,0x01,0x01,0x01,0x05,0x00,0x03,0x81,0x8d,0x00,0x30,0x81,0x89,0x02
    ,0x81,0x81,0x00,0xd1,0x33,0xbc,0x81,0x70,0xee,0x3e,0x30,0x32,0x25,0x2a,0xdf,0xda
    ,0x79,0xbb,0x93,0x41,0x25,0x5e,0x20,0x97,0xb1,0xeb,0xb3,0x42,0x8f,0x11,0x4d,0xa2
    ,0x6b,0xbd,0xe0,0x8c,0x8e,0x02,0x42,0x45,0x93,0x1c,0x52,0x60,0xa3,0x32,0x13,0x78
    ,0xeb,0xec,0x05,0xbd,0xf5,0x8e,0xf0,0x8a,0x23,0x98,0xdb,0x6b,0xb8,0xad,0xff,0x45
    ,0x47,0xa0,0x94,0x75,0x0d,0x46,0x6a,0x36,0xef,0x24,0xeb,0x8a,0xad,0xea,0xc5,0x56
    ,0xd7,0x36,0x3b,0x56,0xd8,0xdb,0xc8,0x3c,0x07,0x4d,0x1f,0x7c,0xb6,0xed,0xe0,0xd9
    ,0x0f,0x69,0xad,0xc0,0x70,0x2e,0x5d,0x1e,0xc4,0x34,0x3d,0xa7,0x13,0x91,0x37,0x68
    ,0xde,0xea,0xc4,0xef,0x47,0x91,0xd5,0x71,0xa6,0xf1,0x6f,0xd1,0x35,0x78,0xdc,0x72
    ,0xb7,0x23,0x43,0x02,0x03,0x01,0x00,0x01,0xa3,0x82,0x03,0x3c,0x30,0x82,0x03,0x38
    ,0x30,0x0b,0x06,0x03,0x55,0x1d,0x0f,0x04,0x04,0x03,0x02,0x05,0xa0,0x30,0x44,0x06
    ,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x0f,0x04,0x37,0x30,0x35,0x30,0x0e
    ,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x03,0x02,0x02,0x02,0x00,0x80,0x30,0x0e
    ,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x03,0x04,0x02,0x02,0x00,0x80,0x30,0x07
    ,0x06,0x05,0x2b,0x0e,0x03,0x02,0x07,0x30,0x0a,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7
    ,0x0d,0x03,0x07,0x30,0x1d,0x06,0x03,0x55,0x1d,0x0e,0x04,0x16,0x04,0x14,0x6e,0x11
    ,0xc9,0xc5,0x21,0xe9,0x65,0x7d,0xec,0xec,0xc6,0x1f,0x73,0xcf,0xa6,0x31,0x5c,0x1e
    ,0x62,0x27,0x30,0x17,0x06,0x09,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x14,0x02,0x04
    ,0x0a,0x1e,0x08,0x00,0x55,0x00,0x73,0x00,0x65,0x00,0x72,0x30,0x1f,0x06,0x03,0x55
    ,0x1d,0x23,0x04,0x18,0x30,0x16,0x80,0x14,0x53,0xa0,0x12,0x3e,0x8b,0x4b,0xee,0x43
    ,0xcb,0xe6,0x8c,0x6e,0xf5,0x99,0xa6,0xde,0xcc,0xd3,0xe5,0x3c,0x30,0x82,0x01,0x09
    ,0x06,0x03,0x55,0x1d,0x1f,0x04,0x82,0x01,0x00,0x30,0x81,0xfd,0x30,0x81,0xfa,0xa0
    ,0x81,0xf7,0xa0,0x81,0xf4,0x86,0x81,0xb6,0x6c,0x64,0x61,0x70,0x3a,0x2f,0x2f,0x2f
    ,0x43,0x4e,0x3d,0x77,0x69,0x6e,0x32,0x30,0x30,0x33,0x45,0x6e,0x74,0x43,0x41,0x2c
    ,0x43,0x4e,0x3d,0x72,0x74,0x6b,0x2d,0x6e,0x61,0x63,0x2d,0x70,0x63,0x2c,0x43,0x4e
    ,0x3d,0x43,0x44,0x50,0x2c,0x43,0x4e,0x3d,0x50,0x75,0x62,0x6c,0x69,0x63,0x25,0x32
    ,0x30,0x4b,0x65,0x79,0x25,0x32,0x30,0x53,0x65,0x72,0x76,0x69,0x63,0x65,0x73,0x2c
    ,0x43,0x4e,0x3d,0x53,0x65,0x72,0x76,0x69,0x63,0x65,0x73,0x2c,0x43,0x4e,0x3d,0x43
    ,0x6f,0x6e,0x66,0x69,0x67,0x75,0x72,0x61,0x74,0x69,0x6f,0x6e,0x2c,0x44,0x43,0x3d
    ,0x72,0x74,0x6b,0x2d,0x6e,0x61,0x63,0x2c,0x44,0x43,0x3d,0x63,0x6f,0x6d,0x3f,0x63
    ,0x65,0x72,0x74,0x69,0x66,0x69,0x63,0x61,0x74,0x65,0x52,0x65,0x76,0x6f,0x63,0x61
    ,0x74,0x69,0x6f,0x6e,0x4c,0x69,0x73,0x74,0x3f,0x62,0x61,0x73,0x65,0x3f,0x6f,0x62
    ,0x6a,0x65,0x63,0x74,0x43,0x6c,0x61,0x73,0x73,0x3d,0x63,0x52,0x4c,0x44,0x69,0x73
    ,0x74,0x72,0x69,0x62,0x75,0x74,0x69,0x6f,0x6e,0x50,0x6f,0x69,0x6e,0x74,0x86,0x39
    ,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x72,0x74,0x6b,0x2d,0x6e,0x61,0x63,0x2d,0x70
    ,0x63,0x2e,0x72,0x74,0x6b,0x2d,0x6e,0x61,0x63,0x2e,0x63,0x6f,0x6d,0x2f,0x43,0x65
    ,0x72,0x74,0x45,0x6e,0x72,0x6f,0x6c,0x6c,0x2f,0x77,0x69,0x6e,0x32,0x30,0x30,0x33
    ,0x45,0x6e,0x74,0x43,0x41,0x2e,0x63,0x72,0x6c,0x30,0x82,0x01,0x1e,0x06,0x08,0x2b
    ,0x06,0x01,0x05,0x05,0x07,0x01,0x01,0x04,0x82,0x01,0x10,0x30,0x82,0x01,0x0c,0x30
    ,0x81,0xab,0x06,0x08,0x2b,0x06,0x01,0x05,0x05,0x07,0x30,0x02,0x86,0x81,0x9e,0x6c
    ,0x64,0x61,0x70,0x3a,0x2f,0x2f,0x2f,0x43,0x4e,0x3d,0x77,0x69,0x6e,0x32,0x30,0x30
    ,0x33,0x45,0x6e,0x74,0x43,0x41,0x2c,0x43,0x4e,0x3d,0x41,0x49,0x41,0x2c,0x43,0x4e
    ,0x3d,0x50,0x75,0x62,0x6c,0x69,0x63,0x25,0x32,0x30,0x4b,0x65,0x79,0x25,0x32,0x30
    ,0x53,0x65,0x72,0x76,0x69,0x63,0x65,0x73,0x2c,0x43,0x4e,0x3d,0x53,0x65,0x72,0x76
    ,0x69,0x63,0x65,0x73,0x2c,0x43,0x4e,0x3d,0x43,0x6f,0x6e,0x66,0x69,0x67,0x75,0x72
    ,0x61,0x74,0x69,0x6f,0x6e,0x2c,0x44,0x43,0x3d,0x72,0x74,0x6b,0x2d,0x6e,0x61,0x63
    ,0x2c,0x44,0x43,0x3d,0x63,0x6f,0x6d,0x3f,0x63,0x41,0x43,0x65,0x72,0x74,0x69,0x66
    ,0x69,0x63,0x61,0x74,0x65,0x3f,0x62,0x61,0x73,0x65,0x3f,0x6f,0x62,0x6a,0x65,0x63
    ,0x74,0x43,0x6c,0x61,0x73,0x73,0x3d,0x63,0x65,0x72,0x74,0x69,0x66,0x69,0x63,0x61
    ,0x74,0x69,0x6f,0x6e,0x41,0x75,0x74,0x68,0x6f,0x72,0x69,0x74,0x79,0x30,0x5c,0x06
    ,0x08,0x2b,0x06,0x01,0x05,0x05,0x07,0x30,0x02,0x86,0x50,0x68,0x74,0x74,0x70,0x3a
    ,0x2f,0x2f,0x72,0x74,0x6b,0x2d,0x6e,0x61,0x63,0x2d,0x70,0x63,0x2e,0x72,0x74,0x6b
    ,0x2d,0x6e,0x61,0x63,0x2e,0x63,0x6f,0x6d,0x2f,0x43,0x65,0x72,0x74,0x45,0x6e,0x72
    ,0x6f,0x6c,0x6c,0x2f,0x72,0x74,0x6b,0x2d,0x6e,0x61,0x63,0x2d,0x70,0x63,0x2e,0x72
    ,0x74,0x6b,0x2d,0x6e,0x61,0x63,0x2e,0x63,0x6f,0x6d,0x5f,0x77,0x69,0x6e,0x32,0x30
    ,0x30,0x33,0x45,0x6e,0x74,0x43,0x41,0x2e,0x63,0x72,0x74,0x30,0x29,0x06,0x03,0x55
    ,0x1d,0x25,0x04,0x22,0x30,0x20,0x06,0x0a,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x0a
    ,0x03,0x04,0x06,0x08,0x2b,0x06,0x01,0x05,0x05,0x07,0x03,0x04,0x06,0x08,0x2b,0x06
    ,0x01,0x05,0x05,0x07,0x03,0x02,0x30,0x30,0x06,0x03,0x55,0x1d,0x11,0x04,0x29,0x30
    ,0x27,0xa0,0x25,0x06,0x0a,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x14,0x02,0x03,0xa0
    ,0x17,0x0c,0x15,0x76,0x6f,0x6c,0x64,0x65,0x6d,0x6f,0x72,0x74,0x40,0x72,0x74,0x6b
    ,0x2d,0x6e,0x61,0x63,0x2e,0x63,0x6f,0x6d,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86
    ,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00,0x03,0x82,0x01,0x01,0x00,0x9e,0xca,0xa7,0xd6
    ,0xfc,0x74,0x8b,0xb9,0xee,0xfb,0x3c,0xd2,0xec,0x05,0xa1,0xe7,0xb7,0x45,0x1d,0x66
    ,0x45,0x8a,0x0b,0x69,0xaa,0x54,0x87,0xe9,0x9a,0xd6,0xc7,0x33,0x0e,0x0d,0x08,0xea
    ,0x2c,0xef,0x4b,0x53,0xf4,0x99,0x67,0x56,0xa4,0x55,0x0f,0x3f,0x40,0xa5,0xca,0x47
    ,0xb4,0x44,0x42,0xa0,0xb2,0x42,0xb4,0x13,0xaa,0x34,0xcb,0xdd,0x00,0x91,0x2a,0xcd
    ,0xe0,0xe8,0x31,0xd1,0x51,0xd7,0x46,0xbf,0x64,0x09,0x79,0x3f,0x9e,0x17,0x39,0x08
    ,0x93,0x7a,0x34,0x41,0xef,0x0e,0x7a,0x21,0xfe,0x92,0x2b,0x40,0xfc,0x6f,0xc6,0x70
    ,0x48,0xe7,0x42,0x6e,0xfe,0x0d,0x63,0x5c,0xf4,0x59,0x50,0x4b,0xa8,0x91,0x66,0x64
    ,0xe6,0x35,0x0d,0x2f,0xd9,0x97,0x25,0xbd,0x4b,0x3e,0xe8,0x9f,0xb5,0xf9,0xb2,0xec
    ,0x7e,0x33,0x89,0xc9,0x97,0x97,0x5f,0xc6,0xf4,0x52,0x77,0x09,0x8f,0x9d,0x48,0xef
    ,0x15,0xbf,0x2b,0x18,0x53,0xa7,0xd1,0x75,0x9c,0x32,0xbb,0xb4,0x2f,0x83,0xa9,0xef
    ,0x35,0x82,0x39,0x79,0x2d,0xb3,0x49,0xd2,0x8a,0x31,0xfa,0x6c,0x4b,0x58,0xec,0x87
    ,0xe0,0x50,0x7f,0xf4,0x86,0x5c,0xf9,0xe5,0x3f,0xf1,0xd8,0xb1,0x30,0xb8,0x13,0x0a
    ,0x4c,0x62,0x2c,0x21,0xf6,0xec,0x69,0xa7,0x76,0xa9,0xff,0xdd,0x0e,0x1f,0x4f,0xc3
    ,0xb7,0x41,0x5a,0x01,0x28,0xc3,0x90,0x79,0x6e,0xd8,0xd3,0x6e,0xf5,0x0e,0x99,0x00
    ,0x31,0x85,0xa4,0xed,0x29,0x36,0x3a,0xd9,0xba,0xbf,0xba,0x24,0x5a,0xe9,0xab,0x75
    ,0x6c,0xec,0x0e,0xd9,0xdb,0x8c,0x99,0xa1,0x7b,0x2b,0x32,0x53,0x00,0x04,0x7a,0x30
    ,0x82,0x04,0x76,0x30,0x82,0x03,0x5e,0xa0,0x03,0x02,0x01,0x02,0x02,0x10,0x72,0xd1
    ,0xc0,0x66,0x31,0x99,0xa1,0xa4,0x42,0xe4,0x3c,0x30,0xa3,0xb7,0x72,0xe6,0x30,0x0d
    ,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00,0x30,0x45,0x31
    ,0x13,0x30,0x11,0x06,0x0a,0x09,0x92,0x26,0x89,0x93,0xf2,0x2c,0x64,0x01,0x19,0x16
    ,0x03,0x63,0x6f,0x6d,0x31,0x17,0x30,0x15,0x06,0x0a,0x09,0x92,0x26,0x89,0x93,0xf2
    ,0x2c,0x64,0x01,0x19,0x16,0x07,0x72,0x74,0x6b,0x2d,0x6e,0x61,0x63,0x31,0x15,0x30
    ,0x13,0x06,0x03,0x55,0x04,0x03,0x13,0x0c,0x77,0x69,0x6e,0x32,0x30,0x30,0x33,0x45
    ,0x6e,0x74,0x43,0x41,0x30,0x1e,0x17,0x0d,0x31,0x30,0x31,0x31,0x30,0x34,0x32,0x32
    ,0x33,0x36,0x30,0x33,0x5a,0x17,0x0d,0x31,0x35,0x31,0x31,0x30,0x34,0x32,0x32,0x34
    ,0x35,0x31,0x37,0x5a,0x30,0x45,0x31,0x13,0x30,0x11,0x06,0x0a,0x09,0x92,0x26,0x89
    ,0x93,0xf2,0x2c,0x64,0x01,0x19,0x16,0x03,0x63,0x6f,0x6d,0x31,0x17,0x30,0x15,0x06
    ,0x0a,0x09,0x92,0x26,0x89,0x93,0xf2,0x2c,0x64,0x01,0x19,0x16,0x07,0x72,0x74,0x6b
    ,0x2d,0x6e,0x61,0x63,0x31,0x15,0x30,0x13,0x06,0x03,0x55,0x04,0x03,0x13,0x0c,0x77
    ,0x69,0x6e,0x32,0x30,0x30,0x33,0x45,0x6e,0x74,0x43,0x41,0x30,0x82,0x01,0x22,0x30
    ,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x01,0x05,0x00,0x03,0x82
    ,0x01,0x0f,0x00,0x30,0x82,0x01,0x0a,0x02,0x82,0x01,0x01,0x00,0xb4,0xb4,0x19,0xe1
    ,0x96,0x23,0xab,0x23,0xd2,0x19,0x25,0x84,0x3b,0x95,0x4a,0x13,0x52,0xfa,0x1f,0x3b
    ,0xef,0x19,0x34,0xdc,0xe6,0xa3,0xae,0xe0,0xa0,0xbd,0x26,0xc2,0xb7,0x4c,0xeb,0x59
    ,0xe4,0x7a,0x9a,0xde,0x97,0x44,0x51,0x70,0x5d,0x88,0x64,0xd9,0xa5,0x79,0x7d,0x27
    ,0x06,0xd1,0xae,0xaa,0xdf,0x24,0x53,0x48,0x2b,0x62,0x4d,0xeb,0xfb,0x63,0x52,0x9d
    ,0xd3,0x74,0x7a,0x32,0xc5,0x81,0x9f,0x98,0x0a,0x09,0x8b,0x67,0x56,0x11,0x10,0xab
    ,0xf7,0xef,0xa8,0xca,0xe7,0x89,0xf8,0x25,0xd3,0xcf,0xd9,0x98,0xc0,0x3b,0x7f,0x4b
    ,0x7d,0x04,0xad,0x41,0xfb,0xe5,0x47,0x65,0xa9,0xe0,0x86,0xe2,0xe0,0xd7,0xca,0xee
    ,0x88,0xdb,0xfc,0x31,0xd9,0xc9,0x28,0x60,0xc7,0x27,0xa0,0xe8,0x05,0x70,0x1f,0x4f
    ,0xbc,0x2d,0xa7,0x9d,0x5e,0xa0,0x62,0x29,0xc4,0x33,0x38,0xba,0xda,0x38,0xfe,0x45
    ,0xf0,0x4d,0x6e,0x77,0x39,0x6d,0x66,0xb3,0x35,0xc2,0x94,0x22,0xe3,0xce,0x0f,0xa2
    ,0x97,0x0b,0x8e,0x91,0xb1,0xc0,0x99,0xb2,0x96,0x51,0x09,0x19,0xfd,0x94,0x39,0xee
    ,0x7a,0x48,0x8b,0x96,0x9d,0xbe,0x82,0xe9,0x4c,0x4d,0xc9,0x32,0x26,0x6c,0x82,0xa0
    ,0x04,0xf6,0x9f,0x98,0xbc,0x50,0xa8,0x03,0x29,0xf6,0xe8,0xc8,0xb2,0xa7,0x0b,0x70
    ,0x27,0xbc,0xe8,0xb0,0x03,0xb4,0xed,0x09,0x66,0xb7,0xe4,0x84,0x66,0x06,0x8a,0x56
    ,0x3a,0x63,0x49,0x85,0xdb,0xb5,0x5c,0x27,0x20,0x81,0x8f,0x29,0x4c,0x23,0xda,0x13
    ,0x87,0x0f,0x65,0x96,0x3a,0x84,0x15,0x7d,0x63,0x81,0x35,0x53,0x02,0x03,0x01,0x00
    ,0x01,0xa3,0x82,0x01,0x60,0x30,0x82,0x01,0x5c,0x30,0x0b,0x06,0x03,0x55,0x1d,0x0f
    ,0x04,0x04,0x03,0x02,0x01,0x86,0x30,0x0f,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff
    ,0x04,0x05,0x30,0x03,0x01,0x01,0xff,0x30,0x1d,0x06,0x03,0x55,0x1d,0x0e,0x04,0x16
    ,0x04,0x14,0x53,0xa0,0x12,0x3e,0x8b,0x4b,0xee,0x43,0xcb,0xe6,0x8c,0x6e,0xf5,0x99
    ,0xa6,0xde,0xcc,0xd3,0xe5,0x3c,0x30,0x82,0x01,0x09,0x06,0x03,0x55,0x1d,0x1f,0x04
    ,0x82,0x01,0x00,0x30,0x81,0xfd,0x30,0x81,0xfa,0xa0,0x81,0xf7,0xa0,0x81,0xf4,0x86
    ,0x81,0xb6,0x6c,0x64,0x61,0x70,0x3a,0x2f,0x2f,0x2f,0x43,0x4e,0x3d,0x77,0x69,0x6e
    ,0x32,0x30,0x30,0x33,0x45,0x6e,0x74,0x43,0x41,0x2c,0x43,0x4e,0x3d,0x72,0x74,0x6b
    ,0x2d,0x6e,0x61,0x63,0x2d,0x70,0x63,0x2c,0x43,0x4e,0x3d,0x43,0x44,0x50,0x2c,0x43
    ,0x4e,0x3d,0x50,0x75,0x62,0x6c,0x69,0x63,0x25,0x32,0x30,0x4b,0x65,0x79,0x25,0x32
    ,0x30,0x53,0x65,0x72,0x76,0x69,0x63,0x65,0x73,0x2c,0x43,0x4e,0x3d,0x53,0x65,0x72
    ,0x76,0x69,0x63,0x65,0x73,0x2c,0x43,0x4e,0x3d,0x43,0x6f,0x6e,0x66,0x69,0x67,0x75
    ,0x72,0x61,0x74,0x69,0x6f,0x6e,0x2c,0x44,0x43,0x3d,0x72,0x74,0x6b,0x2d,0x6e,0x61
    ,0x63,0x2c,0x44,0x43,0x3d,0x63,0x6f,0x6d,0x3f,0x63,0x65,0x72,0x74,0x69,0x66,0x69
    ,0x63,0x61,0x74,0x65,0x52,0x65,0x76,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x4c,0x69
    ,0x73,0x74,0x3f,0x62,0x61,0x73,0x65,0x3f,0x6f,0x62,0x6a,0x65,0x63,0x74,0x43,0x6c
    ,0x61,0x73,0x73,0x3d,0x63,0x52,0x4c,0x44,0x69,0x73,0x74,0x72,0x69,0x62,0x75,0x74
    ,0x69,0x6f,0x6e,0x50,0x6f,0x69,0x6e,0x74,0x86,0x39,0x68,0x74,0x74,0x70,0x3a,0x2f
    ,0x2f,0x72,0x74,0x6b,0x2d,0x6e,0x61,0x63,0x2d,0x70,0x63,0x2e,0x72,0x74,0x6b,0x2d
    ,0x6e,0x61,0x63,0x2e,0x63,0x6f,0x6d,0x2f,0x43,0x65,0x72,0x74,0x45,0x6e,0x72,0x6f
    ,0x6c,0x6c,0x2f,0x77,0x69,0x6e,0x32,0x30,0x30,0x33,0x45,0x6e,0x74,0x43,0x41,0x2e
    ,0x63,0x72,0x6c,0x30,0x10,0x06,0x09,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x15,0x01
    ,0x04,0x03,0x02,0x01,0x00,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01
    ,0x01,0x05,0x05,0x00,0x03,0x82,0x01,0x01,0x00,0xb3,0x55,0xe2,0x92,0x5f,0xc5,0x30
    ,0x57,0xdd,0x30,0xac,0x31,0xbf,0xb0,0xe5,0xae,0xe9,0x44,0x01,0xcd,0x89,0x3f,0x33
    ,0x44,0x7f,0x40,0xab,0xe8,0x29,0xcc,0xff,0xfe,0x07,0x9f,0xf6,0x5d,0x29,0xf5,0xeb
    ,0xa1,0x1a,0x5a,0xce,0xbd,0x60,0xd7,0x0d,0xaa,0xc6,0x26,0x74,0x20,0x3c,0xa2,0x39
    ,0x86,0x57,0x04,0xb0,0x4f,0x29,0xea,0xc9,0x27,0x93,0xa6,0x78,0xee,0xed,0x02,0xef
    ,0x1d,0x86,0xb8,0xce,0xc4,0x6b,0xa7,0x69,0xec,0xa0,0x00,0x8e,0x2c,0x4b,0x62,0xa0
    ,0xd5,0xd6,0x5f,0xf0,0x50,0xc8,0x4c,0xef,0xc6,0xe1,0x4e,0xba,0x5c,0x02,0xc7,0x78
    ,0x56,0x4b,0x9d,0xc1,0x0b,0x9c,0x03,0xa4,0xdf,0x3f,0x01,0xca,0x9c,0x03,0xf1,0x72
    ,0xd9,0x2b,0xa3,0x9c,0x61,0xd6,0x88,0x23,0x5c,0x5b,0x52,0x69,0x98,0x85,0xa8,0x30
    ,0x20,0x90,0xf5,0xd6,0x2c,0xb5,0x36,0x44,0x8c,0xb0,0x18,0x8d,0x9f,0x69,0x4c,0xed
    ,0xc6,0x66,0xd0,0xbb,0x2f,0xff,0x98,0xfd,0x8d,0x8e,0x74,0x25,0xe2,0x2f,0x57,0x1f
    ,0x15,0x17,0x55,0x78,0x98,0x69,0x0f,0xa8,0xee,0xd0,0x7e,0xac,0x12,0x95,0x0b,0xdf
    ,0x64,0x81,0x35,0xb8,0x36,0xae,0xda,0x50,0x72,0xab,0xa9,0x4f,0x73,0xe4,0xfe,0x51
    ,0x00,0x0e,0x35,0x34,0xdf,0xd3,0xfd,0xb0,0x27,0x51,0x4a,0xf5,0x07,0xf6,0x67,0xc8
    ,0x4e,0xdb,0x11,0x46,0x42,0x43,0xd6,0xb5,0xc4,0x78,0x56,0x6b,0x96,0x67,0xa8,0x1b
    ,0x0a,0x2e,0x3f,0x36,0x90,0xb5,0x0d,0x0b,0xf3,0x08,0xe4,0xf8,0x97,0x59,0x61,0xa8
    ,0x51,0x87,0x5c,0xaa,0x31,0xc6,0x01,0xd6,0xd8
};

unsigned char eapprikey[] =
    "-----BEGIN RSA PRIVATE KEY-----\r\n"
    "MIICXAIBAAKBgQDRM7yBcO4+MDIlKt/aebuTQSVeIJex67NCjxFNomu94IyOAkJF\r\n"
    "kxxSYKMyE3jr7AW99Y7wiiOY22u4rf9FR6CUdQ1GajbvJOuKrerFVtc2O1bY28g8\r\n"
    "B00ffLbt4NkPaa3AcC5dHsQ0PacTkTdo3urE70eR1XGm8W/RNXjccrcjQwIDAQAB\r\n"
    "AoGAWVw6Fyh/+7xmjw3DVPVOtFXNeml9ophFmBa4k2rfjtnN8BLy7/kozd9NlmRO\r\n"
    "8Kd+xGeRyRmYnwCLAPYB5jv0UO9kHHHGjE+JLHXLhOL3ls1Mq872U6wqT4Mgn9HS\r\n"
    "V5q1RshhRJOxSeKsRCOx7Nplx4XVKrw4ZjOOBYYQ8fttx1ECQQD6dVuzwCQZNvYl\r\n"
    "rtayI6JG5OiSmg8hNaKujsjkrXDtAkRnhRYcqLDksO+7JMePGO+MTi9mF85qyFCW\r\n"
    "UFc7T4JfAkEA1dSx3p52azf0ZcbRRAlSZY2Qajp/LepQqMWEosA6RLtrYAfFatp6\r\n"
    "n6r2UE1C/Tx2fDnQTnACTMirgpXpBQAxnQJAfqkRGiNGXjYMX1w/GVGqBH5JG7Kr\r\n"
    "ihjsz6hH8lXAMSwfWdbVKuMoqk7MvfDItxZeNq9nixLpAdmR36J7qvP3swJAaode\r\n"
    "XfRANsZXoeTdGLI0NJfEXMfr2Gol6Q8ONy7hWKCuq9bzSXqzIJzBvdvgBd17ggQj\r\n"
    "fRBcdPh8ScB5qJ4iGQJBAOkU0lZF9N6vevMQSr/1d9N9+G3AduVWsW70Dy5ixrOR\r\n"
    "gK1v5pqInF60CtVkE6+qKQJUkeGW6X3a3qXGy4zeDVY=\r\n"
    "-----END RSA PRIVATE KEY-----\r\n";

//#define TLSTEST

//#define EAP_TEST
//#define EAP_DBG

#ifdef EAP_DBG

unsigned char polarsslservercert[]=
{
    0x0b,0x00,0x06,0xcf,0x00,0x06,0xcc,0x00,0x03,0x3b,0x30,0x82,0x03,0x37,0x30,0x82
    ,0x02,0x1f,0xa0,0x03,0x02,0x01,0x02,0x02,0x01,0x09,0x30,0x0d,0x06,0x09,0x2a,0x86
    ,0x48,0x86,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00,0x30,0x3b,0x31,0x0b,0x30,0x09,0x06
    ,0x03,0x55,0x04,0x06,0x13,0x02,0x4e,0x4c,0x31,0x11,0x30,0x0f,0x06,0x03,0x55,0x04
    ,0x0a,0x13,0x08,0x50,0x6f,0x6c,0x61,0x72,0x53,0x53,0x4c,0x31,0x19,0x30,0x17,0x06
    ,0x03,0x55,0x04,0x03,0x13,0x10,0x50,0x6f,0x6c,0x61,0x72,0x53,0x53,0x4c,0x20,0x54
    ,0x65,0x73,0x74,0x20,0x43,0x41,0x30,0x1e,0x17,0x0d,0x30,0x39,0x30,0x32,0x31,0x30
    ,0x32,0x32,0x31,0x35,0x31,0x32,0x5a,0x17,0x0d,0x31,0x31,0x30,0x32,0x31,0x30,0x32
    ,0x32,0x31,0x35,0x31,0x32,0x5a,0x30,0x34,0x31,0x0b,0x30,0x09,0x06,0x03,0x55,0x04
    ,0x06,0x13,0x02,0x4e,0x4c,0x31,0x11,0x30,0x0f,0x06,0x03,0x55,0x04,0x0a,0x13,0x08
    ,0x50,0x6f,0x6c,0x61,0x72,0x53,0x53,0x4c,0x31,0x12,0x30,0x10,0x06,0x03,0x55,0x04
    ,0x03,0x13,0x09,0x6c,0x6f,0x63,0x61,0x6c,0x68,0x6f,0x73,0x74,0x30,0x82,0x01,0x22
    ,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x01,0x05,0x00,0x03
    ,0x82,0x01,0x0f,0x00,0x30,0x82,0x01,0x0a,0x02,0x82,0x01,0x01,0x00,0xb0,0x19,0x1d
    ,0x43,0x4a,0xe1,0xf1,0x67,0x80,0x7b,0x44,0x3f,0x25,0xb9,0x10,0xf0,0xf1,0xac,0xaf
    ,0x59,0xfb,0x5c,0xe3,0xe7,0x32,0x49,0xf3,0xb7,0xa0,0xc1,0x90,0x27,0x83,0x04,0x2c
    ,0x0b,0x1b,0xf8,0x3d,0x1e,0xd8,0xc2,0x40,0x67,0x7a,0x4a,0xc0,0xcd,0xab,0x51,0x77
    ,0x34,0xee,0xae,0xac,0x09,0x6b,0x7a,0xcb,0x20,0x23,0xb3,0x44,0xb1,0x7e,0x78,0xa0
    ,0x95,0x50,0x59,0x36,0x97,0x04,0x57,0x9a,0x76,0x65,0xe0,0x08,0x7a,0x09,0x5e,0x61
    ,0x16,0x59,0xc2,0x35,0xeb,0xe0,0xa1,0xfd,0x92,0xf5,0xd5,0x76,0xc3,0x57,0xf3,0x64
    ,0x19,0x25,0xff,0xa9,0xe3,0x48,0x5a,0xc9,0xb7,0xad,0x77,0xc5,0x81,0x24,0x2d,0xc7
    ,0x99,0xd5,0xa5,0x15,0x12,0x67,0x69,0x00,0x2a,0xcd,0x4e,0x4f,0x46,0x40,0x51,0x78
    ,0x36,0xb5,0x26,0x15,0x9c,0x73,0x9c,0xdc,0xbd,0xf9,0xfe,0xac,0x62,0xdc,0x0a,0xc8
    ,0xa0,0x9e,0xaa,0x06,0xe9,0xe5,0x94,0xc2,0xbd,0x2f,0x46,0xd4,0x54,0x08,0xd7,0xd6
    ,0x98,0x69,0x1f,0xde,0x63,0xfc,0x09,0x70,0xf2,0x99,0xc8,0x63,0x27,0xf7,0x00,0x96
    ,0x1e,0xad,0xc3,0xee,0xce,0x80,0xe8,0x75,0xce,0x50,0x6b,0x6c,0x49,0xc8,0xc4,0x92
    ,0x04,0xa0,0x25,0x7c,0x19,0x6e,0xd6,0xe0,0x43,0x45,0x2a,0xd7,0x2a,0x44,0x4a,0x03
    ,0xb9,0x72,0x17,0xa4,0xc7,0x01,0xb9,0x4e,0x88,0x8c,0x82,0x63,0x0f,0xbb,0xc2,0x89
    ,0x98,0x86,0x8e,0x6d,0xd5,0x5e,0xbb,0x0b,0xbf,0x8c,0xd0,0x6f,0x97,0x15,0x39,0xfe
    ,0x11,0xc9,0xcb,0xde,0xc5,0x5b,0x2c,0x47,0x65,0x07,0x20,0xb6,0xcf,0x02,0x03,0x01
    ,0x00,0x01,0xa3,0x4d,0x30,0x4b,0x30,0x09,0x06,0x03,0x55,0x1d,0x13,0x04,0x02,0x30
    ,0x00,0x30,0x1d,0x06,0x03,0x55,0x1d,0x0e,0x04,0x16,0x04,0x14,0x92,0x0c,0x8b,0x3f
    ,0xe9,0xd3,0xee,0x6f,0x08,0x23,0x62,0x0b,0xd1,0x68,0xfd,0xac,0xa6,0x11,0x8f,0x5f
    ,0x30,0x1f,0x06,0x03,0x55,0x1d,0x23,0x04,0x18,0x30,0x16,0x80,0x14,0xcf,0x22,0x31
    ,0x27,0x91,0xd8,0xc2,0x54,0xff,0x1e,0xda,0xd9,0xee,0x8a,0xc5,0x89,0x32,0xad,0x0c
    ,0x21,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00
    ,0x03,0x82,0x01,0x01,0x00,0x69,0x51,0x68,0xd7,0x42,0xcc,0x0c,0xba,0x7d,0x28,0x6b
    ,0x0a,0x36,0x75,0x81,0xce,0x56,0xa7,0x9a,0x84,0x80,0xf7,0x69,0x14,0x33,0x56,0x1c
    ,0xf0,0xb4,0x7d,0xa1,0x37,0x53,0xf1,0x7b,0xec,0x4c,0x41,0x81,0xbe,0xd4,0x3c,0xed
    ,0x01,0x63,0x61,0xa4,0x78,0x2c,0xf9,0xc3,0x03,0xd2,0xab,0x07,0x5b,0x22,0xb6,0x7b
    ,0x63,0x6a,0xf9,0x24,0xf9,0x4f,0xd8,0x97,0x71,0x4d,0x82,0xf4,0x1a,0x59,0x20,0x2b
    ,0x9a,0xb1,0xcb,0x88,0x65,0xfa,0x93,0xbb,0xaa,0xf4,0xe7,0x50,0x31,0xd0,0xc8,0xb1
    ,0xe0,0xb8,0xcc,0xa6,0x5f,0x7e,0xff,0x54,0x25,0xde,0x89,0xdf,0x12,0xc5,0xfe,0x0f
    ,0x0e,0xc0,0x14,0xbb,0x3f,0x97,0xb7,0xb5,0x68,0xaf,0xab,0x05,0x73,0x6d,0x62,0xd0
    ,0xc4,0x2b,0xab,0x2d,0xc2,0xbc,0x2f,0xd3,0xbe,0x0b,0xe7,0x55,0x8a,0x25,0xc2,0xac
    ,0x1c,0xf6,0x40,0x88,0x8f,0x21,0x8c,0xbd,0x21,0xdb,0xb9,0x9f,0xb7,0xb3,0x44,0x5b
    ,0xcb,0x8c,0xcc,0xa5,0x08,0xf7,0xee,0xf9,0x1f,0x92,0xbd,0x0f,0xf5,0x2e,0xc1,0x73
    ,0x6c,0x98,0x7a,0x9d,0x4b,0x93,0x4e,0xb1,0xec,0x7e,0xb7,0x4b,0x7f,0xd3,0xc4,0x2c
    ,0x0d,0x01,0xfe,0xba,0x67,0x63,0x6a,0xa5,0xec,0x29,0xbd,0x00,0x3a,0x46,0xb5,0x43
    ,0x5b,0xf8,0x27,0x94,0xe5,0x7b,0xa2,0x80,0x9a,0x96,0x1b,0x7b,0x4b,0x73,0xfb,0x3b
    ,0xc6,0x22,0xdc,0x11,0x7e,0x27,0xa6,0x95,0xbe,0x3b,0x10,0xde,0xea,0x81,0x6d,0x3b
    ,0x71,0xdf,0x07,0x13,0x9c,0x2c,0x23,0xa6,0x27,0xd6,0x06,0xb9,0xf8,0xc7,0x42,0x93
    ,0x5a,0x92,0x0f,0x32,0x97,0x00,0x03,0x8b,0x30,0x82,0x03,0x87,0x30,0x82,0x02,0x6f
    ,0xa0,0x03,0x02,0x01,0x02,0x02,0x01,0x00,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86
    ,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00,0x30,0x3b,0x31,0x0b,0x30,0x09,0x06,0x03,0x55
    ,0x04,0x06,0x13,0x02,0x4e,0x4c,0x31,0x11,0x30,0x0f,0x06,0x03,0x55,0x04,0x0a,0x13
    ,0x08,0x50,0x6f,0x6c,0x61,0x72,0x53,0x53,0x4c,0x31,0x19,0x30,0x17,0x06,0x03,0x55
    ,0x04,0x03,0x13,0x10,0x50,0x6f,0x6c,0x61,0x72,0x53,0x53,0x4c,0x20,0x54,0x65,0x73
    ,0x74,0x20,0x43,0x41,0x30,0x1e,0x17,0x0d,0x30,0x39,0x30,0x32,0x30,0x39,0x32,0x31
    ,0x31,0x32,0x32,0x35,0x5a,0x17,0x0d,0x31,0x39,0x30,0x32,0x31,0x30,0x32,0x31,0x31
    ,0x32,0x32,0x35,0x5a,0x30,0x3b,0x31,0x0b,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13
    ,0x02,0x4e,0x4c,0x31,0x11,0x30,0x0f,0x06,0x03,0x55,0x04,0x0a,0x13,0x08,0x50,0x6f
    ,0x6c,0x61,0x72,0x53,0x53,0x4c,0x31,0x19,0x30,0x17,0x06,0x03,0x55,0x04,0x03,0x13
    ,0x10,0x50,0x6f,0x6c,0x61,0x72,0x53,0x53,0x4c,0x20,0x54,0x65,0x73,0x74,0x20,0x43
    ,0x41,0x30,0x82,0x01,0x22,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01
    ,0x01,0x01,0x05,0x00,0x03,0x82,0x01,0x0f,0x00,0x30,0x82,0x01,0x0a,0x02,0x82,0x01
    ,0x01,0x00,0xb0,0xc7,0x44,0x7a,0x99,0x90,0xef,0x25,0xb5,0xdc,0x0d,0x9f,0x95,0x14
    ,0x1f,0xb1,0xa6,0x77,0xb0,0xb9,0x9f,0xd7,0xa9,0xfe,0xb6,0x68,0x98,0xe5,0x50,0x4d
    ,0x33,0x9e,0xa1,0xd3,0xbf,0x1e,0xfb,0x71,0xb3,0xe0,0x35,0xaa,0x79,0xe0,0x0f,0xd0
    ,0x6f,0x27,0x3a,0x8c,0xb3,0x2b,0x01,0x69,0xf4,0x98,0x26,0x47,0xb7,0xba,0x40,0x30
    ,0xd6,0x15,0x2f,0x0c,0xe4,0x9a,0xbf,0x3b,0x1e,0xe0,0x97,0xb6,0xae,0x99,0xd3,0xa2
    ,0x89,0x05,0xe6,0x82,0x3f,0x5b,0xed,0x8d,0x3d,0xba,0xce,0x8b,0xa6,0xf9,0xe7,0x0d
    ,0x8a,0x89,0x2d,0x0f,0x07,0x03,0x52,0x40,0x6b,0xac,0xfa,0x21,0x9c,0x28,0xf2,0xe0
    ,0x63,0x6e,0xdd,0x45,0x68,0xc6,0x37,0x9c,0x75,0xbc,0x78,0x74,0x9c,0xe1,0xf3,0xea
    ,0xb4,0x2a,0xd5,0xa6,0xf8,0xa3,0xe6,0xa2,0xbe,0x4c,0x5a,0x31,0xb2,0xc0,0x1d,0x12
    ,0x80,0xfb,0x33,0xbe,0x2a,0x8c,0xa8,0x09,0xc1,0x05,0x0e,0x0b,0x71,0x2e,0xfb,0x7b
    ,0xae,0xd2,0xbc,0xc0,0x5b,0xf5,0x3a,0x59,0xd3,0xa2,0x3a,0xd6,0xf4,0x9f,0xe5,0x55
    ,0xc4,0x37,0x0f,0x49,0x45,0x3d,0xaa,0x88,0x6a,0x7b,0xb1,0xb0,0x33,0x82,0x0f,0x07
    ,0x17,0x94,0x56,0xaf,0xaf,0xe1,0x5f,0x1f,0xd2,0xc0,0x3f,0xf5,0x4d,0x16,0xe1,0xb6
    ,0x99,0x28,0x46,0x43,0x67,0xe0,0x5f,0x63,0xa2,0x44,0xc1,0x26,0x84,0x01,0x89,0x73
    ,0xc8,0x53,0xa0,0x24,0xda,0x20,0x97,0x08,0x09,0x8e,0xd5,0x77,0x43,0x1e,0x3a,0x6d
    ,0x92,0x40,0x55,0x87,0x12,0x5c,0x8d,0x79,0x4e,0xf0,0x8d,0xa9,0x65,0xd1,0x9e,0x60
    ,0xe7,0xeb,0x02,0x03,0x01,0x00,0x01,0xa3,0x81,0x95,0x30,0x81,0x92,0x30,0x0c,0x06
    ,0x03,0x55,0x1d,0x13,0x04,0x05,0x30,0x03,0x01,0x01,0xff,0x30,0x1d,0x06,0x03,0x55
    ,0x1d,0x0e,0x04,0x16,0x04,0x14,0xcf,0x22,0x31,0x27,0x91,0xd8,0xc2,0x54,0xff,0x1e
    ,0xda,0xd9,0xee,0x8a,0xc5,0x89,0x32,0xad,0x0c,0x21,0x30,0x63,0x06,0x03,0x55,0x1d
    ,0x23,0x04,0x5c,0x30,0x5a,0x80,0x14,0xcf,0x22,0x31,0x27,0x91,0xd8,0xc2,0x54,0xff
    ,0x1e,0xda,0xd9,0xee,0x8a,0xc5,0x89,0x32,0xad,0x0c,0x21,0xa1,0x3f,0xa4,0x3d,0x30
    ,0x3b,0x31,0x0b,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x4e,0x4c,0x31,0x11
    ,0x30,0x0f,0x06,0x03,0x55,0x04,0x0a,0x13,0x08,0x50,0x6f,0x6c,0x61,0x72,0x53,0x53
    ,0x4c,0x31,0x19,0x30,0x17,0x06,0x03,0x55,0x04,0x03,0x13,0x10,0x50,0x6f,0x6c,0x61
    ,0x72,0x53,0x53,0x4c,0x20,0x54,0x65,0x73,0x74,0x20,0x43,0x41,0x82,0x01,0x00,0x30
    ,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00,0x03,0x82
    ,0x01,0x01,0x00,0x1c,0x15,0x97,0x39,0x41,0x00,0x07,0xa3,0x07,0x3a,0x30,0x82,0xcb
    ,0x9e,0x01,0xc8,0x09,0xf3,0xe9,0x5f,0x40,0xfc,0x31,0xf5,0x81,0x98,0x6c,0x21,0xa5
    ,0x88,0x59,0x5f,0x98,0x5c,0x4d,0x9c,0x6c,0x4e,0xf5,0x0b,0x9b,0xc4,0x04,0xe1,0x16
    ,0x6f,0x59,0x08,0x4d,0x57,0xfe,0x0e,0x4f,0x53,0xc2,0x10,0x6c,0xd0,0x0d,0x1d,0xe7
    ,0xb9,0x84,0x79,0x1f,0x94,0x94,0xa9,0x84,0x83,0xf8,0xc9,0x2f,0x1c,0x4f,0xdf,0x92
    ,0x19,0x1e,0x66,0x10,0x8e,0x37,0xd5,0x7c,0x14,0xef,0xd9,0xc5,0xc8,0xc8,0xb2,0xc1
    ,0x1e,0xb7,0xea,0xb6,0xca,0xba,0x68,0xc4,0x63,0x72,0xe9,0xea,0x3b,0x96,0x1f,0x10
    ,0x27,0x1a,0x2d,0x52,0x0f,0x68,0x7b,0xea,0x80,0x05,0xef,0x7d,0xb6,0xf7,0x50,0xe8
    ,0x84,0xf3,0x57,0x38,0x0d,0x6a,0x59,0x98,0xc8,0xfb,0xc9,0x56,0x1f,0xa1,0xfa,0xf9
    ,0x94,0x29,0x80,0x4c,0x97,0x00,0x8d,0xad,0x40,0x61,0x68,0xce,0xcd,0x7d,0x63,0xec
    ,0xe3,0xd0,0x18,0x5b,0xac,0x95,0xf8,0x95,0xa8,0x24,0xf2,0x50,0xf2,0xf2,0x8d,0x57
    ,0xe8,0xe3,0xaa,0x86,0x8c,0xfd,0x8a,0x9e,0x5c,0x02,0x7b,0x85,0x9f,0x37,0x87,0x70
    ,0x75,0xb4,0x14,0x3d,0x1e,0x76,0x49,0x0f,0xef,0x6a,0xef,0x95,0xa3,0xac,0xaf,0x51
    ,0xac,0x60,0x4c,0x69,0xcf,0xaa,0x13,0x3f,0xa1,0x7d,0xd8,0x9f,0x9a,0x7e,0x35,0x9c
    ,0xb5,0x69,0x70,0x68,0x69,0xcd,0x4a,0x28,0x4b,0xb7,0x8f,0x31,0xee,0x07,0xd7,0x92
    ,0xf5,0x54,0x5d,0xb5,0xc9,0x78,0xe1,0xa6,0xe6,0x15,0x37,0xf7,0xdd,0x3d,0x38,0x47
    ,0x44,0xb1,0xe2
};
unsigned char clientrandom[]=
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1
};

unsigned char serverrandom[]=
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2
};

unsigned char sessionid[]=
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1
};

unsigned char premasterg[]=
{
    0x03,0x01,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1
};
#endif

void initEAPSettings()
{
    certreq = 1;
    dpconf->eap = 1;
    dpconf->eaptls = (EAPTLS*)malloc(sizeof(EAPTLS));
    memset(dpconf->eaptls, 0, sizeof(EAPTLS));
    dpconf->eaptls->eap = EAP_INIT;
    dpconf->eaptls->identity = (char*)malloc(100);
    strcpy(dpconf->eaptls->identity, "User1");
    dpconf->eaptls->password = (char*)malloc(20);
    strcpy(dpconf->eaptls->password, "Pass123");
    dpconf->eappro = EAP_TYPE_PEAP; //peap protocol

}
void EAPTask(void *data)
{
    unsigned char mar[MAC_ADDR_LEN] = {0};
    mar[0] = 0x01;
    mar[1] = 0x80;
    mar[2] = 0xc2;
    mar[3] = 0x00;
    mar[4] = 0x00;
    mar[5] = 0x03;
    addMulticasAddress (mar);
    setMulticastList();
    do{
        initEAPSettings();
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
        bsp_bits_set(IO_HWFunCtr, 1, BIT_AAB, 1);
#endif
        do
        {
            if (dpconf->eap == 0)
                break;
            eapol();
            dpconf->eaptls->eap = EAP_EAPOL;
#ifdef EAP_TEST
            OSTimeDlyHMSM( 20, 0, 0, 0 ); //wait for 2 secs
#else
            OSTimeDlyHMSM( 0, 0, 30, 0 ); //wait for 3 mins
#endif
        }while (1);
        eapfree();
        OSTaskSuspend(OS_PRIO_SELF);
    }while (1);
}

void eapol()
{
    unsigned char eapolpkt[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x01, 0x00, 0x00};
    unsigned char *pkt = (unsigned char*)malloc(sizeof(eapolpkt));
    memcpy(pkt, eapolpkt, sizeof(eapolpkt));
    memcpy(pkt + 6, dpconf->srcMacAddr[intf], 6);
#if WIN
    DMWGWriteProc(pkt, sizeof(eapolpkt));
    free(pkt);
    pkt = NULL;
#else
    gmacsend_F(((char*)pkt) , sizeof(eapolpkt) , OPT_FREE | OPT_NOCS, pkt);
#endif
}

void eapinit()
{
    md5_starts( &dpconf->eaptls->fin_md5  );
    sha1_startsH( &dpconf->eaptls->fin_sha1 );
}

void eapfree()
{
    if (dpconf->eaptls != NULL)
    {
        if (dpconf->eaptls->keymaterial != NULL)
        {
            free(dpconf->eaptls->keymaterial);
            dpconf->eaptls->keymaterial = NULL;
        }
        if (myvars->keyingMaterial != NULL)
        {
            free(myvars->keyingMaterial);
            myvars->keyingMaterial = NULL;
        }
        if (myvars)
        {
            free(myvars);
            myvars = NULL;
        }
        free(dpconf->eaptls->identity);
        dpconf->eaptls->identity = NULL;
        free(dpconf->eaptls->password);
        dpconf->eaptls->password = NULL;
        x509_free(dpconf->eaptls->server_cert);
        memset( dpconf->eaptls->server_cert, 0, sizeof( x509_cert ) );
        free(dpconf->eaptls->server_cert);
        dpconf->eaptls->server_cert = NULL;
        if (dpconf->eaptls != NULL)
        {
            free(dpconf->eaptls);
            dpconf->eaptls = NULL;
        }
    }
}

void recvEAPReq(EAPPKT **pkt, int len)
{
    if (dpconf->eap == 0)
        return;
    (*pkt)->auth.len = ntohs((*pkt)->auth.len);
    if ((*pkt)->auth.len > 0) //eap exist
    {
        if ((*pkt)->code == 1) //request
        {
            (*pkt)->len2 = ntohs((*pkt)->len2);
            if ((*pkt)->len2 > 0)
            {
                if (dpconf->eaptls->eapid >= (*pkt)->id && !((*pkt)->id <= 1 && dpconf->eaptls->eapid >= 0xfe)) //duplication
                    return;
                if ((*pkt)->type2 == 1 && dpconf->eaptls->eap == EAP_EAPOL) //identity
                {
                    sendEAPRepId((*pkt), (*pkt)->type2);
                    dpconf->eaptls->eap = EAP_IDENTITY;
                }
                else if ((*pkt)->type2 == 1 && dpconf->eaptls->eap != EAP_EAPOL)
                    return;
                printf("recvEAPReq 2\n");
                if (dpconf->eaptls->eap >= EAP_IDENTITY)
                {
                    if ((*pkt)->type2 == EAP_TYPE_TLS || (*pkt)->type2 == EAP_TYPE_PEAP) //EAP-TLS
                    {
                        recvEAPReqTLS(pkt, (*pkt)->type2, len);
                    }
                }
            }
        }
        else if ((*pkt)->code == 3) //success
        {
            eapfree();
            dpconf->eap = 0;
#ifdef EAP_TEST
            initEAPSettings();
            eapol();
            dpconf->eap = 1;
            dpconf->eaptls->eap = EAP_EAPOL;
#endif
        }
        else if ((*pkt)->code == 4) //failure
        {
            //dpconf->eaptls->eap = 1;
        }

    }
}

void sendEAPRepId(EAPPKT *rxpkt, unsigned char type2)
{
    EAPPKT *p = NULL;
    unsigned char Nearest[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};
    //int tmp = 0;
    unsigned char *txpkt = (unsigned char*)malloc(EAP_LEN + strlen(dpconf->eaptls->identity)+1);

    p = (EAPPKT*)txpkt;
    p->auth.ethHdr.ethType = htons(0x888e);
    memcpy(p->auth.ethHdr.srcAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
    memcpy(p->auth.ethHdr.destAddr, Nearest, MAC_ADDR_LEN);
    p->auth.ver = 2;
    p->auth.type = 0; //eap pakcet
    p->auth.len = htons(1 + 1 + 2 + 1 + strlen(dpconf->eaptls->identity));
    p->code = 2; //response
    p->id = rxpkt->id;
    dpconf->eaptls->eapid = p->id;
    p->len2 = p->auth.len;
    p->type2 = type2;

    strcpy((char*)txpkt + EAP_LEN, dpconf->eaptls->identity);
#if WIN
    DMWGWriteProc(txpkt + 2, EAP_LEN + strlen(dpconf->eaptls->identity) - 2);
    free(txpkt);
    txpkt = NULL;
#else
    gmacsend_F(((char*)txpkt) + 2 , EAP_LEN + strlen(dpconf->eaptls->identity) - 2 , OPT_FREE | OPT_NOCS, txpkt);
#endif
}

int recvEAPReqTLS(EAPPKT **rxpkt, unsigned char type2, int len)
{
    if (dpconf->eaptls->eap == 3)
    {
        eapinit();
        sendEAPClientHello(*rxpkt, type2);
        dpconf->eaptls->eap = 4;
    }
    else if (dpconf->eaptls->eap >= 4)
    {
        if (doEAPClientHandshake(rxpkt, len) < 0)
            return -1;
    }
    return 0;
}

void sendEAPClientHello(EAPPKT *rxpkt, unsigned char type2)
{
    //unsigned int len = htonl(RECORD_LAYER_LEN + CLIENT_HELLO_LEN);
    unsigned char *tmp;
    EAPTLSPKT *p = NULL;
    unsigned char Nearest[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};
    unsigned char *txpkt = (unsigned char*)malloc(EAP_LEN + EAP_HDR_TLS_LEN + RECORD_LAYER_LEN + CLIENT_HELLO_LEN);
    p = (EAPTLSPKT*)txpkt;
    p->auth.ethHdr.ethType = htons(0x888e);
    memcpy(p->auth.ethHdr.srcAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
    memcpy(p->auth.ethHdr.destAddr, Nearest, MAC_ADDR_LEN);
    p->auth.ver = 2;
    p->auth.type = 0; //eap pakcet
    p->auth.len = htons(EAP_HDR_LEN + EAP_HDR_TLS_LEN + RECORD_LAYER_LEN + CLIENT_HELLO_LEN -4);
    p->code = 2; //response
    p->id = rxpkt->id;
    dpconf->eaptls->eapid = p->id;
    p->len2 = p->auth.len;
    p->type2 = type2;
    p->flags = 0x00; //Length included

    //memcpy((char*)(&p->flags + 1), &len, 4);
    //p->len3 = htonl(RECORD_LAYER_LEN + CLIENT_HELLO_LEN);
    tmp = txpkt + EAP_LEN + EAP_HDR_TLS_LEN-4;

    addEAPTLSRecord(tmp, 0x16, CLIENT_HELLO_LEN);

    tmp+=5;
    addEAPClientHello(tmp);
#if WIN
    DMWGWriteProc(txpkt + 2, EAP_LEN + EAP_HDR_TLS_LEN + RECORD_LAYER_LEN + CLIENT_HELLO_LEN -4 - 2 );
    free(txpkt);
    txpkt = NULL;
#else
    gmacsend_F(((char*)txpkt) + 2, EAP_LEN + EAP_HDR_TLS_LEN + RECORD_LAYER_LEN + CLIENT_HELLO_LEN - 4 - 2, OPT_FREE | OPT_NOCS, txpkt);
#endif
}

void addEAPTLSRecord(unsigned char *p, unsigned char type, unsigned short len)
{
    p[0] = type;
    p[1] = 0x03;
    p[2] = 0x01;
    len = htons(len);
    memcpy(p+3, &len, 2);
}

void addEAPClientHello(unsigned char *p)
{
    unsigned char *tmp = p;
    int i = 0;
    time_t t;

    *p++ = 1; //client hello
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = CLIENT_HELLO_LEN - 1/*handshake type field len*/ - 3 /*handshake len field len*/;
    *p++ = 0x03;
    *p++ = 0x01;
#ifndef EAP_DBG
    t = time( NULL );
    p[0] = (unsigned char)( t >> 24 );
    p[1] = (unsigned char)( t >> 16 );
    p[2] = (unsigned char)( t >>  8 );
    p[3] = (unsigned char)( t       );


    for (i = 0;i<28;i++)
    {
        p[i+4] = rand()% 256;
    }
    memcpy(dpconf->eaptls->clientRandom, p, 32);

#else
    for (i=0;i<32;i++)
        p[i] = clientrandom[i];
    memcpy(dpconf->eaptls->clientRandom, clientrandom, 32);
#endif
    p += 32;
    //session id
    *p++ = 0x0;
    //ciphersuite, AES
    *p++ = 0x0;
    *p++ = 0x02;
    *p++ = 0x00;
    *p++ = 0x2f;
    //no compression
    *p++ = 0x01;
    *p++ = 0x00;

    md5_updateH( &dpconf->eaptls->fin_md5 , tmp, p - tmp );
    sha1_updateH( &dpconf->eaptls->fin_sha1, tmp, p - tmp );
}

int doEAPClientHandshake(EAPPKT **rxpkt, int len)
{
    int tmplen1 = 0, tmplen2 = 0, choice = 0;
    unsigned char *tmp = NULL;
    EAPTLSPKT *tp = NULL, *p = (EAPTLSPKT*)(*rxpkt);

    tmplen1 = (*rxpkt)->len2;

    if (dpconf->eaptls->recvbuflen == 0 && (dpconf->eaptls->eap < 8 || dpconf->eaptls->eap > 9) && dpconf->eaptls->eap < 12)
    {
        tmp = (unsigned char*)(&p->flags) + 4 + 1;
        if (parseEAPRecordLayer(tmp) == -1)
            return -1;
    }
    else if (dpconf->eaptls->eap == 12)
    {
        tmp = (unsigned char*)(&p->flags) + 1;
        if (parseEAPRecordLayer(tmp) == -1)
            return -1;
    }

    if ((p->flags == 0xc0 || p->flags == 0x80)&& (dpconf->eaptls->eap < 8 || dpconf->eaptls->eap > 9) && dpconf->eaptls->eap < 12)
    {
        tmp += RECORD_LAYER_LEN;
        tmplen1 -= RECORD_LAYER_LEN;
        memcpy(&dpconf->eaptls->totallen, (char*)(&p->flags + 1), 4);
        dpconf->eaptls->totallen = ntohl(dpconf->eaptls->totallen);
        tmplen1 -= (EAP_HDR_LEN+EAP_HDR_TLS_LEN);
    }
    else if ((dpconf->eaptls->eap < 8 || dpconf->eaptls->eap > 9) && dpconf->eaptls->eap < 12)//0x40, 0x00
    {
        tmp = (unsigned char*)(&p->flags) + 1;
        tmplen1 -= (EAP_HDR_LEN + 1/*flag field length*/);
    }
    else if (dpconf->eaptls->eap == 12)
    {
        tmp = (unsigned char*)(&p->flags) + 1 + RECORD_LAYER_LEN;
        tmplen1 -= (EAP_HDR_LEN + 1/*flag field length*/ + RECORD_LAYER_LEN);
    }

    if (dpconf->eaptls->recvbuflen > 0) //fragmented TLS message
    {

        tp = (EAPTLSPKT*)malloc(sizeof(EAPTLSPKT) + dpconf->eaptls->recvbuflen + tmplen1);
        memcpy((char*)(&tp->flags) + 1, dpconf->eaptls->recvbuf, dpconf->eaptls->recvbuflen);
        free(dpconf->eaptls->recvbuf);
        dpconf->eaptls->recvbuf = NULL;
        memcpy((char*)(&tp->flags) + 1 + dpconf->eaptls->recvbuflen, (char*)tmp, tmplen1);

        tp->id = (*rxpkt)->id;
        tp->code = (*rxpkt)->code;
        tp->len2 = (*rxpkt)->len2;
        tp->type2 = (*rxpkt)->type2;
        tp->auth.len = (*rxpkt)->auth.len;
        tp->auth.type = (*rxpkt)->auth.type;
        free(*rxpkt);
        (*rxpkt) = (EAPPKT*)tp;
        p = (EAPTLSPKT*)(*rxpkt);
        tmp = (unsigned char*)(&p->flags) + 1;
        tmplen1 += dpconf->eaptls->recvbuflen;


        dpconf->eaptls->recvbuflen = 0;
    }

    choice = (int)dpconf->eaptls->eap;
    switch (choice)
    {
    case 4:
        if ((tmplen2=parseEAPServerHello(tmp, tmplen1)) < 0)
        {
            break;
        }
        tmp += tmplen2;
        tmplen1 -= tmplen2;
        dpconf->eaptls->eap = 5;

    case 5:
        if ((tmplen2=parseEAPServerCertificate(tmp, tmplen1)) < 0)
        {
            dpconf->eaptls->recvbuf = (unsigned char*)malloc(tmplen1);
            memcpy(dpconf->eaptls->recvbuf, tmp, tmplen1);
            dpconf->eaptls->recvbuflen = tmplen1;
            sendEAPRep(*rxpkt, dpconf->eappro);
            break;
        }
        tmp += tmplen2;
        tmplen1 -= tmplen2;
        dpconf->eaptls->eap = 6;

    case 6:
        if ((tmplen2 = parseEAPCertificateReq(tmp, tmplen1)) < 0)
        {
            if (certreq == 1)
            {
                dpconf->eaptls->recvbuf = (unsigned char*)malloc(tmplen1);
                memcpy(dpconf->eaptls->recvbuf, tmp, tmplen1);
                dpconf->eaptls->recvbuflen = tmplen1;
                sendEAPRep(*rxpkt, dpconf->eappro);
                break;
            }
        }
        if (certreq == 1)
        {
            tmp += tmplen2;
            tmplen1 -= tmplen2;
        }
        dpconf->eaptls->eap = 7;

    case 7:
        if ((tmplen2 = parseEAPServerHelloDone(tmp, tmplen1)) < 0)
        {
            dpconf->eaptls->recvbuf = (unsigned char*)malloc(tmplen1);
            memcpy(dpconf->eaptls->recvbuf, tmp, tmplen1);
            dpconf->eaptls->recvbuflen = tmplen1;
            sendEAPRep(*rxpkt, dpconf->eappro);
            break;
        }
        dpconf->eaptls->eap = 8;
    case 8:
    case 9:
        tmplen2 = (*rxpkt)->id;
        free(*rxpkt);
        *rxpkt = NULL;
        sendEAPCertificateSequence((unsigned char)tmplen2, dpconf->eappro);
        break;
    case 10:
        if ((tmplen2 = parseEAPChangeCipherSpec(tmp, tmplen1)) < 0)
        {
            dpconf->eaptls->recvbuf = (unsigned char*)malloc(tmplen1);
            memcpy(dpconf->eaptls->recvbuf, tmp, tmplen1);
            dpconf->eaptls->recvbuflen = tmplen1;
            free(tp);
            tp = NULL;
            sendEAPRep(*rxpkt, dpconf->eappro);
            break;
        }
        tmp += tmplen2;
        tmplen1 -= tmplen2;
        dpconf->eaptls->eap = 11;
    case 11:
        if (parseEAPRecordLayer(tmp) == -1)
            return -1;
        tmp += RECORD_LAYER_LEN;
        tmplen1 -= RECORD_LAYER_LEN;
        if (tmplen1 < dpconf->eaptls->rec.size)
        {
            dpconf->eaptls->recvbuf = (unsigned char*)malloc(tmplen1);
            memcpy(dpconf->eaptls->recvbuf, tmp, tmplen1);
            dpconf->eaptls->recvbuflen = tmplen1;
            sendEAPRep(*rxpkt, dpconf->eappro);
            break;
        }
        if ((tmplen2 = parseEAPServerFinish(tmp, tmplen1)) < 0)
        {
            //send alert message
            return -1;
        }
        sendEAPRep(*rxpkt, dpconf->eappro);
        dpconf->eaptls->eap = 12;
        break;
    case 12: //for peap
        parseTlsData(*rxpkt, tmp, tmplen1);
        break;
    }
    return 0;
}

void sendEAPRep(EAPPKT *rxpkt, unsigned char type2)
{
    //unsigned int len = htonl(RECORD_LAYER_LEN + CLIENT_HELLO_LEN);
    EAPTLSPKT *p = NULL;
    unsigned char Nearest[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};
    unsigned char *txpkt = (unsigned char*)malloc(EAP_LEN + 1/*flags field length*/);
    p = (EAPTLSPKT*)txpkt;
    p->auth.ethHdr.ethType = htons(0x888e);
    memcpy(p->auth.ethHdr.srcAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
    memcpy(p->auth.ethHdr.destAddr, Nearest, MAC_ADDR_LEN);
    p->auth.ver = 2;
    p->auth.type = 0; //eap pakcet
    p->auth.len = 0x0600;
    p->code = 2; //response
    p->id = rxpkt->id;
    dpconf->eaptls->eapid = p->id;
    p->len2 = 0x0600;
    p->type2 = type2;
    p->flags = 0;
#if WIN
    DMWGWriteProc(txpkt + 2, EAP_LEN + 1 - 2);
    free(txpkt);
    txpkt = NULL;
#else
    gmacsend_F(((char*)txpkt) + 2, EAP_LEN + 1 - 2, OPT_FREE | OPT_NOCS, txpkt);
#endif
}

int parseEAPServerHello(unsigned char *p, unsigned int recvlen)
{
    int msglen, len;
    unsigned char *tmp = p;

    if (p[0] != 0x02) //server hello
        return -1;

    msglen = (p[1]<<16) + (p[2]<<8) + p[3]; //server hello length
    if (recvlen < msglen + 4)
        return -1;

    p += 4;
    p += 2; //version

#ifndef EAP_DBG
    memcpy(dpconf->eaptls->serverRandom, p, 32);
#else
    memcpy(p, serverrandom, 32);
    memcpy(dpconf->eaptls->serverRandom, serverrandom, 32);
#endif
    p += 32;

    len = *p;
    p++;
#ifndef EAP_DBG
    memcpy(dpconf->eaptls->sessionID, p, len);
#else
    memcpy(p, sessionid, len);
    memcpy(dpconf->eaptls->sessionID, sessionid, len);
#endif
    p += len;

    //cipher suite
    switch (ntohs(*((unsigned short*)p)))
    {
    case 0x002f:
        dpconf->eaptls->cipher = ntohs(*((unsigned short*)p));
        break;
    case 0x0005:
        dpconf->eaptls->cipher = ntohs(*((unsigned short*)p));
        break;
    default:
        return -1;
    }

    md5_updateH( &dpconf->eaptls->fin_md5 , tmp, msglen + 4 );
    sha1_updateH( &dpconf->eaptls->fin_sha1, tmp, msglen + 4 );

    return msglen + 4;
}

int parseEAPRecordLayer(unsigned char *p)
{
    if (memcmp((p+1), TLS_VER_1_0, sizeof(TLS_VER_1_0)) == 0)
    {
        dpconf->eaptls->rec.ct = p[0];
        dpconf->eaptls->rec.size = (((unsigned short)p[3]<<8) + (unsigned short)p[4]);
        return 0;
    }

    return -1;
}

int parseEAPServerCertificate(unsigned char *p, unsigned int recvlen)
{
    int ret, msglen, certlen1, servercertlen, tmpmsglen;
    unsigned char *tmp = p;
    unsigned char *servercert;
    unsigned char rc = 0;
    if ((parseEAPRecordLayer(p) == 0 && p[5] == 0x0b) )
    {
        p+=5;
        rc = 1;
    }
#ifdef EAP_DBG
//   tmp = polarsslservercert;
#endif

    if (p[0] != 0x0b) //certificate
        return -1;

    msglen = (p[1]<<16) + (p[2]<<8) + p[3];
#ifdef EAP_DBG
    tmpmsglen = msglen;
#endif
    if (recvlen < msglen + 4)
        return -1;
#ifdef EAP_DBG
    p = tmp;
    msglen = (p[1]<<16) + (p[2]<<8) + p[3];
#endif

    p += 4;

    certlen1 = (p[0]<<16) + (p[1]<<8) + p[2];
    p += 3;

    servercertlen = (p[0]<<16) + (p[1]<<8) + p[2];
    p+=3;

    md5_updateH( &dpconf->eaptls->fin_md5 , tmp, msglen + 4 );
#ifndef EAP_DBG
    sha1_updateH( &dpconf->eaptls->fin_sha1, tmp, msglen + 4 );
#else
    msglen = 1747;
    sha1_updateH( &dpconf->eaptls->fin_sha1, tmp, msglen);
    msglen = tmpmsglen;
#endif

    servercert = (unsigned char*)malloc(servercertlen);
    memcpy(servercert, p, servercertlen);

    dpconf->eaptls->server_cert = (x509_cert *)malloc(sizeof(x509_cert));
    memset( dpconf->eaptls->server_cert, 0, sizeof( x509_cert ) );
    ret = x509parse_crt( dpconf->eaptls->server_cert, (unsigned char *) servercert,  servercertlen);
    if (ret != 0)
    {
        free(servercert);
        return ret;
    }
    free(servercert);

    if (rc)
        msglen += 5;

    return msglen + 4;
}

int parseEAPCertificateReq(unsigned char *p, unsigned int recvlen)
{
    int msglen = 0;
    unsigned char rc = 0;
    if ((parseEAPRecordLayer(p) == 0 && p[5] == 0x0e) )
    {
        p += 5;
        rc = 1;
    }
    if (p[0] == 0x0e)
    {
        certreq = 0;
        return -1;
    }
    else if (p[0] != 0x0d) //certificate request
        return -1;

    msglen = (p[1]<<16) + (p[2]<<8) + p[3];
    if (recvlen < msglen + 4)
        return -1;

    md5_updateH( &dpconf->eaptls->fin_md5 , p, msglen + 4 );
    sha1_updateH( &dpconf->eaptls->fin_sha1, p, msglen + 4 );

    if (rc)
        msglen += 5;
    return msglen+4;
}

int parseEAPServerHelloDone(unsigned char *p, unsigned int recvlen)
{
    int msglen;
    unsigned char rc = 0;
    if ((parseEAPRecordLayer(p) == 0 && p[5] == 0x0e) )
    {
        p += 5;
        rc = 1;
    }
    if (p[0] != 0x0e) //server hello done
        return -1;

    msglen = (p[1]<<16) + (p[2]<<8) + p[3];

    if (recvlen < msglen + 4)
        return -1;

    md5_updateH( &dpconf->eaptls->fin_md5 , p, msglen + 4 );
    sha1_updateH( &dpconf->eaptls->fin_sha1, p, msglen + 4 );

    if (rc)
        msglen += 5;
    return msglen + 4;
}

int parseEAPChangeCipherSpec(unsigned char *p, unsigned int recvlen)
{
    if (dpconf->eaptls->rec.ct != 0x14 || dpconf->eaptls->rec.size != 1 || p[0] != 1)
        return -1;

    if (recvlen < 1 + 4)
        return -1;

    return 1;
}

int parseEAPServerFinish(unsigned char *p, unsigned int recvlen)
{
    int len, i, padlen;
    md5_context  md5;
    sha1_context sha1;
    unsigned char *tmp = p;

    unsigned char tmp1[100] = {0};
    unsigned char tmpbuf[20];

    len = recvlen;

    memcpy( &md5 , &dpconf->eaptls->fin_md5 , sizeof(  md5_context ) );
    memcpy( &sha1, &dpconf->eaptls->fin_sha1, sizeof( sha1_context ) );

    switch ( dpconf->eaptls->ivlen )
    {
    case 16:
        aes_crypt_cbc( (aes_context *) dpconf->eaptls->ctx_dec, AES_DECRYPT, len,
                       (unsigned char *)(dpconf->eaptls->iv_dec), tmp, tmp);
        break;

    }

    padlen = 1 + tmp[len - 1];

    /*
     * TLSv1: always check the padding
     */
    for ( i = 1; i <= padlen; i++ )
    {
        if ( tmp[len - i] != padlen - 1 )
        {
            padlen = 0;
        }
    }

    /*
     * Always compute the MAC (RFC4346, CBCTIME).
     */
    len -= ( dpconf->eaptls->maclen + padlen );


    memcpy(tmpbuf, tmp + len, 20);
    memcpy(tmp1, dpconf->eaptls->in_ctr, 8);

    *(tmp - 5 + 3) = (unsigned char)( len >> 8 );
    *(tmp - 5 + 4) = (unsigned char)( len      );

    memcpy(tmp1 + 8, tmp - 5, 5 + 16);

    if ( dpconf->eaptls->maclen == 16 )
        md5_hmac( dpconf->eaptls->mac_dec, 16,
                  tmp1,  len + 13,
                  tmp + len );
    else
        sha1_hmac( dpconf->eaptls->mac_dec, 20,
                   tmp1,  len + 13,
                   tmp + len);

    if ( memcmp( tmpbuf, tmp + len, dpconf->eaptls->maclen ) != 0 )
    {
        return( -1 );
    }

    /*
     * Finally check the padding length; bad padding
     * will produce the same error as an invalid MAC.
     */
    if ( dpconf->eaptls->ivlen != 0 && padlen == 0 )
        return( -1 );

    for ( i = 7; i >= 0; i-- )
        if ( ++dpconf->eaptls->in_ctr[i] != 0 )
            break;

    return 0;
}

int parseTlsData(EAPPKT* rxpkt, unsigned char *p, unsigned int recvlen)
{
    int len, i, padlen;
    unsigned char *tmp = p;
    unsigned char tmpbuf[20];
    unsigned char id = rxpkt->id;

    len = recvlen;

    if ( recvlen < dpconf->eaptls->maclen )
    {
        return -1;
    }

    switch ( dpconf->eaptls->ivlen )
    {
    case 16:
        aes_crypt_cbc( (aes_context *) dpconf->eaptls->ctx_dec,
                       AES_DECRYPT, recvlen,
                       dpconf->eaptls->iv_dec, p, p );
        break;

    }
    padlen = 1 + tmp[len - 1];

    /*
     * TLSv1: always check the padding
     */
    for ( i = 1; i <= padlen; i++ )
    {
        if ( tmp[len - i] != padlen - 1 )
        {
            padlen = 0;
        }
    }

    /*
     * Always compute the MAC (RFC4346, CBCTIME).
     */
    len -= ( dpconf->eaptls->maclen + padlen );


    *(tmp - 5 + 3) = (unsigned char)( len >> 8 );
    *(tmp - 5 + 4) = (unsigned char)( len      );

    memcpy(tmpbuf, tmp + len, 20);
    memcpy(tmp - 13, dpconf->eaptls->in_ctr, 8);

    //memcpy(tmp1 + 8, tmp - 5, 5 + 16);

    if ( dpconf->eaptls->maclen == 16 )
        md5_hmac( dpconf->eaptls->mac_dec, 16,
                  tmp-13,  len + 13,
                  tmp + len );
    else
        sha1_hmac( dpconf->eaptls->mac_dec, 20,
                   tmp-13,  len + 13,
                   tmp + len);

    if ( memcmp( tmpbuf, tmp + len, dpconf->eaptls->maclen ) != 0 )
    {
        return( -1 );
    }

    /*
     * Finally check the padding length; bad padding
     * will produce the same error as an invalid MAC.
     */
    if ( dpconf->eaptls->ivlen != 0 && padlen == 0 )
        return( -1 );

    for ( i = 7; i >= 0; i-- )
        if ( ++dpconf->eaptls->in_ctr[i] != 0 )
            break;

    if (dpconf->eappro == EAP_TYPE_PEAP)
    {
        do_peap_version0(p, len, id);
    }


    return 0;
}

void peap_unpad_frame(unsigned char *in, int in_size, unsigned char *out, int *out_size)
{
    int i;

    *out_size = in_size - 4;

    for (i=0;i<=*out_size;i++)
    {
        out[i] = in[4+i];
    }
}

void peap_pad_frame(unsigned char *in, int in_size, unsigned char *out, int *out_size)
{
    int i;

    *out_size = in_size + 4;

    memset(out, 0, *out_size);
    for (i=0;i<=in_size;i++)
    {
        out[4+i] = in[i];
    }
}

void do_peap_version0(unsigned char *p, int len, unsigned char id)
{
    char *padded_frame;
    int padded_size, eframe = 0;

    padded_size = len;
    padded_frame = (char *)malloc(len+19);  // It is 19 bytes to pad out.
    memset(padded_frame, 0, len + 19);

    if ((p[4] == 0x21) && (p[5] = 0x80))
    {
        eframe = 1;
        memcpy(padded_frame, p, len);
    }
    if (eframe != 1)
    {
        peap_pad_frame(p, len, (unsigned char *)padded_frame, &padded_size);
    }

    do_peap_version1((unsigned char*)padded_frame, padded_size, id, eframe);

    free(padded_frame);

    eframe = 0;
}
void eapmschapv2_free()
{
    free(myvars->AuthenticatorChallenge);
    myvars->AuthenticatorChallenge = NULL;
    free(myvars->PeerChallenge);
    myvars->PeerChallenge = NULL;
    free(myvars->NtResponse);
    myvars->NtResponse = NULL;
}

void do_peap_version1(unsigned char *p, int len, unsigned char id, unsigned char eframe)
{
    unsigned char *sendbuf;
    unsigned char *databuf = NULL;
    int eapvalue;
    unsigned short out_size = 0;
    MSCHV2_CHAN *challenge = NULL;

    eapvalue = p[4];

    switch (eapvalue)
    {
    case EAP_REQUEST:
        len = strlen(dpconf->eaptls->identity)+5;
        sendbuf = (unsigned char*)malloc(len + 300);

        // In version 1, we answer with an EAP header.
        sendbuf[0] = EAP_RESPONSE;
        sendbuf[1] = p[1];  // Use the same ID #

        out_size = htons(len);
        memcpy((char *)&sendbuf[2], &out_size, 2);  // The length of the username + header.
        sendbuf[4] = EAP_TYPE_ID;
        memcpy(&sendbuf[5], dpconf->eaptls->identity, strlen(dpconf->eaptls->identity)+1);
        len = strlen(dpconf->eaptls->identity)+5;
        databuf = (unsigned char*)malloc(len);
        break;

    case EAP_SUCCESS:
        break;

    case EAP_TYPE_MSCHAP:
        challenge = (MSCHV2_CHAN *)(p+5);
        //printf("challenge->OpCode: %d\n", challenge->OpCode);
        switch (challenge->OpCode)
        {
        case MS_CHAPV2_CHALLENGE:
            myvars = (MSCHV2_VARS *)malloc(sizeof(MSCHV2_VARS));
            eapmschapv2_challenge(challenge, myvars);
            len = 54 + strlen(dpconf->eaptls->identity) + sizeof(struct eap_header);
            sendbuf = (unsigned char*)malloc(len + 300);
            eapmschapv2_challenge_resp(challenge, myvars, id, sendbuf);
            databuf = (unsigned char*)malloc(len);
            break;
        case MS_CHAPV2_RESPONSE:
            break;
        case MS_CHAPV2_SUCCESS:
            if (eapmschapv2_success(&p[5], myvars) != 0)
                return;
            sendbuf = (unsigned char*)malloc(256);
            //printf("MS_CHAPV2_SUCCESS:1\n");
            eapmschapv2_success_resp(myvars, id, sendbuf);
            eapmschapv2_free();
            len = sizeof(struct eap_header) + 1;
            //printf("MS_CHAPV2_SUCCESS:2\n");
            databuf = (unsigned char*)malloc(len);
            break;
        case MS_CHAPV2_FAILURE:
            printf("chapv2 failure\n");
            return;
        default:
            printf("not expected\n");
            return;
        }
        break;

    case PEAP_EAP_EXTENSION: // EAP Extension
        sendbuf = (unsigned char *)malloc(512); //guess output len = input len?
        peap_extensions_process(p, len, sendbuf, &out_size, myvars);
        databuf = (unsigned char*)malloc(out_size);
        break;
    case EAP_TYPE_EXPANDED:
        sendbuf = (unsigned char *)malloc(512); //guess output len = input len?
        peap_soh_extensions_process(p, len, sendbuf, &out_size, id);
        len = out_size;
        databuf = (unsigned char*)malloc(out_size);
        break;
    default:
        printf("exception\n");
        return;
    }

    if (eframe != 1)
    {
        peap_unpad_frame(sendbuf, len, databuf, &len);
    }
    else
    {
        memcpy(databuf, sendbuf, out_size);
        len = out_size;
    }

    sendTlsData(&sendbuf, databuf, len, id);

    //if(eapvalue == EAP_TYPE_MSCHAP)
    //	printf("MS_CHAPV2_SUCCESS\n");

    free(sendbuf);
    free(databuf);
}

void adddata(unsigned char **buf, unsigned int value, unsigned char digit, unsigned short *len)
{
    unsigned char tmp8 = 0;
    unsigned short tmp16 = 0;

    if (digit == 1)
    {
        tmp8 = value;
        memcpy(*buf, &tmp8, 1);
        (*buf)++;
        if (len != 0)
            (*len)++;
    }
    else if (digit == 2)
    {
        tmp16 = value;
        tmp16 = ntohs(tmp16);
        memcpy(*buf, &tmp16, 2);
        (*buf)+=2;
        if (len != 0)
            (*len)+=2;
    }
    else if (digit == 4)
    {
        value = ntohl(value);
        memcpy(*buf, &value, 4);
        (*buf)+=4;
        if (len != 0)
            (*len)+=4;
    }
}

void gensoh(unsigned char *out, int ver, int id, unsigned short *out_size)
{
    unsigned char *dataptr = out;
    unsigned short len = 0;
    int tmp = 0;
    unsigned char *tlv_len, *tlv_len2, *outer_len, *inner_len, *ssoh_len, *end;
    unsigned char correlation_id[24];
    //char *machinename = (char*)dpconf->HostName;
    char *machinename = "CLIENT2.nap.com";
    unsigned short * resultsize = NULL;
    //unsigned char productname[] ={0x4d, 0x00, 0x49, 0x00, 0x43, 0x00, 0x52, 0x00, 0x4f, 0x00, 0x53, 0x00, 0x4f, 0x00,
    //0x46, 0x00, 0x54, 0x00, 0x20, 0x00, 0x50, 0x00, 0x52, 0x00, 0x4f, 0x00, 0x44, 0x00, 0x55, 0x00, 0x43, 0x00,
    //0x54, 0x00, 0x00, 0x00};

    for (;tmp<24;tmp++)
        correlation_id[tmp] = rand() % 256;

    dataptr[0] = EAP_RESPONSE_PKT;
    dataptr[1] = id;	// EAP ID
    resultsize = (unsigned short *) & dataptr[2];

    dataptr[4] = EAP_TYPE_EXPANDED;
    tmp = ntohl(EAP_VENDOR_MICROSOFT);
    memcpy(dataptr + 5, ((char*)(&tmp))+1, 3);
    tmp = ntohl(0x21);
    memcpy(dataptr + 8, &tmp, 4);
    dataptr+=12;

    /* Vendor-Specific TLV (Microsoft) - SoH */
    adddata(&dataptr, EAP_TLV_VENDOR_SPECIFIC_TLV, 2, &len);/* TLV Type */
    //adddata(&dataptr, len, 2, &len); /* Length */ //tlv_len
    tlv_len = dataptr;
    len+=2;
    dataptr+=2;
    adddata(&dataptr, EAP_VENDOR_MICROSOFT, 4, &len); /* Vendor_Id */
    adddata(&dataptr, 0x01, 2, &len); /* TLV Type - SoH TLV */
    //adddata(&dataptr, len, 2, &len); /* Length */ //tlv_len2
    tlv_len2 = dataptr;
    len+=2;
    dataptr+=2;

    /* SoH Header */
    adddata(&dataptr, EAP_TLV_VENDOR_SPECIFIC_TLV, 2, &len);/* Outer Type */
    //adddata(&dataptr, len, 2, &len); //outer_len
    outer_len = dataptr;
    len+=2;
    dataptr+=2;
    adddata(&dataptr, EAP_VENDOR_MICROSOFT, 4, &len); /* IANA SMI Code */
    adddata(&dataptr, ver, 2, &len); /* Inner Type */
    //adddata(&dataptr, len, 2, &len); /* Length */ //inner_len
    inner_len = dataptr;
    len+=2;
    dataptr+=2;

    if (ver == 2)
    {
        /* SoH Mode Sub-Header */
        /* Outer Type */
        adddata(&dataptr, EAP_TLV_VENDOR_SPECIFIC_TLV, 2, &len);
        adddata(&dataptr, 4 + 24 + 1 + 1, 2, &len); /* Length */
        adddata(&dataptr, EAP_VENDOR_MICROSOFT, 4, &len); /* IANA SMI Code */

        /* Value: */
        memcpy(dataptr, correlation_id, sizeof(correlation_id));
        dataptr+=sizeof(correlation_id);
        len += sizeof(correlation_id);

        adddata(&dataptr, 0x01, 1, &len); /* Intent Flag - Request */
        adddata(&dataptr, 0x00, 1, &len); /* Content-Type Flag */
    }

    /* SSoH TLV */
    /* System-Health-Id */
    adddata(&dataptr, 0x0002, 2, &len); /* Type */
    adddata(&dataptr, 4, 2, &len); /* Length */
    adddata(&dataptr, 79616, 4, &len);

    /* Vendor-Specific Attribute */
    adddata(&dataptr, EAP_TLV_VENDOR_SPECIFIC_TLV, 2, &len);
    //adddata(&dataptr, len, 2, &len); /* Length */ //ssoh_len
    ssoh_len = dataptr;
    len+=2;
    dataptr+=2;
    adddata(&dataptr, EAP_VENDOR_MICROSOFT, 4, &len); /* IANA SMI Code */

    /* MS-Packet-Info */
    adddata(&dataptr, SSOH_MS_PACKET_INFO, 1, &len);
    /* Note: IF-TNCCS-SOH v1.0 r8 claims this field to be:
     * Reserved(4 bits) r(1 bit) Vers(3 bits), but Windows XP
     * SP3 seems to be sending 0x11 for SSoH, i.e., r(request/response) bit
     * would not be in the specified location.
     * [MS-SOH] 4.0.2: Reserved(3 bits) r(1 bit) Vers(4 bits)
     */
    adddata(&dataptr, 0x11, 1, &len); /* r=request, vers=1 */

    /* MS-Machine-Inventory */
    /* TODO: get correct values; 0 = not applicable for OS */
    adddata(&dataptr, SSOH_MS_MACHINE_INVENTORY, 1, &len);

    adddata(&dataptr, 0x00000006, 4, &len); /* osVersionMajor */
    adddata(&dataptr, 0x00000001, 4, &len); /* osVersionMinor */
    adddata(&dataptr, 0x1db0, 4, &len); /* osVersionBuild */
    adddata(&dataptr, 0, 2, &len); /* spVersionMajor */
    adddata(&dataptr, 0, 2, &len); /* spVersionMinor */
    adddata(&dataptr, 0, 2, &len); /* procArch */

    /* MS-MachineName */
    adddata(&dataptr, SSOH_MS_MACHINENAME, 1, &len);
    adddata(&dataptr, strlen(machinename)+1, 2, &len);

    memcpy(dataptr, machinename, strlen(machinename)+1);
    dataptr+=(strlen(machinename)+1);
    len += (strlen(machinename)+1);

    /* MS-CorrelationId */
    adddata(&dataptr, SSOH_MS_CORRELATIONID, 1, &len);
    memcpy(dataptr, correlation_id, sizeof(correlation_id));
    dataptr+=sizeof(correlation_id);
    len+=sizeof(correlation_id);
    //adddata(&dataptr, SSOH_MS_CORRELATIONID, 1, &len);

    /* MS-Quarantine-State */
    adddata(&dataptr, SSOH_MS_QUARANTINE_STATE, 1, &len);
    adddata(&dataptr, 9, 2, &len); /* Flags: ExtState=0, f=0, qState=1 */
    adddata(&dataptr, 0xffffffff, 4, &len); /* ProbTime (hi) */
    adddata(&dataptr, 0xffffffff, 4, &len); /* ProbTime (lo) */
    adddata(&dataptr, 1, 2, &len); /* urlLenInBytes */
    adddata(&dataptr, 0, 1, &len); /* null termination for the url */

    /* MS-Machine-Inventory-Ex */
    adddata(&dataptr, SSOH_MS_MACHINE_INVENTORY_EX, 1, &len);
    adddata(&dataptr, 0xdecafbad, 4, &len); /* Reserved (note: Windows XP SP3 uses 0xdecafbad, windows 7 using nap sample sdk use this too) */
    adddata(&dataptr, 1, 1, &len); /* ProductType: Client */

#if 0
    /* System-Health-Id */
    adddata(&dataptr, 0x0002, 2, &len); /* Type */
    adddata(&dataptr, 4, 2, &len); /* Length */
    adddata(&dataptr, 0x000137f0, 4, &len);

    /* Vendor-Specific Attribute */
    adddata(&dataptr, EAP_TLV_VENDOR_SPECIFIC_TLV, 2, &len);
    adddata(&dataptr, 6, 2, &len); /* Length */ //ssoh_len
    adddata(&dataptr, 0x000137f0, 4, &len); /* IANA SMI Code */
    adddata(&dataptr, 0xaa, 1, &len);
    adddata(&dataptr, 0xaa, 1, &len);

    /* System-Health-Id */
    adddata(&dataptr, 0x0002, 2, &len); /* Type */
    adddata(&dataptr, 4, 2, &len); /* Length */
    adddata(&dataptr, 0x00013780, 4, &len);

    /* Vendor-Specific Attribute */
    adddata(&dataptr, EAP_TLV_VENDOR_SPECIFIC_TLV, 2, &len);
    adddata(&dataptr, 8, 2, &len); /* Length */ //ssoh_len
    adddata(&dataptr, 0x00013780, 4, &len); /* IANA SMI Code */
    adddata(&dataptr, 0x3000000, 4, &len);

    /* Vendor-Specific Attribute */
    adddata(&dataptr, EAP_TLV_VENDOR_SPECIFIC_TLV, 2, &len);
    adddata(&dataptr, 8, 2, &len); /* Length */ //ssoh_len
    adddata(&dataptr, 0x00013780, 4, &len); /* IANA SMI Code */
    adddata(&dataptr, 0x01000600, 4, &len);

    /* Health-Class */
    adddata(&dataptr, 0x0008, 2, &len); /* Type */
    adddata(&dataptr, 1, 2, &len); /* Length */
    adddata(&dataptr, 0, 1, &len);

    //productname
    adddata(&dataptr, 0x000a, 2, &len); /* Type */
    adddata(&dataptr, sizeof(productname), 2, &len); /* Length */
    memcpy(dataptr, productname, sizeof(productname));
    dataptr+=sizeof(productname);
    len+=sizeof(productname);

    //Health-Class-Status
    adddata(&dataptr, 0x000b, 2, &len);
    adddata(&dataptr, 4, 2, &len);
    adddata(&dataptr, 0x07, 4, &len);

    /* Health-Class */
    adddata(&dataptr, 0x0008, 2, &len); /* Type */
    adddata(&dataptr, 1, 2, &len); /* Length */
    adddata(&dataptr, 1, 1, &len);

    //Health-Class-Status
    adddata(&dataptr, 0x000b, 2, &len);
    adddata(&dataptr, 4, 2, &len);
    adddata(&dataptr, 0xc0ff0002, 4, &len);

    /* Health-Class */
    adddata(&dataptr, 0x0008, 2, &len); /* Type */
    adddata(&dataptr, 1, 2, &len); /* Length */
    adddata(&dataptr, 2, 1, &len);

    //productname
    adddata(&dataptr, 0x000a, 2, &len); /* Type */
    adddata(&dataptr, sizeof(productname), 2, &len); /* Length */
    memcpy(dataptr, productname, sizeof(productname));
    dataptr+=sizeof(productname);
    len+=sizeof(productname);

    //Health-Class-Status
    adddata(&dataptr, 0x000b, 2, &len);
    adddata(&dataptr, 4, 2, &len);
    adddata(&dataptr, 0x07, 4, &len);

    /* Health-Class */
    adddata(&dataptr, 0x0008, 2, &len); /* Type */
    adddata(&dataptr, 1, 2, &len); /* Length */
    adddata(&dataptr, 3, 1, &len);

    //Health-Class-Status
    adddata(&dataptr, 0x000b, 2, &len);
    adddata(&dataptr, 4, 2, &len);
    adddata(&dataptr, 0x04, 4, &len);

    /* Health-Class */
    adddata(&dataptr, 0x0008, 2, &len); /* Type */
    adddata(&dataptr, 1, 2, &len); /* Length */
    adddata(&dataptr, 4, 1, &len);

    //Health-Class-Status
    adddata(&dataptr, 0x000b, 2, &len);
    adddata(&dataptr, 4, 2, &len);
    adddata(&dataptr, 0x00ff0005, 4, &len);

    /* Vendor-Specific Attribute */
    adddata(&dataptr, EAP_TLV_VENDOR_SPECIFIC_TLV, 2, &len);
    adddata(&dataptr, 8, 2, &len); /* Length */
    adddata(&dataptr, 0x00013780, 4, &len); /* IANA SMI Code */
    adddata(&dataptr, 0x31a82000, 4, &len);

    /* Vendor-Specific Attribute */
    adddata(&dataptr, EAP_TLV_VENDOR_SPECIFIC_TLV, 2, &len);
    adddata(&dataptr, 5, 2, &len); /* Length */
    adddata(&dataptr, 0x00013780, 4, &len); /* IANA SMI Code */
    adddata(&dataptr, 0, 1, &len);

    /* Vendor-Specific Attribute */
    adddata(&dataptr, EAP_TLV_VENDOR_SPECIFIC_TLV, 2, &len);
    adddata(&dataptr, 8, 2, &len); /* Length */
    adddata(&dataptr, 0x00013780, 4, &len); /* IANA SMI Code */
    adddata(&dataptr, 0x00000000, 4, &len);
#endif
    /* Update SSoH Length */
    end = dataptr;
    adddata(&ssoh_len, end - ssoh_len - 2, 2, 0);

    /* TODO: SoHReportEntry TLV (zero or more) */

    /* Update length fields */
    end = dataptr;
    adddata(&tlv_len, end - tlv_len - 2, 2, 0);
    adddata(&tlv_len2, end - tlv_len2 - 2, 2, 0);
    adddata(&outer_len, end - outer_len - 2, 2, 0);
    adddata(&inner_len, end - inner_len - 2, 2, 0);

    *out_size = 12 + len;
    (*resultsize) = htons(*out_size);
}

void peap_soh_extensions_process(unsigned char *p, int len, unsigned char *out, unsigned short * out_size, unsigned char id)
{
    unsigned char * dataptr = NULL;
    unsigned short dataofs = 0;
    unsigned short outdataofs = 0;
    //unsigned short * resultsize = NULL;
    unsigned short length;
    //int skip_change2 = 0;

    dataofs = sizeof(struct eap_header);
    outdataofs = sizeof(struct eap_header);

    dataptr = &p[dataofs];
    if ((dataptr[0]<<16)+(dataptr[1]<<8)+((int)dataptr[2]) == EAP_VENDOR_MICROSOFT)
    {
        dataptr += 3;
        dataptr += 4;
        /* TLV Type */
        if (((dataptr[0]<<8)+dataptr[1]) != EAP_TLV_VENDOR_SPECIFIC_TLV)
            return;
        dataptr += 2;

        /* Length */
        length = (dataptr[0]<<8)+dataptr[1];
        dataptr += 2;

        /* Vendor_Id */
        if ((dataptr[0]<<24)+(dataptr[1]<<16)+(dataptr[2]<<8)+((int)dataptr[3]) != EAP_VENDOR_MICROSOFT) //EAP_VENDOR_MICROSOFT
            return;
        dataptr+=4;

        /* TLV Type */
        if (((dataptr[0]<<8)+(dataptr[1])) != 0x02 /* SoH request TLV */)
            return;

        gensoh(out, 2, id, out_size);
    }

    return ;
}

unsigned char * prf_plus(unsigned char * key, unsigned char * seed, unsigned char len)
{
    unsigned char iterations = 0;
    int i = 0;
    unsigned char * temp_data = NULL;
    unsigned char last_val[20];
    unsigned char * Tn = NULL;
    unsigned char mac[20];
    unsigned int mdlen = 20;

    iterations = (len / 20);

    if ((len % 20) != 0)
        iterations++;	// We need a fractional amount of data, so round up.

    Tn = (unsigned char*)malloc(iterations * 20);

    if (Tn == NULL)
    {
        return NULL;
    }
    memset(Tn, 0, iterations*20);

    temp_data = (unsigned char*)malloc(PEAP_CRYPTOBINDING_IPMK_SEED_LEN + 3);

    if (temp_data == NULL)
    {
        return NULL;
    }
    memset(temp_data, 0, PEAP_CRYPTOBINDING_IPMK_SEED_LEN + 3);

    memcpy(temp_data, seed, PEAP_CRYPTOBINDING_IPMK_SEED_LEN);

    temp_data[PEAP_CRYPTOBINDING_IPMK_SEED_LEN] = 0x01;

    // Malloc should have inited everything else to 0x00, so we don't need to set those.
    sha1_hmac(key, 40, temp_data, (PEAP_CRYPTOBINDING_IPMK_SEED_LEN + 3), (unsigned char *)&mac);

    memcpy(Tn, &mac[0], mdlen);	// Copy the initial data to Tn.
    memcpy(&last_val[0], &mac[0], mdlen);

    free(temp_data);

    if (iterations >= 2)
    {
        for (i = 2; i <= iterations; i++)
        {
            temp_data = (unsigned char*)malloc(20 + PEAP_CRYPTOBINDING_IPMK_SEED_LEN + 3);

            if (temp_data == NULL)
            {
                return NULL;
            }

            memset(temp_data, 0, 20 + PEAP_CRYPTOBINDING_IPMK_SEED_LEN + 3);

            memcpy(temp_data, last_val, 20);
            memcpy(&temp_data[20], seed, PEAP_CRYPTOBINDING_IPMK_SEED_LEN);

            temp_data[20 + PEAP_CRYPTOBINDING_IPMK_SEED_LEN] = i;

            // Malloc should have inited everything else to 0x00, so we don't need to set those.
            sha1_hmac(key, 40, temp_data, (20 + PEAP_CRYPTOBINDING_IPMK_SEED_LEN + 3), (unsigned char *)&mac);

            if (mdlen != 20)
            {
                free(temp_data);
                free(Tn);
                return NULL;
            }

            free(temp_data);

            memcpy(&last_val[0], &mac[0], 20);
            memcpy(&Tn[((i - 1) * 20)], &mac[0], 20);
        }
    }

    return Tn;
}

void peap_extensions_build_cryptobinding_result(unsigned char *ipmk, unsigned char *out, unsigned short *outsize)
{
    PEAP_TLV_HEADER* tlvdata = NULL;
    PEAP_TLV_CRYPTOBINDING_DATA * cryptodata = NULL;
    unsigned char tohash_data[61];
    unsigned char *cmk = NULL;	// Reference pointer (DON'T FREE!)
    unsigned int mdlen = 0;
    unsigned char mac[20];
    unsigned short tmp = 0;
    int i=0;

    tlvdata = (PEAP_TLV_HEADER *)out;
    memset(out, 0x00, 61);			// zero our buffer.

    tmp = htons(PEAP_TLV_CRYPTOBINDING);
    memcpy(&tlvdata->tlv_type, &tmp, 2);
    tmp = htons(PEAP_CRYPTOBINDING_TLV_SIZE);
    memcpy(&tlvdata->tlv_length, &tmp, 2);

    cryptodata = (PEAP_TLV_CRYPTOBINDING_DATA *)&out[sizeof(PEAP_TLV_HEADER)];

    cryptodata->version = 0;
    cryptodata->recvVersion = 0;
    cryptodata->subType = 1;		// This is a response.
    for (;i<32;i++)
        cryptodata->nonce[i] = rand()%256;

    // zero everything out.
    memset(tohash_data, 0x00, sizeof(tohash_data));

    memcpy(tohash_data, out, 60);
    out[60] = EAP_TYPE_PEAP;

    cmk = ipmk + 40;

    sha1_hmac(cmk, 20, &tohash_data[0], 61, (unsigned char *)&mac);

    if (mdlen != 20)
    {
        return;
    }

    memcpy(cryptodata->compoundMac, mac, 20);
}

void peap_extensions_process_cryptobinding_tlv(unsigned char * in, unsigned short in_size, unsigned char * out,
        unsigned short * out_size, MSCHV2_VARS* myvars)
{
    PEAP_TLV_HEADER * tlvdata = NULL;
    PEAP_TLV_CRYPTOBINDING_DATA * cryptodata = NULL;
    PEAP_TLV_CRYPTOBINDING_DATA * hashdata = NULL;
    //unsigned short * result_val = NULL;
    //unsigned short * send_result = NULL;
    unsigned char tohash_data[61];

    unsigned char * tk = NULL;	// Tunnel Key (TK)  -- Reference pointer (DON'T FREE)
    unsigned char isk[32];	// Inner Session Key (ISK)
    unsigned char *ipmk = NULL;	// Intermediate PEAP MAC Key (IPMK)
    //struct tls_vars *mytls_vars = NULL;	// Reference pointer (DON'T FREE)
    unsigned char ipmk_seed[PEAP_CRYPTOBINDING_IPMK_SEED_LEN];

    unsigned char * TempKey = NULL;	// Reference pointer (DON'T FREE!)
    unsigned char *cmk = NULL;	// Reference pointer (DON'T FREE!)
    unsigned char mac[20];

    unsigned short tmp = 0;

#ifdef EAPPEAPTEST

    unsigned char tmpisk[]={0xa3,0xa2,0x7c,0x52,0xca,0xfd,0x26,0xc4,0x5e,0x52,0xc5,0x09,0x3d,0xc9,0x29,0xfd
                            ,0xfe,0x08,0x28,0x53,0x37,0x45,0x96,0xa9,0x95,0xc7,0x0f,0x2f,0xb0,0xf2,0xbb,0xf5
                           };
    unsigned char tmptk[]={0x1a,0x06,0x16,0xba,0xcc,0xba,0x66,0xb5,0x74,0xf9,0x23,0xbe,0xe2,0x49,0x1e,0x7e,0xf7,0x2e,0x52,0xcd
                           ,0xc6,0x10,0x77,0x75,0xd5,0x01,0x4a,0xae,0x4b,0xba,0xc7,0x3e,0xb8,0x68,0xae,0x12,0xf3,0x6c,0x89,0x1f
                          };
    unsigned char tmpin[]={0x00,0x0c,0x00,0x38,0x00,0x00,0x00,0x00,0x50,0x62,0xb7,0x87,0x63,0xf0,0x2a,0xd4,0xda,0x36,0x66,0x8a
                           ,0xbf,0x5a,0xc1,0x70,0xb9,0xe1,0xb4,0x36,0x62,0x07,0xac,0x46,0xea,0x32,0x43,0xe9,0x46,0xd8,0x9a,0x5e
                           ,0x47,0x54,0xc2,0x6e,0x14,0x6a,0x9e,0x9a,0xcb,0x49,0xf2,0xc6,0x9c,0xc0,0xd9,0x77,0x82,0x5b,0xca,0x23
                          };
#endif

    unsigned int mdlen = 20;

#ifdef EAPPEAPTEST
    memcpy(in, tmpin, sizeof(tmpin));
#endif

    tlvdata = (PEAP_TLV_HEADER *) in;
    //tmp = tlvdata->tlv_type;
    memcpy(&tmp, &tlvdata->tlv_type, 2);
    if ((ntohs(tmp) & (~PEAP_TLV_TYPE_FLAGS)) != PEAP_TLV_CRYPTOBINDING)
    {
        return;
    }
    cryptodata = (PEAP_TLV_CRYPTOBINDING_DATA *) & in[sizeof(PEAP_TLV_HEADER)];
    if (cryptodata->subType != 0)
    {
        return;
    }

    memcpy(&tohash_data[0], in, 60);
    tohash_data[60] = EAP_TYPE_PEAP;

    hashdata = (PEAP_TLV_CRYPTOBINDING_DATA *) & tohash_data[sizeof(PEAP_TLV_HEADER)];

    memset(hashdata->compoundMac, 0x00, 20);	// 0 out the MAC.

#ifdef EAPPEAPTEST
    tk = tmptk;
#else
    tk = dpconf->eaptls->keymaterial; // first 60 octets of the keymaterial
#endif
    free(dpconf->eaptls->keymaterial);
    dpconf->eaptls->keymaterial = NULL;

    memset(&isk[0], 0x00, 32);

#ifdef EAPPEAPTEST
    memcpy(isk, tmpisk, 32);
#else
    memcpy(isk, myvars->keyingMaterial, 32);
#endif
    free(myvars->keyingMaterial);
    myvars->keyingMaterial = NULL;

    // Otherwise, leave it all 0s.
    memset(&ipmk_seed[0], 0x00, sizeof(ipmk_seed));
    memcpy(&ipmk_seed[0], PEAP_CRYPTOBINDING_IPMK_SEED_STR, PEAP_CRYPTOBINDING_IPMK_SEED_STR_LEN);
    memcpy(&ipmk_seed[PEAP_CRYPTOBINDING_IPMK_SEED_STR_LEN], isk, 32);

    TempKey = tk;		// First 40 octets.
    ipmk = prf_plus(TempKey, ipmk_seed, 60);
    if (ipmk == NULL)
    {
        return;
    }

    cmk = ipmk + 40;

    sha1_hmac(cmk, 20, &tohash_data[0], 61, (unsigned char *)&mac);

    if (mdlen != 20)
    {
        free(ipmk);
        return;
    }

    if (memcmp(cryptodata->compoundMac, mac, 20) != 0)
    {
        //Compare fail in windows server 2008
    }
    peap_extensions_build_cryptobinding_result(ipmk, out, out_size);
    free(ipmk);
}

void peap_extensions_process(unsigned char *p, int len, unsigned char *out, unsigned short * out_size, MSCHV2_VARS* myvars)
{
    unsigned char * dataptr = NULL;
    unsigned short dataofs = 0, tmp = 0;
    unsigned short outdataofs = 0;
    PEAP_TLV_HEADER * tlvdata = NULL;
    unsigned short * resultsize = NULL;

    dataofs = sizeof(struct eap_header);
    outdataofs = sizeof(struct eap_header);

    while (dataofs < len)
    {
        dataptr = &p[dataofs];
        tlvdata = (PEAP_TLV_HEADER *) dataptr;
        memcpy(&tmp, &tlvdata->tlv_type, 2);
        //tmp = tlvdata->tlv_type;
        if ((ntohs(tmp) & PEAP_TLV_MANDATORY_FLAG) == PEAP_TLV_MANDATORY_FLAG)
        {
            //debug_printf(DEBUG_AUTHTYPES, "TLV is MANDATORY!\n");
        }
        // Switch on the value with the upper flag bits masked out.
        switch ((ntohs(tmp) & (~PEAP_TLV_TYPE_FLAGS)))
        {
        case PEAP_TLV_RESULT:
            peap_extensions_process_result_tlv(dataptr, (len - dataofs), &out[outdataofs], out_size);
            outdataofs = (*out_size) + sizeof(struct eap_header);
            dataofs += sizeof(PEAP_TLV_HEADER) + ntohs(tmp);	// Skip this TLV.
            break;
        case PEAP_TLV_CRYPTOBINDING:
            peap_extensions_process_cryptobinding_tlv(dataptr, (len - dataofs), &out [outdataofs], out_size, myvars);
            outdataofs = (*out_size) + sizeof(struct eap_header);
            dataofs += sizeof(PEAP_TLV_HEADER) + ntohs(tmp);	// Skip this TLV.
            break;
        default:
            dataofs += sizeof(PEAP_TLV_HEADER) + ntohs(tmp);
            break;
        }
    }
    out[0] = EAP_RESPONSE_PKT;
    out[1] = p[1];	// EAP ID
    resultsize = (unsigned short *) & out[2];
    (*out_size) = outdataofs;
    (*resultsize) = htons((*out_size));
    out[4] = PEAP_EAP_EXTENSION;
}

void peap_extensions_process_result_tlv(unsigned char * in, unsigned short in_size, unsigned char * out, unsigned short * out_size)
{
    PEAP_TLV_HEADER * tlvdata = NULL;
    unsigned short * result_val = NULL;
    unsigned short * send_result = NULL, tmp = 0;

    tlvdata = (PEAP_TLV_HEADER *) in;
    //tmp = tlvdata->tlv_type;
    memcpy(&tmp, &tlvdata->tlv_type, 2);
    if ((ntohs(tmp) & (~PEAP_TLV_TYPE_FLAGS)) != PEAP_TLV_RESULT)
    {
        return;
    }

    result_val = (unsigned short *) (&in[sizeof(PEAP_TLV_HEADER)]);

    // Set up the common values.
    tlvdata = (PEAP_TLV_HEADER *) out;
    tmp = htons(2);
    memcpy(&tlvdata->tlv_length, &tmp, 2);
    //tlvdata->tlv_length = htons(2);
    tmp = htons(PEAP_TLV_RESULT | PEAP_TLV_MANDATORY_FLAG);
    memcpy(&tlvdata->tlv_type, &tmp, 2);
    send_result = (unsigned short *) & out[sizeof(PEAP_TLV_HEADER)];
    (*out_size) += (sizeof(PEAP_TLV_HEADER) + sizeof(unsigned short));

    memcpy(&tmp, result_val, 2);
    switch (ntohs(tmp))
    {
    default:
    case PEAP_TLV_RESULT_RESERVED:

        // Fall through.
    case PEAP_TLV_RESULT_FAILURE:
        tmp = htons(PEAP_TLV_RESULT_FAILURE);
        memcpy(send_result, &tmp, 2);
        break;

    case PEAP_TLV_RESULT_SUCCESS:
        tmp = htons(PEAP_TLV_RESULT_SUCCESS);
        memcpy(send_result, &tmp, 2);
        break;
    }
}

char ctonibble(char cnib)
{
    char retVal=0x00;
    char testval=0x00;

    if ((cnib>='0') && (cnib<='9'))
    {
        retVal = cnib - '0';
    }
    else
    {
        testval = toupper(cnib);
        if ((testval>='A') && (testval<='F'))
        {
            retVal = ((testval - 'A') +10);
        }
        else
        {
        }
    }
    return retVal;
}

void process_hex(char *instr, int size, char *outstr)
{
    int i;

    // Make sure we don't try to convert something that isn't byte aligned.
    if ((size % 2) != 0)
    {
        return;
    }

    for (i=0;i<(size/2);i++)
    {
        if (instr[i*2] != 0x00)
        {
            outstr[i] = (ctonibble(instr[i*2]) << 4) + ctonibble(instr[(i*2)+1]);
        }
    }
}

int decode_success(char *instr, int instr_size, char *authstr)
{
    char *temp;

    // The success string passed in should look like this :
    // S=<auth string> M=<message>
    if (instr[0] != 'S') return -1;

    temp = &instr[2];  // Point past the S=

    process_hex(temp, 40, authstr);

    return 0;
}

void HashNtPasswordHash(char *PasswordHash, char *PasswordHashHash)
{
    MD4_CTX md4;

    MD4Init(&md4);
    MD4Update(&md4, (unsigned char*)PasswordHash, 16);
    MD4Final(&md4);
    memcpy(PasswordHashHash, md4.digest, 16);
}

void GenerateAuthenticatorResponse(char *passwd, char *NTResponse, char *PeerChallenge, char *AuthenticatorChallenge,
                                   char *username, char *AuthenticatorResponse)
{
    char PasswordHash[16];
    char PasswordHashHash[16];
    sha1_context sha1;
    char Digest[20];
    char Challenge[8];

    char *password = dpconf->eaptls->password;

    char Magic1[39] =
        {0x4D, 0x61, 0x67, 0x69, 0x63, 0x20, 0x73, 0x65, 0x72, 0x76,
         0x65, 0x72, 0x20, 0x74, 0x6F, 0x20, 0x63, 0x6C, 0x69, 0x65,
         0x6E, 0x74, 0x20, 0x73, 0x69, 0x67, 0x6E, 0x69, 0x6E, 0x67,
         0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x74
        };

    char Magic2[41] =
        {0x50, 0x61, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x6D, 0x61, 0x6B,
         0x65, 0x20, 0x69, 0x74, 0x20, 0x64, 0x6F, 0x20, 0x6D, 0x6F,
         0x72, 0x65, 0x20, 0x74, 0x68, 0x61, 0x6E, 0x20, 0x6F, 0x6E,
         0x65, 0x20, 0x69, 0x74, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6F,
         0x6E
        };

    //uni_passwd = to_uni(passwd);
    NtPasswordHash(password, (char *)&PasswordHash);

    HashNtPasswordHash((char *)&PasswordHash, (char *)&PasswordHashHash);

    sha1_starts(&sha1);
    sha1_update(&sha1, (unsigned char *)&PasswordHashHash, 16);
    sha1_update(&sha1, (unsigned char *)NTResponse, 24);
    sha1_update(&sha1, (unsigned char *)Magic1, 39);
    sha1_finish(&sha1, (unsigned char *)&Digest);

    // Is this normal?  Should AuthenticatorChallenge = PeerChallenge?
    ChallengeHash(PeerChallenge, AuthenticatorChallenge, username, (char *)&Challenge);

    sha1_starts(&sha1);
    sha1_update(&sha1, (unsigned char *)&Digest, 20);
    sha1_update(&sha1, (unsigned char *)&Challenge, 8);
    sha1_update(&sha1, (unsigned char *)Magic2, 41);
    sha1_finish(&sha1, (unsigned char *)AuthenticatorResponse);
}

int CheckAuthenticatorResponse(char *uni_passwd, char *ntresponse, char *peer_chal, char *auth_chal, char *username, char *recv_resp)
{
    char MyResponse[20];

    GenerateAuthenticatorResponse(uni_passwd, ntresponse, peer_chal, auth_chal,
                                  username, (char *)&MyResponse);

    if (memcmp(&MyResponse, recv_resp, 20) != 0)
    {
        return 1;
    }
    return 0;
}
void GetMasterKey(char *PasswordHashHash, char *NTResponse, char *MasterKey)
{
    sha1_context sha1;
    char Digest[20];

    char Magic1[27] =
        { 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74,
          0x68, 0x65, 0x20, 0x4d, 0x50, 0x50, 0x45, 0x20, 0x4d,
          0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x4b, 0x65, 0x79
        };

    memset(&Digest, 0x00, 20);

    sha1_starts(&sha1);
    sha1_update(&sha1, (unsigned char *)PasswordHashHash, 16);
    sha1_update(&sha1, (unsigned char *)NTResponse, 24);
    sha1_update(&sha1, (unsigned char *)Magic1, 27);
    sha1_finish(&sha1, (unsigned char *)Digest);

    memcpy(MasterKey, &Digest, 16);
}

void GetAsymetricStartKey(char *MasterKey, char *SessionKey, int SessionKeyLength, int IsSend, int IsServer)
{
    sha1_context sha1;
    char Digest[20];
    char Magic[84];

    unsigned char Magic2[84] =
        { 0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
          0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
          0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
          0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20, 0x6b, 0x65, 0x79,
          0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73,
          0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73, 0x69, 0x64, 0x65,
          0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
          0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
          0x6b, 0x65, 0x79, 0x2e
        };

    unsigned char Magic3[84] =
        { 0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
          0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
          0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
          0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
          0x6b, 0x65, 0x79, 0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68,
          0x65, 0x20, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73,
          0x69, 0x64, 0x65, 0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73,
          0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20,
          0x6b, 0x65, 0x79, 0x2e
        };

    unsigned char SHSpad1[40] =
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    unsigned char SHSpad2[40] =
        { 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
          0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
          0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
          0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2
        };

    memset(&Digest, 0x00, 20);

    if (IsSend)
    {
        if (IsServer)
        {
            memcpy(&Magic, &Magic3, 84);
        }
        else
        {
            memcpy(&Magic, &Magic2, 84);
        }
    }
    else
    {
        if (IsServer)
        {
            memcpy(&Magic, &Magic2, 84);
        }
        else
        {
            memcpy(&Magic, &Magic3, 84);
        }
    }

    sha1_starts(&sha1);
    sha1_update(&sha1, (unsigned char*)MasterKey, 16);
    sha1_update(&sha1, (unsigned char*)SHSpad1, 40);
    sha1_update(&sha1, (unsigned char*)Magic, 84);
    sha1_update(&sha1, (unsigned char*)SHSpad2, 40);
    sha1_finish(&sha1,(unsigned char*)Digest);

    memcpy(SessionKey, &Digest, SessionKeyLength);
}

int eapmschapv2_success(unsigned char *p, MSCHV2_VARS *myvars)
{
    char *username = dpconf->eaptls->identity;
    char *password = dpconf->eaptls->password;
    char *authstr = (char*)malloc(20);
    char *answer = (char*)(&p[4]);
    unsigned char NtHash[16], NtHashHash[16], MasterKey[16];

    decode_success(answer, strlen(answer), authstr);

    // Now, using the value from authstr, we should validate the server..

    if (0 != CheckAuthenticatorResponse(password, (char *) myvars->NtResponse,
                                        (char *)myvars->PeerChallenge, (char *)myvars->AuthenticatorChallenge, username, authstr))
    {
        free(authstr);
        return -1;
    }

    free(authstr);

    //generate keying material.
    NtPasswordHash(password, (char *)&NtHash);

    HashNtPasswordHash((char *)&NtHash, (char *)&NtHashHash);

    myvars->keyingMaterial = (unsigned char *) malloc(32);	// 16 bytes each.

    memset(myvars->keyingMaterial, 0, 32);

    GetMasterKey((char *)&NtHashHash, (char *)myvars->NtResponse, (char *)&MasterKey);

    // Now, get the send key.
    GetAsymetricStartKey((char*)MasterKey, (char*)myvars->keyingMaterial, 16, 1, 0);

    // And the recv key.
    GetAsymetricStartKey((char*)MasterKey, (char*)(myvars->keyingMaterial+16), 16, 0, 0);

    return 0;
}

void eapmschapv2_challenge_resp(MSCHV2_CHAN *challenge, MSCHV2_VARS *myvars, unsigned char id, unsigned char *sendbuf)
{
    MSCHV2_RES *response = NULL;
    unsigned char *resp = sendbuf;
    unsigned short respsize = 0, tmp = 0;
    struct eap_header *eap_header = NULL;
    char *username = dpconf->eaptls->identity;

    // 54 bytes is the length of the response, including MS-CHAPv2 header.
    respsize = 54 + strlen(username) + sizeof(struct eap_header);
    //resp = (unsigned char*)malloc(respsize);
    eap_header = (struct eap_header *)resp;
    eap_header->eap_code = EAP_RESPONSE_PKT;
    eap_header->eap_identifier = id;
    eap_header->eap_length = htons(respsize);
    eap_header->eap_type = EAP_TYPE_MSCHAPV2;

    // Now, build the MS-CHAPv2 part of the response.
    response = (MSCHV2_RES *)&resp[sizeof(struct eap_header)];
    response->OpCode = MS_CHAPV2_RESPONSE;
    response->MS_CHAPv2_ID = myvars->MS_CHAPv2_ID;
    //response->MS_Length = htons(54 + strlen(username));
    //response->MS_Length = 54 + strlen(username);
    tmp = htons(54 + strlen(username));
    memcpy(&response->MS_Length, &tmp, 2);
    response->Value_Size = 49;

    memcpy((unsigned char *) & response->Peer_Challenge, myvars->PeerChallenge, 16);
    memset((unsigned char *) & response->Reserved, 0x00, 8);
    memcpy((unsigned char *) & response->NT_Response, myvars->NtResponse, 24);
    response->Flags = 0;

    memcpy(&resp[sizeof(struct eap_header) + 54], username,strlen(username));
}

void eapmschapv2_success_resp(MSCHV2_VARS *myvars, unsigned char id, unsigned char *sendbuf)
{
    unsigned char *resp = sendbuf;
    unsigned short respsize = 0;
    struct eap_header *eap_header;

    // 54 bytes is the length of the response, including MS-CHAPv2 header.
    respsize = sizeof(struct eap_header) + 1;
    //resp = (unsigned char*)malloc(respsize);

    // Build the EAP header for the response.
    eap_header = (struct eap_header *)resp;

    eap_header->eap_code = EAP_RESPONSE_PKT;
    eap_header->eap_identifier = id;
    eap_header->eap_length = htons(respsize);
    eap_header->eap_type = EAP_TYPE_MSCHAPV2;

    resp[sizeof(struct eap_header)] = MS_CHAPV2_SUCCESS;
}

void ChallengeHash(char *PeerChallenge, char *AuthenticatorChallenge, char *UserName, char *Challenge)
{
    sha1_context sha1;
    char Digest[30];
    //int retLen = 0;

    memset(Digest, 0x00, 30);
    sha1_starts(&sha1);
    sha1_update(&sha1, (unsigned char*)PeerChallenge, 16);
    sha1_update(&sha1, (unsigned char*)AuthenticatorChallenge, 16);
    sha1_update(&sha1, (unsigned char*)UserName, strlen(UserName));
    sha1_finish(&sha1, (unsigned char*)Digest);

    /*
    * Only the user name (as presented by the peer and
    * excluding any prepended domain name)
    * is used as input to SHAUpdate().
    */

    memcpy(Challenge, Digest, 8);
}

char *to_unicode(char *non_uni)
{
    char *retUni;
    unsigned int i;

    if (!non_uni)
    {
        return NULL;
    }

    retUni = (char *)malloc((strlen(non_uni)+1)*2);
    if (retUni == NULL)
    {
        return NULL;
    }
    memset(retUni, 0, ((strlen(non_uni)+1)*2));

    for (i=0; i<strlen(non_uni); i++)
    {
        retUni[(2*i)] = non_uni[i];
    }
    return retUni;
}

void NtPasswordHash(char *Password, char *PasswordHash)
{
    MD4_CTX md4;
    int len;
    char *uniPassword;

    uniPassword = Password;
    uniPassword = to_unicode(Password);
    len = (strlen(Password))*2;


    MD4Init(&md4);
    MD4Update(&md4, (unsigned char*)uniPassword, len);
    MD4Final(&md4);
    memcpy(PasswordHash, md4.digest, 16);

    free(uniPassword);
}

// Shamelessly take from the hostap code written by Jouni Malinen
void des_encrypt(unsigned char * clear, unsigned char * key, unsigned char * cypher)
{
    unsigned char pkey[8], next, tmp;
    int i;
    des_context des;

    /* Add parity bits to key */
    next = 0;
    for (i = 0; i < 7; i++) {
        tmp = key[i];
        pkey[i] = (tmp >> i) | next | 1;
        next = tmp << (7 - i);
    }
    pkey[i] = next | 1;

    des_setkey_enc( &des, pkey );
    des_crypt_ecb( &des, clear, cypher );

}


void ChallengeResponse(char *Challenge, char *PasswordHash, char *Response)
{
    unsigned char zpwd[7];

    des_encrypt((unsigned char *) Challenge, (unsigned char *) PasswordHash,
                (unsigned char *) Response);
    des_encrypt((unsigned char *) Challenge, (unsigned char *) PasswordHash + 7,
                (unsigned char *) Response + 8);
    zpwd[0] = PasswordHash[14];
    zpwd[1] = PasswordHash[15];
    memset(zpwd + 2, 0, 5);
    des_encrypt((unsigned char *) Challenge, zpwd, (unsigned char *) Response + 16);
}

void GenerateNTResponse(char *AuthenticatorChallenge, char *PeerChallenge,
                        char *UserName, char *Password, char *Response)
{
    char Challenge[8], PasswordHash[16];

    ChallengeHash((char*)PeerChallenge, (char*)AuthenticatorChallenge, UserName, (char *)&Challenge);

    NtPasswordHash(Password, (char *)&PasswordHash);

    ChallengeResponse(Challenge, (char *)&PasswordHash, Response);
}

void eapmschapv2_challenge(MSCHV2_CHAN *challenge, MSCHV2_VARS *myvars)
{
    int i=0;
    char *username = dpconf->eaptls->identity;
    char *passwprd = dpconf->eaptls->password;

    myvars->AuthenticatorChallenge = (unsigned char*)malloc(16);
    myvars->MS_CHAPv2_ID = challenge->MS_CHAPv2_ID;
    if (challenge->Value_Size != 0x10)
    {
        return ;
    }
    memcpy(myvars->AuthenticatorChallenge, &challenge->Challenge,16);
    myvars->PeerChallenge = (unsigned char *) malloc(16);
    for (;i<16;i++)
        myvars->PeerChallenge[i] = rand() % 256;

    myvars->NtResponse = (unsigned char *) malloc(24);
    memset(myvars->NtResponse, 0, 24);

    GenerateNTResponse((char *)myvars->AuthenticatorChallenge,
                       (char *)myvars->PeerChallenge, username,
                       passwprd, (char *)myvars->NtResponse);
}

void sendTlsData(unsigned char **p, unsigned char *data, unsigned int datalen, unsigned char id)
{
    unsigned char *tmp;
    //unsigned char tmpbuf[100] = {0};
    int i = 0;
    int padlen  = 0;
    unsigned int len;
    tmp = *p + 13;
    len = datalen;

    memcpy(tmp, data, datalen);

    //pkt->wp = pkt->wp - 5;
    addEAPTLSRecord(tmp-5 , 0x17, len);

    memcpy(tmp-13, dpconf->eaptls->out_ctr, 8);

    if ( dpconf->eaptls->maclen == 20 )
        sha1_hmac( dpconf->eaptls->mac_enc, 20,
                   tmp - 13,  len + 13, tmp + len);

    memset(tmp - 13, 0, 8);

    //Output message length + mac length
    len += dpconf->eaptls->maclen;

    for ( i = 7; i >= 0; i-- )
        if ( ++dpconf->eaptls->out_ctr[i] != 0 )
            break;

    if (dpconf->eaptls->ivlen == 0)
    {
    }
    else
    {
        padlen = dpconf->eaptls->ivlen - ( len + 1 ) % dpconf->eaptls->ivlen;
        if ( padlen == dpconf->eaptls->ivlen )
            padlen = 0;

        for ( i = 0; i <= padlen; i++ )
            tmp[len + i] = (unsigned char) padlen;

        len += padlen + 1;

        switch ( dpconf->eaptls->ivlen )
        {
        case 16:
            aes_crypt_cbc( (aes_context *) dpconf->eaptls->ctx_enc,
                           AES_ENCRYPT, len,
                           dpconf->eaptls->iv_enc, tmp, tmp );
            break;
        }
    }
    *(tmp - 5 + 3) = (unsigned char)( len >> 8 );
    *(tmp - 5 + 4) = (unsigned char)( len      );

    sendEAPTLSPkt(tmp - RECORD_LAYER_LEN, len + RECORD_LAYER_LEN, (unsigned short)(len + RECORD_LAYER_LEN), 0x00, id);
}

int sendEAPCertificateSequence(unsigned char id, unsigned char type2)
{
    unsigned short eapCertSize = sizeof(eapCert);
    unsigned int totallen = 0;
    unsigned char *tmp;
    unsigned char *buf;

    if (dpconf->eaptls->sendbuflen == 0)
    {
        if (dpconf->eappro == EAP_TYPE_TLS)
            totallen = RECORD_LAYER_LEN + HAND_PRO_HDR + 3/*certificate len field*/
                       + eapCertSize + RECORD_LAYER_LEN + HAND_PRO_HDR
                       + CLI_KEY_XHG_KEY_LEN  + RECORD_LAYER_LEN + HAND_PRO_HDR
                       + CLI_CERT_VER_LEN + RECORD_LAYER_LEN + HAND_PRO_HDR
                       + CHG_CIPHER_SPEC + RECORD_LAYER_LEN + 8 + 5 + CLI_FINISH_LEN ;
        else if (dpconf->eappro == EAP_TYPE_PEAP)
            totallen = RECORD_LAYER_LEN + HAND_PRO_HDR + 3/*certificate len field*/
                       /*+ eapCertSize*/ + RECORD_LAYER_LEN + HAND_PRO_HDR
                       + CLI_KEY_XHG_KEY_LEN /*+ RECORD_LAYER_LEN + HAND_PRO_HDR
				+ CLI_CERT_VER_LEN */+ RECORD_LAYER_LEN + HAND_PRO_HDR
                       + CHG_CIPHER_SPEC + RECORD_LAYER_LEN + 8 + 5 + CLI_FINISH_LEN ;

        buf = (unsigned char*)malloc(1000);
        memset(buf, 0, 1000);
        dpconf->eaptls->buf = buf;

        tmp = buf;

        if (certreq == 1)
            if (addEAPCertificate(&tmp) < 0)
                return -1;

        if (addEAPCliKeyXhg(&tmp) < 0)
            return -1;

        if (dpconf->eappro == EAP_TYPE_TLS)
            if (addEAPCertificateVerify(&tmp) < 0)
                return -1;

        addEAPChangeCipherspec(&tmp);

        if (addEAPClientFinish(&tmp) < 0)
            return -1;

        totallen = tmp - buf;
        dpconf->eaptls->sendbuf = buf;
        dpconf->eaptls->sendbuflen = totallen;
    }

    if (dpconf->eaptls->sendbuflen > 1300)
    {
        if (dpconf->eaptls->eap == 8)
        {
            sendEAPTLSPkt(dpconf->eaptls->sendbuf, totallen, 1300, 0xc0, id);
            dpconf->eaptls->eap = 9;
        }
        else if (dpconf->eaptls->eap == 9)
        {
            sendEAPTLSPkt(dpconf->eaptls->sendbuf, totallen, 1300, 0x40, id);
        }
        dpconf->eaptls->sendbuflen -= 1300;
        dpconf->eaptls->sendbuf += 1300;
    }
    else
    {
        //if(dpconf->eappro == EAP_TYPE_TLS)
        //	sendEAPTLSPkt(dpconf->eaptls->sendbuf, totallen, dpconf->eaptls->sendbuflen, 0x00, id);
        //else if(dpconf->eappro == EAP_TYPE_PEAP)
        sendEAPTLSPkt(dpconf->eaptls->sendbuf, totallen, dpconf->eaptls->sendbuflen, 0x00, id);
        dpconf->eaptls->sendbuflen = 0;
        dpconf->eaptls->sendbuf = NULL;
        dpconf->eaptls->eap = 10;
        free(dpconf->eaptls->buf);
        dpconf->eaptls->buf = NULL;
    }
    return 0;
}

void sendEAPTLSPkt(unsigned char *buf, unsigned int totallen, unsigned short authlen, unsigned char flags, unsigned char id)
{
    unsigned char *tmp;
    unsigned char Nearest[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};
    unsigned char *txpkt = (unsigned char*)malloc(EAP_LEN + EAP_HDR_TLS_LEN + authlen);
    EAPTLSPKT *p = (EAPTLSPKT*)txpkt;
    p->auth.ethHdr.ethType = htons(0x888e);
    memcpy(p->auth.ethHdr.srcAddr, dpconf->srcMacAddr[intf], MAC_ADDR_LEN);
    memcpy(p->auth.ethHdr.destAddr, Nearest, MAC_ADDR_LEN);
    p->auth.ver = 2;
    p->auth.type = 0; //eap pakcet
    if (dpconf->eaptls->eap == 8)
        p->auth.len = ntohs(EAP_HDR_LEN + EAP_HDR_TLS_LEN + authlen - 4);
    else
        p->auth.len = ntohs(EAP_HDR_LEN + 1 + authlen);
    p->code = 2; //response
    p->id = id;
    dpconf->eaptls->eapid = id;
    if (dpconf->eaptls->eap == 8)
        p->len2 = htons(EAP_HDR_LEN + EAP_HDR_TLS_LEN + authlen - 4);
    else
        p->len2 = htons(EAP_HDR_LEN + 1 + authlen);
    p->type2 = dpconf->eappro;
    p->flags = flags;
    totallen = htonl(totallen);
    if (dpconf->eaptls->eap == 8)
    {
        //memcpy(&(p->flags) + 1, &totallen, 4);
        tmp = &(p->flags) + 1/* + 4*/;
    }
    else
        tmp = &(p->flags) + 1;

    memcpy(tmp, buf, authlen);

#if WIN
    if (dpconf->eaptls->eap == 8)
    {
        DMWGWriteProc(txpkt + 2, EAP_LEN + EAP_HDR_TLS_LEN + authlen -4 - 2);
    }
    else if (dpconf->eaptls->eap == 12)
    {
        DMWGWriteProc(txpkt + 2, EAP_LEN - 4 + EAP_HDR_TLS_LEN + authlen - 2);
    }
    else
        DMWGWriteProc(txpkt + 2, EAP_LEN + 1 + authlen - 2);

    free(txpkt);
    txpkt = NULL;
#else

    if (dpconf->eaptls->eap == 8)
    {
        gmacsend_F(((char*)txpkt) + 2, EAP_LEN + EAP_HDR_TLS_LEN + authlen - 4 - 2, OPT_FREE | OPT_NOCS, txpkt);
    }
    else if (dpconf->eaptls->eap == 12)
    {
        gmacsend_F(((char*)txpkt) + 2, EAP_LEN + EAP_HDR_TLS_LEN + authlen - 4 - 2, OPT_FREE | OPT_NOCS, txpkt);
    }
    else
        gmacsend_F(((char*)txpkt) + 2, EAP_LEN + 1 + authlen - 2, OPT_FREE | OPT_NOCS, txpkt);
#endif

}

int addEAPCertificate(unsigned char **p)
{
    unsigned char *tmp  = NULL;
    unsigned int len = 7/*handshake protocol hdr*/;
    if (dpconf->eappro == EAP_TYPE_TLS)
    {
        len += sizeof(eapCert);

        addEAPTLSRecord(*p, 0x16, len);
        (*p) += RECORD_LAYER_LEN;

        tmp = *p;

        **p = 0x0b;
        (*p)++;

        **p = (unsigned char)( ( len - 4 ) >> 16 );
        *(*p+1) = (unsigned char)( ( len - 4 ) >>  8 );
        *(*p+2) = (unsigned char)( ( len - 4 ) );
        (*p) += 3;

        *(*p) = (unsigned char)( ( len - 4 - 3 ) >> 16 );
        *(*p+1) = (unsigned char)( ( len - 4 - 3 ) >>  8 );
        *(*p+2) = (unsigned char)( ( len - 4 - 3 ) );
        (*p) += 3;

        memcpy(*p, eapCert, sizeof(eapCert));
        (*p) += sizeof(eapCert);

    }
    else if (dpconf->eappro == EAP_TYPE_PEAP)
    {
        addEAPTLSRecord(*p, 0x16, len);
        (*p) += RECORD_LAYER_LEN;
        tmp = *p;
        **p = 0x0b;
        *(*p+1) = 0x00;
        *(*p+2) = 0x00;
        *(*p+3) = 0x03;
        *(*p+4) = 0x00;
        *(*p+5) = 0x00;
        *(*p+6) = 0x00;
        (*p) += len;
    }
    md5_updateH( &dpconf->eaptls->fin_md5 , tmp, len );
    sha1_updateH( &dpconf->eaptls->fin_sha1, tmp, len );
    return 0;
}

int addEAPCliKeyXhg(unsigned char **p)
{
    int i, ret, n;
    unsigned char *tmp;
    unsigned char premaster[48];
    unsigned char keyblk[256];
    unsigned char random[64];
    unsigned char *key1;
    unsigned char *key2;

    unsigned int len = 0/*HAND_PRO_HDR + CLI_KEY_XHG_KEY_LEN*/;

    n = dpconf->eaptls->server_cert->rsa.len;
    len = 6 + n;

    addEAPTLSRecord(*p, 0x16, len);
    (*p) += RECORD_LAYER_LEN;

    tmp = *p;

    **p = 0x10;
    (*p)++;

    **p = (unsigned char)( ( len - 4 ) >> 16 );
    *(*p+1) = (unsigned char)( ( len - 4 ) >>  8 );
    *(*p+2) = (unsigned char)( ( len - 4 ) );
    (*p) += 3;

    /*
     * RSA key exchange -- send rsa_public(pkcs1 v1.5(premaster))
     */
    premaster[0] = 0x03;
    premaster[1] = 0x01;

    for ( i = 2; i < 48; i++ )
    {
#ifndef EAP_DBG
        premaster[i] = rand() % 256;
#else
        premaster[i] = premasterg[i];
#endif
    }

    //n = dpconf->eaptls->server_cert->rsa.len;

    **p = (unsigned char)( n >> 8 );
    (*p)++;
    **p = (unsigned char)( n      );
    (*p)++;

    //len = 6 + n;

    ret = rsa_pkcs1_encrypt( &dpconf->eaptls->server_cert->rsa, RSA_PUBLIC, 48, premaster, tmp + 6);
    if ( ret != 0 )
    {
        return -1;
    }

    //(*p) += CLI_KEY_XHG_KEY_LEN - 2;
    (*p) += (len-6);


    md5_updateH( &dpconf->eaptls->fin_md5 , tmp, len );
    sha1_updateH( &dpconf->eaptls->fin_sha1, tmp, len );

    memcpy(random , dpconf->eaptls->clientRandom, 32);
    memcpy(random + 32, dpconf->eaptls->serverRandom, 32);

    //derive keys
    tls1_prf( premaster, 48, "master secret", random, 64, dpconf->eaptls->master, 48 );

    memcpy(random , dpconf->eaptls->clientRandom, 32);
    memcpy(random + 32, dpconf->eaptls->serverRandom, 32);
    dpconf->eaptls->keymaterial = (unsigned char*)malloc(64);
    tls1_prf(dpconf->eaptls->master, 48, "client EAP encryption", random, 64, dpconf->eaptls->keymaterial, 64);

    /*
    * Swap the client and server random values.
    */
    memcpy(random , dpconf->eaptls->serverRandom, 32);
    memcpy(random + 32, dpconf->eaptls->clientRandom, 32);

    tls1_prf(dpconf->eaptls->master, 48, "key expansion", random, 64, keyblk, 256 );

    /*
     * Determine the appropriate key, IV and MAC length.
     */
    switch ( dpconf->eaptls->cipher )
    {
    case 0x002f:
        dpconf->eaptls->keylen = 16;
        dpconf->eaptls->minlen = 20;
        dpconf->eaptls->ivlen = 16;
        dpconf->eaptls->maclen = 20;
        break;
    case 0x0005:
        dpconf->eaptls->keylen = 16;
        dpconf->eaptls->minlen = 20;
        dpconf->eaptls->ivlen = 0;
        dpconf->eaptls->maclen = 20;
        break;

    default:
        return -1;
    }

    /*
     * Finally setup the cipher contexts, IVs and MAC secrets.
     */

    key1 = keyblk + dpconf->eaptls->maclen * 2;
    key2 = keyblk + dpconf->eaptls->maclen * 2 + dpconf->eaptls->keylen;

    memcpy( dpconf->eaptls->mac_enc, keyblk,  dpconf->eaptls->maclen );
    memcpy( dpconf->eaptls->mac_dec, keyblk + dpconf->eaptls->maclen, dpconf->eaptls->maclen );

    memcpy( dpconf->eaptls->iv_enc, key2 + dpconf->eaptls->keylen,  dpconf->eaptls->ivlen );
    memcpy( dpconf->eaptls->iv_dec, key2 + dpconf->eaptls->keylen + dpconf->eaptls->ivlen, dpconf->eaptls->ivlen );

    switch ( dpconf->eaptls->cipher )
    {
    case 0x002f:
        //aes_setkey_enc_sw( (aes_context *) dpconf->eaptls->ctx_enc, key1, 128 );
        //aes_setkey_dec_sw( (aes_context *) dpconf->eaptls->ctx_dec, key2, 128 );
        memcpy(dpconf->eaptls->ctx_enc, key1, dpconf->eaptls->keylen);
        memcpy(dpconf->eaptls->ctx_dec, key2, dpconf->eaptls->keylen);
        break;
        //case 0x0005:
        //arc4_setup( (arc4_context *) dpconf->eaptls->ctx_enc, key1, dpconf->eaptls->keylen );
        //arc4_setup( (arc4_context *) dpconf->eaptls->ctx_dec, key2, dpconf->eaptls->keylen );
        //break;
    default:
        return -1;
    }
    return 0;
}

int addEAPCertificateVerify(unsigned char **p)
{
    int ret, n;
    unsigned char hash[36];
    unsigned int len = HAND_PRO_HDR + CLI_CERT_VER_LEN;
    unsigned char *tmp;
    rsa_context rsa;

    addEAPTLSRecord(*p, 0x16, len);
    (*p) += RECORD_LAYER_LEN;

    tmp = *p;

    **p = 0x0f;
    (*p)++;

    **p = (unsigned char)( ( len - 4 ) >> 16 );
    *(*p+1) = (unsigned char)( ( len - 4 ) >>  8 );
    *(*p+2) = (unsigned char)( ( len - 4 ) );
    (*p) += 3;

    /*
     * Make an RSA signature of the handshake digests
     */
    ssl_calc_verify( hash );

    ret = x509parse_key( &rsa, (unsigned char *) eapprikey, strlen( (char*)eapprikey ), NULL, 0 );

    if ( ret != 0 )
    {
        printf( " failed\n  !  x509parse_key returned %d\n\n", ret );

    }
    n = rsa.len;

    **p = (unsigned char)( n >> 8 );
    (*p)++;
    **p = (unsigned char)( n      );
    (*p)++;

    if ( ( ret = rsa_pkcs1_sign( &rsa, RSA_PRIVATE, RSA_RAW, 36, hash, tmp + 6 ) ) != 0 )
    {
        rsa_free(&rsa);
        return -1;
    }
    rsa_free(&rsa);
    md5_updateH( &dpconf->eaptls->fin_md5 , tmp, len );
    sha1_updateH( &dpconf->eaptls->fin_sha1, tmp, len );

    (*p) += CLI_CERT_VER_LEN - 2;
    return 0;
}

void ssl_calc_verify( unsigned char hash[36] )
{
    md5_context md5;
    sha1_context sha1;

    memcpy( &md5 , &dpconf->eaptls->fin_md5 , sizeof(  md5_context ) );
    memcpy( &sha1, &dpconf->eaptls->fin_sha1, sizeof( sha1_context ) );

    md5_finishH( &md5,  hash );
    sha1_finishH( &sha1, hash + 16 );

    return;
}

void addEAPChangeCipherspec(unsigned char **p)
{
    **p = 0x14;
    (*p)++;
    **p = 0x03;
    (*p)++;
    **p = 0x01;
    (*p)++;
    **p = 0;
    (*p)++;
    **p = 1;
    (*p)++;
    **p = 1;
    (*p)++;
}

int addEAPClientFinish(unsigned char **p)
{
    unsigned int len = HAND_PRO_HDR + 12;
    unsigned char *tmp;
    unsigned char tmpbuf[100] = {0};
    int i, padlen;

    md5_context  md5;
    sha1_context sha1;

    addEAPTLSRecord(*p, 0x16, len);
    (*p) += RECORD_LAYER_LEN;

    tmp = *p;

    memcpy( &md5 , &dpconf->eaptls->fin_md5 , sizeof(  md5_context ) );
    memcpy( &sha1, &dpconf->eaptls->fin_sha1, sizeof( sha1_context ) );

    ssl_calc_finished(dpconf->eaptls->master, tmp + 4, 1, &md5, &sha1);

    **p = 0x14;
    (*p)++;

    **p = (unsigned char)( ( len - 4 ) >> 16 );
    *(*p+1) = (unsigned char)( ( len - 4 ) >>  8 );
    *(*p+2) = (unsigned char)( ( len - 4 ) );
    (*p) += 3;

    md5_updateH( &dpconf->eaptls->fin_md5 , tmp, len );
    sha1_updateH( &dpconf->eaptls->fin_sha1, tmp, len );


    memcpy(tmpbuf, dpconf->eaptls->out_ctr, 8);
    memcpy(tmpbuf + 8 , tmp - 5, 5);
    memcpy(tmpbuf + 13, tmp, 16);

    if ( dpconf->eaptls->maclen == 20 )
        sha1_hmac( dpconf->eaptls->mac_enc, 20,
                   tmpbuf, 13 + len,
                   tmp + 16);

    len += dpconf->eaptls->maclen;

    if (dpconf->eaptls->ivlen == 0)
    {
        //arc4_crypt( (arc4_context *)
        //	tmp, 36 );
        //padlen = 0;

    }
    else
    {
        padlen = dpconf->eaptls->ivlen - ( len + 1 ) % dpconf->eaptls->ivlen;
        if ( padlen == dpconf->eaptls->ivlen )
            padlen = 0;

        for ( i = 0; i <= padlen; i++ )
            tmp[36 + i] = (unsigned char) padlen;

        len += padlen + 1;

        switch ( dpconf->eaptls->ivlen )
        {
        case 16:
            aes_crypt_cbc( (aes_context *) dpconf->eaptls->ctx_enc,
                           AES_ENCRYPT, len,
                           dpconf->eaptls->iv_enc, tmp, tmp );
            break;
        }
    }
    *(tmp - 5 + 3) = (unsigned char)( len >> 8 );
    *(tmp - 5 + 4) = (unsigned char)( len      );

    (*p) = tmp;
    (*p) += len;

    for ( i = 7; i >= 0; i-- )
        if ( ++dpconf->eaptls->out_ctr[i] != 0 )
            break;
    return 0;
}
