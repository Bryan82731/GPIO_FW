/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	TCP.c

Abstract:	TCP module

*****************************************************************************
*/

/*--------------------------Include Files-----------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "ip.h"
#include "tcp.h"
#include "lib.h"
#include "bsp.h"
#include "arp.h"
#include "wcom.h"
#include "mDNS.h"

#if CONFIG_VERSION >= IC_VERSION_EP_RevA
#include "hwpf.h"
extern TCAM_Allocation TCAMMem[NumOfTCAMSet];
extern TCAMRule TCAMRuleMem[NumOfPFRule];
#endif

extern FWSIG *fwsig;

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
ROM_EXTERN PortRecord IBPortMatchTable[MaxPortMatchRecords];
ROM_EXTERN PortRecord OOBPortMatchTable[MaxPortMatchRecords];
#endif

#ifdef CONFIG_PortMatch_Test
PMTPortRecord PMTPRs[MaxPortMatchRecords*2];//InBand and OOB
#endif

#ifdef CONFIG_mDNS_OFFLOAD_ENABLED
extern struct _mDNSOffloadCmd mDNSOffloadCmd;
#endif

#if CONFIG_TEREDO_ENABLED
extern void sendTeredoBubblePkt(RTSkt* TeredoSkt, unsigned char* ip, unsigned short originPort, unsigned int originIP);
#endif

/*--------------------------Global variables----------------------------------*/
ROM_EXTERN TCPPCB *ActivePCBsList;

extern OS_EVENT *SktSem;
extern OS_EVENT *ArpTableSem;
extern DPCONF *dpconf;
extern _IPInfo IPInfo[];
extern INT8U gmac_reset;
extern RTSkt *dashcs;
extern RTSkt *dashtlscs;
const unsigned short Default_RSV_Port[]= {59 /*USB*/, 87/*TCR*/, 57/*TCR-SSH*/, 0x8168/*Firmware update*/};

#if CONFIG_ROM_VERIFY || !defined(CONFIG_BUILDROM)
void tcpProcess(PKT *pkt, TCPPCB *pcbPtr)
{
    TCPPCB	*newPCB;
    PEthHdr	eHdr;
    IPHdr	*ipHdr = NULL;
    IPv6Hdr *ipv6Hdr = NULL;
    TCPHdr	*tcpHdr;
    TCPHdrOpt *opt;
    unsigned int tmpSeqNo = 0;
    unsigned char hLen = 0;

    if (pkt->ipVer == IPv4)
    {
        ipHdr = (IPHdr *)(pkt->wp - IP_HLEN);
        ipHdr->len = ntohs(ipHdr->len);
    }
    else
    {
        ipv6Hdr = (IPv6Hdr *)(pkt->wp - IPv6_HLEN);
        ipv6Hdr->len = ntohs(ipv6Hdr->len);
    }

    tcpHdr = (TCPHdr *)pkt->wp;

    if (tcpHdr->flags & TCP_RST)
    {
        DEBUGMSG(TCP_RX_DEBUG,"Flag is Reset");
        //Sometimes 3-way handsahking not build, then RST
        //Do not change server port() state
        if ( pcbPtr != ActivePCBsList && pcbPtr->destPort != 0 && pcbPtr->state != TIME_WAIT)
            pcbPtr->state = CLOSED;
        else
        {
            return;
        }
    }

    switch (pcbPtr->state)
    {
    case CLOSED:
    {
        DEBUGMSG(TCP_RX_DEBUG,"CLOSED");
        removePCB_F(pcbPtr);
        return;
    }
    break;
    case LISTEN:
    {
        DEBUGMSG(TCP_DEBUG, "LISTEN");
        //Unknow connection
        if (!(tcpHdr->flags & TCP_SYN))
        {
            DEBUGMSG(TCP_DEBUG, "unknow connection\n");
            return;
        }

        newPCB = tcpPCBNew_F();
        //Will copy tx/rx sem
        //First we use server tx/rx sem and skt
        memcpy(newPCB, pcbPtr, sizeof(TCPPCB));
        newPCB->state = SYN_RCVD;
        newPCB->protocol = IP_PROTO_TCP;

        if (pkt->ipVer == IPv4)
        {
            DEBUGMSG(TCP_RX_DEBUG,"IPv4 Client");
            eHdr = (PEthHdr)(pkt->wp - IP_HLEN - ETH_HDR_SIZE);
        }
        else
        {
            DEBUGMSG(TCP_RX_DEBUG,"IPv6 Client");
            eHdr = (PEthHdr)(pkt->wp - IPv6_HLEN - ETH_HDR_SIZE);
        }

        memcpy(newPCB->dest_MAR, eHdr->srcAddr, MAC_ADDR_LEN);

        if (pkt->ipVer == IPv4)
        {
            newPCB->destIPAddr.addr = ipHdr->src.addr;
            newPCB->ipVer = IPv4;
        }
        else
        {
            memcpy(newPCB->destIPv6Addr, ipv6Hdr->src, IPv6_ADR_LEN);
            newPCB->ipVer = IPv6;
        }

        newPCB->destPort = tcpHdr->src;
        //If SSL connection ,SSL will be created in getPCBMatchSSL.

        newPCB->TLSDataStage = 0;

        newPCB->next = NULL;
        newPCB->clientList = NULL;
        newPCB->txQStart = NULL;
        newPCB->txQEnd = NULL;
        newPCB->rxQStart = NULL;
        newPCB->rxQEnd = NULL;

        //Http
        newPCB->httpParseStatus = HTTPINIT;
        newPCB->httpContentLen = 0;
        newPCB->httpReadLen = 0;
        newPCB->httpReqData = 0;

        //In Three-way handshaking
        //ack num = sequence num(ISN) + 1
        newPCB->sendAckNum = tcpHdr->seqno + 1;
        newPCB->cwnd = tcpHdr->wnd;
        newPCB->sendPkts = 0;
        newPCB->probe = 0;
	newPCB->intf = pkt->intf;

        opt = (TCPHdrOpt *) (pkt->wp + TCP_HDR_SIZE);
        if (opt->optCode == 2)
        {
            newPCB->cmss = ntohs(opt->optMss);
        }

        tcpSend_F(newPCB, NULL, TCP_ACK|TCP_SYN);

        pcbPtr->curConnections++;
        tcpConnectionIn_F(pcbPtr, newPCB);
        //Noifty(Listen socket) a new connection has been accepted
        //Client state is SYN_RCVD

        if (!pcbPtr->apRx)
        {
            DEBUGMSG(TCP_DEBUG | SKT_DEBUG, "A Socket connection establish request incoming. post rxSem is %x", pcbPtr->skt->rxSem);
            OSSemPost(pcbPtr->skt->rxSem);
        }
    }
    break;
    case SYN_SENT:
    {
        DEBUGMSG(TCP_DEBUG, "SYN_SENT");
        if (tcpHdr->ackno != pcbPtr->seqNum +1)
        {
            DEBUGMSG(TCP_DEBUG, "Seq no error in SYN_SENT");
            break;
        }

        pcbPtr->seqNum = tcpHdr->ackno;
        pcbPtr->sendAckNum = tcpHdr->seqno + 1;
        tcpSend_F(pcbPtr, NULL, TCP_ACK);
        pcbPtr->state = ESTABLISHED;
        if (pcbPtr->skt && !pcbPtr->apRx)
        {
            DEBUGMSG(TCP_DEBUG | SKT_DEBUG, "Socket Connection establish successfully");
            OSSemPost(pcbPtr->skt->rxSem);
        }
    }
    break;
    case SYN_RCVD:
    {
        DEBUGMSG(TCP_DEBUG, "SYN_RCVD");
        if (tcpHdr->flags & TCP_SYN)
        {
            DEBUGMSG(TCP_DEBUG, "Retransmit SYN-ACK");
            //Retransmit SYN packet
            tcpSend_F(pcbPtr, NULL, TCP_ACK|TCP_SYN);
            return;
        }
        else if ((tcpHdr->flags & TCP_ACK) && ! (tcpHdr->flags & TCP_RST))
        {
            //Three-way handing shaking finish and connection established.
            if (tcpHdr->ackno == pcbPtr->seqNum)
            {
                DEBUGMSG(TCP_DEBUG | SKT_DEBUG, "Set state to ESTABLISHED");

                tcpDequeue_F(pcbPtr, tcpHdr->ackno);
                pcbPtr->state = ESTABLISHED;
                if (pcbPtr->skt && !pcbPtr->apRx)
                {
                    OSSemPost(pcbPtr->skt->rxSem);
                }
            }
        }
        else
        {
            DEBUGMSG(TCP_DEBUG, "Warring unexpected flag in SYN_RCVD");
        }

    }
    break;
    case ESTABLISHED:
    {
        DEBUGMSG(TCP_DEBUG, "ESTABLISHED");

        //When Client send many packets(above 3) at very short time, and server only
        //process the first packet and send correspond packet to client.
        //At this time pcbPtr->seqNum is increase, so other packet will be judge to
        // retransmission(They tcpHdr->ackno is the same)
        if ((tcpHdr->ackno < pcbPtr->seqNum) && (tcpHdr->seqno < pcbPtr->sendAckNum))
        {
            DEBUGMSG(TCP_DEBUG,"TCP Retransmission");
            tcpSend_F(pcbPtr, NULL, TCP_ACK);
            return;
        }

        //out of order data
        //May be last seqment be send first
        if (tcpHdr->seqno != pcbPtr->sendAckNum)
        {
            DEBUGMSG(TCP_DEBUG,"TCP Out of order");
            tcpSend_F(pcbPtr, NULL, TCP_ACK);
            return;
        }

        hLen = tcpHdr->offset * 4;

        //Just ACK
        if ( (pkt->ipVer ==IPv4) && (tcpHdr->flags == TCP_ACK) &&
                (ipHdr->len - IP_HLEN - hLen == 0))
        {
            DEBUGMSG(TCP_DEBUG,"TCP ACK(IPv4)");
            tcpDequeue_F(pcbPtr, tcpHdr->ackno);
            //sendTCPData_F(pcbPtr);
            pcbPtr->alive = 1;
        }
        else if ( (pkt->ipVer ==IPv6) && (tcpHdr->flags == TCP_ACK) &&
                  (ipv6Hdr->len - hLen == 0))
        {
            DEBUGMSG(TCP_DEBUG,"TCP ACK(IPv6)");
            tcpDequeue_F(pcbPtr, tcpHdr->ackno);
            //sendTCPData_F(pcbPtr);
            pcbPtr->alive = 1;
        }
        else if (tcpHdr->flags != (TCP_FIN | TCP_ACK))
        {
            tcpDequeue_F(pcbPtr, tcpHdr->ackno);

            if (pkt->ipVer == IPv4)
            {
                pcbPtr->sendAckNum = tcpHdr->seqno + ipHdr->len - hLen - IP_HLEN;
                pkt->len = ipHdr->len - IP_HLEN - hLen;
            }
            else
            {
                pcbPtr->sendAckNum = tcpHdr->seqno + ipv6Hdr->len - hLen;
                pkt->len = ipv6Hdr->len - hLen;
            }

            tmpSeqNo = pcbPtr->seqNum;
            pkt->wp = pkt->wp + hLen;

            if (pcbPtr->apRx)
            {
                DEBUGMSG(TCP_DEBUG,"Packet based");
                pcbPtr->apRx(pkt->wp, pkt->len, pcbPtr);

                if (pcbPtr->apRx == 0)
                {
                    postListenPort_F(pcbPtr);
                    if (pcbPtr->TLSEnable)
                        tcpRxInput_F(pkt, pcbPtr, 1);
                    else
                        tcpRxInput_F(pkt, pcbPtr, 0);
                }
            }
            else
            {
                tcpRxInput_F(pkt, pcbPtr, 0);
            }
            //If AP receive a packet but do not send any data to client, we send a ack
            if (tmpSeqNo == pcbPtr->seqNum)
                tcpSend_F(pcbPtr, NULL, TCP_ACK);
        }
        else if (tcpHdr->flags & TCP_FIN)
        {
            if (pkt->ipVer == IPv4)
                pcbPtr->sendAckNum = tcpHdr->seqno + ipHdr->len - hLen - IP_HLEN + 1;
            else
                pcbPtr->sendAckNum = tcpHdr->seqno - hLen + 1;


            tcpSend_F(pcbPtr, NULL, TCP_FIN | TCP_ACK);
            pcbPtr->state = LAST_ACK;
        }
    }
    break;
    case FIN_WAIT_1:
    {
        //AP request to close
        DEBUGMSG(TCP_DEBUG,"FIN_WAIT_1");
        if (tcpHdr->flags & TCP_ACK)
        {
            tcpDequeue_F(pcbPtr, tcpHdr->ackno);
        }

        if (tcpHdr->flags & TCP_FIN)
        {
            pcbPtr->sendAckNum++;
            //Send right now
            pcbPtr->delayAck = FirstDelayAck;
            tcpSend_F(pcbPtr, NULL, TCP_ACK);
            pcbPtr->state = CLOSED;
            removePCB_F(pcbPtr);
            return;
        }
    }
    break;
    case FIN_WAIT_2:
    {
        /*
        //1.Tx FIN-ACK(last packet)
        //2.Rx FIN-ACK
        //3.TX ACK(ack rx a FIN-ACK)
        DEBUGMSG(TCP_DEBUG,"FIN_WAIT_2");

        if ( (tcpHdr->flags & TCP_FIN) )
        {
        	pcbPtr->sendAckNum++;
        	pcbPtr->seqNum++;
        	//Change delay ack state or it maybe can not be send
        	pcbPtr->delayAck = FirstDelayAck;
			tcpSend_F(pcbPtr, NULL, TCP_ACK);
        	pcbPtr->state = CLOSED;
			removePCB_F(pcbPtr);
        	return;
        }

        if ( (tcpHdr->flags & TCP_ACK) )
        {
        	pcbPtr->seqNum++;
			tcpSend_F(pcbPtr, NULL, TCP_RST);
        	pcbPtr->state = CLOSED;
			removePCB_F(pcbPtr);
        	return;
        }*/
    }
    break;
    case CLOSE_WAIT:
    {
        DEBUGMSG(TCP_DEBUG,"CLOSE_WAIT");
        /*
        if ( (tcpHdr->flags & TCP_ACK) )
        {
        temp_label:
        	DEBUGMSG(TCP_DEBUG,"CLOSE_WAIT");
			tcpSend_F(pcbPtr, NULL, TCP_FIN);
        	pcbPtr->state = LAST_ACK;
        	break;
        }*/
    }
    break;
    case CLOSING:
    {
    }
    break;
    case LAST_ACK:
    {
        DEBUGMSG(TCP_DEBUG,"LAST_ACK");
        if (tcpHdr->flags & TCP_ACK)
        {
            pcbPtr->state = CLOSED;
            if (pcbPtr->timeWaitEnable && pcbPtr->rxQStart)
            {
                //removePCB when client rx all packet or time out
                pcbPtr->state = TIME_WAIT;
            }
            else
            {
                removePCB_F(pcbPtr);
            }
            return;
        }
    }
    break;
    case TIME_WAIT:
    {
        tcpSend_F(pcbPtr, NULL, TCP_RST);
    }
    break;
    default:
        DEBUGMSG(CRITICAL_ERROR|TCP_DEBUG,"Incorrect TCP state");
    }

    DEBUGMSG(TCP_RX_DEBUG,"tcpProcess <==");
}

