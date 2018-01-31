/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	TLS.c

Abstract:	TLS module

*****************************************************************************
*/

/*--------------------------Include File------------------------------------*/
#include <time.h>
#include <stdlib.h>
#include <string.h>
//#include "tls.h"
#include "http.h"
#include "aes.h"
#include "bsp.h"
#include "x509.h"
#include "soap.h"
#include "tls_client.h"
#include "lib.h"
#include "md5.h"


/*--------------------------Global Parameter------------------------------------*/
char m_serverIP[4];
int m_serverPort;
tlsDataStruct m_tlsData;
TLS_CONNECT_STATUS m_tlsConnStatus;
int m_serverCertiLen;
//struct tcp_pcb *m_pcb; 
//unsigned char m_tlsMultiPkt;
//int m_tlsTotalLen;
unsigned char *m_pReleaseSSL;
/*--------------------------Define------------------------------------*/
//#define DEBUG_MD5_SHA1 1
//#define DEBUG_RANDOM 1
//#define DEBUG_RSA 1
/*--------------------------Function------------------------------------*/
int sendClientFinish(struct tcp_pcb *pcb);
#if 0
void allocDebug_tlsclient(){
	unsigned char *debugBuff1,*debugBuff2,*debugBuff3,*debugBuff4;

	debugBuff1 = malloc(200);
	debugBuff2 = malloc(200);
	debugBuff3 = malloc(200);
	debugBuff4 = malloc(200);
	printf("tlsclient:");
	printf("debugBuff1=0x%x ,",debugBuff1);
	printf("debugBuff2=0x%x ,",debugBuff2);
	printf("debugBuff3=0x%x ,",debugBuff3);
	printf("debugBuff4=0x%x ,",debugBuff4);
	printf("\n");

	free(debugBuff1);
	free(debugBuff2);
	free(debugBuff3);
	free(debugBuff4);

}



void printFreeMemAddrALL_tlsclient(int id,int idx){
	//This functin is used  to check if there are ant duplicate memory block in the list.
	//int idx,len;
	char str[32];
	int count,fdx,size;
	char *buff[20];
	//char *st;

	//debug_OS_MEM *debugOSMemTbl;	
	//debugOSMemTbl = (debug_OS_MEM *)0x8021a34c;
	
	memset(str,0x00,32);
	count = OSMemTbl[idx].OSMemNFree;
	size = OSMemTbl[idx].OSMemBlkSize;
	printf("(%d)FreeAddr[0x%x],free size(%d):",id,size,count);
	for(fdx=0;fdx<count;fdx++){
		buff[fdx] = malloc(size);
		printf("[%x]-",buff[fdx]);
	}
	printf("\n");

	for(fdx=0;fdx<count;fdx++){
		free(buff[fdx]);
	}
	printf("(%d)freSize = %d\n",id,OSMemTbl[idx].OSMemNFree);
	
}

void printFreeMemAddr_tlsclient(int id,int idx){
	
	//int idx,len;
	char str[32];
	//char *st;

	//debug_OS_MEM *debugOSMemTbl;	
	//debugOSMemTbl = (debug_OS_MEM *)0x8021a34c;
	
	memset(str,0x00,32);
	printf("[%d]tls_client:freeList[%d]%x , (%d)\n",id,idx,(INT32U)OSMemTbl[idx].OSMemFreeList,OSMemTbl[idx].OSMemNFree);
}

void printFreeMemAll_tlsclient(int id){

	int idx,len;
	char str[32];
	char *st;

	//debug_OS_MEM *debugOSMemTbl;	
	//debugOSMemTbl = (debug_OS_MEM *)0x8021a34c;
	
	memset(str,0x00,32);	
	st = str;
	len = sprintf(st,"[%d]",id);
	st+=len;
	for(idx=0;idx<OS_MAX_MEM_PART;idx++){		
		len = sprintf(st,"%d-",OSMemTbl[idx].OSMemNFree);		
		st+=len;
	}
	sprintf(st,"\n");	
	printf(str);

}

#endif

#if 0
void rtk_md5Test(){
	char *md5buffer;
	unsigned char result[16];
	
	int idx;

	
	//--------------------
	md5_starts(&m_tlsData.sslc->finMd5);	
	//--------------------	
	md5buffer = malloc(350);
	for(idx=0;idx<350;idx++){
		md5buffer[idx] = idx%0xFF;
	}
	//-------------------
	for(idx=0;idx<350;idx+=50){
		md5_updateH( &m_tlsData.sslc->finMd5 , md5buffer+idx,50);
	}
	md5_finishH(  &m_tlsData.sslc->finMd5 , result );

	printf("[MD5]1:");
	for(idx=0;idx<16;idx++){
		printf("%x-",result[idx]);
	}
	printf("\n");


	
		//--------------------
		md5_starts(&m_tlsData.sslc->finMd5);	
		//--------------------	
		md5buffer = malloc(350);
		for(idx=0;idx<350;idx++){
			md5buffer[idx] = idx%0xFF;
		}
		//-------------------
		for(idx=0;idx<350;idx+=70){
			md5_updateH( &m_tlsData.sslc->finMd5 , md5buffer+idx,70);
		}
		md5_finishH(  &m_tlsData.sslc->finMd5 , result );
	
		printf("[MD5]2:");
		for(idx=0;idx<16;idx++){
			printf("%x-",result[idx]);
		}
		printf("\n");
	OSTimeDly(OS_TICKS_PER_SEC * 500);	
}
#endif
void rtkRandom(unsigned char *buffer,int len){

	memset(buffer,0x66,len);//just for test


}

PKT* readTLSBuffer (struct tcp_pcb *pcb){

	PKT* rxPkt=NULL;
	int status=0,sslLen=0;
	unsigned char *buffer=NULL;
	int ret = 0;
	//unsigned char *debugPkt;
	//char debugiv[16];
	
	//rxPkt = rtSktRx_F(pcb->skt, 100, &status);
	while ((rxPkt = rtSktRx_F(pcb->skt,100,&status)) == 0){
		if (status == SKTDISCONNECT){
		 	printf("[RTK]connectToTlsServer : SKTDISCONNECT\n");
			return NULL;
		}else if (status == OS_ERR_TIMEOUT){
		 	//printf("[RTK]connectToTlsServer : OS_ERR_TIMEOUT\n");
			//return NULL;
		}
	}
	buffer = rxPkt->wp;
	sslLen = 0;
	
	
	
	if(buffer[0] == 0x17){//(0x17)Contect type : Application Data		
	
		
		sslLen = buffer[3]*256 + buffer[4];		
		//printf("[RTK] SSL len = %d(in tls header)\n",sslLen);		          
		pcb->ssl->recordLayer.size = sslLen;		
		ret = deCodeTLSData(buffer+TLS_RECORD_SIZE , sslLen , 0,pcb,RECORD_APP_PROTOCOL_DATA);//pcb->skt-
		if(ret != 0){
			printf("[Error]readTLSBuffer decode Fail,ret=%d\n",ret);
		}else{
			printf("[Error]readTLSBuffer decode Success\n");
		}
	}
	
	
	return rxPkt;
}


void createClientPCBSSL(struct tcp_pcb *pcb)
{
	pcb->ssl = createSSL();
	if(pcb->ssl != NULL){
		pcb->ssl->next = 0x00;
		md5_starts(&pcb->ssl->finMd5);
   		sha1_startsH(&pcb->ssl->finSha1);	
	}else{
		printf("Create SSL Fail\n");
	}
}


void sendTLSBuffer(unsigned char *p,int len, struct tcp_pcb *pcb){

	//debug-----------------------------------------
	/*	
		printf("[ivDev]");
		for(idx=0;idx<16;idx++){
			printf("-%x" , pcb->ssl->ivEnc[idx]);		
		}
		printf("\n");
	*/	
	//----------------------------------------------
		
	sendTLSWebPage(p , len , pcb);


	
}

/**
* Description: Parse ServerFinish
*
*
* Parameters:
*				P -> start address to fill server finish content
*				pcb -> Connection
*
* Returns:		None
*
*/

TLS_CONNECT_RESULT parseServerFinish(unsigned char *p, int len, struct tcp_pcb *pcb)
{
	int i;
	
	aes_crypt_cbc( (aes_context*) pcb->ssl->ctxDec, AES_DECRYPT , len , pcb->ssl->ivDec, p, p);	
	#ifdef RTK_SSL_DEBUG
	/*
	printf("[ServerFinishDecrypt]:");
	
	for(idx=0;idx<48;idx++){
			printf("%x-",p[idx]);
	}	
	printf("\n");
	*/

 	for ( i = 7; i >= 0; i-- )
        if ( ++pcb->ssl->inCounter[i] != 0 )
            break;

	
	#endif

	if(p[0] == HS_FINISHED){
		if(p[3] == 0x0c){
			return TLS_RESULT_SUCCESS;
		}
	}
	return TLS_RESULT_FAIL;
}



