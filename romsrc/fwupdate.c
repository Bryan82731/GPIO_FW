#include "fwupdate.h"
#include "bsp.h"
#include "lib.h"
#include "aes.h"
#include "app_cfg.h"
#include "rtskt.h"
#include "lib.h"
#include "udp.h"
#include <stdlib.h>
#include <string.h>

//should always check 0x81000004 for 32 bits
//put firmware version in 0x81000004 for update comparison

//program code started at 0x81010000
//configurations related to 8111DP is at 0x81001000, please check lib.h for
//more information

static unsigned int FWUInIP;
static unsigned int FWUInPort;
static unsigned int FWUAuthIP;
static unsigned int FWUAuthPort;
//static unsigned char FWUAuthUser[16];
//static unsigned char FWUAuthPw[16];
static unsigned char FWUVerifyCode[16];
static unsigned char FWUAuthCode[16];
static unsigned int FWUBlkNum;
static unsigned int FWUWAddr;

//unsigned char DtAuthUser[16] = "Realtek";
//unsigned char DtFWUAuthPw[16] = "Secret";

unsigned char CurFWAuthUser[16] = "";
unsigned char CurFWUAuthPw[16] = "";

unsigned char* SSI_MAP = (unsigned char*) FLASH_BASE_ADDR;
unsigned int Sector = 0;

extern DPCONF *dpconf;

//DPCONF *fwconf = (DPCONF *) 0x81001000;

//#define FW_SYSTEM_USER_INFO         0x8100A000
#define SPI_BLK_SIZE 64

//static FWUserInfo* FWUserHead = 0;
extern FWSIG *fwsig;

