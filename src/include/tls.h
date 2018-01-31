/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	tls.h

*****************************************************************************
*/

#ifndef _SSL_LIB_INCLUDED
#define _SSL_LIB_INCLUDED

#include "rsa.h"
#include "md5.h"
#include "sha1.h"
#include "lib.h"
#include "x509.h"
#include <sys/inet_types.h>

/*--------------------------Definitions------------------------------------*/
#define SSL_RANDOM_SIZE	32
#define SESSION_ID_LEN	32
#define TLS_SHELLO_SIZE 74
#define MAX_KEY_BYTE_SIZE 512  /* for a 4096 bit key */
#define TLS_RECORD_SIZE	5

#define SFINISH	0
#define CFINISH	1
#define VERIFY_DATA_LEN 12


#define SSL_MAX_CONTENT_LEN 16384

//Cipher suite
#define SSL_RSA_RC4_128_SHA 0x5
#define TLS_RSA_WITH_AES_128_CBC_SHA 0x2F
#define TLS_PSK_WITH_AES_128_CBC_SHA 0x8c
#define TLS_FRAGMENT_PKT -5

enum {HMAC_RC4, HMAC_MD5, HMAC_SHA1};

typedef struct _EAPRECLAYER
{
	unsigned char ct;
	unsigned short size;
}EAPRECLAYER;
typedef struct _EAPTLS
{
	unsigned char eap;
	unsigned char eapid;
	char *identity; 
	char *password;
	unsigned int totallen;
	unsigned char *buf;
	unsigned char *sendbuf;
	unsigned int sendbuflen;
	unsigned char *recvbuf;
	unsigned int recvbuflen;
	int keylen;                         /*!<  symmetric key length    */    
	int minlen;                         /*!<  min. ciphertext length  */    
	int ivlen;                          /*!<  IV length               */    
	int maclen;                         /*!<  MAC length              */
	md5_context fin_md5;               /*!<  Finished MD5 checksum   */	
	sha1_context fin_sha1;              /*!<  Finished SHA-1 checksum */
	unsigned char clientRandom[32];		/* client's random sequence */		
	unsigned char serverRandom[32];		/* server's random sequence */
	unsigned char iv_enc[16];           /*!<  IV (encryption)         */    
	unsigned char iv_dec[16];           /*!<  IV (decryption)         */    
	unsigned char mac_enc[32];          /*!<  MAC (encryption)        */    
	unsigned char mac_dec[32];          /*!<  MAC (decryption)        */    
	unsigned long ctx_enc[128];         /*!<  encryption context      */    
	unsigned long ctx_dec[128];         /*!<  decryption context      */
	unsigned char sessionID[32];
	unsigned short cipher;				/*!< chosen cipher      */
	x509_cert *server_cert;               /*!<  peer X.509 cert chain   */
	x509_cert *cli_cert;                /*!<  own X.509 certificate   */
	int major_ver;          /*!< max. major version from client   */    
	int minor_ver;          /*!< max. minor version from client   */	
	unsigned char out_ctr[8];
	unsigned char in_ctr[8];
	unsigned char master[48];
	EAPRECLAYER rec;
	unsigned char *keymaterial;
}EAPTLS;
//#define TLS_FLASH_DATA_ADDRESS 0x81180000
/*-----------------------------Structures------------------------------------*/
/* handshaking types */

enum
{
    TLS_DECODE_FAIL_IV_LEN = -5,	
    TLS_DECODE_FAIL_MAC_2 = -4,
    TLS_DECODE_FAIL_MAC_1 = -3,
    TLS_DECODE_FAIL_PKT_LEN = -2,
    TLS_DECODE_FAIL_RECORD_SIZE = -1,
    TLS_DECODE_SUCCESS = 0
		
};

enum
{
    HS_HELLO_REQUEST = 0,
    HS_CLIENT_HELLO,
    HS_SERVER_HELLO,
    HS_CERTIFICATE = 11,
    HS_SERVER_KEY_XCHG,
    HS_CERT_REQ,
    HS_SERVER_HELLO_DONE,
    HS_CERT_VERIFY,
    HS_CLIENT_KEY_XCHG,
    HS_FINISHED = 20,
    HS_UNDEFINED = 100,
    HS_COMPLETED
};

enum
{
    RECORD_CHANGE_CIPHER_SPEC = 20,
    RECORD_ALERT_PROTOCOL,
    RECORD_HANDSHAKE_PROTOCOL,
    RECORD_APP_PROTOCOL_DATA, 
    RECORD_TOO_LARGE = 100,
    RECORD_TOO_SMALL,
    RECORD_COMPLETED
};

