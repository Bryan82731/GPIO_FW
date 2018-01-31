
#include "krb.h"
#include "rc4.h"
#include "md5.h"
#include "kerberos.h"

static const char *const l40 = "fortybits";

#if 0
krb5_error_code krb5_hmac_md5(const struct krb5_hash_provider *hash, const krb5_keyblock *key,
                              unsigned int icount, const krb5_data *input, krb5_data *output)
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
        return(ENOMEM);
    if ((ihash = (unsigned char *) malloc(hashsize)) == NULL) {
        free(xorkey);
        return(ENOMEM);
    }
    if ((hashin = (krb5_data *)malloc(sizeof(krb5_data)*(icount+1))) == NULL) {
        free(ihash);
        free(xorkey);
        return(ENOMEM);
    }

    /* create the inner padded key */

    memset(xorkey, 0x36, blocksize);

    for (i=0; i<key->length; i++)
        xorkey[i] ^= key->contents[i];

    /* compute the inner hash */

    for (i=0; i<icount; i++) {
        hashin[0].length = blocksize;
        hashin[0].data = (char *) xorkey;
        hashin[i+1] = input[i];
    }

    hashout.length = hashsize;
    hashout.data = (char *) ihash;

    if ((ret = ((*(hash->hash))(icount+1, hashin, &hashout))))
        goto cleanup;
    //sha1_hmac
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
#endif

void krb5_arcfour_encrypt_length(
    const struct krb5_enc_provider *enc,
    const struct krb5_hash_provider *hash,
    int inputlen, int *length)
{
    int blocksize, hashsize;

    blocksize = enc->block_size;
    hashsize = hash->hashsize;

    /* checksum + (confounder + inputlen, in even blocksize) */
    *length = hashsize + krb5_roundup(8 + inputlen, blocksize);
}