#if 0
unsigned short
htons(unsigned short n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ntohs(unsigned short n)
{
    return htons(n);
}
/*-----------------------------------------------------------------------------------*/
unsigned int
htonl(unsigned int n)
{
    return ((n & 0xff) << 24) |
           ((n & 0xff00) << 8) |
           ((n & 0xff0000) >> 8) |
           ((n & 0xff000000) >> 24);
}
/*-----------------------------------------------------------------------------------*/
unsigned int
ntohl(unsigned int n)
{
    return htonl(n);
}

#endif

/**********************************************************************/
/* Message digest routines:                                           */
/* To form the message digest for a message M                         */
/*    (1) Initialize a context buffer md using MDINIT                 */
/*    (2) Call MDUPDATE on md and each character of M in turn         */
/*    (3) Call MDFINAL on md                                          */
/* The message digest is now in md->D[0...15]                         */
/**********************************************************************/
/* An MDCTX structure is a context buffer for a message digest        */
/*  computation; it holds the current "state" of a message digest     */
/*  computation                                                       */
struct MDCTX
{
    unsigned char  D[48];   /* buffer for forming digest in */
    /* At the end, D[0...15] form the message */
    /*  digest */
    unsigned char  C[16];   /* checksum register */
    unsigned char  i;       /* number of bytes handled, modulo 16 */
    unsigned char  L;       /* last checksum char saved */
};
/* The table S given below is a permutation of 0...255 constructed    */
/*  from the digits of pi.  It is a ``random'' nonlinear byte         */
/*  substitution operation.                                           */
const int S[256] = {
    41, 46, 67,201,162,216,124,  1, 61, 54, 84,161,236,240,  6, 19,
    98,167,  5,243,192,199,115,140,152,147, 43,217,188, 76,130,202,
    30,155, 87, 60,253,212,224, 22,103, 66,111, 24,138, 23,229, 18,
    190, 78,196,214,218,158,222, 73,160,251,245,142,187, 47,238,122,
    169,104,121,145, 21,178,  7, 63,148,194, 16,137, 11, 34, 95, 33,
    128,127, 93,154, 90,144, 50, 39, 53, 62,204,231,191,247,151,  3,
    255, 25, 48,179, 72,165,181,209,215, 94,146, 42,172, 86,170,198,
    79,184, 56,210,150,164,125,182,118,252,107,226,156,116,  4,241,
    69,157,112, 89,100,113,135, 32,134, 91,207,101,230, 45,168,  2,
    27, 96, 37,173,174,176,185,246, 28, 70, 97,105, 52, 64,126, 15,
    85, 71,163, 35,221, 81,175, 58,195, 92,249,206,186,197,234, 38,
    44, 83, 13,110,133, 40,132,  9,211,223,205,244, 65,129, 77, 82,
    106,220, 55,200,108,193,171,250, 36,225,123,  8, 12,189,177, 74,
    120,136,149,139,227, 99,232,109,233,203,213,254, 59,  0, 29, 57,
    242,239,183, 14,102, 88,208,228,166,119,114,248,235,117, 75, 10,
    49, 68, 80,180,143,237, 31, 26,219,153,141, 51,159, 17,131, 20,
};
/*The routine MDINIT initializes the message digest context buffer md.*/
/* All fields are set to zero.                                        */
void MDINIT(struct MDCTX *md)
{
    int i;
    for (i=0;i<16;i++) md->D[i] = md->C[i] = 0;
    md->i = 0;
    md->L = 0;
}
/* The routine MDUPDATE updates the message digest context buffer to  */
/*  account for the presence of the character c in the message whose  */
/*  digest is being computed.  This routine will be called for each   */
/*   message byte in turn.                                            */
void MDUPDATE(  struct MDCTX *md, unsigned char c)
{
    register unsigned char i,j,t,*p;
    /**** Put i in a local register for efficiency ****/
    i = md->i;
    /**** Add new character to buffer ****/
    md->D[16+i] = c;
    md->D[32+i] = c ^ md->D[i];
    /**** Update checksum register C and value L ****/
    md->L = (md->C[i] ^= S[0xFF & (c ^ md->L)]);
    /**** Increment md->i by one modulo 16 ****/
    i = md->i = (i + 1) & 15;
    /**** Transform D if i=0 ****/
    if (i == 0)
    {
        t = 0;
        for (j=0;j<18;j++)
        {/*The following is a more efficient version of the loop:*/
            /*  for (i=0;i<48;i++) t = md->D[i] = md->D[i] ^ S[t]; */
            p = md->D;
            for (i=0;i<8;i++)
            {
                t = (*p++ ^= S[t]);
                t = (*p++ ^= S[t]);
                t = (*p++ ^= S[t]);
                t = (*p++ ^= S[t]);
                t = (*p++ ^= S[t]);
                t = (*p++ ^= S[t]);
            }
            /* End of more efficient loop implementation */
            t = t + j;
        }
    }
}
/* The routine MDFINAL terminates the message digest computation and  */
/* ends with the desired message digest being in md->D[0...15].       */
void MDFINAL(struct MDCTX *md)
{
    int i,padlen;
    /* pad out to multiple of 16 */
    padlen  = 16 - (md->i);
    for (i=0;i<padlen;i++) MDUPDATE(md,(unsigned char)padlen);
    /* extend with checksum */
    /* Note that although md->C is modified by MDUPDATE, character    */
    /* md->C[i] is modified after it has been passed to MDUPDATE, so  */
    /* the net effect is the same as if md->C were not being modified.*/
    for (i=0;i<16;i++) MDUPDATE(md,md->C[i]);
}

#if 0
void fwInituser_R()
{
    //64 byte header + n*User info
    //sector 10
    //FWUserInfoHdr* hdr = (FWUserInfoHdr*) FW_SYSTEM_USER_INFO;

    //Only use configuration data user info to verify
#define DEFAULT_USER_INFO 0x81001000
    //FWUserInfoHdr* hdr = (FWUserInfoHdr*) DEFAULT_USER_INFO;
    FWUserInfo *tmp;
    FWUserInfo *rom;
    unsigned char buf[64];
    aes_context_sw dctx;
    unsigned char key[] = {"^^rEaLtEk^^@@#&("};
    unsigned char iv[] ={"REALTEKCNSD7SWDP"};
    //int i = 0;

    //1 sector length is 4k
    //After erase, default value is 0xff
    //if(hdr->length == 0 || hdr->number == 0 || hdr->length > 4096 || hdr->number > 0xff)
    //{
    //	FWUserHead = 0;
    //	return;
    //}

    //buf = malloc(hdr->length);
    //buf = malloc(64);

    aes_setkey_dec_sw( &dctx, key, 16*8 );
    aes_crypt_cbc_sw( &dctx, AES_DECRYPT_SW, 64, iv, (void *) DEFAULT_USER_INFO, buf);

    rom = (FWUserInfo *) buf;
    tmp = malloc(sizeof(FWUserInfo));
    FWUserHead = tmp;
    memset(FWUserHead, 0, sizeof(UserInfo));

    strcpy(tmp->name, rom->name);
    strcpy(tmp->passwd, rom->passwd);
    strcpy(tmp->caption, rom->caption);
    tmp->role = rom->role;

#if 0
    for (i = 0 ; i< hdr->number; i++)
    {
        strcpy(tmp->name, rom->name);
        strcpy(tmp->passwd, rom->passwd);
        strcpy(tmp->caption, rom->caption);
        tmp->role = rom->role;
        rom++;

        if (i != hdr->number - 1)
        {
            tmp->next = malloc(sizeof(UserInfo));
            memset(tmp->next, 0, sizeof(UserInfo));
            tmp = tmp->next;
        }
    }
#endif
    tmp->next = 0;
    //free(buf);
}
#endif

PKT* allocFWUPkt(int size)
{
    PKT* pkt;
    pkt = (PKT*) malloc(sizeof(PKT));

    pkt->start = (unsigned char*) malloc(ETH_HDR_SIZE + IP_HDR_SIZE + UDP_HDR_SIZE + sizeof(FWUpdateHdr) + size);
    pkt->wp = pkt->start + ETH_HDR_SIZE + IP_HDR_SIZE + UDP_HDR_SIZE;
    memset(pkt->wp, 0, sizeof(FWUpdateHdr) + size);
    pkt->len = size;
    pkt->ipVer = IPv4;

    return pkt;
}

void makeFWUHdr(PKT* pkt, unsigned char opCode, unsigned short blkNum, unsigned short len)
{
    FWUpdateHdr *hdr = (FWUpdateHdr *)(pkt->wp);
    hdr->opCode = opCode;
    hdr->blkNum = blkNum;
    hdr->size = len;
}

void genVerifyCode(unsigned char* code)
{
    unsigned char key[16] = {0};
    unsigned char iv[16] = {0};
    unsigned char rn[16];
    aes_context_sw ectx;
    int i = 0;

    for (i = 0; i < 16; i++)
    {
        rn[i] = rand();
    }

    for (i = 0; i < strlen((const char *)CurFWUAuthPw); i++)
    {
        key[i] = iv[i] = CurFWUAuthPw[i];
    }

    memcpy(FWUAuthCode, rn ,16);
    aes_setkey_enc_sw( &ectx, key, 16*8 );
    aes_crypt_cbc_sw( &ectx, AES_ENCRYPT_SW, 16 , iv, rn, code);
}

void fwICInfo(void* pData, struct tcp_pcb* pcb)
{
    PKT* sendPkt;
	ICInfo info;
    
    sendPkt = allocFWUPkt(sizeof(ICInfo));	
	info.pkg_det = ReadIBReg(PCR);
	info.ver = IC_VERSION_FP_RevA;	
    makeFWUHdr(sendPkt, FW_ICINFO, 0, 0);
	memcpy(sendPkt->wp + sizeof(FWUpdateHdr), &info, sizeof(ICInfo));
    udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr) + sizeof(ICInfo));
}

