/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	telnet.c

Abstract:	telnet module

*****************************************************************************
*/


#include <string.h>
#include <stdlib.h>
#include "tcp.h"
#include "rtskt.h"
#include "usb.h"
#include "bsp.h"
#include "http.h"
#include "telnet.h"
#include "lib.h"
#include "rmcp.h"
#include "OOBMAC_COMMON.h"

#include "i2c_gpio_slave.h"
//#include "i2c_gpio_master.h"


extern TCRCB tcrcb;
extern USBCB usbcb;
extern OS_EVENT *TCRQ;
extern DPCONF *dpconf;
extern unsigned char tip[4];
extern const INT8U tcr_hsstr[4][6];
extern const INT8U tcr_hslen[4];
extern OS_EVENT *USBQ;
extern const INT16U usb_sector_size[3]; 
RTSkt *usbcs;
extern RTSkt* usbSrv;

const INT8U lf[3] = {0x0d, 0x0a, 0x00};

#ifdef CONFIG_USB_ENABLED
void sendUSBPkt_F(PKT *rxPkt)
{
    static INT16U nowsize = 0;
    static INT16U remainsize = 0;

    usbcb.usblen -= rxPkt->len;

    if((usbcb.usblen == 0) && (usbcb.sendlen > 32768) && (usbcb.usbmode != SERVER_MODE))
    {
        usbcb.usbSktconnected = 0;
        stopconn_F(usbcs);
        usbcs = rtSkt_F(IPv4, IP_PROTO_TCP);
#if CONFIG_VERSION >= 6
        rtSktEnableIBPortFilter_F(usbcs);
#endif       
        rtSktEnableTimeWait_F(usbcs);
        while (rtSktConnect_F(usbcs, tip, usbcb.port) == -1)
            OSTimeDly(1);
        usbcb.usbSktconnected = 1;
    }

    while (rxPkt->len)
    {

        if (nowsize + rxPkt->len >= 2048)
        {
            remainsize = 2048 - nowsize;
            memcpy(usbcb.rbuf+nowsize, rxPkt->wp, remainsize);
            usbcb.usb_ep_start_transfer(2048, usbcb.rbuf, is_IN );
            nowsize = 0;
            rxPkt->wp += remainsize;
            rxPkt->len -= remainsize;
        }
        else if (nowsize + rxPkt->len >= 1024)
        {
            remainsize = 1024 - nowsize;
            memcpy(usbcb.rbuf+nowsize, rxPkt->wp, remainsize);
            usbcb.usb_ep_start_transfer(1024, usbcb.rbuf, is_IN );
            nowsize = 0;
            rxPkt->wp += remainsize;
            rxPkt->len -= remainsize;
        }
        else if (nowsize + rxPkt->len >= 512)
        {
            remainsize = 512 - nowsize;
            memcpy(usbcb.rbuf+nowsize, rxPkt->wp, remainsize);
            usbcb.usb_ep_start_transfer(512, usbcb.rbuf, is_IN );
            nowsize = 0;
            rxPkt->wp += remainsize;
            rxPkt->len -= remainsize;
        }
        else
        {
            memcpy(usbcb.rbuf+nowsize, rxPkt->wp, rxPkt->len);
            nowsize += rxPkt->len;
            rxPkt->len = 0;
        }
    }
}

