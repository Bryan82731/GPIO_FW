/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	http.c

Abstract:	http module

*****************************************************************************
*/

/*--------------------------Include File------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bsp.h"
#include "tcp.h"
#include "http.h"
#include "htm.h"
#include "post.h"
#include "dhcp.h"
#include "lib.h"
#include "md5.h"
#include "b64.h"
#include "rtskt.h"
#include "app_cfg.h"
#ifdef CONFIG_MSAD_ENABLED
#include "krb.h"
#include "gssapi.h"
#include "spnego.h"
#endif

extern const unsigned char headerHtm[];
extern DPCONF *dpconf;

#ifdef CONFIG_PortMatch_Test
char PortTest = 0;
char CPortTest = 0;
char C6PortTest = 0;
PMMsg GPMMsg;
extern PMTPortRecord PMTPRs[MaxPortMatchRecords];
#endif


#ifdef  CONFIG_WEB_SKT_STYLE
OS_STK  WebThreadStk[TASK_WebSrvThread_STK_SIZE];
#endif

OS_EVENT *WebSem;


HTTPAuthorization m_httpAuthcredential[HTTP_AUTH_MAX];



extern void setLanguageType(enum LanguageType type);
/*--------------------------Exported variables------------------------------*/


/*--------------------------Global variables--------------------------------*/
const char responseHeader[] ={"HTTP/1.1 200 \r\nContent-type: text/html\r\nContent-Length: 41\r\n\r\n<html><body><p>Hello</p></body></html>\r\n"};
const char body[] = {"<html><body><p>Hello</p><img src=\"1.jpg\" width=100 height=112 border=0 alt=\"picture\"></body></html>\r\n\0"};
const char UnauthBasic[] = {"HTTP/1.1 401 Unauthorized\r\nConnection: close\r\nWWW-Authenticate: Basic realm=\"OPENWSMAN\"\r\n\r\n401 Authorization required"};
const char UnauthDigest[] = {"HTTP/1.1 401 Unauthorized\r\nConnection: close\r\nWWW-Authenticate: Digest realm=\"OPENWSMAN\", qop=\"auth\", nonce=\"acaf43d6b56a9b9\", opaque =\"72aed704\"\r\n\r\n401 Authorization required"};
const char WebUnauthBasic[] = {"HTTP/1.1 401 Unauthorized\r\nConnection: close\r\nWWW-Authenticate: Basic realm=\"Realtek.com\"\r\n\r\n401 Authorization required"};
const char WebUnauthDigest[] = {"HTTP/1.1 401 Unauthorized\r\nConnection: close\r\nWWW-Authenticate: Digest realm=\"OPENWSMAN\", qop=\"auth\", nonce=\"%x%x\", opaque =\"%x\"\r\n\r\n401 Authorization required"};//Web
const char WebUnauthAD[] = {"HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Negotiate\r\nServer: Apache/2.2.8 (Fedora)\r\nConnection: Keep-Alive\r\n\r\n"};
#if 0
const char WebUnauthAD[] ={
"HTTP/1.1 401 Authorization Required\r\n\
Date: Mon, 3 Jan 2011 03:45:01 GMT\r\n\
Server: Apache/2.2.8 (Fedora)\r\n\
WWW-Authenticate: Negotiate\r\n\
Keep-Alive: timeout=15, max=100\r\n\
Connection: Keep-Alive\r\n"
};
#endif
extern int COOKIE_ID[3];

/*----------------------------Functions-------------------------------------*/


