/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	mail_wakeup.c

Abstract:	Mail Wake-up module

*****************************************************************************
*/

/*--------------------------Include File------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
#include "mail_wakeup.h"

#include "bsp.h"
#include "tls_client.h"


#if CONFIG_VERSION <= IC_VERSION_DP_RevF
#define strtok_p strtok
#define strtok_r_p strtok_r
#else
#define strtok_p strtok_rtk
#define strtok_r_p strtok_r_rtk
#endif


//for TLS connection ////////////////////////////////////////////
//#include "tls.h"
//#include "tls_client.h"
//////////////////////////////////////////////////////////////
#define WARRING_MSG_LEN 32

extern DPCONF *dpconf;
int MWU_running = 0;//0 : not running, 1 : running
int m_rounds = 0;
INT32U m_mailIpv4addr = 0;
unsigned char m_mailWarringMsg[WARRING_MSG_LEN];

// Enum Parameter ////////////////////////////////////////////////

typedef enum {
	CENCODING_7BIT = 0,
	CENCODING_8BIT = 1,
	CENCODING_BINARY = 2,
	CENCODING_QUOTED_PRINTABLE = 3,
	CENCODING_BASE64 = 4,
	
	CENCODING_UNKNOWN = 99,
}ContentEncodingTypeEnum;


//debug Parameter ////////////////////////////////////////////////

typedef struct debug_os_mem_ {                   /* MEMORY CONTROL BLOCK                                      */
    void   *OSMemAddr;                    /* Pointer to beginning of memory partition                  */
    void   *OSMemFreeList;                /* Pointer to list of free memory blocks                     */
    INT32U  OSMemBlkSize;                 /* Size (in bytes) of each block of memory                   */
    INT32U  OSMemNBlks;                   /* Total number of blocks in this partition                  */
    INT32U  OSMemNFree;                   /* Number of memory blocks remaining in this partition       */
    INT8U   OSMemName[16];  /* Memory partition name                                     */

} debug_OS_MEM;


//debug function ////////////////////////////////////////////////
void printFreeMem(unsigned int id){

	int idx,len;
	char str[50];
	char *st;

	debug_OS_MEM *debugOSMemTbl;
	debugOSMemTbl = (debug_OS_MEM *)0x8a0198f0;

	
	st = str;
	len = sprintf(st,"[%d]",id);
	st+=len;
	for(idx=0;idx<OS_MAX_MEM_PART;idx++){		
		len = sprintf(st,"%d-",debugOSMemTbl[idx].OSMemNFree );		
		st+=len;
	}
	sprintf(st,"\n");	
	printf(str);
	
	

}


void rtk_printf(char *str){
	int len = strlen(str);
	int idx;

	
	for(idx=0;idx<(len-2);idx++){//the last \r\n will not be replaced
		if((str[idx] == ('\r'))&&(str[idx+1] == ('\n'))){
			str[idx] = '@';
			str[idx+1] = '@';
		}else if(str[idx] == ('\n')){
			str[idx] = '-';
		}	
	}
	printf("[---]%s\n",str);
}
////////////////////////////////////////////////////////////////////


#if CONFIG_VERSION > IC_VERSION_DP_RevF

char *
strtok_r_rtk(char *string, const char *seps, char **context)
{
        char *head;  /* start of word */
        char *tail;  /* end of word */

        /* If we're starting up, initialize context */
        if (string) {
                *context = string;
        }

        /* Get potential start of this next word */
        head = *context;
        if (head == NULL) {
                return NULL;
        }

        /* Skip any leading separators */
        while (*head && strchr(seps, *head)) {
                head++;
        }

        /* Did we hit the end? */
        if (*head == 0) {
                /* Nothing left */
                *context = NULL;
                return NULL;
        }

        /* skip over word */
        tail = head;
        while (*tail && !strchr(seps, *tail)) {
                tail++;
        }

        /* Save head for next time in context */
        if (*tail == 0) {
                *context = NULL;
        }
        else {
                *tail = 0;
                tail++;
                *context = tail;
        }

        /* Return current word */
        return head;
}