krb5_keyusage
krb5int_arcfour_translate_usage(krb5_keyusage usage)
{
    switch (usage) {
    case 1:			/* AS-REQ PA-ENC-TIMESTAMP padata timestamp,  */
        return 1;
    case 2:			/* ticket from kdc */
        return 2;
    case 3:			/* as-rep encrypted part */
        return 8;
    case 4:			/* tgs-req authz data */
        return 4;
    case 5:			/* tgs-req authz data in subkey */
        return 5;
    case 6:			/* tgs-req authenticator cksum */
        return 6;
    case 7:				/* tgs-req authenticator */
        return 7;
    case 8:
        return 8;
    case 9:			/* tgs-rep encrypted with subkey */
        return 8;
    case 10:			/* ap-rep authentication cksum */
        return 10;			/* xxx  Microsoft never uses this*/
    case 11:			/* app-req authenticator */
        return 11;
    case 12:			/* app-rep encrypted part */
        return 12;
    case 23: /* sign wrap token*/
        return 13;
    default:
        return usage;
    }
}
#if 0
krb5_error_code
krb5_c_random_make_octets(krb5_data *data)
{
    data->data[0] = 0x84;
    data->data[1] = 0x81;
    data->data[2] = 0xca;
    data->data[3] = 0x4d;
    data->length = 4;
    return 0;
}
#endif
krb5_error_code krb5_arcfour_encrypt(
    const struct krb5_enc_provider *enc,
    const struct krb5_hash_provider * hash,
    const krb5_keyblock *key, krb5_keyusage usage,const krb5_data *ivec,
    const krb5_data *input,
    krb5_data *output, char shift)
{
    krb5_keyblock k1, k2, k3;
    krb5_data d1, d2, d3, salt, plaintext, checksum, ciphertext, confounder;
    krb5_keyusage ms_usage;
    int keylength, keybytes, blocksize, hashsize;
    krb5_error_code ret;
    //unsigned char ivec[8]={0};

    blocksize = enc->block_size;
    keybytes = enc->keybytes;
    keylength = enc->keylength;
    hashsize = hash->hashsize;

    d1.length=keybytes;
    d1.data=malloc(d1.length);
    if (d1.data == NULL)
        return (-1);
    memcpy(&k1, key, sizeof (krb5_keyblock));
    k1.length=d1.length;
    k1.contents= (void *) d1.data;

    d2.length=keybytes;
    d2.data=malloc(d2.length);
    if (d2.data == NULL)
    {
        free(d1.data);
        return (-1);
    }
    memcpy(&k2, key, sizeof (krb5_keyblock));
    k2.length=d2.length;
    k2.contents=(void *) d2.data;

    d3.length=keybytes;
    d3.data=malloc(d3.length);
    if (d3.data == NULL)
    {
        free(d1.data);
        free(d2.data);
        return (-1);
    }
    memcpy(&k3, key, sizeof (krb5_keyblock));
    k3.length=d3.length;
    k3.contents= (void *) d3.data;

    salt.length=14;
    salt.data=malloc(salt.length);
    if (salt.data == NULL)
    {
        free(d1.data);
        free(d2.data);
        free(d3.data);
        return (-1);
    }

    /* is "input" already blocksize aligned?  if it is, then we need this
       step, otherwise we do not */
    plaintext.length=krb5_roundup(input->length+CONFOUNDERLENGTH,blocksize);
    plaintext.data=(unsigned char*)malloc(plaintext.length);
    if (plaintext.data == NULL)
    {
        free(d1.data);
        free(d2.data);
        free(d3.data);
        free(salt.data);
        return(-1);
    }

    /* setup convienient pointers into the allocated data */
    checksum.length=hashsize;
    checksum.data=output->data;
    ciphertext.length=krb5_roundup(input->length+CONFOUNDERLENGTH,blocksize);
    ciphertext.data=output->data+hashsize;
    confounder.length=CONFOUNDERLENGTH;
    confounder.data=plaintext.data;
    output->length = plaintext.length+hashsize;

    /* begin the encryption, computer K1 */
    ms_usage=krb5int_arcfour_translate_usage(usage);
    if (key->enctype == ENCTYPE_ARCFOUR_HMAC_EXP)
    {
        strncpy((char*)salt.data, l40, salt.length);
        salt.data[10]=ms_usage & 0xff;
        salt.data[11]=(ms_usage >> 8) & 0xff;
        salt.data[12]=(ms_usage >> 16) & 0xff;
        salt.data[13]=(ms_usage >> 24) & 0xff;
    }
    else
    {
        salt.length=4;
        salt.data[0]=ms_usage & 0xff;
        salt.data[1]=(ms_usage >> 8) & 0xff;
        salt.data[2]=(ms_usage >> 16) & 0xff;
        salt.data[3]=(ms_usage >> 24) & 0xff;
    }
    //krb5_hmac_md5(hash, key, 1, &salt, &d1);
    md5_hmac(key->contents, key->length, salt.data, salt.length, d1.data);
    memcpy(k2.contents, k1.contents, k2.length);

    if (key->enctype==ENCTYPE_ARCFOUR_HMAC_EXP)
        memset(k1.contents+7, 0xab, 9);

    ret=krb5_c_random_make_octets(0, &confounder);
    memcpy(plaintext.data+confounder.length, input->data, input->length);
    if (ret)
        goto cleanup;

    //krb5_hmac_md5(hash, &k2, 1, &plaintext, &checksum);
    md5_hmac(k2.contents, k2.length, plaintext.data, plaintext.length, checksum.data);

    //krb5_hmac_md5(hash, &k1, 1, &checksum, &d3);
    md5_hmac(k1.contents, k1.length, checksum.data, checksum.length, d3.data);

    ret=(*(enc->encrypt))(&k3, ivec, &plaintext, &ciphertext);

cleanup:
    memset(d1.data, 0, d1.length);
    memset(d2.data, 0, d2.length);
    memset(d3.data, 0, d3.length);
    memset(salt.data, 0, salt.length);
    memset(plaintext.data, 0, plaintext.length);

    free(d1.data);
    free(d2.data);
    free(d3.data);
    free(salt.data);
    free(plaintext.data);
    return (ret);
}

