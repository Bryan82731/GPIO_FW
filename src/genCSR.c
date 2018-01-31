#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include "asn1.h"
#include "lib.h"
#include "x509.h"
#include "sha1.h"
#include "tls.h"
#include "bsp.h"
#include "b64.h"

extern INT8U aeskey[];

extern int getRSAKey(int index, char* buf);

#define KEY_LEN 128
#if 0
#define RSA_N   "cb4eacf98357f669d23229b4bcade6f726218933304340a335d9de2601d6b4f0bc0a1955993bf14c9160b6fd74348d5ac762eccef2d602ce5732f4358c71a06d652ae780ae2959cf3673f87c4a7390fc3028d5467d35a44ec99f907be2092136c5a8ec85829a32b4913bc1d64f9fd1f86f68f41dd2069132cc9a48fdcd987f2f"

#define RSA_E   "010001"

#define RSA_D	"a3f596303f898ff3358e38ee467fe4d58acddeed064e2671b97b33d9e8ffb3bab59b5b651a082a88ba718a020b565a6d4c816e4353b9de57742166ab4cd1523396be814c67d5597ddda444fbe5e1319ccc1b4902aeb96fe118306ba2aba4fda7ad713ddb73bb4bccf84723d76ee900c9395dc272c6ef1740aee5656986e54a09"

#define RSA_P   "f02c5735937e037a836cc04f23755dc0a08d4a3355110127ca031b360c3b54b7713e9b29cd9ce2a89106773b0ce1db27381c0ac485852cdda195b1813aab80b3"

#define RSA_Q   "d8b46ad19633c554c438efaaa1c24d2f515175a09ef62179ee3fc6b6890503c14a2be30bd3f4bca4fb3e304753a763b4a48b2866879f5bc7aa6c98ecf08c8d95"

#define RSA_DP   "8d552a84c6a4b31b575799db783c4acd5e2d86eaaa36757d9aad35481aee972dd4204962d2326e574ff0841d65b94b23cb2b05531c57804d741708fbc930bf6d"

#define RSA_DQ   "934c976b1d20d59a17c2e7cde1241498fc3a55ad373ff06353f9cfd5956f6e2cfd80b554fdded637827bb40f57a2691889dbf93b3026b7191213bd23192340bd"

#define RSA_QP   "5a109ff7e88793338747228a398961d431d4c037d2129fa1959620f47d373f6b2b690cd4c6058363f0c863adcabd6efc9f2f3addb685cfba27cf6d4c6068095e"


static unsigned char RSAN[] = {0xcb,0x4e,0xac,0xf9,0x83,0x57,0xf6,0x69,0xd2,0x32,0x29,0xb4,0xbc,0xad,
0xe6,0xf7,0x26,0x21,0x89,0x33,0x30,0x43,0x40,0xa3,0x35,0xd9,0xde,0x26,0x01,0xd6,0xb4,0xf0,0xbc,0x0a,0x19,
0x55,0x99,0x3b,0xf1,0x4c,0x91,0x60,0xb6,0xfd,0x74,0x34,0x8d,0x5a,0xc7,0x62,0xec,0xce,0xf2,0xd6,0x02,0xce,
0x57,0x32,0xf4,0x35,0x8c,0x71,0xa0,0x6d,0x65,0x2a,0xe7,0x80,0xae,0x29,0x59,0xcf,0x36,0x73,0xf8,0x7c,0x4a,
0x73,0x90,0xfc,0x30,0x28,0xd5,0x46,0x7d,0x35,0xa4,0x4e,0xc9,0x9f,0x90,0x7b,0xe2,0x09,0x21,0x36,0xc5,0xa8,
0xec,0x85,0x82,0x9a,0x32,0xb4,0x91,0x3b,0xc1,0xd6,0x4f,0x9f,0xd1,0xf8,0x6f,0x68,0xf4,0x1d,0xd2,0x06,0x91,
0x32,0xcc,0x9a,0x48,0xfd,0xcd,0x98,0x7f,0x2f};
#endif
unsigned char *csrInfoSeq = NULL;
unsigned char *csrInfoSeqLongForm = NULL;
unsigned char *csrInfoSeqLongFormLen1 = NULL;
unsigned char *csrInfoSeqLongFormLen2 = NULL;
unsigned char *totalSeq = NULL;
unsigned char *totalSeqLongForm = NULL;
unsigned char *totalSeqLongFormLen1 = NULL;
unsigned char *totalSeqLongFormLen2 = NULL;
unsigned char *pubModContent = NULL;
unsigned char *glo_csr_ptr;
unsigned char *glo_csr_head;
unsigned char *glo_tbs_tail;
const unsigned char *csr;
const unsigned char id_at_CN[] = {0x55, 0x04, 0x03};
const unsigned char id_at_O[] = {0x55, 0x04, 0x0a};
const unsigned char id_at_C[] = {0x55, 0x04, 0x06};
const unsigned char id_at_OU[] = {0x55, 0x04, 0x0b};
const unsigned char id_at_L[] = {0x55, 0x04, 0x07};
rsa_context rsa;

#if CONFIG_VERSION < 6


void cert_write(INT8U *buf, INT32S len)
{
    INT32U i;
    OS_CPU_SR cpu_sr = 0;

    struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
    //INT32U flashaddr = (INT32U ) (SYSTEM_CERT_DATA - DPCONF_ADDR_CACHED);
    INT32U flashaddr = (INT32U ) (SYSTEM_CERT_DATA & 0x00FFFFFF);

    OS_ENTER_CRITICAL();
    spi_se_erase(ssi_map, flashaddr);
    for (i = 0 ; i < ifloor(len, 64) ; i++)
        spi_write(ssi_map, flashaddr+i*64, buf+i*64, 64);
    OS_EXIT_CRITICAL();
}


