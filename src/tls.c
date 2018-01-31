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
#include "tls.h"
#include "http.h"
#include "aes.h"
#include "bsp.h"
#include "x509.h"
#include "soap.h"

//#define TLSTEST 1
#ifdef TLSTEST
unsigned char server[32] = {0x00,0x00,0x00,0x00 ,0x00 ,0x2d ,0xcf ,0x46 ,0x29 ,0x04 		,0xb4 ,0x78 ,0xd8 ,0x68 ,0xa7 ,0xff ,0x3f ,0x2b ,0xf1 ,0xfc ,0xd9 ,0x7a ,0x96 ,0x09 ,0x2c ,0xa5 		,0x57 ,0x74 ,0x64 ,0xc4 ,0xaf ,0x15};
unsigned char client[32]=  {0x4a ,0xc1 ,0xce ,0x7e ,0x5f ,0x5f ,0xb4 ,0xa3 ,0x25 ,0x9a ,0x72 ,0xb4 ,0x41 ,0x37 		,0xca ,0x64 ,0xcd ,0x76 ,0x3b ,0xe9 ,0x3f ,0x6e ,0xb7 ,0x67 ,0x3d ,0x0a ,0x48 ,0x1f ,0x8f ,0x0f ,0x48 ,0x6d};

#endif
static SSL *ActiveSSL = NULL;
extern DPCONF *dpconf;
int processClientHelloSSLv2(unsigned char *p, int len, struct tcp_pcb *pcb);
/**
* Description:	Allocated a SSL.
*
* Returns: certificate addr
*
*/
unsigned char* getCertificate(int* size)
{
    unsigned char *pos = 0;

    sscanf((void *) TLS_FLASH_DATA_ADDRESS, "%d\n", size);
    pos = strchr((void *) TLS_FLASH_DATA_ADDRESS, '\n');

    if (pos != 0)
        pos++;

    return pos;
}

/**
* Description:	Allocated a SSL.
*
* Returns:
*
*/

SSL* createSSL()
{
    SSL* tmp = (SSL*)malloc(sizeof(SSL));
    memset(tmp, 0, sizeof(SSL));
    return tmp;
}

void authedSSL(unsigned char* sessionID)
{
    SSL* curSSL = ActiveSSL;

    while (curSSL)
    {
        if (memcmp(curSSL->sessionID , sessionID, SESSION_ID_LEN) == 0)
        {
            curSSL->httpAuthPass = PASS;
        }
        curSSL= curSSL->next;
    }

}

/**
* Description:	Search SSL list to see if have exist SSL.
*
*
* Parameters:
*				p -> Record layer start sddress
*				pcb -> Connection
*
* Returns:
*
*/
void getPCBMatchSSL(unsigned char *p ,struct tcp_pcb *pcb)
{
    SSL* curSSL = 0;
    SSL* prevSSL = 0;
    SSL* newSSL = 0;
    SSL* removeSSL = 0;
    int count = 0;

    if (ActiveSSL == 0)
    {
        ActiveSSL = createSSL();
        ActiveSSL->next = 0;
        pcb->ssl = ActiveSSL;
    }
    else if (pcb->ssl == 0)
    {
        curSSL = ActiveSSL;
        while (curSSL && !(p[0] & 0x80))
        {
            if (memcmp((p + TLS_RECORD_SIZE + 6 + SSL_RANDOM_SIZE + 1) , curSSL->sessionID, SESSION_ID_LEN) == 0)
            {
                DEBUGMSG(HTTP_CB_DEBUG, "Session ID found\n");

                newSSL = createSSL();
                newSSL->next = 0;
                pcb->ssl = newSSL;
                memcpy(pcb->ssl, curSSL, sizeof(SSL));
                pcb->ssl->next = 0;
                pcb->ssl->cPkt = 0;
                pcb->ssl->resume = 1;
                //Reset; Do not set session ID and master to 0
                memset( pcb->ssl->inCounter, 0, 8);
                memset( pcb->ssl->outCounter, 0, 8);

                memset( pcb->ssl->ivEnc, 0, 16);
                memset( pcb->ssl->ivDec, 0, 16);
                memset( pcb->ssl->macEnc, 0, 32);
                memset( pcb->ssl->macDec, 0, 32);
                memset( pcb->ssl->ctxEnc, 0, 16);
                memset( pcb->ssl->ctxDec, 0, 16);

                memset(&pcb->ssl->finSha1, 0, sizeof(sha1_context));
                memset(&pcb->ssl->finMd5, 0, sizeof(md5_context));
                pcb->ssl->clientStage = 0;
                pcb->ssl->crLen = 0;
                pcb->httpAuthPass = pcb->ssl->httpAuthPass = curSSL->httpAuthPass;
                pcb->ssl->coalesceLen = 0;
                break;
            }
            prevSSL = curSSL;
            curSSL = curSSL->next;
        }
        //Do not found any SSL session
        if (pcb->ssl == 0)
        {
            curSSL = ActiveSSL;

            prevSSL = 0;
            removeSSL = 0;

            while (curSSL)
            {
                if (curSSL->resume == 0)
                {
                    if (!count)
                        removeSSL = curSSL;
                    count++;
                }

                if (!removeSSL)
                    prevSSL = curSSL;
                if (count == 2)
                    break;
                curSSL = curSSL->next;
            }

            if (count >= 2)
            {
                if (ActiveSSL == removeSSL)
                    ActiveSSL = removeSSL->next;
                else
                    prevSSL->next = removeSSL->next;

                free(removeSSL);
            }

            //Only keep 2 SSL
            newSSL = createSSL();
            newSSL->next = 0;
            pcb->ssl = newSSL;
            //Add to SSL list

            curSSL = ActiveSSL;

            while (curSSL)
            {
                prevSSL= curSSL;
                curSSL = curSSL->next;
            }
            prevSSL->next = newSSL;
            DEBUGMSG(HTTP_CB_DEBUG, "New SSL\n");
        }
    }

    md5_starts(&pcb->ssl->finMd5);
    sha1_startsH(&pcb->ssl->finSha1);
}

/**
* Description:	Process the record layer.
*
*
* Parameters:
*				ivLen -> IV length
*				outMsgLen ->output message length
*
* Returns:		How many byte that should be padded(only padding content length).
*
*/
#if CONFIG_VERSION  < IC_VERSION_DP_RevF
//EP_RevA define in cert.c
int checkPadLen(int ivLen, int outMsgLen)
{
    if (ivLen == 0)
        return 0;
    else
        return (ivLen - (outMsgLen + 1) % ivLen);
}
#endif
/**
* Description:	Process the record layer.
*
*
* Parameters:
*				p -> Record layer start sddress
*				len ->How many byte that can be parse.
*				PCB -> Connection
*
* Returns:
*
*/
int parseRecordLayer(unsigned char *p, int len, struct tcp_pcb *pcb)
{

    //Check TLS version
    if ( (p[1] != 0x03 || p[2] != 0x01))
    {
        if ((p[0] & 0x80)/*SSL2 */ && (p[2] == 0x01) /*Client Hello*/
                && (p[3] == 0x03 && p[4] == 0x01)/*TLS*/)
        {
            /*
            * SSLv2 Client Hello
            *
            * Record layer:
            *     0  .   1   message length
            *
            * SSL layer:
            *     2  .   2   message type
            *     3  .   4   protocol version
            */

            ;//Only Accept Client Hello message
        }
        else
        {
            //DEBUGF(HTTP_CB_DEBUG, ("Record layer version is not TLSv1%x %x %x",p[0], p[1], p[2]));
            //	DEBUGF(HTTP_CB_DEBUG, ("size %x %x",p[3], p[4]));

            tcpSend_F(pcb, NULL, TCP_ACK | TCP_RST);
            removePCB_F(pcb);
            return -1;
        }
    }

    if (p[0] & 0x80)
    {
        getPCBMatchSSL(p, pcb);
        pcb->ssl->recordLayer.contentType = RECORD_HANDSHAKE_PROTOCOL;
    }
    else
    {
        //Find corresponse SSL
        if (pcb->ssl == 0)
        {
            getPCBMatchSSL(p, pcb);
        }

        if (len < TLS_RECORD_SIZE)
        {
            DEBUGMSG(HTTP_CB_DEBUG, "Len too short for parsing in parseRecordLayer\n");
            return -1;
        }

        pcb->ssl->recordLayer.contentType = p[0];
        pcb->ssl->recordLayer.size = (((unsigned short)p[3]<<8) + (unsigned short)p[4]);
    }
    return 0;
}