// initSSL : Determine the key, IV and MAC length.
void initSSL(unsigned short ciphersuit , SSL *ssl){

	if(ssl->cipher != TLS_RSA_WITH_AES_128_CBC_SHA){
		ssl->cipher = TLS_RSA_WITH_AES_128_CBC_SHA; //+briank.rtk
	}
	
    switch (ssl->cipher)
    {
        //minLen = max(keyLen, ivLen, macLen) ???
    case SSL_RSA_RC4_128_SHA:
        ssl->keyLen = 16;
        ssl->minLen = 20;
        ssl->ivLen = 0;
        ssl->macLen = 20;
        break;
    case TLS_RSA_WITH_AES_128_CBC_SHA:
        ssl->keyLen = 16;
        ssl->minLen = 20;
        ssl->ivLen = 16;
        ssl->macLen = 20;
        break;
    }
}
void initTLS(int ipv4,int port){
	//m_serverIP[0] = 0xC0;//192
	//m_serverIP[1] = 0xA8;//168
	//m_serverIP[2] = 0x00;//0
	//m_serverIP[3] = 0x16;//22	
	//m_tlsMultiPkt = 0;
	
	#if 1
	//m_serverIP[0] = 0x41;//65
	//m_serverIP[1] = 0x36;//54
	//m_serverIP[2] = 0xba;//186
	//m_serverIP[3] = 0x6b;//107	
	#else
	m_serverIP[0] = 0xAC;//172
	m_serverIP[1] = 0x15;//21
	m_serverIP[2] = 0x47;//71
	m_serverIP[3] = 0xAD;//173	
	#endif

	m_serverIP[0] = ipv4%256;//65
	ipv4 = ipv4 >> 8;
	m_serverIP[1] = (ipv4)%256;//54
	ipv4 = ipv4 >> 8;
	m_serverIP[2] = (ipv4)%256;//186
	ipv4 = ipv4 >> 8;
	m_serverIP[3] = (ipv4)%256;//107
	
	m_serverCertiLen = 0;
	m_serverPort = port;

	memset(&m_tlsData,0x00, sizeof(tlsDataStruct));

	m_tlsData.cipherSuit = 0x2f00;
	m_tlsData.tlsClientVersion = 0x0103;

	//printf("[RTK] sizeof(x509_cert) length = %d\n",sizeof(x509_cert));
	m_tlsData.serverCrt = (x509_cert *)malloc(sizeof(x509_cert));
	if(m_tlsData.serverCrt == NULL){
		printf("[RTK]m_tlsData.serverCrt == NULL\n");
	}
    memset( m_tlsData.serverCrt , 0, sizeof( x509_cert ) );

	
#ifdef DEBUG_MD5_SHA1
	m_tlsData.handshake_message = malloc(1500);
	if(m_tlsData.handshake_message == NULL){
		printf("[RTK] init TLS:not enough buffer for handshake_message\n");	
	}	
	memset(m_tlsData.handshake_message,0x00,1500);
#endif	
	//m_tlsData.hskMsgLen = 0;

	m_tlsConnStatus = TLS_CONNECT_START;
		
	//m_tlsData.keylen = 16;
    //m_tlsData.minlen = 20;
    //m_tlsData.ivlen = 16;
    //m_tlsData.maclen = 20;
}

//Function : buildHandShake_Msg
//Description : Copy HandshakeProtocol packet (without record layer) for "ClientFinish"
//                    
void buildHandShake_Msg(unsigned char *buff,int len,SSL *pSsl){

	//memcpy(m_tlsData.handshake_message + m_tlsData.hskMsgLen ,buff, len);	
	//m_tlsData.hskMsgLen+=len;

	
	//md5_updateH( &m_tlsData.sslc->finMd5 , buff, len );
	//sha1_updateH( &m_tlsData.sslc->finSha1, buff, len );
	md5_updateH( &pSsl->finMd5 , buff, len );
	sha1_updateH( &pSsl->finSha1, buff, len );

	
//OSTimeDly((OS_TICKS_PER_SEC/2));
	//printf("[RTK] copy Hsk_Msg (%d) \n",len);
	
}

//wp is the start address of data segment of the packet.
void buildClientHelloPkt(unsigned char *wp){ 
	int size, clientHelloSize;	
	clientHelloStruct *pClienthello;
	char *tmpPoint;
	
	pClienthello = (clientHelloStruct *)(wp + TLS_RECORD_SIZE);
	clientHelloSize = sizeof(clientHelloStruct);
		
	pClienthello->client_random[0] = 0x50;
	pClienthello->client_random[1] = 0x80;
	pClienthello->client_random[2] = 0xb8;

	memcpy(m_tlsData.clientRandom,pClienthello->client_random,32);
	
	
	size = clientHelloSize - sizeof(handshakeHeader);	
	tmpPoint = (unsigned char *)(&size);
	
	pClienthello->header.length[2] = tmpPoint[0];
	pClienthello->header.length[1] = tmpPoint[1];
	pClienthello->header.length[0] = tmpPoint[2];
		
	pClienthello->header.type = HS_CLIENT_HELLO;
		
	
	pClienthello->version = m_tlsData.tlsClientVersion;
	
	pClienthello->SessionIDLen = 0x00;
	
	
	pClienthello->cipherSuitsLen = ntohs(0x0002);
	pClienthello->cipherSuits = m_tlsData.cipherSuit;
	pClienthello->compressMethLen=0x01;
	pClienthello->compressMeth=0x00;

	//buildHandShake_Msg(pClienthello,clientHelloSize);
}

//rfc2246 chapter 7.4.9: A finished msg is always set immediately after a change cipher spec.
TLS_CONNECT_RESULT parseServerFinishPkt(PKT * pkt,RTSkt *skt){
	unsigned char *bufPos;
	recordLayerHeader *pRecdLayer;
	int recordLayerLen,pktLen;
	
	bufPos = pkt->wp;
	pktLen = pkt->len;


	while(pktLen > 0){
		pRecdLayer = (recordLayerHeader *)bufPos;
		recordLayerLen = ntohs(pRecdLayer->length);		
		bufPos = bufPos + TLS_RECORD_SIZE;
		
		if(pRecdLayer->version != 0x0103){
			return TLS_RESULT_FAIL;	
		}
		
		if((pRecdLayer->version == 0x0103)&&(pRecdLayer->type == RECORD_CHANGE_CIPHER_SPEC)){
			
			if(*bufPos == 0x01){
				m_tlsConnStatus = TLS_CONNECT_SERVER_CHANGE_CIPHER_SPEC;
			}
			bufPos+=1;
		}else if((pRecdLayer->version == 0x0103)&&(pRecdLayer->type == RECORD_HANDSHAKE_PROTOCOL)){
		
			if(m_tlsConnStatus == TLS_CONNECT_SERVER_CHANGE_CIPHER_SPEC){
				//this must be ServerFinish , and this is encrypted
				//printf("[SERVER FINISH]OKOK\n");

				if(parseServerFinish(bufPos,recordLayerLen,skt->pcb) == TLS_RESULT_SUCCESS){
					return TLS_RESULT_SUCCESS;
				}else{
					return TLS_RESULT_FAIL_SERVER_FINISH;
				}
			}else{
				bufPos = bufPos + recordLayerLen;
			}			
		}else{
			bufPos = bufPos + recordLayerLen;
		}
		pktLen -=(TLS_RECORD_SIZE+recordLayerLen); 
	}

	//printf("[RTK] - parseServerFinishPkt\n");
	return TLS_RESULT_SUCCESS;	
	
}
#if 1

TLS_CONNECT_RESULT parseHandshake_ServerHelloDone(unsigned char *pBuf,int handshakeLen , int pktLen , SSL *sslSt){
	
	//printf("[RTK] + parseHandshake_ServerHelloDone.[%x][%x][%x][%x]\n",pBuf[0],pBuf[1],pBuf[2],pBuf[3]);
	buildHandShake_Msg((unsigned char *)pBuf,handshakeLen+4,sslSt);
	m_tlsConnStatus = TLS_CONNECT_SERVER_HELLODONE;


	return TLS_RESULT_SUCCESS;
}
//receive certificate at next packet.
TLS_CONNECT_RESULT recvLeftCertificate(RTSkt *rxSkt , unsigned char *certBuf,int singleLeft, int totalLeft ,SSL *sslSt){
	PKT* rxPkt = NULL;
	int status = 0;
	int finishLen = 0;
	unsigned char *certi = certBuf;

	//printf("[RTK]+ recvLeftCertificate,total left=%d, singleLeft =%d\n",totalLeft,singleLeft);		
	while(totalLeft > 0){	
		
		//printf("[RTK]+ recvLeftCertificate,m_pcb=0x%x, m_pcb->skt =0x%x\n",m_pcb,m_pcb->skt);		
		//while ((rxPkt = rtSktRx_F(m_pcb->skt,100,&status)) == 0){
		while ((rxPkt = rtSktRx_F(rxSkt,100,&status)) == 0){
		//printf("[RTK]+ recvLeftCertificate,total left=%d, singleLeft =%d\n");			
			 if (status == SKTDISCONNECT){
			 	printf("[RTK]recvLeftCertificate : SKTDISCONNECT\n");
				return TLS_RESULT_FAIL_SERVER_HELLO;
			 }else if (status == OS_ERR_TIMEOUT){
			 	//printf("[RTK]recvLeftCertificate : OS_ERR_TIMEOUT\n");
				//return TLS_RESULT_TIMEOUT;
			 }else{
				 if(rxPkt != NULL ){
				 	//printf("[RTK]recvLeftCertificate : recv len %d\n",rxPkt->len);
				 }else{
				 	//printf("[RTK]recvLeftCertificate : recv NULL , status=0x%x\n",status);
				 }
			 
			}
			 
		}
		//printf("[RTK] recv one packet len=%d,singleLeft=(%d),m_pcb=(0x%x)\n",rxPkt->len,singleLeft,m_pcb);

		if(singleLeft > 0){
			if(rxPkt->len >= singleLeft){
				//buildHandShake_Msg((unsigned char *)rxPkt->wp , singleLeft );
				memcpy(certi,rxPkt->wp,singleLeft);				
				//printf("Recv ALL Certificate ...(%d)\n",singleLeft);
				singleLeft = 0;
			}else{
				//buildHandShake_Msg((unsigned char *)rxPkt->wp , rxPkt->len );				
				memcpy(certi,rxPkt->wp,rxPkt->len);
				singleLeft -= rxPkt->len;
				certi+=rxPkt->len;
			}
		}
			
		//printf("totalLeft=%d\n",totalLeft);
		
		if(totalLeft >= rxPkt->len){			
			buildHandShake_Msg((unsigned char *)rxPkt->wp , rxPkt->len ,sslSt);
		 	totalLeft -=rxPkt->len;
		}
		else{
			buildHandShake_Msg((unsigned char *)rxPkt->wp , totalLeft ,sslSt);
			finishLen = rxPkt->len - totalLeft;
			//Assume the next handshake is ServerHelloDone !!! . This shuld be update later . 
			parseHandshake_ServerHelloDone(rxPkt->wp + totalLeft , 0 , finishLen , sslSt);			
		 	totalLeft = 0;	 
		}
		//printf("[RTK]+recvLeftCertificatefree rxPkt\n");
		if(rxPkt)
			freePkt_F(rxPkt);
		//printf("[RTK]-recvLeftCertificatefree rxPkt\n");
	}
	//printf("[RTK]- recvLeftCertificate\n");
	
	return 	TLS_RESULT_SUCCESS;
}

