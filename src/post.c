/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	post.c

Abstract:	post module. Do post actions.

*****************************************************************************
*/

/*--------------------------Include File------------------------------------*/
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "tcp.h"
#include "http.h"
#include "htm.h"
#include "lib.h"
#include "smbus.h"
#include "bsp.h"
#include "usb.h"
#include "debug.h"
#include "rmcp.h"
#include "telnet.h"
#include "mail_wakeup.h"


/*--------------------------Exported variables------------------------------*/
#if CONFIG_EZSHAE_ENABLED || defined(CONFIG_EZSHAE_TASK_ENABLED)
extern void USBSrvTask(void *data);
#ifdef CONFIG_USB_ENABLED
extern OS_STK  TaskUSBStk[TASK_USB_STK_SIZE] _ATTRIBUTE_STK;
#endif
#endif

extern const unsigned char headerHtm[];
extern smbiosrmcpdata *smbiosrmcpdataptr;
extern DPCONF *dpconf;
extern USBCB usbcb;

char echoEnable[] = {"<body><H2><font color=\"#006600\">Echo Service Enable</H2>"};
char echoDisable[] = {"<body><H2><font color=\"#006600\">Echo Service Disable</H2>"};
char unKnownOperation[] = {"<body><H2><font color=\"#006600\">Unknown Operation</H2>"};
char invalidParameters[] = {"<body><H2><font color=\"#006600\">Invalid Parameters</H2>"};
char pwdError[] = {"<body><H2><font color=\"#006600\">Incorrect password</H2>"};
char IBChgIPWarrningMsg[] = {"<body><H2><font color=\"#006600\">Please change IP address by OS</H2>"};

char settingSuccess[] = {"<body><H2><font color=\"#006600\">Setting Success</H2>"};

/*----------------------------Functions-------------------------------------*/

#define NS_IN6ADDRSZ 16
#define INT16SZ   2
#define NS_INT16SZ 2
#define NS_INADDRSZ   4

static int inet_pton4(const char *src,unsigned char *dst)
{
    int saw_digit, octets, ch;
    unsigned char tmp[NS_INADDRSZ], *tp;

    saw_digit = 0;
    octets = 0;
    *(tp = tmp) = 0;
    while ((ch = *src++) != '\0') {

        if (ch >= '0' && ch <= '9') {
            unsigned int newv = *tp * 10 + (ch - '0');

            if (newv > 255)
                return (0);
            *tp = newv;
            if (! saw_digit) {
                if (++octets > 4)
                    return (0);
                saw_digit = 1;
            }
        } else if (ch == '.' && saw_digit) {
            if (octets == 4)
                return (0);
            *++tp = 0;
            saw_digit = 0;
        } else
            return (0);
    }
    if (octets < 4)
        return (0);
    memcpy(dst, tmp, NS_INADDRSZ);
    return (1);
}


/* int
 * inet_pton6(src, dst)
 *	convert presentation level address to network order binary form.
 * return:
 *	1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *	(1) does not touch `dst' unless it's returning 1.
 *	(2) :: in a full address is silently ignored.
 * credit:
 *	inspired by Mark Andrews.
 * author:
 *	Paul Vixie, 1996.
 */
