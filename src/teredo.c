#include <sys/ucos_ii.h>
#include <string.h>
#include <stdlib.h>
#include "rtskt.h"
#include "lib.h"
#include "icmp.h"
#include "udp.h"
#include "app_cfg.h"
#include "rmcp.h"
#include "md5.h"

#if CONFIG_TEREDO_ENABLED
RTSkt* TeredoSkt;
extern _IPInfo IPInfo[];
extern DPCONF *dpconf;
extern smbiosrmcpdata *smbiosrmcpdataptr;
UDPAddrInfo TeredoAddrInfo;

TeredoPktItem* TeredoTxQueueList;
TeredoPktItem* CurTeredoTxQueue;
TeredoPktItem* TeredoPendQueueList;
TeredoPktItem* CurTeredoPendQueue;
TeredoEntry TeredoPeerAddr[TEREDO_ENTRY_NUM];
OS_STK  TeredoTxSrvStk[TASK_TEREDO_TX_SRV_STK_SIZE];

char TeredoState;
char TeredoNewTrust;
unsigned short TmpOriginPort;
unsigned int TmpOriginIP;
OS_EVENT *TeredoEntrySem;
OS_EVENT *TeredoTxSem;

extern int teredoRA(PKT* pkt);
extern int chkTeredoIndicator(PKT* pkt, unsigned short* originPort, unsigned int* originIP);

void generateTeredoIPaddr(unsigned char intf)
{
    TeredoAddrFmt tAddr;

    memcpy(&tAddr.prefix, &dpconf->IPv6Prefix, sizeof(tAddr.prefix) + sizeof(tAddr.SrvIP));
    tAddr.C = 0;
    tAddr.z = 0;//rand()%256;
    tAddr.z1 = 0;//rand()% 32;
    tAddr.UG = 0;
    memcpy(&tAddr.CPort, &IPInfo[intf].TeredoCPort, sizeof(tAddr.CPort));
    memcpy(&tAddr.CIP, &IPInfo[intf].TeredoCIP, sizeof(tAddr.CIP));

    setIPv6Address_F((unsigned char*)&tAddr, intf);
}

void sendTeredoBubblePkt(RTSkt* TeredoSkt, unsigned char* ip, unsigned short originPort, unsigned int originIP)
{
    PKT* txPkt;
    unsigned short pktLen;
    TCPPCB	*newPCB;

    pktLen = 0;
    txPkt = allocIPPkt(pktLen, IPv6);

    newPCB = (TCPPCB*) malloc(sizeof(TCPPCB));
    memset(newPCB, 0, sizeof(TCPPCB));
    newPCB->protocol = IP_PROTO_NONHDR;
    memcpy(newPCB->destIPv6Addr, ip, IPv6_ADR_LEN);
    TmpOriginIP = originIP ^ 0xffffffff;
    TmpOriginPort = ntohs(originPort) ^ 0xffff;
    addIPHeader_F(newPCB, (PKT*)txPkt);
    free(newPCB);
}

void teredoTxPendEnqueue(PKT* pkt)
{
    unsigned char err;

    OSSemPend(TeredoEntrySem, 0 , &err);
    if (TeredoPendQueueList == 0)
    {
        TeredoPendQueueList = (TeredoPktItem*)malloc(sizeof(TeredoPktItem));
        TeredoPendQueueList->pkt = pkt;
        TeredoPendQueueList->next = 0;
        TeredoPendQueueList->addr = 0;
        CurTeredoPendQueue = TeredoPendQueueList;
    }
    else
    {
        CurTeredoPendQueue->next = (TeredoPktItem*)malloc(sizeof(TeredoPktItem));
        CurTeredoPendQueue = CurTeredoPendQueue->next;
        CurTeredoPendQueue->pkt = pkt;
        CurTeredoPendQueue->next = 0;
        CurTeredoPendQueue->addr = 0;
    }
    OSSemPost(TeredoEntrySem);
}