void USBClientTask(void *data)
{
    PKT *rxPkt;
    INT32S status = 0;
    INT8U usbtype  = 0;

    INT8U err;
    QHdr *rxq;

    INT8U count = 0;
    rxq = (QHdr *)data;
    usbcb.port = rxq->port;
    free(rxq);
    do
    {
        usbcb.usbSktconnected = 0;
        usbcs = rtSkt_F(IPv4, IP_PROTO_TCP);
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
        rtSktEnableIBPortFilter_F(usbcs);
#endif
        rtSktEnableTimeWait_F(usbcs);
        if (rtSktConnect_F(usbcs, tip, usbcb.port) == -1)
        {
            rtSktClose_F(usbcs);
            OSSemDel(usbcs->rxSem, OS_DEL_ALWAYS, &err);
            OSSemDel(usbcs->txSem, OS_DEL_ALWAYS, &err);
            usbcs->rxSem=0;
            usbcs->txSem=0;
            if (usbcs)
                free(usbcs);
            usbcs=NULL;
            count++;
        }
        else
            usbcb.usbSktconnected = 1;

        if (count < 50) //5 seconds timeout
            OSTimeDly(10);
        else
            OSTaskDel(OS_PRIO_SELF);

    } while (!usbcs);

    //get sector 17 to identify whether it is a ISO image
    if (USBDataWebGet(usbcs, 32768, 32774) == SKTDISCONNECT)
    {
        OSTaskDel(OS_PRIO_SELF);
        if (usbcs)
            free(usbcs);
        usbcs = NULL;
        usbcb.usbSktconnected = 0;
    }
    usbcb.waitRx = 1;
    usbcb.usbstate = ACCEPTED;
    usbcb.usbmode = CLIENT_MODE;
    usbcb.disksize = 0;

    while (OS_TRUE)
    {
        if (usbcb.waitRx)
        {
            while ((rxPkt = rtSktRx_F(usbcs, OS_TICKS_PER_SEC/*50*/, &status)) == 0)
            {
                if (status == SKTDISCONNECT)
                {
                    usbcb.usbSktconnected = 0;
                    stopconn_F(usbcs);
                    usbcs = rtSkt_F(IPv4, IP_PROTO_TCP);
#if CONFIG_VERSION == IC_VERSION_DP_RevF
                    rtSktEnableIBPortFilter_F(usbcs);
#endif
                    rtSktEnableTimeWait_F(usbcs);
                    while (rtSktConnect_F(usbcs, tip, usbcb.port) == -1)
                        OSTimeDly(1);
                    usbcb.usbSktconnected = 1;
                }
            }

            if (getWebData(rxPkt))
                continue;

            //ask for default MBR
            if (usbcb.usbstate == ACCEPTED)
            {
                if(usbtype == 0)
                {
                    getType(rxPkt);
                    usbtype = 1;
                    freePkt_F(rxPkt);
                    continue;
                }
                else
                    getMBR(rxPkt);

            }
            else
            {
                sendUSBPkt_F(rxPkt);
				//uncomment this for EP?
                /*
                if (usbcb.usblen == 0)
                {
                    usbcb.usbSktconnected = 0;
                    stopconn_F(usbcs);
                    usbcs = rtSkt_F(IPv4, IP_PROTO_TCP);
                    rtSktEnableTimeWait_F(usbcs);
                    while (rtSktConnect_F(usbcs, tip, usbcb.port) == -1)
                        OSTimeDly(1);
                    usbcb.usbSktconnected = 1;
                }
                */
            }

            freePkt_F(rxPkt);

            if (usbcb.usblen == 0)
            {
                usbcb.waitRx = 0;
                if(!dpconf->usbotg)
                {
                    ata_csw();
                }
                rlx_irq_set_mask(usbcb.irqnum, OS_FALSE);
            }

        }
        else
        {
            rxq = OSQPend(USBQ, 0, &err);
            usbcb.qhdr = (INT8U *) rxq;
            USBQHandle(rxq);
            free(rxq);
            usbcb.qhdr = NULL;

        }

    }

}

extern  OS_CPU_SR  cpu_sr;

void FREQ()
{
	OOBMAC_gpio_init(GPIO12,OUTPUT,0);
	while(1)
		{
		OOBMAC_gpio_output(GPIO12,1);
		OOBMAC_gpio_output(GPIO12,0);
		}
}

void USBSrvTask(void *data)
{

    QHdr *rxq;
    INT8U err;
#if !defined(CONFIG_USB_BULK_TEST) && !defined(CONFIG_USB_FLASH)
    PKT *rxPkt;
    usbSrv = rtSkt_F(IPv4, IP_PROTO_TCP);
    INT32S status = 0;

    rxq = (QHdr *)data;

    rtSktBind_F(usbSrv, rxq->port);
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F(usbSrv);
#endif
    rtSktListen_F(usbSrv, 1);


    usbcb.usbstate = LISTENED;
    usbcs = rtSktAccept_F(usbSrv);
    usbcb.usbstate = ACCEPTED;

    usbcb.usbtype = rxq->option;
    usbcb.sectorsize = usb_sector_size[usbcb.usbtype];
    usbcb.waitRx = 0;
    usbcb.usbmode = SERVER_MODE;
    usbcb.usbstate = ENABLED;
    usbcb.bsp_usb_enabled();
	 usbcb.usbstate = ENABLED;//+Tom 20131105
    free(rxq);
#else
	usbcb.bsp_usb_disabled();//otg_usb_disabled
	UMAC_RST();
    usbcb.waitRx = 0;
    usbcb.usbmode = SERVER_MODE;
	usbcb.bsp_usb_init();//otg_usb_init
	usbcb.bsp_usb_enabled();//otg_usb_enabled
#endif
    while (OS_TRUE)
    {
#if !defined(CONFIG_USB_BULK_TEST) && !defined(CONFIG_USB_FLASH)
        if (usbcb.waitRx)
        {
            while ((rxPkt = rtSktRx_F(usbcs, 50, &status)) == 0)
            {
                if (usbcb.delrequest == 1)
                    OSTimeDly(50);
                else if (status == SKTDISCONNECT)
                {
                    stopUSB();
                }
            }

            usbcb.pkt = rxPkt;

            if (usbcb.CBW->rbc[0] == READ_FORMAT_CAPACITIES || usbcb.CBW->rbc[0] == READ_CAPACITY )
            {
                //should be read capacity or read format capacity
                //rxPkt->wp[3] is the length for sent
                //usblen is the length that in CBW

                usbcb.usb_ep_start_transfer(rxPkt->wp[3], rxPkt->wp+4, is_IN );
                usbcb.CSW->dCSWDataResidue = usbcb.usblen - rxPkt->wp[3];
                usbcb.usblen = 0;
            }
            else if (usbcb.CBW->rbc[0] == READ_TOC || usbcb.CBW->rbc[0] == GET_CONFIGURATION || usbcb.CBW->rbc[0] == READ_TRACK_INFORMATION)
            {
                usbcb.usb_ep_start_transfer(usbcb.usblen, rxPkt->wp+4, is_IN );
                usbcb.usblen = 0;

            }
            else
            {
                sendUSBPkt_F(rxPkt);
            }
            freePkt_F(rxPkt);
            usbcb.pkt = NULL;

            if (usbcb.usblen == 0)
            {
                usbcb.waitRx = 0;
                if(!dpconf->usbotg)
                {
                    ata_csw();
                }
                rlx_irq_set_mask(usbcb.irqnum, OS_FALSE);
            }
        }
        else
#endif
        {


        
        	printk("ENter GPIO_test \r\n");
			I2C_gpio_init();
        
            //rxq = OSQPend(USBQ, 50, &err);
            rxq = OSQPend(USBQ, 0, &err);
            //if (rxq)
            {
                usbcb.qhdr = (INT8U *) rxq;
				OS_ENTER_CRITICAL();
                USBQHandle(rxq);
				OS_EXIT_CRITICAL();
                free(rxq);
                usbcb.qhdr = NULL;
            }
#if 0
#if !defined(CONFIG_USB_BULK_TEST) && !defined(CONFIG_USB_FLASH)
            else if (usbcb.waitRx == 0)
            {
                rxPkt = rtSktRx_F(usbcs, 50, &status);
                if (rxPkt)
                    freePkt_F(rxPkt);
            }
#endif
#endif

        }

    }
}
#endif


