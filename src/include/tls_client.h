
/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	tls.h

*****************************************************************************
*/

#include "tls.h"

#ifndef _TLS_CLIENT_LIB_INCLUDED
#define _TLS_CLIENT_LIB_INCLUDED

#define RTK_SSL_DEBUG 0
/*--------------------------Definitions------------------------------------*/

#define TLS_HANDSHAKE_HEADER_LEN 4
//#define TLS_HANDSHAKE_CHELLO_LEN 85 //include HandSHAKE - HEADER
#define CLIENT_KEY_XCHG_LEN 262
/*-----------------------------enum------------------------------------*/
typedef enum {
	TLS_CONNECT_START = 0,
	TLS_CONNECT_CLIENT_HELLO = 1,
	TLS_CONNECT_SERVER_HELLO = 2,		
	TLS_CONNECT_SERVER_CERTIFICATE = 3,
	TLS_CONNECT_SERVER_HELLODONE = 4,		
	TLS_CONNECT_CLIENT_FINISH = 5,
	TLS_CONNECT_SERVER_CHANGE_CIPHER_SPEC = 6,
	TLS_CONNECT_SERVER_FINISH = 7,

	TLS_CONNECT_SUCCESS = 10,
	TLS_CONNECT_FAIL = 20,
}TLS_CONNECT_STATUS;

typedef enum {
	TLS_RESULT_SUCCESS = 0,
	TLS_RESULT_TCP_CONN_FAIL = 1,
	TLS_RESULT_TIMEOUT = 2,
	TLS_RESULT_FAIL_SERVER_HELLO = 3,
	TLS_RESULT_FAIL_PARSE_PKT = 4,
	TLS_RESULT_FAIL_SERVER_FINISH = 5,
	TLS_RESULT_FAIL_BUFFER_ALLOC = 6,
	
	TLS_RESULT_FAIL = 99,
}TLS_CONNECT_RESULT;

/*-----------------------------Structures------------------------------------*/
#pragma pack(1) //set struct alignment to 1 byte


typedef struct _recordLayerHeader
{
	unsigned char type;
	unsigned short version;	
	unsigned short length;	
}recordLayerHeader;



typedef struct _handshakeHeader
{
	unsigned char type;
	unsigned char length[3];	
}handshakeHeader;

typedef struct _clientHello
{
	handshakeHeader header;
	unsigned short version;//0x0103 : (0x0301)
	unsigned char client_random[32];
	unsigned char SessionIDLen;
	unsigned short cipherSuitsLen;// 2 
	unsigned short cipherSuits; // ©T©w : 0x2f : TLS_RSA_WITH_AES_128_CBC_SHA
	unsigned char compressMethLen;
	unsigned char compressMeth;// ©T©w : 0x00 
	//unsigned char compressMeth2;// ©T©w : 0x00 
	//unsigned short expressionLen;
}clientHelloStruct;



//serverHelloStruct
//only first 35 bytes , because the remnant part is abritrary
typedef struct _serverHello
{
	unsigned short version;//0x0103 : (0x0301)
	unsigned char server_random[32];
	unsigned char SessionIDLen;
}serverHelloStruct;



typedef struct __preMasterSecret{
	unsigned short client_version;
	unsigned char random[46];	
}preMasterSecretStruct;


// tlsDataStruct : This is for record the information of TLS procedure.
typedef struct _tlsData{
	unsigned short tlsClientVersion;
	unsigned short cipherSuit;
	unsigned char clientRandom[SSL_RANDOM_SIZE];
	unsigned char serverRandom[SSL_RANDOM_SIZE];	
	unsigned char sessionIDLen;
	unsigned char *sessionId;
	//unsigned char masterSectet[48];
	//unsigned char *serverCrt;
	//int serverCrtLen;
	//preMasterSecretStruct preMaster;
	x509_cert *serverCrt;
	#ifdef DEBUG_MD5_SHA1
	unsigned char *handshake_message;//Originally , this for record the handshake message (for hash) , but it is unnecessary now.
	#endif
//	int hskMsgLen;
	
	//int keylen;                         /*!<  symmetric key length    */    
	//int minlen;                         /*!<  min. ciphertext length  */    
	//int ivlen;                          /*!<  IV length               */    
	//int maclen;                         /*!<  MAC length              */

	//unsigned char mac_enc[32];          /*!<  MAC (encryption)        */    
	//unsigned char mac_dec[32];          /*!<  MAC (decryption)        */
	//unsigned char iv_enc[16];           /*!<  IV (encryption)         */    
	//unsigned char iv_dec[16];           /*!<  IV (decryption)         */
	//unsigned long ctx_enc[128];         /*!<  encryption context      */    
	//unsigned long ctx_dec[128];         /*!<  decryption context      */
	//SSL *sslc;
}tlsDataStruct;




/*--------------------------Function------------------------------------*/

void initTLS();
unsigned int connectToTlsServer(RTSkt *skt);
int clientInitKey(struct tcp_pcb *pcb);
void closeConnectToTlsServer(RTSkt *skt);
void md5_singleString(unsigned char *src,int srcLen , unsigned char *dst,SSL *pSsl);
void buildHandShake_Msg(unsigned char *buff,int len,SSL *pSsl);
void rtkRandom(unsigned char *buffer,int len);

void sendTLSBuffer(unsigned char *p,int len, struct tcp_pcb *pcb);
PKT* readTLSBuffer (struct tcp_pcb *pcb);
TLS_CONNECT_RESULT parseServerFinish(unsigned char *p, int len, struct tcp_pcb *pcb);

/*-----------------------------extern Variable------------------------------------*/
extern tlsDataStruct m_tlsData;//+briank.rtk

/*-----------------------------extern finction------------------------------------*/



#endif
