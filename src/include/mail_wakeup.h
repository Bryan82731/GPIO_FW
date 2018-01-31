/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	mail_wakeup.h

Abstract:	Mail Wake-up module

*****************************************************************************
*/
#ifndef _MWU_INCLUDED
#define _MWU_INCLUDED
/*====================================================================*/
//due to TCP's Max packet size = 1460, BUF_LEN = 1460
#define BUF_LEN  1460 //RFC 5322 --> each line max size 998
#define HDR_LEN  4096 
#define BODY_LEN 1024 //128 //1024 //check only the first 1024 byte of the email's content



//char match_email_addr[256]={"runec@192.168.0.68"};//from email
//char match_email_title[256]={"power on"}; 
//char match_email_content[256]={"power on"};//support MIME - the first 1024 bytes
//char mail_server[256]="192.168.0.68";//support POP3
//int  server_port;
//char mail_account[256]="firmware@192.168.0.68";
//char mail_account[256]="runec@192.168.0.68";
//char mail_pass[256]="qw12ER34";
//lite POP3
//char POP3_ok[]={"+OK"};
//char POP3_err[]={"-ERR"};                                            

#define RX_TIMEOUT  0 //3 //3 second

struct mailwakeup_ts 
{
   unsigned short year;
   unsigned char  month;
   unsigned char  date;
   unsigned char  hour;
   unsigned char  minute;
   unsigned char  second;
};


#define MWU_MAX_STR 32
#define MWU_MAX_STR_LEN  MWU_MAX_STR
//----------------------------------------------------------------------------
// init. user's profile --> after test, must move these to the FLASH
//----------------------------------------------------------------------------
//MEU(MailWakeUp)
//#define MWU_MAX_STR_LEN  128
void clearMailServeripv4Addr();
#endif
