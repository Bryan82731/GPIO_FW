#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "bsp.h"
#include "rmcp.h"
#include "sha1.h"
#include "smbus.h"
#include "tcp.h"
#include "ip.h"
#include "lib.h"


extern smbiosrmcpdata *smbiosrmcpdataptr;
extern asf_config_t *asfconfig;
extern DPCONF *dpconf;

RMCPCB *rmcpcb = NULL;
RMCPCB *rmcpcbarr[3];
//[1] for administrator and [0] for operator
//[2] for ASF 1.0

UDPAddrInfo AsfInfo;
static INT8U  SIK[SIK_KEY_SIZE];
static INT8U Remote_Control = 0;

static const INT8U Status[]=
{
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f
};

//static const INT8U RMCP_IANA[] = {0x00, 0x00 , 0x11, 0xBE};

#if 1
static INT8U KeyA[SIK_KEY_SIZE]= {0x10,0x67,0xb4,0xd0,0xd5,0x2b,0xc5,0x0c,0x3a,0xaf,0x66,0x69,0xb7,0x34,0x45,0x99,0x28,0xf4,0xf9,0xcc};

static INT8U KeyO[SIK_KEY_SIZE]= {0x98,0x73,0xf0,0x5b,0x1c,0x67,0xcc,0x50,0xd1,0x1e,0x2b,0xb1,0x10,0x0d,0xbb,0xc3,0x8d,0xd7,0x08,0xe2};

static INT8U KeyG[SIK_KEY_SIZE]= {0xde,0xef,0x16,0x3b,0xfb,0x8c,0x30,0x50,0x6b,0x3b,0xf1,0x13,0x77,0x35,0x4d,0x9c,0x55,0xbd,0xeb,0x87};
#else
static INT8U KeyA[SIK_KEY_SIZE]= {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41};

static INT8U KeyO[SIK_KEY_SIZE]= {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41};

static INT8U KeyG[SIK_KEY_SIZE]= {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41};
#endif

static INT8U Slide[WindowSIZE] = {0};

INT8U  sw_check(INT32U serial, INT8U Reset)
{
    static INT32U slidestart = 0;
    static INT32U slideindex;
    INT32U index;

    if (Reset == 1 )
    {
        slidestart = 0;
        slideindex = 0;
        for (index = 0 ; index < WindowSIZE; index++)
            Slide[index] = 0;
    }

    //sliding window
    else {
        if (slidestart == 0 && Slide[0] == 0) //initial condition
        {
            slidestart = serial;
            slideindex = 0;
        }

        index = serial - slidestart;

        if (index < 0 || index > 31)
            return 1; //error

        if (Slide[(index+slideindex)%WindowSIZE] == 1)
            return 1; //duplicate message
        else
            Slide[(slideindex+index)%WindowSIZE] = 1;

        if (Slide[slideindex] == 1) //slide the window
        {
            for (index = 0; index < WindowSIZE ; index++)
            {
                if (Slide[(slideindex+index)%WindowSIZE] == 1)
                    Slide[(slideindex+index)%WindowSIZE] = 0;
                else if (Slide[(slideindex+index)%WindowSIZE] == 0) //find a non-zero start
                {
                    slideindex = (slideindex+index)% WindowSIZE;
                    slidestart += index;
                    break;
                }
            }
        }

    }
    return 0;
}