void tcpRxInput(PKT *pkt, TCPPCB *pcb, INT8U TLS1st)
{
    int v = 0;
    PKT *tmpPkt = 0;

    if (pcb->TLSEnable && !TLS1st)
    {
        if (pcb->ssl && pcb->ssl->crLen)
        {
            tmpPkt = pcb->ssl->cPkt;

            if(pcb->ssl->fragpkt== 2)
                tmpPkt=tmpPkt->next;

            while (tmpPkt)
            {
                if (tmpPkt->next == 0)
                    break;

                tmpPkt = tmpPkt->next;
            }

            if (tmpPkt)
            {
                tmpPkt->next = pkt;
                pkt->queued = 1;
            }

            pcb->ssl->crLen = pcb->ssl->crLen - pkt->len;
            if (pcb->ssl->crLen > 0)
                return;

            //Do not free packet here, it will be freed at tcptask
            //freePkt(pkt);
            //pkt = pcb->ssl->cPkt;
            //pcb->ssl->cPkt = 0;
            pcb->TLSDataStage = 1;
            //deCodeTLSData(pkt->wp, pkt->len, pcb, RECORD_APP_PROTOCOL_DATA);
            if(pcb->ssl->fragpkt== 2)
            {
                deCodeTLSData_F(pcb->ssl->cPkt->next->wp, pcb->ssl->cPkt->next->len, pcb->ssl->cPkt->next, pcb, RECORD_APP_PROTOCOL_DATA);
                pcb->ssl->fragpkt=0;
                pcb->ssl->crLen=0;
            }
            else
                deCodeTLSData_F(pcb->ssl->cPkt->wp, pcb->ssl->cPkt->len, pcb->ssl->cPkt, pcb, RECORD_APP_PROTOCOL_DATA);
        }
        else
        {
#if CONFIG_VERSION  == IC_VERSION_DP_RevF
            v = fwsig->doTLSAccept_F(pkt->wp, pkt->len, pcb);
#else
            v = doTLSAccept_F(pkt->wp, pkt->len, pcb);
#endif

            if (v == TLS_FRAGMENT_PKT)
            {
                pcb->ssl->cPkt = pkt;
                pkt->wp += TLS_RECORD_SIZE;
                pkt->len -= TLS_RECORD_SIZE;
                pkt->queued = 1;
                return;
            }

            if (v == -1)
            {
                return;
            }

            if (pcb->ssl->clientStage == RECORD_TOO_SMALL)
            {
                pkt = (PKT*)pcb->ssl->pktsmall;
                pkt->len = pcb->ssl->pktsmallLen;
                pcb->ssl->pktsmallLen = 0;
                pcb->ssl->clientStage = RECORD_COMPLETED;
            }
            else
            {
                pkt->wp = pkt->wp + TLS_RECORD_SIZE;
                pkt->len = pcb->ssl->recordLayer.size;// pkt->len - TLS_RECORD_SIZE;
            }
            if (!pcb->TLSDataStage)
            {
                DEBUGMSG(HTTPS_DEBUG ,"Not at TLS dataStage");
                return;
            }
            DEBUGMSG(HTTPS_DEBUG ,"A TLS pkt income");
        }
    }
    else if (pcb->TLSEnable && TLS1st)
    {
        if(pcb->ssl->fragpkt== 2)
            return;
        if (pcb->ssl && pcb->ssl->pktsmall)
        {
            pkt = (PKT*)pcb->ssl->pktsmall;
            pkt->len = pcb->ssl->pktsmallLen;
        }
        else
        {
            pkt->wp = pkt->wp + TLS_RECORD_SIZE;
            pkt->len = pcb->ssl->recordLayer.size;// pkt->len - TLS_RECORD_SIZE;
        }
    }


    //TLS packet should not been queued
    pkt->queued = 1;

    tmpPkt = pkt;

    if (pcb->TLSEnable && pcb->ssl && pcb->ssl->cPkt)
    {
        tmpPkt = pcb->ssl->cPkt;
    }

    while (tmpPkt)
    {
        pkt = tmpPkt;

        tmpPkt = tmpPkt->next;

        pkt->next = 0;
        //Empty
        if (pcb->rxQStart ==0 && pcb->rxQEnd == 0)
        {
            pcb->rxQStart = pcb->rxQEnd = pkt;
        }
        else
        {
            //Only one packet
            if (!pcb->rxQEnd->prev)
            {
                pcb->rxQStart->next = pkt;
                pkt->prev = pcb->rxQStart;
                pcb->rxQEnd = pkt;
            }
            else
            {
                //2-n
                pcb->rxQEnd->next = pkt;
                pkt->prev = pcb->rxQEnd;
                pcb->rxQEnd = pkt;
            }
        }
        DEBUGMSG(SKT_DEBUG ,"A socket packet income Post %x", pcb->skt->rxSem);
        OSSemPost(pcb->skt->rxSem);
    }
    if(pcb->ssl)
    {
        pcb->ssl->cPkt = 0;
    }
}
#endif

void removePCB(TCPPCB *pcb)
{
    TCPPCB *iPCB;
    TCPPCB *tPCB;
    TCPPCB *prevPCB;
    PKT *tmp = pcb->txQStart;

    DEBUGMSG(TCP_DEBUG,"removePCB ==>");

#ifdef CONFIG_PROVISIONING
#if 1
#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    if (pcb->ssl && fwsig->SIG == NORMAL_SIG && fwsig->ProvTCPHook == NEED_PATCH)
        fwsig->ProvTCPHook_F(pcb);
#else
    if (pcb->ssl && fwsig->SIG == NORMAL_SIG)
        removeProvPCB(pcb);
#endif
#endif
 //   removeProvPCB(pcb);
#endif
    /*
        if (pcb->ssl)
        {
            if (pcb->ssl->peer_cert != NULL)
            {
                x509_free( pcb->ssl->peer_cert  );
                memset( pcb->ssl->peer_cert, 0, sizeof( x509_cert ) );
                free(pcb->ssl->peer_cert);
                pcb->ssl->peer_cert = NULL;
            }
            if (pcb->ssl->own_cert != NULL)
            {
                x509_free( pcb->ssl->own_cert  );
                memset( pcb->ssl->own_cert, 0, sizeof( x509_cert ) );
                free(pcb->ssl->own_cert);
                pcb->ssl->own_cert = NULL;
            }
            if (pcb->ssl->ca_crl != NULL)
            {
                x509_crl_free( pcb->ssl->ca_crl  );
                free(pcb->ssl->ca_crl);
                pcb->ssl->ca_crl = NULL;
            }
        }
    #endif
    */

    if ( pcb->ssl && pcb->ssl->resume)
    {
        DEBUGMSG(HTTPS_DEBUG | TCP_DEBUG,"Free SSL");

        while (pcb->ssl->cPkt)
        {
            tmp = pcb->ssl->cPkt;
            freePkt(tmp);
            pcb->ssl->cPkt = pcb->ssl->cPkt->next;
        }
        tmp = pcb->txQStart;
        free(pcb->ssl);
    }

    if (pcb->httpReqData)
    {
        DEBUGMSG(HTTP_DEBUG | TCP_DEBUG,"Free request data");
        free(pcb->httpReqData);
    }

    //Do not remove ActivePCBsList
    //ActivePCBsList is the server listen port
    if (ActivePCBsList->next == NULL)
    {
        DEBUGMSG(TCP_DEBUG,"removePCB error");
        return;
    }

    //Free tx queue data
    while (tmp != NULL)
    {
        pcb->txQStart = tmp->next;
        freePkt_F(tmp);

        if (pcb->txQStart != NULL )
            tmp = pcb->txQStart;
        else
            break;
    }

    //Free Rx queue data
    tmp = pcb->rxQStart;

    while (tmp != NULL)
    {
        pcb->rxQStart = tmp->next;
        freePkt_F(tmp);

        if (pcb->rxQStart != NULL )
            tmp = pcb->rxQStart;
        else
            break;
    }

    pcb->txQStart = NULL;
    pcb->txQEnd = NULL;
    pcb->rxQStart = NULL;
    pcb->rxQEnd = NULL;

    if (pcb->destPort == 0 && pcb->localPort == 0)
    {
        free(pcb);
        pcb = 0;
        return;
    }

    tPCB = ActivePCBsList->next;

    for (iPCB = ActivePCBsList->next ; iPCB != NULL; iPCB = iPCB->next)
    {
        //Found
        if (iPCB == pcb)
        {
            //At PCB chain first
            if (iPCB == ActivePCBsList->next)
            {
                //Remove first connection and move the second connection PCB to
                //ActivePCBsList->next
                if (iPCB->next)
                    ActivePCBsList->next = iPCB->next;
                else //only one connection
                {
                    DEBUGMSG(TCP_DEBUG,"Remove last connection. No connection exist");
                    ActivePCBsList->next = NULL;
                }
            }
            else
            {
                tPCB->next = iPCB->next;
            }

            if (pcb->skt)
            {
                //Post rxSem to notify process that this connection has been disconnected
                DEBUGMSG(TCP_DEBUG | SKT_DEBUG,"A socket connect. Notify rx semphore remove event");
                pcb->skt->pcb = 0;
                //skt will check if pcb exist, post rx sem after pcb to 0
                OSSemPost(pcb->skt->rxSem);
            }
            DEBUGMSG(TCP_DEBUG | SKT_DEBUG,"Remove a client connection. PCB is %x", pcb);

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
            if (pcb->hwIBPortFilterEnable || pcb->hwOOBPortFilterEnable)
            {
                removePortFilterTable_F(pcb);

            }
#endif
            free(pcb);
            return;
        }

        //Service Client list PCB
        if (iPCB->localPort == pcb->localPort)
        {
            tPCB = iPCB->clientList;
            prevPCB = iPCB;

            while (tPCB)
            {
                if (tPCB->destPort == pcb->destPort)
                {
                    DEBUGMSG(TCP_DEBUG,"Found a service client connection");
                    if (tPCB->clientList)
                    {
                        prevPCB->clientList = tPCB->clientList;
                    }
                    else
                    {
                        prevPCB->clientList = NULL;
                    }

                    //If packet based client, it does not have skt
                    if (tPCB->skt)
                    {
                        DEBUGMSG(TCP_DEBUG | SKT_DEBUG,"Remove a service client socket based connection");

                        if (tPCB->accepted != 1)
                            /*accepted not true -> skt pcb do not have been created*/;
                        else
                        {
                            tPCB->skt->pcb = 0;
                            DEBUGMSG(TCP_DEBUG | SKT_DEBUG,"Remove a service client connection.set pcb to 0");
                        }
                        //Post rxSem to notify process that this connection has been disconnected
                        //skt will check if pcb exist, post rx sem after pcb to 0
                        DEBUGMSG(TCP_DEBUG | SKT_DEBUG,"Remove a service client connection.post rx sem is %x", tPCB->skt->rxSem);
                        if (!tPCB->apRx)
                            OSSemPost(tPCB->skt->rxSem);
                    }

                    DEBUGMSG(TCP_DEBUG | SKT_DEBUG,"Remove a service client connection. PCB is %x", tPCB);
                    free(tPCB);
                    iPCB->curConnections--;
                    return;
                }
                prevPCB = tPCB;
                tPCB = tPCB->clientList;
            }//end while
        }

        tPCB = iPCB;
    }//end for

    DEBUGMSG(TCP_DEBUG,"No connection has been removed");
}