/**
* Description:	Parse TLS request(Parse content type)
*
*
* Parameters:
*				buf -> TLS data start sddress
*				size-> Total TLS length
*				pcb -> Connection
*
* Returns:		None
*
*/
int doTLSAccept(unsigned char *buf, int len, struct tcp_pcb *pcb)
{
    int tmpLen = 0, tmpLen1 = 0;
    int httprxResult = 0; 
    DEBUGMSG(HTTPS_DEBUG,"doTLSAccept==>");

    do
    {
        if (pcb->ssl&& (pcb->ssl->handshakeStage == HS_UNDEFINED))
        {
            //concate
            pcb->ssl->cPkt = (PKT*)allocPkt_F(pcb->ssl->hPkt->len + len);
            memcpy(pcb->ssl->cPkt->wp, pcb->ssl->hPkt->wp, pcb->ssl->hPkt->len);
            memcpy(pcb->ssl->cPkt->wp + pcb->ssl->hPkt->len, buf, len);
            freePkt_F(pcb->ssl->hPkt);
            pcb->ssl->hPkt = pcb->ssl->cPkt;
            pcb->ssl->cPkt = NULL;
            if (parseRecordLayer(pcb->ssl->hPkt->wp, pcb->ssl->hPkt->len, pcb) == -1)
                return -1;
#if 0
            //hao do it in doServerHandshake
            if (pcb->ssl->recordLayer.contentType == RECORD_CHANGE_CIPHER_SPEC)
            {
                parseChangeCipherSpec(pcb->ssl->hPkt->wp + TLS_RECORD_SIZE, pcb->ssl->hPkt->len - TLS_RECORD_SIZE, pcb);
                pcb->ssl->hPkt->wp += pcb->ssl->recordLayer.size;
                pcb->ssl->hPkt->len -= pcb->ssl->recordLayer.size;
            }
#endif
            if (-1 == doServerHandshake(pcb->ssl->hPkt->wp + TLS_RECORD_SIZE, pcb->ssl->hPkt->len - TLS_RECORD_SIZE, pcb))
                return -1;
            //sendServerFinish(pcb);
            freePkt_F(pcb->ssl->hPkt);
            pcb->ssl->handshakeStage = HS_COMPLETED;

            break;
        }
#if 1
        if (pcb->ssl && pcb->ssl->fragpkt == 1)
        {
            unsigned int tmplen =  pcb->ssl->cPkt->len;
            memcpy(pcb->ssl->cPkt->wp +  pcb->ssl->cPkt->len - pcb->ssl->crLen, buf, len);
            pcb->ssl->crLen = pcb->ssl->crLen - len;
            if (pcb->ssl->crLen > 0)
                return -1;
            buf = pcb->ssl->cPkt->wp;
            pcb->TLSDataStage = 1;
            pcb->ssl->fragpkt = 0;
            deCodeTLSData(buf, tmplen, 0, pcb, RECORD_APP_PROTOCOL_DATA);
            if (pcb->apRx)
                httpRx(buf, tmplen, pcb);
            freePkt(pcb->ssl->cPkt);
            pcb->ssl->cPkt = 0;
            return 0;
        }
#endif
        DEBUGMSG(HTTPS_DEBUG,"parseRecordLayer1");
        if (parseRecordLayer(buf, len, pcb) == -1)
            return -1;
        DEBUGMSG(HTTPS_DEBUG,"parseRecordLayer2");

        switch (pcb->ssl->recordLayer.contentType)
        {
        case RECORD_CHANGE_CIPHER_SPEC:
            DEBUGMSG(HTTPS_DEBUG,"RECORD_CHANGE_CIPHER_SPEC");
            if (len > 0)
            {
                len = len - TLS_RECORD_SIZE;
                buf = buf + TLS_RECORD_SIZE;
                parseChangeCipherSpec(buf, len, pcb);
                buf = buf + pcb->ssl->recordLayer.size;
                len = len - pcb->ssl->recordLayer.size;
                if (!pcb->ssl->resume)
                {
                    parseRecordLayer(buf, len, pcb);
                    buf = buf + TLS_RECORD_SIZE;
                    len = len - pcb->ssl->recordLayer.size;
                    parseClientFinish(buf, len, pcb);
                    sendServerFinish(pcb);
                    len = 0;
                }
            }
            pcb->ssl->clientStage = RECORD_CHANGE_CIPHER_SPEC;
            break;
            //Currently, we do not process RECORD_ALERT_PROTOCOL
        case RECORD_ALERT_PROTOCOL:
            DEBUGMSG(HTTPS_DEBUG,"RECORD_ALERT_PROTOCOL");
            deCodeTLSData(buf + TLS_RECORD_SIZE, len - TLS_RECORD_SIZE, 0, pcb, RECORD_ALERT_PROTOCOL);
            return 0;

        case RECORD_HANDSHAKE_PROTOCOL:
            DEBUGMSG(HTTPS_DEBUG,"RECORD_HANDSHAKE_PROTOCOL");
            if (len - TLS_RECORD_SIZE > 0)
            {
                if (buf[0]&0x80)
                {
                    processClientHelloSSLv2(buf, len, pcb);
                    sendServerHelloSequence(pcb);
                    len = 0;
                }
                else
                {
                    tmpLen1 = (((unsigned short)buf[3])<<8) + (unsigned short)buf[4];
                    if (tmpLen1 > len)
                    {
                        pcb->ssl->hPkt = (PKT*)allocPkt_F(len);
                        pcb->ssl->hPkt->len = len;
                        memcpy(pcb->ssl->hPkt->wp, buf, len);
                        pcb->ssl->handshakeStage = HS_UNDEFINED;
                        return -1;
                    }
                    buf = buf + TLS_RECORD_SIZE;

                    tmpLen = doServerHandshake(buf, len - TLS_RECORD_SIZE, pcb);
                    if (tmpLen >=0 && pcb->ssl->resume)
                    {
                        //When resume, Send server hello and Change cipherspec and Server Fin
                        //separately and will cause Change cipherspec and Server Fin
                        //retransmit.(have 2 ack)
                        buf = buf + tmpLen1;
                        len = len - tmpLen1 - TLS_RECORD_SIZE;
                    }
                    else if (tmpLen >=0)
                    {
                        len = tmpLen;
                        buf = buf + pcb->ssl->recordLayer.size;
                    }
                    else
                    {
                        return -1;
                    }
                }
            }
            break;
        case RECORD_APP_PROTOCOL_DATA:

            DEBUGMSG(HTTPS_DEBUG,"RECORD_APP_PROTOCOL_DATA");

            pcb->TLSDataStage = 1;
#if 0
            if (pcb->apRx && (pcb->ssl->recordLayer.size + TLS_RECORD_SIZE < len))
            {
                tmpLen = ((unsigned short)(*(buf + pcb->ssl->recordLayer.size + TLS_RECORD_SIZE + 3))<<8)
                         + (unsigned short)(*(buf + pcb->ssl->recordLayer.size + TLS_RECORD_SIZE + 4));
                pcb->ssl->cPkt = (PKT*)allocPkt_F(tmpLen);
                memcpy(pcb->ssl->cPkt->wp, buf + pcb->ssl->recordLayer.size + TLS_RECORD_SIZE + TLS_RECORD_SIZE, len - (pcb->ssl->recordLayer.size + TLS_RECORD_SIZE));
                pcb->TLSDataStage = 0;

                pcb->ssl->cPkt->len = tmpLen;
                pcb->ssl->crLen = tmpLen -(len- (pcb->ssl->recordLayer.size + TLS_RECORD_SIZE)-TLS_RECORD_SIZE);
                pcb->ssl->recordLayer.size = tmpLen;
                return -1;
            }
#endif
            //extra data appended, two conditions, packets are split to 2 packets or  not.
            if (pcb->ssl->recordLayer.size + TLS_RECORD_SIZE < len)
            {
                PKT* tmppkt = (PKT*)allocPkt_F(ETH_PAYLOAD_LEN);
                unsigned char *tmpbuf = buf;
                unsigned char *tmpbuf2 = tmppkt->wp;
                int totalUnDecryptedLen = 0;
                pcb->ssl->pktsmallLen=0;
                tmpLen = ((unsigned short)(*(tmpbuf + 3))<<8) + (unsigned short)(*(tmpbuf + 4));
                while (1)
                {
                    pcb->ssl->recordLayer.size = tmpLen;
                    deCodeTLSData(tmpbuf + TLS_RECORD_SIZE, tmpLen, 0, pcb, RECORD_APP_PROTOCOL_DATA);

                    if (pcb->ssl->recordLayer.size > 0)
                    {
                        memcpy(tmpbuf2, tmpbuf + TLS_RECORD_SIZE, pcb->ssl->recordLayer.size);
                        tmpbuf2+=pcb->ssl->recordLayer.size;
                        pcb->ssl->pktsmallLen += pcb->ssl->recordLayer.size;
                    }
                    tmpbuf += (tmpLen + TLS_RECORD_SIZE);
                    totalUnDecryptedLen += (tmpLen + TLS_RECORD_SIZE);
                    if (tmpbuf[0] == RECORD_APP_PROTOCOL_DATA)
                    {
                        tmpLen = ((unsigned short)(*(tmpbuf + 3))<<8) + (unsigned short)(*(tmpbuf + 4));
                    }
                    else break;
                    if (totalUnDecryptedLen + tmpLen + TLS_RECORD_SIZE == len) //in case of fragment packet
                        continue;
                    else if (totalUnDecryptedLen + tmpLen + TLS_RECORD_SIZE > len)
                    {
                        PKT *XtmpPkt;
                        tmppkt->len=pcb->ssl->pktsmallLen;
                        pcb->ssl->cPkt=tmppkt;
                        XtmpPkt=(PKT*)allocPkt_F(2048);
                        if (XtmpPkt)
                            pcb->ssl->cPkt->next = XtmpPkt;

                        pcb->TLSDataStage = 1;

                        pcb->ssl->recordLayer.size = tmpLen;
                        memcpy(XtmpPkt->wp, buf + totalUnDecryptedLen + TLS_RECORD_SIZE, len - totalUnDecryptedLen- TLS_RECORD_SIZE);
                        pcb->ssl->cPkt->next->len = len - totalUnDecryptedLen- TLS_RECORD_SIZE;
                        pcb->ssl->crLen = totalUnDecryptedLen + tmpLen - len + TLS_RECORD_SIZE;
                        pcb->ssl->fragpkt = 2;
                        if(pcb->apRx)
                            httpRx(tmppkt->wp, pcb->ssl->pktsmallLen , pcb);

                        return -1;

                    }
                }
                pcb->ssl->pktsmall = (unsigned char*)tmppkt;
                if (pcb->apRx){
                    //   httpRx(buf + TLS_RECORD_SIZE, pcb->ssl->recordLayer.size, pcb);
                    httprxResult = httpRx(tmppkt->wp, pcb->ssl->pktsmallLen , pcb);
                    if(httprxResult == 1){ 
	               if(tmppkt){		
		    	  freePkt_F(tmppkt);
			  tmppkt = NULL;		
  		        }
                    }
                }
                else
                    pcb->ssl->clientStage = RECORD_TOO_SMALL;



                return 0;
            }

            //fragment data
            else if (pcb->ssl->recordLayer.size - len + TLS_RECORD_SIZE > 0)
            {
                pcb->TLSDataStage = 0;
                pcb->ssl->crLen = pcb->ssl->recordLayer.size - len + TLS_RECORD_SIZE;
                if (!pcb->apRx)
                {
                    //				pcb->ssl->cPkt = (PKT*)allocPkt_F(pcb->ssl->recordLayer.size);

                    //				memcpy(pcb->ssl->cPkt->wp, buf + TLS_RECORD_SIZE, len - TLS_RECORD_SIZE);
                    pcb->ssl->coalesceLen = pcb->ssl->recordLayer.size;
//					pcb->ssl->crLen = pcb->ssl->recordLayer.size - len + TLS_RECORD_SIZE;
                    //pcb->ssl->fragpkt = 1;
                    return TLS_FRAGMENT_PKT;
                }
                else
                {
                    pcb->ssl->cPkt = (PKT*)allocPkt_F(pcb->ssl->recordLayer.size);

                    memcpy(pcb->ssl->cPkt->wp, buf + TLS_RECORD_SIZE, len - TLS_RECORD_SIZE);
                    pcb->ssl->cPkt->len = pcb->ssl->recordLayer.size;
                    //				pcb->ssl->crLen = pcb->ssl->recordLayer.size - len + TLS_RECORD_SIZE;
                    pcb->ssl->fragpkt = 1;
                    return -1;
                }

            }

            deCodeTLSData(buf + TLS_RECORD_SIZE, len - TLS_RECORD_SIZE, 0, pcb, RECORD_APP_PROTOCOL_DATA);

            if (pcb->apRx)
                httpRx(buf + TLS_RECORD_SIZE, pcb->ssl->recordLayer.size, pcb);

            len = 0;

            break;
        default:
            DEBUGMSG(CRITICAL_ERROR | HTTPS_DEBUG,"Unknow record layer type");
        }
    } while (len > 0);

    DEBUGMSG(HTTPS_DEBUG,"doTLSAccept<==");
    return 0;
}