#if  defined (CONFIG_TELNET_ENABLED)  || defined (CONFIG_SSH_ENABLED)
OS_STK  TelnetClientStk[TASK_TELNET_CLIENT_STK_SIZE];
#endif

#if  defined (CONFIG_TELNET_ENABLED)  || defined (CONFIG_SSH_ENABLED) || defined(CONFIG_TCR_ENABLED)
unsigned char TelLogin[] = {"Login: "};
unsigned char TelPwd[] = {"Password: "};
unsigned char TelLoginSuccMsg[] =
{
    "\r\n\r\nLogin successful"
};
unsigned char TelLoginMsg[] =
{
    "\r\n\r\nWelcome to Realtek out of band service"
    "\r\nYou can type \"help\" to get help\r\n"
};
unsigned char TelByeByeMsg[] = {"Bye Bye!"};
unsigned char TelHelpMsg[] =
{
    "boot           Boot system"
    "\r\nexit       Exit the shell"
    "\r\nipinfo     Show current ip setting"
    "\r\nrestart    System restart"
};
unsigned char TelUnknowCMDMsg[] = {"Command not found"};
unsigned char NewTelCli = 0;

typedef struct _TelnetCliArg
{
    RTSkt* cs;
    TelnetSession* session;
} TelnetCliArg;
UserInfo *uInfo;
extern DPCONF *dpconf;
extern smbiosrmcpdata *smbiosrmcpdataptr;

void flushTelnetSendBuffer(TelnetSession* session)
{
    PKT* txPkt;
    unsigned char *p;

    if ((session->sendBufOffset + session->optBufOffset) == 0)
        return;

    if (session->ssh)
    {
        if (session->ssh->banner == 0)
        {
            txPkt = (PKT*)allocSSHPkt_F(session->ssh, 4+session->optBufOffset+session->sendBufOffset, 1);
        }
        else
        {
            txPkt = (PKT*)allocSSHPkt_F(session->ssh, 4+session->optBufOffset+session->sendBufOffset+4+2, 0);
        }
        p = (unsigned char*)(txPkt->wp + 4);
    }
    else
    {
        txPkt = (PKT*)allocPkt_F(session->sendBufOffset + session->optBufOffset);
        p = (unsigned char*)txPkt->wp;
    }
    if (session->optBufOffset)
    {
        memcpy(p, session->optBuf, session->optBufOffset);
    }

    if (session->sendBufOffset)
    {
        memcpy(p + session->optBufOffset, session->sendBuf, session->sendBufOffset);
    }


    if (session->ssh)
    {
        setInt(txPkt->wp, session->sendBufOffset + session->optBufOffset);
        if (session->ssh->banner == 0)
        {
            sshEncodeData(txPkt, 4+session->sendBufOffset + session->optBufOffset, session->ssh);
        }
        else
        {
            setInt(txPkt->wp+4+session->sendBufOffset + session->optBufOffset, 2);
            memcpy(txPkt->wp+4+session->sendBufOffset + session->optBufOffset+4, "en", 2);
            sshEncodeBanner(txPkt, 4+session->sendBufOffset + session->optBufOffset+4+2, session->ssh);
        }
    }
    else
    {
        rtSktSend_F(session->cs, txPkt, txPkt->len);
    }
    session->sendBufOffset = 0;
    session->optBufOffset = 0;
}

