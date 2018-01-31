#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sha1.h"
#include "krb_aes.h"
#include "kerberos.h"
#include "hash.h"

#define K5CLENGTH 5 /* 32 bit net byte order integer + one byte seed */

#define aes_bad     0           /* bad function return value            */
#define aes_good    1           /* good function return value           */
#define KV5M_KEYBLOCK                            (-1760647421L)
#define enc(OUT, IN, CTX) (aes_crypt_ecb((CTX),AES_ENCRYPT,(IN),(OUT)) == aes_good ?  0 : abort())
#define dec(OUT, IN, CTX) (aes_crypt_ecb((CTX),AES_DECRYPT,(IN),(OUT)) == aes_good ?  0 : abort())
#if 0
void krb5_dk_encrypt_length(const struct krb5_enc_provider *enc,
                            const struct krb5_hash_provider *hash,
                            int inputlen, int *length)
{
    int blocksize, hashsize;

    blocksize = enc->block_size;
    hashsize = hash->hashsize;
    *length = krb5_roundup(blocksize+inputlen, blocksize) + hashsize;
}

krb5_error_code
krb5_dk_encrypt(const struct krb5_enc_provider *enc,
                const struct krb5_hash_provider *hash,
                const krb5_keyblock *key, krb5_keyusage usage,
                const krb5_data *ivec, const krb5_data *input,
                krb5_data *output)
{
    int blocksize, keybytes, keylength, plainlen, enclen;
    krb5_error_code ret;
    unsigned char constantdata[K5CLENGTH];
    krb5_data d1, d2;
    unsigned char *plaintext, *kedata, *kidata, *cn;
    krb5_keyblock ke, ki;

    /* allocate and set up plaintext and to-be-derived keys */

    blocksize = enc->block_size;
    keybytes = enc->keybytes;
    keylength = enc->keylength;
    plainlen = krb5_roundup(blocksize+input->length, blocksize);

    krb5_dk_encrypt_length(enc, hash, input->length, &enclen);

    /* key->length, ivec will be tested in enc->encrypt */

    if (output->length < enclen)
        return(KRB5_BAD_MSIZE);

    if ((kedata = (unsigned char *) malloc(keylength)) == NULL)
        return(ENOMEM);
    if ((kidata = (unsigned char *) malloc(keylength)) == NULL) {
        free(kedata);
        return(ENOMEM);
    }
    if ((plaintext = (unsigned char *) malloc(plainlen)) == NULL) {
        free(kidata);
        free(kedata);
        return(ENOMEM);
    }

    ke.contents = kedata;
    ke.length = keylength;
    ki.contents = kidata;
    ki.length = keylength;

    /* derive the keys */

    d1.data = constantdata;
    d1.length = K5CLENGTH;

    d1.data[0] = (usage>>24)&0xff;
    d1.data[1] = (usage>>16)&0xff;
    d1.data[2] = (usage>>8)&0xff;
    d1.data[3] = usage&0xff;

    d1.data[4] = (char) 0xAA;

    if ((ret = krb5_derive_key(enc, key, &ke, &d1)))
        goto cleanup;

    d1.data[4] = 0x55;

    if ((ret = krb5_derive_key(enc, key, &ki, &d1)))
        goto cleanup;

    /* put together the plaintext */

    d1.length = blocksize;
    d1.data = plaintext;

    if ((ret = krb5_c_random_make_octets(/* XXX */ 0, &d1)))
        goto cleanup;

    memcpy(plaintext+blocksize, input->data, input->length);

    memset(plaintext+blocksize+input->length, 0,
           plainlen - (blocksize+input->length));

    /* encrypt the plaintext */

    d1.length = plainlen;
    d1.data = plaintext;

    d2.length = plainlen;
    d2.data = output->data;

    if ((ret = ((*(enc->encrypt))(&ke, ivec, &d1, &d2))))
        goto cleanup;

    if (ivec != NULL && ivec->length == blocksize)
        cn = d2.data + d2.length - blocksize;
    else
        cn = NULL;

    /* hash the plaintext */

    d2.length = enclen - plainlen;
    d2.data = output->data+plainlen;

    output->length = enclen;

    if ((ret = krb5_hmac(hash, &ki, 1, &d1, &d2)))
    {
        memset(d2.data, 0, d2.length);
        goto cleanup;
    }

    /* update ivec */
    if (cn != NULL)
        memcpy(ivec->data, cn, blocksize);

    /* ret is set correctly by the prior call */

cleanup:
    memset(kedata, 0, keylength);
    memset(kidata, 0, keylength);
    memset(plaintext, 0, plainlen);

    free(plaintext);
    free(kidata);
    free(kedata);

    return(ret);
}
#endif