/* This is the arcfour-hmac decryption routine */
//krb5_error_code
//krb5_arcfour_decrypt(const struct krb5_enc_provider *enc,
//					 const struct krb5_hash_provider *hash,
//			const krb5_keyblock *key, krb5_keyusage usage, const krb5_data *input,
//			krb5_data *output)
krb5_error_code krb5_arcfour_decrypt(
    const struct krb5_enc_provider *enc,
    const struct krb5_hash_provider *hash,
    const krb5_keyblock *key,
    krb5_keyusage usage,
    const krb5_data *ivec,
    const krb5_data *input,
    krb5_data *output,
    char shift)
{
    krb5_keyblock k1,k2,k3;
    krb5_data d1,d2,d3,salt,ciphertext,plaintext,checksum;
    krb5_keyusage ms_usage;
    int keybytes, keylength, hashsize, blocksize;
    krb5_error_code ret;

    blocksize = 1/*enc->block_size*/;
    keybytes = 16/*enc->keybytes*/;
    keylength = 16/*enc->keylength*/;
    hashsize = hash->hashsize;

    d1.length=keybytes;
    d1.data=(unsigned char*)malloc(d1.length);
    if (d1.data == NULL)
        return (-1);
    memcpy(&k1, key, sizeof (krb5_keyblock));
    k1.length=d1.length;
    k1.contents= (void *) d1.data;

    d2.length=keybytes;
    d2.data=(unsigned char*)malloc(d2.length);
    if (d2.data == NULL)
    {
        free(d1.data);
        return (-1);
    }
    memcpy(&k2, key, sizeof(krb5_keyblock));
    k2.length=d2.length;
    k2.contents= (void *) d2.data;

    d3.length=keybytes;
    d3.data=(unsigned char*)malloc(d3.length);
    if  (d3.data == NULL)
    {
        free(d1.data);
        free(d2.data);
        return (-1);
    }
    memcpy(&k3, key, sizeof(krb5_keyblock));
    k3.length=d3.length;
    k3.contents= (void *) d3.data;

    salt.length=14;
    salt.data=(unsigned char*)malloc(salt.length);
    if (salt.data==NULL)
    {
        free(d1.data);
        free(d2.data);
        free(d3.data);
        return (-1);
    }

    ciphertext.length=input->length-hashsize;
    ciphertext.data=input->data+hashsize;
    plaintext.length=ciphertext.length;
    plaintext.data=(unsigned char*)malloc(plaintext.length);
    if (plaintext.data == NULL)
    {
        free(d1.data);
        free(d2.data);
        free(d3.data);
        free(salt.data);
        return (-1);
    }

    checksum.length=hashsize;
    checksum.data=input->data;

    /* compute the salt */
    ms_usage=krb5int_arcfour_translate_usage(usage);
    if (key->enctype == ENCTYPE_ARCFOUR_HMAC_EXP)
    {
        strncpy((char*)salt.data, l40, salt.length);
        salt.data[10]=ms_usage & 0xff;
        salt.data[11]=(ms_usage>>8) & 0xff;
        salt.data[12]=(ms_usage>>16) & 0xff;
        salt.data[13]=(ms_usage>>24) & 0xff;
    }
    else
    {
        salt.length=4;
        salt.data[0]=ms_usage & 0xff;
        salt.data[1]=(ms_usage>>8) & 0xff;
        salt.data[2]=(ms_usage>>16) & 0xff;
        salt.data[3]=(ms_usage>>24) & 0xff;
    }
    //ret=krb5_hmac_md5(hash, key, 1, &salt, &d1);

    md5_hmac(key->contents, key->length, salt.data, salt.length, d1.data);
    //if (ret)
    //  goto cleanup;

    memcpy(k2.contents, k1.contents, k2.length);

    if (key->enctype == ENCTYPE_ARCFOUR_HMAC_EXP)
        memset(k1.contents+7, 0xab, 9);

    //ret = krb5_hmac_md5(hash, &k1, 1, &checksum, &d3);
    md5_hmac(k1.contents, k1.length, checksum.data, checksum.length, d3.data);
    //if (ret)
    //goto cleanup;


    ret=k5_arcfour_docrypt(&k3, ivec, &ciphertext, &plaintext);
    if (ret)
        goto cleanup;

    //ret=krb5_hmac_md5(hash, &k2, 1, &plaintext, &d1);
    md5_hmac(k2.contents, k2.length, plaintext.data, plaintext.length, d1.data);
    //if (ret)
    //goto cleanup;

    if (memcmp(checksum.data, d1.data, hashsize) != 0)
    {
        ret=KRB5KRB_AP_ERR_BAD_INTEGRITY;
        goto cleanup;
    }

    if (shift == 1)
        memcpy(output->data+2, plaintext.data+CONFOUNDERLENGTH, (plaintext.length-CONFOUNDERLENGTH));
    else
        memcpy(output->data, plaintext.data+CONFOUNDERLENGTH, (plaintext.length-CONFOUNDERLENGTH));

    output->length=plaintext.length-CONFOUNDERLENGTH;

cleanup:
    memset(d1.data, 0, d1.length);
    memset(d2.data, 0, d2.length);
    memset(d3.data, 0, d2.length);
    memset(salt.data, 0, salt.length);
    memset(plaintext.data, 0, plaintext.length);

    free(d1.data);
    free(d2.data);
    free(d3.data);
    free(salt.data);
    free(plaintext.data);
    return (ret);
}



/* gets the next byte from the PRNG */
#if ((__GNUC__ >= 2) )
static __inline__ unsigned int k5_arcfour_byte(ArcfourContext *);
#else
static unsigned int k5_arcfour_byte(ArcfourContext *);
#endif /* gcc inlines*/

