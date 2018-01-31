/*
 *  RFC 1321 compliant MD5 implementation
 *
 *  Copyright (C) 2006-2007  Christophe Devine
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */
/*
 *  The MD5 algorithm was designed by Ron Rivest in 1991.
 *
 *  http://www.ietf.org/rfc/rfc1321.txt
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "md5.h"
#include "bsp.h"
#include "lib.h"

/*
 * 32-bit integer manipulation macros (little endian)
 */
#ifndef GET_ULONG_LE
#define GET_ULONG_LE(n,b,i)                             \
{                                                       \
    (n) = ( (unsigned long) (b)[(i)    ]       )        \
        | ( (unsigned long) (b)[(i) + 1] <<  8 )        \
        | ( (unsigned long) (b)[(i) + 2] << 16 )        \
        | ( (unsigned long) (b)[(i) + 3] << 24 );       \
}
#endif

#ifndef PUT_ULONG_LE
#define PUT_ULONG_LE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n)       );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 3] = (unsigned char) ( (n) >> 24 );       \
}
#endif

#if (defined(CONFIG_CODEC_TEST) && (CONFIG_VERSION  < IC_VERSION_DP_RevF)) || defined(CONFIG_BUILDROM)
extern const unsigned char md5_padding[64];
#endif

const unsigned char md5_padding[64] =
{
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
/*
 * MD5 context setup
 */
void md5_starts( md5_context *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
}

#if CONFIG_VERSION <= IC_VERSION_DP_RevF || CONFIG_VERSION >= IC_VERSION_FP_RevA
// Do not use 64N to do hash
//Need to swap 54n and 60n, and copy 64n data
static void md5_processH( md5_context *ctx, unsigned char* data, int ilen )
{
    //Must 4-byte alignment
    int X[2];
    //unsigned char key[36] = {0};
    unsigned char *key;
    unsigned char *tmp = malloc(ilen);
    memcpy(tmp, data, ilen);
	
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    key = malloc(36);
    memset(key, 0, 36);
#else
    key = malloc(80);
    memset(key, 0, 80); //offset 0x40~ (0x40+16) is result
#endif	
    //X[0] = *((int*)(data + 56));
    //X[1] = *((int*)(data + 60));
    memcpy(&X[0], (tmp + ilen - 8), 4);
    memcpy(&X[1], (tmp + ilen - 4), 4);
    //Endian issue
    X[0] = htonl(X[0]);
    X[1] = htonl(X[1]);

    //Md5 should swap X[14] and X[15]
    memcpy(tmp + ilen - 8, X + 1, 4);
    memcpy(tmp + ilen - 4, X, 4);

    
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	memcpy(key + 20, ctx->state, 16);
	tls_set(key, tmp, MD5_SHA1, ilen);
#else
	memcpy(key, ctx->state, 16);
	tls_set(key, tmp, MD5_HASH, ilen);
#endif

    if (tls_poll() == -1)
		;
	else
	{
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
		memcpy(ctx->state, key + 20, 16);
#else
		memcpy(ctx->state, key + 64, 16);
#endif
	}
    free(key);
    free(tmp);
}

void md5_updateH( md5_context *ctx, unsigned char *input, int ilen )
{
    int fill;
    unsigned long left;
     int t = 0;

    if ( ilen <= 0 )
        return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if ( ctx->total[0] < (unsigned long) ilen )
        ctx->total[1]++;

    if ( left && ilen >= fill )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, fill );
        md5_processH( ctx, ctx->buffer ,64);
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while( ilen >= 64 )
    {
    		if(ilen >= 1600)
    		{        
        	md5_processH( ctx, input, 1600 );
        	input += 1600;
        	ilen  -= 1600;        	        	
        }
        else
        {
        	t = (ilen % 64);
        	md5_processH( ctx, input, ilen - t);
        	input += ilen;        	
        	ilen = t; 
        	input -= ilen;       	
        }
    }    

    if ( ilen > 0 )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, ilen );
    }
}