int matchIntf(TCPPCB *pcb, PKT *pkt)
{
	if(pcb->specIntf)
	{
		return (pcb->intf == pkt->intf);
	}
	else
	{
		return 1;
	}
}
#if defined(CONFIG_TEREDO_ENABLED) || (CONFIG_VERSION != IC_VERSION_DP_RevF) || CONFIG_ROM_VERIFY
void addTcpHeader(TCPPCB *pcb, PKT* pkt, unsigned char flag)
{
    TCPHdr* tcphdr;
    //IPAddress localIP;
    IPv6PseudoHdr* ipv6PseudoHdr;

    pcb->delayAck = NoDelayAck;
    pkt->wp = pkt->wp - TCP_HDR_SIZE;
    pkt->intf = pcb->intf;

    if (pkt->ipVer == IPv4)
        tcphdr = (TCPHdr*)(pkt->wp);
    else
        tcphdr = (TCPHdr*)(pkt->wp);

    tcphdr->src = htons(pcb->localPort);
    tcphdr->dest = htons(pcb->destPort);
    tcphdr->unused = 0;

    if (flag == (TCP_PSH | TCP_ACK))
    {
        tcphdr->seqno = htonl(pcb->seqNum);
        //Next packet seqNum
        pcb->seqNum += pkt->len;
        tcphdr->ackno = htonl(pcb->sendAckNum);
        //General TCP Header 5* 4 = 20
        tcphdr->offset = 5;
        tcphdr->flags = flag;
        tcphdr->wnd = htons(TCPMAXWINSIZE);
		if(pcb->skt->rxSem->OSEventCnt > 0)
		{
			if(pcb->skt->rxSem->OSEventCnt > 8)
				tcphdr->wnd = 0;
			else
			{
				tcphdr->wnd = (8 - pcb->skt->rxSem->OSEventCnt)*1024;
				tcphdr->wnd = htons(tcphdr->wnd);
			}
		}
		
    }
    else if (flag == TCP_SYN)
    {
        pcb->seqNum = 0;
        tcphdr->seqno = htonl(0);
        pcb->seqNum = 0;
        tcphdr->ackno = 0;
        //Option Length: 6* 4 = 24
        tcphdr->offset = 6;
        tcphdr->flags = TCP_SYN;
        tcphdr->wnd = htons(TCPMAXWINSIZE);
        pcb->cwnd = TCPMAXWINSIZE;
        pcb->state = SYN_SENT;
    }
    else if (flag == (TCP_ACK|TCP_SYN) )
    {
        pcb->seqNum = 0;
        tcphdr->seqno = htonl(0);
        pcb->seqNum = 0;
        //After SYN-ACK,seqNum++
        pcb->seqNum++;
        tcphdr->ackno = htonl(pcb->sendAckNum);
        //Option Length: 6* 4 = 24
        tcphdr->offset = 6;
        tcphdr->flags = TCP_ACK|TCP_SYN;
        tcphdr->wnd = htons(TCPMAXWINSIZE);
    }
    else if (flag == TCP_ACK)
    {
        if (pcb->probe)
        {
            //sned incorrect sequence number
            tcphdr->seqno = htonl(pcb->seqNum + 1000);
        }
        else
        {
            tcphdr->seqno = htonl(pcb->seqNum);
            //Next packet seqNum
            pcb->seqNum += pkt->len;
        }
        tcphdr->ackno = htonl(pcb->sendAckNum);
        //General TCP Header 5* 4 = 20
        tcphdr->offset = 5;
        tcphdr->flags = flag;
        tcphdr->wnd = htons(TCPMAXWINSIZE);
		
		if(pcb->skt->rxSem->OSEventCnt > 0)
		{
			if(pcb->skt->rxSem->OSEventCnt > 8)
				tcphdr->wnd = 0;
			else
			{
				tcphdr->wnd = (8 - pcb->skt->rxSem->OSEventCnt)*1024;
				tcphdr->wnd = htons(tcphdr->wnd);
			}
		}
    }
    else if (flag & TCP_FIN)
    {
        tcphdr->seqno = htonl(pcb->seqNum);
        tcphdr->ackno = htonl(pcb->sendAckNum);

        pcb->seqNum++;
        pcb->seqNum += pkt->len;// For FIN PSUH ACK

        //General TCP Header 5* 4 = 20
        tcphdr->offset = 5;
        tcphdr->flags = flag;

        tcphdr->wnd = htons(TCPMAXWINSIZE);
    }
    else if ((flag == TCP_RST) || (flag == (TCP_ACK | TCP_RST)) )
    {
        tcphdr->seqno = htonl(pcb->seqNum);
        /**just foer test ****/
        pcb->seqNum += pkt->len;
        tcphdr->ackno = htonl(pcb->sendAckNum);
        //General TCP Header 5* 4 = 20
        tcphdr->offset = 5;
        tcphdr->flags = flag;

        tcphdr->wnd = htons(TCPMAXWINSIZE);
    }

    if (pkt->ipVer == IPv4)
    {
        tcphdr->urgp = 0;
        tcphdr->chksum = 0;
#ifndef CKHSUM_OFFLOAD
        localIP.addr = getIPAddress_F(intf);
        tcphdr->chksum = inet_chksum_pseudo((void *)tcphdr,&localIP.addr, &pcb->destIPAddr, IP_PROTO_TCP, (unsigned short)(TCP_HDR_SIZE + pkt->len));
#endif

    }
    else
    {
        ipv6PseudoHdr = (IPv6PseudoHdr*)(pkt->wp - sizeof(IPv6PseudoHdr));
        memcpy(ipv6PseudoHdr->src, getIPv6Address_F(pcb->intf), IPv6_ADR_LEN);
        memcpy(ipv6PseudoHdr->dest, pcb->destIPv6Addr, IPv6_ADR_LEN);
        ipv6PseudoHdr->nextHdr = IP_PROTO_TCP;
        ipv6PseudoHdr->unUsed = 0;
        ipv6PseudoHdr->payLoadLen = htons(pkt->len + sizeof(TCPHdr));
        tcphdr->chksum = 0;

        if (dpconf->useTeredo)
        {
            tcphdr->chksum = inet_chksum(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr) + pkt->len + sizeof(TCPHdr));
        }
        else
        {
            //Rtk HW limlit
            //Must give IPv6 pseudo header checksum
            tcphdr->chksum = v6pseudochksum_F(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr));
        }

//#ifndef CKHSUM_OFFLOAD
        //tcphdr->chksum = inet_chksum(pkt->wp - sizeof(IPv6PseudoHdr),  sizeof(IPv6PseudoHdr) + pkt->len + sizeof(TCPHdr));
//#endif
    }

    DEBUGMSG(TCP_DEBUG,"addTcpHeader <==");
    addIPHeader_F(pcb, pkt);
}

void tcpDequeue(TCPPCB *pcb, unsigned int ackNum)
{
    PKT *pkt = pcb->txQStart;
    TCPHdr	*tcphdr;
    int hdrOffset = 0;

    pcb->oppAckNo = ackNum;

    if ( pkt == NULL)
        return;

    if (pkt->ipVer == IPv4)
    {
        hdrOffset = ETH_HDR_SIZE + IP_HLEN;
        tcphdr = (TCPHdr*)(pkt->wp + hdrOffset);
    }
    else
    {
#if CONFIG_TEREDO_ENABLED
        if (dpconf->teredoEnable)
        {
            hdrOffset = IPv6_HLEN;
        }
        else
        {
            hdrOffset = ETH_HDR_SIZE + IPv6_HLEN;
        }
#else
        hdrOffset = ETH_HDR_SIZE + IPv6_HLEN;
#endif
        tcphdr = (TCPHdr*)(pkt->wp + hdrOffset);
    }

    //Free all packets that client have accepted
    //Using seqno + packet len can free correctly when sequence number is greater than
    //2^32
    while (( (ntohl(tcphdr->seqno) + pkt->len) <= ackNum) && pkt)
    {
        pcb->sendPkts--;
        if (pcb->skt)
        {
            OSSemPost(pcb->skt->txSem);
            //if (!pcb->apRx)
            //   OSSemPost(pcb->skt->rxSem);
        }
        if (pkt->next == NULL)
        {
            pcb->txQStart = NULL;
            pcb->txQEnd = NULL;
            freePkt_F(pkt);
            pcb->sendPkts = 0;
            return;
        }
        else
        {
            pcb->txQStart = pkt->next;
            freePkt_F(pkt);

            if (pcb->txQStart == NULL)
                break;
            pkt = pcb->txQStart;
        }

        if (pkt == NULL)
            break;

        if (pkt->ipVer == IPv4)
            tcphdr = (TCPHdr*)(pkt->wp + hdrOffset);
        else
            tcphdr = (TCPHdr*)(pkt->wp + hdrOffset);
    }
}

/* 1: RST TACK*/
int tcpPeriodChk(TCPPCB *pcb)
{
    char rtFlag = 0;
    char ret = 0;
    PKT* pkt = pcb->txQStart;

    if (pcb->state == TIME_WAIT)
    {
        pkt = pcb->rxQStart;

        if (pkt == 0)
            return 1;
        else if (pkt->rTime > 20)
            return 1;
        else
        {
            pkt->rTime++;
            return 0;
        }
    }

    while (pkt)
    {
        pkt->rTime++;

        if (pkt->rTime > 4)
        {
            //Prevent send ack packet
            //if (pkt->len != 0)
#if CONFIG_TEREDO_ENABLED
            if ((pkt->ipVer == IPv6) && dpconf->teredoEnable )
            {
                ret = sendTeredoPkt(pcb, pkt);
            }
            else
            {
                ret = sendPkt(pkt);
            }
#else
            ret = sendPkt_F(pkt);
#endif
        }

        if (ret)
        {
            return ret;
        }

        pkt = pkt->next;
        rtFlag = 1;
    }

    if (pcb->delayAck && (rtFlag != 1) )
    {
        if (pcb->delayAck == SecondDelayAck)
        {
            pkt = (PKT*)allocPkt_F(0);
            pkt->ipVer = pcb->ipVer;
            addTcpHeader_F(pcb, pkt, TCP_ACK);
            //free at addEthernetHeader_F
            //freePkt_F(pkt);
            pcb->delayAck = NoDelayAck;
        }
        else
        {
            //If delayAck set and call tcPTimerTask execute immediately
            pcb->delayAck = SecondDelayAck;
        }
    }
    return ret;
}
#endif

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION >= IC_VERSION_DP_RevF )
int reserveIBPort(TCPPCB *pcb, unsigned short port)
{
    unsigned int size = sizeof(OSOOBHdr) + sizeof(PortRecord);
    OSOOBHdr *hdr;
    PortRecord *pr;
    char *data;
    int success = 0;
    int i =0 ;
    char isDefaultPort = 0;

    //if (bsp_in_band_exist() == 0)
    if(bsp_IODrvAP_exist() == 0)
        return 	success;

    //Check if have been Reserved
    #if 0
    for (i = 0; i < MaxPortMatchRecords; i++)
    {
        if (IBPortMatchTable[i].port == pcb->localPort)
        {
            return 	success;
        }
    }
#endif
    data = (char*) malloc(size);
    memset(data, 0, size);
    hdr = (OSOOBHdr *)data;
    hdr->type = OOB_REQ_CLIPORT;
    hdr->len = sizeof(PortRecord);
    pr = (PortRecord *) (data + sizeof(OSOOBHdr));
    pr->port = port;

    if (pcb)
    {
        if (pcb->ipVer == IPv4)
        {
            if (pcb->protocol == IP_PROTO_UDP)
                pr->ipv4UDP =1;
            else
                pr->ipv4TCP =1;
        }

        if (pcb->ipVer == IPv6)
        {
            if (pcb->protocol == IP_PROTO_UDP)
                pr->ipv6UDP =1;
            else
                pr->ipv6TCP =1;
        }
    }
    else
    {
        pr->ipv4UDP =1;
        pr->ipv6UDP =1;
        pr->ipv4TCP =1;
        pr->ipv6TCP =1;
    }

    for ( i = 0; i < sizeof(Default_RSV_Port)/sizeof(unsigned short); i++)
    {
        if (Default_RSV_Port[i] == port)
        {
            isDefaultPort = 1;
        }
    }

    if (!isDefaultPort)
    {
        if (bsp_get_inband_data(data, size) == PCI_DMA_OK)
        {
            if (hdr->type == OOB_REQ_DENY)
            {
                success = -1;
            }
            else
                success = 0;
        }
        else
            success = -1;
    }

    free(data);
    return success;
}
#endif

#if (CONFIG_VERSION  == IC_VERSION_DP_RevF)
void removePortFilterTable(TCPPCB *pcb)
{
    PortRecord* PortMatchTable = 0;
    unsigned int i = 0, r = 0, v = 0;
    OSOOBHdr *hdr;
    PortRecord *pr;
    char *data;

    if (pcb->hwIBPortFilterEnable)
        PortMatchTable = IBPortMatchTable;
    else if (pcb->hwOOBPortFilterEnable)
        PortMatchTable = OOBPortMatchTable;
    else
        return;

    for (i = 0; i < MaxPortMatchRecords; i++)
    {
        if (PortMatchTable[i].port == pcb->localPort)
        {
            if (pcb->ipVer == IPv4)
            {
                if (pcb->protocol == IP_PROTO_UDP)
                    PortMatchTable[i].ipv4UDP = 0;
                else
                    PortMatchTable[i].ipv4TCP = 0;
            }
            else if (pcb->ipVer == IPv6)
            {
                if (pcb->protocol == IP_PROTO_UDP)
                    PortMatchTable[i].ipv6UDP = 0;
                else
                    PortMatchTable[i].ipv6TCP = 0;
            }

            if ((PortMatchTable == OOBPortMatchTable) && bsp_in_band_exist())
            {
            }
            else if ((PortMatchTable == IBPortMatchTable) && !bsp_in_band_exist())
            {
            }
            else
            {
                if (i < 2)
                {
                    r = REG32(IOREG_IOBASE + IO_HWFunCtr);

                    if (pcb->ipVer == IPv4)
                    {
                        if (pcb->protocol == IP_PROTO_UDP)
                            v = 2;
                        else
                            v = 8;
                    }

                    if (pcb->ipVer == IPv6)
                    {
                        if (pcb->protocol == IP_PROTO_UDP)
                            v = 1;
                        else
                            v = 4;
                    }

                    if ( i == 0)
                    {
                        r = r & (~(v << 28));
                        REG32(IOREG_IOBASE + IO_HWFunCtr) = r;
                    }
                    else
                    {
                        r = r & (~(v << 24));
                        REG32(IOREG_IOBASE + IO_HWFunCtr) = r;
                    }
                }
            }

            //do reg setting
            if (!PortMatchTable[i].ipv4UDP && !PortMatchTable[i].ipv4TCP &&
                    !PortMatchTable[i].ipv6UDP && !PortMatchTable[i].ipv6TCP)
            {
                PortMatchTable[i].port = 0;

                if ((PortMatchTable == OOBPortMatchTable) && bsp_in_band_exist())
                    return;

                if ((PortMatchTable == IBPortMatchTable) && !bsp_in_band_exist())
                    return;

                if (i < 4)
                {
                    v = i*2;
                    REG16(IOREG_IOBASE + I0_OOB_PORT0 + v) = 0;
                }
                else
                    REG32(IOREG_IOBASE + I0_OOB_PORT4) &= 0xffff0000;

                switch (i)
                {
                case 0:
                case 1:
                    r = REG32(IOREG_IOBASE + IO_HWFunCtr);
                    if (i == 0)
                        r = r & (~(0xF << 28));
                    else
                        r = r & (~(0xF << 24));
                    REG32(IOREG_IOBASE + IO_HWFunCtr) = r;
                    break;
                case 2:
                case 3:
                case 4:
                    r = REG32(IOREG_IOBASE + MAC_RxCR);
                    if (i == 2)
                        r = r & ~(0x08000000);
                    else if (i == 3)
                        r = r & ~(0x04000000);
                    else
                        r = r & ~(0x00040000);

                    REG32(IOREG_IOBASE + MAC_RxCR) = r;
                    break;
                }

                if (pcb->hwOOBPortFilterEnable)
                    return ;

                i = sizeof(OSOOBHdr) + sizeof(PortRecord);
                data = (char*) malloc(i);
                memset(data, 0, i);
                hdr = (OSOOBHdr *)data;
                hdr->type = OOB_RELEASE_CLIPORT;
                hdr->len = sizeof(PortRecord);
                pr = (PortRecord *) (data + sizeof(OSOOBHdr));
                pr->port = pcb->localPort;
                bsp_get_inband_data(data, i);
                free(data);
            }
        }
    }
}