void fwAuth(void* pData, struct tcp_pcb* pcb)
{
    int i = 0;
    unsigned char vCode[16] = {0};
    //unsigned char *p;

    PKT* sendPkt;
    FWUpdateHdr *hdr;
    UserInfo* uinfoTmp;
    struct MDCTX md2;

    hdr = (FWUpdateHdr*) pData;

    //if flash contain user info, copy it to the array
    //else use the default

    i = FW_AUTH_FAIL;

    //will always has account information
    //if (dpconf->admin)
    {
        uinfoTmp = (UserInfo *) &dpconf->admin;//FWUserHead;

        while (uinfoTmp)
        {
            MDINIT(&md2);
            for (i = 0; i < strlen((const char *)uinfoTmp->name); i++)
            {
                MDUPDATE(&md2, uinfoTmp->name[i]);
            }
            MDFINAL(&md2);

            if (memcmp(md2.D,hdr->auth,16) == 0)
            {
                if (uinfoTmp->role == 0x7f)
                {
                    memcpy(CurFWAuthUser, uinfoTmp->name, 16);
                    memcpy(CurFWUAuthPw, uinfoTmp->passwd, 16);
                    i = FW_AUTH_PASS;
                }
                else
                    i = FW_AUTH_PFAIL;

                break;
            }
            uinfoTmp = uinfoTmp->next;
        }
    }
    /*
    else
    {
    MDINIT(&md2);
    for(i = 0; i < strlen(DtAuthUser); i++)
    {
    MDUPDATE(&md2, DtAuthUser[i]);
    }
    MDFINAL(&md2);
    if(memcmp(md2.D,hdr->auth,16) == 0)
    {
    memcpy(CurFWAuthUser, DtAuthUser, 16);
    memcpy(CurFWUAuthPw, DtFWUAuthPw, 16);
    i = FW_AUTH_PASS;
    }
    }
    */

    sendPkt = allocFWUPkt(0);

    if ( i == FW_AUTH_PASS)
    {
        ///i = (REG32(IOREG_IOBASE + IO_CONFIG) & 0x7f);
        genVerifyCode(vCode);
        makeFWUHdr(sendPkt, FW_AUTH_PASS, i, 0);
        hdr = (FWUpdateHdr*) sendPkt->wp;
        memcpy(hdr->auth, vCode, 16);
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
    }
    else if ( i == FW_AUTH_PFAIL)
    {
        makeFWUHdr(sendPkt, FW_AUTH_PFAIL, 0, 0);
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
        return;
    }
    else
    {
        makeFWUHdr(sendPkt, FW_AUTH_FAIL, 0, 0);
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
        return;
    }
}