void generateCipher(aes_context *encrypt, char* inData, unsigned char* pBuffer, int* length)
{
    unsigned char* buf;
    int dataLen = 0;
    int padLen = 0;
    int i = 0;
#define aesivE  "!@#$%^&*()!@#$%^"	
   // INT8U aesivE[] = {"REALTEKCNSD7SWDP"};
    unsigned char ivE[16];

    memcpy(ivE, aesivE, sizeof(aesivE));

    //Encryption
    dataLen = (int)strlen(inData);

    //IV, InLen
    padLen = checkPadLen(16, dataLen);

    // output data length and padding length
    sprintf(pBuffer+*length, "%d\n", dataLen + padLen + 1);
    *length+= (DigitLength(dataLen+padLen+1) + 1);

    buf = (unsigned char*)malloc(dataLen + padLen + 1);

    memcpy(buf, inData, dataLen);

    for ( i = 0; i <= padLen; i++ )
    {
        buf[dataLen + i] = (unsigned char) padLen;
    }

    aes_crypt_cbc_sw( encrypt,
                      0, dataLen + padLen + 1,
                      ivE, buf, buf);

    for ( i = 0; i < dataLen + padLen + 1; i++ )
    {
        *(pBuffer+*length)=buf[i];
        *length+=1;
    }

    free(buf);
}