void md5_finishH( md5_context *ctx, unsigned char output[16] )
{
	unsigned long last, padn;
    unsigned long high, low;
    unsigned char msglen[8];

    high = ( ctx->total[0] >> 29 )
           | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_ULONG_LE( low,  msglen, 0 );
    PUT_ULONG_LE( high, msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    md5_updateH( ctx, (unsigned char *) md5_padding, padn );
    md5_updateH( ctx, msglen, 8 );
    memcpy(output, ctx->state, 16);

#if 0
    PUT_ULONG_LE( ctx->state[0], output,  0 );
    PUT_ULONG_LE( ctx->state[1], output,  4 );
    PUT_ULONG_LE( ctx->state[2], output,  8 );
    PUT_ULONG_LE( ctx->state[3], output, 12 );
#endif
}

void md5H( unsigned char *input, int ilen, unsigned char output[16] )
{
    //md5_context ctx;
    md5_context *ctx = malloc(sizeof(md5_context));

    md5_starts( ctx );
    md5_updateH( ctx, input, ilen );
    md5_finishH( ctx, output );

    free(ctx);

    //memset( &ctx, 0, sizeof( md5_context ) );
}

void md5_hmac_startsH( md5_context *ctx, unsigned char *key, int keylen )
{
    int i;
    unsigned char sum[16];

    if( keylen > 64 )
    {
        md5H( key, keylen, sum );
        keylen = 16;
        key = sum;
    }

    memset( ctx->ipad, 0x36, 64 );
    memset( ctx->opad, 0x5C, 64 );

    for( i = 0; i < keylen; i++ )
    {
        ctx->ipad[i] = (unsigned char)( ctx->ipad[i] ^ key[i] );
        ctx->opad[i] = (unsigned char)( ctx->opad[i] ^ key[i] );
    }

    md5_starts( ctx );
    md5_updateH( ctx, ctx->ipad, 64 );

    memset( sum, 0, sizeof( sum ) );
}

/*
 * MD5 HMAC process buffer
 */

void md5_hmac_updateH( md5_context *ctx, unsigned char *input, int ilen )
{
    md5_updateH( ctx, input, ilen );
}
/*
 * MD5 HMAC final digest
 */
void md5_hmac_finishH( md5_context *ctx, unsigned char output[16] )
{
    unsigned char tmpbuf[16];

    md5_finishH( ctx, tmpbuf );
    md5_starts( ctx );
    md5_updateH( ctx, ctx->opad, 64 );
    md5_updateH( ctx, tmpbuf, 16 );
    md5_finishH( ctx, output );

    memset( tmpbuf, 0, sizeof( tmpbuf ) );

}

/*
 * output = HMAC-MD5( hmac key, input buffer )
 */
void md5_hmac( unsigned char *key, int keylen, unsigned char *input, int ilen,
               unsigned char output[16] )
{   
    unsigned char *hk = 0;
    unsigned char *inData = 0;
	
#if CONFIG_VERSION <= IC_VERSION_DP_RevF	
    if(ilen > 1600 || (keylen != 16 && keylen != 24))
#else
	if(ilen > 1600 || (keylen != 16))
#endif		 
    {
    	md5_hmacSWHash(key, keylen, input, ilen, output);
    	return;
    }

    if (keylen == 16)
    {
#if CONFIG_VERSION <= IC_VERSION_DP_RevF		
        hk = malloc(52);
        inData = malloc(ilen);
#else
        hk = malloc(80);
		memset(hk, 0, 80);
        inData = malloc(ilen);
#endif	

        memcpy(hk, key, 16);
        memcpy(inData, input, ilen);

        tls_set(hk, inData, MD5_HMAC_16, ilen);

        if (tls_poll() == -1)
            ;
        else
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
            memcpy(output, hk, 16);
#else
			memcpy(output, hk + 64, 16);	
#endif

        free(hk);
        free(inData);
    }
    else if (keylen == 24)
    {
        hk = malloc(24);
        inData = malloc(ilen);

        memcpy(hk, key, 24);
        memcpy(inData, input, ilen);

        tls_set(hk, inData, 0x6, ilen);

        if (tls_poll() == -1)
            ;
        else
            memcpy(output, hk, 16);

        free(hk);
        free(inData);
    }
    else
    {
        ;
    }
}
#endif

void md5_hmacSWHash( unsigned char *key, int keylen, unsigned char *input, int ilen, unsigned char output[16] )
{
    //md5_context ctx;
    md5_context *ctx = malloc(sizeof(md5_context));

    md5_hmac_starts( ctx, key, keylen );
    md5_hmac_update( ctx, input, ilen );
    md5_hmac_finish( ctx, output );

    free(ctx);
    //memset( &ctx, 0, sizeof( md5_context ) );	
}

int stricmp(const char *dst, const char *src)
{
    int ch1, ch2;
    do
    {
        if ( ((ch1 = (unsigned char)(*(dst++))) >= 'A') &&(ch1 <= 'Z') )
            ch1 += 0x20;

        if ( ((ch2 = (unsigned char)(*(src++))) >= 'A') &&(ch2 <= 'Z') )
            ch2 += 0x20;
    } while ( ch1 && (ch1 == ch2) );

    return(ch1 - ch2);
}

void CvtHex(
    IN HASH Bin,
    OUT HASHHEX Hex
)
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++) {
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
            Hex[i*2] = (j + '0');
        else
            Hex[i*2] = (j + 'a' - 10);
        j = Bin[i] & 0xf;
        if (j <= 9)
            Hex[i*2+1] = (j + '0');
        else
            Hex[i*2+1] = (j + 'a' - 10);
    };
    Hex[HASHHEXLEN] = '\0';
};