char *strtok_rtk(char *s, const char *delim)
{
	char *spanp;
	int c, sc;
	char *tok;
	static char *last;


	if (s == NULL && (s = last) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		last = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}
#endif

void error(const char *msg)
{
    //perror(msg);
    DEBUGMSG(1,"[Error]%s\n",msg);
    return ;//exit(0);
}


/*
  txPkt->len = strlen(buffer);
  txPkt = (PKT*)allocPkt(txPkt->len);
  
  memset(txPkt->wp, 0, txPkt->len);
  memcpy(txPkt->wp, buffer, txPkt->len);
*/


static void setWarringMsg(char *msg,int len){
	memset(m_mailWarringMsg,0x00,WARRING_MSG_LEN);
	if(WARRING_MSG_LEN>len){
		memcpy(m_mailWarringMsg,msg,len);
	}
}


int write_rtskt(RTSkt* s, char *buff, int len)
{

   PKT* txPkt;
   int ret;
   unsigned char err;

   txPkt = NULL;   
   txPkt = (PKT*)allocPkt(len);
   txPkt->len = len;
   memset(txPkt->wp,    0, txPkt->len);
   memcpy(txPkt->wp, buff, txPkt->len);     
   ret = rtSktSend_F(s, txPkt, txPkt->len);    //DO NOT free txPkt, let driver free it (for memory efficiency)  
   if ((ret == SKTDISCONNECT) )
   {
        DEBUGMSG(1,"[write_rtskt]ret == SKTDISCONNECT \n");
        rtSktClose_F(s); //reference client.c - clientTask1()
        err = 0;
        OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
        OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
        s->rxSem = 0;
        s->rxSem = 0;
        if (s)
        {
            free(s);
            s = NULL;
        }                
 
       ret = -1;
   }
   else
   {
       ret = txPkt->len;
   }   
 
   return ret;
}
                                              

int read_rtskt(RTSkt *s, char *buff, int len)
//int read_rtskt(RTSkt* s, PKT *rxPkt, int *status)       
{
    int status;
    PKT* rxPkt;    
    int ret;
    unsigned char err;
   
    ret = -1;
    do
    {
    
        rxPkt = rtSktRx_F(s, 0, &status);//0 -> timeout ???  
        if (rxPkt)
        {
            if (rxPkt->len <= BUF_LEN)
            {
                memcpy(buff, rxPkt->wp, rxPkt->len);
    
                ret = rxPkt->len;
                if (rxPkt)
                {
                   freePkt_F(rxPkt);
				   rxPkt = NULL;
                }
                break;            
            }
            else //rxPkt->len too large
            {
                ret = -1;               
                printf("[Error]read_rtskt,read too long(%d)\n",rxPkt->len);
                break;
            }
        }
        else
        {
            if ((status == SKTDISCONNECT))
            {
                DEBUGMSG(1,"[read_rtskt]status == SKTDISCONNECT \n");
                rtSktClose_F(s); //reference client.c - clientTask1()
                err = 0;
                OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
                OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
                s->rxSem = 0;
                s->rxSem = 0;
                if (s)
                {
                    free(s);
                    s = NULL;
                }                
 
            }
            ret = -1; //error
            break;
        }
  
    } while ((rxPkt == 0));
	
    if (rxPkt)
    {
    	freePkt_F(rxPkt);//+briank.rtk
    }
    
    return ret;
}



//NOT case-sensity
//field_name MUST NOT contain ":", only the name.
//For example, "From" is right, but "From:" is wrong.
char * email_hdr_field_extract(char *input_str, char *field_name)
{
    int i,match;
//DEBUGMSG(1,"[%s]--[%s]\n",field_name, input_str);    
    if (strlen(input_str)<strlen(field_name))
    {
       return NULL;
    }
    
    match = 1;
    for (i=0; i<strlen(field_name); i++)
    {
//DEBUGMSG(1,"input_str[%d]='%c' vs field_name[%d]='%c'\n",i,toupper(input_str[i]),i,toupper(field_name[i]));    
        if (toupper(input_str[i]) != toupper(field_name[i]))
        {
            match = 0;
            break;
        }
    }    


    if (match)
    {
       //i+2 to skip ':' and '' '(space)
       return &input_str[i+2];   //the_token[i][i+1] must be ': '
    }
    else
    {
        return NULL; 
    }
}

//NOT case-sensity
//input_str must be the header part of email 
//field_name MUST NOT contain ":", only the name.
//For example, "From" is right, but "From:" is wrong.
//return  1: match, 0:not match
int email_hdr_field_match(char *buffer, char *field_name, char *keyword)
{
    char *buffer_cpy;
    char *the_token;
    char *field_str;
    char *match_str;
    int n;
    char *here_ptr;
    
    //header processing
    n = strlen(buffer);
    buffer_cpy = (char *) malloc(n);//strtok will affect buffer, so we clone it
    memcpy(buffer_cpy, buffer, n);
    //simple parsing for the value - from,subject,date    
    here_ptr =  NULL;      
    the_token = strtok_r_p(buffer_cpy,"\r\n",&here_ptr);
    field_str = NULL;
    match_str = NULL;               
    while (the_token != NULL)
    {
        //extract the field 
        field_str = email_hdr_field_extract(the_token,field_name);
        if (field_str != NULL)
        {
            //compare
            match_str = strstr(field_str,keyword);
            if (match_str != NULL) //match
            {
                DEBUGMSG(1,"(email_hdr_field_match)[%s][%s] match line = [%s][len=%d]\n",field_name, keyword ,field_str ,strlen(field_str));        
				if(buffer_cpy)
					 free(buffer_cpy);
                return 1;
            }
            else
            {                    
                DEBUGMSG(1,"(email_hdr_field_match)[%s][%s] not match line [%s]\n", field_name, keyword, the_token);        
            }           
        }
        
        the_token = strtok_r_p(NULL, "\r\n", &here_ptr); 
    }
	if(buffer_cpy)
    	free(buffer_cpy);
    
    return 0;
}

//
// 1 : is  [0-9][0-9][0-9] 
// 0 : is not [0-9][0-9][0-9]
int is_3_digit(char *str)
{
    int i;
    if (str == NULL)
       return 0;
       
    if ((strlen(str)>3) || (strlen(str)<1))
       return 0;
    
    for (i=0; i<strlen(str); i++)
    {
       if ((str[i] > '9') || (str[i]<'0'))
           return 0;
    }   

    return 1;        
}
//return 1 : valid, 0 : not valid
//valid : "."
//if valid , address = IP address (32-bit) 
//else  address = 0
int valid_IP_str(char *mail_server, INT32U *address)
{
    char *tmp_str;
    char  *token;
    int   is_num;
    int   num_count;
    int   tmp_num;
      
    *address = 0;  
    if (mail_server == NULL)
       return 0;
       
       
    tmp_str = (char *) malloc(strlen(mail_server));
    memcpy(tmp_str,mail_server,strlen(mail_server));
        
    //DEBUGMSG(1,"%s  ==================================\n",tmp_str);
    is_num = 1; //1: data is numberic, 0 : data is not numberic
    num_count = 0; //xxx.xxx.xxx.xxx ,  "xxx" must count to 4
        
    token = strtok_p(tmp_str,".");
    while (token && is_num && (num_count <4))
    {
        if (is_3_digit(token))
        {
            tmp_num = atoi(token);
            if ((tmp_num <=255) && (tmp_num>=0) )
            {
                *address = *address + (tmp_num << (num_count*8));
                num_count++;
                token = strtok_p(NULL,".");
            }
            else
            {
                is_num = 0;
                break;
            }            
        }
        else
        {
            is_num = 0;
            break;
        
        }
    } ;
    
    if (tmp_str)
    {
       free(tmp_str);
    }
    

    if ((is_num == 1) && (num_count == 4))
        return 1;  
    else
        return 0;
   
}

//output : socket fd (-1 for fail, else successful)
//int create_seesion(char *mail_server, int portno)
RTSkt* create_seesion(char *mail_server, int port_no ,INT32U *ipv4 )
{
    RTSkt* sockfd = NULL;//, portno, n;
    char ip[4];
    int counter_skt;
    unsigned char err;    
    char *buffer;//[BUF_LEN]; 
    int n;   
	INT32U address = 0;
//---

address = *ipv4;

#if 1 // DNS "pop.163.com" 

	if(address == 0){
		if (valid_IP_str(mail_server,&address) == 0)  // domain name
		{	
				printf("[RTK]Query [%s]\n",mail_server);
				rtGethostbyname(IPv4, mail_server,(INT8U *) &address);		   

				if (address == 0)//query fail
				{
					printf("[Error][create_seesion]:IP not found(%s)\n", dpconf->email_server);
					setWarringMsg("Unknown Server",14);
					return NULL;
				}
			
		} 
		else //IP address
		{
			   //IP address is at address by valid_IP_str 
		} 
		*ipv4 = address;
		//if the dpconf->email_server is already IP address, the "address" will still 0 after calling rtGethostbyname(..)	   
		
		
	}

	ip[0]= (address&0x000000ff)>>0;  //clear it, if reGethostbyname fail, address is still 0 
	ip[1]= (address&0x0000ff00)>>8;
	ip[2]= (address&0x00ff0000)>>16;
	ip[3]= (address&0xff000000)>>24;

	printf("[RTK] Create Session[%d.%d.%d.%d]\n",ip[0],ip[1],ip[2],ip[3]);

#endif		
//---
	

    sockfd = rtSkt_F(IPv4, IP_PROTO_TCP);    


    
    
    if (sockfd == NULL)
    {
        error("[create_seesion]:sockfd==NULL");        
		setWarringMsg("Common Error(02)",16);
        return NULL;                              
    }
	
    sockfd->pcb->localPort = 60000+(rand()%2000);//52000;//36000;
    tcpPCBRegister_F(sockfd->pcb); 

	
    counter_skt = 0;
	
	printf("[RTK] Prepare Connect to Server\n");

    while (rtSktConnect_F(sockfd, ip, port_no) == -1)
    {
        if (counter_skt <3) 
        {
           counter_skt++;
        }
        else
        {
          //time-out
          //DEBUGMSG(1,"[create_seesion]time-out ->%s [localPort=%d]\n","Cannot connect to the mail server",sockfd->pcb->localPort);
          printf("[create_seesion]timeout\n");		  
          break;                 
        }
    }
	
    if (counter_skt >= 3) //time-out
    {
        rtSktClose_F(sockfd); //reference client.c - clientTask1()
        err = 0;
        OSSemDel(sockfd->rxSem, OS_DEL_ALWAYS, &err);
        OSSemDel(sockfd->rxSem, OS_DEL_ALWAYS, &err);
        sockfd->rxSem = 0;
        sockfd->rxSem = 0;
        if (sockfd)
        {
            free(sockfd);
//            sockfd = NULL;
        }       
		setWarringMsg("Connection Timeout",18);
        return NULL;
    }
       
    //DEBUGMSG(1,"%s\n","mail server connected[POP3 session created]");
    //DEBUGMSG(1,"===========================================================\n");
       
//then , read the welcome message from mail server.
    buffer = (char *)malloc(BUF_LEN);
    bzero(buffer,BUF_LEN);
    n = read_rtskt(sockfd,buffer,BUF_LEN);

    if (n < 0)
    {
          error("ERROR reading from socket(1)\n");
          if (buffer)
          {
              free(buffer);
          }
          setWarringMsg("Server no response",18);
          return NULL;
    } 
    else
    {
          buffer[n]='\0';//correct the display string
          //check +OK or -ERR
          if ((buffer[0]=='+') && (buffer[1]=='O') && (buffer[2]=='K'))//"+OK"
          {
              //DEBUGMSG(1,"->%s\n",buffer);
          }
          else
          {          
              //DEBUGMSG(1,"->something wrong [%s]\n",buffer);
              printf("[create_seesion]:Got error(%s)\n", buffer);
              if (buffer)
              {
                  free(buffer);
              }
              setWarringMsg("Server error(Create Session)",16);
              return NULL;
          }
          
          //DEBUGMSG(1,"-> %s(len=%d)",buffer,n);
    }
    if (buffer)
    {
        free(buffer);
    }
//    DEBUGMSG(1,"[%s] finished \n",__func__); 


    return sockfd;
}

//output : socket fd (-1 for fail, else successful)
int close_session(RTSkt*  sockfd)
{
    return rtSktClose_F(sockfd);
}


int mail_login(RTSkt* sockfd, char *account, char *passwd)
{
    char *buffer;//[BUF_LEN];
    int n;

    buffer = (char *)malloc(BUF_LEN);
    bzero(buffer,BUF_LEN);        
    //user     
    sprintf(buffer,"user %s\r\n",account);
//DEBUGMSG(1,"%s [%s](len=%d)",__func__,buffer,strlen(buffer));    
    n = write_rtskt(sockfd, buffer, strlen(buffer));//withoud '\0'
    if (n < 0) 
    {
         error("ERROR writing to socket(1)\n");
         if (buffer)
         {
             free(buffer);
         }
		 setWarringMsg("Common Error(03)",16);
         return -1;     
    }
    else
    {
         buffer[n]='\0';//correct the display string
         //DEBUGMSG(1,"<- %s\n",buffer);
    }
         
    bzero(buffer,BUF_LEN);
    n = read_rtskt(sockfd,buffer,BUF_LEN);
    if (n < 0) 
    {
         error("ERROR reading from socket(2)\n");
         if (buffer)
         {
             free(buffer);
         }         
		 setWarringMsg("Login no repose(1)",15);
         return -1;
    }
    else  
    {
          buffer[n]='\0';//correct the display string
          //check +OK or -ERR
          if ((buffer[0]=='+') && (buffer[1]=='O') && (buffer[2]=='K'))//"+OK"
          {
              //DEBUGMSG(1,"->%s\n",buffer);
          }
          else
          {          
              //DEBUGMSG(1,"->[something wrong]%s\n",buffer);
              error("Login Fail(1)");
              if (buffer)
              {
                  free(buffer);
              }        
			  setWarringMsg("Login Fail(1)",13);
              return -1;
          }
                
//         DEBUGMSG(1,"-> %s(len=%d)",buffer,n);
    }
//    DEBUGMSG(1,"\n");   
          
    //pass     
    sprintf(buffer,"pass %s\r\n",passwd);
    n = write_rtskt(sockfd,buffer,strlen(buffer));//withoud '\0'
    if (n < 0) 
    {    
         error("ERROR writing to socket(2)\n");
         if (buffer)
         {
             free(buffer);
         }         
		 setWarringMsg("Common Error(04)",16);
         return -1;
    }
    else
    {
         buffer[n]='\0';//correct the display string    
         //DEBUGMSG(1,"<- %s",buffer);
    }    

    bzero(buffer,BUF_LEN);
    n = read_rtskt(sockfd,buffer,BUF_LEN);
    if (n < 0) 
    {
         error("ERROR reading from socket(3)\n");
         if (buffer)
         {
             free(buffer);
         }
		 setWarringMsg("Login Fail(2)",13);
         return -1;
    }
    else
    {
          buffer[n]='\0';//correct the display string
          //check +OK or -ERR
          if ((buffer[0]=='+') && (buffer[1]=='O') && (buffer[2]=='K'))//"+OK"
          {
              //DEBUGMSG(1,"->%s\n",buffer);
          }
          else
          {          
              //DEBUGMSG(1,"->[something wrong]%s\n",buffer);
              error("Login Fail(3)");
              if (buffer)
              {
                  free(buffer);
              }
			  setWarringMsg("Login Fail(3)",13);
              return -1;
          }
                 
//         DEBUGMSG(1,"-> %s(len=%d)",buffer,n);
    }         
//    DEBUGMSG(1,"\n");   
   
    if (buffer)
    {
        free(buffer);
    }
    return 0;
}

int mail_logout(RTSkt*  sockfd)
{
    char *buffer;//[BUF_LEN];
    
    int n;
        
    buffer = (char *)malloc(BUF_LEN);
    bzero(buffer,BUF_LEN);        
    
    //user     
    sprintf(buffer,"quit\r\n");
    n = write_rtskt(sockfd,buffer,strlen(buffer));//withoud '\0'
    if (n < 0) 
    {
         error("ERROR writing to socket(3)\n");
         if (buffer)
         {
             free(buffer);
         }
         return -1;     
    }
    else
    {
         buffer[n]='\0';//correct the display string
         //DEBUGMSG(1,"<- %s(len=%d)",buffer,n);
    }
         
    bzero(buffer,BUF_LEN);
    n = read_rtskt(sockfd,buffer,BUF_LEN);
    if (n < 0) 
    {
         error("ERROR reading from socket(4)\n");
         if (buffer)
         {
             free(buffer);
         }
         return -1;
    }
    else  
    {
        buffer[n]='\0';//correct the display string
        //check +OK or -ERR
        if ((buffer[0]=='+') && (buffer[1]=='O') && (buffer[2]=='K'))//"+OK"
        {
            //DEBUGMSG(1,"->%s\n",buffer);
        }
        else
        {          
            //DEBUGMSG(1,"->[something wrong]%s\n",buffer);
            if (buffer)
            {
                free(buffer);
            }
             
            return -1;
        }
                
//         DEBUGMSG(1,"-> %s(len=%d)",buffer,n);
    }
    //DEBUGMSG(1,"\n");     

    if (buffer)
    {
        free(buffer);
    }
    return 0;
}

//output : the number(amount) of email on the server 
int mail_num(RTSkt*  sockfd)
{
    char *buffer;//[BUF_LEN];
    int n;
    
    buffer = (char *)malloc(BUF_LEN);
    bzero(buffer,BUF_LEN);            
    
    //list     
    //to get how many number of email
    sprintf(buffer,"list\r\n");
    n = write_rtskt(sockfd, buffer, strlen(buffer));//withoud '\0'
    if (n < 0) 
    {
         error("ERROR writing to socket(4)\n");
         if (buffer)
         {
             free(buffer);
         }         
         return -1;    
    }
    else
    {
         buffer[n]='\0';//correct the display string
//         DEBUGMSG(1,"<- %s(len=%d)",buffer,n);
    }
     
    //return format
    //+OK N total_size\r\n
    //...
    //N size_N\r\n
    //.
    char *the_token;
    int  the_N_len;
    bzero(buffer,BUF_LEN);          
    do
    {
        n = read_rtskt(sockfd,buffer,BUF_LEN);
        if (n < 0) 
        {
             error("ERROR reading from socket(5)\n");
             if (buffer)
             {
                 free(buffer);
             }
             
             return -1;        
        }
       else     
       {
            buffer[n]='\0';//correct the display string
            //check +OK or -ERR
            if ((buffer[0]=='+') && (buffer[1]=='O') && (buffer[2]=='K'))//"+OK"
            {
//                DEBUGMSG(1,"->%s\n",buffer);
            }
            else
            {          
                //DEBUGMSG(1,"->[something wrong]%s\n",buffer);
                if (buffer)
                {
                    free(buffer);
                }                
                return -1;
            }
                   
//            DEBUGMSG(1,"-> %s\n(len=%d)\n",buffer,n);

            //list response format[1]
            //+OK xxxxxx\r\n
            //...
            //N xxx\r\n
            //.\r\n

			//---list response format[2]
			// +ok N size\r\n
			// 1 size_1\r\n
			// 2 size_2\r\n 
			// 3 size_3\r\n
			// .\r\n
            the_N_len = 0;
            
            the_token = strtok_p(buffer,"\r\n");//strtok will modify the content of buffer
            while (the_token != NULL)
            {
                the_N_len ++; 
                the_token = strtok_p (NULL, "\r\n"); 
            }
             the_N_len -= 2; //substract the first line - "+OK..." and  the last line - ".\r\n"
             if (buffer)
             {
                 free(buffer);
             }             
             return the_N_len;

            /*
            if (((buffer[0]=='+')&&(buffer[1]=='O')&&(buffer[2]=='K')) ||
                ((buffer[0]=='+')&&(buffer[1]=='o')&&(buffer[2]=='k')) )
            {
                 the_N = &buffer[4];  
                 the_N_len = 0;
                 //the_N string begin from buffer[4] to buffer[the_N_len]
                 while ((buffer[4+the_N_len] != ' '))
                 {
                    //DEBUGMSG(1,"buffer[%d]='%c'\n",the_N_len,buffer[4+the_N_len]);
                    the_N_len++;
                 }
                 buffer[4+the_N_len]='\0';//overwrite buffer to extract N as a NULL-end string
                 return atoi(the_N);
                 //DEBUGMSG(1,"string the_N=[%s],convert to integer=%d\n",the_N,N);
            }     
            */      
                   
       }    
    }while (!((buffer[n-3]=='.')&&(buffer[n-2]=='\r')&&(buffer[n-1]=='\n')));                      

//    DEBUGMSG(1,"\n");   
  
    if (buffer)
    {
        free(buffer);
    }
    return -1;
}


//input ts_base, ts
//output :
//    0 : ts == (the same) ts_base [only compare (year,month,date)]
//    1 : ts > (after) ts_base [only compare (year,month,date)]
//    2 : ts < (before) ts_base [only compare (year,month,date)]
//    <0 : error
int date_cmp(struct mailwakeup_ts *ts_base, struct mailwakeup_ts *ts)
{   
   if (ts->year > ts_base->year)
   {
       return 1;
   }
   else if (ts->year < ts_base->year)
        {
            return 2;
        }
        else //the same year
        {
             if (ts->month > ts_base->month)
             {
                 return 1;
             }
             else if (ts->month < ts_base->month)
                  {
                      return 2;  
                  }
                  else // the same month
                  {
                      if (ts->date > ts_base->date)
                      {
                            return 1;
                      }
                      else if (ts->date < ts_base->date) 
                           {
                               return 2;           
                           }
                           else
                           {
                               return 0;
                           }
                      
                  } 
        }
   return -1;//should not run to here     

}

//input ts_base, ts
//output :
//    0 : ts == (the same) ts_base [only compare (hour,minute,second)]
//    1 : ts > (after) ts_base [only compare (hour,minute,second)]
//    2 : ts < (before)ts_base [only compare (hour,minute,second)]
//    <0 : error
int time_cmp(struct mailwakeup_ts *ts_base, struct mailwakeup_ts *ts)
{   
   if (ts->hour > ts_base->hour)
   {
       return 1;
   }
   else if (ts->hour < ts_base->hour)
        {
            return 2;
        }
        else //the same hour
        {
             if (ts->minute > ts_base->minute)
             {
                 return 1;
             }
             else if (ts->minute < ts_base->minute)
                  {
                      return 2;  
                  }
                  else // the same minute
                  {
                      if (ts->second > ts_base->second)
                      {
                            return 1;
                      }
                      else if (ts->second < ts_base->second)
                           {
                               return 2;           
                           }
                           else
                           {
                               return 0;
                           }
                  } 
        }
        
   return -1;//should not run to here     

}


//RFC 5322
//no error check , must carefull
char *month_table[] =
{
   "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};
int mon_to_num(char *month_str)
{
  int i;
  
  for (i=0; i<12; i++)
  {
      if ((month_str[0] == month_table[i][0])&&
          (month_str[1] == month_table[i][1])&&
          (month_str[2] == month_table[i][2]))
      {
         return i+1;
         break; 
      }
  } 
  return -1;
}

//input ts_base, ts
//       Date MUST be the "Date" field extracted from email header
//output :
//    0 : successful   (convert to GMT)
//    1 : fail
int Date_to_ts(char *Date, struct mailwakeup_ts *ts)
{  
   char *the_date,*the_token; 
//   DEBUGMSG(1,"[%s] Date=%s\n",__func__,Date);
   
   //for exampe, Date MUST be "Mon, 8 oct 2012 17:54:04 +0800"  
   the_date = strtok_p(Date,",");//the_date = "Mon"
   the_date = strtok_p(NULL,",");//the_data = "8 oct 2012 17:54:04 +0800"
   
   the_token = strtok_p(the_date," ");//8
   ts->date = atoi(the_token);
   the_token = strtok_p(NULL," ");//oct
   ts->month = mon_to_num(the_token);
   the_token = strtok_p(NULL," ");//2012
   ts->year  = atoi(the_token);
   
   the_token = strtok_p(NULL," ");//17:54:04
   ts->hour = (the_token[0]-0x30)*10 + (the_token[1]-0x30); //"-0x30" ==> convert '0'..'9' to 0..9
   ts->minute = (the_token[3]-0x30)*10 + (the_token[4]-0x30); //"-0x30" ==> convert '0'..'9' to 0..9
   ts->second = (the_token[6]-0x30)*10 + (the_token[7]-0x30); //"-0x30" ==> convert '0'..'9' to 0..9
   
   the_token = strtok_p(NULL," ");//+0800
   
   //adjust time to GMT
   if (the_token[0]=='+')// must subtract
   {
       ts->hour -= (the_token[1]-0x30)*10 + (the_token[2]-0x30); //"-0x30" ==> convert '0'..'9' to 0..9
       /* //-briank.rtk 20130225 : this is always false.
	       	if (ts->hour < 0)
	       {
	           ts->hour = ts->hour + 24;//24H
	           ts->date = ts->date -1; 
	       	}*/
       
   }
   else // must add
   {
       ts->hour += (the_token[1]-0x30)*10 + (the_token[2]-0x30); //"-0x30" ==> convert '0'..'9' to 0..9
       if (ts->hour >=24)
       {
           ts->hour = ts->hour % 24;//24H
           ts->date = ts->date + 1; 
       }

   }
   
   //DEBUGMSG(1,"time stamp =(%d,%d,%d,  %d, %d, %d)\n",ts->year, ts->month, ts->date, ts->hour, ts->minute, ts->second);
           
   return 0;        
}


//struct mailwakeup_ts *ts_base;
//struct mailwakeup_ts ts_base_dpconfig={1970,01,01,00,00,00};//factory setting
//struct mailwakeup_ts ts_base_dpconfig={2013,01,01,11,30,30};
//struct mailwakeup_ts ts_base_dpconfig={2012,10,8,8,24,50}; //MUST be GMT , not +0800 time-zone
struct mailwakeup_ts *m_tsBase;// = &dpconf->ts_base;//&ts_base_dpconfig;
//struct mailwakeup_ts *ts;
//struct mailwakeup_ts *ts_last;//store the best last ts of this round 


//time stamp check if a new email
//output : the number of email on the server
//    1 : new mail (for firmware)
//    0 : old mail (for firmware)
int mail_ts_check(RTSkt* sockfd, int mail_no ,struct mailwakeup_ts *ts)
{
    char *buffer;//[BUF_LEN];
    int size;
    int check_if_ok,date_str_found;
    char *the_token = NULL, *date_str = NULL;
    int is_new,cmp_result;
 
//    DEBUGMSG(1,"%s start\n",__func__);  //--> write_rtskt blocking
    
    buffer = (char *) malloc(BUF_LEN);
    bzero(buffer,BUF_LEN);            
    
    //top  1 1 <-- strange syntax of Raibow mail server
    sprintf(buffer,"top %d 0\r\n", mail_no);//top n 0 --> return header only
//    sprintf(buffer,"retr %d \r\n",mail_no);//top n 0 --> return header only
    size = write_rtskt(sockfd, buffer, strlen(buffer));//withoud '\0'
    printf("[RTK](send)%s , return size(%d)\n",buffer,size);
    if (size < 0) 
    {
         error("ERROR writing to socket(5)\n");
         if (buffer)
         {
             free(buffer);
         }
         return -1;    
    }
    else
    {
         buffer[size]='\0';//correct the display string
//         DEBUGMSG(1,"<- %s(len=%d)",buffer,n);
    }
    
    bzero(buffer,BUF_LEN);    

    check_if_ok = 0;   //check server response "+OK xxx" or "-ERR xxx"  
    date_str_found = 0;//0 : not found, 1 : found 
    do
    {
    	size = read_rtskt(sockfd,buffer,BUF_LEN);
		//printf("[RTK]read rtskt , size=(%d)\n",size);
        if (size < 0) 
        {
             error("ERROR reading from socket(6)\n");
             if (buffer)
             {
                 free(buffer);
             }             
             return -1;
        }
       	else     
       	{
//            DEBUGMSG(1,"***[%s]\n",buffer);
            if (check_if_ok == 0)
            {
                if ((buffer[0]=='+') &&(buffer[1]=='O') &&(buffer[2]=='K'))
                {
                    check_if_ok = 1;                 
                }
                else//-ERR
                {
                    DEBUGMSG(1,"[RTK] Fail from top command(%d)[%x %x %x]\n",size,buffer[0],buffer[1],buffer[2]);
                    if (buffer)
                    {
                        free(buffer);
                    }                    
                    return -1;
                } 

            }
            if (date_str_found == 0) // not found
            {
                the_token = strtok_p(buffer,"\r\n");
                do
                {
                    date_str = email_hdr_field_extract(the_token,"Date"); 
                    if (date_str != NULL)
                    {
                       //DEBUGMSG(1,"date=[%s]\n",date_str);
                       date_str_found = 1;//found
                       //printf("[RTK]find date at packet\n");
                       break;
                    }
                    the_token = strtok_p(NULL,"\r\n");
                }while (the_token != NULL);
                
                if (date_str != NULL)
                {
//                   DEBUGMSG(1,"-> %s\n(len=%d)\n",date_str,n);
                }
                else
                {
//                   DEBUGMSG(1,"-> date_str is NULL\n");
                }            
            }
            
            //if the "Date" string found, we still need to read-out all the incoming socket data
       }    
    }while (!((buffer[size-3]=='.')&&(buffer[size-2]=='\r')&&(buffer[size-1]=='\n')));
	//printf("[RTK] - finish\n");
    
    if (date_str_found == 0)//"Date" not found in the header
    {
        if (buffer)
        {
           free(buffer);
        }                    
        return -1;        
    }
    
    //now date is in date_str
//    DEBUGMSG(1,"date=[%s]\n",date_str);
    //conver date_str into mailwakeup_ts format
//    ts = (struct mailwakeup_ts *) malloc(sizeof(struct mailwakeup_ts));

    Date_to_ts(date_str,ts);
    
    //compare if an new mail
    is_new = 0;

	//printf("[RTK]date= %d-%d-%d %d:%d:%d\n",m_tsBase->year,m_tsBase->month,m_tsBase->date,m_tsBase->hour,m_tsBase->minute,m_tsBase->second);
	//printf("[RTK]date= %d-%d-%d %d:%d:%d\n",ts->year,ts->month,ts->date,ts->hour,ts->minute,ts->second);
	
    cmp_result = date_cmp(m_tsBase, ts); //new mail  (0,1,2)=(==,>,<)
    if (cmp_result==1)
    {
        is_new = 1;
    }
    else if (cmp_result==2)
         {
             is_new = 0;
         }
         else //the same year/month/date
         {
             cmp_result = time_cmp(m_tsBase,ts);//new mail (0,1,2)=(==,>,<)
             if (cmp_result==1)
             {
                 is_new = 1;  
             }
             else //the same time is considered as "before" 
             {
                 is_new = 0;
             }        
         }
    
   //DEBUGMSG(1,"%s finished\n",__func__);
    //free(ts);
    if (buffer)
    {
        free(buffer);
    } 

	
//is_new = 1;	//briank.rtk this is just for debug


    return is_new;
}

/*
//output : the number of email on the server
int mail_header(RTSkt* sockfd, int mail_no)
{
    char *buffer;//[BUF_LEN];
    int n;
    int top_len;
    
    buffer = (char *)malloc(BUF_LEN);
    bzero(buffer,BUF_LEN);                
    
    //top
    //to get how many number of email
    sprintf(buffer,"top %d 0\r\n",mail_no);//top n 0 : return header only
    n = write_rtskt(sockfd,buffer,strlen(buffer));//withoud '\0'
    if (n < 0) 
    {
         error("ERROR writing to socket\n");
         if (buffer)
         {
             free(buffer);
         }             
         return -1;    
    }
    else
    {
         buffer[n]='\0';//correct the display string
//         DEBUGMSG(1,"<- %s(len=%d)",buffer,n);
    }
    
    bzero(buffer,BUF_LEN);    

    top_len = 0;      
    do
    {
        n = read_rtskt(sockfd,buffer,BUF_LEN);
        if (n < 0) 
        {
             error("ERROR reading from socket\n");
             if (buffer)
             {
                 free(buffer);
             }                 
             return -1;
        }
       else     
       {
            //DEBUGMSG(1,"-> %s\n(len=%d)\n",buffer,n);
            top_len += n;      
       }    
    }while (!((buffer[n-3]=='.')&&(buffer[n-2]=='\r')&&(buffer[n-1]=='\n')));                      

//    DEBUGMSG(1,"header length=%d\n",top_len);  
   
    if (buffer)
    {
        free(buffer);
    }    
    return -1;
}
*/
 
//case-sensitive
char *email_body_keyword_match(char *buffer, char *keyword)
{
    return strstr(buffer, keyword);
}


#if 0

int mail_match(RTSkt* sockfd, int mail_no, char *email, char *subject, char *keyword)
{
}

#else

//output : bit 0 == 1 email match, 
//         bit 1 == 1 subject match,
//         bit 2 == 1 keyword match
int mail_match(RTSkt* sockfd, int mail_no, char *email, char *subject, char *keyword)
{
    char *buffer=NULL;//[BUF_LEN];
    char *lastbuffer=NULL;//[BUF_LEN];    
    char *tmpbuffer=NULL;//[BUF_LEN];    
    unsigned char *b64buffer=NULL,*b64buffer2=NULL;
    int lastbuffer_len;
    int bufferLen;
    int match;   
    //time stamp
    //int mail_i;    
    
    buffer = (char *)malloc(BUF_LEN);	
    bzero(buffer,BUF_LEN);                
               
    //write retr mail_no
    sprintf(buffer,"retr %d\r\n",mail_no);
    bufferLen = write_rtskt(sockfd,buffer,strlen(buffer));//withoud '\0'
    if (bufferLen < 0) 
    {
         error("ERROR writing to socket(6)\n");
         if (buffer)
         {
             free(buffer);
         }		 
         return -1;        
    }
    else
    {
         //DEBUGMSG(1,"<- %s(len=%d)",buffer,n);    
    }
            
    // "+OK xxx" or "-ERR xxx" are mixed with the email header "Recie...." by TCP combination
    // so we must identify "+OK xxx"/"-ERR xxx" before processing the email header 
    //email header and body
    bzero(buffer,BUF_LEN);       
    match = 0;
    
    int response_okay_check;        
    int header_end;
    int body_got;
    int body_finish;
    char *mailbody = NULL , *contentStart=NULL,*contentEnd=NULL,*tmpPtr=NULL;
	unsigned char isContentCheck;
    //int body_len;
    //char *buffer_cpy;//,*the_token; 
    //char *subject_str, *from_str, *date_str;
    int header_len;
    char *token,*nextToken;
    char *here_ptr;
	unsigned char isGiveUp;
	char *boundary=NULL,*boundary_end=NULL;
	int boundary_len = 0;
	int boundary_find=0;
	char *quote1=NULL,*quote2=NULL;
 	ContentEncodingTypeEnum ctype;
	unsigned char isCutHalf = 0x00;//check if the packet is end with "\r\n" , if isCutHalf==1 , it means it is "not" with "\r\n"
		
    //buffer_cpy =  NULL;
	ctype = CENCODING_UNKNOWN;
    //body_len = BODY_LEN;
    //body = (char *) malloc(BODY_LEN);//the first 1024 byte
    //bzero(body,BODY_LEN); 
    header_end = 0;//false
    body_got= 0;//false 
    header_len = 0;
    response_okay_check = 0;//the first line must be "+OK ..." or "-ERR..."  
    body_finish = 0;// "\r\n." 
    here_ptr = NULL;
	isGiveUp = 0x00;	
	
	//boundary = malloc(64);
	//memset(boundary,0x00,64);
	isContentCheck = 0x00;
    do
    {
        bzero(buffer,BUF_LEN);          
        bufferLen = read_rtskt(sockfd, buffer, BUF_LEN);
		//printf("[RTK]Receive a Packet\n");				
		
		mailbody = 0x00;
        if (bufferLen < 0) 
        {
             error("ERROR reading from socket(7)\n");
             if (buffer)
             {
                 free(buffer);
             }
			 if(boundary){
			 	free(boundary);
			 }
			 if(boundary_end){
			 	free(boundary_end);
			 }
			  if(lastbuffer){
			 	free(lastbuffer);
			 }
             return -1;        
        }
        else 
        {          

//      DEBUGMSG(1,"buffer = [%s] [len = %d]\n\n", buffer, n);
//            token = strtok(buffer,"\r\n");

	


#if 1
			//+searching the mail body start address---
			if(response_okay_check == 1){	
				if(body_got == 0){					
					mailbody = strstr(buffer,"\r\n\r\n");
					
					if(mailbody!=0x00){
						//printf("[RTK]find mail body\n");
						memset(mailbody,0x00,4);//split Header and Body
						body_got = 0x1;
						mailbody+=4;
					}
				}else{
					//header was finish at last packet.			
					mailbody = buffer;//this packet is all mail-body
					//printf("[RTK]ALL Mail Body\n");
				}
			}
				
		
			//-parsing mail Header---
			
			if(mailbody != buffer){
				//printf("[RTK]Parsing Header\n");
				token = strtok_p(buffer,"\r\n");

				while (token)
		        {
		        	if(response_okay_check == 0){
						if(strstr(token,"+OK")!=NULL)
	                    {
	                        response_okay_check = 1;
							printf("[RTK]retr : +OK\n");
	                    }
		        	}
					else{
			        	if (token[0]=='.')//body ending character
			            {
			            	body_finish = 1;
			                //DEBUGMSG(1,"body ending [%s]\n",token);
			                
			            }else{							
							
				            	if(memcmp(token,"From:",5) == 0){
									
									if(strstr(token,email)!=NULL){
										printf("[RTK] Sender Match!\n");
										match = match | 0x01;
									}else{
										printf("[RTK] Sender Not Matching!\n");
										//isGiveUp = 1;
										//break;
									}
				            	}else if(memcmp(token,"Subject:",8) == 0){
				            		if(strstr(token,subject)!=NULL){
										printf("[RTK] Subject Match!\n");	
										match = match | 0x02;
				            		}else{
				            					            			
							
										if(strstr(token,"=?UTF-8?B?")){			
											int blen;										
					            			blen = strlen(token);
											b64buffer2 = malloc(blen);
											memset(b64buffer2,0x00,blen);
											//Subject: =?UTF-8?B?													
											sscanf(token+9,"=?UTF-8?B?%[^?]",b64buffer2);
											b64_decode(b64buffer2,b64buffer2,strlen(b64buffer2));
											printf("[RTK]B64=<%s>\n",b64buffer2);
											if(strstr(b64buffer2,subject)!=NULL){
												printf("[RTK] SubjectB64 Match!\n");	
												match = match | 0x02;
						            		}											
											free(b64buffer2);
											b64buffer2=0x00;
										}else{	
											printf("[RTK] Subject Not Matching!\n");
										}
										//isGiveUp = 1;
										//break;
									}
				            	}
								else{ 
									//printf("[RTK]%s\n",token);
									if((tmpPtr = strstr(token,"boundary=")) != NULL ){
									//Find Boundary Declaration	-------------------------------
										quote1 = strchr(tmpPtr,'"');////find the first double-quote										
										if(quote1 != 0){											
											quote2 = strrchr(quote1+1,'"');//find the last double-quote
											if(quote2 != 0){
												boundary_len = (quote2-quote1)-1;																						
												boundary = malloc(boundary_len+2+1);//add "--" at begin of boundary , and add 0x00 at end  
												boundary_end = malloc(boundary_len+2+2+1);//add "--" at begin of boundary ,"--" at end of boundary, and add 0x00 at end  
																					  //ex: "--this_is_boundary--"																					  
												memset(boundary,0x00,boundary_len+3);
												memset(boundary_end,0x00,boundary_len+5);
												memset(boundary,'-',boundary_len+2);
												memset(boundary_end,'-',boundary_len+4);
												sscanf(tmpPtr,"boundary=\"%[^\"]\"",boundary+2);
												memcpy(boundary_end,boundary,boundary_len+2);	
												printf("boundary(%d)=<%s>\n",boundary_len,boundary);
												printf("boundary_end(%d)=<%s>\n",boundary_len,boundary_end);
											}
										}else{
											boundary_len = strlen(token) - (tmpPtr+9-token);
											boundary = malloc(boundary_len+2+1);
											boundary_end = malloc(boundary_len+2+2+1);
											memset(boundary_end,0x00,boundary_len+5);
											memset(boundary_end,'-',boundary_len+4);
											memset(boundary,0x00,boundary_len+3);
											memset(boundary,'-',boundary_len+2);
											sscanf(tmpPtr,"boundary=%s",boundary+2);
											memcpy(boundary_end,boundary,boundary_len+2);	
											printf("boundary.(%d)=<%s>\n",boundary_len,boundary);
											printf("boundary_end.(%d)=<%s>\n",boundary_len,boundary_end);
										}
									}	
			            		}
			        
			            }
						token = strtok_p(NULL,"\r\n");	
					}
				}
			}


			//-- start paring mail body------------------------------------------------------------------------
			if(mailbody!=NULL){
				//printf("[RTK]Parsing Mail Body\n");
				
				if(isCutHalf == 0x01){//must parsing last line at  last packet					
					if(lastbuffer!=NULL){
						//must use tmpBuffer , 避免兩個連續的packet 都被切
						//printf("[RTK]Create a new tmpBuffer(%d+%d)\n",lastbuffer_len,bufferLen);
						tmpbuffer = malloc(lastbuffer_len+bufferLen+1);						
						if(tmpbuffer != NULL){
							tmpbuffer[lastbuffer_len+bufferLen]=0x00;
							memcpy(tmpbuffer               ,lastbuffer ,lastbuffer_len);
							memcpy(tmpbuffer+lastbuffer_len,buffer     ,bufferLen);
							mailbody = tmpbuffer;						
							free(lastbuffer);
							lastbuffer = 0x00;
							lastbuffer_len=0;
						}else{
							printf("[RTK][XXXXX]memory allocate fail 01(%d)\n",lastbuffer_len+bufferLen);							
						}
					}
				}
				if(!(buffer[bufferLen-2]=='\r' && buffer[bufferLen-1]=='\n')){
					//printf("[RTK]Packet was cuted\n");
					isCutHalf = 0x01;
				}else{
					//printf("[RTK]Packet is OK\n");
					isCutHalf = 0x00;
				}
				//---
				//!!!Body must has boundary tag------	
				//
				contentStart = 0x00;
				if(!isContentCheck){
					//Only check the first boundary block	
					token = NULL;
					if(!boundary_find){//boundary already found at previous packet
						token = strstr(mailbody,boundary);//find the boundary first
						if(!token){
							if(memcmp(mailbody,boundary,boundary_len+2)){
								token = mailbody;
								printf("[RTK]Got Boundary\n");
							}
						}
					}
					if(token){
						contentStart = strstr(token,"\r\n\r\n");//find the boundary first
						boundary_find=1;
					}else{
						contentStart = strstr(mailbody,"\r\n\r\n");//find the boundary first
					}					
					if(contentStart!=NULL){
						contentStart+=4;
						isContentCheck = 1;
						contentEnd = strstr(contentStart,boundary);
						
						if(contentEnd == NULL){
							contentEnd = strstr(contentStart,"\r\n\r\n");
						}
						if(contentEnd != NULL){
							b64buffer = malloc(contentEnd-contentStart+1);
							memcpy(b64buffer,contentStart,(contentEnd-contentStart));
							b64buffer[contentEnd-contentStart] = 0x00;

							printf("[RTK]B64Code(%d)=[%s]\n",contentEnd-contentStart,b64buffer);
						}
					}
				}			
				token = strtok_p(mailbody,"\r\n");
				nextToken = strtok_p(NULL,"\r\n");
				while(token != 0x00){
					
					//if ((token[0]=='.')&&(token[1]=='\r')&&(token[2]=='\n'))//body ending character
					if ((token[0]=='.')&&(token[2]=='\n'))//body ending character
				    {
				    	body_finish = 1;
				        //printf("[RTK]find-body-finish \n");       
				    }else if((contentStart)&&(token >= contentStart )){
				    	//Description : Only check the first line of the content , and discard all others
				    	contentStart = 0x00;
				    	//+-parsing content--------------------------------------
				    	if(ctype == CENCODING_BASE64){

							#if 0
							b64buffer = malloc(strlen(token)+1);
							memset(b64buffer,0x00,strlen(token)+1);
							#endif
							if(b64buffer){
								b64_decode(b64buffer,b64buffer,strlen(b64buffer));
								printf("[RTK]B64 decode:<%s>\n",b64buffer);

								//if(memcmp(b64buffer,keyword,strlen(keyword)) == 0){
								//	printf("[RTK] Keyword Match!\n");
								//	match = match | 0x04;
								//}
								if(strstr(b64buffer,keyword)!=0){
									printf("[RTK] Keyword Match!\n");
									match = match | 0x04;
								}																
								else{
									printf("[RTK] Keyword Not Match\n");
								}
								free(b64buffer);
								b64buffer = 0x00;
							}else{
								printf("[XXXX]decode b64 fail(%d)\n",strlen(token));											
							}				
							
						}else{
							if(b64buffer){
								free(b64buffer);
								b64buffer = 0x00;	
							}
			          		printf("Content:<%s>\n",token);
								if(memcmp(token,keyword,strlen(keyword)) == 0){
									printf("[RTK] Keyword Match!\n");
									match = match | 0x04;
								}else{
									printf("[RTK] Keyword Not Match\n");
							}
						}
						//--parsing content--------------------------------------
				    }
					else{				    	
						//find boundary and MIME parameter
						if(memcmp(token,boundary_end,boundary_len+4) == 0){//this is the last boundary
							printf("[RTK]find endB<%s>\n",token);								
						}else if(memcmp(token,boundary,boundary_len+2) == 0){
							printf("[RTK]find B<%s>\n",token);	
							boundary_find=1;
						}else{											    								
							if(memcmp(token,"Content-Transfer-Encoding:",26) == 0){
							//printf("[RTK]Encoding Type = %s\n",token);
								if(strstr(token,"7bit")){
									ctype = CENCODING_7BIT;
								}else if(strstr(token,"8bit")){
									ctype = CENCODING_8BIT;
								}else if(strstr(token,"quoted-printable")){
									ctype = CENCODING_QUOTED_PRINTABLE;
								}else if(strstr(token,"binary")){
									ctype = CENCODING_BINARY;
								}else if(strstr(token,"base64")){
									ctype = CENCODING_BASE64;									
								}
								printf("[RTK]ctype=0x%x\n",ctype);
			            	}else if(memcmp(token,"Content-Type:",13) == 0){
			            		//do nothing
			            	}
						}	
				       	
				    }

					//This will check all mail-body,back-up mechanism
					if(strstr(token,keyword)){
						printf("[RTK] Keyword Match!(backup)\n");
						match = match | 0x04;
					}
					//-------------------------------

						
					
					token = nextToken;
					nextToken = strtok_p(NULL,"\r\n");	
					if(nextToken == NULL){
							//Got the Last Token
						if(isCutHalf == 0x01){
							//Save the last Line			
							lastbuffer_len = strlen(token);
							//printf("[RTK]save the last line(%d)\n",lastbuffer_len);
							lastbuffer = malloc(lastbuffer_len+1);
							if(lastbuffer!=NULL){
								memset(lastbuffer,0x00,lastbuffer_len+1);
								memcpy(lastbuffer,token,lastbuffer_len);
							}
							break;
						}
						
					}
					
				
				}				
				
				if(tmpbuffer!=0x00){
					//printf("[RTK]free tmpbuffer[0x%x]\n",tmpbuffer);
					free(tmpbuffer);
					tmpbuffer = 0x00;
				}
			}	



			
			//-----parsing this packet done
			/*
			if(mailbody!=0x00){
				if(mailbodyTail!=0x00){
					mailbodyTail[0] = 0x00;
					printf("[RTK](MAIL_BODY)%s\n",mailbody);
					mailbodyTail[0] = '\r';						
						
				}else{
					printf("[RTK]mail body is not finish yet\n");
				}	
			}*/

			
		}    

    }while (body_finish == 0);	
		
#else

            token = strtok_r(buffer,"\n",&here_ptr);
            while (token)
            {              
                //DEBUGMSG(1,"response_okay_check=%d\n",response_okay_check);
                //DEBUGMSG(1,"header_finish=%d\n",header_finish);
                //DEBUGMSG(1,"body_finish=%d\n\n",body_finish);
            
                //1. check if +OK
                //check if response is "+OK"            
                if (response_okay_check == 0)
                {
                    if ((token[0]=='+') && (token[1]=='O') && (token[2]=='K'))//"+OK"
                    {
                        response_okay_check = 1;
                        //DEBUGMSG(1,"response okay\n");
                    }
                    else
                    {          
                        DEBUGMSG(1,"->[something wrong]%s\n",buffer);
                        if (buffer)
                        {
                            free(buffer);
                        }                                     
                        return -1;
                    }     
                               
                }
                //2. header processing and 3. body prcossing    
                //header MUST like this format -> xxx: xxxx\r\n , include ": "
                else   
                {   
                     //DEBUGMSG(1,"[%s](token[0]=%x)\n",token,token[0]);                
                 
                    
                     if (header_finish == 0)
                     {
                         if (token[0] != '\r') //still header
                         {
                             //DEBUGMSG(1,"header [%s](token[0]=%x)\n",token,token[0]);  
                             
                              if (email_hdr_field_match(token, "SUBJECT", subject) == 1)
                              {
                                 match = match | 0x02;//bit 1 == 1 subject match
                                 //DEBUGMSG(1,"%s include %s \n","SUBJECT", subject);
                              }
                              else
                              {
                                 //DEBUGMSG(1,"%s NOT include %s \n","SUBJECT", subject);                
                              }
                                 
              
                              if (email_hdr_field_match(token, "FROM", email) == 1)
                              {
                                 match = match | 0x01;//bit 0 == 1 email match
                                 //DEBUGMSG(1,"%s include %s \n","FROM", email);
                              }
                              else
                              {
                                 //DEBUGMSG(1,"%s NOT include %s \n","FROM", email);                
                              }                                                                  
                         
                         }
                         else // the first line of body
                         {
                              if (token[0]=='.')//body ending character
                              {
                                  body_finish = 1;
                                  //DEBUGMSG(1,"body ending [%s]\n",token);
                              }
                              header_finish = 1;
                              //DEBUGMSG(1,"body_1st [%s]\n",token);                
                         
                         }
                     }
                     else
                     {
                          //DEBUGMSG(1,"body [%s]\n",token);                
                          if (token[0]=='.')//body ending character
                          {
                              body_finish = 1;
                              //DEBUGMSG(1,"body ending [%s]\n",token);
                          }
                          else
                          {
                              //find the keyword in the content

							if(match == 0x03){
								printf("[RTK]keyword=<%s>\n",keyword);
								memcpy(keyword,"1234",4);
							}
							  
                              if (email_body_keyword_match(token, keyword) != NULL)//match
                              {
                                  match = match | 0x04;//bit 2 == 1 keyword match                              
                                  //DEBUGMSG(1,"[%s] matched in the content.\n",keyword);
    
    //                              body[BODY_LEN]='\0';// for correcting display
                                  //DEBUGMSG(1,"***[%s] matched in the content.\n",token);
                                  //we cannot return due to must flush out READ data in TCP queue
                              }
                              else
                              {
                                 //DEBUGMSG(1,"[%s] ***NOT matched in the content.\n",keyword);                                
                              }
                           
                          
                          }
                     
                     }
                     
                }                              
               
//               token = strtok(NULL,"\r\n");
               token = strtok_r(NULL,"\n",&here_ptr);
            }
		 }    

    }while(body_finish == 0);	
#endif
            
       

    /*
    if (body != NULL)
        DEBUGMSG(1,"->*****1024 of body******[%s](len=%d)\n",body,strlen(body));    
    */
    //free(body);

	#if 0
    DEBUGMSG(1,"-------------------------------------------\n"); 
    DEBUGMSG(1,"email #%d\n",mail_no); 
    DEBUGMSG(1,"email_match = %d\n",match & 0x01);
    DEBUGMSG(1,"subject_match = %d\n",(match & 0x02)>>1);
    DEBUGMSG(1,"keyword_match = %d\n",(match & 0x04)>>2);
    DEBUGMSG(1,"-------------------------------------------\n\n");
	#endif
     
    if (buffer)
    {
        free(buffer);
    } 
	if(boundary){
		free(boundary);
	}
	if(boundary_end){
		free(boundary_end);
	}
	if(lastbuffer){
		free(lastbuffer);
	}
	if(tmpbuffer){
		free(tmpbuffer);					
	}
    return match;
}


