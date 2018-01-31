/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	http.h

*****************************************************************************
*/

#ifndef _HTTP_INCLUDED
#define _HTTP_INCLUDED
#include "tcp.h"

//#define  WEB_SKT_STYLE

/*--------------------------Definitions------------------------------------*/

#define	MIN_REQ_LEN	16		/* "GET / HTTP/1.1\n\n"		*/
#define MAX_HTTPMSG_LEN 256
#define MAX_WEBPAGE_PATH_LEN 128
#define PASS	1
#define UNPASS	0
//Http Method
#define WEBGET		1
#define WEBPOST		2
#define WSMANPOST	3

#define WSMANSRVPORT	623

#define HTTP_AUTH_MAX  6


/*-----------------------------Structures------------------------------------*/


//Http Athu Method
#define Basic 0
#define Digest 1
#define Kerb 2
#if 0
struct digetItem
{
    char *username;
    char *realm;
    char *password;
    char *method;
    char *digestURI;
    char *nonce;
    char *noncecount;
    char *clientNonce;
    char *qop;
};
#endif

typedef struct _HTTPHdrInfo
{
	unsigned char url[32];//Tmp
	UserInfo* uInfo;
	unsigned char *httpDataPos;
	unsigned char *httpCharSet;
	unsigned short HttpHdrLen;
	unsigned short httpContentLen;
	unsigned char httpMethod:2, httpAuthMethod:2, httpAuthPass:1,
	invaildPost:1;
	unsigned char httpParseStatus:2;
}HTTPHdrInfo;


typedef struct http_Authorization {
	
	unsigned char nonce[32];
	unsigned char opaque[16];
          unsigned int nc;	
	unsigned int time;
	unsigned char pass;
	
}HTTPAuthorization;
	


/*-----------------------------Functions------------------------------------*/
void initTCPService();
int httpRx(void *arg, int len, struct tcp_pcb *pcb);
void httpGet(void *pData, int len, struct tcp_pcb *pcb);
void httpPost(void *pData, int len, struct tcp_pcb *pcb);
int send_http_errorMSG(struct tcp_pcb *pcb, int code, char* phrase, char *msg, int plen,int mlen);
int	parsePostRequest(char *p, int len, struct tcp_pcb *pcb);
int getHttpHeaderInfo(unsigned char* data, unsigned int len, struct tcp_pcb *pcb, HTTPHdrInfo* hinfo);
void ws_input(TCPPCB* pcb,char *px,int len);
void telnetAccept(TCPPCB* pcb);
void webSrv(void *data);
void webPost(struct tcp_pcb *pcb, char *p, int len);
void webEZshareSrvThread(void *data);

int setAuthCredential(unsigned char *nonce  , unsigned char*opaque);


#endif