void rmcpackSend(RTSkt *s)
{

    RMCPHdr    *outPkt;
    RSPHdr     *rsphdr = NULL;
    RSPTrailer *rsptrailer;
    PKT* pkt;

    pkt = (PKT *)  allocUDPPkt_F(128, IPv4);
    if (pkt == NULL)
        return;
    pkt->len = 0;

    outPkt = (RMCPHdr *) pkt->wp;

    if (rmcpcb->version == 2)
    {
        rsphdr = (RSPHdr *) outPkt;
        memcpy(rsphdr,rmcpcb->rsp_hdr,sizeof(RSPHdr));
        if (rmcpcb->state && rmcpcb->rmcp_data->MsgType != RMCP_RAKP_M3)
        {
            rmcpcb->session_seq++;
            rsphdr->seq_num = htonl(rmcpcb->session_seq);
        }
        pkt->len += sizeof(RSPHdr);
        outPkt = (RMCPHdr *) ((INT8U *) outPkt + sizeof(RSPHdr));
    }

    memcpy(outPkt, rmcpcb->rmcp_hdr, sizeof(RMCPHdr));
    outPkt->com |= 0x80;

    pkt->len += sizeof(RMCPHdr);

    //add RSP Trailer, only when a session has been created
    if (rmcpcb->state == 1 && rmcpcb->rmcp_data->MsgType != RMCP_RAKP_M3 && rsphdr)
    {
        rsptrailer = (RSPTrailer *)(pkt->wp + pkt->len);
        rsptrailer->padding     = 0x0000;
        rsptrailer->pad_len     = 0x02;
        rsptrailer->next_header = 0x06;
        pkt->len += sizeof(RSPTrailer);
        sha1_hmac(SIK, SIK_KEY_SIZE, (INT8U *) rsphdr ,pkt->len-SIK_KEY_SIZE_USE,rsptrailer->integrity);
    }
    rtSktUDPSendTo_F(s, pkt, &AsfInfo);

}
void RMCP_Response(RTSkt *s)
{

    INT16U sha_len = 0;
    //INT8U PreData[128] ;
    INT8U *PreData = malloc(128);
    //INT8U RandomSeed[SIK_KEY_SIZE] = {0};
    INT8U *RandomSeed = malloc(SIK_KEY_SIZE);
    INT8U index = 0;

    RMCPHdr    *outPkt;
    RSPHdr     *rsphdr = NULL;
    RMCPData   *rmcpdata;
    RSPTrailer *rsptrailer;
    INT8U      *vardata;
    PKT *pkt;

    memset(RandomSeed, 0, SIK_KEY_SIZE);

    pkt = (PKT *) allocUDPPkt_F(128, IPv4);
    if (pkt == NULL)
    {
        free(RandomSeed);
        free(PreData);
        return;
    }
    pkt->len = 0;
    outPkt = (RMCPHdr *) pkt->wp;

    if (rmcpcb->version == 2)
    {
        rsphdr = (RSPHdr *) outPkt;
        memcpy(rsphdr,rmcpcb->rsp_hdr, sizeof(RSPHdr));
        if (rmcpcb->state)
        {
            rmcpcb->session_seq++;
            rsphdr->seq_num = htonl(rmcpcb->session_seq);
        }
        pkt->len += sizeof(RSPHdr);
        outPkt = (RMCPHdr *) ((INT8U *) outPkt + sizeof(RSPHdr));
    }

    //copy rmcp header from the received packet
    memcpy(outPkt, rmcpcb->rmcp_hdr, sizeof(RMCPHdr));
    if (outPkt->seq_num != 0xff)
        outPkt->seq_num++;
    pkt->len += sizeof(RMCPHdr);

    rmcpdata = (RMCPData *) ((INT8U *) outPkt + sizeof(RMCPHdr));

    memcpy(rmcpdata, rmcpcb->rmcp_data, sizeof(RMCPData));
    pkt->len += sizeof(RMCPData);
    vardata = (INT8U *) (rmcpdata ) + sizeof(RMCPData);

    switch (rmcpcb->rmcp_data->MsgType)
    {
    case RMCP_PING:
        rmcpdata->Data_Length = 0x10;
        rmcpdata->MsgType = RMCP_PONG;
        memset(vardata, 0 , 4);    //IANA
        memset(vardata+4, 0, 4);   //OEM
        *(vardata+8) = 0x01;       //ASF Version 1.0
        if (outPkt->seq_num != 0xff)
            *(vardata+9) = 0x80;       //Supported Inteartions
        else
            *(vardata+9) = 0xA0;
        //DASH should be 0xA0
        //memset(vardata+10, 0 , 6);
        *(vardata+10) = (dpconf->fwMajorVer << 4) + (dpconf->fwMinorVer);
        *(vardata+11) = dpconf->fwExtraVer;
        *((INT32U *)(vardata+12)) = dpconf->fwBuildVer;

        break;

    case RMCP_OPS_REQ:
        //keep needed field
        memcpy(rmcpcb->MgtSID, rmcpcb->var_data, sizeof(rmcpcb->MgtSID));

        rmcpdata->Data_Length = 0x1C;
        rmcpdata->MsgType = RMCP_OPS_RES;

        //should check status, if error only return the first 8 bytes
        //if(rmcp->status)

        //RMCP Data content
        *vardata = rmcpcb->status;
        memset(vardata+1, 0, 3);     //reserved
        memcpy(vardata+4, rmcpcb->MgtSID, sizeof(rmcpcb->MgtSID));
        *(INT32U *) (vardata+8) = htonl(rmcpcb->client_sid);
        memcpy(vardata+12,rmcpcb->var_data+4, 16);
        //Authentication Payload, Integrity Payload

        break;

    case RMCP_RAKP_M1:
        if (rmcpcb->role == ASF_Operator)
            rmcpcb->key = KeyO;
        else
            rmcpcb->key = KeyA;

        rmcpdata->Data_Length = 0x38;
        rmcpdata->MsgType = RMCP_RAKP_M2;

        //RMCP Data content
        *vardata = rmcpcb->status;
        memset(vardata+1, 0, 3);     //reserved
        memcpy(vardata+4, rmcpcb->MgtSID, sizeof(rmcpcb->MgtSID));


        //Client Random number
        srand(time(0));
        while (index < SIK_KEY_SIZE)
        {
            *(INT32S *) (RandomSeed + index) = rand();
            index += 4;
        }
        memset(PreData, 0 , 8);
        sha1_hmac(RandomSeed, SIK_KEY_SIZE , PreData, 8, vardata+8);
        memcpy(rmcpcb->CltRandom, vardata+8, 16);

        //Client Global UID, should from SMBIOS implementation
        //memset(vardata+24, 0xff, 16);
        getGUID(vardata+24);

        //Generate Integrity Check Value
        memcpy(PreData, rmcpcb->MgtSID, sizeof(rmcpcb->MgtSID));
        //memcpy(PreData+4, &rmcp->client_sid, sizeof(rmcp->client_sid));
        *(INT32U *) (PreData+4) = htonl(rmcpcb->client_sid);
        memcpy(PreData+8,  rmcpcb->var_data+4 , sizeof(rmcpcb->MgtRandom));
        memcpy(PreData+24, rmcpcb->CltRandom ,  sizeof(rmcpcb->CltRandom));

        getGUID(PreData+40);
        //memcpy(PreData+40, RTGUID, sizeof(RTGUID));

        PreData[56] = rmcpcb->role;
        PreData[57] = rmcpcb->name_length;
        memcpy(&PreData[58], rmcpcb->username, rmcpcb->name_length);


        sha_len = 58 + rmcpcb->name_length;
        sha1_hmac(rmcpcb->key, SIK_KEY_SIZE, PreData,sha_len,vardata+40);
        break;

    case RMCP_CAP_REQ:
        rmcpdata->Data_Length = 0x10;
        rmcpdata->MsgType = RMCP_CAP_RES;
        memset(vardata, 0 , 4);    //IANA
        memset(vardata+4, 0, 4);   //OEM
        memcpy(vardata+8, &asfconfig->RMCPCap[4], 3);
        ///*(vardata+10) = asfconfig->RMCPCap[6];
        memcpy(vardata+11, &asfconfig->RMCPCap[0], 4);
        *(vardata+15) = 0;
#if 0
        *(vardata+8) = 0x00;       //special command
        *(vardata+9)  = 0x1f;
        *(vardata+10) = 0xff;        //System Capabilities Supported
        *(vardata+11) = 0x67;        //System Firmware Capabilities
        *(vardata+12) = 0xF8;
        memset(vardata+13, 0 ,3);
#endif

        break;

    case RMCP_SST_REQ:
        rmcpdata->Data_Length = 0x04;
        rmcpdata->MsgType = RMCP_SST_RES;
        *vardata     = bsp_get_sstate();
        //smbiosrmcpdataptr->sstate; //default is unknow state
        *(vardata+1) = smbiosrmcpdataptr->expired; //watchdog timeout
        memset(vardata+2, 0 , 2);

        break;

    case RMCP_CLS_REQ:
        rmcpdata->Data_Length = 0x01;
        rmcpdata->MsgType = RMCP_CLS_RES;
        *vardata = rmcpcb->status;
        *(vardata+1) = 0; //padding
        if (Remote_Control == 2)
            Remote_Control = 1;
        break;

    default:
        break;
    }

    pkt->len += rmcpdata->Data_Length;

    //add RSP Trailer, only when a session has been created
    if (rmcpcb->state == 1 && rsphdr)
    {
        if (rmcpcb->rmcp_data->MsgType == RMCP_CLS_REQ )
            pkt->len--;  //alignment issue

        rsptrailer = (RSPTrailer *)(pkt->wp + pkt->len);
        if (rmcpcb->rmcp_data->MsgType == RMCP_CLS_REQ )
        {
            rsptrailer->pad_len     = 0x01;
            rmcpcb->state = 0;
        }
        else
        {
            rsptrailer->padding     = 0x0000;
            rsptrailer->pad_len     = 0x02;
        }
        rsptrailer->next_header = 0x06;
        pkt->len += sizeof(RSPTrailer);
        sha1_hmac(SIK, SIK_KEY_SIZE, (INT8U *) rsphdr ,pkt->len-SIK_KEY_SIZE_USE,rsptrailer->integrity);
    }

    //pkt->len = len;
    free(RandomSeed);
    free(PreData);
    rtSktUDPSendTo_F(s, pkt, &AsfInfo);

}