// parameters
// [IN]pBuffer: the buffer need to decrypt in Sector 15
// [OUT]pCert: the certificate
// [OUT]pctx: the RSA context
int RtkDecryptCredential(unsigned char* pBuffer, unsigned char* pCert, rsa_context* pctx)
{
    unsigned int len, len1, offset;
    unsigned int i;
    aes_context decrypt;
    unsigned char *tmp;
    unsigned char ivE[20];

#define RSA_HASH_STRING "CNSD7@Realtek&2379"

    const INT8U aesivE2[20]=  "!@#$%^&*()!@#$%^";
#if 0
    unsigned char sha1Buf[16] = {0xfd, 0x4b, 0x14, 0x3f, 0xe3, 0xbf, 0x4e, 0x9e
		, 0xb4, 0xde, 0x3c, 0x22, 0x51, 0x5c, 0x32, 0x39};
#endif
    unsigned char *msg = malloc(sizeof(RSA_HASH_STRING));
    //sha1_context sha1;
    sha1_context *sha1 = malloc(sizeof(sha1_context));
    unsigned char sha1Buf[20];

    memset(msg, 0, sizeof(RSA_HASH_STRING));
    memcpy(msg, RSA_HASH_STRING, strlen(RSA_HASH_STRING));
    //Generate Key

    sha1_starts( sha1 );
    sha1_update( sha1, msg, strlen(RSA_HASH_STRING));
    sha1_finish( sha1, sha1Buf);

    free(sha1);

    aes_setkey_dec_sw( (aes_context *) &decrypt, sha1Buf + 4, 128);
    free(msg);

    tmp = malloc(512);


    sscanf(pBuffer,"%d\n",&len);
    len1 = DigitLength(len);
    offset=len1+1;

    for (i=0;i<len;i++)
        pCert[i]=pBuffer[i+offset];
    offset+=len;

    // RSA_N
    sscanf(pBuffer+offset,"%d\n",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE2, sizeof(aesivE2));
    aes_crypt_cbc_sw( &decrypt, AES_DECRYPT, len, ivE, tmp, tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1;i<=tmp[len-1];i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    //mpi_init(&pctx->N);

    mpi_read_string(&pctx->N,16,tmp);
    offset+=len;

    // RSA_E
    sscanf(pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1 + 1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE2, sizeof(aesivE2));
    aes_crypt_cbc( &decrypt, AES_DECRYPT, len, ivE, tmp, tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1;i<=tmp[len-1];i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    //mpi_init(&pctx->E);
    mpi_read_string(&pctx->E,16,tmp);
    offset+=len;

    // RSA_D
    sscanf(pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1 + 1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE2, sizeof(aesivE2));
    aes_crypt_cbc( &decrypt, AES_DECRYPT, len, ivE, tmp, tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1;i<=tmp[len-1];i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->D,16,tmp);
    offset+=len;

    // RSA_P
    sscanf(pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE2, sizeof(aesivE2));
    aes_crypt_cbc( &decrypt, AES_DECRYPT, len, ivE, tmp, tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1;i<=tmp[len-1];i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->P,16,tmp);
    offset+=len;

    // RSA_Q
    sscanf(pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE2, sizeof(aesivE2));
    aes_crypt_cbc( &decrypt, AES_DECRYPT, len, ivE, tmp, tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1;i<=tmp[len-1];i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->Q,16,tmp);
    offset+=len;


    // RSA_DP
    sscanf(pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE2, sizeof(aesivE2));
    aes_crypt_cbc( &decrypt, AES_DECRYPT, len, ivE, tmp, tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1;i<=tmp[len-1];i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->DP,16,tmp);
    offset+=len;

    // RSA_DQ
    sscanf(pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE2, sizeof(aesivE2));
    aes_crypt_cbc( &decrypt, AES_DECRYPT, len, ivE, tmp, tmp);

    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1;i<=tmp[len-1];i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->DQ,16,tmp);
    offset+=len;

    // RSA_QInv
    sscanf(pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE2, sizeof(aesivE2));
    aes_crypt_cbc( &decrypt, AES_DECRYPT, len, ivE, tmp, tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1;i<=tmp[len-1];i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->QP,16,tmp);
    offset+=len;
    free(tmp);

    return 0;
}

// pctx : RSA context
// pCert 	 : certificate has to be output
// pBuf : The output data buffer
// pBufLen : the output data length. the minimal length is required 1312.
int RtkOutCredential(rsa_context* pctx, unsigned char* pCert, unsigned char* pBuf, int* pBufLen)//, unsigned char* pw, int pwlen)
{
    int certLen;
    //unsigned char key[20];
    ///char buffer[300];
    unsigned char *buffer;
    int ret = 0;
    int bufLen;
    aes_context encrypt;
    //int length;
    int i;
    int offset;

    /*if (*pBufLen < 1312) {
    	*pBufLen=1312;
    	return CERT_BUFFER_NOT_ENOUGH;
    }
    */
#define RSA_HASH_STRING "CNSD7@Realtek&2379"


    unsigned char *msg = malloc(sizeof(RSA_HASH_STRING));
    //sha1_context sha1;
    sha1_context *sha1 = malloc(sizeof(sha1_context));
    unsigned char sha1Buf[20];

    memset(msg, 0, sizeof(RSA_HASH_STRING));
    memcpy(msg, RSA_HASH_STRING, strlen(RSA_HASH_STRING));
    //Generate Key

    sha1_starts( sha1 );
    sha1_update( sha1, msg, strlen(RSA_HASH_STRING));
    sha1_finish( sha1, sha1Buf);

    free(sha1);

    aes_setkey_enc_sw( (aes_context *) &encrypt, sha1Buf + 4, 128);
    free(msg);
//    memcpy(hk->AES_Key, sha1Buf + 4, 16);
//    memcpy(hk->IV,ivD, 16);

	

    buffer = malloc(512);
    if(buffer == NULL)
		ret = -1;
    certLen=(unsigned long)(pCert[2])*256+(unsigned long)(pCert[3])+4;
    sprintf(pBuf,"%d\n",certLen);
    *pBufLen=1312 + certLen + strlen(pBuf);

    // output certificate data
    offset= strlen(pBuf);
    for (i=0;i<certLen;i++) {
        *(pBuf+offset)=pCert[i];
        offset++;
    }

    //sha1(pw,pwlen,key);
    //sha1(pw, pwlen, aeskey);
    

    // RSA_N
    bufLen=300;
    memset(buffer, 0, 300);
    ret = mpi_write_string(&pctx->N, 16, buffer, &bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    // RSA_E
    bufLen=300;
    memset(buffer, 0, 300);
    ret = mpi_write_string(&pctx->E, 16, buffer, &bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    // RSA_D
    bufLen=300;
    memset(buffer, 0, 300);
    ret = mpi_write_string(&pctx->D, 16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    // RSA_P
    bufLen=300;
    memset(buffer, 0, 300);
    ret = mpi_write_string(&pctx->P, 16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    //RSA_Q
    bufLen=300;
    memset(buffer, 0, 300);
    ret = mpi_write_string(&pctx->Q, 16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    //RSA_DP
    bufLen=300;
    memset(buffer, 0, 300);
    ret = mpi_write_string(&pctx->DP,16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    // RSA_DQ
    bufLen=300;
    memset(buffer, 0, 300);
    ret = mpi_write_string(&pctx->DQ,16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    // RSA_QP
    bufLen=300;
    memset(buffer, 0, 300);
    ret = mpi_write_string(&pctx->QP, 16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    free(buffer);
    return ret;
}

#endif

INT8S getRSACtx(rsa_context *ctx)
{
#define MAX_RSA_KEY_SIZE 260
	int ret = 0;
	unsigned char *tmp= malloc(MAX_RSA_KEY_SIZE);
	memset( ctx, 0, sizeof( rsa_context ) );
	ctx->len = KEY_LEN;
	getRSAKey(0, tmp);
	ret = mpi_read_string( &ctx->N , 16, tmp  );
	memset(tmp, 0, MAX_RSA_KEY_SIZE);
	getRSAKey(1, tmp);
	ret = mpi_read_string( &ctx->E , 16, tmp  );
	memset(tmp, 0, MAX_RSA_KEY_SIZE);
	getRSAKey(2, tmp);
	ret = mpi_read_string( &ctx->D , 16, tmp  );
	memset(tmp, 0, MAX_RSA_KEY_SIZE);
	getRSAKey(3, tmp);
	ret = mpi_read_string( &ctx->P , 16, tmp  );
	memset(tmp, 0, MAX_RSA_KEY_SIZE);
	getRSAKey(4, tmp);
	ret = mpi_read_string( &ctx->Q , 16, tmp  );
	memset(tmp, 0, MAX_RSA_KEY_SIZE);
	getRSAKey(5, tmp);
	ret = mpi_read_string( &ctx->DP, 16, tmp );
	memset(tmp, 0, MAX_RSA_KEY_SIZE);
	getRSAKey(6, tmp);
	ret = mpi_read_string( &ctx->DQ, 16, tmp );
	memset(tmp, 0, MAX_RSA_KEY_SIZE);
	getRSAKey(7, tmp);
	ret = mpi_read_string( &ctx->QP, 16, tmp );
	free (tmp);
	if ( rsa_check_pubkey(  ctx ) != 0 ||
		rsa_check_privkey( ctx ) != 0 )
	{
		return( -1 );
	}	
	return 0;
}

void initCsr()
{
	csr = (unsigned char*)malloc(5120 * sizeof(unsigned char));
	memset((unsigned char*)csr, 0, 5120);

	//Init RSA
	memset( &rsa, 0, sizeof( rsa_context ) );
	rsa.len = KEY_LEN;
		
	glo_csr_head = glo_csr_ptr = (unsigned char*)csr;
	
	totalSeq = glo_csr_head;
	*totalSeq = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	totalSeqLongForm = totalSeq + 1;
	*totalSeqLongForm = 0x82;
	totalSeqLongFormLen1 = totalSeqLongForm + 1;
	totalSeqLongFormLen2 = totalSeqLongFormLen1 + 1;
	glo_csr_ptr = totalSeqLongFormLen2 + 1;

}

int set_asn1_CertificationRequestInfo_Init(unsigned char version)
{
	unsigned char *verInt = NULL;
	unsigned char *verLen = NULL;
	unsigned char *verContent = NULL;
	csrInfoSeq = glo_csr_ptr;
	*csrInfoSeq = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	csrInfoSeqLongForm = csrInfoSeq + 1;
	*csrInfoSeqLongForm = 0x82;
	csrInfoSeqLongFormLen1 = csrInfoSeqLongForm + 1;
	csrInfoSeqLongFormLen2 = csrInfoSeqLongFormLen1 + 1;
	verInt = csrInfoSeqLongFormLen2 + 1;
	*verInt = ASN1_INTEGER;
	verLen = verInt + 1;
	*verLen = 1;
	verContent = verLen + 1;
	*verContent = version; //version = 0
	glo_csr_ptr = verContent + 1;
	return 0;
}

int set_asn1_subject(Subject *sub)
{
	unsigned char tmp ;
	unsigned char* totalLen = NULL;
	unsigned char* uuidSet = NULL;
	unsigned char* uuidSetLen = NULL;
	unsigned char* uuidSeq = NULL;
	unsigned char* uuidLen = NULL;
	unsigned char* uuidOID = NULL;
	unsigned char* uuidOIDLen = NULL;	
	unsigned char* uuidOIDContent = NULL;
	unsigned char* uuidPrintStr = NULL;
	unsigned char* uuidPrintStrLen = NULL;
	unsigned char* uuidPrintStrContent = NULL;
	unsigned char tmpchar = 0;

	//int len = strlen(CN1)+strlen(CN2);
	(*glo_csr_ptr) = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	totalLen = glo_csr_ptr + 1;
	uuidSet = totalLen + 1;
	(*uuidSet) = ASN1_SET | ASN1_CONSTRUCTED;
	uuidSetLen = uuidSet + 1;
	uuidSeq = uuidSetLen + 1;
	(*uuidSeq) = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	uuidLen = uuidSeq + 1;	
	uuidOID = uuidLen + 1;
	(*uuidOID) = ASN1_OID;
	uuidOIDLen = uuidOID + 1;
	(*uuidOIDLen) = 0x03;
	uuidOIDContent = uuidOIDLen + 1;
	memcpy(uuidOIDContent, id_at_C, 3);
	uuidPrintStr = uuidOIDLen + (*uuidOIDLen) + 1;
	(*uuidPrintStr) = ASN1_PRINTABLE_STRING;
	uuidPrintStrLen = uuidPrintStr + 1;
	(*uuidPrintStrLen) = strlen(sub->Country);
	uuidPrintStrContent = uuidPrintStrLen + 1;
	memcpy(uuidPrintStrContent, sub->Country, strlen(sub->Country));
	uuidSet = uuidPrintStrContent + strlen(sub->Country); // second, Organization	
	(*uuidSetLen) = uuidSet - uuidSetLen - 1;
	(*uuidLen) = (*uuidSetLen) - 2;

#if 0
	(unsigned char)(*uuidSet) = ASN1_SET | ASN1_CONSTRUCTED;
	uuidSetLen = uuidSet + 1;
	uuidSeq = uuidSetLen + 1;
	(unsigned char)(*uuidSeq) = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	uuidLen = uuidSeq + 1;
	uuidOID = uuidLen + 1;
	(unsigned char)(*uuidOID) = ASN1_OID;
	uuidOIDLen = uuidOID + 1;
	(unsigned char)(*uuidOIDLen) = 0x03;
	uuidOIDContent = uuidOIDLen + 1;
	memcpy(uuidOIDContent, id_at_L, 3);
	uuidPrintStr = uuidOIDLen + (unsigned char)(*uuidOIDLen) + 1;
	(unsigned char)(*uuidPrintStr) = ASN1_PRINTABLE_STRING;
	uuidPrintStrLen = uuidPrintStr + 1;
	(unsigned char)(*uuidPrintStrLen) = strlen(L);
	uuidPrintStrContent = uuidPrintStrLen + 1;
	memcpy(uuidPrintStrContent, L, strlen(L));
	uuidSet = uuidPrintStrContent + strlen(L); // third, Organization Unit
	(unsigned char)(*uuidSetLen) = uuidSet - uuidSetLen - 1;
	(unsigned char)(*uuidLen) = (unsigned char)(*uuidSetLen) - 2;
#endif

	(*uuidSet) = ASN1_SET | ASN1_CONSTRUCTED;
	uuidSetLen = uuidSet + 1;
	uuidSeq = uuidSetLen + 1;
	(*uuidSeq) = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	uuidLen = uuidSeq + 1;
	uuidOID = uuidLen + 1;
	(*uuidOID) = ASN1_OID;
	uuidOIDLen = uuidOID + 1;
	(*uuidOIDLen) = 0x03;
	uuidOIDContent = uuidOIDLen + 1;
	memcpy(uuidOIDContent, id_at_O, 3);
	uuidPrintStr = uuidOIDLen + (*uuidOIDLen) + 1;
	(*uuidPrintStr) = ASN1_PRINTABLE_STRING;
	uuidPrintStrLen = uuidPrintStr + 1;
	(*uuidPrintStrLen) = strlen(sub->Organization);
	uuidPrintStrContent = uuidPrintStrLen + 1;
	memcpy(uuidPrintStrContent, sub->Organization, strlen(sub->Organization));
	uuidSet = uuidPrintStrContent + strlen(sub->Organization); // fourth, Organization Unit
	(*uuidSetLen) = uuidSet - uuidSetLen - 1;
	(*uuidLen) = (*uuidSetLen) - 2;

#if 0
	(unsigned char)(*uuidSet) = ASN1_SET | ASN1_CONSTRUCTED;
	uuidSetLen = uuidSet + 1;
	uuidSeq = uuidSetLen + 1;
	(unsigned char)(*uuidSeq) = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	uuidLen = uuidSeq + 1;
	uuidOID = uuidLen + 1;
	(unsigned char)(*uuidOID) = ASN1_OID;
	uuidOIDLen = uuidOID + 1;
	(unsigned char)(*uuidOIDLen) = 0x03;
	uuidOIDContent = uuidOIDLen + 1;
	memcpy(uuidOIDContent, id_at_OU, 3);
	uuidPrintStr = uuidOIDLen + (unsigned char)(*uuidOIDLen) + 1;
	(unsigned char)(*uuidPrintStr) = ASN1_PRINTABLE_STRING;
	uuidPrintStrLen = uuidPrintStr + 1;
	(unsigned char)(*uuidPrintStrLen) = strlen(OU);
	uuidPrintStrContent = uuidPrintStrLen + 1;
	memcpy(uuidPrintStrContent, OU, strlen(OU));
	uuidSet = uuidPrintStrContent + strlen(OU); // fourth, Organization Unit
	(unsigned char)(*uuidSetLen) = uuidSet - uuidSetLen - 1;
	(unsigned char)(*uuidLen) = (unsigned char)(*uuidSetLen) - 2;
#endif

#if 0
	(unsigned char)(*uuidSet) = ASN1_SET | ASN1_CONSTRUCTED;
	uuidSetLen = uuidSet + 1;
	uuidSeq = uuidSetLen + 1;
	(unsigned char)(*uuidSeq) = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	uuidLen = uuidSeq + 1;
	uuidOID = uuidLen + 1;
	(unsigned char)(*uuidOID) = ASN1_OID;
	uuidOIDLen = uuidOID + 1;
	(unsigned char)(*uuidOIDLen) = 0x03;
	uuidOIDContent = uuidOIDLen + 1;
	memcpy(uuidOIDContent, id_at_CN, 3);
	uuidPrintStr = uuidOIDLen + (unsigned char)(*uuidOIDLen) + 1;
	(unsigned char)(*uuidPrintStr) = ASN1_PRINTABLE_STRING;
	uuidPrintStrLen = uuidPrintStr + 1;
	(unsigned char)(*uuidPrintStrLen) = 21;
	uuidPrintStrContent = uuidPrintStrLen + 1;
	memcpy(uuidPrintStrContent, sub->CommonName1, 21 );
	uuidSet = uuidPrintStrContent + 21; // fourth, Organization Unit
	(unsigned char)(*uuidSetLen) = uuidSet - uuidSetLen - 1;
	(unsigned char)(*uuidLen) = (unsigned char)(*uuidSetLen) - 2;
#endif

	(*uuidSet) = ASN1_SET | ASN1_CONSTRUCTED;
	uuidSetLen = uuidSet + 1;
	uuidSeq = uuidSetLen + 1;
	(*uuidSeq) = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	uuidLen = uuidSeq + 1;
	uuidOID = uuidLen + 1;
	(*uuidOID) = ASN1_OID;
	uuidOIDLen = uuidOID + 1;
	(*uuidOIDLen) = 0x03;
	uuidOIDContent = uuidOIDLen + 1;
	memcpy(uuidOIDContent, id_at_CN, 3);
	uuidPrintStr = uuidOIDLen + (*uuidOIDLen) + 1;
	(*uuidPrintStr) = ASN1_PRINTABLE_STRING;
	uuidPrintStrLen = uuidPrintStr + 1;
	(*uuidPrintStrLen) = strlen(sub->CommonName1);
	uuidPrintStrContent = uuidPrintStrLen + 1;
	memcpy(uuidPrintStrContent, sub->CommonName1, strlen(sub->CommonName1));
	uuidSet = uuidPrintStrContent + strlen(sub->CommonName1); // fourth, Organization Unit
	(*uuidSetLen) = uuidSet - uuidSetLen - 1;
	(*uuidLen) = (*uuidSetLen) - 2;
	
	glo_csr_ptr = uuidPrintStrContent + strlen(sub->CommonName1);

	(*totalLen) = uuidSet - totalLen - 1;
	return 0;
}

int set_asn1_subjectPKInfo(int pubLen)//512, 1024, 2048
{
	int len = 0, i = 0;
	unsigned char rsaEncryptionId[] = {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01};
	unsigned char *totalLongForm = NULL;
	unsigned char *totalLongFormLen = NULL; // > 127, 0x81, len(1 byte)
	unsigned char *oidSeq = NULL;
	unsigned char *oidSeqLen = NULL;
	unsigned char *oid = NULL;
	unsigned char *oidLen = NULL;
	unsigned char *oidContent = NULL;
	unsigned char paddingNULL[] = {0x05, 0x00};
	
	unsigned char *pubBS = NULL; // bit string
	unsigned char *pubBSLongForm = NULL;
	unsigned char *pubBSLongFormLen = NULL;

	unsigned char *paddingBSUnusedByte = NULL;
	
	unsigned char *pubSeq = NULL;
	unsigned char *pubSeqLongForm = NULL;
	unsigned char *pubSeqLongFormLen = NULL;


	unsigned char *pubInt = NULL;
	unsigned char *pubIntLongForm = NULL;
	unsigned char *pubIntLongFormLen = NULL;

	unsigned char *paddingIntLeadingByte = NULL;

	
	unsigned char *pubExpInt = NULL;
	unsigned char *pubExpIntLen = NULL;
	unsigned char *pubExpIntContent = NULL;
	unsigned char pubExp[] = {0x01, 0x00, 0x01};

	//add algorithm identifier
	
	(*glo_csr_ptr) = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	totalLongForm = glo_csr_ptr + 1;
	(*totalLongForm) = 0x81;
	totalLongFormLen = totalLongForm + 1;
	oidSeq = totalLongFormLen + 1;
	(*oidSeq) = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	
	oidSeqLen = oidSeq + 1;
	
	oid = oidSeqLen + 1;
	(*oid) = ASN1_OID;
	oidLen = oid + 1;
	oidContent = oidLen + 1;
	memcpy(oidContent, rsaEncryptionId, sizeof(rsaEncryptionId));
	*oidLen = sizeof(rsaEncryptionId);

	glo_csr_ptr = oidContent + sizeof(rsaEncryptionId);

	memcpy(glo_csr_ptr, paddingNULL, sizeof(paddingNULL));
	*oidSeqLen = glo_csr_ptr + sizeof(paddingNULL) - 1 - oidSeqLen;
	
	glo_csr_ptr += sizeof(paddingNULL);

	//BIT STRING for SubjectPublicKeyInfo
	
	pubBS = glo_csr_ptr;

	(*pubBS) = ASN1_BIT_STRING;

	if(pubLen == 1024)
	{
		pubBSLongForm = pubBS + 1;
		*pubBSLongForm = 0x81; // len = 1 byte
		pubBSLongFormLen = pubBSLongForm + 1;
		//(unsigned char*)pubLongFormLen =
		paddingBSUnusedByte = pubBSLongFormLen + 1;
		*paddingBSUnusedByte = 0x00;
		pubSeq = paddingBSUnusedByte + 1;
		*pubSeq = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
		pubSeqLongForm = pubSeq + 1;
		*pubSeqLongForm = 0x81;
		pubSeqLongFormLen = pubSeqLongForm + 1;
		pubInt = pubSeqLongFormLen + 1;
		*pubInt = ASN1_INTEGER;
		pubIntLongForm = pubInt + 1;
		*pubIntLongForm = 0x81;
		pubIntLongFormLen = pubIntLongForm + 1;
		paddingIntLeadingByte = pubIntLongFormLen + 1;
		*paddingIntLeadingByte = 0x00;
		pubModContent = paddingIntLeadingByte + 1;
	}

	if(pubLen == 1024)
	{
		if( 1 == getRSACtx(&rsa) )
			return -1;
		
		mpi_write_binary(&rsa.N,pubModContent,pubLen/8);
		//memcpy(pubModContent, rsa.N.p, pubLen/8);
		pubExpInt = pubModContent + pubLen/8;
		glo_csr_ptr = pubExpInt;
		*pubIntLongFormLen = glo_csr_ptr - 1 - pubIntLongFormLen;
		*pubExpInt = ASN1_INTEGER;
		pubExpIntLen = pubExpInt + 1;
		pubExpIntContent = pubExpIntLen + 1;
		memcpy(pubExpIntContent, pubExp, sizeof(pubExp));
		*pubExpIntLen = sizeof(pubExp);
		glo_csr_ptr = pubExpIntContent + sizeof(pubExp);
		*pubSeqLongFormLen = glo_csr_ptr - 1 - pubSeqLongFormLen;
		*pubBSLongFormLen = glo_csr_ptr - 1 - pubBSLongFormLen;
		*totalLongForm = 0x81;
		*totalLongFormLen = glo_csr_ptr - 1 - totalLongFormLen;
	}
	

	return 0;
}

int set_asn1_attributes(int pubLen)
{
	unsigned char attributes[] = {
		0xa0,0x60,0x30,0x5e,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x0e,
		0x31,0x51,0x30,0x4f,0x30,0x09,0x06,0x03,0x55,0x1d,0x13,0x04,
		0x02,0x30,0x00,0x30,0x0b,0x06,0x03,0x55,0x1d,0x0f,0x04,0x04,0x03,0x02,0x05,0xa0,
		0x30,0x16,0x06,0x03,0x55,0x1d,0x25,0x01,0x01,0xff,0x04,0x0c,0x30,0x0a,0x06,0x08,
		0x2b,0x06,0x01,0x05,0x05,0x07,0x03,0x01,
		
		0x30,0x1d,0x06,0x03,0x55,0x1d,0x0e,0x04,0x16,0x04,0x14,
		
		0x0e, 0xb3,0xac,0x19,0xf0,0x06,0x26,0x01,0x51,0xce,0x6b,0x9b,
		0xff,0xcf,0x6e,0x5b,0x02,0xe5,0xa1,0x9c,		
	};
	int len = sizeof(attributes);

	unsigned char rsanHash[20];
	sha1(pubModContent, pubLen/8, rsanHash);
	memcpy(attributes + sizeof(attributes) - 20, rsanHash, 20);
	memcpy(glo_csr_ptr, attributes, sizeof(attributes));
	glo_csr_ptr += sizeof(attributes);
	glo_tbs_tail = glo_csr_ptr - 1;

	len = glo_csr_ptr - csrInfoSeqLongFormLen2 - 1;
	*csrInfoSeqLongFormLen1 = len/0x100;
	*csrInfoSeqLongFormLen2 = len%0x100;
	return 0;
}

int set_asn1_sig(int pubLen)
{
	unsigned char sha1WithRSAEncryptionId[] = {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05};	
	unsigned char *oidSeq = NULL;
	unsigned char *oidSeqLen = NULL;
	unsigned char *oid = NULL;
	unsigned char *oidLen = NULL;
	unsigned char *oidContent = NULL;
	
	unsigned char paddingNULL[] = {0x05, 0x00};
	unsigned char *hashBS = NULL;
	unsigned char *hashBSLongForm = NULL;
	unsigned char *hashBSLongFormLen = NULL;
	unsigned char *hashBSContent = NULL;
	unsigned char *paddingBSUnusedByte = NULL;
	unsigned char hash[20];
	unsigned char sig[128];
	
	unsigned char ivE[20];
	int padLen = 0, i;
	int ret = 0, len = 0;
	INT8U rand_parameter[20];

	oidSeq = glo_csr_ptr;
	*oidSeq = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	oidSeqLen = oidSeq + 1;
	oid = oidSeqLen + 1;
	*oid = ASN1_OID;
	oidLen = oid + 1;
	oidContent = oidLen + 1;
	memcpy(oidContent, sha1WithRSAEncryptionId, sizeof(sha1WithRSAEncryptionId));
	(*oidLen) = sizeof(sha1WithRSAEncryptionId);
	glo_csr_ptr = oidContent + sizeof(sha1WithRSAEncryptionId);
	(*oidSeqLen) = glo_csr_ptr - oidSeq;
	
	memcpy(glo_csr_ptr, paddingNULL, sizeof(paddingNULL));
	glo_csr_ptr += sizeof(paddingNULL);

	//signature
	hashBS = glo_csr_ptr;
	*hashBS = ASN1_BIT_STRING;
	hashBSLongForm = hashBS + 1;
	if(pubLen == 1024)
	    *hashBSLongForm = 0x81;
	hashBSLongFormLen = hashBSLongForm + 1;
	*hashBSLongFormLen = pubLen/8 + 1 /*paddingBSUnusedByte = 1 byte*/;
	paddingBSUnusedByte = hashBSLongFormLen + 1;
	*paddingBSUnusedByte = 0x00;
	hashBSContent = paddingBSUnusedByte + 1;
	//glo_csr_ptr = hashBSContent + 1;
	ret = glo_tbs_tail - glo_csr_head + 1; //0x11e
	sha1(glo_csr_head+4, ret-4, hash);


#if 0	
	memset( &rsa, 0, sizeof( rsa_context ) );	
	rsa.len = pubLen/8;	
	mpi_read_string( &rsa.N , 16, RSA_N  );	
	mpi_read_string( &rsa.E , 16, RSA_E  );	
	mpi_read_string( &rsa.D , 16, RSA_D  );	
	mpi_read_string( &rsa.P , 16, RSA_P  );	
	mpi_read_string( &rsa.Q , 16, RSA_Q  );	
	mpi_read_string( &rsa.DP, 16, RSA_DP );	
	mpi_read_string( &rsa.DQ, 16, RSA_DQ );	
	mpi_read_string( &rsa.QP, 16, RSA_QP );
#endif
	ret = rsa_pkcs1_sign(&rsa, RSA_PRIVATE, RSA_SHA1, 20, hash, sig);
	if(ret < 0)
	{
		
		printf( "sign failed\n" );   
		return -1;
	}

	glo_csr_ptr = hashBSContent;
	memcpy(glo_csr_ptr, sig, pubLen/8);	

	if( rsa_pkcs1_verify( &rsa, RSA_PUBLIC, RSA_SHA1, 20,hash, glo_csr_ptr ) != 0 )    
	{       
		printf( "verify failed\n" );        
		return -1;    
	}
	glo_csr_ptr = hashBSContent + sizeof(sig);
	len = glo_csr_ptr - totalSeqLongFormLen2 - 1;	
	*totalSeqLongFormLen1 = len/0x100;
	*totalSeqLongFormLen2 = len%0x100;
	return 0;
}

int destroyCsr()
{
	rsa_free( &rsa );	
	free((unsigned char*)csr);
	return 0;
}


int bin2b64(int *tsize)
{
	int size, size2;
	unsigned char *tmp = NULL;
	size = ((((unsigned char)glo_csr_head[2])<<8) + (unsigned char)glo_csr_head[3] + 4)*2;
	tmp = malloc(size);
	size2 = size/2;
	b64_encode( tmp, &size, glo_csr_head, size2  );
	*tsize = size;
	memcpy((unsigned char*)csr, tmp, size);
	free(tmp);
	return 0;
}

#ifdef csr_self_test
int write_csr()
{
	unsigned char * tmp = NULL;
	int size,size2,i;
	FILE *fp = NULL;	
	size = ((((unsigned char)glo_csr_head[2])<<8) + (unsigned char)glo_csr_head[3] + 4)*2;
	tmp = malloc(size);
	size2 = size/2;
	b64_encode( tmp, &size, glo_csr_head, size2  );

	if((fp = fopen("csr.pem", "w")) == NULL)
	{
		printf("file open error\n");
		return -1;
	}
	fwrite((char*)tmp, 1, size, fp);
	
	fclose(fp);
	free(tmp);
	return 0;
}
unsigned char tbs[]=
{
0x30,0x82,0x01,0x06,0x02,0x01,0x00,0x30,0x5d,0x31,0x0b,0x30
,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x53,0x47,0x31,0x11,0x30,0x0f,0x06,0x03
,0x55,0x04,0x0a,0x13,0x08,0x4d,0x32,0x43,0x72,0x79,0x70,0x74,0x6f,0x31,0x12,0x30
,0x10,0x06,0x03,0x55,0x04,0x03,0x13,0x09,0x6c,0x6f,0x63,0x61,0x6c,0x68,0x6f,0x73
,0x74,0x31,0x27,0x30,0x25,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x01
,0x16,0x18,0x61,0x64,0x6d,0x69,0x6e,0x40,0x73,0x65,0x72,0x76,0x65,0x72,0x2e,0x65
,0x78,0x61,0x6d,0x70,0x6c,0x65,0x2e,0x64,0x6f,0x6d,0x30,0x81,0x9f,0x30,0x0d,0x06
,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x01,0x05,0x00,0x03,0x81,0x8d,0x00
,0x30,0x81,0x89,0x02,0x81,0x81,0x00,0xcb,0x4e,0xac,0xf9,0x83,0x57,0xf6,0x69,0xd2
,0x32,0x29,0xb4,0xbc,0xad,0xe6,0xf7,0x26,0x21,0x89,0x33,0x30,0x43,0x40,0xa3,0x35
,0xd9,0xde,0x26,0x01,0xd6,0xb4,0xf0,0xbc,0x0a,0x19,0x55,0x99,0x3b,0xf1,0x4c,0x91
,0x60,0xb6,0xfd,0x74,0x34,0x8d,0x5a,0xc7,0x62,0xec,0xce,0xf2,0xd6,0x02,0xce,0x57
,0x32,0xf4,0x35,0x8c,0x71,0xa0,0x6d,0x65,0x2a,0xe7,0x80,0xae,0x29,0x59,0xcf,0x36
,0x73,0xf8,0x7c,0x4a,0x73,0x90,0xfc,0x30,0x28,0xd5,0x46,0x7d,0x35,0xa4,0x4e,0xc9
,0x9f,0x90,0x7b,0xe2,0x09,0x21,0x36,0xc5,0xa8,0xec,0x85,0x82,0x9a,0x32,0xb4,0x91
,0x3b,0xc1,0xd6,0x4f,0x9f,0xd1,0xf8,0x6f,0x68,0xf4,0x1d,0xd2,0x06,0x91,0x32,0xcc
,0x9a,0x48,0xfd,0xcd,0x98,0x7f,0x2f,0x02,0x03,0x01,0x00,0x01,0xa0,0x00
};

unsigned char total[]={
 0x30,0x82,0x01,0x9d,  
 0x30,0x82,0x01,0x06,0x02,0x01,0x00,0x30
,0x5d,0x31,0x0b,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02
,0x53,0x47,0x31,0x11,0x30,0x0f,0x06,0x03,0x55,0x04,0x0a,0x13
,0x08,0x4d,0x32,0x43,0x72,0x79,0x70,0x74,0x6f,0x31,0x12,0x30
,0x10,0x06,0x03,0x55,0x04,0x03,0x13,0x09,0x6c,0x6f,0x63,0x61
,0x6c,0x68,0x6f,0x73,0x74,0x31,0x27,0x30,0x25,0x06,0x09,0x2a
,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x01,0x16,0x18,0x61,0x64
,0x6d,0x69,0x6e,0x40,0x73,0x65,0x72,0x76,0x65,0x72,0x2e,0x65
,0x78,0x61,0x6d,0x70,0x6c,0x65,0x2e,0x64,0x6f,0x6d,0x30,0x81
,0x9f,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01
,0x01,0x01,0x05,0x00,0x03,0x81,0x8d,0x00,0x30,0x81,0x89,0x02
,0x81,0x81,0x00, /*0x87*/ 0xaf,0x59,0xd8,0x63,0x54,0x2b,0x96,0x5d,0x6b
,0xb8,0x1f,0xc5,0xaa,0x50,0x91,0xae,0xbe,0x67,0xbe,0xea,0x5d
,0x20,0xdf,0xb7,0xc0,0x5e,0xea,0xf7,0x5e,0x6d,0xbc,0x44,0x28
,0x73,0xbe,0x1b,0x9e,0xee,0x9b,0xf0,0x86,0xdb,0x19,0x13,0x21
,0xcd,0xdc,0xe6,0xbd,0x0e,0x12,0xcc,0x57,0xd5,0x10,0x0b,0x8c
,0x32,0xe4,0xc7,0x7b,0x26,0xcf,0xab,0x9b,0x61,0xed,0x80,0xeb
,0x4c,0xd8,0xb3,0x28,0xac,0x4e,0x06,0x7a,0x84,0xd8,0xa6,0x2e
,0x5f,0xd5,0xf6,0xd8,0x0d,0x4a,0x87,0x8f,0x6c,0xa0,0x92,0x45
,0x83,0xa9,0x0f,0x6f,0xd3,0x14,0x82,0x26,0x41,0x88,0xc9,0x6f
,0xb5,0xbe,0xca,0xdc,0x21,0xdf,0x83,0x0e,0x56,0x6a,0x05,0xa7
,0x2e,0x5f,0x0b,0x6f,0xe7,0x1c,0xbc,0xa6,0x59,0x97,0xef,0x02
,0x03,0x01,0x00,0x01,0xa0,0x00,  /*0x10e*/ 0x30,0x0d,0x06,0x09,0x2a,0x86
,0x48,0x86,0xf7,0x0d,0x01,0x01,0x04,0x05,0x00,0x03,0x81,0x81
,0x00, /*0x121*/  0x7a,0x68,0x46,0x9e,0x58,0x4b,0x9e,0x42,0x66,0x9c,0xbe
,0xc1,0xd8,0xa0,0x40,0x4c,0x23,0x2f,0xfc,0x12,0x96,0xeb,0xe8
,0xf9,0x68,0xed,0xa6,0xf3,0xf4,0x62,0x80,0x4c,0x26,0xee,0x15
,0x30,0xa7,0x99,0x8b,0x8d,0x39,0x47,0xba,0x3c,0xa0,0x4c,0x22
,0x3d,0xd9,0x6b,0xae,0x58,0x8a,0x36,0x49,0xc5,0x98,0x72,0x88
,0x68,0x22,0x93,0x2d,0x17,0x14,0xe7,0xd4,0x9c,0x03,0xa0,0x03
,0x10,0x85,0x94,0xce,0xa9,0x94,0xcc,0xfe,0x42,0xb3,0xa8,0xeb
,0x49,0x1a,0x37,0x34,0xa7,0xe0,0xd5,0xb7,0x74,0xf4,0x3d,0x4a
,0xf6,0xbb,0x10,0x91,0x17,0x3d,0x52,0xbb,0xfd,0x99,0x10,0x48
,0xb2,0xb7,0x9d,0x1a,0x76,0x04,0x08,0xd7,0x91,0x68,0xae,0x51
,0xd7,0x2c,0xe9,0x3a,0x8c,0x27,0x8a,0x75,0xc2};

int self_test()
{
	int ret = 0;
	rsa_context rsa;
	unsigned char sig[128];
	unsigned char hash[16];


	int len;
	len = sizeof(tbs);
	memcpy(total+0x87, RSAN, 128);
	md5(tbs, len, hash);
    memset( &rsa, 0, sizeof( rsa_context ) );
    rsa.len = 128;
    mpi_read_string( &rsa.N , 16, RSA_N  );
    mpi_read_string( &rsa.E , 16, RSA_E  );
    mpi_read_string( &rsa.D , 16, RSA_D  );
    mpi_read_string( &rsa.P , 16, RSA_P  );
    mpi_read_string( &rsa.Q , 16, RSA_Q  );
    mpi_read_string( &rsa.DP, 16, RSA_DP );
    mpi_read_string( &rsa.DQ, 16, RSA_DQ );
    mpi_read_string( &rsa.QP, 16, RSA_QP );
	
	ret = rsa_pkcs1_sign(&rsa, RSA_PRIVATE, RSA_MD5, 16, hash, sig);
	if(ret < 0)
	{
		return -1;
	}
	memcpy(total+0x121, sig, 128);

    if ( rsa_pkcs1_verify( &rsa, RSA_PUBLIC, RSA_MD5, 16,                          hash, sig ) != 0 )    {
        printf( "verify failed\n" );
        return( -1 );
    }
	return 0;
}

int write_csr_test()
{
	unsigned char * tmp = NULL;
	int size,size2,i;
	FILE *fp = NULL;	
	size = ((((unsigned char)total[2])<<8) + (unsigned char)total[3] + 4)*2;
	tmp = malloc(size);
	size2 = size/2;
	b64_encode( tmp, &size, total, size2  );

	if((fp = fopen("csr.pem", "w")) == NULL)
	{
		printf("file open error\n");
		return -1;
	}
	for(i=0;i<size;i++)
		fputc((char)tmp[i], fp);
//	fwrite((char*)tmp, 1, size, fp);

	fclose(fp);
	free(tmp);

	return 0;
}

#endif