#endif

void closeConnection(RTSkt* skt){
	INT8U err = 0;
	
	rtSktClose_F(skt);
    OSSemDel(skt->rxSem, OS_DEL_ALWAYS, &err);
    OSSemDel(skt->txSem, OS_DEL_ALWAYS, &err);
    skt->rxSem=0;
    skt->txSem=0;
    if (skt)
    	free(skt);
    //skt=NULL;
}



//now we implement three cases 7 (A & S)
//                       case  12 (A | S | C)
//                       case  13 (A & S & C)  
// condition_code  1 : match address
//                       2 : match subject
//                       4 : match content
//This function is used to desic 
int isMatchCondition(int match, int condition_code)
{                         
    int active_match;
       
    if (match <= 0)
    {
       DEBUGMSG(1,"Not Matching (0x%x)\n", match);
       return -1;
    } else{
    	printf("[RTK]match is 0x%x\n",match);
    }
       
    active_match = 0;                            
    
	if( (match & condition_code) == condition_code){
		active_match = 1;
	}
	
    
    return active_match;

}

unsigned char isInS0(){
	if(bsp_get_sstate() == S_S0){
		return 0x01;
	}else{
		return 0x00;
	}
}
void lanWakeup(){
	printf("[RTK]======= msnWakeup =======\n");
	
	if(!isInS0()){
		
		

		#if CONFIG_VERSION <= IC_VERSION_DP_RevF	
			bsp_bits_set(IO_CONFIG, 1, BIT_OOB_LANWAKE, 1);
                	bsp_bits_set(IO_CONFIG, 0, BIT_OOB_LANWAKE, 1);
		#else
			bsp_bits_set(MAC_LAN_WAKE, 1, BIT_OOB_LANWAKE, 1);	
		#endif
    		//bsp_bits_set(MAC_LAN_WAKE, 0, BIT_OOB_LANWAKE, 1);
		
	}
}

