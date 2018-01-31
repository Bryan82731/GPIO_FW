#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "bsp.h"
#include "rtskt.h"
#include "ssh.h"
#include "sha1.h"
#include "app_cfg.h"

#if  defined (CONFIG_TELNET_ENABLED)  || defined (CONFIG_SSH_ENABLED) || defined(CONFIG_TCR_ENABLED)

#define mallocx(x) malloc(x)
#define freex(x) free(x)

const unsigned int dh_g_val = 2;
const unsigned char CRLF[] = "\r\n";
unsigned char NewSSHCli = 0;
extern DPCONF *dpconf;

static const int DH_G_VAL = 2;
int debugssh = 0;

#define SSH_PROTOVER "SSH-2.0-Realtek\r\n"
#define SSHNameListNum 8
#define SSH_COOKIE_LEN 16

const char SSHKexAlog[] = {"diffie-hellman-group1-sha1"};
const char SSHSrvHostKeyAlog[] = {"ssh-rsa"};
const char SSHEnAlog[] = {"aes128-cbc"};
const char SSHMacAlogSrv2Cli[] = {"hmac-sha1"};
const char SSHCompressAlog[]= {"none"};
const char* SSHNameList[SSHNameListNum]={SSHKexAlog, SSHSrvHostKeyAlog,SSHEnAlog,SSHEnAlog,SSHMacAlogSrv2Cli,SSHMacAlogSrv2Cli,SSHCompressAlog,SSHCompressAlog};

extern OS_STK  TelnetClientStk[TASK_TELNET_CLIENT_STK_SIZE];
extern unsigned int getShellCmd(TelnetSession* session, unsigned char* data, unsigned int offset, unsigned int len);
extern void parseCmdBufffer(TelnetSession* session);
extern void flushTelnetSendBuffer(TelnetSession* session);
extern void sendShellTag(TelnetSession* session);
extern void putSendBuf(TelnetSession* session, unsigned char data);



/* diffie-hellman-group1-sha1 values for g and p */
const unsigned char dh_p_val[] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2,
    0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
    0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6,
    0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
    0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D,
    0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
    0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9,
    0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
    0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11,
    0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE6, 0x53, 0x81,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

#define DH_P_LEN 128

/* initialise an already allocated buffer. The data won't be freed before
 * malloc */
void buf_init(buffer* buf, unsigned int size)
{
    if (size > 0)
    {
        buf->data = (unsigned char*)malloc(size);
    }
    else
    {
        buf->data = NULL;
    }

    buf->len = 0;
}

/* free a buffer's data */
void buf_clear(buffer* buf)
{
    free(buf->data);
}

buffer* buf_new(unsigned int size)
{
    buffer* ret;

    ret = (buffer*)malloc(sizeof(buffer));
    buf_init(ret, size);
    return ret;
}

/* free the buffer's data and the buffer itself */
void buf_free(buffer* buf)
{
    buf_clear(buf);
    free(buf);
}


void setInt(unsigned char *dst, unsigned int Int)
{
    Int= ntohl(Int);
    memcpy(dst, &Int, 4);
}

static void putByte(buffer *buf, unsigned char byte)
{
    memcpy(buf->data+buf->len, &byte, 1);
    buf->len++;
}


static void putString(buffer *buf, unsigned char *str, unsigned int len)
{
    setInt(buf->data+buf->len, len);
    buf->len+=4;
    memcpy(buf->data+buf->len, str, len);
    buf->len+=len;
}

/*
 * Do an RSA operation to sign the message digest
 */
int rsa_pkcs1_sign_sechash( rsa_context *ctx,
                            int mode,
                            int hash_id,
                            int hashlen,
                            unsigned char *hash,
                            unsigned char *sig )
{
    int nb_pad, olen;
    unsigned char *p = sig;

    olen = ctx->len;

    switch ( ctx->padding )
    {
    case RSA_PKCS_V15:

        switch ( hash_id )
        {
        case RSA_RAW:
            nb_pad = olen - 3 - hashlen;
            break;

        case RSA_MD2:
        case RSA_MD4:
        case RSA_MD5:
            nb_pad = olen - 3 - 34;
            break;

        case RSA_SHA1:
            nb_pad = olen - 3 - 35;
            break;

        default:
            return( XYSSL_ERR_RSA_BAD_INPUT_DATA );
        }

        if ( nb_pad < 8 )
            return( XYSSL_ERR_RSA_BAD_INPUT_DATA );

        *p++ = 0;
        *p++ = RSA_SIGN;
        memset( p, 0xFF, nb_pad );
        p += nb_pad;
        *p++ = 0;
        break;

    default:

        return( XYSSL_ERR_RSA_INVALID_PADDING );
    }

    switch ( hash_id )
    {
    case RSA_RAW:
        memcpy( p, hash, hashlen );
        break;

    case RSA_MD2:
        memcpy( p, ASN1_HASH_MDX, 18 );
        memcpy( p + 18, hash, 16 );
        p[13] = 2;
        break;

    case RSA_MD4:
        memcpy( p, ASN1_HASH_MDX, 18 );
        memcpy( p + 18, hash, 16 );
        p[13] = 4;
        break;

    case RSA_MD5:
        memcpy( p, ASN1_HASH_MDX, 18 );
        memcpy( p + 18, hash, 16 );
        p[13] = 5;
        break;

    case RSA_SHA1:
        memcpy( p, ASN1_HASH_SHA1, 15 );
        sha1H(hash, 20, p + 15);
//            memcpy( p + 15, hash, 20 );
        break;

    default:
        return( XYSSL_ERR_RSA_BAD_INPUT_DATA );
    }

    return( ( mode == RSA_PUBLIC )
            ? rsa_public(  ctx, sig, sig )
            : rsa_private( ctx, sig, sig ) );
}

static int putSign(buffer *buf, SSH *ssh, unsigned char *hash)
{
    unsigned char *tmpptr;

    setInt(buf->data+buf->len, 0);
    tmpptr = buf->data+buf->len;
    buf->len+=4;
    putString(buf, SSH_SIGNKEY_RSA, SSH_SIGNKEY_RSA_LEN);
    setInt(buf->data+buf->len, 128);
    buf->len+=4;
    if (rsa_pkcs1_sign_sechash(ssh->dphostkey, RSA_PRIVATE, RSA_SHA1, 20, hash, buf->data+buf->len) != 0)
        return -1;
    buf->len+=128;
    setInt(tmpptr, buf->data+buf->len-tmpptr-4);
    return 0;
}

