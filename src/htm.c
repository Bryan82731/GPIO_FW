/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	htm.c

Abstract:	Generate web pages or relative files

*****************************************************************************
*/

/*--------------------------Include File------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "htm.h"
#include "lib.h"
#include "tcp.h"
#include "genpage.h"
#include "smbus.h"
#ifdef CONFIG_MSAD_ENABLED
#include "gssapi.h"
#endif
#include "eventlog.h"
#include "bsp.h"
#include "b64.h"
#include "language.h"

extern void chkBufSize(void *p, int offset);
extern smbiosrmcpdata *smbiosrmcpdataptr;
extern DPCONF *dpconf;

extern unsigned char *getComputerSysProperty(char *eltname); //+briank.rtk 20130523  define in dash.c
extern unsigned char *getPowerMangSrvProperty(char *eltname);
extern unsigned char *getPowerMangCapProperty(char *eltname,int *count);
extern unsigned char *getOperationSysProperty(char *eltname);
extern void UpdateComputerSystemCIM();
//extern void UpdateInbandCIM();
extern void UpdateSoftwareIdCIM();
extern void UpdateBIOSCIM();
extern void releaseSoftwareIdCIM();
extern void releaseBIOSCIM();
//extern unsigned char *getSoftwareIdProperty(char *eltname,int swidx);
extern int getSwIdInstanceCount();
extern unsigned char *getBIOSElemProperty(char *eltname,int eleidx);
extern unsigned char *getBIOSAttrProperty(char *eltname,int attridx);
extern int getbiosAttrInstanceCount();
extern int getbiosEleInstanceCount();
/*--------------------------Structures and define----------------------------*/

/*-------------------------Html source code---------------------------------*/

const char okPage[] ={"HTTP/1.1 200 \r\nContent-type: text/html\r\n\r\n 200 OK"};
#ifdef TESTAPREP
unsigned char adokcon[]= //Server: have to appear to pass AP_REP
{
"HTTP/1.1 200 OK\r\n\
Server: Apache/2.0.64 (Unix) mod_auth_kerb/5.4\r\n\
WWW-Authenticate: Negotiate oYGhMIGeoAMKAQChCwYJKoZIgvcSAQICooGJBIGGYIGDBgkqhkiG9xIBAgICAG90MHKgAwIBBaEDAgEPomYwZKADAgEXol0EW+GvsTj+9OAcj34EjxUyJcFbG3uPaBM5tdGhU6f1sMS0C5S2qmUzpofoWIJvpevJBPQtTsdJa9/ezw52j81xIt4v/RxS4qbsrfzqkYEk5obsKBrz43qQib0650A=\r\n\
Last-Modified: Wed, 29 Dec 2010 18:43:53 GMT\r\n\
ETag: \"67dc7-7-f3646c40\"\r\n\
Accept-Ranges: bytes\r\n\
Content-Length: 7\r\n\
Keep-Alive: timeout=15, max=99\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/html\r\n\r\n\
bbbbb\n"
};
#endif
const unsigned char headerHtm[] =
{
    "HTTP/1.1 200 \r\nContent-type: text/html\r\nContent-Length: %5d\r\n\r\n"
};
#ifdef CONFIG_MSAD_ENABLED
const  char headerHtm2[] =
{
    "HTTP/1.1 200 OK\r\nServer: Realtek Management Technology\r\nContent-Length: %5d\r\n"
};
const char negoHeader[] =
{
    "WWW-Authenticate: Negotiate "
};
const char headerHtm3[] =
{
    "HTTP/1.1 200 \r\nContent-Length: %5d\r\n\r\n"
};
#endif
const unsigned char cookieHdrHtm[] =
{
    "HTTP/1.1 200 \r\nContent-type: text/html\r\nSet-Cookie: wsmanid=%d\r\nContent-Length: %5d\r\n\r\n"
};

#if CONFIG_ACER_WEB_UI
const unsigned char rdcss[] =
{
"H2{COLOR:#5d5d5d;FONT-FAMILY:Calibri,Arial;font-size:24px;line-height:1em;word-spacing:0.1em;text-indent:0.5em;}\n\
H3{COLOR:#617D5E}\n\
H4{COLOR:#617D5E}\n\
HR{COLOR:#dddddd;FONT-FAMILY:Calibri,Arial}\n\
B{COLOR:blue}\n\
A{float: left;TEXT-DECORATION: none}\n\
A:link{COLOR:#cc9900}\n\
A:visited{COLOR:gray}\n\
A:active{COLOR: #cc9900}\n\
A:hover{FONT-WEIGHT: bold; CURSOR: pointer; COLOR: #3366ff}\n\
body{background:url(bg-triangle.gif) no-repeat top left;margin:20}\n\
table{color:#5d5d5d}\n\
td{FONT-FAMILY:Calibri,Arial;background-color:#d2ddd7;padding:2 1 3 0;}\n\
tdimg{width:100%;height:100%}\n\
th{FONT-WEIGHT: normal;FONT-FAMILY: Calibri, Arial;TEXT-ALIGN: left;background-color: #eaebe9;padding:1.8 0;}\n\
P{COLOR: #097B68;FONT-FAMILY:Calibri, Arial}\n\
.t{color:#869557;padding: 2 10 3 0}\n\
.list-label{width:11;height:100%}\n\
.list-label{width:11;height:100%;background:#d2ddd7}\n\
td input{color:#9d9d9d;font-family:Arial;font-size:13;backbround:#F3F6F5;}\n\
.set-table{background-color: #D2DDD7;padding:6 50 6 40;margin:-15 22;*width:330}\n\
.rt{direction:rtl}  \n\
.submit-btn{width:100;font-family:Calibri, Arial;font-size:13}\n\
#table-tab{background:#D1DCD6; color:#145C00; font-weight:600; font-size:18px; text-indent:10;  vertical-align:middle; padding: 3px 16px;}\n\
.w-border{background:#fff;}\n\
.w-border td {background-color:#E0E8E4;line-height:1.8; padding: 18px 0 16px 30px;}"
};

const static unsigned char bannerHtm[] =
{
"<STYLE type=text/css>	BODY{background-image: url(\"banner-bg.gif\"); background-repeat: repeat-y;}</STYLE>\n\
<BODY><TABLE> <TBODY><tr height=15><td></td></tr><TR><TD width=36></TD><TD><IMG src=\"logo.gif\" width=130></TD></TR></TBODY></TABLE></BODY>"
};

const static unsigned char homeHtm[] =
{
"<html>\n\
<head>\n\
<title>Realtek Dash Client Tool</title>\n\
</head>\n\
<FRAMESET rows=\"87,*\"border=\"0\">\n\
<frame name=\"banner\" scrolling=\"no\" noresize target=\"contents\" src=\"banner.htm\">\n\
<frameset cols=\"200,*\" border=\"0\">\n\
<frame name=\"contents\" scrolling=\"no\" target=\"main\" src=\"info.htm\">\n\
<frame name=\"main\" src=\"welcome.htm\" scrolling=\"auto\">\n\
</frameset>\n\
<noframes>\n\
<body>\n\
</body>\n\
</noframes>\n\
</frameset>\n\
</html>"
};

const static unsigned char welcomeHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n\
<head>\n\
<title>Welcome to use Realtek Gigabit</title>\n\
<STYLE type=text/css>body{background:url(welcome-bg-triangle.gif) no-repeat top left; margin:20\n\
</STYLE>\n\
</head>\n\
<body>\n"
"<H2>Welcome To Use Realtek\n\
<H2>Gigabit Ethernet Controller\n\
<H2>With Dash Technology.\n"
"</body>\n\
</html>"
};
//remove <td vAlign=top></td><td><a target=\"main\" href=\"syslog.htm\">System Event Log</a></td><tr>\n\ from infoHtm
const static unsigned char infoHtm[] =
{"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<STYLE type=text/css>\n\
body {margin: 0px;background-image: url(\"info-bg.gif\"); background-repeat: repeat-y;}\n\
a {font-weight: bold; font-size: 11pt; color: white; font-family: calibri, arial; text-decoration: none}\n\
big {font-weight: bold; font-size: 13pt; margin: 0pt; color: #233f36; font-family: calibri, arial}\n\
p {font-weight: normal; font-size: 11pt; color: white; font-family: calibri, arial}\n\
table {font-weight: normal; font-size: 11pt; color: white; font-family: calibri, arial}\n\
td {line-height:1.2; text-indent:15; font-weight: normal; font-size: 11pt; color: white; font-family:calibri,arial}\n\
.topic {line-height:1.5; color: #233f36; font-weight:bold; font-size:13pt; font-family:calibri,arial; text-decoration:none; margin: 0pt;}\n\
.topic a {line-height:1.5; color: #233f36; font-weight:bold; font-size: 13pt;display: block; text-decoration:none; margin:0pt;}\n\
.topic a:hover{background: url(\"info-a-bg.gif\") repeat-y; color:#fff;}\n\
td a {display:block;font-family:calibri,arial}\n\
td a:hover{background: url(\"info-a-bg.gif\") repeat-y; color:#fff}\n\
</STYLE>\n\
<BODY leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">\n\
<TABLE cellSpacing=0 cellPadding=0 border=0 width=195>\n\
<TBODY>\n\
<tr height=20><td vAlign=top width=8></td></tr>\n\
<tr><td colSpan=2></td></tr>\n"
#ifndef CONFIG_WAKE_UP_PATTERN_ENABLED
"<tr><td colSpan=2 class=topic>System information</td></tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"bios.htm\">BIOS</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"sysinfo.htm\">System Information</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"board.htm\">Based Board Information</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"chassis.htm\">Chassis</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"cpu.htm\">CPU</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"cache.htm\">Cache</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"port.htm\">Port Connection</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"slot.htm\">Slot</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"phymem.htm\">Physical Memory</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"memdev.htm\">Memory Device</a></td>\n\
<tr><td colspan=2 height=8></td></tr>\n\
<td colSpan=2 class=topic><a target=\"main\" href=\"remoteCtl.htm\">Remote Control</td><tr>\n\
<td colSpan=2 class=topic><a target=\"main\" href=\"eventLog.htm\">Event Log</td><tr>\n"
#else
"<td colSpan=2><a target=\"main\" href=\"eventLog.htm\"><BIG>Patterns setting</BIG></td><tr>\n"
#endif
"<td colSpan=2 class=topic><a target=\"main\" href=\"netSet.htm\">Network setting</td><tr>\n\
<td colSpan=2 class=topic><a target=\"main\" href=\"srvSet.htm\">Service setting</td><tr>\n"
#if CONFIG_EZSHAE_TASK_ENABLED
    "<td colSpan=2><a target=\"main\" href=\"http:\\\\%s:6230\\ezShare.htm\"><BIG>ezShare</BIG></td><tr>\n"
#elif CONFIG_EZSHAE_ENABLED
    "<td colSpan=2><a target=\"main\" href=\"ezShare.htm\"><BIG>ezShare</BIG></td><tr>\n"
#endif
"<td colSpan=2 class=topic><a target=\"main\" href=\"about.htm\">About</td><tr>\n\
<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};

const static unsigned char remoteCtlHtmTop[] =
{
"<HTML><HEAD><LINK href=\"rdcss.css\" type=text/css rel=stylesheet></HEAD>\n\
<BODY><H2>Power managment</H2>\n\
<table CellSpacing=3 class=\"w-border\">\n\
<tr><FORM action=/remoteCtl method=post>\n\
<td vAlign=top width=200 id=\"table-tab\"><FONT>Power Control</FONT></td>\n\
</tr><tr><td>\n\
<INPUT type=radio value=reset name=power>Reset</br>\n\
<INPUT type=radio value=up name=power>Power up</br>\n\
<INPUT type=radio value=off name=power>Power off</br>\n\
<INPUT type=radio value=cycle name=power>Power Cycle Reset</td>"
};

const static unsigned char remoteCtlHtmBottom[] =
{
"</tr></table><p><INPUT type=submit value=Submit class=\"submit-btn\"></P></FORM></BODY></HTML>"
};

const static unsigned char eventLogTopHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n"
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
"<head><title>Wake Up Patterns Setting</title></head>\n\
<body>\n\
<H2>Wake Up Patterns</H2>\n\
<hr><table cellspacing=1 border = 1>"
#else
"<head><title>Event Log</title></head>\n\
<body>\n\
<H2>Event Log</H2>\n\
<table cellspacing=0 cellpadding=0><TBODY>"
#endif
};

const static unsigned char netSetIPv4Htm[] =
{
"<HTML><HEAD><TITLE>Network Setting</TITLE>\n\
<LINK href=\"rdcss.css\" type=text/css rel=stylesheet></HEAD>\n\
<BODY><FORM action=netSet method=post>\n\
<H2>IPv4 Setting</H2>\n\
<P style=\"font-weight:bold\"><INPUT type=radio %s value=v4 name=IPv4 %s>Set IPv4 static Address</P>\n\
<TABLE class=\"set-table\">\n\
<TBODY>\n\
<TR><TD class=\"rt\">IP Address</TD>\n\
<TD><INPUT maxLength=15 size=16 %s value=%s name=v4Addr></TD><td></td>\n\
<TR><TD class=\"rt\">Subnet Mask</TD>\n\
<TD><INPUT maxLength=15 size=16 %s value=%s name=v4Mask></TD><td></td><TR>\n\
<TD class=\"rt\">Gateway</TD>\n\
<TD><INPUT maxLength=15 size=16 %s value=%s name=v4Gateway></TD><td></td>\n\
<TR><TD class=\"rt\">DNS Server</TD>\n\
<TD><INPUT maxLength=15 size=16 %s value=%s name=v4DNS></TD><td></td></TR></TBODY></TABLE>\n\
%s<P><P style=\"padding-top:20; font-weight:bold\"><INPUT type=radio value=DHCPv4 name=IPv4 %s>Use DHCPv4%s</P>\n"
#if 0
"<P></P><P style=\"font-weight:bold\"><INPUT type=radio %s value=IPv4Disable name=IPv4 %s>Disable IPv4 Protocol</P>\n"
#endif
"<P><INPUT type=submit value=Submit class=\"submit-btn\"></P></FORM></BODY></HTML>"
};

const static unsigned char srvSetHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n\
<head>\n\
<title>Service Setting</title>\n\
</head>\n\
<body>\n\
<form method=\"POST\" action=\"srvSet\">\n\
<H2>Service Setting</h2>\n\
<table CellSpacing=3 width=230 class=\"w-border\">\n\
<tr><td id=\"table-tab\"><FONT>*Echo Service</FONT></td></tr>\n\
<td><INPUT type=radio %s value=on name=srv>Enable</br>\n\
<INPUT type=radio %s value=off name=srv>Disable</td></tr></table>\n\
<P><INPUT type=submit value=Submit class=\"submit-btn\"></P>\n\
</FORM></BODY></HTML>"
};

const static unsigned char notFound[] =
{
"<HTML><HEAD>\n\
<style type=text/css>\n\
#rectangle {width:300px; height:40px; background:#7BA650; COLOR: #fff;font-size:16pt; font-weight:bold; font-family:Calibri, Arial;\n\
text-align:center; line-height:40px; position:absolute; top:50px; left:30px;}\n\
</style></HEAD><BODY><div id=\"rectangle\">!I Not Found</div></BODY></HTML>"
};

const static unsigned char fake_notFound[] = 
{
"<HTML><HEAD>\n\
<style type=text/css>\n\
#rectangle {width:300px; height:40px; background:#000066; COLOR: #fff;font-size:16pt; font-weight:bold; font-family:Calibri, Arial;\n\
text-align:center; line-height:40px; position:absolute; top:50px; left:30px;}\n\
</style></HEAD><BODY><div style=\"display:none\">0x%x</div><div id=\"rectangle\">Not Found</div></BODY></HTML>"
};

const static unsigned char infoHtm_head[] =
{
"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<STYLE type=text/css>body {margin: 0px;background-image: url(\"info-bg.gif\"); background-repeat: repeat-y;}\n\
a {font-weight: bold; font-size: 11pt; color: white; font-family: calibri, arial; text-decoration: none}\n\
big {font-weight: bold; font-size: 13pt; margin: 0pt; color: #233f36; font-family: calibri, arial}\n\
p {font-weight: normal; font-size: 11pt; color: white; font-family: calibri, arial}\n\
table {font-weight: normal; font-size: 11pt; color: white; font-family: calibri, arial}\n\
td {line-height:1.2; text-indent:15; font-weight: normal; font-size: 11pt; color: white; font-family:calibri,arial}\n\
.topic {line-height:1.5; color: #233f36; font-weight:bold; font-size:13pt; font-family:calibri,arial; text-decoration:none; margin: 0pt;}\n\
.topic a {line-height:1.5; color: #233f36; font-weight:bold; font-size: 13pt;display: block; text-decoration:none; margin:0pt;}\n\
.topic a:hover{background: url(\"info-a-bg.gif\") repeat-y; color:#fff;}\n\
td a {display:block;font-family:calibri,arial}\n\
td a:hover{background: url(\"info-a-bg.gif\") repeat-y; color:#fff}\n\
</STYLE>\n\
<BODY leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">\n\
<TABLE cellSpacing=2 cellPadding=1 border=0>\n\
<TBODY>\n\
<tr><td vAlign=top></td></tr>\n\
<tr><td colSpan=2></td></tr>\n"
};

const static unsigned char infoHtm_body1[] =
{
"<tr><td colSpan=2><a target=\"main\" href=\"computersys.htm\"><BIG>%s</BIG></a></td></tr>\n\
<tr><td colSpan=2><a target=\"main\" href=\"operationsys.htm\"><BIG>%s</BIG></a></td></tr>\n\
<tr><td colSpan=2><a target=\"main\" href=\"softwareid.htm\"><BIG>%s</BIG></a></td></tr>\n"

#ifndef CONFIG_WAKE_UP_PATTERN_ENABLED
"<tr><td colSpan=2><BIG2>%s</BIG2></td></tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"bios.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"cpu.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"memdev.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"fan.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"sensor.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"asset.htm\">&nbsp;%s</a></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"remoteCtl.htm\"><BIG>%s</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"eventLog.htm\"><BIG>%s</BIG></td><tr>\n"
#else
"<td colSpan=2><a target=\"main\" href=\"eventLog.htm\"><BIG>%s</BIG></td><tr>\n"
#endif
"<td colSpan=2><a target=\"main\" href=\"netSet.htm\"><BIG>%s</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"srvSet.htm\"><BIG></BIG></td><tr>\n"
#if CONFIG_EZSHAE_TASK_ENABLED
    "<td colSpan=2><a target=\"main\" href=\"http:\\\\%s:6230\\ezShare.htm\"><BIG>ezShare</BIG></td><tr>\n"
#elif CONFIG_EZSHAE_ENABLED
    "<td colSpan=2><a target=\"main\" href=\"ezShare.htm\"><BIG>ezShare</BIG></td><tr>\n"
#endif
};

const static unsigned char infoHtm_body2[] =
{
#ifdef CONFIG_MAIL_WAKEUP_ENABLED
"\<tr><td colSpan=2><BIG2>%s</BIG2></td></tr>\
<td vAlign=top></td><td><a target=\"main\" href=\"mail_acc.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"mail_mat.htm\">&nbsp;%s</a></td><tr>\n"
#endif

"\<tr><td colSpan=2><a target=\"main\" href=\"about.htm\"><BIG>%s</BIG></td></tr><tr>\n\
<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};

#else   //CONFIG_ACER_WEB_UI

const unsigned char rdcss[] =
{
"<style type=\"text/css\">\n\
<!--\n\
h2 {color:green; font-family: Calibri, Arial;}\n\
h3 {color:#2222AA; font-size:16pt font-family: Calibri, Arial;}\n\
h4 {color:#2288aa; font-size:14pt font-family: Calibri, Arial;}\n\
hr {color:#550000; font-family: Calibri, Arial;}\n\
B {color:blue;}\n\
A{FLOAT: LEFT;text-decoration:none;font-size:10pt font-family: Calibri, Arial;}\n\
A:link{color : #cc9900;}\n\
A:visited{color : #cc3399}\n\
A:active{color : #cc9900}\n\
A:hover{cursor:pointer;color : \"#3366ff\" ;font-weight:bold}\n\
th{FONT-FAMILY:Calibri, Arial;text-align:left;font-weight=normal}\n\
td{FONT-FAMILY:Calibri, Arial;background-color:#FF9900}\n\
-->\n\
</style>"
};

const static unsigned char bannerHtm[] =
{
"<table><td><img src=\"logo.gif\" width=\"48\" height=\"48\"></td><td width=5></td><td>\n\
<H2><font color=blue font face=\"Monotype Corsiva, Arial\" font size=6>%s</td></table>"
};

const static unsigned char homeHtm[] =
{
"<html>\n\
<head>\n\
<title>Realtek Dash Client Tool</title>\n\
</head>\n\
<frameset rows=\"87,*\">\n\
<frame name=\"banner\" scrolling=\"no\" noresize target=\"contents\" src=\"banner.htm\">\n\
<frameset cols=\"190,*\">\n\
<frame name=\"contents\" target=\"main\" src=\"info.htm\">\n\
<frame name=\"main\" src=\"welcome.htm\" scrolling=\"auto\">\n\
</frameset>\n\
<noframes>\n\
<body>\n\
</body>\n\
</noframes>\n\
</frameset>\n\
</html>"
};


const static unsigned char fake_notFound[] = 
{
"<HTML><HEAD>\n\
<style type=text/css>\n\
#rectangle {width:300px; height:40px; background:#000066; COLOR: #fff;font-size:16pt; font-weight:bold; font-family:Calibri, Arial;\n\
text-align:center; line-height:40px; position:absolute; top:50px; left:30px;}\n\
</style></HEAD><BODY><div style=\"display:none\">0x%x</div><div id=\"rectangle\">Not Found</div></BODY></HTML>"
};


const static unsigned char welcomeHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n\
<head>\n\
<title>Welcome to use Realtek Gigabit</title>\n\
</head>\n\
<body>\n"
#ifndef CONFIG_WAKE_UP_PATTERN_ENABLED
"<H2>%s\n\
<H2>%s\n\
<H2>%s\n"
#else
"<H2>Welcome To Use Realtek\n\
<H2>Gigabit Ethernet Controller\n\
<H2>With Offload Engine.\n"
#endif
"</body>\n\
</html>"
};
//remove <td vAlign=top></td><td><a target=\"main\" href=\"syslog.htm\">System Event Log</a></td><tr>\n\ from infoHtm.
//remove
/*
<td vAlign=top></td><td><a target=\"main\" href=\"sysinfo.htm\">&nbsp;System Information</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"board.htm\">&nbsp;Based Board Information</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"chassis.htm\">&nbsp;Chassis</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"cache.htm\">&nbsp;Cache</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"port.htm\">&nbsp;Port Connection</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"slot.htm\">&nbsp;Slot</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"phymem.htm\">&nbsp;Physical Memory</a></td><tr>\n\

<td vAlign=top></td><td><a target=\"main\" href=\"syslog.htm\">&nbsp;System Event Log</a></td><tr>\n\ 
*/
#if 0
const static unsigned char infoHtm[] =
{
"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<STYLE type=text/css>BODY{MARGIN: 6px}\n\
A{FONT-WEIGHT: bold; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial; TEXT-DECORATION: none}\n\
BIG {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #09c; FONT-FAMILY: Calibri, Arial}\n\
BIG2 {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #aaa; FONT-FAMILY: Calibri, Arial}\n\
P{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
TABLE{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
td{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
</STYLE>\n\
<BODY bgColor=#000066 leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">\n\
<TABLE cellSpacing=2 cellPadding=1 border=0>\n\
<TBODY>\n\
<tr><td vAlign=top></td></tr>\n\
<tr><td colSpan=2></td></tr>\n"

"<tr><td colSpan=2><a target=\"main\" href=\"computersys.htm\"><BIG>123</BIG></a></td></tr>\n\
<tr><td colSpan=2><a target=\"main\" href=\"operationsys.htm\"><BIG>123</BIG></a></td></tr>\n\
<tr><td colSpan=2><a target=\"main\" href=\"softwareid.htm\"><BIG>123</BIG></a></td></tr>\n"

#ifndef CONFIG_WAKE_UP_PATTERN_ENABLED
"<tr><td colSpan=2><BIG2>System information</BIG2></td></tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"bios.htm\">&nbsp;BIOS</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"cpu.htm\">&nbsp;CPU</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"memdev.htm\">&nbsp;Memory Device</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"fan.htm\">&nbsp;Fan</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"sensor.htm\">&nbsp;Sensor</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"asset.htm\">&nbsp;Asset</a></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"remoteCtl.htm\"><BIG>Remote Control</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"eventLog.htm\"><BIG>Event Log</BIG></td><tr>\n"
#else
"<td colSpan=2><a target=\"main\" href=\"eventLog.htm\"><BIG>Patterns setting</BIG></td><tr>\n"
#endif
"<td colSpan=2><a target=\"main\" href=\"netSet.htm\"><BIG>Network setting</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"srvSet.htm\"><BIG>Service setting</BIG></td><tr>\n"
#if CONFIG_EZSHAE_TASK_ENABLED
    "<td colSpan=2><a target=\"main\" href=\"http:\\\\%s:6230\\ezShare.htm\"><BIG>ezShare</BIG></td><tr>\n"
#elif CONFIG_EZSHAE_ENABLED
    "<td colSpan=2><a target=\"main\" href=\"ezShare.htm\"><BIG>ezShare</BIG></td><tr>\n"
#endif

#ifdef CONFIG_MAIL_WAKEUP_ENABLED
"\<tr><td colSpan=2><BIG2>Mail Wake Up</BIG2></td></tr>\
<td vAlign=top></td><td><a target=\"main\" href=\"mail_acc.htm\">&nbsp;Mail Account</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"mail_mat.htm\">&nbsp;Match Condition</a></td><tr>\n"
#endif
"\<tr><td colSpan=2><a target=\"main\" href=\"about.htm\"><BIG>About</BIG></td></tr><tr>\n\
<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};
#else
const static unsigned char infoHtm_head[] =
{
"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">"
"<STYLE type=text/css>BODY{MARGIN: 6px}\n\
A{FONT-WEIGHT: bold; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial; TEXT-DECORATION: none}\n\
BIG {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #09c; FONT-FAMILY: Calibri, Arial}\n\
BIG2 {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #aaa; FONT-FAMILY: Calibri, Arial}\n\
P{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
TABLE{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
td{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
</STYLE>\n\
<BODY bgColor=#000066 leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">"
"<TABLE cellSpacing=2 cellPadding=1 border=0>\n\
<TBODY>\n\
<tr><td vAlign=top></td></tr>\n\
<tr><td colSpan=2></td></tr>\n"
};

const static unsigned char infoHtm_body1[] =
{
"<tr><td colSpan=2><a target=\"main\" href=\"computersys.htm\"><BIG>%s</BIG></a></td></tr>\n\
<tr><td colSpan=2><a target=\"main\" href=\"operationsys.htm\"><BIG>%s</BIG></a></td></tr>\n\
<tr><td colSpan=2><a target=\"main\" href=\"softwareid.htm\"><BIG>%s</BIG></a></td></tr>\n"

#ifndef CONFIG_WAKE_UP_PATTERN_ENABLED
"<tr><td colSpan=2><BIG2>%s</BIG2></td></tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"bios.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"cpu.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"memdev.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"fan.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"sensor.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"asset.htm\">&nbsp;%s</a></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"remoteCtl.htm\"><BIG>%s</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"eventLog.htm\"><BIG>%s</BIG></td><tr>\n"
#else
"<td colSpan=2><a target=\"main\" href=\"eventLog.htm\"><BIG>%s</BIG></td><tr>\n"
#endif
"<td colSpan=2><a target=\"main\" href=\"netSet.htm\"><BIG>%s</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"srvSet.htm\"><BIG></BIG></td><tr>\n"
#ifdef CONFIG_WIFI_ENABLED
"<td colSpan=2><a target=\"main\" href=\"wifiConf.htm\"><BIG>Wifi Configuration</BIG></td><tr>\n"
#endif
#if CONFIG_EZSHAE_TASK_ENABLED
    "<td colSpan=2><a target=\"main\" href=\"http:\\\\%s:6230\\ezShare.htm\"><BIG>ezShare</BIG></td><tr>\n"
#elif CONFIG_EZSHAE_ENABLED
    "<td colSpan=2><a target=\"main\" href=\"ezShare.htm\"><BIG>ezShare</BIG></td><tr>\n"
#endif
};

const static unsigned char infoHtm_body2[] =
{
#ifdef CONFIG_MAIL_WAKEUP_ENABLED
"\<tr><td colSpan=2><BIG2>%s</BIG2></td></tr>\
<td vAlign=top></td><td><a target=\"main\" href=\"mail_acc.htm\">&nbsp;%s</a></td><tr>\n\
<td vAlign=top></td><td><a target=\"main\" href=\"mail_mat.htm\">&nbsp;%s</a></td><tr>\n"
#endif

"\<tr><td colSpan=2><a target=\"main\" href=\"about.htm\"><BIG>%s</BIG></td></tr><tr>\n\
<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};
#endif

const static unsigned char remoteCtlHtmTop[] =
{
 "<html>\n\
<head><H2>Power managment</H2><link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<style>td{FONT-FAMILY:Calibri, Arial;background-color:#ffffff}</style>\n\
</head>\n\
<body>\n\
<form method=\"POST\" action=\"/remoteCtl\">\n\
<font COLOR=\"FF9900\", font face=\"Comic Sans MS, Arial\">Power Control</font>\n\
<table>\n\
<td><input type=\"radio\" name=\"power\" value=\"reset\" checked>Reset</td><tr>\n\
<td><input type=\"radio\" name=\"power\" value=\"up\">Power up</td><tr>\n\
<td><input type=\"radio\" name=\"power\" value=\"off\">Power off</td><tr>\n\
<td><input type=\"radio\" name=\"power\" value=\"cycle\">Power Cycle Reset</td><tr><tr>\n"
};

const static unsigned char remoteCtlHtmBottom[] =
{
"</table>\n\
<input type=submit value=\"Submit\">\n\
</form>\n\
</body>\n\
</html>"
};

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
const static unsigned char EditPageTopHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<STYLE type=text/css>\n\
td{FONT-FAMILY:Calibri, Arial}\n\
</STYLE>\n\
<html>\n\
<head><title>Wake Up Patterns Setting</title></head>\n\
<body>\n"
};
#endif

const static unsigned char eventLogTopHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<STYLE type=text/css>\n\
td{FONT-FAMILY:Calibri, Arial}\n\
</STYLE>\n\
<html>\n"
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
"<head><title>Wake Up Patterns Setting</title></head>\n\
<body>\n\
<H2>Wake Up Patterns</H2>\n\
<hr><table cellspacing=1 border = 1>"
#else
"<head><title>Event Log</title></head>\n\
<body>\n\
<H2>Event Log</H2>\n\
<hr><table cellspacing=0>"
#endif
};

const static unsigned char netSetIPv4Htm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n\
<head>\n\
<title>Network Setting</title>\n\
</head>\n\
<body>\n\
<form method=\"POST\" action=\"netSet\">\n\
<p></p>\n\
<H2>IPv4 Setting</h2>\n\
<Hr>\n\
<p><input type=\"radio\" %s name=\"IPv4\" value=\"v4\" %s>Set IPv4 static Address</p>\n\
<table>\n\
<td width=20 style=\"background-color:#FFffff\"></td><td style=\"background-color:#FFffff\">IP Address:</td><td><input type=text name=\"v4Addr\" %s value=%s size=16 maxlength=15></td><tr>\n\
<td style=\"background-color:#FFffff\"></td><td style=\"background-color:#FFffff\">Subnet Mask:</td><td><input type=text name=\"v4Mask\" %s value=%s size=16 maxlength=15></td><tr>\n\
<td style=\"background-color:#FFffff\"></td><td style=\"background-color:#FFffff\">Gateway:</td><td><input type=text name=\"v4Gateway\" %s value=%s size=16 maxlength=15></td><tr>\n\
<td style=\"background-color:#FFffff\"></td><td style=\"background-color:#FFffff\">DNS Server:</td><td><input type=text name=\"v4DNS\" %s value=%s size=16 maxlength=15></td>\n\
</table>\n\
%s<p><input type=\"radio\" name=\"IPv4\" value=\"DHCPv4\" %s>Use DHCPv4%s</p>\n"
#if 0
"<P><input type=\"radio\" name=\"IPv4\" %s value=\"IPv4Disable\" %s>Disable IPv4 Protocol</P>\n"
#endif
"<Hr>\n\
<p><input type=submit value=\"Submit\"></p>\n"
#ifdef CONFIG_TEREDO_ENABLED
"<p>Ipv6 Teredo address is %s</p>\n"
#endif
"</form>\n\
</body>\n\
</html>"
};

#if 0
static unsigned char netSetHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n\
<head>\n\
<title>Network Setting</title>\n\
</head>\n\
<body bgcolor=\"F0F7F5\">\n\
<form method=\"POST\" action=\"netSet\">\n\
<p></p>\n\
<p>Computer Name: \n\
<input type=text name=\"cn\" size=21 maxlength=20 ></p>\n\
<fieldset style=\"padding: 2\">\n\
<legend>IPv4 Setting</legend>\n\
<p><input type=\"radio\" name=\"IPv4\" value=\"v4\">Set IPv4 static Address </p>\n\
<pre>   IP Address:  <input type=text name=\"v4Addr\" size=16 maxlength=15></pre>\n\
<pre>   Subnet Mask: <input type=text name=\"v4Mask\"  size=16 maxlength=15></pre>\n\
<pre>   Gateway:     <input type=text name=\"v4Gateway\" size=16 maxlength=15></pre>\n\
<pre>   DNS Server:  <input type=text name=\"v4DNS\" size=16 maxlength=15></pre>\n\
<p><input type=\"radio\" name=\"IPv4\" value=\"DHCPv4\">Use DHCPv4</p>\n\
<P><input type=\"radio\" name=\"IPv4\" value=\"IPv4Disable\">Disable IPv4 Protocol</P>\n\
</fieldset></P><hr><fieldset style=\"padding: 2\">\n\
<legend>IPv6 Setting</legend>\n\
<p><input type=\"radio\" name=\"IPv6\" value=\"v6\"> Set IPv6 static Address </p>\n\
<pre>    IPv6 Address: <input type=text name=\"v6Addr\" size=16 maxlength=50></pre>\n\
<p><input type=\"radio\" name=\"IPv6\" value=\"DHCPv6\">Use DHCPv6</p>\n\
<P><input type=\"radio\" name=\"IPv6\" value=\"IPv6Disable\">Disable IPv6 Protocol</P>\n\
</fieldset></P>\n\
<p><input type=submit value=\"Submit\"></p>\n\
</form>\n\
</body>\n\
</html>"
};
#endif

/*
static unsigned char pwrstateHtm[] =
{
    "<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
    <html>\n\
    <head>\n\
    <title>Power State Information</title>\n\
    </head>\n\
    <body bgcolor=\"F0F7F5\">\n\
    <H2>%2d\n\
    </body>\n\
    </html>"
};
*/

const static unsigned char srvSetHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n\
<head>\n\
<title>Service Setting</title>\n\
</head>\n\
<body>\n\
<form method=\"POST\" action=\"srvSet\">\n\
<H2>Service Setting</h2>\n\
<Hr>\n\
<p><font COLOR\=\"FF9900\", font face=\"Comic Sans MS, Arial\">*Echo Service</font></p>\n\
<font COLOR=\"#6FD0FF\", font face=\"Comic Sans MS, Arial\">\n\
<p><input type=\"radio\" %s name=\"srv\" value=\"on\">Enable</p>\n\
<p><input type=\"radio\" %s name=\"srv\" value=\"off\">Disable</p>\n\
</font>\n\
<Hr>\n\
<p><input type=submit value=\"Submit\"></p>\n\
</form>\n\
</body>\n\
</html>"
};

#if CONFIG_WIFI_ENABLED
const static unsigned char wifiConfHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\"><HTML>\n\
<body><form method=\"POST\" action=\"wificfg\">\n\
<H2>WIFI On/Off</h2><Hr>\n\
WIFI On:<input type=radio %s name=wifiSt value=on>\n\
<br>WIFI Off:<input type=radio %s name=wifiSt value=off>\n\
<Hr><H2>Profile Setting</h2><Hr>\n\
<p>Profile name:<input type=text name=pn value=%s></p>\n\
<p>Network name(SSID):<input type=text name=netn value=%s></p>\n\
<table><tr><th>Network Authentication:</th><td>\n\
<select name=auth>\n\
<OPTION VALUE=0 %s> auto</OPTION> <OPTION VALUE=1 %s>OPEN</OPTION><OPTION VALUE=2 %s>WPA PSK</OPTION><OPTION VALUE=3 %s>WPA2 PSK</OPTION>\n\
</select><tr><th>Encryption:</th><td>\n\
<select name=enc>\n\
<OPTION VALUE=0 %s>auto</OPTION> <OPTION VALUE=1 %s>OPEN</OPTION><OPTION VALUE=2 %s>TKIP</OPTION><OPTION VALUE=3 %s>CCMP</OPTION>\n\
</SELECT><tr><th>Passphrase:</th>\n\
<td><input type=password name=pw value=%s maxlength=15></td>\n\
<tr><th>Confirm Passphrase:</th>\n\
<td><input type=password name=cfmpw value=%s maxlength=15></td>\n\
</table><Hr><input type=submit value=connect>\n\
</form></body></html>"
};
#endif