/* calculate H(A1) as per spec */
void DigestCalcHA1(
    IN char * pszAlg,
    IN char * pszUserName,
    IN char * pszRealm,
    IN char * pszPassword,
    IN char * pszNonce,
    IN char * pszCNonce,
    OUT HASHHEX SessionKey
)
{
    //md5_context Md5Ctx;
    md5_context *Md5Ctx = malloc(sizeof(md5_context));
    //HASH HA1;
    INT8U *HA1 = malloc(HASHLEN+1);

    md5_starts(Md5Ctx);
    md5_updateH(Md5Ctx, pszUserName, strlen(pszUserName));
    md5_updateH(Md5Ctx, ":", 1);
    md5_updateH(Md5Ctx, pszRealm, strlen(pszRealm));
    md5_updateH(Md5Ctx, ":", 1);
    md5_updateH(Md5Ctx, pszPassword, strlen(pszPassword));
    md5_finishH(Md5Ctx, HA1);
    if (stricmp(pszAlg, "md5-sess") == 0) {
        md5_starts(Md5Ctx);
        md5_updateH(Md5Ctx, HA1, HASHLEN);
        md5_updateH(Md5Ctx, ":", 1);
        md5_updateH(Md5Ctx, pszNonce, strlen(pszNonce));
        md5_updateH(Md5Ctx, ":", 1);
        md5_updateH(Md5Ctx, pszCNonce, strlen(pszCNonce));
        md5_finishH(Md5Ctx, HA1);
    };
    CvtHex(HA1, SessionKey);

    free(HA1);
    free(Md5Ctx);
};