/* Not necessarily "AES", per se, but "a CBC+CTS mode block cipher
   with a 96-bit truncated HMAC".  */
void
krb5int_aes_encrypt_length(const struct krb5_enc_provider *enc, const struct krb5_hash_provider *hash, int inputlen, int *length)
{
    int blocksize, hashsize;

    blocksize = enc->block_size;
    hashsize = 96 / 8;

    /* No roundup, since CTS requires no padding once we've hit the
       block size.  */
    *length = blocksize+inputlen + hashsize;
}

static krb5_error_code trunc_hmac(const struct krb5_hash_provider *hash, const krb5_keyblock *ki, unsigned int num,
                                  const krb5_data *input, const krb5_data *output)
{
    unsigned int hashsize;
    krb5_data tmp;
    krb5_error_code ret;

    hashsize = hash->hashsize;
    if (hashsize < output->length)
        return KRB5_CRYPTO_INTERNAL;
    tmp.length = hashsize;
    tmp.data = malloc(hashsize);
    if (tmp.data == NULL)
        return -1;
    ret = krb5_hmac(hash, ki, num, input, &tmp);
    if (ret == 0)
        memcpy(output->data, tmp.data, output->length);
    memset(tmp.data, 0, hashsize);
    free(tmp.data);
    return ret;
}