//MAC = HMAC(Sequence number + messgae length + message content + two fixed string)
//messgae(include recorder layer)
int tlsEncodeData(PKT *pkt, int len, struct tcp_pcb *pcb, int type)
{
    int i = 0, tlen;
    int padLen  = 0;

    //move wp to recorder layer header
    pkt->wp = pkt->wp - 5;
    makeTLSRecordLayer(pkt->wp , type, len, pcb);

    memcpy(pkt->wp - 8, pcb->ssl->outCounter, 8);

    if ( pcb->ssl->macLen == 20 )
        sha1_hmac( pcb->ssl->macEnc, 20,
                   pkt->wp - 8,  len + 13,
                   (pkt->wp + TLS_RECORD_SIZE) +  len);

    memset(pkt->wp - 8, 0, 8);

    //Output message length + mac length
    tlen = len + pcb->ssl->macLen;

    for ( i = 7; i >= 0; i-- )
        if ( ++pcb->ssl->outCounter[i] != 0 )
            break;

    if ( pcb->ssl->ivLen == 0 )
    {
#if 0
        arc4_crypt( (arc4_context *) pcb->ssl->ctxEnc,
                    (pkt->wp + 5), tlen );
#endif
    }
    else
    {
        padLen = checkPadLen(pcb->ssl->ivLen, tlen);
        if (padLen == pcb->ssl->ivLen)
            padLen = 0;
        for ( i = 0; i <= padLen; i++ )
            pkt->wp[ 5 + tlen + i] = (unsigned char) padLen;
        tlen = tlen + padLen + 1;
        /**
        * \brief          AES-CBC buffer encryption/decryption
        *
        * \param ctx      AES context
        * \param mode     AES_ENCRYPT or AES_DECRYPT
        * \param length   length of the input data
        * \param iv       initialization vector (updated after use)
        * \param input    buffer holding the input data
        * \param output   buffer holding the output data
        */
        switch (pcb->ssl->ivLen)
        {
        case 16:
            aes_crypt_cbc( (aes_context *) pcb->ssl->ctxEnc,
                           AES_ENCRYPT, tlen, pcb->ssl->ivEnc, (pkt->wp + 5),(pkt->wp + 5));
            break;
        default:
            DEBUGMSG(HTTP_CB_DEBUG,"Incorrect IV Length");
            return -1;
        }
    }

    makeTLSRecordLayer(pkt->wp, type, tlen, pcb);
    pkt->len = tlen + 5;
    tcpEnqueue_F(pcb, pkt);
    tcpSend_F(pcb, pkt, TCP_PSH | TCP_ACK);

    return 0;
}