int isLegalAuthcredential(unsigned char *nonce  , unsigned char*opaque , unsigned char *nc){

	int idx = 0;		
	unsigned char *pt;


	int counter = strtol(nc , pt , 16);
	
	for(idx=0;idx<HTTP_AUTH_MAX;idx++){			
		#if 0
			if(memcmp(m_httpAuthcredential[idx].opaque , opaque , strlen(opaque)  ) == 0){
				DEBUGMSG(HTTP_DEBUG, "OK m_httpAuthcredential[%d].opaque = %s\n" , idx , m_httpAuthcredential[idx].opaque);	
				if(memcmp(m_httpAuthcredential[idx].nonce , nonce , strlen(nonce)  ) == 0){

					if(m_httpAuthcredential[idx].nc < counter  ){
						m_httpAuthcredential[idx].nc = counter;
						
						DEBUGMSG(HTTP_DEBUG, "OK nc = %d , old nc is %d\n" , counter , m_httpAuthcredential[idx].nc);

						m_httpAuthcredential[idx].pass  = 1;
						m_httpAuthcredential[idx].time = OSTimeGet();
						
						return 0;
					}else{
						DEBUGMSG(HTTP_DEBUG, "fail nc = %d , old nc is %d\n" , counter , m_httpAuthcredential[idx].nc);
					}
				}else{
					DEBUGMSG(HTTP_DEBUG, "fail nonce = %s\n" , m_httpAuthcredential[idx].nonce , nonce);
				}
			}else{
			#else
			if(memcmp(m_httpAuthcredential[idx].nonce , nonce , strlen(nonce)  ) == 0){
				DEBUGMSG(HTTP_DEBUG, "OK m_httpAuthcredential[%d].nonce = %s\n" , idx , m_httpAuthcredential[idx].nonce);					

				if(m_httpAuthcredential[idx].nc < counter  ){
					m_httpAuthcredential[idx].nc = counter;
						
					DEBUGMSG(HTTP_DEBUG, "OK nc = %d , old nc is %d\n" , counter , m_httpAuthcredential[idx].nc);

					m_httpAuthcredential[idx].pass  = 1;
					m_httpAuthcredential[idx].time = OSTimeGet();
					
					return 0;
				}else{
					DEBUGMSG(HTTP_DEBUG, "fail nc = %d , old nc is %d\n" , counter , m_httpAuthcredential[idx].nc);
				}
				
			}else{
			#endif
			
				DEBUGMSG(HTTP_DEBUG, "fail opaque = %s\n" , m_httpAuthcredential[idx].opaque , opaque);
			} 	
		
	}
	return -1;
}

int setAuthCredential(unsigned char *nonce  , unsigned char*opaque){
	int idx = 0 , sacrifice = -1;	
	 int time , range;

	time = OSTimeGet();

	sacrifice = -1;
	for(idx=0;idx<HTTP_AUTH_MAX;idx++){
		
		if(m_httpAuthcredential[idx].opaque[0] == 0x00){
			
			sacrifice = idx;
			break;
		}
		if(m_httpAuthcredential[idx].pass == 0){
			sacrifice = idx;
		}
	}
	//--------no free space--------------
	
	if(sacrifice == -1){
		sacrifice = 0;
		range = time - m_httpAuthcredential[0].time;
		
		
          	for(idx=1;idx<HTTP_AUTH_MAX;idx++){
		 	 if(range <  ( time - m_httpAuthcredential[idx].time)){
			 	range = time - m_httpAuthcredential[idx].time;
				sacrifice = idx;			
		 	 }else{
		 	 }
          	}
	}
	//---------------------------------
	
		memset(m_httpAuthcredential[sacrifice].nonce , 0 , 32);
		memset(m_httpAuthcredential[sacrifice].opaque , 0 , 16);
		m_httpAuthcredential[sacrifice].pass  = 0; 
		m_httpAuthcredential[sacrifice].time  = 0; 
		
		if(strlen(opaque) < 16){
			memcpy(m_httpAuthcredential[sacrifice].opaque , opaque , strlen(opaque) );
		}else{
			memcpy(m_httpAuthcredential[sacrifice].opaque , opaque , 16 );
		}
		if(strlen(nonce) < 32){
			memcpy(m_httpAuthcredential[sacrifice].nonce , nonce , strlen(nonce) );
		}else{
			memcpy(m_httpAuthcredential[sacrifice].nonce , nonce , 32 );
		}
		m_httpAuthcredential[sacrifice].nc = 0;

}


int releaseAuthCredential(unsigned char *nonce  , unsigned char*opaque){
	int idx;
	
	for(idx=0;idx<HTTP_AUTH_MAX;idx++){
		if(memcmp(m_httpAuthcredential[idx].opaque , opaque , strlen(opaque)  ) == 0){
			if(memcmp(m_httpAuthcredential[idx].nonce , nonce , strlen(nonce)  ) == 0){
				memset(m_httpAuthcredential[idx].opaque , 0x00 , 16 );
				m_httpAuthcredential[idx].pass  = 0; 
				  DEBUGMSG(HTTP_DEBUG, "releaseAuthCredential at  <%d>\n", idx);
				return 0;
			}
			return 3;
		}
		
	}
	return 1;
}



/**
* Description:	Do basic authentication
*
* Parameters:
*				pcb ->			Connection
*				tmp, tmp1 ->	password start address
*
* Returns:		None
*
*/

int basicAuth(struct tcp_pcb *pcb, char *tmp, char *tmp1, HTTPHdrInfo* hinfo)
{
    char *pw;
    char *tmpPW;
    UserInfo* ui;
    int i = 0;
    //Move to basic credential
    tmp = tmp1 + 1;
    tmp1 = strchr(tmp,'\r');
    pw = (char*)malloc((tmp1 - tmp + 1));
    memset(pw, 0, (tmp1 - tmp + 1));
    i = 0;

    pcb->httpAuthPass = UNPASS;

    while ((*tmp !='\n')&&(*tmp != '\r'))
    {
        pw[i] = *tmp;
        tmp++;
        i++;
    }

    //Decode data len is about 3/4 of encode data len
    tmpPW = (char*)malloc(i + 1);
    memset(tmpPW, 0, i+1 );
    b64_decode( pw, tmpPW, i + 1 );

    ui = getuserinfo();

    while (ui)
    {
        i = strchr(tmpPW, ':') - tmpPW;

        if (i == strlen(ui->name))
        {
            //Compare user name
            if (strncmp(tmpPW, ui->name, i) == 0)
            {
                i = strlen(strchr(tmpPW, ':') + 1);
                if (i == strlen(ui->passwd))
                {
                    if (strncmp(strchr(tmpPW, ':') + 1, ui->passwd, i) == 0)
                    {
                        pcb->httpAuthPass = PASS;
                        hinfo->httpAuthPass = PASS;
                        hinfo->uInfo = ui;
                        break;
                    }
                }
            }
        }
        ui = ui->next;
    }

    free(tmpPW);
    free(pw);

    return 0;
}

int digestAuth(struct tcp_pcb *pcb, char *tmp, HTTPHdrInfo* hinfo)
{
    char *digestToken[] =
    {
        "username=",
        "nc=",
        "qop=",
        "uri=",
        "realm=",
        "nonce=",
        "opaque=",
        "cnonce=",
        "response=",
    };

    enum digestItem
    {
        dusername = 0,
        dnc,
        dqop,
        duri,
        drealm,
        dnonce,
        dopaque = 6,
        dcnonce,
        dresponse = 8
    };

    const int DigestTokenCount = 9;

    int itemCount = 0;
    int size = 0;
    char *pch,*head, *tail;
    char *digestItemValue[9] = {0};
    UserInfo* ui;
    INT8U *HA1 = malloc(HASHHEXLEN+1);
    INT8U *HA2 = malloc(HASHHEXLEN+1);
    INT8U *Response = malloc(HASHHEXLEN+1);
    //HASHHEX HA1;
    //HASHHEX HA2 = "";
    //HASHHEX Response;
    int i = 0;


    for (i = 0; i < DigestTokenCount ; i++)
    {
        head = 0;
        tail = 0;

        pch = strstr(tmp, digestToken[i]);
        //Only fist request opaque will be send,
        //subsequent requests opaque will not be send.
        if (pch==0 && i!=dopaque)
            break;
        else if (i == dopaque && pch ==0)
        {
            digestItemValue[dopaque] = 0;
            continue;
        }

        tail = strchr(pch,',');
        head = strchr(pch,'"');

        if ((tail != 0 && head > tail) || (head ==0))
        {
            head = strchr(pch,'=');
            tail = strchr(head + 1,',');
        }
        else
        {
            tail = strchr(head + 1,'"');
        }

        if ((head != 0) &&(tail !=0))
        {
            size = tail - head;
            digestItemValue[i] = (char*)malloc(size);
            memset(digestItemValue[i], 0, size);
            itemCount++;
            memcpy(digestItemValue[i], head + 1, size-1);
        }
    }

    ui = getuserinfo();

    while (ui)
    {
        i = strlen(ui->name);
        if ((strncmp(digestItemValue[dusername], ui->name,i) == 0) && (strlen(digestItemValue[dusername]) == strlen(ui->name)))
            break;;
        ui = ui->next;
    }


    if ((itemCount == 9 || ((itemCount == 8) && (digestItemValue[dopaque] == 0))) && ui)
    {
        DigestCalcHA1("md5", digestItemValue[dusername], digestItemValue[drealm],
                      ui->passwd, digestItemValue[dnonce], digestItemValue[dcnonce], HA1);

        if (pcb->httpMethod == WEBGET)
            DigestCalcResponse(HA1, digestItemValue[dnonce], digestItemValue[dnc],
                               digestItemValue[dcnonce], digestItemValue[dqop],
                               "GET", digestItemValue[duri], HA2, Response);
        else
            DigestCalcResponse(HA1, digestItemValue[dnonce], digestItemValue[dnc],
                               digestItemValue[dcnonce], digestItemValue[dqop],
                               "POST", digestItemValue[duri], HA2, Response);

        DEBUGMSG(HTTP_CA_DEBUG, "Calculate Response:%s\n",Response);

        if ( strlen(digestItemValue[dresponse]) < HASHHEXLEN )
        {
            if (strncmp( digestItemValue[dresponse], (char*)Response, strlen(digestItemValue[dresponse])) == 0)
                pcb->httpAuthPass = PASS;
        }
        else
        {
            if (strncmp( digestItemValue[dresponse], (char*)Response, HASHHEXLEN) == 0)
                pcb->httpAuthPass = PASS;
        }
		
	
	
    }

	if (pcb->httpAuthPass == PASS){
		
		if(isLegalAuthcredential(digestItemValue[dnonce] ,  digestItemValue[dopaque] ,   digestItemValue[dnc] ) ==0){
			   DEBUGMSG(HTTP_DEBUG, "HTTPS Digest Auth OK\n");	
		}else{
			   DEBUGMSG(HTTP_DEBUG, "HTTPS Digest Auth Fail\n");
			   pcb->httpAuthPass = UNPASS;
    }

	}
	

    if (pcb->httpAuthPass == PASS)
    {
        hinfo->httpAuthPass = PASS;
        hinfo->uInfo = ui;
    }else{
    	   DEBUGMSG(HTTP_DEBUG, "digestAuth : not pass , opaque is <%s>\n",digestItemValue[dopaque] );

	    releaseAuthCredential(digestItemValue[dnonce] ,  digestItemValue[dopaque]  );
		   
    }

    for (i = 0; i < DigestTokenCount ; i++)
    {
        if (digestItemValue[i])
            free(digestItemValue[i]);
    }

    free(HA1);
    free(HA2);
    free(Response);
    return 0;
}

void webPost(struct tcp_pcb *pcb, char *p, int len)
{
    int size = 0;
    PKT *pkt = 0;
 //----------------------
    unsigned int nonceA = 0, nonceB = 0,nonceC = 0;
    unsigned char *authFailStr = NULL;
 //----------------------    	

    if (pcb->httpAuthPass == UNPASS && pcb->TLSEnable && pcb->postURL != AUTHPOST)
    {
        sendStaticWebPage(pcb, AUTHPAGE);
    }
    else if (pcb->httpAuthPass == UNPASS && !pcb->TLSEnable)
    {
        if (dpconf->Security == BASIC_AUTH)
        {
            size = strlen(WebUnauthBasic);
            pkt = (PKT*)allocPkt_F(size);
            memcpy(pkt->wp, WebUnauthBasic, size);
        }
#ifdef CONFIG_MSAD_ENABLED
        if (dpconf->Security == KERB_AUTH)
        {
            size = strlen(WebUnauthAD);
            pkt = (PKT*)allocPkt(size);
            memcpy(pkt->wp, WebUnauthAD, size);
        }
#endif
        if (dpconf->Security == DIGEST_AUTH)
        {       
        	//----------------------
        	  srand(REG32(TIMER_IOBASE + TIMER_CV));
        	  
        	  nonceA = rand() ;	  
	  nonceB = rand() ;	  
	  nonceC = rand() ;	
	  authFailStr = malloc(256);
	  
	  if(authFailStr){
	  	memset(authFailStr,0x00,256);
	  	  sprintf(authFailStr,WebUnauthDigest,nonceA,nonceB,nonceC);
		  size = strlen(authFailStr);
	            pkt = (PKT*)allocPkt_F(size);
	            memcpy(pkt->wp, authFailStr, size);
		  free(authFailStr);		
	 	  authFailStr = NULL;
	  }else{
		 size = strlen(WebUnauthDigest);
	            pkt = (PKT*)allocPkt_F(size);
		  if(pkt){
	            	memcpy(pkt->wp, WebUnauthDigest, size);
		  }
	  }        
        }

        if (pkt)
        {
            tcpEnqueue_F(pcb, pkt);
            tcpClose_F(pcb, pkt);
        }
    }
    else if (pcb->postURL == REMOTECTLPOST)
    {
        remoteCtlSet(pcb, p, len);
    }
    else if (pcb->postURL == SRVPOST)
    {
        serviceSet(pcb, p, len);
    }
    else if (pcb->postURL == NETPOST)
    {
        netSet(pcb, p, len);
    }
    else if (pcb->postURL == AUTHPOST)
    {
        tlsAuth(pcb, p, len);
    }
#if CONFIG_LLDP_ENABLED
	else if (pcb->postURL == LLDPPOST)
    {
       	lldpSetFunc(pcb, p, len); //+briank.rtk LLDP
    }
#endif
#if  CONFIG_WEB_SKT_STYLE || defined (CONFIG_EZSHAE_TASK_ENABLED)
    else if (pcb->postURL == EzSharePOST)
    {
        EzShare(pcb, p, len);
    }
#endif

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
    else if (pcb->postURL == PTNEDTPOST)
    {
        PtnEdt(pcb, p, len);
    }
    else if (pcb->postURL == PTNADDPOST)
    {
        PtnAdd(pcb, p, len);
    }
#endif
#ifdef CONFIG_PRINTER_WEB_PAGE
    else if (pcb->postURL == SMBPOST)
    {
        smbSet(pcb, p, len);
    }
#endif
    //mail_wakeup 20121025 eccheng
#ifdef CONFIG_MAIL_WAKEUP_ENABLED   
    else if (pcb->postURL == MAIL_ACCPOST)
    {
        mail_acc_set(pcb, p, len);
    }
    else if (pcb->postURL == MAIL_MATPOST)
    {
        mail_mat_set(pcb, p, len);    
    }
#endif
#if CONFIG_WIFI_ENABLED	
	else if(pcb->postURL == WIFICFGPOST)
	{
		wificfgfun(pcb, p, len);
	}
#endif

}

/**
* Description:	Do authenticate and send data to wsman
*
* Parameters:
*				p->		Http data start address
*				len ->	Http content length
*				pcb ->	Connection
* Returns:		None
* Notes:		Every http entity headers and data are split by \r\n.
*				Http header ends by \r\n
*/
int	parsePostRequest(char *p, int len, struct tcp_pcb *pcb)
{
    char *tmp = NULL; //Point to current parse position
    char *tmp1 = NULL;
    int v = 0; //Record read length
    unsigned int i = 0;
    char *authMethod;
    HTTPHdrInfo *hinfo = malloc(sizeof(HTTPHdrInfo));

    //HTTP header
    if ( pcb->httpParseStatus == HTTPHEADER )
    {
        v = 0;
        while ( v < len)
        {
            if (v == 0)
            {
                //Begin Header
                //'\n' = 0xa -> line feed
                if ( strncmp(p, "POST ", 4) == 0)
                {
                    tmp = strchr(p, '\n');
                    if ((tmp - p + 1) == len )
                    {
                        //First post request
                        break;
                    }
                }
                else
                {
                    tmp = p - 1;//Middle Header, minus one for tmp++
                }
            }
            else
            {
                //Find next entity
                tmp = strchr(tmp, '\n');
            }

            if (tmp == NULL)
            {
                v = -1;
                DEBUGMSG(HTTP_DEBUG, "Abnormal parse in post header\n");
                break;
            }

            tmp++;

            //Http header end
            if (*tmp  == '\r' && *(tmp + 1) == '\n')
            {
                pcb->httpParseStatus = HTTPDATA;
                tmp+=2;
                break;
            }

            if (strncmp(tmp, "Authorization", strlen("Authorization")) == 0)
            {
                tmp = strchr(tmp, ' ');
                if (tmp == NULL)
                    DEBUGMSG(HTTP_CA_DEBUG, "Error in auth\n");

                //Retrive authMethod
                tmp++;
                tmp1 = strchr(tmp,' ');
                if ((tmp == NULL) || (*tmp == '\n'))
                    DEBUGMSG(HTTP_CA_DEBUG, "Error in auth ethod\n");

                authMethod = (char*)malloc((tmp1 - tmp +1));
                memset(authMethod, 0, (tmp1 - tmp + 1));
                i = 0;
                while ((*tmp !=' ') && tmp < tmp1)
                {
                    authMethod[i] = *tmp;
                    tmp++;
                    i++;
                }
                //Currently tmp == tmp1
                if ( i == strlen("Basic"))
                {
                    if (strncmp("Basic", authMethod, strlen("Basic")) == 0)
                    {
                        basicAuth(pcb, tmp, tmp1, hinfo);
                        pcb->httpAuthMethod = Basic;
                    }
                    else
                        DEBUGMSG(HTTP_CA_DEBUG, "Basic auth fail \n");
                }
                else if ( i == strlen("Digest"))
                {
                    if (strncmp("Digest", authMethod, strlen("Digest")) == 0)
                    {
                        digestAuth(pcb, tmp, hinfo);
                        pcb->httpAuthMethod = Digest;
                    }
                    else
                        DEBUGMSG(HTTP_CA_DEBUG, "Digest auth fail \n");
                }
                else
                {
                    DEBUGMSG(HTTP_CA_DEBUG, "Fatal error-> unknow  Authorization \n");
                }
                DEBUGMSG(HTTP_CA_DEBUG, "%s\n", authMethod);
                free(authMethod);
            }

            if (strncmp(tmp, "Content-Length", strlen("Content-Length")) == 0)
            {
                tmp = strchr(tmp,' ');
                if (tmp == NULL)
                    DEBUGMSG(HTTP_CA_DEBUG,"Error in Content-Length\n");
                tmp++;
                pcb->httpContentLen = 0;
                while (((*tmp & 0xf0) == 0x30) && ((*tmp&0x0f) <= 0x09))
                {
                    pcb->httpContentLen = pcb->httpContentLen *10 + (*tmp - 0x30);
                    tmp++;
                }

                if (pcb->httpReqData)
                    free(pcb->httpReqData);

                pcb->httpReqData = (char*)malloc(pcb->httpContentLen + 1);
                memset(pcb->httpReqData, 0, pcb->httpContentLen +1);
                DEBUGMSG(HTTP_CA_DEBUG,"Http content len is %d\n", pcb->httpContentLen);
            }
            v++;
        }

        //unauthentication
        if ((pcb->httpParseStatus == HTTPDATA) && !pcb->httpAuthPass)
        {
            //ws man will send unauthentication message to client and close this connection
            if (!pcb->apRx/*pcb->localPort == WSMANSRVPORT*/)
            {
                //For signle packet issue
                pcb->httpContentLen = 0;
                pcb->httpReadLen = 1;
                OSSemPost(pcb->skt->rxSem);//ws_input(pcb, p, len);
            }
            else
            {
                webPost(pcb, tmp, pcb->httpContentLen);
                //Do not free wsman task data
                pcb->httpReadLen = 0;
                free(pcb->httpReqData);
                pcb->httpReqData = 0;
                //Only web
                pcb->httpContentLen = 0;
                pcb->httpParseStatus = HTTPINIT;
            }
            free(hinfo);
            return 0;
        }
    }

    //HTML data
    if (pcb->httpParseStatus == HTTPDATA)
    {

        //All packet is data content
        if (tmp == NULL)
        {
            tmp = p;
            memcpy((pcb->httpReqData + pcb->httpReadLen), p, len);
            pcb->httpContentLen -= len;
            pcb->httpReadLen += len;
        }
        else //Packet includes header
        {
            if ( (len - (tmp - p)) < 0 )
            {
                DEBUGMSG(HTTP_DEBUG,"Error len %d %d\n", len, (tmp - p));
                free(hinfo);
                return -1;
            }
            else if ( (len - (tmp - p)) == 0 )
            {
                //If wsman-anon should go to here, and return to get body data.
                //if not wsman-anon, this is a error.
                DEBUGMSG(HTTP_DEBUG,"zero\n");
                //disable for comply, enable for wsman-anon
                //return -1;

            }
            //Header length = tmp - p
            i = tmp - p;
            //Data length = len - Header length
            i = len - i;

            memcpy((pcb->httpReqData + pcb->httpReadLen), tmp, i);
            pcb->httpContentLen = pcb->httpContentLen - i;
            pcb->httpReadLen = pcb->httpReadLen + i;
        }

        if (pcb->httpContentLen == 0)
        {
            v= 0;
            if (!pcb->apRx/*pcb->localPort == WSMANSRVPORT*/)
                OSSemPost(pcb->skt->rxSem);//ws_input(pcb, pcb->httpReqData, pcb->httpReadLen);
            else
            {
                webPost(pcb, pcb->httpReqData, pcb->httpReadLen);

                pcb->httpContentLen = 0;
                pcb->httpReadLen = 0;
                free(pcb->httpReqData);
                pcb->httpReqData = 0;
            }
        }
        else if (pcb->httpContentLen < 0)
            DEBUGMSG(HTTP_DEBUG, "error content len  %d\n", pcb->httpContentLen);
    }
    free(hinfo);
    return 0;
}

/**
* Description:	isPostPath will parse request URL and check if a valid request
*
* Parameters:
*				pcb ->	Connection
*				rul ->	Request URL
*
* Returns:		valid = 1, invlid = 0
*
*/
static int isPostPath(struct tcp_pcb *pcb, char *url)
{
    int i = 0;

    static char *actions[] =
        {	"/remoteCtl",
          "/boot",
          "/security",
          "/netSet",
          "/srvSet",
          "/auth",
#if CONFIG_WIFI_ENABLED
	 "/wificfg",
#endif
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
          "/PEdit",
          "/PAdd",
#endif
#ifdef CONFIG_PRINTER_WEB_PAGE
          "/smbSet",
#endif
          "/EzShare",
          //mail_wakeup 20121025 eccheng
#ifdef CONFIG_MAIL_WAKEUP_ENABLED          
          "/mail_acc_set",
          "/mail_mat_set",
#endif          
                   
#ifdef CONFIG_LLDP_ENABLED
"/lldpSet"  //+ briank.rtk
#endif          
        };

	printf("[RTK] is PostPath\n");
	

    if (url == NULL)
    {
        return 0;
    }

    for (i = 0; i < sizeof(actions)/sizeof(actions[0]) ; i++)
    {
        if (strncmp(url, actions[i], strlen(actions[i])) == 0)
        {
            pcb->postURL = i;
			
			printf("[RTK] is PostPath : postURL = 0x%x\n",pcb->postURL);
			
#ifndef CONFIG_EZSHAE_ENABLED
#ifndef CONFIG_EZSHAE_TASK_ENABLED
            if (pcb->postURL == EzSharePOST)
                break;
#endif
#endif
            return 1;
        }
    }

    //Incorrect post
    sendWebPages(pcb, "NotFound");
    return 0;
}

/**
* Description:	httpPost will call wsman if its URI is /wsman
*
* Parameters:
*				pData-> Http data start address
*				len ->	Http content length
*				pcb ->	Connection
* Returns:		None
*
*/
void httpPost(void *pData, int len, struct tcp_pcb *pcb)
{
    char *tmp;
    char *p =(char*)pData;
    DEBUGMSG(HTTP_DEBUG,"httpPost ==>");

    tmp = strchr(p, '/');

    if (tmp == NULL)
    {
        return;
    }

    if ( strncmp(tmp, "/wsman", strlen("/wsman")) )
    {
        if (!isPostPath(pcb, tmp))
        {
            return;
        }
    }

    if (strncmp(tmp, "/wsman-anon", strlen("/wsman-anon")) == 0)
    {
        ///wsman-anon do not do auth.
        pcb->httpAuthPass = PASS;
    }

    pcb->httpParseStatus = HTTPHEADER;
    parsePostRequest(p, len, pcb);
    DEBUGMSG(HTTP_DEBUG,"httpPost <==");
}

/**
* Description:	check if this conection is authorized
*
* Parameters:
*				pData-> Http data start address
*				len ->	Http content length
*				pcb ->	Connection
* Returns:		1-> Pass, 0-> UNPASS
*
*/

static int chkAuth(void *pData, int len, struct tcp_pcb *pcb)
{
    int i = 0, size = 0;
    char *tmp = NULL;
    char *tmp1 = NULL;
    char *authMethod = NULL;
    char *nonceBuf = NULL, *opaqueBuf = NULL;
    //krb5_error_code min_stat;
    //int type;
#ifdef CONFIG_MSAD_ENABLED
    struct _work_plan *work;
#endif
    unsigned char *auth;
    PKT	 *pkt;
    HTTPHdrInfo *hinfo = malloc(sizeof(HTTPHdrInfo));
    DEBUGMSG(HTTP_DEBUG,"chkAuth ==>, (pcb->httpAuthPass =%x) \n" , pcb->httpAuthPass);


	
	//----------------------
	   unsigned int nonceA = 0, nonceB = 0,nonceC = 0;
	   unsigned char *authFailStr = NULL;
	//----------------------

    if (pcb->httpAuthPass == UNPASS)
    {
#ifdef CONFIG_MSAD_ENABLED
        if (pcb->msad.negostatus)
        {
            tmp = pData;
            goto NEGO;
        }
#endif
        tmp = strchr(pData, '\n');

        while (tmp != NULL)
        {
            tmp++;
            //Http header finish
            if (*tmp  == '\r' && *(tmp + 1) == '\n')
            {
                pcb->httpParseStatus = HTTPDATA;
                tmp+=2;
                break;
            }

            if (strncmp(tmp, "Authorization", strlen("Authorization")) == 0)
            {
                tmp = strchr(tmp, ' ');
                if (tmp == NULL)
                    break;

                //Retrive authMethod
                tmp++;
                tmp1 = strchr(tmp,' ');
                if ((tmp == NULL) || (*tmp == '\n'))
                    DEBUGMSG(HTTP_CA_DEBUG, "Error in auth ethod\n");

                authMethod = (char*)malloc((tmp1 - tmp +1));
                memset(authMethod, 0, (tmp1 - tmp + 1));
                i = 0;
                while ((*tmp !=' ') && tmp < tmp1)
                {
                    authMethod[i] = *tmp;
                    tmp++;
                    i++;
                }
                //Currently tmp == tmp1
                if ( i == (int)strlen("Basic"))
                {
                    if (strncmp("Basic", authMethod, strlen("Basic")) == 0)
                    {
                        basicAuth(pcb, tmp, tmp1, hinfo);
                        pcb->httpAuthMethod = Basic;
                    }
                    else
                        DEBUGMSG(HTTP_CA_DEBUG, "Basic auth fail \n");
                }
#ifdef CONFIG_MSAD_ENABLED
                else if ( i == (int)strlen("Negotiate"))
                {
                    if (strncmp("Negotiate", authMethod, strlen("Negotiate")) == 0)
                    {
NEGO:
                        pcb->msad.auth_buffer = (gss_buffer_t)malloc(sizeof(gss_buffer_desc));
                        pcb->msad.auth_buffer->value = (void*)malloc(4096);
                        memset(pcb->msad.auth_buffer->value,0,4096);
                        auth = strstr(tmp, "\r\n\r\n");
                        if (auth)
                        {
                            pcb->msad.negostatus = 0;
                            if (pcb->msad.rPkt)
                            {
                                memcpy((unsigned char*)pcb->msad.auth_buffer->value, pcb->msad.rPkt->wp, pcb->msad.rPkt->len);
                                memcpy((unsigned char*)pcb->msad.auth_buffer->value + pcb->msad.rPkt->len, tmp, len);
                                pcb->msad.rPkt->len += len;
                                pcb->msad.auth_buffer->length = pcb->msad.rPkt->len;
                                freePkt(pcb->msad.rPkt);
                                pcb->msad.rPkt = 0;
                            }
                            else
                            {
                                memcpy((unsigned char*)pcb->msad.auth_buffer->value, tmp, len);
                                (pcb->msad.auth_buffer->length) = auth-(unsigned char*)tmp;
                            }
                        }
                        else
                        {
                            free(pcb->msad.auth_buffer->value);
                            free(pcb->msad.auth_buffer);
                            if (pcb->msad.rPkt == 0)
                            {
                                pcb->msad.rPkt = allocPkt(4096-PKT_HDR_SIZE);
                                pcb->msad.rPkt->len = 0;
                            }
                            memcpy(pcb->msad.rPkt->wp+pcb->msad.rPkt->len, tmp, len-(tmp-(char*)pData));
                            pcb->msad.rPkt->len += (len-(tmp-(char*)pData));
                            pcb->msad.negostatus = 1;
#if 0
                            pcb->wpid =  getWebPageID(url);
                            if (pcb->wpid==NOTFOUND)
                            {
                                if (chkSMBIOSReqPages(url, &type) == 0)
                                    pcb->smbiosid = -1;
                                else pcb->smbiosid = type;
                            }
                            else
                                pcb->smbiosid = -1;
#endif
                            free(authMethod);
                            free(hinfo);
                            return UNPASS;
                        }
                        b64_decode((unsigned char*)(pcb->msad.auth_buffer->value), (unsigned char*)(pcb->msad.auth_buffer->value), pcb->msad.auth_buffer->length);
                        pcb->msad.auth_buffer->length =( (*( ((unsigned char*)pcb->msad.auth_buffer->value) + 2))<<8 )+ (*( ((unsigned char*)pcb->msad.auth_buffer->value) + 3)) + 4;
                        if (strcmp(pcb->msad.auth_buffer->value, "NTLMSSP") == 0)
                        {
                            pcb->httpAuthPass = UNPASS;
                            pcb->httpAuthMethod = Kerb;
                            free(pcb->msad.auth_buffer->value);
                            free(pcb->msad.auth_buffer);
                        }
                        else
                        {
                            work = (struct _work_plan*)malloc(sizeof(struct _work_plan));
                            work->server_creds = pcb->msad.server_creds;
                            work->s = NULL;
                            if (spnego_worker_bee((void*)work, pcb->msad.auth_buffer))
                            {
                            	int minstat;
                                printf("Authorization required\n");
					gss_release_buffer(&minstat, pcb->msad.auth_buffer);
					free(pcb->msad.auth_buffer);
					pcb->msad.auth_buffer = NULL;	
                                pcb->httpAuthPass = UNPASS;
                            }
                            else
                                pcb->httpAuthPass = PASS;

                            free(work);
                            pcb->httpAuthMethod = Kerb;
                        }
                    }
                    else
                        DEBUGMSG(HTTP_DEBUG, ("Kerb auth fail \n"));
                }
#endif
                else if ( i == (int)strlen("Digest"))
                {
                    if (strncmp("Digest", authMethod, strlen("Digest")) == 0)
                    {
                        digestAuth(pcb, tmp, hinfo);
                        pcb->httpAuthMethod = Digest;
                    }
                    else
                        DEBUGMSG(HTTP_CA_DEBUG, "Digest auth fail \n");
                }
                else
                {
                    DEBUGMSG(HTTP_CA_DEBUG, "Fatal error-> unknow  Authorization \n");
                }
                DEBUGMSG(HTTP_CA_DEBUG, "%s\n", authMethod);
                free(authMethod);
                if (pcb->httpAuthPass != PASS)
                    break;
            }

            if (strncmp(tmp, "Content-Length", strlen("Content-Length")) == 0)
            {
                tmp = strchr(tmp,' ');
                if (tmp == NULL)
                    DEBUGMSG(HTTP_CA_DEBUG,"Error in Content-Length\n");
                tmp++;
                pcb->httpContentLen = 0;
                while (((*tmp & 0xf0) == 0x30) && ((*tmp&0x0f) <= 0x09))
                {
                    pcb->httpContentLen = pcb->httpContentLen *10 + (*tmp - 0x30);
                    tmp++;
                }
                pcb->httpReqData = (char*)malloc(pcb->httpContentLen + 1);
                memset(pcb->httpReqData, 0, pcb->httpContentLen);
                DEBUGMSG(HTTP_CA_DEBUG,"Http content len is %d\n", pcb->httpContentLen);
            }
            tmp = strchr(tmp + 1, '\n');
        }
    }
    else
    {
        free(hinfo);
        return PASS;
    }

    if (pcb->httpAuthPass != PASS)
    {
        //Send authentication request
        if (dpconf->Security == BASIC_AUTH)
        {
            size = strlen(WebUnauthBasic);
            pkt = (PKT*)allocPkt_F(size);
            memcpy(pkt->wp, WebUnauthBasic, size);

            if (pcb->TLSEnable)
            {
                sendTLSWebPage(pkt->wp, size, pcb);
                freePkt_F(pkt);
                tcpClose_F(pcb, NULL);
            }
            else
            {
                tcpEnqueue_F(pcb, pkt);
                tcpClose_F(pcb, pkt);
            }
        }
#ifdef CONFIG_MSAD_ENABLED
        if (dpconf->Security == KERB_AUTH)
        {
            size = strlen(WebUnauthAD);
            pkt = (PKT*)allocPkt(size);
            memcpy(pkt->wp, WebUnauthAD, size);
            if (pcb->TLSEnable)
            {
                sendTLSWebPage(pkt->wp, size, pcb);
                freePkt_F(pkt);
                tcpClose_F(pcb, NULL);
            }
            else
            {
                tcpEnqueue_F(pcb, pkt);
                tcpClose_F(pcb, pkt);
            }
        }
#endif
        if (dpconf->Security == DIGEST_AUTH)
        {
	 //----------------------        
	srand(REG32(TIMER_IOBASE + TIMER_CV));
	 
        	  nonceA = rand() ;	
	  nonceB = rand() ;		
	  nonceC = rand() ;	
	  authFailStr = malloc(256);
	  
	//----------------------


	nonceBuf=malloc(32);
	if(chkAuth != NULL){
		memset(nonceBuf , 0x00 ,32);
		sprintf(nonceBuf , "%x%x" ,nonceA,nonceB );	
	}
	opaqueBuf=malloc(32);
	if(opaqueBuf != NULL){		
		memset(opaqueBuf , 0x00 ,32);
		sprintf(opaqueBuf , "%x" ,nonceC );
		setAuthCredential(nonceBuf , opaqueBuf  );
	}
	
	DEBUGMSG(HTTP_DEBUG,"setAuthCredential [<%s> , <%s> ]\n",nonceBuf , opaqueBuf  );

	if(nonceBuf){
	free(nonceBuf);}
	if(opaqueBuf){
	free(opaqueBuf);}
	
	//----------------------        	  	

	//----------------------------
	srand(REG32(TIMER_IOBASE + TIMER_CV));
	//-----------------------------

	  if(authFailStr){
	  	memset(authFailStr,0x00,256);
	  	sprintf(authFailStr,WebUnauthDigest,nonceA,nonceB,nonceC);
		size = strlen(authFailStr);
	            pkt = (PKT*)allocPkt_F(size);
	            memcpy(pkt->wp, authFailStr, size);
		  free(authFailStr);		
		 authFailStr = NULL;		
	  }else{
		size = strlen(WebUnauthDigest);
	           pkt = (PKT*)allocPkt_F(size);
	           if(pkt){
	           	memcpy(pkt->wp, WebUnauthDigest, size);	  
	           }
	  }
	 //---------------------- 	

            if (pcb->TLSEnable)
            {
                sendTLSWebPage(pkt->wp, size, pcb);
                freePkt_F(pkt);
                tcpClose_F(pcb, NULL);
            }
            else
            {
                tcpEnqueue_F(pcb, pkt);
                tcpClose_F(pcb, pkt);
            }
        }
    }
    else
    {
        if ((pcb->httpParseStatus == HTTPDATA) && (pcb->httpMethod == WEBPOST))
        {
            //Header length = tmp - p
            i = ((int)tmp - (int)pData);
            //Data length = len - Header length
            i = len - i;
            if (pcb->httpContentLen == 0)
            {
                tcpSend_F(pcb, NULL, TCP_ACK);
                free(hinfo);
                return PASS;
            }

            memcpy((pcb->httpReqData + pcb->httpReadLen), tmp, i);
            pcb->httpContentLen = pcb->httpContentLen - i;
            pcb->httpReadLen = pcb->httpReadLen + i;
        }
        free(hinfo);
        return PASS;
    }
    DEBUGMSG(HTTP_DEBUG,"chkAuth Fail");
    free(hinfo);
    return UNPASS;
}
#ifdef CONFIG_MSAD_ENABLED        
int chkAuthDash(void *pData, int len, struct tcp_pcb *pcb)
{
    int i = 0, size = 0;
    char *tmp = NULL;
    char *tmp1 = NULL;
    char *authMethod = NULL;
    //krb5_error_code min_stat;
    //int type;
#ifdef CONFIG_MSAD_ENABLED
    struct _work_plan *work;
#endif
    unsigned char *auth;
    PKT	 *pkt;
    HTTPHdrInfo *hinfo = malloc(sizeof(HTTPHdrInfo));
    DEBUGMSG(HTTP_DEBUG,"chkAuth ==>");

    if (pcb->httpAuthPass == UNPASS)
    {

        tmp = strchr(pData, '\n');

        while (tmp != NULL)
        {
            tmp++;
            //Http header finish
            if (*tmp  == '\r' && *(tmp + 1) == '\n')
            {
                pcb->httpParseStatus = HTTPDATA;
                tmp+=2;
                break;
            }

            if (strncmp(tmp, "Authorization", strlen("Authorization")) == 0)
            {
                tmp = strchr(tmp, ' ');
                if (tmp == NULL)
                    break;

                //Retrive authMethod
                tmp++;
                tmp1 = strchr(tmp,' ');
                if ((tmp == NULL) || (*tmp == '\n'))
                    DEBUGMSG(HTTP_CA_DEBUG, "Error in auth ethod\n");

                authMethod = (char*)malloc((tmp1 - tmp +1));
                memset(authMethod, 0, (tmp1 - tmp + 1));
                i = 0;
                while ((*tmp !=' ') && tmp < tmp1)
                {
                    authMethod[i] = *tmp;
                    tmp++;
                    i++;
                }

#ifdef CONFIG_MSAD_ENABLED

                if ( i == (int)strlen("Negotiate"))
                {
                    if (strncmp("Negotiate", authMethod, strlen("Negotiate")) == 0) 
                    {
NEGO:
                        pcb->httpAuthPass = UNPASS;
                        pcb->msad.auth_buffer = (gss_buffer_t)malloc(sizeof(gss_buffer_desc));
                        pcb->msad.auth_buffer->value = (void*)malloc(4096);
                        memset(pcb->msad.auth_buffer->value,0,4096);
                        auth = strstr(tmp, "\r\n");
                        if (auth)
                        {
                          //  memcpy((unsigned char*)pcb->msad.auth_buffer->value, tmp, len);
                            (pcb->msad.auth_buffer->length) = auth-(unsigned char*)tmp;  
                            memcpy((unsigned char*)pcb->msad.auth_buffer->value, tmp, pcb->msad.auth_buffer->length);                         
                        }
                       
                        b64_decode((unsigned char*)(pcb->msad.auth_buffer->value), (unsigned char*)(pcb->msad.auth_buffer->value), pcb->msad.auth_buffer->length);
                        pcb->msad.auth_buffer->length =( (*( ((unsigned char*)pcb->msad.auth_buffer->value) + 2))<<8 )+ (*( ((unsigned char*)pcb->msad.auth_buffer->value) + 3)) + 4;
                       
                        work = (struct _work_plan*)malloc(sizeof(struct _work_plan));
                        work->server_creds = pcb->msad.server_creds;
                        work->s = NULL;
                        if (spnego_worker_bee((void*)work, pcb->msad.auth_buffer))
                        {
                          int minstat;
                          gss_release_buffer(&minstat, pcb->msad.auth_buffer);
			  free(pcb->msad.auth_buffer);
			  pcb->msad.auth_buffer = NULL;	
                          pcb->httpAuthPass = UNPASS;
                        }
                        else
                          pcb->httpAuthPass = PASS;

                        free(work);
                        pcb->httpAuthMethod = Kerb;
                        
                    }
                    else
                        DEBUGMSG(HTTP_DEBUG, ("Kerb auth fail \n"));
                }
#endif
 
                DEBUGMSG(HTTP_CA_DEBUG, "%s\n", authMethod);
                free(authMethod);
                if (pcb->httpAuthPass != PASS)
                    break;
            }
            tmp = strchr(tmp + 1, '\n');
        }
    }
    else
    {
        free(hinfo);
        return PASS;
    }

    DEBUGMSG(HTTP_DEBUG,"chkAuth Fail");
    free(hinfo);
    return UNPASS;
}
#endif

static int chkWebAuth(void *pData, int len, struct tcp_pcb *pcb, HTTPHdrInfo* hinfo)
{
    int i = 0, size = 0;
    char *tmp = NULL;
    char *tmp1 = NULL;
    char *authMethod = NULL;
    PKT	 *pkt;

    //if (pcb->TLSEnable)
    //    return 1;
	//----------------------
	   unsigned int nonceA = 0, nonceB = 0,nonceC = 0;
	   unsigned char *authFailStr = NULL;
	//----------------------


    if (pcb->httpAuthPass == UNPASS && hinfo->httpParseStatus != HTTPDATA)
    {
        tmp = strchr(pData, '\n');

        while (tmp != NULL)
        {
            tmp++;
            //Http header finish
            if (*tmp  == '\r' && *(tmp + 1) == '\n')
            {
                pcb->httpParseStatus = HTTPDATA;
                hinfo->httpParseStatus = HTTPDATA;
                tmp+=2;
                hinfo->httpDataPos = tmp;
                break;
            }

            if (strncmp(tmp, "Authorization", strlen("Authorization")) == 0)
            {
                tmp = strchr(tmp, ' ');
                if (tmp == NULL)
                    break;

                //Retrive authMethod
                tmp++;
                tmp1 = strchr(tmp,' ');
                if ((tmp == NULL) || (*tmp == '\n'))
                    DEBUGMSG(HTTP_CA_DEBUG, "Error in auth ethod\n");

                authMethod = (char*)malloc((tmp1 - tmp +1));
                memset(authMethod, 0, (tmp1 - tmp + 1));
                i = 0;
                while ((*tmp !=' ') && tmp < tmp1)
                {
                    authMethod[i] = *tmp;
                    tmp++;
                    i++;
                }
                //Currently tmp == tmp1
                if ( i == (int)strlen("Basic"))
                {
                    if (strncmp("Basic", authMethod, strlen("Basic")) == 0)
                    {
                        basicAuth(pcb, tmp, tmp1, hinfo);
                        pcb->httpAuthMethod = Basic;
                        hinfo->httpAuthMethod = Basic;
                    }
                    else
                        DEBUGMSG(HTTP_CA_DEBUG, "Basic auth fail \n");
                }
                else if ( i == (int)strlen("Digest"))
                {
                    if (strncmp("Digest", authMethod, strlen("Digest")) == 0)
                    {
                        digestAuth(pcb, tmp, hinfo);
                        pcb->httpAuthMethod = Digest;
                        hinfo->httpAuthMethod = Digest;
                    }
                    else
                        DEBUGMSG(HTTP_CA_DEBUG, "Digest auth fail \n");
                }
                else
                {
                    DEBUGMSG(CRITICAL_ERROR, "Fatal error-> unknow  Authorization \n");
                }

                free(authMethod);
                if (pcb->httpAuthPass != PASS)
                {
                    hinfo->httpAuthPass = UNPASS;
                    break;
                }
            }

            if (strncmp(tmp, "Content-Length", strlen("Content-Length")) == 0)
            {
                tmp = strchr(tmp,' ');
                if (tmp == NULL)
                    DEBUGMSG(HTTP_CA_DEBUG,"Error in Content-Length\n");
                tmp++;
                pcb->httpContentLen = 0;
                while (((*tmp & 0xf0) == 0x30) && ((*tmp&0x0f) <= 0x09))
                {
                    pcb->httpContentLen = pcb->httpContentLen *10 + (*tmp - 0x30);
                    tmp++;
                }
                //If wsman auth fail, httpReqData will not be free
                if (pcb->httpReqData)
                    free(pcb->httpReqData);
                pcb->httpReqData = (char*)malloc(pcb->httpContentLen + 1);
                memset(pcb->httpReqData, 0, pcb->httpContentLen);
                hinfo->httpContentLen = pcb->httpContentLen;
                DEBUGMSG(HTTP_CA_DEBUG,"Http content len is %d\n", pcb->httpContentLen);
            }

            if (strncmp(tmp, "Content-Type", strlen("Content-Type")) == 0)
            {
                hinfo->httpCharSet = strstr(tmp,"charset=");
                if (hinfo->httpCharSet)
                    hinfo->httpCharSet += strlen("charset=");
            }

            tmp = strchr(tmp + 1, '\n');
        }
    }
    else
    {
        hinfo->httpAuthPass = PASS;
        return PASS;
    }

    if (pcb->httpAuthPass != PASS && hinfo->httpMethod != WSMANPOST)
    {
        //need sktsem???
        //Send authentication request
        if (dpconf->Security == BASIC_AUTH)
        {
            size = strlen(WebUnauthBasic);
            pkt = (PKT*)allocPkt_F(size);
            memcpy(pkt->wp, WebUnauthBasic, size);
            tcpSendData_F(pcb, pkt);
        }

        if (dpconf->Security == DIGEST_AUTH)
        {
        
         srand(REG32(TIMER_IOBASE + TIMER_CV));
		 
        	  nonceA = rand() ;	  
	  nonceB = rand() ;	  
	  nonceC = rand() ;	
	  authFailStr = malloc(256);
	  
	  if(authFailStr){
	  	memset(authFailStr,0x00,256);
	  	sprintf(authFailStr,WebUnauthDigest,nonceA,nonceB,nonceC);
		size = strlen(authFailStr);
	           pkt = (PKT*)allocPkt_F(size);
	           memcpy(pkt->wp, authFailStr, size);
		 free(authFailStr);
		 authFailStr= NULL;
	  }else{
	  	size = strlen(WebUnauthDigest);
	            pkt = (PKT*)allocPkt_F(size);
		  if(pkt){
	            	memcpy(pkt->wp, WebUnauthDigest, size);
		  }		  
	  }
	 //---------------------- 	
            tcpSendData_F(pcb, pkt);
        }
    }
    else if (pcb->httpAuthPass != PASS && hinfo->httpMethod == WSMANPOST)
    {
        hinfo->httpAuthPass = UNPASS;
    }
    else
    {
        //Post request
        if ((pcb->httpParseStatus == HTTPDATA) && (pcb->httpMethod == WEBPOST))
        {
            //Header length = tmp - p
            hinfo->HttpHdrLen = ((int)tmp - (int)pData);
            hinfo->httpDataPos = tmp;
        }
        //Parse Http finish and connect have been authenticated
        hinfo->httpAuthPass = PASS;
        return PASS;
    }

    hinfo->httpAuthPass = UNPASS;
    return UNPASS;
}
/**
* Description:	httpGet will parse http request and send correspondent web pages
*
* Parameters:
*				pData-> Http data start address
*				len ->	Http content length
*				pcb ->	Connection
* Returns:		None
*
*/
void httpGet(void *pData, int len, struct tcp_pcb *pcb)
{
    int i = 0;
    int cookieID = 0;
    char *p =(char*)pData;
    //char path[MAX_WEBPAGE_PATH_LEN] = {0};
#ifdef CONFIG_MSAD_ENABLED
    gss_buffer_t auth_buffer = GSS_C_NO_BUFFER;
#endif
    char *path = malloc(MAX_WEBPAGE_PATH_LEN);
    DEBUGMSG(HTTP_DEBUG,"httpGet ==>\n");

    memset(path, 0, MAX_WEBPAGE_PATH_LEN);
    i = 0;
    //Get request URL Path
    while ( *(p+4+i) !=' ' && i < MAX_WEBPAGE_PATH_LEN)
    {
        path[i] = *(p+4+i);
        i++;
    }

    //Sometime web will reset connection and reset TLS session.
    //Use http session to keep identify client.
    if (pcb->TLSEnable && pcb->httpAuthPass == UNPASS)
    {
        p = strstr(p, "wsmanid=");
        if (p)
        {
            p = p + strlen("wsmanid=");
            sscanf(p ,"%d\r", &cookieID);

            for (i = 0; i < 3;i++)
            {
                if (COOKIE_ID[i] == cookieID)
                {
                    pcb->httpAuthPass = PASS;
                    break;
                }
            }
        }
    }
#ifdef CONFIG_MSAD_ENABLED
    if (pcb->msad.negostatus)
    {
        memcpy(path, pcb->msad.path, strlen(pcb->msad.path) + 1);
        if (pcb->msad.path != NULL)
        {
            free(pcb->msad.path);
            pcb->msad.path = NULL;
        }
    }

    if (pcb->httpAuthPass == UNPASS || (pcb->httpAuthPass == PASS && (strstr(pData, "\r\n\r\n") == NULL)))
    {
        if (pcb->httpAuthPass == PASS && (strstr(pData, "\r\n\r\n") == NULL))
            pcb->httpAuthPass = UNPASS;
	  pcb->msad.auth_buffer = auth_buffer;
        if (chkAuth(pData, len, pcb) == UNPASS)
        {
            if (pcb->msad.negostatus == 1)
            {
                pcb->msad.path = malloc(strlen(path) + 1);
                memcpy(pcb->msad.path, path, strlen(path));
                pcb->msad.path[strlen(path)] = '\0';
            }
            free(path);
            return;
        }
    }
    sendWebPages(pcb, path);
#else
    if (pcb->httpAuthPass == UNPASS)
    {
        if (chkAuth(pData, len, pcb) == UNPASS)
        {
            free(path);
            return;
        }
    }
#if 1
   else{
   	DEBUGMSG(HTTP_DEBUG,"Check Authorization for each packet\n");
   	pcb->httpAuthPass = UNPASS;
   	if (chkAuth(pData, len, pcb) == UNPASS)
	{
		free(path);
	           return;
	}
    }
 #endif 
	DEBUGMSG(HTTP_DEBUG,"sendWebPages\n");
    sendWebPages(pcb, path);
#endif

    free(path);

	DEBUGMSG(HTTP_DEBUG,"< === httpGet \n");

    return;
}


void http_debug(void *pData){
	char *buffer,*plan,*planEnd;
	int len = 0;
	buffer = malloc(64);
	if(buffer){
		memset(buffer,0x00,64);		
		memcpy(buffer,pData,4);
		printf("[RTK] (%s)\n",buffer);
		memset(buffer,0x00,64);
		plan = strstr(pData,"Accept-Language:");
		plan = plan+strlen("Accept-Language:") + 1;
		len = sscanf(plan,"%s\n",buffer);
		printf("[RTK] (%d)<%s>\n",len,buffer);
		
		free(buffer);		
	}else{
		printf("[RTK]http_debug: Alloc Buffer Fail\n ");
	}
}

enum LanguageType getLanguageType(void *pData){
	enum LanguageType lanType;	
	char *buffer,*plang;
		
	lanType = en;
	buffer = malloc(64);
	if(buffer){
		memset(buffer,0x00,64);
		plang = strstr(pData,"Accept-Language:");
		if(plang != NULL){
			plang = plang+strlen("Accept-Language:") + 1;
			sscanf(plang,"%s\n",buffer);
			if(strstr(buffer,"zh-TW")){
					lanType = zh_TW;
			}else if(strstr(buffer,"en")){
				lanType = zh;
			}
		}
		free(buffer);
	}
	
	//printf("[RTK] (%d)<%s>\n",len,buffer);	

	return lanType;
}


/**
* Description:	Http start entry. It will parse http header and judge if a DASH packet
*
* Parameters:
*				pData-> Http data start address
*				len ->	Http content length
*				pcb ->	Connection
* Returns:		None
*
*/

int httpRx(void *pData, int len, struct tcp_pcb *pcb)
{
    //If it is not a multiple byte language, we do not to do any edian transfer
    char *p =(char*)pData;


	

    DEBUGMSG(HTTP_DEBUG,"httpRx ==>\n");

	//http_debug(pData);
	mLanguageType= getLanguageType(pData);
	setLanguageType(mLanguageType);
    if (pcb->TLSEnable && !pcb->TLSDataStage)
    {
        DEBUGMSG(HTTP_DEBUG,"httpRx TLS\n");
        return doTLSAccept((INT8U *) p, len, pcb);
    }
    else if (strncmp(p, "POST /wsman", 11 ) == 0)
    {
        pcb->apRx = 0;
    }
    else if (!dpconf->httpService && !pcb->TLSEnable)
    {
        //http
        tcpClose_F(pcb, NULL);
    }
    else if (!dpconf->httpsService && pcb->TLSEnable)
    {
        //https
        tcpClose_F(pcb, NULL);
    }
    //If we want to setup a web server, we should support Get method
    else
#ifdef CONFIG_MSAD_ENABLED
        if (strncmp(p, "GET ", 4) == 0 || pcb->msad.negostatus )
#else
        if (strncmp(p, "GET ", 4) == 0)
#endif
        {
	#ifdef CONFIG_DISABLE_WEB_MANG
	tcpClose_F(pcb, NULL);	
	#else
		
            pcb->httpMethod = WEBGET;
            DEBUGMSG(HTTP_DEBUG,"httpRx Get\n");
            httpGet(pData, len, pcb);

            if (pcb->TLSEnable)
                pcb->TLSDataStage = 0;

	   DEBUGMSG(HTTP_DEBUG,"httpRx  <== GET \n");
	   return 1;

	#endif

	   
        }//Post method for wsman
        else if (strncmp(p, "POST ", strlen("POST ")) == 0)
        {
            DEBUGMSG(HTTP_DEBUG,"httpRx post\n");
            pcb->httpMethod = WEBPOST;
            httpPost(pData, len, pcb);

            if (pcb->TLSEnable)
                pcb->TLSDataStage = 0;
        }
        else if ( (pcb->httpParseStatus == HTTPHEADER)|| (pcb->httpParseStatus == HTTPDATA) )
        {
            parsePostRequest(p, len, pcb);
        }
    DEBUGMSG(HTTP_DEBUG,"httpRx <==\n");
    return 0;
    //free data will be done at the addEthernetHeader function
}

int getHttpHeaderInfo(unsigned char* data, unsigned int len, struct tcp_pcb *pcb, HTTPHdrInfo* hinfo)
{
    int i = 0;
    char *p =(char*)data;
    char *tmp;
#ifdef CONFIG_MSAD_ENABLED
    gss_buffer_t auth_buffer;
#endif

    if (hinfo->httpParseStatus == HTTPINIT)
        pcb->httpParseStatus = HTTPHEADER;

#ifdef CONFIG_PortMatch_Test
    if (strncmp(p, "PMT", 3) == 0)
    {
        memset(&GPMMsg, 0, sizeof(PMMsg));
        memcpy(&GPMMsg, p, sizeof(PMMsg));
        PortTest = 1;
    }
    else if (strncmp(p, "CPMT", 4) == 0)
    {
        CPortTest = 1;
    }
    else if (strncmp(p, "C6PMT", 5) == 0)
    {
        C6PortTest = 1;
    }else
#endif
        if (strncmp(p, "GET ", 4) == 0)
        {
            pcb->httpMethod = WEBGET;
            hinfo->httpMethod = WEBGET;
            i = 0;
            //Get request URL Path
            while ( *(p+4+i) !=' ' && i < MAX_WEBPAGE_PATH_LEN)
            {
                hinfo->url[i] = *(p+4+i);
                i++;
            }

            chkWebAuth(data, len, pcb, hinfo);
        }//Post method for wsman
        else if (strncmp(p, "POST ", strlen("POST ")) == 0)
        {
            pcb->httpMethod = WEBPOST;
            hinfo->httpMethod = WEBPOST;

            tmp = strchr(p, '/');

            if (tmp == NULL)
            {
                hinfo->invaildPost = 1;
                return 1;
            }

            if ( strncmp(tmp, "/wsman", strlen("/wsman")) )
            {
                if (!isPostPath(pcb, tmp))
                {
                    hinfo->invaildPost = 1;
                    DEBUGMSG(HTTP_DEBUG,"isPostPath");
                    return 1;
                }
            }
            else
            {
                pcb->httpMethod = WSMANPOST;
                hinfo->httpMethod = WSMANPOST;
            }

            if (strncmp(tmp, "/wsman-anon", strlen("/wsman-anon")) == 0)
            {
                ///wsman-anon do not do auth.
                pcb->httpAuthPass = PASS;
            }

            chkWebAuth(data, len, pcb, hinfo);
        }
    return 0;
}

/**
* Description:	Initiate a socket. It should be called after driver initiate ok
*
* Returns:		None
*
*/


void initTCPService()
{
#ifndef CONFIG_DASH_ENABLED
    RTSkt* s = 0;
    RTSkt* s1 = 0;
#endif

#ifdef  CONFIG_WEB_SKT_STYLE

    if (1)
    {
        /*
        s1 = rtSkt(IPv4, IP_PROTO_TCP);
        s1->pcb->apRx = httpRx;
        s1->pcb->TLSEnable = 1;
        rtSktBind(s1, 443);
        rtSktListen(s1, 5);
        */
    }

#else
    if (dpconf->httpService)
    {
#ifndef CONFIG_DASH_ENABLED
        s = rtSkt_F(IPv4, IP_PROTO_TCP);
        s->pcb->apRx = httpRx;
        rtSktBind_F(s, 80);

#if CONFIG_VERSION >= IC_VERSION_DP_RevF
        rtSktEnableIBPortFilter_F(s);
#endif
        rtSktListen_F(s, 5);

#ifdef CONFIG_MSAD_ENABLED
        if (dpconf->Security == KERB_AUTH)
        {
            rtk_server_acquire_creds("host", &s->pcb->server_creds);
        }
#endif

        if (dpconf->IPv6Enable)
        {
            s = rtSkt_F(IPv6, IP_PROTO_TCP);
            s->pcb->apRx = httpRx;
            rtSktBind_F(s, 80);
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
            rtSktEnableIBPortFilter_F(s);
#endif
            rtSktListen_F(s, 5);
        }
#endif
    }
    if (dpconf->httpsService)
    {
#ifndef CONFIG_DASH_ENABLED
        s1 = rtSkt_F(IPv4, IP_PROTO_TCP);
        s1->pcb->apRx = httpRx;
        s1->pcb->TLSEnable = 1;
        rtSktBind_F(s1, 443);
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
        rtSktEnableIBPortFilter_F(s1);
#endif
        rtSktListen_F(s1, 5);
#endif
    }
#endif

}

#ifdef CONFIG_EZSHAE_TASK_ENABLED
void webEZshareSrvThread(void *data)
{
    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_TCP);
    RTSkt* cs;
    PKT* rxPkt;
    int status = 0;
    HTTPHdrInfo hinfo;
    unsigned char err = 0;

    rtSktBind_F(s, 6230);
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F(s);
#endif
    rtSktListen_F(s, 5);

    while (1)
    {
webEZshareSrvThreadAccept:
        memset(&hinfo, 0, sizeof(HTTPHdrInfo));

        cs = rtSktAccept_F(s);
        do
        {
            rxPkt = rtSktRx_F(cs, 0, &status);

            if (status == SKTDISCONNECT)
            {
                rtSktClose_F(cs);
                goto webEZshareSrvThreadAccept;
            }
        }while (rxPkt == 0);

        OSSemPend(SktSem, 0, &err);

        getHttpHeaderInfo(rxPkt->wp, rxPkt->len, cs->pcb, &hinfo);

        if (hinfo.httpAuthPass == PASS && hinfo.httpMethod == WEBGET)
            sendWebPages(cs->pcb, (char*)&hinfo.url[0]);
        else if (hinfo.httpAuthPass == PASS && hinfo.httpMethod == WEBPOST && !hinfo.invaildPost)
        {
            webPost(cs->pcb, (char*)&hinfo.httpDataPos[0], hinfo.httpContentLen);
        }

        freePkt_F(rxPkt);

        OSSemPost(SktSem);

        rtSktClose_F(cs);
    }
}
#endif