/* calculate request-digest/response-digest as per HTTP Digest spec */
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
)
{
    //md5_context Md5Ctx;
    md5_context *Md5Ctx = malloc(sizeof(md5_context));
    //HASH HA2;
    //HASH RespHash;
    //HASHHEX HA2Hex;
    INT8U *HA2 = malloc(HASHLEN+1);
    INT8U *RespHash = malloc(HASHLEN+1);
    INT8U *HA2Hex = malloc(HASHHEXLEN+1);

    // calculate H(A2)
    md5_starts(Md5Ctx);
    md5_updateH(Md5Ctx, pszMethod, strlen(pszMethod));
    md5_updateH(Md5Ctx, ":", 1);
    md5_updateH(Md5Ctx, pszDigestUri, strlen(pszDigestUri));
    if (stricmp(pszQop, "auth-int") == 0) {
        md5_updateH(Md5Ctx, ":", 1);
        md5_updateH(Md5Ctx, HEntity, HASHHEXLEN);
    };
    md5_finishH(Md5Ctx, HA2);
    CvtHex(HA2, HA2Hex);

    // calculate response
    md5_starts(Md5Ctx);
    md5_updateH(Md5Ctx, HA1, HASHHEXLEN);
    md5_updateH(Md5Ctx, ":", 1);
    md5_updateH(Md5Ctx, pszNonce, strlen(pszNonce));
    md5_updateH(Md5Ctx, ":", 1);
    if (*pszQop) {

        md5_updateH(Md5Ctx, pszNonceCount, strlen(pszNonceCount));
        md5_updateH(Md5Ctx, ":", 1);
        md5_updateH(Md5Ctx, pszCNonce, strlen(pszCNonce));
        md5_updateH(Md5Ctx, ":", 1);
        md5_updateH(Md5Ctx, pszQop, strlen(pszQop));
        md5_updateH(Md5Ctx, ":", 1);
    };
    md5_updateH(Md5Ctx, HA2Hex, HASHHEXLEN);
    md5_finishH(Md5Ctx, RespHash);
    CvtHex(RespHash, Response);

    free(HA2);
    free(RespHash);
    free(HA2Hex);
    free(Md5Ctx);
};


#if CONFIG_CODEC_TEST || (CONFIG_VERSION > IC_VERSION_DP_RevF)
void md5_process( md5_context *ctx, unsigned char data[64] )
{
    unsigned long X[16], A, B, C, D;

    GET_ULONG_LE( X[ 0], data,  0 );
    GET_ULONG_LE( X[ 1], data,  4 );
    GET_ULONG_LE( X[ 2], data,  8 );
    GET_ULONG_LE( X[ 3], data, 12 );
    GET_ULONG_LE( X[ 4], data, 16 );
    GET_ULONG_LE( X[ 5], data, 20 );
    GET_ULONG_LE( X[ 6], data, 24 );
    GET_ULONG_LE( X[ 7], data, 28 );
    GET_ULONG_LE( X[ 8], data, 32 );
    GET_ULONG_LE( X[ 9], data, 36 );
    GET_ULONG_LE( X[10], data, 40 );
    GET_ULONG_LE( X[11], data, 44 );
    GET_ULONG_LE( X[12], data, 48 );
    GET_ULONG_LE( X[13], data, 52 );
    GET_ULONG_LE( X[14], data, 56 );
    GET_ULONG_LE( X[15], data, 60 );

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define P(a,b,c,d,k,s,t)                                \
{                                                       \
    a += F(b,c,d) + X[k] + t; a = S(a,s) + b;           \
}

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];

#define F(x,y,z) (z ^ (x & (y ^ z)))

    P( A, B, C, D,  0,  7, 0xD76AA478 );
    P( D, A, B, C,  1, 12, 0xE8C7B756 );
    P( C, D, A, B,  2, 17, 0x242070DB );
    P( B, C, D, A,  3, 22, 0xC1BDCEEE );
    P( A, B, C, D,  4,  7, 0xF57C0FAF );
    P( D, A, B, C,  5, 12, 0x4787C62A );
    P( C, D, A, B,  6, 17, 0xA8304613 );
    P( B, C, D, A,  7, 22, 0xFD469501 );
    P( A, B, C, D,  8,  7, 0x698098D8 );
    P( D, A, B, C,  9, 12, 0x8B44F7AF );
    P( C, D, A, B, 10, 17, 0xFFFF5BB1 );
    P( B, C, D, A, 11, 22, 0x895CD7BE );
    P( A, B, C, D, 12,  7, 0x6B901122 );
    P( D, A, B, C, 13, 12, 0xFD987193 );
    P( C, D, A, B, 14, 17, 0xA679438E );
    P( B, C, D, A, 15, 22, 0x49B40821 );