/**
* Description:	Decode TLS Encryption message.
*
*
* Parameters:
*				p -> Protocol message start sddress
*				len -> Protocol message total length
*				PCB -> Connection
*
* Returns:		None
*
*/
int deCodeTLSData(unsigned char *p, int len, PKT *pktList,struct tcp_pcb *pcb, int type)
{
    unsigned char *tp = NULL;
    unsigned char macVaule[20];
    unsigned char macVauleOutput[20];
    int padLen = 0, i = 0;
    PKT* tmpPkt = NULL;
    md5_context *ctxMD5 = NULL;
    sha1_context *ctxSHA = NULL;

    if ( pcb->ssl->ivLen == 0 )
    {
#if 0
        padLen = 0;
        arc4_crypt( (arc4_context *) pcb->ssl->ctxDec,
                    p, len);
#endif
    }
    else
    {
        if (pcb->ssl->recordLayer.size % pcb->ssl->ivLen !=0 )
        {
            DEBUGMSG(HTTP_CB_DEBUG,"Error aes message length: %d\n", pcb->ssl->recordLayer.size);
            return TLS_DECODE_FAIL_RECORD_SIZE;
        }
        switch (pcb->ssl->ivLen)
        {
        case 16:
            if (pktList)
            {
                tmpPkt = pktList;
                while (tmpPkt && tmpPkt->next)
                {
                    i = tmpPkt->len % 32;//use 48?? mac 20 padding 20
                    if (i != 0)
                    {
                        tmpPkt->len = tmpPkt->len - i;
                        tmpPkt->next->wp = tmpPkt->next->wp - i;
                        memcpy(tmpPkt->next->wp , tmpPkt->wp + tmpPkt->len, i);
                        tmpPkt->next->len = tmpPkt->next->len + i;
                    }
                    tmpPkt = tmpPkt->next;
                }

                tmpPkt = pktList;

                while (tmpPkt)
                {
                    if ((tmpPkt->len % 16) != 0)
                        return TLS_DECODE_FAIL_PKT_LEN;
                    aes_crypt_cbc( (aes_context*) pcb->ssl->ctxDec,
                                   AES_DECRYPT, tmpPkt->len,
                                   pcb->ssl->ivDec, tmpPkt->wp, tmpPkt->wp);

                    if (!tmpPkt->next)
                    {
                        break;
                    }
                    tmpPkt = tmpPkt->next;
                }
            }
            else
            {
                aes_crypt_cbc( (aes_context*) pcb->ssl->ctxDec,
                               AES_DECRYPT, pcb->ssl->recordLayer.size,
                               pcb->ssl->ivDec, p, p);
            }
            break;
        default:
            DEBUGMSG(HTTP_CB_DEBUG,"Incorrect IV Length");
            return TLS_DECODE_FAIL_IV_LEN;
        }

        if (pktList)
        {
            padLen = 1 + tmpPkt->wp[tmpPkt->len - 1];
        }
        else
        {
            padLen = 1 + p[pcb->ssl->recordLayer.size - 1];
        }

        /*
        * If CBC we should check the padding length
        */

        if (pktList)
        {
            for ( i = 1; i <= padLen; i++ )
            {
                if ( tmpPkt->wp[tmpPkt->len - i] != padLen - 1 )
                {
                    padLen = 0;
                }
            }
            if(pcb->ssl->macLen + padLen > tmpPkt->len)
                padLen=tmpPkt->len-(pcb->ssl->macLen + padLen);
        }
        else
        {
            for ( i = 1; i <= padLen; i++ )
            {
                if ( p[pcb->ssl->recordLayer.size - i] != padLen - 1 )
                {
                    padLen = 0;
                }
            }
        }
    }

    pcb->ssl->recordLayer.size = pcb->ssl->recordLayer.size - (pcb->ssl->macLen + padLen);

    if (pktList)
    {
        p = pktList->wp;
    }

    tp = p - 5;
    tp[0] = type;
    tp[1] = 0x03;
    tp[2] = 0x01;
    tp[3] =  (unsigned char)(pcb->ssl->recordLayer.size >> 8);
    tp[4] =  (unsigned char)(pcb->ssl->recordLayer.size);

    if (pktList)
    {
        if ( pcb->ssl->macLen == 16 )
        {
            ctxMD5 = malloc(sizeof(md5_context));
            md5_hmac_startsH( ctxMD5, pcb->ssl->macDec, 16 );
        }
        else
        {
            ctxSHA = malloc(sizeof(sha1_context));
            sha1_hmac_startsH( ctxSHA, pcb->ssl->macDec, 20 );
        }


        i = pcb->ssl->recordLayer.size;
        tmpPkt = pktList;

        while (tmpPkt)
        {
            if (i - tmpPkt->len >= 0)
            {
                i = i - tmpPkt->len;
                tmpPkt = tmpPkt->next;
            }
            else
            {
                break;
            }
        }
        if(tmpPkt)
        {
            memcpy(macVaule, tmpPkt->wp + i, pcb->ssl->macLen);
            memcpy(tp - 8, pcb->ssl->inCounter, 8 );
            tmpPkt->len = tmpPkt->len - (pcb->ssl->macLen + padLen);
        }
        tmpPkt = pktList;
        /*
        if ( pcb->ssl->macLen == 16 )
        	md5_hmac( pcb->ssl->macDec, 16,
        	tp - 8,  tmpPkt->len + 13,
        	macVauleOutput );
        else
        	sha1_hmac( pcb->ssl->macDec, 20,
        	tp - 8,  tmpPkt->len + 13,
        	macVauleOutput);*/

        if ( pcb->ssl->macLen == 16 )
        {
            md5_hmac_updateH(ctxMD5, tp - 8,  tmpPkt->len + 13);
        }
        else
        {
            sha1_hmac_updateH(ctxSHA, tp - 8,  tmpPkt->len + 13);
        }

        tmpPkt = tmpPkt->next;

        while (tmpPkt)
        {
            /*
            if ( pcb->ssl->macLen == 16 )
            	md5_hmac( pcb->ssl->macDec, 16,
            	tmpPkt->wp,  tmpPkt->len,
            	macVauleOutput );
            else
            	sha1_hmac( pcb->ssl->macDec, 20,
            	tmpPkt->wp,  tmpPkt->len,
            	macVauleOutput);
            	*/
            if (pcb->ssl->macLen == 16)
            {
                md5_hmac_updateH(ctxMD5, tmpPkt->wp,  tmpPkt->len);
            }
            else
            {
                sha1_hmac_updateH(ctxSHA, tmpPkt->wp,  tmpPkt->len);
            }

            tmpPkt = tmpPkt->next;
        }

        if ( pcb->ssl->macLen == 16 )
        {
            md5_hmac_finishH( ctxMD5, macVauleOutput );
            free(ctxMD5);
        }
        else
        {
            sha1_hmac_finishH( ctxSHA, macVauleOutput );
            free(ctxSHA);
        }


        if ( memcmp( macVaule, macVauleOutput,
                     pcb->ssl->macLen ) != 0 )
        {
            DEBUGMSG(HTTP_CB_DEBUG,"Client finish:Mac does not match\n");
            return TLS_DECODE_FAIL_MAC_1;
        }

    }
    else
    {
        //Copy HMAC
        memcpy(macVaule, p + pcb->ssl->recordLayer.size, pcb->ssl->macLen);
        memcpy(tp - 8, pcb->ssl->inCounter, 8 );

        if ( pcb->ssl->macLen == 16 ){
            md5_hmac( pcb->ssl->macDec, 16,
                      tp - 8,  pcb->ssl->recordLayer.size + 13,
                      p + pcb->ssl->recordLayer.size );
        }
        else{
			
            sha1_hmac( pcb->ssl->macDec, 20,
                       tp - 8,  pcb->ssl->recordLayer.size + 13,
                       p + pcb->ssl->recordLayer.size);
        }

        if ( memcmp( macVaule, p + pcb->ssl->recordLayer.size,
                     pcb->ssl->macLen ) != 0 )
        {
            DEBUGMSG(HTTP_CB_DEBUG,"Client finish:Mac does not match\n");			
            return TLS_DECODE_FAIL_MAC_2;
        }
    }
    for ( i = 7; i >= 0; i-- )
        if ( ++pcb->ssl->inCounter[i] != 0 )
            break;

    return TLS_DECODE_SUCCESS;
}
#ifdef CONFIG_PROVISIONING

int checkCer(struct tcp_pcb* pcb)
{
    x509_name *p_x509name;
    unsigned char tmp;
    int ret = 0;

    //check OU, EKU, dns suffix
    p_x509name = &pcb->ssl->peer_cert->subject;
    while (p_x509name)
    {
        if (p_x509name->oid.p[2] == 0x0b)//OU
        {
            if (!strncmp(p_x509name->val.p, dpconf->OUString, strlen(dpconf->OUString)))
                ret = 1;
            else
            {
                ret = -1;
                break;
            }
        }
        else if (dpconf->pkidata.dnssuf && (p_x509name->oid.p[2] == 0x03))//CN DNS domain suffix of the certificate
        {
            tmp = *(p_x509name->val.p+p_x509name->val.len);
            *(p_x509name->val.p+p_x509name->val.len) = '\0';
            if (strstr(p_x509name->val.p, dpconf->pkidata.dnssuf))
                ret = 1;
            else
            {
                ret = -1;
                break;
            }
            *(p_x509name->val.p+p_x509name->val.len) = tmp;
        }
        p_x509name = p_x509name->next;
    }
    return ret;
}

int processClientCertificate(unsigned char *buf, int len, struct tcp_pcb *pcb)
{
    int msgLen = (((unsigned char)buf[1])<<16) + (((unsigned char)buf[2])<<8) + (unsigned char)buf[3];

    int ZTCCertLen = (((unsigned char)buf[7])<<16) + (((unsigned char)buf[8])<<8) + (unsigned char)buf[9];
    int RootCertLen = (((unsigned char)buf[10+ZTCCertLen])<<16) + (((unsigned char)buf[10+ZTCCertLen+1])<<8)
                      + (unsigned char)buf[10+ZTCCertLen+2];
    int i = 0, ret;

    x509_cert *p_Rootx509cert = NULL;
    unsigned char *cli_cert = NULL;
    unsigned char RootHash[20];
    INT8U found = 0;
    md5_updateH( &pcb->ssl->finMd5 , buf, msgLen+4);
    sha1_updateH( &pcb->ssl->finSha1, buf, msgLen+4);

    cli_cert = malloc(RootCertLen * sizeof(unsigned char));

    while (i < RootCertLen)
    {
        cli_cert[i] = buf[10 + ZTCCertLen + 3 + i];
        i++;
    }

    sha1H(cli_cert, RootCertLen, RootHash);
    for (i=0; i<6; i++)
    {
        if (memcmp(dpconf->pkidata.certhash[i], RootHash, 20) == 0)
        {
            found = 1;
        }
    }
    if (!found)
    {
        free(cli_cert);
        return -1;
    }

    p_Rootx509cert = (x509_cert *)malloc(sizeof(x509_cert));
    memset( p_Rootx509cert, 0, sizeof( x509_cert ) );
    ret = x509parse_crt( p_Rootx509cert, (unsigned char *) cli_cert,  RootCertLen);
    free(cli_cert);
    if ( ret != 0 )
    {
        printf( " failed\n  !  x509parse_crt returned %d\n\n", ret );
        return ret ;
    }
    cli_cert = malloc(ZTCCertLen * sizeof(unsigned char));
    i = 0;
    while (i < ZTCCertLen)
    {
        cli_cert[i] = buf[10 + i];
        i++;
    }

    pcb->ssl->peer_cert = (x509_cert *)malloc(sizeof(x509_cert));
    memset( pcb->ssl->peer_cert, 0, sizeof( x509_cert ) );
    ret = x509parse_crt( pcb->ssl->peer_cert, (unsigned char *) cli_cert,  ZTCCertLen);
    free(cli_cert);
    if ( ret != 0 )
    {
        return( ret );
    }

    return checkCer(pcb);
}
#endif