//Connect to MailServer , and paring mail content. 
int main_mail_check(char *email_server, int port_no, char *email_accout, char *email_passwd,
                    char *match_address, char *match_subject, char *match_keyword, int match_code , INT32U *ipv4addr)
{         
      RTSkt* sockfd;// = rtSkt_F(IPv4, IP_PROTO_TCP);
      int status;
      int mail_quantity;
      //int condition_code;
      int idx;
      int match;
      int retMailTSCheck;

	struct mailwakeup_ts *ts;
	struct mailwakeup_ts *ts_last;//store the best last ts of this round 

        
      sockfd = NULL; //MUST init. as NULL or system will halt    
//printFreeMem(m_rounds++);
      //printf("[RTK]email_server=(%s) , port_no=%d , ipv4addr=0x%x\n",email_server,port_no,ipv4addr);
      sockfd = create_seesion(email_server, port_no,ipv4addr);//(sockfd==NULL , error ) ,Create a TCP session , connecting to mail-server  

      if (sockfd == NULL)
      {
      	   printf("[RTK] socket creation was fail.\n");
          DEBUGMSG(1,"socket creation was fail.\n")
		  //setWarringMsg("Common Error(01)",16);
          return -1;
      }
//printFreeMem(2);
      status = mail_login(sockfd, email_accout, email_passwd);  
      if (status < 0)
      {
          printf("login fail. (email_acount=[%s], email_passwr=[%s])\n", email_accout, email_passwd);
          //free(sockfd);
          closeConnection(sockfd);
		  setWarringMsg("Login Server fail",17);
          return -1;
      }
//printFreeMem(3);

#if 1 //Description : [list] : Query the quantity of mail.
      mail_quantity = mail_num(sockfd);
      if (mail_quantity < 0)
      {
          error("POP3 command : List fail \n");
          //free(sockfd);
          closeConnection(sockfd);
		  setWarringMsg("POP3 command : List fail",24);
          return -1;
      }else{
      		printf("[RTK] the amount of mails = [%d]\n",mail_quantity);
      }
#endif  
	  memset(m_mailWarringMsg, 0x00 , WARRING_MSG_LEN );//clearWarringMsg
	  
	  //printf("[RTK]total mail = %d\n",mail_quantity);      
	  //mail_header(sockfd,8);  
      //DEBUGMSG(1,"\n=============[login successful]==================\n\n");     
      //for mail_ts_check(...)
	  
#if 1 //Parsing all mail .
      m_tsBase = (struct mailwakeup_ts *) malloc(sizeof(struct mailwakeup_ts));
      memcpy(m_tsBase,&dpconf->ts_base,sizeof(struct mailwakeup_ts));   //load the previous last_ts value

      ts = (struct mailwakeup_ts *) malloc(sizeof(struct mailwakeup_ts));
      ts_last = (struct mailwakeup_ts *) malloc(sizeof(struct mailwakeup_ts));
      memcpy(ts_last,&dpconf->ts_base,sizeof(struct mailwakeup_ts));   //load the previous last_ts value

			
      for (idx=1; idx<=mail_quantity; idx++)
      {   
	//printf("[RTK]========mail#%d\n",idx);	
	    retMailTSCheck = mail_ts_check(sockfd,idx,ts);
          if (retMailTSCheck == 1)//new mail
          {  
              //check mail
              //check if mail match
              match = mail_match(sockfd, idx, match_address, match_subject, match_keyword);
    
                
              //now we implement three cases 7 (A & S)
              //                       case  12 (A | S | C)
              //                       case  13 (A & S & C)  
              if  (isMatchCondition(match,match_code) == 1)
              {
                  // turn on PC
                 	lanWakeup();                       
				  	//printFreeMem(99);
              }
               // DEBUGMSG(1,"\n");       

              //find the best last ts of this round
              //compare if an new mail
              int cmp_result;
              cmp_result = date_cmp(ts_last, ts); //new mail  (0,1,2)=(==,>,<)     
              
              if (cmp_result==1)
              {
                   // newer
                   memcpy(ts_last,ts,sizeof(struct mailwakeup_ts));
                  
              }
              else if (cmp_result==0)//the same year/month/date
              {
                   cmp_result = time_cmp(ts_last,ts);//new mail (0,1,2)=(==,>,<)
                   if (cmp_result==1)
                   {
                       // newer
                       memcpy(ts_last,ts,sizeof(struct mailwakeup_ts));                           
                   }
              }               
          }
          else if(retMailTSCheck == 0)
          {
             //DEBUGMSG(1,"mail #%d is OLD.\n\n",idx);  
             printf("[RTK] mail #%d is OLD.\n",idx);
          }else if(retMailTSCheck == -1 ){
          	 printf("[Error] Socket Fail.\n");
	       break;
          }else{
          	 printf("[Error] Unknow return form mail_ts_check(%d).\n",retMailTSCheck);
		 break;
          }

      }
        
      //update ts_base as this new email's ts
//      memcpy(ts_base,ts_last,sizeof(struct mailwakeup_ts));
      memcpy(&dpconf->ts_base,ts_last,sizeof(struct mailwakeup_ts));
      setdirty(DPCONFIGTBL);
      
                 
      free(ts_last);
      free(ts);
	  if(m_tsBase!=NULL){	  	
      	free(m_tsBase);
		m_tsBase = 0x00;
	  }
#endif	  
   
      //[quit]: logout mail server    
      mail_logout(sockfd);
//printFreeMem(8);	  
      //close_session(sockfd);
      closeConnection(sockfd);
	  
//printFreeMem(9);
      return 0;
}