TLS_CONNECT_RESULT parseHandshake_ServerHello(unsigned char *pBuf,int handshakeLen , int pktLen , SSL *sslSt){
	//pktLen : include the whole Handshake Header(4 bytes)
	serverHelloStruct *serverhello;
	unsigned char *bufPos = pBuf+4;// 4bytes for Handshake Header ,  2 bytes for Handshake Version

	//printf("[RTK] + parseHandshake_ServerHello\n");
	
	buildHandShake_Msg((unsigned char *)pBuf,handshakeLen+4,sslSt);
	//printf("[RTK] Parse TLS_CONNECT_SERVER_HELLO , len=%d\n",handshakeLen);
	serverhello = (serverHelloStruct *)bufPos;
	bufPos = bufPos + sizeof(serverHelloStruct);
	if(serverhello->version != m_tlsData.tlsClientVersion){
			printf("[RTK][ERROR] version not match\n");
			return TLS_RESULT_FAIL_SERVER_HELLO;
	}
	//copy ServerRandom-----------------------------------------------
	memcpy(m_tlsData.serverRandom,serverhello->server_random,32);
	//copy Session ID---------------------------------------------------
	m_tlsData.sessionIDLen = serverhello->SessionIDLen;
	//printf("[RTK]m_tlsData.sessionIDLen=%d\n",m_tlsData.sessionIDLen);
	m_tlsData.sessionId = malloc(m_tlsData.sessionIDLen);								
	if(m_tlsData.sessionId == NULL){
		printf("[RTK]ALLoc m_tlsData.session Fail(%d)\n",m_tlsData.sessionIDLen);
		return TLS_RESULT_FAIL;
	}
	memcpy(m_tlsData.sessionId,bufPos,m_tlsData.sessionIDLen);
	//----------------------------------------------------------------
	bufPos += m_tlsData.sessionIDLen;				
	//compare cipher suite
	if(memcmp(bufPos,&(m_tlsData.cipherSuit),2) != 0){
		printf("[RTK][ERROR] cipher suite not match\n");
		return TLS_RESULT_FAIL_SERVER_HELLO;
	}
	
	initSSL(m_tlsData.cipherSuit,sslSt);		
	m_tlsConnStatus = TLS_CONNECT_SERVER_HELLO;
	return TLS_RESULT_SUCCESS;
	
}

TLS_CONNECT_RESULT parseHandshake_ServerCertificate(RTSkt *rxSkt,unsigned char *pBuf,int handshakeLen , int pktLen , SSL *sslSt){
	//pktLen : include the whole Handshake Header(4 bytes)
	//pBuf  : point to the start of Handshake Header 
	int ret=0;
	char *crtBuf,*bufPos;
	//char *certificateBuf;
	int certiftLen_total = 0 , certiftLen_single = 0;
	//unsigned char *debug_buffer;

	//printf("[RTK] + parseHandshake_ServerCertificate , handshakeLen =%d\n",handshakeLen);	
	bufPos = pBuf + 4 ;// 4bytes for Handshake Header 

	if(handshakeLen < pktLen){
		buildHandShake_Msg((unsigned char *)pBuf,handshakeLen+4,sslSt);	
		//-get the certificate length (3 bytes), maybe several cert at one pkt-------------------	
		memcpy(((char *)&certiftLen_total)+1 , bufPos , 3);
		certiftLen_total = ntohl(certiftLen_total);		
		bufPos = bufPos + 3;
		//-get the certificate length (3 bytes), -------------------
		memcpy(((char *)&certiftLen_single)+1 , bufPos , 3);
		certiftLen_single = ntohl(certiftLen_single);
		bufPos = bufPos + 3;
		//m_tlsData.serverCrtLen = certiftLen;
		//printf("[RTK]certiftLen_single=%d\n",certiftLen_single);
		crtBuf = malloc(certiftLen_single);
		if(crtBuf == NULL){
			printf("[RTK]crtBuf alloc fail(%d)\n",certiftLen_single);
			return TLS_RESULT_FAIL;
		}
		memcpy(crtBuf, bufPos,certiftLen_single);
		
		ret = x509parse_crt( m_tlsData.serverCrt, (unsigned char *) crtBuf,  certiftLen_single);
		
		//printf("[RTK]rsa.N.p[63]=%d\n",(unsigned int)m_tlsData.serverCrt->rsa.N.p[63]);
		

		
		if(ret != 0){
			printf("Parse Server Certification Error(%x) , len = %d\n",ret,certiftLen_single);	
		}else{
			//printf("Parse Server Certification OK, len = %d\n",certiftLen_single);	
		}					
		//printf("[RTK] copy certificate , total=%d , len=%d\n",certiftLen_total,certiftLen);					
		
	 	m_tlsConnStatus = TLS_CONNECT_SERVER_CERTIFICATE;
		free(crtBuf);
		crtBuf=NULL;
		return TLS_RESULT_SUCCESS;
	}else{
		//CertificateLength is more than Pkt ,

		buildHandShake_Msg((unsigned char *)pBuf , pktLen ,sslSt);
		
		pktLen -= (4+3);// 4 for handshake header. 3 for certiftLen_total length
		memcpy(((char *)&certiftLen_total)+1 , bufPos , 3);
		certiftLen_total = ntohl(certiftLen_total);		
		m_serverCertiLen = certiftLen_total;
		bufPos +=3;//Length of certificate Total Len
		
		memcpy(((char *)&certiftLen_single)+1 , bufPos , 3);
		certiftLen_single = ntohl(certiftLen_single);		
		bufPos+=3;//Length of certificate Single Len
		
		crtBuf = malloc(certiftLen_single);		
		if(crtBuf == NULL){
			printf("[RTL]Alloc crtBuf fail\n");
			return TLS_RESULT_FAIL_BUFFER_ALLOC;			
		}
		memset(crtBuf,0x00,certiftLen_single);
		
		if(certiftLen_single < (pktLen-3) ){
			//printf("[RTL]Recv All Certificate (%d) form (%d)\n",certiftLen_single,pktLen);
			memcpy(crtBuf,bufPos,certiftLen_single);
			recvLeftCertificate(rxSkt , NULL , 0 , certiftLen_total-pktLen , sslSt);
		}else{
			//printf("[RTL]parseHandshake_ServerCertificate : Not enough (%d) form (%d)\n",certiftLen_single,pktLen);
			memcpy(crtBuf,bufPos,(pktLen-3));
			recvLeftCertificate(rxSkt , crtBuf+(pktLen-3) , certiftLen_single - (pktLen-3)  , certiftLen_total-pktLen , sslSt);
		}

		if(m_tlsData.serverCrt == NULL){
			printf("[RTK]parseHandshake_ServerCertificate: m_tlsData.serverCrt is null\n");
			return TLS_RESULT_FAIL;
		}
		if(crtBuf == NULL){
			printf("[RTK]parseHandshake_ServerCertificate: crtBuf is null\n");
			return TLS_RESULT_FAIL;
		}

		//printf("[RTK] + x509parse_crt , certiftLen_single=%d,crtBuf=%x , serverCrt=%x\n",certiftLen_single,crtBuf,m_tlsData.serverCrt);

		//for(debug_idx=0;debug_idx<10;debug_idx++){
		//	printf("[RTK] ready to x509parse_crt..[%d]\n",debug_idx);	
		//	OSTimeDly((OS_TICKS_PER_SEC *0.2 ));
		//}
		/*
		debug_buffer = crtBuf;
		printf("[RTK](crtBuf)%x-%x-%x-%x-%x\n",(unsigned char)debug_buffer[0]
									,(unsigned char)debug_buffer[1]
									,(unsigned char)debug_buffer[2]
									,(unsigned char)debug_buffer[3]
									,(unsigned char)debug_buffer[4]);
		debug_buffer = m_tlsData.serverCrt;
		printf("[RTK](crtBuf)%x-%x-%x-%x-%x\n",(unsigned char)debug_buffer[0]
									,(unsigned char)debug_buffer[1]
									,(unsigned char)debug_buffer[2]
									,(unsigned char)debug_buffer[3]
								,(unsigned char)debug_buffer[4]);
		*/

		
		//printf("Parse x509 certificate :crtBuf =0x%x , certiftLen_single=%d, m_tlsData.serverCrt=0x%x\n",crtBuf,certiftLen_single,m_tlsData.serverCrt);
		ret = x509parse_crt( m_tlsData.serverCrt, (unsigned char *) crtBuf,  certiftLen_single);

		//printf("[RTK]rsa.N.p[63]=%d\n",(unsigned int)m_tlsData.serverCrt->rsa.N.p[63]);
		//printf("[RTK]rsa.N.p=0x%x\n",(unsigned int)m_tlsData.serverCrt->rsa.N.p);
		//printFreeMem_tlsclient(63);
		
		if(ret != 0){
			printf("Parse Server Certification Error(%d) , len = %d\n",ret,certiftLen_single);	
		}else{
			//printf("Parse Server Certification OK, len = %d\n",certiftLen_single);	
		}

		

		#if 0
		certificateBuf = malloc(certiftLen_total);
		if(certificateBuf == NULL){
			printf("[RTL][Error]Alloc Certificate Buffer Fail\n");
			return TLS_RESULT_FAIL_BUFFER_ALLOC;	
		}else{
		printf("[RTL][Error]Alloc Certificate Buffer OK(%d)\n",certiftLen_total);
		}
		free(certificateBuf);
		#endif
		free(crtBuf);
		return TLS_RESULT_SUCCESS;
	}
}