krb5_error_code krb5int_aes_dk_encrypt(const struct krb5_enc_provider *enc, const struct krb5_hash_provider *hash,
                                       const krb5_keyblock *key, krb5_keyusage usage, const krb5_data *ivec,
                                       const krb5_data *input, krb5_data *output, char shift)
{
    int blocksize, keybytes, keylength, plainlen, enclen;
    krb5_error_code ret;
    unsigned char constantdata[K5CLENGTH];
    krb5_data d1, d2;
    unsigned char *plaintext, *kedata, *kidata, *cn;
    krb5_keyblock ke, ki;
    aes_context aes_ctx;

    /* allocate and set up plaintext and to-be-derived keys */

    blocksize = enc->block_size;
    keybytes = enc->keybytes;
    keylength = enc->keylength;
    plainlen = blocksize+input->length;

    krb5int_aes_encrypt_length(enc, hash, input->length, &enclen);

    /* key->length, ivec will be tested in enc->encrypt */

    if (output->length < enclen)
        return(KRB5_BAD_MSIZE);

    if ((kedata = (unsigned char *) malloc(keylength)) == NULL)
        return(-1);
    if ((kidata = (unsigned char *) malloc(keylength)) == NULL)
    {
        free(kedata);
        return(-1);
    }
    if ((plaintext = (unsigned char *) malloc(plainlen)) == NULL)
    {
        free(kidata);
        free(kedata);
        return(-1);
    }

    ke.contents = kedata;
    ke.length = keylength;
    ki.contents = kidata;
    ki.length = keylength;

    /* derive the keys */

    d1.data = constantdata;
    d1.length = K5CLENGTH;

    d1.data[0] = (usage>>24)&0xff;
    d1.data[1] = (usage>>16)&0xff;
    d1.data[2] = (usage>>8)&0xff;
    d1.data[3] = usage&0xff;

    d1.data[4] = (char) 0xAA;


    aes_setkey_enc_sw( &aes_ctx, (unsigned char *) (key->contents), 16*8 );
    //if ((ret = krb5_derive_key(enc, key, &ke, &d1)))
    if ((ret = krb5_derive_key_aes(&aes_ctx, &ki, &d1)) != 0)
        goto cleanup;

    d1.data[4] = 0x55;

    aes_setkey_enc_sw( &aes_ctx, (unsigned char *) (key->contents), 16*8 );
    //if ((ret = krb5_derive_key(enc, key, &ki, &d1)))
    if ((ret = krb5_derive_key_aes(&aes_ctx, &ki, &d1)) != 0)
        goto cleanup;

    /* put together the plaintext */

    d1.length = blocksize;
    d1.data = plaintext;

    if ((ret = krb5_c_random_make_octets(/* XXX */ 0, &d1)))
        goto cleanup;

    memcpy(plaintext+blocksize, input->data, input->length);

    /* Ciphertext stealing; there should be no more.  */
    if (plainlen != blocksize + input->length)
        abort();

    /* encrypt the plaintext */

    d1.length = plainlen;
    d1.data = plaintext;

    d2.length = plainlen;
    d2.data = output->data;

    //if ((ret = ((*(enc->encrypt))(&ke, ivec, &d1, &d2))))
    //	goto cleanup;
    aes_setkey_enc_sw( &aes_ctx, (unsigned char *) (ke.contents), 16*8 );
    krb5int_aes_encrypt(&aes_ctx, ivec, &d1, &d2);

    if (ivec != NULL && ivec->length == blocksize)
    {
        int nblocks = (d2.length + blocksize - 1) / blocksize;
        cn = d2.data + blocksize * (nblocks - 2);
    }
    else
        cn = NULL;

    /* hash the plaintext */

    d2.length = enclen - plainlen;
    d2.data = output->data+plainlen;
    if (d2.length != 96 / 8)
        abort();

    if ((ret = trunc_hmac(hash, &ki, 1, &d1, &d2)))
    {
        memset(d2.data, 0, d2.length);
        goto cleanup;
    }

    output->length = enclen;

    /* update ivec */
    if (cn != NULL)
    {
        memcpy(ivec->data, cn, blocksize);
#if 0
        {
            int i;
            printf("\n%s: output:", __func__);
            for (i = 0; i < output->length; i++) {
                if (i % 16 == 0)
                    printf("\n%s: ", __func__);
                printf(" %02x", i[(unsigned char *)output->data]);
            }
            printf("\n%s: outputIV:", __func__);
            for (i = 0; i < ivec->length; i++) {
                if (i % 16 == 0)
                    printf("\n%s: ", __func__);
                printf(" %02x", i[(unsigned char *)ivec->data]);
            }
            printf("\n");
            fflush(stdout);
        }
#endif
    }

    /* ret is set correctly by the prior call */

cleanup:
    memset(kedata, 0, keylength);
    memset(kidata, 0, keylength);
    memset(plaintext, 0, plainlen);

    free(plaintext);
    free(kidata);
    free(kedata);

    return(ret);
}

/*
n-fold(k-bits):
  l = lcm(n,k)
  r = l/k
  s = k-bits | k-bits rot 13 | k-bits rot 13*2 | ... | k-bits rot 13*(r-1)
  compute the 1's complement sum:
        n-fold = s[0..n-1]+s[n..2n-1]+s[2n..3n-1]+..+s[(k-1)*n..k*n-1]
*/

/* representation: msb first, assume n and k are multiples of 8, and
   that k>=16.  this is the case of all the cryptosystems which are
   likely to be used.  this function can be replaced if that
   assumption ever fails.  */

/* input length is in bits */