void sendSrvSSHPtlVerExMsg(SSH *ssh)
{
    PKT* txPkt;

    txPkt = (PKT*)allocPkt_F(strlen(SSH_PROTOVER));
    memcpy(txPkt->wp, SSH_PROTOVER, txPkt->len);

    rtSktSend_F(ssh->cs, txPkt, strlen(SSH_PROTOVER));
}
#ifdef SSH_TEST
unsigned char test_transkex_payload[]=
{
    0x14,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x1a,0x64,0x69,0x66,0x66,0x69,0x65,0x2d,0x68,0x65,0x6c,0x6c,
    0x6d,0x61,0x6e,0x2d,0x67,0x72,0x6f,0x75,0x70,0x31,0x2d,0x73,0x68,0x61,0x31,0x00,
    0x00,0x00,0x07,0x73,0x73,0x68,0x2d,0x72,0x73,0x61,0x00,0x00,0x00,0x0a,0x61,0x65,
    0x73,0x31,0x32,0x38,0x2d,0x63,0x62,0x63,0x00,0x00,0x00,0x0a,0x61,0x65,0x73,0x31,
    0x32,0x38,0x2d,0x63,0x62,0x63,0x00,0x00,0x00,0x12,0x68,0x6d,0x61,0x63,0x2d,0x73,
    0x68,0x61,0x31,0x2c,0x68,0x6d,0x61,0x63,0x2d,0x6d,0x64,0x35,0x00,0x00,0x00,0x12,
    0x68,0x6d,0x61,0x63,0x2d,0x73,0x68,0x61,0x31,0x2c,0x68,0x6d,0x61,0x63,0x2d,0x6d,
    0x64,0x35,0x00,0x00,0x00,0x04,0x6e,0x6f,0x6e,0x65,0x00,0x00,0x00,0x04,0x6e,0x6f,
    0x6e,0x65,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
#endif

void SSHFree(SSH **ssh)
{
    if ((*ssh)->session_id != NULL)
    {
        freex((*ssh)->session_id);
        (*ssh)->session_id = NULL;
    }
    if ((*ssh)->dphostkey != NULL)
    {
        rsa_free( (*ssh)->dphostkey );
        free((*ssh)->dphostkey);
        (*ssh)->dphostkey = NULL;
    }
    if ((*ssh)->keys != NULL)
    {
        freex((*ssh)->keys->recv_IV);
        (*ssh)->keys->recv_IV = NULL;
        freex((*ssh)->keys->trans_IV);
        (*ssh)->keys->trans_IV = NULL;
        freex((*ssh)->keys);
        (*ssh)->keys = NULL;
    }
    if ((*ssh)->username != NULL)
    {
        freex((*ssh)->username);
        (*ssh)->username = NULL;
    }
    if ((*ssh)->chan != NULL)
    {
        freex((*ssh)->chan);
        (*ssh)->chan = NULL;
    }
    if ((*ssh)->rxbuf != NULL)
    {
        freex((*ssh)->rxbuf);
        (*ssh)->rxbuf = NULL;
    }
    free((*ssh));
    (*ssh) = NULL;
}

/* return len bytes of pseudo-random data */
void genrandom(unsigned char* buf, unsigned int len)
{
    unsigned int i;

    for (i=0;i<len;i++)
    {
        buf[i] = rand() % 256;
    }
}

//Server: Key Exchange Init
void srvKeyExInit(SSH *ssh)
{
    char cookie[SSH_COOKIE_LEN];
    char padLen = 0;
    int i = 0, payloadLen = 0;
    PKT* txPkt;
    unsigned char* p;
    SSHTranLayerHdr* hdr;
    int *len;
    int sLen;
#ifndef SSH_TEST
    genrandom(cookie, SSH_COOKIE_LEN);

    i = 5 + 1/*type*/ + sizeof(cookie) + 4 + strlen(SSHKexAlog)
        + 4 + strlen(SSHSrvHostKeyAlog) + 2*(4 + strlen(SSHEnAlog))
        + 2*(4 + strlen(SSHMacAlogSrv2Cli)) + 2*(4 + strlen(SSHCompressAlog)) + 4*2/*lang*/
        + 5/*1:kex flow, 4:Reserved*/;

    payloadLen = i - 5;

    //padding len must multiple of 8 and at least 4 byte
    padLen = 8 - (i%8);
    if (padLen < 4)
        padLen+=8;

    i = i + padLen;

    txPkt = (PKT*)allocPkt_F(i);
    hdr = (SSHTranLayerHdr*)txPkt->wp;
    i = i - 4;
    hdr->length = htonl(i)/*not include length field*/;
    hdr->padLen = padLen;

    p = txPkt->wp + 5;//sizeof(SSHTranLayerHdr) => wiil be 8
    /*Msg Code*/
    *p = SSH_MSG_KEXINIT;
    p = p + 1;
    /*Cookie*/
    memcpy(p, cookie, SSH_COOKIE_LEN);
    p = p + SSH_COOKIE_LEN;

    //Have alignment issue
    for (i = 0 ; i < SSHNameListNum; i++)
    {
        len = (int*)p;
        sLen = strlen(SSHNameList[i]);
        p+=4;
        memcpy(p, SSHNameList[i], sLen);
        p+=sLen;
        sLen = htonl(sLen);
        memcpy(len, &sLen, sizeof(int));
    }

    len = (int*)p;

    //Set lang to 0
    memset(len, 0, sizeof(int)*2);
    p+=8;

    //Set kex flow and Reserved to 0
    memset(p, 0, 5);
    p+=5;

    genrandom(p, padLen);

    ssh->srvkeyinit = buf_new(500);
    /* I_S, the payload of the server's SSH_MSG_KEXINIT */
    memcpy(ssh->srvkeyinit->data, txPkt->wp+5, ntohl(hdr->length) - padLen - 1);
    ssh->srvkeyinit->len = ntohl(hdr->length) - padLen - 1;
//   putString(ssh->srvkeyinit, txPkt->wp+5, ntohl(hdr->length) - padLen - 1);
#else
    padLen = 4;
    txPkt = (PKT*)allocPkt_F(sizeof(test_transkex_payload) + 5 + padLen);

    sLen = htonl(164);
    memcpy(txPkt->wp, &sLen, 4);

    memcpy(txPkt->wp+4, &padLen, 1);

    memcpy(txPkt->wp+5, test_transkex_payload, sizeof(test_transkex_payload));

    *(txPkt->wp+5+sizeof(test_transkex_payload)) = 1;

    for (i=1;i<padLen;i++)
    {
        *(txPkt->wp+5+sizeof(test_transkex_payload) + i) = 0;
    }

    ssh->kexhashbuf->len = sizeof(test_transkex_payload);
    buf_putstring(ssh->kexhashbuf, test_transkex_payload, sizeof(test_transkex_payload) );
#endif
    ssh->transseq++;
    //tcpSendData(pcb, txPkt);
    rtSktSend_F(ssh->cs, txPkt, 0);
}

static int chkCliSSHPtlVer(unsigned char *pktbuf, int pktlen, SSH *ssh)
{
    char *pch;
    char * verstr;
    unsigned char* p = pktbuf;
    if (pktlen < 4)
        return 0;

    verstr = strchr(SSH_PROTOVER, '\r');
    if (strncmp(p, "SSH-2.0", strlen("SSH-2.0")) == 0)
    {
        ssh->kexhashbuf = buf_new(2000);
        pch = strchr(p, '\r'); //may be missing
        if (pch == NULL)
            pch = strchr(p, '\n');
        if (pch)
        {
            /* V_C, the client's version string (CR and NL excluded) */
            putString(ssh->kexhashbuf, p, (unsigned int)pch- (unsigned int)p);
            /* V_S, the server's version string (CR and NL excluded) */
            putString(ssh->kexhashbuf, SSH_PROTOVER, (unsigned int)verstr - (unsigned int)SSH_PROTOVER);
            srvKeyExInit(ssh);
            ssh->state = SRV_SSH_KEY_INIT_EX;
            return 0;
        }
        else
            return -1;
    }
    return -1;
}

static int clientKeyINITEx(unsigned char *pktbuf, int pktlen, SSH *ssh) //Client: Key Exchange Init(20)
{
    unsigned char* p = pktbuf;
    unsigned char* pattern;
    SSHTranLayerHdr* hdr = (SSHTranLayerHdr*)p;
    int *len;
    int sLen;
    int i = 0;
    p = p + PACKET_PAYLOAD_OFF;
    hdr->length = ntohl(hdr->length);
    //buf_putstring(ssh->kexhashbuf, p, hdr->length - hdr->padLen - 1);
    /* I_C, the payload of the client's SSH_MSG_KEXINIT */
    putString(ssh->kexhashbuf, p, hdr->length - hdr->padLen - 1);
    /* I_S, the payload of the server's SSH_MSG_KEXINIT */
    putString(ssh->kexhashbuf, ssh->srvkeyinit->data, ssh->srvkeyinit->len);
    buf_free(ssh->srvkeyinit);
    if (*p != SSH_MSG_KEXINIT)
        return -1;
    p++;

    i = hdr->length - hdr->padLen -1 - 1;

    p+=SSH_COOKIE_LEN;

    for (i = 0 ; i < SSHNameListNum; i++)
    {
        len = (int*)p;
        memcpy(&sLen, len, sizeof(int));
        sLen = ntohl(sLen);
        if (sLen - hdr->length > 0)
            break;
        else
        {
            pattern = strstr(p+4, SSHNameList[i]);//len 0x00000010 will ==>null string
            if (pattern == 0)
                break;
            else
            {
                if (pattern - p > sLen)
                    break;
            }
            p = p + 4 + sLen;
        }
    }

    if (i != SSHNameListNum)
        rtSktClose_F(ssh->cs);
    else
        ssh->state = CLI_SSH_KEY_INIT_EX;

    return 0;
}

/* Reset the kex state, ready for a new negotiation */
int SSHInit(SSH **ssh, RTSkt *cs)
{
    rsa_context *rsa;

    (*ssh) = mallocx(sizeof(SSH));
    memset((*ssh), 0, sizeof(SSH));

    rsa = malloc(sizeof(rsa_context));
    memset( rsa, 0, sizeof( rsa ) );
    rsa_init( rsa, RSA_PKCS_V15, 0, NULL, NULL );
#define MAX_RSA_KEY_SIZE 260
    unsigned char *tmp= malloc(MAX_RSA_KEY_SIZE);

    //Init RSA
    memset( rsa, 0, sizeof( rsa_context ) );
    rsa->len = 128;

    getRSAKey(0, tmp);
    mpi_read_string( &rsa->N , 16, tmp  );
    memset(tmp, 0, MAX_RSA_KEY_SIZE);
    getRSAKey(1, tmp);
    mpi_read_string( &rsa->E , 16, tmp  );
    memset(tmp, 0, MAX_RSA_KEY_SIZE);
    getRSAKey(2, tmp);
    mpi_read_string( &rsa->D , 16, tmp  );
    memset(tmp, 0, MAX_RSA_KEY_SIZE);
    getRSAKey(3, tmp);
    mpi_read_string( &rsa->P , 16, tmp  );
    memset(tmp, 0, MAX_RSA_KEY_SIZE);
    getRSAKey(4, tmp);
    mpi_read_string( &rsa->Q , 16, tmp  );
    memset(tmp, 0, MAX_RSA_KEY_SIZE);
    getRSAKey(5, tmp);
    mpi_read_string( &rsa->DP, 16, tmp );
    memset(tmp, 0, MAX_RSA_KEY_SIZE);
    getRSAKey(6, tmp);
    mpi_read_string( &rsa->DQ, 16, tmp );
    memset(tmp, 0, MAX_RSA_KEY_SIZE);
    getRSAKey(7, tmp);
    mpi_read_string( &rsa->QP, 16, tmp );
    free (tmp);
    if ( rsa_check_pubkey(  rsa ) != 0 ||
            rsa_check_privkey( rsa ) != 0 )
    {
        rsa_free(rsa);
        free(rsa);
        return( -1 );
    }
    (*ssh)->dphostkey = rsa;

    (*ssh)->session_id = NULL;
    (*ssh)->kexhashbuf = NULL;
    (*ssh)->transseq = 0;
    (*ssh)->recvseq = 0;
    (*ssh)->K = NULL;
    // (*ssh)->authtypes |= AUTH_TYPE_PUBKEY;
    (*ssh)->authtypes |= AUTH_TYPE_PASSWORD;
    (*ssh)->transseq = 0;
    (*ssh)->recvseq = 0;
    (*ssh)->macsize = SHA1_HASH_SIZE;
    (*ssh)->keysize = 16;//aes128-cbc
    (*ssh)->cs = cs;
    sendSrvSSHPtlVerExMsg(*ssh);
    (*ssh)->state = SRV_SSH_PROTOCOL_EX;
    return 0;
}

static void send_plain_packet(buffer *writepayload, SSH *ssh)
{
    unsigned char padlen;
    int len = 0;
    PKT* pkt = NULL;

    padlen = 8-((writepayload->len+5)%8);
    if (padlen < 4)
        padlen += 8;

    pkt = (PKT*)allocPkt_F(writepayload->len+5+padlen);

    len = htonl(writepayload->len + padlen + 1);
    memcpy(pkt->wp, &len, 4);
    memcpy(pkt->wp + 4, &padlen, 1);
    memcpy(pkt->wp + 5, writepayload->data, writepayload->len);

    genrandom(pkt->wp + writepayload->len+5, padlen);
    rtSktSend_F(ssh->cs, pkt, 0);
    ssh->transseq++;
}


static void putInt(buffer *buf, mpi *X)
{
    int len, pad;
    len = mpi_msb(X);
    pad = (len%8 == 0) ? 1 : 0;
    len = len/8+1;
    setInt(buf->data+buf->len, len);
    buf->len += 4;
    if (pad)
    {
        memset(buf->data+buf->len, 0, 1);
        buf->len++;
    }
    mpi_write_binary(X, buf->data+buf->len, len-pad);
    buf->len += (len-pad);
}

static void putKey(buffer *buf, rsa_context *dphostkey)
{
    unsigned char *tmpptr;

    tmpptr = buf->data+buf->len;
    setInt(buf->data + buf->len, 0); //temparay
    buf->len+=4;
    putString(buf, SSH_SIGNKEY_RSA, SSH_SIGNKEY_RSA_LEN);
    putInt(buf, &dphostkey->E);
    putInt(buf, &dphostkey->N);
    setInt(tmpptr, buf->data+buf->len-tmpptr-4);
}

static int send_msg_kexdh_reply(SSH *ssh, mpi *e)
{
    //mp_int dh_p, dh_q, dh_g, dh_y, dh_f;
    unsigned char randbuf[DH_P_LEN];
    buffer *writepayload;
    int dh_q_len;

    mpi p, q, g, y, f, rr;
    mpi_init(&p, &q, &g, &y, &f, &rr, NULL);

    mpi_read_binary(&p, (unsigned char*)dh_p_val, DH_P_LEN);

    mpi_lset(&g, dh_g_val);

    /*
     * Signed substraction: X = A - b
    int mpi_sub_int( mpi *X, mpi *A, int b )
     */

    mpi_sub_int(&q, &p, 1);

    mpi_shift_r(&q, 1);

    dh_q_len = mpi_size(&q);

    do
    {
        genrandom(randbuf, dh_q_len);

        mpi_read_binary(&y, randbuf, dh_q_len);
    }while (mpi_cmp_int(&y, 0) <= 0 ||  mpi_cmp_mpi(&y, &q) >= 0);

    /* f = g^y mod p */
    if (mpi_exp_mod( &f, &g, &y, &p, &rr ) != 0)
    {
        return -1;
    }
    mpi_free(&g, NULL);

    mpi_free(&rr, NULL);
    mpi_init(&rr, NULL);
    ssh->K = (mpi*)mallocx(sizeof(mpi));
    mpi_init(ssh->K, NULL);

    /* K = e^y mod p */
    if (mpi_exp_mod( ssh->K, e, &y, &p, &rr ) != 0)
    {
        return -1;
    }
    mpi_free(&y, &p, &q, &rr, NULL);

    /* K_S, the host key */
    putKey(ssh->kexhashbuf, ssh->dphostkey);

    /* e, exchange value sent by the client */
    putInt(ssh->kexhashbuf, e);
    /* f, exchange value sent by the server */
    putInt(ssh->kexhashbuf, &f);
    /* K, the shared secret */
    putInt(ssh->kexhashbuf, ssh->K);
    /* calculate the hash H */
    sha1H(ssh->kexhashbuf->data, ssh->kexhashbuf->len, ssh->hash);

    /* first time around, we set the session_id to H */
    if (!ssh->session_id)
    {
        /* create the session_id, this never needs freeing */
        ssh->session_id = (unsigned char*)malloc(SHA1_HASH_SIZE);
        memcpy(ssh->session_id, ssh->hash, SHA1_HASH_SIZE);
    }

    writepayload = buf_new(ETH_PAYLOAD_LEN);

    putByte(writepayload, SSH_MSG_KEXDH_REPLY);
    putKey(writepayload, ssh->dphostkey);
    putInt(writepayload, &f);

    mpi_free(&f, NULL);
    putSign(writepayload, ssh, ssh->hash);

    /* the SSH_MSG_KEXDH_REPLY is done */
    //buf_free(tmp);
    buf_free(ssh->kexhashbuf);

    send_plain_packet(writepayload, ssh);

    buf_free(writepayload);
    return 0;
}

//Server: New Keys
static void send_msg_newkeys(SSH *ssh)
{
    PKT *pkt = NULL;
    SSHTranLayerHdr *hdr = NULL;
    int len = 0, padlen = 0;
    char msg_code = SSH_MSG_NEWKEYS;

    len = 1; /*SSH_MSG_NEWKEY*/

    padlen = 8-((len+PACKET_PAYLOAD_OFF)%8);
    if (padlen < 4)
        padlen += 8;

    pkt = (PKT*)allocPkt_F(PACKET_PAYLOAD_OFF + len + padlen);

    hdr = (SSHTranLayerHdr*)pkt->wp;

    hdr->length = htonl(1 + len + padlen);
    hdr->padLen = padlen;

    memcpy(pkt->wp + PACKET_PAYLOAD_OFF, &msg_code, 1);

    genrandom(pkt->wp + PACKET_PAYLOAD_OFF + len, padlen);

    ssh->transseq++;

    //tcpSendData(pcb, pkt);
    rtSktSend_F(ssh->cs, pkt ,0);
}

//Client: Diffie-hellman Key Exchange Init(30)
static int recvCliDFKexinit(unsigned char *pktbuf, int pktlen, SSH *ssh)
{
    //mp_init=>multiple-precision integer library
    unsigned char *tmphdr = NULL;
    int len = 0;
    mpi e;
    SSHTranLayerHdr* hdr = (SSHTranLayerHdr*)pktbuf;
    mpi_init(&e, NULL);
    hdr->length = ntohl(hdr->length);
    tmphdr = pktbuf + PACKET_PAYLOAD_OFF;
    if (*tmphdr != SSH_MSG_KEXDH_INIT)
        return -1;

    memcpy(&len, pktbuf + PACKET_PAYLOAD_OFF + 1, 4);
    len = ntohl(len);
    mpi_read_binary(&e, pktbuf + PACKET_PAYLOAD_OFF + 1 + 4, len);
    if (send_msg_kexdh_reply(ssh, &e) !=0 )
    {
        return -1;
    }
    mpi_free(&e, NULL);
    ssh->state = CLI_SSH_NEW_KEY;
    send_msg_newkeys(ssh);
    return 0;
}



static void send_encrypt_packet(PKT *pkt, int len, int type, SSH *ssh, int isdata)
{
    unsigned int totalLen = 0;
    unsigned char padlen = 0;

    pkt->wp = pkt->wp-(PACKET_PAYLOAD_OFF + 1);
    if (type == SSH_MSG_CHANNEL_DATA)
        pkt->wp -= 4;
    totalLen = PACKET_PAYLOAD_OFF + 1 + len;
    if (type == SSH_MSG_CHANNEL_DATA)
        totalLen += 4;
    padlen = 16-((totalLen)%16);
    if (padlen < 4)
        padlen += 16;
    totalLen+=padlen;
    setInt(pkt->wp, totalLen-PACKET_PADDING_OFF);
    memcpy(pkt->wp+4, &padlen, 1);
    *(pkt->wp+PACKET_PAYLOAD_OFF) = type;
    if (type == SSH_MSG_CHANNEL_DATA)
    {
        setInt(pkt->wp + PACKET_PAYLOAD_OFF + 1, ssh->chan->recvChanId);
    }
    genrandom(pkt->wp + totalLen - padlen, padlen);

    if (ssh->macsize > 0)
    {
        sha1_context ctx;
        unsigned char seqbuf[4];
        unsigned char output[20];
        sha1_hmac_startsH( &ctx, ssh->keys->transmackey, ssh->macsize );
        /* sequence number */
        STORE32H(ssh->transseq, seqbuf);
        sha1_hmac_updateH( &ctx, seqbuf, 4 );
        sha1_hmac_updateH( &ctx, pkt->wp, totalLen );
        sha1_hmac_finishH( &ctx, output );

        memcpy(pkt->wp + totalLen, output, ssh->macsize);
    } /* hash */


    aes_crypt_cbc((aes_context*)ssh->keys->trans_algo_crypt, AES_ENCRYPT, totalLen, ssh->keys->trans_IV, pkt->wp, pkt->wp);

    ssh->transseq++;

    rtSktSend_F(ssh->cs, pkt, 0);
}

static void gen_enc_keys(sha1_context *ctx, int outlen, unsigned char *output, unsigned char X, unsigned char *sid)
{
    sha1_context ctx2;
    unsigned char *k2 = NULL;
    memcpy(&ctx2, ctx, sizeof(sha1_context));
    sha1_update(&ctx2, &X, 1);
    sha1_update(&ctx2, sid, SHA1_HASH_SIZE);
    sha1_finish( &ctx2, output );
    if (SHA1_HASH_SIZE < outlen) {
        /* need to extend */
        memcpy(&ctx2, ctx, sizeof(sha1_context));
        sha1_update(&ctx2, output, SHA1_HASH_SIZE);
        k2 = mallocx(SHA1_HASH_SIZE);
        sha1_finish(&ctx2, k2);
        memcpy(&output[SHA1_HASH_SIZE], k2, outlen - SHA1_HASH_SIZE);
    }
}

static void gen_new_keys(SSH *ssh)
{
    unsigned char IV[MAX_IV_LEN];
    unsigned char key[MAX_KEY_LEN];
    //hash_state hs;
    sha1_context ctx;
    buffer * buf;
    unsigned int keysize;

    sha1_starts( &ctx );
    buf = buf_new(512 + 20); /* max buffer is a 4096 bit key,
								plus header + some leeway*/
#if 0
    buf_putmpint(buf, ssh->dh_K);
    mp_clear(ssh->dh_K);
    freex(ssh->dh_K);
#endif

    putInt(buf, ssh->K);
    mpi_free(ssh->K, NULL);
    free(ssh->K);

    sha1_update( &ctx, buf->data, buf->len );
    buf_free(buf);
    sha1_update( &ctx, ssh->hash, SHA1_HASH_SIZE );


    gen_enc_keys(&ctx, SHA1_HASH_SIZE, IV, 'A', ssh->session_id);

    keysize = 16; /*aes128-cbc*/
    gen_enc_keys(&ctx, keysize, key, 'C', ssh->session_id);

    ssh->newkeys = (struct key_context*)mallocx(sizeof(struct key_context));

    memcpy(ssh->newkeys->recv_algo_crypt, key, keysize);

    ssh->newkeys->recv_IV = (unsigned char*)mallocx(MAX_IV_LEN);
    memcpy(ssh->newkeys->recv_IV, IV, MAX_IV_LEN);

    /* server->client IV */
    gen_enc_keys(&ctx, SHA1_HASH_SIZE, IV, 'B', ssh->session_id);

    /* server->client encryption key */
    keysize = 16;

    gen_enc_keys(&ctx, SHA1_HASH_SIZE, key, 'D', ssh->session_id);

//    aes_setkey_enc_sw( &ssh->newkeys->trans_algo_crypt, key, keysize*8 );
    memcpy(ssh->newkeys->trans_algo_crypt, key, keysize);

    ssh->newkeys->trans_IV = (unsigned char*)mallocx(MAX_IV_LEN);

    memcpy(ssh->newkeys->trans_IV, IV, MAX_IV_LEN);

    /* MAC key client->server */
    keysize = 20; /*hmac-sha1*/
    gen_enc_keys(&ctx, SHA1_HASH_SIZE, ssh->newkeys->recvmackey, 'E', ssh->session_id);

    /* MAC key server->client */
    keysize = 20; /*hmac-sha1*/
    gen_enc_keys(&ctx, SHA1_HASH_SIZE, ssh->newkeys->transmackey, 'F', ssh->session_id);

    /* change the keys */
    ssh->keys = ssh->newkeys;
    ssh->newkeys = NULL;
}

static int recvCliNewKey(unsigned char *pktbuf, int pktlen, SSH *ssh)
{
    unsigned char *p;
    p = pktbuf + PACKET_PAYLOAD_OFF;
    if (*p != SSH_MSG_NEWKEYS)
        return -1;

    gen_new_keys(ssh);
    ssh->state = CLI_SSH_SRV_REQ;
    return 0;
}

static int recvCliSrvReq(unsigned char *pktbuf, int pktlen, SSH *ssh)
{
    unsigned char *name, *p;
    unsigned int nameLen;
    PKT *sshPkt;
    int ret = -1;

    p = pktbuf + PACKET_PAYLOAD_OFF;
    if (*p != SSH_MSG_SERVICE_REQUEST)
        return -1;
    p++;
    memcpy(&nameLen, p, 4);
    p+=4;
    nameLen = ntohl(nameLen);
    name = mallocx(nameLen + 1);
    memcpy(name, p, nameLen);
    *(name + nameLen) = '\0';

    if (strcmp(name, SSH_SERVICE_USERAUTH) == 0 ||
            strcmp(name, SSH_SERVICE_CONNECTION) == 0)
    {
        sshPkt = allocSSHPkt_F(ssh, 4+nameLen, 0);
        setInt(sshPkt->wp, nameLen);
        memcpy(sshPkt->wp+4, name, nameLen);
        send_encrypt_packet(sshPkt, 4+nameLen, SSH_MSG_SERVICE_ACCEPT, ssh, 0);
        ssh->state = CLI_SSH_USERAUTH_REQ;
        ret = 0;
        ssh->connecttime = 1;
    }
    else
        ret = -1;
    freex(name);

    return ret;
}

static int send_msg_userauth_failure(SSH *ssh, unsigned char partial, unsigned char incrfail)
{
    unsigned int len = 0;
    PKT *sshPkt;
    unsigned char *p, *tmpbuf = mallocx(30);

    p = tmpbuf + 4;
    if (ssh->authtypes & AUTH_TYPE_PUBKEY)
    {
        len += AUTH_METHOD_PUBKEY_LEN;
        memcpy(p, AUTH_METHOD_PUBKEY, AUTH_METHOD_PUBKEY_LEN);
        p += AUTH_METHOD_PUBKEY_LEN;
        if (ssh->authtypes & AUTH_TYPE_PASSWORD)
        {
            len++;
            memcpy(p, ",", 1);
            p++;
        }
    }

    if (ssh->authtypes & AUTH_TYPE_PASSWORD)
    {
        len += AUTH_METHOD_PASSWORD_LEN;
        memcpy(p, AUTH_METHOD_PASSWORD, AUTH_METHOD_PASSWORD_LEN);
        p+=AUTH_METHOD_PASSWORD_LEN;
    }

    memcpy(p, &partial, 1);

    p++;

    setInt(tmpbuf, len);

    sshPkt = allocSSHPkt_F(ssh, 4/*name list length*/ + len + 1/*partial*/, 0);
    memcpy(sshPkt->wp, tmpbuf, 4+len+1);
    send_encrypt_packet(sshPkt, 4+len+1, SSH_MSG_USERAUTH_FAILURE, ssh, 0);

    free(tmpbuf);
    if (incrfail)
        ssh->failcount++;
    if (ssh->failcount >= 5)
    {
        return -1;
    }
    return 0;
}

static void send_msg_userauth_success(SSH *ssh)
{
    PKT *pkt = allocSSHPkt_F(ssh, 0, 0);
    send_encrypt_packet(pkt, 0, SSH_MSG_USERAUTH_SUCCESS, ssh, 0);
}

static int recvCliUserAuthReq(unsigned char *pktbuf, int pktlen ,SSH *ssh, unsigned char iskexauth)
{
    unsigned char *p;
    unsigned char *username, *servicename, *methodname, *passwd, changepw;
    unsigned int userlen, servicelen, methodlen, passwdlen;
    int ret = -1;

    p = pktbuf + PACKET_PAYLOAD_OFF;
    if (*p != SSH_MSG_USERAUTH_REQUEST)
        return -1;
    p++;

    memcpy(&userlen, p, 4);
    p+=4;
    userlen = ntohl(userlen);
    username = mallocx(userlen+1);
    memcpy(username, p, userlen);
    *(username+userlen)='\0';
    p+=userlen;

    memcpy(&servicelen, p, 4);
    p+=4;
    servicelen = ntohl(servicelen);
    servicename = mallocx(servicelen+1);
    memcpy(servicename, p, servicelen);
    *(servicename+servicelen)='\0';
    p+=servicelen;

    memcpy(&methodlen, p, 4);
    p+=4;
    methodlen = ntohl(methodlen);
    methodname = mallocx(methodlen+1);
    memcpy(methodname, p, methodlen);
    *(methodname+methodlen)='\0';
    p+=methodlen;

    if (servicelen != SSH_SERVICE_CONNECTION_LEN
            && (strncmp(servicename, SSH_SERVICE_CONNECTION,SSH_SERVICE_CONNECTION_LEN) != 0))
    {
        free(username);
        free(servicename);
        free(methodname);
        ret = send_msg_userauth_failure(ssh, 0, 1);
        goto  out;
    }
    /* user wants to know what methods are supported */
    if (methodlen == AUTH_METHOD_NONE_LEN &&
            strncmp(methodname, AUTH_METHOD_NONE, AUTH_METHOD_NONE_LEN) == 0)
    {
        ret = send_msg_userauth_failure(ssh, 0, 0);
        ssh->connecttime = 1;
        goto out;
    }

    if (ssh->username == NULL || strcmp(username, ssh->username) != 0)
    {
        //find username is in the user list
        //send_msg_userauth_failure(s, 0);
        ssh->chan = mallocx(sizeof(Channel));
        memset(ssh->chan, 0, sizeof(Channel));
        memcpy(ssh->chan->session.cmdBuf, username, userlen);
        ssh->chan->session.cmdBufOffset = userlen;
        ssh->chan->session.state = TEL_GET_USERNAME;
        ssh->chan->session.ssh = ssh;
        ssh->chan->session.cs = ssh->cs;
        ssh->chan->session.shell = !iskexauth;
        parseCmdBufffer(&ssh->chan->session);
        if (ssh->chan->session.state != TEL_PWD)
        {
            ret = send_msg_userauth_failure(ssh, 0, 1);
            ssh->connecttime = 1;
            goto out;
        }
        if (ssh->username != NULL)
        {
            freex(ssh->username);
        }
        ssh->username = mallocx(userlen+1);
        strcpy(ssh->username, username);
    }

    /* user wants to try password auth */
    if (methodlen == AUTH_METHOD_PASSWORD_LEN &&
            strncmp(methodname, AUTH_METHOD_PASSWORD, AUTH_METHOD_PASSWORD_LEN) == 0)
    {
        changepw = *p;
        p++;
        if (changepw)
        {
            ret = send_msg_userauth_failure(ssh, 0, 1);
            goto out;
        }
        //find password according to ssh->username
        memcpy(&passwdlen, p, 4);
        p+=4;
        passwdlen = ntohl(passwdlen);
        passwd = mallocx(passwdlen+1);
        memcpy(passwd, p, passwdlen);
        *(passwd+passwdlen)='\0';
        p+=passwdlen;
        if (ssh->chan)
        {
            memcpy(ssh->chan->session.cmdBuf, passwd, passwdlen);
            ssh->chan->session.cmdBufOffset = passwdlen;
            ssh->chan->session.state = TEL_GET_PASSWORD;
            ssh->chan->session.ssh->banner = 1;
            parseCmdBufffer(&ssh->chan->session);
            if (ssh->chan->session.state != TEL_AUTH_PASS)
            {
                ret = send_msg_userauth_failure(ssh, 0, 1);
                ssh->connecttime = 1;
                goto out;
            }
            ssh->connecttime = 0;
            ssh->chan->session.ssh->banner = 0;
            ssh->authdone = 1;
            send_msg_userauth_success(ssh);
            ssh->state = CLI_SSH_CHAN_OPEN;
            ret = 0;
            goto out;
        }
        freex(passwd);
    }
    ret = send_msg_userauth_failure(ssh, 0, 1);
out:
    freex(username);
    freex(servicename);
    freex(methodname);

    return ret;
}

static void send_msg_channel_open_success(SSH *ssh, Channel *chan, unsigned int recvwindow, unsigned int recvmaxpacket)
{
    PKT *pkt = NULL;
    unsigned char *buf = mallocx(20);
    unsigned int tmpint=0;
    unsigned char *p = buf;

    tmpint = chan->recvChanId;
    tmpint = htonl(tmpint);
    memcpy(p, &tmpint, 4);
    p+=4;
    tmpint = chan->senderChanId;
    tmpint = htonl(tmpint);
    memcpy(p, &tmpint, 4);
    p+=4;
    tmpint = recvwindow;
    tmpint = htonl(tmpint);
    memcpy(p, &tmpint, 4);
    p+=4;
    tmpint = recvmaxpacket;
    tmpint = htonl(tmpint);
    memcpy(p, &tmpint, 4);
    p+=4;

    pkt = allocSSHPkt_F(ssh, p-buf, 0);
    memcpy(pkt->wp, buf, p-buf);
    send_encrypt_packet(pkt, p-buf, SSH_MSG_CHANNEL_OPEN_CONFIRMATION, ssh, 0);

    free(buf);
}

static int recv_msg_channel_open(unsigned char *pktbuf, int pktlen, SSH *ssh)
{
    SSHTranLayerHdr *hdr = NULL;
    unsigned char *p  = NULL;
    unsigned int channelTypeLen = 0;
    char *channelType = NULL;
    unsigned int senderChannel = 0;
    unsigned int winSize = 0;
    unsigned int maxPktSize = 0;
    int ret = -1;

    hdr = (SSHTranLayerHdr*)pktbuf;
    hdr->length = ntohl(hdr->length);
    p = pktbuf + PACKET_PAYLOAD_OFF;

    if (*p != SSH_MSG_CHANNEL_OPEN)
        return -1;
    p++;

    memcpy(&channelTypeLen, p, 4);
    channelTypeLen = ntohl(channelTypeLen);
    p+=4;
    channelType = mallocx(channelTypeLen+1);
    memcpy(channelType, p, channelTypeLen);
    p+=channelTypeLen;
    *(channelType+channelTypeLen)='\0';

    memcpy(&senderChannel, p, 4);
    senderChannel = ntohl(senderChannel);
    p+=4;
    memcpy(&winSize, p, 4);
    winSize = ntohl(winSize);
    p+=4;
    memcpy(&maxPktSize, p, 4);
    maxPktSize = ntohl(maxPktSize);
    p+=4;

    if (strcmp(channelType, "session") == 0)
    {
        ssh->chan->type= CHANNEL_ID_SESSION;
        ssh->chan->senderChanId = 1;
        ssh->chan->recvChanId = senderChannel;//default only 1 receiving channel id = 0
        ssh->chan->winSize = winSize;
        ssh->chan->maxPktSize = maxPktSize;
        send_msg_channel_open_success(ssh, ssh->chan, RECV_MAXWINDOW, RECV_MAXPACKET);
        ssh->state = CLI_SSH_CHAN_REQ;
        ret = 0;
    }
    else
        ret = -1;

    freex(channelType);
    return ret;
}

static void send_msg_channel_success(SSH *ssh, Channel *chan)
{
    PKT *pkt = NULL;
    pkt = allocSSHPkt_F(ssh, 4, 0);
    setInt(pkt->wp, ssh->chan->recvChanId);
    send_encrypt_packet(pkt, 4, SSH_MSG_CHANNEL_SUCCESS, ssh, 0);
}

int sshEncodeData(PKT *pkt, int len, SSH *ssh)
{
    send_encrypt_packet(pkt, len, SSH_MSG_CHANNEL_DATA, ssh, 1);
    return 0;
}

int sshEncodeBanner(PKT *pkt, int len, SSH *ssh)
{
    send_encrypt_packet(pkt, len, SSH_MSG_USERAUTH_BANNER, ssh, 0);
    return 0;
}
static int recv_msg_channel_request(unsigned char *pktbuf, int pktlen, SSH *ssh, unsigned char iskexauth)
{
    unsigned char *p  = NULL;
    unsigned int recvChanId = 0, termLen = 0, sesCmdLen = 0, termModeLen = 0;
    unsigned char wantReply = 0;
    unsigned char *sesCmd = NULL, *term = NULL, *termMode = NULL;

    p = pktbuf + PACKET_PAYLOAD_OFF;
    if (*p != SSH_MSG_CHANNEL_REQUEST)
        return -1;
    p++;
    memcpy(&recvChanId, p, 4);
    recvChanId = ntohl(recvChanId);
    p+=4;
    //if(recvChanId != 0) //default only 1 receiving channel id = 0
    //	return -1;

    memcpy(&sesCmdLen, p, 4);
    sesCmdLen = ntohl(sesCmdLen);
    p+=4;
    sesCmd = mallocx(sesCmdLen+1);
    memcpy(sesCmd, p, sesCmdLen);
    *(sesCmd+sesCmdLen) = '\0';
    p+=sesCmdLen;

    memcpy(&wantReply, p, 1);
    p++;

    if (strcmp(sesCmd, "pty-req") == 0)
    {
        memcpy(&termLen, p, 4);
        termLen = ntohl(termLen);
        p+=4;
        term = mallocx(termLen+1);
        memcpy(term, p, termLen);
        *(term+termLen) = '\0';
        p+=termLen;

        memcpy(&ssh->chan->termc, p, 4);
        ssh->chan->termc = ntohl(ssh->chan->termc);
        p+=4;

        memcpy(&ssh->chan->termr, p, 4);
        ssh->chan->termr = ntohl(ssh->chan->termr);
        p+=4;

        memcpy(&ssh->chan->termw, p, 4);
        ssh->chan->termw = ntohl(ssh->chan->termw);
        p+=4;

        memcpy(&ssh->chan->termh, p, 4);
        ssh->chan->termh = ntohl(ssh->chan->termh);
        p+=4;

        memcpy(&termModeLen, p, 4);
        termModeLen = ntohl(termModeLen);
        p+=4;
        termMode = mallocx(termModeLen+1);
        memcpy(termMode, p, termModeLen);
        *(termMode+termModeLen) = '\0';
        p+=termModeLen;

        freex(term);
        freex(termMode);
    }

    else if (strcmp(sesCmd, "shell") == 0)
    {
        /*send shell tag*/
        //sendShellTag(ssh);
        if (iskexauth)
            ssh->chan->session.shell = 0;
        else
            ssh->chan->session.shell = 1;

        if (wantReply)
            send_msg_channel_success(ssh, ssh->chan);
        wantReply = 0;
        putSendBuf(&ssh->chan->session, '\r');
        putSendBuf(&ssh->chan->session, '\n');
        sendShellTag(&ssh->chan->session);
        flushTelnetSendBuffer(&ssh->chan->session); //for first time shell tag
        //sshDataStage = 1;
    }
    else if (strcmp(sesCmd, "window-change") == 0)
    {
    }	
    else if (strcmp(sesCmd, "shell"))
    {
        freex(sesCmd);
        return -1;
    }	
    //	sshDataStage = 1;

    if (wantReply)
        send_msg_channel_success(ssh, ssh->chan);

    ssh->state = CLI_SSH_CHAN_DATA;

    freex(sesCmd);

    return 0;
}

#if 0
static int send_msg_channel_eof(SSH *ssh)
{
    PKT *txPkt = NULL;
    txPkt = allocSSHPkt(ssh, 4, 0);
    setInt(txPkt->wp, ssh->chan->recvChanId);
    ssh->state = CLI_SSH_CHAN_EOF;
    ssh->chan->transeof = 1;
    send_encrypt_packet(txPkt, 4, SSH_MSG_CHANNEL_EOF, ssh, 0);
    return 0;
}
#endif

int send_msg_channel_close(SSH *ssh)
{
    PKT *txPkt = NULL;

    if (ssh->chan->sentclosed == 1)
        return 0;

    ssh->chan->sentclosed = 1;
    txPkt = allocSSHPkt_F(ssh, 4, 0);
    setInt(txPkt->wp, ssh->chan->recvChanId);
    ssh->state = CLI_SSH_CHAN_EOF;
    ssh->chan->transeof = 1;
    send_encrypt_packet(txPkt, 4, SSH_MSG_CHANNEL_CLOSE, ssh, 0);
    return 0;
}

static int recv_msg_channel_eof(unsigned char *pktbuf, int pktlen, SSH *ssh)
{
    unsigned int recvChanId = 0;
    unsigned char *p = NULL;
    p = pktbuf + PACKET_PAYLOAD_OFF + 1;

    memcpy(&recvChanId, p, 4);
    if (ntohl(recvChanId)!= 1)
        return -1;
    //ssh->chan->recvChanId = htonl(recvChanId);

    ssh->chan->recveof = 1;
    if (ssh->chan->transeof==1)
    {
        send_msg_channel_close(ssh);
    }
    return 0;
}

static int recv_msg_channel_close(unsigned char *pktbuf, int pktlen, SSH *ssh)
{
    unsigned int recvChanId = 0;
    unsigned char *p = NULL;
    p = pktbuf + PACKET_PAYLOAD_OFF + 1;

    memcpy(&recvChanId, p, 4);
    if (ntohl(recvChanId) != 1)
        return -1;
    //ssh->chan->recvChanId = htonl(recvChanId);

    ssh->chan->recveof = 1;

    //tcpClose(pcb, 0);
    rtSktClose_F(ssh->cs);
    return 0;
}

#if 0
static int send_msg_channel_data(unsigned char *data, int dataLen, SSH *ssh)
{
    PKT *txPkt = allocSSHPkt(ssh, 4+dataLen, 1);
    setInt(txPkt->wp, dataLen);
    memcpy(txPkt->wp+4, data, dataLen);
    sshEncodeData(txPkt, 4+dataLen, ssh);
    return 0;
}
#endif

static int recv_msg_channel_data(unsigned char *pktbuf, int pktlen, SSH *ssh, unsigned char iskexauth)
{
    unsigned char *p = NULL;
    int dataLen = 0, recvChanId, ret = -1;
    if (pktlen > ssh->chan->maxPktSize)
        pktlen = ssh->chan->maxPktSize;

    p = pktbuf + PACKET_PAYLOAD_OFF;
    if (*p != SSH_MSG_CHANNEL_DATA)
        return -1;
    p++;

    memcpy(&recvChanId, p, 4);
    if (ntohl(recvChanId) != 1)
        return -1;
    p+=4;

    memcpy(&dataLen, p, 4);
    dataLen = ntohl(dataLen);
    p+=4;

    if (iskexauth == 0)
    {
        ssh->chan->session.echo  =1;
        ssh->chan->session.ssh = ssh;
        ssh->chan->session.cs = ssh->cs;
        if ((ret=getShellCmd(&ssh->chan->session, p, 0, 1)) <= 0)
        {
        }
        flushTelnetSendBuffer(&ssh->chan->session);
    }
    else
    {
        ssh->decryptedlen = dataLen;
        ssh->decryptedpkt = p;
        //send_msg_channel_data(p, dataLen, ssh);
    }
    ssh->state = CLI_SSH_CHAN_DATA;

    return 0;
}

int channelRequest(unsigned char *pktbuf, int pktlen, SSH *ssh, unsigned char iskexauth)
{
    int ret = -1;
    switch (*(pktbuf+5))
    {
    case SSH_MSG_CHANNEL_OPEN:
        printf("8\n");
        debugssh = 8;
        ret = recv_msg_channel_open(pktbuf, pktlen, ssh);
        break;
    case SSH_MSG_CHANNEL_REQUEST:
        printf("9\n");
        debugssh = 9;
        ret = recv_msg_channel_request(pktbuf, pktlen, ssh, iskexauth);
        break;
    case SSH_MSG_CHANNEL_DATA:
        printf("10\n");
        debugssh = 10;
        ret = recv_msg_channel_data(pktbuf, pktlen, ssh, iskexauth);
        break;
    case SSH_MSG_CHANNEL_EOF:
        debugssh = 11;
        ret = recv_msg_channel_eof(pktbuf, pktlen, ssh);
        break;
    case SSH_MSG_CHANNEL_CLOSE:
        debugssh = 12;
        ret = recv_msg_channel_close(pktbuf, pktlen, ssh);
        break;
    default:
        debugssh = 13;
        ret = recv_unimplemented(pktbuf, pktlen, ssh);
    }
    return ret;
}

int recv_unimplemented(unsigned char *pktbuf, int pktlen, SSH *ssh)
{
    return 0;
}

static int doSSHAccept(unsigned char *pktbuf, int pktlen, SSH *ssh, unsigned char iskexauth)
{
    int ret = -1;

    // if (ssh->state >= SRV_SSH_KEY_INIT_EX)
    // {
    //  ssh->recvseq++;
    //}
    switch (*(pktbuf+5))
    {
    case SSH_MSG_IGNORE:
        ret = 0;
        goto exit;
    }
    switch (ssh->state)
    {
    case SRV_SSH_PROTOCOL_EX:
        printf("1\n");
        debugssh=1;
        ret = chkCliSSHPtlVer(pktbuf, pktlen, ssh);
        break;
    case SRV_SSH_KEY_INIT_EX:
        printf("2\n");
        debugssh = 2;
        ret = clientKeyINITEx(pktbuf, pktlen, ssh);
        break;
    case CLI_SSH_KEY_INIT_EX:
        printf("3\n");
        debugssh = 3;
        ret = recvCliDFKexinit(pktbuf, pktlen, ssh);
        break;
    case CLI_SSH_NEW_KEY:
        printf("4\n");
        debugssh=4;
        ret = recvCliNewKey(pktbuf, pktlen, ssh);
        break;
    case CLI_SSH_SRV_REQ:
        printf("5\n");
        debugssh=5;
        ret = recvCliSrvReq(pktbuf, pktlen, ssh);
        break;
    case CLI_SSH_USERAUTH_REQ:
        debugssh=6;
        ret = recvCliUserAuthReq(pktbuf, pktlen ,ssh, iskexauth);
        break;
    case CLI_SSH_CHAN_OPEN:
    case CLI_SSH_CHAN_REQ:
    case CLI_SSH_CHAN_DATA:
    case CLI_SSH_CHAN_EOF:
    case CLI_SSH_CHAN_CLOSE:
        printf("6\n");
        ret = channelRequest(pktbuf, pktlen, ssh, iskexauth);
        break;
    default:
        printf("7");
        debugssh=7;
        recv_unimplemented(pktbuf, pktlen, ssh);
    }

exit:
    return ret;
}

PKT* sshrx(SSH **ssh, RTSkt* cs, unsigned char iskexauth, INT32S* status)
{
    PKT *rxPkt = NULL;
    int ret = -1;
    int init = 0;
    unsigned int datasize = 0, len=0, left = 0;
    if (*status == 1)
    {
        if ((rxPkt = rtSktRx_F(cs, SOLINTERVAL, status)) != 0)
        {
            return rxPkt;
        }
	return NULL;
    }
    *status = 0;
    while (1)
    {
        if (!(*ssh))
        {
            ret = SSHInit(ssh,cs);

            (*ssh)->rxbuf = mallocx(2048);
            left = 0;
            datasize=0;
            /*send version packte*/
            if (ret != 0)
                goto SSH_CLIENT_EXIT;
        }
        do
        {
            rxPkt = rtSktRx_F(cs, SOLINTERVAL, status);

            if (*status == SKTDISCONNECT || NewSSHCli)
            {
                *status = -2;
                goto SSH_CLIENT_EXIT;
            }
            if ((*ssh)->connecttime >= 1)
            {
                (*ssh)->connecttime ++;
                if ((*ssh)->connecttime >= 600) //0.1*6000 = 600s = 10m
                {
                    *status = -2;
                    goto SSH_CLIENT_EXIT;
                }
            }

            if (rxPkt == 0 &&(*ssh)->state == CLI_SSH_CHAN_DATA && iskexauth)
            {
                *status = 0;
                return 0;
            }
        }while (rxPkt==0);
        if (datasize == 0)
        {
            memset((*ssh)->rxbuf, 0, 2048);
        }
        memcpy((*ssh)->rxbuf+datasize, rxPkt->wp, rxPkt->len);
        datasize += rxPkt->len;
        *((*ssh)->rxbuf+datasize) = '\0';

        if ((*ssh)->state < CLI_SSH_SRV_REQ)
        {
            if (init > 0)
            {
                memcpy(&len, (*ssh)->rxbuf, 4);
                len = ntohl(len) + 4;
            }
            else
            {
                len = strlen((*ssh)->rxbuf);
                init = 1;
            }
            if (len > rxPkt->len && (left == 0 || left > rxPkt->len))
            {
                left = len - rxPkt->len;
                freePkt(rxPkt);
                rxPkt = NULL;
                continue;
            }
            else if (left > 0 && left < rxPkt->len)
            {
                printf("packet too large\n");
                goto SSH_CLIENT_EXIT;
            }
            if ((*ssh)->state >= SRV_SSH_KEY_INIT_EX)
                (*ssh)->recvseq++;
            ret = doSSHAccept((*ssh)->rxbuf, datasize, (*ssh), 0);
            if (ret !=0)
            {
                //send failure
                goto SSH_CLIENT_EXIT;
            }
            freePkt(rxPkt);
            rxPkt = NULL;
            left = 0;
            datasize = 0;
        }
        else if ((*ssh)->state >= CLI_SSH_SRV_REQ)
        {
            sha1_context ctx;
            unsigned char seqbuf[4];
            unsigned char output[20];
            unsigned char *p = (*ssh)->rxbuf;
            do
            {
                /*decrypt the first 8 bytes*/

                aes_crypt_cbc((aes_context*) (*ssh)->keys->recv_algo_crypt, AES_DECRYPT, 16, (*ssh)->keys->recv_IV, p, p);


                memcpy(&len, p, 4);
                len = ntohl(len) + 4;

                left = datasize - len - SHA1_HASH_SIZE;
                if (len > 16)
                {
                    //aes_crypt_cbc_sw(&ssh->keys->recv_algo_crypt, AES_DECRYPT, len - 16, ssh->keys->recv_IV, (*ssh)->rxbuf + 16, (*ssh)->rxbuf + 16);
                    aes_crypt_cbc((aes_context*) (*ssh)->keys->recv_algo_crypt, AES_DECRYPT, len - 16, (*ssh)->keys->recv_IV, p + 16, p + 16);

                }

                sha1_hmac_startsH( &ctx, (*ssh)->keys->recvmackey, SHA1_HASH_SIZE );				/* sequence number */
                STORE32H((*ssh)->recvseq, seqbuf);
                sha1_hmac_updateH( &ctx, seqbuf, 4 );
                sha1_hmac_updateH( &ctx, p, len );
                sha1_hmac_finishH( &ctx, output );

                if (memcmp(output, p+len, SHA1_HASH_SIZE) != 0)
                {
                    goto SSH_CLIENT_EXIT;
                }
                (*ssh)->recvseq++;
                datasize = len;
                ret = doSSHAccept(p, datasize, (*ssh), iskexauth);
                if (ret != 0)
                {
                    goto SSH_CLIENT_EXIT;
                }
                if (left>0)
                {
                    p+=(len+SHA1_HASH_SIZE);
                    datasize = left;
                }
                else
                {
                    left = 0;
                    datasize = 0;
                }
            }while (left>0);
            freePkt(rxPkt);
            rxPkt = NULL;
            if (iskexauth && (*ssh)->state == CLI_SSH_CHAN_DATA && (*ssh)->decryptedlen > 0)
            {
                rxPkt = allocPkt_F((*ssh)->decryptedlen);
                memcpy(rxPkt->wp, (*ssh)->decryptedpkt, (*ssh)->decryptedlen);
                rxPkt->len = (*ssh)->decryptedlen;

                return rxPkt;
            }
            else if (iskexauth && (*ssh)->state == CLI_SSH_CHAN_DATA ) //open tcr
            {
                *status = 2;
                return (void*)1;
            }
            continue;
        }
    }
SSH_CLIENT_EXIT:
    if ((*ssh)->authdone ==1 &&  (*ssh)->chan && (*ssh)->chan->sentclosed == 0)
    {
        send_msg_channel_close((*ssh));
    }
    //stopconn(cs);
    SSHFree(ssh);
    //if (rxPkt)
    {
        //     freePkt(rxPkt);
    }
    //*status = ret;
    return NULL;
}

void sshClient(void *data)
{
    RTSkt* cs = (RTSkt*)data;
    PKT* rxPkt;
    int status;

    SSH *ssh = NULL;

    if ((rxPkt = sshrx(&ssh, cs, 0, &status)) != 0)
    {
            if (status == SKTDISCONNECT || NewSSHCli)
            {
                
            }	
    }
    stopconn(cs);
    if (rxPkt)
    {
        freePkt(rxPkt);
    }
    OSTaskDel(OS_PRIO_SELF);
}
#endif

#ifdef CONFIG_SSH_ENABLED
void sshSrv(void *data)
{
    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_TCP);
    RTSkt* cs;
    OS_TCB pdata;

    rtSktBind_F(s, 22);
    rtSktListen_F(s, 2);

    while (1)
    {
        cs = rtSktAccept_F(s);

        NewSSHCli = 1;

        while (OSTaskQuery(TASK_TELNET_CLIENT_PRIO, &pdata) == OS_ERR_NONE)
        {
            //	rtSktClose(pcs);
            printf("Task exist");
            OSTimeDly(10);
        }

        //	pcs = cs;

        NewSSHCli = 0;
        OSTaskCreate(sshClient, cs, (unsigned long*)&TelnetClientStk[TASK_TELNET_CLIENT_STK_SIZE - 1], TASK_TELNET_CLIENT_PRIO);
    }
}
#endif