int processCertificateVerify(unsigned char *buf, int len, struct tcp_pcb *pcb)
{
    int msgLen = 0;
    if (buf[0] != 0x0f)
        return -1;
    msgLen = (((unsigned char)buf[1])<<16) + (((unsigned char)buf[2])<<8) + (unsigned char)buf[3];
    md5_updateH( &pcb->ssl->finMd5 , buf, msgLen+4);
    sha1_updateH( &pcb->ssl->finSha1, buf, msgLen+4);
    return 0;
}
/**
* Description:	Process the handshake record.
*
*
* Parameters:
*				p -> Protocol message start sddress
*				len -> Protocol message total length
*				PCB -> Connection
*
* Returns:		Remained data need to be parsed.
*
*/

int doServerHandshake(unsigned char *p, int len, struct tcp_pcb *pcb)
{
    char handshake_type = p[0];
    char certVer = 0, tlsrecord = 0;

    if (pcb->ssl->clientStage == RECORD_CHANGE_CIPHER_SPEC)
    {
        //Maybe following data will 0x01 or 0x10
        return parseClientFinish(p, len, pcb);
    }

    switch (handshake_type)
    {
    case HS_CLIENT_HELLO:
        if (-1 == processClientHello(p, len, pcb))
            return -1;
        sendServerHelloSequence(pcb);
        if (pcb->ssl->resume)
        {
            sendServerFinish(pcb);
        }
        break;
#if 0
    case HS_CLIENT_KEY_XCHG:
        tcpSend_F(pcb, NULL, TCP_ACK);
        processClientKeyExchange(p, len, pcb);
        len = len - pcb->ssl->recordLayer.size;
        return len;
#endif

#ifdef CONFIG_PROVISIONING
    case HS_CERTIFICATE:
        tcpSend_F(pcb, NULL, TCP_ACK);

        if (processClientCertificate(p, len, pcb) == -1)
        {
            sendTLSAlert_F(pcb);
            return -1;
        }
        pcb->ssl->handshakeStage = HS_CERTIFICATE;
        len = len - pcb->ssl->recordLayer.size;
        p += pcb->ssl->recordLayer.size;

        if (parseRecordLayer(p, len, pcb) == -1)
            return -1;
#endif

    case HS_CLIENT_KEY_XCHG:
        if ((dpconf->ProvisioningState != PROVISIONED && dpconf->PKIEnabled == 0 && dpconf->PSKEnabled == 1)
                || dpconf->ProvisioningState == PROVISIONED)
        {
            tcpSend_F(pcb, NULL, TCP_ACK);
            processClientKeyExchange(p, len, pcb);
            len = len - pcb->ssl->recordLayer.size;
            return len;
        }

#ifdef CONFIG_PROVISIONING
        if (p[0] == 0x16 && p[1] == 0x03 && p[2] == 0x01)
        {
            tlsrecord = 1;
            len -= TLS_RECORD_SIZE;
        }
        if (len /*- TLS_RECORD_SIZE*/ - pcb->ssl->recordLayer.size >= 0)
        {
            if (pcb->ssl->handshakeStage != HS_UNDEFINED)
            {
                if (tlsrecord == 1)
                {
                    p = p + TLS_RECORD_SIZE;
                    tlsrecord = 0;
                }
            }
            processClientKeyExchange(p, len, pcb);
            pcb->ssl->handshakeStage = HS_CLIENT_KEY_XCHG;
        }
        else
        {
            pcb->ssl->hPkt = (PKT*)allocPkt_F(pcb->ssl->recordLayer.size);
            pcb->ssl->hPkt->len = len;
            memcpy(pcb->ssl->hPkt->wp, p, len);
            pcb->ssl->handshakeStage = HS_UNDEFINED;
            return -1;
        }
        len = len - pcb->ssl->recordLayer.size;
        p += pcb->ssl->recordLayer.size;

        if (parseRecordLayer(p, len, pcb) == -1)
            return -1;
#endif

    case HS_CERT_VERIFY:
        if (p[0] == 0x16 && p[1] == 0x03 && p[2] == 0x01)
        {
            tlsrecord = 1;
            len -= TLS_RECORD_SIZE;
        }
        if (len - pcb->ssl->recordLayer.size >= 0)
        {
            if (pcb->ssl->handshakeStage != HS_UNDEFINED)
            {
                if (tlsrecord == 1)
                {
                    p = p + TLS_RECORD_SIZE;
                    tlsrecord = 0;
                }
                if (-1 != processCertificateVerify(p, len, pcb))
                {
                    certVer = 1;
                    pcb->ssl->handshakeStage = HS_CERT_VERIFY;
                }
            }
        }
        else
        {
            pcb->ssl->hPkt = (PKT*)allocPkt_F(pcb->ssl->recordLayer.size);
            pcb->ssl->hPkt->len = len;
            memcpy(pcb->ssl->hPkt->wp, p, len);
            pcb->ssl->handshakeStage = HS_UNDEFINED;
            return -1;
        }
        if (certVer == 1)
        {
            len = len - pcb->ssl->recordLayer.size;
            p += pcb->ssl->recordLayer.size;
            if (parseRecordLayer(p, len, pcb) == -1)
                return -1;
        }
        if (len - TLS_RECORD_SIZE - pcb->ssl->recordLayer.size >= 0)
        {
            if (pcb->ssl->handshakeStage != HS_UNDEFINED)
            {
                p = p + TLS_RECORD_SIZE;
                len -= TLS_RECORD_SIZE;
            }
            parseChangeCipherSpec(p, len, pcb);
            pcb->ssl->clientStage = RECORD_CHANGE_CIPHER_SPEC;
        }
        else
        {
            pcb->ssl->hPkt = (PKT*)allocPkt(pcb->ssl->recordLayer.size);
            pcb->ssl->hPkt->len = len;
            memcpy(pcb->ssl->hPkt->wp, p, len);
            pcb->ssl->clientStage = RECORD_COMPLETED;
            return -1;
        }
        len = len - pcb->ssl->recordLayer.size;
        p += pcb->ssl->recordLayer.size;
        if (parseRecordLayer(p, len, pcb) == -1)
            return -1;
        if (len - TLS_RECORD_SIZE - pcb->ssl->recordLayer.size >= 0)
        {
            if (pcb->ssl->handshakeStage != HS_UNDEFINED)
            {
                p = p + TLS_RECORD_SIZE;
                len -= TLS_RECORD_SIZE;
            }
            parseClientFinish(p, len, pcb);
            sendServerFinish(pcb);
            return 0;
        }
        else
        {
            pcb->ssl->hPkt = (PKT*)allocPkt_F(pcb->ssl->recordLayer.size);
            pcb->ssl->hPkt->len = len;
            memcpy(pcb->ssl->hPkt->wp, p, len);
            pcb->ssl->handshakeStage = HS_UNDEFINED;
            return -1;
        }
        break;
    default:
        parseClientFinish(p, len, pcb);
        sendServerFinish(pcb);
        return 0;
    }
    return 0;
}

/**
* Description:	Fill TLS record layer.
*
*
* Parameters:
*				buf -> Record layer start sddress
*				ct  -> Content type
*				size-> Length of protocol message
*				PCB -> Connection
*
* Returns:		None
*
*/
void makeTLSRecordLayer(unsigned char* buf, char ct, int size, struct tcp_pcb *pcb)
{
    buf[0] = ct;
    buf[1] = 0x03;
    buf[2] = 0x01;
    buf[3] = ((size & 0xff00) >> 8);
    buf[4] = size & 0x00ff;
}

/**
* Description:	Process a client hello message.
*
*
* Parameters:
*				p -> Protocol message start sddress
*				len -> Protocol message total length
*				PCB -> Connection
*
* Returns:		None
*
*/

int processClientHello(unsigned char *p, int len, struct tcp_pcb *pcb)
{
    const int RandOffset = 6;
    //int msgLen = (p[1]<<16) + (p[2]<<8) + p[3];
    int offset = RandOffset + SSL_RANDOM_SIZE;
    int idLen = 0;
    int csLen = 0; //cipher suite length
    int i = 0;
    //Check TLS version
    if (p[4] != 0x3 || p[5]!=0x1)
        DEBUGMSG(HTTP_CB_DEBUG, "Not TLSv1 in processClientHello");
#ifdef TLSTEST
    memcpy( &p[RandOffset], client, SSL_RANDOM_SIZE);
#endif
    memcpy(pcb->ssl->clientRandom, &p[RandOffset], SSL_RANDOM_SIZE);
    //Get session ID Length
    idLen = p[offset];
    offset++;
    offset += idLen;
    //Get cipher suite length
    csLen = (p[offset]<<8) + p[offset + 1];
    offset += 2;
    md5_updateH( &pcb->ssl->finMd5 , p, len );
    sha1_updateH( &pcb->ssl->finSha1, p, len );

    if (csLen  > (len - offset) )
    {
        DEBUGMSG(HTTP_CB_DEBUG, "Chiper suite len is large\n");
        return -1;
    }

    for ( i = 0; i < csLen ; i+=2)
    {
#if 0
        if ((p[offset + i] == 0x00) && (p[offset + i + 1] == SSL_RSA_RC4_128_SHA))
            pcb->ssl->cipher = SSL_RSA_RC4_128_SHA;
#else
        if ((p[offset + i] == 0x00) && (p[offset + i + 1] == TLS_RSA_WITH_AES_128_CBC_SHA))
            pcb->ssl->cipher = TLS_RSA_WITH_AES_128_CBC_SHA;
        else if ((p[offset + i] == 0x00) && (p[offset + i + 1] == TLS_PSK_WITH_AES_128_CBC_SHA))
            pcb->ssl->cipher = TLS_PSK_WITH_AES_128_CBC_SHA;
#endif
    }

    if (pcb->ssl->cipher == 0)
    {
        DEBUGMSG(HTTP_CB_DEBUG, "No cipher match\n");
        return -1;
    }
    return 0;
}