void reloadFilterTable()
{
    int ib = bsp_in_band_exist();
    PortRecord* PortMatchTable = 0;
    int i = 0, r = 0, v = 0;

    if (bsp_bits_get(IO_CONFIG, BIT_SINGLE_IP, 1) == 0)
        return;

    if (ib)
        PortMatchTable = IBPortMatchTable;
    else
        PortMatchTable = OOBPortMatchTable;

    //Clear Hw bit
    REG32(IOREG_IOBASE + IO_PortCtr) &= 0x00ffffff;
    r = 0;
    r = r | 0x08000000;
    r = r | 0x04000000;
    r = r | 0x00040000;
    REG32(IOREG_IOBASE + MAC_RxCR) &= (~r);
    bsp_bits_set(IO_HWFunCtr, 1, BIT_AATCP_UDPB, 1);

    if (ib)
    {
        if (IPInfo[eth0].RMCPFilter)
        {
            bsp_bits_set(IO_HWFunCtr, 1, BIT_FILTERRMCP, 1);
        }
    }
    else
    {
        bsp_bits_set(IO_PortCtr, 1, BIT_OOBFILTERDISABLE, 1);
    }
    //Clear Port
    for (i = 0; i < MaxPortMatchRecords; i++)
    {
        if (i < 4)
        {
            v = i * 2;
            REG16(IOREG_IOBASE + I0_OOB_PORT0 + v) = 0;
        }
        else
            REG32(IOREG_IOBASE + I0_OOB_PORT4) &= 0xffff0000;
    }

    for (i = 0; i < MaxPortMatchRecords; i++)
    {
        if (PortMatchTable[i].port)
        {
            if (i < 4)
            {
                v = i * 2;
                REG16(IOREG_IOBASE + I0_OOB_PORT0 + v) = htons(PortMatchTable[i].port);
            }
            else
                REG32(IOREG_IOBASE + I0_OOB_PORT4) |= PortMatchTable[i].port;

            switch (i)
            {
            case 0:
            case 1:
                r = REG32(IOREG_IOBASE + IO_HWFunCtr);
                v = 0;

                if (PortMatchTable[i].ipv4UDP)
                    v += 2;
                if (PortMatchTable[i].ipv4TCP)
                    v += 8;
                if (PortMatchTable[i].ipv6UDP)
                    v += 1;
                if (PortMatchTable[i].ipv6TCP)
                    v += 4;

                if ( i == 0)
                {
                    r = r | (v << 28);
                    REG32(IOREG_IOBASE + IO_HWFunCtr) = r;
                }
                else
                {
                    r = r | (v << 24);
                    REG32(IOREG_IOBASE + IO_HWFunCtr) = r;
                }
                break;
            case 2:
            case 3:
            case 4:
                r = REG32(IOREG_IOBASE + MAC_RxCR);

                if (i == 2)
                    r = r | 0x08000000;
                else if (i ==3)
                    r = r | 0x04000000;
                else if (i == 4)
                    r = r | 0x00040000;

                REG32(IOREG_IOBASE + MAC_RxCR) = r;
                break;
            }

            //if (ib)
            //    reserveIBPort_F(0, PortMatchTable[i].port);
        }
    }
}

void setPortFilterTable(TCPPCB *pcb)
{
    int i = 0, r = 0, v = 0;
    PortRecord* PortMatchTable = 0;

    if (pcb->hwIBPortFilterEnable)
        PortMatchTable = IBPortMatchTable;
    else if (pcb->hwOOBPortFilterEnable)
        PortMatchTable = OOBPortMatchTable;
    else
        return;


    for (i = 0; i < MaxPortMatchRecords; i++)
    {
        if (PortMatchTable[i].port == pcb->localPort)
        {
            if (pcb->ipVer == IPv4)
            {
                if (pcb->protocol == IP_PROTO_UDP)
                    PortMatchTable[i].ipv4UDP = 1;
                else
                    PortMatchTable[i].ipv4TCP = 1;
            }

            if (pcb->ipVer == IPv6)
            {
                if (pcb->protocol == IP_PROTO_UDP)
                    PortMatchTable[i].ipv6UDP = 1;
                else
                    PortMatchTable[i].ipv6TCP = 1;
            }
            break;
        }
    }

    if (i == MaxPortMatchRecords)
    {
        for (i = 0; i < MaxPortMatchRecords; i++)
        {
            if (PortMatchTable[i].port == 0)
            {
                PortMatchTable[i].port = pcb->localPort;
                if (pcb->ipVer == IPv4)
                {
                    if (pcb->protocol == IP_PROTO_UDP)
                        PortMatchTable[i].ipv4UDP = 1;
                    else
                        PortMatchTable[i].ipv4TCP = 1;
                }

                if (pcb->ipVer == IPv6)
                {
                    if (pcb->protocol == IP_PROTO_UDP)
                        PortMatchTable[i].ipv6UDP = 1;
                    else
                        PortMatchTable[i].ipv6TCP = 1;
                }
                break;
            }
        }
    }

    if ( i != MaxPortMatchRecords)
    {
        //If inband exist, do not set broadcast/multicast filter
        if (bsp_in_band_exist() && pcb->hwOOBPortFilterEnable)
            return;
        //If oobt, do not set unicast filter
        if ((bsp_in_band_exist() == 0)&& pcb->hwIBPortFilterEnable)
            return;

        //Store little endian(Port0~3), Store big endian Port4
        if (i < 4)
        {
            v = i * 2;
            REG16(IOREG_IOBASE + I0_OOB_PORT0 + v) = htons(pcb->localPort);
        }
        else
        {
            REG32(IOREG_IOBASE + I0_OOB_PORT4) |= pcb->localPort;
        }

        switch (i)
        {
        case 0:
        case 1:
            r = REG32(IOREG_IOBASE + IO_HWFunCtr);
            if (pcb->ipVer == IPv4)
            {
                if (pcb->protocol == IP_PROTO_UDP)
                    v = 2;
                else
                    v = 8;
            }

            if (pcb->ipVer == IPv6)
            {
                if (pcb->protocol == IP_PROTO_UDP)
                    v = 1;
                else
                    v = 4;
            }

            if ( i == 0)
            {
                r = r | (v << 28);
                REG32(IOREG_IOBASE + IO_HWFunCtr) = r;
            }
            else
            {
                r = r | (v << 24);
                REG32(IOREG_IOBASE + IO_HWFunCtr) = r;
            }

            break;
        case 2:
        case 3:
        case 4:
            r = REG32(IOREG_IOBASE + MAC_RxCR);
            if (i == 2)
                r = r | 0x08000000;
            else if (i ==3)
                r = r | 0x04000000;
            else if (i == 4)
                r = r | 0x00040000;

            REG32(IOREG_IOBASE + MAC_RxCR) = r;
            break;
        }
    }
}
#elif (CONFIG_VERSION >= IC_VERSION_EP_RevA)
void SetIBPF()
{
    setNewIBTCAMPortPF_F(IPv4, IP_PROTO_TCP);
    setNewIBTCAMPortPF_F(IPv4, IP_PROTO_UDP);
    setNewIBTCAMPortPF_F(IPv6, IP_PROTO_TCP);
    setNewIBTCAMPortPF_F(IPv6, IP_PROTO_UDP);
}

void SetOOBPF()
{
    int i = 0;
    SetOOBBasicRule_F();
    RstSharePFRuleMem(&(TCAMRuleMem[OOBPortPFRule]), 0);
    //OOB udp port filter rule, IPv4/IPv6 share the samle rule
    TCAMRuleMem[OOBPortPFRule].FROOB = 1;
TCAMRuleMem[OOBPortPFRule].FRIBNotDrop = 1;

    TCAMRuleMem[OOBPortPFRule].MARType = MULCAST_MAR | BROADCAST_MAR;
    TCAMRuleMem[OOBPortPFRule].OOBMAR = 1;

    TCAMRuleMem[OOBPortPFRule].DPortBitMap = 0;

    while( i < TCAMMem[TCAMDDPortSet].num)
    {
        TCAMRuleMem[OOBPortPFRule].DPortBitMap = TCAMRuleMem[OOBPortPFRule].DPortBitMap + (1 << i);
        i++;
    }

    TCAMRuleMem[OOBPortPFRule].ruleNO = setPFRule_F(&TCAMRuleMem[OOBPortPFRule]);

}
void removePortFilterTableEP(TCPPCB *pcb)
{
    PortRecord* PortMatchTable = 0;
    unsigned int i = 0, j = 0, dataIdx = 0;
    unsigned short* port;
    unsigned char isInBand = 0;
    OSOOBHdr *hdr;
    PortRecord *pr;
    char *data;
    unsigned char *tmp;

    if (pcb->hwIBPortFilterEnable)
        PortMatchTable = IBPortMatchTable;
    else if (pcb->hwOOBPortFilterEnable)
        PortMatchTable = OOBPortMatchTable;
    else
        return;

    for (i = 0; i < MaxPortMatchRecords; i++)
    {
        if (PortMatchTable[i].port == pcb->localPort)
        {
            if (pcb->ipVer == IPv4)
            {
                if (pcb->protocol == IP_PROTO_UDP)
                    PortMatchTable[i].ipv4UDP = 0;
                else
                    PortMatchTable[i].ipv4TCP = 0;
            }
            else if (pcb->ipVer == IPv6)
            {
                if (pcb->protocol == IP_PROTO_UDP)
                    PortMatchTable[i].ipv6UDP = 0;
                else
                    PortMatchTable[i].ipv6TCP = 0;
            }

            isInBand = bsp_in_band_exist();

            //Clear PortMatchTable entry, set new port filter list, and set all  ipver/protocol again
            if ((TCAMMem[TCAMDDPortSet].num!=0) && !PortMatchTable[i].ipv4UDP && !PortMatchTable[i].ipv4TCP &&
                    !PortMatchTable[i].ipv6UDP && !PortMatchTable[i].ipv6TCP)
            {
                if((TCAMMem[TCAMDDPortSet].num - 1) <= 0)
                {
                    tmp = 0;
                }
                else
                {
                    tmp = (unsigned char*)malloc((TCAMMem[TCAMDDPortSet].num - 1) * 2);
                }

                if(TCAMMem[TCAMDDPortSet].data)
                {
                    for(j = 0 ; j < TCAMMem[TCAMDDPortSet].num; j++)
                    {
                        port = (unsigned short*)(TCAMMem[TCAMDDPortSet].data + (j*2));
                        if(*port != PortMatchTable[i].port)
                        {
                            if(tmp!=0){
                            		memcpy(tmp + (dataIdx * 2), port, 2);
                            }
                            dataIdx++;
                        }
                    }
                    free(TCAMMem[TCAMDDPortSet].data);
                }

                if(TCAMMem[TCAMDDPortSet].num)
                {
                    TCAMMem[TCAMDDPortSet].num--;
                }

                TCAMMem[TCAMDDPortSet].data = tmp;
                PortMatchTable[i].port = 0;
                setTCAMData_F(&TCAMMem[TCAMDDPortSet]);

                reloadFilterTable_F();
#if 0

                if((PortMatchTable == IBPortMatchTable) && isInBand)
                {
                    SetIBPF_F();
                }
                else if((PortMatchTable == OOBPortMatchTable) && !isInBand)
                {
                    SetOOBPF_F();
                }
#endif

                if (pcb->hwOOBPortFilterEnable)
                    return ;

                i = sizeof(OSOOBHdr) + sizeof(PortRecord);
                data = (char*) malloc(i);
                memset(data, 0, i);
                hdr = (OSOOBHdr *)data;
                hdr->type = OOB_RELEASE_CLIPORT;
                hdr->len = sizeof(PortRecord);
                pr = (PortRecord *) (data + sizeof(OSOOBHdr));
                pr->port = pcb->localPort;
                bsp_get_inband_data(data, i);
                free(data);
            }
            else
            {
                if ((PortMatchTable == IBPortMatchTable) && isInBand)
                {
                    setNewIBTCAMPortPF_F(pcb->ipVer, pcb->protocol);
                }
            }
        }
    }

}

