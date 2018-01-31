/*
 *  FIPS-180-1 compliant SHA-1 implementation
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
 *  The SHA-1 standard was published by NIST in 1993.
 *
 *  http://www.itl.nist.gov/fipspubs/fip180-1.htm
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bsp.h"
#include "sha1.h"

#if defined(CONFIG_BUILDROM) || (CONFIG_VERSION  < IC_VERSION_DP_RevF)

#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
{                                                       \
    (n) = ( (unsigned long) (b)[(i)    ] << 24 )        \
        | ( (unsigned long) (b)[(i) + 1] << 16 )        \
        | ( (unsigned long) (b)[(i) + 2] <<  8 )        \
        | ( (unsigned long) (b)[(i) + 3]       );       \
}
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}
#endif


static const unsigned char sha1_padding[64] =
{
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#if CONFIG_VERSION <= IC_VERSION_DP_RevF || CONFIG_VERSION >= IC_VERSION_FP_RevA
void sha1_startsH( sha1_context *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    //ctx->state[0] = 0x67452301;
    //ctx->state[1] = 0xEFCDAB89;
    //ctx->state[2] = 0x98BADCFE;
    //ctx->state[3] = 0x10325476;
    //ctx->state[4] = 0xC3D2E1F0;
    ctx->state[0] = 0x01234567;
    ctx->state[1] = 0x89ABCDEF;
    ctx->state[2] = 0xFEDCBA98;
    ctx->state[3] = 0x76543210;
    ctx->state[4] = 0xF0E1D2C3;
}

void sha1_processH( sha1_context *ctx, unsigned char* data, int len)
{
	unsigned char* tmp;
    //Must 52
	unsigned char* hk;
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	hk = malloc(52);
#else
	hk = malloc(64+20);
#endif
	tmp = malloc(len);

    memcpy(tmp, data, len);
    memcpy(hk, ctx->state, 20);

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    tls_set(hk, tmp, 0x00, len);
#else
	tls_set(hk, tmp, SHA1_HASH, len);
#endif

    if (tls_poll() == -1)
        ;
    else
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
        memcpy(ctx->state, hk, 20);
#else
	memcpy(ctx->state, hk+64, 20);
#endif

    free(hk);

	if(tmp != data)
	{
    free(tmp);
}
}

void sha1_updateH( sha1_context *ctx, unsigned char *input, int ilen )
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
        //To HW padding message
        sha1_processH( ctx, ctx->buffer, 64 );
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while( ilen >= 64 )
    {
    		if(ilen >= 1600)
    		{        
        	sha1_processH( ctx, input, 1600 );
        	input += 1600;
        	ilen  -= 1600;        	        	
        }
        else
        {
        	t = (ilen % 64);
        	sha1_processH( ctx, input, ilen - t);
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

void sha1_finishH( sha1_context *ctx, unsigned char output[20] )
{
    unsigned long last, padn;
    unsigned long high, low;
    unsigned char msglen[8];

    high = ( ctx->total[0] >> 29 )
           | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_ULONG_BE( high, msglen, 0 );
    PUT_ULONG_BE( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    sha1_updateH( ctx, (unsigned char *) sha1_padding, padn );
    sha1_updateH( ctx, msglen, 8 );
    memcpy(output, ctx->state, 20);
}

void sha1H( unsigned char *input, int ilen, unsigned char output[20] )
{
    //sha1_context ctx;
    sha1_context *ctx = malloc(sizeof(sha1_context));

    sha1_startsH( ctx );
    sha1_updateH( ctx, input, ilen );
    sha1_finishH( ctx, output );

    free(ctx);
    //memset( &ctx, 0, sizeof( sha1_context ) );  
}

void sha1_hmac_startsH( sha1_context *ctx, unsigned char *key, int keylen )
{
    int i;
    unsigned char sum[20];

    if( keylen > 64 )
    {
        sha1H( key, keylen, sum );
        keylen = 20;
        key = sum;
    }

    memset( ctx->ipad, 0x36, 64 );
    memset( ctx->opad, 0x5C, 64 );

    for( i = 0; i < keylen; i++ )
    {
        ctx->ipad[i] = (unsigned char)( ctx->ipad[i] ^ key[i] );
        ctx->opad[i] = (unsigned char)( ctx->opad[i] ^ key[i] );
    }

    sha1_startsH( ctx );
    sha1_updateH( ctx, ctx->ipad, 64 );

    memset( sum, 0, sizeof( sum ) );
}

/*
 * SHA-1 HMAC process buffer
 */

