#include <stdlib.h>
#include <string.h>
#include "aes.h"
#include "tls.h"
#include "tcp.h"
#include "rsasw.h"
#include "bsp.h"

#if (CONFIG_VERSION  < IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
extern INT8U aeskey[];
extern INT8U aesivE[];

/*
 * Example RSA-1024 keypair, for test purposes
 */
#define KEY_LEN 128
extern DPCONF *dpconf;
#define PT_LEN 7
//#define RSA_PT  "\x00\x01\x03\x00\x01\x03\x68"
//unsigned char RSA_PT[7] = {0x00,0x01,0x03,0x00,0x01,0x03,0x68};

/*
#define RSA_PT  "\xAA\xBB\xCC\x03\x02\x01\x00\xFF\xFF\xFF\xFF\xFF" \
               "\x11\x22\x33\x0A\x0B\x0C\xCC\xDD\xDD\xDD\xDD\xDD"
	       */

int resumeInitKey(int verbose, unsigned char *p, struct tcp_pcb *pcb)
{
    //unsigned char random[SSL_RANDOM_SIZE*2];
    //unsigned char keyBlk[256];
    unsigned char *random = malloc(SSL_RANDOM_SIZE*2);
    unsigned char *keyBlk = malloc(256);
    unsigned char *key1;
    unsigned char *key2;

    /*
    * Swap the client and server random values.
    */
    memcpy(random , pcb->ssl->serverRandom, SSL_RANDOM_SIZE);
    memcpy(random + SSL_RANDOM_SIZE, pcb->ssl->clientRandom, SSL_RANDOM_SIZE);

    tls1_prf(pcb->ssl->master, 48, "key expansion", random, SSL_RANDOM_SIZE*2,
             keyBlk, 256 );
    /*
    * Determine the key, IV and MAC length.
    */
    switch (pcb->ssl->cipher)
    {
        //minLen = max(keyLen, ivLen, macLen) ???
    case SSL_RSA_RC4_128_SHA:
        pcb->ssl->keyLen = 16;
        pcb->ssl->minLen = 20;
        pcb->ssl->ivLen = 0;
        pcb->ssl->macLen = 20;
        break;
    case TLS_RSA_WITH_AES_128_CBC_SHA:
        pcb->ssl->keyLen = 16;
        pcb->ssl->minLen = 20;
        pcb->ssl->ivLen = 16;
        pcb->ssl->macLen = 20;
        break;
    }

    /*
    * Finally setup the cipher contexts, IVs and MAC secrets.
    */
    key1 = keyBlk + pcb->ssl->macLen * 2 + pcb->ssl->keyLen;
    key2 = keyBlk + pcb->ssl->macLen * 2;

    memcpy( pcb->ssl->macDec, keyBlk,  pcb->ssl->macLen );
    memcpy( pcb->ssl->macEnc, keyBlk + pcb->ssl->macLen, pcb->ssl->macLen );

    memcpy( pcb->ssl->ivDec, key1 + pcb->ssl->keyLen,  pcb->ssl->ivLen );
    memcpy( pcb->ssl->ivEnc, key1 + pcb->ssl->keyLen + pcb->ssl->ivLen,
            pcb->ssl->ivLen );

    switch (pcb->ssl->cipher)
    {
    case SSL_RSA_RC4_128_SHA:
#if 0
        arc4_setup( (arc4_context *) pcb->ssl->ctxEnc, key1, pcb->ssl->keyLen );
        arc4_setup( (arc4_context *) pcb->ssl->ctxDec, key2, pcb->ssl->keyLen );
#endif
        break;
    case TLS_RSA_WITH_AES_128_CBC_SHA:
        //aes_setkey_enc( (aes_context *) pcb->ssl->ctxEnc, key1, pcb->ssl->keyLen*8);
        //aes_setkey_dec( (aes_context *) pcb->ssl->ctxDec, key2, pcb->ssl->keyLen*8);
        memcpy(pcb->ssl->ctxEnc, key1, pcb->ssl->keyLen);
        memcpy(pcb->ssl->ctxDec, key2, pcb->ssl->keyLen);
        break;
    }

    free(random);
    free(keyBlk);
    return( 0 );
}