void krb5_nfold(unsigned int inbits, const unsigned char *in, unsigned int outbits, unsigned char *out)
{
    int a,b,c,lcm;
    int byte, i, msbit;

    /* the code below is more readable if I make these bytes
       instead of bits */

    inbits >>= 3;
    outbits >>= 3;

    /* first compute lcm(n,k) */

    a = outbits;
    b = inbits;

    while (b != 0)
    {
        c = b;
        b = a%b;
        a = c;
    }

    lcm = outbits*inbits/a;

    /* now do the real work */

    memset(out, 0, outbits);
    byte = 0;

    /* this will end up cycling through k lcm(k,n)/k times, which
       is correct */
    for (i=lcm-1; i>=0; i--) {
        /* compute the msbit in k which gets added into this byte */
        msbit = (/* first, start with the msbit in the first, unrotated
		    byte */
                    ((inbits<<3)-1)
                    /* then, for each byte, shift to the right for each
                       repetition */
                    +(((inbits<<3)+13)*(i/inbits))
                    /* last, pick out the correct byte within that
                       shifted repetition */
                    +((inbits-(i%inbits))<<3)
                )%(inbits<<3);

        /* pull out the byte value itself */
        byte += (((in[((inbits-1)-(msbit>>3))%inbits]<<8)|
                  (in[((inbits)-(msbit>>3))%inbits]))
                 >>((msbit&7)+1))&0xff;

        /* do the addition */
        byte += out[i%outbits];
        out[i%outbits] = byte&0xff;

#if 0
        printf("msbit[%d] = %d\tbyte = %02x\tsum = %03x\n", i, msbit,
               (((in[((inbits-1)-(msbit>>3))%inbits]<<8)|
                 (in[((inbits)-(msbit>>3))%inbits]))
                >>((msbit&7)+1))&0xff, byte);
#endif

        /* keep around the carry bit, if any */
        byte >>= 8;

#if 0
        printf("carry=%d\n", byte);
#endif
    }

    /* if there's a carry bit left over, add it back in */
    if (byte)
    {
        for (i=outbits-1; i>=0; i--)
        {
            /* do the addition */
            byte += out[i];
            out[i] = byte&0xff;

            /* keep around the carry bit, if any */
            byte >>= 8;
        }
    }
}



static void xorblock(char *out, char *in)
{
    int z;
    for (z = 0; z < BLOCK_SIZE; z++)
        out[z] ^= in[z];
}