int processClientHelloSSLv2(unsigned char *p, int len, struct tcp_pcb *pcb)
{
    /* Record layer:
    *     0  .   1   message length
    *
    * SSL layer:
    *     2  .   2   message type
    *     3  .   4   protocol version
    */
    int offset = 2 /*len*/+ 1/*type*/ + 2 /*ver*/ ;

    int csLen = 0; //cipher suite length
    int idLen = 0; //session id len
    int chalLen = 0; //challenge length

    int i = 0;

    i = ( ( p[0] << 8 ) | p[1] ) & 0x7FFF;

    if ( i < 17 || i > 512 )
    {
        return -1;
    }

    md5_updateH( &pcb->ssl->finMd5 , p + 2, i );
    sha1_updateH( &pcb->ssl->finSha1, p + 2, i );

    /*
    *    0  .   1   cipherlist length
    *    2  .   3   session id length
    *    4  .   5   challenge length
    *    6  .  ..   cipherlist
    *   ..  .  ..   session id
    *   ..  .  ..   challenge
    */

    //Get cipher suite length
    csLen = (p[offset]<<8) + p[offset + 1];
    offset += 2;
    //Get session ID Length
    idLen = (p[offset]<<8) + p[offset + 1];
    offset+=2;

    chalLen= (p[offset]<<8) + p[offset + 1];
    offset+=2;

    if ( csLen < 3 || ( csLen % 3 ) != 0 )
        return -1;

    if ( idLen < 0 || idLen > 32 )
        return -1;

    if ( chalLen < 8 || chalLen > 32 )
        return -1;

    if (i != ( 3 + 6 + csLen + idLen + chalLen))
        return -1;

    memset( pcb->ssl->clientRandom, 0, SSL_RANDOM_SIZE );
    memcpy( pcb->ssl->clientRandom , p + offset + csLen, chalLen );

    if (csLen  > (len - offset) )
    {
        return -1;
    }

    for ( i = 0; i < csLen ; i+=3)
    {
        if ((p[offset + i] == 0x00) && (p[offset + i + 1] == 0x00) && (p[offset + i + 2] == TLS_RSA_WITH_AES_128_CBC_SHA))
            pcb->ssl->cipher = TLS_RSA_WITH_AES_128_CBC_SHA;
    }

    if (pcb->ssl->cipher == 0)
    {
        return -1;
    }
    return 0;
}

/**
* Description:	Make server hello message
*
*
* Parameters:
*				buf -> buffer start address
*				pcb -> Connection
*
* Returns:		None
*
*/
int serverHello(unsigned char* buf, struct tcp_pcb *pcb)
{
    int i = 0;
    int offset = 6 + SSL_RANDOM_SIZE;
    time_t t;
    //HandShake type
    buf[0] = HS_SERVER_HELLO;
    //Protocol message length
    buf[1] = 0;
    buf[2] = 0;
    /* byte 3 is calculated later */
    //TLS version
    buf[4] = 0x03;
    buf[5] = 0x01;

    //server random = Time + 28 byte random
    t = time( NULL );
    buf[i + 6] = (unsigned char)( t >> 24 );
    buf[i + 7] = (unsigned char)( t >> 16 );
    buf[i + 8] = (unsigned char)( t >>  8 );
    buf[i + 9] = (unsigned char)( t       );
    i = 4;
    //srand must once for 8168DP issue(if srand, the rand number will the same before previous)
    //srand(time(NULL));

    for ( ; i < SSL_RANDOM_SIZE; i++)
        buf[i + 10 - 4] = (unsigned char)rand();
#ifdef TLSTEST
    memcpy(&buf[6], server, SSL_RANDOM_SIZE);
#endif
    memcpy(pcb->ssl->serverRandom, &buf[6], SSL_RANDOM_SIZE);
    //Session ID length
    buf[offset] = SESSION_ID_LEN;
    offset++;

    //Generate a session id, and assign to SSL
    if (pcb->ssl->resume == 0)
    {
        for ( i=0; i < SSL_RANDOM_SIZE ; i++)
        {
            buf[offset] = (unsigned char)rand();
            pcb->ssl->sessionID[i] = buf[offset];
            offset++;
        }
    }
    else
    {
        resumeInitKey(1, buf, pcb);
        memcpy(&buf[offset], pcb->ssl->sessionID, SSL_RANDOM_SIZE);
        offset+= SSL_RANDOM_SIZE;
    }

    //Set Cipher suite
    buf[offset] = 0x0;
    offset++;
    buf[offset] = pcb->ssl->cipher;
    offset++;
    //no compression
    buf[offset] = 0;
    offset++;
    //handshake data size not include handshake type(1) and length(3)
    buf[3] = offset - 4;

    md5_updateH( &pcb->ssl->finMd5 , buf, offset);
    sha1_updateH( &pcb->ssl->finSha1, buf, offset);

    return 0;
}

/**
* Description:	Make server cerificate message
*
*
* Parameters:
*				buf -> buffer start address
*				dbytelen -> cer len
*				datasize ->	cer len
*				pcb -> Connection
*
* Returns:		None
*
*/

int serverCertificate(unsigned char* buf, int dbytelen, int datasize, struct tcp_pcb *pcb,unsigned char* certificate)
{
    //Because the .cer will contain 0x1A and 0xFF, open file with flag b
    //FILE *f = fopen("RTTLS.cer","rb");
    //char ch;
    int i =0;
#ifdef CONFIG_PROVISIONING
    int ret;
#endif
    int size = 3 + 3 + 2 +  dbytelen + datasize;//cer len(2) + cer len(2) + header(2)
    buf[0] = HS_CERTIFICATE;
    buf[1] = (size & 0xff0000)>>16;
    buf[2] = (size & 0xff00)>>8;
    buf[3] = size & 0xff;
    //First cerificate length
    size -=3;
    buf[4] = (size & 0xff0000)>>16;
    buf[5] = (size & 0xff00)>>8;
    buf[6] = size & 0xff;
    //Second cerificate length
    size -=3;
    buf[7] = (size & 0xff0000)>>16;
    buf[8] = (size & 0xff00)>>8;
    buf[9] = size & 0xff;
    //cer file size
    size = 2 +  dbytelen + datasize;
    //Because the .cer will contain 0x1A and 0xFF, do not use EFO to judge this file is end
    while (i < size)
    {
        //ch = fgetc(f);
        //buf[i + 10] = ch;
        buf[i + 10] = certificate[i];
        i++;
    }
    // 4 is header size(Type(1) + length(3))
    // 6 is byte length of two cerificate length(3*2)
    md5_updateH( &pcb->ssl->finMd5 , buf, size+ 6 + 4);
    sha1_updateH( &pcb->ssl->finSha1, buf, size+ 6 + 4);

#ifdef CONFIG_PROVISIONING
    if (dpconf->ProvisioningState != PROVISIONED)
    {
        pcb->ssl->own_cert = (x509_cert *)malloc(sizeof(x509_cert));
        memset( pcb->ssl->own_cert, 0, sizeof( x509_cert ) );
        ret = x509parse_crt( pcb->ssl->own_cert, (unsigned char *) certificate,  size);
        if ( ret != 0 )
        {
            x509_free( pcb->ssl->own_cert  );
            memset( pcb->ssl->own_cert, 0, sizeof( x509_cert ) );
            free(pcb->ssl->own_cert);
            return( ret );
        }
#if 0
        x509_free( pcb->ssl->own_cert  );
        memset( pcb->ssl->own_cert, 0, sizeof( x509_cert ) );
        free(pcb->ssl->own_cert);
#endif
    }
#endif
    return 0;
}

/**
* Description:	Fill server Hello done column
*
*
* Parameters:
*				Start address to fill hello done
*				pcb -> Connection
*
* Returns:		None
*
*/
void serverHelloDone(unsigned char* buf, struct tcp_pcb *pcb)
{
    buf[0] = HS_SERVER_HELLO_DONE;
    //Set message length to 2
    memset(buf + 1, 0, 3);

    md5_updateH( &pcb->ssl->finMd5 , buf, 4);
    sha1_updateH( &pcb->ssl->finSha1, buf, 4);
}

void serverCertificateReq(unsigned char *buf, struct tcp_pcb *pcb)
{
    buf[0] = HS_CERT_REQ;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x04;
    buf[4] = 0x01;
    buf[5] = 0x01;
    buf[6] = 0x00;
    buf[7] = 0x00;
    md5_updateH( &pcb->ssl->finMd5 , buf, 8);
    sha1_updateH( &pcb->ssl->finSha1, buf, 8);
}