INT8U *strip_rsp(RMCPCB *rmcp, RSPHdr *rsphdr)
{
    rmcp->session_seq = htonl(rsphdr->seq_num);

    if (rmcp->state && sw_check(rmcp->session_seq,0))
        return NULL;

    rmcp->rsp_hdr = rsphdr;
    return ((INT8U*) rsphdr + sizeof(RSPHdr));
}

INT8U check_rmcp_hdr(RMCPCB *rmcp)
{
    RMCPHdr *rmcphdr = rmcp->rmcp_hdr;

    if (rmcphdr->version != RMCP_HdrVersion)
        return 1;

    if (rmcphdr->seq_num  == 0xff && rmcphdr->com != RMCP_ClassofACK)
        rmcp->type = RMCP_SEND_RES;

    //update the sliding window index for version 2.0
    if (rmcphdr->com == RMCP_ClassofACK)
    {
        return 1; //bypass ACK
    }
    else if (rmcphdr->com != RMCP_ClassofMessage)
        return 1;

    return 0;
}

INT8U check_rmcp_data(RMCPCB *rmcp)
{
    RMCPData *rmcpdata = rmcp->rmcp_data;
    INT8U *vardata = rmcp->var_data;
    //INT8U PreData[128], SHAData[SIK_KEY_SIZE], sha_len;
    INT8U sha_len;
    INT8U *PreData = malloc(128);
    INT8U *SHAData = malloc(SIK_KEY_SIZE);
    //INT8U loop_count = 0;

    //temp mark for AMD DMC
    /*
    if (memcmp(rmcpdata->IANA, RMCP_IANA, sizeof(RMCP_IANA)))
    {
        free(PreData);
        free(SHAData);
        return 1;
    }
    */

    if (rmcp->version == 1)
    {
        if (rmcpdata->Data_Length != 0)
        {
            free(PreData);
            free(SHAData);
            return 1;
        }

    }
    else
    {
        if (rmcpdata->MsgType == RMCP_OPS_REQ)
        {
            rmcp->status = Status[0];
            if (*(vardata+4) != Payload_Type_AA || *(vardata+7) != 0x08 || *(vardata+8) != 0x01 || *(vardata+12) != Payload_Type_IA || *(vardata+15) != 0x08 || *(vardata+16) != 0x01 || *(vardata+19) != 0x00)
            {
                free(PreData);
                free(SHAData);
                return 4; //error
            }
        }
        else if (rmcpdata->MsgType == RMCP_RAKP_M1)
        {
            if (htonl(*(INT32U *) vardata) != rmcp->client_sid)
                rmcp->status = Status[3];
            else if (*(vardata+20) > ASF_Administrator)
                rmcp->status = Status[10];
            else if (*(vardata+20) != ASF_Administrator && *(vardata+20) != ASF_Operator)
                rmcp->status = Status[11];
            else
                rmcp->status = Status[0];

            //keep needed field
            memcpy(rmcp->MgtRandom,vardata+4, sizeof(rmcp->MgtRandom));
            rmcp->role = *(vardata + 20);           //User Role
            rmcp->name_length = *(vardata + 23);    //User Name Length
            memcpy(rmcp->username, vardata+24, rmcp->name_length);
            if (rmcp->name_length > 16)
                rmcp->name_length = 16;

            if (rmcp->role == ASF_Operator)
                rmcp->key = KeyO;
            else
                rmcp->key = KeyA;

        }
        else if (rmcpdata->MsgType == RMCP_RAKP_M3)
        {
            rmcp->type   = RMCP_SEND_ACK;
            rmcp->state  = 1;
            rmcp->status = *vardata;

            sw_check(0,1); //Reset sliding window index

            if (htonl(*((INT32U *) vardata + 1)) != rmcp->client_sid)
                rmcp->status = Status[3];

            //resonable range  10 > statu > 0
            //if(rmcp->status > 0x10 || addrinfo.client_sid != CltSID)
            //    return 1;

            memcpy(PreData, rmcp->CltRandom, sizeof(rmcp->CltRandom));
            memcpy(PreData+16, rmcp->MgtSID, sizeof(rmcp->MgtSID));
            PreData[20] = rmcp->role;
            PreData[21] = rmcp->name_length;
            memcpy(PreData+22, rmcp->username, rmcp->name_length);
            sha_len = 22 + rmcp->name_length;

            sha1_hmac(rmcp->key,SIK_KEY_SIZE,PreData,sha_len,SHAData);
            if (memcmp(rmcp->var_data+8, SHAData, SIK_KEY_SIZE_USE))
            {
                free(PreData);
                free(SHAData);
                return 1; //error
            }

            //SIK generation
            memcpy(PreData, rmcp->MgtRandom, sizeof(rmcp->MgtRandom));
            memcpy(PreData+16, rmcp->CltRandom, sizeof(rmcp->CltRandom));
            PreData[32] = rmcp->role;
            PreData[33] = rmcp->name_length;
            memcpy(PreData+34, rmcp->username, rmcp->name_length);
            sha_len = 34 + rmcp->name_length;
            sha1_hmac(KeyG,SIK_KEY_SIZE,PreData,sha_len,SHAData);
            memcpy(SIK, SHAData, sizeof(SIK));

        }
        else if (rmcp->state && (rmcpdata->MsgType & 0x10) == 0x10)
        {
            rmcp->type = RMCP_SEND_ACK;
            if (rmcpdata->Data_Length >0)
                memcpy(smbiosrmcpdataptr->bootopt,vardata,sizeof(smbiosrmcpdataptr->bootopt));
            else
                memset(smbiosrmcpdataptr->bootopt,0 , sizeof(smbiosrmcpdataptr->bootopt));

            smbiosrmcpdataptr->MsgType = rmcpdata->MsgType;

#if 0
            while (((REG8(SMBUS_BASE_ADDR+SEIO_Status)&0x01) != 0x00) && (loop_count < 2)) //0 idle
            {
                OSTimeDly(1);
                loop_count++;
            }
#endif

            //SMBus_Prepare_RmtCtrl(smbiosrmcpdataptr->MsgType, 0);
            if (!memcmp(vardata+4,"DPVEMP", 6))
                Remote_Control = 2;
            else
                Remote_Control = 1;
        }
        /*
        else
        {
           rmcp->type = RMCP_SEND_NONE;
        }
        */


    }

    free(PreData);
    free(SHAData);
    return 0;

}

