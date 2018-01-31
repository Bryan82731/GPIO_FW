#include "hash.h"
#include "sha1.h"
#include <stdlib.h>
#include <string.h>
#include "md5.h"

extern  krb5_keyusage krb5int_arcfour_translate_usage(krb5_keyusage usage);


/*
 * the HMAC transform looks like:
 *
 * H(K XOR opad, H(K XOR ipad, text))
 *
 * where H is a cryptographic hash
 * K is an n byte key
 * ipad is the byte 0x36 repeated blocksize times
 * opad is the byte 0x5c repeated blocksize times
 * and text is the data being protected
 */
krb5_error_code krb5_hmac(const struct krb5_hash_provider *hash, const krb5_keyblock *key, unsigned int icount,
                          const krb5_data *input, krb5_data *output)
{
    int hashsize, blocksize;
    unsigned char *xorkey, *ihash;
    int i;
    krb5_data *hashin, hashout;
    krb5_error_code ret;

    hashsize = hash->hashsize;
    blocksize = hash->blocksize;

    if (key->length > blocksize)
        return(KRB5_CRYPTO_INTERNAL);
    if (output->length < hashsize)
        return(KRB5_BAD_MSIZE);
    /* if this isn't > 0, then there won't be enough space in this
       array to compute the outer hash */
    if (icount == 0)
        return(KRB5_CRYPTO_INTERNAL);

    /* allocate space for the xor key, hash input vector, and inner hash */

    if ((xorkey = (unsigned char *) malloc(blocksize)) == NULL)
        return(-1);
    if ((ihash = (unsigned char *) malloc(hashsize)) == NULL) {
        free(xorkey);
        return(-1);
    }
    if ((hashin = (krb5_data *)malloc(sizeof(krb5_data)*(icount+1))) == NULL) {
        free(ihash);
        free(xorkey);
        return(-1);
    }

    /* create the inner padded key */

    memset(xorkey, 0x36, blocksize);

    for (i=0; i<key->length; i++)
        xorkey[i] ^= key->contents[i];

    /* compute the inner hash */

    for (i=0; i<icount; i++)
    {
        hashin[0].length = blocksize;
        hashin[0].data = (char *) xorkey;
        hashin[i+1] = input[i];
    }

    hashout.length = hashsize;
    hashout.data = (char *) ihash;

    if ((ret = ((*(hash->hash))(icount+1, hashin, &hashout))))
        goto cleanup;

    /* create the outer padded key */

    memset(xorkey, 0x5c, blocksize);

    for (i=0; i<key->length; i++)
        xorkey[i] ^= key->contents[i];

    /* compute the outer hash */

    hashin[0].length = blocksize;
    hashin[0].data = (char *) xorkey;
    hashin[1] = hashout;

    output->length = hashsize;

    if ((ret = ((*(hash->hash))(2, hashin, output))))
        memset(output->data, 0, output->length);

    /* ret is set correctly by the prior call */

cleanup:
    memset(xorkey, 0, blocksize);
    memset(ihash, 0, hashsize);

    free(hashin);
    free(ihash);
    free(xorkey);

    return(ret);
}


/*
microsoft rc4-hmac checksum
K = the Key
T = the message type, encoded as a little-endian four-byte integer

CHKSUM(K, T, data)
	Ksign = HMAC(K, "signaturekey") //includes zero octet at end
	tmp = MD5(concat(T, data))
	CHKSUM = HMAC(Ksign, tmp)
*/
static krb5_error_code k5_hmac_md5_hash (const krb5_keyblock *key, krb5_keyusage usage, const krb5_data *iv,
        const krb5_data *input, krb5_data *output)
{
    krb5_keyusage ms_usage;
    krb5_error_code ret;
    krb5_keyblock ks;
    krb5_data ds, ks_constant, md5tmp;
    md5_context ctx;
    unsigned char digest[16];
    char t[4];

    ds.length = key->length;
    ks.length = key->length;
    ds.data = malloc(ds.length);
    if (ds.data == NULL)
        return -1;
    ks.contents = (void *) ds.data;

    ks_constant.data = "signaturekey";
    ks_constant.length = strlen(ks_constant.data)+1; /* Including null*/

    //Ksign = HMAC(K, "signaturekey") //includes zero octet at end
    md5_hmac(key->contents, key->length, ks_constant.data, ks_constant.length, ds.data);
    md5_starts(&ctx);
    ms_usage = krb5int_arcfour_translate_usage (usage);
    t[0] = (ms_usage) & 0xff;
    t[1] = (ms_usage>>8) & 0xff;
    t[2] = (ms_usage >>16) & 0xff;
    t[3] = (ms_usage>>24) & 0XFF;
    //tmp = MD5(concat(T, data))
    md5_updateH(&ctx, (unsigned char * ) &t, 4);
    md5_updateH(&ctx, (unsigned char *) input-> data, (unsigned int) input->length);
    md5_finishH(&ctx, digest);
    md5tmp.data = (void *) digest;
    md5tmp.length = 16;
    //CHKSUM = HMAC(Ksign, tmp)
    md5_hmac(ks.contents, ks.length, md5tmp.data, md5tmp.length, output->data);
    ret = 0;
    memset(&ctx, 0, sizeof(ctx));
    memset (ks.contents, 0, ks.length);
    free (ks.contents);
    return ret;
}

static krb5_error_code
k5_sha1_hash(unsigned int icount, const krb5_data *input,
             krb5_data *output)
{
    //SHS_INFO ctx;
    sha1_context ctx;
    int i;

    if (output->length != SHA1_DIGESTSIZE)
        return(KRB5_CRYPTO_INTERNAL);

    //shsInit(&ctx);
    sha1_starts( &ctx );
    for (i=0; i<icount; i++)
    {
        //shsUpdate(&ctx, (unsigned char *) input[i].data, input[i].length);
        sha1_update( &ctx, (unsigned char *) input[i].data, input[i].length );
    }
    //shsFinal(&ctx);
    sha1_finish( &ctx, output->data );

//   for (i=0; i<(sizeof(ctx.digest)/sizeof(ctx.digest[0])); i++) {
    //output->data[i*4] = (ctx.digest[i]>>24)&0xff;
    //output->data[i*4+1] = (ctx.digest[i]>>16)&0xff;
    //output->data[i*4+2] = (ctx.digest[i]>>8)&0xff;
    //output->data[i*4+3] = ctx.digest[i]&0xff;
//   }


    return(0);
}

const struct krb5_hash_provider krb5int_hash_md5 = {
    RSA_MD5_CKSUM_LENGTH,
    64,
    NULL
    /*k5_md5_hash*/
};

const struct krb5_keyhash_provider krb5int_keyhash_hmac_md5 = {
    16,
    k5_hmac_md5_hash,
    NULL /*checksum  again*/
};


const struct krb5_hash_provider krb5int_hash_sha1 = {
    SHA1_DIGESTSIZE,
    SHA1_DATASIZE,
    k5_sha1_hash
};