int getRSAKey(int index, char* buf)
{

#if CONFIG_VERSION < IC_VERSION_DP_RevF
#define RSA_HASH_STRING "CNSD7@Realtek&2379"
#define ivD  "!@#$%^&*()!@#$%^"

    unsigned char *msg = malloc(sizeof(RSA_HASH_STRING));
    //sha1_context sha1;
    sha1_context *sha1 = malloc(sizeof(sha1_context));
    unsigned char sha1Buf[20];
#endif

    aesHwKey *hk = malloc(sizeof(aesHwKey));
    unsigned char *pos;
    int padLen = 0;
    int i = 0;
    int len = 0;
    int keyCount = 0;
#if CONFIG_BUILDROM
    aes_context asSW;
#endif

#if CONFIG_VERSION < IC_VERSION_DP_RevF
    memset(msg, 0, sizeof(RSA_HASH_STRING));
    memcpy(msg, RSA_HASH_STRING, strlen(RSA_HASH_STRING));

    sha1_startsH( sha1 );
    sha1_updateH( sha1, msg, strlen(RSA_HASH_STRING));
    sha1_finishH( sha1, sha1Buf);

    free(sha1);

    memcpy(hk->AES_Key, sha1Buf + 4, 16);
    memcpy(hk->IV,ivD, 16);
#else
    memcpy(hk->AES_Key, aeskey + 4, 16);
    memcpy(hk->IV,aesivE, 16);
#endif

    //Certificate
    sscanf((void *) TLS_FLASH_DATA_ADDRESS, "%d\n", &len);
    pos = strchr((void *) TLS_FLASH_DATA_ADDRESS, '\n');
    pos++;
    pos+=len;

    //key start address
    while (keyCount<8)
    {
        sscanf(pos, "%d\n", &len);
        pos = strchr(pos, '\n');
        pos++;
        if (keyCount == index)
        {
            memcpy(buf, pos, len);
            break;
        }
        pos+=len;
        keyCount++;
    }
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    tls_set((INT8U *) hk, buf, 0x2,len);

    if (tls_poll() == -1)
        ;
#endif

#if CONFIG_BUILDROM
    aes_crypt_cbc( (aes_context *) hk->AES_Key, AES_DECRYPT, len, hk->IV, buf, buf);
#endif

    padLen = buf[len - 1] + 1;

    for ( i = 1; i <= padLen; i++ )
    {
        if ( buf[len - i] != padLen - 1)
        {
            padLen = 0;
        }
    }

    if (padLen!=0)
    {
        for ( i = 1; i <= padLen; i++ )
        {
            buf[len - i] = 0;
        }
    }

    free(hk);
#if CONFIG_VERSION < IC_VERSION_DP_RevF
    free(msg);
#endif
    return 1;
}

/*
 * Checkup routine
 */