void sendNVTOpt(TelnetSession* session, unsigned char negoCode, unsigned char optCode)
{
    if ((session->optBufOffset + 3) >  MAX_TLENET_BUF_SIZE)
    {
        flushTelnetSendBuffer(session);
    }
    session->optBuf[session->optBufOffset + 0] = NVT_CTL_IAC;
    session->optBuf[session->optBufOffset + 1] = negoCode;
    session->optBuf[session->optBufOffset + 2] = optCode;
    session->optBufOffset += 3;
}
void sendNVTSubOpt(TelnetSession* session, unsigned char optCode)
{
    if ((session->optBufOffset + 5) >  MAX_TLENET_BUF_SIZE)
    {
        flushTelnetSendBuffer(session);
    }
    session->optBuf[session->optBufOffset + 0] = NVT_CTL_IAC;
    session->optBuf[session->optBufOffset + 1] = NVT_CTL_SB;
    session->optBuf[session->optBufOffset + 2] = optCode;
    session->optBuf[session->optBufOffset + 3] = 0x01;
    session->optBuf[session->optBufOffset + 4] = NVT_CTL_IAC;
    session->optBuf[session->optBufOffset + 5] = NVT_CTL_SE;
    session->optBufOffset += 6;
}
void sendTelnetData(TelnetSession* session, unsigned char* data, unsigned int len)
{
    PKT* txPkt;
    unsigned int i = 0;
    if ((len >= MAX_TLENET_BUF_SIZE) || ((len + session->sendBufOffset) >= MAX_TLENET_BUF_SIZE) )
    {
        flushTelnetSendBuffer(session);
        i = 0;
        while (len)
        {
            if (len >= ETH_PAYLOAD_LEN)
            {
                txPkt = (PKT*)allocPkt_F(ETH_PAYLOAD_LEN);
                memcpy(txPkt->wp, data + (i * ETH_PAYLOAD_LEN), ETH_PAYLOAD_LEN);
                rtSktSend_F(session->cs, txPkt, ETH_PAYLOAD_LEN);
                len = len - ETH_PAYLOAD_LEN;
                i++;
            }
            else
            {
                txPkt = (PKT*)allocPkt_F(len);
                memcpy(txPkt->wp, data + (i * ETH_PAYLOAD_LEN), len);
                rtSktSend_F(session->cs, txPkt, len);
                len = 0;
            }
        }
    }
    else
    {
        memcpy(session->sendBuf + session->sendBufOffset, data, len);
        session->sendBufOffset += len;
    }
}
void putCMDBuf(TelnetSession* session, unsigned char data)
{
    session->cmdBuf[session->cmdBufOffset] = data;
    session->cmdBufOffset++;
    if (session->cmdBufOffset >= MAX_TLENET_BUF_SIZE)
    {
        session->cmdBufOffset = 0;
    }
}
void putSendBuf(TelnetSession* session, unsigned char data)
{
    if (session->sendBufOffset > (MAX_TLENET_BUF_SIZE/2) )
    {
        flushTelnetSendBuffer(session);
    }
    session->sendBuf[session->sendBufOffset] = data;
    session->sendBufOffset++;
}
void parseCmdBufffer(TelnetSession* session)
{
    char* tmp = (char*)malloc(32);
    if (session->state == TEL_AUTH_PASS)
    {
        putSendBuf(session, '\r');
        putSendBuf(session, '\n');
        switch (session->cmdBufOffset)
        {
        case 4:
            if (strncmp("help", (const char *)session->cmdBuf, 4) == 0)
            {
                sendTelnetData(session, TelHelpMsg, strlen((const char *)TelHelpMsg));
            }
            else if (strncmp("exit", (const char *)session->cmdBuf, 4) == 0)
            {
                sendTelnetData(session, TelByeByeMsg, strlen((const char *)TelByeByeMsg));
                if (session->ssh)
                    send_msg_channel_close(session->ssh);
                else
                    rtSktClose_F(session->cs);
            }
            else if (strncmp("boot", (const char *)session->cmdBuf, 4) == 0)
            {
                SMBus_Prepare_RmtCtrl(RMCP_PWR_On, 0);
            }
            else
            {
                goto TelUnknowCmd;
            }
            break;
        case 6:
            if (strncmp("ipinfo", (const char *)session->cmdBuf, 6) == 0)
            {
                sprintf(tmp,"IP:%d.%d.%d.%d", (dpconf->HostIP[session->cs->pcb->intf].addr&0xff),
                        (dpconf->HostIP[session->cs->pcb->intf].addr&0xff00)>>8, (dpconf->HostIP[session->cs->pcb->intf].addr&0xff0000)>>16,
                        (dpconf->HostIP[session->cs->pcb->intf].addr&0xff000000)>>24);
                sendTelnetData(session, (unsigned char*)tmp, strlen((const char *)tmp));
                sprintf(tmp,"\r\nSubnet Mask:%d.%d.%d.%d", (dpconf->SubnetMask[session->cs->pcb->intf].addr&0xff),
                        (dpconf->SubnetMask[session->cs->pcb->intf].addr&0xff00)>>8, (dpconf->SubnetMask[session->cs->pcb->intf].addr&0xff0000)>>16,
                        (dpconf->SubnetMask[session->cs->pcb->intf].addr&0xff000000)>>24);
                sendTelnetData(session, (unsigned char*)tmp, strlen((const char *)tmp));
            }
            else
            {
                goto TelUnknowCmd;
            }
            break;
        case 7:
            if (strncmp("restart", (const char *)session->cmdBuf, 7) == 0)
            {
                SMBus_Prepare_RmtCtrl(RMCP_Reset, 0);
            }
            else
            {
                goto TelUnknowCmd;
            }
            break;
        default:
TelUnknowCmd:
            sendTelnetData(session, TelUnknowCMDMsg, strlen((const char *)TelUnknowCMDMsg));
        }
        flushTelnetSendBuffer(session);
    }
    else if (session->state == TEL_GET_USERNAME)
    {
        uInfo = getuserinfo();
        while (uInfo)
        {
            if (strlen((const char *)&(uInfo->name[0])) == (session->cmdBufOffset))
            {
                if (strncmp((const char *)&(uInfo->name[0]), (const char *)&(session->cmdBuf[0]), session->cmdBufOffset) == 0)
                {
                    break;
                }
            }
            uInfo = uInfo->next;
        }
        if (!uInfo)
        {
            session->state = TEL_LOGIN;
        }
        else
        {
            session->state = TEL_PWD;
        }
    }
    else if (session->state == TEL_GET_PASSWORD)
    {
        if (!uInfo)
        {
            session->state = TEL_LOGIN;
        }
        else
        {
            if (strlen((const char *)&(uInfo->passwd[0])) == (session->cmdBufOffset))
            {
                if (strncmp((const char *)&(uInfo->passwd[0]), (const char *)&(session->cmdBuf[0]), session->cmdBufOffset) == 0)
                {
                    session->state = TEL_AUTH_PASS;
                    if (session->shell)
                    {
                        sendTelnetData(session, TelLoginMsg, sizeof(TelLoginMsg));
                    }
                    else
                    {
                        sendTelnetData(session, TelLoginSuccMsg, sizeof(TelLoginSuccMsg));
                    }
                    flushTelnetSendBuffer(session);
                    session->uInfo = uInfo;
                }
            }
        }
        if (session->state != TEL_AUTH_PASS)
        {
            session->state = TEL_LOGIN;
        }
    }
    session->cmdBufOffset = 0;
    free(tmp);
}