void teredoTxPendDequeue()
{
    TeredoPktItem* CurTeredoPktItm;
    TeredoPktItem* tmpPktItem;
    TeredoPktItem* tmpTeredoPendQueueList = 0;
    TeredoPktItem* CurPendTeredoPktItm = 0;
    //TeredoPktItem* nextPktItem;
    unsigned char err;
    IPv6Hdr	*ipv6Hdr;
    int tmp;
    UDPAddrInfo* addr;

    if (TeredoNewTrust == 0)
    {
        return;
    }

    TeredoNewTrust = 0;
    OSSemPend(TeredoEntrySem, 0, &err);

    CurTeredoPktItm = TeredoPendQueueList;

    while (CurTeredoPktItm)
    {
        tmpPktItem = CurTeredoPktItm;
        CurTeredoPktItm = CurTeredoPktItm->next;

        ipv6Hdr = (IPv6Hdr*)(tmpPktItem->pkt->wp);
        tmp = chkTeredoPeerList(ipv6Hdr->dest);

        if (tmp >= 0)
        {
            addr = (UDPAddrInfo*)malloc(sizeof(UDPAddrInfo));
            memcpy(&addr->destIPAddr.addr, TeredoPeerAddr[tmp].MappedIP, IPv4_ADR_LEN);
            memcpy(&addr->destPort ,TeredoPeerAddr[tmp].MappedPort, sizeof(addr->destPort));
            rtSktUDPSendTo_F(TeredoSkt, tmpPktItem->pkt, addr);
            free(addr);
            free(tmpPktItem);
        }
        else if (tmp == TEREDO_NO_ENTRY)
        {
            freePkt(tmpPktItem->pkt);
            free(tmpPktItem);
        }
        else if (tmp == TEREDO_NO_TRUST_ENTRY)
        {
            // not trust
            if (tmpTeredoPendQueueList == 0)
            {
                tmpTeredoPendQueueList = tmpPktItem;
                CurPendTeredoPktItm = tmpPktItem;
                tmpPktItem->next = 0;
            }
            else
            {
                CurPendTeredoPktItm->next = tmpPktItem;
                tmpPktItem->next = 0;
            }
        }
    }

    TeredoPendQueueList = tmpTeredoPendQueueList;

    OSSemPost(TeredoEntrySem);
}

void teredoTxEnqueue(PKT* pkt, UDPAddrInfo *addr)
{
    unsigned char err;
    OSSemPend(TeredoEntrySem, 0 , &err);
    if (TeredoTxQueueList == 0)
    {
        TeredoTxQueueList = (TeredoPktItem*)malloc(sizeof(TeredoPktItem));
        TeredoTxQueueList->next = 0;
        TeredoTxQueueList->pkt = pkt;
        TeredoTxQueueList->addr = addr;
        CurTeredoTxQueue = TeredoTxQueueList;
    }
    else
    {
        CurTeredoTxQueue->next = (TeredoPktItem*)malloc(sizeof(TeredoPktItem));
        CurTeredoTxQueue  = CurTeredoTxQueue->next;
        CurTeredoTxQueue->next = 0;
        CurTeredoTxQueue->pkt = pkt;
        CurTeredoTxQueue->addr = addr;
    }
    OSSemPost(TeredoEntrySem);
    OSSemPost(TeredoTxSem);
}

void teredoDequeue()
{
    unsigned char err;
    TeredoPktItem* tmpTeredoTxQueue;
    TeredoPktItem* tmpNxtTeredoTxQueue;
    unsigned int tmpWP;

    OSSemPend(TeredoEntrySem, 0 , &err);
    tmpTeredoTxQueue = TeredoTxQueueList;
    TeredoTxQueueList = 0;
    CurTeredoTxQueue = 0;
    OSSemPost(TeredoEntrySem);

    while (tmpTeredoTxQueue)
    {
        tmpNxtTeredoTxQueue = tmpTeredoTxQueue->next;
        tmpWP = (unsigned int)tmpTeredoTxQueue->pkt->wp;
        if ((tmpWP%4) != 0)
        {
            tmpWP = (unsigned int)(tmpTeredoTxQueue->pkt->wp - (tmpWP%4));
            memcpy((void*)tmpWP, tmpTeredoTxQueue->pkt->wp, tmpTeredoTxQueue->pkt->len);
            tmpTeredoTxQueue->pkt->wp = (unsigned char*)tmpWP;
        }
        rtSktUDPSendTo_F(TeredoSkt, tmpTeredoTxQueue->pkt, tmpTeredoTxQueue->addr);
        free(tmpTeredoTxQueue->addr);
        free(tmpTeredoTxQueue);
        tmpTeredoTxQueue = tmpNxtTeredoTxQueue;
    }
}