/* Initializes the context and sets the key. */
static krb5_error_code k5_arcfour_init(ArcfourContext *ctx, const unsigned char *key,
                                       unsigned int keylen);

/* Encrypts/decrypts data. */
static void k5_arcfour_crypt(ArcfourContext *ctx, unsigned char *dest,
                             const unsigned char *src, unsigned int len);

/* Interface layer to kerb5 crypto layer */
//static krb5_error_code
//k5_arcfour_docrypt(const krb5_keyblock *, const krb5_data *,
//		   const krb5_data *, krb5_data *);

/* from a random bitstrem, construct a key */
//static krb5_error_code
//k5_arcfour_make_key(const krb5_data *, krb5_keyblock *);

static const unsigned char arcfour_weakkey1[] = {0x00, 0x00, 0xfd};
static const unsigned char arcfour_weakkey2[] = {0x03, 0xfd, 0xfc};
static const krb5_data arcfour_weakkeys[] = {
    {KV5M_DATA, sizeof (arcfour_weakkey1),
        (char * ) arcfour_weakkey1},
    {KV5M_DATA, sizeof (arcfour_weakkey2),
     (char * ) arcfour_weakkey2},
    {KV5M_DATA, 0, 0}
};

/*xxx we really should check for c9x here and use inline on
 * more than just gcc. */
#if ((__GNUC__ >= 2) )
static __inline__ unsigned int k5_arcfour_byte(ArcfourContext * ctx)
#else
static unsigned int k5_arcfour_byte(ArcfourContext * ctx)
#endif /* gcc inlines*/
{
    unsigned int x;
    unsigned int y;
    unsigned int sx, sy;
    unsigned char *state;

    state = ctx->state;
    x = (ctx->x + 1) & 0xff;
    sx = state[x];
    y = (sx + ctx->y) & 0xff;
    sy = state[y];
    ctx->x = x;
    ctx->y = y;
    state[y] = sx;
    state[x] = sy;
    return state[(sx + sy) & 0xff];
}

static void k5_arcfour_crypt(ArcfourContext *ctx, unsigned char *dest,
                             const unsigned char *src, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
        dest[i] = src[i] ^ k5_arcfour_byte(ctx);
}


static krb5_error_code
k5_arcfour_init(ArcfourContext *ctx, const unsigned char *key,
                unsigned int key_len)
{
    unsigned int t, u;
    unsigned int keyindex;
    unsigned int stateindex;
    unsigned char* state;
    unsigned int counter;

    if (key_len != 16)
        return KRB5_BAD_MSIZE;     /*this is probably not the correct error code
				 to return */
    for (counter=0;arcfour_weakkeys[counter].length >0; counter++)
        if (memcmp(key, arcfour_weakkeys[counter].data,
                   arcfour_weakkeys[counter].length) == 0)
            return KRB5DES_WEAK_KEY; /* most certainly not the correct error */

    state = &ctx->state[0];
    ctx->x = 0;
    ctx->y = 0;
    for (counter = 0; counter < 256; counter++)
        state[counter] = counter;
    keyindex = 0;
    stateindex = 0;
    for (counter = 0; counter < 256; counter++)
    {
        t = state[counter];
        stateindex = (stateindex + key[keyindex] + t) & 0xff;
        u = state[stateindex];
        state[stateindex] = t;
        state[counter] = u;
        if (++keyindex >= key_len)
            keyindex = 0;
    }
    return 0;
}