krb5_error_code krb5int_aes_encrypt(aes_context *aes_ctx, const krb5_data *ivec, krb5_data *input, krb5_data *output)
{
    //aes_ctx ctx;
    unsigned char tmp[BLOCK_SIZE], tmp2[BLOCK_SIZE], tmp3[BLOCK_SIZE];
    int nblocks = 0, blockno;

    /*    CHECK_SIZES; */

    //if (aes_enc_key(key->contents, key->length, &ctx) != aes_good)
    //abort();

    if (ivec)
        memcpy(tmp, ivec->data, BLOCK_SIZE);
    else
        memset(tmp, 0, BLOCK_SIZE);

    nblocks = (input->length + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (nblocks == 1)
    {
        /* XXX Used for DK function.  */
        //enc(output->data, input->data, aes_ctx);
        aes_crypt_ecb_sw(aes_ctx, AES_ENCRYPT, input->data, output->data);
    }
    else
    {
        unsigned int nleft;

        for (blockno = 0; blockno < nblocks - 2; blockno++)
        {
            xorblock((char*)tmp, (char*)(input->data + blockno * BLOCK_SIZE));
            //enc(tmp2, tmp, &ctx);
            aes_crypt_ecb_sw(aes_ctx,AES_ENCRYPT,tmp,tmp2);
            memcpy(output->data + blockno * BLOCK_SIZE, tmp2, BLOCK_SIZE);

            /* Set up for next block.  */
            memcpy(tmp, tmp2, BLOCK_SIZE);
        }
        /* Do final CTS step for last two blocks (the second of which
           may or may not be incomplete).  */
        xorblock((char*)tmp, (char*)(input->data + (nblocks - 2) * BLOCK_SIZE));
        //enc(tmp2, tmp, &ctx);
        aes_crypt_ecb_sw(aes_ctx, AES_ENCRYPT, tmp, tmp2);
        nleft = input->length - (nblocks - 1) * BLOCK_SIZE;
        memcpy(output->data + (nblocks - 1) * BLOCK_SIZE, tmp2, nleft);
        memcpy(tmp, tmp2, BLOCK_SIZE);

        memset(tmp3, 0, sizeof(tmp3));
        memcpy(tmp3, input->data + (nblocks - 1) * BLOCK_SIZE, nleft);
        xorblock((char *)tmp, (char*)(tmp3));


        //enc(tmp2, tmp, aes_ctx);
        aes_crypt_ecb_sw(aes_ctx, AES_ENCRYPT, tmp, tmp2);
        memcpy(output->data + (nblocks - 2) * BLOCK_SIZE, tmp2, BLOCK_SIZE);
        if (ivec)
            memcpy(ivec->data, tmp2, BLOCK_SIZE);
    }

    return 0;
}

krb5_error_code krb5int_aes_decrypt(aes_context *aes_ctx, const krb5_data *ivec, const krb5_data *input, krb5_data *output)
{
    //aes_ctx ctx;
    unsigned char tmp[BLOCK_SIZE], tmp2[BLOCK_SIZE], tmp3[BLOCK_SIZE];
    int nblocks = 0, blockno;

    //if (aes_dec_key(key->contents, key->length, &ctx) != aes_good)
    //abort();

    if (ivec)
        memcpy(tmp, ivec->data, BLOCK_SIZE);
    else
        memset(tmp, 0, BLOCK_SIZE);

    nblocks = (input->length + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (nblocks == 1) {
        if (input->length < BLOCK_SIZE)
            abort();

        //dec(output->data, input->data, &ctx);
        aes_crypt_ecb_sw(aes_ctx, AES_DECRYPT, input->data, output->data);

    }
    else
    {
        for (blockno = 0; blockno < nblocks - 2; blockno++)
        {
            //dec(tmp2, input->data + blockno * BLOCK_SIZE, &ctx);
            aes_crypt_ecb_sw(aes_ctx, AES_DECRYPT, input->data + blockno * BLOCK_SIZE, tmp2);
            xorblock((char*)tmp2, (char*)tmp);
            memcpy(output->data + blockno * BLOCK_SIZE, tmp2, BLOCK_SIZE);
            memcpy(tmp, input->data + blockno * BLOCK_SIZE, BLOCK_SIZE);
        }
        /* Do last two blocks, the second of which (next-to-last block
           of plaintext) may be incomplete.  */
        //dec(tmp2, input->data + (nblocks - 2) * BLOCK_SIZE, &ctx);
        aes_crypt_ecb_sw(aes_ctx, AES_DECRYPT, input->data + (nblocks - 2) * BLOCK_SIZE, tmp2);
        /* Set tmp3 to last ciphertext block, padded.  */
        memset(tmp3, 0, sizeof(tmp3));
        memcpy(tmp3, input->data + (nblocks - 1) * BLOCK_SIZE,
               input->length - (nblocks - 1) * BLOCK_SIZE);
        /* Set tmp2 to last (possibly partial) plaintext block, and
           save it.  */
        xorblock((char*)tmp2, (char*)tmp3);
        memcpy(output->data + (nblocks - 1) * BLOCK_SIZE, tmp2,
               input->length - (nblocks - 1) * BLOCK_SIZE);
        /* Maybe keep the trailing part, and copy in the last
           ciphertext block.  */
        memcpy(tmp2, tmp3, input->length - (nblocks - 1) * BLOCK_SIZE);
        /* Decrypt, to get next to last plaintext block xor previous
           ciphertext.  */
        //dec(tmp3, tmp2, &ctx);
        aes_crypt_ecb_sw(aes_ctx, AES_DECRYPT, tmp2, tmp3);
        xorblock((char*)tmp3, (char*)tmp);
        memcpy(output->data + (nblocks - 2) * BLOCK_SIZE, tmp3, BLOCK_SIZE);
        if (ivec)
            memcpy(ivec->data, input->data + (nblocks - 2) * BLOCK_SIZE,
                   BLOCK_SIZE);
    }

    return 0;
}



static krb5_error_code k5_aes_make_key(const krb5_data *randombits, krb5_keyblock *key)
{
    if (key->length != 16 && key->length != 32)
        return(-1);
    if (randombits->length != key->length)
        return(-1);

    key->magic = KV5M_KEYBLOCK;

    memcpy(key->contents, randombits->data, randombits->length);
    return(0);
}

#if 0
static krb5_error_code
krb5int_aes_init_state (const krb5_keyblock *key, krb5_keyusage usage,
                        krb5_data *state)
{
    state->length = 16;
    state->data = (unsigned char *) malloc(16);
    if (state->data == NULL)
        return ENOMEM;
    memset(state->data, 0, state->length);
    return 0;
}
#endif

krb5_error_code krb5_derive_key_aes(aes_context *aes_ctx, krb5_keyblock *outkey, const krb5_data *in_constant)
{
    int blocksize, keybytes, keylength, n;
    unsigned char *inblockdata, *outblockdata, *rawkey;
    krb5_data inblock, outblock;
    //unsigned char zero_iv[8] = {0};


    blocksize = /*enc->block_size*/16;
    keybytes = /*enc->keybytes*/16;
    keylength = /*enc->keylength*/16;


    /* allocate and set up buffers */

    if ((inblockdata = (unsigned char *) malloc(blocksize)) == NULL)
        return(-1);

    if ((outblockdata = (unsigned char *) malloc(blocksize)) == NULL) {
        free(inblockdata);
        return(-1);
    }

    if ((rawkey = (unsigned char *) malloc(keybytes)) == NULL) {
        free(outblockdata);
        free(inblockdata);
        return(-1);
    }

    inblock.data = inblockdata;
    inblock.length = blocksize;

    outblock.data = outblockdata;
    outblock.length = blocksize;

    /* initialize the input block */

    if (in_constant->length == inblock.length)
    {
        memcpy(inblock.data, in_constant->data, inblock.length);
    }
    else
    {
        krb5_nfold(in_constant->length*8, in_constant->data, inblock.length*8, inblock.data);
    }

    /* loop encrypting the blocks until enough key bytes are generated */

    n = 0;
    while (n < keybytes)
    {
#if 0
        des3_crypt_cbc( ctx3,
                        DES_ENCRYPT, inblock.length,
                        zero_iv, inblock.data, outblock.data );
#endif
        krb5int_aes_encrypt(aes_ctx, 0, &inblock, &outblock);

        if ((keybytes - n) <= outblock.length) {
            memcpy(rawkey+n, outblock.data, (keybytes - n));
            break;
        }

        memcpy(rawkey+n, outblock.data, outblock.length);
        memcpy(inblock.data, outblock.data, outblock.length);
        n += outblock.length;
    }

    /* postprocess the key */

    inblock.data = rawkey;
    inblock.length = keybytes;

    k5_aes_make_key(&inblock, outkey);

    /* clean memory, free resources and exit */

    memset(inblockdata, 0, blocksize);
    memset(outblockdata, 0, blocksize);
    memset(rawkey, 0, keybytes);

    free(rawkey);
    free(outblockdata);
    free(inblockdata);

    return(0);
}

krb5_error_code krb5_dk_decrypt_maybe_trunc_hmac(const struct krb5_enc_provider *enc,const struct krb5_hash_provider *hash,
        const krb5_keyblock *key, krb5_keyusage usage,const krb5_data *ivec,
        const krb5_data *input, krb5_data *output, int hmacsize,int ivec_mode, char shift)
{
    int ret = 0;
    aes_context aes_ctx;

    int hashsize, blocksize, keybytes, keylength, enclen, plainlen;
    unsigned char *plaindata, *kedata, *kidata, *cksum, *cn;
    krb5_keyblock ke, ki;
    krb5_data d1, d2;
    unsigned char constantdata[K5CLENGTH];
    //int tmp = ticket->data;


    d1.data =  constantdata;
    d1.length = 5;

    /* allocate and set up ciphertext and to-be-derived keys */

    hashsize = hash->hashsize;
    blocksize = enc->block_size;
    keybytes = enc->keybytes;
    keylength = enc->keylength;


    enclen = input->length - hmacsize;

    if ((kedata = (unsigned char *) malloc(keylength)) == NULL)
        return(-1);
    if ((kidata = (unsigned char *) malloc(keylength)) == NULL) {
        free(kedata);
        return(-1);
    }
    if ((plaindata = (unsigned char *) malloc(enclen)) == NULL) {
        free(kidata);
        free(kedata);
        return(-1);
    }
    if ((cksum = (unsigned char *) malloc(hashsize)) == NULL) {
        free(plaindata);
        free(kidata);
        free(kedata);
        return(-1);
    }

    ke.contents = kedata;
    ke.length = keylength;
    ki.contents = kidata;
    ki.length = keylength;

    /* derive the keys */
    d1.data[0] = (usage>>24)&0xff;
    d1.data[1] = (usage>>16)&0xff;
    d1.data[2] = (usage>>8)&0xff;
    d1.data[3] = usage&0xff;

    d1.data[4] = (char) 0xAA;


    aes_setkey_enc_sw( &aes_ctx, (unsigned char *) (key->contents), 16*8 );
    if ((ret = krb5_derive_key_aes(&aes_ctx, &ke, &d1 )) != 0)
        goto cleanup;

    d1.data[4] = 0x55;

    if ((ret = krb5_derive_key_aes(&aes_ctx, &ki, &d1)) != 0)
        goto cleanup;

    /* decrypt the ciphertext */

    d1.length = enclen;
    d1.data = input->data;

    d2.length = enclen;
    d2.data = plaindata;

    aes_setkey_dec_sw( &aes_ctx, (unsigned char *) ke.contents, 16*8 );
    if ((ret=krb5int_aes_decrypt( &aes_ctx, 0, &d1, &d2))!= 0)
        goto cleanup;
    //if ((ret = ((*(enc->decrypt))(&ke, ivec, &d1, &d2))) != 0)
    //	goto cleanup;
#if 1
    if (ivec != NULL && ivec->length == blocksize)
    {
        if (ivec_mode == 0)
        {
            cn = (unsigned char *) d1.data + d1.length - blocksize;
        }
        else if (ivec_mode == 1)
        {
            int nblocks = (d1.length + blocksize - 1) / blocksize;
            cn = d1.data + blocksize * (nblocks - 2);
        }
        else
            abort();
    }
    else
#endif
        cn = NULL;

    d1.length = hashsize;
    d1.data = (char*)cksum;
    sha1_hmac(ki.contents, ki.length, d2.data, d2.length, d1.data);
    //if ((ret = krb5_hmac_sha1(hash, &ki, 1, &d2, &d1)) != 0)
    //	goto cleanup;

    if (memcmp(cksum, input->data+enclen, hmacsize) != 0) {
        ret = -1;
        goto cleanup;
    }

    /* because this encoding isn't self-describing wrt length, the
       best we can do here is to compute the length minus the
       confounder. */

    plainlen = enclen - blocksize;

    if (output->length < plainlen)
        return(KRB5_BAD_MSIZE);

    output->length = plainlen;

    //memcpy(output->data, d2.data+blocksize, output->length);

    if (shift == 1)
        memcpy(output->data+2, d2.data+blocksize, output->length);
    //memcpy(output->data+2, plaintext.data+CONFOUNDERLENGTH, (plaintext.length-CONFOUNDERLENGTH));
    else
        memcpy(output->data, d2.data+blocksize, output->length);
    //memcpy(output->data, plaintext.data+CONFOUNDERLENGTH, (plaintext.length-CONFOUNDERLENGTH));

    if (cn != NULL)
        memcpy(ivec->data, cn, blocksize);

    ret = 0;
cleanup:
    memset(kedata, 0, keylength);
    memset(kidata, 0, keylength);
    memset(plaindata, 0, enclen);
    memset(cksum, 0, hashsize);

    free(cksum);
    free(plaindata);
    free(kidata);
    free(kedata);

    return(ret);
}
krb5_error_code krb5int_aes_dk_decrypt(const struct krb5_enc_provider *enc, const struct krb5_hash_provider *hash, const krb5_keyblock *key,
                                       krb5_keyusage usage, const krb5_data *ivec, const krb5_data *input, krb5_data *output, char shift)
{
    return krb5_dk_decrypt_maybe_trunc_hmac(enc, hash, key, usage, ivec, input, output, 96 / 8, 1, shift);
}
const struct krb5_enc_provider krb5int_enc_aes128 = {
    16,
    16, 16,
    NULL,
    //krb5int_aes_encrypt,
    NULL,
    //krb5int_aes_decrypt,
    k5_aes_make_key,
    NULL,
    //krb5int_aes_init_state,
    NULL
    //krb5int_default_free_state
};
