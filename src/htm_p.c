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

extern void chkBufSize(void *p, int offset);
extern smbiosrmcpdata *smbiosrmcpdataptr;
extern DPCONF *dpconf;

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
-->\n\
</style>\n"
};

//ht 20110713 : modify for new web page
const static unsigned char bannerHtm[] =
{
"<table><td><img src=\"logo.gif\" width=\"48\" height=\"48\"></td>\n\
<tr><td><H2><font color=blue font face=\"Monotype Corsiva, Arial\" font size=4>Realtek Semiconductor Corp.</td>\n\
<tr><td><a target=\"contents\" href=\"jobinfo.htm\" Onclick=\"top.frames[2].location='joblist.htm'\">Jobs</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"statusinfo.htm\" Onclick=\"top.frames[2].location='general.htm'\">Status</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"propertyinfo.htm\" Onclick=\"top.frames[2].location='machine.htm'\">Properties</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"mainteninfo.htm\" Onclick=\"top.frames[2].location='errorhistory.htm'\">Maintenance</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"wakeupinfo.htm\" Onclick=\"top.frames[2].location='eventLog.htm'\">Wake Up Patterns</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"remoteinfo.htm\" Onclick=\"top.frames[2].location='remoteCtl.htm'\">Remote Power Management</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"supportinfo.htm\" Onclick=\"top.frames[2].location='support.htm'\">Support</a></td></table>\n"
};

//ht 20110810 : it's ok, need to open jobinfo (and others) javascript
/*
"<table><td><img src=\"logo.gif\" width=\"48\" height=\"48\"></td>\n\
<tr><td><H2><font color=blue font face=\"Monotype Corsiva, Arial\" font size=4>Realtek Semiconductor Corp.</td>\n\
<tr><td><a target=\"contents\" href=\"jobinfo.htm\">Jobs</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"statusinfo.htm\">Status</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"propertyinfo.htm\">Properties</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"mainteninfo.htm\">Maintenance</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"wakeupinfo.htm\">Wake Up Patterns</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<a target=\"contents\" href=\"supportinfo.htm\">Support</a></td></table>\n"
*/

/*
//ht 20110808
const static unsigned char joballHtm[] =
{
"<html>\n\
<head>\n\
<title>Realtek Printer Setup Tool</title>\n\
</head>\n\
<frameset rows=\"115,*\">\n\
<frame name=\"banner\" scrolling=\"no\" src=\"banner.htm\">\n\
<frameset cols=\"190,*\">\n\
<frame name=\"contents\" src=\"jobinfo.htm\">\n\
<frame name=\"main\" src=\"joblist.htm\" scrolling=\"auto\">\n\
</frameset>\n\
</frameset>\n\
</html>\n"
};

//ht 20110808
const static unsigned char statusallHtm[] =
{
"<html>\n\
<head>\n\
<title>Realtek Printer Setup Tool</title>\n\
</head>\n\
<frameset rows=\"115,*\">\n\
<frame name=\"banner\" scrolling=\"no\" src=\"banner.htm\">\n\
<frameset cols=\"190,*\">\n\
<frame name=\"contents\" src=\"statusinfo.htm\">\n\
<frame name=\"main\" src=\"general.htm\" scrolling=\"auto\">\n\
</frameset>\n\
</frameset>\n\
</html>\n"
};

//ht 20110808
const static unsigned char propertyallHtm[] =
{
"<html>\n\
<head>\n\
<title>Realtek Printer Setup Tool</title>\n\
</head>\n\
<frameset rows=\"115,*\">\n\
<frame name=\"banner\" scrolling=\"no\" src=\"banner.htm\">\n\
<frameset cols=\"190,*\">\n\
<frame name=\"contents\" src=\"propertyinfo.htm\">\n\
<frame name=\"main\" src=\"machine.htm\" scrolling=\"auto\">\n\
</frameset>\n\
</frameset>\n\
</html>\n"
};

//ht 20110808
const static unsigned char maintenallHtm[] =
{
"<html>\n\
<head>\n\
<title>Realtek Printer Setup Tool</title>\n\
</head>\n\
<frameset rows=\"115,*\">\n\
<frame name=\"banner\" scrolling=\"no\" src=\"banner.htm\">\n\
<frameset cols=\"190,*\">\n\
<frame name=\"contents\" src=\"mainteninfo.htm\">\n\
<frame name=\"main\" src=\"errorhistory.htm\" scrolling=\"auto\">\n\
</frameset>\n\
</frameset>\n\
</html>\n"
};

//ht 20110808
const static unsigned char supportallHtm[] =
{
"<html>\n\
<head>\n\
<title>Realtek Printer Setup Tool</title>\n\
</head>\n\
<frameset rows=\"115,*\">\n\
<frame name=\"banner\" scrolling=\"no\" src=\"banner.htm\">\n\
<frameset cols=\"190,*\">\n\
<frame name=\"contents\" src=\"supportinfo.htm\">\n\
<frame name=\"main\" src=\"support.htm\" scrolling=\"auto\">\n\
</frameset>\n\
</frameset>\n\
</html>\n"
};

//ht 20110809
const static unsigned char wakeupallHtm[] =
{
"<html>\n\
<head>\n\
<title>Realtek Printer Setup Tool</title>\n\
</head>\n\
<frameset rows=\"115,*\">\n\
<frame name=\"banner\" scrolling=\"no\" src=\"banner.htm\">\n\
<frameset cols=\"190,*\">\n\
<frame name=\"contents\" src=\"wakeupinfo.htm\">\n\
<frame name=\"main\" src=\"eventLog.htm\" scrolling=\"auto\">\n\
</frameset>\n\
</frameset>\n\
</html>\n"
};
*/

//ht 20110713
const static unsigned char homeHtm[] =
{
"<html>\n\
<head>\n\
<title>Realtek Printer Setup Tool</title>\n\
</head>\n\
<frameset rows=\"115,*\">\n\
<frame name=\"banner\" scrolling=\"no\" target=\"contents\" src=\"banner.htm\">\n\
<frameset cols=\"190,*\">\n\
<frame name=\"contents\" target=\"main\" src=\"jobinfo.htm\">\n\
<frame name=\"main\" src=\"joblist.htm\" scrolling=\"auto\">\n\
</frameset>\n\
</frameset>\n\
</html>\n"
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
"<H2>Welcome To Use Realtek\n\
<H2>Gigabit Ethernet Controller\n\
<H2>With Dash Technology.\n"
#else
"<H2>Welcome To Use Realtek\n\
<H2>Gigabit Ethernet Controller\n\
<H2>With Offload Engine.\n"
#endif
"</body>\n\
</html>\n"
};