void fwVerify(void* pData, struct tcp_pcb* pcb)
{
    PKT* sendPkt;
    FWUpdateHdr *hdr;
    hdr = (FWUpdateHdr*) pData;

    //Setting authentication info
    sendPkt = allocFWUPkt(0);

    if (memcmp(FWUAuthCode,hdr->auth, 16))
    {
        makeFWUHdr(sendPkt, FW_VERIFY_FAIL, 0, 0);
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
    }
    else
    {
        memcpy(FWUVerifyCode, hdr->auth, 16);
        FWUAuthIP = FWUInIP;
        FWUAuthPort = FWUInPort;
        FWUBlkNum = 0;
        Sector = 0;
        makeFWUHdr(sendPkt, FW_VERIFY_PASS, 0, 0);
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
        FWUWAddr = 0;
    }
}

void fwWAddr(void* pData, int rxLen, struct tcp_pcb* pcb)
{
    PKT* sendPkt;
    FWUpdateHdr *hdr;
    char* wp = (char*)pData;
    hdr = (FWUpdateHdr*) pData;

    if (memcmp(FWUVerifyCode, hdr->auth, 16) != 0 || FWUAuthIP != FWUInIP || FWUAuthPort != FWUInPort)
    {
        sendPkt = allocFWUPkt(0);
        makeFWUHdr(sendPkt, FW_ABORT, 0, 0);
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
        return;
    }

    memcpy(&FWUWAddr, wp + sizeof(FWUpdateHdr), sizeof(FWUWAddr));
    sendPkt = allocFWUPkt(0);
    //ack block number and rx length
    makeFWUHdr(sendPkt, FW_ACK, 0, rxLen - sizeof(FWUpdateHdr));
    udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
}