void serverKeyExchange(unsigned char *buf, struct tcp_pcb *pcb)
{
    buf[0] = HS_SERVER_KEY_XCHG;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x0a;
    buf[4] = 0x00;
    buf[5] = 0x08;
    memcpy(&buf[6], dpconf->pskdata.PID, 8);
    md5_updateH( &pcb->ssl->finMd5, buf, 14);
    sha1_updateH( &pcb->ssl->finSha1, buf, 14);
}

/**
* Description:	Send the entire server hello sequence
*
*
* Parameters:
*				pcb -> Connection
*
* Returns:		None
*
*/

int sendServerHelloSequence(struct tcp_pcb *pcb)
{
    int size = 0;
    unsigned int i = 0;
    int tmp = 0;
    int tlsDataLenByte = 0;
    int tlsDataSize = 0;
    unsigned char c;
    unsigned char* certificate = 0;
    unsigned int cerSize = 0;
    PKT *pkt;

    certificate = getCertificate(&cerSize);
    //Calculate the packet size
    //FILE *f;
    if (pcb->ssl->resume)
    {
        //Only Server Hello
        size = TLS_RECORD_SIZE + TLS_SHELLO_SIZE;
        pkt = (PKT*)allocPkt_F(size);

        makeTLSRecordLayer(pkt->wp, RECORD_HANDSHAKE_PROTOCOL, size - TLS_RECORD_SIZE, pcb);
        serverHello(pkt->wp + TLS_RECORD_SIZE, pcb);

        tcpEnqueue_F(pcb, pkt);
        tcpSend_F(pcb, pkt, TCP_PSH | TCP_ACK);
    }
    else
    {
        if (pcb->ssl->cipher == TLS_RSA_WITH_AES_128_CBC_SHA)
        {
            //Record layer size is 5; Server Hello is 74
            //Hello done is 4
            size = TLS_RECORD_SIZE + TLS_SHELLO_SIZE + 4;
            //Server certificate size
            //type(1) +total len(3)+cer len(3)+cer len(3)+ offset<header>(2)
            size += 12;
            do {
                c = certificate[i];
                if (i == 1)
                {
                    //the bytes of cer data length = offset 1(bit 0-3)
                    tlsDataLenByte = c & 0x0f;
                    tmp = tlsDataLenByte;
                    i++;
                    //the bytes of cerdata length <header>
                    size +=tmp;
                    continue;
                }

                if (tmp != 0)
                {
                    tlsDataSize = (tlsDataSize<<8) | (c&0xff);
                    tmp--;
                    if (tmp == 0)
                        break;
                }
                i++;
            } while (i < cerSize);
            //fclose(f);
            //cer data size
            if (dpconf->ProvisioningState != PROVISIONED)
                size += tlsDataSize + 8;
            else
                size += tlsDataSize;
        }
        else if (pcb->ssl->cipher == TLS_PSK_WITH_AES_128_CBC_SHA)
        {
            size = TLS_RECORD_SIZE + TLS_SHELLO_SIZE + 14 + 4; /*psk server hello size + psk server key xchg size + done size*/
        }
        pkt = (PKT*)allocPkt_F(size);

        makeTLSRecordLayer(pkt->wp, RECORD_HANDSHAKE_PROTOCOL, size - TLS_RECORD_SIZE, pcb);
        serverHello(pkt->wp + TLS_RECORD_SIZE, pcb);
        if (pcb->ssl->cipher == TLS_RSA_WITH_AES_128_CBC_SHA)
        {
            serverCertificate(pkt->wp + TLS_RECORD_SIZE + TLS_SHELLO_SIZE, tlsDataLenByte, tlsDataSize, pcb, certificate);
            if (dpconf->ProvisioningState != PROVISIONED)
                serverCertificateReq(pkt->wp + size - 4 - 8, pcb);
        }
        else if (pcb->ssl->cipher == TLS_PSK_WITH_AES_128_CBC_SHA)
        {
            serverKeyExchange(pkt->wp + TLS_RECORD_SIZE + TLS_SHELLO_SIZE, pcb);
        }
        serverHelloDone(pkt->wp + size - 4, pcb);
#if 1
        if (pkt->len - ETH_PAYLOAD_LEN > 0)
        {
            PKT* tmppkt = (PKT*)allocPkt_F(ETH_PAYLOAD_LEN);
            PKT* tmppkt2 = (PKT*)allocPkt_F(pkt->len - ETH_PAYLOAD_LEN);
            tmppkt->len = ETH_PAYLOAD_LEN;
            tmppkt2->len = pkt->len - ETH_PAYLOAD_LEN;
            memcpy(tmppkt->wp, pkt->wp, ETH_PAYLOAD_LEN);
            memcpy(tmppkt2->wp, pkt->wp + ETH_PAYLOAD_LEN, pkt->len - ETH_PAYLOAD_LEN);
            freePkt_F(pkt);
            tcpEnqueue_F(pcb, tmppkt);
            tcpSend_F(pcb, tmppkt, TCP_PSH | TCP_ACK);
            pkt = tmppkt2;
        }
#endif
        tcpEnqueue_F(pcb, pkt);
        tcpSend_F(pcb, pkt, TCP_PSH | TCP_ACK);
    }
    return 0;
}

void tlsPktSend(struct tcp_pcb *pcb, PKT *pkt)
{
    tcpEnqueue_F(pcb, pkt);
    tcpSend_F(pcb, pkt, TCP_PSH | TCP_ACK);
}

/**
* Description:	Process the handshake record.
*
*
* Parameters:
*				buf -> Protocol message start sddress
*				len -> total TLS length of this packet
*				PCB -> Connection
*
* Returns:		The size of this handshake message
*
*/

int processClientKeyExchange(unsigned char *buf, int len, struct tcp_pcb *pcb)
{
    int msgLen = (((unsigned char)buf[1])<<16) + (((unsigned char)buf[2])<<8) + (unsigned char)buf[3];
    int offset = 4;

    md5_updateH( &pcb->ssl->finMd5 ,buf, msgLen + 4);
    sha1_updateH( &pcb->ssl->finSha1, buf, msgLen + 4);

    //Get how many bytes to present length
    do
    {
        if (msgLen&0xf0)
        {
            offset += 2;
            msgLen = msgLen>>16;
        }
        else
        {
            offset++;
            msgLen = msgLen>>8;
        }
    } while (msgLen);

    /*
    * Determine the appropriate key, IV and MAC length.
    */
    return tlsInitKeys(1, &buf[offset], pcb);
}

/**
* Description:	parse change cipher spec.
*
*
* Parameters:
*				buf -> Protocol message start sddress
*				len -> Protocol message total length
*				PCB -> Connection
*
* Returns:		The size of this handshake message
*
*/
int parseChangeCipherSpec(unsigned char *p, int len, struct tcp_pcb *pcb)
{
    if (pcb->ssl->recordLayer.contentType != RECORD_CHANGE_CIPHER_SPEC ||
            pcb->ssl->recordLayer.size!= 1 || p[0] !=1)
    {
        DEBUGMSG(HTTP_CB_DEBUG, "parseChangeCipherSpec fail\n");
        return -1;
    }
    else
    {
        DEBUGMSG(HTTP_CB_DEBUG,"parseChangeCipherSpec pass\n");
    }
    return 0;
}

/**
* Description:	Decrypt data, check HMAC and calculate verify data.
*
*
* Parameters:
*				p -> Protocol message start sddress
*				len -> Protocol message total length
*				PCB -> Connection
*
* Returns:		The size of this handshake message
*
*/