TLS_CONNECT_RESULT parseServerRecordLayer(RTSkt *rxSkt,char *pBuf,int recordLen , int pktLen , SSL *sslSt){
		//pBuf is the start  of RecordLayer Data (without RecordLayer Header!!!)
		//recordLen : data len of RecordLayer
		handshakeHeader *phsHeader;
		char *nextHandShakeHd;//,bufPos;
		int handshakeLen = 0;		
		TLS_CONNECT_RESULT result;

		//printf("[RTK] + parseServerRecordLayer\n");

		result = TLS_RESULT_SUCCESS;
		
		if(pktLen < 4){ // 4 is handshake header length
			return TLS_RESULT_FAIL_SERVER_HELLO;
		}
		nextHandShakeHd = pBuf;
		do{		
			handshakeLen = 0;	
			phsHeader = (handshakeHeader *)(nextHandShakeHd);	
			memcpy((char *)&handshakeLen + 1,phsHeader->length,3);
			handshakeLen = ntohl(handshakeLen);
			
			if(phsHeader->type == HS_SERVER_HELLO ){
				
				result = parseHandshake_ServerHello((unsigned char *)nextHandShakeHd,handshakeLen,pktLen,sslSt);
			}else if(phsHeader->type == HS_CERTIFICATE ){
				
				result = parseHandshake_ServerCertificate(rxSkt,(unsigned char *)nextHandShakeHd,handshakeLen,pktLen,sslSt);
			}else if(phsHeader->type == HS_SERVER_HELLO_DONE){
				
				result = parseHandshake_ServerHelloDone((unsigned char *)nextHandShakeHd,handshakeLen,pktLen,sslSt);
			}			
			//printf("[RTK]parseServerRecordLayer : pktLen=%d,recordLen=%d,nextHandShakeHd=0x%x\n",pktLen,recordLen,nextHandShakeHd);
			pktLen = pktLen - (handshakeLen + 4);
			recordLen =  recordLen - (handshakeLen + 4);
			nextHandShakeHd = nextHandShakeHd + (handshakeLen + 4);
		}while((result == TLS_RESULT_SUCCESS)&&(pktLen > 0)&&(recordLen > 0));

		//printf("[RTK] - parseServerRecordLayer,result=%d\n",result);
		
		return result;	
}
TLS_CONNECT_RESULT parseServerHelloConnectPakt(RTSkt *rxSkt,PKT * pkt , SSL *sslSt){
	char *bufPos;
	int recordLayerLen;	
	recordLayerHeader *pRecdLayer;	
	int pktLen = 0;
	TLS_CONNECT_RESULT result;

	//pkt->len
	if(pkt->len < TLS_RECORD_SIZE){
		return TLS_RESULT_FAIL_SERVER_HELLO;
	}
	
	bufPos = pkt->wp;
	pktLen = pkt->len;
	//printf("[RTK] record layer Length = %d\n",recordLayerLen);
	//left = recordLayerLen; 
	//------------------------------------------------------------------------------------------------	
	//nextHandShakeHd = bufPos;	
	//buildHandShake_Msg(bufPos,recordLayerLen);	
	
    while(pktLen > 0){		
		
		pRecdLayer = (recordLayerHeader *)bufPos;
		bufPos = bufPos + TLS_RECORD_SIZE;
		recordLayerLen = ntohs(pRecdLayer->length);
		pktLen -= TLS_RECORD_SIZE;

		
		result = parseServerRecordLayer(rxSkt,bufPos , recordLayerLen , pktLen , sslSt);
		
		if(result != TLS_RESULT_SUCCESS){
			return result;
		}

		pktLen -= recordLayerLen;		
	}
	
	//printf("[RTK] - parseServerHelloConnectPakt\n");
	return TLS_RESULT_SUCCESS;
	
}

#else
TLS_CONNECT_RESULT parseServerHelloConnectPakt(PKT * pkt , SSL *sslSt){
	char *bufPos,*nextHandShakeHd;
	int recordLayerLen,handshakeLen,left;	
	recordLayerHeader *pRecdLayer;
	handshakeHeader *phsHeader;
	serverHelloStruct *serverhello;
	int pktLen = 0;

	//pkt->len
	if(pkt->len < TLS_RECORD_SIZE){
		return TLS_RESULT_FAIL_SERVER_HELLO;
	}
	pktLen = pkt->len;
	bufPos = pkt->wp;	
	pRecdLayer = (recordLayerHeader *)bufPos;
	bufPos = bufPos + TLS_RECORD_SIZE;
	recordLayerLen = ntohs(pRecdLayer->length);
	//printf("[RTK] record layer Length = %d\n",recordLayerLen);
	left = recordLayerLen; 
	//------------------------------------------------------------------------------------------------	
	nextHandShakeHd = bufPos;
	pktLen -= TLS_RECORD_SIZE;
	//buildHandShake_Msg(bufPos,recordLayerLen);




	
    while(left > 0){
		if(pktLen < 4){ // 4 is handshake header length
			return TLS_RESULT_FAIL_SERVER_HELLO;
		}
		phsHeader = (handshakeHeader *)(nextHandShakeHd);	
		bufPos = nextHandShakeHd + 4;//TLS Handshake Header Length is 4
		handshakeLen = 0;
		memcpy((char *)&handshakeLen + 1,phsHeader->length,3);
		handshakeLen = ntohl(handshakeLen);		
		left -= (handshakeLen+4);		
		nextHandShakeHd = nextHandShakeHd + handshakeLen + 4;	
		
			if(phsHeader->type == HS_SERVER_HELLO ){			
				buildHandShake_Msg((unsigned char *)phsHeader,handshakeLen+4);
				//printf("[RTK] Parse TLS_CONNECT_SERVER_HELLO , len=%d\n",handshakeLen);
				serverhello = (serverHelloStruct *)bufPos;
				bufPos = bufPos + sizeof(serverHelloStruct);
				if(serverhello->version != m_tlsData.tlsClientVersion){
						printf("[RTK][ERROR] version not match\n");
						return TLS_RESULT_FAIL_SERVER_HELLO;
				}
				//copy ServerRandom-----------------------------------------------
				memcpy(m_tlsData.serverRandom,serverhello->server_random,32);
				//copy Session ID---------------------------------------------------
				m_tlsData.sessionIDLen = serverhello->SessionIDLen;
				m_tlsData.sessionId = malloc(m_tlsData.sessionIDLen);								
				memcpy(m_tlsData.sessionId,bufPos,m_tlsData.sessionIDLen);
				//----------------------------------------------------------------
				bufPos += m_tlsData.sessionIDLen;				
				//compare cipher suite
				if(memcmp(bufPos,&(m_tlsData.cipherSuit),2) != 0){
					printf("[RTK][ERROR] cipher suite not match\n");
					return TLS_RESULT_FAIL_SERVER_HELLO;
				}
				
				initSSL(m_tlsData.cipherSuit,sslSt);
				
				// 1 bytes for compression method
				bufPos++;
				
				m_tlsConnStatus = TLS_CONNECT_SERVER_HELLO;
			}else if(phsHeader->type == HS_CERTIFICATE ){
					int certiftLen = 0,certiftLen_total=0,ret=0;
					char *crtBuf;

					buildHandShake_Msg((unsigned char *)phsHeader,handshakeLen+4);
					//printf("[RTK] Parse TLS_CONNECT_SERVER_CERTIFICATE, len=%d\n",handshakeLen);
					//-get the certificate length (3 bytes), maybe several cert at one pkt-------------------
					memcpy(((char *)&certiftLen_total)+1 , bufPos , 3);
					certiftLen_total = ntohl(certiftLen_total);
					m_serverCertiLen = certiftLen_total;					
					bufPos = bufPos + 3;
					//-get the certificate length (3 bytes), -------------------
					memcpy(((char *)&certiftLen)+1 , bufPos , 3);
					certiftLen = ntohl(certiftLen);
					bufPos = bufPos + 3;
					//m_tlsData.serverCrtLen = certiftLen;
					crtBuf = malloc(certiftLen);
					memcpy(crtBuf, bufPos,certiftLen);
					
					ret = x509parse_crt( m_tlsData.serverCrt, (unsigned char *) crtBuf,  certiftLen);
					if(ret != 0){
						printf("Parse Server Certification Error(%d) , len = %d\n",ret,certiftLen);	
					}else{
						printf("Parse Server Certification OK, len = %d\n",certiftLen);	
					}					
					//printf("[RTK] copy certificate , total=%d , len=%d\n",certiftLen_total,certiftLen);					

					
					m_tlsConnStatus = TLS_CONNECT_SERVER_CERTIFICATE;
			}else if(phsHeader->type == HS_SERVER_HELLO_DONE ){
					//printf("[RTK] Parse TLS_CONNECT_SERVER_HELLODONE, len=%d\n",handshakeLen);
					buildHandShake_Msg((unsigned char *)phsHeader,handshakeLen+4);
					m_tlsConnStatus = TLS_CONNECT_SERVER_HELLODONE;
					printf("[RTK] left = 0x%x\n",left);
			}		
		
	}
	
	printf("[RTK] - parseServerHelloConnectPakt\n");
	return TLS_RESULT_SUCCESS;
	
}
#endif