void fwWrite(void* pData, int rxLen, struct tcp_pcb* pcb)
{
    int i = 0;
    INT32U rdid = 0;
    PKT* sendPkt;
    FWUpdateHdr *hdr;
    char* wp = (char*)pData;
    //unsigned char* FWAddr = (INT8U *) 0xA1010000;
    int wsize = 0;
    OS_CPU_SR cpu_sr = 0;

    hdr = (FWUpdateHdr*) pData;

    //Not expect client
    if (memcmp(FWUVerifyCode, hdr->auth, 16) != 0 || FWUAuthIP != FWUInIP || FWUAuthPort != FWUInPort || FWUWAddr == 0)
    {
        sendPkt = allocFWUPkt(0);
        makeFWUHdr(sendPkt, FW_ABORT, 0, 0);
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
        return;
    }

    if (hdr->blkNum == FWUBlkNum)
    {
        sendPkt = allocFWUPkt(0);
        //ack block number and rx length
        makeFWUHdr(sendPkt, FW_ACK, FWUBlkNum, rxLen - sizeof(FWUpdateHdr));
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));

        //block is the block number
        //buffer contains data written with size in byte
        //Erase code sector
        OS_ENTER_CRITICAL();
        if ((hdr->blkNum % 4 )  == 0)
        {
            rdid = (spi_read_id(SSI_MAP)  & 0x00ff0000) >> 16;
            if (rdid == 0x1f)
                spi_se_unprotect(SSI_MAP, FWUWAddr+Sector);
            spi_se_erase(SSI_MAP, FWUWAddr+Sector);
            Sector += 0x001000;

        }

        wsize = rxLen  - sizeof(FWUpdateHdr);
        i = 0;

        while (wsize)
        {
            if (wsize > SPI_BLK_SIZE)
            {
			spi_write(SSI_MAP, FWUWAddr + 0x000400 * hdr->blkNum + i * SPI_BLK_SIZE, (INT8U *) wp + sizeof(FWUpdateHdr) + i * SPI_BLK_SIZE, SPI_BLK_SIZE);
		        wsize = wsize -  SPI_BLK_SIZE;
                i++;
            }
            else
            {
                spi_write(SSI_MAP, FWUWAddr + 0x000400 * hdr->blkNum + i * SPI_BLK_SIZE, (INT8U *) wp + sizeof(FWUpdateHdr) + i * SPI_BLK_SIZE, wsize);	
                wsize = 0;
            }
        }
        OS_EXIT_CRITICAL();

        //kick watch dog timer
        bsp_wdt_kick();
        //fwrite(pkt->wp + sizeof(FWUpdateHdr),1 ,pkt->len  - sizeof(FWUpdateHdr), f);
        FWUBlkNum++;
        return;
    }
    else
    {
        //Out of order data
        sendPkt = allocFWUPkt(0);
        makeFWUHdr(sendPkt, FW_ACK, FWUBlkNum, 0);
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
        return;
    }
}

void fwCfgWrite(void* pData, struct tcp_pcb* pcb)
{
#define CFG_SECTOR_OFFSET (DPCONF_ROM_START_ADDR  & 0x00FFFFFF)
    int i = 0;
    INT32U rdid = 0;
    int blkSize = 4096;
    PKT* sendPkt;
    FWUpdateHdr *hdr;
    unsigned char *cfg;
    OS_CPU_SR  cpu_sr = 0;
    hdr = (FWUpdateHdr*) pData;
    //aes_context *ctx = malloc(sizeof(aes_context));


    //Not expect client
    if (memcmp(FWUVerifyCode, hdr->auth, 16) != 0 || FWUAuthIP != FWUInIP || FWUAuthPort != FWUInPort || FWUWAddr == 0)
    {
        sendPkt = allocFWUPkt(0);
        makeFWUHdr(sendPkt, FW_ABORT, 0, 0);
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
        return;
    }

    cfg = malloc(blkSize);
    //read configure block
    //memcpy(cfg, dpconf, blkSize);
    memcpy(cfg, (INT8U *) DPCONF_ROM_START_ADDR, blkSize);

    //memset(ctx, 0, sizeof(aes_context));
    //aes_crypt_sw(ctx, (INT8U *) dpconf, cfg, AES_ENCRYPT);

    //blkNum is offset
    memcpy(cfg + hdr->blkNum, pData + sizeof(FWUpdateHdr), hdr->size);
    //Erase configure sector

    OS_ENTER_CRITICAL();
    rdid = (spi_read_id(SSI_MAP)  & 0x00ff0000) >> 16;
    if (rdid == 0x1f)
        spi_se_unprotect(SSI_MAP, CFG_SECTOR_OFFSET);
    spi_se_erase(SSI_MAP, CFG_SECTOR_OFFSET);

    while (blkSize)
    {
        if (blkSize > SPI_BLK_SIZE)
        {
            spi_write(SSI_MAP, CFG_SECTOR_OFFSET + i * SPI_BLK_SIZE, cfg + i * SPI_BLK_SIZE, SPI_BLK_SIZE);
            blkSize = blkSize -  SPI_BLK_SIZE;
            i++;
        }
        else
        {
            spi_write(SSI_MAP, CFG_SECTOR_OFFSET + i * SPI_BLK_SIZE, cfg + i * SPI_BLK_SIZE, blkSize);
            blkSize = 0;
        }
    }
    OS_EXIT_CRITICAL();

    free(cfg);
    //kick watch dog timer
    bsp_wdt_kick();
    sendPkt = allocFWUPkt(0);
    makeFWUHdr(sendPkt, FW_ACKWCFG, 0, 0);
    udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
    return;
}