//ht 20110713
const static unsigned char jobinfoHtm[] =
{
"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<STYLE type=text/css>BODY{MARGIN: 6px}\n\
A{FONT-WEIGHT: bold; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial; TEXT-DECORATION: none}\n\
BIG {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #09c; FONT-FAMILY: Calibri, Arial}\n\
P{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
TABLE{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
td{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
</STYLE>\n"
/*<script language=javascript>window.top.frames[2].location.href='joblist.htm'</script>\n\*/
"<BODY leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">\n\
<TABLE cellSpacing=2 cellPadding=1 border=0>\n\
<TBODY>\n\
<tr><td vAlign=top></td></tr>\n\
<tr><td colSpan=2></td></tr>\n\
<td colSpan=2><BIG>Realtek Printer</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"joblist.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Job List</u></font></td><tr>\n\
<td colSpan=2><font color=green>&nbsp;&nbsp;Job History List</font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"jobreport.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>Job History Report</u></font></td><tr>\n"
"<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};

//ht 20110803
const static unsigned char mainteninfoHtm[] =
{
"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<STYLE type=text/css>BODY{MARGIN: 6px}\n\
A{FONT-WEIGHT: bold; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial; TEXT-DECORATION: none}\n\
BIG {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #09c; FONT-FAMILY: Calibri, Arial}\n\
P{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
TABLE{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
td{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
</STYLE>\n"
/*<script language=javascript>window.top.frames[2].location.href='errorhistory.htm'</script>\n\*/
"<BODY leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">\n\
<TABLE cellSpacing=2 cellPadding=1 border=0>\n\
<TBODY>\n\
<tr><td vAlign=top></td></tr>\n\
<tr><td colSpan=2></td></tr>\n\
<td colSpan=2><BIG>Realtek Printer</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"errorhistory.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Error History Report</u></font></td><tr>\n"
"<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};

//ht 20110803
const static unsigned char propertyinfoHtm[] =
{
"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<STYLE type=text/css>BODY{MARGIN: 6px}\n\
A{FONT-WEIGHT: bold; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial; TEXT-DECORATION: none}\n\
BIG {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #09c; FONT-FAMILY: Calibri, Arial}\n\
P{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
TABLE{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
td{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
</STYLE>\n"
/*<script language=javascript>window.top.frames[2].location.href='machine.htm'</script>\n\*/
"<BODY leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">\n\
<TABLE cellSpacing=2 cellPadding=1 border=0>\n\
<TBODY>\n\
<tr><td vAlign=top></td></tr>\n\
<tr><td colSpan=2></td></tr>\n\
<td colSpan=2><BIG>Realtek Printer</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"machine.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Machine Details</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"memory.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Configuration</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"counter.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Counters</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"papertrayattri.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Paper Tray Attributes</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"papersetting.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Paper Settings</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"powersetting.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Power Saver Settings</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"internetsetting.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Internet Services Settings</u></font></td><tr>\n\
<td colSpan=2><font color=green>&nbsp;&nbsp;Port Settings</font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"ethernet.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>Ethernet</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"usb.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>USB</u></font></td><tr>\n\
<td colSpan=2><font color=green>&nbsp;&nbsp;Protocol Settings</font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"smb.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>SMB</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"tcpip.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>TCP/IP</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"lpd.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>LPD</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"snmp.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>SNMP</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"ipp.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>IPP</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"port9100.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>Port9100</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"http.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>HTTP</u></font></td><tr>\n\
<td colSpan=2><font color=green>&nbsp;&nbsp;Emulation Settings</font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"printmode.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>Print Mode</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"postscript.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>PostScript</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"hpgl2.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>HP-GL/2</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"tiff.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>TIFF</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"pcl.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>PCL</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"memorysetting.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Memory Settings</u></font></td><tr>\n"
"<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};

//ht 20110803
const static unsigned char statusinfoHtm[] =
{
"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<STYLE type=text/css>BODY{MARGIN: 6px}\n\
A{FONT-WEIGHT: bold; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial; TEXT-DECORATION: none}\n\
BIG {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #09c; FONT-FAMILY: Calibri, Arial}\n\
P{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
TABLE{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
td{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
</STYLE>\n"
/*<script language=javascript>window.top.frames[2].location.href='general.htm'</script>\n\*/
"<META HTTP-EQUIV=\"Refresh\" CONTENT=\"number of seconds to wait until refresh\" URL=\"general.htm\" TARGET=\"main\">\n\
<BODY leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">\n\
<TABLE cellSpacing=2 cellPadding=1 border=0>\n\
<TBODY>\n\
<tr><td vAlign=top></td></tr>\n\
<tr><td colSpan=2></td></tr>\n\
<td colSpan=2><BIG>Realtek Printer</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"general.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>General</u></font></td><tr>\n\
<td colSpan=2><font color=green>&nbsp;&nbsp;Trays</font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"papertray.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>Paper Tray</u></font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"outputtray.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>Output Tray</u></font></td><tr>\n\
<td colSpan=2><font color=green>&nbsp;&nbsp;Consumables</font></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"drum.htm\"><font color=\"#000000\">&nbsp;&nbsp;&nbsp;&nbsp;<u>Drum/Toner Cartridge</u></font></td><tr>\n\
<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};

//ht 20110803
const static unsigned char supportinfoHtm[] =
{
"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<STYLE type=text/css>BODY{MARGIN: 6px}\n\
A{FONT-WEIGHT: bold; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial; TEXT-DECORATION: none}\n\
BIG {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #09c; FONT-FAMILY: Calibri, Arial}\n\
P{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
TABLE{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
td{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
</STYLE>\n"
/*<script language=javascript>window.top.frames[2].location.href='support.htm'</script>\n\*/
"<BODY leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">\n\
<TABLE cellSpacing=2 cellPadding=1 border=0>\n\
<TBODY>\n\
<tr><td vAlign=top></td></tr>\n\
<tr><td colSpan=2></td></tr>\n\
<td colSpan=2><BIG>Realtek Printer</BIG></td><tr>\n\
<td colSpan=2><font color=\"#000000\">&nbsp;&nbsp;Name: </font><font color=blue>Realtek Printer</font></td><tr>\n\
<td colSpan=2><font color=\"#000000\">&nbsp;&nbsp;IP Address: </font><font color=blue>192.168.0.10</font></td><tr>\n\
<td colSpan=2><font color=\"#000000\">&nbsp;&nbsp;Product Name: </font><font color=blue>Realtek Printer</font></td><tr>\n\
<td colSpan=2><font color=\"#000000\">&nbsp;&nbsp;Location: </font><font color=blue>-</font></td><tr>\n\
<td colSpan=2><font color=\"#000000\">&nbsp;&nbsp;Status: </font><font color=blue>Ready</font></td><tr>\n\
<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};

//ht 20110803
const static unsigned char remoteinfoHtm[] =
{
"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<STYLE type=text/css>BODY{MARGIN: 6px}\n\
A{FONT-WEIGHT: bold; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial; TEXT-DECORATION: none}\n\
BIG {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #09c; FONT-FAMILY: Calibri, Arial}\n\
P{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
TABLE{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
td{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
</STYLE>\n"
/*<script language=javascript>window.top.frames[2].location.href='support.htm'</script>\n\*/
"<BODY leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">\n\
<TABLE cellSpacing=2 cellPadding=1 border=0>\n\
<TBODY>\n\
<tr><td vAlign=top></td></tr>\n\
<tr><td colSpan=2></td></tr>\n\
<td colSpan=2><BIG>Realtek Printer</BIG></td><tr>\n\
<td colSpan=2><font color=\"#000000\">&nbsp;&nbsp;Name: </font><font color=blue>Realtek Printer</font></td><tr>\n\
<td colSpan=2><font color=\"#000000\">&nbsp;&nbsp;Current Status: </font><font color=blue>Ready</font></td><tr>\n\
<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};


//ht 20110809
const static unsigned char wakeupinfoHtm[] =
{
"<HTML><HEAD><TITLE>Realtek</TITLE>\n\
<META http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<STYLE type=text/css>BODY{MARGIN: 6px}\n\
A{FONT-WEIGHT: bold; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial; TEXT-DECORATION: none}\n\
BIG {FONT-WEIGHT: bold; FONT-SIZE: 12pt; MARGIN: 0pt; COLOR: #09c; FONT-FAMILY: Calibri, Arial}\n\
P{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
TABLE{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
td{FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: white; FONT-FAMILY: Calibri, Arial}\n\
</STYLE>\n"
/*<script language=javascript>window.top.frames[2].location.href='eventLog.htm'</script>\n\*/
"<BODY leftMargin=0 topMargin=0 marginheight=\"0\" marginwidth=\"0\">\n\
<TABLE cellSpacing=2 cellPadding=1 border=0>\n\
<TBODY>\n\
<tr><td vAlign=top></td></tr>\n\
<tr><td colSpan=2></td></tr>\n\
<td colSpan=2><BIG>Realtek Printer</BIG></td><tr>\n\
<td colSpan=2><a target=\"main\" href=\"eventLog.htm\"><font color=\"#000000\">&nbsp;&nbsp;<u>Wake Up Patterns Setting</u></font></td><tr>\n\
<tr><td vAlign=top></td><td colSpan=2></tr>\n\
</TBODY></TABLE></BODY></HTML>\n"
};

//ht 20110714
const static unsigned char joblistHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job List</title></head>\n\
<body><p><b><font size=\"5\">Job List</font></b></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr><td><p align=\"center\"><font size=\"4\">There are no jobs.</font></td>\n\
</tr></table></body></html>\n"
};

//ht 20110802
const static unsigned char jobreportHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body><p><b><font size=\"5\">Job History Report</font></b></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"122\">Job Name</td>\n\
<td width=\"62\">Owner</td>\n\
<td width=\"90\">Job Status</td>\n\
<td width=\"75\">Job Type</td>\n\
<td width=\"47\">Pages</td>\n\
<td width=\"157\">Output Destination</td>\n\
<td width=\"122\">Host Interface</td>\n\
<td>Data/Time Completed</td>\n\
</tr>\n\
<tr>\n\
<td width=\"122\">estatement.pdf</td>\n\
<td width=\"62\">phinex</td>\n\
<td width=\"90\">Completed</td>\n\
<td width=\"75\">Print</td>\n\
<td width=\"47\">2</td>\n\
<td width=\"157\">Center Output Tray</td>\n\
<td width=\"122\">LPD</td>\n\
<td>2011/07/04 08:56 PM</td>\n\
</tr>\n\
<tr>\n\
<td width=\"122\">estatement.pdf</td>\n\
<td width=\"62\">phinex</td>\n\
<td width=\"90\">Completed</td>\n\
<td width=\"75\">Print</td>\n\
<td width=\"47\">0</td>\n\
<td width=\"157\">-</td>\n\
<td width=\"122\">LPD</td>\n\
<td>2011/07/04 08:55 PM</td>\n\
</tr>\n\
</table>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char generalHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>General Information</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Name</td>\n\
<td width=\"389\">Realtek Printer</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">IP Address</td>\n\
<td width=\"389\">192.168.0.10</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Product Name</td>\n\
<td width=\"389\">Realtek Printer</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Location</td>\n\
<td width=\"389\">-</td>\n\
</tr>\n\
</table>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char papertraysHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title>\n\
</head>\n\
<body>\n\
<p><font size=\"5\"><b>Paper Tray</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"140\">Trays</td>\n\
<td width=\"112\">Status</td>\n\
<td width=\"167\">Paper Remaining</td>\n\
<td width=\"114\">Paper Size</td>\n\
<td width=\"129\">Paper Type</td>\n\
<td width=\"50\">Priority</td>\n\
</tr>\n\
<tr>\n\
<td width=\"140\">Tray1</td>\n\
<td width=\"112\">Ready</td>\n\
<td width=\"167\">100%</td>\n\
<td width=\"114\">A4</td>\n\
<td width=\"129\">Plain</td>\n\
<td width=\"50\">2</td>\n\
</tr>\n\
<tr>\n\
<td width=\"140\">Tray2</td>\n\
<td width=\"112\">Ready</td>\n\
<td width=\"167\">100%</td>\n\
<td width=\"114\">A4</td>\n\
<td width=\"129\">Plain</td>\n\
<td width=\"50\">1</td>\n\
</tr></table></body></html>\n"
};

//ht 20110803
const static unsigned char outputtraysHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Output Tray</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Trays</td>\n\
<td width=\"389\">Status</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Center Output Tray</td>\n\
<td width=\"389\">Ready</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Rear Output Tray</td>\n\
<td width=\"389\">Ready</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Offset Catch Tray</td>\n\
<td width=\"389\">Ready</td>\n\
</tr>\n\
</table></body></html>\n"
};

//ht 20110803
const static unsigned char drumHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Drum/Toner Cartridge</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Drum/Toner Cartridge</td>\n\
<td width=\"389\">Status</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Black</td>\n\
<td width=\"389\">Ready</td>\n\
</tr>\n\
</table></body></html>\n"
};

//ht 20110803
const static unsigned char machineHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title>\n\
</head>\n\
<body>\n\
<p><font size=\"5\"><b>Machine Information</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Product Name</td>\n\
<td width=\"389\">&nbsp;Realtek Printer</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Serial No.</td>\n\
<td width=\"389\">&nbsp;125071</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Name</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T8\" size=\"42\"></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Location</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T9\" size=\"42\"></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Contact Person</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T10\" size=\"42\"></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Administrator E-mail Address</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T11\" size=\"42\"></td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char memoryHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Memory</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">RAM Size</td>\n\
<td width=\"389\">192MBytes</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">USB</td>\n\
<td width=\"389\">64KBytes</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">SMB</td>\n\
<td width=\"389\">256KBytes</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">LPD</td>\n\
<td width=\"389\">1024KBytes</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">IPP</td>\n\
<td width=\"389\">256KBytes</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Port9100</td>\n\
<td width=\"389\">256KBytes</td>\n\
</tr>\n\
</table></body></html>\n"
};

//ht 20110803
const static unsigned char counterHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Counters</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Total Impressions</td>\n\
<td width=\"389\">162245</td>\n\
</tr>\n\
</table></body></html>\n"
};

//ht 20110803
const static unsigned char papertrayattriHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Paper Tray Attributes</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Trays</td>\n\
<td width=\"389\">Paper Type</td>\n\
<td width=\"389\">&nbsp;Priority</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Tray 1</td>\n\
<td width=\"389\">Plain</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D1\">\n\
<option>1</option>\n\
<option selected>2</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Tray 2</td>\n\
<td width=\"389\">Plain</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D2\">\n\
<option selected>1</option>\n\
<option>2</option>\n\
</select></td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char papersettingHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Paper Settings</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Paper Type</td>\n\
<td width=\"389\">Paper Name</td>\n\
<td width=\"389\">&nbsp;Priority</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Plain</td>\n\
<td width=\"389\">-</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D1\">\n\
<option selected>1</option>\n\
<option>2</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Recycled</td>\n\
<td width=\"389\">-</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D2\">\n\
<option>1</option>\n\
<option selected>2</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Heavyweight 1</td>\n\
<td width=\"389\">-</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D3\">\n\
<option>On</option>\n\
<option selected>Off</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Heavyweight 2</td>\n\
<td width=\"389\">-</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D8\">\n\
<option>On</option>\n\
<option selected>Off</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Custom Paper 1</td>\n\
<td width=\"389\">-</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D9\">\n\
<option>On</option>\n\
<option selected>Off</option>\n\
</select></td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char powersettingHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Power Saver Settings</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Low Power Mode Time-Out</td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C2\" value=\"ON\" checked>Enable&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<input type=\"text\" name=\"T1\" size=\"7\" value=\"5\">Minutes (0 - 60)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Sleep Mode Time-Out</td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C3\" value=\"ON\" checked>Enable&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<input type=\"text\" name=\"T7\" size=\"7\" value=\"10\">Minutes (0 - 60)</td>\n\
</tr>\n\
</table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char internetsettingHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Internet Service Settings</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Auto Refresh Interval</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T1\" size=\"6\" value=\"120\">Seconds (0:Void, 20 - 600)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Display Language</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D1\">\n\
<option selected>English</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Administrator Mode</td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C1\" value=\"ON\" checked>Enable</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Administrator Name</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T5\" size=\"27\" value=\"Administrator\"></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Administrator Password</td>\n\
<td width=\"389\">&nbsp;<input type=\"password\" name=\"T6\" size=\"26\" value=\"Realtek\"></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Confirm Password</td>\n\
<td width=\"389\">&nbsp;<input type=\"password\" name=\"T4\" size=\"26\" value=\"Realtek\"></td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char portstatusHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Port Status</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Port</td>\n\
<td width=\"389\">Enable</td>\n\
<td width=\"389\">&nbsp;Transport Protocol</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Parallel</td>\n\
<td width=\"389\"><input type=\"checkbox\" name=\"C1\" value=\"ON\">Enable</td>\n\
<td width=\"389\">&nbsp;</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">USB</td>\n\
<td width=\"389\"><input type=\"checkbox\" name=\"C2\" value=\"ON\" checked>Enable</td>\n\
<td width=\"389\">&nbsp;</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">EtherTalk</td>\n\
<td width=\"389\"><input type=\"checkbox\" name=\"C3\" value=\"ON\">Enable</td>\n\
<td width=\"389\">&nbsp;</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">SMB</td>\n\
<td width=\"389\"><input type=\"checkbox\" name=\"C4\" value=\"ON\" checked>Enable</td>\n\
<td width=\"389\"><input type=\"checkbox\" name=\"C6\" value=\"ON\" checked>TCP/IP\n\
<input type=\"checkbox\" name=\"C7\" value=\"ON\" checked>NetBEUI</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">NetWare</td>\n\
<td width=\"389\"><input type=\"checkbox\" name=\"C5\" value=\"ON\">Enable</td>\n\
<td width=\"389\"><input type=\"checkbox\" name=\"C8\" value=\"ON\" checked>IPX/SPX<input type=\"checkbox\" name=\"C9\" value=\"ON\" checked>TCP/IP</td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char ethernetHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Ethernet</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Ethernet Settings</td>\n\
<td width=\"389\"><select size=\"1\" name=\"ethernet\">\n\
<option selected>Auto</option>\n\
<option>100BASE-TX</option>\n\
<option>10BASE-T</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">MAC Address</td>\n"
/*<td width=\"389\">08:00:37:86:5C:0B</td>\n\*/
"<td width=\"389\">%s</td>\n\
</tr>\n\
</table>\n\
<blockquote><blockquote><blockquote><blockquote><blockquote><blockquote><blockquote>\n\
<p><input type=\"button\" value=\"Apply New Settings\" name=\"B3\"></p>\n\
<p>&nbsp;&nbsp;&nbsp;<input type=\"submit\" value=\"Restore Settings\" name=\"B1\"></p>\n\
</blockquote></blockquote></blockquote></blockquote></blockquote></blockquote></blockquote>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char usbHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>USB</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Auto Eject Time</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T1\" size=\"6\" value=\"30\">Seconds\n\
(5 - 1275)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Adobe Communication Protocol</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D1\">\n\
<option selected>Normal</option>\n\
</select></td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char smbHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<form method=\"POST\" action=\"smbSet\">\n\
<p><font size=\"5\"><b>SMB</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Workgroup</td>\n\
<td width=\"389\">&nbsp;<input type=text name=\"Workname\" size=34 value=\"WORKGROUP\"></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Host Name</td>\n"
/*<td width=\"389\">&nbsp;<input type=\"text\" name=\"T10\" size=\"34\" value=\"FX-865C0B\"></td>\n\*/
"<td width=\"389\">&nbsp;<input type=text name=\"Hostname\" value=%s size=34></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Administrator Name</td>\n"
/*<td width=\"389\">&nbsp;<input type=\"text\" name=\"T11\" size=\"34\" value=\"ADMIN\"></td>\n\*/
"<td width=\"389\">&nbsp;<input type=text name=\"Adminname\" value=%s size=34></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Administrator Password</td>\n"
/*<td width=\"389\">&nbsp;<input type=\"password\" name=\"T12\" size=\"33\" value=\"Realtek\"></td>\n\*/
"<td width=\"389\">&nbsp;<input type=password name=\"Passname\" value=%s size=33></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Confirm Password</td>\n"
/*<td width=\"389\">&nbsp;<input type=\"password\" name=\"T13\" size=\"33\" value=\"Realtek\"></td>\n\*/
"<td width=\"389\">&nbsp;<input type=password name=\"Confname\" value=%s size=33></td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\"></center></p>\n\
</form></body></html>\n"
};

//ht 20110803
const static unsigned char tcpipHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>TCPIP Setting</title></head>\n\
<body>\n\
<form method=\"POST\" action=\"netSet\">\n\
<p><font size=\"5\"><b>TCP/IP</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Host Name</td>\n"                   
/*<td width=\"389\">&nbsp;<input type=text name=\"Hostname\" value=%s size=34></td>\n\*/
"<td width=\"389\">&nbsp;%s</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Get IP Address</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"method\">\n\
<option selected>Manual</option>\n\
<option>DHCP</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">IP Address</td>\n\
<td width=\"389\">&nbsp;<input type=text name=\"v4Addr\" value=%s size=34 maxlength=15></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Subnet Mask</td>\n\
<td width=\"389\">&nbsp;<input type=text name=\"v4Mask\" value=%s size=34 maxlength=15></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Gateway Address</td>\n\
<td width=\"389\">&nbsp;<input type=text name=\"v4Gateway\" value=%s size=34 maxlength=15></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">DNS Server</td>\n\
<td width=\"389\">&nbsp;<input type=text name=\"v4DNS\" value=%s size=34 maxlength=15></td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</form></body></html>\n"
};

//ht 20110803
const static unsigned char lpdHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>LPD</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Port Number</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T9\" size=\"6\" value=\"515\">(515,	80, 8000 - 9999)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">TBCP Filter</td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C1\" value=\"ON\">Enable</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Connection Time-Out</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T8\" size=\"6\" value=\"16\">Seconds (2 - 3600)</td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char snmpHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>SNMP</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Community Name (Read Only)</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T9\" size=\"34\"></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Community Name (Read/Write)</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T10\" size=\"34\"></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Community Name (Trap Notification)</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T11\" size=\"34\"></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">System Administrator</td>\n\
<td width=\"389\">&nbsp;<input type=\"password\" name=\"T12\" size=\"33\"></td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char ippHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>IPP</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Port Number</td>\n\
<td width=\"389\">&nbsp;631</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Add Port No.</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T7\" size=\"6\" value=\"80\">(0:Void, 80, 8000 - 9999)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">TBCP Filter</td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C1\" value=\"ON\">Enable</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Access Control</td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C2\" value=\"ON\">Enable</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">DNS</td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C3\" value=\"ON\" checked>Enable</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Connection Time-Out</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T8\" size=\"6\" value=\"60\">Seconds (0 - 65535)</td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char port9100Htm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Port9100</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Port Number</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T9\" size=\"6\" value=\"9100\">(8000 - 9999)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">TBCP Filter</td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C1\" value=\"ON\">Enable</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Connection Time-Out</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T8\" size=\"6\" value=\"60\">Seconds (2 - 65535)</td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char httpHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>HTTP</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Port Number</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T1\" size=\"6\" value=\"80\">(80, 8000 - 9999)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Maximum Sessions</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T2\" size=\"6\" value=\"5\">(1 - 15)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Connection Time-Out</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T3\" size=\"6\" value=\"30\">Seconds (1 - 255)</td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char printmodeHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Print Mode</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">USB</td>\n\
<td width=\"389\"><select size=\"1\" name=\"D9\">\n\
<option selected>Auto</option>\n\
</select></td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C3\" value=\"ON\" checked>PJL </td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">SMB</td>\n\
<td width=\"389\"><select size=\"1\" name=\"D1\">\n\
<option selected>Auto</option>\n\
</select></td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C2\" value=\"ON\" checked>PJL </td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">LPD</td>\n\
<td width=\"389\"><select size=\"1\" name=\"D10\">\n\
<option selected>Auto</option>\n\
</select></td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C4\" value=\"ON\" checked>PJL </td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">IPP</td>\n\
<td width=\"389\"><select size=\"1\" name=\"D11\">\n\
<option selected>Auto</option>\n\
</select></td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C5\" value=\"ON\" checked>PJL </td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Port9100</td>\n\
<td width=\"389\"><select size=\"1\" name=\"D12\">\n\
<option selected>Auto</option>\n\
</select></td>\n\
<td width=\"389\">&nbsp;<input type=\"checkbox\" name=\"C6\" value=\"ON\" checked>PJL </td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char postscriptHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>PostScript</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Logical Printer Number</td>\n\
<td width=\"389\">Number <input type=\"text\" name=\"T1\" size=\"6\" value=\"1\">\n\
(1-10)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<input type=\"button\" value=\"Send\" name=\"B3\"></td>\n\
</tr></table>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char hpgl2Htm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>HP-GL/2</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Logical Printer Number</td>\n\
<td width=\"389\">Number <input type=\"text\" name=\"T1\" size=\"6\" value=\"1\">\n\
(1-10)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<input type=\"button\" value=\"Send\" name=\"B3\"></td>\n\
</tr></table>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char tiffHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>TIFF</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Logical Printer Number</td>\n\
<td width=\"389\">Number <input type=\"text\" name=\"T1\" size=\"6\" value=\"1\">\n\
(1-10)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
<input type=\"button\" value=\"Send\" name=\"B3\"></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Memory Settings</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D1\">\n\
<option selected>Factory settings</option>\n\
</select></td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char pclHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>PCL</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Paper Tray</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D2\">\n\
<option selected>Auto</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Output Size</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D1\">\n\
<option selected>A4</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Output Destination</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D3\">\n\
<option selected>Center Output Tray</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Quantity</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T5\" size=\"7\" value=\"1\">(1 - 999)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">2 Sided Print</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D4\">\n\
<option selected>1 Sided Print</option>\n\
</select></td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Image Orientation</td>\n\
<td width=\"389\">&nbsp;<select size=\"1\" name=\"D5\">\n\
<option selected>Portrait</option>\n\
</select></td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char memorysettingHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Memory Settings</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Free Space : 10518KB</td>\n\
<td width=\"389\">&nbsp;Print Page Buffer 71032KB</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Spool</td>\n\
<td width=\"389\">&nbsp;Total Size</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">Parallel</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T7\" size=\"14\" value=\"64\">KB&nbsp;&nbsp;(64 - 1024)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">USB</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T11\" size=\"14\" value=\"64\">KB&nbsp;&nbsp;(64 - 1024)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">EtherTalk</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T12\" size=\"14\" value=\"1024\">KB&nbsp;&nbsp;(1024 - 2048)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">SMB <select size=\"1\" name=\"D1\">\n\
<option>On</option>\n\
<option selected>Off</option>\n\
</select></td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T13\" size=\"14\" value=\"256\">KB&nbsp;&nbsp;(64 - 1024: Off) (512 - 32768: Memory)</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">NetWare</td>\n\
<td width=\"389\">&nbsp;<input type=\"text\" name=\"T14\" size=\"14\" value=\"256\">KB&nbsp;&nbsp;(64 - 1024)</td>\n\
</tr></table>\n\
<p><center><input type=\"submit\" value=\"Apply New Settings\" name=\"B1\"></center></p>\n\
<p><center><input type=\"submit\" value=\"Restore Settings\" name=\"B2\"></center></p>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char errorhistoryHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Error</b></font><b><font size=\"5\"> History Report</font></b></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Date / Time</td>\n\
<td width=\"389\">Error Code</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">2011/05/27 03:36 PM</td>\n\
<td width=\"389\">008-155</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">2011/05/26 04:29 PM</td>\n\
<td width=\"389\">010-105</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">2011/04/06 07:44 AM</td>\n\
<td width=\"389\">010-105</td>\n\
</tr>\n\
<tr>\n\
<td width=\"335\">2011/04/06 07:06 AM</td>\n\
<td width=\"389\">008-156</td>\n\
</tr></table>\n\
</body></html>\n"
};

//ht 20110803
const static unsigned char supportHtm[] =
{
"<html><head>\n\
<meta http-equiv=\"Content-Language\" content=\"zh-tw\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>Job History Report</title></head>\n\
<body>\n\
<p><font size=\"5\"><b>Support</b></font></p>\n\
<table width=\"740\" id=\"table1\" style=\"border: 3px double #000000; padding: 0\">\n\
<tr>\n\
<td width=\"335\">Customer Support</td>\n\
<td width=\"389\"><a target=_top href=\"http://www.realtek.com\">www.realtek.com</a></td>\n\
</tr></table>\n\
</body></html>\n"
};

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
const static unsigned char remoteCtlHtmTop1[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<STYLE>P{COLOR:6FD0FF;FONT-FAMILY:Comic Sans MS,Arial}</STYLE>\n\
<html>\n\
<head><H2>Power managment</H2></head>\n"
};
//<p><H3>Current Power State : XX </H3></p>\n"
//};

const static unsigned char remoteCtlHtmTop2[] =
{
"<body>\n\
<hr>\n\
<table>\n\
<form method=\"POST\" action=\"/remoteCtl\">\n\
<font COLOR=\"FF9900\", font face=\"Comic Sans MS, Arial\">Power Control</font>\n\
<p><input type=\"radio\" name=\"power\" value=\"reset\" checked>Reset</p>\n\
<p><input type=\"radio\" name=\"power\" value=\"up\">Power up</p>\n\
<p><input type=\"radio\" name=\"power\" value=\"off\">Power off</p>"
};

const static unsigned char remoteCtlHtmBottom[] =
{
"</table>\n\
<hr><p></p>\n\
<input type=submit value=\"Submit\">\n\
</form>\n\
</body>\n\
</html>"
};

#else
const static unsigned char remoteCtlHtmTop[] =
{
 "<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<STYLE>P{COLOR:6FD0FF;FONT-FAMILY:Comic Sans MS,Arial}</STYLE>\n\
<html>\n\
<head><H2>Power managment</H2></head>\n\
<body>\n\
<hr>\n\
<table>\n\
<td width=10></td>\n\
<td valign = top>\n\
<form method=\"POST\" action=\"/remoteCtl\">\n\
<font COLOR=\"FF9900\", font face=\"Comic Sans MS, Arial\">Power Control</font>\n\
<p><input type=\"radio\" name=\"power\" value=\"reset\" checked>Reset</p>\n\
<p><input type=\"radio\" name=\"power\" value=\"up\">Power up</p>\n\
<p><input type=\"radio\" name=\"power\" value=\"off\">Power off</p>\n\
<p><input type=\"radio\" name=\"power\" value=\"cycle\">Power Cycle Reset</p>\n\
</td>\n\
<td width=30></td>\n\
<td valign=top>\n\
<font COLOR=\"FF9900\", font face=\"Comic Sans MS, Arial\">Boot Device</font>"
};

const static unsigned char remoteCtlHtmBottom[] =
{
"</td>\n\
<tr><tr><td></td><td></td>\n\
</table>\n\
<hr><p></p>\n\
<input type=submit value=\"Submit\">\n\
</form>\n\
</body>\n\
</html>"
};
#endif

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

const static unsigned char netSetIPv4Htm[] =
{
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n\
<STYLE type=text/css>\n\
p{COLOR: #FF9900; FONT-FAMILY:Comic Sans MS, Arial}\n\
td{COLOR:#6FD0FF; FONT-FAMILY: Comic Sans MS, Arial}\n\
</STYLE>\n\
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
<td width=20></td><td>IP Address:</td><td><input type=text name=\"v4Addr\" %s value=%s size=16 maxlength=15></td><tr>\n\
<td></td><td>Subnet Mask:</td><td><input type=text name=\"v4Mask\" %s value=%s size=16 maxlength=15></td><tr>\n\
<td></td><td>Gateway:</td><td><input type=text name=\"v4Gateway\" %s value=%s size=16 maxlength=15></td><tr>\n\
<td></td><td>DNS Server:</td><td><input type=text name=\"v4DNS\" %s value=%s size=16 maxlength=15></td>\n\
</table>\n\
%s<p><input type=\"radio\" name=\"IPv4\" value=\"DHCPv4\" %s>Use DHCPv4%s</p>\n\
<P><input type=\"radio\" name=\"IPv4\" %s value=\"IPv4Disable\" %s>Disable IPv4 Protocol</P>\n\
<Hr>\n\
<p><input type=submit value=\"Submit\"></p>\n"
#ifdef CONFIG_TEREDO_ENABLED
"<p>Ipv6 Teredo address is %s</p>\n"
#endif
"</form>\n\
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
  
const static unsigned char notFound[] =
{
    "<H1>Not Found<H1>"
};

const static char *webPages[] =
{
    0, rdcss, logo, bannerHtm, homeHtm ,welcomeHtm, 0, 0, remoteinfoHtm, securityHtm, netSetIPv4Htm/*10*/, netSetIPv6Htm, 0 ,0, authHtm, authFailHtm, okPage, ezShareHtm, 0,jobinfoHtm, statusinfoHtm, propertyinfoHtm, mainteninfoHtm, supportinfoHtm/*bin*/,wakeupinfoHtm , joblistHtm, /*joballHtm*/
    jobreportHtm, generalHtm, papertraysHtm, outputtraysHtm, drumHtm, machineHtm, memoryHtm, counterHtm, papertrayattriHtm, papersettingHtm, powersettingHtm, internetsettingHtm, portstatusHtm, ethernetHtm, usbHtm, smbHtm, /*41*/
    tcpipHtm, lpdHtm, snmpHtm, ippHtm, port9100Htm, httpHtm, printmodeHtm, postscriptHtm, hpgl2Htm, tiffHtm, pclHtm, memorysettingHtm, errorhistoryHtm, supportHtm/*, joballHtm, statusallHtm, propertyallHtm, maintenallHtm, supportallHtm, wakeupallHtm */ 
};

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
        /*
        else if ((strncmp(url,"/dindxbut.png", strlen(url)) == 0))
        {
            return DINDPNG;
        }
        */
        else if ((strncmp(url,"/banner.htm", strlen(url)) == 0))
        {
            return BANNERPAGE;
        }
        else if ((strncmp(url,"/welcome.htm", strlen(url)) == 0))
        {
            return WELCOMEPAGE;
        }
        //ht 20110713 : useless
        /*
        else if ((strncmp(url,"/info.htm", strlen(url)) == 0))
        {
            return INFOPAGE;
        }
        else if ((strncmp(url,"/hdhome.htm", strlen(url)) == 0))
        {
            return HDHOMEPAGE;
        }
        */
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
        else if ((strncmp(url,"/about.htm", strlen(url)) == 0))
        {
            return ABOUTPAGE;
        }
        /*
        else if ((strncmp(url,"/joball.htm", strlen(url)) == 0))
        {
            return JOBALLPAGE;
        }
        else if ((strncmp(url,"/statusall.htm", strlen(url)) == 0))
        {
            return STATUSALLPAGE;
        }
        else if ((strncmp(url,"/propertyall.htm", strlen(url)) == 0))
        {
            return PROPERTYALLPAGE;
        }
        else if ((strncmp(url,"/maintenall.htm", strlen(url)) == 0))
        {
            return MAINTENALLPAGE;
        }
        else if ((strncmp(url,"/supportall.htm", strlen(url)) == 0))
        {
            return SUPPORTALLPAGE;
        }
        else if ((strncmp(url,"/wakeupall.htm", strlen(url)) == 0))
        {
            return WAKEUPALLPAGE;
        }
        */
        else if ((strncmp(url,"/jobinfo.htm", strlen(url)) == 0))
        {
            return JOBINFOPAGE;
        }
        else if ((strncmp(url,"/statusinfo.htm", strlen(url)) == 0))
        {
            return STAINFOPAGE;
        }
        else if ((strncmp(url,"/propertyinfo.htm", strlen(url)) == 0))
        {
            return PROINFOPAGE;
        }
        else if ((strncmp(url,"/mainteninfo.htm", strlen(url)) == 0))
        {
            return MAIINFOPAGE;
        }
        else if ((strncmp(url,"/supportinfo.htm", strlen(url)) == 0))
        {
            return SUPINFOPAGE;
        }
        else if ((strncmp(url,"/remoteinfo.htm", strlen(url)) == 0))
        {
            return REMOTECTLINFO;
        }
        else if ((strncmp(url,"/wakeupinfo.htm", strlen(url)) == 0))
        {
            return WAKEINFOPAGE;
        }
        else if ((strncmp(url,"/joblist.htm", strlen(url)) == 0))
        {
            return JOBLISTPAGE;
        }
        else if ((strncmp(url,"/jobreport.htm", strlen(url)) == 0))
        {
            return JOBREPORTPAGE;
        }
        else if ((strncmp(url,"/general.htm", strlen(url)) == 0))
        {
            return GENERALPAGE;
        }
        else if ((strncmp(url,"/papertray.htm", strlen(url)) == 0))
        {
            return PAPERTRAYPAGE;
        }
        else if ((strncmp(url,"/outputtray.htm", strlen(url)) == 0))
        {
            return OUTPUTTRAYPAGE;
        }
        else if ((strncmp(url,"/drum.htm", strlen(url)) == 0))
        {
            return DRUMPAGE;
        }
        else if ((strncmp(url,"/machine.htm", strlen(url)) == 0))
        {
            return MACHINEPAGE;
        }
        else if ((strncmp(url,"/memory.htm", strlen(url)) == 0))
        {
            return MEMORYPAGE;
        }
        else if ((strncmp(url,"/counter.htm", strlen(url)) == 0))
        {
            return COUNTERPAGE;
        }
        else if ((strncmp(url,"/papertrayattri.htm", strlen(url)) == 0))
        {
            return PAPERTRAYATTRIPAGE;
        }
        else if ((strncmp(url,"/papersetting.htm", strlen(url)) == 0))
        {
            return PAPERSETTINGPAGE;
        }
        else if ((strncmp(url,"/powersetting.htm", strlen(url)) == 0))
        {
            return POWERSETTINGPAGE;
        }
        else if ((strncmp(url,"/internetsetting.htm", strlen(url)) == 0))
        {
            return INTERNETSETTINGPAGE;
        }
        else if ((strncmp(url,"/portstatus.htm", strlen(url)) == 0))
        {
            return PORTSTATUSPAGE;
        }
        else if ((strncmp(url,"/ethernet.htm", strlen(url)) == 0))
        {
            return ETHERNETPAGE;
        }
        else if ((strncmp(url,"/usb.htm", strlen(url)) == 0))
        {
            return USBPAGE;
        }
        else if ((strncmp(url,"/smb.htm", strlen(url)) == 0))
        {
            return SMBPAGE;
        }
        else if ((strncmp(url,"/tcpip.htm", strlen(url)) == 0))
        {
            return TCPIPPAGE;
        }
        else if ((strncmp(url,"/lpd.htm", strlen(url)) == 0))
        {
            return LPDPAGE;
        }
        else if ((strncmp(url,"/snmp.htm", strlen(url)) == 0))
        {
            return SNMPPAGE;
        }
        else if ((strncmp(url,"/ipp.htm", strlen(url)) == 0))
        {
            return IPPPAGE;
        }
        else if ((strncmp(url,"/port9100.htm", strlen(url)) == 0))
        {
            return PORT9100PAGE;
        }
        else if ((strncmp(url,"/http.htm", strlen(url)) == 0))
        {
            return HTTPPAGE;
        }
        else if ((strncmp(url,"/printmode.htm", strlen(url)) == 0))
        {
            return PRINTMODEPAGE;
        }
        else if ((strncmp(url,"/postscript.htm", strlen(url)) == 0))
        {
            return POSTSCRIPTPAGE;
        }
        else if ((strncmp(url,"/hpgl2.htm", strlen(url)) == 0))
        {
            return HPGl2PAGE;
        }
        else if ((strncmp(url,"/tiff.htm", strlen(url)) == 0))
        {
            return TIFFPAGE;
        }
        else if ((strncmp(url,"/pcl.htm", strlen(url)) == 0))
        {
            return PCLPAGE;
        }
        else if ((strncmp(url,"/memorysetting.htm", strlen(url)) == 0))
        {
            return MEMORYSETTINGPAGE;
        }
        else if ((strncmp(url,"/errorhistory.htm", strlen(url)) == 0))
        {
            return ERRORHISTORYPAGE;
        }
        else if ((strncmp(url,"/support.htm", strlen(url)) == 0))
        {
            return SUPPORTPAGE;
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

//ht 20110817 : add for change MAC addr to string
void MACtoStr(unsigned char* mac, unsigned char* str)
{
    if (mac)
        sprintf(str ,"%02x:%02x:%02x:%02x:%02x:%02x",
                mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    else
        sprintf(str," ");
}

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

int genNetSetPage(char* buf)
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
        IPtoStr(dpconf->HostIP.addr, HostIP);
        IPtoStr(dpconf->SubnetMask.addr, SubnetMask);
        IPtoStr(dpconf->GateWayIP.addr, GateWayIP);
        IPtoStr(dpconf->DNSIP.addr, DNSIP);
    }

    rv = sprintf(buf , netSetIPv4Htm, (isSingleIP&&isIB)?"disabled":"", (!dpconf->DHCPv4Enable)?"checked":"",
                 (isSingleIP&&isIB)?"disabled":"",(dpconf->HostIP.addr && !dpconf->DHCPv4Enable)?HostIP:"\"\"",
                 (isSingleIP&&isIB)?"disabled":"",(dpconf->SubnetMask.addr && !dpconf->DHCPv4Enable)?SubnetMask:"\"\"",
                 (isSingleIP&&isIB)?"disabled":"",(dpconf->GateWayIP.addr && !dpconf->DHCPv4Enable)?GateWayIP:"\"\"",
                 (isSingleIP&&isIB)?"disabled":"",(dpconf->DNSIP.addr&& !dpconf->DHCPv4Enable)?DNSIP:"\"\"",
                 isIB?"<!--":"<p>",(dpconf->DHCPv4Enable)?"checked":"",isIB?"-->":"</p>",
                 (isSingleIP&&isIB)?"disabled":"",(dpconf->IPv4Enable == 0)?"checked":""
#ifdef CONFIG_TEREDO_ENABLED
                 ,HostIPv6
#endif
                );
 
    offset += rv;
    return offset;
}

//ht 20110811
int genIPSetPage(char* buf)
{
    int offset = 0;
    int rv = 0;
    
    unsigned char HostIP[16]={0};
    unsigned char SubnetMask[16]={0};
    unsigned char GateWayIP[16]={0};
    unsigned char DNSIP[16]={0};

    if (dpconf->DHCPv4Enable == 0)
    {
        IPtoStr(dpconf->HostIP.addr, HostIP);
        IPtoStr(dpconf->SubnetMask.addr, SubnetMask);
        IPtoStr(dpconf->GateWayIP.addr, GateWayIP);
        IPtoStr(dpconf->DNSIP.addr, DNSIP);
    }

    rv = sprintf(buf ,tcpipHtm, dpconf->HostName,
                (dpconf->HostIP.addr && !dpconf->DHCPv4Enable)?HostIP:"\"\"", (dpconf->SubnetMask.addr && !dpconf->DHCPv4Enable)?SubnetMask:"\"\"", 
                (dpconf->GateWayIP.addr && !dpconf->DHCPv4Enable)?GateWayIP:"\"\"", (dpconf->DNSIP.addr&& !dpconf->DHCPv4Enable)?DNSIP:"\"\"");

    offset += rv;
    return offset;
}

//ht 20110817
int genEthernetPage(char* buf)
{
    int offset = 0;
    int rv = 0;

    unsigned char MacAddr[18]={0};

    MACtoStr(dpconf->srcMacAddr[intf], MacAddr);

    rv = sprintf(buf , ethernetHtm, MacAddr);
                      
    offset += rv;
    return offset;
}

//ht 20110816
int genSMBSetPage(char* buf)
{
    int offset = 0;
    int rv = 0;

    rv = sprintf(buf ,smbHtm, dpconf->HostName, dpconf->admin.name, dpconf->admin.passwd, dpconf->admin.passwd);

    offset += rv;
    return offset;
}

int genNetSetv6Page(char* buf)
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
        IPv6toStr(dpconf->HostIPv6, HostIPv6);
        IPv6toStr(dpconf->IPv6GateWay, GateWayIP);
        IPv6toStr(dpconf->IPv6DNS, DNSIP);
    }

    sprintf(PrefixLen, "%d", dpconf->IPv6PrefixLen);
    rv = sprintf(buf , netSetIPv6Htm, (!dpconf->DHCPv6Enable)?"checked":"",
                 (!dpconf->DHCPv6Enable)?HostIPv6:"\"\"",
                 (!dpconf->DHCPv6Enable)?PrefixLen:"\"\"",
                 (!dpconf->DHCPv6Enable)?GateWayIP:"\"\"",
                 (!dpconf->DHCPv6Enable)?DNSIP:"\"\"",
                 (dpconf->DHCPv6Enable)?"checked":"", (dpconf->IPv6Enable == 0)?"checked":"");
    offset += rv;
    return offset;
}

int genAboutPage(char* buf)
{
    int offset = 0;
    int rv = 0;

    rv = sprintf(buf ,"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n"\
                 "<STYLE type=text/css>\n"\
                 "td{COLOR: #FF9900; FONT-FAMILY:Comic Sans MS, Arial}\n"\
                 "</STYLE>"\
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
        IPtoStr(dpconf->HostIP.addr, HostIP);
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

void sendEventLogPage(struct tcp_pcb *pcb)
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
//ht 20110712 : here will cause compiler fail in F
//#endif
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

        getEventLogParser(ed, tmpBuf);

        if (i%2)
            current->len += sprintf(current->payLoadAddr + current->len, "<td>%s</td><tr>\n", tmpBuf);
        else
            current->len += sprintf(current->payLoadAddr + current->len, "<td bgcolor=FF9900>%s</td><tr>\n", tmpBuf);

        current = chkDataListBufSize(current);
        ed = ed->next;
        i++;
    }

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
//ht 20110712 : here will cause compiler fail in F, add #endif
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
    //INT8U ptr[3];
    //INT8U state;
    DataList *head;
    DataList *current;
    int headerSize = 0, i = 0;
#ifndef CONFIG_WAKE_UP_PATTERN_ENABLED
    char bootDevice[] = {0x01, 0x06, 0x03, 0x06, 0x00};
#else
    char bootDevice[] = {0x0};
    const INT8U statestr[][3] = {"S0", "S1", "S2", "S3", "S4", "S5"};
#endif
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

    current->len += sprintf(current->payLoadAddr + current->len, remoteCtlHtmTop1);
    current->len += sprintf(current->payLoadAddr + current->len, "<p><H3>Current Power State : %s </H3></p>\n",statestr[bsp_get_sstate()]); 
    current->len += sprintf(current->payLoadAddr + current->len, remoteCtlHtmTop2);

    while (bootDevice[i] != 0)
    {
        current = chkDataListBufSize(current);

        if (bootDevice[i] == 6)
        {
            i++;
            continue;
        }
        current->len += sprintf(current->payLoadAddr + current->len, "<p><input type=\"radio\" name=\"boot\" value=");
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
#ifndef CONFIG_WAKE_UP_PATTERN_ENABLED
    current->len += sprintf(current->payLoadAddr + current->len, "<p><input type=\"radio\" name=\"boot\" value=\"disable\" checked>Disable</p>");
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
        if (wpid == LOGOGIF)
        {
            buf = (char*)malloc(sizeof(logo) + 200);
            pageSize = sizeof(logo)/sizeof(unsigned char);
            rv = sprintf(buf, "HTTP/1.1 200 \r\nContent-type: image/gif\r\nContent-Length: %d\r\n\r\n", pageSize);
            offset += rv;
            //Do not use sprintf here
            memcpy(buf + offset, logo, pageSize);
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
        else if (wpid == SRVSETPAGE || wpid == NETSETPAGE || wpid == NETSETv6PAGE || wpid == ABOUTPAGE )
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
                pageSize = genNetSetPage(buf + offset);
            else if (wpid == NETSETv6PAGE)
                pageSize = genNetSetv6Page(buf + offset);
            else if (wpid == ABOUTPAGE)              
                pageSize = genAboutPage(buf + offset);
            
            offset += pageSize;

            //buf[rv] will append 0 for string end
            tmpChar = buf[rv];
            sprintf(buf ,headerHtm, pageSize);
            buf[rv] = tmpChar;
#endif
        }
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
        //ht 20110811 : dynamic update tcpip.htm
        else if (wpid == TCPIPPAGE)
        {
            buf = (char*)malloc(ETH_PAYLOAD_LEN);
            //headerHtm => %5d => fixed length
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;

            pageSize = genIPSetPage(buf + offset);
            
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
        //ht 20110816 : dynamic update smb.htm
        else if (wpid == SMBPAGE)
        {
            buf = (char*)malloc(ETH_PAYLOAD_LEN);
            //headerHtm => %5d => fixed length
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;

            pageSize = genSMBSetPage(buf + offset);
            
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
        //ht 20110816 : dynamic update ethernet.htm
        else if (wpid == ETHERNETPAGE)
        {
            buf = (char*)malloc(ETH_PAYLOAD_LEN);
            //headerHtm => %5d => fixed length
            rv = sprintf(buf, headerHtm, 0);
            offset += rv;

            pageSize = genEthernetPage(buf + offset);
            
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