int tlsInitKeys( int verbose, unsigned char *p, struct tcp_pcb *pcb)
{
    //Max size is 256 + 1(NULL space)
    int len = 0;
    int i = 0;
    //rsa_context rsa;
    //unsigned char random[SSL_RANDOM_SIZE*2];
    //unsigned char rsa_decrypted[68];
    //unsigned char keyBlk[256];
    rsa_context *rsa = malloc(sizeof(rsa_context));
    unsigned char *random;
    unsigned char *rsa_decrypted = malloc(68);
    unsigned char *keyBlk;
    unsigned char *key1;
    unsigned char *key2;

    if (pcb->ssl->cipher == TLS_RSA_WITH_AES_128_CBC_SHA)    {
#define MAX_RSA_KEY_SIZE 260
        unsigned char *tmp= malloc(MAX_RSA_KEY_SIZE);

        //Init RSA
        memset( rsa, 0, sizeof( rsa_context ) );
        rsa->len = KEY_LEN;

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
            free(rsa);
            free(rsa_decrypted);
            return( -1 );
        }

        len = PMSLEN;
        if ( (i = rsa_pkcs1_decrypt( rsa, RSA_PRIVATE, &len, p, rsa_decrypted ) )!= 0 )
        {
            free(rsa);
            free(rsa_decrypted);
            return( -1 );
        }

    }
    else if (pcb->ssl->cipher == TLS_PSK_WITH_AES_128_CBC_SHA)
    {
        //unsigned int zero32[32]={0};
        unsigned int *zero32 = malloc(32*4);
        memset(zero32, 0, 32*4);
        len = PMSLEN;
        rsa_decrypted[0] = 0x00;
        rsa_decrypted[1] = 0x20;
        memcpy(&rsa_decrypted[2], zero32, 32);
        *(&rsa_decrypted[2] + 32) = 0x00;
        *(&rsa_decrypted[2] + 33) = 0x20;
        memcpy(&rsa_decrypted[2]+34, dpconf->pskdata.PPS, 32);
        free(zero32);
    }

    random = malloc(SSL_RANDOM_SIZE*2);
    keyBlk = malloc(256);

    memcpy(random, pcb->ssl->clientRandom, SSL_RANDOM_SIZE);
    memcpy(random + SSL_RANDOM_SIZE, pcb->ssl->serverRandom, SSL_RANDOM_SIZE);


    if (pcb->ssl->cipher == TLS_RSA_WITH_AES_128_CBC_SHA)
    {
        tls1_prf(rsa_decrypted, PMSLEN, "master secret", random, SSL_RANDOM_SIZE*2,
                 pcb->ssl->master, len );
    }
    else if (pcb->ssl->cipher == TLS_PSK_WITH_AES_128_CBC_SHA)
    {

        tls1_prf(rsa_decrypted, 68, "master secret", random, SSL_RANDOM_SIZE*2,
                 pcb->ssl->master, len );
    }
    /*
    * Swap the client and server random values.
    */
    memcpy(random , pcb->ssl->serverRandom, SSL_RANDOM_SIZE);
    memcpy(random + SSL_RANDOM_SIZE, pcb->ssl->clientRandom, SSL_RANDOM_SIZE);
    tls1_prf(pcb->ssl->master, 48, "key expansion", random, SSL_RANDOM_SIZE*2, keyBlk, 256 );
    /*
    * Determine the key, IV and MAC length.
    */
    switch (pcb->ssl->cipher)
    {
        //minLen = max(keyLen, ivLen, macLen) ???
    case SSL_RSA_RC4_128_SHA:
        pcb->ssl->keyLen = 16;
        pcb->ssl->minLen = 20;
        pcb->ssl->ivLen = 0;
        pcb->ssl->macLen = 20;
        break;
    case TLS_PSK_WITH_AES_128_CBC_SHA:
    case TLS_RSA_WITH_AES_128_CBC_SHA:
        pcb->ssl->keyLen = 16;
        pcb->ssl->minLen = 20;
        pcb->ssl->ivLen = 16;
        pcb->ssl->macLen = 20;
        break;
    }

    /*
    * Finally setup the cipher contexts, IVs and MAC secrets.
    */
    key1 = keyBlk + pcb->ssl->macLen * 2 + pcb->ssl->keyLen;
    key2 = keyBlk + pcb->ssl->macLen * 2;

    memcpy( pcb->ssl->macDec, keyBlk,  pcb->ssl->macLen );
    memcpy( pcb->ssl->macEnc, keyBlk + pcb->ssl->macLen, pcb->ssl->macLen );

    memcpy( pcb->ssl->ivDec, key1 + pcb->ssl->keyLen,  pcb->ssl->ivLen );
    memcpy( pcb->ssl->ivEnc, key1 + pcb->ssl->keyLen + pcb->ssl->ivLen,
            pcb->ssl->ivLen );

    switch (pcb->ssl->cipher)
    {
    case SSL_RSA_RC4_128_SHA:
#if 0
        arc4_setup( (arc4_context *) pcb->ssl->ctxEnc, key1, pcb->ssl->keyLen );
        arc4_setup( (arc4_context *) pcb->ssl->ctxDec, key2, pcb->ssl->keyLen );
#endif
        break;
    case TLS_PSK_WITH_AES_128_CBC_SHA:
    case TLS_RSA_WITH_AES_128_CBC_SHA:
        //aes_setkey_enc( (aes_context *) pcb->ssl->ctxEnc, key1, pcb->ssl->keyLen*8);
        //aes_setkey_dec( (aes_context *) pcb->ssl->ctxDec, key2, pcb->ssl->keyLen*8);
        memcpy(pcb->ssl->ctxEnc, key1, pcb->ssl->keyLen);
        memcpy(pcb->ssl->ctxDec, key2, pcb->ssl->keyLen);
        break;
    }
    if (pcb->ssl->cipher == TLS_RSA_WITH_AES_128_CBC_SHA)
        rsa_free( rsa );

    free(rsa);
    free(rsa_decrypted);
    free(random);
    free(keyBlk);

    return( 0 );
}
#endif