const static unsigned char notFound[] =
{
"<HTML><HEAD><style type=text/css>\n\
#rectangle {width:300px; height:40px; background:#000066; COLOR: #fff;font-size:15pt; font-weight:bold; font-family:Calibri, Arial;\n\
text-align:center; line-height:40px; position:absolute; top:50px; left:30px;\n\
}</style></HEAD><BODY><div id=\"rectangle\">!I Not Found</div></BODY></HTML>"
};
#endif
const static unsigned char securityHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n\
<head>\n\
<title>Security Setting</title>\n\
</head>\n\
<body bgcolor=\"F0F7F5\">\n\
<form method=\"POST\" action=\"/security\">\n\
<fieldset style=\"padding: 2\">\n\
<legend><b>Security Setting</b></legend>\n\
<p>Class A</p>\n\
<p>&nbsp;&nbsp;&nbsp;<input type=\"radio\" name=\"Class\" value=\"Basic\">Basic</p>\n\
<p>&nbsp;&nbsp;&nbsp;<input type=\"radio\" name=\"Class\" value=\"Digest\">Digest</p>\n\
<p>Class B</p>\n\
<p>&nbsp;&nbsp;&nbsp;<input type=\"radio\" name=\"TLS\" value=\"Class\">TLSv1</p>\n\
</fieldset><input type=\"submit\" value=\"Submit\"></form>\n\
</body>\n\
</html>"
};

const static unsigned char netSetIPv6Htm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n\
<head>\n\
<title>IPv6 Network Setting</title>\n\
</head>\n\
<body bgcolor=\"F0F7F5\">\n\
<form method=\"POST\" action=\"netSetIPv6\">\n\
<p></p>\n\
<fieldset style=\"padding: 2\">\n\
<legend>IPv6 Setting</legend>\n\
<p><input type=\"radio\" name=\"IPv6\" value=\"v6\" %s>Set IPv6 static Address </p>\n\
<pre>   IPv6 Address:  <input type=text name=\"v6Addr\" value=%s size=40 maxlength=39></pre>\n\
<pre>   Prefix length: <input type=text name=\"v6Plen\" value=%s size=4 maxlength=3></pre>\n\
<pre>   Gateway:     <input type=text name=\"v6Gateway\" value=%s size=40 maxlength=39></pre>\n\
<pre>   DNS Server:  <input type=text name=\"v6DNS\" value=%s size=40 maxlength=39></pre>\n\
<p><input type=\"radio\" name=\"IPv6\" value=\"DHCPv6\" %s>Use DHCPv6</p>\n\
<P><input type=\"radio\" name=\"IPv6\" value=\"IPv6Disable\" %s>Disable IPv6 Protocol</P>\n\
</fieldset>\n\
<p><input type=submit value=\"Submit\"></p>\n\
</form>\n\
</body>\n\
</html>"
};

const static unsigned char authHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n\
<head>\n\
<title>Network Setting</title>\n\
</head>\n\
<body bgcolor=\"F0F7F5\">\n\
<center>\n\
<form method=\"POST\" action=\"auth\">\n\
<p></p>\n\
<p>User Name:\n\
<input type=text name=\"UN\" size=17 maxlength=16 ></p>\n\
<p>Password: \n\
<input type=password name=\"PW\" size=17 maxlength=16 ></p>\n\
<pre>                   <input type=submit value=\"Submit\"></pre>\n\
</center>\n\
</form>\n\
</body>\n\
</html>"
};

const static unsigned char authFailHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html>\n\
<head>\n\
<title>Network Setting</title>\n\
</head>\n\
<body bgcolor=\"F0F7F5\">\n\
<center>\n\
<p><font color=\"#FF0000\">User name not exist or password input error(check if caps lock is on).</font></p>\n\
<form method=\"POST\" action=\"auth\">\n\
<p></p>\n\
<p>User Name:\n\
<input type=text name=\"UN\" size=17 maxlength=16 ></p>\n\
<p>Password: \n\
<input type=password name=\"PW\" size=17 maxlength=16 ></p>\n\
<pre>                   <input type=submit value=\"Submit\"></pre>\n\
</center>\n\
</form>\n\
</body>\n\
</html>"
};

const static unsigned char ezShareHtm[] =
{"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<HTML>\n\
<HEAD>\n\
<TITLE>\n\ActiveX test</TITLE>\n\
</HEAD>\n\
<BODY bgcolor=#EEEEEE>\n\
<table>\n\
</tr>\n\
<td>\n\
<OBJECT ID=\"EzShare\" WIDTH=580 HEIGHT=200\n\
CODEBASE=\"EzShare.cab\"\n\
CLASSID=\"CLSID:6151D0ED-9184-4D3E-B6B9-221253474FE6\">\n\
<PARAM NAME=\"_Version\" VALUE=\"65536\">\n\
<PARAM NAME=\"_ExtentX\" VALUE=\"12642\">\n\
<PARAM NAME=\"_ExtentY\" VALUE=\"8567\">\n\
<PARAM NAME=\"_StockProps\" VALUE=\"0\">\n\
</OBJECT>\n\
</td>\n\
</tr>\n\
</table>\n\
</BODY>\n\
</HTML>"
};

#if CONFIG_ACER_WEB_UI
static unsigned char logo[] =
{
0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x98, 0x00, 0x25, 0x00, 0xC4, 0x00, 0x00, 0x83, 0xB8, 0x1A, 0xC1, 0xDB, 0x8C, 
0xE0, 0xED, 0xC6, 0xA2, 0xCA, 0x53, 0x8B, 0xBC, 0x28, 0xF7, 0xFB, 0xF1, 0xB2, 0xD3, 0x70, 0xEF, 0xF6, 0xE2, 0xE8, 0xF2, 
0xD4, 0x9A, 0xC5, 0x45, 0xD0, 0xE4, 0xA9, 0xD8, 0xE9, 0xB7, 0x93, 0xC1, 0x37, 0xC9, 0xE0, 0x9A, 0xAA, 0xCE, 0x62, 0xB9, 
0xD7, 0x7F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xF9, 0x04, 0x01, 0x00, 0x00, 0x10, 0x00, 0x2C, 0x00, 
0x00, 0x00, 0x00, 0x98, 0x00, 0x25, 0x00, 0x00, 0x05, 0xFF, 0x20, 0x24, 0x8E, 0x64, 0x29, 0x1E, 0x42, 0xBA, 0x04, 0x6C, 
0xEB, 0xB6, 0x69, 0x61, 0xCE, 0xB4, 0x99, 0x36, 0x2F, 0x2C, 0xD4, 0x3C, 0x2A, 0xE4, 0xAC, 0x46, 0x8C, 0x47, 0x3C, 0x09, 
0x56, 0x40, 0xD6, 0x62, 0x57, 0x14, 0x29, 0x1C, 0x04, 0x80, 0x74, 0x4A, 0xAD, 0x5A, 0x01, 0x0C, 0x87, 0xE2, 0xD0, 0x2C, 
0x21, 0x02, 0x0E, 0xC6, 0xF5, 0x4A, 0x08, 0x14, 0x0B, 0x8B, 0xC7, 0x60, 0x3C, 0x26, 0x0C, 0x02, 0xCC, 0x66, 0x7A, 0xCD, 
0xB6, 0x12, 0x14, 0x45, 0x43, 0x7D, 0x3F, 0x36, 0x70, 0x8B, 0x08, 0x06, 0x51, 0x7C, 0x57, 0x78, 0x33, 0x05, 0x4F, 0x84, 
0x7B, 0x0C, 0x01, 0x32, 0x34, 0x07, 0x82, 0x8A, 0x76, 0x44, 0x0A, 0x92, 0x96, 0x00, 0x66, 0x35, 0x02, 0x74, 0x97, 0x54, 
0x0E, 0x26, 0x05, 0x01, 0x83, 0x9D, 0x6D, 0x71, 0x23, 0x90, 0xA4, 0x55, 0x44, 0x0E, 0xA9, 0x84, 0x06, 0x33, 0x08, 0x9C, 
0xAD, 0x00, 0x03, 0x25, 0xA2, 0xB3, 0x75, 0x0B, 0x24, 0x0A, 0xA3, 0xAD, 0xB5, 0x3C, 0xBD, 0xB8, 0x57, 0xBA, 0x24, 0x0D, 
0xC2, 0x54, 0x23, 0x08, 0x09, 0xC7, 0x6C, 0x04, 0x8E, 0x7A, 0xC7, 0x86, 0x34, 0x0B, 0xCC, 0x6C, 0x0C, 0x23, 0x05, 0xD0, 
0xD5, 0x4E, 0xC1, 0xD5, 0x54, 0x78, 0xDA, 0xC2, 0x8E, 0x34, 0x01, 0x53, 0x03, 0x03, 0x0E, 0x3A, 0x02, 0x08, 0x33, 0x28, 
0x01, 0xCB, 0x7B, 0x5C, 0x05, 0xF0, 0x8A, 0xE7, 0xE9, 0x2C, 0xE7, 0xE7, 0xF0, 0x04, 0x10, 0x95, 0x7B, 0x6F, 0x4B, 0xC6, 
0x8D, 0x10, 0xA0, 0x20, 0x52, 0x9D, 0x00, 0x0F, 0xAA, 0x3D, 0xE8, 0xC2, 0x70, 0x86, 0x80, 0x6E, 0x54, 0x76, 0xD0, 0xAB, 
0xA3, 0xE5, 0x4F, 0x13, 0x7F, 0x63, 0x18, 0x28, 0x10, 0xC8, 0x23, 0x5B, 0x1D, 0x59, 0x63, 0x06, 0x08, 0xB1, 0x08, 0x6A, 
0x5D, 0xC3, 0x93, 0x5E, 0xEA, 0xFF, 0x08, 0x08, 0x47, 0xA6, 0xD1, 0x49, 0x01, 0x75, 0x0C, 0x70, 0x6C, 0x02, 0x91, 0x50, 
0x02, 0x53, 0x28, 0x73, 0x76, 0x89, 0xB9, 0xC7, 0xC1, 0xCC, 0x33, 0x35, 0x01, 0xBC, 0xCA, 0x39, 0x51, 0x52, 0x82, 0x9F, 
0x3A, 0x93, 0xD2, 0x68, 0xD5, 0x40, 0x27, 0xAB, 0x90, 0x3A, 0x0F, 0xA4, 0xC2, 0xA9, 0xB4, 0xAA, 0x09, 0x63, 0xA4, 0x32, 
0xA1, 0x84, 0xD9, 0x86, 0x24, 0xC3, 0x79, 0xA4, 0xAE, 0x59, 0x1D, 0x2B, 0x02, 0xC1, 0x0A, 0x07, 0x45, 0x25, 0x7D, 0xD2, 
0x29, 0x66, 0x8C, 0xD6, 0x86, 0xBC, 0x52, 0x2D, 0x24, 0x8B, 0x12, 0x05, 0x8E, 0x01, 0x6D, 0x71, 0x39, 0xD3, 0x89, 0xF1, 
0x0A, 0x52, 0x1A, 0x08, 0x1A, 0xE4, 0x25, 0x45, 0x95, 0xAE, 0x97, 0x1F, 0x01, 0x06, 0xA4, 0x65, 0xF6, 0xF6, 0x24, 0x48, 
0x2A, 0x43, 0x79, 0x98, 0x05, 0x33, 0x98, 0x4D, 0xB9, 0x66, 0x86, 0x41, 0xCD, 0xF1, 0xB6, 0xE7, 0x2F, 0x11, 0xA9, 0x6C, 
0x88, 0x0D, 0x5C, 0x61, 0x00, 0xEF, 0xA5, 0x3B, 0x7D, 0xAD, 0xAC, 0xCD, 0x5C, 0xA0, 0xC1, 0x62, 0xCE, 0x56, 0x22, 0xA3, 
0x4C, 0x18, 0x3A, 0x40, 0xE9, 0xA0, 0xF5, 0xD8, 0x3D, 0x1D, 0x23, 0x8D, 0x2C, 0x2A, 0xD8, 0x8A, 0x44, 0xA3, 0x7C, 0xED, 
0x4D, 0xE3, 0x08, 0xDC, 0x5E, 0x95, 0x16, 0xA0, 0x3D, 0x8B, 0xC1, 0x00, 0x03, 0xC4, 0xEB, 0x78, 0xEE, 0x01, 0x7C, 0x0A, 
0x01, 0x03, 0xA6, 0xA8, 0xB1, 0x49, 0x40, 0x17, 0x41, 0x65, 0x42, 0xE8, 0x02, 0x08, 0x61, 0x37, 0xE2, 0x3B, 0x1F, 0xEE, 
0x3A, 0xB5, 0x7B, 0x73, 0x03, 0x67, 0x06, 0xF3, 0x2B, 0x73, 0xAD, 0x22, 0xC0, 0x0D, 0xC0, 0xCD, 0x83, 0x00, 0x4B, 0x3E, 
0x93, 0x92, 0x7D, 0xF2, 0x72, 0x2B, 0x37, 0xE9, 0x8C, 0x44, 0x84, 0x79, 0x53, 0x90, 0x57, 0xD5, 0x7C, 0xCD, 0x3C, 0x60, 
0x60, 0xFF, 0x17, 0x58, 0x5D, 0xD2, 0x58, 0x43, 0x8F, 0x7D, 0xF4, 0xC6, 0x78, 0x75, 0xD5, 0xC1, 0xCF, 0x58, 0x11, 0x4A, 
0x91, 0xC0, 0x82, 0x0C, 0x45, 0xC7, 0x46, 0x53, 0x3A, 0x65, 0x88, 0x05, 0x0B, 0x29, 0x90, 0xD5, 0xE0, 0x15, 0xFC, 0xE9, 
0xC4, 0xD5, 0x18, 0x1C, 0x76, 0x01, 0x5A, 0x27, 0x0F, 0x32, 0x24, 0x62, 0x8C, 0x55, 0xED, 0x56, 0xC8, 0x58, 0x2C, 0x41, 
0xA6, 0xD3, 0x89, 0x55, 0x44, 0x97, 0xA2, 0x8C, 0x6C, 0xFC, 0x58, 0x55, 0x67, 0x63, 0x05, 0x45, 0x63, 0x11, 0x33, 0x5A, 
0x38, 0x5D, 0x0D, 0x22, 0xEE, 0x45, 0x97, 0x7A, 0x57, 0xA0, 0x67, 0xD5, 0x47, 0x39, 0x15, 0x50, 0x87, 0x77, 0x75, 0x24, 
0x90, 0x5C, 0x13, 0xEF, 0x59, 0x71, 0x14, 0x5D, 0x5D, 0x56, 0x71, 0xE4, 0x49, 0x3D, 0x6D, 0xC9, 0x43, 0x6A, 0x54, 0x5C, 
0x93, 0x63, 0x6C, 0x0B, 0x2C, 0x79, 0x8A, 0x7F, 0x6D, 0x04, 0xD0, 0x62, 0x17, 0x25, 0x8A, 0x40, 0xA0, 0x14, 0x73, 0xE6, 
0x64, 0xD3, 0x03, 0x0A, 0xD4, 0x49, 0x02, 0x02, 0x29, 0x10, 0xF4, 0x0E, 0x1B, 0x0B, 0xAD, 0xC8, 0x87, 0x1B, 0xE7, 0xB0, 
0xE0, 0x40, 0x3E, 0x8A, 0xA5, 0x92, 0x40, 0x3E, 0xF7, 0xD9, 0xC6, 0x68, 0xA3, 0x56, 0x34, 0x85, 0x40, 0x1D, 0x62, 0x8D, 
0xE5, 0xA1, 0x37, 0xE4, 0x89, 0x58, 0x1D, 0x33, 0xEC, 0xF0, 0x58, 0x85, 0x90, 0x39, 0x41, 0xF9, 0x29, 0x16, 0xC9, 0xD0, 
0x47, 0x4A, 0x02, 0x6B, 0x32, 0x23, 0x96, 0xA7, 0xC2, 0x59, 0xD5, 0x2A, 0x67, 0xB2, 0x21, 0xE8, 0x8D, 0x4C, 0x36, 0x7A, 
0xB3, 0x90, 0x95, 0xD2, 0x65, 0xD6, 0x80, 0xAA, 0xB3, 0x10, 0xE0, 0x95, 0x32, 0xDE, 0x18, 0x72, 0x0B, 0x67, 0x3B, 0x98, 
0x5A, 0xC5, 0x2F, 0x99, 0x1D, 0xF0, 0x00, 0xB0, 0x9D, 0xF8, 0x41, 0x83, 0x02, 0x77, 0x5A, 0x22, 0x25, 0x45, 0x04, 0xCE, 
0x42, 0x6B, 0x93, 0x08, 0xB3, 0x02, 0x00, 0x62, 0x66, 0x23, 0xAC, 0x30, 0x80, 0xB6, 0x55, 0x20, 0x2A, 0x5E, 0x61, 0x26, 
0x20, 0xA0, 0x46, 0x2A, 0x22, 0xFD, 0xB4, 0x59, 0x2B, 0x8C, 0x38, 0xD2, 0xA4, 0x99, 0xE0, 0x42, 0xA0, 0x42, 0x12, 0x01, 
0xF4, 0x19, 0xE8, 0x58, 0x80, 0xE2, 0x80, 0xAF, 0x10, 0x43, 0xB8, 0x98, 0x02, 0xBE, 0x2C, 0xE8, 0x9B, 0x42, 0x72, 0x07, 
0x24, 0xD1, 0x00, 0xBD, 0x39, 0x85, 0x00, 0x00, 0x3B
};