/*====================================================================*/


/*
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
*/
//--------------------------Exported variables------------------------------




/*
Please use 
 
                bsp_bits_set(IO_CONFIG, 1, BIT_OOB_LANWAKE, 1);
                bsp_bits_set(IO_CONFIG, 0, BIT_OOB_LANWAKE, 1);
 
to wakeup your platform.
 
Be sure to check the system state first,
 
    if (bsp_get_sstate() == S_S0)
        //return failed, since it is in S0 state instead of S5
 


*/
/*
void MailWakeUp_power_on(void *data)//power on control (done)
{
    while (OS_TRUE)
    {       
        OSTimeDlyHMSM(0, 0, 5, 0); // Wait 5 second
        
        if (bsp_get_sstate() == S_S0)//PC is on, this module do nothing
        {
           //do nothing
            DEBUGMSG(1,"%s (bsp_get_sstate()=%d)\n","PC is ON ",bsp_get_sstate());            
        }
        else //
        {           
            DEBUGMSG(1,"%s (bsp_get_sstate()=%d)\n","PC is OFF",bsp_get_sstate());  
                      
            //if the PC is cold-booting (the first time power on)
            //this code NOT works,you must turn on PC once manually
            //power on the PC
            bsp_bits_set(IO_CONFIG, 1, BIT_OOB_LANWAKE, 1);
            bsp_bits_set(IO_CONFIG, 0, BIT_OOB_LANWAKE, 1);                             
            
            DEBUGMSG(1,"finiish to turn it on... (bsp_get_sstate()=%d)\n",bsp_get_sstate());              
        }
    }

}
*/

