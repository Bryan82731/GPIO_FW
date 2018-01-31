/*
 *  SSLv3/TLSv1 shared functions
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
 *  The SSL 3.0 specification was drafted by Netscape in 1996,
 *  and became an IETF standard in 1999.
 *
 *  http://wp.netscape.com/eng/ssl3/
 *  http://www.ietf.org/rfc/rfc2246.txt
 *  http://www.ietf.org/rfc/rfc4346.txt
 */
#include "rsasw.h"
#include "md5.h"
#include "sha1.h"
#include "bsp.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "sys/autoconf.h"

#if CONFIG_VERSION < IC_VERSION_DP_RevF || CONFIG_BUILDROM

/*
 * Key material generation
 */
int tls1_prf( unsigned char *secret, int slen, char *label,
              unsigned char *random, int rlen,
              unsigned char *dstbuf, int dlen )
{
#define XYSSL_ERR_SSL_BAD_INPUT_DATA                    -0x1800

    int nb, hs;
    int i, j, k;
    unsigned char *S1, *S2;
    unsigned char tmp[128];
    unsigned char h_i[20];

    if ( sizeof( tmp ) < 20 + strlen( label ) + rlen )
        return ( XYSSL_ERR_SSL_BAD_INPUT_DATA );

    hs = ( slen + 1 ) / 2;
    S1 = secret;
    S2 = secret + slen - hs;

    nb = strlen( label );
    memcpy( tmp + 20, label, nb );
    memcpy( tmp + 20 + nb, random, rlen );
    nb += rlen;

    /*
     * First compute P_md5(secret,label+random)[0..dlen]
     */
    md5_hmac( S1, hs, tmp + 20, nb, 4 + tmp );

    for ( i = 0; i < dlen; i += 16 )
    {
        md5_hmac( S1, hs, 4 + tmp, 16 + nb, h_i );
        md5_hmac( S1, hs, 4 + tmp, 16,  4 + tmp );

        k = ( i + 16 > dlen ) ? dlen % 16 : 16;

        for ( j = 0; j < k; j++ )
            dstbuf[i + j]  = h_i[j];
    }

    /*
     * XOR out with P_sha1(secret,label+random)[0..dlen]
     */
    sha1_hmac( S2, hs, tmp + 20, nb, tmp );

    for ( i = 0; i < dlen; i += 20 )
    {
        sha1_hmac( S2, hs, tmp, 20 + nb, h_i );
        sha1_hmac( S2, hs, tmp, 20,      tmp );

        k = ( i + 20 > dlen ) ? dlen % 20 : 20;

        for ( j = 0; j < k; j++ )
            dstbuf[i + j] = (unsigned char)( dstbuf[i + j] ^ h_i[j] );
    }

    memset( tmp, 0, sizeof( tmp ) );
    memset( h_i, 0, sizeof( h_i ) );

    return( 0 );
}

void ssl_calc_finished(unsigned char *master, unsigned char *buf, int from, md5_context *md5, sha1_context *sha1)
{
    int len = 12;
    char *sender;
    unsigned char padbuf[48];
    unsigned char md5sum[16];
    unsigned char sha1sum[20];

    /*
     * SSLv3:
     *   hash =
     *      MD5( master + pad2 +
     *          MD5( handshake + sender + master + pad1 ) )
     *   + SHA1( master + pad2 +
     *         SHA1( handshake + sender + master + pad1 ) )
     *
     * TLSv1:
     *   hash = PRF( master, finished_label,
     *               MD5( handshake ) + SHA1( handshake ) )[0..11]
     */


    sender = ( from == 1 )
             ? (char *) "client finished"
             : (char *) "server finished";
    //sender = (char *) "client finished";

    /**
     * \brief          MD5 final digest
     *
     * \param ctx      MD5 context
     * \param output   MD5 checksum result
     */
    md5_finishH(  md5, padbuf );
    sha1_finishH( sha1, padbuf + 16 );

    tls1_prf( master, 48, sender,
              padbuf, 36, buf, len );

    memset(  md5, 0, sizeof(  md5_context ) );
    memset( sha1, 0, sizeof( sha1_context ) );

    memset(  padbuf, 0, sizeof(  padbuf ) );
    memset(  md5sum, 0, sizeof(  md5sum ) );
    memset( sha1sum, 0, sizeof( sha1sum ) );
}

#endif