#undef F

#define F(x,y,z) (y ^ (z & (x ^ y)))

    P( A, B, C, D,  1,  5, 0xF61E2562 );
    P( D, A, B, C,  6,  9, 0xC040B340 );
    P( C, D, A, B, 11, 14, 0x265E5A51 );
    P( B, C, D, A,  0, 20, 0xE9B6C7AA );
    P( A, B, C, D,  5,  5, 0xD62F105D );
    P( D, A, B, C, 10,  9, 0x02441453 );
    P( C, D, A, B, 15, 14, 0xD8A1E681 );
    P( B, C, D, A,  4, 20, 0xE7D3FBC8 );
    P( A, B, C, D,  9,  5, 0x21E1CDE6 );
    P( D, A, B, C, 14,  9, 0xC33707D6 );
    P( C, D, A, B,  3, 14, 0xF4D50D87 );
    P( B, C, D, A,  8, 20, 0x455A14ED );
    P( A, B, C, D, 13,  5, 0xA9E3E905 );
    P( D, A, B, C,  2,  9, 0xFCEFA3F8 );
    P( C, D, A, B,  7, 14, 0x676F02D9 );
    P( B, C, D, A, 12, 20, 0x8D2A4C8A );

#undef F
    
#define F(x,y,z) (x ^ y ^ z)

    P( A, B, C, D,  5,  4, 0xFFFA3942 );
    P( D, A, B, C,  8, 11, 0x8771F681 );
    P( C, D, A, B, 11, 16, 0x6D9D6122 );
    P( B, C, D, A, 14, 23, 0xFDE5380C );
    P( A, B, C, D,  1,  4, 0xA4BEEA44 );
    P( D, A, B, C,  4, 11, 0x4BDECFA9 );
    P( C, D, A, B,  7, 16, 0xF6BB4B60 );
    P( B, C, D, A, 10, 23, 0xBEBFBC70 );
    P( A, B, C, D, 13,  4, 0x289B7EC6 );
    P( D, A, B, C,  0, 11, 0xEAA127FA );
    P( C, D, A, B,  3, 16, 0xD4EF3085 );
    P( B, C, D, A,  6, 23, 0x04881D05 );
    P( A, B, C, D,  9,  4, 0xD9D4D039 );
    P( D, A, B, C, 12, 11, 0xE6DB99E5 );
    P( C, D, A, B, 15, 16, 0x1FA27CF8 );
    P( B, C, D, A,  2, 23, 0xC4AC5665 );

#undef F

#define F(x,y,z) (y ^ (x | ~z))

    P( A, B, C, D,  0,  6, 0xF4292244 );
    P( D, A, B, C,  7, 10, 0x432AFF97 );
    P( C, D, A, B, 14, 15, 0xAB9423A7 );
    P( B, C, D, A,  5, 21, 0xFC93A039 );
    P( A, B, C, D, 12,  6, 0x655B59C3 );
    P( D, A, B, C,  3, 10, 0x8F0CCC92 );
    P( C, D, A, B, 10, 15, 0xFFEFF47D );
    P( B, C, D, A,  1, 21, 0x85845DD1 );
    P( A, B, C, D,  8,  6, 0x6FA87E4F );
    P( D, A, B, C, 15, 10, 0xFE2CE6E0 );
    P( C, D, A, B,  6, 15, 0xA3014314 );
    P( B, C, D, A, 13, 21, 0x4E0811A1 );
    P( A, B, C, D,  4,  6, 0xF7537E82 );
    P( D, A, B, C, 11, 10, 0xBD3AF235 );
    P( C, D, A, B,  2, 15, 0x2AD7D2BB );
    P( B, C, D, A,  9, 21, 0xEB86D391 );

