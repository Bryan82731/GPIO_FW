/**
* \file sha1.h
*/
#include <sys/dpdef.h>
#ifndef XYSSL_SHA1_H
#define XYSSL_SHA1_H

/**
 * \brief          SHA-1 context structure
 */
typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[5];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
}
sha1_context;

#ifdef __cplusplus
extern "C" {
#endif

    void sha1_starts( sha1_context *ctx );
    void sha1_update( sha1_context *ctx, unsigned char *input, int ilen );
    void sha1_finish( sha1_context *ctx, unsigned char output[20] );
    void sha1_hmac_starts( sha1_context *ctx, unsigned char *key, int keylen );
    void sha1_hmac_update( sha1_context *ctx, unsigned char *input, int ilen );
    void sha1_hmacSW( unsigned char *key, int keylen, unsigned char *input, int ilen,
                      unsigned char output[20] );
    void sha1( unsigned char *input, int ilen, unsigned char output[20] );
    void sha1_hmac( unsigned char *key, int keylen,
                    unsigned char *input, int ilen,
                    unsigned char output[20] );
    int sha1_self_test( int verbose );
    void sha1_process( sha1_context *ctx, unsigned char data[64] );
    void sha1_hmac_finish( sha1_context *ctx, unsigned char output[20] );
    void sha1_hmacSWHash( unsigned char *key, int keylen, unsigned char *input, int ilen, unsigned char output[20] );

#if CONFIG_VERSION <= IC_VERSION_DP_RevF || CONFIG_VERSION >= IC_VERSION_FP_RevA
    void sha1H( unsigned char *input, int ilen, unsigned char output[20] );
    void sha1_startsH( sha1_context *ctx );
    void sha1_updateH( sha1_context *ctx, unsigned char *input, int ilen );
    void sha1_finishH( sha1_context *ctx, unsigned char output[20] );
    void sha1_processH( sha1_context *ctx, unsigned char* data, int len);
#endif

#if CONFIG_VERSION > IC_VERSION_DP_RevF && CONFIG_VERSION < IC_VERSION_FP_RevA
#define sha1H(x, y, z) sha1(x,y,z)
#define sha1_startsH sha1_starts
#define sha1_processH sha1_process
#define sha1_updateH sha1_update
#define sha1_finishH sha1_finish
#define sha1_hmac_startsH sha1_hmac_starts
#define sha1_hmac_updateH sha1_hmac_update
#define sha1_hmac_finishH sha1_hmac_finish
#define sha1_hmac sha1_hmacSWHash
#endif

#ifdef __cplusplus
}
#endif

#endif /* sha1.h */