void sha1_hmac_updateH( sha1_context *ctx, unsigned char *input, int ilen )
{
    sha1_updateH( ctx, input, ilen );
}

/*
 * SHA-1 HMAC final digest
 */
void sha1_hmac_finishH( sha1_context *ctx, unsigned char output[20] )
{
    unsigned char tmpbuf[20];

    sha1_finishH( ctx, tmpbuf );
    sha1_startsH( ctx );
    sha1_updateH( ctx, ctx->opad, 64 );
    sha1_updateH( ctx, tmpbuf, 20 );
    sha1_finishH( ctx, output );

    memset( tmpbuf, 0, sizeof( tmpbuf ) );
}

/*
 * output = HMAC-SHA-1( hmac key, input buffer )
 */
void sha1_hmac( unsigned char *key, int keylen, unsigned char *input, int ilen,
                unsigned char output[20] )
{
    unsigned char *hk = 0;
    unsigned char *inData = 0;

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
    if(ilen > 1600 || (keylen != 20 && keylen != 24))
#else
	if(ilen > 1600 || (keylen != 20))
#endif			
    {
    	sha1_hmacSWHash(key, keylen, input, ilen, output);
    	return;
    }
    
    if (keylen == 20)
    {
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
        hk = malloc(20);
#else
		hk = malloc(64+20);
		memset(hk, 0, 84);
#endif
        inData = malloc(ilen);

        memcpy(hk, key, 20);
        memcpy(inData, input, ilen);
        tls_set(hk, inData, SHA1_HMAC_20, ilen);

        if (tls_poll() == -1)
            ;
        else
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
            memcpy(output, hk, 20);
#else
			memcpy(output, hk+0x40, 20);
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
        tls_set(hk, inData, 0x5, ilen);

        if (tls_poll() == -1)
            ;
        else
            memcpy(output, hk, 20);

        free(hk);
        free(inData);
    }
    else
    {
        ;
    }
}
#endif

void sha1_hmacSWHash( unsigned char *key, int keylen, unsigned char *input, int ilen, unsigned char output[20] )
{
    //sha1_context ctx;
    sha1_context *ctx = malloc(sizeof(sha1_context));

    sha1_hmac_starts( ctx, key, keylen );
    sha1_hmac_update( ctx, input, ilen );
    sha1_hmac_finish( ctx, output );

    //memset( &ctx, 0, sizeof( sha1_context ) );
    free(ctx);
}


void sha1_starts( sha1_context *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}

void sha1_process( sha1_context *ctx, unsigned char data[64] )
{
    unsigned long temp, W[16], A, B, C, D, E;

    GET_ULONG_BE( W[ 0], data,  0 );
    GET_ULONG_BE( W[ 1], data,  4 );
    GET_ULONG_BE( W[ 2], data,  8 );
    GET_ULONG_BE( W[ 3], data, 12 );
    GET_ULONG_BE( W[ 4], data, 16 );
    GET_ULONG_BE( W[ 5], data, 20 );
    GET_ULONG_BE( W[ 6], data, 24 );
    GET_ULONG_BE( W[ 7], data, 28 );
    GET_ULONG_BE( W[ 8], data, 32 );
    GET_ULONG_BE( W[ 9], data, 36 );
    GET_ULONG_BE( W[10], data, 40 );
    GET_ULONG_BE( W[11], data, 44 );
    GET_ULONG_BE( W[12], data, 48 );
    GET_ULONG_BE( W[13], data, 52 );
    GET_ULONG_BE( W[14], data, 56 );
    GET_ULONG_BE( W[15], data, 60 );

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                                            \
(                                                       \
    temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^     \
           W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],      \
    ( W[t & 0x0F] = S(temp,1) )                         \
)

#define P(a,b,c,d,e,x)                                  \
{                                                       \
    e += S(a,5) + F(b,c,d) + K + x; b = S(b,30);        \
}

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];