#undef F

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
}

/*
 * MD5 process buffer
 */
void md5_update( md5_context *ctx, unsigned char *input, int ilen )
{
    int fill;
    unsigned long left;

    if( ilen <= 0 )
        return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if( ctx->total[0] < (unsigned long) ilen )
        ctx->total[1]++;

    if( left && ilen >= fill )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, fill );
        md5_process( ctx, ctx->buffer );
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while( ilen >= 64 )
    {
        md5_process( ctx, input );
        input += 64;
        ilen  -= 64;
    }

    if( ilen > 0 )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, ilen );
    }
}

void md5_finish( md5_context *ctx, unsigned char output[16] )
{
    unsigned long last, padn;
    unsigned long high, low;
    unsigned char msglen[8];

    high = ( ctx->total[0] >> 29 )
         | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_ULONG_LE( low,  msglen, 0 );
    PUT_ULONG_LE( high, msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    md5_update( ctx, (unsigned char *) md5_padding, padn );
    md5_update( ctx, msglen, 8 );

    PUT_ULONG_LE( ctx->state[0], output,  0 );
    PUT_ULONG_LE( ctx->state[1], output,  4 );
    PUT_ULONG_LE( ctx->state[2], output,  8 );
    PUT_ULONG_LE( ctx->state[3], output, 12 );
}

/*
 * output = MD5( input buffer )
 */
void md5( unsigned char *input, int ilen, unsigned char output[16] )
{
    //md5_context ctx;
    md5_context *ctx = malloc(sizeof(md5_context));

    md5_starts( ctx );
    md5_update( ctx, input, ilen );
    md5_finish( ctx, output );

    free(ctx);
    //memset( &ctx, 0, sizeof( md5_context ) );
}

void md5_hmac_starts( md5_context *ctx, unsigned char *key, int keylen )
{
    int i;
    unsigned char sum[16];

    if( keylen > 64 )
    {
        md5( key, keylen, sum );
        keylen = 16;
        key = sum;
    }

    memset( ctx->ipad, 0x36, 64 );
    memset( ctx->opad, 0x5C, 64 );

    for( i = 0; i < keylen; i++ )
    {
        ctx->ipad[i] = (unsigned char)( ctx->ipad[i] ^ key[i] );
        ctx->opad[i] = (unsigned char)( ctx->opad[i] ^ key[i] );
    }

    md5_starts( ctx );
    md5_update( ctx, ctx->ipad, 64 );

    memset( sum, 0, sizeof( sum ) );
}

/*
 * MD5 HMAC process buffer
 */
void md5_hmac_update( md5_context *ctx, unsigned char *input, int ilen )
{
    md5_update( ctx, input, ilen );
}

/*
 * MD5 HMAC final digest
 */
void md5_hmac_finish( md5_context *ctx, unsigned char output[16] )
{
    unsigned char tmpbuf[16];

    md5_finish( ctx, tmpbuf );
    md5_starts( ctx );
    md5_update( ctx, ctx->opad, 64 );
    md5_update( ctx, tmpbuf, 16 );
    md5_finish( ctx, output );

    memset( tmpbuf, 0, sizeof( tmpbuf ) );
}

/*
 * output = HMAC-MD5( hmac key, input buffer )
 */
void md5_hmacSW( unsigned char *key, int keylen, unsigned char *input, int ilen,
               unsigned char output[16] )
{
    //md5_context ctx;	
    md5_context *ctx = malloc(sizeof(md5_context));

    md5_hmac_starts( ctx, key, keylen );
    md5_hmac_update( ctx, input, ilen );
    md5_hmac_finish( ctx, output );

    free(ctx);
    //memset( &ctx, 0, sizeof( md5_context ) );
}
#endif

