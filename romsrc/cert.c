#include "bsp.h"
#include "sha1.h"
#include "tls.h"
#include "aes.h"
#include <string.h>
#include <stdlib.h>

extern const INT8U certtmp[837];
const INT8U daesmsg[20] = "CNSD7@Realtek&2379";
INT8U  aesmsg[20] _ATTRIBUTE_ROMS;
//INT32U validkey   _ATTRIBUTE_ROMS;
INT8U  aeskey[20] _ATTRIBUTE_ROMS;
INT32U validaeskey _ATTRIBUTE_ROMS;
const INT8U aesivE[20] = "!@#$%^&*()!@#$%^";

void cert_write(INT8U *buf, INT32S len);
#define CERT_BUFFER_NOT_ENOUGH	0x90000052
#define DECRYPTION_ERROR 0x90000053

int RtkRand(void* rndparams)
{
    unsigned char hashvalue[20];
    sha1((unsigned char*)rndparams,20,hashvalue);
    memcpy(rndparams,hashvalue,20);
    return (int)hashvalue[10];
}

int RtkGenerateCertificate(rsa_context* pctx, unsigned char* pCert, int* pLength)
{
    int sig_offset=0x2c5;
    int publicKeyModulus_offset=0x1b7;
    int subjectKID_offset=0x29e;
    int subpubkeyinfo_offset=0x1b0;
    int serial_offset=15;
    unsigned char key[128];
    int keylen;
    unsigned char sig[128];
    int i;
    unsigned char hashValue[20];

    if (*pLength < 837) return CERT_BUFFER_NOT_ENOUGH;
    memcpy(pCert,certtmp,837);
    *pLength=837;

    // serial number
    for (i=0; i<20; i++)
        pCert[serial_offset+i]=(unsigned char)pctx->f_rng(pctx->p_rng);
    *(pCert+serial_offset)&=0x7f;

    // public key info
    keylen=128;
    mpi_write_binary(&pctx->N,key,keylen);
    memcpy(pCert+publicKeyModulus_offset,key,128);

    // subject public key info
    sha1(pCert+subpubkeyinfo_offset,0x08c,hashValue);
    memcpy(pCert+subjectKID_offset,hashValue,20);

    // signature
    // tbsCert hash
    sha1(pCert+4,686,hashValue);
    rsa_pkcs1_sign(pctx,RSA_PRIVATE,RSA_SHA1,20,hashValue,sig);
    memcpy(pCert+sig_offset,sig,128);
    return 0;
}