void rmcpInput(RTSkt *s, PKT *rxPkt)
{

    getUDPAddressInfo_F(rxPkt, &AsfInfo);

    if (rmcpcb == NULL)
    {
        rmcpcb = (RMCPCB *) malloc(sizeof(RMCPCB));
        memset(rmcpcb, 0 , sizeof(RMCPCB));
        rmcpcb->client_sid = 0x01;
    }

    if (s->pcb->localPort == RMCP_V1_PORT)
        rmcpcb->version = 1;
    else
        rmcpcb->version = 2;

    rmcpcb->type = (RMCP_SEND_ACK | RMCP_SEND_RES);
    //default to send both ack and response

    //rmcp version 1 packet comes in the opened rmcp v2 session
    /*
    if (rmcpcb->version == 1 && rmcpcb->state)
        return ;
    */

    if (rmcpcb->version == 2)
        rxPkt->wp = strip_rsp(rmcpcb,(RSPHdr *) rxPkt->wp);
    //strip the RSP header

    //set the pointers
    rmcpcb->rmcp_hdr  = (RMCPHdr *) rxPkt->wp;
    rmcpcb->rmcp_data = (RMCPData *) (rxPkt->wp + sizeof(RMCPHdr));
    rmcpcb->var_data  = (INT8U *) rmcpcb->rmcp_data + sizeof(RMCPData);

    if (check_rmcp_hdr(rmcpcb))
        return ;

    if (check_rmcp_data(rmcpcb))
        return ;

    if ((rmcpcb->type & RMCP_SEND_ACK) == RMCP_SEND_ACK)
    {
        //rmcpackSend(s);
        rmcpackSend(s);
    }

    if ((rmcpcb->type & RMCP_SEND_RES) == RMCP_SEND_RES)
    {
        RMCP_Response(s);
    }

    //do the remote control at the last to prevent sending packet fail
    //during the loop back
    if (Remote_Control == 1)
    {

        Remote_Control = 0;
        SMBus_Prepare_RmtCtrl(smbiosrmcpdataptr->MsgType, 0);
    }
}
void ASFV1Task(void *p_arg)
{
    (void) p_arg;

    //rtSktEnableRMCPPortFilter();

    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_UDP);

#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F(s);
#endif

    rtSktUDPBind_F(s, 623);
    PKT *rxPkt;
    INT32U status = 0;

    while (OS_TRUE)
    {
        do {
            rxPkt = rtSktRx_F(s, 0, &status);
        } while (rxPkt == 0);

        rmcpInput(s,rxPkt);
        freePkt_F(rxPkt);

    }
}

void ASFV2Task(void *p_arg)
{
    (void) p_arg;
    //rtSktEnableRMCPPortFilter();

    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_UDP);

#if CONFIG_VERSION >= IC_VERSION_DP_RevF
    rtSktEnableIBPortFilter_F(s);
#endif

    rtSktUDPBind_F(s, 664);
    PKT *rxPkt;
    INT32U status = 0;

    while (OS_TRUE)
    {
        do {
            rxPkt = rtSktRx_F(s, 0, &status);
        } while (rxPkt == 0);

        rmcpInput(s,rxPkt);
        freePkt_F(rxPkt);

    }

}