int parseClientFinish(unsigned char *p, int len, struct tcp_pcb *pcb)
{
    unsigned char *tp;
    unsigned short size = (((unsigned short)p[3])<<8) + (unsigned short)p[4];
    unsigned short padLen = 0;
    short i = 0;

    INT8U *buf = malloc(36);
    INT8U *tmp = malloc(20);
    INT8U *tmp1 = malloc(100);
    md5_context  *tMd5 = malloc(sizeof(md5_context));
    sha1_context *tSha1 = malloc(sizeof(sha1_context));

    memset(tmp1, 0, 100);
    memcpy(tMd5, &pcb->ssl->finMd5, sizeof(md5_context));
    memcpy(tSha1, &pcb->ssl->finSha1, sizeof(sha1_context));

    if (pcb->ssl->recordLayer.size < pcb->ssl->minLen)
    {
        DEBUGMSG(HTTP_CB_DEBUG,"short  message length in parseClientFinish\n");
    }
    /*
    * TLS encrypted messages can have up to 256 bytes of padding
    */

    if (pcb->ssl->recordLayer.size > (pcb->ssl->minLen + SSL_MAX_CONTENT_LEN + 256))
    {
        DEBUGMSG(HTTP_CB_DEBUG,"large message length in parseClientFinish\n");
    }


    if ( pcb->ssl->ivLen == 0 )
    {
#if 0
        padLen = 0;
        arc4_crypt( (arc4_context *) pcb->ssl->ctxDec,
                    p, pcb->ssl->recordLayer.size);
#endif
    }
    else
    {
        if (pcb->ssl->recordLayer.size % pcb->ssl->ivLen !=0 )
        {
            DEBUGMSG(HTTP_CB_DEBUG,"Error aes message length: %d\n", pcb->ssl->recordLayer.size);
            free(buf);
            free(tmp);
            free(tmp1);
            free(tMd5);
            free(tSha1);
            return -1;
        }
        switch (pcb->ssl->ivLen)
        {
        case 16:
            aes_crypt_cbc( (aes_context*) pcb->ssl->ctxDec,
                           AES_DECRYPT, pcb->ssl->recordLayer.size,
                           pcb->ssl->ivDec, p, p);
            break;
        default:
            DEBUGMSG(HTTP_CB_DEBUG,"Incorrect IV Length");
            free(buf);
            free(tmp);
            free(tmp1);
            free(tMd5);
            free(tSha1);
            return -1;
        }
        padLen = 1 + p[pcb->ssl->recordLayer.size - 1];

        /*
        * TLSv1: always check the padding
        */
        for ( i = 1; i <= padLen; i++ )
        {
            if ( p[pcb->ssl->recordLayer.size - i] != padLen - 1 )
            {
                padLen = 0;
            }
        }
    }

    pcb->ssl->recordLayer.size = pcb->ssl->recordLayer.size - (pcb->ssl->macLen + padLen);
    //Compute the MAC
    memcpy(tmp, p + pcb->ssl->recordLayer.size, 20);
    memcpy(tmp1, pcb->ssl->inCounter, 8);

    tp = p - TLS_RECORD_SIZE;
    tp[3] = (unsigned char)pcb->ssl->recordLayer.size >> 8;
    tp[4] = (unsigned char)pcb->ssl->recordLayer.size;
    memcpy(tmp1 + 8, tp, TLS_RECORD_SIZE + 16);

    if ( pcb->ssl->macLen == 16 )
        md5_hmac( pcb->ssl->macDec, 16,
                  tmp1,  pcb->ssl->recordLayer.size + 13,
                  p + pcb->ssl->recordLayer.size );
    else
        sha1_hmac( pcb->ssl->macDec, 20,
                   tmp1,  pcb->ssl->recordLayer.size + 13,
                   p + pcb->ssl->recordLayer.size);

    if ( memcmp( tmp, p + pcb->ssl->recordLayer.size,
                 pcb->ssl->macLen ) != 0 )
    {
        DEBUGMSG(HTTP_CB_DEBUG,"Client finish:Mac does not match\n");
        free(buf);
        free(tmp);
        free(tmp1);
        free(tMd5);
        free(tSha1);
        return -1;
    }

    md5_updateH( &pcb->ssl->finMd5 , p, 16 );
    sha1_updateH( &pcb->ssl->finSha1, p, 16 );

    ssl_calc_finished(pcb->ssl->master, buf, CFINISH, tMd5, tSha1);

    if (memcmp( p + 4, buf, VERIFY_DATA_LEN ) != 0)
    {
        DEBUGMSG(HTTP_CB_DEBUG,"Client finish:Message do not match\n");
        free(buf);
        free(tmp);
        free(tmp1);
        free(tMd5);
        free(tSha1);
        return -1;
    }

    for ( i = 7; i >= 0; i-- )
        if ( ++pcb->ssl->inCounter[i] != 0 )
            break;

    free(buf);
    free(tmp);
    free(tmp1);
    free(tMd5);
    free(tSha1);
    return size;
}

/**
* Description:	Send change cipher spec and server finish
*
*
* Parameters:
*				P -> start address to fill server finish content
*				pcb -> Connection
*
* Returns:		None
*
*/
int serverFinish(unsigned char *p, struct tcp_pcb *pcb)
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
    //4 = message type(1) + length(3)
    ssl_calc_finished(pcb->ssl->master, p + 4, SFINISH, tMd5, tSha1);

    p[0] = HS_FINISHED;
    p[1] = (unsigned char)((len - 4) >> 16);
    p[2] = (unsigned char)((len - 4) >> 8);
    p[3] = (unsigned char)len - 4;

    md5_updateH(&pcb->ssl->finMd5 , p, len);
    sha1_updateH(&pcb->ssl->finSha1, p, len);


    memcpy(tmp1, pcb->ssl->outCounter, 8);
    memcpy(tmp1 + 8 , p - 5, 5);
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

/**
* Description:	Send change cipher spec and server finish
*
*
* Parameters:
*				pcb -> Connection
*
* Returns:		None
*
*/

int sendServerFinish(struct tcp_pcb *pcb)
{
    //TLS_RECORD_SIZE*2: change cipher spec + server finish
    //1:change cipher spec messgae
    //36 = Message type(1) + message len(3) + Finish message(12byte) + MAC(20 for SHA1)
    int size = TLS_RECORD_SIZE*2 + 1 + 36 + checkPadLen(pcb->ssl->ivLen, 36);
    PKT* pkt;

    //checkPadLen only return padding content length, not include padding field
    if (checkPadLen(pcb->ssl->ivLen, 36))
        size += 1;

    pkt = (PKT*)allocPkt_F(size);

    makeTLSRecordLayer(pkt->wp, RECORD_CHANGE_CIPHER_SPEC, 1, pcb);
    pkt->wp[TLS_RECORD_SIZE] = 0x01;

    makeTLSRecordLayer(pkt->wp + TLS_RECORD_SIZE + 1, RECORD_HANDSHAKE_PROTOCOL, 16, pcb);
    serverFinish(pkt->wp + TLS_RECORD_SIZE*2 + 1, pcb);

    if (checkPadLen(pcb->ssl->ivLen, 36))
        makeTLSRecordLayer(pkt->wp + TLS_RECORD_SIZE + 1, RECORD_HANDSHAKE_PROTOCOL,1 + 36 + checkPadLen(pcb->ssl->ivLen, 36), pcb);
    else
        makeTLSRecordLayer(pkt->wp + TLS_RECORD_SIZE + 1, RECORD_HANDSHAKE_PROTOCOL, 36, pcb);

    tcpEnqueue_F(pcb, pkt);
    tcpSend_F(pcb, pkt, TCP_PSH | TCP_ACK);
    return 0;
}

void sendTLSWebPage(unsigned char *buf, int offset, struct tcp_pcb *pcb)
{
    PKT *tlsPkt;

#if 1
	if(offset>1380){

	    tlsPkt = allocTLSPkt(pcb, 1024);
	    memcpy(tlsPkt->wp, buf, 1024);
	    tlsEncodeData_F(tlsPkt, 1024, pcb, RECORD_APP_PROTOCOL_DATA);

	 offset = offset - 1024;
	 
	    tlsPkt = allocTLSPkt(pcb, offset);
	    memcpy(tlsPkt->wp, buf+1024, offset);
	    tlsEncodeData_F(tlsPkt, offset, pcb, RECORD_APP_PROTOCOL_DATA);
		
	}else{
    tlsPkt = allocTLSPkt(pcb, offset);
    memcpy(tlsPkt->wp, buf, offset);
    tlsEncodeData_F(tlsPkt, offset, pcb, RECORD_APP_PROTOCOL_DATA);
}
	#else
	tlsPkt = allocTLSPkt(pcb, offset);
	    memcpy(tlsPkt->wp, buf, offset);
	    tlsEncodeData_F(tlsPkt, offset, pcb, RECORD_APP_PROTOCOL_DATA);	
	#endif
	
}

void sendTLSAlert(ptcp_pcb pcb)
{
    PKT *tlsPkt;
    tlsPkt = allocTLSPkt(pcb, 2);
    tlsPkt->wp[0] = 0x02;
    tlsPkt->wp[1] = 0x00;
    tlsEncodeData_F(tlsPkt, 2, pcb, RECORD_ALERT_PROTOCOL);
    tcpSend_F(pcb, NULL, TCP_RST | TCP_ACK);
}

#if 0
void removeSSLSession(unsigned char *p, ptcp_pcb pcb)
{
    SSL* curSSL = ActiveSSL;

    while (curSSL)
    {
        if (pcb->ssl && memcmp(pcb->ssl->sessionID , curSSL->sessionID, SESSION_ID_LEN) == 0)
        {
            if (pcb->ssl->peer_cert != NULL)
            {
                x509_free( pcb->ssl->peer_cert  );
                memset( pcb->ssl->peer_cert, 0, sizeof( x509_cert ) );
                free(pcb->ssl->peer_cert);
            }
            if (pcb->ssl->own_cert!=NULL)
            {
                x509_free( pcb->ssl->own_cert  );
                memset( pcb->ssl->own_cert, 0, sizeof( x509_cert ) );
                free(pcb->ssl->own_cert);
            }
            if (pcb->ssl->ca_crl!=NULL)
            {
                x509_crl_free( pcb->ssl->ca_crl  );
            }
            free(pcb->ssl);
            pcb->ssl = 0;
            curSSL = 0;
            ActiveSSL = 0;
        }
        else
            curSSL = curSSL->next;
    }
}
#endif
