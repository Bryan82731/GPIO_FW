/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: debug.h,v 1.17 2001/01/31 10:57:26 adam Exp $
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <sys/autoconf.h>

//#if !CONFIG_UART_DEBUG && !CONFIG_REMOTE_DEBUG
#if !CONFIG_REMOTE_DEBUG
#define printf(...)
#define DEBUGMSG(debug, ...) 
#else

#ifndef CONFIG_LIB_DEBUG
#define CONFIG_LIB_DEBUG       0 
#endif

#ifndef CONFIG_ARP_DEBUG
#define CONFIG_ARP_DEBUG       0
#endif

#ifndef CONFIG_IP_DEBUG 
#define CONFIG_IP_DEBUG        0
#endif

#ifndef CONFIG_UDP_DEBUG
#define CONFIG_UDP_DEBUG       0
#endif

#ifndef CONFIG_ICMP_DEBUG
#define CONFIG_ICMP_DEBUG      0
#endif

#ifndef CONFIG_DHCPv4_DEBUG 
#define CONFIG_DHCPv4_DEBUG    0
#endif

#ifndef CONFIG_DHCPv6_DEBUG
#define CONFIG_DHCPv6_DEBUG    0
#endif

#ifndef CONFIG_TCP_DEBUG
#define CONFIG_TCP_DEBUG       0
#endif

#ifndef CONFIG_TCP_RX_DEBUG   
#define CONFIG_TCP_RX_DEBUG    0
#endif

#ifndef CONFIG_TCP_TX_DEBUG 
#define CONFIG_TCP_TX_DEBUG    0
#endif

#ifndef CONFIG_SKT_DEBUG
#define CONFIG_SKT_DEBUG       0
#endif

#ifndef CONFIG_HTTP_DEBUG
#define CONFIG_HTTP_DEBUG      0
#endif

#ifndef CONFIG_HTTPS_DEBUG
#define CONFIG_HTTPS_DEBUG     0
#endif

#ifndef CONFIG_CLIENT_DEBUG
#define CONFIG_CLIENT_DEBUG    0
#endif

#ifndef CONFIG_SRV_DEBUG
#define CONFIG_SRV_DEBUG       0
#endif

#ifndef CONFIG_HTTP_CA_DEBUG
#define CONFIG_HTTP_CA_DEBUG   0
#endif

#ifndef CONFIG_HTTP_CB_DEBUG
#define CONFIG_HTTP_CB_DEBUG   0
#endif

#ifndef CONFIG_HTML_DEBUG
#define CONFIG_HTML_DEBUG      0
#endif

#ifndef CONFIG_SMBIOS_DEBUG
#define CONFIG_SMBIOS_DEBUG    0
#endif

#ifndef CONFIG_DASH_DEBUG
#define CONFIG_DASH_DEBUG      0
#endif

#ifndef CONFIG_PROVISION_DEBUG
#define CONFIG_PROVISION_DEBUG 0
#endif

#ifndef CONFIG_PLDM_DEBUG
#define CONFIG_PLDM_DEBUG      0
#endif

#ifndef CONFIG_WCOM_DEBUG
#define CONFIG_WCOM_DEBUG      0
#endif

#ifndef CONFIG_USB_DEBUG
#define CONFIG_USB_DEBUG       0
#endif

#ifndef CONFIG_TCR_DEBUG
#define CONFIG_TCR_DEBUG       0
#endif

#ifndef CONFIG_HOOK_DEBUG
#define CONFIG_HOOK_DEBUG      0
#endif

#ifndef CONFIG_SMBUS_DEBUG
#define CONFIG_SMBUS_DEBUG     0
#endif

#ifndef CONFIG_SMBUSTRA_DEBUG
#define CONFIG_SMBUSTRA_DEBUG     0
#endif

#ifndef CONFIG_SENSOR_DEBUG
#define CONFIG_SENSOR_DEBUG    0
#endif

#ifndef CONFIG_DRIVER_DEBUG   
#define CONFIG_DRIVER_DEBUG    0
#endif

#ifndef CONFIG_MESSAGE_DEBUG
#define CONFIG_MESSAGE_DEBUG   0
#endif

#ifndef CONFIG_CRITICAL_ERROR
#define CONFIG_CRITICAL_ERROR  0
#endif

#define LIB_DEBUG        (CONFIG_LIB_DEBUG)
#define ARP_DEBUG        (CONFIG_ARP_DEBUG       <<  1)
#define IP_DEBUG         (CONFIG_IP_DEBUG        <<  2)
#define UDP_DEBUG	 (CONFIG_UDP_DEBUG       <<  3)
#define ICMP_DEBUG       (CONFIG_ICMP_DEBUG      <<  4)
#define DHCPv4_DEBUG	 (CONFIG_DHCPv4_DEBUG    <<  5)
#define DHCPv6_DEBUG	 (CONFIG_DHCPv6_DEBUG    <<  6)
#define TCP_DEBUG        (CONFIG_TCP_DEBUG       <<  7)
#define TCP_RX_DEBUG	 (CONFIG_TCP_RX_DEBUG    <<  8)
#define TCP_TX_DEBUG	 (CONFIG_TCP_TX_DEBUG    <<  9)
#define SKT_DEBUG	 (CONFIG_SKT_DEBUG       << 10)		
#define HTTP_DEBUG       (CONFIG_HTTP_DEBUG      << 11)
#define HTTPS_DEBUG	 (CONFIG_HTTPS_DEBUG     << 12)
#define CLIENT_DEBUG	 (CONFIG_CLIENT_DEBUG    << 13)
#define SRV_DEBUG        (CONFIG_SRV_DEBUG       << 14)
//Dash Class A
#define HTTP_CA_DEBUG    (CONFIG_HTTP_CA_DEBUG   << 15)
//Dash Class B
#define HTTP_CB_DEBUG    (CONFIG_HTTP_CB_DEBUG   << 16)
#define HTML_DEBUG	 (CONFIG_HTML_DEBUG      << 17)	
#define SMBIOS_DEBUG	 (CONFIG_SMBIOS_DEBUG    << 18)
#define DASH_DEBUG       (CONFIG_DASH_DEBUG      << 19)
#define PROVISION_DEBUG  (CONFIG_PROVISION_DEBUG << 20)
#define PLDM_DEBUG       (CONFIG_PLDM_DEBUG      << 21)
#define WCOM_DEBUG       (CONFIG_WCOM_DEBUG      << 22)
#define USB_DEBUG        (CONFIG_USB_DEBUG       << 23)
#define TCR_DEBUG        (CONFIG_TCR_DEBUG       << 24)
#define HOOK_DEBUG       (CONFIG_HOOK_DEBUG      << 25)

#define SMBUSTRA_DEBUG   (CONFIG_SMBUSTRA_DEBUG  << 26)
#define SMBUS_DEBUG      (CONFIG_SMBUS_DEBUG     << 27)
#define SENSOR_DEBUG     (CONFIG_SENSOR_DEBUG    << 28)
#define DRIVER_DEBUG     (CONFIG_DRIVER_DEBUG    << 29)
#define MESSAGE_DEBUG	 (CONFIG_MESSAGE_DEBUG   << 30)		 
#define CRITICAL_ERROR   (CONFIG_CRITICAL_ERROR  << 31)
 
#define DBGSETTINGS      0xFFFFFFFF

#define TCP_PROXY_DEBUG  0
#define TCP_PROXY_RTO_DEBUG  1
#define IP_PROXY_DEBUG   0


#define DEBUGMSG(debug, ...) { if((debug) & DBGSETTINGS) printk(__VA_ARGS__); }
#endif

#endif /* __DEBUG_H__ */






