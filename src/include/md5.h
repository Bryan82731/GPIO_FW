/**
 * \file md5.h
 */
#include <sys/dpdef.h>
#ifndef XYSSL_MD5_H
#define XYSSL_MD5_H

/**
 * \brief          MD5 context structure
 */
typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[4];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
}
md5_context;

#ifdef __cplusplus
extern "C" {
#endif

    void md5_hmac( unsigned char *key, int keylen,
                   unsigned char *input, int ilen,
                   unsigned char output[16] );


#define IN
#define OUT
#define HASHLEN 16
    typedef char HASH[HASHLEN];
#define HASHHEXLEN 32
    typedef char HASHHEX[HASHHEXLEN+1];

    void DigestCalcHA1(
        IN char * pszAlg,
        IN char * pszUserName,
        IN char * pszRealm,
        IN char * pszPassword,
        IN char * pszNonce,
        IN char * pszCNonce,
        OUT HASHHEX SessionKey
    );

    void DigestCalcResponse(
        IN HASHHEX HA1,           /* H(A1) */
        IN char * pszNonce,       /* nonce from server */
        IN char * pszNonceCount,  /* 8 hex digits */
        IN char * pszCNonce,      /* client nonce */
        IN char * pszQop,         /* qop-value: "", "auth", "auth-int" */
        IN char * pszMethod,      /* method from the request */
        IN char * pszDigestUri,   /* requested URL */
        IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
        OUT HASHHEX Response      /* request-digest or response-digest */
    );

#ifdef __cplusplus
}
#endif

void md5_starts( md5_context *ctx );

void md5_update( md5_context *ctx, unsigned char *input, int ilen );
void md5_finish( md5_context *ctx, unsigned char output[16] );
void md5( unsigned char *input, int ilen, unsigned char output[16] );
void md5_hmac_starts( md5_context *ctx, unsigned char *key, int keylen );
void md5_hmac_update( md5_context *ctx, unsigned char *input, int ilen );
void md5_hmac_finish( md5_context *ctx, unsigned char output[16] );
void md5_hmacSW( unsigned char *key, int keylen,
               unsigned char *input, int ilen,
               unsigned char output[16] );
void md5_hmacSWHash( unsigned char *key, int keylen, unsigned char *input, int ilen, unsigned char output[16] );
void md5_process( md5_context *ctx, unsigned char data[64] );

#if (CONFIG_VERSION >= IC_VERSION_EP_RevA) && (CONFIG_VERSION < IC_VERSION_FP_RevA)
#define md5H(x, y, z) md5(x,y,z)
#define md5_updateH md5_update
#define md5_finishH md5_finish
#define md5_hmac md5_hmacSWHash
#define md5_hmac_startsH md5_hmac_starts
#define md5_hmac_updateH md5_hmac_update
#define md5_hmac_finishH md5_hmac_finish
#else
void md5_updateH( md5_context *ctx, unsigned char *input, int ilen );
void md5_finishH( md5_context *ctx, unsigned char output[16] );
#endif

#endif /* md5.h */