static unsigned char AcerWBgTriGif[] =
{
	0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x32, 0x00, 0x2C, 0x00, 0xD5, 0x00, 0x00, 0xED, 0xF0, 0xEA, 0xE7, 0xEB, 0xE3, 
0xE8, 0xEC, 0xE4, 0xED, 0xF1, 0xEB, 0xF0, 0xF3, 0xEE, 0xEA, 0xEE, 0xE6, 0xF1, 0xF4, 0xEF, 0xF0, 0xF3, 0xED, 0xED, 0xF1, 
0xEA, 0xF2, 0xF5, 0xF0, 0xF3, 0xF5, 0xF1, 0xFE, 0xFE, 0xFE, 0xF6, 0xF8, 0xF5, 0xF2, 0xF5, 0xF1, 0xF4, 0xF6, 0xF2, 0xFE, 
0xFE, 0xFD, 0xFB, 0xFC, 0xFB, 0xF3, 0xF6, 0xF1, 0xF2, 0xF4, 0xF0, 0xF8, 0xF9, 0xF7, 0xFB, 0xFC, 0xFA, 0xF1, 0xF4, 0xEE, 
0xF9, 0xFA, 0xF9, 0xF4, 0xF6, 0xF3, 0xEE, 0xF2, 0xEC, 0xFC, 0xFD, 0xFC, 0xFA, 0xFB, 0xFA, 0xFC, 0xFC, 0xFB, 0xFD, 0xFD, 
0xFC, 0xF7, 0xF9, 0xF6, 0xF3, 0xF6, 0xF2, 0xF7, 0xF8, 0xF5, 0xE9, 0xED, 0xE6, 0xF7, 0xF9, 0xF7, 0xF2, 0xF5, 0xEF, 0xF9, 
0xFA, 0xF7, 0xF5, 0xF8, 0xF4, 0xF9, 0xFA, 0xF8, 0xE9, 0xEE, 0xE7, 0xFD, 0xFE, 0xFD, 0xE9, 0xEC, 0xE5, 0xEE, 0xF1, 0xEC, 
0xF5, 0xF7, 0xF3, 0xF4, 0xF7, 0xF3, 0xEC, 0xF0, 0xEA, 0xEF, 0xF2, 0xEC, 0xEE, 0xF2, 0xEB, 0xE8, 0xED, 0xE5, 0xEC, 0xF0, 
0xE9, 0xE7, 0xEC, 0xE4, 0xEB, 0xEF, 0xE8, 0xEA, 0xEE, 0xE7, 0xE9, 0xEE, 0xE6, 0xE8, 0xED, 0xE4, 0xFF, 0xFF, 0xFF, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x2C, 0x00, 0x00, 0x06, 0xFF, 0xC0, 0x40, 
0x2C, 0x26, 0xA8, 0x19, 0x5F, 0x2F, 0x9A, 0xB2, 0x30, 0x6B, 0xCA, 0x9E, 0xB0, 0x28, 0x8B, 0x85, 0x18, 0x0C, 0x5C, 0xD8, 
0x96, 0xF6, 0x70, 0x20, 0x78, 0x0D, 0xE0, 0x44, 0xA2, 0xA1, 0x28, 0x3B, 0x2E, 0x0C, 0x21, 0xD1, 0x58, 0x43, 0x2A, 0x69, 
0x4C, 0x27, 0x54, 0x4A, 0xB5, 0x62, 0x5D, 0xDA, 0x16, 0xD7, 0x4B, 0x00, 0x1B, 0xC4, 0x64, 0x66, 0x1F, 0x0B, 0x6A, 0x45, 
0x47, 0x49, 0x4B, 0x4D, 0x33, 0x4F, 0x32, 0x51, 0x30, 0x53, 0x55, 0x57, 0x59, 0x5B, 0x5D, 0x5F, 0x61, 0x63, 0x65, 0x0A, 
0x13, 0x36, 0x36, 0x85, 0x6C, 0x6E, 0x89, 0x72, 0x8D, 0x74, 0x91, 0x77, 0x79, 0x7B, 0x96, 0x7F, 0x98, 0x0A, 0x25, 0x9C, 
0x9D, 0x43, 0x86, 0x6D, 0x88, 0x70, 0x8A, 0x8C, 0x8E, 0x90, 0x76, 0x93, 0x7A, 0x95, 0x7D, 0x97, 0x64, 0x1A, 0xAE, 0xAF, 
0x6B, 0x87, 0x6F, 0x71, 0x8B, 0x73, 0x8F, 0x75, 0x92, 0x78, 0x94, 0x7C, 0x7E, 0x62, 0x11, 0x10, 0xC1, 0xC2, 0xB1, 0xA0, 
0xB4, 0xA2, 0xB7, 0xCA, 0xA6, 0xCD, 0xA9, 0x09, 0x11, 0x1C, 0xD3, 0xD4, 0x9F, 0xB3, 0xC6, 0xB6, 0xA4, 0xB9, 0xCC, 0xBB, 
0xCE, 0x60, 0x17, 0x27, 0xE1, 0xE2, 0xC4, 0xA1, 0xC7, 0xA3, 0xC9, 0xA5, 0xBA, 0xA8, 0xBD, 0x0E, 0x0B, 0xEF, 0xF0, 0xB2, 
0xC5, 0xB5, 0xC8, 0x70, 0x2D, 0x3B, 0xC5, 0xCB, 0x00, 0x83, 0x7D, 0xFC, 0x3C, 0xC5, 0xBB, 0x36, 0x2F, 0x9B, 0xBD, 0x74, 
0xF8, 0x18, 0xF0, 0x73, 0xA5, 0xD0, 0x9F, 0x3C, 0x73, 0xF5, 0xD0, 0x11, 0xF4, 0x62, 0x61, 0x22, 0x45, 0x58, 0xE3, 0xFE, 
0x61, 0x3B, 0x37, 0x90, 0x52, 0x85, 0x8E, 0x1E, 0x39, 0x55, 0xB4, 0x56, 0x2E, 0xA0, 0xB6, 0x7B, 0x15, 0x32, 0xA4, 0xFC, 
0x38, 0xCC, 0x22, 0x43, 0x8C, 0x02, 0xB7, 0xB5, 0x10, 0x21, 0x73, 0xA6, 0x4A, 0xC1, 0x90, 0x0B, 0x5B, 0xD2, 0xCB, 0x39, 
0x49, 0x02, 0x42, 0x9F, 0xFD, 0x58, 0x02, 0x1C, 0xFA, 0x12, 0x8F, 0x3E, 0xA4, 0x34, 0xAB, 0x91, 0x5B, 0xEA, 0x10, 0x9D, 
0x0A, 0xA8, 0xC1, 0x56, 0x4E, 0x1D, 0x99, 0x51, 0x92, 0x44, 0xAC, 0x51, 0x43, 0x5E, 0x74, 0x59, 0x6A, 0x04, 0xD8, 0xAC, 
0x40, 0x6D, 0x0A, 0xAD, 0x8A, 0x81, 0xC2, 0x59, 0xB4, 0x35, 0x95, 0x72, 0x85, 0x84, 0x61, 0xC3, 0x5B, 0xB8, 0x52, 0x45, 
0x36, 0xA4, 0x43, 0xE0, 0xE8, 0xDD, 0xA4, 0x5B, 0xF7, 0x3E, 0xEA, 0xFB, 0x17, 0xAF, 0xD8, 0x9B, 0xC8, 0x3C, 0x14, 0x9E, 
0xA6, 0x55, 0xAF, 0xB9, 0x15, 0x8B, 0x19, 0xA7, 0x95, 0xDB, 0x70, 0x53, 0x64, 0xC3, 0x41, 0x01, 0xC2, 0xB0, 0x7C, 0x39, 
0x6C, 0x66, 0x27, 0x30, 0xA4, 0x75, 0xC6, 0xAC, 0x56, 0x51, 0x0B, 0xD1, 0xA3, 0x3D, 0x97, 0x9E, 0x91, 0xE2, 0x41, 0x6A, 
0xC9, 0x71, 0xA7, 0x4A, 0x70, 0xFD, 0x9A, 0x34, 0x4B, 0x09, 0xB5, 0xC3, 0x35, 0x56, 0x42, 0x22, 0xB7, 0xEE, 0xC9, 0x88, 
0x3A, 0xF8, 0xFE, 0x1D, 0xDB, 0x84, 0xDB, 0xE1, 0xB0, 0xAB, 0xC9, 0x38, 0x8E, 0x9C, 0x34, 0x0B, 0xDA, 0xCD, 0x31, 0xBB, 
0x80, 0x1E, 0x3D, 0x6C, 0x85, 0xEA, 0xEF, 0x0A, 0x45, 0xC0, 0x9E, 0x7D, 0xC8, 0x57, 0xEE, 0x8C, 0x41, 0x84, 0x00, 0xFF, 
0x0E, 0x05, 0x30, 0xF2, 0xE1, 0x82, 0x00, 0x00, 0x3B
};

static unsigned char AcerInfoABgGif[] =
{
	0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0xC8, 0x00, 0x01, 0x00, 0xE6, 0x00, 0x00, 0x8D, 0x8B, 0x85, 0x99, 0x97, 0x90, 
0x78, 0x75, 0x72, 0x8F, 0x8C, 0x87, 0x72, 0x70, 0x6E, 0x89, 0x87, 0x82, 0x87, 0x85, 0x81, 0x7C, 0x7A, 0x77, 0xAA, 0xAA, 
0xA3, 0x75, 0x73, 0x70, 0x6A, 0x69, 0x68, 0x69, 0x68, 0x66, 0x83, 0x80, 0x7C, 0x6C, 0x6A, 0x69, 0x67, 0x66, 0x65, 0x5D, 
0x5E, 0x5E, 0x64, 0x64, 0x63, 0x5F, 0x60, 0x5F, 0x80, 0x7E, 0x7A, 0x66, 0x65, 0x64, 0x70, 0x6E, 0x6C, 0x5F, 0x5F, 0x5F, 
0x77, 0x75, 0x72, 0x60, 0x61, 0x60, 0xDC, 0xE1, 0xD7, 0x7E, 0x7B, 0x78, 0x61, 0x61, 0x60, 0x93, 0x91, 0x8B, 0x76, 0x74, 
0x71, 0x68, 0x67, 0x66, 0xB3, 0xB4, 0xAC, 0x7B, 0x7A, 0x76, 0xB9, 0xBA, 0xB1, 0x63, 0x63, 0x62, 0x81, 0x7F, 0x7B, 0x94, 
0x92, 0x8B, 0xC9, 0xCC, 0xC3, 0x9D, 0x9B, 0x94, 0xBB, 0xBC, 0xB3, 0xC0, 0xC3, 0xB9, 0xD1, 0xD7, 0xCD, 0xA9, 0xA8, 0xA0, 
0x9C, 0x9A, 0x93, 0xA2, 0xA1, 0x9A, 0xB6, 0xB7, 0xAF, 0xDD, 0xE4, 0xDA, 0xC9, 0xCD, 0xC4, 0x9E, 0x9D, 0x96, 0xBE, 0xC0, 
0xB7, 0x8B, 0x89, 0x84, 0xD7, 0xDD, 0xD4, 0xDA, 0xE0, 0xD6, 0xD2, 0xD8, 0xCE, 0x7A, 0x78, 0x75, 0x97, 0x95, 0x8F, 0xC2, 
0xC5, 0xBC, 0x6A, 0x69, 0x67, 0xAC, 0xAC, 0xA4, 0x6E, 0x6C, 0x6A, 0x9A, 0x98, 0x91, 0x92, 0x90, 0x8A, 0xB0, 0xB1, 0xA8, 
0x62, 0x62, 0x62, 0x63, 0x63, 0x63, 0xB7, 0xB9, 0xB0, 0x86, 0x84, 0x7F, 0x8F, 0x8D, 0x88, 0xD8, 0xDE, 0xD5, 0xC8, 0xCB, 
0xC2, 0xCC, 0xD1, 0xC7, 0x86, 0x86, 0x86, 0xA4, 0xA3, 0x9C, 0xA7, 0xA7, 0xA0, 0xC4, 0xC7, 0xBE, 0xA3, 0xA2, 0x9B, 0xC6, 
0xCA, 0xC0, 0xD6, 0xDB, 0xD1, 0xD7, 0xDC, 0xD2, 0xDD, 0xE3, 0xD9, 0xAF, 0xB0, 0xA8, 0xB2, 0xB2, 0xAA, 0xCF, 0xD4, 0xCA, 
0xB4, 0xB5, 0xAD, 0xC5, 0xC8, 0xBF, 0xB5, 0xB6, 0xAE, 0xAE, 0xAE, 0xA6, 0xA5, 0xA5, 0x9D, 0xE1, 0xE6, 0xDC, 0xA7, 0xA6, 
0x9F, 0xCB, 0xCF, 0xC5, 0x9E, 0x9C, 0x95, 0x9F, 0x9E, 0x97, 0x72, 0x70, 0x6D, 0x73, 0x72, 0x6F, 0x65, 0x64, 0x64, 0x84, 
0x81, 0x7D, 0x85, 0x82, 0x7E, 0x86, 0x83, 0x7F, 0xDF, 0xE5, 0xDB, 0xBD, 0xBF, 0xB6, 0xD4, 0xD9, 0xCF, 0xD1, 0xD5, 0xCC, 
0xCD, 0xD2, 0xC8, 0xC1, 0xC4, 0xBA, 0x79, 0x77, 0x74, 0x7B, 0x79, 0x76, 0x96, 0x94, 0x8D, 0xA6, 0xA5, 0x9E, 0xCE, 0xD3, 
0xC9, 0xC3, 0xC6, 0xBD, 0xBF, 0xC1, 0xB8, 0xBA, 0xBB, 0xB2, 0x7F, 0x7D, 0x79, 0xAD, 0xAD, 0xA5, 0xA0, 0xA0, 0x98, 0x95, 
0x93, 0x8C, 0xA1, 0xA1, 0x99, 0x6F, 0x6D, 0x6B, 0x71, 0x6F, 0x6D, 0xBB, 0xBD, 0xB4, 0x6D, 0x6B, 0x6A, 0x62, 0x62, 0x61, 
0x61, 0x61, 0x61, 0x9B, 0x99, 0x92, 0xB1, 0xB1, 0xA9, 0x90, 0x8E, 0x88, 0xE2, 0xE7, 0xDE, 0x5D, 0x5E, 0x5D, 0x21, 0xF9, 
0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x00, 0x07, 0xAB, 0x80, 0x46, 
0x7F, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x87, 0x0F, 0x8B, 0x15, 0x15, 0x11, 0x11, 0x17, 0x17, 0x1A, 0x7A, 0x79, 0x3E, 
0x21, 0x3F, 0x10, 0x10, 0x5E, 0x13, 0x13, 0x0E, 0x0E, 0x1D, 0x0B, 0xA1, 0x38, 0x0A, 0x0D, 0x0D, 0x78, 0x78, 0x3A, 0x75, 
0x75, 0x14, 0x14, 0x76, 0x5C, 0x04, 0x5D, 0xB1, 0x09, 0x1C, 0x16, 0x16, 0x02, 0x68, 0x68, 0x35, 0x69, 0x1F, 0x07, 0x19, 
0x19, 0x70, 0x12, 0x12, 0x22, 0x0C, 0x0C, 0x5F, 0x60, 0x61, 0x41, 0x41, 0x06, 0x05, 0xCC, 0x31, 0x31, 0x00, 0x03, 0x03, 
0x42, 0x7D, 0x3C, 0x3C, 0x1B, 0x23, 0x73, 0x6A, 0x36, 0x01, 0x01, 0x3B, 0x7B, 0x2A, 0x25, 0x5A, 0x2F, 0x5B, 0x72, 0x74, 
0x2B, 0x4A, 0x47, 0x56, 0x6B, 0x58, 0x48, 0x29, 0x08, 0x08, 0x39, 0x71, 0x55, 0x4F, 0x3D, 0x7C, 0x50, 0x1E, 0x52, 0x54, 
0x2C, 0x40, 0x20, 0x6F, 0x26, 0x77, 0x63, 0x60, 0xB8, 0x39, 0x71, 0xE6, 0x46, 0x9B, 0x24, 0x53, 0x96, 0x10, 0x21, 0xE1, 
0x22, 0x4B, 0x11, 0x33, 0x6C, 0xA2, 0x94, 0x41, 0x41, 0x83, 0x0C, 0x93, 0x26, 0x32, 0x86, 0xCC, 0xC0, 0xE0, 0xA4, 0x85, 
0x98, 0x2B, 0x7E, 0x42, 0x8A, 0x1C, 0xE9, 0x27, 0x10, 0x00, 0x3B
};

static unsigned char AcerInfoBgGif[] =
{
	0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0xFA, 0x00, 0x01, 0x00, 0xE6, 0x00, 0x00, 0xB5, 0xC9, 0xA1, 0xAE, 0xC5, 0x95, 
0xF0, 0xF3, 0xEE, 0xAA, 0xC2, 0x8D, 0x97, 0xB5, 0x71, 0xEC, 0xF1, 0xEA, 0xF4, 0xF6, 0xF3, 0xC7, 0xD6, 0xBE, 0x8E, 0xB0, 
0x66, 0x98, 0xB6, 0x75, 0xE3, 0xE9, 0xDF, 0x9D, 0xB9, 0x7A, 0xC9, 0xD8, 0xC2, 0xE5, 0xEB, 0xE2, 0xDE, 0xE6, 0xDB, 0xDD, 
0xE5, 0xD8, 0xB8, 0xCB, 0xA5, 0xB4, 0xC9, 0x9E, 0xD2, 0xDE, 0xCE, 0xA5, 0xBE, 0x86, 0x8B, 0xAE, 0x61, 0xE9, 0xEE, 0xE6, 
0xB0, 0xC6, 0x98, 0xD8, 0xE2, 0xD4, 0xD6, 0xE1, 0xD3, 0xD1, 0xDD, 0xCC, 0xCE, 0xDA, 0xC8, 0xBA, 0xCD, 0xA9, 0x89, 0xAC, 
0x5E, 0xAD, 0xC3, 0x92, 0xFD, 0xFD, 0xFC, 0xF8, 0xF9, 0xF6, 0xF9, 0xFA, 0xF8, 0xC2, 0xD2, 0xB6, 0xC0, 0xD0, 0xB2, 0xF3, 
0xF5, 0xF1, 0x9E, 0xBA, 0x7D, 0xEB, 0xEF, 0xE9, 0x8F, 0xB1, 0x67, 0xCC, 0xDA, 0xC7, 0xCB, 0xD9, 0xC5, 0xBF, 0xD0, 0xB1, 
0x7C, 0xA6, 0x50, 0xB3, 0xC7, 0x9D, 0xA8, 0xC0, 0x8B, 0xA6, 0xBF, 0x89, 0xA3, 0xBD, 0x85, 0xA0, 0xBC, 0x80, 0x95, 0xB4, 
0x6F, 0x94, 0xB3, 0x6E, 0x92, 0xB2, 0x6B, 0x8C, 0xAF, 0x63, 0x84, 0xAA, 0x59, 0x7F, 0xA7, 0x53, 0x81, 0xA8, 0x55, 0xE0, 
0xE7, 0xDC, 0x7F, 0xA8, 0x54, 0xE8, 0xED, 0xE4, 0xCF, 0xDC, 0xCA, 0xFE, 0xFF, 0xFE, 0xD6, 0xE0, 0xD1, 0xBC, 0xCE, 0xAC, 
0x86, 0xAB, 0x5B, 0xF6, 0xF8, 0xF5, 0xC4, 0xD4, 0xB9, 0xEF, 0xF3, 0xED, 0xDA, 0xE3, 0xD5, 0xCE, 0xDB, 0xCA, 0x7D, 0xA7, 
0x52, 0xD5, 0xDF, 0xD0, 0x7B, 0xA6, 0x50, 0x85, 0xAA, 0x5A, 0xE3, 0xEA, 0xE0, 0xE6, 0xEC, 0xE3, 0xB9, 0xCC, 0xA7, 0xBD, 
0xD2, 0xA7, 0xA2, 0xBC, 0x83, 0x82, 0xA9, 0x56, 0x83, 0xAA, 0x57, 0xFC, 0xFC, 0xFB, 0xDF, 0xE7, 0xDC, 0xDB, 0xE4, 0xD7, 
0x9B, 0xB8, 0x78, 0xBE, 0xCF, 0xB0, 0xD3, 0xDF, 0xCF, 0xA7, 0xC0, 0x8A, 0x87, 0xAC, 0x5C, 0xC8, 0xD7, 0xC0, 0x93, 0xB3, 
0x6C, 0xD2, 0xDE, 0xCD, 0xAE, 0xC4, 0x93, 0xE8, 0xEE, 0xE5, 0x9A, 0xB8, 0x77, 0xBE, 0xCF, 0xAF, 0x91, 0xB1, 0x6A, 0x83, 
0xAA, 0x58, 0xFB, 0xFC, 0xFA, 0xDD, 0xE6, 0xDA, 0xF7, 0xF9, 0xF6, 0xFD, 0xFE, 0xFD, 0xAB, 0xC2, 0x8F, 0xEA, 0xEF, 0xE7, 
0xD7, 0xE2, 0xD4, 0xEC, 0xF0, 0xE9, 0xED, 0xF1, 0xEB, 0x8A, 0xAD, 0x5F, 0x8D, 0xAF, 0x64, 0xDA, 0xE3, 0xD6, 0xCD, 0xDA, 
0xC8, 0xF1, 0xF4, 0xF0, 0xF3, 0xF6, 0xF2, 0xE1, 0xE8, 0xDD, 0xFA, 0xFB, 0xF9, 0xFA, 0xFB, 0xFA, 0xBD, 0xCE, 0xAE, 0xC1, 
0xD1, 0xB4, 0xC1, 0xD2, 0xB5, 0xC3, 0xD3, 0xB8, 0xF5, 0xF8, 0xF4, 0xD0, 0xDC, 0xCB, 0xB2, 0xC7, 0x9B, 0xDC, 0xE4, 0xD7, 
0x88, 0xAC, 0x5C, 0xE7, 0xEC, 0xE4, 0xC5, 0xD4, 0xBA, 0xC6, 0xD5, 0xBB, 0xA1, 0xBC, 0x82, 0xBB, 0xCD, 0xAB, 0x2C, 0x00, 
0x00, 0x00, 0x00, 0xFA, 0x00, 0x01, 0x00, 0x00, 0x07, 0xE8, 0x80, 0x46, 0x82, 0x2A, 0x44, 0x85, 0x35, 0x35, 0x38, 0x38, 
0x36, 0x36, 0x4D, 0x4D, 0x4E, 0x5F, 0x34, 0x34, 0x47, 0x47, 0x3E, 0x3E, 0x56, 0x7A, 0x1C, 0x1C, 0x69, 0x9B, 0x14, 0x14, 
0x33, 0x6A, 0x6A, 0x08, 0x08, 0x26, 0xA4, 0x5E, 0x5E, 0x32, 0x32, 0x58, 0x31, 0x31, 0x30, 0x30, 0x04, 0xAF, 0x04, 0x09, 
0xB2, 0xB2, 0x5C, 0x52, 0x0B, 0xB7, 0x0B, 0x24, 0xBA, 0x2F, 0xBC, 0x7E, 0x4C, 0x2E, 0x2E, 0x13, 0xC2, 0x2D, 0x2D, 0x55, 
0x2C, 0x2C, 0x03, 0xC9, 0x64, 0x64, 0x1D, 0xCD, 0x5A, 0x5A, 0x01, 0xD1, 0x16, 0xD3, 0x78, 0x2B, 0x2B, 0x11, 0xD8, 0x00, 
0xDA, 0x10, 0xDC, 0x10, 0x4A, 0x4A, 0x1B, 0x1B, 0x7F, 0x3D, 0x3D, 0x72, 0x5D, 0x53, 0x29, 0x29, 0x22, 0x73, 0x73, 0x74, 
0x21, 0x75, 0x75, 0x40, 0x40, 0x7C, 0x7D, 0x7D, 0x07, 0xF7, 0x57, 0x0C, 0xFA, 0x28, 0x28, 0x27, 0x27, 0x6C, 0x1A, 0x34, 
0x0C, 0x19, 0xA8, 0x43, 0xC7, 0x9D, 0x0C, 0x08, 0xB3, 0x48, 0xA0, 0xC2, 0xB0, 0x48, 0x11, 0x1E, 0x3C, 0x30, 0x48, 0x34, 
0x73, 0xA1, 0xA2, 0x10, 0x21, 0x6B, 0xA2, 0xE4, 0xC9, 0xF3, 0xE0, 0x41, 0x18, 0x07, 0x0E, 0xA0, 0x40, 0xB9, 0x71, 0xE3, 
0x8D, 0x82, 0x93, 0x0A, 0x90, 0xA8, 0x6C, 0xD0, 0x20, 0x49, 0x92, 0x3D, 0x39, 0x72, 0x6C, 0xD9, 0x52, 0xA1, 0x42, 0x99, 
0x12, 0x25, 0xCE, 0x9C, 0x29, 0x80, 0xA6, 0x67, 0x90, 0x20, 0x02, 0x82, 0xB6, 0x69, 0x33, 0x62, 0x84, 0x1B, 0x03, 0x48, 
0xED, 0xD8, 0xF9, 0xF1, 0x43, 0xCC, 0x07, 0x10, 0x50, 0x41, 0xC0, 0x89, 0x03, 0xE6, 0x89, 0x87, 0xAB, 0x63, 0x76, 0xEC, 
0x08, 0x04, 0x00, 0x3B
};

static unsigned char AcerBannerBgGif[] =
{
0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x7C, 0x01, 0x01, 0x00, 0xE6, 0x00, 0x00, 0xE5, 0xF1, 0xD8, 0xF8, 0xFB, 0xF5, 
0xF0, 0xF7, 0xEA, 0xDC, 0xEC, 0xCC, 0xEC, 0xF5, 0xE4, 0xF6, 0xFA, 0xF3, 0xFE, 0xFF, 0xFE, 0xE4, 0xF0, 0xD7, 0xE2, 0xEF, 
0xD5, 0xDE, 0xED, 0xCE, 0xFC, 0xFD, 0xFB, 0xEA, 0xF3, 0xE0, 0xF7, 0xFB, 0xF4, 0xD9, 0xEA, 0xC6, 0xF6, 0xFA, 0xF2, 0xE1, 
0xEF, 0xD3, 0xF2, 0xF7, 0xEC, 0xFD, 0xFE, 0xFD, 0xF5, 0xF9, 0xF0, 0xFD, 0xFE, 0xFC, 0xE9, 0xF3, 0xDE, 0xE3, 0xF0, 0xD5, 
0xDD, 0xED, 0xCD, 0xD8, 0xEA, 0xC5, 0xDA, 0xEB, 0xC9, 0xE7, 0xF2, 0xDC, 0xD9, 0xEB, 0xC7, 0xF3, 0xF8, 0xEE, 0xEF, 0xF6, 
0xE9, 0xE0, 0xEE, 0xD2, 0xEB, 0xF4, 0xE2, 0xDF, 0xEE, 0xD0, 0xE9, 0xF3, 0xE0, 0xE6, 0xF1, 0xDA, 0xDB, 0xEC, 0xCB, 0xEC, 
0xF4, 0xE3, 0xED, 0xF5, 0xE5, 0xE9, 0xF3, 0xDF, 0xEE, 0xF5, 0xE6, 0xFB, 0xFD, 0xF9, 0xFA, 0xFC, 0xF7, 0xE8, 0xF2, 0xDC, 
0xF4, 0xF9, 0xF0, 0xDD, 0xED, 0xCC, 0xFC, 0xFD, 0xFA, 0xE2, 0xEF, 0xD4, 0xEB, 0xF4, 0xE1, 0xE9, 0xF3, 0xDD, 0xFD, 0xFE, 
0xFB, 0xE7, 0xF2, 0xDB, 0xFB, 0xFD, 0xFA, 0xF4, 0xF9, 0xEF, 0xE6, 0xF1, 0xDB, 0xDF, 0xEE, 0xCF, 0xF6, 0xFA, 0xF1, 0xDA, 
0xEB, 0xCA, 0xD8, 0xEA, 0xC6, 0xE8, 0xF2, 0xDD, 0xD9, 0xEB, 0xC8, 0xEE, 0xF5, 0xE7, 0xDB, 0xEC, 0xCA, 0xE3, 0xF0, 0xD6, 
0xEF, 0xF6, 0xE7, 0xF5, 0xF9, 0xF1, 0xED, 0xF5, 0xE4, 0xF3, 0xF8, 0xEF, 0xEC, 0xF4, 0xE4, 0xFF, 0xFF, 0xFF, 0xED, 0xF5, 
0xE6, 0xF7, 0xFB, 0xF3, 0xFF, 0xFF, 0xFE, 0xE1, 0xEF, 0xD4, 0xD7, 0xE9, 0xC4, 0xE4, 0xF0, 0xD6, 0xEA, 0xF3, 0xE1, 0xDA, 
0xEB, 0xC8, 0xDE, 0xED, 0xCF, 0xD9, 0xEA, 0xC7, 0xEC, 0xF4, 0xE2, 0xD7, 0xE9, 0xC5, 0xDF, 0xEE, 0xD1, 0xF9, 0xFB, 0xF6, 
0xF1, 0xF7, 0xEB, 0xE6, 0xF1, 0xD9, 0xE0, 0xEE, 0xD1, 0xF2, 0xF8, 0xED, 0xEF, 0xF6, 0xE8, 0xFA, 0xFC, 0xF8, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x00, 
0x00, 0x00, 0x00, 0x7C, 0x01, 0x01, 0x00, 0x00, 0x07, 0xF7, 0x80, 0x48, 0x4F, 0x17, 0x84, 0x84, 0x38, 0x87, 0x0D, 0x89, 
0x8A, 0x4D, 0x1A, 0x8D, 0x8D, 0x3A, 0x90, 0x4B, 0x18, 0x93, 0x93, 0x37, 0x96, 0x3C, 0x98, 0x22, 0x9A, 0x22, 0x03, 0x9D, 
0x9E, 0x03, 0x2B, 0xA1, 0x16, 0xA3, 0xA3, 0x09, 0xA6, 0xA7, 0x4C, 0x35, 0xAA, 0x1F, 0xAC, 0xAC, 0x54, 0xAF, 0xB0, 0x50, 
0xB0, 0x1D, 0xB4, 0xB4, 0x0F, 0xB7, 0xB8, 0x47, 0x2D, 0xBB, 0x08, 0xBD, 0xBE, 0x15, 0xC0, 0xC0, 0x3D, 0xC3, 0x49, 0x07, 
0xC6, 0xC7, 0x00, 0xC9, 0xCA, 0x00, 0x53, 0xCD, 0xCE, 0xCD, 0x21, 0xD1, 0x21, 0x34, 0xD4, 0x31, 0xD6, 0x19, 0xD8, 0xD8, 
0x29, 0xDB, 0x29, 0x39, 0xDE, 0x2F, 0xE0, 0x14, 0xE2, 0xE2, 0x25, 0xE5, 0x25, 0x20, 0xE8, 0xE8, 0x0B, 0xEB, 0xEC, 0x4A, 
0x2E, 0xEF, 0x1E, 0xF1, 0xF1, 0x4E, 0x23, 0xF5, 0x23, 0x42, 0x42, 0x04, 0xFA, 0xFB, 0x04, 0x40, 0x40, 0x24, 0x00, 0x49, 
0x10, 0x21, 0x62, 0xA2, 0xA0, 0x89, 0x1D, 0x08, 0x7D, 0xF8, 0xB0, 0xC2, 0xB0, 0xA1, 0x43, 0x0E, 0x10, 0x21, 0x0A, 0x98, 
0x48, 0x51, 0x80, 0x94, 0x8B, 0x18, 0x2F, 0x42, 0xD8, 0xC8, 0xB1, 0x8A, 0xC7, 0x8F, 0x1F, 0x37, 0x88, 0x14, 0x19, 0x24, 
0xC8, 0x8C, 0x93, 0x2A, 0x52, 0xAA, 0x90, 0xC0, 0xB2, 0xE5, 0x8F, 0x1F, 0x36, 0x62, 0x3A, 0x98, 0x49, 0xB3, 0x80, 0xCD, 
0x9B, 0x05, 0x8A, 0x14, 0x61, 0xC0, 0xB3, 0x67, 0x80, 0x9F, 0x40, 0x03, 0x44, 0x19, 0x4A, 0x74, 0x28, 0x8A, 0xA3, 0x28, 
0xAE, 0x28, 0x5D, 0xCA, 0xF4, 0xCA, 0x89, 0xA7, 0x27, 0x64, 0x48, 0x65, 0x41, 0x55, 0x81, 0xD5, 0xAB, 0x30, 0xB2, 0x4E, 
0xD8, 0xCA, 0x35, 0x82, 0xD7, 0xAF, 0x06, 0xC2, 0x8A, 0x35, 0x60, 0xC4, 0xC8, 0x90, 0x21, 0x81, 0x00, 0x00, 0x3B
};