int clientInitKey(struct tcp_pcb *pcb)
{
    //unsigned char random[SSL_RANDOM_SIZE*2];
    //unsigned char keyBlk[256];
    unsigned char *random = malloc(SSL_RANDOM_SIZE*2);
    unsigned char *keyBlk = malloc(256);
    unsigned char *key1;
    unsigned char *key2;

   
    // Swap the client and server random values.
    //printf("[RTK]clientInitKey : pcb=0x%x\n",pcb);
	
	if(random == NULL){
		printf("[RTK] clientInitKey:alloc random fail\n");
	}
	if(keyBlk == NULL){
		printf("[RTK] clientInitKey:alloc keyBlk fail\n");
	}
	
	memset(random,0x00,SSL_RANDOM_SIZE*2);
	memset(keyBlk,0x00,256);
	
    memcpy(random , m_tlsData.serverRandom, SSL_RANDOM_SIZE);
    memcpy(random + SSL_RANDOM_SIZE, m_tlsData.clientRandom, SSL_RANDOM_SIZE);
	
	//-Make the Master Secret
	/*
	if (pcb->ssl->cipher == TLS_RSA_WITH_AES_128_CBC_SHA)
       {
        tls1_prf((unsigned char *)&(m_tlsData.preMaster), PMSLEN, "master secret", random, SSL_RANDOM_SIZE*2,
                 pcb->ssl->master, PMSLEN );
    	}*/
	//----
	
	//printf("+tls1_prf at clientInitKey\n");
    tls1_prf(pcb->ssl->master , 48, "key expansion", random , SSL_RANDOM_SIZE*2,
             keyBlk, 256 );
	//printf("-tls1_prf at clientInitKey \n");
	
  	//printf("-tls1_prf at clientInitKey , (0x%x)\n",pcb->ssl->cipher);
    // Determine the key, IV and MAC length.
    /*
	if(pcb->ssl->cipher != TLS_RSA_WITH_AES_128_CBC_SHA){
		pcb->ssl->cipher = TLS_RSA_WITH_AES_128_CBC_SHA; //+briank.rtk
	}
	
    switch (pcb->ssl->cipher)
    {
        //minLen = max(keyLen, ivLen, macLen) ???
    case SSL_RSA_RC4_128_SHA:
        pcb->ssl->keyLen = 16;
        pcb->ssl->minLen = 20;
        pcb->ssl->ivLen = 0;
        pcb->ssl->macLen = 20;
        break;
    case TLS_RSA_WITH_AES_128_CBC_SHA:
        pcb->ssl->keyLen = 16;
        pcb->ssl->minLen = 20;
        pcb->ssl->ivLen = 16;
        pcb->ssl->macLen = 20;
        break;
    }*/

    /*
    * Finally setup the cipher contexts, IVs and MAC secrets.
    */
    // this is from server ? --
    /*
    key1 = keyBlk + pcb->ssl->macLen * 2 + pcb->ssl->keyLen;//macLen = 20
    key2 = keyBlk + pcb->ssl->macLen * 2;

    memcpy( pcb->ssl->macDec, keyBlk,  pcb->ssl->macLen );
    memcpy( pcb->ssl->macEnc, keyBlk + pcb->ssl->macLen, pcb->ssl->macLen );

    memcpy( pcb->ssl->ivDec, key1 + pcb->ssl->keyLen,  pcb->ssl->ivLen );
    memcpy( pcb->ssl->ivEnc, key1 + pcb->ssl->keyLen + pcb->ssl->ivLen, pcb->ssl->ivLen );
    */
   // printf("WTF(0)\n");
	
    key1 = keyBlk + pcb->ssl->macLen * 2;
	key2 = keyBlk + pcb->ssl->macLen * 2 + pcb->ssl->keyLen;//macLen = 20
    
	//printf("WTF(1)\n");
	
    memcpy( pcb->ssl->macEnc, keyBlk,  pcb->ssl->macLen );
    memcpy( pcb->ssl->macDec, keyBlk + pcb->ssl->macLen, pcb->ssl->macLen );

	//printf("WTF(2)\n");

    memcpy( pcb->ssl->ivEnc, key2 + pcb->ssl->keyLen,  pcb->ssl->ivLen );
    memcpy( pcb->ssl->ivDec, key2 + pcb->ssl->keyLen + pcb->ssl->ivLen, pcb->ssl->ivLen );
	
	//printf("[INIT][IVENC] %x-%x-%x-%x-%x-\n",pcb->ssl->ivEnc[0],pcb->ssl->ivEnc[1],pcb->ssl->ivEnc[2],pcb->ssl->ivEnc[3],pcb->ssl->ivEnc[4]);
	
    switch (pcb->ssl->cipher)
    {
    case TLS_RSA_WITH_AES_128_CBC_SHA:
        //aes_setkey_enc( (aes_context *) pcb->ssl->ctxEnc, key1, pcb->ssl->keyLen*8);
        //aes_setkey_dec( (aes_context *) pcb->ssl->ctxDec, key2, pcb->ssl->keyLen*8);
        memcpy(pcb->ssl->ctxEnc, key1, pcb->ssl->keyLen);
        memcpy(pcb->ssl->ctxDec, key2, pcb->ssl->keyLen);
		//printf("[RTK]OK:pcb->ssl->cipher=0x%x\n",pcb->ssl->cipher);
        break;
	default :
		printf("[RTK]WTF:pcb->ssl->cipher=0x%x\n",pcb->ssl->cipher);
		break;
    }

    free(random);
    free(keyBlk);

	//printf("[RTK]- clientInitKey\n");
	
    return( 0 );
}



//void parseServerCertification(char *server_cert,int ServerCertLen){
//printf("[RTK] + parseServerCertification\n");
//}