typedef struct _RecordLayer
{
	  unsigned char unUsed;
    unsigned char contentType;
    unsigned char mVer;
    unsigned char lVer;
    unsigned short size;
}RecordLayer;

typedef struct _SSL{
    struct _SSL *next;
    struct _PKT* cPkt;
    struct _PKT* hPkt;
    int keyLen;                         /*!<  symmetric key length    */
    int minLen;                         /*!<  min. ciphertext length  */
    int ivLen;                          /*!<  IV length               */
    int macLen;                         /*!<  MAC length              */

    //Server use SESSION_ID_LEN for session ID length
    unsigned char sessionID[SESSION_ID_LEN];
    unsigned char ivEnc[16];           /*!<  IV (encryption)         */
    unsigned char ivDec[16];           /*!<  IV (decryption)         */
    unsigned char macEnc[32];          /*!<  MAC (encryption)        */
    unsigned char macDec[32];          /*!<  MAC (decryption)        */
    unsigned char ctxEnc[16];         /*!<  encryption context      */
    unsigned char ctxDec[16];         /*!<  decryption context      */


    unsigned char clientRandom[SSL_RANDOM_SIZE]; /* client's random sequence */
    unsigned char serverRandom[SSL_RANDOM_SIZE]; /* server's random sequence */
    unsigned char master[PMSLEN]; //Decode master
    RecordLayer recordLayer;

    md5_context	finMd5;
    sha1_context finSha1;

    INT8U cipher;
    char resume:1, httpAuthPass:1;
    unsigned char inCounter[8];
    unsigned char outCounter[8];
    unsigned short crLen; 
	unsigned short pktsmallLen; 
	unsigned char *pktsmall;
	unsigned char fragpkt;
    char clientStage;
    char handshakeStage;
    x509_cert *own_cert;                /*!<  own X.509 certificate   */    
    x509_crl *ca_crl;                   /*!<  trusted CA CRLs         */    
    x509_cert *peer_cert;               /*!<  peer X.509 cert chain   */
    unsigned short coalesceLen;
}SSL;

/*-----------------------------Functions------------------------------------*/
unsigned char* getCertificate(int* size);
int doTLSAccept(unsigned char *p, int len, ptcp_pcb pcb);
int doServerHandshake(unsigned char *p, int len, ptcp_pcb pcb);
int processClientHello(unsigned char *p, int len, ptcp_pcb pcb);
int sendServerHelloSequence(ptcp_pcb pcb);
void makeTLSRecordLayer(unsigned char* buf, char ct, int size, ptcp_pcb pcb);
int serverHello(unsigned char *p, ptcp_pcb pcb);
int serverCertificate(unsigned char* buf, int dbytelen, int datasize, ptcp_pcb pcb,unsigned char* certificate);
void serverHelloDone(unsigned char* p, ptcp_pcb pcb);
int processClientKeyExchange(unsigned char *p, int len, ptcp_pcb pcb);
int parseChangeCipherSpec(unsigned char *p, int len, ptcp_pcb pcb);
int parseClientFinish(unsigned char *p, int len, ptcp_pcb pcb);
int checkPadLen(int ivLen, int outMsgLen);
int tlsEncodeData(PPKT pkt, int len, ptcp_pcb pcb, int type);
void tlsPktSend(struct tcp_pcb *pcb, struct _PKT *pkt);
int sendServerFinish(ptcp_pcb pcb);
int deCodeTLSData(unsigned char *p, int len, struct _PKT *pktList, ptcp_pcb pcb, int type);
void sendTLSWebPage(unsigned char *buf, int offset, ptcp_pcb pcb);
int parseClientFinish(unsigned char *p, int len, ptcp_pcb pcb);
void ssl_calc_finished(unsigned char *master, unsigned char *buf, int from, md5_context *md5, sha1_context *sha1);
void authedSSL(unsigned char* sessionID);
void tls_test();
void sendTLSAlert(ptcp_pcb pcb);
void removeSSLSession(unsigned char *p, ptcp_pcb pcb);
#if CONFIG_VERSION < IC_VERSION_EP_RevA
void (*deCodeTLSData_F)(unsigned char *p, int len, struct _PKT *pktList, ptcp_pcb pcb, int type);
int  (*doTLSAccept_F)(INT8U *p, int len, ptcp_pcb pcb);
#endif
#endif