void reloadFilterTableEP()
{
    int ib = bsp_in_band_exist();
    PortRecord* PortMatchTable = 0;
    unsigned char *tmp;
    int i = 0;
    int count = 0, dataIdx = 0;

    if(dpconf->ipmode == DUAL_IP_MODE)
    {
        //accept match IPv4 packet(allow tcp and icmp), multucast packet/broadcast packet with special ports
        //ib and oob use the same setting
        return;
    }

    //InBand and OOB share the same port filter rule no.
    if (ib)
    {
        PortMatchTable = IBPortMatchTable;
        RstSharePFRuleMem_F(&(TCAMRuleMem[OOBUnicastPFRule]), 0);
        RstSharePFRuleMem_F(&(TCAMRuleMem[OOBIPv6PFRule]), 0);
        RstSharePFRuleMem_F(&(TCAMRuleMem[OOBPortPFRule]), 0);

    }
    else
    {
        PortMatchTable = OOBPortMatchTable;
        RstSharePFRuleMem_F(&(TCAMRuleMem[IBIPv4TCPPortPFRule]), 0);
        RstSharePFRuleMem_F(&(TCAMRuleMem[IBIPv4UDPPortPFRule]), 0);
        RstSharePFRuleMem_F(&(TCAMRuleMem[IBIPv6TCPPortPFRule]), 0);
        RstSharePFRuleMem_F(&(TCAMRuleMem[IBIPv6UDPPortPFRule]), 0);
        //OOB only use n0.1~ no.3 rule
        TCAMRuleMem[IBIPv6UDPPortPFRule].ruleNO = setPFRule_F(&TCAMRuleMem[IBIPv6UDPPortPFRule]);
        //Disable Rule 3
        PacketFilterRuleEn(TCAMRuleMem[IBIPv6UDPPortPFRule].ruleNO, 0);

    }

    for (i = 0; i < MaxPortMatchRecords; i++)
    {
        if(PortMatchTable[i].port)
        {
            count++;
        }
    }

    if(TCAMMem[TCAMDDPortSet].data)
    {
        free(TCAMMem[TCAMDDPortSet].data);
    }

    TCAMMem[TCAMDDPortSet].num = count;

    if(count)
    {
        tmp = (unsigned char*)malloc(count * 2);

        for (i = 0; i < MaxPortMatchRecords; i++)
        {
            if(PortMatchTable[i].port)
            {
                memcpy(tmp + (dataIdx * 2), &(PortMatchTable[i].port), 2);
                dataIdx++;
            }
        }

        TCAMMem[TCAMDDPortSet].data = tmp;
        setTCAMData_F(&TCAMMem[TCAMDDPortSet]);
    }
    else
    {
        TCAMMem[TCAMDDPortSet].data = 0;
    }


    if(ib)
    {
        SetIBPF_F();
    }
    else
    {
        SetOOBPF_F();
    }

}

void setNewIBTCAMPortPF(unsigned char ipVer, unsigned char protocol)
{
    TCAMRule tmpRule;
    int ruleIdx = 0;
    int i = 0;

    //IPv4 TCP, IPv4 UDP, IPv6 TCP, IPv6 UDP TCAM rule set independently
    //Get rule index at first
    memset(&tmpRule, 0, sizeof(tmpRule));
    tmpRule.FROOB = 1;

    if(ipVer == IPv4)
    {
        tmpRule.TypeBitMap = IPv4TypeBitMap;

        if(protocol == IP_PROTO_TCP)
        {
            ruleIdx = IBIPv4TCPPortPFRule;
            tmpRule.MARType = UNICAST_MAR;
            tmpRule.DIPv4BitMap = EnableUniIPv4Addr;
            tmpRule.IPv4PTLBitMap = IPv4PTLTCP;

        }
        else if(protocol == IP_PROTO_UDP)
        {
            ruleIdx = IBIPv4UDPPortPFRule;
            tmpRule.IPv4PTLBitMap = IPv4PTLUDP;
        }
    }
    else if(ipVer == IPv6)
    {
        tmpRule.TypeBitMap = IPv6TypeBitMap;
        if(protocol == IP_PROTO_TCP)
        {
            ruleIdx = IBIPv6TCPPortPFRule;
            tmpRule.IPv6PTLBitMap = IPv6PTLTCP;

        }
        else if(protocol == IP_PROTO_UDP)
        {
            ruleIdx = IBIPv6UDPPortPFRule;
            tmpRule.IPv6PTLBitMap = IPv6PTLUDP;
        }
    }

    if(protocol == IP_PROTO_TCP)
    {
        tmpRule.MARType = UNICAST_MAR;
    }
    else if(protocol == IP_PROTO_UDP)
    {
        tmpRule.MARType = UNICAST_MAR | MULCAST_MAR | BROADCAST_MAR;
    }

    tmpRule.MACIDBitMap = OOBMacBitMap;


    RstSharePFRuleMem_F(&(TCAMRuleMem[ruleIdx]), &tmpRule);


    for(i = 0; i < MaxPortMatchRecords; i++)
    {
        if(IBPortMatchTable[i].port == 0)
        {
            continue;
        }

        switch(ruleIdx)
        {
        case IBIPv4TCPPortPFRule:
            if(IBPortMatchTable[i].ipv4TCP)
            {
                TCAMRuleMem[ruleIdx].DPortBitMap = (1 << i);
            }
            break;
        case IBIPv4UDPPortPFRule:
            if(IBPortMatchTable[i].ipv4UDP)
            {
                TCAMRuleMem[ruleIdx].DPortBitMap = (1 << i);
            }
            break;

        case IBIPv6TCPPortPFRule:
            if(IBPortMatchTable[i].ipv6TCP)
            {
                TCAMRuleMem[ruleIdx].DPortBitMap = (1 << i);
            }
            break;
        case IBIPv6UDPPortPFRule:
            if(IBPortMatchTable[i].ipv6UDP)
            {
                TCAMRuleMem[ruleIdx].DPortBitMap = (1 << i);
            }
            break;

        }

    }

    TCAMRuleMem[ruleIdx].ruleNO = setPFRule_F(&TCAMRuleMem[ruleIdx]);
    if((ruleIdx == IBIPv4TCPPortPFRule) || (ruleIdx == IBIPv4UDPPortPFRule) ||
            (ruleIdx == IBIPv6TCPPortPFRule)|| (ruleIdx == IBIPv6UDPPortPFRule))
    {
        if(TCAMRuleMem[ruleIdx].DPortBitMap == 0)
        {
            TCAMRuleMem[ruleIdx].ruleNO = setPFRule_F(&TCAMRuleMem[ruleIdx]);
            PacketFilterRuleEn(TCAMRuleMem[ruleIdx].ruleNO, 0);
        }
    }
}

void setPortFilterTableEP(TCPPCB *pcb)
{
    int i = 0, isInabnd = 0;
    char newAddPort = 0;
    PortRecord* PortMatchTable = 0;
    unsigned char *tmp, isSingleIP = 0;

    if (pcb->hwIBPortFilterEnable)
        PortMatchTable = IBPortMatchTable;
    else if (pcb->hwOOBPortFilterEnable)
        PortMatchTable = OOBPortMatchTable;
    else
        return;

    for (i = 0; i < MaxPortMatchRecords; i++)
    {
        if (PortMatchTable[i].port == pcb->localPort)
        {
            if (pcb->ipVer == IPv4)
            {
                if (pcb->protocol == IP_PROTO_UDP)
                    PortMatchTable[i].ipv4UDP = 1;
                else
                    PortMatchTable[i].ipv4TCP = 1;
            }

            if (pcb->ipVer == IPv6)
            {
                if (pcb->protocol == IP_PROTO_UDP)
                    PortMatchTable[i].ipv6UDP = 1;
                else
                    PortMatchTable[i].ipv6TCP = 1;
            }
            break;
        }
    }

    if (i == MaxPortMatchRecords)
    {
        newAddPort = 1;
        for (i = 0; i < MaxPortMatchRecords; i++)
        {
            if (PortMatchTable[i].port == 0)
            {
                PortMatchTable[i].port = pcb->localPort;
                if (pcb->ipVer == IPv4)
                {
                    if (pcb->protocol == IP_PROTO_UDP)
                        PortMatchTable[i].ipv4UDP = 1;
                    else
                        PortMatchTable[i].ipv4TCP = 1;
                }

                if (pcb->ipVer == IPv6)
                {
                    if (pcb->protocol == IP_PROTO_UDP)
                        PortMatchTable[i].ipv6UDP = 1;
                    else
                        PortMatchTable[i].ipv6TCP = 1;
                }
                break;
            }
        }
    }

    isInabnd = bsp_in_band_exist();
    isSingleIP = bsp_bits_get(MAC_OOBREG, BIT_SINGLE_IP, 1);

    if((i != MaxPortMatchRecords) && newAddPort)
    {
        //If inband exist and single ip mode, do not set broadcast/multicast filter
        if (isInabnd && isSingleIP && pcb->hwOOBPortFilterEnable)
            return;
        //If oob, do not set unicast filter
        if (!isInabnd && pcb->hwIBPortFilterEnable)
            return;

        tmp = (unsigned char*)malloc((TCAMMem[TCAMDDPortSet].num + 1) * 2);
        if(TCAMMem[TCAMDDPortSet].data)
        {
            memcpy(tmp, TCAMMem[TCAMDDPortSet].data, TCAMMem[TCAMDDPortSet].num*2);
            free( TCAMMem[TCAMDDPortSet].data);
        }
        memcpy(tmp + (TCAMMem[TCAMDDPortSet].num *2), &pcb->localPort, 2);
        TCAMMem[TCAMDDPortSet].num++;
        TCAMMem[TCAMDDPortSet].data = tmp;
        setTCAMData_F(&TCAMMem[TCAMDDPortSet]);

        if(PortMatchTable == OOBPortMatchTable)
        {
            RstSharePFRuleMem_F(&(TCAMRuleMem[OOBPortPFRule]), 0);
            i = 0;
            if(!isSingleIP)
            {
            	TCAMRuleMem[OOBPortPFRule].FRIBNotDrop = 1;
            }
            //OOB udp port filter rule, IPv4/IPv6 share the samle rule
            TCAMRuleMem[OOBPortPFRule].FROOB = 1;
		TCAMRuleMem[OOBPortPFRule].FRIBNotDrop = 1;
            TCAMRuleMem[OOBPortPFRule].MARType = MULCAST_MAR | BROADCAST_MAR;
            TCAMRuleMem[OOBPortPFRule].OOBMAR = 1;
            TCAMRuleMem[OOBPortPFRule].DPortBitMap = 0;

            while( i < TCAMMem[TCAMDDPortSet].num)
            {
                TCAMRuleMem[OOBPortPFRule].DPortBitMap = TCAMRuleMem[OOBPortPFRule].DPortBitMap + (1 << i);
                i++;
            }

            TCAMRuleMem[OOBPortPFRule].ruleNO = setPFRule_F(&TCAMRuleMem[OOBPortPFRule]);
        }
    }

    if(i != MaxPortMatchRecords)
    {
        //If oob, do not set unicast filter
        if (!isInabnd && pcb->hwIBPortFilterEnable)
            return;
        if(PortMatchTable == IBPortMatchTable)
        {
            setNewIBTCAMPortPF_F(pcb->ipVer, pcb->protocol);
        }
    }
    else
    {
        //Must increase TCAM Port filter Num
    }

}
#endif