//This will build "ClientKeyExchange" +  "ChangeCipherSpec" + "Finish"
#if 0
PKT *buildClientKeyExchangePkt(int *pktLen){
	
	PKT *pkt;
	//int pktLen = 0;
	int ret;
	//unsigned char random[64];
	unsigned char *pktbuf;
	//unsigned char hashresult[36];
	int idx;
	//unsigned char *keyblk;
	//unsigned char *key1;
    //unsigned char *key2;
	
	#ifdef DEBUG_RSA
	int olen=0;
	//unsigned char outBuf[256];
	#endif
	

	*pktLen = 0;

	//keyblk = malloc(256);
	//memset(hashresult,0x00,36);
	//pkt = (PKT*)allocPkt_F(326); //326 == (262+5)+(1+5)+(48+5)	
	pkt = (PKT*)allocPkt_F(267); //273 == (262+5)
	
	//1.ClientKeyExchange :build Client Premaster Secret ----------------------------------
	//     1.1  create TLS header
	pkt->wp[0] = RECORD_HANDSHAKE_PROTOCOL;
	pkt->wp[1] = 0x03;pkt->wp[2] = 0x01;
	pkt->wp[3] = 0x01;pkt->wp[4] = 0x06; //0x0106 = 262
	//     1.2  create Handshake Header 
	pkt->wp[5] = HS_CLIENT_KEY_XCHG;
	pkt->wp[6] = 0x00;pkt->wp[7] = 0x01;pkt->wp[8] = 0x02; //0x000102 = 258	
	//     1.3  create MasterSecret Len	
	pkt->wp[9] = 0x01;pkt->wp[10] = 0x00;//0x0100 = 256
	//     1.3.1  Make premaster secret
	m_tlsData.preMaster.client_version = m_tlsData.tlsClientVersion;
	rtkRandom(m_tlsData.preMaster.random , 46);
	// 	 1.3.2 use Server public key to RSA encrypt pre-master Key
	ret = rsa_pkcs1_encrypt(&m_tlsData.serverCrt->rsa , RSA_PUBLIC, 48, &(m_tlsData.preMaster) , pkt->wp + 11);    	
	
	if ( ret != 0 )
    {
		freePkt_F(pkt);
        return NULL;
    }
	*pktLen = 11+256;
	pktbuf = pkt->wp + *pktLen;

	buildHandShake_Msg(pkt->wp+5,262);

	
	
/*
	//2.ChangeCipherSpec --------------------------------------------------------------------
	//     2.1  create TLS header
	pktbuf[0] = RECORD_CHANGE_CIPHER_SPEC;
	pktbuf[1] = 0x03;pktbuf[2] = 0x01;
	pktbuf[3] = 0x00;pktbuf[4] = 0x01; //0x0001 = 1	
	pktbuf[5] = 0x01;//change cipher spec message.
	*pktLen += 6;
	pktbuf = pkt->wp + *pktLen;
	//3.Client Finish --------------------------------------------------------------------------
	//	3.1 create TLS header
	pktbuf[0] = RECORD_HANDSHAKE_PROTOCOL;	
	pktbuf[1] = 0x03;pktbuf[2] = 0x01;
	pktbuf[3] = 0x00;pktbuf[4] = 0x30; //0x0030 = 48
	//	3.2 build finish message
	// 	3.2.1 build master secret first
	memcpy(random , m_tlsData.clientRandom, 32);
    memcpy(random + 32, m_tlsData.serverRandom , 32);	

		#ifdef DEBUG_RANDOM
		printf("ClientRand");
		for(idx=0;idx<64;idx++){
			printf("%x ",random[idx]);
			if(idx == 31){
				printf("\n");
				printf("ServerRand");
			}
		}
		printf("\n");
		#endif
	
	tls1_prf( &m_tlsData.preMaster,48,"master secret",random,64,m_tlsData.masterSectet, 48 );
	//	3.2.2 hash handshake messages

		#ifdef DEBUG_MD5_&_SHA1
		memcpy(m_tlsData.handshake_message,"The quick brown fox jumps over the lazy dog",strlen("The quick brown fox jumps over the lazy dog"));
		m_tlsData.hskMsgLen = strlen("The quick brown fox jumps over the lazy dog");
		m_tlsData.handshake_message[m_tlsData.hskMsgLen] = 0x00;
		printf("---->set handshake msg len(%d):%s\n[MD5]",m_tlsData.hskMsgLen,m_tlsData.handshake_message);
		#endif
	
	printf("[RTK]Len of Handshake Messages = %d\n",m_tlsData.hskMsgLen);
		
	md5H(m_tlsData.handshake_message,m_tlsData.hskMsgLen,hashresult);	
	sha1(m_tlsData.handshake_message,m_tlsData.hskMsgLen,hashresult+16);	
	
		#ifdef DEBUG_MD5_&_SHA1
		for(idx=0;idx<16;idx++){
			printf("-%x",hashresult[idx]);
		}
		printf("\n[SHA1]");
		for(idx=0;idx<20;idx++){
			printf("-%x",hashresult[idx+16]);
		}
		printf("\n");
		#endif

	//   3.2.3	 Make (MAC secret )(Session Key)(IVs)

	tls1_prf( m_tlsData.masterSectet, 48, "key expansion", random, 64, keyblk, 256 );
	key1 = keyblk + m_tlsData.maclen * 2;
    key2 = keyblk + m_tlsData.maclen * 2 + m_tlsData.keylen;

    memcpy( m_tlsData.mac_enc, keyblk,  m_tlsData.maclen );
    memcpy( m_tlsData.mac_dec, keyblk + m_tlsData.maclen, m_tlsData.maclen );

    memcpy( m_tlsData.iv_enc, key2 + m_tlsData.keylen,  m_tlsData.ivlen );
    memcpy( m_tlsData.iv_dec, key2 + m_tlsData.keylen + m_tlsData.ivlen, m_tlsData.ivlen );

	memcpy(m_tlsData.ctx_enc, key1, m_tlsData.keylen);
    memcpy(m_tlsData.ctx_dec, key2, m_tlsData.keylen);	
	
	
	//	3.2.4 PRF 
	tls1_prf( m_tlsData.masterSectet, 48 ,"client finished", hashresult,36, pktbuf+5 , 48 );
	*pktLen += 53;
*/		
	//----------------------------------------------------------------------------------------
	//printf("[RTK]buildClientKeyExchangePkt finish , len=(%d)\n",pktLen);	


	//free(keyblk);
	return pkt;
}
#endif

//Function	: write data to Server
//return 		: int : how many bytes are sent
//input 		: length - how many bytes to sent , without padding. 
void closeConnectToTlsServer(RTSkt *skt){
	INT8U err = 0;
	//printf("[RTK] closeConnectToTlsServer\n");
//printFreeMem(28);
	
	#ifdef DEBUG_MD5_SHA1
	if(m_tlsData.handshake_message)
		free(m_tlsData.handshake_message);
	#endif

	
	//printFreeMemAll_tlsclient(2);

	//printFreeMem(281);

	if(m_tlsData.serverCrt){
		x509_free(m_tlsData.serverCrt);
    	memset( m_tlsData.serverCrt, 0, sizeof( x509_cert ) );
	    free(m_tlsData.serverCrt);
		m_tlsData.serverCrt=NULL;
	}

	
	//
	
	
	if(m_tlsData.sessionId){
		free(m_tlsData.sessionId);
		m_tlsData.sessionId = NULL;
		
	}

	//printFreeMem(281);	
	if(m_pReleaseSSL){
		free(m_pReleaseSSL);
		m_pReleaseSSL = NULL;
	}
	//printFreeMem(282);	
	
	//m_tlsData.sslc = NULL;
	
#if 0 //because the skt->pcb is null already
	printf("[RTK]skt->pcb=0x%x\n",skt->pcb);
	if(skt->pcb->ssl){
		printf("[RTK]skt->pcb->ssl=0x%x\n",skt->pcb->ssl);		
		free(skt->pcb->ssl);
		skt->pcb->ssl=NULL;
		
	}
#endif	
#if 0 //print all [7] , because i want to know the ssl is free already?
	int idx;
	SSL* wtf[18];

	printf("ALLMem:");
	for(idx=0;idx<18;idx++){
		wtf[idx] = (SSL*)malloc(sizeof(SSL));
		printf("[%x]",wtf[idx]);
	}
	printf("\n");

#endif
	if (skt){
		
	rtSktClose_F(skt);
    OSSemDel(skt->rxSem, OS_DEL_ALWAYS, &err);
    OSSemDel(skt->txSem, OS_DEL_ALWAYS, &err);
    skt->rxSem=0;
    skt->txSem=0;


	}
	
    if (skt)
    	free(skt);

	
//printFreeMem(29);

	//printFreeMemAddrALL_tlsclient(29,7);


	//allocDebug_tlsclient();
    //skt=NULL;
}


void md5_singleString(unsigned char *src,int srcLen , unsigned char *dst,SSL *pSsl){
	//int idx=0;

	//printf("[MD5]m_tlsData.sslc->finMd5=0x%x,srcLen=%d\n",&m_tlsData.sslc->finMd5,srcLen);
	
	md5_starts(&pSsl->finMd5);	

	//printf("[MD5][buffer1]");
	//for(idx=0;idx<64;idx++){
	//	printf("-%x",m_tlsData.sslc->finMd5.buffer[idx]);
	//}
	//printf("\n");
	
	md5_updateH(&pSsl->finMd5 , src,srcLen);

	//printf("[MD5][buffer2]");
	//for(idx=0;idx<64;idx++){
	//	printf("-%x",m_tlsData.sslc->finMd5.buffer[idx]);
	//}
	//printf("\n");
	
	md5_finishH(&pSsl->finMd5 , dst );
	
	//printf("[MD5]source=%s\n[MD5]dst=",src);
	//for(idx=0;idx<16;idx++){
	//	printf("-%x",dst[idx]);
	//}
	//printf("\n");
}