void fwUpdateFinish(void* pData, struct tcp_pcb* pcb)
{
    INT32U rdid = 0;
    INT8U i = 0;
    OS_CPU_SR cpu_sr = 0;

    PKT* sendPkt;
    FWUpdateHdr *hdr;
    FWSIG *fws = malloc(4096);

    memset(fws, 0xFF, 4096);
    hdr = (FWUpdateHdr*) pData;
    memcpy(fws, pData + sizeof(FWUpdateHdr), sizeof(FWSIG));
    fws->SIG = FWUPDATEROM;

    //Not expect client
    if (memcmp(FWUVerifyCode, hdr->auth, 16) != 0 || FWUAuthIP != FWUInIP || FWUAuthPort != FWUInPort || FWUWAddr == 0)
    {
        sendPkt = allocFWUPkt(0);
        makeFWUHdr(sendPkt, FW_ABORT, 0, 0);
        udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));
        return;
    }
    sendPkt = allocFWUPkt(0);
    makeFWUHdr(sendPkt, FW_FIN, 0, 0);
    udpSend_F(pcb, sendPkt, sizeof(FWUpdateHdr));

    ///OSTimeDly(1);

    OS_ENTER_CRITICAL();

    rdid = (spi_read_id(SSI_MAP)  & 0x00ff0000) >> 16;
    if (rdid == 0x1f)
        spi_se_unprotect(SSI_MAP, (DPCONF_ADDR & 0x00FFFFFF));
    spi_se_erase(SSI_MAP, (DPCONF_ADDR & 0x00FFFFFF) );

    //handling 4k at most for sector 0
    for (i = 0 ; i < 64; i++)
        spi_write(SSI_MAP, (DPCONF_ADDR & 0x00FFFFFF) + i * 64, ((INT8U *) fws) + i*64 , 64 );

    OS_EXIT_CRITICAL();

#if 0

    //Sig
    spi_write_word(SSI_MAP, 0x000000, FWUPDATE);
    //CRC32
    spi_write_word(SSI_MAP, 0x000010, fws.CRC32);
    //CRC32Start
    spi_write_word(SSI_MAP, 0x000014, fws.CRC32Start);
    //CRC32Size
    spi_write_word(SSI_MAP, 0x000018, fws.CRC32Size);
    //FWStartAddr
    spi_write_word(SSI_MAP, 0x00001C, fws.FWStartAddr);
    //FWSize
    spi_write_word(SSI_MAP, 0x000020, fws.FWSize);
    //ROMStartAddr
    spi_write_word(SSI_MAP, 0x000024, fws.ROMStartAddr);
#endif
    DisableDrvWaitOOB_F();

    OSTimeDly(1);

    bsp_irq_disable();

    //clear firmware update mode bit
    bsp_bits_set(MAC_OOBREG, 0, BIT_FWMODE, 1);
    _boot();
    //If reset too fast, tx buffer will be clear
    //cpu_reset();
}

void fwUpdate_R(void* pData, int rxLen, struct tcp_pcb* pcb)
{
    FWUpdateHdr *hdr;

    hdr = (FWUpdateHdr*) pData;

    switch (hdr->opCode)
    {
	case FW_ICINFO:
		fwICInfo(pData, pcb);
		break;
    case FW_AUTH_REQ:
        fwAuth(pData, pcb);
        break;
    case FW_VERIFY_REQ:
        fwVerify(pData, pcb);
        break;
    case FW_WADDR:
        fwWAddr(pData, rxLen, pcb);
        break;
    case FW_WRQ:
		fwWrite(pData, rxLen, pcb);		
        break;
    case FW_WCFGRQ:
        fwCfgWrite(pData, pcb);
        break;
    case FW_FIN:
        fwUpdateFinish(pData, pcb);
        break;
    }
    return;
}


void fwUpdateInit_R()
{
    unsigned char err = 0;
    RTSkt* s;

    FWUInIP = 0;
    FWUInPort = 0;
    Sector = 0;
	
    s = rtSkt_F(IPv4, IP_PROTO_UDP);
    rtSktEnableIBPortFilter_F(s);
    EnableDrvWaitOOB_F();
    s->pcb->apRx = fwUpdate_R;
    rtSktUDPBind_F(s, FWUpdateSrvPt);

    if (s->rxSem)
    {
        OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
    }
}