/* The workhorse of the arcfour system, this impliments the cipher */
krb5_error_code k5_arcfour_docrypt(
    const krb5_keyblock *key, const krb5_data *state,
    const krb5_data *input, krb5_data *output)
{
    ArcfourContext *arcfour_ctx;
    ArcFourCipherState *cipher_state;
    int ret;

    if (key->length != 16)
        return(KRB5_BAD_KEYSIZE);
    if (state && (state->length != sizeof (ArcFourCipherState)))
        return(KRB5_BAD_MSIZE);
    if (input->length != output->length)
        return(KRB5_BAD_MSIZE);

    if (state)
    {
        cipher_state = (ArcFourCipherState *) state->data;
        arcfour_ctx=&cipher_state->ctx;
        if (cipher_state->initialized == 0)
        {
            if ((ret=k5_arcfour_init(arcfour_ctx, key->contents, key->length)))
            {
                return ret;
            }
            cipher_state->initialized = 1;
        }
        k5_arcfour_crypt(arcfour_ctx, (unsigned char *) output->data, (const unsigned char *) input->data, input->length);
    }
    else
    {
        arcfour_ctx=malloc(sizeof (ArcfourContext));
        if (arcfour_ctx == NULL)
            return -1;
        if ((ret=k5_arcfour_init(arcfour_ctx, key->contents, key->length)))
        {
            free(arcfour_ctx);
            return (ret);
        }
        k5_arcfour_crypt(arcfour_ctx, (unsigned char * ) output->data,
                         (const unsigned char * ) input->data, input->length);
        memset(arcfour_ctx, 0, sizeof (ArcfourContext));
        free(arcfour_ctx);
    }

    return 0;
}
#if 0
static krb5_error_code k5_arcfour_make_key(const krb5_data *randombits, krb5_keyblock *key)
{
    if (key->length != 16)
        return(KRB5_BAD_KEYSIZE);
    if (randombits->length != 16)
        return(KRB5_CRYPTO_INTERNAL);

    key->magic = KV5M_KEYBLOCK;
    key->length = 16;

    memcpy(key->contents, randombits->data, randombits->length);

    return(0);
}
#endif

static void asctouni(unsigned char *unicode, unsigned char *ascii, int len)
{
    int counter;
    for (counter=0;counter<len;counter++)
    {
        unicode[2*counter]=ascii[counter];
        unicode[2*counter + 1]=0x00;
    }
}

#if 0
krb5_error_code krb5int_arcfour_string_to_key(
    const struct krb5_enc_provider *enc,
    const krb5_data *string, const krb5_data *salt,
    const krb5_data *params, krb5_keyblock *key)
{
    int len,slen;
    unsigned char *copystr;
    krb5_MD4_CTX md4_context;

    if (params != NULL)
        return KRB5_ERR_BAD_S2K_PARAMS;

    if (key->length != 16)
        return (KRB5_BAD_MSIZE);

    /* We ignore salt per the Microsoft spec*/

    /* compute the space needed for the new string.
       Since the password must be stored in unicode, we need to increase
       that number by 2x.

       This should be re-evauated in the future, it makes the assumption that
       thes user's password is in ascii.
    */
    slen = ((string->length)>128)?128:string->length;
    len=(slen)*2;

    copystr = malloc(len);
    if (copystr == NULL)
        return ENOMEM;

    /* make the string.  start by creating the unicode version of the password*/
    asctouni(copystr, string->data, slen );

    /* the actual MD4 hash of the data */
    krb5_MD4Init(&md4_context);
    krb5_MD4Update(&md4_context, (unsigned char *)copystr, len);
    krb5_MD4Final(&md4_context);
    memcpy(key->contents, md4_context.digest, 16);

#if 0
    /* test the string_to_key function */
    printf("Hash=");
    {
        int counter;
        for (counter=0;counter<16;counter++)
            printf("%02x", md4_context.digest[counter]);
        printf("\n");
    }
#endif /* 0 */

    /* Zero out the data behind us */
    memset (copystr, 0, len);
    memset(&md4_context, 0, sizeof(md4_context));
    free(copystr);
    return 0;
}



static krb5_error_code k5_arcfour_init_state (const krb5_keyblock *key, krb5_keyusage keyusage, krb5_data *new_state)
{
    /* Note that we can't actually set up the state here  because the key
     * will change  between now and when encrypt is called
     * because  it is data dependent.  Yeah, this has strange
     * properties. --SDH
     */
    new_state->length = sizeof (ArcFourCipherState);
    new_state->data = malloc (new_state->length);
    if (new_state->data) {
        memset (new_state->data, 0 , new_state->length);
        /* That will set initialized to zero*/
    }else {
        return (ENOMEM);
    }
    return 0;
}
#endif

krb5_error_code krb5int_default_free_state (krb5_data *state)
{
    if (state->data) {
        free (state->data);
        state-> data = NULL;
        state->length = 0;
    }
    return 0;
}


/* Since the arcfour cipher is identical going forwards and backwards,
   we just call "docrypt" directly
*/
const struct krb5_enc_provider krb5int_enc_arcfour = {
    /* This seems to work... although I am not sure what the
       implications are in other places in the kerberos library */
    1,
    /* Keysize is arbitrary in arcfour, but the constraints of the
       system, and to attempt to work with the MSFT system forces us
       to 16byte/128bit.  Since there is no parity in the key, the
       byte and length are the same.  */
    16, 16,
    k5_arcfour_docrypt,
    k5_arcfour_docrypt,
    //k5_arcfour_make_key,
    NULL,
    NULL,
    //krb5int_default_free_state
    NULL
};