TLS_CONNECT_RESULT connectToTlsServer(RTSkt *skt)
{
    PKT * txpkt;
	int size;
    //RTSkt *skt = rtSkt_F( IPv4, IP_PROTO_TCP );
	int clientHelloSize;	
	PKT* rxPkt;
	int status=0,connTimes=0;
	TLS_CONNECT_RESULT result;
	
	#ifdef DEBUG_MD5_SHA1
		int idx;
		unsigned char hashresult[36];//+briank.rtk	
		memset(hashresult,0x00,36);
	#endif

	//printFreeMem(221);
	//printFreeMemAddrALL_tlsclient(221,7);
	
	//printf("[RTK] + connectToTlsServer , skt->pcb=0x%x \n",skt->pcb,skt->pcb->ssl);


	//printFreeMemAll_tlsclient(0);
	createClientPCBSSL(skt->pcb);

	//printf("[RTK] + connectToTlsServer , skt->pcb->ssl=0x%x \n" ,skt->pcb->ssl);
	
	//printFreeMemAll_tlsclient(1);
	m_pReleaseSSL = (unsigned char*)skt->pcb->ssl;


	//m_pcb = skt->pcb;
	
	//m_tlsData.sslc = skt->pcb->ssl;	
	m_tlsConnStatus = TLS_CONNECT_START;

//rtk_md5Test(); //+briank.rtk..............
	connTimes= 0 ;
	do{
		status =  rtSktConnect_F( skt, m_serverIP , m_serverPort);
		if(status == -1){
			printf("[RTK] Fail(%d).........m_serverIP=0x%x , m_serverPort=%d \n",++connTimes,m_serverIP,m_serverPort);	
			OSTimeDly(OS_TICKS_PER_SEC*0.5);			
		}
	}while((status == -1)&&(connTimes < 3));
	if(status==-1){
		printf("[RTK] Fail(%d)\n",connTimes);
        return TLS_RESULT_TCP_CONN_FAIL;
    }

//1.Send Client Hello-------------------------------------------------------
	clientHelloSize = sizeof(clientHelloStruct);
	//printf("[RTK]client size = %d \n",clientHelloSize);	
	size = TLS_RECORD_SIZE + clientHelloSize;
	txpkt = (PKT*)allocPkt_F(size);	

	if(txpkt == NULL){
		printf("[RTK]Alloc pkt fail......\n");	
        return TLS_RESULT_FAIL_BUFFER_ALLOC;
	}

	
	makeTLSRecordLayer(txpkt->wp, RECORD_HANDSHAKE_PROTOCOL,clientHelloSize,skt->pcb);
	buildClientHelloPkt(txpkt->wp);
	buildHandShake_Msg(txpkt->wp+TLS_RECORD_SIZE ,(size-5),skt->pcb->ssl);


	//printFreeMem(222);
	
	//printf("[RTK]+send client HELLO\n");
	
	status = rtSktSend_F( skt, txpkt, TLS_RECORD_SIZE + clientHelloSize );
	#if 0
	if(status != SKTOK){
		if(status == SKTDISCONNECT){
			printf("send client HELLO SKTDISCONNECT\n");
		}else if(status == SKT_TX_FAIL){
			printf("send client HELLO SKT_TX_FAIL\n");
		}else if(status == SKTHOSTNOTFOUND){
			printf("send client HELLO SKTHOSTNOTFOUND\n");
		}else if(status == SKTCLOSING){
			printf("send client HELLO SKTCLOSING\n");
		}
		OSTimeDly((OS_TICKS_PER_SEC *0.5 ));
		//return TLS_RESULT_FAIL;
	}
	#endif
	m_tlsConnStatus = TLS_CONNECT_CLIENT_HELLO;
	//printFreeMem(223);
//1.1 Read Server Hello , and Certificate and HelloDone-------------------------------------------------------	
	while(TLS_CONNECT_SERVER_HELLODONE > m_tlsConnStatus){
	//--recv packet--------------------------------
		//printf("[RTK]connectToTlsServer +rtSktRx_F , skt->pcb=0x%x\n",skt->pcb);
		while ((rxPkt = rtSktRx_F(skt,100,&status)) == 0){
			//printf("[RTK] recv NO packet , skt->pcb=0x%x\n",skt->pcb);
			 if (status == SKTDISCONNECT){
			 	printf("[RTK]connectToTlsServer : SKTDISCONNECT\n");
				return TLS_RESULT_FAIL_SERVER_HELLO;
			 }else if (status == OS_ERR_TIMEOUT){
			 	//printf("[RTK]connectToTlsServer : OS_ERR_TIMEOUT\n");
				//return TLS_RESULT_TIMEOUT;
			 }
		}
		//printf("[RTK]connectToTlsServer rxPkt=0x%x\n",rxPkt);
		//-------------------------------------------
		if(parseServerHelloConnectPakt(skt,rxPkt , skt->pcb->ssl) != TLS_RESULT_SUCCESS){
			//printf("Error : Client Disconnect now\n");
			//rtSktClose_F(skt);
			closeConnectToTlsServer(skt);
			skt = NULL;

			if(rxPkt)
				freePkt_F(rxPkt);
			
			return TLS_RESULT_FAIL_PARSE_PKT;
		}

		//printf("[RTK]+ connectToTlsServer freePkt\n");	
		if(rxPkt)
			freePkt_F(rxPkt);		
		//printf("[RTK]- connectToTlsServer freePkt\n");	
	}	
	//if (pkt) {
    //	if (!pkt->queued){
    // 		freePkt(pkt);}}	
	
	//printFreeMem(224);
//2.Send Client key Exchange&ChangeCipherSpec&ClientFinish-------------------------------------------------------	
	if(TLS_CONNECT_SERVER_HELLODONE <= m_tlsConnStatus){		
		//-Make Handshake Messgaes first------------------------------------		
		#ifdef DEBUG_MD5_SHA1
			md5H(m_tlsData.handshake_message,m_tlsData.hskMsgLen,hashresult);	
			sha1(m_tlsData.handshake_message,m_tlsData.hskMsgLen,hashresult+16);
			for(idx=0;idx<16;idx++){
				printf("-%x",hashresult[idx]);
			}
			printf("\n[SHA1]");
			for(idx=0;idx<20;idx++){
				printf("-%x",hashresult[idx+16]);
			}
			printf("\n");
			md5_starts(&skt->pcb->ssl->finMd5);
	   		sha1_startsH(&skt->pcb->ssl->finSha1);
		#endif		
		
		//----------------------------------------------------------------		
		//printf("Finish clientInitKey\n");
		sendClientFinish(skt->pcb);
		//printf("Finish send Client Finish\n");		
	}
	//printFreeMem(225);
	m_tlsConnStatus = TLS_CONNECT_CLIENT_FINISH;
//3.Read Server Finish-------------------------------------------------------		
	do{
		while ((rxPkt = rtSktRx_F(skt,100,&status)) == 0){
			//printf("[RTK] recv one packet\n");
			 if (status == SKTDISCONNECT){
			 	//printf("[RTK]connectToTlsServer : SKTDISCONNECT\n");
				return TLS_RESULT_FAIL_SERVER_FINISH;
			 }else if (status == OS_ERR_TIMEOUT){
			 	//printf("[RTK]connectToTlsServer : OS_ERR_TIMEOUT\n");
				//return TLS_RESULT_TIMEOUT;
			 }
		}
		//-------------------------------------------
		result = parseServerFinishPkt(rxPkt,skt);
		//printf("[RTK] parseServerFinishPkt : rxPkt=0x%x\n",rxPkt);
		if(rxPkt)
			freePkt_F(rxPkt);	
		if(result != TLS_RESULT_SUCCESS){
			printf("Error : Client Disconnect now\n");
			//rtSktClose_F(skt);
			closeConnectToTlsServer(skt);
			skt=NULL;			
			return result;
		}else{
			return TLS_RESULT_SUCCESS;;
		}
	}while(TLS_CONNECT_SERVER_FINISH > m_tlsConnStatus);
		
}

int clientFinish(unsigned char *p, struct tcp_pcb *pcb)
{
    md5_context  *tMd5 = malloc(sizeof(md5_context));
    sha1_context *tSha1 = malloc(sizeof(sha1_context));;
    int len = 16;
    int padLen = 0;
    int i = 0;
    //unsigned char tmp1[100] = {0};
    INT8U *tmp1= malloc(100);


    memset(tmp1, 0, 100);
    memcpy(tMd5, &pcb->ssl->finMd5, sizeof(md5_context));
    memcpy(tSha1, &pcb->ssl->finSha1, sizeof(sha1_context));
    // 4  = message type(1) + length(3) (Handshake Layer Header)
	
	//-----------------------------------------------
	//md5_updateH( &pcb->ssl->finMd5 , m_tlsData.handshake_message, m_tlsData.hskMsgLen );
	//sha1_updateH( &pcb->ssl->finSha1, m_tlsData.handshake_message, m_tlsData.hskMsgLen );
	//-----------------------------------------------
    ssl_calc_finished(pcb->ssl->master, p + 4, CFINISH, tMd5, tSha1);
     //ssl_calc_finished : including 1.Get Hash from MD5 and SHA-1 , 2. then pass to PRF , 3.get Finished Message (12 bytes)



	#if RTK_SSL_DEBUG
	//printf("handshake_message Length = %d\n" , m_tlsData.hskMsgLen);
	printf("[Finish]:");
	for(i=0;i<16;i++){
		printf("%x-",p[i]);
	}
	printf("\n");
	
	
	/*
	printf("[MasterSecret]:");
	for(i=0;i<PMSLEN;i++){
		printf("%x-",pcb->ssl->master[i]);
	}
	printf("\n");
	
	printf("[ServerRandom]:");
	for(i=0;i<SSL_RANDOM_SIZE;i++){
		printf("%x-",m_tlsData.serverRandom[i]);
	}
	printf("\n");

	printf("[ClientRandom]:");
	for(i=0;i<SSL_RANDOM_SIZE;i++){
		printf("%x-",m_tlsData.clientRandom[i]);
	}
	printf("\n");
	*/
	#endif
	

    p[0] = HS_FINISHED;
    p[1] = (unsigned char)((len - 4) >> 16);
    p[2] = (unsigned char)((len - 4) >> 8);
    p[3] = (unsigned char)len - 4;

    md5_updateH(&pcb->ssl->finMd5 , p, len);
    sha1_updateH(&pcb->ssl->finSha1, p, len);

//printf("[RTK]ssl->outCounter = [%x-%x-%x-%x-%x-%x-%x-%x]\n",pcb->ssl->outCounter[0],pcb->ssl->outCounter[1],pcb->ssl->outCounter[2],pcb->ssl->outCounter[3]
//	,pcb->ssl->outCounter[4],pcb->ssl->outCounter[5],pcb->ssl->outCounter[6],pcb->ssl->outCounter[7]);

    memcpy(tmp1, pcb->ssl->outCounter, 8);
    memcpy(tmp1 + 8 , p - 5, 5);//p-5 means the TLS record layer header.
    memcpy(tmp1 + 13, p, 16);

    if ( pcb->ssl->macLen == 20 )
        sha1_hmac( pcb->ssl->macEnc, 20,
                   tmp1,  len + 13,
                   p + 16 );

    if (pcb->ssl->ivLen == 0)
    {
#if 0
        arc4_crypt( (arc4_context *) pcb->ssl->ctxEnc,
                    p, 36 );
        padLen = 0;
#endif
    }
    else
    {
        padLen = checkPadLen(pcb->ssl->ivLen, 36);
        for ( i = 0; i <= padLen; i++ )
            p[36 + i] = (unsigned char) padLen;
        switch (pcb->ssl->ivLen)
        {
        case 16:
            aes_crypt_cbc( (aes_context *) pcb->ssl->ctxEnc,
                           AES_ENCRYPT, (36 + padLen + 1),
                           pcb->ssl->ivEnc, p, p);
            break;
        }
    }

    for ( i = 7; i >= 0; i-- )
        if ( ++pcb->ssl->outCounter[i] != 0 )
            break;

    free(tmp1);
    free(tMd5);
    free(tSha1);
    return 0;
}