int chkTeredoPeerList(unsigned char *ipv6Addr)
{
    int i = 0;

    for (;i < TEREDO_ENTRY_NUM; i++)
    {
        if (memcmp(TeredoPeerAddr[i].SrcIPv6, ipv6Addr, IPv6_ADR_LEN) == 0)
        {
            if (TeredoPeerAddr[i].trust)
            {
                return i;
            }
            else
            {
                return TEREDO_NO_TRUST_ENTRY;
            }
        }

    }

    return TEREDO_NO_ENTRY; //not found
}

void createTeredoAddrEntry(unsigned char *ipv6Addr,int nonce)
{
    unsigned char i = 0;
    unsigned char oldTime = 0;
    unsigned char oldEntryIdx = 0;

    for (i = 0; i < TEREDO_ENTRY_NUM; i++)
    {
        if (TeredoPeerAddr[i].time == 0)
        {
            memset(&TeredoPeerAddr[i], 0 ,sizeof(TeredoEntry));
            TeredoPeerAddr[i].time = 1;
            TeredoPeerAddr[i].cv = nonce;
            memcpy(TeredoPeerAddr[i].SrcIPv6, ipv6Addr, IPv6_ADR_LEN);
            break;
        }
        else
        {
            if (TeredoPeerAddr[i].time > oldTime)
            {
                oldTime = TeredoPeerAddr[i].time;
                oldEntryIdx = i;
            }
        }
    }

    if (i == TEREDO_ENTRY_NUM)
    {
        memset(&TeredoPeerAddr[oldEntryIdx], 0 ,sizeof(TeredoEntry));
        TeredoPeerAddr[oldEntryIdx].time = 1;
        TeredoPeerAddr[oldEntryIdx].cv = nonce;
        memcpy(TeredoPeerAddr[oldEntryIdx].SrcIPv6, ipv6Addr, IPv6_ADR_LEN);
    }
}

void updateTeredoPeerEntry(unsigned char* p, char cmpNonce, unsigned port, unsigned int ip)
{
    IPv6Hdr	*ipv6Hdr;
    IPHdr *iphdr;
    UDPHdr *uh;
    int i = 0;

    ipv6Hdr = (IPv6Hdr*) (p - IPv6_HLEN);
    uh = (UDPHdr*) (p - IPv6_HLEN - UDP_HDR_SIZE);
    iphdr = (IPHdr*) (p - IPv6_HLEN - UDP_HDR_SIZE - IP_HLEN);

    for (i = 0; i < TEREDO_ENTRY_NUM; i++)
    {
        if (memcmp(&TeredoPeerAddr[i].SrcIPv6, ipv6Hdr->src, IPv6_ADR_LEN) == 0)
        {
            if (cmpNonce)
            {
                if (memcmp(p + sizeof(ICMPHdr), &TeredoPeerAddr[i].cv, sizeof(int)) == 0)
                {
                    TeredoPeerAddr[i].time = 1;
                    TeredoPeerAddr[i].trust = 1;
                    uh->srcPort = ntohs(uh->srcPort);
                    memcpy(&TeredoPeerAddr[i].MappedPort, &uh->srcPort, sizeof(TeredoPeerAddr[i].MappedPort));
                    memcpy(&TeredoPeerAddr[i].MappedIP, &iphdr->src.addr, IPv4_ADR_LEN);
                    TeredoNewTrust = 1;
                }
            }
            else
            {
                port = port ^ 0xffff;
                ip = ip ^ 0xffffffff;
                TeredoPeerAddr[i].time = 1;
                TeredoPeerAddr[i].trust = 1;
                port = ntohs(port);
                memcpy(&TeredoPeerAddr[i].MappedPort, &port, sizeof(TeredoPeerAddr[i].MappedPort));
                memcpy(&TeredoPeerAddr[i].MappedIP, &ip, IPv4_ADR_LEN);
                TeredoNewTrust = 1;
            }
            break;
        }
    }
}