#if (CONFIG_VERSION  >= IC_VERSION_FP_RevA) || defined(CONFIG_ROM_VERIFY)
void tcpInput(PKT *pkt)
{
    TCPPCB	*newPCB;
    PEthHdr	eHdr;

    TCPHdr	*tcpHdr;
    TCPPCB	*pcb, *clientPcb = 0;
    IPHdr	*ipHdr = NULL;
    IPv6Hdr *ipv6Hdr = NULL;

#if CONFIG_mDNS_OFFLOAD_ENABLED
    unsigned int i = 0;
#endif

    DEBUGMSG(TCP_RX_DEBUG,"tcpInput ==>");

    tcpHdr = (TCPHdr *)pkt->wp;

    if (pkt->ipVer == IPv4)
        ipHdr = (IPHdr *)(pkt->wp - IP_HLEN);
    else
        ipv6Hdr = (IPv6Hdr *)(pkt->wp - IPv6_HLEN);

    if (pkt->ipVer == IPv4)
    {
        if (ipHdr->dest.addr != getIPAddress_F(pkt->intf))
        {
            DEBUGMSG(CRITICAL_ERROR | TCP_RX_DEBUG,"IPv4 address not matching");
            return;
        }
    } else
    {
        if (memcmp(ipv6Hdr->dest, getIPv6Address_F(pkt->intf), IPv6_ADR_LEN) != 0)
        {
            DEBUGMSG(CRITICAL_ERROR | TCP_RX_DEBUG,"IPv6 address not matching");
            return;
        }
    }

#if CONFIG_mDNS_OFFLOAD_ENABLED
    for (i = 0; i < mDNSOffloadCmd.numTCPPorts; i++)
    {
        if (mDNSOffloadCmd.tcpPorts[i] == tcpHdr->dest)
        {
            if ((tcpHdr->flags & TCP_SYN) ||( (ntohs(tcpHdr->dest) == 22) && !(tcpHdr->flags & TCP_FIN) ))
            {
                //Power on Syn
                SMBus_Prepare_RmtCtrl(RMCP_PWR_On, 0);
            }
            return;
        }
    }
#endif

    /* Convert fields in TCP header to host byte order. */
    tcpHdr->src = ntohs(tcpHdr->src);
    tcpHdr->dest = ntohs(tcpHdr->dest);
    tcpHdr->seqno = ntohl(tcpHdr->seqno);
    tcpHdr->ackno = ntohl(tcpHdr->ackno);
    tcpHdr->wnd = ntohs(tcpHdr->wnd);


    //HW will do TCP checksum
    //Do not care urgent pointer

    //Check if a new conection or not
    for (pcb = ActivePCBsList; pcb != NULL; pcb = pcb->next)
    {
        if (pkt->ipVer != pcb->ipVer || pcb->protocol != IP_PROTO_TCP)
        {
            continue;
        }
        //client connection
        if ((tcpHdr->dest == pcb->localPort) && (tcpHdr->src == pcb->destPort))
            break;

        //Server Port List
        if (tcpHdr->dest == pcb->localPort)
        {
            clientPcb = pcb->clientList;

            DEBUGMSG(TCP_RX_DEBUG,"Request a service");

            while (clientPcb)
            {
                if ( (pkt->ipVer ==IPv4) && (clientPcb->state != LISTEN)
                        && (tcpHdr->dest == clientPcb->localPort)
                        && (tcpHdr->src == clientPcb->destPort)
                        && (ipHdr->src.addr == clientPcb->destIPAddr.addr))
                    break;
                if ( (pkt->ipVer == IPv6) && (clientPcb->state != LISTEN)
                        && (tcpHdr->dest == clientPcb->localPort)
                        && (tcpHdr->src == clientPcb->destPort)
                        && (memcmp(ipv6Hdr->src, clientPcb->destIPv6Addr, IPv6_ADR_LEN) == 0))
                    break;
                clientPcb = clientPcb->clientList;
            }

            //Found client
            if (clientPcb)
            {
                DEBUGMSG(TCP_RX_DEBUG,"Exist connection");
                pcb = clientPcb;
            }
            else
            {
                //Check if new connection request, but client do not have clientPcb
                if (!(tcpHdr->flags & TCP_SYN) && pcb->destPort != 0)
                    pcb = 0;

                DEBUGMSG(TCP_RX_DEBUG,"A new connection");
                //A new connection
                ;
            }
            break;
        }

        //Client List
        if ((pcb->ipVer == IPv4) && (pcb->state != LISTEN)
                && (tcpHdr->dest == pcb->localPort)
                && (tcpHdr->src == pcb->destPort)
                && (ipHdr->src.addr == pcb->destIPAddr.addr))
            break;
        if ((pcb->ipVer == IPv6) && (pcb->state != LISTEN)
                && (tcpHdr->dest == pcb->localPort)
                && (tcpHdr->src == pcb->destPort)
                && (memcmp(ipv6Hdr->src, pcb->destIPv6Addr, IPv6_ADR_LEN) == 0))
            break;
    }

    if ( pcb != NULL )
    {
        if ((pcb->state == LISTEN) && (pcb->curConnections >= pcb->maxConnections))
            pcb = NULL;

        if ( pcb != NULL )
        {
            //Use pcb state to judge if a new connection(LISTEN) or exist connection
            tcpProcess_F(pkt, pcb);
        }
    }

    //If a RST packet, do not RST again
    if ( pcb == NULL && !(tcpHdr->flags & TCP_RST))
    {
        DEBUGMSG(CRITICAL_ERROR | TCP_DEBUG, "Not found matching TCP PCB");

        newPCB = tcpPCBNew_F();
        memset(newPCB, 0, sizeof(TCPPCB));
        newPCB->protocol = IP_PROTO_TCP;
		newPCB->intf = pkt->intf;

        if (pkt->ipVer == IPv4)
        {
            DEBUGMSG(TCP_RX_DEBUG,"IPv4 Client");
            eHdr = (PEthHdr)(pkt->wp - IP_HLEN - ETH_HDR_SIZE);
        }
        else
        {
            DEBUGMSG(TCP_RX_DEBUG,"IPv6 Client");
            eHdr = (PEthHdr)(pkt->wp - IPv6_HLEN - ETH_HDR_SIZE);
        }

        memcpy(newPCB->dest_MAR, eHdr->srcAddr, MAC_ADDR_LEN);

        if (pkt->ipVer == IPv4)
        {
            newPCB->destIPAddr.addr = ipHdr->src.addr;
            newPCB->ipVer = IPv4;
        }
        else
        {
            memcpy(newPCB->destIPv6Addr, ipv6Hdr->src, IPv6_ADR_LEN);
            newPCB->ipVer = IPv6;
        }

        newPCB->destPort = tcpHdr->src;
        newPCB->localPort = tcpHdr->dest;

        //ack num = sequence num(ISN) + 1
        newPCB->sendAckNum = tcpHdr->seqno + 1;
        newPCB->seqNum = tcpHdr->ackno + 1;
        newPCB->cwnd = 512;

        if (tcpHdr->flags &  TCP_FIN)
        {
            newPCB->delayAck = FirstDelayAck;
            tcpSend_F(newPCB, NULL, TCP_ACK);
        }
        else
            tcpSend_F(newPCB, NULL, TCP_ACK | TCP_RST);

        //Do not use removePCB, because pcb do not be registered
        free(newPCB);
    }

    return;
}
#endif

#if (CONFIG_VERSION  < IC_VERSION_DP_RevF) || CONFIG_ROM_TCP_DBG || CONFIG_BUILDROM
/*----------------------------Functions-------------------------------------*/

/****************************************************************************/
/**							  PCB functions									*/
/****************************************************************************/

TCPPCB *tcpPCBNew(void)
{
    TCPPCB *pcb;
    pcb = (TCPPCB*)malloc(sizeof(TCPPCB));
    memset(pcb, 0, sizeof(TCPPCB));
    return pcb;
}

int tcpListen(TCPPCB *pcb)
{
    pcb->state = LISTEN;
    tcpPCBRegister_F(pcb);
    return 0;
}

void tcpPCBRegister(TCPPCB *pcb)
{
    TCPPCB *ipcb;
    int i = 0;

    //ActivePCBsList will be assigned a pcb when the program starts
    if ( ActivePCBsList == NULL)
    {
        ActivePCBsList = pcb;
        return;
    }

    //Check if it has been registered
    for (ipcb = ActivePCBsList; ipcb->next != NULL; ipcb = ipcb->next) {
        if (pcb == ipcb) {
            DEBUGMSG(TCP_DEBUG,"tcpPCBRegister: already registered\n");
            return;
        }
        i++;
    }

    //i include ActivePCBsList(server node)
    //if connections are overfollow, removing the first client node
    if ( i > MAXCONNECTIONS)
    {
        //Reset the connection
        tcpSend_F(ActivePCBsList->next, 0, TCP_RST);
        //FIFO:Remove first connection
        ActivePCBsList->next->state = CLOSED;
        DEBUGMSG(TCP_DEBUG,"i > MAXCONNECTIONS\n");
        removePCB_F(ActivePCBsList->next);
    }

    if (ActivePCBsList->next)
    {
        pcb->next = ActivePCBsList->next;
        ActivePCBsList->next = pcb;
    }
    else
        ipcb->next = pcb;

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    //InBand must reserve server port
    if (pcb->destPort == 0 && pcb->hwIBPortFilterEnable)
    {
        reserveIBPort_F(pcb, pcb->localPort);
    }

    if (pcb->hwIBPortFilterEnable || pcb->hwOOBPortFilterEnable)
    {
        setPortFilterTable_F(pcb);
    }

#endif
}

void tcpPCBUnRegister(TCPPCB *pcb)
{
    TCPPCB *ipcb;
    TCPPCB *prePcb = 0;
    PKT* pkt;
    PKT* npkt;

    //ActivePCBsList will be assigned a pcb when the program starts
    if ( ActivePCBsList == NULL)
    {
        return;
    }
#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
    removePortFilterTable_F(pcb);
#endif
    //Free queued pkt
    pkt = pcb->rxQStart;
    while (pkt)
    {
        npkt = pkt->next;
        freePkt_F(pkt);
        pkt = npkt;
    }

    pkt = pcb->txQStart;
    while (pkt)
    {
        npkt = pkt->next;
        freePkt_F(pkt);
        pkt = npkt;
    }

    pcb->rxQStart = 0;
    pcb->txQStart = 0;

    //Check if it has been registered
    for (ipcb = ActivePCBsList; ipcb != NULL; ipcb = ipcb->next)
    {
        if (pcb == ipcb)
        {
            if (prePcb)
            {
                if (pcb->next)
                    prePcb->next = pcb->next;
                else
                    prePcb->next = 0;
            }
            else
            {
                ActivePCBsList = NULL;
            }
            pcb->next = 0;
            return;
        }
        prePcb = ipcb;
    }
}

void removeSerivce(int port)
{
    TCPPCB *curPcb = 0;
    TCPPCB *prePcb = 0;
    TCPPCB *nextPcb = 0;

    //ActivePCBsList will be assigned a pcb when the program starts
    if ( ActivePCBsList == NULL)
    {
        return;
    }

    //Check if it has been registered
    for (curPcb = ActivePCBsList; curPcb != NULL; curPcb = curPcb->next) {
        if (curPcb->localPort == port) {
            if (curPcb->next)
                prePcb->next =  curPcb->next;
            else
                prePcb->next = 0;


            while (curPcb)
            {
                nextPcb = curPcb->clientList;
                removePCB_F(curPcb);
                curPcb = nextPcb;
            }
            return;
        }
        prePcb = curPcb;
    }
}

int resetIPPtl(unsigned char IPVer)
{
    TCPPCB *curPcb = 0;
    TCPPCB *clientPcb = 0;
    TCPPCB *tmpPcb = 0;
    unsigned char err;

    /*
    err = 0;
    //Check if gamc is ready
    while((gmac_reset == RESETTING) && (err++ < 150))
    OSTimeDly(OS_TICKS_PER_SEC * 0.4);
    */

    OSSemPend(SktSem, 0, &err);

    for (curPcb = ActivePCBsList; curPcb != NULL;)
    {
        if (curPcb->ipVer != IPVer)
        {
            curPcb = curPcb->next;
            continue;
        }

        if (curPcb && curPcb->state == LISTEN)
        {
            //TCP Server PCB
            clientPcb = curPcb->clientList;
            while (clientPcb)
            {
                tmpPcb = clientPcb->clientList;
                tcpSend_F(clientPcb, NULL, TCP_ACK | TCP_RST);
                OSTimeDly(OS_TICKS_PER_SEC*0.2);
                removePCB_F(clientPcb);
                clientPcb = tmpPcb;
            }
            curPcb = curPcb->next;
        }
        else if (curPcb && curPcb->protocol == IP_PROTO_TCP)
        {
            //TCP Client PCB
            tmpPcb = curPcb->next;
            tcpSend_F(curPcb, NULL, TCP_ACK | TCP_RST);
            OSTimeDly(OS_TICKS_PER_SEC*0.2);
            removePCB_F(curPcb);
            curPcb = tmpPcb;
        }
        else
        {
            curPcb = curPcb->next;
        }
    }
    OSSemPost(SktSem);
    return 1;
}

void tcpConnectionIn(TCPPCB *srvPcb, TCPPCB *clientPcb)
{
    TCPPCB	*tmp;

    DEBUGMSG(TCP_RX_DEBUG,"A new client connect. <tcpConnectionIn>");

    tmp = srvPcb;

    while (tmp->clientList)
        tmp = tmp->clientList;

    clientPcb->accepted = 0;
    tmp->clientList = clientPcb;
}

void postListenPort(TCPPCB *pcb)
{
    TCPPCB *ipcb;
    /*
    TCPPCB *prev = NULL;
    TCPPCB *tPCB;
    TCPPCB *hPCB;
    */
    //Check if it has been registered
    for (ipcb = ActivePCBsList; ipcb != NULL; ipcb = ipcb->next)
    {
        if (ipcb->localPort == pcb->localPort && ipcb->protocol == pcb->protocol)
        {
#if 0
            //Move to first
            if (ipcb->clientList != pcb && ipcb->clientList)
            {
                hPCB = ipcb->clientList;
                tPCB = ipcb->clientList;

                while (tPCB)
                {
                    if (tPCB == pcb && prev != NULL)
                    {
                        prev->clientList = pcb->clientList;
                        ipcb->clientList = pcb;
                        pcb->clientList = hPCB;
                        break;
                    }
                    prev = tPCB;
                    tPCB = tPCB->clientList;
                }
            }
#endif
            OSSemPost(ipcb->skt->rxSem);
            return;
        }
    }
}

void tcpRx(TCPPCB *pcb,
           int (* apRx)(void *arg, int len,TCPPCB *newpcb),
           void *apRxArg)
{
    pcb->apRx = apRx;
}

void tcpEnqueue(TCPPCB *pcb, PKT *pkt)
{
    pkt->ipVer = pcb->ipVer;
    pkt->queued = 1;

    if (pcb->txQStart ==0 && pcb->txQEnd == 0)
    {
        pcb->txQStart = pcb->txQEnd = pkt;
    }
    else
    {
        if (!pcb->txQEnd->prev)
        {
            pcb->txQStart->next = pkt;
            pkt->prev = pcb->txQStart;
            pcb->txQEnd = pkt;
        }
        else
        {
            pcb->txQEnd->next = pkt;
            pkt->prev = pcb->txQEnd;
            pcb->txQEnd = pkt;
        }
    }
}





int tcpProbe(TCPPCB *pcb)
{
    pcb->probeCount = 0;
    pcb->alive = 0;
    pcb->probe = 1;

    //Probe 10 times
    while (pcb->alive == 0 && pcb->probeCount < 10)
    {
        pcb->probeCount++;
        tcpSend_F(pcb, NULL, TCP_ACK);
        //Spec is delay 75s
        OSTimeDly(OS_TICKS_PER_SEC*75);
    }

    pcb->probe = 0;

    return pcb->alive;
}