int sendClientFinish(struct tcp_pcb *pcb)
{
    //TLS_RECORD_SIZE*2: change cipher spec + server finish
    //1:change cipher spec messgae
    //36 = Message type(1) + message len(3) + Finish message(12byte) + MAC(20 for SHA1)
    //int size = TLS_RECORD_SIZE*2 + 1 + 36 + checkPadLen(pcb->ssl->ivLen, 36);
    int size = TLS_RECORD_SIZE*3 + CLIENT_KEY_XCHG_LEN + 1 + 36 + checkPadLen(pcb->ssl->ivLen, 36);
    PKT* pkt;
	int ret;
	unsigned char *preMaster;
	//int idx;
	unsigned char *random;

//printf("[RTK] + pcb=0x%x \n",pcb);

	preMaster = malloc(PMSLEN);	

	
	if(preMaster == NULL){
		printf("[Error][tls.c]Allocate buffer for preMaster Fail\n");
		return -1;
	}
	memset(preMaster,0x00,PMSLEN);
	
	random = malloc(SSL_RANDOM_SIZE*2);		
	if(random == NULL){
		printf("[Error][tls.c]Allocate buffer for random Fail\n");
		free(preMaster);
		return -1;
	}
	memset(random,0x00,SSL_RANDOM_SIZE*2);
	
    //checkPadLen only return padding content length, not include padding field
    if (checkPadLen(pcb->ssl->ivLen, 36))
        size += 1;
	
	//printf("[RTK] + sendClientFinish,size=%d\n",size);
    pkt = (PKT*)allocPkt_F(size);

//printFreeMem_tls(0);

	if(pkt == NULL){
		printf("[RTK][Error]sendClientFinish Alloc pkt=NULL\n");
		free(preMaster);
		free(random);
		return -1;
	}

	//Make Handshake : Change Cipher Spec--------------
    	makeTLSRecordLayer(pkt->wp, RECORD_HANDSHAKE_PROTOCOL, CLIENT_KEY_XCHG_LEN, pcb);
//printFreeMem_tls(1);		
		//Make Master Secret
		pkt->wp[TLS_RECORD_SIZE] = HS_CLIENT_KEY_XCHG;
		pkt->wp[TLS_RECORD_SIZE+1] = 0x00;pkt->wp[TLS_RECORD_SIZE+2] = 0x01;pkt->wp[TLS_RECORD_SIZE+3] = 0x02; //0x000102 = 258
		pkt->wp[TLS_RECORD_SIZE+4] = 0x01;pkt->wp[TLS_RECORD_SIZE+5] = 0x00;
		preMaster[0] = 0x03;preMaster[1] = 0x01;
		rtkRandom(preMaster + 2  , 46);
		memcpy(random , m_tlsData.clientRandom , SSL_RANDOM_SIZE);
		memcpy(random+SSL_RANDOM_SIZE,m_tlsData.serverRandom,SSL_RANDOM_SIZE);
		//printf("[RTK] + tls1_prf\n");
//OSTimeDly((OS_TICKS_PER_SEC/2));		

//printf("[RTK](1) PCB->SSL=0x%x, m_tlsData.sslc=0x%x\n",pcb->ssl,m_tlsData.sslc);
//printf("[RTK](1) cipher suit=0x%x, 0x%x, 0x%x\n",pcb->ssl->cipher,m_tlsData.cipherSuit,m_tlsData.sslc->cipher); 	
//printFreeMem_tls(2);

		tls1_prf(preMaster, PMSLEN, "master secret", random, SSL_RANDOM_SIZE*2,pcb->ssl->master, PMSLEN );		
//printf("[RTK](2) PCB->SSL=0x%x, m_tlsData.sslc=0x%x\n",pcb->ssl,m_tlsData.sslc);
//printf("[RTK](2) cipher suit=0x%x, 0x%x, 0x%x\n",pcb->ssl->cipher,m_tlsData.cipherSuit,m_tlsData.sslc->cipher);				
//printf("[RTK] + rsa_pkcs1_encrypt\n");
//printFreeMem_tls(3);

//printf("[RTK]m_tlsData.serverCrt=%x,preMaster=0x%x,rsa.len=%d\n",m_tlsData.serverCrt,preMaster,m_tlsData.serverCrt->rsa.len);
//printf("premaster=");
//for(idx=0;idx<48;idx++){printf("-%x",preMaster[idx]);}
//printf("\n");
//OSTimeDly((OS_TICKS_PER_SEC/2));		
		ret = rsa_pkcs1_encrypt(&m_tlsData.serverCrt->rsa , RSA_PUBLIC, 48, preMaster , pkt->wp +TLS_RECORD_SIZE+6);
		//ret = rsa_pkcs1_encrypt_bk(&m_tlsData.serverCrt->rsa , RSA_PUBLIC, 48, preMaster , pkt->wp +TLS_RECORD_SIZE+6);
		

//+ briank.rtk just for debug--------------------
#if 0
		if(ret != 0x00){
			printf("RSA ret 0x%x\n",ret*-1);
		}else{
			printf("RSA ret Ok\n");
		}
		ret = rsa_pkcs1_encrypt(&m_tlsData.serverCrt->rsa , RSA_PUBLIC, 48, preMaster , pkt->wp +TLS_RECORD_SIZE+6);
		if(ret != 0x00){
			printf("RSA ret 0x%x\n",ret*-1);
		}else{
			printf("RSA ret OK\n");
		}
		debug_stop();
#endif		
//*-----------------------------------------
		
//printFreeMem_tls(4);


		//printf("[RTK] - rsa_pkcs1_encrypt,ret = 0x%x\n",ret);
		if ( ret != 0 )
    	{
    		ret = ret * -1;
    		printf("Error:SendClientFinish : rsa_pkcs1_encrypt fail,(0x%x)\n",ret);
			freePkt_F(pkt);
			free(preMaster);
			free(random);
        	return 0;
    	}
		buildHandShake_Msg(pkt->wp+5,262,pcb->ssl);


	#ifdef RTK_SSL_DEBUG
	/*
	printf("[preMasterSecret]:");
	for(idx=0;idx<PMSLEN;idx++){
		printf("%x-",preMaster[idx]);
	}
	
	printf("\n");
	*/
	#endif
	//-got master secret , then " MAC secret" , " init sessionkey " and " initial vector "

	//printf("[RTK] + clientInitKey\n");

	//printFreeMem_tls(5);
	
	clientInitKey(pcb);
		#ifdef RTK_SSL_DEBUG
		/*
			printf("[Client MAC]:");
			for(idx=0;idx < pcb->ssl->macLen;idx++){
			printf("%x-",pcb->ssl->macEnc[idx]);
			}		
			printf("\n");	

			printf("[Server MAC]:");
			for(idx=0;idx < pcb->ssl->macLen;idx++){
			printf("%x-",pcb->ssl->macDec[idx]);
			}		
			printf("\n");


			printf("[Client key]:");
			for(idx=0;idx < pcb->ssl->keyLen;idx++){
			printf("%x-",pcb->ssl->ctxEnc[idx]);
			}		
			printf("\n");

			printf("[Server key]:");
			for(idx=0;idx < pcb->ssl->keyLen;idx++){
			printf("%x-",pcb->ssl->ctxDec[idx]);
			}		
			printf("\n");

			printf("[Client Ivs]:");
			for(idx=0;idx < pcb->ssl->ivLen;idx++){
			printf("%x-",pcb->ssl->ivEnc[idx]);
			}		
			printf("\n");

			printf("[Server Ivs]:");
			for(idx=0;idx < pcb->ssl->ivLen;idx++){
			printf("%x-",pcb->ssl->ivDec[idx]);
			}		
			printf("\n");

		*/
		#endif 
		
	//printf("[RTK] + makeTLSRecordLayer 2\n");
	//Make Handshake : Change Cipher Spec--------------
    makeTLSRecordLayer(pkt->wp+TLS_RECORD_SIZE+CLIENT_KEY_XCHG_LEN, RECORD_CHANGE_CIPHER_SPEC, 1, pcb);
    pkt->wp[TLS_RECORD_SIZE*2+CLIENT_KEY_XCHG_LEN] = 0x01;
	//-----------------------------------------------

    makeTLSRecordLayer(pkt->wp + TLS_RECORD_SIZE*2 + CLIENT_KEY_XCHG_LEN + 1, RECORD_HANDSHAKE_PROTOCOL, 16, pcb); // 20130705 : this is necessary!!!
    clientFinish(pkt->wp + TLS_RECORD_SIZE*3 +CLIENT_KEY_XCHG_LEN + 1, pcb);

    if (checkPadLen(pcb->ssl->ivLen, 36))//checkPadLen : return the pad length
        makeTLSRecordLayer(pkt->wp + TLS_RECORD_SIZE*2 +CLIENT_KEY_XCHG_LEN + 1, RECORD_HANDSHAKE_PROTOCOL  ,1 + 36 + checkPadLen(pcb->ssl->ivLen, 36), pcb);
    else
        makeTLSRecordLayer(pkt->wp + TLS_RECORD_SIZE*2 +CLIENT_KEY_XCHG_LEN + 1, RECORD_HANDSHAKE_PROTOCOL, 36, pcb);

    tcpEnqueue_F(pcb, pkt);
    tcpSend_F(pcb, pkt, TCP_PSH | TCP_ACK);

	
	free(preMaster);
	free(random);
    return 0;
}