#ifdef  CONFIG_WEB_SKT_STYLE
void sotpWebSrvThread(RTSkt* cs)
{
    unsigned char err = 0;

    OSSemDel(cs->rxSem, OS_DEL_ALWAYS, &err);

    if (cs->txSem != 0)
        OSSemDel(cs->txSem, OS_DEL_ALWAYS, &err);

    cs->rxSem = cs->txSem = 0;

    if (cs)
        free(cs);

    OSSemPost(WebSem);
    DEBUGMSG(HTTP_DEBUG,"sotpWebSrvThread\n");

    OSTaskDel(OS_PRIO_SELF);
}

void webSrvThread(void *data)
{
    RTSkt* cs = (RTSkt*)data;
    PKT* rxPkt;
    int status = 0;
    HTTPHdrInfo hinfo;
    unsigned char err = 0;

#if CONFIG_PortMatch_Test
    RTSkt* s = 0;
    PKT* txPkt = 0;
    unsigned char ip[4];
    unsigned char ipv6[IPv6_ADR_LEN] = {0};
#endif

    memset(&hinfo, 0, sizeof(HTTPHdrInfo));

    while (1)
    {
        do
        {
            rxPkt = rtSktRx_F(cs, 0, &status);

            if (status == SKTDISCONNECT)
            {
                sotpWebSrvThread(cs);
                break;
            }
        }while (rxPkt == 0);


        OSSemPend(SktSem, 0, &err);

        getHttpHeaderInfo(rxPkt->wp, rxPkt->len, cs->pcb, &hinfo);

        if (hinfo.httpAuthPass == PASS && hinfo.httpMethod == WEBGET)
            sendWebPages(cs->pcb, (char*)&hinfo.url[0]);
        else if (hinfo.httpAuthPass == PASS && hinfo.httpMethod == WEBPOST && !hinfo.invaildPost)
        {
            webPost(cs->pcb, (char*)&hinfo.httpDataPos[0], hinfo.httpContentLen);
        }

        freePkt_F(rxPkt);

        OSSemPost(SktSem);

#if CONFIG_PortMatch_Test
        if (PortTest || CPortTest || C6PortTest)
        {
            txPkt = (PKT*)allocPkt_F(sizeof(PMMsg));
            memset(txPkt->wp, "test", 4);
        }

        if (PortTest)
        {
            doPMTAct((char*)&GPMMsg);
            PortTest = 0;
        }

        if (CPortTest)
        {
            ip[0]= 192;
            ip[1]= 168;
            ip[2]= 0;
            ip[3]= 123;
            s = rtSkt_F(IPv4, IP_PROTO_TCP);
            rtSktEnableIBPortFilter_F(s);
            rtSktConnect_F(s, ip, 1234);
            OSTimeDly(OS_TICKS_PER_SEC);
            rtSktClose_F(s);
            OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
            OSSemDel(s->txSem, OS_DEL_ALWAYS, &err);
            s->rxSem=0;
            s->txSem=0;
            if (s)
                free(s);
            CPortTest = 0;
        }

        if (C6PortTest)
        {
            ipv6[0] = 0xfe;
            ipv6[1] = 0xc0;
            ipv6[IPv6_ADR_LEN -4] = 0x00;
            ipv6[IPv6_ADR_LEN -3] = 0x00;
            ipv6[IPv6_ADR_LEN -2] = 0x01;
            ipv6[IPv6_ADR_LEN -1] = 0x23;
            s = rtSkt_F(IPv6, IP_PROTO_TCP);
            rtSktEnableIBPortFilter_F(s);
            rtSktConnect_F(s, ipv6, 1234);
            OSTimeDly(OS_TICKS_PER_SEC);
            rtSktClose_F(s);
            OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
            OSSemDel(s->txSem, OS_DEL_ALWAYS, &err);
            s->rxSem=0;
            s->txSem=0;
            if (s)
                free(s);
            C6PortTest = 0;
        }
        if (txPkt)
            rtSktSend_F(cs, txPkt, 0);

#endif

        rtSktClose_F(cs);
        sotpWebSrvThread(cs);
    }
}