void tcpClose(TCPPCB *pcb, PKT* pkt)
{
    pcb->state = FIN_WAIT_1;

    if (pkt)
        tcpSend_F(pcb, pkt, TCP_ACK | TCP_FIN);
    else
        tcpSend_F(pcb, 0,	TCP_ACK | TCP_FIN);
}

void tcpSend(TCPPCB *pcb, PKT* pkt,unsigned char flag)
{
    TCPHdrOpt* opt;

    DEBUGMSG(TCP_DEBUG,"tcpSend\n");

    if (flag == (TCP_PSH | TCP_ACK))
    {
        addTcpHeader_F(pcb, pkt, flag);
    }
    //Three way handshaking(Client)
    else if ( flag == (TCP_ACK|TCP_SYN) )
    {
        //opt data size is 4
        pkt = (PKT*)allocPkt_F(4);
        pkt->ipVer = pcb->ipVer;
        pcb->ipID = 0;

        opt = (TCPHdrOpt*)(pkt->wp);

        opt->optCode = 2;
        opt->optLen = 4;

        if (pcb->cmss >= TCPMAXSEGSIZE)
            opt->optMss = htons(TCPMAXSEGSIZE);
        else
            opt->optMss = htons(pcb->cmss);

        //We need not to queue ACK SYN data.
        //If ACK SYN lost, we will retransmit the packet
        tcpEnqueue_F(pcb, pkt);
        addTcpHeader_F(pcb, pkt, flag);
        pkt->len = 0;
    }
    //Three way handshaking(Client SYN)
    else if ( flag == TCP_SYN )
    {
        pcb->ipID = 0;
        //opt data size is 4
        pkt = (PKT*)allocPkt_F(4);
        pkt->ipVer = pcb->ipVer;

        opt = (TCPHdrOpt*)(pkt->wp);

        opt->optCode = 2;
        opt->optLen = 4;

        opt->optMss = htons(TCPMAXSEGSIZE);
        addTcpHeader_F(pcb, pkt, flag);
    }
    else if ( flag == TCP_ACK )
    {
        if ( pkt != NULL)
        {
            addTcpHeader_F(pcb, pkt, flag);
        }
        else
        {
            if (pcb->probe)
            {
                pkt = (PKT*)allocPkt_F(1);
                pkt->ipVer = pcb->ipVer;
                addTcpHeader_F(pcb, pkt, flag);
            }
            else if (pcb->delayAck == NoDelayAck)
            {
                pcb->delayAck = FirstDelayAck;
            }
            else
            {
                pkt = (PKT*)allocPkt_F(0);
                pkt->ipVer = pcb->ipVer;
                addTcpHeader_F(pcb, pkt, flag);
            }
        }
    }
    else if (flag == (TCP_ACK|TCP_FIN))
    {
        if ( pkt != NULL)
        {
            addTcpHeader_F(pcb, pkt, flag);
        } else
        {
            /*Three way handshaking ACK-FIN or Two way handshaking*/
            pkt = (PKT*)allocPkt_F(0);
            pkt->ipVer = pcb->ipVer;

            tcpEnqueue_F(pcb, pkt);
            addTcpHeader_F(pcb, pkt, flag);
        }
    }
    else if ( flag == TCP_FIN)
    {
        pkt = (PKT*)allocPkt_F(0);
        pkt->ipVer = pcb->ipVer;
        tcpEnqueue_F(pcb, pkt);
        addTcpHeader_F(pcb, pkt, flag);
    }
    else if ( flag == TCP_RST || flag == (TCP_RST | TCP_ACK) )
    {
        //Do not need to enqueue data
        pkt = (PKT*)allocPkt_F(0);
        pkt->ipVer = pcb->ipVer;
        addTcpHeader_F(pcb, pkt, flag);
    }
}

void tcpSendData(TCPPCB *pcb, PKT *pkt)
{
    tcpEnqueue_F(pcb, pkt);
    tcpSend_F(pcb, pkt, TCP_PSH | TCP_ACK);
}

unsigned int v6pseudochksum(void *data,int len)
{
    unsigned int	acc, sum;
    acc = chksum(data, len);
    sum = (acc & 0xffff) + (acc >> 16);
    sum += (sum >> 16);
    return sum;
}




#endif

#ifdef CONFIG_PortMatch_Test
int PMRxFun(void *pData, int len, struct tcp_pcb *pcb)
{
    PKT *pkt;
    if (pcb->protocol == IP_PROTO_UDP)
    {
        pkt = (PKT*)allocPkt_F(10);
        memcpy(pkt->wp, "UDP OK", 6);
        udpSend_F(pcb, pkt, 10);
    }

    return 0;
}

void doPMTAct(char *p)
{
    unsigned char err;
    int i = 0;
    PMMsg *pmMsg = (PMMsg *)p;
    RTSkt* tmpSkt = 0;


    if (pmMsg->Add)
    {
        for (i = 0; i < MaxPortMatchRecords * 2; i++)
        {
            if (PMTPRs[i].port == pmMsg->port)
            {
                //Port exist
                break;
            }
        }

        if (i == MaxPortMatchRecords * 2)
        {
            //New port
            for (i = 0; i < MaxPortMatchRecords * 2; i++)
            {
                if (PMTPRs[i].port == 0)
                {
                    break;
                }
            }
            //PMTPRs full
            if (i == MaxPortMatchRecords * 2)
                return;
        }

        PMTPRs[i].port = pmMsg->port;

        if (pmMsg->PMTIPv4 && pmMsg->PMTTCP)
        {
            tmpSkt = rtSkt_F(IPv4, IP_PROTO_TCP);
            PMTPRs[i].sv4TCP = tmpSkt;
        }

        if (pmMsg->PMTIPv6 && pmMsg->PMTTCP)
        {
            tmpSkt = rtSkt_F(IPv6, IP_PROTO_TCP);
            PMTPRs[i].sv6TCP = tmpSkt;
        }

        if (pmMsg->PMTIPv4 && pmMsg->PMTUDP)
        {
            tmpSkt = rtSkt_F(IPv4, IP_PROTO_UDP);
            PMTPRs[i].sv4UDP = tmpSkt;
        }

        if (pmMsg->PMTIPv6 && pmMsg->PMTUDP)
        {
            tmpSkt = rtSkt_F(IPv6, IP_PROTO_UDP);
            PMTPRs[i].sv6UDP = tmpSkt;
        }

        tmpSkt->pcb->apRx = PMRxFun;

        if (pmMsg->unicast)
        {
            rtSktEnableIBPortFilter_F(tmpSkt);
        }
        else
        {
            rtSktEnableOOBPortFilter_F(tmpSkt);
        }

        if (pmMsg->PMTTCP)
        {
            rtSktBind_F(tmpSkt, pmMsg->port);
            rtSktListen_F(tmpSkt, 1);
        }

        if (pmMsg->PMTUDP)
        {
            rtSktUDPBind_F(tmpSkt, pmMsg->port);
        }
    }
    else
    {
        for (i = 0; i < MaxPortMatchRecords * 2; i++)
        {
            if (PMTPRs[i].port == pmMsg->port)
            {
                if (pmMsg->PMTIPv4 && pmMsg->PMTUDP && PMTPRs[i].sv4UDP)
                {
                    tmpSkt = PMTPRs[i].sv4UDP;
                    PMTPRs[i].sv4UDP = 0;
                }

                if (pmMsg->PMTIPv4 && pmMsg->PMTTCP && PMTPRs[i].sv4TCP)
                {
                    tmpSkt = PMTPRs[i].sv4TCP;
                    PMTPRs[i].sv4TCP = 0;
                }

                if (pmMsg->PMTIPv6 && pmMsg->PMTUDP && PMTPRs[i].sv6UDP)
                {
                    tmpSkt = PMTPRs[i].sv6UDP;
                    PMTPRs[i].sv6UDP = 0;
                }

                if (pmMsg->PMTIPv6 && pmMsg->PMTTCP && PMTPRs[i].sv6TCP)
                {
                    tmpSkt = PMTPRs[i].sv6TCP;
                    PMTPRs[i].sv6TCP = 0;
                }

                rtSktClose_F(tmpSkt);
                OSSemDel(tmpSkt->rxSem, OS_DEL_ALWAYS, &err);
                OSSemDel(tmpSkt->txSem, OS_DEL_ALWAYS, &err);
                tmpSkt->rxSem=0;
                tmpSkt->txSem=0;

                if ((PMTPRs[i].sv4TCP == 0) && (PMTPRs[i].sv4UDP == 0) && (PMTPRs[i].sv6TCP == 0) && (PMTPRs[i].sv6UDP == 0))
                {
                    PMTPRs[i].port = 0;
                }
            }
        }
    }

}
#endif

#if defined(CONFIG_BUILDROM) && !CONFIG_ROM_VERIFY
void tcpTimerTask(void *data)
{
    TCPPCB *ipcb;
    TCPPCB *cpcb;
    unsigned char err = 0;
    unsigned char arpTimeOut = 0;
    unsigned char isLinkChange = 0;
    unsigned char pfTimeOut = 0;

    while (1)
    {
        isLinkChange = 0;

        if (IPInfo[eth0].LinkChange)
        {
            arpTableTimeOutReset_F();

            if (dpconf->DHCPv4Enable & (1<<eth0))
            {
                if (dpconf->GateWayIP[eth0].addr)
                {
                    //If link change, wait for link on to prevent packet loss
                    OSTimeDly(OS_TICKS_PER_SEC*3);
                    memset(IPInfo[eth0].IPv4GateWayMAR, 0, MAC_ADDR_LEN);
                    if (getGateWayMAR_F(0) == -1)
                    {
                        IPInfo[eth0].IPv4Reset = 1;
                        IPInfo[eth0].IPv6Reset = 1;
                        isLinkChange = 1;
                    }
                }
            }
            else
                isLinkChange = 1;

            IPInfo[eth0].LinkChange = 0;
        }


        if (IPInfo[eth0].IPv4Reset)
        {
            if (resetIPPtl_F(IPv4) == 1)
            {
                IPInfo[eth0].IPv4Reset = 0;
            }
        }

        if (IPInfo[eth0].IPv6Reset)
        {
            if (resetIPPtl_F(IPv6) == 1)
            {
                IPInfo[eth0].IPv6Reset = 0;
            }
        }

        arpTimeOut++;

        if (isLinkChange)
        {
            if ((dpconf->DHCPv4Enable&(1<<eth0))  && !bsp_in_band_exist())
                OSTaskResume(TASK_DHCPv4_PRIO);
        }
        else if (arpTimeOut > 12)
        {
            //Arp timeout in linux is about 4H
            //realtek use 10s as Arp timeout
            arpTableTimeOutUpdate_F();
            arpTimeOut = 0;
            if ( dpconf->HostIP[eth0].addr == 0 && (dpconf->DHCPv4Enable &(1<<eth0))&& !bsp_in_band_exist())
                OSTaskResume(TASK_DHCPv4_PRIO);
        }


#if CONFIG_VERSION	>= IC_VERSION_DP_RevF
        if (IPInfo[eth0].DoReloadFilterTable)
        {
            //Max wait time is 30s
            if (pfTimeOut > 75)
            {
                pfTimeOut = 0;
                IPInfo[eth0].DoReloadFilterTable = 0;
            }

            pfTimeOut++;

            if (bsp_in_band_exist())
            {
                if (bsp_AP_exist())
                {
                    IPInfo[eth0].DoReloadFilterTable = 0;
                    pfTimeOut = 0;
                }
            }
            else
            {
                IPInfo[eth0].DoReloadFilterTable = 0;
                pfTimeOut = 0;
            }


            //Set Port match register when fisrt in or pfTimeOut = 0
            if (pfTimeOut <= 1)
                reloadFilterTable_F();
        }

#endif
        OSTimeDly(TCP_TIMER_DLY);
    }
}

#elif (CONFIG_VERSION  <= IC_VERSION_DP_RevF) ||  CONFIG_ROM_VERIFY
int notifyIBBindPort()
{
    int ib = bsp_in_band_exist();
    PortRecord* PortMatchTable = 0;
    int i = 0;

    if (dpconf->ipmode == DUAL_IP_MODE)
        return 0;

#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    if (ib)
        PortMatchTable = IBPortMatchTable;
    else
        return 0;

    for (i = 0; i < MaxPortMatchRecords; i++)
    {
        if (PortMatchTable[i].port)
        {
            if (ib)
            {
                if(reserveIBPort_F(0, PortMatchTable[i].port) == -1)
                	return -1;
        	}
                
        }
    }
    return 0;
#endif
}