static unsigned char AcerListLabelGif[] =
{
0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x0B, 0x00, 0x15, 0x00, 0xA2, 0x00, 0x00, 0xD6, 0xE0, 0xDB, 0xED, 0xF2, 0xF0, 
0xD9, 0xE2, 0xDE, 0xD3, 0xDD, 0xD9, 0xF6, 0xF8, 0xF7, 0xFC, 0xFD, 0xFC, 0xD0, 0xDB, 0xD6, 0xFF, 0xFF, 0xFF, 0x2C, 0x00, 
0x00, 0x00, 0x00, 0x0B, 0x00, 0x15, 0x00, 0x00, 0x03, 0x32, 0x58, 0x62, 0xDC, 0x7D, 0xCA, 0xB9, 0x03, 0x97, 0x34, 0xB4, 
0xDE, 0x0C, 0x81, 0xE4, 0x07, 0x31, 0x4C, 0xA0, 0xF8, 0x80, 0xE1, 0x88, 0xA1, 0xE9, 0xCA, 0x8A, 0x6C, 0x16, 0xC4, 0x69, 
0x6C, 0xBE, 0x2A, 0x6A, 0x32, 0xA5, 0xCA, 0x67, 0x05, 0x0F, 0x49, 0xF3, 0x21, 0x7E, 0x22, 0x17, 0x03, 0x32, 0x99, 0x00, 
0x00, 0x3B
};

static unsigned char AcerBGTriangleGif[] =
{0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x32, 0x00, 0x2C, 0x00, 0xD5, 0x00, 0x00, 0xED, 0xF0, 0xEA, 0xE7, 0xEB, 0xE3, 
0xE8, 0xEC, 0xE4, 0xED, 0xF1, 0xEB, 0xF0, 0xF3, 0xEE, 0xEA, 0xEE, 0xE6, 0xF1, 0xF4, 0xEF, 0xF0, 0xF3, 0xED, 0xED, 0xF1, 
0xEA, 0xF2, 0xF5, 0xF0, 0xF3, 0xF5, 0xF1, 0xFE, 0xFE, 0xFE, 0xF6, 0xF8, 0xF5, 0xF2, 0xF5, 0xF1, 0xF4, 0xF6, 0xF2, 0xFE, 
0xFE, 0xFD, 0xFB, 0xFC, 0xFB, 0xF3, 0xF6, 0xF1, 0xF2, 0xF4, 0xF0, 0xF8, 0xF9, 0xF7, 0xFB, 0xFC, 0xFA, 0xF1, 0xF4, 0xEE, 
0xF9, 0xFA, 0xF9, 0xF4, 0xF6, 0xF3, 0xEE, 0xF2, 0xEC, 0xFC, 0xFD, 0xFC, 0xFA, 0xFB, 0xFA, 0xFC, 0xFC, 0xFB, 0xFD, 0xFD, 
0xFC, 0xF7, 0xF9, 0xF6, 0xF3, 0xF6, 0xF2, 0xF7, 0xF8, 0xF5, 0xE9, 0xED, 0xE6, 0xF7, 0xF9, 0xF7, 0xF2, 0xF5, 0xEF, 0xF9, 
0xFA, 0xF7, 0xF5, 0xF8, 0xF4, 0xF9, 0xFA, 0xF8, 0xE9, 0xEE, 0xE7, 0xFD, 0xFE, 0xFD, 0xE9, 0xEC, 0xE5, 0xEE, 0xF1, 0xEC, 
0xF5, 0xF7, 0xF3, 0xF4, 0xF7, 0xF3, 0xEC, 0xF0, 0xEA, 0xEF, 0xF2, 0xEC, 0xEE, 0xF2, 0xEB, 0xE8, 0xED, 0xE5, 0xEC, 0xF0, 
0xE9, 0xE7, 0xEC, 0xE4, 0xEB, 0xEF, 0xE8, 0xEA, 0xEE, 0xE7, 0xE9, 0xEE, 0xE6, 0xE8, 0xED, 0xE4, 0xFF, 0xFF, 0xFF, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x2C, 0x00, 0x00, 0x06, 0xFF, 0xC0, 0x40, 
0x2C, 0x26, 0xA8, 0x19, 0x5F, 0x2F, 0x9A, 0xB2, 0x30, 0x6B, 0xCA, 0x9E, 0xB0, 0x28, 0x8B, 0x85, 0x18, 0x0C, 0x5C, 0xD8, 
0x96, 0xF6, 0x70, 0x20, 0x78, 0x0D, 0xE0, 0x44, 0xA2, 0xA1, 0x28, 0x3B, 0x2E, 0x0C, 0x21, 0xD1, 0x58, 0x43, 0x2A, 0x69, 
0x4C, 0x27, 0x54, 0x4A, 0xB5, 0x62, 0x5D, 0xDA, 0x16, 0xD7, 0x4B, 0x00, 0x1B, 0xC4, 0x64, 0x66, 0x1F, 0x0B, 0x6A, 0x45, 
0x47, 0x49, 0x4B, 0x4D, 0x33, 0x4F, 0x32, 0x51, 0x30, 0x53, 0x55, 0x57, 0x59, 0x5B, 0x5D, 0x5F, 0x61, 0x63, 0x65, 0x0A, 
0x13, 0x36, 0x36, 0x85, 0x6C, 0x6E, 0x89, 0x72, 0x8D, 0x74, 0x91, 0x77, 0x79, 0x7B, 0x96, 0x7F, 0x98, 0x0A, 0x25, 0x9C, 
0x9D, 0x43, 0x86, 0x6D, 0x88, 0x70, 0x8A, 0x8C, 0x8E, 0x90, 0x76, 0x93, 0x7A, 0x95, 0x7D, 0x97, 0x64, 0x1A, 0xAE, 0xAF, 
0x6B, 0x87, 0x6F, 0x71, 0x8B, 0x73, 0x8F, 0x75, 0x92, 0x78, 0x94, 0x7C, 0x7E, 0x62, 0x11, 0x10, 0xC1, 0xC2, 0xB1, 0xA0, 
0xB4, 0xA2, 0xB7, 0xCA, 0xA6, 0xCD, 0xA9, 0x09, 0x11, 0x1C, 0xD3, 0xD4, 0x9F, 0xB3, 0xC6, 0xB6, 0xA4, 0xB9, 0xCC, 0xBB, 
0xCE, 0x60, 0x17, 0x27, 0xE1, 0xE2, 0xC4, 0xA1, 0xC7, 0xA3, 0xC9, 0xA5, 0xBA, 0xA8, 0xBD, 0x0E, 0x0B, 0xEF, 0xF0, 0xB2, 
0xC5, 0xB5, 0xC8, 0x70, 0x2D, 0x3B, 0xC5, 0xCB, 0x00, 0x83, 0x7D, 0xFC, 0x3C, 0xC5, 0xBB, 0x36, 0x2F, 0x9B, 0xBD, 0x74, 
0xF8, 0x18, 0xF0, 0x73, 0xA5, 0xD0, 0x9F, 0x3C, 0x73, 0xF5, 0xD0, 0x11, 0xF4, 0x62, 0x61, 0x22, 0x45, 0x58, 0xE3, 0xFE, 
0x61, 0x3B, 0x37, 0x90, 0x52, 0x85, 0x8E, 0x1E, 0x39, 0x55, 0xB4, 0x56, 0x2E, 0xA0, 0xB6, 0x7B, 0x15, 0x32, 0xA4, 0xFC, 
0x38, 0xCC, 0x22, 0x43, 0x8C, 0x02, 0xB7, 0xB5, 0x10, 0x21, 0x73, 0xA6, 0x4A, 0xC1, 0x90, 0x0B, 0x5B, 0xD2, 0xCB, 0x39, 
0x49, 0x02, 0x42, 0x9F, 0xFD, 0x58, 0x02, 0x1C, 0xFA, 0x12, 0x8F, 0x3E, 0xA4, 0x34, 0xAB, 0x91, 0x5B, 0xEA, 0x10, 0x9D, 
0x0A, 0xA8, 0xC1, 0x56, 0x4E, 0x1D, 0x99, 0x51, 0x92, 0x44, 0xAC, 0x51, 0x43, 0x5E, 0x74, 0x59, 0x6A, 0x04, 0xD8, 0xAC, 
0x40, 0x6D, 0x0A, 0xAD, 0x8A, 0x81, 0xC2, 0x59, 0xB4, 0x35, 0x95, 0x72, 0x85, 0x84, 0x61, 0xC3, 0x5B, 0xB8, 0x52, 0x45, 
0x36, 0xA4, 0x43, 0xE0, 0xE8, 0xDD, 0xA4, 0x5B, 0xF7, 0x3E, 0xEA, 0xFB, 0x17, 0xAF, 0xD8, 0x9B, 0xC8, 0x3C, 0x14, 0x9E, 
0xA6, 0x55, 0xAF, 0xB9, 0x15, 0x8B, 0x19, 0xA7, 0x95, 0xDB, 0x70, 0x53, 0x64, 0xC3, 0x41, 0x01, 0xC2, 0xB0, 0x7C, 0x39, 
0x6C, 0x66, 0x27, 0x30, 0xA4, 0x75, 0xC6, 0xAC, 0x56, 0x51, 0x0B, 0xD1, 0xA3, 0x3D, 0x97, 0x9E, 0x91, 0xE2, 0x41, 0x6A, 
0xC9, 0x71, 0xA7, 0x4A, 0x70, 0xFD, 0x9A, 0x34, 0x4B, 0x09, 0xB5, 0xC3, 0x35, 0x56, 0x42, 0x22, 0xB7, 0xEE, 0xC9, 0x88, 
0x3A, 0xF8, 0xFE, 0x1D, 0xDB, 0x84, 0xDB, 0xE1, 0xB0, 0xAB, 0xC9, 0x38, 0x8E, 0x9C, 0x34, 0x0B, 0xDA, 0xCD, 0x31, 0xBB, 
0x80, 0x1E, 0x3D, 0x6C, 0x85, 0xEA, 0xEF, 0x0A, 0x45, 0xC0, 0x9E, 0x7D, 0xC8, 0x57, 0xEE, 0x8C, 0x41, 0x84, 0x00, 0xFF, 
0x0E, 0x05, 0x30, 0xF2, 0xE1, 0x82, 0x00, 0x00, 0x3B
};
#else
static unsigned char logo[] =
{
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x30, 0x00, 0x30, 0x00, 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0xC0, 0xC0,
    0xC0, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33,
    0x00, 0x00, 0x66, 0x00, 0x00, 0x99, 0x00, 0x00, 0xCC, 0x00, 0x00, 0xFF, 0x00, 0x33, 0x00, 0x00, 0x33, 0x33, 0x00, 0x33,
    0x66, 0x00, 0x33, 0x99, 0x00, 0x33, 0xCC, 0x00, 0x33, 0xFF, 0x00, 0x66, 0x00, 0x00, 0x66, 0x33, 0x00, 0x66, 0x66, 0x00,
    0x66, 0x99, 0x00, 0x66, 0xCC, 0x00, 0x66, 0xFF, 0x00, 0x99, 0x00, 0x00, 0x99, 0x33, 0x00, 0x99, 0x66, 0x00, 0x99, 0x99,
    0x00, 0x99, 0xCC, 0x00, 0x99, 0xFF, 0x00, 0xCC, 0x00, 0x00, 0xCC, 0x33, 0x00, 0xCC, 0x66, 0x00, 0xCC, 0x99, 0x00, 0xCC,
    0xCC, 0x00, 0xCC, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x33, 0x00, 0xFF, 0x66, 0x00, 0xFF, 0x99, 0x00, 0xFF, 0xCC, 0x00,
    0xFF, 0xFF, 0x33, 0x00, 0x00, 0x33, 0x00, 0x33, 0x33, 0x00, 0x66, 0x33, 0x00, 0x99, 0x33, 0x00, 0xCC, 0x33, 0x00, 0xFF,
    0x33, 0x33, 0x00, 0x33, 0x33, 0x33, 0x33, 0x33, 0x66, 0x33, 0x33, 0x99, 0x33, 0x33, 0xCC, 0x33, 0x33, 0xFF, 0x33, 0x66,
    0x00, 0x33, 0x66, 0x33, 0x33, 0x66, 0x66, 0x33, 0x66, 0x99, 0x33, 0x66, 0xCC, 0x33, 0x66, 0xFF, 0x33, 0x99, 0x00, 0x33,
    0x99, 0x33, 0x33, 0x99, 0x66, 0x33, 0x99, 0x99, 0x33, 0x99, 0xCC, 0x33, 0x99, 0xFF, 0x33, 0xCC, 0x00, 0x33, 0xCC, 0x33,
    0x33, 0xCC, 0x66, 0x33, 0xCC, 0x99, 0x33, 0xCC, 0xCC, 0x33, 0xCC, 0xFF, 0x33, 0xFF, 0x00, 0x33, 0xFF, 0x33, 0x33, 0xFF,
    0x66, 0x33, 0xFF, 0x99, 0x33, 0xFF, 0xCC, 0x33, 0xFF, 0xFF, 0x66, 0x00, 0x00, 0x66, 0x00, 0x33, 0x66, 0x00, 0x66, 0x66,
    0x00, 0x99, 0x66, 0x00, 0xCC, 0x66, 0x00, 0xFF, 0x66, 0x33, 0x00, 0x66, 0x33, 0x33, 0x66, 0x33, 0x66, 0x66, 0x33, 0x99,
    0x66, 0x33, 0xCC, 0x66, 0x33, 0xFF, 0x66, 0x66, 0x00, 0x66, 0x66, 0x33, 0x66, 0x66, 0x66, 0x66, 0x66, 0x99, 0x66, 0x66,
    0xCC, 0x66, 0x66, 0xFF, 0x66, 0x99, 0x00, 0x66, 0x99, 0x33, 0x66, 0x99, 0x66, 0x66, 0x99, 0x99, 0x66, 0x99, 0xCC, 0x66,
    0x99, 0xFF, 0x66, 0xCC, 0x00, 0x66, 0xCC, 0x33, 0x66, 0xCC, 0x66, 0x66, 0xCC, 0x99, 0x66, 0xCC, 0xCC, 0x66, 0xCC, 0xFF,
    0x66, 0xFF, 0x00, 0x66, 0xFF, 0x33, 0x66, 0xFF, 0x66, 0x66, 0xFF, 0x99, 0x66, 0xFF, 0xCC, 0x66, 0xFF, 0xFF, 0x99, 0x00,
    0x00, 0x99, 0x00, 0x33, 0x99, 0x00, 0x66, 0x99, 0x00, 0x99, 0x99, 0x00, 0xCC, 0x99, 0x00, 0xFF, 0x99, 0x33, 0x00, 0x99,
    0x33, 0x33, 0x99, 0x33, 0x66, 0x99, 0x33, 0x99, 0x99, 0x33, 0xCC, 0x99, 0x33, 0xFF, 0x99, 0x66, 0x00, 0x99, 0x66, 0x33,
    0x99, 0x66, 0x66, 0x99, 0x66, 0x99, 0x99, 0x66, 0xCC, 0x99, 0x66, 0xFF, 0x99, 0x99, 0x00, 0x99, 0x99, 0x33, 0x99, 0x99,
    0x66, 0x99, 0x99, 0x99, 0x99, 0x99, 0xCC, 0x99, 0x99, 0xFF, 0x99, 0xCC, 0x00, 0x99, 0xCC, 0x33, 0x99, 0xCC, 0x66, 0x99,
    0xCC, 0x99, 0x99, 0xCC, 0xCC, 0x99, 0xCC, 0xFF, 0x99, 0xFF, 0x00, 0x99, 0xFF, 0x33, 0x99, 0xFF, 0x66, 0x99, 0xFF, 0x99,
    0x99, 0xFF, 0xCC, 0x99, 0xFF, 0xFF, 0xCC, 0x00, 0x00, 0xCC, 0x00, 0x33, 0xCC, 0x00, 0x66, 0xCC, 0x00, 0x99, 0xCC, 0x00,
    0xCC, 0xCC, 0x00, 0xFF, 0xCC, 0x33, 0x00, 0xCC, 0x33, 0x33, 0xCC, 0x33, 0x66, 0xCC, 0x33, 0x99, 0xCC, 0x33, 0xCC, 0xCC,
    0x33, 0xFF, 0xCC, 0x66, 0x00, 0xCC, 0x66, 0x33, 0xCC, 0x66, 0x66, 0xCC, 0x66, 0x99, 0xCC, 0x66, 0xCC, 0xCC, 0x66, 0xFF,
    0xCC, 0x99, 0x00, 0xCC, 0x99, 0x33, 0xCC, 0x99, 0x66, 0xCC, 0x99, 0x99, 0xCC, 0x99, 0xCC, 0xCC, 0x99, 0xFF, 0xCC, 0xCC,
    0x00, 0xCC, 0xCC, 0x33, 0xCC, 0xCC, 0x66, 0xCC, 0xCC, 0x99, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFF, 0xCC, 0xFF, 0x00, 0xCC,
    0xFF, 0x33, 0xCC, 0xFF, 0x66, 0xCC, 0xFF, 0x99, 0xCC, 0xFF, 0xCC, 0xCC, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x33,
    0xFF, 0x00, 0x66, 0xFF, 0x00, 0x99, 0xFF, 0x00, 0xCC, 0xFF, 0x00, 0xFF, 0xFF, 0x33, 0x00, 0xFF, 0x33, 0x33, 0xFF, 0x33,
    0x66, 0xFF, 0x33, 0x99, 0xFF, 0x33, 0xCC, 0xFF, 0x33, 0xFF, 0xFF, 0x66, 0x00, 0xFF, 0x66, 0x33, 0xFF, 0x66, 0x66, 0xFF,
    0x66, 0x99, 0xFF, 0x66, 0xCC, 0xFF, 0x66, 0xFF, 0xFF, 0x99, 0x00, 0xFF, 0x99, 0x33, 0xFF, 0x99, 0x66, 0xFF, 0x99, 0x99,
    0xFF, 0x99, 0xCC, 0xFF, 0x99, 0xFF, 0xFF, 0xCC, 0x00, 0xFF, 0xCC, 0x33, 0xFF, 0xCC, 0x66, 0xFF, 0xCC, 0x99, 0xFF, 0xCC,
    0xCC, 0xFF, 0xCC, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x33, 0xFF, 0xFF, 0x66, 0xFF, 0xFF, 0x99, 0xFF, 0xFF, 0xCC, 0xFF,
    0xFF, 0xFF, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x00, 0x08, 0xFF, 0x00, 0x1F, 0x08, 0x1C, 0x48, 0xB0,
    0xA0, 0xC1, 0x83, 0x08, 0x13, 0x2A, 0x5C, 0xC8, 0xB0, 0xA1, 0xC3, 0x87, 0x10, 0x23, 0x4A, 0x9C, 0x48, 0xB1, 0x22, 0x43,
    0x06, 0x18, 0x07, 0x62, 0x64, 0x60, 0x51, 0xE0, 0x46, 0x8D, 0x19, 0x0B, 0x7E, 0xF4, 0x18, 0xD2, 0xE2, 0xC8, 0x07, 0x27,
    0x41, 0x72, 0x24, 0xB9, 0xD2, 0x64, 0xC9, 0x94, 0x2C, 0x55, 0x76, 0x44, 0xF9, 0xB2, 0xA4, 0xCA, 0x8D, 0x30, 0x27, 0xE2,
    0xDC, 0x69, 0x70, 0x27, 0xCE, 0x99, 0x3E, 0x73, 0x06, 0xB5, 0x49, 0x71, 0x68, 0xCB, 0x9E, 0x44, 0x67, 0xD2, 0x3C, 0x8A,
    0x30, 0x27, 0xD0, 0xA4, 0x48, 0x99, 0x2A, 0x75, 0x2A, 0x12, 0x6A, 0xCC, 0xAA, 0x52, 0x65, 0x2E, 0xA4, 0xBA, 0x34, 0x6A,
    0x42, 0xAE, 0x37, 0xB3, 0x0A, 0xB5, 0xFA, 0x73, 0x2B, 0xCF, 0x83, 0x52, 0xA1, 0xFA, 0xBC, 0x78, 0xB6, 0x27, 0x52, 0xB4,
    0x6D, 0xCD, 0x96, 0x25, 0x98, 0xB6, 0xEE, 0xDC, 0x86, 0x6B, 0xE9, 0xBE, 0x0D, 0x9B, 0x55, 0x2E, 0xCC, 0xBA, 0x74, 0xE3,
    0x42, 0xBC, 0xBB, 0x34, 0x2F, 0x4B, 0xAB, 0x78, 0xC1, 0x36, 0x25, 0x3C, 0x58, 0x31, 0x5C, 0xC7, 0x5F, 0x05, 0x2B, 0x34,
    0x3C, 0xD9, 0xE8, 0xD0, 0xA8, 0x96, 0x25, 0x67, 0x0E, 0x8A, 0x79, 0x33, 0x51, 0xA3, 0x85, 0x39, 0x1F, 0xCE, 0x0B, 0x7A,
    0x23, 0x0B, 0x16, 0x8F, 0x3D, 0x33, 0x36, 0xC8, 0xC2, 0xE7, 0xE9, 0xD4, 0xAA, 0x11, 0x0F, 0x6C, 0x3D, 0x97, 0xF6, 0xC7,
    0xD8, 0x54, 0x71, 0xBE, 0x8E, 0x69, 0x1A, 0x35, 0xDF, 0xCD, 0x58, 0x45, 0xDF, 0xC6, 0xB8, 0x7B, 0xB4, 0xE7, 0x85, 0xB6,
    0x79, 0xB7, 0xA4, 0x4D, 0x33, 0x74, 0xE8, 0x95, 0x7D, 0x19, 0x14, 0x87, 0x9E, 0xB1, 0xFA, 0xCF, 0x9A, 0x2D, 0x4F, 0x52,
    0xBF, 0x8C, 0x52, 0xB9, 0x73, 0xEC, 0x5A, 0xBB, 0xA7, 0x39, 0xF6, 0xE8, 0xDD, 0x26, 0xF8, 0xA6, 0x5B, 0xC9, 0x77, 0xB5,
    0xDE, 0xFC, 0x3C, 0xEC, 0x90, 0xD0, 0xD5, 0x0F, 0x6F, 0xCF, 0xD1, 0x7D, 0x67, 0xF8, 0xF2, 0xD5, 0xD3, 0xDF, 0x7F, 0xF5,
    0x7D, 0x7C, 0xF1, 0x7D, 0xF1, 0x37, 0x95, 0x7E, 0x91, 0x35, 0xA7, 0x14, 0x80, 0x0F, 0xC9, 0x16, 0x51, 0x80, 0x07, 0x36,
    0xE8, 0xE0, 0x83, 0x10, 0x12, 0x14, 0x10, 0x00, 0x3B
};
#endif


#if CONFIG_MAIL_WAKEUP_ENABLED

//due to the full profile's size is greater thant ETH_PAYLOAD_LEN (1300 bytes)
//we split the webpage into two ones.
//don't let it more than 1022 bytes after apply sprintf(...)
//mail_wakeup 20121025 eccheng
const static unsigned char mail_wakeup_accHtm[] =
{
"<html><head>\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\"> \
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> \
<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<STYLE type=text/css>\n\
p{COLOR: #FF9900; FONT-FAMILY:Comic Sans MS, Arial}\n\
td{COLOR:#6FD0FF; FONT-FAMILY: Comic Sans MS, Arial}\n\
</STYLE>\n\
<title>Mail Wake Up Setting - Email Account</title></head> \
<body> \
<b>Mail Wake Up Setting - Account</b> \
<form method=\"POST\" action=\"mail_acc_set\">\
<table> \
<tr> \
<td>Mail Wakeup Status</td> \
<td><input type=\"checkbox\" name=\"ckbox_en\" value=\"E\" %s>Enable</td>\n\
</tr> \
<tr> \
<td>Email Server</td> \
<td><input type=\"text\" name=\"e_svr\" size=\"30\" maxlength=\"30\" value=\"%s\">IP or domain name</td> \
</tr> \
<tr> \
<td>SSL Connection</td> \
<td><input type=\"checkbox\" name=\"ckbox_ssl\" value=\"E\" disabled %s>Enable</td>\
</tr> \
<tr>\
<td>Email Account</td>\
<td><input type=\"text\" name=\"e_acc\" size=\"30\" maxlength=\"30\" value=\"%s\"></td>\
</tr>\
<tr>\
<td>Email Passwaord</td>\
<td><input type=\"text\" name=\"e_psw\" size=\"30\" maxlength=\"30\" value=\"%s\"></td>\
</tr>\
<tr>\
</table> \
<p><input type=\"submit\" value=\"Save Settings\" name=\"save\"></p> \
<p><input type=\"reset\" value=\"Restore Settings\"></p> \
</form> \
<p>last email's check %04d:%02d:%02d:%02d:%02d:%02d</p>\
<p>%s</p>\
</body> \
</html>"
};
//<font size=\"1\" color=\"red\"> Font size 1 </font>\
//notice: name=\"period\" <== no space int "period" or will parsing error
// i.e., "perio " or " period" is not allowed



const static unsigned char mail_wakeup_matHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n \
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n \
<STYLE type=text/css>\n\
p{COLOR: #FF9900; FONT-FAMILY:Comic Sans MS, Arial}\n\
td{COLOR:#6FD0FF; FONT-FAMILY: Comic Sans MS, Arial}\n\
</STYLE>\n\
<title>Mail Wake Up Setting - Matching Condition</title></head>\n \
<body>\n \
<p><b>Mail Wake Up Setting - Matching Condition</b></p>\n \
<form method=\"POST\" action=\"mail_mat_set\">\
<table>\n \
<tr>\n\
<td>match_address</td>\n\
<td><input type=\"text\" name=\"m_add\" size=\"30\" maxlength=\"30\" value=\"%s\"></td>\n\
</tr>\n\
<tr>\n\
<td>match_subject</td>\n\
<td><input type=\"text\" name=\"m_sub\" size=\"30\" maxlength=\"30\" value=\"%s\"></td>\n\
</tr>\n\
<tr>\n\
<td>match_content</td>\n\
<td><input type=\"text\" name=\"m_cnt\" size=\"30\" maxlength=\"30\" value=\"%s\">only check the first 1024 bytes of email body</td>\n\
</tr>\n\
<tr>\n\
<td>Activate_condition  </td>\n\
<td>\n\
<input type=\"checkbox\" name=\"op_addr\" value=\"E\"%s> Address \n\
<input type=\"checkbox\" name=\"op_sub\" value=\"E\"%s> Subject \n\
<input type=\"checkbox\" name=\"op_cont\" value=\"E\"%s> Content \n\
</td>\n\
</tr>\n\
<tr>\n\
<td>Checking Period</td>\n\
<td><input type=\"text\" name=\"Period\" size=\"2\" maxlength=\"2\" value=\"%d\">5~99</td>\n\
</tr>\n\
</table>\n \
<p><input type=\"submit\" value=\"Save Settings\" name=\"save\"></p>\n \
<p><input type=\"reset\" value=\"Restore Settings\"></p>\n \
</form> \
</body> \
</html>"
};
#endif

const static unsigned char computerSysHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html><head>\n\
<title>Computer System</title>\n\
<body>\n\
 <H2>Computer System</H2>\n\
<hr>\n\
<table cellspacing=0>\n\
<td>Name:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Primary Owner:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Primary Owner Contact:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Enabled State:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Requested State:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Dedicated to:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>IdentifyingDescriptions:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>OtherIdentifyingInfo:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Power State:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Supported Power Caoabilities</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>   </td><td width=10></td><td> </td><td width=100></td><tr>\n\
</table>\n\
<hr>\n\
</body></html>"
};