#define F(x,y,z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

    P( A, B, C, D, E, W[0]  );
    P( E, A, B, C, D, W[1]  );
    P( D, E, A, B, C, W[2]  );
    P( C, D, E, A, B, W[3]  );
    P( B, C, D, E, A, W[4]  );
    P( A, B, C, D, E, W[5]  );
    P( E, A, B, C, D, W[6]  );
    P( D, E, A, B, C, W[7]  );
    P( C, D, E, A, B, W[8]  );
    P( B, C, D, E, A, W[9]  );
    P( A, B, C, D, E, W[10] );
    P( E, A, B, C, D, W[11] );
    P( D, E, A, B, C, W[12] );
    P( C, D, E, A, B, W[13] );
    P( B, C, D, E, A, W[14] );
    P( A, B, C, D, E, W[15] );
    P( E, A, B, C, D, R(16) );
    P( D, E, A, B, C, R(17) );
    P( C, D, E, A, B, R(18) );
    P( B, C, D, E, A, R(19) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0x6ED9EBA1

    P( A, B, C, D, E, R(20) );
    P( E, A, B, C, D, R(21) );
    P( D, E, A, B, C, R(22) );
    P( C, D, E, A, B, R(23) );
    P( B, C, D, E, A, R(24) );
    P( A, B, C, D, E, R(25) );
    P( E, A, B, C, D, R(26) );
    P( D, E, A, B, C, R(27) );
    P( C, D, E, A, B, R(28) );
    P( B, C, D, E, A, R(29) );
    P( A, B, C, D, E, R(30) );
    P( E, A, B, C, D, R(31) );
    P( D, E, A, B, C, R(32) );
    P( C, D, E, A, B, R(33) );
    P( B, C, D, E, A, R(34) );
    P( A, B, C, D, E, R(35) );
    P( E, A, B, C, D, R(36) );
    P( D, E, A, B, C, R(37) );
    P( C, D, E, A, B, R(38) );
    P( B, C, D, E, A, R(39) );

#undef K
#undef F

#define F(x,y,z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

    P( A, B, C, D, E, R(40) );
    P( E, A, B, C, D, R(41) );
    P( D, E, A, B, C, R(42) );
    P( C, D, E, A, B, R(43) );
    P( B, C, D, E, A, R(44) );
    P( A, B, C, D, E, R(45) );
    P( E, A, B, C, D, R(46) );
    P( D, E, A, B, C, R(47) );
    P( C, D, E, A, B, R(48) );
    P( B, C, D, E, A, R(49) );
    P( A, B, C, D, E, R(50) );
    P( E, A, B, C, D, R(51) );
    P( D, E, A, B, C, R(52) );
    P( C, D, E, A, B, R(53) );
    P( B, C, D, E, A, R(54) );
    P( A, B, C, D, E, R(55) );
    P( E, A, B, C, D, R(56) );
    P( D, E, A, B, C, R(57) );
    P( C, D, E, A, B, R(58) );
    P( B, C, D, E, A, R(59) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6

    P( A, B, C, D, E, R(60) );
    P( E, A, B, C, D, R(61) );
    P( D, E, A, B, C, R(62) );
    P( C, D, E, A, B, R(63) );
    P( B, C, D, E, A, R(64) );
    P( A, B, C, D, E, R(65) );
    P( E, A, B, C, D, R(66) );
    P( D, E, A, B, C, R(67) );
    P( C, D, E, A, B, R(68) );
    P( B, C, D, E, A, R(69) );
    P( A, B, C, D, E, R(70) );
    P( E, A, B, C, D, R(71) );
    P( D, E, A, B, C, R(72) );
    P( C, D, E, A, B, R(73) );
    P( B, C, D, E, A, R(74) );
    P( A, B, C, D, E, R(75) );
    P( E, A, B, C, D, R(76) );
    P( D, E, A, B, C, R(77) );
    P( C, D, E, A, B, R(78) );
    P( B, C, D, E, A, R(79) );

#undef K
#undef F

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
    ctx->state[4] += E;
}

/*
 * SHA-1 process buffer
 */
void sha1_update( sha1_context *ctx, unsigned char *input, int ilen )
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
        sha1_process( ctx, ctx->buffer );
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while( ilen >= 64 )
    {
        sha1_process( ctx, input );
        input += 64;
        ilen  -= 64;
    }

    if( ilen > 0 )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, ilen );
    }
}