//----------------------------------------------------------------------------
// init. user's profile --> after test, must move these to the FLASH
//----------------------------------------------------------------------------

//0:disable, 1:enable, other:illegal, default: 0  
int MAIL_CHECK = 0;//1;//1;//0;

//{IP, Domain Name}//POP3(110) and POP3S(995)
//char EMAIL_SERVER[MWU_MAX_STR_LEN] = {"192.168.0.18"}; 
char EMAIL_SERVER[MWU_MAX_STR_LEN] = {"pop.163.com"}; 

//0:disable, 1:enable, other:illegal, default : 1 {disable (port=110), enable (port=995)}
int  MWU_SSL = 0;

//customer's email account
//char EMAIL_ACCOUNT[MWU_MAX_STR_LEN] = {"firmware"}; 
//char EMAIL_ACCOUNT[MWU_MAX_STR_LEN] = {"runec"}; 
char EMAIL_ACCOUNT[MWU_MAX_STR_LEN] = {"RealtekNIC@163.com"}; 


//customer's password
//char EMAIL_PASSWD[MWU_MAX_STR_LEN]= {"123"};
char EMAIL_PASSWD[MWU_MAX_STR_LEN]= {"NICPM2012"}; 

//customer's desired matching email address
char MATCH_ADDRESS[MWU_MAX_STR_LEN] = {"<runec@kiss99.com>"};//email address must contain "<" and ">" 
//char MATCH_ADDRESS[MWU_MAX_STR_LEN] = {"<firmware@192.168.0.18>"};//email address must contain "<" and ">" 
//char MATCH_ADDRESS[MWU_MAX_STR_LEN] = {"<eccheng@realtek.com.tw>"};//email address must contain "<" and ">" 