void generateCipher(aes_context *encrypt, char* inData, unsigned char* pBuffer, int* length)
{
    unsigned char* buf;
    int dataLen = 0;
    int padLen = 0;
    int i = 0;

    unsigned char ivE[20];

    memcpy(ivE, aesivE, sizeof(aesivE));

    //Encryption
    dataLen = (int)strlen(inData);

    //IV, InLen
    padLen = checkPadLen(16, dataLen);

    // output data length and padding length
    sprintf((char *)pBuffer+*length, "%d\n", dataLen + padLen + 1);
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
    char *tmp;
    unsigned char ivE[20];

    tmp = malloc(512);
    aes_setkey_dec_sw( (aes_context *) &decrypt, aeskey+4, 128);

    sscanf((const char *)pBuffer,"%d\n",&len);
    len1 = DigitLength(len);
    offset=len1+1;

    for (i=0; i<len; i++)
        pCert[i]=pBuffer[i+offset];
    offset+=len;

    // RSA_N
    sscanf((const char *)pBuffer+offset,"%d\n",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE, sizeof(aesivE));
    aes_crypt_cbc_sw( &decrypt, AES_DECRYPT, len, ivE, (INT8U *)tmp, (INT8U *)tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1; i<=tmp[len-1]; i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    //mpi_init(&pctx->N);

    mpi_read_string(&pctx->N,16,tmp);
    offset+=len;

    // RSA_E
    sscanf((const char *)pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1 + 1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE, sizeof(aesivE));
    aes_crypt_cbc_sw( &decrypt, AES_DECRYPT, len, ivE, (INT8U *)tmp, (INT8U *)tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1; i<=tmp[len-1]; i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    //mpi_init(&pctx->E);
    mpi_read_string(&pctx->E,16,tmp);
    offset+=len;

    // RSA_D
    sscanf((const char *)pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1 + 1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE, sizeof(aesivE));
    aes_crypt_cbc_sw( &decrypt, AES_DECRYPT, len, ivE, (INT8U *)tmp, (INT8U *)tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1; i<=tmp[len-1]; i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->D,16,tmp);
    offset+=len;

    // RSA_P
    sscanf((const char *)pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE, sizeof(aesivE));
    aes_crypt_cbc_sw( &decrypt, AES_DECRYPT, len, ivE, (INT8U *)tmp, (INT8U *)tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1; i<=tmp[len-1]; i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->P,16,tmp);
    offset+=len;

    // RSA_Q
    sscanf((const char *)pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE, sizeof(aesivE));
    aes_crypt_cbc_sw( &decrypt, AES_DECRYPT, len, ivE, (INT8U *)tmp, (INT8U *)tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1; i<=tmp[len-1]; i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->Q,16,tmp);
    offset+=len;


    // RSA_DP
    sscanf((const char *)pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE, sizeof(aesivE));
    aes_crypt_cbc_sw( &decrypt, AES_DECRYPT, len, ivE, (INT8U *)tmp, (INT8U *)tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1; i<=tmp[len-1]; i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->DP,16,tmp);
    offset+=len;

    // RSA_DQ
    sscanf((const char *)pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE, sizeof(aesivE));
    aes_crypt_cbc_sw( &decrypt, AES_DECRYPT, len, ivE, (INT8U *)tmp, (INT8U *)tmp);

    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1; i<=tmp[len-1]; i++)
        if (tmp[len-1-i] != tmp[len-1]) return DECRYPTION_ERROR;
        else tmp[len-1-i]=0;

    mpi_read_string(&pctx->DQ,16,tmp);
    offset+=len;

    // RSA_QInv
    sscanf((const char *)pBuffer+offset,"%d",&len);
    len1 = DigitLength(len);
    offset+=len1+1;

    memset(tmp,0,300);
    memcpy(tmp, pBuffer+offset,len);
    memcpy(ivE, aesivE, sizeof(aesivE));
    aes_crypt_cbc_sw( &decrypt, AES_DECRYPT, len, ivE, (INT8U *)tmp, (INT8U *)tmp);

    // check the padding
    if (tmp[len-1] > 16) return DECRYPTION_ERROR;
    for (i=1; i<=tmp[len-1]; i++)
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
    char *buffer;
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


    buffer = malloc(512);
    certLen=(unsigned long)(pCert[2])*256+(unsigned long)(pCert[3])+4;
    sprintf((char *)pBuf,"%d\n",certLen);
    *pBufLen=1312 + certLen + strlen((const char *)pBuf);

    // output certificate data
    offset= strlen((const char *)pBuf);
    for (i=0; i<certLen; i++) {
        *(pBuf+offset)=pCert[i];
        offset++;
    }

    //sha1(pw,pwlen,key);
    //sha1(pw, pwlen, aeskey);
    aes_setkey_enc_sw( (aes_context *) &encrypt, aeskey+4, 128);

    // RSA_N
    bufLen=300;
    memset(buffer, 0, 300);
    mpi_write_string(&pctx->N, 16, buffer, &bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    // RSA_E
    bufLen=300;
    memset(buffer, 0, 300);
    mpi_write_string(&pctx->E, 16, buffer, &bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    // RSA_D
    bufLen=300;
    memset(buffer, 0, 300);
    mpi_write_string(&pctx->D, 16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    // RSA_P
    bufLen=300;
    memset(buffer, 0, 300);
    mpi_write_string(&pctx->P, 16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    //RSA_Q
    bufLen=300;
    memset(buffer, 0, 300);
    mpi_write_string(&pctx->Q, 16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    //RSA_DP
    bufLen=300;
    memset(buffer, 0, 300);
    mpi_write_string(&pctx->DP,16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    // RSA_DQ
    bufLen=300;
    memset(buffer, 0, 300);
    mpi_write_string(&pctx->DQ,16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    // RSA_QP
    bufLen=300;
    memset(buffer, 0, 300);
    mpi_write_string(&pctx->QP, 16, buffer,&bufLen);
    generateCipher(&encrypt, buffer, pBuf, &offset);

    free(buffer);
    return 0;
}

void init_cert()
{
    rsa_context rsactx;
    //unsigned char cert[837];
    //unsigned char buffer[1400];
    //unsigned char msg[] = "CNSD7@Realtek&2379";
    int certLen=837;
    INT8U i = 0;
    int bufferLen= 4096;
    unsigned char *buffer;

    /* random number seed. Please set these values by acquiring time() */
    //unsigned char rand_parameter[20]={0x01,0x23,0x45,0x67,0x89,0xab,0xcd, 0xef, 0xfe,0xde,0xba,0x98,0x76,0x54,0x32,0x10, 0x33,0xcc,0x00,0xff};
    INT8U rand_parameter[20];
    INT8U *cert;

    for (i = 0; i <5 ; i++)
        *(((INT32U *) rand_parameter) + i) = REG32(TIMER_IOBASE+TIMER_CV);

    cert   = malloc (1024);
    buffer = (unsigned char *) malloc(bufferLen);
    memset(buffer, 0, bufferLen);

    /* RSA Key Gerneration					*/
    rsa_init(&rsactx,
             RSA_PKCS_V15,
             0,
             RtkRand,
             (void*)rand_parameter);

    rsa_gen_key(&rsactx,
                1024,
                65537);


    RtkGenerateCertificate(&rsactx,cert,&certLen);

    RtkOutCredential(&rsactx,cert,buffer,&bufferLen);


    cert_write(buffer, bufferLen);

    rsa_free(&rsactx);

    free(buffer);
    free(cert);


}

void cert_write(INT8U *buf, INT32S len)
{
    INT32U i;
    OS_CPU_SR cpu_sr = 0;

    struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
    //INT32U flashaddr = (INT32U ) (SYSTEM_CERT_DATA - 0x81000000);
    INT32U flashaddr = (INT32U ) (SYSTEM_CERT_DATA & 0x00FFFFFF);

    OS_ENTER_CRITICAL();
    spi_se_erase(ssi_map, flashaddr);
    for (i = 0 ; i < ifloor(len, 64) ; i++)
        spi_write(ssi_map, flashaddr+i*64, buf+i*64, 64);
    OS_EXIT_CRITICAL();
}

void key_reload()
{
    //INT8U i = 0;
    INT32U sig;
    INT8U  len;
    INT16U numkey;

    numkey = ffs(0xFFFF - REG16(MAC_BASE_ADDR + MAC_AES_KEY_NUM));
    sig = REG32(MAC_BASE_ADDR + MAC_AES_KEY_SIG + numkey*20 ) & 0x00FFFFFF;
    len = (REG32(MAC_BASE_ADDR + MAC_AES_KEY_SIG + numkey*20 ) & 0xFF000000) >> 24;

    //since it comes from PG write back, do not need to check
    if (sig == VALID_KEY_SIG && len >= 4 && len <= 16)
    {

        memset(aesmsg, 0, sizeof(aesmsg));

        memcpy(aesmsg, (const void *)(MAC_BASE_ADDR + MAC_AES_KEY + numkey*20), len);
        aesmsg[len] = '\0';

        //clear the autoload
        memset((void *)(MAC_BASE_ADDR + MAC_AES_KEY), 0, 20*numkey);

        validaeskey = VALID_KEY_SIG;

        memset(aeskey, 0, sizeof(aeskey));
        sha1(aesmsg, strlen((const char *)aesmsg), aeskey);
        //shuffle the original cipher words
        sha1(aeskey, 20, aesmsg);

        //set the bit to indicate the aes key has been moved into memory
        bsp_bits_set(MAC_NO_CLEAR, 1, BIT_AES_KEY, 1);

    }

}

void aes_init()
{
    //INT8U i = 0;
    INT32U efusecrc;
	INT32U clacrc;
    INT8U  len;
	INT8U  valid;
    INT16U numkey = 0;
    INT32U validkey;
	INT8U  crcbuf[16 + 1];

    //wait efuse load complete
    while (!(bsp_bits_get(MAC_MAC_STATUS,BIT_AUTOLOAD, 1)))
        bsp_wait(1);

	#if CONFIG_VERSION >= IC_VERSION_FP_RevA
    validkey = bsp_bits_get(MAC_NO_CLEAR, BIT_AES_KEY, 1);

    if(!validkey)
    {
        numkey = REG8(MAC_BASE_ADDR + MAC_AES_KEY_NUM);
		valid = 0;
		
		if(((numkey & 0x30)>>4) < 3)
		{
			numkey = 2;
		}else if(((numkey & 0x0c)>>2) < 3)
		{
			numkey = 1;
		}
		else if((numkey & 0x03) < 3)
		{
			numkey = 0;
		}

        if(numkey < 3)
        {            
			memcpy(&efusecrc , MAC_BASE_ADDR + MAC_AES_KEY + numkey*21, 4);
            len = REG8(MAC_BASE_ADDR + MAC_AES_KEY + numkey*21 + 4);
			if(len >= 4 && len <= 16)
			{
				//5 = crc (4)+ len(1)
				crcbuf[0] = len;
				memcpy(crcbuf + 1 , MAC_BASE_ADDR + MAC_AES_KEY + 5 + numkey*21, len);				
				clacrc = crc32(0, crcbuf, len + 1);

				if(efusecrc == clacrc)
				{					
					memcpy(aesmsg , MAC_BASE_ADDR + MAC_AES_KEY + 5 + numkey*21, len);	
					aesmsg[len] = '\0';
					//clear the autoload
					memset((void *)(MAC_BASE_ADDR + MAC_AES_KEY), 0, 21*(numkey + 1));

					validaeskey = VALID_KEY_SIG;
					valid = 1;
				}
			}
        }
        
		if(!valid)
        {
            memset(aesmsg, 0, sizeof(aesmsg));
            memcpy(aesmsg, daesmsg, strlen((const char *)daesmsg));
            validaeskey = 0;
        }

        memset(aeskey, 0, sizeof(aeskey));
        sha1(aesmsg, strlen((const char *)aesmsg), aeskey);
        //shuffle the original cipher words
        sha1(aeskey, 20, aesmsg);

        //set the bit to indicate the aes key has been moved into memory
        bsp_bits_set(MAC_NO_CLEAR, 1, BIT_AES_KEY, 1);
    }	
	#else
    validkey = bsp_bits_get(MAC_NO_CLEAR, BIT_AES_KEY, 1);


    if(!validkey)
    {
        numkey = ffs(0xFFFF - REG16(MAC_BASE_ADDR + MAC_AES_KEY_NUM));

        if(numkey > 0)
        {
            sig = REG32(MAC_BASE_ADDR + MAC_AES_KEY_SIG + numkey*20 ) & 0x00FFFFFF;
            len = (REG32(MAC_BASE_ADDR + MAC_AES_KEY_SIG + numkey*20 ) & 0xFF000000) >> 24;
            if(sig == VALID_KEY_SIG && len >= 4 && len <= 16)
            {
                memcpy(aesmsg, (const void *)(MAC_BASE_ADDR + MAC_AES_KEY + numkey*20), len);
                aesmsg[len] = '\0';
                //clear the autoload
                memset((void *)(MAC_BASE_ADDR + MAC_AES_KEY), 0, 20*numkey);

                validaeskey = VALID_KEY_SIG;
            }
        }
        else
        {
            memset(aesmsg, 0, sizeof(aesmsg));
            memcpy(aesmsg, daesmsg, strlen((const char *)daesmsg));
            validaeskey = 0;
        }

        memset(aeskey, 0, sizeof(aeskey));
        sha1(aesmsg, strlen((const char *)aesmsg), aeskey);
        //shuffle the original cipher words
        sha1(aeskey, 20, aesmsg);

        //set the bit to indicate the aes key has been moved into memory
        bsp_bits_set(MAC_NO_CLEAR, 1, BIT_AES_KEY, 1);
    }
	#endif

}

INT32U GetKeyStatus()
{
    return validaeskey;
}

INT8U SetAESKey(INT8U *key, INT8U len)
{
    if (len >= 4 && len <=16)
    {
        memcpy(aesmsg, key, len);
        memset(aeskey, 0, sizeof(aeskey));
        sha1(aesmsg, len, aeskey);
        //shuffle the original cipher words
        sha1(aeskey, 20, aesmsg);
        validaeskey = VALID_KEY_SIG;
        return 0;
    }
    else
        return 1;

}

void updateCert(INT8U *key, INT8U len)
{
    rsa_context rsactx;
    INT8U rand_parameter[20];
    INT8U *pCert = malloc(1024);
    INT32S bufferLen= 4096;
    INT8U *buffer = malloc(bufferLen);
    INT8U i = 0;

    memset(buffer, 0, bufferLen);
    memset(pCert, 0, 1024);

    for (i = 0; i <5 ; i++)
        *(((INT32U *) rand_parameter) + i) = REG32(TIMER_IOBASE+TIMER_CV);


    rsa_init(&rsactx,
             RSA_PKCS_V15,
             0,
             RtkRand,
             (void*)rand_parameter);

    //using the existing key to do decryption
    RtkDecryptCredential((INT8U *)SYSTEM_CERT_DATA, pCert, &rsactx);

    SetAESKey(key, len);

    //using the new key to do encrypt
    RtkOutCredential(&rsactx,pCert,buffer,&bufferLen);

    cert_write(buffer, bufferLen);

    free(buffer);
    free(pCert);
    rsa_free(&rsactx);
}
int checkPadLen(int ivLen, int outMsgLen)
{
    if (ivLen == 0)
        return 0;
    else
        return (ivLen - (outMsgLen + 1) % ivLen);
}

int ffs(int word)
{
  int i;

  if (!word)
    return 0;

  i = 0;
  for (;;)
    {
      if (((1 << i++) & word) != 0)
	return i;
    }
}