void sendShellTag(TelnetSession* session)
{
    sendTelnetData(session, session->uInfo->name, strlen((char*)session->uInfo->name));
    putSendBuf(session, '@');
    sendTelnetData(session, (unsigned char*)dpconf->HostName, strlen((char*)dpconf->HostName));
    putSendBuf(session, '>');
}

unsigned int getShellCmd(TelnetSession* session, unsigned char* data, unsigned int offset, unsigned int len)
{
    int i = 0;
    if (data[offset + i] == NVT_CTL_IAC)
    {
        putCMDBuf(session, data[offset]);
        putSendBuf(session, data[offset]);
        i++;
    }
    else
    {
        while ((i < len) && !(data[offset + i] & NVT_CTL_CHAR) && (session->cmdBufOffset < MAX_TLENET_BUF_SIZE) )
        {
            if ((data[offset + i] >= 32) && (data[offset + i] <= 127))
            {
                putCMDBuf(session, data[offset + i]);
                if ((session->state != TEL_GET_PASSWORD) && session->echo)
                {
                    putSendBuf(session, data[offset + i]);
                }
            }
            else
            {
                if (data[offset + i] == '\r')
                {
                    parseCmdBufffer(session);
                    if (session->echo)
                    {
                        putSendBuf(session, '\r');
                        putSendBuf(session, '\n');
                    }
                    if ((session->state == TEL_AUTH_PASS) && session->shell)
                    {
                        sendShellTag(session);
                    }
                }
                else if ((data[offset + i] == 0x08))
                {
                    if (session->cmdBufOffset > 0)
                    {
                        session->cmdBufOffset--;
                        if (session->echo)
                        {
                            putSendBuf(session, data[offset + i]);
                        }
                    }
                    else
                    {
                        putSendBuf(session, 0x07);
                    }
                }
                else
                {
                    if ((data[offset + i] != '\n') && session->echo)
                    {
                        putSendBuf(session, data[offset + i]);
                    }
                }
            }
            i++;
        }
    }
    return i;
}
void negoNVTCTLWill(TelnetSession* session, unsigned char opt)
{
    switch (opt)
    {
    case TEL_OPT_WIN_SIZE:
    case TEL_OPT_TERM_TYPE:
        if (session->cliWillWinSize && (opt == TEL_OPT_WIN_SIZE))
        {
        }
        else if (session->cliWillTermType && (opt == TEL_OPT_TERM_TYPE))
        {
            sendNVTSubOpt(session, opt);
        }
        else
        {
            sendNVTOpt(session, NVT_CTL_DO,  opt);
            session->cliWillTermType = 1;
        }
        break;
    case TEL_OPT_ECHO:
        break;
    case TEL_OPT_GA:
        sendNVTOpt(session, NVT_CTL_DO,  TEL_OPT_GA);
        break;
    default:
        sendNVTOpt(session, NVT_CTL_DONT,  opt);
        break;
    }
}
void negoNVTCTLWont(TelnetSession* session, unsigned char opt)
{
}
void negoNVTCTLDo(TelnetSession* session, unsigned char opt)
{
    switch (opt)
    {
    case TEL_OPT_TERM_TYPE:
        sendNVTOpt(session, NVT_CTL_WILL,  opt);
        break;
    case TEL_OPT_ECHO:
        if (session->srvWillEcho)
        {
            session->echo = 1;
        }
        else
        {
            sendNVTOpt(session, NVT_CTL_WILL,  TEL_OPT_ECHO);
            session->echo = 1;
            session->srvWillEcho = 1;
        }
        break;
    case TEL_OPT_GA:
        sendNVTOpt(session, NVT_CTL_WILL,  TEL_OPT_GA);
        break;
    default:
        sendNVTOpt(session, NVT_CTL_WONT,  opt);
        break;
    }
}
void negoNVTCTLDont(TelnetSession* session, unsigned char opt)
{
}
unsigned short negoNVTCTLSB(TelnetSession* session, unsigned char *data, unsigned short len)
{
    unsigned short offset = 0;
    int i = 0;
    unsigned char goodSBOpt = 0;
    unsigned char termType[8] = {0};
    switch (data[0])
    {
    case TEL_OPT_TERM_TYPE:
        if (data[1] != 0)
            goto NVTCTLERRORSB;
        offset = 2;
        while ((offset < len) && (i < 8))
        {
            termType[i] = data[offset];
            offset++;
            i++;
            if ((data[offset] == NVT_CTL_IAC) && (data[offset + 1] == NVT_CTL_SE))
            {
                goodSBOpt = 1;
                offset++;
                break;
            }
        }
        if (!goodSBOpt)
        {
            offset = 0;
            goto NVTCTLERRORSB;
        }
        else
        {
            while (i >= 0)
            {
                termType[i] = toupper(termType[i]);
                i--;
            }
            if (strcmp((const char*)termType, "VT100") == 0)
            {
                session->vt100 = 1;
            }
            else if (strcmp((const char*)termType, "ANSI") == 0)
            {
                session->ANSI = 1;
            }
        }
        break;
    case TEL_OPT_WIN_SIZE:
        if ((len < 7) || (data[5] != NVT_CTL_IAC) || (data[6] != NVT_CTL_SE))
        {
            return len;
        }
        else
        {
            session->wsWidth = (data[1] << 8) | data[2];
            session->wsHeigh = (data[3] << 8) | data[4];
            offset = 7;
        }
        break;
    default:
NVTCTLERRORSB:
        while ((offset < len) && (data[offset] != NVT_CTL_SE))
        {
            offset++;
        }
    }
    return offset;
}
unsigned int parseNVTOpt(TelnetSession* session, unsigned char optCode, unsigned char* data, unsigned int offset)
{
    switch (optCode)
    {
    case NVT_CTL_WILL:
        negoNVTCTLWill(session, data[offset]);
        break;
    case NVT_CTL_WONT:
        negoNVTCTLWont(session, data[offset]);
        break;
    case NVT_CTL_DO:
        negoNVTCTLDo(session, data[offset]);
        break;
    case NVT_CTL_DONT:
        negoNVTCTLDont(session, data[offset]);
        break;
    }
    return 1;
}