//customer's desired matching email subject
//char MATCH_SUBJECT[MWU_MAX_STR_LEN] = {"2.71828"};
char MATCH_SUBJECT[MWU_MAX_STR_LEN] = {"power on"};

//customer's desired matching email body's keyword
char MATCH_KEYWORD[MWU_MAX_STR_LEN] = {"2.71828"};

// Activate_condition {condition code = 1..13 } 
// 7 : MATCH_ADDRESS and MATCH SUBJECT match
// 12 : MATCH_ADDRESS or MATCH SUBJECT or MATCH_KEYWORD match 
// 13 : MATCH_ADDRESS and MATCH SUBJECT and MATCH_KEYWORD match 
int  MATCH_CODE;

//check mail every PERIOD second {default:5 second}
int  m_check_period_secs = 15;//3; 
int  m_check_period_mins = 0;//3; 



unsigned char isConnectionOK(){
	if(m_mailWarringMsg[0] == 0x00){
		return 0x01;		
	}else{
		return 0x00;
	}
}

static void clearWarringMsg(){
	memset(m_mailWarringMsg,0x00,WARRING_MSG_LEN);
}


unsigned char *getWarringMsg(){
	return m_mailWarringMsg;
}
void clearMailServeripv4Addr(){
	//memcpy(EMAIL_SERVER,hotname,len);
	clearWarringMsg();
	m_mailIpv4addr = 0x00;
	//printf("[RTK] set server name to <<<%s>>>",EMAIL_SERVER);
}


int load_profile()
{

//printf("[RTK] + load_profile \n");

	if(dpconf->smtp_SSL== 0xFF){ //the value in the flash is illegal
		printf("[Error] Mailwakeup parameter broken : Set to Default Value\n");
	
		    dpconf->mail_wake_up = 0;//1, //on/off {on : != 0, off : = 0}
		    memset(dpconf->email_server , 0x00 , MWU_MAX_STR);
		    memcpy(dpconf->email_server , "pop.163.com" , strlen("pop.163.com"));
		    dpconf->smtp_SSL = 0; //enable {disable (port=110), enable (port=995)}		   
		    memset(dpconf->email_acount , 0x00 , MWU_MAX_STR);
		    memcpy(dpconf->email_acount , "lobster172" , strlen("lobster172"));

		    memset(dpconf->email_passwd , 0x00 , MWU_MAX_STR);
		    memcpy(dpconf->email_passwd , "realtek172" , strlen("realtek172"));

		    memset(dpconf->match_address , 0x00 , MWU_MAX_STR);
		    memcpy(dpconf->match_address , "lobster193@163.com" , strlen("llobster193@163.com"));

		    memset(dpconf->match_subject , 0x00 , MWU_MAX_STR);
		    memcpy(dpconf->match_subject , "test" , strlen("test"));

		    memset(dpconf->match_keyword , 0x00 , MWU_MAX_STR);
		    memcpy(dpconf->match_keyword , "20130307" , strlen("20130307"));
			
		    
		    dpconf->Activate_condition = 7; //{condition code = 1..13 }, now only support 7,12,13
		    dpconf->mailPeriod= 15;// (second) {default:5 second}   

		   dpconf->ts_base.year = 1970;
		   dpconf->ts_base.month = 01;
		   dpconf->ts_base.date = 01;
		   dpconf->ts_base.hour = 00;
		   dpconf->ts_base.minute = 0;
		   dpconf->ts_base.second = 0;
			
	}



    MAIL_CHECK = dpconf->mail_wake_up;

    memset(EMAIL_SERVER, 0, MWU_MAX_STR_LEN); 
    memcpy(EMAIL_SERVER, dpconf->email_server, strlen(dpconf->email_server)); 

    MWU_SSL = dpconf->smtp_SSL; 

    memset(EMAIL_ACCOUNT, 0, MWU_MAX_STR_LEN); 
    memcpy(EMAIL_ACCOUNT, dpconf->email_acount, strlen(dpconf->email_acount)); 

    memset(EMAIL_PASSWD, 0, MWU_MAX_STR_LEN); 
    memcpy(EMAIL_PASSWD, dpconf->email_passwd, strlen(dpconf->email_passwd)); 

    memset(MATCH_ADDRESS, 0, MWU_MAX_STR_LEN); 
    memcpy(MATCH_ADDRESS, dpconf->match_address, strlen(dpconf->match_address)); 

    memset(MATCH_SUBJECT, 0, MWU_MAX_STR_LEN); 
    memcpy(MATCH_SUBJECT, dpconf->match_subject, strlen(dpconf->match_subject)); 

    memset(MATCH_KEYWORD, 0, MWU_MAX_STR_LEN); 
    memcpy(MATCH_KEYWORD, dpconf->match_keyword, strlen(dpconf->match_keyword)); 

    MATCH_CODE = dpconf->Activate_condition; 

    m_check_period_secs = dpconf->mailPeriod;

//printf("[RTK] dpconf->SSL =0x%x\n",dpconf->SSL);
//printf("[RTK] MAIL_CHECK = 0x%x , m_check_period_secs= 0x%x\n",MAIL_CHECK,m_check_period_secs);

	

   	
    return 0;
}