void doTeredoConnTest(unsigned char *ipv6Addr)
{
    int nonce = rand();

    createTeredoAddrEntry(ipv6Addr, nonce);
    icmpv6EchoRequest(ipv6Addr, &nonce);
}

void teredoTxSrv(void* data)
{
    unsigned char err;

    while (1)
    {
        OSSemPend(TeredoTxSem, 0, &err);
        teredoDequeue();
    }
}

void teredoRxSrv(void* data)
{
    PKT* rxPkt;
    int status = 0;
    int time = 0;
    IPv6Hdr* ipv6Hdr;
    ICMPv6Hdr* icmpv6Hdr;
    unsigned short originPort;
    unsigned int originIP;
#if CONFIG_TEREDO_WAKEUP_ENABLED
    char rmcptemp[8] = {0x00, 0x00, 0x11, 0xbe, 0x00, 0x00, 0x00, 0x00};
#endif

    while (TeredoState == TEREDO_INIT)
    {
        OSTimeDly(OS_TICKS_PER_SEC * 3);
    }

    while (1)
    {
        do
        {
            teredoTxPendDequeue();
            teredoDequeue();
            rxPkt = rtSktRx(TeredoSkt, TEREDO_PKT_RX_TIMEOUT, &status);
            time++;

            if (((TEREDO_PKT_RX_TIMEOUT*time) > TEREDO_MAINTAN_TIME) && (TeredoState == TEREDO_QUAILIFIED))
            {
                teredoRS();
                time = 0;
            }
        }while (rxPkt == 0);

        //All socket packet will be queued
        rxPkt->queued = 0;

        originPort = 0;
        originIP = 0;

        if (TeredoState == TEREDO_START)
        {
            if (teredoRA(rxPkt) != TEREDO_FAIL)
            {
                TeredoState = TEREDO_QUAILIFIED;
            }
        }
        else
        {
            do
            {
                if ((rxPkt->wp[0]&0xf0) == 0x60)
                {
                    ipv6Hdr = (IPv6Hdr *)rxPkt->wp;
                    if (ipv6Hdr->nexthdr == IP_PROTO_NONHDR)
                    {
#if CONFIG_TEREDO_WAKEUP_ENABLED
                        //power up
                        memcpy(smbiosrmcpdataptr->bootopt, rmcptemp, sizeof(rmcptemp));
                        smbiosrmcpdataptr->bootopt[4] = 0x00;
                        SMBus_Prepare_RmtCtrl(RMCP_PWR_On, 0);
#endif
                        if (originPort && originIP)
                        {
                            //restricted cone
                            if ((ipv6Hdr->src[0] == 0x20) &&(ipv6Hdr->src[1] == 0x01))
                            {
                                createTeredoAddrEntry(ipv6Hdr->src, 0);
                                updateTeredoPeerEntry(rxPkt->wp + IPv6_HLEN , 0, originPort, originIP);
                            }
                            sendTeredoBubblePkt(TeredoSkt, ipv6Hdr->src, originPort, originIP);
                        }
                        else
                        {
                            if ((ipv6Hdr->src[0] == 0x20) &&(ipv6Hdr->src[1] == 0x01))
                            {
                                memcpy(&originPort , ipv6Hdr->src + 10, 2);
                                memcpy(&originIP , ipv6Hdr->src + 12, IPv4_ADR_LEN);
                                updateTeredoPeerEntry(rxPkt->wp + IPv6_HLEN , 0, originPort, originIP);
                                sendTeredoBubblePkt(TeredoSkt, ipv6Hdr->src, originPort, originIP);
                            }
                        }
                    }
                    else if (ipv6Hdr->nexthdr == IP_PROTO_ICMPv6)
                    {
                        //Do not care RA
                        icmpv6Hdr = (ICMPv6Hdr*)(rxPkt->wp + IPv6_HLEN);
                        if (icmpv6Hdr->type == ROUTERADVERTISEMENT)
                        {
                            printf("RA reply\n");
                            break;
                        }
                        else
                        {
                            rxPkt->wp = rxPkt->wp + IPv6_HLEN;
                            icmpv6Input(rxPkt->wp, ntohs(ipv6Hdr->len));
                        }
                    }
                    else
                    {
                        printf("IPv6 packet\n");
                        ipInput_F(rxPkt);
                    }
                    break;
                }
                else if (rxPkt->wp[0] == 0)
                {
                    if (rxPkt->wp[1] == TEREDO_AUTH_TYPE)
                    {
                        if (chkTeredoAuthHdr(rxPkt) == TEREDO_FAIL)
                        {
                            break;
                        }
                    }
                    else if (rxPkt->wp[1] == TEREDO_ORIGIN_TYPE)
                    {
                        if (chkTeredoIndicator(rxPkt, &originPort, &originIP) == TEREDO_FAIL)
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }while (rxPkt->len);
        }
        if (!rxPkt->queued)
        {
            freePkt(rxPkt);
        }
    }
}

int teredoSecWOL(void *pData, int len, struct tcp_pcb *pcb)
{
    md5_context ctx;
    unsigned char secStr[16];

    md5_starts(&ctx);
    md5_updateH(&ctx, "admin", 5);
    md5_finishH(&ctx, secStr);

    if (memcmp(pData, secStr, 16) == 0)
    {
        bsp_bits_set(IO_CONFIG, 1, BIT_OOB_LANWAKE, 1);
        bsp_bits_set(IO_CONFIG, 0, BIT_OOB_LANWAKE, 1);
    }
    return 0;
}

void initTeredo()
{
    unsigned char ip[IPv4_ADR_LEN] = {0};
    char times = 0;
    RTSkt* teredoAuthSkt = 0;

    OS_TCB pdata;

    TeredoState = TEREDO_INIT;

#if CONFIG_TEREDO_SEC_WOL_ENABLED
    teredoAuthSkt = rtSkt(IPv6, IP_PROTO_UDP);
    teredoAuthSkt->pcb->apRx = teredoSecWOL;
    rtSktUDPBind_F(teredoAuthSkt, 0x8618);
    bsp_bits_set(IO_CONFIG, 1, BIT_LANWAKE_IB_DIS, 1);
#endif

    TeredoSkt = rtSkt(IPv4, IP_PROTO_UDP);

    memcpy(dpconf->teredoSrv, "teredo-debian.remlab.net", strlen("teredo-debian.remlab.net"));

    while (ip[0] == 0)
    {
        while (rtGethostbyname_F(IPv4, dpconf->teredoSrv, ip) == 0)
        {
        }
    }

    memcpy(&TeredoAddrInfo.destIPAddr.addr, ip, IPv4_ADR_LEN);
    TeredoAddrInfo.destPort = TEREDO_SRV_PORT;

    rtSktConnect_F(TeredoSkt, ip, TEREDO_SRV_PORT);

    TeredoState = TEREDO_START;



    if (OSTaskQuery(TASK_TEREDO_TX_SRV_PRIO, &pdata) == OS_ERR_NONE)
    {
        memset(TeredoPeerAddr, 0, sizeof(TeredoEntry)*TEREDO_ENTRY_NUM);
    }
    else
    {
        TeredoTxQueueList = 0;
        TeredoPendQueueList = 0;
        CurTeredoPendQueue = 0;
        CurTeredoTxQueue = 0;
        TeredoNewTrust = 0;
        TeredoEntrySem = OSSemCreate(1);
        TeredoTxSem = OSSemCreate(1);

        OSTaskCreateExt (teredoTxSrv,
                         (void *) 0,
                         (OS_STK *) & TeredoTxSrvStk[TASK_TEREDO_TX_SRV_STK_SIZE - 1],
                         TASK_TEREDO_TX_SRV_PRIO,
                         TASK_TEREDO_TX_SRV_PRIO,
                         (OS_STK *) & TeredoTxSrvStk[0],
                         TASK_TEREDO_TX_SRV_STK_SIZE,
                         (void *) 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    }

    do
    {
        teredoRS();
        OSTimeDly(TEREDO_PKT_RX_TIMEOUT);
    }while ((times++ < TEREDO_REPEAT_TIMES) && (TeredoState == TEREDO_START));
}
#endif