void negoTelnetOpt(TelnetSession* session, unsigned char* data, unsigned short len)
{
    unsigned short offset = 0;
    while (offset < len)
    {
        if (data[offset] == NVT_CTL_IAC)
        {
            switch (data[offset + 1])
            {
            case NVT_CTL_WILL:
                negoNVTCTLWill(session, data[offset + 2]);
                break;
            case NVT_CTL_WONT:
                negoNVTCTLWont(session, data[offset + 2]);
                break;
            case NVT_CTL_DO:
                negoNVTCTLDo(session, data[offset + 2]);
                break;
            case NVT_CTL_DONT:
                negoNVTCTLDont(session, data[offset + 2]);
                break;
            case NVT_CTL_SB:
                offset = offset + negoNVTCTLSB(session, data + offset + 2, len - offset) - 1;
                break;
            default:
                offset--; //Only one bye to represent opt
                break;
            }
            offset = offset + 2;
        }
        offset++;
    }
}


int telnetConsole(RTSkt* cs, TelnetSession* session, char onlyAuth)
{
    PKT* rxPkt;
    int status = 0;
    unsigned int offset = 0;
    unsigned int r = 0;
    unsigned int expireCount = 0;

    session->cs = cs;

    if (session->state == TEL_AUTH_PASS)
    {
        goto TELRX;
    }

    if (!session->cliWillTermType)
    {
        sendNVTOpt(session, NVT_CTL_DO,  TEL_OPT_TERM_TYPE);
        session->cliWillTermType = 1;
    }

    if (!session->cliWillWinSize)
    {
        sendNVTOpt(session, NVT_CTL_DO,  TEL_OPT_WIN_SIZE);
        session->cliWillWinSize = 1;
    }

    flushTelnetSendBuffer(session);

    do
    {
        rxPkt = rtSktRx_F(cs, 10, &status);

        if (rxPkt)
        {
            negoTelnetOpt(session, rxPkt->wp, rxPkt->len);
            flushTelnetSendBuffer(session);
            freePkt_F(rxPkt);
        }
    } while (rxPkt && (status != SKTDISCONNECT) && (!NewTelCli));

    if ((status == SKTDISCONNECT) || NewTelCli)
    {
        goto TELNET_CLIENT_EXIT;
    }

    session->state = TEL_LOGIN;

    while (1)
    {
        if (session->state == TEL_LOGIN)
        {
            if (!session->srvWillEcho)
            {
                sendNVTOpt(session, NVT_CTL_WILL,  TEL_OPT_ECHO);
                session->srvWillEcho = 1;
            }

            sendTelnetData(session, TelLogin, sizeof(TelLogin));
            flushTelnetSendBuffer(session);
            session->state = TEL_GET_USERNAME;
            expireCount = 0;
        }
        else if (session->state == TEL_PWD)
        {
            sendTelnetData(session, TelPwd, sizeof(TelPwd));
            flushTelnetSendBuffer(session);
            session->state = TEL_GET_PASSWORD;
            expireCount = 0;
        }
        else if ((session->state == TEL_AUTH_PASS) && onlyAuth)
        {
            return TEL_AUTH_PASS;
        }

TELRX:

        do
        {
            rxPkt = rtSktRx_F(cs, 50, &status);

            if ((session->state == TEL_GET_USERNAME) || (session->state == TEL_GET_PASSWORD))
            {
                expireCount++;
                //20s
                if (expireCount > 40)
                    status = SKTDISCONNECT;
            }

        } while ((rxPkt == 0) && (status != SKTDISCONNECT) && (!NewTelCli));

        if ((status == SKTDISCONNECT) || NewTelCli)
        {
            goto TELNET_CLIENT_EXIT;
        }


        offset = 0;

        while (offset < rxPkt->len)
        {
            r = 0;

            switch (session->nvtCTL)
            {
            case NVT_CTL_OPEN:
                if (rxPkt->wp[offset] == NVT_CTL_IAC)
                {
                    session->nvtCTL = NVT_CTL_IAC;
                }
                else
                {
                    r = getShellCmd(session, rxPkt->wp, offset, rxPkt->len - offset);
                }
                break;
            case NVT_CTL_IAC:
                if (rxPkt->wp[offset] == NVT_CTL_IAC)
                {
                    r = getShellCmd(session, rxPkt->wp, offset, rxPkt->len - offset);
                }
                else
                {
                    switch (rxPkt->wp[offset])
                    {
                    case NVT_CTL_WILL:
                        session->nvtCTL = NVT_CTL_WILL;
                        break;
                    case NVT_CTL_WONT:
                        session->nvtCTL = NVT_CTL_WONT;
                        break;
                    case NVT_CTL_DO:
                        session->nvtCTL = NVT_CTL_DO;
                        break;
                    case NVT_CTL_DONT:
                        session->nvtCTL = NVT_CTL_DONT;
                        break;
                    case NVT_CTL_SB:
                        session->nvtCTL = NVT_CTL_SB;
                        break;
                    default:
                        session->nvtCTL = NVT_CTL_OPEN;
                        break;
                    }
                }
                break;
            case NVT_CTL_WILL:
            case NVT_CTL_WONT:
            case NVT_CTL_DO:
            case NVT_CTL_DONT:
                r = parseNVTOpt(session, session->nvtCTL, rxPkt->wp, offset);
                session->nvtCTL = NVT_CTL_OPEN;
                break;
            case NVT_CTL_SB:
                r = negoNVTCTLSB(session, rxPkt->wp + offset, rxPkt->len - offset) - 1;
                session->nvtCTL = NVT_CTL_OPEN;
                break;
            }

            if (r)
            {
                offset += r;
            }
            else
            {
                offset++;
            }
            if ((session->sendBufOffset > (MAX_TLENET_BUF_SIZE/2)) || (session->optBufOffset > (MAX_TLENET_BUF_SIZE/2)))
            {
                flushTelnetSendBuffer(session);
            }
        }
        flushTelnetSendBuffer(session);
        freePkt_F(rxPkt);
    }

TELNET_CLIENT_EXIT:
    if (rxPkt)
    {
        freePkt_F(rxPkt);
    }
    return SKTDISCONNECT;
}
#endif
#ifdef CONFIG_TELNET_ENABLED
void telnetClient(void *data)
{
    TelnetCliArg *telnetCliArg = (TelnetCliArg *)data;

    telnetConsole(telnetCliArg->cs, telnetCliArg->session,0);
    stopconn(telnetCliArg->cs);
    free(telnetCliArg->session);
    OSTaskDel(OS_PRIO_SELF);
}
void telnetSrv(void *data)
{
    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_TCP);
    RTSkt* cs;
    TelnetSession* session;
    int rCode;
    TelnetCliArg telnetCliArg;

    OS_TCB pdata;

    rtSktBind_F(s, 23);
    rtSktListen_F(s, 2);
    while (1)
    {
        cs = rtSktAccept_F(s);

        session = (TelnetSession*)malloc(sizeof(TelnetSession));
        memset(session, 0, sizeof(TelnetSession));
        session->shell = 1;

        rCode = telnetConsole(cs, session, 1);

        if (rCode != TEL_AUTH_PASS)
        {
            stopconn(cs);
            free(session);
            continue;
        }

        NewTelCli = 1;
        while (OSTaskQuery(TASK_TELNET_CLIENT_PRIO, &pdata) == OS_ERR_NONE)
        {
            printf("Task exist");
            OSTimeDly(10);
        }
        NewTelCli = 0;

        telnetCliArg.cs = cs;
        telnetCliArg.session = session;

        OSTaskCreate(telnetClient, &telnetCliArg, (OS_STK *)&TelnetClientStk[TASK_TELNET_CLIENT_STK_SIZE - 1], TASK_TELNET_CLIENT_PRIO);
    }
}
#endif