int show_profile()
{
    DEBUGMSG(1,"\n");
    
    DEBUGMSG(1,"MAIL_CHECK = %d\n", MAIL_CHECK );
    DEBUGMSG(1,"EMAIL_SERVER = %s\n", EMAIL_SERVER); 
    DEBUGMSG(1,"MWU_SSL =%d\n", MWU_SSL); 
    DEBUGMSG(1,"EMAIL_ACCOUNT = %s\n", EMAIL_ACCOUNT); 
    DEBUGMSG(1,"EMAIL_PASSWD = %s\n", EMAIL_PASSWD); 
    DEBUGMSG(1,"MATCH_ADDRESS = %s\n", MATCH_ADDRESS); 
    DEBUGMSG(1,"MATCH_SUBJECT = %s\n", MATCH_SUBJECT); 
    DEBUGMSG(1,"MATCH_KEYWORD = %s\n", MATCH_KEYWORD); 
    DEBUGMSG(1,"MATCH_CODE = %d\n", MATCH_CODE); 
    DEBUGMSG(1,"PERIOD = %d\n", m_check_period_secs);

    DEBUGMSG(1,"\n");
   
    return 0;
}

extern flash_data_t dirty[ENDTBL];	

void MailWakeUpTask(void *data)
{
    //int period;
    int port_no;
    long num_mail_check;//statistics   
    //INT32U ipv4addr = 0;
	//------------------------------------------------------------

    #if 0
	TLS_CONNECT_RESULT result;
	char *buffer = NULL;	
	RTSkt *skt = rtSkt_F( IPv4, IP_PROTO_TCP );
	unsigned int ipv4addr = 0;
	int len = 0;
	PKT* rxPkt = NULL;	
	#endif
//------------------------------------------------------------

OSTimeDly(OS_TICKS_PER_SEC * 5);

	//
	//printf("[RTK] + MailWakeUpTask , dirty[DPCONFIGTBL].length=%d , (2)sizeof(DPCONF)=%d\n", dirty[DPCONFIGTBL].length , sizeof(DPCONF));
	//printf("[RTK] +  , MWU_SSL = %d\n ",MWU_SSL);
	//dirty[DPCONFIGTBL].length = sizeof(DPCONF);
	//setdirty(DPCONFIGTBL);
	
    //period = PERIOD;
    num_mail_check = 1;//statistics
    m_rounds = 0;//Count the rounds of logging mail server .
    while (OS_TRUE)
    {
    //printf("[RTK] + MailWakeUpTask , dirty[DPCONFIGTBL].length=%d , (2)sizeof(DPCONF)=%d\n", dirty[DPCONFIGTBL].length , sizeof(DPCONF));
    //printf("[RTK] +  , MWU_SSL = %d \n ",MWU_SSL);
	//if(dirty[DPCONFIGTBL].length != sizeof(DPCONF)){
    	//	dirty[DPCONFIGTBL].length = sizeof(DPCONF);
	//}
	printFreeMem(1);
//------------------------------------------------------------
#if 0 //this is for SSL connection

ipv4addr = 0x2100a8c0; //192.168.0.33

//ipv4addr = 0xAD4715ac, // 172.21.71.173
//ipv4addr = 0x994715ac, // 172.21.71.153
//ipv4addr = 0x6c857d4a, // 74.125.133.108

printf("start connect to HTTPS (0x%x), delay 5s\n",0x2100a8c0);
OSTimeDly(5*OS_TICKS_PER_SEC);

initTLS(ipv4addr,443);//65.54.186.107 : //443 //995
result = connectToTlsServer(skt);

if(result == TLS_RESULT_SUCCESS){
		printf("[RTK]Connect TLS Success\n");		
		
	}else{
		printf("[RTK]Connect TLS Fail\n");
		closeConnectToTlsServer(skt);
		skt= NULL;
		return TLS_CONNECT_FAIL;
	}




buffer = malloc(256);
	if(buffer == NULL){
		printf("[RTK] Buffer alloc fail..\n");
		return;
	}
	memset(buffer,0x0,256);
	len = sprintf(buffer,"GET /test.html HTTP/1.1\r\nHost: 192.168.0.10\r\n\r\n");
	//len = sprintf(buffer,"user logoutk@gmail.com\r\n");
	//"GET / HTTP/1.1\r\nHost: 192.168.0.22\r\n\r\n"	
	sendTLSBuffer(buffer,len,skt->pcb);
	//printf("[RTK]readTLSBuffer\n");
	do{		
		//printf("[RTK] + readTLSBuffer\n");
		
		rxPkt = readTLSBuffer(skt->pcb); 
		//printf("[RTK]Finish readTLSBuffer\n");
		//printf("[result](%d)%s\n",rxPkt->len,rxPkt->wp+5);	
		//rtk_printf(rxPkt->wp);
		
		//printf("[RTK] - readTLSBuffer\n");
	}while(rxPkt != NULL);
	//for(idx=0;idx<rxPkt->len;idx++){
	//	printf("%c",rxPkt->wp[idx]);
	//}
free(buffer);

OSTimeDly(1000*OS_TICKS_PER_SEC);
#endif
//------------------------------------------------------------


        //dpconfig verification        
        
        //show_profile();//for debugging

	//printf("[RTK] Before delay , (%d) , (%d)\n",m_check_period_secs,MWU_SSL);

	if(m_check_period_secs > 60){
		m_check_period_mins = m_check_period_secs/60;		
		if(m_check_period_mins >=60){
			m_check_period_mins = 59;
		}
	}

        OSTimeDlyHMSM(0, m_check_period_mins, m_check_period_secs%59, 0);  //check every "period" second         	
	//printf("[RTK] After delay\n");
		load_profile();
	//printf("[RTK] After load_profile MWU_SSL=%d\n",MWU_SSL);
		//memset(m_mailWarringMsg, 0x00 , WARRING_MSG_LEN );//clearWarringMsg

		  
        //check PC on or off
        if (bsp_get_sstate() == S_S0)//PC is on, this module do nothing
        {
           //do nothing
            //DEBUGMSG(1,"%s (bsp_get_sstate()=%d)\n","PC is ON ",bsp_get_sstate());
            printf("[RTK] PC is on, this module do nothing\n");	
		   	OSTimeDly(15 * OS_TICKS_PER_SEC);
            continue;            

		
        }else{
        	printf("[RTK] PC is not S0(%d) \n",m_check_period_secs);	
        }
        

        if (MAIL_CHECK == 1)//enable
        {

	//printf("[RTK] MAIL_CHECK == 0x01\n");
		
       /*
            if (MWU_running == 1)//already running, do nothing <== no re-entry call
            {
                //do nothing, just let the previous one finished
                DEBUGMSG(1,"%s (bsp_get_sstate()=%d)\n","do nothing, just let the previous one finished",bsp_get_sstate());
            }
            else
            */    
            {
                //init. random seed
                srand(REG32(TIMER_IOBASE + TIMER_CV));//[optional for setting tcp localport number]
                
                //check mail every "Period" second  
                
                if (MWU_SSL==0)//port 110
                   port_no = 110;
                else if (MWU_SSL==1) //port 995
                   port_no = 995;
                else
                {
                	port_no = -1;//error
                    DEBUGMSG(1,"#%d MailWakeUp port_num=%d not legal\n", port_no);
                }    
                     
                //MWU_running = 1; //rise the flag    
                //printf("[RTK]EMAIL_SERVER=[%s]\n",EMAIL_SERVER); 
                main_mail_check(EMAIL_SERVER, port_no, EMAIL_ACCOUNT, EMAIL_PASSWD,  //-briank.rtk just for debug
                                MATCH_ADDRESS, MATCH_SUBJECT, MATCH_KEYWORD, MATCH_CODE,&m_mailIpv4addr);   //return -1 : fail                                        

                //MWU_running = 0;//clear the flag
                //DEBUGMSG(1,"#%d MailWakeUp checking done\n\n", num_mail_check++);                   
            }
        }
        else //disable
        {
            //do nothing
            //DEBUGMSG(1,"MailWakeUp checking DISABLE\n\n");   
            
            MWU_running = 0;    
        } 

    }
    
}

/*
//testing
//rtGethostbyname()
//Example
//{'p','t','t','.','c','c'} => {0x03,'p','t','t',0x2,'c','c', 0};
//return value check : unknown
//INT8U hostname[]={3,'p','o','p',3,'1','6','3',3,'c','o','m',0};//pop.163.com
INT8U hostname[50];//={"pop.163.com"};
INT8U *ptr = hostname;
INT32U address;
   sprintf(hostname,"pop.163.com");
   rtGethostbyname(IPv4,ptr,(INT8U *) &address);
   DEBUGMSG(1,"%s [%s] IP = %d.%d.%d.%d\n",
              __func__,
              ptr,
              (address&0x000000ff)>>0,
              (address&0x0000ff00)>>8,
              (address&0x00ff0000)>>16,
              (address&0xff000000)>>24);              
   
   sprintf(hostname,"pop.gmail.com");
   rtGethostbyname(IPv4,ptr,(INT8U *) &address);
   DEBUGMSG(1,"%s [%s] IP = %d.%d.%d.%d\n",
              __func__,
              ptr,
              (address&0x000000ff)>>0,
              (address&0x0000ff00)>>8,
              (address&0x00ff0000)>>16,
              (address&0xff000000)>>24);              

   sprintf(hostname,"pop.mail.yahoo.com.tw");
   rtGethostbyname(IPv4,ptr,(INT8U *) &address);
   DEBUGMSG(1,"%s [%s] IP = %d.%d.%d.%d\n",
              __func__,
              ptr,
              (address&0x000000ff)>>0,
              (address&0x0000ff00)>>8,
              (address&0x00ff0000)>>16,
              (address&0xff000000)>>24);              

   address = 0; //clear it, if reGethostbyname fail, address is still 0
   sprintf(hostname,"pop3.pchome.com.tw");
//   sprintf(hostname,"pop3.phhhhhchome.com.tw");
   rtGethostbyname(IPv4,ptr,(INT8U *) &address);
   DEBUGMSG(1,"%s [%s] IP = %d.%d.%d.%d\n",
              __func__,
              ptr,
              (address&0x000000ff)>>0,
              (address&0x0000ff00)>>8,
              (address&0x00ff0000)>>16,
              (address&0xff000000)>>24);              

  


*/