static int
inet_pton6(const char *src, unsigned char *dst)
{
    static const char xdigits_l[] = "0123456789abcdef",
                                    xdigits_u[] = "0123456789ABCDEF";
    unsigned char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
    const char *xdigits, *curtok;
    int ch, saw_xdigit;
    unsigned int val;

    memset((tp = tmp), '\0', NS_IN6ADDRSZ);
    endp = tp + NS_IN6ADDRSZ;
    colonp = NULL;
    /* Leading :: requires some special handling. */
    if (*src == ':')
        if (*++src != ':')
            return (0);
    curtok = src;
    saw_xdigit = 0;
    val = 0;
    while ((ch = *src++) != '\0') {
        const char *pch;

        if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
            pch = strchr((xdigits = xdigits_u), ch);
        if (pch != NULL) {
            val <<= 4;
            val |= (pch - xdigits);
            if (val > 0xffff)
                return (0);
            saw_xdigit = 1;
            continue;
        }
        if (ch == ':') {
            curtok = src;
            if (!saw_xdigit) {
                if (colonp)
                    return (0);
                colonp = tp;
                continue;
            } else if (*src == '\0')
                return (0);
            if (tp + INT16SZ > endp)
                return (0);
            *tp++ = (unsigned char) (val >> 8) & 0xff;
            *tp++ = (unsigned char) val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }

        if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
                inet_pton4(curtok, tp) > 0) {
            tp += NS_INADDRSZ;
            saw_xdigit = 0;
            break;	/* '\0' was seen by inet_pton4(). */
        }
        return (0);
    }
    if (saw_xdigit) {
        if (tp + NS_INT16SZ > endp)
            return (0);
        *tp++ = (unsigned char) (val >> 8) & 0xff;
        *tp++ = (unsigned char) val & 0xff;
    }
    if (colonp != NULL) {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;

        if (tp == endp)
            return (0);
        for (i = 1; i <= n; i++) {
            endp[- i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        return (0);
    memcpy(dst, tmp, NS_IN6ADDRSZ);
    return (1);
}

int verifyIPField(int num)
{
    if (num > 255 || num < 0 )
    {
        return -1;
    }
    return 0;
}
               
//ht 20110815                               
#ifdef CONFIG_PRINTER_WEB_PAGE
int SetSMBValue(char* str, char *token)
{
    char *tmp,*start,*end, *start_conf, *end_conf;
    char invalid = 0;
    
    tmp = strstr(str, token);
    
    if (!tmp)
    {
            invalid = 1;
    }
    else
    {   
        //maybe can use sscanf
        start = strchr(tmp, '=') + 1;
        end = strchr(start, '&');
        *end = '\0'; 

        if(token == "Confname=")
        {
            tmp = strstr(str, "Passname=");
            start_conf = strchr(tmp, '=') + 1;
            end_conf = strchr(start_conf, '&');
            *end_conf = '\0';
            
            if( strcmp(start, start_conf) == 0 )
                memcpy(dpconf->admin.passwd, start, sizeof(dpconf->admin.passwd));
            else
                return -1;
        }
        if(token == "Adminname=")
            memcpy(dpconf->admin.name, start, sizeof(dpconf->admin.name));
        if(token == "Hostname=")
            memcpy(dpconf->HostName, start, sizeof(dpconf->HostName));
    }
    
    if (invalid)
        return -1;
    else
        return 0;
}               
#endif
                                          
int netSetIPv4Value(char* str, char *token, int* value)
{
    char *tmp;
    int ipv4[4] = {0};
    int i = 0;
    char invalid = 0;

    do
    {
        tmp = strstr(str, token);

        if (!tmp)
        {
            invalid = 1;
            break;
        }
	
        tmp +=  strlen(token);
        
        for(i = 0; i < strlen(tmp);i++)
        {
        	if(!isdigit(tmp[i]))
        	{
        		if(tmp[i] == '.')
        			continue;
        		else if(tmp[i] == '&')
        			break;
        		else 
        		{
        			invalid = 1;
            			break;
        		}
        	}
        }

        if (sscanf(tmp ,"%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2], &ipv4[3])!=4)
        {
            invalid = 1;
            break;
        }

        for (i = 0 ; i < 4; i++)
        {
            if (verifyIPField(ipv4[i]) == -1)
            {
                invalid = 1;
                break;
            }
        }

        if (i != 4)
        {
            invalid = 1;
            break;
        }
        *value = ipv4[0] << 24 | ipv4[1] << 16 | ipv4[2] << 8 | ipv4[3];
    }while (0);

    if (invalid)
        return -1;
    else
        return 0;
}


int netSetIPv6Value(char* str, char *token, char *ipv6Tmp, int *len)
{
    char *p = 0;
    char *tmp = 0;
    char *end = 0;
    char *delimiter;
    int i = 0;

    p = strstr(str, token);

    if (!p)
        return -1;

    p += strlen(token);

    *len = *len - (p - str);

    end = strchr(p, '&');

    if (!end)
    {
        end = p + *len;
    }


    while (p < end)
    {
        delimiter = strstr(p, "%3A");
        tmp = p;

        if (delimiter && delimiter < end)
        {
            memcpy(ipv6Tmp + i, p, delimiter - p);
            i += delimiter - p;
            ipv6Tmp[i] = ':';
            i++;
            p = delimiter + strlen("%3A");
        }
        else
        {
            memcpy(ipv6Tmp + i, p, end - p);
            p = end;
        }

        *len = *len - (p - tmp);


        if (delimiter > end)
            break;
    }
    return p - str;
}

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
void PtnAdd(struct tcp_pcb *pcb, char *p, int len)
{
    char *start, *end, *tmp;
    WAKE_UP_t *ptr;
    char numstr[3];
    int i = 0, modify = 1;


    if (strstr(p, "ptr="))
    {
        end = strchr(p, '&');
        start = p + 4;
        if (end)
        {
            *end = '\0';
            ptr = (WAKE_UP_t *) strtoul(start, NULL, 0);
        }

        if (!ptr)
        {
            ptr = malloc(sizeof(WAKE_UP_t));
            memset(ptr, 0, sizeof(WAKE_UP_t));
            modify = 0;
        }

        p = end + 1;

    }

    if (strstr(p, "chk="))
    {
        start = strchr(p, '=') + 1;
        end = strchr(start, '&');
        *end = '\0';
        if (strcmp(start, "on"))
            ptr->valid = 0;
        else
            ptr->valid = 1;
        ptr->exist = 1;
        p = end + 1;
    }
    else
        ptr->valid = 0;

    if (strstr(p, "pname="))
    {
        start = strchr(p, '=') + 1;
        end = strchr(start, '&');
        *end = '\0';
        while (tmp = strchr(start, '+'))
            *tmp = ' ';
        while (tmp = strstr(start, "%28"))
        {
            tmp[0] = ' ';
            tmp[1] = '(';
            tmp[2] = ' ';
        }
        while (tmp = strstr(start, "%29"))
        {
            tmp[0] = ' ';
            tmp[1] = ')';
            tmp[2] = ' ';
        }



        strcpy(ptr->name, start);
        p = end + 1;
    }
    if (strstr(p, "pstart="))
    {
        start = strchr(p, '=') + 1;
        end = strchr(start, '&');
        *end = '\0';
        ptr->start = strtol(start, NULL, 16);
        if (ptr->start < 0 || ptr->start > 128)
            ptr->start = 0;
        p = end + 1;
    }
    if (strstr(p, "plength="))
    {
        start = strchr(p, '=') + 1;
        end = strchr(start, '&');
        *end = '\0';
        ptr->length = strtol(start, NULL, 16);
        if (ptr->length > 128 || ptr->length < 0)
            ptr->length = 128;
        p = end + 1;
    }
    if (strstr(p, "pptns="))
    {
        start = strchr(p, '=') + 1;
        end = strchr(start, '&');
        for (i = 0 ; i < ptr->length; i++)
        {
            strncpy(numstr, start, 2);
            numstr[2] = '\0';
            start += 2;
            ptr->pattern[i] = strtol(numstr, NULL, 16);
        }

        p = end +1;
    }
    if (strstr(p, "pmask="))
    {
        start = strchr(p, '=') + 1;
        end = strchr(start, '&');
        for (i = 0 ; i < ifloor(ptr->length,8); i++)
        {
            strncpy(numstr, start, 2);
            numstr[2] = '\0';
            start += 2;
            ptr->mask[i] = strtol(numstr, NULL, 16);
        }

        p = end +1;
    }

    if (modify == 0)
        WakePtnAdd(ptr);
    sendEventLogPage(pcb);
}

void PtnEdt(struct tcp_pcb *pcb, char *p, int len)
{
    char *start, *end;
    WAKE_UP_t *ptr;
    if (strstr(p, "PM_"))
    {
        end = strchr(p, '=');
        start = p + 3;
        if (end)
        {
            *end = '\0';
            ptr = (WAKE_UP_t *) strtoul(start, NULL, 0);
            sendEditPage(pcb, ptr);
        }

    }
    else if (strstr(p, "PD_"))
    {
        end = strchr(p, '=');
        start = p + 3;
        if (end)
        {
            *end = '\0';
            ptr = (WAKE_UP_t *) strtoul(start, NULL, 0);
            WakePtnDel(ptr);
        }
        sendEventLogPage(pcb);

    }
    else
    {

        sendEventLogPage(pcb);
    }
    //sendEventLogPage(pcb);

}
#endif

//ht 20110816
#ifdef CONFIG_PRINTER_WEB_PAGE
void smbSet(struct tcp_pcb *pcb, char *p, int len)
{
    char *token;
    char *value;
    char *buf;
    PKT *pkt;
    
    //ht 20110815
    //INT8U *name = malloc(16);
    
    int invalid = 0;
    
    int offset = 0, rv = 0;
    
    buf = malloc(ETH_PAYLOAD_LEN);
    memset(buf, 0, ETH_PAYLOAD_LEN);

    //ht 20110815
    //memset(name, 0 ,16);
    
    token = strstr(p, "Workname="); 
    if (token != 0)
    {
        value = token + strlen("Workname=");
        if (strncmp(value, "WORKGROUP", strlen("WORKGROUP")) == 0)
        {
            if (SetSMBValue(p, "Confname=") == -1)
                invalid = 1;
            if (SetSMBValue(p, "Adminname=") == -1)
                invalid = 1;
            if (SetSMBValue(p, "Hostname=") == -1)
                invalid = 1;
        }
    }
    
    if (invalid)
    {
        rv = sprintf(buf , headerHtm, strlen(invalidParameters));
        offset+=rv;
        rv = sprintf(buf + rv, invalidParameters);
        offset+=rv;
    }
    else
    {
        rv = sprintf(buf , headerHtm, strlen(settingSuccess));
        offset+=rv;
        rv = sprintf(buf + rv, settingSuccess);
        offset+=rv;
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
    
    free(buf);
}
#endif

void netSet(struct tcp_pcb *pcb, char *p, int len)
{
    char *token;
    char *value;
    char *buf;
    PKT *pkt;

    ServiceCfg tmpScfg;
    int invalid = 0;
    //unsigned char ipv6[16] = {0};
    //7*5 + 4*4 - 1
    //unsigned char ipv6Host[50] = {0};
    //unsigned char ipv6Gateway[50] = {0};
    //unsigned char ipv6DNS[50] = {0};
    INT8U *ipv6 = malloc(16);
    INT8U *ipv6Host = malloc(50);
    INT8U *ipv6Gateway = malloc(50);
    INT8U *ipv6DNS = malloc(50);

    unsigned int setIPv4 = 0;
    unsigned int setIPv4SubnetMask = 0;
    unsigned int setIPv4Gateway = 0;
    unsigned int setIPv4DNS = 0;

    int offset = 0, rv = 0;
    unsigned int prefixLen = 0;

    unsigned char error;
    unsigned char ipv4Cfg = 0;
    unsigned char ipv6Cfg = 0;

    unsigned char isIB = bsp_in_band_exist();
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
#if CONFIG_VERSION >= IC_VERSION_EP_RevA
	unsigned char isSingleIP = bsp_bits_get(MAC_OOBREG, BIT_SINGLE_IP, 1);
#else
    unsigned char isSingleIP = bsp_bits_get(IO_CONFIG1, BIT_SINGLE_IP, 1);
#endif
#else
    unsigned char isSingleIP = 0;
#endif

    buf = malloc(ETH_PAYLOAD_LEN);
    memset(buf, 0, ETH_PAYLOAD_LEN);

    memset(ipv6, 0 ,16);
    memset(ipv6Host, 0 ,50);
    memset(ipv6Gateway, 0, 50);
    memset(ipv6DNS, 0, 50);

    memset(&tmpScfg, 0, sizeof(ServiceCfg));

    if (isIB && isSingleIP)
    {
        invalid = 1;
    }
    else
    {
#ifdef CONFIG_PRINTER_WEB_PAGE
        //ht 20110812 : modify for new web page
        token = strstr(p, "method="); 	
#else
        //IPv4
        token = strstr(p, "IPv4=");
#endif
        if (token != 0)
        {
            ipv4Cfg = 1;
#ifdef CONFIG_PRINTER_WEB_PAGE
            value = token + strlen("method="); 
            if (strncmp(value, "Manual", strlen("Manual")) == 0)		
#else
            value = token + strlen("IPv4=");
            if (strncmp(value, "v4", strlen("v4")) == 0)
#endif
            {
                //ht 20110816 : change to SMB page
                /*
                if (SetHostNameValue(p, "Hostname=", &name) == -1)
                    invalid = 1;
                */    
                if (netSetIPv4Value(p, "v4Addr=", &setIPv4) == -1)
                    invalid = 1;

                if (netSetIPv4Value(p, "v4Mask=", &setIPv4SubnetMask) == -1)
                    invalid = 1;

                if (netSetIPv4Value(p, "v4Gateway=", &setIPv4Gateway) == -1)
                {
                    token = strstr(p, "v4Gateway=");
                    token = strchr(token, '=');
                    value = strchr(token, '&');

                    if (value - token != 1)
                        invalid = 1;
                }

                if (netSetIPv4Value(p, "v4DNS=", &setIPv4DNS) == -1)
                {
                    token = strstr(p, "v4DNS=");
                    if (token - p + strlen("v4DNS=") != len)
                        invalid = 1;
                }

                if (!invalid)
                {
                    tmpScfg.IPv4Enable = 1;
                    tmpScfg.DHCPv4Enable = 0;
                }
            }
            else if (strncmp(value, "DHCPv4", strlen("DHCPv4")) == 0)
            {
                tmpScfg.IPv4Enable = 1;
                tmpScfg.DHCPv4Enable = 1;
            }
            else if (strncmp(value, "IPv4Disable", strlen("IPv4Disable")) == 0)
            {
                tmpScfg.IPv4Enable = 0;
            }
            else //ht 20110817 : choice DHCP (new web page)
            {
                tmpScfg.IPv4Enable = 1;
                tmpScfg.DHCPv4Enable = 1;
            }
        }

        token = strstr(p, "IPv6=");

        if (token != 0)
        {
            ipv6Cfg = 1;
            value = token + strlen("IPv6=");

            if (strncmp(value, "v6", strlen("v6")) == 0)
            {
                do
                {
                    token = strstr(p, "v6Plen=");
                    value = strchr(token, '&');
                    if (token && value && (value - token < 3 + strlen("v6Plen=") + 1))
                    {
                        sscanf(token + strlen("v6Plen="),"%d" ,&prefixLen);
                    }
                    else
                    {
                        invalid = 1;
                        break;
                    }

                    p = p + netSetIPv6Value(p, "v6Addr=", ipv6Host, &len);

                    if (inet_pton6(ipv6Host, ipv6) == 0)
                    {
                        invalid = 1;
                        break;
                    }

                    p = p + netSetIPv6Value(p, "Gateway=", ipv6Gateway, &len);

                    if (inet_pton6(ipv6Gateway, ipv6) == 0)
                    {
                        invalid = 1;
                        break;
                    }

                    netSetIPv6Value(p, "DNS=", ipv6DNS, &len);

                    if (inet_pton6(ipv6DNS, ipv6) == 0)
                    {
                        invalid = 1;
                        break;
                    }

                    tmpScfg.IPv6Enable = 1;
                }while (0);

            }
            else if (strncmp(value, "DHCPv6", strlen("DHCPv6")) == 0)
            {
                tmpScfg.IPv6Enable = 1;
                tmpScfg.DHCPv6Enable= 1;
            }
            else if (strncmp(value, "IPv6Disable", strlen("IPv6Disable")) == 0)
            {
                DEBUGMSG(HTML_DEBUG,"IP IPv6Disable\n");
                tmpScfg.IPv6Enable = 0;
            }
        }
    }

    if (invalid)
    {
        if (isIB && isSingleIP)
        {
            rv = sprintf(buf , headerHtm, strlen(IBChgIPWarrningMsg));
            offset+=rv;
            rv = sprintf(buf + rv, IBChgIPWarrningMsg);
            offset+=rv;
        }
        else
        {
            rv = sprintf(buf , headerHtm, strlen(invalidParameters));
            offset+=rv;
            rv = sprintf(buf + rv, invalidParameters);
            offset+=rv;
        }
    }
    else
    {
        rv = sprintf(buf , headerHtm, strlen(settingSuccess));
        offset+=rv;
        rv = sprintf(buf + rv, settingSuccess);
        offset+=rv;
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

    free(buf);

    if (!invalid)
    {
        if (ipv4Cfg)
            dpconf->IPv4Enable = tmpScfg.IPv4Enable;

        if (ipv6Cfg)
            dpconf->IPv6Enable = tmpScfg.IPv6Enable;

        if (tmpScfg.DHCPv4Enable && ipv4Cfg)
        {
            if (!dpconf->DHCPv4Enable)
            {
                dpconf->DHCPv4Enable |= (1<<pcb->intf);
                error = OSTaskResume(TASK_DHCPv4_PRIO);
            }
        }
        else if (tmpScfg.IPv4Enable && ipv4Cfg)
        {
            setIPAddress_F(setIPv4, pcb->intf);
            setSubnetMask_F(setIPv4SubnetMask, pcb->intf);
            setGateWayIP_F(setIPv4Gateway, pcb->intf);
            setDNSIP_F(setIPv4DNS, pcb->intf);
            dpconf->DHCPv4Enable &= ~(1<<pcb->intf);
        }

        if (tmpScfg.DHCPv6Enable && ipv6Cfg)
        {
            if (!dpconf->DHCPv6Enable)
            {
                dpconf->DHCPv6Enable|= (1<<pcb->intf);
                error = OSTaskResume(TASK_DHCPv6_PRIO);
            }
        }
        else if (tmpScfg.IPv6Enable && ipv6Cfg)
        {
            inet_pton6(ipv6Gateway, ipv6);
            setIPv6GateWay_F(ipv6, pcb->intf);
            inet_pton6(ipv6DNS, ipv6);
            setIPv6DNS_F(ipv6, pcb->intf);
            dpconf->IPv6PrefixLen[pcb->intf] = prefixLen;
            dpconf->DHCPv6Enable &= ~(1<<pcb->intf);
            inet_pton6(ipv6Host, ipv6);
            setIPv6Address_F(ipv6, pcb->intf);
        }
        setdirty(DPCONFIGTBL);
    }

    free(ipv6);
    free(ipv6Host);
    free(ipv6Gateway);
    free(ipv6DNS);
}


void serviceSet(struct tcp_pcb *pcb, char *p, int len)
{
    char *buf;
    PKT *pkt;
    int rv = 0;
    int offset =0;

    buf = malloc(ETH_PAYLOAD_LEN);
    memset(buf, 0, ETH_PAYLOAD_LEN);

    if (strncmp("srv=on", p, len) == 0)
    {
        dpconf->EchoService = on;
        setdirty(DPCONFIGTBL);
        rv = sprintf(buf, headerHtm, strlen(echoEnable));
        offset+=rv;
        rv = sprintf(buf + rv, echoEnable);
        offset+=rv;
    }
    else if (strncmp("srv=off", p, len) == 0)
    {
        dpconf->EchoService = off;
        setdirty(DPCONFIGTBL);
        rv = sprintf(buf, headerHtm, strlen(echoDisable));
        offset+=rv;
        rv = sprintf(buf + rv, echoDisable);
        offset+=rv;
    }
    else
    {
        rv = sprintf(buf , headerHtm, strlen(unKnownOperation));
        offset+=rv;
        rv = sprintf(buf + rv, unKnownOperation);
        offset+=rv;
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

    free(buf);
}

void remoteCtlSet(struct tcp_pcb *pcb, char *p, int len)
{
    char *buf;
    char *ptr, *ptr1;
    PKT	 *pkt;
    int rv = 0;
    int offset =0;
    char unknown = 0;
    int value = 0;
    char rmcptemp[8] = {0x00, 0x00, 0x11, 0xbe, 0x00, 0x00, 0x00, 0x00};
    char retRemoteCtl = 0; 	

    buf = malloc(ETH_PAYLOAD_LEN);
    memset(buf, 0, ETH_PAYLOAD_LEN);

    if (len > 100)
        unknown = 1;
    else
    {
        ptr = strchr(p, '=');
        ptr1 = strchr(ptr, '&');
        if(ptr1==NULL){
		snprintf(buf, strlen(ptr), ptr + 1);	
        }else{
        snprintf(buf, ptr1 - ptr, ptr + 1);
        }
        //buf[ptr1 - ptr] = '\0';
        DEBUGMSG(HTTP_DEBUG,"%s\n", buf);

        if (strncmp("reset", buf, strlen("reset")) == 0 )
            value = RMCP_Reset;
        else if (strncmp("up", buf, strlen("up")) == 0 )
            value = RMCP_PWR_On;
        else if (strncmp("off", buf, strlen("off")) == 0 )
            value = RMCP_PWR_Off;
        else if (strncmp("cycle", buf, strlen("cycle")) == 0 )
            value = RMCP_PWR_CR;
        else
            unknown = 1;

	if(ptr1!=NULL){
        ptr = strchr(ptr1, '=');
        snprintf(buf, p + len - ptr, ptr + 1);
        //buf[p + len - ptr ] = '\0';
        DEBUGMSG(HTTP_DEBUG,"%s\n", buf);

#ifndef CONFIG_WAKE_UP_PATTERN_ENABLED
        memcpy(smbiosrmcpdataptr->bootopt, rmcptemp, sizeof(rmcptemp));
        if (strncmp("fp", buf, strlen("fp")) == 0 )
            smbiosrmcpdataptr->bootopt[4] = 0x00;
        else if (strncmp("hd", buf, strlen("hd")) == 0 )
            smbiosrmcpdataptr->bootopt[4] = 0x02;
        else if (strncmp("cd", buf, strlen("cd")) == 0 )
            smbiosrmcpdataptr->bootopt[4] = 0x05;
        else if (strncmp("net", buf, strlen("net")) == 0 )
            smbiosrmcpdataptr->bootopt[4] = 0x01;
        else if (strncmp("usb", buf, strlen("usb")) == 0 )
            smbiosrmcpdataptr->bootopt[4] = 0x04;
        else if (strncmp("disable", buf, strlen("disable")) == 0 )
            smbiosrmcpdataptr->bootopt[4] = 0x00;
	//-briank.rtk@20131220 : Remove setting Boot device from WebPage.
       // else
       //     unknown = 1;
#endif
    }
    }

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
    PrinterRmtl(value);
#else
    retRemoteCtl = SMBus_Prepare_RmtCtrl(value, 0);
#endif

    if (unknown == 0)
    {
	if(retRemoteCtl  != DASH_SUCCESS ){
	       dpconf->EchoService = on;
	        rv = sprintf(buf, headerHtm, strlen(invalidParameters));
	        offset+=rv;
	        rv = sprintf(buf + rv, invalidParameters);
	        offset+=rv;
		
	}else{	
	
	        dpconf->EchoService = on;
	        rv = sprintf(buf, headerHtm, strlen(settingSuccess));
	        offset+=rv;
	        rv = sprintf(buf + rv, settingSuccess);
	        offset+=rv;
	}
    }
    else
    {
        rv = sprintf(buf , headerHtm, strlen(unKnownOperation));
        offset+=rv;
        rv = sprintf(buf + rv, unKnownOperation);
        offset+=rv;
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
    free(buf);
}

void tlsAuth(struct tcp_pcb *pcb, char *p, int len)
{
    char *ptr;
    char *ptr1;
    UserInfo* ui;
    char username[17] = {0};
    char password[17] = {0};

    do
    {
        if (strncmp("UN=", p, strlen("UN=") - 1) == 0)
        {
            ptr = strchr(p, '=') + 1;
            ptr1 = strchr(ptr, '&');

            if (ptr1 - ptr > sizeof(username))
                break;

            //snprintf(username, ptr1 - ptr, ptr);
            memcpy(username, ptr, ptr1 - ptr);

            if (strncmp("PW=", ptr1 + 1, strlen("PW=") - 1) == 0)
            {
                ptr = strchr(ptr1 + 1, '=') + 1;

                len = len - (ptr - p);

                if (len > sizeof(password))
                    break;

                memcpy(password, ptr, len);
                ui = getuserinfo();

                while (ui)
                {
                    if (strncmp(ui->name, username, strlen(username)) == 0 && (strlen(username) == strlen(ui->name)))
                    {
                        if (strncmp(ui->passwd, password, strlen(password)) == 0 && (strlen(password) == strlen(ui->passwd)))
                        {
                            pcb->httpAuthPass = PASS;
                            pcb->ssl->httpAuthPass = PASS;
                            authedSSL(pcb->ssl->sessionID);
                            sendStaticWebPage(pcb, HOMEPAGE);
                        }
                    }
                    ui = ui->next;
                }

            }
        }
    }while (0);
    sendStaticWebPage(pcb, AUTHFAILPAGE);
}

#if CONFIG_EZSHAE_ENABLED || defined(CONFIG_EZSHAE_TASK_ENABLED)
void EzShare(struct tcp_pcb *pcb, char *p, int len)
{
    QHdr* qhdr = (QHdr*)malloc(sizeof(QHdr));
    int type = 0;
    int act = 0;
    unsigned char err;

    memset(qhdr, 0,sizeof(QHdr));

    if (strstr(p, "MSD"))
        qhdr->option = USB_MSD;
    else if (strstr(p, "CDROM"))
        qhdr->option = USB_CDROM;
    else if (strstr(p, "Floppy"))
        qhdr->option = USB_FLOPPY;
    else
        qhdr->option = 255;

    if (strstr(p, "open"))
        qhdr->cmd = SRV_START;
    else if (strstr(p, "close"))
        qhdr->cmd = SRV_STOP;
    else
        qhdr->cmd = 255;

    if (!pcb->apRx)
    {
        OSSemPost(SktSem);
        if (usbcb.usbstate != DISABLED)
            stopUSB();
        OSSemPend(SktSem, 0, &err);
    }

    if (qhdr->option != 255 && qhdr->cmd != 255)
    {
        qhdr->port = 59;
        if (qhdr->cmd == SRV_START)
            OSTaskCreateExt (USBSrvTask,
                             (void *) qhdr,
                             (OS_STK *) & TaskUSBStk[TASK_USB_STK_SIZE - 1],
                             TASK_USB_PRIO,
                             TASK_USB_ID,
                             (OS_STK *) & TaskUSBStk[0],
                             TASK_USB_STK_SIZE,
                             (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    }
#ifdef CONFIG_MSAD_ENABLED
    sendStaticWebPage(pcb, OKPAGE, 0);
#else
    sendStaticWebPage(pcb, OKPAGE);
#endif
    tcpSend_F(pcb, NULL, TCP_ACK | TCP_RST);
    //sendStaticWebPage(pcb, OKPAGE);
}
#endif


#ifdef CONFIG_MAIL_WAKEUP_ENABLED
//mail_wakeup 20121025 eccheng
void decode_url(char *url)
{
    int i=0;
    
    while(*(url+i)) //not end of url
    {
       *url = *(url+i) ; 
       if (*url == '%')//special char encoded by RFC 5322
       {
          *url=*(url+i+1);
          if (*(url+i+1) >= 'A')
              *url = ((*(url+i+1) & 0xDF) - 'A') + 10;
          else
              *url = (*(url+i+1) - '0');           
          *url=(*url) * 16; //2 digit hex value to dec value converting

          if (*(url+i+2) >= 'A')
              *url = *url + ((*(url+i+2) & 0xDF) - 'A') + 10 ;
          else 
              *url = *url +  (*(url+i+2) - '0');                                   
          i+=2; //%HH, 2 characters were processed
       }
       else if (*(url+i)=='+')//space decoding
       {
          *url=' ';
       }
       url++;
    }
    *url='\0';
}

//mail_wakeup 20121025 eccheng
//url must be decoded and format as "keyword1=value1&keyword2=value2&...&keywordn=valuen"
//value_str need to be malloc first and clean all to be zero before call this function
int url_value(char *url_decoded, char *keyword, char *value_str)
{
    char *loc;
    char *tmp_char;
    int  len;

    loc = strstr(url_decoded, keyword);
    if (loc == NULL) //keyword not found
    {
        return -1;
    }
    else
    {
//        printf("%s is at %d \n",keyword, (int) (loc - url_decoded));
        tmp_char = loc + strlen(keyword) + 1;//+1 =>skip '='
        len = 0;
        while ((*tmp_char != '&')&&(*tmp_char != '\0'))
        {
            tmp_char++;
            len++;
        }
        memcpy(value_str, loc + strlen(keyword) + 1, len); 
        return 0;
    }

}

//#define MWU_MAX_STR 128 //***must include mail_wakeup.h to replace this

//mail_wakeup 20121025 eccheng
//#ifdef CONFIG_PRINTER_WEB_PAGE
//pcb->postURL=7 ,MAIL_ACCPOST=7
//p=[mail_wake_up=0&e_svr=192.168.0.18&SSL=0&e_acc=runec%40192.168.0.18&e_psw=123&save=Save+Settings][len=95]
void mail_acc_set(struct tcp_pcb *pcb, char *p, int len)
{
    char *buf;
    PKT *pkt; 
    int invalid = 0;
    int offset = 0, rv = 0;
    char *value_str;
    char *keyword;    
    char *loc;

    buf = malloc(ETH_PAYLOAD_LEN);
    memset(buf, 0, ETH_PAYLOAD_LEN);

    invalid = 0;

	//printf("[RTK] + mail_acc_set\n");
	
/*
    INT8U mail_wake_up; //on/off {on : != 0, off : = 0}
    INT8U email_server[MWU_MAX_STR]; //[max.length=128] {IP, Domain Name}//POP3(110) and POP3S(995)
    INT8U SSL; //enable {disable (port=110), enable (port=995)}
    INT8U email_acount[MWU_MAX_STR]; //[max.length=128] 
    INT8U email_passwd[MWU_MAX_STR]; //[max.length=128] 
    
    INT8U match_address[MWU_MAX_STR]; //(from) [max.length=128]
    INT8U match_subject[MWU_MAX_STR]; //[max.length=128] 
    INT8U match_keyword[MWU_MAX_STR]; //[max.length=128]„²only check email content ¡¥s first 1024 bytes
    INT8U Activate_condition; //{condition code = 1..13 }, now only support 7,12,13
    INT8U Period;// (second) {default:3 second}
*/ 

    //DEBUGMSG(1,"[%s][len=%d]\n",p,len);
    decode_url(p);	
    //DEBUGMSG(1,"after decode=\n[%s]\n",p);

    //invalid value will be skipped to set
    //only valid value is set
	clearMailServeripv4Addr();
	

#if 0	
    keyword = (char *) malloc(strlen("mail_wake_up"));
    sprintf(keyword,"%s","mail_wake_up");    
    loc = strstr(p,keyword);
    value_str = (char *) malloc(strlen(p));
    memset(value_str,0,strlen(p));
    if (url_value(p,keyword,value_str) != 0) //error
    {
        DEBUGMSG(1,"[%s] not found! \n",keyword);  
        invalid = 1;  
    }
    else
    {
        if ((value_str[0] != '0')&&(value_str[0] != '1'))//only 0 and 1 are valid
        {
            DEBUGMSG(1,"%s = [%s] not valid (0 and 1 are valid)\n", keyword, value_str);
            invalid = 1;
        }        
        else
        {
            //DEBUGMSG(1,"%s = [%s] \n", keyword, value_str);
            dpconf->mail_wake_up = atoi(value_str);      
//             setdirty(DPCONFIGTBL);//write back to the flash      
        }        
    }
    free(value_str);
    free(keyword);
#else
   dpconf->mail_wake_up = 0;
   
    if(strstr(p,"ckbox_en") != NULL){
		dpconf->mail_wake_up = 0x01;
    }
#endif


    keyword = (char *) malloc(strlen("e_svr"));
    sprintf(keyword,"%s","e_svr");    
    loc = strstr(p,keyword);
    value_str = (char *) malloc(strlen(p));
    memset(value_str,0,strlen(p));    
    if (url_value(p,keyword,value_str) != 0) //error
    {
        DEBUGMSG(1,"%s not found! \n",keyword);    
        invalid = 1;          
    }
    else
    {
//        DEBUGMSG(1,"%s = [%s] \n", keyword, value_str);
        memset(dpconf->email_server,0,MWU_MAX_STR);
        memcpy(dpconf->email_server, value_str, strlen(value_str));
//        setdirty(DPCONFIGTBL);//write back to the flash      
        
    }
    free(value_str);
    free(keyword);

 #if 0

    keyword = (char *) malloc(strlen("SSL"));
    sprintf(keyword,"%s","SSL");    
    loc = strstr(p,keyword);
    value_str = (char *) malloc(strlen(p));
    memset(value_str,0,strlen(p));    
    if (url_value(p,keyword,value_str) != 0) //error
    {
        DEBUGMSG(1,"%s not found! \n",keyword);    
        invalid = 1;  
    }
    else
    {
        if ((value_str[0] != '0')&&(value_str[0] != '1'))//only 0 and 1 are valid
        {
//            DEBUGMSG(1,"%s = [%s] not valid (0 and 1 are valid)\n", keyword, value_str);
            invalid = 1;
        }        
        else
        {
//             DEBUGMSG(1,"%s = [%s] \n", keyword, value_str);
             dpconf->SSL = atoi(value_str);            
//             setdirty(DPCONFIGTBL);//write back to the flash      
        }        
    }
    free(value_str);
    free(keyword);
#else
    dpconf->smtp_SSL = 0;
   
    if(strstr(p,"ckbox_ssl") != NULL){
		dpconf->smtp_SSL = 0x01;
    }
#endif


    keyword = (char *) malloc(strlen("e_acc"));
    sprintf(keyword,"%s","e_acc");    
    loc = strstr(p,keyword);
    value_str = (char *) malloc(strlen(p));
    memset(value_str,0,strlen(p));    
    if (url_value(p,keyword,value_str) != 0) //error
    {
        DEBUGMSG(1,"%s not found! \n",keyword);    
        invalid = 1;  
    }
    else
    {
//        DEBUGMSG(1,"%s = [%s] \n", keyword, value_str);
        memset(dpconf->email_acount,0,MWU_MAX_STR);
        memcpy(dpconf->email_acount, value_str, strlen(value_str));
//        setdirty(DPCONFIGTBL);//write back to the flash      
    }
    free(value_str);
    free(keyword);

    keyword = (char *) malloc(strlen("e_psw"));
    sprintf(keyword,"%s","e_psw");    
    loc = strstr(p,keyword);
    value_str = (char *) malloc(strlen(p));
    memset(value_str,0,strlen(p));    
    if (url_value(p,keyword,value_str) != 0) //error
    {
        DEBUGMSG(1,"%s not found! \n",keyword);    
        invalid = 1;  
    }
    else
    {
//        DEBUGMSG(1,"%s = [%s] \n", keyword, value_str);   
        memset(dpconf->email_passwd,0,MWU_MAX_STR);
        memcpy(dpconf->email_passwd, value_str, strlen(value_str));
//        setdirty(DPCONFIGTBL);//write back to the flash      
     }
    free(value_str);
    free(keyword);


    if (invalid)
    {
        rv = sprintf(buf , headerHtm, strlen(invalidParameters));
        offset+=rv;
        rv = sprintf(buf + rv, invalidParameters);
        offset+=rv;
    }
    else
    {
        rv = sprintf(buf , headerHtm, strlen(settingSuccess));
        offset+=rv;
        rv = sprintf(buf + rv, settingSuccess);
        offset+=rv;
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
 
    setdirty(DPCONFIGTBL);//write back to flash    

	//printf("[RTK] set dirty DPCONFIGTBL\n");
	
    free(buf);

}

//mail_wakeup 20121025 eccheng
//#ifdef CONFIG_PRINTER_WEB_PAGE
//pcb->postURL=7 ,MAIL_ACCPOST=7
//p=[m_add=firmware%40192.168.0.18&m_sub=power+on&m_cnt=2.71828&a_cnd=13&Period+=5&save=Save+Settings][len=96]
void mail_mat_set(struct tcp_pcb *pcb, char *p, int len)
{
    char *buf;
    PKT *pkt; 
    int invalid = 0;
    int offset = 0, rv = 0;
    char *value_str;
    char *keyword;    
    char *loc;

    buf = malloc(ETH_PAYLOAD_LEN);
    memset(buf, 0, ETH_PAYLOAD_LEN);

    invalid = 0;
/*
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
<td>Activate_condition</td>\n\
<td><input type=\"text\" name=\"a_cnd\" size=\"2\" maxlength=\"2\" value=\"%d\">only support 7,12,13 now</td>\n\
</tr>\n\
<tr>\n\
<td>Checking Period</td>\n\
<td><input type=\"text\" name=\"Period \" size=\"2\" maxlength=\"2\" value=\"%d\">1~99</td>\n\
</tr>\n\
</table>\n \


    INT8U mail_wake_up; //on/off {on : != 0, off : = 0}
    INT8U email_server[MWU_MAX_STR]; //[max.length=128] {IP, Domain Name}//POP3(110) and POP3S(995)
    INT8U SSL; //enable {disable (port=110), enable (port=995)}
    INT8U email_acount[MWU_MAX_STR]; //[max.length=128] 
    INT8U email_passwd[MWU_MAX_STR]; //[max.length=128] 
    
    INT8U match_address[MWU_MAX_STR]; //(from) [max.length=128]
    INT8U match_subject[MWU_MAX_STR]; //[max.length=128] 
    INT8U match_keyword[MWU_MAX_STR]; //[max.length=128]„²only check email content ¡¥s first 1024 bytes
    INT8U Activate_condition; //{condition code = 1..13 }, now only support 7,12,13
    INT8U Period;// (second) {default:3 second}
*/ 

    //DEBUGMSG(1,"[%s][len=%d]\n",p,len);
    decode_url(p);
    //DEBUGMSG(1,"after decode=\n[%s]\n",p);

    //invalid value will be skipped to set
    //only valid value is set
    keyword = (char *) malloc(strlen("m_add"));
    sprintf(keyword,"%s","m_add");    
    loc = strstr(p,keyword);
    value_str = (char *) malloc(strlen(p));
    memset(value_str,0,strlen(p));    
    if (url_value(p,keyword,value_str) != 0) //error
    {
        DEBUGMSG(1,"%s not found! \n",keyword);    
        invalid = 1;          
    }
    else
    {
        //DEBUGMSG(1,"%s = [%s] \n", keyword, value_str);
        memset(dpconf->match_address,0,MWU_MAX_STR);
        memcpy(dpconf->match_address, value_str, strlen(value_str));
//        setdirty(DPCONFIGTBL);//write back to the flash      
        
    }
    free(value_str);
    free(keyword);


    keyword = (char *) malloc(strlen("m_sub"));
    sprintf(keyword,"%s","m_sub");    
    loc = strstr(p,keyword);
    value_str = (char *) malloc(strlen(p));
    memset(value_str,0,strlen(p));    
    if (url_value(p,keyword,value_str) != 0) //error
    {
        DEBUGMSG(1,"%s not found! \n",keyword);    
        invalid = 1;          
    }
    else
    {
//        DEBUGMSG(1,"%s = [%s] \n", keyword, value_str);
        memset(dpconf->match_subject,0,MWU_MAX_STR);
        memcpy(dpconf->match_subject, value_str, strlen(value_str));
//        setdirty(DPCONFIGTBL);//write back to the flash      
        
    }
    free(value_str);
    free(keyword);
    
    keyword = (char *) malloc(strlen("m_cnt"));
    sprintf(keyword,"%s","m_cnt");    
    loc = strstr(p,keyword);
    value_str = (char *) malloc(strlen(p));
    memset(value_str,0,strlen(p));    
    if (url_value(p,keyword,value_str) != 0) //error
    {
        DEBUGMSG(1,"%s not found! \n",keyword);    
        invalid = 1;          
    }
    else
    {
        DEBUGMSG(1,"%s = [%s] \n", keyword, value_str);
        memset(dpconf->match_keyword,0,MWU_MAX_STR);
        memcpy(dpconf->match_keyword, value_str, strlen(value_str));
//        setdirty(DPCONFIGTBL);//write back to the flash      
        
    }
    free(value_str);
    free(keyword);    
                  
#if 0
				  
    keyword = (char *) malloc(strlen("a_cnd"));
    sprintf(keyword,"%s","a_cnd");    
    loc = strstr(p,keyword);
    value_str = (char *) malloc(strlen(p));
    memset(value_str,0,strlen(p));
    if (url_value(p,keyword,value_str) != 0) //error
    {
        DEBUGMSG(1,"[%s] not found! \n",keyword);  
        invalid = 1;  
    }
    else
    {   //only 7,12,13 support, the htm will quarantee this limitation
/*        
        if ((value_str[0] != '0')&&(value_str[0] != '1'))//only 0 and 1 are valid
        {
            DEBUGMSG(1,"%s = [%s] not valid (0 and 1 are valid)\n", keyword, value_str);
            invalid = 1;
        }        
        else
        {
*/
//            DEBUGMSG(1,"%s = [%s] \n", keyword, value_str);
            dpconf->Activate_condition = atoi(value_str);      
//             setdirty(DPCONFIGTBL);//write back to the flash      
//        }        
    }
    free(value_str);
    free(keyword);
#else
	dpconf->Activate_condition = 0;
   
    if(strstr(p,"op_addr") != NULL){
		dpconf->Activate_condition += 0x01;
    }
	
    if(strstr(p,"op_sub")  != NULL){
		dpconf->Activate_condition += 0x02;
    }	
    if(strstr(p,"op_cont")  != NULL){
		dpconf->Activate_condition += 0x04;
    }

	//printf("[RTK] Activate_condition = 0x%x\n",dpconf->Activate_condition);
	
#endif
	

    keyword = (char *) malloc(strlen("Period"));
    sprintf(keyword,"%s","Period");    
    loc = strstr(p,keyword);
    value_str = (char *) malloc(strlen(p));
    memset(value_str,0,strlen(p));
    if (url_value(p,keyword,value_str) != 0) //error
    {
        DEBUGMSG(1,"[%s] not found! \n",keyword);  
        invalid = 1;  
    }
    else
    {   //only 1-99 support, the htm will quarantee this limitation
/*        
        if ((value_str[0] != '0')&&(value_str[0] != '1'))//only 0 and 1 are valid
        {
            DEBUGMSG(1,"%s = [%s] not valid (0 and 1 are valid)\n", keyword, value_str);
            invalid = 1;
        }        
        else
        {
*/
//            DEBUGMSG(1,"%s = [%s] \n", keyword, value_str);
            dpconf->mailPeriod= atoi(value_str);      
//             setdirty(DPCONFIGTBL);//write back to the flash      
//        }        
    }
    free(value_str);
    free(keyword);

    if (invalid)
    {
        rv = sprintf(buf , headerHtm, strlen(invalidParameters));
        offset+=rv;
        rv = sprintf(buf + rv, invalidParameters);
        offset+=rv;
    }
    else
    {
        rv = sprintf(buf , headerHtm, strlen(settingSuccess));
        offset+=rv;
        rv = sprintf(buf + rv, settingSuccess);
        offset+=rv;
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
 
    setdirty(DPCONFIGTBL);//write back to flash   
    free(buf);

}

//#endif
#endif
char* urltoken(char *in, char *token, int* len)
{
	int tokenlen = 0;
	char *ptr;
	char *ptr1;
	char *cmpstr;

	tokenlen = strlen(token);
	cmpstr = (char*)malloc(tokenlen + 2);
	memcpy(cmpstr, token, tokenlen);
	cmpstr[tokenlen] = '=';
	cmpstr[tokenlen + 1] = '\0';
	ptr = strstr(in, cmpstr);
	if(ptr)
	{
		ptr = ptr + tokenlen + 1;
	}
	free(cmpstr);
	
	if(ptr)
	{		
		ptr1 = strchr(ptr, '&');
		if(ptr1)
		{
			*len = ptr1 - ptr;
			return ptr;			
		}
		else
		{
			*len = strlen(ptr);			
			return  ptr;
		}
	}	
	else
		return 0;

	return 0;
}

#if CONFIG_WIFI_ENABLED	
void wificfgfun(struct tcp_pcb *pcb, char *p, int len)
{
	char *buf;
	char *ptr;
	char *ptr1;
	PKT	 *pkt;	
	int rv = 0, rv1 = 0;
	int offset =0;
	char code = 0;
	WIFICfg wificfg;

	buf = malloc(ETH_PAYLOAD_LEN);
	memset(buf, 0, ETH_PAYLOAD_LEN);

	if(len > 100)
		code = 1;
	else
	{	
		memset(&wificfg, 0, sizeof(wificfg));
		do{
			ptr = urltoken(p, "wifiSt", &rv);
			
			if(ptr == 0)
			{
				code = 1;
				break;
			}
			else
			{
				if (strncmp("on", ptr, strlen("on")) == 0)
					wificfg.wifion =1;
				else
					wificfg.wifion = 0;
				
			}
			
			ptr = urltoken(p, "pn", &rv);			
			if(ptr == 0)
			{
				code = 1;
				break;
			}
			else
			{
				memcpy(wificfg.pf_name, ptr, rv);
			}

			ptr = urltoken(p, "netn", &rv);			
			if(ptr == 0)
			{
				code = 1;
				break;
			}
			else
			{
				memcpy(wificfg.SSID, ptr, rv);
			}

			ptr = urltoken(p, "auth", &rv);			
			if(ptr == 0)
			{
				code = 1;
				break;
			}
			else
			{
				wificfg.auth = ptr[0] - '0';				
			}

			ptr = urltoken(p, "enc", &rv);			
			if(ptr == 0)
			{
				code = 1;
				break;
			}
			else
			{
				wificfg.enc = ptr[0] - '0';				
			}
			
			ptr = urltoken(p, "pw", &rv);
			ptr1 = urltoken(p, "cfmpw", &rv1);
			if((ptr == 0) || (ptr1 == 0))
			{
				code = 2;
				break;
			}
			else
			{
				if(rv != rv1)
				{				
					code = 2;
					break;
				}
				else
				{
					if (strncmp(ptr, ptr1, rv) == 0)
					{
						memcpy( wificfg.pwd, ptr, rv);
					}
					else
					{
						code = 2;
						break;
					}
				}				
			}
			
			memcpy(&dpconf->wificfg, &wificfg, sizeof(wificfg));
			setdirty(DPCONFIGTBL);
		}while(0);
	}

	if(code == 0)
	{			
		rv = sprintf(buf, headerHtm, strlen(settingSuccess));
		offset+=rv;
		rv = sprintf(buf + rv, settingSuccess);
		offset+=rv;
	}	
	else if(code == 1)
	{
		rv = sprintf(buf , headerHtm, strlen(unKnownOperation));
		offset+=rv;
		rv = sprintf(buf + rv, unKnownOperation);
		offset+=rv;
	}else if(code == 2)
	{
		rv = sprintf(buf , headerHtm, strlen(pwdError));
		offset+=rv;
		rv = sprintf(buf + rv, pwdError);
		offset+=rv;
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
    free(buf);
}
#endif