void sha1_finish( sha1_context *ctx, unsigned char output[20] )
{
    unsigned long last, padn;
    unsigned long high, low;
    unsigned char msglen[8];

    high = ( ctx->total[0] >> 29 )
         | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_ULONG_BE( high, msglen, 0 );
    PUT_ULONG_BE( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    sha1_update( ctx, (unsigned char *) sha1_padding, padn );
    sha1_update( ctx, msglen, 8 );

    PUT_ULONG_BE( ctx->state[0], output,  0 );
    PUT_ULONG_BE( ctx->state[1], output,  4 );
    PUT_ULONG_BE( ctx->state[2], output,  8 );
    PUT_ULONG_BE( ctx->state[3], output, 12 );
    PUT_ULONG_BE( ctx->state[4], output, 16 );
}

void sha1( unsigned char *input, int ilen, unsigned char output[20] )
{
    sha1_context ctx;

    sha1_starts( &ctx );
    sha1_update( &ctx, input, ilen );
    sha1_finish( &ctx, output );

}

/*
 * output = SHA-1( file contents )
 */
#if 0
int sha1_file( char *path, unsigned char output[20] )
{
    FILE *f;
    size_t n;
    sha1_context ctx;
    unsigned char buf[1024];

    if( ( f = fopen( path, "rb" ) ) == NULL )
        return( 1 );

    sha1_starts( &ctx );

    while( ( n = fread( buf, 1, sizeof( buf ), f ) ) > 0 )
        sha1_update( &ctx, buf, (int) n );

    sha1_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sha1_context ) );

    if( ferror( f ) != 0 )
    {
        fclose( f );
        return( 2 );
    }

    fclose( f );
    return( 0 );
}
#endif
/*
 * SHA-1 HMAC context setup
 */
void sha1_hmac_starts( sha1_context *ctx, unsigned char *key, int keylen )
{
    int i;
    unsigned char sum[20];

    if( keylen > 64 )
    {
        sha1( key, keylen, sum );
        keylen = 20;
        key = sum;
    }

    memset( ctx->ipad, 0x36, 64 );
    memset( ctx->opad, 0x5C, 64 );

    for( i = 0; i < keylen; i++ )
    {
        ctx->ipad[i] = (unsigned char)( ctx->ipad[i] ^ key[i] );
        ctx->opad[i] = (unsigned char)( ctx->opad[i] ^ key[i] );
    }

    sha1_starts( ctx );
    sha1_update( ctx, ctx->ipad, 64 );

    memset( sum, 0, sizeof( sum ) );
}

/*
 * SHA-1 HMAC process buffer
 */
void sha1_hmac_update( sha1_context *ctx, unsigned char *input, int ilen )
{
    sha1_update( ctx, input, ilen );
}

/*
 * SHA-1 HMAC final digest
 */
void sha1_hmac_finish( sha1_context *ctx, unsigned char output[20] )
{
    unsigned char tmpbuf[20];

    sha1_finish( ctx, tmpbuf );
    sha1_starts( ctx );
    sha1_update( ctx, ctx->opad, 64 );
    sha1_update( ctx, tmpbuf, 20 );
    sha1_finish( ctx, output );

    memset( tmpbuf, 0, sizeof( tmpbuf ) );
}

/*
 * output = HMAC-SHA-1( hmac key, input buffer )
 */
void sha1_hmacSW( unsigned char *key, int keylen, unsigned char *input, int ilen, unsigned char output[20] )
{
    //sha1_context ctx;
    sha1_context *ctx = malloc(sizeof(sha1_context));

    //memset(ctx, 0, sizeof(sha1_context));

    sha1_hmac_starts( ctx, key, keylen );
    sha1_hmac_update( ctx, input, ilen );
    sha1_hmac_finish( ctx, output );

    free(ctx);
    //memset( &ctx, 0, sizeof( sha1_context ) );
}
#endif