const static unsigned char operationSysHtm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html><head>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Operation System</title>\n\
</head><body>\n\
 <H2>Operation System</H2>\n\
<hr>\n\
<table cellspacing=0>\n\
<td>Name:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>OSType:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Enable:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>RequestedState:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>AvailableRequestedStates:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>TransitioningToState:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
</table>\n\
<hr>\n\
</body></html>"
};
const static unsigned char fanHtm_h[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html><head>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Fan</title>\n\
</head><body>\n\
 <H2>Fan</H2>\n\
<hr>\n\
<table cellspacing=0>\n"};

//<td>Name:</td><td width=10></td><td>�ӧa</td><td width=100></td><tr>\n\
//<th>OSType:</th><th width=10></th><th>�H�H</th><th width=100></th><tr>\n\
//</table>\n\
//<hr>\n\
//</body></html>"

const static unsigned char sensorHtm_h[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html><head>\n\
<title>Sensor</title>\n\
<body>\n\
 <H2>Sensor</H2>\n\
<hr>\n\
<table cellspacing=0>\n"};

const static unsigned char assetHtm_h[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html><head>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Asset</title>\n\
</head><body>\n\ 
\n"};

const static unsigned char memoryHtm_h[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html><head>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Memory</title>\n\
</head><body>\n\
 <H2>Memory</H2>\n\
<hr>\n"
};

//NumberOfBlocks
//ConsumableBlocks
const static unsigned char memoryHtm_table[] =
{
"<td>Name:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>DeviceID:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>HealthState:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>EnabledState:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>RequestedState:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Access:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Total Memory:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Available Memory:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Volatile:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th> </th><th width=10></th><th> </th><th width=100></th><tr>\n"
};

const static unsigned char fanHtm_table[] =
{
"<td>ElementName:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>DeviceID:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>HealthState:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>VariableSpeed:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>DesiredSpeed:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>ActiveCooling:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>EnabledState:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>RequestedState:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>OperatingStatus:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th> </th><th width=10></th><th> </th><th width=100></th><tr>\n"
};

const static unsigned char phyMemHtm_table[] =
{
"<td>ElementName:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Manufacturer:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Model:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>SerialNumber:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>PartNumber:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>SKU:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Tag:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>MemoryType:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Speed:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Capacity:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>BankLabel:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th> </th><th width=10></th><th> </th><th width=100></th><tr>\n"
};

const static unsigned char cardHtm_table[] =
{
"<td>ElementName:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Manufacturer:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Model:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>SerialNumber:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>PartNumber:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>SKU:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Tag:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th> </th><th width=10></th><th> </th><th width=100></th><tr>\n"
};

const static unsigned char chassisHtm_table[] =
{
"<td>ElementName:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Manufacturer:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Model:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>SerialNumber:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>PartNumber:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>SKU:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Tag:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Version:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<th> </th><th width=10></th><th> </th><th width=100></th><tr>\n"
};


const static unsigned char sensorHtm_table[] =
{
"<td>ElementName:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>DeviceID:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>SensorType:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>PossibleStates:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>CurrentState:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>HealthState:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>EnabledState:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>RequestedState:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>OperatingStatus:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th> </th><th width=10></th><th> </th><th width=100></th><tr>\n"
};

const static unsigned char normal_Htm_tail[] =
{
"</body></html>"
};

const static unsigned char processorHtm_h[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html><head>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Processor</title>\n\
</head><body>\n\
 <H2>Processor</H2>\n\
<hr>\n"
};


const static unsigned char processorHtm_table[] =
{
"<table cellspacing=0>\n\
<td>Name:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>DeviceID:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Family:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>CPUStatus:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>HealthState:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>CurrentClockSpeed:</th><th width=10></th><th>%s MHz</th><th width=100></th><tr>\n\
<td>MaxClockSpeed:</td><td width=10></td><td>%s MHz</td><td width=100></td><tr>\n\
<th>ExternalBusClockSpeed:</th><th width=10></th><th>%s MHz</th><th width=100></th><tr>\n\
<td>LoadPercentage:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>EnabledState:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>RequestedState:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>OperatingStatus:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
</table>\n"
};


const static unsigned char softwareIdentiyHtm_h[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html><head>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Software Identity</title>\n\
</head><body>\n\
 <H2>Software Identity</H2>\n\
<hr>\n"
};
const static unsigned char softwareIdentiyHtm_t[] =
{
"</body></html>"
};

const static unsigned char softwareIdPropertyHtm[] =
{
"<table cellspacing=0>\n\
<td>InstanceID:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>IsEntity:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>VersionString:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>MajorVersion:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>MinorVersion:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>RevisionNumber:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>BuildNumber:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>IdentityInfoType:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>IdentityInfoValue:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Classifications:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
</table><hr>\n\n"
};



const static unsigned char biosHtm_h[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<html><head>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> \
<title>BIOS Information</title>\n\
</head>\n\
<body>\n\
 <H2>BIOS Information</H2>\n\
<hr>\n"
};

const static unsigned char biosHtm[] =
{
//<table cellspacing=0>\n
"<td>BIOS name:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>Manufacturer:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>Version:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>PrimaryBIOS:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>SoftwareElementID:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>SoftwareElementState:</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
<td>TargetOperatingSystem:</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th></th><th width=10></th><th></th><th width=100></th><tr>"
//</table><hr>\n\n
};

const static unsigned char biosattrHtm[] =
{
"<table cellspacing=0>\n\
<td>%s</td><td width=10></td><td>%s</td><td width=100></td><tr>\n\
<th>%s</th><th width=10></th><th>%s</th><th width=100></th><tr>\n\
</table><hr>\n\n"
};

const static unsigned char biosHtm_t[] =
{
"</body></html>"
};
//<font size=\"1\" color=\"red\"> Font size 1 </font>\
//notice: name=\"period\" <== no space int "period" or will parsing error
// i.e., "perio " or " period" is not allowed


const static char *webPages[] =
{
    0, rdcss, logo, bannerHtm, homeHtm, welcomeHtm, 0, 0, 0, 0, securityHtm/*10*/, netSetIPv4Htm, netSetIPv6Htm, 0 ,0, authHtm, authFailHtm, okPage, ezShareHtm, 0/*bin*/
    #if CONFIG_MAIL_WAKEUP_ENABLED
	,mail_wakeup_accHtm, mail_wakeup_matHtm
    #endif
    #if CONFIG_ACER_WEB_UI
    ,AcerWBgTriGif/*20*/, AcerInfoABgGif, AcerInfoBgGif, AcerBannerBgGif, AcerListLabelGif, AcerBGTriangleGif 
    #endif    	
	,computerSysHtm,operationSysHtm
};

/*
<select name=\"a_cnd\">
  <option value="7">7</option>
  <option value="12">12</option>
  <option value="13" selected>13</option>
</select>

 <input type=\"text\" name=\"a_cnd\" size=\"2\" maxlength=\"2\" value=\"%d\">only support 7,12,13 now

*/

#ifdef CONFIG_MAIL_WAKEUP_ENABLED

//mail_wakeup 20121025 eccheng
int genMail_wakeup_accPage(char* buf)
{
    int offset = 0;
    int rv = 0;
    char checked[]={"checked"};
    char smtp_ssl[8],smtp_en[8];

    memset(smtp_ssl,0x00,8);	
    memset(smtp_en,0x00,8);		

	if(dpconf->mail_wake_up != 0x00){
		memcpy(smtp_en,checked,7);			
	}

	if(dpconf->smtp_SSL == 0x01){
		memcpy(smtp_ssl,checked,7);
	}
	
                         
    //translate the time-zone from GreenWhich to TW (+8)
    if ((dpconf->ts_base.hour+8)<=24)
    {
        rv = sprintf(buf ,mail_wakeup_accHtm , 
                       smtp_en,
                       dpconf->email_server,
                       smtp_ssl,
                       dpconf->email_acount,
                       dpconf->email_passwd,
                       dpconf->ts_base.year,
                       dpconf->ts_base.month,
                       dpconf->ts_base.date,
                       dpconf->ts_base.hour+8,
                       dpconf->ts_base.minute,
                       dpconf->ts_base.second,
                       getWarringMsg());        
    }                     
    else
    {
        rv = sprintf(buf ,mail_wakeup_accHtm , 
                       smtp_en,
                       dpconf->email_server,
                       smtp_ssl,
                       dpconf->email_acount,
                       dpconf->email_passwd,
                       dpconf->ts_base.year,
                       dpconf->ts_base.month,
                       (dpconf->ts_base.date + 1),
                       (dpconf->ts_base.hour + 8 - 24),
                       dpconf->ts_base.minute,
                       dpconf->ts_base.second,
                       getWarringMsg());                
    }

//DEBUGMSG(1,"%s [mail_wakeupHtm_acc length=%d, buf length=%d, ETH_PAYLOAD_LEN=%d]\n", __func__, strlen(mail_wakeup_accHtm), strlen(buf), ETH_PAYLOAD_LEN);
//DEBUGMSG(1,"%s [%s]\n", __func__, buf);
                                              
    offset += rv;
    return offset;
}

//mail_wakeup 20121025 eccheng
int genMail_wakeup_matPage(char* buf)
{
    int offset = 0;
    int rv = 0;
    char checked[]={"checked"};
    char con_addr[8],con_sub[8],con_cont[8];	

    memset(con_addr,0x00,8);	
	memset(con_sub,0x00,8);	
	memset(con_cont,0x00,8);	

//printf("[RTK]Activate_condition=0x%x\n",dpconf->Activate_condition);

//printf("[RTK]0x%x,0x%x,0x%x\n",dpconf->Activate_condition & 0x01 
//							,dpconf->Activate_condition & 0x02 
//							,dpconf->Activate_condition & 0x04 );

if((dpconf->Activate_condition & 0x01) == 0x01){
	memcpy(con_addr,checked,7);
}
if((dpconf->Activate_condition & 0x02) == 0x02){
	memcpy(con_sub,checked,7);		
}
if((dpconf->Activate_condition & 0x04) == 0x04){
	memcpy(con_cont,checked,7);
}
                      
    rv = sprintf(buf ,mail_wakeup_matHtm , 
                   dpconf->match_address,
                   dpconf->match_subject,
                   dpconf->match_keyword,
                   //dpconf->Activate_condition,
                   con_addr,con_sub,con_cont,
                   dpconf->mailPeriod);

//DEBUGMSG(1,"%s [mail_wakeupHtm_mat length=%d, buf length=%d, ETH_PAYLOAD_LEN=%d]\n", __func__, strlen(mail_wakeup_matHtm), strlen(buf), ETH_PAYLOAD_LEN);
//DEBUGMSG(1,"%s [%s]\n", __func__, buf);

    offset += rv;
    return offset;
}
#endif
//mail_wakeup 20121025 eccheng
//const static char *webPages[] =
//{
//  0, rdcss, logo, bannerHtm, homeHtm, welcomeHtm, infoHtm, 0, 0, 0, securityHtm/*10*/, netSetIPv4Htm, netSetIPv6Htm, 0 ,0, authHtm, authFailHtm, okPage, ezShareHtm, mail_wakeup_accHtm, mail_wakeup_matHtm, 0/*bin*/
//};

//description : Change a decimal string to unsigned int
unsigned long long atoull(unsigned char *deciStr){
	int len =0,idx=0;
	unsigned long long value = 0;

	value = 0;
	len = strlen(deciStr);
	//printf("[RTK] len = %d\n",len);
	for(idx=0;idx<len;idx++){
		value  *=10;
		if((deciStr[idx]<='9')&&(deciStr[idx]>='0')){
			value += deciStr[idx]-'0';
			//printf("[RTK] value = %u,(%d)\n",value,(deciStr[idx]-'0'));
		}else{
			//printf("[RTK] XX[%c] \n",deciStr[idx]);
			return 0;
		}		
	}
	//printf("[RTK] value = %llu , (%d)B\n",value,value/(1024));
	
	return value ;
}

void parseBytesToSizeString(unsigned long long ibytes,char str[],int len){
	
	memset(str,0x00,len);	
	
	if(ibytes>1024*1024){
		sprintf(str,"%d MB",ibytes/(1024*1024));
	}else if(ibytes>1024){
		sprintf(str,"%d KB",ibytes/1024);
	}else{
		sprintf(str,"%d B",ibytes);
	}
		
}


int  genWelcomePage(struct tcp_pcb *pcb,char* buf){
	
	 int offset = 0;
	 int rv = 0;
	
	 rv = sprintf(buf ,welcomeHtm , s_welstr , s_gigaEthControl , s_dashtech);  
	offset+= rv;		
	return offset;					
}

int  genBannerPage(struct tcp_pcb *pcb,char* buf){
	
	 int offset = 0;
	 int rv = 0;
	
	 rv = sprintf(buf ,bannerHtm ,s_realtekCop);         
	offset+= rv;		
	return offset;					
}

void sendWebPagePkt(struct tcp_pcb *pcb,int rv,char* buf ){
	PKT	 *pkt = NULL;
	int diff=0 , newLen;

	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{	
		
		 pkt = (PKT*)allocPkt_F(rv );
		 
		 if(pkt != NULL){
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
		 }else{
			//no enough buffer 
			if( (rv%2)  == 1 ){
				diff = 1;
			}
			if((rv/2) > 0){
				newLen = rv/2;
				sendWebPagePkt(pcb , newLen , buf );
				sendWebPagePkt(pcb , newLen + diff , buf +newLen);	
			}
		 }
	}
	
}

int  genInfoPage(struct tcp_pcb *pcb,char* buf){
	
	 int offset = 0,rv = 0;
	int count = 0, length = 0,idx=0;
	char *strbuff = NULL,*pStr=NULL;	
	PKT	 *pkt = NULL;

	length= 0;	

	//--------------------------------------------------
	//caculate total length
	length  += sprintf(buf ,infoHtm_head);       
	length  += sprintf(buf ,infoHtm_body1,s_computerSys,s_opSystem,s_softwIdentity,
		#ifndef CONFIG_WAKE_UP_PATTERN_ENABLED
		s_systemInfo,s_bios,s_cpu,s_memDevice,s_fan,s_sensorstring,s_asset,s_remoteControl,s_eventLog,
		#else
		"Patern Setting",
		#endif
		s_networkSetting,s_serviceSetting);
	length  += sprintf(buf ,infoHtm_body2,
		#ifdef CONFIG_MAIL_WAKEUP_ENABLED
		s_mailWakeup,s_mailAccount,s_matchCondition,
		#endif
		s_about);         

	//--------------------------------------------------

	rv = sprintf(buf, headerHtm, length);
	rv += sprintf(buf+rv, infoHtm_head);	

	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{
	
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
		
		
	 
	rv  = sprintf(buf ,infoHtm_body1,s_computerSys,s_opSystem,s_softwIdentity,
		#ifndef CONFIG_WAKE_UP_PATTERN_ENABLED
		s_systemInfo,s_bios,s_cpu,s_memDevice,s_fan,s_sensorstring,s_asset,s_remoteControl,s_eventLog,
		#else
		"Patern Setting",
		#endif
		s_networkSetting,s_serviceSetting);

	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{	
		pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
	
	rv  =  sprintf(buf ,infoHtm_body2,
		#ifdef CONFIG_MAIL_WAKEUP_ENABLED
		s_mailWakeup,s_mailAccount,s_matchCondition,
		#endif
		s_about);     

	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{	
		pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);	
	}*/
	


	return 0;			
}

int genOperationSystemPage(char* buf)
{
	 int offset = 0;
	 int rv = 0;
	 char *strbuff = NULL,*pStr=NULL;
	 int idx = 0,len=0;

	UpdateOperationSysCIM();

	 rv = sprintf(buf ,operationSysHtm,getOperationSysProperty("Name")	 
	 	,getOperationSysProperty("OSType")
	 	,getOperationSysProperty("EnabledState")
	 	,getOperationSysProperty("RequestedState")
	 	,getOperationSysProperty("AvailableRequestedStates")
	 	,getOperationSysProperty("TransitioningToState"));           //Support Power Capability


	offset+= rv;

	releaseOperationSysCIM();
		
	return offset;						
}
int  genPROCESSORPage(struct tcp_pcb *pcb,char* buf){
	int rv = 0;
	int length = 0,idx=0;
	PKT	 *pkt = NULL;
	int count = 0;

	UpdateProcessorCIM();
	count = getProcessorInstanceCount();	
	//-------------------------------------
	//caculate the total length
	length = 0;
	length += sprintf(buf, processorHtm_h );
	for(idx =0;idx<count;idx++){
		length += sprintf(buf, processorHtm_table , getCommonProperty("ElementName" , idx,"CIM_PROCESSOR"),
									getCommonProperty("DeviceID" , idx,"CIM_PROCESSOR"),
									getCommonProperty("Stepping" , idx,"CIM_PROCESSOR"),
									getCommonProperty("CPUStatus" , idx,"CIM_PROCESSOR"),
									getCommonProperty("HealthState" , idx,"CIM_PROCESSOR"),
									getCommonProperty("CurrentClockSpeed" , idx,"CIM_PROCESSOR"),
									getCommonProperty("MaxClockSpeed" , idx,"CIM_PROCESSOR"),
									getCommonProperty("ExternalBusClockSpeed" , idx,"CIM_PROCESSOR"),
									getCommonProperty("LoadPercentage" , idx,"CIM_PROCESSOR"),
									getCommonProperty("EnabledState" , idx,"CIM_PROCESSOR"),
									getCommonProperty("RequestedState" , idx,"CIM_PROCESSOR"),
									getCommonProperty("OperationalStatus" , idx,"CIM_PROCESSOR"));
	}
	length += sprintf(buf, normal_Htm_tail );
	
	//-------------------------------------
	rv = sprintf(buf, headerHtm, length);
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
		
	rv = sprintf(buf, processorHtm_h );
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/


	for(idx =0;idx<count;idx++){
		rv = sprintf(buf, processorHtm_table , getCommonProperty("ElementName" , idx,"CIM_PROCESSOR"),
									getCommonProperty("DeviceID" , idx,"CIM_PROCESSOR"),
									getCommonProperty("Stepping" , idx,"CIM_PROCESSOR"),
									getCommonProperty("CPUStatus" , idx,"CIM_PROCESSOR"),
									getCommonProperty("HealthState" , idx,"CIM_PROCESSOR"),
									getCommonProperty("CurrentClockSpeed" , idx,"CIM_PROCESSOR"),
									getCommonProperty("MaxClockSpeed" , idx,"CIM_PROCESSOR"),
									getCommonProperty("ExternalBusClockSpeed" , idx,"CIM_PROCESSOR"),
									getCommonProperty("LoadPercentage" , idx,"CIM_PROCESSOR"),
									getCommonProperty("EnabledState" , idx,"CIM_PROCESSOR"),
									getCommonProperty("RequestedState" , idx,"CIM_PROCESSOR"),
									getCommonProperty("OperationalStatus" , idx,"CIM_PROCESSOR"));
		sendWebPagePkt(pcb,rv,buf);
		/*
		if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
		}else{
			 pkt = (PKT*)allocPkt_F(rv );
		        memcpy(pkt->wp, buf, rv );
		        tcpSendData_F(pcb, pkt);
		}*/
	}

		
	 rv = sprintf(buf, normal_Htm_tail );
	 sendWebPagePkt(pcb,rv,buf);
	 /*
	 if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);		
	}*/
	//------------------------
	releaseProcessorCIM();
	
}

int  genMemoryPage(struct tcp_pcb *pcb,char* buf){
	int rv = 0;
	int length = 0,idx=0;
	int count = 0;
	unsigned int blockSize = 0;
	unsigned long long numBlk1=0,numBlk2=0;
	PKT	 *pkt = NULL;
	unsigned char tmpstr2[32],tmpstr1[32];

	UpdateMemoryCIM();	
	count = getMemInstanceCount();	
	//-------------------------------------
	//caculate the total length
	length = 0;
	length += sprintf(buf, memoryHtm_h );
	for(idx =0;idx<count;idx++){
		if(idx == 0){
			length += sprintf(buf, "<table cellspacing=0>\n");
		}
		//----
		memset(tmpstr1,0x00,32);
		memset(tmpstr2,0x00,32);
		blockSize = atoull(getCommonProperty("BlockSize" , idx,"CIM_MEMORY"));
		numBlk1 = atoull(getCommonProperty("NumberOfBlocks" , idx,"CIM_MEMORY"));
		numBlk2 = atoull(getCommonProperty("ConsumableBlocks" , idx,"CIM_MEMORY"));

		//printf("[RTK]NumberOfBlocks=(%s)\n",getCommonProperty("NumberOfBlocks" , idx,"CIM_MEMORY"));
		//printf("[RTK]ConsumableBlocks=(%s)\n",getCommonProperty("ConsumableBlocks" , idx,"CIM_MEMORY"));
		
		
		parseBytesToSizeString(numBlk1,tmpstr1,32);
		parseBytesToSizeString(numBlk2,tmpstr2,32);
		/*
		if(numBlk1>1024*1024){
			sprintf(tmpstr1,"%d MB",numBlk1/(1024*1024));
		}else if(numBlk1>1024){
			sprintf(tmpstr1,"%d KB",numBlk1/1024);
		}else{
			sprintf(tmpstr1,"%d B",numBlk1);
		}	
		if(numBlk2>1024*1024){
			sprintf(tmpstr2,"%d MB",numBlk2/(1024*1024));
		}else if(numBlk1>1024){
			sprintf(tmpstr2,"%d KB",numBlk2/1024);
		}else{
			sprintf(tmpstr2,"%d B",numBlk2);
		}*/	
		//---
		length += sprintf(buf, memoryHtm_table , getCommonProperty("ElementName" , idx,"CIM_MEMORY"),
									getCommonProperty("DeviceID" , idx,"CIM_MEMORY"),
									getCommonProperty("HealthState" , idx,"CIM_MEMORY"),
									getCommonProperty("EnabledState" , idx,"CIM_MEMORY"),
									getCommonProperty("RequestedState" , idx,"CIM_MEMORY"),
									getCommonProperty("Access" , idx,"CIM_MEMORY"),
									tmpstr1,
									tmpstr2,
									getCommonProperty("Volatile" , idx,"CIM_MEMORY"));
		if(idx == (count-1)){
			length += sprintf(buf, "</table>\n");
			
		}
	}
	length += sprintf(buf, normal_Htm_tail );
	
	//-------------------------------------
	rv = sprintf(buf, headerHtm, length);
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
		
	rv = sprintf(buf, memoryHtm_h );
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/


	for(idx =0;idx<count;idx++){
		rv = 0;
		if(idx == 0){			
			rv = sprintf(buf, "<table cellspacing=0>\n");
			
		}
		//----
		memset(tmpstr1,0x00,32);
		memset(tmpstr2,0x00,32);
		blockSize = atoull(getCommonProperty("BlockSize" , idx,"CIM_MEMORY"));
		numBlk1 = atoull(getCommonProperty("NumberOfBlocks" , idx,"CIM_MEMORY"));
		numBlk2 = atoull(getCommonProperty("ConsumableBlocks" , idx,"CIM_MEMORY"));
		parseBytesToSizeString(numBlk1,tmpstr1,32);
		parseBytesToSizeString(numBlk2,tmpstr2,32);

		//printf("[RTK](%d,%d,%d)\n",blockSize,numBlk1,numBlk2);
		
		/*
		if(numBlk1>1024*1024){
			sprintf(tmpstr1,"%d MB",numBlk1/(1024*1024));
		}else if(numBlk1>1024){
			sprintf(tmpstr1,"%d KB",numBlk1/1024);
		}else{
			sprintf(tmpstr1,"%d B",numBlk1);
		}	
		if(numBlk2>1024*1024){
			sprintf(tmpstr2,"%d MB",numBlk2/(1024*1024));
		}else if(numBlk1>1024){
			sprintf(tmpstr2,"%d KB",numBlk2/1024);
		}else{
			sprintf(tmpstr2,"%d B",numBlk2);
		}*/	
		//---
		rv += sprintf(buf+rv, memoryHtm_table , getCommonProperty("ElementName" , idx,"CIM_MEMORY"),
									getCommonProperty("DeviceID" , idx,"CIM_MEMORY"),
									getCommonProperty("HealthState" , idx,"CIM_MEMORY"),
									getCommonProperty("EnabledState" , idx,"CIM_MEMORY"),
									getCommonProperty("RequestedState" , idx,"CIM_MEMORY"),
									getCommonProperty("Access" , idx,"CIM_MEMORY"),
									tmpstr1,
									tmpstr2,
									getCommonProperty("Volatile" , idx,"CIM_MEMORY"));

		if(idx == (count-1)){
			rv += sprintf(buf+rv, "</table>\n");
			
		}
		sendWebPagePkt(pcb,rv,buf);
		/*
		if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
		}else{
			 pkt = (PKT*)allocPkt_F(rv );
		        memcpy(pkt->wp, buf, rv );
		        tcpSendData_F(pcb, pkt);
		}*/
	}

		
	 rv = sprintf(buf, normal_Htm_tail );
	 sendWebPagePkt(pcb,rv,buf);
	 /*
	 if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);		
	}*/
	//------------------------

	


	releaseMemoryCIM();
	
}
int  genFanPage(struct tcp_pcb *pcb,char* buf){
	int offset = 0,rv = 0;
	int count = 0, length = 0,idx=0;
	char *strbuff = NULL,*pStr=NULL;	
	PKT	 *pkt = NULL;

	UpdateFanCIM();
	count = getFanInstanceCount();
	length= 0;

		
	 length += sprintf(buf ,fanHtm_h);           
	
	for(idx=0;idx<count;idx++){
		
		length += sprintf(buf, fanHtm_table , getCommonProperty("ElementName" , idx,"CIM_FAN"),
								getCommonProperty("DeviceID" , idx,"CIM_FAN"),
								getCommonProperty("HealthState" , idx,"CIM_FAN"),
								getCommonProperty("VariableSpeed" , idx,"CIM_FAN"),
								getCommonProperty("DesiredSpeed" , idx,"CIM_FAN"),
								getCommonProperty("ActiveCooling" , idx,"CIM_FAN"),
								getCommonProperty("EnabledState" , idx,"CIM_FAN"),
								getCommonProperty("RequestedState" , idx,"CIM_FAN"),
								getCommonProperty("OperationalStatus" , idx,"CIM_FAN"));
	}
	
	length += sprintf(buf, "</table>\n");
	length += sprintf(buf ,normal_Htm_tail);           //Support Power Capability	

	//--------------------------------------------------

	rv = sprintf(buf, headerHtm, length);
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{	
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
		
	 
	rv  = sprintf(buf ,fanHtm_h);         
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/

	rv = 0;
	for(idx=0;idx<count;idx++){
		
		rv = sprintf(buf, fanHtm_table , getCommonProperty("ElementName" , idx,"CIM_FAN"),
								getCommonProperty("DeviceID" , idx,"CIM_FAN"),
								getCommonProperty("HealthState" , idx,"CIM_FAN"),
								getCommonProperty("VariableSpeed" , idx,"CIM_FAN"),
								getCommonProperty("DesiredSpeed" , idx,"CIM_FAN"),
								getCommonProperty("ActiveCooling" , idx,"CIM_FAN"),
								getCommonProperty("EnabledState" , idx,"CIM_FAN"),
								getCommonProperty("RequestedState" , idx,"CIM_FAN"),
								getCommonProperty("OperationalStatus" , idx,"CIM_FAN"));
		sendWebPagePkt(pcb,rv,buf);
		/*
		if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
		}else{
			pkt = (PKT*)allocPkt_F(rv );
		        memcpy(pkt->wp, buf, rv );
		        tcpSendData_F(pcb, pkt);
		}*/
	}
	
	rv = 0;
	rv += sprintf(buf+rv, "</table>\n");
	rv += sprintf(buf+rv,normal_Htm_tail); 
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);	
	}*/		


	releaseFanCIM();
	return 0;
}

int  genSensorPage(struct tcp_pcb *pcb,char* buf){
	int offset = 0,rv = 0;
	int count = 0, length = 0,idx=0;
	char *strbuff = NULL,*pStr=NULL;	
	PKT	 *pkt = NULL;

	UpdateSensorCIM();
	count = getSensorInstanceCount();
	length= 0;

		
	 length += sprintf(buf ,sensorHtm_h);           
	
	for(idx=0;idx<count;idx++){
		
		length += sprintf(buf, sensorHtm_table , getCommonProperty("ElementName" , idx,"CIM_SENSOR"),
								getCommonProperty("DeviceID" , idx,"CIM_SENSOR"),
								getCommonProperty("SensorType" , idx,"CIM_SENSOR"),
								getCommonProperty("PossibleStates" , idx,"CIM_SENSOR"),
								getCommonProperty("CurrentState" , idx,"CIM_SENSOR"),
								getCommonProperty("HealthState" , idx,"CIM_SENSOR"),
								getCommonProperty("EnabledState" , idx,"CIM_SENSOR"),
								getCommonProperty("RequestedState" , idx,"CIM_SENSOR"),
								getCommonProperty("OperatingStatus" , idx,"CIM_SENSOR"));

	}
	
	length += sprintf(buf, "</table>\n");
	length += sprintf(buf ,normal_Htm_tail);           //Support Power Capability	

	//--------------------------------------------------

	rv = sprintf(buf, headerHtm, length);
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
		
	 
	rv  = sprintf(buf ,sensorHtm_h);     
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/

	rv = 0;
	for(idx=0;idx<count;idx++){
		printf("[RTK] ready\n");
		
		rv = sprintf(buf, sensorHtm_table , getCommonProperty("ElementName" , idx,"CIM_SENSOR"),
								getCommonProperty("DeviceID" , idx,"CIM_SENSOR"),
								getCommonProperty("SensorType" , idx,"CIM_SENSOR"),
								getCommonProperty("PossibleStates" , idx,"CIM_SENSOR"),
								getCommonProperty("CurrentState" , idx,"CIM_SENSOR"),
								getCommonProperty("HealthState" , idx,"CIM_SENSOR"),
								getCommonProperty("EnabledState" , idx,"CIM_SENSOR"),
								getCommonProperty("RequestedState" , idx,"CIM_SENSOR"),
								getCommonProperty("OperationalStatus" , idx,"CIM_SENSOR"));
		sendWebPagePkt(pcb,rv,buf);
		/*
		if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
		}else{
			pkt = (PKT*)allocPkt_F(rv );
		        memcpy(pkt->wp, buf, rv );
		        tcpSendData_F(pcb, pkt);
		}*/
	}
	
	rv = 0;
	rv += sprintf(buf+rv, "</table>\n");
	rv += sprintf(buf+rv,normal_Htm_tail); 
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);		
	}*/


	releaseSensorCIM();
	return 0;
}

int  genAssetPage(struct tcp_pcb *pcb,char* buf){
	int offset = 0,rv = 0;
	int count = 0, length = 0,idx=0;
	char *strbuff = NULL,*pStr=NULL;	
	PKT	 *pkt = NULL;
	unsigned long long llBytes = 0;
	char capbility[32];

	//----------------------------------------------------------------------------------
	 length= 0;		
	 length += sprintf(buf ,assetHtm_h);           
	//card--------------------------------------------------	
	count = UpdateCommonCIM("CIM_Card");
	length += sprintf(buf, "<H2>Card</H2><table cellspacing=0>\n");
	for(idx=0;idx<count;idx++){
		
		length += sprintf(buf, cardHtm_table , getCommonProperty("ElementName" , idx,"CIM_Card"),
								getCommonProperty("Manufacturer" , idx,"CIM_Card"),
								getCommonProperty("Model" , idx,"CIM_Card"),
								getCommonProperty("SerialNumber" , idx,"CIM_Card"),
								getCommonProperty("PartNumber" , idx,"CIM_Card"),
								getCommonProperty("SKU" , idx,"CIM_Card"),
								getCommonProperty("Tag" , idx,"CIM_Card"));
	}

	length += sprintf(buf, "</table>\n");
	releaseCommonCIM("CIM_Card");	        
	//chassis--------------------------------------------------	
	count = UpdateCommonCIM("CIM_Chassis");
	length += sprintf(buf, "<H2>Chassis</H2><table cellspacing=0>\n");
	for(idx=0;idx<count;idx++){
		
		length += sprintf(buf, chassisHtm_table , getCommonProperty("ElementName" , idx,"CIM_Chassis"),
								getCommonProperty("Manufacturer" , idx,"CIM_Chassis"),
								getCommonProperty("Model" , idx,"CIM_Chassis"),
								getCommonProperty("SerialNumber" , idx,"CIM_Chassis"),
								getCommonProperty("PartNumber" , idx,"CIM_Chassis"),
								getCommonProperty("SKU" , idx,"CIM_Chassis"),
								getCommonProperty("Tag" , idx,"CIM_Chassis"),
								getCommonProperty("Version" , idx,"CIM_Chassis"));
	}
	
	length += sprintf(buf, "</table>\n");
	releaseCommonCIM("CIM_Chassis");
	//chip--------------------------------------------------	
	count = UpdateCommonCIM("CIM_Chip");
	length += sprintf(buf, "<H2>Chip(Processor)</H2><table cellspacing=0>\n");
	for(idx=0;idx<count;idx++){
		//[Note]!! the cardHtm_table is the same as Chip Table. !!	
		length += sprintf(buf, cardHtm_table , getCommonProperty("ElementName" , idx,"CIM_Chip"),
								getCommonProperty("Manufacturer" , idx,"CIM_Chip"),
								getCommonProperty("Model" , idx,"CIM_Chip"),
								getCommonProperty("SerialNumber" , idx,"CIM_Chip"),
								getCommonProperty("PartNumber" , idx,"CIM_Chip"),
								getCommonProperty("SKU" , idx,"CIM_Chip"),
								getCommonProperty("Tag" , idx,"CIM_Chip"));
	}
	
	length += sprintf(buf, "</table>\n");
	releaseCommonCIM("CIM_Chip");
	//CIM_PhysicalMemory--------------------------------------------------	
	count = UpdateCommonCIM("CIM_PhysicalMemory");
	
	for(idx=0;idx<count;idx++){
		//[Note]!! the cardHtm_table is the same as Chip Table. !!	
		if(idx == 0){
			length += sprintf(buf, "<H2>PhysicalMemory</H2><table cellspacing=0>\n");
		}
		//-------------------------
		llBytes = atoull(getCommonProperty("Capacity" , idx,"CIM_PhysicalMemory"));		
		parseBytesToSizeString(llBytes,capbility,32);		
		//-------------------------
		length += sprintf(buf, phyMemHtm_table , getCommonProperty("ElementName" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("Manufacturer" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("Model" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("SerialNumber" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("PartNumber" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("SKU" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("Tag" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("MemoryType" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("Speed" , idx,"CIM_PhysicalMemory"),
								//getCommonProperty("Capacity" , idx,"CIM_PhysicalMemory"),
								capbility,
								getCommonProperty("BankLabel" , idx,"CIM_PhysicalMemory"));
	}
	
	length += sprintf(buf, "</table>\n");
	releaseCommonCIM("CIM_PhysicalMemory");
	//CIM_Slot--------------------------------------------------	
	count = UpdateCommonCIM("CIM_Slot");	
	length += sprintf(buf, "<H2>Slot</H2><table cellspacing=0>\n");
	for(idx=0;idx<count;idx++){
		//[Note]!! the cardHtm_table is the same as Chip Table. !!	
		length += sprintf(buf, cardHtm_table , getCommonProperty("ElementName" , idx,"CIM_Slot"),
								getCommonProperty("Manufacturer" , idx,"CIM_Slot"),
								getCommonProperty("Model" , idx,"CIM_Slot"),
								getCommonProperty("SerialNumber" , idx,"CIM_Slot"),
								getCommonProperty("PartNumber" , idx,"CIM_Slot"),
								getCommonProperty("SKU" , idx,"CIM_Slot"),
								getCommonProperty("Tag" , idx,"CIM_Slot"));
	}
	
	length += sprintf(buf, "</table>\n");
	releaseCommonCIM("CIM_Slot");
	//--------------------------------------------------
	length += sprintf(buf ,normal_Htm_tail);    
	//--------------------------------------------------

	rv = sprintf(buf, headerHtm, length);
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
	 
	rv  = sprintf(buf ,assetHtm_h);         
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
	//--------------------------------------------------
	rv = 0;
	//UpdateCardCIM();
	count = UpdateCommonCIM("CIM_Card");
	rv += sprintf(buf, "<H2>Card</H2><table cellspacing=0>\n");
	for(idx=0;idx<count;idx++){
		
		rv += sprintf(buf+rv, cardHtm_table , getCommonProperty("ElementName" , idx,"CIM_Card"),
								getCommonProperty("Manufacturer" , idx,"CIM_Card"),
								getCommonProperty("Model" , idx,"CIM_Card"),
								getCommonProperty("SerialNumber" , idx,"CIM_Card"),
								getCommonProperty("PartNumber" , idx,"CIM_Card"),
								getCommonProperty("SKU" , idx,"CIM_Card"),
								getCommonProperty("Tag" , idx,"CIM_Card"));
		if(idx == (count-1)){
			rv += sprintf(buf+rv, "</table>\n");
		}
		sendWebPagePkt(pcb,rv,buf);
		/*
		if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
		}else{
			pkt = (PKT*)allocPkt_F(rv );
		        memcpy(pkt->wp, buf, rv );
		        tcpSendData_F(pcb, pkt);
		}*/
		rv = 0;	
	}
	releaseCommonCIM("CIM_Card");
	//chassis--------------------------------------------------	
	rv = 0;
	count = UpdateCommonCIM("CIM_Chassis");
	rv += sprintf(buf, "<H2>Chassis</H2><table cellspacing=0>\n");
	for(idx=0;idx<count;idx++){
		
		rv += sprintf(buf+rv, chassisHtm_table , getCommonProperty("ElementName" , idx,"CIM_Chassis"),
								getCommonProperty("Manufacturer" , idx,"CIM_Chassis"),
								getCommonProperty("Model" , idx,"CIM_Chassis"),
								getCommonProperty("SerialNumber" , idx,"CIM_Chassis"),
								getCommonProperty("PartNumber" , idx,"CIM_Chassis"),
								getCommonProperty("SKU" , idx,"CIM_Chassis"),
								getCommonProperty("Tag" , idx,"CIM_Chassis"),
								getCommonProperty("Version" , idx,"CIM_Chassis"));
		if(idx == (count-1)){
			rv += sprintf(buf+rv, "</table>\n");
		}
		sendWebPagePkt(pcb,rv,buf);
		/*
		if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
		}else{
			pkt = (PKT*)allocPkt_F(rv );
		        memcpy(pkt->wp, buf, rv );
		        tcpSendData_F(pcb, pkt);
		}*/
		rv = 0;	
	}	
	//releaseChassisCIM();       
	releaseCommonCIM("CIM_Chassis");
	//chassis--------------------------------------------------	
	rv = 0;
	count = UpdateCommonCIM("CIM_Chip");
	rv += sprintf(buf, "<H2>Chip(Processor)</H2><table cellspacing=0>\n");
	for(idx=0;idx<count;idx++){
		
		rv += sprintf(buf+rv, cardHtm_table , getCommonProperty("ElementName" , idx,"CIM_Chip"),
								getCommonProperty("Manufacturer" , idx,"CIM_Chip"),
								getCommonProperty("Model" , idx,"CIM_Chip"),
								getCommonProperty("SerialNumber" , idx,"CIM_Chip"),
								getCommonProperty("PartNumber" , idx,"CIM_Chip"),
								getCommonProperty("SKU" , idx,"CIM_Chip"),
								getCommonProperty("Tag" , idx,"CIM_Chip"));
		if(idx == (count-1)){
			rv += sprintf(buf+rv, "</table>\n");
		}
		sendWebPagePkt(pcb,rv,buf);
		/*
		if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
		}else{
			pkt = (PKT*)allocPkt_F(rv );
		        memcpy(pkt->wp, buf, rv );
		        tcpSendData_F(pcb, pkt);
		}*/
		rv = 0;
	}	
	//releaseChassisCIM();       
	releaseCommonCIM("CIM_Chip");
	//Physical Memory--------------------------------------------------	
	rv = 0;
	count = UpdateCommonCIM("CIM_PhysicalMemory");	
	for(idx=0;idx<count;idx++){
		if(idx == 0){
			rv += sprintf(buf, "<H2>PhysicalMemory</H2><table cellspacing=0>\n");
		}
		//-------------------------
		llBytes = atoull(getCommonProperty("Capacity" , idx,"CIM_PhysicalMemory"));
		parseBytesToSizeString(llBytes,capbility,32);		
		//-------------------------
		rv += sprintf(buf+rv, phyMemHtm_table , getCommonProperty("ElementName" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("Manufacturer" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("Model" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("SerialNumber" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("PartNumber" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("SKU" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("Tag" , idx,"CIM_PhysicalMemory"),								
								getCommonProperty("MemoryType" , idx,"CIM_PhysicalMemory"),
								getCommonProperty("Speed" , idx,"CIM_PhysicalMemory"),
								//getCommonProperty("Capacity" , idx,"CIM_PhysicalMemory"),
								capbility,
								getCommonProperty("BankLabel" , idx,"CIM_PhysicalMemory"));
		if(idx == (count-1)){
			rv += sprintf(buf+rv, "</table>\n");
		}
		sendWebPagePkt(pcb,rv,buf);
		/*
		if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
		}else{
			pkt = (PKT*)allocPkt_F(rv );
		        memcpy(pkt->wp, buf, rv );
		        tcpSendData_F(pcb, pkt);
		}*/
		rv = 0;
	}	
	//releaseChassisCIM();       
	releaseCommonCIM("CIM_PhysicalMemory");
	//Slot--------------------------------------------------	
	rv = 0;
	count = UpdateCommonCIM("CIM_Slot");
	rv += sprintf(buf, "<H2>Slot</H2><table cellspacing=0>\n");
	for(idx=0;idx<count;idx++){
		
		rv += sprintf(buf+rv, cardHtm_table , getCommonProperty("ElementName" , idx,"CIM_Slot"),
								getCommonProperty("Manufacturer" , idx,"CIM_Slot"),
								getCommonProperty("Model" , idx,"CIM_Slot"),
								getCommonProperty("SerialNumber" , idx,"CIM_Slot"),
								getCommonProperty("PartNumber" , idx,"CIM_Slot"),
								getCommonProperty("SKU" , idx,"CIM_Slot"),
								getCommonProperty("Tag" , idx,"CIM_Slot"));
		if(idx == (count-1)){
			rv += sprintf(buf+rv, "</table>\n");
		}
		sendWebPagePkt(pcb,rv,buf);
		/*
		if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
		}else{
			pkt = (PKT*)allocPkt_F(rv );
		        memcpy(pkt->wp, buf, rv );
		        tcpSendData_F(pcb, pkt);
			}*/
		rv = 0;
	}    
	releaseCommonCIM("CIM_Slot");
	//--------------------------------------------------

	
	rv = 0;
	
	rv += sprintf(buf+rv,normal_Htm_tail); 
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
	
	return 0;
}


int genBIOSPage(struct tcp_pcb *pcb,char* buf){
	int length = 0;
	int rv = 0;
	char *propertyBuf = NULL;
	PKT	 *pkt = NULL;
	int eleQuantity = 0,attrQuantity = 0,idx=0;

	UpdateBIOSElementCIM();	
	eleQuantity = getbiosEleInstanceCount();
	//------------------------------------------
	//caculate the html file length
	length = 0;
	length+= sprintf(buf, biosHtm_h );	
	length+= sprintf(buf,"<table cellspacing=0>\n");	
	for(idx=0;idx<eleQuantity;idx++){
		length+= sprintf(buf, biosHtm , getBIOSElemProperty("Name",idx)  
			 						, getBIOSElemProperty("Manufacturer",idx)
			 						, getBIOSElemProperty("Version",idx)
			 						, getBIOSElemProperty("PrimaryBIOS",idx)
			 						, getBIOSElemProperty("SoftwareElementID",idx)
			 						, getBIOSElemProperty("SoftwareElementState",idx)
		 							, getBIOSElemProperty("TargetOperatingSystem",idx));
	}
	releaseBIOSElementCIM();		
	length+= sprintf(buf,"</table><hr>\n\n");	
	length += sprintf(buf, "<table cellspacing=0>\n<th><br><I><FONT COLOR=#0000BB>AttributeName</font></br></th><th width=10></th><th><br><I><FONT COLOR=#0000BB>CurrentValue</br></th><th width=100></th><tr>\n" );
	UpdateBIOSAttrCIM();
	attrQuantity = getbiosAttrInstanceCount();
	for(idx=0;idx<attrQuantity;idx++){
		if(idx%2==0){
			length+= sprintf(buf, "<td>%s</td><td width=10></td><td>%s</td><td width=100></td><tr>\n",getBIOSAttrProperty("AttributeName" ,idx),getBIOSAttrProperty("CurrentValue" ,idx) );
		}else{
			length+= sprintf(buf, "<th>%s</th><th width=10></th><th>%s</th><th width=100></th><tr>\n",getBIOSAttrProperty("AttributeName" ,idx) ,getBIOSAttrProperty("CurrentValue" ,idx));
		}
	}
	length+= sprintf(buf, "</table><hr>\n" );	
	length+= sprintf(buf, biosHtm_t );	
	releaseBIOSAttrCIM();		
	//------------------------------------------
	rv = sprintf(buf, headerHtm, length);
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{	
	 	pkt = (PKT*)allocPkt_F(rv );
        		memcpy(pkt->wp, buf, rv );
        		tcpSendData_F(pcb, pkt);
	}*/
	//------	
	rv = sprintf(buf, biosHtm_h );
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{			
	     pkt = (PKT*)allocPkt_F(rv );
	      memcpy(pkt->wp, buf, rv );
	      tcpSendData_F(pcb, pkt);
	}*/
	  
	//------------------------------------------------
	UpdateBIOSElementCIM();
	eleQuantity = getbiosEleInstanceCount();
	rv = 0;
	rv += sprintf(buf,"<table cellspacing=0>\n");
	for(idx=0;idx<eleQuantity;idx++){
		rv += sprintf(buf+rv, biosHtm , getBIOSElemProperty("Name",idx)  
			 						, getBIOSElemProperty("Manufacturer",idx)
			 						, getBIOSElemProperty("Version",idx)
			 						, getBIOSElemProperty("PrimaryBIOS",idx)
			 						, getBIOSElemProperty("SoftwareElementID",idx)
			 						, getBIOSElemProperty("SoftwareElementState",idx)
		 							, getBIOSElemProperty("TargetOperatingSystem",idx));
		//rv += sprintf(buf+rv,"<hr>" );
		
	}
	rv+= sprintf(buf+rv,"</table><hr>\n\n");
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{
		pkt = (PKT*)allocPkt_F(rv );
	      	memcpy(pkt->wp, buf, rv );
	      	tcpSendData_F(pcb, pkt);
	}*/
	releaseBIOSElementCIM();
	//------------------------------------------------	
	rv = 0;
	rv+= sprintf(buf, "<table cellspacing=0>\n<th><br><I><FONT COLOR=#0000BB>AttributeName</font></br></th><th width=10></th><th><br><I><FONT COLOR=#0000BB>CurrentValue</br></th><th width=100></th><tr>\n" );
	UpdateBIOSAttrCIM();
	attrQuantity = getbiosAttrInstanceCount();
	for(idx=0;idx<attrQuantity;idx++){
		if(idx%2==0){
			rv+= sprintf(buf+rv, "<td>%s</td><td width=10></td><td>%s</td><td width=100></td><tr>\n",getBIOSAttrProperty("AttributeName" ,idx),getBIOSAttrProperty("CurrentValue" ,idx) );
		}else{
			rv+= sprintf(buf+rv, "<th>%s</th><th width=10></th><th>%s</th><th width=100></th><tr>\n",getBIOSAttrProperty("AttributeName" ,idx),getBIOSAttrProperty("CurrentValue" ,idx));
		}
	}
	rv+= sprintf(buf+rv, "</table><hr>\n" );
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{
		pkt = (PKT*)allocPkt_F(rv );
      		memcpy(pkt->wp, buf, rv );
      		tcpSendData_F(pcb, pkt);	
	}*/
	releaseBIOSAttrCIM();
	//------
	rv = sprintf(buf, biosHtm_t );
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{
		pkt = (PKT*)allocPkt_F(rv );
        		memcpy(pkt->wp, buf, rv );
        		tcpSendData_F(pcb, pkt);
	}*/	
	//------------------------
	
}

int genSoftwareIdentityPage(struct tcp_pcb *pcb,char* buf){
	int length = 0;
	int rv = 0;
	char *propertyBuf = NULL;
	PKT	 *pkt = NULL;
	int swQuantity = 0,idx=0;
	//propertyBuf = malloc(1024);

	UpdateSoftwareIdCIM();

	swQuantity = getSwIdInstanceCount();
		
	length+= sprintf(buf, softwareIdentiyHtm_h );


	for(idx=0;idx<swQuantity;idx++){
		length+= sprintf(buf, softwareIdPropertyHtm , getCommonProperty("InstanceID",idx,"CIM_SoftwareIdentity")  
		 									, getCommonProperty("IsEntity",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("VersionString",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("MajorVersion",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("MinorVersion",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("RevisionNumber",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("BuildNumber",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("IdentityInfoType",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("IdentityInfoValue",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("Classifications",idx,"CIM_SoftwareIdentity"));
	
	}
	
	length+= sprintf(buf, softwareIdentiyHtm_t );
	//-------------------------------------------------------------------------------
	rv = sprintf(buf, headerHtm, length);
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
	//-------------
	rv = sprintf(buf, softwareIdentiyHtm_h );
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
	//-------------
	for(idx=0;idx<swQuantity;idx++){
		rv = sprintf(buf, softwareIdPropertyHtm , getCommonProperty("InstanceID",idx,"CIM_SoftwareIdentity")  
		 									, getCommonProperty("IsEntity",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("VersionString",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("MajorVersion",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("MinorVersion",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("RevisionNumber",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("BuildNumber",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("IdentityInfoType",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("IdentityInfoValue",idx,"CIM_SoftwareIdentity")
		 									, getCommonProperty("Classifications",idx,"CIM_SoftwareIdentity"));
		sendWebPagePkt(pcb,rv,buf);
		/*
		if (pcb->TLSEnable){
			sendTLSWebPage(buf, rv, pcb);
		}else{
			pkt = (PKT*)allocPkt_F(rv );
				memcpy(pkt->wp, buf, rv );
			       tcpSendData_F(pcb, pkt);	
		}*/
	}
	
	
	rv = sprintf(buf, softwareIdentiyHtm_t );
	sendWebPagePkt(pcb,rv,buf);
	/*
	if (pcb->TLSEnable){
		sendTLSWebPage(buf, rv, pcb);
	}else{
		 pkt = (PKT*)allocPkt_F(rv );
	        memcpy(pkt->wp, buf, rv );
	        tcpSendData_F(pcb, pkt);
	}*/
	//-------------------------------------------------------------------------------

	

	//free(propertyBuf);
	releaseSoftwareIdCIM();
}

int genComputerSystemPage(char* buf)
{
	 int offset = 0;
	 int rv = 0;
	 char *strbuff = NULL,*pStr=NULL;
	 int idx = 0,len=0;

	
	
	
	UpdateComputerSystemCIM();
	
	strbuff = malloc(256);//256 is less the total size of powMangCapProp_powChg table, so , it is enough.
	if(strbuff == NULL){//
		rv = sprintf(buf ,computerSysHtm,getComputerSysProperty("Name") //name
						,getComputerSysProperty("PrimaryOwnerName")		//primary owener						
						,getComputerSysProperty("PrimaryOwnerContact")		//primary owener Contact 
						,getComputerSysProperty("EnabledState")			//Enable State
						,getComputerSysProperty("RequestedState")		//Requested State
						,getComputerSysProperty("Dedicated")		//Dedicated
						,getComputerSysProperty("IdentifyingDescriptions")		//IdentifyingDescriptions
						,getComputerSysProperty("OtherIdentifyingInfo")		//IdentifyingDescriptions
						,getPowerMangSrvProperty("PowerState")		//Power State	
						,"memory error");           //Support Power Capability
	}else{
	
		memset(strbuff,0x00,256);
		idx = 0;
		pStr = strbuff;
		while(idx>=0){
			idx++;
			len= sprintf(pStr,"%s<BR>" , getPowerMangCapProperty("PowerChangeCapabilities",&idx));						
			pStr+=len;		
		}
	
		#if 1
		
		rv = sprintf(buf ,computerSysHtm,getComputerSysProperty("Name") //name
							,getComputerSysProperty("PrimaryOwnerName")		//primary owener						
							,getComputerSysProperty("PrimaryOwnerContact")		//primary owener Contact 
							,getComputerSysProperty("EnabledState")			//Enable State
							,getComputerSysProperty("RequestedState")		//Requested State
							,getComputerSysProperty("Dedicated")		//Dedicated
							,getComputerSysProperty("IdentifyingDescriptions")		//IdentifyingDescriptions
							,getComputerSysProperty("OtherIdentifyingInfo")		//IdentifyingDescriptions
							,getPowerMangSrvProperty("PowerState")		//Power State
							,strbuff);           //Support Power Capability	
		
		#else
		rv = sprintf(buf ,computerSysHtm, ("Name") //name
							, ("PrimaryOwnerName")		//primary owener						
							, ("PrimaryOwnerContact")		//primary owener Contact 
							, ("EnabledState")			//Enable State
							, ("RequestedState")		//Requested State
							, ("Dedicated")		//Dedicated
							, ("PowerState")		//Power State
							, ("PowerState"));           //Support Power Capability	
		#endif
		free(strbuff);
	}
	
	offset+= rv;	
	
	return offset;
}


int  genShowCounterPage(struct tcp_pcb *pcb,char* buf){
	 int offset = 0 , length = 0;
	 int counter = dpconf->counter ;


	length =   sprintf(buf , fake_notFound ,  counter);
	  

	offset = sprintf(buf, headerHtm, length);
	offset += sprintf(buf+offset, fake_notFound, counter);
	sendWebPagePkt(pcb , offset ,buf); 
	
}

int  genClearCounterPage(struct tcp_pcb *pcb,char* buf){
	 int offset = 0 , length = 0;
	 int counter = 0;

	 dpconf->counter = 0;
	setdirty(DPCONFIGTBL);
	 
	 length =   sprintf(buf , fake_notFound ,  counter);
	offset = sprintf(buf, headerHtm, length);
	offset += sprintf(buf+offset, fake_notFound, counter);
	sendWebPagePkt(pcb , offset ,buf); 
}


#if 0
const static int WebPageSize[] =
{
    0, sizeof(rdcss), sizeof(logo), sizeof(bannerHtm), sizeof(homeHtm), sizeof(welcomeHtm),  0, 0, 0, sizeof(securityHtm)/*10*/, 
    sizeof(netSetIPv4Htm), sizeof(netSetIPv6Htm), 0 ,0, sizeof(authHtm), sizeof(authFailHtm), sizeof(okPage), sizeof(ezShareHtm), 0/*bin*/
    #if CONFIG_MAIL_WAKEUP_ENABLED
	,sizeof(mail_wakeup_accHtm), sizeof(mail_wakeup_matHtm)
    #endif
    #if CONFIG_ACER_WEB_UI
    ,sizeof(AcerWBgTriGif)/*20*/, sizeof(AcerInfoABgGif), sizeof(AcerInfoBgGif), sizeof(AcerBannerBgGif), sizeof(AcerListLabelGif), sizeof(AcerBGTriangleGif)
    #endif
};
#endif

int COOKIE_ID[3];
int COOKIE_Idx = 0;
#ifdef CONFIG_MSAD_ENABLED
int addADauth(unsigned char *addr, gss_buffer_t *auth_buffer)
{
    int len = 0, minstat, tmplen;
    unsigned char *tmp;
    if ((*auth_buffer) == NULL)
        return 0;
    tmplen = (*auth_buffer)->length*2;
    tmp = (unsigned char*)malloc(tmplen);
    memset(tmp, 0, tmplen);
    b64_encode(tmp, &tmplen, (unsigned char*)(*auth_buffer)->value, tmplen/2);
    memcpy(addr + len, (char*)tmp, tmplen);
    len += tmplen;
    sprintf(addr + len, "\r\n\r\n");
    len+=4;
    gss_release_buffer(&minstat, *auth_buffer);
    free( (*auth_buffer));
    (*auth_buffer) = NULL;
    free(tmp);
    return len;
}
#endif
/*----------------------------Functions-------------------------------------*/

/**
* Description:	Check if a SMBIOS request
*
* Parameters:
*				rul ->	Request URL
*				type ->	SMBIOS type
*
* Returns:		If not a valid request return 0 else return 1
*
*/
int chkSMBIOSReqPages(char *url, int* type)
{
    if (strncmp(url,"/bios.htm", strlen(url)) == 0)
        *type = BIOS_INFO;
    else if (strncmp(url,"/sysinfo.htm", strlen(url)) == 0)
        *type = SYSTEM_INFO;
    else if (strncmp(url,"/board.htm", strlen(url)) == 0)
        *type = BASE_BOARD_INFO;
    else if (strncmp(url,"/chassis.htm", strlen(url)) == 0)
        *type = CHASSIS;
    else if (strncmp(url,"/cpu.htm", strlen(url)) == 0)
        *type = PROCESSOR;
    else if (strncmp(url,"/cache.htm", strlen(url)) == 0)
        *type = CACHE;
    else if (strncmp(url,"/port.htm.htm", strlen(url)) == 0)
        *type = PORT_CONNECTOR;
    else if (strncmp(url,"/slot.htm", strlen(url)) == 0)
        *type = SLOTS;
    else if (strncmp(url,"/onboarddev.htm", strlen(url)) == 0)
        *type = ON_BOARD_DEV;
    else if (strncmp(url,"/syslog.htm", strlen(url)) == 0)
        *type = SYS_EVENT_LOG;
    else if (strncmp(url,"/phymem.htm", strlen(url)) == 0)
        *type = PHY_MEM_ARRAY;
    else if (strncmp(url,"/memdev.htm", strlen(url)) == 0)
        *type = MEM_DEV;
    else if (strncmp(url,"/memadr.htm", strlen(url)) == 0)
        *type = MEM_ARRAY_MAP_ADDR;
    else if (strncmp(url,"/nic.htm", strlen(url)) == 0)
        return 0;
    else
        return 0;
    return 1;
}


/**
* Description:	Check if a web page request
*
* Parameters:
*
*				rul ->	Request URL
* Returns:		web page ID
*
*/
WebPageID getWebPageID(char *url)
{
#ifdef CONFIG_MSAD_ENABLED
    if (strlen(url) > 0)
    {
#endif
        if ( (strncmp(url,"/", strlen(url)) == 0) || (strncmp(url,"home.htm", strlen(url)) == 0))
        {
            return HOMEPAGE;
        }
        else if ((strncmp(url,"/rdcss.css", strlen(url)) == 0))
        {
            return RDCSS;
        }
        else if ((strncmp(url,"/logo.gif", strlen(url)) == 0))
        {
            return LOGOGIF;
        }
        else if ((strncmp(url,"/banner.htm", strlen(url)) == 0))
        {
            return BANNERPAGE;
        }
        else if ((strncmp(url,"/welcome.htm", strlen(url)) == 0))
        {
            return WELCOMEPAGE;
        }
        else if ((strncmp(url,"/info.htm", strlen(url)) == 0))
        {
            return INFOPAGE;
        }
        else if ((strncmp(url,"/remoteCtl.htm", strlen(url)) == 0))
        {
            return REMOTECTLPAGE;
        }
        else if ((strncmp(url,"/eventLog.htm", strlen(url)) == 0))
        {
            return EVENTLOGPAGE;
        }
        else if ((strncmp(url,"/bootOpt.htm", strlen(url)) == 0))
        {
            return BOOTSETPAGE;
        }
        else if ((strncmp(url,"/security.htm", strlen(url)) == 0))
        {
            return SECSETPAGE;
        }
        else if ((strncmp(url,"/netSet.htm", strlen(url)) == 0))
        {
            return NETSETPAGE;
        }
        else if ((strncmp(url,"/netSetv6.htm", strlen(url)) == 0))
        {
            return NETSETv6PAGE;
        }
        else if ((strncmp(url,"/srvSet.htm", strlen(url)) == 0))
        {
            return SRVSETPAGE;
        }
#if CONFIG_EZSHAE_ENABLED || defined(CONFIG_EZSHAE_TASK_ENABLED)
        else if ((strncmp(url,"/ezShare.htm", strlen(url)) == 0))
        {
            return EZSHAREPAGE;
        }
#endif
        else if ((strncmp(url,"/EzShare.cab", strlen(url)) == 0))
        {
            return EZSHAREBIN;
        }
        else if ((strncmp(url,"/wifiConf.htm", strlen(url)) == 0))
        {
            return WIFICONFPAGE;
        }
        else if ((strncmp(url,"/about.htm", strlen(url)) == 0))
        {
            return ABOUTPAGE;//ACTPAGE;
        }
        //mail_wakeup 20121025 eccheng
#if CONFIG_MAIL_WAKEUP_ENABLED
        else if ((strncmp(url,"/mail_acc.htm", strlen(url)) == 0))
        {
            return MAIL_WAKEUP_ACC;
        }
        //mail_wakeup 20121025 eccheng
        else if ((strncmp(url,"/mail_mat.htm", strlen(url)) == 0))
        {
            return MAIL_WAKEUP_MAT;
        }
#endif		
        #if CONFIG_ACER_WEB_UI
        else if((strncmp(url,"/welcome-bg-triangle.gif", strlen(url)) == 0))
        {
        	return ACERWBGTRIGGIF;
        }
        else if((strncmp(url,"/info-a-bg.gif", strlen(url)) == 0))
        {
        	return ACERINFOABGGIF;
        }
        else if((strncmp(url,"/info-bg.gif", strlen(url)) == 0))
        {
        	return ACERINFOBGGIF;
        }
        else if((strncmp(url,"/banner-bg.gif", strlen(url)) == 0))
        {
        	return ACERBANBGGIF;
        }
        else if((strncmp(url,"/list-label.gif", strlen(url)) == 0))
        {
        	return ACERLISTLABELGIF;
        }
        else if((strncmp(url,"/bg-triangle.gif", strlen(url)) == 0))
        {
        	return ACERBGTRIGIF;
        }
        #endif
	else if ((strncmp(url,"/computersys.htm", strlen(url)) == 0))
        {
            return COMPUTERSYSPAGE;
        }
	else if ((strncmp(url,"/operationsys.htm", strlen(url)) == 0))
        {
            return OPERATIONSYSPAGE;
        }
	else if ((strncmp(url,"/softwareid.htm", strlen(url)) == 0))
        {
            return SOFTWAREIDPAGE;
        }
	else if ((strncmp(url,"/bios.htm", strlen(url)) == 0))
        {
            return BIOSPAGE;
        }
	else if ((strncmp(url,"/cpu.htm", strlen(url)) == 0))
        {
            return PROCESSORPAGE;
        }
	else if ((strncmp(url,"/memdev.htm", strlen(url)) == 0))
        {
            return MEMORYPAGE;
        }
	else if ((strncmp(url,"/fan.htm", strlen(url)) == 0))
        {
            return FANPAGE;
        }
	else if ((strncmp(url,"/sensor.htm", strlen(url)) == 0))
        {
            return SENSORPAGE;
        }
	else if ((strncmp(url,"/asset.htm", strlen(url)) == 0))
        {
            return ASSETPAGE;
        }
       else if ((strncmp(url,"/showcounter", strlen(url)) == 0))
        {
            return SHOWCOUNTER;
        }
       else if ((strncmp(url,"/resetcounter", strlen(url)) == 0))
        {
            return RESETCOUNTER;
        }
#ifdef CONFIG_MSAD_ENABLED
        else
            return NOTFOUND;
    }
#endif
    else
        return NOTFOUND;

}

int genSrvSetPage(char* buf)
{
    int offset = 0;
    int rv = 0;

    rv = sprintf(buf ,srvSetHtm, dpconf->EchoService?"checked":"", (dpconf->EchoService == 0)?"checked":"");

    offset += rv;
    return offset;
}

#if CONFIG_WIFI_ENABLED
int genWifiConfPage(struct tcp_pcb *pcb, char* buf)
{
    int rv = 0, t = 0;
	
	rv = sprintf(buf ,wifiConfHtm, dpconf->wificfg.wifion?"checked":"",dpconf->wificfg.wifion?"":"checked", 
	dpconf->wificfg.pf_name, dpconf->wificfg.SSID,
	(dpconf->wificfg.auth==0)?"SELECTED":"", (dpconf->wificfg.auth==1)?"SELECTED":"", (dpconf->wificfg.auth==2)?"SELECTED":"", (dpconf->wificfg.auth==3)?"SELECTED":"",
	(dpconf->wificfg.enc==0)?"SELECTED":"", (dpconf->wificfg.enc==1)?"SELECTED":"", (dpconf->wificfg.enc==2)?"SELECTED":"", (dpconf->wificfg.enc==3)?"SELECTED":"",
	dpconf->wificfg.pwd, dpconf->wificfg.pwd);
	t = sprintf(buf ,headerHtm, rv);
	rv = sprintf(buf + t ,wifiConfHtm, dpconf->wificfg.wifion?"checked":"",dpconf->wificfg.wifion?"":"checked", 
	dpconf->wificfg.pf_name, dpconf->wificfg.SSID,
	(dpconf->wificfg.auth==0)?"SELECTED":"", (dpconf->wificfg.auth==1)?"SELECTED":"", (dpconf->wificfg.auth==2)?"SELECTED":"", (dpconf->wificfg.auth==3)?"SELECTED":"",
	(dpconf->wificfg.enc==0)?"SELECTED":"", (dpconf->wificfg.enc==1)?"SELECTED":"", (dpconf->wificfg.enc==2)?"SELECTED":"", (dpconf->wificfg.enc==3)?"SELECTED":"",
	dpconf->wificfg.pwd, dpconf->wificfg.pwd);
	
	sendWebPagePkt(pcb,rv + t,buf);
	return rv;
}
#endif

void IPtoStr(int ip, unsigned char* str)
{
    if (ip)
        sprintf(str ,"%d.%d.%d.%d",
                ip & 0xff,
                (ip & 0xff00) >> 8,
                (ip & 0xff0000) >> 16,
                (ip & 0xff000000) >> 24);
    else
        sprintf(str," ");


}

void IPv6toStr(unsigned char*ip , unsigned char* str)
{
    int i = 0;
    int j = 0;
    unsigned char v = 0;

    for (i = 0; i < IPv6_ADR_LEN; i++)
    {
        if ((i!=0) && (i%2)==0)
        {
            str[j] = ':';
            j++;
        }

        v = (ip[i] & 0xf0) >> 4;

        if (v < 10)
        {
            str[j] = v + 48;
        }
        else
        {
            str[j] = v + 65 - 10;
        }
        j++;

        v = (ip[i] & 0x0f);

        if (v < 10)
        {
            str[j] = v + 48;
        }
        else
        {
            str[j] = v + 65 - 10;
        }
        j++;
    }
}

int genNetSetPage(char* buf, unsigned char intf)
{
    int offset = 0;
    int rv = 0;
    unsigned char HostIP[16]={0};
    unsigned char SubnetMask[16]={0};
    unsigned char GateWayIP[16]={0};
    unsigned char DNSIP[16]={0};
    unsigned char isIB = bsp_in_band_exist();
    unsigned char isSingleIP = 0;
#ifdef CONFIG_TEREDO_ENABLED
    unsigned char HostIPv6[40]={0};
    IPv6toStr(dpconf->HostIPv6, HostIPv6);
#endif

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
    isSingleIP = bsp_bits_get(MAC_OOBREG, BIT_SINGLE_IP, 1);
#else
    isSingleIP = bsp_bits_get(IO_CONFIG1, BIT_SINGLE_IP, 1);
#endif
#endif

    if (dpconf->DHCPv4Enable == 0)
    {
        IPtoStr(dpconf->HostIP[intf].addr, HostIP);
        IPtoStr(dpconf->SubnetMask[intf].addr, SubnetMask);
        IPtoStr(dpconf->GateWayIP[intf].addr, GateWayIP);
        IPtoStr(dpconf->DNSIP[intf].addr, DNSIP);
    }



    rv = sprintf(buf , netSetIPv4Htm, (isSingleIP&&isIB)?"disabled":"", (!(dpconf->DHCPv4Enable & (1<<intf)))?"checked":"",
                 (isSingleIP&&isIB)?"disabled":"",(dpconf->HostIP[intf].addr && !(dpconf->DHCPv4Enable & (1<<intf)))?HostIP:"\"\"",
                 (isSingleIP&&isIB)?"disabled":"",(dpconf->SubnetMask[intf].addr && !(dpconf->DHCPv4Enable& (1<<intf)))?SubnetMask:"\"\"",
                 (isSingleIP&&isIB)?"disabled":"",(dpconf->GateWayIP[intf].addr && !(dpconf->DHCPv4Enable& (1<<intf)))?GateWayIP:"\"\"",
                 (isSingleIP&&isIB)?"disabled":"",(dpconf->DNSIP[intf].addr&& !(dpconf->DHCPv4Enable& (1<<intf)))?DNSIP:"\"\"",
                 isIB?"<!--":"<p>",(dpconf->DHCPv4Enable)?"checked":"",isIB?"-->":"</p>",
                 (isSingleIP&&isIB)?"disabled":"",((dpconf->IPv4Enable& (1<<intf)) == 0)?"checked":""
#ifdef CONFIG_TEREDO_ENABLED
                 ,HostIPv6
#endif
                );
    offset += rv;
    return offset;
}

int genNetSetv6Page(char* buf, unsigned char intf)
{
    int offset = 0;
    int rv = 0;
    //8*4 + 7(:) + 1 'null'
    unsigned char HostIPv6[40]={0};
    unsigned char GateWayIP[40]={0};
    unsigned char DNSIP[40]={0};
    unsigned char PrefixLen[3]={0};

    if (!dpconf->DHCPv6Enable)
    {
        IPv6toStr(dpconf->HostIPv6[intf], HostIPv6);
        IPv6toStr(dpconf->IPv6GateWay[intf], GateWayIP);
        IPv6toStr(dpconf->IPv6DNS[intf], DNSIP);
    }

    sprintf(PrefixLen, "%d", dpconf->IPv6PrefixLen[intf]);
    rv = sprintf(buf , netSetIPv6Htm, (!(dpconf->DHCPv6Enable& (1<<intf)))?"checked":"",
                 (!(dpconf->DHCPv6Enable & (1<<intf)))?HostIPv6:"\"\"",
                 (!(dpconf->DHCPv6Enable & (1<<intf)))?PrefixLen:"\"\"",
                 (!(dpconf->DHCPv6Enable & (1<<intf)))?GateWayIP:"\"\"",
                 (!(dpconf->DHCPv6Enable & (1<<intf)))?DNSIP:"\"\"",
                 (dpconf->DHCPv6Enable & (1<<intf))?"checked":"", ((dpconf->IPv6Enable & (1<<intf)) == 0)?"checked":"");
    offset += rv;
    return offset;
}

#if CONFIG_ACER_WEB_UI
int genAboutPage(char* buf)
{
    int offset = 0;
    int rv = 0;

    rv = sprintf(buf ,"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n"\
                 "<html>\n"\
                 "<head>\n"\
                 "<title>Firmware Infomation</title>\n"\
                 "</head>\n"\
                 "<body>\n"\
                 "<H2>Firmware Infomation</H2>\n"\
                 "<TABLE cellSpacing=0><TBODY><tr><td colspan=3></td></tr><TR>");

    offset += rv;

    rv = sprintf(buf + offset,"<TD width=150 class=\"rt\">Frimware Version</td><td width=\"14\">:</td><td width=130 class=\"t\">%d.%d.%d.%d</td><tr>", dpconf->fwMajorVer,dpconf->fwMinorVer, dpconf->fwExtraVer, dpconf->fwBuildVer);
    offset += rv;

    rv = sprintf(buf + offset,"<Td class=\"rt\">Built Date </td><td>:</td><td class=\"t\">%d</td><tr>", dpconf->builddate);
    offset += rv;

    rv = sprintf(buf + offset,"<tr><td colspan=3></td></tr></TBODY></TABLE></BODY></HTML>");

    offset += rv;
    return offset;
}
#else
int genAboutPage(char* buf)
{
    int offset = 0;
    int rv = 0;

    rv = sprintf(buf ,"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n"\
                 "<html>\n"\
                 "<head>\n"\
                 "<title>Firmware Infomation</title>\n"\
                 "</head>\n"\
                 "<body>\n"\
                 "<H2>Firmware Infomation</H2>\n"\
                 "<hr><table>");

    offset += rv;

    rv = sprintf(buf + offset,"<td>Frimware Version: </td><td width=\"10\"></td><td>%d.%d.%d.%d</td><tr>", dpconf->fwMajorVer,dpconf->fwMinorVer, dpconf->fwExtraVer, dpconf->fwBuildVer);
    offset += rv;

    rv = sprintf(buf + offset,"<td>Built Date: </td><td></td><td>%d</td><tr>", dpconf->builddate);
    offset += rv;

    rv = sprintf(buf + offset,"</table><hr>\n</body>\n</html>");

    offset += rv;
#ifdef CONFIG_IPERF_Test
    OSTaskResume(TASK_IPERF_PRIO);
#endif
    return offset;
}
#endif

void sendStaticWebPage(struct tcp_pcb *pcb,int wpid)
{
    DataList *head;
    DataList *current;
    int rv = 0, offset = 0, pageSize = 0;
#if CONFIG_EZSHAE_TASK_ENABLED
    unsigned char HostIP[16]={0};
#endif
#ifdef CONFIG_MSAD_ENABLED
    unsigned char kerbEnable = pcb->msad.auth_buffer != NULL ? 1 : 0;
#endif
    head = newDataList();
    current = head;
#ifdef CONFIG_MSAD_ENABLED
    if (wpid == LOGOGIF)
        pageSize = sizeof(logo)/sizeof(unsigned char);
    else
#endif
        pageSize = strlen(webPages[wpid]);

#if CONFIG_EZSHAE_TASK_ENABLED
    if (wpid == INFOPAGE)
    {
        IPtoStr(dpconf->HostIP[intf].addr, HostIP);
        pageSize = pageSize - 2;
        pageSize = pageSize + strlen(HostIP);
    }
#endif

    if (pcb->TLSEnable &&(wpid == HOMEPAGE))
    {
        if (COOKIE_Idx >= 3)
            COOKIE_Idx = 0;
        COOKIE_ID[COOKIE_Idx] = rand();
        rv = sprintf(current->payLoadAddr, cookieHdrHtm, COOKIE_ID[COOKIE_Idx] , pageSize);
        COOKIE_Idx++;
        current->len = rv;
    }
#if 0
    else if (wpid == RDCSS)
    {
        //In 10M, client will not request rdcss.css
        rv = sprintf(current->payLoadAddr, "HTTP/1.1 200 \r\nCache-Control: max-age=600\r\nContent-type: text/html\r\nContent-Length: %5d\r\n\r\n", pageSize);
    }
#endif
    else
#ifdef CONFIG_MSAD_ENABLED
    {

        if (dpconf->Security == KERB_AUTH)
        {
            if (kerbEnable)
            {
                if (wpid==LOGOGIF)
                    current->len = sprintf(current->payLoadAddr,
                                           "HTTP/1.1 200 OK\r\nServer: Realtek Management Technology\r\nContent-type: image/gif\r\nContent-Length: %5d\r\n", pageSize);
                else
                    current->len = sprintf(current->payLoadAddr, headerHtm2, pageSize);
                current->len += sprintf(current->payLoadAddr + current->len, negoHeader);
                current->len += addADauth(current->payLoadAddr + current->len, &pcb->msad.auth_buffer);
            }
            else
            {
                if (wpid == LOGOGIF)
                    rv = sprintf(current->payLoadAddr, "HTTP/1.1 200 \r\nContent-type: image/gif\r\nContent-Length: %d\r\n\r\n", pageSize);
                else
                    rv = sprintf(current->payLoadAddr, headerHtm3, pageSize);
                current->len = rv;
            }
        }
        else
        {
            if (wpid==LOGOGIF)
                rv = sprintf(current->payLoadAddr, "HTTP/1.1 200 \r\nContent-type: image/gif\r\nContent-Length: %d\r\n\r\n", pageSize);
            else
#endif
                rv = sprintf(current->payLoadAddr, headerHtm, pageSize);

            current->len = rv;
#ifdef CONFIG_MSAD_ENABLED
        }
    }
#endif
    do{
        //if((pageSize - (ETH_PAYLOAD_LEN - LEAST_PAGE_LINE_LEN)) > LEAST_PAGE_LINE_LEN)
        if (( pageSize + current->len) > ETH_PAYLOAD_LEN)
        {
            memcpy(current->payLoadAddr + current->len, webPages[wpid] + offset, ETH_PAYLOAD_LEN - current->len);
            rv = ETH_PAYLOAD_LEN - current->len;
            pageSize = pageSize - rv;
            offset += rv;
            current->len += rv;
            current = chkDataListBufSize(current);
        }
        else
        {
#ifdef CONFIG_MSAD_ENABLED
            if (wpid==LOGOGIF)
            {
                memcpy(current->payLoadAddr + current->len, webPages[wpid] + offset, pageSize);
                current->len += pageSize;
            }
            else
#endif
#if CONFIG_EZSHAE_TASK_ENABLED
                if (wpid == INFOPAGE)
                {
                    current->len += sprintf(current->payLoadAddr + current->len, webPages[wpid] + offset, HostIP);
                }
                else
#endif
                    current->len += sprintf(current->payLoadAddr + current->len, webPages[wpid] + offset, pageSize);
            pageSize = 0;
        }
    }while (pageSize);
    sendDataList(pcb, head);
}

#ifdef CONFIG_MSAD_ENABLED
void send4Page(struct tcp_pcb *pcb, WebPageID wpid)
{
    int offset=0, rv=0;
    DataList *head;
    DataList *current;
    int headerSize = 0, pageSize = 0;
    char tmpChar;
    char tmpBuf[MAX_PAGE_LINE_LEN] = {0};
	unsigned char kerbEnable = pcb->msad.auth_buffer != NULL ? 1 : 0;
    head = newDataList();
    current = head;

    if (dpconf->Security == KERB_AUTH)
    {
        if (!kerbEnable)
        {
            headerSize = sprintf(current->payLoadAddr, headerHtm3);
            current->len = headerSize;
        }
    }
    else
    {
        headerSize = sprintf(current->payLoadAddr, headerHtm, 0);
        current->len = headerSize;
    }

    if (wpid == SRVSETPAGE)
        pageSize = genSrvSetPage(current->payLoadAddr + current->len);
    else if (wpid == NETSETPAGE)
        pageSize = genNetSetPage(current->payLoadAddr + current->len);
    else if (wpid == NETSETv6PAGE)
        pageSize = genNetSetv6Page(current->payLoadAddr + current->len);
    else if (wpid == ABOUTPAGE)
        pageSize = genAboutPage(current->payLoadAddr + current->len);

    current->len += pageSize;

    if (dpconf->Security == KERB_AUTH && kerbEnable)
    {
        PKT* pkt = (PKT*)allocPkt(ETH_PAYLOAD_LEN);
        headerSize = sprintf(pkt->wp, headerHtm2, pageSize);
        headerSize += sprintf(pkt->wp + headerSize, negoHeader);
		headerSize += addADauth(pkt->wp + headerSize, &pcb->msad.auth_buffer);

        pkt->len = headerSize;
        tcpSendData_F(pcb, pkt);
    }

    tmpChar = head->payLoadAddr[headerSize];
    if (dpconf->Security == KERB_AUTH)
    {
        if (!kerbEnable)
        {
            sprintf(head->payLoadAddr, headerHtm3, getDataListBufSize(head) - headerSize);
        }
    }
    else
    {
        sprintf(head->payLoadAddr, headerHtm, getDataListBufSize(head) - headerSize);
    }
    head->payLoadAddr[headerSize] = tmpChar;
    //buf[rv] = tmpChar;
    sendDataList(pcb, head);
}
#endif

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
void sendEditPage(struct tcp_pcb *pcb, WAKE_UP_t *wptr)
{
    DataList *head;
    DataList *current;
    int headerSize = 0;
    int j= 0;
    char tmpBuf[MAX_PAGE_LINE_LEN] = {0};

    head = newDataList();
    current = head;

    headerSize = sprintf(current->payLoadAddr, headerHtm, 0);
    current->len = headerSize;
    current->len += sprintf(current->payLoadAddr + current->len, "%s<H2>%s A Wake Up Pattern </H2><hr><p>", EditPageTopHtm, wptr ? "Edit" : "Add" );

    current->len += sprintf(current->payLoadAddr + current->len, "<form method=\"POST\" action=\"/PAdd\"><input type=\"hidden\" name=\"ptr\" value=\"%p\"><table cellspacing = 0><tr><td>Rule Enabled :</td>", wptr);
    if (wptr && wptr->valid)
        current->len += sprintf(current->payLoadAddr +  current->len, "<td><input type =\"checkbox\" name=\"chk\" checked ></td>" );
    else
        current->len += sprintf(current->payLoadAddr +  current->len, "<td><input type =\"checkbox\" name=\"chk\"></td>" );


    current->len += sprintf(current->payLoadAddr + current->len, "</tr><td>Rule Name: </td>");
    if (wptr)
        current->len += sprintf(current->payLoadAddr +  current->len, "<td><input type=\"text\" name=\"pname\" value=\"%s\"></td>" , wptr->name);
    else
        current->len += sprintf(current->payLoadAddr +  current->len, "<td><input type=\"text\" name=\"pname\" ></td>");

    current->len += sprintf(current->payLoadAddr + current->len, "</tr><td>Pattern Start Offset (Hex): </td>");

    if (wptr)
        current->len += sprintf(current->payLoadAddr +  current->len, "<td><input type=\"text\" name=\"pstart\" value=\"%02X\"></td>" , wptr->start);
    else
        current->len += sprintf(current->payLoadAddr +  current->len, "<td><input type=\"text\" name=\"pstart\"></td>");

    current->len += sprintf(current->payLoadAddr + current->len, "</tr><td>Pattern Length (Hex): </td>");

    if (wptr)
        current->len += sprintf(current->payLoadAddr +  current->len, "<td><input type=\"text\" name=\"plength\" value=\"%02X\"></td>" , wptr->length);
    else
        current->len += sprintf(current->payLoadAddr +  current->len, "<td><input type=\"text\" name=\"plength\"></td>");

    current->len += sprintf(current->payLoadAddr + current->len, "</tr><td>Patterns (Hex Strings): </td><td>&nbsp</td><tr><td colspan = \"2\"><textarea name=\"pptns\" rows=\"6\" cols=\"65\">");
    if (wptr)
    {
        for (j = 0 ; j < wptr->length; j++)
            current->len += sprintf(current->payLoadAddr +  current->len, "%02X", wptr->pattern[j]);
    }
    current->len += sprintf(current->payLoadAddr +  current->len, "</textarea></td></tr><tr><td>Pattern Masks (Hex Strings):</td><td><input type=\"text\" name=\"pmask\" ");

    if (wptr)
    {

        current->len += sprintf(current->payLoadAddr +  current->len,  "value=\"");
        for (j = 0 ; j < ifloor(wptr->length, 8); j++)
            current->len += sprintf(current->payLoadAddr +  current->len, "%02X" , wptr->mask[j]);
        current->len += sprintf(current->payLoadAddr +  current->len,  "\"");
    }

    current->len += sprintf(current->payLoadAddr +  current->len, "></td></tr></table><p><hr><p><input type=\"submit\" name=\"PTN\" value=\"%s Rule\"></form></body>\n</html>", wptr ? "Edit The" : "Add A" );

    //current->len += sprintf(current->payLoadAddr + current->len, "</form></table></body>\n</html>");

    sprintf(tmpBuf, headerHtm,getDataListBufSize(head) - headerSize);
    memcpy(head->payLoadAddr, tmpBuf,strlen(headerHtm)-1);

    sendDataList(pcb, head);
}

void sendEventLogPage(struct tcp_pcb *pcb) // not eventlog , this is for Wakeup Patter Rule Edit
{
    DataList *head;
    DataList *current;
    WAKE_UP_t *wptr = GetWakePtnHead();

    int headerSize = 0;
    char tmpBuf[MAX_PAGE_LINE_LEN] = {0};
    int i, j = 0;

    head = newDataList();
    current = head;

    headerSize = sprintf(current->payLoadAddr, headerHtm, 0);
    current->len = headerSize;
    current->len += sprintf(current->payLoadAddr + current->len, "%s",eventLogTopHtm);


    current->len += sprintf(current->payLoadAddr + current->len, "<form method=\"POST\" action=\"/PEdit\"><tr><td>Enabled</td><td>Rule Name</td><td>Pattern Start</td><td>Pattern Length</td><td>Patterns </td><td>Pattern Masks</td><td>&nbsp;</td><td>&nbsp;</td></tr>");


    while (wptr)
    {
        current = chkDataListBufSize(current);
        current->len += sprintf(current->payLoadAddr + current->len, "<tr>");

        current->len += sprintf(current->payLoadAddr +  current->len, "<td align=\"center\">%s</td>", wptr->valid ? "YES"  : "NO" );
        /*
            if (wptr->valid)
                current->len += sprintf(current->payLoadAddr +  current->len, "<td align=\"center\"><input type =\"checkbox\" checked ></td>" );
            else
                current->len += sprintf(current->payLoadAddr +  current->len, "<td align=\"center\"><input type =\"checkbox\"></td>" );
            */
        current->len += sprintf(current->payLoadAddr +  current->len, "<td align=\"center\">%s</td>" , wptr->name);

        current->len += sprintf(current->payLoadAddr +  current->len, "<td align=\"center\">0x%X</td>" , wptr->start);
        current->len += sprintf(current->payLoadAddr +  current->len, "<td align=\"center\">0x%X</td>" , wptr->length);
        current->len += sprintf(current->payLoadAddr +  current->len, "<td>");
        for (j = 0 ; j < wptr->length; j++)
        {
            if ((wptr->mask[j/8] >> j%8) & 0x01)
                current->len += sprintf(current->payLoadAddr +  current->len, "%02X&nbsp;" , wptr->pattern[j]);
            else
                current->len += sprintf(current->payLoadAddr +  current->len, "XX&nbsp;" );
            if (((j+1) % 8 ) == 0)
                current->len += sprintf(current->payLoadAddr +  current->len, "<br>" );

            current = chkDataListBufSize(current);
        }
        current->len += sprintf(current->payLoadAddr +  current->len, "</td>");
        current->len += sprintf(current->payLoadAddr +  current->len, "<td>");
        for (j = 0 ; j < ifloor(wptr->length, 8); j++)
            current->len += sprintf(current->payLoadAddr +  current->len, "%02X<br>" , wptr->mask[j]);
        current->len += sprintf(current->payLoadAddr +  current->len, "</td>");

        current->len += sprintf(current->payLoadAddr +  current->len, "<td align=\"center\"><input type =\"submit\" name=\"PM_%p\" value=\"Modify\"></td>", wptr );
        current->len += sprintf(current->payLoadAddr +  current->len, "<td align=\"center\"><input type =\"submit\" name=\"PD_%p\" value=\"Delete\"></td>", wptr );
        current->len += sprintf(current->payLoadAddr + current->len, "</tr>");
        wptr = wptr->next;
    }
    current->len += sprintf(current->payLoadAddr + current->len, "<tr><td colspan=\"8\" align=\"center\"><input type=\"submit\" name=\"PM_0\" value =\"Add A Rule\"></td></tr></form></table></body>\n</html>");

    sprintf(tmpBuf, headerHtm,getDataListBufSize(head) - headerSize);
    memcpy(head->payLoadAddr, tmpBuf,strlen(headerHtm)-1);

    sendDataList(pcb, head);
}
#else

void sendEventLogPage(struct tcp_pcb *pcb)
{
    DataList *head;
    DataList *current;

    eventdata *ed;
#ifdef CONFIG_MSAD_ENABLED
    unsigned char kerbEnable = pcb->msad.auth_buffer != NULL ? 1 : 0;
#endif
    ///struct _elist *el;
    int headerSize = 0;
    //char tmpBuf[MAX_PAGE_LINE_LEN] = {0};
    INT8U *tmpBuf = malloc(MAX_PAGE_LINE_LEN);
    int i = 0;
    memset(tmpBuf, 0, MAX_PAGE_LINE_LEN);

    head = newDataList();
    current = head;
#ifdef CONFIG_MSAD_ENABLED
    if (dpconf->Security == KERB_AUTH)
    {
        if (kerbEnable)
        {
            current->len = sprintf(current->payLoadAddr, headerHtm2, 0);
            current->len += sprintf(current->payLoadAddr + current->len, negoHeader);
            current->len += addADauth(current->payLoadAddr + current->len, &pcb->msad.auth_buffer);
            headerSize = current->len;
        }
        else
        {
            headerSize = sprintf(current->payLoadAddr, headerHtm3, 0);
            current->len = headerSize;
        }
    }
    else
    {
#endif
        headerSize = sprintf(current->payLoadAddr, headerHtm, 0);
        current->len = headerSize;
#ifdef CONFIG_MSAD_ENABLED
    }
#endif
    current->len += sprintf(current->payLoadAddr + current->len, "%s",eventLogTopHtm);
    ed = eventGet();

#if CONFIG_ACER_WEB_UI
    while (ed)
    {    	
        ///ed = el->addr;
        if (i%2 == 0)
        {
        	current->len += sprintf(current->payLoadAddr + current->len, "<TR><td class=\"list-label\"><img src=\"list-label.gif\"></td>");   
            	current->len += sprintf(current->payLoadAddr + current->len, "<TD bgColor=#d2ddd7>&nbsp;%s</td> <TD width=10 bgColor=#d2ddd7>:</TD>",ctime ((time_t *) &(ed->timestamp)) );
        }
        else
        {
                current->len += sprintf(current->payLoadAddr + current->len, "<th></th><th>&nbsp;%s</th> <th>:</th>",ctime ((time_t *) &(ed->timestamp)) );
        }

        getEventLogParser(ed, tmpBuf);

 	if (i %2 == 0)
 	{
 		current->len += sprintf(current->payLoadAddr + current->len, "<TD bgColor=#d2ddd7 class=\"t\">%s</TD><tr>\n", tmpBuf); 		
 	}
 	else
 	{  
            current->len += sprintf(current->payLoadAddr + current->len, "<th class=\"t\">%s</th><tr>\n", tmpBuf);
        }

        current = chkDataListBufSize(current);
        ed = ed->next;
        i++;
    }

    current->len += sprintf(current->payLoadAddr + current->len, "</TBODY></table><hr>\n</body>\n</html>");
    #else
    while (ed)
    {
        ///ed = el->addr;
        if (i == 0)
            current->len += sprintf(current->payLoadAddr + current->len, "<td bgcolor=FF9900>>%s</td><td width=10 bgcolor=FF9900>:</td>",ctime ((time_t *) &(ed->timestamp)) );
        else
        {
            if (i%2)
                current->len += sprintf(current->payLoadAddr + current->len, "<td>>%s</td><td>:</td>",ctime ((time_t *) &(ed->timestamp)) );
            else
                current->len += sprintf(current->payLoadAddr + current->len, "<td bgcolor=FF9900>>%s</td><td bgcolor=FF9900>:</td>",ctime ((time_t *) &(ed->timestamp)) );
        }

	printf("[RTK] <0x%x, 0x%x>\n",ed->Event_Sensor_Type,ed->Event_Type   );

#if CONFIG_FWLOG_ENABLED
	if(ed->Event_Sensor_Type == 0x7F && ed->Event_Type == 0x1F ){
	      getFwLogParser(ed, tmpBuf);

	}else
#endif	
        getEventLogParser(ed, tmpBuf);

        if (i%2)
            current->len += sprintf(current->payLoadAddr + current->len, "<td>%s</td><tr>\n", tmpBuf);
        else
            current->len += sprintf(current->payLoadAddr + current->len, "<td bgcolor=FF9900>%s</td><tr>\n", tmpBuf);

        current = chkDataListBufSize(current);
        ed = ed->next;
        i++;
    }
#endif
    current->len += sprintf(current->payLoadAddr + current->len, "</table><hr>\n</body>\n</html>");
#ifdef CONFIG_MSAD_ENABLED
    if (dpconf->Security == KERB_AUTH)
    {
        if (kerbEnable)
        {
            sprintf(tmpBuf, headerHtm2, getDataListBufSize(head) - headerSize);
            memcpy(head->payLoadAddr, tmpBuf,strlen(headerHtm2));
        }
        else
        {
            sprintf(tmpBuf, headerHtm3, getDataListBufSize(head) - headerSize);
            memcpy(head->payLoadAddr, tmpBuf,strlen(headerHtm3)-1);
        }
    }
    else
    {
#endif
        sprintf(tmpBuf, headerHtm,getDataListBufSize(head) - headerSize);
        memcpy(head->payLoadAddr, tmpBuf,strlen(headerHtm)-1);
#ifdef CONFIG_MSAD_ENABLED
    }
#endif

    free(tmpBuf);
    sendDataList(pcb, head);
}
#endif

#ifdef CONFIG_SMBIOS_ENABLED
int sendSMBIOSWebPage(struct tcp_pcb *pcb, int type)
{
    DataList *head;
    DataList *current;
    int headerSize = 0;
    char tmpChar;
#ifdef CONFIG_MSAD_ENABLED
    char tmpBuf[MAX_PAGE_LINE_LEN] = {0};
	unsigned char kerbEnable = pcb->msad.auth_buffer != NULL ? 1 : 0;
#endif
    head = newDataList();
    current = head;
#ifdef CONFIG_MSAD_ENABLED
    if (dpconf->Security == KERB_AUTH)
    {
        if (kerbEnable)
        {
            current->len = sprintf(current->payLoadAddr, headerHtm2, 0);
            current->len += sprintf(current->payLoadAddr + current->len, negoHeader);
            current->len += addADauth(current->payLoadAddr + current->len, &pcb->msad.auth_buffer);
            headerSize = current->len;
        }
        else
        {
            headerSize = sprintf(current->payLoadAddr, headerHtm3, 0);
            current->len = headerSize;
        }
    }
    else
    {
#endif
        headerSize = sprintf(current->payLoadAddr, headerHtm, 0);
        current->len = headerSize;
#ifdef CONFIG_MSAD_ENABLED
    }
#endif
    if (genSMBIOSWebPage(type, current) == -1)
        return -1;

    tmpChar = head->payLoadAddr[headerSize];
#ifdef CONFIG_MSAD_ENABLED
    if (dpconf->Security == KERB_AUTH)
    {
        if (kerbEnable)
        {
            sprintf(tmpBuf, headerHtm2, getDataListBufSize(head) - headerSize);
            memcpy(head->payLoadAddr, tmpBuf,strlen(headerHtm2));
        }
        else
        {
            sprintf(head->payLoadAddr, headerHtm3, getDataListBufSize(head) - headerSize);
        }
    }
    else
    {
#endif
        sprintf(head->payLoadAddr, headerHtm, getDataListBufSize(head) - headerSize);
#ifdef CONFIG_MSAD_ENABLED
    }
#endif
    head->payLoadAddr[headerSize] = tmpChar;
    sendDataList(pcb, head);
    return 0;
}
#endif

void sendRmoteCtlPage(struct tcp_pcb *pcb)
{
    DataList *head;
    DataList *current;
    int headerSize = 0, i = 0;
    char bootDevice[] = {0x01, 0x06, 0x03, 0x06, 0x00};
    char tmpChar;
#ifdef CONFIG_MSAD_ENABLED
    char tmpBuf[MAX_PAGE_LINE_LEN] = {0};
    unsigned char kerbEnable = pcb->msad.auth_buffer != NULL ? 1 : 0;
#endif
    head = newDataList();
    current = head;
#ifdef CONFIG_MSAD_ENABLED
    if (dpconf->Security == KERB_AUTH)
    {
        if (kerbEnable)
        {
            current->len = sprintf(current->payLoadAddr, headerHtm2, 0);
            current->len += sprintf(current->payLoadAddr + current->len, negoHeader);
            current->len += addADauth(current->payLoadAddr + current->len, &pcb->msad.auth_buffer);
            headerSize = current->len;
        }
        else
        {
            headerSize = sprintf(current->payLoadAddr, headerHtm3,0 );
            current->len = headerSize;
        }
    }
    else
    {
#endif
        headerSize = sprintf(current->payLoadAddr, headerHtm, 0);
        current->len = headerSize;
#ifdef CONFIG_MSAD_ENABLED
    }
#endif
    current->len += sprintf(current->payLoadAddr + current->len, remoteCtlHtmTop);

#if 0
    while (bootDevice[i] != 0)
    {
        current = chkDataListBufSize(current);

        if (bootDevice[i] == 6)
        {
            i++;
            continue;
        }
        
        current->len += sprintf(current->payLoadAddr + current->len, "<p><input type=radio name=boot value=");
        switch (bootDevice[i])
        {
        case 1:
            current->len += sprintf(current->payLoadAddr + current->len, "\"fp\">Floppy</p>");
            break;
        case 2:
            current->len += sprintf(current->payLoadAddr + current->len, "\"hd\">Hard-Disk</p>");
            break;
        case 3:
            current->len += sprintf(current->payLoadAddr + current->len, "\"cd\">CD/DVD</p>");
            break;
        case 4:
            current->len += sprintf(current->payLoadAddr + current->len, "\"net\">NetWork</p>");
            break;
        case 5:
            current->len += sprintf(current->payLoadAddr + current->len, "\"usb\">USB</p>");
            break;
        default:
            ;
        }
        current->len += sprintf(current->payLoadAddr + current->len, "\n");
        i++;
    }
  current->len += sprintf(current->payLoadAddr + current->len, "<p><input type=\"radio\" name=\"boot\" value=\"disable\" checked>Disable");
#endif	    
    current->len += sprintf(current->payLoadAddr + current->len, remoteCtlHtmBottom);
    tmpChar = head->payLoadAddr[headerSize];
#ifdef CONFIG_MSAD_ENABLED
    if (dpconf->Security == KERB_AUTH)
    {
        if (kerbEnable)
        {
            sprintf(tmpBuf, headerHtm2, getDataListBufSize(head) - headerSize);
            memcpy(head->payLoadAddr, tmpBuf,strlen(headerHtm2));
        }
        else
        {
            sprintf(head->payLoadAddr, headerHtm3, getDataListBufSize(head) - headerSize);
        }
    }
    else
    {
#endif
        sprintf(head->payLoadAddr, headerHtm,getDataListBufSize(head) - headerSize);
#ifdef CONFIG_MSAD_ENABLED
    }
#endif
    head->payLoadAddr[headerSize] = tmpChar;
    sendDataList(pcb, head);
}

/**
* Description:	sendWebPages will parse request URL and send correspondent web pages
*
* Parameters:
*				pcb ->	Connection
*				rul ->	Request URL
*
* Returns:		None
*
*/
int sendWebPages(struct tcp_pcb *pcb, char *url)
{
    int rv = 0;
    int pageSize = 0;
    int offset = 0;
    int type;
    char *buf = 0;
    PKT	 *pkt;
    WebPageID wpid;
    char tmpChar;
    unsigned char err = 0;
    unsigned char* ezShareBin = 0x81080000;

#ifdef IN_BAND_TEST
    unsigned char *data;
    unsigned char *data1;
    unsigned char v1;
    unsigned char v2;
    int  i = 0;
#endif

    DEBUGMSG(HTTP_DEBUG,"sendWebPages ==>");
    wpid =  getWebPageID(url);
#ifdef CONFIG_MSAD_ENABLED
    if (pcb->msad.wpid != 0)
    {
        wpid = pcb->msad.wpid;
        pcb->msad.wpid = 0;
    }
#endif
    if (wpid != NOTFOUND)
    {
#ifndef CONFIG_MSAD_ENABLED
        if ((wpid == LOGOGIF)
        #if CONFIG_ACER_WEB_UI
    	|| (wpid == ACERWBGTRIGGIF) || (wpid == ACERINFOABGGIF)|| (wpid == ACERINFOBGGIF) || (wpid == ACERBANBGGIF) || (wpid == ACERLISTLABELGIF) || (wpid == ACERBGTRIGIF)
   	#endif
        )
        {
       	//image can not use strlen to get size

	printf("[RTK]  wpid=%d \n ",wpid);
	//printf("[RTK]  WebPageSize[wpid]=%d \n ",WebPageSize[wpid]);	
	//printf("[RTK] webPages[wpid]=0x%x \n ",webPages[wpid]);	

	switch(wpid){
		case LOGOGIF:
			pageSize = sizeof(logo);
		break;	
	#if CONFIG_ACER_WEB_UI
		case ACERWBGTRIGGIF:
			pageSize = sizeof(AcerWBgTriGif);
		break;
		case ACERINFOABGGIF:
			pageSize = sizeof(AcerInfoABgGif);	
		break;	
		case ACERINFOBGGIF:
			pageSize =sizeof(AcerInfoBgGif);	
		break;	
		case ACERBANBGGIF:
			pageSize = sizeof(AcerBannerBgGif);	
		break;	
		case ACERLISTLABELGIF:
			pageSize = sizeof(AcerListLabelGif);
		break;		
		case ACERBGTRIGIF:
			pageSize = sizeof(AcerBGTriangleGif);
		break;
	#endif
		default:
			pageSize = 0;
		break;
	}
	

	 
	 buf = (char*)malloc(pageSize + 200);	
	 
	//printf("[RTK]  pageSize=%d , but ACERBGTRIGIF size=%d\n ",pageSize ,  sizeof(AcerBGTriangleGif));	
            //buf = (char*)malloc(WebPageSize[wpid] + 200);
           // pageSize = WebPageSize[wpid];
           
		   
            rv = sprintf(buf, "HTTP/1.1 200 \r\nContent-type: image/gif\r\nContent-Length: %d\r\n\r\n", pageSize);
            offset += rv;
            //Do not use sprintf here
            memcpy(buf + offset, webPages[wpid], pageSize);
            offset += pageSize;
        }
#endif
#ifdef CONFIG_MSAD_ENABLED
        if ( wpid == EVENTLOGPAGE)
#else
        else if ( wpid == EVENTLOGPAGE)
#endif
        {
            sendEventLogPage(pcb);
            return 0;
        }
        else if (wpid == SRVSETPAGE || wpid == NETSETPAGE || wpid == NETSETv6PAGE || wpid == ABOUTPAGE)
        {
            //headerHtm => %5d => fixed length
#ifdef CONFIG_MSAD_ENABLED
            send4Page(pcb, wpid);
            return 0;
#else
            buf = (char*)malloc(ETH_PAYLOAD_LEN);
            //headerHtm => %5d => fixed length
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;

            if (wpid == SRVSETPAGE)
                pageSize = genSrvSetPage(buf + offset);
            else if (wpid == NETSETPAGE)
                pageSize = genNetSetPage(buf + offset, pcb->intf);
            else if (wpid == NETSETv6PAGE)
                pageSize = genNetSetv6Page(buf + offset, pcb->intf);
            else if (wpid == ABOUTPAGE)
                pageSize = genAboutPage(buf + offset);

            offset += pageSize;

            //buf[rv] will append 0 for string end
            tmpChar = buf[rv];
            sprintf(buf ,headerHtm, pageSize);
            buf[rv] = tmpChar;
#endif
        }
		#if CONFIG_WIFI_ENABLED
		else if(wpid == WIFICONFPAGE)
		{
			buf = (char*)malloc(ETH_PAYLOAD_LEN);
			memset(buf,0x00,ETH_PAYLOAD_LEN);
			
			genWifiConfPage(pcb,buf);

			if (buf)
				free(buf);
			return 0;		
		}
		#endif
        else if (wpid == REMOTECTLPAGE)
        {
#ifdef IN_BAND_TEST
            data = malloc(1024);
            data1 = malloc(1024);

            for (i = 0; i < 1024; i++)
                data[i] = data1[i] = (rand() % 255);

            if (bsp_get_inband_data(data, 1024) != -1)
            {
                for (i = 0; i < 1024; i++)
                {
                    v1 = data[i];
                    v2 = data1[i] + 10;
                    if (v1 != v2)
                    {
                        for (i = 0; i < 1024; i++)
                            data[i] = data1[i] = 0;
                    }
                }
            }
            else
            {
                i++;
            }
            free(data);
            free(data1);
#endif
		sendRmoteCtlPage(pcb);
#ifdef CONFIG_MSAD_ENABLED
            return 0;
#endif
        }
#if CONFIG_EZSHAE_ENABLED || defined (CONFIG_EZSHAE_TASK_ENABLED)
        else if (wpid == EZSHAREBIN)
        {
            if (pcb->apRx)
                goto NOTFOUND;
            else
            {
                OSSemPost(SktSem);
                buf = (char*)malloc(ETH_PAYLOAD_LEN);
                //4 byte length + data
                pageSize = *((int*)ezShareBin);
                ezShareBin += 4;
                rv = sprintf(buf, "HTTP/1.1 200 \r\nContent-type: application/octet-stream\r\nContent-Length: %d\r\n\r\n", pageSize);
                offset += rv;

                pkt = (PKT*)allocPkt_F(offset);
                memcpy(pkt->wp, buf, offset);
                free(buf);
                offset = 0;

                while (pageSize)
                {
                    rv = rtSktSend_F(pcb->skt, pkt, 0);
                    if (rv == SKTDISCONNECT)
                        break;

                    if (pageSize > ETH_PAYLOAD_LEN)
                    {
                        pkt = (PKT*)allocPkt_F(ETH_PAYLOAD_LEN);
                        memcpy(pkt->wp, ezShareBin + offset, ETH_PAYLOAD_LEN);
                        pageSize = pageSize - ETH_PAYLOAD_LEN;
                        offset = offset + ETH_PAYLOAD_LEN;
                    }
                    else
                    {
                        pkt = (PKT*)allocPkt_F(pageSize);
                        memcpy(pkt->wp, ezShareBin + offset, pageSize);
                        rv = rtSktSend_F(pcb->skt, pkt, 0);
                        pageSize = 0;
                    }
                }
                OSSemPend(SktSem, 0, &err);
                return rv;
            }
        }
#endif
#if CONFIG_MAIL_WAKEUP_ENABLED
        else if (wpid == MAIL_WAKEUP_ACC)
        {
            buf = (char*)malloc(ETH_PAYLOAD_LEN);
            //headerHtm => %5d => fixed length
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;

            pageSize = genMail_wakeup_accPage(buf + offset);
            
            offset += pageSize;

            //buf[rv] will append 0 for string end
            tmpChar = buf[rv];
            sprintf(buf ,headerHtm, pageSize);
            buf[rv] = tmpChar;
            /* // it's ok
            sendStaticWebPage(pcb, wpid);
            return 0;
            */                              
        }   
        else if (wpid == MAIL_WAKEUP_MAT)
        {
            buf = (char*)malloc(ETH_PAYLOAD_LEN);
            //headerHtm => %5d => fixed length
            memset(buf,0x00,ETH_PAYLOAD_LEN);
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;
	//printf("[RTK] ready to genMail_wakeup_matPage(%d,%d)\n",offset,rv);

            pageSize = genMail_wakeup_matPage(buf + offset);
            
            offset += pageSize;

	//printf("[RTK] page size=%d,offset=%d\n",pageSize,offset);
	
            //buf[rv] will append 0 for string end
            tmpChar = buf[rv];
            sprintf(buf ,headerHtm, pageSize);
            buf[rv] = tmpChar;

	
			
            /* // it's ok
            sendStaticWebPage(pcb, wpid);
            return 0;
            */                              
        }  
#endif		
	else if (wpid == COMPUTERSYSPAGE){
		 buf = (char*)malloc(ETH_PAYLOAD_LEN);
            //headerHtm => %5d => fixed length
            memset(buf,0x00,ETH_PAYLOAD_LEN);
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;
	//printf("[RTK] ready to genMail_wakeup_matPage(%d,%d)\n",offset,rv);

            pageSize = genComputerSystemPage(buf + offset);
            
            offset += pageSize;

	//printf("[RTK] page size=%d,offset=%d\n",pageSize,offset);
	
            //buf[rv] will append 0 for string end
            tmpChar = buf[rv];
            sprintf(buf ,headerHtm, pageSize);
            buf[rv] = tmpChar;

	
			
            /* // it's ok
            sendStaticWebPage(pcb, wpid);
            return 0;
            */                              
	}
	else if (wpid == OPERATIONSYSPAGE){
		 buf = (char*)malloc(ETH_PAYLOAD_LEN);
            
            memset(buf,0x00,ETH_PAYLOAD_LEN);
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;
		//printf("[RTK] ready to genMail_wakeup_matPage(%d,%d)\n",offset,rv);
            pageSize = genOperationSystemPage(buf + offset);            
            offset += pageSize;
            tmpChar = buf[rv];
            sprintf(buf ,headerHtm, pageSize);
            buf[rv] = tmpChar;
	}
	else if (wpid == SOFTWAREIDPAGE){	
		
		buf = (char*)malloc(ETH_PAYLOAD_LEN);
		memset(buf,0x00,ETH_PAYLOAD_LEN);
	#if 1
		
		genSoftwareIdentityPage(pcb,buf);

		 if (buf)
	        	free(buf);
	    	return 0;
	#endif		
		
	/*	
		 
		 
		//calculate total length
		
		rv = sprintf(buf, headerHtm, 0);
		offset += rv;
		 
            
            memset(buf,0x00,ETH_PAYLOAD_LEN);
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;
		//printf("[RTK] ready to genMail_wakeup_matPage(%d,%d)\n",offset,rv);
            pageSize = genSoftwareIdentityPage(buf + offset);            
            offset += pageSize;
            tmpChar = buf[rv];
            sprintf(buf ,headerHtm, pageSize);
            buf[rv] = tmpChar;
        */
	}
	else if (wpid == BIOSPAGE){	
		
		buf = (char*)malloc(ETH_PAYLOAD_LEN);
		memset(buf,0x00,ETH_PAYLOAD_LEN);
	
		
		genBIOSPage(pcb,buf);

		 if (buf)
	        	free(buf);
	    	return 0;
	
	}
	else if (wpid == PROCESSORPAGE){	
		
		buf = (char*)malloc(ETH_PAYLOAD_LEN);
		memset(buf,0x00,ETH_PAYLOAD_LEN);
	
		
		genPROCESSORPage(pcb,buf);

		 if (buf)
	        	free(buf);
	    	return 0;
	
	}
	else if (wpid == MEMORYPAGE){	
		
		buf = (char*)malloc(ETH_PAYLOAD_LEN);
		memset(buf,0x00,ETH_PAYLOAD_LEN);		
		genMemoryPage(pcb,buf);
		 if (buf)
	        	free(buf);
	    	return 0;	
	}
	else if (wpid == FANPAGE){	
		
		buf = (char*)malloc(ETH_PAYLOAD_LEN);
		memset(buf,0x00,ETH_PAYLOAD_LEN);		
		genFanPage(pcb,buf);
		 if (buf)
	        	free(buf);
	    	return 0;	
	}
	else if (wpid == SENSORPAGE){	
		
		buf = (char*)malloc(ETH_PAYLOAD_LEN);
		memset(buf,0x00,ETH_PAYLOAD_LEN);		
		genSensorPage(pcb,buf);
		 if (buf)
	        	free(buf);
	    	return 0;	
	}
	else if (wpid == ASSETPAGE){	
		
		buf = (char*)malloc(ETH_PAYLOAD_LEN);
		memset(buf,0x00,ETH_PAYLOAD_LEN);		
		genAssetPage(pcb,buf);
		 if (buf)
	        	free(buf);
	    	return 0;	
	}
	else if (wpid == BANNERPAGE){
		buf = (char*)malloc(ETH_PAYLOAD_LEN);            
            memset(buf,0x00,ETH_PAYLOAD_LEN);
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;		
            pageSize = genBannerPage(pcb,buf + offset);            
            offset += pageSize;
            tmpChar = buf[rv];
            sprintf(buf ,headerHtm, pageSize);
            buf[rv] = tmpChar;
	}
	else if (wpid == WELCOMEPAGE){
		buf = (char*)malloc(ETH_PAYLOAD_LEN);            
            memset(buf,0x00,ETH_PAYLOAD_LEN);
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;		
            pageSize = genWelcomePage(pcb,buf + offset);            
            offset += pageSize;
            tmpChar = buf[rv];
            sprintf(buf ,headerHtm, pageSize);
            buf[rv] = tmpChar;
	}
	else if (wpid == INFOPAGE){
		buf = (char*)malloc(ETH_PAYLOAD_LEN);
		memset(buf,0x00,ETH_PAYLOAD_LEN);		
		genInfoPage(pcb,buf);
		 if (buf)
	        	free(buf);
	    	return 0;	
	}
	else if (wpid == SHOWCOUNTER){	
		
		buf = (char*)malloc(ETH_PAYLOAD_LEN);
		memset(buf,0x00,ETH_PAYLOAD_LEN);		
		genShowCounterPage(pcb,buf);
		 if (buf)
	        		free(buf);
	    	return 0;	
	}
	else if (wpid == RESETCOUNTER){	
		
		buf = (char*)malloc(ETH_PAYLOAD_LEN);
		memset(buf,0x00,ETH_PAYLOAD_LEN);		
		genClearCounterPage(pcb,buf);
		 if (buf)
	        		free(buf);
	    	return 0;	
	}
        else
        {
            sendStaticWebPage(pcb, wpid);
            return 0;
        }
    }
#ifdef CONFIG_SMBIOS_ENABLED
    //SMBIOS Info
    else if (chkSMBIOSReqPages(url, &type) != 0)
    {
        if ( sendSMBIOSWebPage(pcb, type) == -1)
            goto NOTFOUND;
        else
            return 0;
    }
#endif
    else
    {
NOTFOUND:
        buf = (char*)malloc(ETH_PAYLOAD_LEN);
        pageSize = strlen(notFound);
#ifdef CONFIG_MSAD_ENABLED
        if (dpconf->Security == KERB_AUTH)
        {
            rv = sprintf(buf, headerHtm3, pageSize);
            offset += rv;
            offset += addADauth(buf + offset, &pcb->msad.auth_buffer);
        }
        else
        {
#endif
            rv = sprintf(buf, headerHtm, pageSize);
            offset += rv;
#ifdef CONFIG_MSAD_ENABLED
        }
#endif
        rv = sprintf(buf + offset, notFound, pageSize);
        offset += rv;
    }

    if (pcb->TLSEnable)
    {
        sendTLSWebPage(buf, offset, pcb);
    }
    else
    {
        pkt = (PKT*)allocPkt_F(offset);
        memcpy(pkt->wp, buf, offset);
        tcpSendData_F(pcb, pkt);
    }

    if (buf)
        free(buf);
    return 0;
}