void webSrv(void *data)
{
    unsigned char err;
    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_TCP);
    RTSkt* cs;
    OS_TCB pdata;

#if CONFIG_PortMatch_Test
    unsigned char ip[4];
    unsigned char ipv6[IPv6_ADR_LEN] = {0};
    unsigned char mar[MAC_ADDR_LEN];
    //224.0.0.1
    //resetMulticasAddress_F();
    mar[0] = 0x01;
    mar[1] = 0x00;
    mar[2] = 0x5e;
    mar[3] = 0x00;
    mar[4] = 0x00;
    mar[5] = 0x01;

    addMulticasAddress_F(mar);
    mar[0] = 0x33;
    mar[1] = 0x33;
    mar[2] = 0xff;
    mar[3] = 0x00;
    mar[4] = 0x00;
    mar[5] = 0x10;
    addMulticasAddress_F(mar);

    setMulticastList_F();
#endif

    WebSem = OSSemCreate(1);

//	rtSktBind_F(s, 443);
//	rtSktBind_F(s, 1234);
#if CONFIG_PortMatch_Test
    rtSktBind_F(s, 623);
    rtSktListen_F(s, 1);
#else
    rtSktBind_F(s, 80);
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F(s);
#endif
    rtSktListen_F(s, 5);
#endif
    //rtSktBind_F(s, 23);

    //s->pcb->TLSEnable = 1;
    DEBUGMSG(HTTP_DEBUG,"Srv task start\n");

    while (1)
    {
        DEBUGMSG(HTTP_DEBUG, "Pend web Sem\n");
        OSSemPend(WebSem, 0, &err);
//		OSTimeDly(1);

#if 1
        while (OSTaskQuery(TASK_WEB_SRV_THREAD_PRIO, &pdata) == OS_ERR_NONE)
        {
            DEBUGMSG(HTTP_DEBUG, "Task exist");
            OSTaskDel(TASK_WEB_SRV_THREAD_PRIO);
            //system("pause");
        }
#endif
        DEBUGMSG(HTTP_DEBUG, "To telnetSrvAcpt will pend %x\n", s->rxSem);
        cs = rtSktAccept_F(s);
        DEBUGMSG(HTTP_DEBUG, "telnetSrvAcpt Port is %d txsem:%x rxSem:%x \n", cs->pcb->destPort, cs->txSem, cs->rxSem);

        OSTaskCreate(webSrvThread, cs, (unsigned long*)&WebThreadStk[TASK_WebSrvThread_STK_SIZE - 1], TASK_WEB_SRV_THREAD_PRIO);

    }

    for (;;)
    {
        OSTimeDlyHMSM(1, 1, 1, 0);              // delay task for 1 second
    }
}
#endif