//extern INT32U CPU_CLK_FREQ;
//INT32U CPU_CLK_FREQ_D;
extern INT8U pcie_up_timer;
void tcpTimerTask(void *data)
{
	TCPPCB *ipcb;
	TCPPCB *cpcb;
	unsigned char err = 0;
	unsigned char arpTimeOut = 0;
	unsigned char isLinkChange = 0;
	unsigned char	pfTimeOut = 0;
	unsigned char notifyIBSuccess = 1;
	
    while (1)
    {
        isLinkChange = 0;
        OSSemPend(SktSem, 0, &err);
        for (ipcb = ActivePCBsList; ipcb != NULL; ipcb = ipcb->next)
        {
            if (ipcb->protocol != IP_PROTO_TCP)
                continue;

            if (tcpPeriodChk_F(ipcb))
            {
                removePCB_F(ipcb);
                break;
            }

            cpcb = ipcb->clientList;
            while (cpcb)
            {
                if (tcpPeriodChk_F(cpcb))
                {
                    removePCB_F(cpcb);
                    break;
                }
                cpcb = cpcb->clientList;
            }
        }
        OSSemPost(SktSem);

        if (IPInfo[eth0].LinkChange)
        {

		bsp_timer_reinit();//+briank.rtk

		
            arpTableTimeOutReset_F();

            if (dpconf->DHCPv4Enable & (1<<eth0))
            {
                if (dpconf->GateWayIP[eth0].addr)
                {
                    //If link change, wait for link on to prevent packet loss
                    OSTimeDly(OS_TICKS_PER_SEC*3);
                    memset(IPInfo[eth0].IPv4GateWayMAR, 0, MAC_ADDR_LEN);
                    if (getGateWayMAR_F(0) == -1)
                    {
                        IPInfo[eth0].IPv4Reset = 1;
                        IPInfo[eth0].IPv6Reset = 1;
                        isLinkChange = 1;
                    }else{
                    #if CONFIG_LINKCHG_REGETDHCP_ENABLED
	                	isLinkChange = 1;
		#endif			
                    }
                }else{
                	isLinkChange = 1; 
                }
            }
            else
                isLinkChange = 1;

            IPInfo[eth0].LinkChange = 0;
#if CONFIG_mDNS_OFFLOAD_ENABLED
            if (mDNSOffloadCmd.linkChangeCount > 40)
                SMBus_Prepare_RmtCtrl(RMCP_PWR_On, 0);
#endif
        }

#if CONFIG_mDNS_OFFLOAD_ENABLED
        if (mDNSOffloadCmd.linkChangeCount < 50)
            mDNSOffloadCmd.linkChangeCount++;
#endif

        if (IPInfo[eth0].IPv4Reset)
        {
            if (resetIPPtl_F(IPv4) == 1)
            {
                IPInfo[eth0].IPv4Reset = 0;
            }
        }

        if (IPInfo[eth0].IPv6Reset)
        {
            if (resetIPPtl_F(IPv6) == 1)
            {
                IPInfo[eth0].IPv6Reset = 0;
            }
        }

        arpTimeOut++;

        if (isLinkChange)
        {
            if ((dpconf->DHCPv4Enable & (1<<eth0)) && !bsp_in_band_exist()){				
                OSTaskResume(TASK_DHCPv4_PRIO);
            	}
        }
        else if (arpTimeOut > 12)
        {
            //Arp timeout in linux is about 4H
            //realtek use 10s as Arp timeout
            arpTableTimeOutUpdate_F();
            arpTimeOut = 0;
            if ( dpconf->HostIP[eth0].addr == 0 && (dpconf->DHCPv4Enable &(1<<eth0))&& !bsp_in_band_exist()){				
                OSTaskResume(TASK_DHCPv4_PRIO);
            	}
        }

#if 0
#ifdef CONFIG_DASH_ENABLED
        if (gmac_reset)
        {
            gmac_reset = 0;
            //dashtimeout = 50;
            if (dashcs)
            {
                OS_SEM_DATA SemData;
                if (OSSemQuery(dashcs->rxSem, &SemData) == OS_ERR_NONE)
                {
                    if (SemData.OSCnt == 0)
                    {
                        OSSemPost(dashcs->rxSem);
                    }
                }
            }
            if (dashtlscs)
            {
                OS_SEM_DATA SemData;
                if (OSSemQuery(dashtlscs->rxSem, &SemData) == OS_ERR_NONE)
                {
                    if (SemData.OSCnt == 0)
                    {
                        OSSemPost(dashtlscs->rxSem);
                    }
                }
            }
        }
#endif
#endif

#if CONFIG_VERSION  >= IC_VERSION_DP_RevF
       // unsigned char	pfTimeOut = 0;
       
       if (IPInfo[eth0].DoReloadFilterTable)
       {
       		IPInfo[eth0].DoReloadFilterTable = 0;
       		notifyIBSuccess = 0;
       		reloadFilterTable_F();     	
       }
       
        if (!notifyIBSuccess)
        {
            //Max wait time is 30s
            if (pfTimeOut > 75)
            {
                pfTimeOut = 0;
                notifyIBSuccess = 1;
            }

            pfTimeOut++;

            if (bsp_in_band_exist())
            {
                if (bsp_AP_exist())
                {                    
                    pfTimeOut = 0;
                    if(notifyIBBindPort() == -1)
                    {
                    	notifyIBSuccess = 0;
                    }
                    else
                    {
                    	notifyIBSuccess = 1;
                    }
                }
            }
            else
            {
                notifyIBSuccess = 1;
                pfTimeOut = 0;
            }
            
        }

#endif

	/*
	#ifdef CONFIG_POWER_SAVING_ENABLED
	       #if CONFIG_VERSION == IC_VERSION_EP_RevB   

			if(pcie_up_timer > 0){
				pcie_up_timer--;
				if(pcie_up_timer == 0){
					//EnableGigaPowerSavingParameter();
					//printf("[RTK] TCR Handler\n");					
					#ifdef CONFIG_LANWAKEUP_ENABLED
					//bsp_bits_set(MAC_OOB_LANWAKE , 0 , BIT_OOBLANWAKE , 1);//set BIT_OOBLANWAKE to 1
					#endif
					
				}else{
					//printf("[RTK] pcie timer = %d\n",pcie_up_timer);
				}
			}
		   
		 #endif
	#endif	 
	*/

        OSTimeDly(TCP_TIMER_DLY);
    }
}
#else
void tcpTimerTask(void *data)
{
    TCPPCB *ipcb;
    TCPPCB *cpcb;
    unsigned char err = 0;
    unsigned char arpTimeOut = 0;
    unsigned char isLinkChange = 0;
    unsigned char pfTimeOut = 0;


    while (1)
    {
        isLinkChange = 0;

        if (IPInfo[eth0].LinkChange)
        {
            arpTableTimeOutReset_F();

            if (dpconf->DHCPv4Enable & (1<<eth0))
            {
                if (dpconf->GateWayIP[eth0].addr)
                {
                    //If link change, wait for link on to prevent packet loss
                    OSTimeDly(OS_TICKS_PER_SEC*3);
                    memset(IPInfo[eth0].IPv4GateWayMAR, 0, MAC_ADDR_LEN);
                    if (getGateWayMAR_F(0) == -1)
                    {
                        IPInfo[eth0].IPv4Reset = 1;
                        IPInfo[eth0].IPv6Reset = 1;
                        isLinkChange = 1;
                    }else{
                    #if CONFIG_LINKCHG_REGETDHCP_ENABLED	
                    	isLinkChange = 1;
		#endif			
                    }
                }else{
                	isLinkChange = 1;
                }
            }
            else
                isLinkChange = 1;

            IPInfo[eth0].LinkChange = 0;
        }


        if (IPInfo[eth0].IPv4Reset)
        {
            if (resetIPPtl_F(IPv4) == 1)
            {
                IPInfo[eth0].IPv4Reset = 0;
            }
        }

        if (IPInfo[eth0].IPv6Reset)
        {
            if (resetIPPtl_F(IPv6) == 1)
            {
                IPInfo[eth0].IPv6Reset = 0;
            }
        }

        arpTimeOut++;

        if (isLinkChange)
        {
            if ((dpconf->DHCPv4Enable&(1<<eth0)) && !bsp_in_band_exist()){
                OSTaskResume(TASK_DHCPv4_PRIO);
            	}
        }
        else if (arpTimeOut > 12)
        {
            //Arp timeout in linux is about 4H
            //realtek use 10s as Arp timeout
            arpTableTimeOutUpdate_F();
            arpTimeOut = 0;
            if (dpconf->HostIP[eth0].addr == 0 && dpconf->DHCPv4Enable && !bsp_in_band_exist()){		 
                OSTaskResume(TASK_DHCPv4_PRIO);
            	}
        }


#if CONFIG_VERSION	>= IC_VERSION_DP_RevF
        if (IPInfo[eth0].DoReloadFilterTable)
        {
            //Max wait time is 30s
            if (pfTimeOut > 75)
            {
                pfTimeOut = 0;
                IPInfo[eth0].DoReloadFilterTable = 0;
            }

            pfTimeOut++;

            if (bsp_in_band_exist())
            {
                if (bsp_AP_exist())
                {
                    IPInfo[eth0].DoReloadFilterTable = 0;
                    pfTimeOut = 0;
                }
            }
            else
            {
                IPInfo[eth0].DoReloadFilterTable = 0;
                pfTimeOut = 0;
            }


            //Set Port match register when fisrt in or pfTimeOut = 0
            if (pfTimeOut <= 1)
                reloadFilterTable_F();
        }

#endif 
    /*
	#ifdef CONFIG_POWER_SAVING_ENABLED
	       #if CONFIG_VERSION == IC_VERSION_EP_RevB   

			if(pcie_up_timer > 0){
				pcie_up_timer--;
				if(pcie_up_timer == 0){
					//EnableGigaPowerSavingParameter();
					//printf("[RTK] TCR Handler\n");
					#ifdef CONFIG_LANWAKEUP_ENABLED					
					//bsp_bits_set(MAC_OOB_LANWAKE , 0 , BIT_OOBLANWAKE , 1);//set BIT_OOBLANWAKE to 1
					#endif
				}else{
					//printf("[RTK] pcie timer = %d\n",pcie_up_timer);
				}
			}
		   
		 #endif
	#endif	 
	*/
	
        OSTimeDly(TCP_TIMER_DLY);
    }
}

#endif

#ifdef CONFIG_IPERF_Test
int iperfRx(void *pData, int len, struct tcp_pcb *pcb)
{
}

void iperfTask(void *data)
{
	#if 0
	RTSkt* s;
	RTSkt* s1;
	PKT* pkt;
	int status = 0;

	
	s = rtSkt_F(IPv4, IP_PROTO_UDP);
    rtSktEnableOOBPortFilter_F(s);

	rtSktUDPBind_F(s, 5300);
	s->pcb->apRx = iperfRx;
    while (1)
    {
        OSTimeDly(OS_TICKS_PER_SEC * 1000);
    }
    while (1)
    {
		pkt = rtSktRx_F(s, 0, &status);

		if (status == SKTDISCONNECT ||status == OS_ERR_TIMEOUT)
		{
			//printf("mDNS Rx Error\n");
			break;
		}
		if(pkt)
		{
			//printf("Rx\n");			
			freePkt(pkt);
		}
    }
	#endif
	#if 1
#define IPERF_RX 1
#define IPERF_AP_RX 1
#define IPERF_Srv_Port 5001
#define MAX_IPERF_TX_CNT 50000

    struct client_hdr {
        int flags;
        int numThreads;
        int mPort;
        int bufferlen;
        int mWinBand;
        int mAmount;
    } cliHdr;

    unsigned char err;
    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_TCP);
    RTSkt* cs;
    PKT* rxPkt;
    PKT* txPkt;
    int status = 0;
    int i = 0;
    unsigned char tip[4] = {192,168,0,222};

#if IPERF_RX
    rtSktBind_F(s, IPERF_Srv_Port);
    rtSktListen_F(s, 10);

#if IPERF_AP_RX
    s->pcb->apRx = iperfRx;
    while (1)
    {
        OSTimeDly(OS_TICKS_PER_SEC * 1000);
    }
#else
    while (1)
    {
iperfAccept:
        cs = rtSktAccept_F(s);

        while (1)
        {
            do
            {
                rxPkt = rtSktRx_F(cs, 10000, &status);

                if (status == SKTDISCONNECT)
                {
                    rtSktClose_F(cs);
                    goto iperfAccept;
                }

                if (status == OS_ERR_TIMEOUT)
                {
                    rtSktClose_F(cs);
                    goto iperfAccept;
                }
            } while (rxPkt == 0);

            freePkt_F(rxPkt);
        }
    }
#endif
#else

    while (1)
    {

        OSTaskSuspend(OS_PRIO_SELF);
        OSTimeDly(OS_TICKS_PER_SEC * 2);

        while (rtSktConnect_F(s, tip, IPERF_Srv_Port) == -1)
        {
            rtSktClose_F(s);
            OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
            OSSemDel(s->txSem, OS_DEL_ALWAYS, &err);
            s->rxSem=0;
            s->txSem=0;
            if (s)
                free(s);
            s=NULL;
            s = rtSkt_F(IPv4, IP_PROTO_TCP);
        }


        cliHdr.flags = 0;
        cliHdr.numThreads = htonl(1);
        cliHdr.mPort = htonl(IPERF_Srv_Port);
        cliHdr.bufferlen = 0;
        cliHdr.mWinBand = htonl(0);
        cliHdr.mAmount = 0xe8ffffff;//10s

        txPkt = (PKT*)allocPkt_F(ETH_PAYLOAD_LEN);
        memcpy(txPkt->wp, &cliHdr, sizeof(cliHdr));

        while (rtSktSend_F(s, txPkt, 0) != SKTDISCONNECT)
        {
            i++;
            if (i>MAX_IPERF_TX_CNT)
                break;
            txPkt = (PKT*)allocPkt_F(ETH_PAYLOAD_LEN);
        }

        rtSktClose_F(s);
        OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
        OSSemDel(s->txSem, OS_DEL_ALWAYS, &err);
        s->rxSem=0;
        s->txSem=0;
        if (s)
            free(s);
        s=NULL;
        s = rtSkt_F(IPv4, IP_PROTO_TCP);
        i =  0;
    }


#endif
#endif

}
/*
void iperfTask(void *data)
{
unsigned char err;
RTSkt* s = rtSkt(IPv4, IP_PROTO_TCP);
RTSkt* cs;
PKT* rxPkt;
int status = 0;

rtSktBind(s, 5001);
rtSktListen(s, 10);

#if 1
s->pcb->apRx = iperfRx;
while(1)
{
OSTimeDly(OS_TICKS_PER_SEC * 1000);

}
#endif

while(1)
{
iperfAccept:
cs = rtSktAccept(s);

while(1)
{
do
{
rxPkt = rtSktRx(cs, 100000, &status);

if (status == SKTDISCONNECT)
{
rtSktClose(cs);
goto iperfAccept;
}

if(status == OS_ERR_TIMEOUT)
{
rtSktClose(cs);
goto iperfAccept;
}
}while (rxPkt == 0);

freePkt(rxPkt);
}
}
}
*/
#endif