#ifdef CONFIG_TCR_ENABLED
void SOLTask(void *data)
{
    tcrcb.tcrSrv  = rtSkt_F(IPv4, IP_PROTO_TCP);
    PKT  *txPkt   = NULL;
    PKT *rxPkt;
    QHdr *tcrq;
    INT32S status = 0;
    //INT8U seq = 0;
    SSH *ssh = NULL;
    int len = 0;
    ///unsigned char *p = NULL;
   // unsigned char padlen = 0;
    int padlen=0; 
    TelnetSession* session;
    int rCode;
    ///INT8U gotdata = 0;
    tcrq = (QHdr *)data;
    INT8U i= 0;
    OS_CPU_SR cpu_sr = 0;
    INT8U err;

    rtSktBind_F(tcrcb.tcrSrv, tcrq->port);
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F(tcrcb.tcrSrv);
#endif
    tcrcb.type  = tcrq->option;
    tcrcb.contype = tcrq->contype;
    free(tcrq);

    rtSktListen_F(tcrcb.tcrSrv, 1);
    tcrcb.state = LISTENED;

    if (tcrcb.contype == SSH_TCR)
    {
        tcrcb.tcrcs = rtSktAccept_F(tcrcb.tcrSrv);
        tcrcb.state = ACCEPTED;
    }
    else
    {
        while (1)
        {
            if (tcrcb.state == DISCONNECTED)
            {
                TCRDisable();  
                stopTCR();
            }

            tcrcb.tcrcs = rtSktAccept_F(tcrcb.tcrSrv);
            tcrcb.state = ACCEPTED;

            session = (TelnetSession*)malloc(sizeof(TelnetSession));
            memset(session, 0, sizeof(TelnetSession));

            rCode = telnetConsole( tcrcb.tcrcs, session, 1);

            if (rCode != TEL_AUTH_PASS)
            {
                stopconn(tcrcb.tcrcs);
                tcrcb.tcrcs = 0;
                free(session);

                if (rCode == SKTDISCONNECT)
                {
                    stopTCR();
                }
                continue;
            }
            else
            {
                break;
            }
        }

        free(session);
        tcrcb.state = CONNECTED;
        TCREnable();    
        //bsp_tcr_imr_enable();
    }

    //for Smasung only
    if (dpconf->vendor == SAMSUNG)
        tcr_received("\x13\x12", 2);

    //get tcrdata if any
    while (OS_TRUE)
    {
        //handling Rx from console, including the handshaking

        //if ((rxPkt = rtSktRx_F(tcrcb.tcrcs, 5, &status)) != 0)
        status = tcrcb.contype;
        if ((rxPkt = (PKT*)sshrx(&ssh, tcrcb.tcrcs, 1, &status)) != 0)
        {
            //tcrcb.pkt = rxPkt;
            //if (rxPkt->wp[0] == 0xff)
            if (tcrcb.contype == SSH_TCR)
            {
                if (status == 2)
                {
                    //txPkt = (PKT*)allocPkt_F(rxPkt->len);
                    //memcpy(txPkt->wp, rxPkt->wp, txPkt->len);
                    //txPkt->len = tcr_hslen[seq];
                    //memcpy(txPkt->wp, tcr_hsstr[seq], txPkt->len);
                    //seq++;
                    //status = rtSktSend_F(tcrcb.tcrcs, txPkt, 0);


                    if (tcrcb.state == ACCEPTED)
                    {
                        tcrcb.state = CONNECTED;		    
			TCREnable();
                        ///bsp_tcr_imr_enable();
                    }
                    continue;
                }
            }
#if 0
            else
            {
                txPkt = (PKT*)allocPkt_F(rxPkt->len);
                memcpy(txPkt->wp, rxPkt->wp, txPkt->len);
                status = rtSktSend_F(tcrcb.tcrcs, txPkt, 0);

                if (tcrcb.state == ACCEPTED && status != -1)
                {

                }
            }
#endif
#ifdef PATCH_HP_CONSOLE
            if (strstr(rxPkt->wp, lf) && tcrcb.state == ACCEPTED)
            {
                tcrcb.state = CONNECTED;
                seq++;
                ///bsp_tcr_imr_enable();
            }
            else
#endif
                //no need to get data first and then send the keys
                if (tcrcb.state == CONNECTED)/// && gotdata)
                {
                    tcr_received(rxPkt->wp, rxPkt->len);
                }

            freePkt_F(rxPkt);
            //tcrcb.pkt = NULL;
        }


        if (status<0||/*status == SKTDISCONNECT || */tcrcb.state == DISCONNECTED)
        {
        	 
	 TCRDisable();		 
            stopTCR();
        }
        else
        {

            txPkt = OSQAccept(TCRQ, &err);
            if(txPkt)
            {
                if (tcrcb.contype == SSH_TCR)
                {

                    len = 4+PACKET_PAYLOAD_OFF + 1/*msg code*/ + txPkt->len;
                    len += 4 /*recvChanId*/;
                    padlen = 16-((len)%16);
                    if (padlen < 4)
                        padlen += 16;
                    len+=padlen;
                    len += ssh->macsize;

                    padlen = txPkt->len;
                    txPkt->len = len;
                    setInt(txPkt->wp, padlen);
                    sshEncodeData(txPkt, 4+padlen, ssh);
                }
                else
                {
                    rtSktSend_F(tcrcb.tcrcs, txPkt, 0);
                }
            }
        }
    }
}


#endif
