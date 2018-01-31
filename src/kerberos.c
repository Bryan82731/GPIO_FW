#include  <time.h>
#include "b64.h"

#include "aes.h"
#include "rc4.h"
#include "gssapi.h"
#include "spnego.h"
#include "kerberos.h"
#include "krb_aes.h"
#include "pac.h"
#include "x509.h"

//#define TESTAPREP

#define in_clock_skew(date) (labs((date)-currenttime) < context->clockskew)

#define krb5_is_ap_req(dat) ((dat) && (dat)->length && ((dat)->data[0] == 0x6e || (dat)->data[0] == 0x4e))

extern DPCONF *dpconf;
ROM_EXTERN UserInfo *userhead;
ROM_EXTERN UserInfo *usertail;
extern const struct krb5_enc_provider krb5int_enc_aes128;
extern void *dbgmalloc(int size, char *fn, int line);
extern void dbgfree(void *ptr, char *fn, int line);

static krb5_error_code krb5_copy_keyblock(krb5_context context, const krb5_keyblock *from, krb5_keyblock **to);

static void krb5_free_enc_tkt_part(krb5_context context, krb5_enc_tkt_part *val);

static krb5_error_code krb5_gss_init_context (krb5_context *ctxp);

gss_mech_info g_mechList = 0;
gss_mech_info g_mechListTail = 0;

gss_OID GSS_C_NT_USER_NAME           = 0;
gss_OID gss_nt_user_name             = oids+0;

gss_OID GSS_C_NT_MACHINE_UID_NAME    = oids+1;
gss_OID gss_nt_machine_uid_name      = oids+1;

gss_OID GSS_C_NT_STRING_UID_NAME     = oids+2;
gss_OID gss_nt_string_uid_name       = oids+2;

gss_OID GSS_C_NT_HOSTBASED_SERVICE_X = oids+3;
gss_OID gss_nt_service_name_v2       = oids+3;

gss_OID GSS_C_NT_HOSTBASED_SERVICE   = oids+4;
gss_OID gss_nt_service_name          = oids+4;

gss_OID GSS_C_NT_ANONYMOUS           = oids+5;

gss_OID GSS_C_NT_EXPORT_NAME         = oids+6;
gss_OID gss_nt_exported_name         = oids+6;

const unsigned char ap_req_pvno_msgtyp[]=
{
    0xa0,0x03,0x02,0x01,0x05,
    0xa1,0x03,0x02,0x01,0x0f,
};


/* these will be linear searched.  if they ever get big, a binary
   search or hash table would be better, which means these would need
   to be sorted.  An array would be more efficient, but that assumes
   that the keytypes are all near each other.  I'd rather not make
   that assumption. */

const struct krb5_keytypes krb5_enctypes_list[] =
{
    {
        ENCTYPE_ARCFOUR_HMAC,
        "arcfour-hmac","ArcFour with HMAC/md5",
        &krb5int_enc_arcfour,
        &krb5int_hash_md5,
        0,
        krb5_arcfour_encrypt_length,
        krb5_arcfour_encrypt,
        krb5_arcfour_decrypt,
        NULL,//krb5int_arcfour_string_to_key,
        NULL, /*PRF*/
        CKSUMTYPE_HMAC_MD5_ARCFOUR, 1
    },

    {
        ENCTYPE_AES128_CTS_HMAC_SHA1_96,
        "aes128-cts-hmac-sha1-96", "AES-128 CTS mode with 96-bit SHA-1 HMAC",
        &krb5int_enc_aes128,
        &krb5int_hash_sha1,
        16,
        krb5int_aes_encrypt_length,
        krb5int_aes_dk_encrypt,
        krb5int_aes_dk_decrypt,
        //NULL,
        //krb5int_aes_string_to_key,
        NULL,
        //krb5int_dk_prf,
        NULL,
        CKSUMTYPE_HMAC_SHA1_96_AES128, 1
    },

};

const int krb5_enctypes_length =sizeof(krb5_enctypes_list)/sizeof(struct krb5_keytypes);


const struct krb5_cksumtypes krb5_cksumtypes_list[] =
{
    {
        CKSUMTYPE_RSA_MD5, 0,
        "md5", "RSA-MD5",
        0, NULL,
        &krb5int_hash_md5
    },

    {   CKSUMTYPE_HMAC_MD5_ARCFOUR, 0,
        "hmac-md5-rc4", "Microsoft HMAC MD5 (RC4 key)",
        ENCTYPE_ARCFOUR_HMAC, &krb5int_keyhash_hmac_md5,
        NULL
    },
    { CKSUMTYPE_HMAC_SHA1_96_AES128, KRB5_CKSUMFLAG_DERIVE,
      "hmac-sha1-96-aes128", "HMAC-SHA1 AES128 key",
      0, NULL,
      &krb5int_hash_sha1, 12
    },
};

const int krb5_cksumtypes_length = sizeof(krb5_cksumtypes_list)/sizeof(struct krb5_cksumtypes);

const gss_OID_desc krb5_gss_oid_array[] = 
{
    /* this is the official, rfc-specified OID */
    {GSS_MECH_KRB5_OID_LENGTH, GSS_MECH_KRB5_OID},
    /* this pre-RFC mech OID */
    {GSS_MECH_KRB5_OLD_OID_LENGTH, GSS_MECH_KRB5_OLD_OID},
    /* this is the unofficial, incorrect mech OID emitted by MS */
    {GSS_MECH_KRB5_WRONG_OID_LENGTH, GSS_MECH_KRB5_WRONG_OID},
    /* this is the v2 assigned OID */
    {9, "\052\206\110\206\367\022\001\002\003"},
    /* these two are name type OID's */

    /* 2.1.1. Kerberos Principal Name Form:  (rfc 1964)
     * This name form shall be represented by the Object Identifier {iso(1)
     * member-body(2) United States(840) mit(113554) infosys(1) gssapi(2)
     * krb5(2) krb5_name(1)}.  The recommended symbolic name for this type
     * is "GSS_KRB5_NT_PRINCIPAL_NAME". */
    {10, "\052\206\110\206\367\022\001\002\002\001"},

    /* gss_nt_krb5_principal.  Object identifier for a krb5_principal. Do not use. */
    {10, "\052\206\110\206\367\022\001\002\002\002"},
    { 0, 0 }
};

const gss_OID_desc * const gss_mech_krb5              = krb5_gss_oid_array+0;
const gss_OID_desc * const gss_mech_krb5_old          = krb5_gss_oid_array+1;
const gss_OID_desc * const gss_mech_krb5_wrong        = krb5_gss_oid_array+2;
const gss_OID_desc * const gss_nt_krb5_name           = krb5_gss_oid_array+4;
const gss_OID_desc * const gss_nt_krb5_principal      = krb5_gss_oid_array+5;
const gss_OID_desc * const GSS_KRB5_NT_PRINCIPAL_NAME = krb5_gss_oid_array+4;

static const gss_OID_set_desc oidsets[] =
{
    {1, (gss_OID) krb5_gss_oid_array+0},
    {1, (gss_OID) krb5_gss_oid_array+1},
    {3, (gss_OID) krb5_gss_oid_array+0},
    {1, (gss_OID) krb5_gss_oid_array+2},
    {3, (gss_OID) krb5_gss_oid_array+0},
};
const gss_OID_set_desc * const gss_mech_set_krb5 = oidsets+0;
const gss_OID_set_desc * const gss_mech_set_krb5_old = oidsets+1;
const gss_OID_set_desc * const gss_mech_set_krb5_both = oidsets+2;

OM_uint32 gssint_get_mech_type( gss_OID OID, gss_buffer_t	token);

krb5_error_code krb5_timeofday(krb5_context context, register krb5_timestamp *timeret);

krb5_error_code krb5_c_random_make_octets(krb5_context context, krb5_data *data)
{
    unsigned int i = 0;
    for (;i<data->length;i++)
    {
        data->data[i] = rand();
    }
    return 0;
}

krb5_error_code krb5_derive_key(const struct krb5_enc_provider *enc, const krb5_keyblock *inkey,
	krb5_keyblock *outkey, const krb5_data *in_constant)
{
    unsigned int blocksize, keybytes, keylength, n;
    unsigned char *inblockdata, *outblockdata, *rawkey;
    krb5_data inblock, outblock;

    blocksize = enc->block_size;
    keybytes = enc->keybytes;
    keylength = enc->keylength;

    if ((inkey->length != keylength) || (outkey->length != keylength))
        return(KRB5_CRYPTO_INTERNAL);

    /* allocate and set up buffers */

    if ((inblockdata = (unsigned char *) malloc(blocksize)) == NULL)
        return(-1);

    if ((outblockdata = (unsigned char *) malloc(blocksize)) == NULL)
    {
        free(inblockdata);
        return(-1);
    }

    if ((rawkey = (unsigned char *) malloc(keybytes)) == NULL)
    {
        free(outblockdata);
        free(inblockdata);
        return(-1);
    }

    inblock.data = inblockdata;
    inblock.length = blocksize;

    outblock.data = outblockdata;
    outblock.length = blocksize;

    /* initialize the input block */

    if (in_constant->length == inblock.length) {
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
        (*(enc->encrypt))(inkey, 0, &inblock, &outblock);

        if ((keybytes - n) <= outblock.length)
        {
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

    (*(enc->make_key))(&inblock, outkey);

    /* clean memory, free resources and exit */

    memset(inblockdata, 0, blocksize);
    memset(outblockdata, 0, blocksize);
    memset(rawkey, 0, keybytes);

    free(rawkey);
    free(outblockdata);
    free(inblockdata);

    return(0);
}

OM_uint32 krb5_gss_delete_sec_context( OM_uint32 *minor_status, gss_ctx_id_t *context_handle, 
	gss_buffer_t output_token)
{
    krb5_context context = NULL;
    krb5_gss_ctx_id_rec *ctx;

    if (output_token)
    {
        output_token->length = 0;
        output_token->value = NULL;
    }

    /*SUPPRESS 29*/
    if (*context_handle == GSS_C_NO_CONTEXT)
    {
        *minor_status = 0;
        return(GSS_S_COMPLETE);
    }

    ctx = (krb5_gss_ctx_id_t) (*context_handle)->internal_ctx_id;
    if (ctx && ctx->k5_context)
        context = ctx->k5_context;

    if (ctx->auth_context)
    {
        if (ctx->cred_rcache)
            (void)krb5_auth_con_setrcache(context, ctx->auth_context, NULL);

        krb5_auth_con_free(context, ctx->auth_context);
    }

    if (ctx->ticket)
        krb5_free_ticket(context, ctx->ticket);

    if (ctx->mech_used)
        gss_release_oid(minor_status, &ctx->mech_used);

    if (ctx->k5_context)
        krb5_free_context(ctx->k5_context);

    /* Zero out context */
    memset(ctx, 0, sizeof(*ctx));
    free(ctx);

    /* zero the handle itself */
    free((*context_handle)->mech_type);
    free(*context_handle);

    *minor_status = 0;
    return(GSS_S_COMPLETE);
}

unsigned int krb5_gss_acquire_cred(OM_uint32 *minor_status,gss_name_t desired_name, OM_uint32 time_req, 
	gss_OID_set desired_mechs, gss_cred_usage_t cred_usage,gss_cred_id_t *cred, gss_OID_set *actual_mechs,
	OM_uint32 *time_rec)
{
    krb5_gss_cred_id_t cred_for_output;
    int req_old = 0, req_new = 0;
    //krb5_error_code code;

    *cred = NULL;
    //cred = (krb5_gss_cred_id_t) malloc(sizeof(krb5_gss_cred_id_rec));
    //memset(cred, 0, sizeof(krb5_gss_cred_id_rec));

    if (desired_mechs == GSS_C_NULL_OID_SET)
    {
        req_old = 1;
        req_new = 1;
    }
    if ((cred_for_output = (krb5_gss_cred_id_t) malloc(sizeof(krb5_gss_cred_id_rec))) == NULL)
    {
        *minor_status = -1;
        return(GSS_S_FAILURE);
    }
    memset(cred_for_output, 0, sizeof(krb5_gss_cred_id_rec));

    cred_for_output->usage = cred_usage;
    cred_for_output->princ = NULL;
    cred_for_output->prerfc_mech = req_old;
    cred_for_output->rfc_mech = req_new;

    cred_for_output->keytab = NULL;
    cred_for_output->ccache = NULL;

    /* if requested, acquire credentials for accepting */
    /* this will fill in cred->princ if the desired_name is not specified */

    if ((cred_usage == GSS_C_ACCEPT) || (cred_usage == GSS_C_BOTH))
        cred_for_output->prerfc_mech = 0;
    //if ((ret = acquire_accept_cred(context, minor_status, desired_name,
    //	&(cred->princ), cred))!= GSS_S_COMPLETE)
    //{
    //	if (cred->princ)
    //		krb5_free_principal(context, cred->princ);
    //	//k5_mutex_destroy(&cred->lock);
    //	//xfree(cred);
    //	/* minor_status set by acquire_accept_cred() */
    //	//krb5_free_context(context);
    //	return(ret);
    //}

    //if (!cred_for_output->princ && (desired_name != GSS_C_NO_CREDENTIAL))
    //{
    //	if ((code = krb5_copy_principal(0, (krb5_principal) desired_name,  &(cred_for_output->princ))))
    //	{
    //		*minor_status = code;
    //		return(GSS_S_FAILURE);
    //	}
    //}

    if (!cred_for_output->princ && (desired_name != (gss_name_t)GSS_C_NO_CREDENTIAL))
    {
        //cred_for_output->princ = (krb5_principal)malloc(sizeof(krb5_principal_data));
        cred_for_output->princ = (krb5_principal)desired_name;
        //cred_for_output->princ->length = ((krb5_principal) desired_name)->length;
        //cred_for_output->princ->magic = ((krb5_principal) desired_name)->magic;
        //cred_for_output->princ->realm.data = ((krb5_principal) desired_name)->realm.data;
        //cred_for_output->princ->realm.length = ((krb5_principal) desired_name)->realm.length;
        //cred_for_output->princ->data = (krb5_data *)malloc(sizeof(krb5_data));
        //cred_for_output->princ->data->data = ((krb5_principal) desired_name)->data->data;
        //cred_for_output->princ->data->length = ((krb5_principal) desired_name)->data->length;
        //cred_for_output->princ->type = ((krb5_principal) desired_name)->type;
    }
    if (cred_usage == GSS_C_ACCEPT)
    {
        if (time_rec)
            *time_rec = GSS_C_INDEFINITE;
    }
    *minor_status = 0;
    *cred = (gss_cred_id_t) cred_for_output;

    return(GSS_S_COMPLETE);
}

void stoupper(char *ptr)
{
    char *tmp = ptr;
    while (*tmp != '\0')
    {
        *tmp = toupper(*tmp);
        tmp++;
    }
}

void stolower(char *ptr)
{
    char *tmp = ptr;
    while (*tmp != '\0')
    {
        *tmp = tolower(*tmp);
        tmp++;
    }
}

krb5_error_code krb5_get_host_realm(char *host, char **realmsp)
{
    char *realm, *cp, *temp_realm;
    cp = host;
    realm = (char *)NULL;
    temp_realm = 0;
    while (cp)
    {
        temp_realm = (char*)malloc(strlen((char*)dpconf->DomainName )+1);
        stoupper((char *)dpconf->DomainName);
        strcpy(temp_realm, (const char *)dpconf->DomainName );
        stolower((char *)dpconf->DomainName);
        if (temp_realm != (char *)NULL)
            break;	/* Match found */
    }
    *realmsp = temp_realm;
    return 0;
}

krb5_error_code krb5_build_principal(krb5_principal * princ, unsigned int rlen, const char * realm, char* sname, 
	char *host)
{
    //krb5_error_code retval;
    krb5_principal pr_ret = (krb5_principal)malloc(sizeof(krb5_principal_data));
    register int /*i,*/ count = 0;
    //register char *next;
    //char *tmpdata;
    krb5_data *data;
    if (!pr_ret)
        return -1;
    count = 2;
    data = (krb5_data *) malloc(sizeof(krb5_data) * count);
    krb5_princ_set_realm_length(NULL, pr_ret, rlen);
    krb5_princ_set_realm_data(NULL, pr_ret, (unsigned char*)realm);
    data[0].length = strlen(sname);
    data[0].data = (INT8U *)sname;
    data[1].length = strlen(host);
    data[1].data = (INT8U *)host;

    pr_ret->data = data;
    pr_ret->length = 2;
    pr_ret->type = KRB5_NT_UNKNOWN;
    pr_ret->magic = KV5M_PRINCIPAL;
    *princ = pr_ret;
    return 0;
}

krb5_error_code rtk_krb5_sname_to_principal(char *hostname, char *sname,krb5_int32 type, 
	krb5_principal *ret_princ)
{
    char *hrealms=0, *realm, *remote_host;
    krb5_error_code retval;
    register char *cp;
    char localname[64];

    if ((type == KRB5_NT_UNKNOWN) || (type == KRB5_NT_SRV_HST))
    {
        /* if hostname is NULL, use local hostname */
        if (! hostname)
        {
            strcpy(localname, (const char *)dpconf->HostName);
            strcpy(localname, ".");
            strcpy(localname, (const char *)dpconf->DomainName);
            hostname = localname;
        }
        /* if sname is NULL, use "host" */
        if (! sname)
            sname = "host";

        /* copy the hostname into non-volatile storage */

        remote_host = hostname;

        if (type == KRB5_NT_SRV_HST)
        {
            for (cp = remote_host; *cp; cp++)
            {
                if (isupper((unsigned char) (*cp)))
                    *cp = tolower((unsigned char) (*cp));
            }
        }

        if ((retval = krb5_get_host_realm(/*context, */remote_host, &hrealms)))
        {
            //free(remote_host);
            return retval;
        }

        if (!hrealms)
        {
            //free(remote_host);
            krb5_xfree(hrealms);
            return KRB5_ERR_HOST_REALM_UNKNOWN;
        }

        realm = hrealms;

        retval = krb5_build_principal(ret_princ, strlen(realm), realm, sname, hostname);

        krb5_princ_type(context, *ret_princ) = type;

        free(hrealms);
        return retval;
    }
    else 
    {
        return KRB5_SNAME_UNSUPP_NAMETYPE;
    }
}

unsigned int rtk_krb5_gss_import_name(unsigned int *minor_status, gss_buffer_t input_name_buffer, 
	gss_OID input_name_type, gss_name_t *output_name)
{
    krb5_error_code code;
    krb5_principal princ;

    if ((input_name_type != GSS_C_NULL_OID) &&
            (g_OID_equal(input_name_type, gss_nt_service_name) ||
             g_OID_equal(input_name_type, gss_nt_service_name_v2)))
    {
        char *service, *host = 0;
        service = input_name_buffer->value;
        code = rtk_krb5_sname_to_principal(host, service, KRB5_NT_SRV_HST, &princ);
    }
    *output_name = (gss_name_t) princ;
    return(GSS_S_COMPLETE);
}

void krb5_free_address(krb5_context context, krb5_address *val)
{
    if (val->contents)
        //krb5_xfree(val->contents);
        krb5_xfree(val);
}

krb5_error_code krb5_auth_con_free(krb5_context context, krb5_auth_context auth_context)
{
    if (auth_context->local_addr)
        krb5_free_address(context, auth_context->local_addr);
    if (auth_context->remote_addr)
        krb5_free_address(context, auth_context->remote_addr);
    if (auth_context->local_port)
        krb5_free_address(context, auth_context->local_port);
    if (auth_context->remote_port)
        krb5_free_address(context, auth_context->remote_port);
    if (auth_context->authentp)
        krb5_free_authenticator(context, auth_context->authentp);
    if (auth_context->keyblock)
        krb5_free_keyblock(context, auth_context->keyblock);
//   if (auth_context->send_subkey)
    //krb5_free_keyblock(context, auth_context->send_subkey);
//   if (auth_context->recv_subkey)
    //krb5_free_keyblock(context, auth_context->recv_subkey);
//   if (auth_context->rcache)
    //krb5_rc_close(context, auth_context->rcache);
    if (auth_context->permitted_etypes)
        krb5_xfree(auth_context->permitted_etypes);
    free(auth_context);
    return 0;
}

krb5_error_code krb5_auth_con_setrcache(krb5_context context, krb5_auth_context auth_context, 
	krb5_rcache rcache)
{
    auth_context->rcache = rcache;
    return 0;
}

void krb5_free_ticket(krb5_context context, krb5_ticket *val)
{
    if (val->server)
        krb5_free_principal(context, val->server, 0);
    //  if (val->enc_part.ciphertext.data)
    //krb5_xfree(val->enc_part.ciphertext.data);
    if (val->enc_part2)
        krb5_free_enc_tkt_part(context, val->enc_part2);
    if (val->scratch)
    {
        if (val->scratch->data)
        {
            val->scratch->data -= 2;
            free(val->scratch->data);
        }
        free(val->scratch);
    }
    krb5_xfree(val);
}

void krb5_free_context(krb5_context ctx)
{
    //krb5_os_free_context(ctx);

    if (ctx->in_tkt_ktypes) {
        free(ctx->in_tkt_ktypes);
        ctx->in_tkt_ktypes = 0;
    }

    if (ctx->tgs_ktypes) {
        free(ctx->tgs_ktypes);
        ctx->tgs_ktypes = 0;
    }

    if (ctx->default_realm) {
        free(ctx->default_realm);
        ctx->default_realm = 0;
    }

    if (ctx->ser_ctx_count && ctx->ser_ctx) {
        free(ctx->ser_ctx);
        ctx->ser_ctx = 0;
    }

    //krb5_clear_error_message(ctx);

    ctx->magic = 0;
    free(ctx);
}

void krb5_free_authenticator_contents(krb5_context context, krb5_authenticator *val)
{
    if (val->checksum) {
        krb5_free_checksum(context, val->checksum);
        val->checksum = 0;
    }
    if (val->client) {
        krb5_free_principal(context, val->client, 0);
        val->client = 0;
    }
    if (val->subkey) {
        krb5_free_keyblock(context, val->subkey);
        val->subkey = 0;
    }
    if (val->authorization_data) {
        krb5_free_authdata(context, val->authorization_data);
        val->authorization_data = 0;
    }

    free(val->scratch->data);
    free(val->scratch);
}

void krb5int_c_free_keyblock_contents(krb5_context context, register krb5_keyblock *key)
{
    if (key->contents) {
        krb5int_zap_data (key->contents, key->length);
        //krb5_xfree(key->contents);
        key->contents = 0;
    }
}

void krb5int_c_free_keyblock(krb5_context context, register krb5_keyblock *val)
{
    krb5int_c_free_keyblock_contents(context, val);
    krb5_xfree(val);
}

void krb5_free_keyblock(krb5_context context, register krb5_keyblock *val)
{
    krb5int_c_free_keyblock (context, val);
}

//int decode_ticket(int enctype, krb5_ticket *ticket, krb5_data* output)
int decode_ticket(int enctype, const krb5_keyblock *key, krb5_keyusage usage, const krb5_data *ivec,  
	krb5_enc_data *input, krb5_data* output)
{
    krb5_error_code retval;
    //if(enctype == rc4_hmac)
    //{
    int i;

    for (i=0; i<krb5_enctypes_length; i++)
    {
        if (krb5_enctypes_list[i].etype == key->enctype)
            break;
    }

    if (i == krb5_enctypes_length)
        return(KRB5_BAD_ENCTYPE);

    if ((input->enctype != ENCTYPE_UNKNOWN) && (krb5_enctypes_list[i].etype != input->enctype))
        return(KRB5_BAD_ENCTYPE);

    return((*(krb5_enctypes_list[i].decrypt))
           (krb5_enctypes_list[i].enc, krb5_enctypes_list[i].hash, key, usage, ivec, &input->ciphertext, output, 
           krb5_enctypes_list[i].shift));
 
    return retval;
} 

int checkValidGroup(unsigned int gid)
{
    ADUserInfo *ptr = (ADUserInfo *)(userhead->next);

    while (ptr)
    {
        if (ptr->ad && memcmp(ptr->sid + 24, &gid, 4) == 0)
        {
            return 0;
        }
        ptr=(ADUserInfo *)(ptr->next);
    }
    return -1;
}

int checkValidUser(unsigned char *sid, unsigned int user_rid)
{
    ADUserInfo *ptr = (ADUserInfo *)(userhead->next);

    while (ptr)
    {
        if (ptr->ad && memcmp(ptr->sid + 4, sid + 4, 20) == 0 && memcmp(ptr->sid + 24, &user_rid, 4) == 0)
        {
            return 0;
        }
        ptr=(ADUserInfo *)(ptr->next);
    }
    return -1;
}

int decode_krb5_enc_tkt_part(krb5_context context, krb5_ticket* ticket, krb5_data* code, krb5_enc_tkt_part **rep, 
	krb5_keyblock *key)
{
    int len, ret, i;
    unsigned char* p = code->data;
    unsigned char *end = p + code->length;
    unsigned char unused;
    int mm = 0;
    krb5_flags f = 0;

    //APPLICATION 3
    if ( ( ret = asn1_get_tag(&p, end, &len, 0x63) ) != 0)
        return ret;
    //SEQUENCE
    if ( ( ret = asn1_get_tag(&p , end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_0 ) ) != 0 )
        return ret;
    //flags[0]		TicketFlags
    if ( ( ret = asn1_get_tag(&p, end, &len , ASN1_BIT_STRING) ) != 0 )
        return ret;

    unused = *p;/*# of padding bits */
    p++;
    /* Number of unused bits must be between 0 and 7. */
    if (unused > 7)
        return POLARSSL_ERR_ASN1_INVALID_DATA;
    len--;

    for (i = 0; i < len; i++)
    {
        if (i<4)
        {
            f = (f<<8) | ((krb5_flags)(*p)&0xFF);
        }
        p++;
    }

    //p += len;
    if (len <= 4)
    {
        f &= ~(krb5_flags)0 << unused;
    }
    if (len < 4)
        f <<= (4 - len) * 8;

    (*rep)->flags = f;

    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_1 ) ) != 0 )
        return ret;

    //SEQUENCE
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
        return ret;
    //key[1]		EncryptionKey::= SEQUENCE{keytype[0]	Int32, keyvalue[1]	OCTET STRING}
    //keytype[0]	Int32
    (*rep)->session = (krb5_keyblock*)malloc(sizeof(krb5_keyblock));

    if ( ( ret = asn1_get_session_key(&p, end, (*rep)->session ) ) != 0 )
        return ret;

    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_2 ) ) != 0 )
        return ret;
    (*rep)->client = (krb5_principal)malloc(sizeof(krb5_principal_data));
    //crealm[2]		Realm,
    if ( ( ret = asn1_get_string(&p, end, &len, &(*rep)->client->realm ) ) != 0 )
        return ret;

    /*add for verify pac, using client->realm*/
    context->default_realm = (char*)malloc((*rep)->client->realm.length + 1);
    memcpy(context->default_realm, (*rep)->client->realm.data, (*rep)->client->realm.length);
    *(context->default_realm + (*rep)->client->realm.length) = 0;

    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_3 ) ) != 0 )
        return ret;
    //cname[3]		PrincipalName,
    if ( ( ret = asn1_get_principal_name( &p, end, (*rep)->client ) ) != 0 )
        return ret;
    //(*rep)->client->length = 1;
    //(*rep)->client->type = 1;
    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_4 ) ) != 0 )
        return ret;
    if ( ( ret = asn1_get_transited(&p, end, &(*rep)->transited) ) )
        return ret;

    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_5 ) ) != 0 )
        return ret;
    //authtime[5]		KerberosTime,
    if ( ( ret = asn1_get_GeneralizedTime( &p, end, &(*rep)->times.authtime )))
        return ret;

    //explicit constructed OPTIONAL
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_6 ) ) != 0 )
    {
        (*rep)->times.authtime = (*rep)->times.starttime;
    }
    else
    {
        //starttime[6]		KerberosTime,
        if ( ( ret = asn1_get_GeneralizedTime( &p, end, &(*rep)->times.starttime )))
            return ret;
    }
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_7 ) ) != 0 )
        return ret;
    //endtime[7]		KerberosTime,
    if ( ( ret = asn1_get_GeneralizedTime( &p, end, &(*rep)->times.endtime )))
        return ret;

    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_8 ) ) != 0 )
    {
        (*rep)->times.renew_till = 0;
    }
    else
    {
        //starttime[8]		KerberosTime,
        if ( ( ret = asn1_get_GeneralizedTime(&p, end, &(*rep)->times.renew_till )))
            return ret;
    }
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_9 ) ) != 0 )
        ;//return 0;
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_10 ) ) != 0 )
        ;//return 0;
    else
    {
        krb5_pac ppac;
        KVI *kvi;
        PLI2 pli2;
        unsigned char *tmp = NULL;
        krb5_error_code ret = 0;
        unsigned int i = 0, rid;
        unsigned char valid = 0;
        if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
		;
        if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
		;
        if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_0 ) ) != 0 )
		;
        if ( ( ret = asn1_get_int(&p, end, &mm) ) != 0 )
		;
        if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_1 ) ) != 0 )
		;
        if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_OCTET_STRING ) ) != 0 )
            return ret;
        if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
		;
        if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
		;
        if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_0 ) ) != 0 )
		;
        if ( ( ret = asn1_get_int(&p, end, &mm) ) != 0 )
		;
        if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_1 ) ) != 0 )
		;
        if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_OCTET_STRING ) ) != 0 )
            return ret;

        ticket->enc_part2 = (*rep);
        tmp = (unsigned char*)malloc(len);
        memcpy(tmp, p, len);
        krb5_pac_parse(NULL, tmp, len, &ppac);

        //PAC_LOGON_INFO
        //kvi = (KVI*)(p + ppac->pac->Buffers[0].Offset + 20/*marshall rpc*/);
        kvi = (KVI*)malloc(sizeof(KVI));
        //memset(kvi, 0, sizeof(KVI));
        memcpy(kvi, p + ppac->pac->Buffers[0].Offset + 20, sizeof(KVI));
        //used for NDR-encoded
        p += (sizeof(KVI)+ppac->pac->Buffers[0].Offset+20);
        memset(&pli2, 0, sizeof(PLI2));

        /*
        uni_str_len: The length, in bytes, of the string pointed to by the Buffer member,
        			not including the terminating null character if any. The length MUST be a multiple of 2.
        			The length SHOULD equal the entire size of the Buffer, in which case there is no terminating null character.
        			Any method that accesses this structure MUST use the Length specified instead of relying
        				on the presence or absence of a null character.
        */

        memcpy(&pli2.uni_user_name, p, 3* 4);
        p+=4*3;
        if (pli2.uni_user_name.uni_str_len > 0)
        {
            if (pli2.uni_user_name.uni_str_len % 2 == 1)
                pli2.uni_user_name.uni_str_len++;
            pli2.uni_user_name.buffer = (unsigned short*)malloc((pli2.uni_user_name.uni_str_len+1)*2);
            memcpy(pli2.uni_user_name.buffer, p, pli2.uni_user_name.uni_str_len*2);
        }
        p += pli2.uni_user_name.uni_str_len*2;
        if (pli2.uni_user_name.buffer != NULL)
        {
            free(pli2.uni_user_name.buffer);
            pli2.uni_user_name.buffer = NULL;
        }

        memcpy(&pli2.uni_full_name, p, 3* 4);
        p+=4*3;
        if (pli2.uni_full_name.uni_str_len > 0)
        {
            if (pli2.uni_full_name.uni_str_len % 2 == 1)
                pli2.uni_full_name.uni_str_len++;
            pli2.uni_full_name.buffer = (unsigned short*)malloc((pli2.uni_full_name.uni_str_len+1)*2);
            memcpy(pli2.uni_full_name.buffer, p, (pli2.uni_full_name.uni_str_len+1)*2);
        }
        p += pli2.uni_full_name.uni_str_len*2;

        //logon script
        memcpy(&pli2.uni_logon_script, p, 3* 4);
        p+=4*3;
        p += pli2.uni_logon_script.uni_str_len*2;

        //profile path
        memcpy(&pli2.uni_profile_path, p, 3* 4);
        p+=4*3;
        p += pli2.uni_profile_path.uni_str_len*2;

        //home directory
        memcpy(&pli2.uni_home_dir, p, 3* 4);
        p+=4*3;
        p += pli2.uni_home_dir.uni_str_len*2;

        //directory dirve
        memcpy(&pli2.uni_dir_drive, p, 3* 4);
        p+=4*3;
        p += pli2.uni_dir_drive.uni_str_len*2;

        //GROUP_MEMBERSHIP_ARRAY
        //pli2.groups.count = kvi->group_count;
        pli2.groups.count = kvi->group_count;
        //memcpy(&pli2.groups.count, &kvi->group_count, 4);
        p += sizeof(unsigned int);

        for (i = 0;i<kvi->group_count;i++)
        {
            //check predefined group

            //rid = ((GROUP_MEMBERSHIP*)(p + i*sizeof(GROUP_MEMBERSHIP)))->rid;
            memcpy(&rid, (p + i*sizeof(GROUP_MEMBERSHIP)), 4);

            if (rid == RTK_DOMAIN_GROUP_RID_ADMINS)
            {
                printf("RTK_DOMAIN_GROUP_RID_ADMINS\n");
            }
            else if (rid == RTK_DOMAIN_GROUP_RID_USERS)
            {
                printf("RTK_DOMAIN_GROUP_RID_USERS\n");
            }
            else if (rid == RTK_DOMAIN_GROUP_RID_GUESTS)
            {
                printf("RTK_DOMAIN_GROUP_RID_GUESTS\n");
            }
            if (checkValidGroup(rid) == 0)
            {
                valid = 1;
            }
        }

        if (valid == 0)
        {
            free(kvi);
            free(tmp);
            krb5_pac_free(context, ppac);
            return -1;
        }

        p += kvi->group_count * sizeof(GROUP_MEMBERSHIP);

        //domain controller, server (AMD-SCCM)
        memcpy(&pli2.uni_domain_controller, p, 3* 4);
        if (pli2.uni_domain_controller.uni_str_len % 2 == 1)
            pli2.uni_domain_controller.uni_str_len++;
        p+=4*3;
        p += pli2.uni_domain_controller.uni_str_len*2;

        //domain name, SCCMTEST
        memcpy(&pli2.uni_domain_name, p, 3* 4);
        if (pli2.uni_domain_name.uni_str_len % 2 == 1)
            pli2.uni_domain_name.uni_str_len++;
        p+=4*3;
        p += pli2.uni_domain_name.uni_str_len*2;

        pli2.dom_sid.num_auths = (unsigned int)*p;
        p += sizeof(unsigned int);
        pli2.dom_sid.sid.sid_rev_num = (unsigned char)*p;

        //memcpy(&tmpuserrid, kvi->user_rid, 4);
        //if (checkValidUser(p, kvi->user_rid) != 0)
        {
            //    free(kvi);
            //    krb5_pac_free(context, ppac);
            //     return -1;
        }
        free(kvi);
        p += sizeof(unsigned char);
        pli2.dom_sid.sid.num_auths = (unsigned char)*p;
        p += sizeof(unsigned char);

        memcpy(pli2.dom_sid.sid.id_auth, (unsigned char*)p, 6);
        p += 6 * sizeof(unsigned char);

        if ((ret = krb5_pac_verify(context, ppac, ticket->enc_part2->times.authtime, ticket->enc_part2->client, key, NULL)) != 0)
        {
            free(tmp);
            krb5_pac_free(context, ppac);
            printf("PAC checksum fail\n");
            return -1;
        }
        free(tmp);
        krb5_pac_free(context, ppac);

    }
    (*rep)->magic = KV5M_ENC_TKT_PART;
    return 0;
}

int krb5_decrypt_tkt_part(krb5_context context, krb5_data **scratch_for_ticket, int enctype, register krb5_ticket *ticket)
{
    int ret = 0, found = 0;
    unsigned char *t;
    krb5_enc_tkt_part *dec_tkt_part;
    krb5_keyblock *machinekey;
    //ADUserInfo *tmp = (ADUserInfo*)(userhead->next);
    //krb5_keytab_entry ktent;

    //krb5_data *scratch;
    (*scratch_for_ticket) = (krb5_data*)malloc(sizeof(krb5_data));
    (*scratch_for_ticket)->length = ticket->enc_part.ciphertext.length;
    if (!((*scratch_for_ticket)->data = malloc(ticket->enc_part.ciphertext.length+2)))
        return(-1);

    ticket->scratch = (*scratch_for_ticket);
    //decrypted_data->length = ticket->enc_part.ciphertext.length;
    //if (!(decrypted_data->data = (unsigned char*)malloc( ticket->enc_part.ciphertext.length)))
    //	return(ENOMEM);


    //key.contents = (unsigned char*)aeskey;
    //key.length = 16;
    //key.enctype = enctype;
    //decrypt ticket to plaintext
    //while(tmp)

    //	if(tmp->ad)

    machinekey = (krb5_keyblock*)malloc(sizeof(krb5_keyblock));
    memset(machinekey, 0, sizeof(krb5_keyblock));
    machinekey->contents = (unsigned char*)malloc(sizeof(dpconf->enckey));
    machinekey->length = sizeof(dpconf->enckey);
    machinekey->enctype = enctype;
    memcpy(machinekey->contents, dpconf->enckey, machinekey->length);

    ret = decode_ticket(enctype, machinekey, KRB5_KEYUSAGE_KDC_REP_TICKET,
                        0, &ticket->enc_part, (*scratch_for_ticket));
    if (!ret)
    {
        found = 1;
        //break;
    }
    //tmp = (ADUserInfo*)tmp->next;
    /*for alignment issue*/
    t = (unsigned char*)((*scratch_for_ticket)->data);
    (*scratch_for_ticket)->data = t + 2;

    if (found == 0)
        return ret;

    /*  now decode the decrypted stuff */
    dec_tkt_part = (krb5_enc_tkt_part*)malloc(sizeof(krb5_enc_tkt_part));
    ret = decode_krb5_enc_tkt_part(context, ticket, (*scratch_for_ticket), &dec_tkt_part, machinekey/*&rc4hmac_keyblock*/);
    free(machinekey->contents);
    free(machinekey);
    ticket->enc_part2 = dec_tkt_part;
    if (ret)
    {
        free(ticket->enc_part2->session);
        free(ticket->enc_part2->client->data);
        free(ticket->enc_part2->client);
        return ret;
    }
    //clean_scratch();
    return ret;
}

int decode_authenticator(int enctype, krb5_keyblock* key, krb5_keyusage usage, const krb5_data *ivec,
                         krb5_enc_data *input, krb5_data* output)
{
    //krb5_error_code retval;
    //if(enctype == rc4_hmac)
    int i;

    for (i=0; i<krb5_enctypes_length; i++)
    {
        if (krb5_enctypes_list[i].etype == key->enctype)
            break;
    }

    if (i == krb5_enctypes_length)
        return(KRB5_BAD_ENCTYPE);

    if ((input->enctype != ENCTYPE_UNKNOWN) && (krb5_enctypes_list[i].etype != input->enctype))
        return(KRB5_BAD_ENCTYPE);

    return((*(krb5_enctypes_list[i].decrypt))
           (krb5_enctypes_list[i].enc, krb5_enctypes_list[i].hash, key, usage, ivec, &input->ciphertext, output, 0));


    //struct krb5_hash_provider hash;
    //hash.blocksize = 64;
    //hash.hashsize = 16;

    //retval = krb5_arcfour_decrypt(&hash, key, KRB5_KEYUSAGE_AP_REQ_AUTH, 0, input, output, 0);
    //return 1;
    //return retval;
    //else if(enctype == aes128_cts_hmac_sha1_96)
    {
#if 0
        //int tmp = ticket->data;
        struct krb5_hash_provider hash;
        krb5_keyblock key;

        hash.blocksize = 64;
        hash.hashsize = 20;
        //hash.hash = k5_sha1_hash;

        key.contents = (unsigned char*)aeskey;
        key.length = 16;
        key.enctype = enctype;

        retval = krb5int_aes_dk_decrypt(NULL, &hash, &key, KRB5_KEYUSAGE_AP_REQ_AUTH, 0, input, output, 96/8, 1);
        return retval;
#endif
    }
    return -1;
}


int decode_krb5_authenticator(krb5_data *code, krb5_authenticator **rep)
{
    int len, ret, kvno;
    unsigned char* p = code->data;
    unsigned char *end = p + code->length;

    *rep = (krb5_authenticator*)malloc(sizeof(krb5_authenticator));
    memset(*rep, 0, sizeof(krb5_authenticator));

    /*Authenticator ::= [APPLICATION 2] SEQUENCE
    */
    //APPLICATION 2
    if ( ( ret = asn1_get_tag(&p, end, &len, 0x62) ) != 0)
        return ret;
    //SEQUENCE
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_0 ) ) != 0 )
        return ret;
    //authenticator-vno[0]
    if ( ( ret = asn1_get_int( &p, end, &kvno) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_1 ) ) != 0 )
        return ret;
	
    (*rep)->client = (krb5_principal)malloc(sizeof(krb5_principal_data));

    //crealm[1]		Realm,
    if ( ( ret = asn1_get_string(&p, end, &len, &(*rep)->client->realm ) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_2 ) ) != 0 )
        return ret;
    //cname[2]		PrincipalName,
    if ( ( ret = asn1_get_principal_name( &p, end, (*rep)->client ) ) != 0 )
        return ret;
    //(*rep)->client->length = 1;
    //(*rep)->client->type = 1;
    //(*rep)->client->data->data
    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_3 ) ) != 0 )
	;
    else
    {
        (*rep)->checksum = (krb5_checksum *)malloc(sizeof(krb5_checksum));
        //cname[3]		Checksum OPTIONAL,
        if ( ( ret = asn1_get_checksum( &p, end, (*rep)->checksum ) ) != 0 )
            return ret;
        p += (*rep)->checksum->length;
    }
    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_4 ) ) != 0 )
        return ret;
    //cusec[4]		INTERGER,
    if ( ( ret = asn1_get_int( &p, end, &(*rep)->cusec) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_5 ) ) != 0 )
        return ret;
    //ctime[5]		KerberosTime,
    if ( ( ret = asn1_get_GeneralizedTime( &p, end, &(*rep)->ctime) ) != 0 )
        return ret;
    //explicit constructed
     if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_6 ) ) != 0 )
	;
    else
    {
        //subkey[6]		EncryptionKey OPTIONAL
        //keytype[0]	Int32
        //SEQUENCE
        if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
            return ret;
        (*rep)->subkey = (krb5_keyblock*)malloc(sizeof(krb5_keyblock));

        if ( ( ret = asn1_get_session_key(&p, end, (*rep)->subkey ) ) != 0 )
            return ret;
    }

    //explicit constructed
    if ( ( ret = asn1_get_tag(&p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_7 ) ) != 0 )
	;
    else
    {
        //seq-number[7]		INTERGER OPTIONAL
        if ( ( ret = asn1_get_int( &p, end, &(*rep)->seq_number) ) != 0 )
            return ret;
    }
    return 0;
}

int decrypt_authenticator(krb5_ap_req *request, krb5_data **scratch_for_authenticator,
                          unsigned char *p, unsigned char *end, krb5_authenticator **authpp)
{
    int ret, len;
    krb5_authenticator *local_auth = NULL;
    krb5_keyblock *sesskey;
    (*scratch_for_authenticator) = malloc(sizeof(krb5_data));

    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_4 ) ) != 0 )
        return ret;
    /*   EncryptedData   ::= SEQUENCE {
           etype   [0] Int32 -- EncryptionType --,
           kvno    [1] UInt32 OPTIONAL,
           cipher  [2] OCTET STRING -- ciphertext
    */
    //SEQUENCE
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
        return ret;
    //etype   [0] Int32 -- EncryptionType --,
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_0 ) ) != 0 )
        return ret;
    if ( ( ret = asn1_get_int( &p, end, &request->authenticator.enctype) ) != 0 )
        return ret;

    //kvno    [1] UInt32 OPTIONAL,
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_1 ) ) != 0 )
        request->authenticator.kvno = 0;
    else if ( ( ret = asn1_get_uint( &p, end, &request->authenticator.kvno) ) != 0 )
        return ret;

    //cipher  [2] OCTET STRING -- ciphertext
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_2 ) ) != 0 )
        return ret;
    //cipher
    if ( ( ret = asn1_get_ticket( &p, end, &request->authenticator.ciphertext) ) != 0 )
        return ret;

    p += request->authenticator.ciphertext.length;

    sesskey = request->ticket->enc_part2->session; //decrypt authenticator using the session key extracted from the decrypted ticket
    (*scratch_for_authenticator)->length = request->authenticator.ciphertext.length;

    if (!((*scratch_for_authenticator)->data = (unsigned char*)malloc((*scratch_for_authenticator)->length)))
        return -1;

    if ((ret = decode_authenticator(request->authenticator.enctype, sesskey, KRB5_KEYUSAGE_AP_REQ_AUTH, 0, 
		&request->authenticator, (*scratch_for_authenticator))))
        return ret;
	
#define clean_scratch_for_authenticator() {memset(scratch_for_authenticator.data, 0, scratch_for_authenticator.length); \
free(scratch_for_authenticator.data);}

    /*  now decode the decrypted stuff */
    if (!(ret = decode_krb5_authenticator((*scratch_for_authenticator), &local_auth)))
    {
        *authpp = local_auth;
    }
    (*authpp)->scratch = (*scratch_for_authenticator);
    //clean_scratch();

    return ret;
}

krb5_boolean krb5_realm_compare(krb5_context context, krb5_const_principal princ1, krb5_const_principal princ2)
{
    if (krb5_princ_realm(context, princ1)->length !=
            krb5_princ_realm(context, princ2)->length ||
            memcmp (krb5_princ_realm(context, princ1)->data,
                    krb5_princ_realm(context, princ2)->data,
                    krb5_princ_realm(context, princ2)->length))
        return 0;

    return 1;
}

krb5_boolean krb5_principal_compare(krb5_context context, krb5_const_principal princ1, krb5_const_principal princ2)
{
    register int i;
    krb5_int32 nelem;

    nelem = krb5_princ_size(context, princ1);
    if (nelem != krb5_princ_size(context, princ2))
        return 0;

    if (! krb5_realm_compare(context, princ1, princ2))
        return 0;

    for (i = 0; i < (int) nelem; i++) {
        register const krb5_data *p1 = krb5_princ_component(context, princ1, i);
        register const krb5_data *p2 = krb5_princ_component(context, princ2, i);
        if (p1->length != p2->length ||
                memcmp(p1->data, p2->data, p1->length))
            return 0;
    }
    return 1;
}

static krb5_error_code krb5_rd_req_decoded_opt(krb5_context context, krb5_auth_context *auth_context,
        krb5_ap_req *request, krb5_const_principal server, krb5_keytab keytab, unsigned char *p, int size, 
        krb5_flags *ap_req_options,int check_valid_flag)

//int krb5_decode_ap_req(krb5_ap_req *request, unsigned char *p, int size)
{
    /*
       AP-REQ ::= [APPLICATION 14] SEQUENCE {
               pvno [0]        INTEGER,        -- indicates Version 5
               msg-type [1]    INTEGER,        -- indicates KRB_AP_REQ
               ap-options[2]   APOptions,
               ticket[3]       Ticket,
               authenticator[4]        EncryptedData
       }
    */
    //int x=0;
    krb5_error_code ret = 0;
    int pvno = 0, msg_type = 0, ap_options = 0, i = 0/*, tkt_vno = 0, name_type = 0, ticket_etype = 0*/;
    int len = 0, ticket_len = 0;
    //krb5_enc_tkt_part *tmp;
    //unsigned int kvno = 0;
    unsigned char *end = p+ size ;
    //krb5_authenticator authpp;
    //request->ticket = (krb5_ticket*)(malloc(sizeof(krb5_ticket)));
    krb5_data *scratch_for_ticket = NULL;
    krb5_data *scratch_for_authenticator = NULL;

    //APPLICATION 14
    if ( ( ret = asn1_get_tag(&p, end, &len, 0x6e) ) != 0)
        return ret;
    //SEQUENCE
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag( &p, end, &len,  ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_0 ) ) != 0 )
        return ret;
    //pvno [0]        INTEGER,
    if ( ( ret = asn1_get_int( &p, end, &pvno ) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_1 ) ) != 0 )
        return ret;
    //msg-type [1]    INTEGER,
    if ( ( ret = asn1_get_int( &p, end, &msg_type ) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_2 ) ) != 0 )
        return ret;
    //ap-options[2]   APOptions,
    //APOptions ::= BIT STRING{ reserved(0), use-session-key(1), mutual-required(2) }
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_BIT_STRING ) ) != 0 )
        return ret;

    //end = p + len;

    len--;
    p++;
    for (i = 0; i < len; i++)
    {
        if (i<4)
            ap_options = (ap_options<<8) | (p[i]&0xFF);
    }
    request->ap_options = ap_options;
    p += len;

    //explicit constructed
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_3 ) ) != 0 )
        return ret;
    ticket_len = len;
    /*
       Ticket ::= [APPLICATION 1] SEQUENCE {
               tkt-vno [0]     INTEGER,        -- indicates Version 5
               realm [1]       Realm,
               sname [2]       PrincipalName,
               enc-part [3]    EncryptedData
       }
    */
    //APPLICATION 1
    if ( ( ret = asn1_get_tag(&p, end, &len, 0x61) ) != 0)
        return ret;
    //SEQUENCE
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_0 ) ) != 0 )
        return ret;
    //tkt-vno [0]     INTEGER,        -- indicates Version 5
    if ( ( ret = asn1_get_int( &p, end, &request->ticket->magic ) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_1 ) ) != 0 )
        return ret;
    request->ticket->server = (krb5_principal)malloc(sizeof(krb5_principal_data));
    //realm [1]       Realm,
    if ( ( ret = asn1_get_string( &p, end, &len, &request->ticket->server->realm ) ) != 0 )
        return ret;
    //explicit constructed
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_2 ) ) != 0 )
        return ret;
    //sname [2]       PrincipalName,

    //SEQUENCE
    //request->ticket->server->data = (krb5_data*)malloc(sizeof(krb5_data));
    if ( (ret = asn1_get_principal_name( &p, end, request->ticket->server) ) )
        return ret;
    //request->ticket->server->type = 3;
    //enc-part[3] EncryptedData
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_3 ) ) != 0 )
        return ret;
    /*   EncryptedData   ::= SEQUENCE {
           etype   [0] Int32 -- EncryptionType --,
           kvno    [1] UInt32 OPTIONAL,
           cipher  [2] OCTET STRING -- ciphertext
    */
    //SEQUENCE
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
        return ret;
    //etype   [0] Int32 -- EncryptionType --,
    if ( ( ret = asn1_get_tag( &p, end, &len,  ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_0 ) ) != 0 )
        return ret;
    if ( ( ret = asn1_get_int( &p, end, &request->ticket->enc_part.enctype) ) != 0 )
        return ret;
    //kvno    [1] UInt32 OPTIONAL,
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_1 ) ) != 0 )
        return ret;
    if ( ( ret = asn1_get_uint( &p, end, &request->ticket->enc_part.kvno) ) != 0 )
        return ret;
    //cipher  [2] OCTET STRING -- ciphertext
    if ( ( ret = asn1_get_tag( &p, end, &len, ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | ASN1_EXPLICIT_TAG_2 ) ) != 0 )
        return ret;
    //cipher
    if ( ( ret = asn1_get_ticket( &p, end, &request->ticket->enc_part.ciphertext ) ) != 0 )
        return ret;

    p += request->ticket->enc_part.ciphertext.length;

    //decrypted_ticket = (krb5_data*)(malloc(sizeof(krb5_data)));
    //dec_tkt_part = (krb5_enc_tkt_part*)(malloc(sizeof(krb5_enc_tkt_part)));

    if ((ret = krb5_decrypt_tkt_part(context, &scratch_for_ticket, request->ticket->enc_part.enctype, request->ticket)) != 0)
        goto cleanup;

    /*tmp = request->ticket->enc_part2;*/
    //free(output.data);

    //free(request->ticket->server->data);
    /*tmp = request->ticket->enc_part2;*/
    //authenticator   [4] EncryptedData -- Authenticator
    /*
       -- Unencrypted authenticator
       Authenticator ::= [APPLICATION 2] SEQUENCE  {
               authenticator-vno[0]    INTEGER,
               crealm[1]               Realm,
               cname[2]                PrincipalName,
               cksum[3]                Checksum OPTIONAL,
               cusec[4]                INTEGER,
               ctime[5]                KerberosTime,
    */

    if ((ret = decrypt_authenticator(request, &scratch_for_authenticator, p, end, &((*auth_context)->authentp))))
    {
        free(request->ticket->enc_part2->session);
        while (request->ticket->enc_part2->client->length)
        {
            free(&request->ticket->enc_part2->client->data[request->ticket->enc_part2->client->length-1]);
            request->ticket->enc_part2->client->length--;
        }
        //free(request->ticket->enc_part2->client->data);
        free(request->ticket->enc_part2->client);
        goto cleanup;
    }

    if (!krb5_principal_compare(context, (*auth_context)->authentp->client, request->ticket->enc_part2->client))
    {
        ret = KRB5KRB_AP_ERR_BADMATCH;
        goto cleanup;
    }
#if 0
    retval = krb5_validate_times(context, &req->ticket->enc_part2->times);
    if (retval != 0)
        goto cleanup;
    if ((retval = krb5_timeofday(context, &currenttime)))
        goto cleanup;
    if (!in_clock_skew((*auth_context)->authentp->ctime))
    {
        retval = KRB5KRB_AP_ERR_SKEW;
        goto cleanup;
    }
#endif
    free(request->ticket->enc_part2->client->data);
    free(request->ticket->enc_part2->client);

    (*auth_context)->remote_seq_number = (*auth_context)->authentp->seq_number;
    if ((*auth_context)->authentp->subkey)
    {
        (*auth_context)->recv_subkey = (*auth_context)->authentp->subkey;

        //if ((retval = krb5_copy_keyblock(context,
        //				 (*auth_context)->authentp->subkey,
        //				 &((*auth_context)->recv_subkey))))
        //	goto cleanup;

        //retval = krb5_copy_keyblock(context, (*auth_context)->authentp->subkey,
        //				&((*auth_context)->send_subkey));
        (*auth_context)->send_subkey = (*auth_context)->authentp->subkey;
        if (ret)
        {
            //krb5_free_keyblock(context, (*auth_context)->recv_subkey);
            (*auth_context)->recv_subkey = NULL;
            (*auth_context)->send_subkey = NULL;
            goto cleanup;
        }
    }
    else
    {
        (*auth_context)->recv_subkey = 0;
        (*auth_context)->send_subkey = 0;
    }
    //  if ((retval = krb5_copy_keyblock(context, request->ticket->enc_part2->session,
    //		     &((*auth_context)->keyblock))))
    //goto cleanup;
    (*auth_context)->keyblock = request->ticket->enc_part2->session;

    /*
     * If not AP_OPTS_MUTUAL_REQUIRED then and sequence numbers are used
     * then the default sequence number is the one's complement of the
     * sequence number sent ot us.
     */
    if ((!(request->ap_options & AP_OPTS_MUTUAL_REQUIRED)) && (*auth_context)->remote_seq_number)
    {
        (*auth_context)->local_seq_number ^= (*auth_context)->remote_seq_number;
    }

    if (ap_req_options)
        *ap_req_options = request->ap_options;
    ret = 0;

cleanup:
    //  if (server == &princ_data)
    //krb5_free_default_realm(context, princ_data.realm.data);
    if (ret)
    {
        if (request->ticket)
        {
            krb5_free_ticket(context, request->ticket);
        }
        if (scratch_for_authenticator)
        {
            free(scratch_for_authenticator->data);
            free(scratch_for_authenticator);
        }
        /* only free if we're erroring out...otherwise some
           applications will need the output. */
        //if (request->ticket->enc_part2)
        //	krb5_free_enc_tkt_part(context, request->ticket->enc_part2);
        //	request->ticket->enc_part2 = NULL;
    }
    return ret;
}

OM_uint32  krb5_gss_release_cred(OM_uint32 *minor_status, gss_cred_id_t *cred_handle)
{
    krb5_context context;
    krb5_gss_cred_id_t cred;
    krb5_error_code code1/*, code2, code3*/;
    code1 = krb5_gss_init_context(&context);
    if (code1)
    {
        *minor_status = code1;
        return GSS_S_FAILURE;
    }
    if (*cred_handle == GSS_C_NO_CREDENTIAL)
    {
        *minor_status = 0;
        krb5_free_context(context);
        return(GSS_S_COMPLETE);
    }
    //if (! kg_delete_cred_id(*cred_handle)) {
    //   *minor_status = (OM_uint32) G_VALIDATE_FAILED;
    //   krb5_free_context(context);
    //   return(GSS_S_CALL_BAD_STRUCTURE|GSS_S_NO_CRED);
    //}
    cred = (krb5_gss_cred_id_t)*cred_handle;
    //k5_mutex_destroy(&cred->lock);
    /* ignore error destroying mutex */
    //if (cred->ccache)
    //   code1 = krb5_cc_close(context, cred->ccache);
    //else
    //   code1 = 0;
    //if (cred->keytab)
    //   code2 = krb5_kt_close(context, cred->keytab);
    //else
    //   code2 = 0;
    //if (cred->rcache)
    //   code3 = krb5_rc_close(context, cred->rcache);
    //else
    //   code3 = 0;
    if (cred->princ)
        krb5_free_principal(context, cred->princ, 0);
    if (cred->req_enctypes)
        free(cred->req_enctypes);
    free(cred);
    krb5_free_context(context);
    *cred_handle = NULL;
    *minor_status = 0;
    //if (code1)
    //   *minor_status = code1;
    //if (code2)
    //   *minor_status = code2;
    //if (code3)
    //   *minor_status = code3;
    return(*minor_status?GSS_S_FAILURE:GSS_S_COMPLETE);
}

void krb5_free_ap_req(krb5_context context, register krb5_ap_req *val)
{
    //  if (val->ticket)
    //krb5_free_ticket(context, val->ticket);
    //  if (val->authenticator.ciphertext.data)
    //krb5_xfree(val->authenticator.ciphertext.data);
    krb5_xfree(val);
}

krb5_error_code krb5_rd_req(krb5_context context, krb5_auth_context *auth_context, const krb5_data *inbuf,
                            krb5_const_principal server, krb5_keytab keytab, krb5_flags *ap_req_options, krb5_ticket **ticket)
{
    krb5_ap_req *request;
    krb5_error_code 	  retval;

    request = (krb5_ap_req*)malloc(sizeof(krb5_ap_req));
    request->magic = KV5M_AP_REQ;
    request->ticket = (krb5_ticket*)malloc(sizeof(krb5_ticket));
    memset(request->ticket, 0, sizeof(krb5_ticket));

    if (!server)
    {
        server = request->ticket->server;
    }

    retval = krb5_rd_req_decoded_opt(context, auth_context, request, server, keytab, inbuf->data, inbuf->length, 
		ap_req_options, 1); /* check_valid_flag */

    if (retval!=0)
        return -1;
    (*ticket) = request->ticket;
    krb5_free_ap_req(context, request);
    //krb5_decode_ap_req(&request, linux_rc4hmac, sizeof(linux_rc4hmac));
    return retval;
}


static krb5_error_code krb5_gss_init_context (krb5_context *ctxp)
{
    krb5_context ctx = 0;
    *ctxp = 0;
    ctx = (krb5_context)malloc(sizeof(struct _krb5_context));
    memset(ctx, 0, sizeof(struct _krb5_context));
    ctx->magic = KV5M_CONTEXT;

    ctx->clockskew = 300;

    *ctxp = ctx;
    return 0;
}

/*
 * Check to see whether or not a GSSAPI krb5 credential is valid.  If
 * it is not, return an error.
 */

//OM_uint32 krb5_gss_validate_cred_1(OM_uint32 *minor_status, gss_cred_id_t cred_handle,
//			 krb5_context context)
//{
//  krb5_gss_cred_id_t cred;
//    krb5_error_code code;
//    krb5_principal princ;

//  if (!kg_validate_cred_id(cred_handle)) {
//*minor_status = (OM_uint32) G_VALIDATE_FAILED;
//return(GSS_S_CALL_BAD_STRUCTURE|GSS_S_DEFECTIVE_CREDENTIAL);
//  }

// cred = (krb5_gss_cred_id_t) cred_handle;

//code = k5_mutex_lock(&cred->lock);
/* if (code) {
*minor_status = code;
return GSS_S_FAILURE;
 }*/

// if (cred->ccache) {
//		if ((code = krb5_cc_get_principal(context, cred->ccache, &princ))) {
////			k5_mutex_unlock(&cred->lock);
//			*minor_status = code;
//			return(GSS_S_DEFECTIVE_CREDENTIAL);
//		}
//		if (!krb5_principal_compare(context, princ, cred->princ)) {
//		 //   k5_mutex_unlock(&cred->lock);
//			*minor_status = KG_CCACHE_NOMATCH;
//			return(GSS_S_DEFECTIVE_CREDENTIAL);
//		}
//(void)krb5_free_principal(context, princ);
// }
// *minor_status = 0;
// return GSS_S_COMPLETE;
//}

OM_uint32 krb5_gss_validate_cred(OM_uint32* minor_status, gss_cred_id_t cred_handle)
{
    krb5_context context;
    krb5_error_code code;

    code = krb5_gss_init_context(&context);
    if (code)
    {
        *minor_status = code;
        return GSS_S_FAILURE;
    }

    *minor_status = 0;
    //krb5_gss_cred_id_t cred = (krb5_gss_cred_id_t) cred_handle;
    //k5_mutex_assert_locked(&cred->lock);
    //k5_mutex_unlock(&cred->lock);
    krb5_free_context(context);
    return GSS_S_COMPLETE;
}

/* returns decoded length, or < 0 on failure.  Advances buf and
   decrements bufsize */

static int der_read_length(unsigned char **buf, int *bufsize)
{
    unsigned char sf;
    int ret;

    if (*bufsize < 1)
        return(-1);
    sf = *(*buf)++;
    (*bufsize)--;
    if (sf & 0x80) {
        if ((sf &= 0x7f) > ((*bufsize)-1))
            return(-1);
        if (sf > sizeof(int))
            return (-1);
        ret = 0;
        for (; sf; sf--) {
            ret = (ret<<8) + (*(*buf)++);
            (*bufsize)--;
        }
    } else {
        ret = sf;
    }

    return(ret);
}

/*
 * Given a buffer containing a token, reads and verifies the token,
 * leaving buf advanced past the token header, and setting body_size
 * to the number of remaining bytes.  Returns 0 on success,
 * G_BAD_TOK_HEADER for a variety of errors, and G_WRONG_MECH if the
 * mechanism in the token does not match the mech argument.  buf and
 * *body_size are left unmodified on error.
 */

gss_int32 g_verify_token_header(const gss_OID_desc * mech, unsigned int *body_size, unsigned char **buf_in,
                                int tok_type, unsigned int toksize_in, int wrapper_required)
{
    unsigned char *buf = *buf_in;
    int seqsize;
    gss_OID_desc toid;
    int toksize = toksize_in;

    if ((toksize-=1) < 0)
        return(G_BAD_TOK_HEADER);
    if (*buf++ != 0x60) {
        if (wrapper_required)
            return(G_BAD_TOK_HEADER);
        buf--;
        toksize++;
        goto skip_wrapper;
    }

    if ((seqsize = der_read_length(&buf, &toksize)) < 0)
        return(G_BAD_TOK_HEADER);

    if (seqsize != toksize)
        return(G_BAD_TOK_HEADER);

    if ((toksize-=1) < 0)
        return(G_BAD_TOK_HEADER);
    if (*buf++ != 0x06)
        return(G_BAD_TOK_HEADER);

    if ((toksize-=1) < 0)
        return(G_BAD_TOK_HEADER);
    toid.length = *buf++;

    if ((toksize-=toid.length) < 0)
        return(G_BAD_TOK_HEADER);
    toid.elements = buf;
    buf+=toid.length;

    if (! g_OID_equal(&toid, mech))
        return  G_WRONG_MECH;
skip_wrapper:
    if (tok_type != -1) {
        if ((toksize-=2) < 0)
            return(G_BAD_TOK_HEADER);

        if ((*buf++ != ((tok_type>>8)&0xff)) ||
                (*buf++ != (tok_type&0xff)))
            return(G_WRONG_TOKID);
    }
    *buf_in = buf;
    *body_size = toksize;

    return 0;
}

krb5_error_code krb5_auth_con_init(krb5_context context, krb5_auth_context *auth_context)
{
    *auth_context = (krb5_auth_context)malloc(sizeof(struct _krb5_auth_context));
    if (!*auth_context)
        return -1;

    memset(*auth_context, 0, sizeof(struct _krb5_auth_context));

    /* Default flags, do time not seq */
    (*auth_context)->auth_context_flags =
        KRB5_AUTH_CONTEXT_DO_TIME |  KRB5_AUTH_CONN_INITIALIZED;

    (*auth_context)->req_cksumtype = context->default_ap_req_sumtype;
    (*auth_context)->safe_cksumtype = context->default_safe_sumtype;
    (*auth_context) -> checksum_func = NULL;
    (*auth_context)->checksum_func_data = NULL;
    (*auth_context)->magic = KV5M_AUTH_CONTEXT;
    return 0;
}

void krb5_free_authdata(krb5_context context, krb5_authdata **val)
{
    //register krb5_authdata **temp;

//   for (temp = val; *temp; temp++) {
    //if ((*temp)->contents)
    //    krb5_xfree((*temp)->contents);
    //krb5_xfree(*temp);
//   }
    krb5_xfree(val);
}

#if 0
static krb5_error_code krb5_copy_authdatum(krb5_context context, krb5_authdata *inad, krb5_authdata **outad)
{
    krb5_authdata *tmpad;

    if (!(tmpad = (krb5_authdata *)malloc(sizeof(*tmpad))))
        return -1;
    *tmpad = *inad;
    if (!(tmpad->contents = (krb5_octet *)malloc(inad->length))) {
        krb5_xfree(tmpad);
        return -1;
    }
    memcpy((char *)tmpad->contents, (char *)inad->contents, inad->length);
    *outad = tmpad;
    return 0;
}
#endif

void krb5_free_checksum_contents(krb5_context context, register krb5_checksum *val)
{
    if (val->contents)
    {
        //krb5_xfree(val->contents);
        val->contents = 0;
    }
}

void krb5_free_checksum(krb5_context context, register krb5_checksum *val)
{
    krb5_free_checksum_contents(context, val);
    krb5_xfree(val);
}

void krb5_free_principal(krb5_context context, krb5_principal val, int type)
{
    register krb5_int32 i;

    if (!val)
        return;

    if (val->data) {
        i = krb5_princ_size(context, val);
        if (type == 1)
        {
            while (--i >= 0)
                free(krb5_princ_component(context, val, i)->data);
        }
        krb5_xfree(val->data);
    }
    if (type == 1)
    {
        if (val->realm.data)
            krb5_xfree(val->realm.data);
    }
    krb5_xfree(val);
}

/*
 * Copy a keyblock, including alloc'ed storage.
 */
static krb5_error_code krb5_copy_keyblock(krb5_context context, const krb5_keyblock *from, krb5_keyblock **to)
{
    krb5_keyblock	*new_key;

    if (!(new_key = (krb5_keyblock *) malloc(sizeof(krb5_keyblock))))
        return -1;
    *new_key = *from;
    if (!(new_key->contents = (krb5_octet *)malloc(new_key->length))) {
        krb5_xfree(new_key);
        return(-1);
    }
    memcpy((char *)new_key->contents, (char *)from->contents, new_key->length);
    *to = new_key;
    return 0;
}

krb5_error_code krb5_copy_checksum(krb5_context context, const krb5_checksum *ckfrom, krb5_checksum **ckto)
{
    krb5_checksum *tempto;

    if (!(tempto = (krb5_checksum *)malloc(sizeof(*tempto))))
        return -1;
    *tempto = *ckfrom;

    if (!(tempto->contents =
                (krb5_octet *)malloc(tempto->length))) {
        krb5_xfree(tempto);
        return -1;
    }
    memcpy((char *) tempto->contents, (char *) ckfrom->contents,
           ckfrom->length);

    *ckto = tempto;
    return 0;
}

krb5_error_code krb5_copy_authenticator(krb5_context context, const krb5_authenticator *authfrom, krb5_authenticator **authto)
{
    krb5_error_code retval;
    krb5_authenticator *tempto;

    if (!(tempto = (krb5_authenticator *)malloc(sizeof(*tempto))))
        return -1;
    *tempto = *authfrom;

    retval = krb5_copy_principal(context, authfrom->client, &tempto->client);
    if (retval) {
        krb5_xfree(tempto);
        return retval;
    }

    if (authfrom->checksum &&
            (retval = krb5_copy_checksum(context, authfrom->checksum, &tempto->checksum))) {
        krb5_free_principal(context, tempto->client, 0);
        krb5_xfree(tempto);
        return retval;
    }

    if (authfrom->subkey) {
        retval = krb5_copy_keyblock(context, authfrom->subkey, &tempto->subkey);
        if (retval) {
            krb5_xfree(tempto->subkey);
            krb5_free_checksum(context, tempto->checksum);
            krb5_free_principal(context, tempto->client, 0);
            krb5_xfree(tempto);
            return retval;
        }
    }

    if (authfrom->authorization_data) {
        //retval = krb5_copy_authdata(context, authfrom->authorization_data,
        //		    &tempto->authorization_data);
        //if (retval) {
        //    krb5_xfree(tempto->subkey);
        //    krb5_free_checksum(context, tempto->checksum);
        //    krb5_free_principal(context, tempto->client);
        //    krb5_free_authdata(context, tempto->authorization_data);
        //    krb5_xfree(tempto);
        //    return retval;
        //}
    }

    *authto = tempto;
    return 0;
}

krb5_error_code krb5_auth_con_setflags(krb5_context context, krb5_auth_context auth_context, krb5_int32 flags)
{
    auth_context->auth_context_flags = flags;
    return 0;
}

krb5_error_code krb5_auth_con_getauthenticator(krb5_context context, krb5_auth_context auth_context, krb5_authenticator **authenticator)
{
    (*authenticator) = (krb5_authenticator *)malloc(sizeof(krb5_authenticator));
    (*authenticator)->client = auth_context->authentp->client;
    if (auth_context->authentp->checksum)
        (*authenticator)->checksum = auth_context->authentp->checksum;
    if (auth_context->authentp->subkey)
        (*authenticator)->subkey = auth_context->authentp->subkey;
    if (auth_context->authentp->authorization_data)
        (*authenticator)->authorization_data = auth_context->authentp->authorization_data;

    return 0;
}

krb5_error_code krb5_c_checksum_length(krb5_context context, krb5_cksumtype cksumtype, int *length)
{
    int i;

    for (i=0; i<krb5_cksumtypes_length; i++)
    {
        if (krb5_cksumtypes_list[i].ctype == cksumtype)
            break;
    }

    if (i == krb5_cksumtypes_length)
        return(KRB5_BAD_ENCTYPE);

    if (krb5_cksumtypes_list[i].keyhash)
        *length = krb5_cksumtypes_list[i].keyhash->hashsize;
    else if (krb5_cksumtypes_list[i].trunc_size)
        *length = krb5_cksumtypes_list[i].trunc_size;
    else
        *length = krb5_cksumtypes_list[i].hash->hashsize;

    return(0);
}

krb5_error_code krb5_auth_con_getrecvsubkey(krb5_context ctx, krb5_auth_context ac, krb5_keyblock **keyblock)
{
    if (ac->recv_subkey != NULL)
        return krb5_copy_keyblock(ctx, ac->recv_subkey, keyblock);
    *keyblock = NULL;
    return 0;
}

void krb5_free_keyblock_contents(krb5_context context, register krb5_keyblock *key)
{
    krb5int_c_free_keyblock_contents (context, key);
}

void krb5_free_addresses(krb5_context context, krb5_address **val)
{
    register krb5_address **temp;

    for (temp = val; *temp; temp++) {
        if ((*temp)->contents)
            krb5_xfree((*temp)->contents);
        krb5_xfree(*temp);
    }
    krb5_xfree(val);
}

krb5_error_code krb5_auth_con_getremoteseqnumber(krb5_context context, krb5_auth_context auth_context, krb5_int32 *seqnumber)
{
    *seqnumber = auth_context->remote_seq_number;
    return 0;
}

krb5_error_code krb5_auth_con_getflags(krb5_context context, krb5_auth_context auth_context, krb5_int32 *flags)
{
    *flags = auth_context->auth_context_flags;
    return 0;
}

krb5_error_code krb5_generate_seq_number(krb5_context context, const krb5_keyblock *key, krb5_ui_4 *seqno)
{
    krb5_data seed;
    krb5_error_code retval = 0;

    seed.length = key->length;
    seed.data = key->contents;
//   if ((retval = krb5_c_random_add_entropy(context, KRB5_C_RANDSOURCE_TRUSTEDPARTY, &seed)))
    //return(retval);

    seed.length = sizeof(*seqno);
    seed.data = (unsigned char *) seqno;
    //seed.data[0] = 0xc5;
    //seed.data[1] = 0xf6;
    //seed.data[2] = 0xc4;
    //seed.data[3] = 0x89;
    retval = krb5_c_random_make_octets(context, &seed);
    if (retval)
        return retval;
    /*
     * Work around implementation incompatibilities by not generating
     * initial sequence numbers greater than 2^30.  Previous MIT
     * implementations use signed sequence numbers, so initial
     * sequence numbers 2^31 to 2^32-1 inclusive will be rejected.
     * Letting the maximum initial sequence number be 2^30-1 allows
     * for about 2^30 messages to be sent before wrapping into
     * "negative" numbers.
     */
    *seqno &= 0x3fffffff;
    if (*seqno == 0)
        *seqno = 1;
    return 0;
}

#ifdef TEST_KERBEROS
unsigned char unEncryptedEncPart[]=
{
    0x7b, 0x41, 0x30, 0x3f,

    /*KerberosTime*/
    0xa0, 0x11, 0x18, 0x0f, 0x32, 0x30, 0x31, 0x30, 0x30, 0x34, 0x30, 0x37,
    0x31, 0x37, 0x35, 0x34, 0x32, 0x38, 0x5a,

    /*Integer*/
    0xa1, 0x05, 0x02, 0x03, 0x07, 0xe0, 0x19,

    /*EncryptionKey*/
    0xa2, 0x1b, 0x30, 0x19,
    0xa0, 0x03, 0x02, 0x01, 0x17,
    0xa1, 0x12, 0x04, 0x10, 0xc1, 0x0e, 0x77, 0x4b, 0xe6,
    0xe8, 0x87, 0x3e, 0x66, 0xef, 0x73, 0x3c, 0x62,
    0x40, 0x15, 0xc3,

    /*Integer*/
    0xa3, 0x06, 0x02, 0x04, 0x28,  0xc6, 0x1a, 0x91

};


unsigned char ap_req_tmpl[]=
{
    0x6f,0x74,
    0x30,0x72,
    0xa0,0x03,0x02,0x01,0x05,
    0xa1,0x03,0x02,0x01,0x0f,

    /*EncryptedData::=SEQUENCE
    {
    etype[0] Int32
    kvno[1] UInt32 OPTIONAL
    cipher [2] OCTET STRING
    }
    */
    0xa2,0x66,
    0x30,0x64,
    /*etype, rc4-hmac*/
    0xa0,0x03,0x02,0x01,0x17,
    /*cipher*/
    0xa2,0x5d,0x04,
    0x5b,0xfe,0xb2,0x0c,0x22,0x1c,0xde,0xb5,0xaa,0xec,0x87,0x6c,0x21,0x14,0xcb,0xa2,0xe7,0x4e
    ,0x01,0x57,0xe6,0x1f,0xa9,0x0c,0x23,0xeb,0x51,0x4d,0x46,0xa6,0x94,0x56,0x3c,0x5b
    ,0x4b,0x49,0xb6,0x48,0x1b,0x8c,0x68,0x57,0xdf,0x47,0xe4,0x02,0x28,0xdf,0xa5,0xa3
    ,0x5a,0x22,0x98,0x94,0x9c,0xc0,0x15,0x37,0x15,0x7c,0xe2,0x64,0x1e,0xa7,0xf3,0x0d
    ,0xdf,0xd9,0x36,0xc4,0x1e,0x86,0x33,0x7b,0xc5,0xe0,0x87,0x67,0x1b,0x85,0x56,0x8d
    ,0x00,0x1b,0x61,0x8d,0x34,0x0d,0x9f,0x68,0xdd,0x97
};
#endif

asn1_error_code asn1_insert_charstring(unsigned char *buf, const unsigned int len, const char *s)
{
    //asn1_error_code retval;
    unsigned int length;

    //retval = asn1buf_ensure_space(buf,len);
    //if(retval) return retval;
    for (length=0; length<len; length++,(buf)++)
        *(buf) = (char)(s[length]);
    return 0;
}

static asn1_error_code encode_integer(unsigned char *tmpusec,  int val, unsigned int *retlen)
{
    unsigned int length = 0;
    long valcopy;
    int digit;
    unsigned char *tmp = tmpusec;
    valcopy = val;
    do
    {
        digit = (int) (valcopy&0xFF);
        *tmp = digit;
        length++;
        valcopy = valcopy >> 8;
    } while (valcopy != 0 && valcopy != ~0);
    if ((val > 0) && ((digit&0x80) == 0x80)) /* make sure the high bit is */
    {
        *tmp = digit;
        length++;
    }
    else if ((val < 0) && ((digit&0x80) != 0x80))
    {
        *tmp = digit;
        length++;
    }
    *retlen = length;
    return 0;
}
unsigned char* genEncAPRepPart(unsigned char **tmprep, krb5_ap_rep_enc_part *rep)
{
//	asn1_error_code retval;
    unsigned char *encPart = NULL;
    unsigned char *encPartLen = NULL;
    unsigned char *encPartSeq = NULL;
    unsigned char *encPartSeqLen = NULL;
    unsigned char *ctime = NULL;
    unsigned char *ctimeLen = NULL;
    unsigned char *ctimeKerberosTime = NULL;
    unsigned char *ctimeKerberosTimeLen = NULL;
    unsigned char *cusec = NULL;
    unsigned char *cusecLen = NULL;
    unsigned char *cusecInt = NULL;
    unsigned char *cusecIntLen = NULL;
    unsigned char *cusecIntContent = NULL;

    unsigned char *subkey = NULL;
    unsigned char *subkeyLen = NULL;
    unsigned char *subkeySeq = NULL;
    unsigned char *subkeySeqLen = NULL;
    //unsigned char *subkeySeqContent = NULL;
    unsigned char *keytype = NULL;
    unsigned char *keytypeLen = NULL;
    unsigned char *keytypeInt = NULL;
    unsigned char *keytypeIntLen = NULL;
    unsigned char *keytypeIntContent = NULL;
    unsigned char *keyvalue = NULL;
    unsigned char *keyvalueLen = NULL;
    unsigned char *keyvalueStr = NULL;
    unsigned char *keyvalueStrLen = NULL;
    unsigned char *keyvalueStrContent = NULL;
    unsigned char *seqNum = NULL;
    unsigned char *seqNumLen = NULL;
    unsigned char *seqNumInt = NULL;
    unsigned char *seqNumIntLen = NULL;
    unsigned char *seqNumIntContent = NULL;
    unsigned char *end=  NULL;
    unsigned int useclen = 0, i,j;
    unsigned char tmpusecbuf[10];

    //char s[16], *sp;
    //struct tm *gtime, gtimebuf;
    //time_t gmt_time = (*rep).ctime;
    encPart = (*tmprep);
    *encPart = 0x7b; //application 27
    encPartLen = encPart + 1;
    encPartSeq = encPartLen + 1;
    encPartSeqLen = encPartSeq + 1;
    *encPartSeq = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
    encPartSeqLen = encPartSeq + 1;
    ctime = encPartSeqLen + 1;
    *ctime = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0x00; //0xa0
    ctimeLen = ctime + 1;
    *ctimeLen = 0x11;
    ctimeKerberosTime = ctimeLen + 1;
    ctimeKerberosTimeLen = ctimeKerberosTime + 1;
    *ctimeKerberosTime = 0x18;
    *ctimeKerberosTimeLen = 0x0f;
#if 0
    gtime = gmtime(&gmt_time);
    if (gtime == NULL)
        return NULL;
    memcpy(&gtimebuf, gtime, sizeof(gtimebuf));
    gtime = &gtimebuf;

    if (gtime->tm_year > 8099 || gtime->tm_mon > 11 ||
            gtime->tm_mday > 31 || gtime->tm_hour > 23 ||
            gtime->tm_min > 59 || gtime->tm_sec > 59)
        return NULL;
    sprintf(s, "%04d%02d%02d%02d%02d%02dZ",
            1900+gtime->tm_year, gtime->tm_mon+1, gtime->tm_mday,
            gtime->tm_hour, gtime->tm_min, gtime->tm_sec);
    sp = s;
    retval = asn1_insert_charstring(ctimeKerberosTimeLen+1,15,sp);
#endif
    sprintf((char *)(ctimeKerberosTimeLen+1), "20101229205104Z");
    //if(retval) return retval;
    cusec = ctimeKerberosTimeLen + 15 + 1;
    //cusec = ctimeLen + 15 + 1;
    *cusec = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0x01; //0xa1

    cusecLen = cusec + 1;

    encode_integer(tmpusecbuf, (*rep).cusec, &useclen);
    *cusecLen = useclen + 2;

#ifdef TESTAPREP
    *cusecLen = 0x05;
#endif
    cusecInt = cusecLen+1;
    *cusecInt = ASN1_INTEGER;
    cusecIntLen = cusecInt + 1;

    cusecIntContent = cusecIntLen + 1;

    *cusecIntLen = useclen;
    for (i=useclen, j=0;i>0;i--,j++)
        *(cusecIntContent + j) = *(tmpusecbuf+i-1);

#ifdef TESTAPREP
    *cusecIntLen = 3;
#endif

#ifdef TESTAPREP
    *cusecIntContent = 0x05;
    *(cusecIntContent + 1) = 0x04;
    *(cusecIntContent + 2) = 0x79;
#endif
    subkey = cusecIntContent + (unsigned char)*cusecIntLen;
    *subkey = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0x02; //0xa2
    subkeyLen = subkey + 1;
    *subkeyLen = 0x1b;
    subkeySeq = subkeyLen + 1;
    *subkeySeq = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
    subkeySeqLen = subkeySeq + 1;
    *subkeySeqLen = 0x19;

    keytype = subkeySeqLen + 1;
    *keytype = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0x00; //0xa0 keytype
    keytypeLen = keytype + 1;
    *keytypeLen = 3;
    keytypeInt = keytypeLen + 1;
    *keytypeInt = ASN1_INTEGER;
    keytypeIntLen = keytypeInt + 1;
    *keytypeIntLen = 1;
    keytypeIntContent = keytypeIntLen + 1;
    *keytypeIntContent = 0x17;//rc4-hmac
    keyvalue = keytypeIntContent + (unsigned char)*keytypeIntLen;
    *keyvalue = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0x01; //0xa1 keyvalue
    keyvalueLen = keyvalue + 1;
    *keyvalueLen = rep->subkey->length + 2;
    keyvalueStr = keyvalueLen + 1;
    *keyvalueStr = ASN1_OCTET_STRING;
    keyvalueStrLen = keyvalueStr + 1;
    *keyvalueStrLen = rep->subkey->length;
    keyvalueStrContent = keyvalueStrLen + 1;
    memcpy(keyvalueStrContent, rep->subkey->contents, rep->subkey->length);
    seqNum = keyvalueStrLen + rep->subkey->length + 1;
    //seqNum = keyvalueStrContent + (unsigned char)*keyvalueStrLen;

    *seqNum = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0x03;
    seqNumLen = seqNum + 1;
    *seqNumLen = 0x06;
    seqNumInt = seqNumLen + 1;
    *seqNumInt = ASN1_INTEGER;
    seqNumIntLen = seqNumInt + 1;
    *seqNumIntLen = 4;
    seqNumIntContent = seqNumIntLen + 1;
    memcpy(seqNumIntContent, &rep->seq_number, 4);
    //(unsigned char)*(seqNumIntContent+1) = 0xc6;
    //(unsigned char)*(seqNumIntContent+2) = 0x1a;
    //(unsigned char)*(seqNumIntContent+3) = 0x91;
    end = seqNumIntContent + 4-1;

    *encPartLen = end - encPart - 1;
    *encPartSeqLen = (unsigned char)*encPartLen - 2;
    return end;
}

krb5_error_code encode_krb5_ap_rep(const krb5_ap_rep *rep, krb5_data **code)
{
    unsigned char *totalSeq = NULL;
    unsigned char *totalSeqLen = NULL;
    unsigned char *totalApp = NULL;
    unsigned char *totalAppLen = NULL;
    unsigned char *pvno = NULL;
    //unsigned char *pvnoLen = NULL;
    //unsigned char *pvnoInt = NULL;
    //unsigned char *msgType = NULL;
    //unsigned char *msgTypeLen = NULL;
    unsigned char *encPart = NULL;
    unsigned char *encPartLen = NULL;
    unsigned char *encPartSeq = NULL;
    unsigned char *encPartSeqLen = NULL;
    unsigned char *etype = NULL;
    unsigned char *etypeType = NULL;
    unsigned char *cipher = NULL;
    unsigned char *cipherLen = NULL;
    unsigned char *cipherStr = NULL;
    unsigned char *cipherStrLen = NULL;
    unsigned char *cipherStrContent = NULL;

    unsigned char *totalRep = (unsigned char*)malloc(4096);
    memset(totalRep, 0, 4096);

    totalApp = totalRep;
    *totalApp = 0x6f;
    totalAppLen = totalApp+1;
    totalSeq = totalAppLen + 1;
    *totalSeq = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
    totalSeqLen = totalSeq + 1;
    pvno = totalSeqLen + 1;
    *pvno = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0x00;//0xa0
    memcpy(pvno, ap_req_pvno_msgtyp, sizeof(ap_req_pvno_msgtyp));
    //pvnoLen = pvno + 1;
    //(unsigned char)*pvnoLen = sizeof(ap_req_pvno_msgtyp);
    //pvnoInt = pvnoLen + 1;
    //memcpy(pvnoInt, ap_req_pvno_msgtyp, sizeof(ap_req_pvno_msgtyp));
    encPart = pvno+ sizeof(ap_req_pvno_msgtyp);
    *encPart = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0x02;//0xa2
    encPartLen = encPart + 1;
    *encPartLen = rep->enc_part.ciphertext.length + 2 + 9;
    encPartSeq = encPartLen + 1;
    *encPartSeq = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
    encPartSeqLen = encPartSeq + 1;
    *encPartSeqLen = rep->enc_part.ciphertext.length + 2 + 7;
    etype = encPartSeqLen + 1;
    *etype = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0x00; //0xa0 etype
    *(etype+1) = 0x03;
    *(etype+2) = 0x02;
    *(etype+3) = 0x01;
    etypeType = etype + 4;
    *etypeType = 0x17;//rc4-hmac
    cipher = etypeType + 1;
    *cipher = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0x02; //0xa2 cipher
    cipherLen = cipher + 1;
    *cipherLen = rep->enc_part.ciphertext.length + 2;
    cipherStr = cipherLen + 1;
    *cipherStr = ASN1_OCTET_STRING;
    cipherStrLen = cipherStr + 1;
    *cipherStrLen = rep->enc_part.ciphertext.length;
    cipherStrContent = cipherStrLen + 1;
    memcpy(cipherStrContent, rep->enc_part.ciphertext.data, (unsigned char)*cipherStrLen);
    //cipherStr = cipherLen + 1;
    //(unsigned char)*cipherStr = ASN1_OCTET_STRING;
    //cipherStrLen = cipherStr + 1;
    //(unsigned char)*cipherStrLen = (*code)->length;
    //cipherStrContent = cipherStrLen + 1;
    //memcpy(cipherStrContent, (*code)->data, (*code)->length);

    *totalAppLen = cipherStrContent + (unsigned char)*cipherStrLen - totalRep - 2;
    *totalSeqLen = (unsigned char)*totalAppLen - 2;
    (*code)->data = totalApp;
    (*code)->length = (unsigned char)*totalAppLen + 2;
    return 0;
}


krb5_error_code encode_krb5_ap_rep_enc_part(krb5_ap_rep_enc_part *rep, krb5_data **code)
{

    /*EncAPRepPart ::= [APPLICATION 27] SEQUENCE {
            ctime [0]       KerberosTime,
            cusec [1]       INTEGER,
            subkey [2]      EncryptionKey OPTIONAL,
            seq-number [3]  INTEGER OPTIONAL
    }*/

    //unsigned char *totalSeq = NULL;
    //unsigned char *totalSeqLen = NULL;
    //unsigned char *totalApp = NULL;
    //unsigned char *totalAppLen = NULL;
    //unsigned char *pvno = NULL;
    //unsigned char *pvnoLen = NULL;
    //unsigned char *pvnoInt = NULL;
    //unsigned char *msgType = NULL;
    //unsigned char *msgTypeLen = NULL;
    //unsigned char *encPart = NULL;
    //unsigned char *encPartLen = NULL;
    //unsigned char *encPartSeq = NULL;
    //unsigned char *encPartSeqLen = NULL;
    //unsigned char *etype = NULL;
    //unsigned char *etypeType = NULL;
    //unsigned char *cipher = NULL;
    //unsigned char *cipherLen = NULL;
    //unsigned char *cipherStr = NULL;
    //unsigned char *cipherStrLen = NULL;

    unsigned char *glo_csr_ptr, *end;

    unsigned char *encryptedpart = (unsigned char*)malloc(1024);
    memset(encryptedpart, 0, 1024);
    glo_csr_ptr = encryptedpart;

    end = genEncAPRepPart(&glo_csr_ptr, rep);

    (*code)->data = encryptedpart;
    (*code)->length = end - encryptedpart + 1;
    return 0;
}

krb5_error_code krb5_c_encrypt_length(krb5_context context, krb5_enctype enctype, int inputlen, int *length)
{
    int i;
    for (i=0; i<krb5_enctypes_length; i++) {
        if (krb5_enctypes_list[i].etype == enctype)
            break;
    }
    if (i == krb5_enctypes_length)
        return(KRB5_BAD_ENCTYPE);

    (*(krb5_enctypes_list[i].encrypt_len))
    (krb5_enctypes_list[i].enc, krb5_enctypes_list[i].hash,
     inputlen, length);
    return(0);
}

krb5_error_code krb5_c_encrypt(krb5_context context, const krb5_keyblock *key, krb5_keyusage usage, const krb5_data *ivec,
                               const krb5_data *input, krb5_enc_data *output)
{
    int i;
    for (i=0; i<krb5_enctypes_length; i++)
    {
        if (krb5_enctypes_list[i].etype == key->enctype)
            break;
    }
    if (i == krb5_enctypes_length)
        return(KRB5_BAD_ENCTYPE);
    output->magic = KV5M_ENC_DATA;
    output->kvno = 0;
    output->enctype = key->enctype;
    return((*(krb5_enctypes_list[i].encrypt))
           (krb5_enctypes_list[i].enc, krb5_enctypes_list[i].hash,
            key, usage, ivec, input, &output->ciphertext, 0));
}

krb5_error_code krb5_encrypt_helper(krb5_context context, const krb5_keyblock *key, krb5_keyusage usage,
                                    const krb5_data *plain, krb5_enc_data *cipher)
{
    krb5_error_code ret = 0;
    int enclen;
#if 1
    if ((ret = krb5_c_encrypt_length(context, key->enctype, plain->length, &enclen)))
        return(ret);
    cipher->ciphertext.length = enclen;
    if ((cipher->ciphertext.data = (unsigned char *) malloc(enclen)) == NULL)
        return(ret);
    ret = krb5_c_encrypt(context, key, usage, 0, plain, cipher);
    if (ret)
    {
        free(cipher->ciphertext.data);
        cipher->ciphertext.data = NULL;
    }
#endif
#if 0
    if (key->enctype == ENCTYPE_ARCFOUR_HMAC)
    {
        krb5_arcfour_encrypt_length(&krb5int_enc_arcfour, &krb5int_hash_md5, plain->length, &enclen);
    }
//   if ((ret = krb5_c_encrypt_length(context, key->enctype, plain->length,
    //			     &enclen)))
    //return(ret);

    cipher->ciphertext.length = enclen;
    if ((cipher->ciphertext.data = (unsigned char *) malloc(enclen)) == NULL)
        return(ret);
//   ret = krb5_arcfour_encrypt(context, key, usage, 0, plain, cipher);
//   if (ret)
    //{
    //	free(cipher->ciphertext.data);
    //	cipher->ciphertext.data = NULL;
//   }
    if (key->enctype == ENCTYPE_ARCFOUR_HMAC)
    {
        ret = krb5_arcfour_encrypt(&krb5int_enc_arcfour, &krb5int_hash_md5, key, usage,
                                   0, plain, &cipher->ciphertext, 0);
        if (ret)
        {
            free(cipher->ciphertext.data);
            cipher->ciphertext.data = NULL;
        }
    }
#endif
    return(ret);
}

void krb5_free_data(krb5_context context, krb5_data *val)
{
    if (val->data)
        krb5_xfree(val->data);
    krb5_xfree(val);
}


krb5_error_code krb5_mk_rep(krb5_context context, krb5_auth_context auth_context, krb5_data *outbuf)
{
    krb5_error_code 	  retval;
    krb5_ap_rep_enc_part  repl;
    krb5_ap_rep 	  reply;
    krb5_data 		* scratch;
    krb5_data 		* toutbuf;

#ifdef TESTAPREP
    unsigned char testsubkey[]={0xc0,0x6f,0x40,0x0f,0xc0,0x25,0x61,0xf5,
                                0x66,0x7f,0x69,0x7c,0xcb,0xa7,0x2e,0x5b
                               };
    int testctime = 1282577682;
    int testcusec = 328825;
    unsigned int testseqnum = 341500966;
    unsigned char testkey[] = {0xef,0x77,0x22,0x5a,0x0d,0x6f,0x66,0xe4,
                               0xf7,0xb2,0x39,0x85,0x26,0xba,0x0d,0xc6
                              };
#endif
    /* Make the reply */
    if (((auth_context->auth_context_flags & KRB5_AUTH_CONTEXT_DO_SEQUENCE) ||
            (auth_context->auth_context_flags & KRB5_AUTH_CONTEXT_RET_SEQUENCE)) &&
            (auth_context->local_seq_number == 0))
    {
        if ((retval = krb5_generate_seq_number(context, auth_context->keyblock,
                                               &auth_context->local_seq_number)))
            return(retval);
    }

#ifdef TESTAPREP
    auth_context->authentp->ctime = testctime;
#endif
    repl.ctime = auth_context->authentp->ctime;
#ifdef TESTAPREP
    auth_context->authentp->cusec = testcusec;
#endif
    repl.cusec = auth_context->authentp->cusec;
//   if (auth_context->auth_context_flags & KRB5_AUTH_CONTEXT_USE_SUBKEY)
    //{
    //	retval = krb5int_generate_and_save_subkey (context, auth_context,
    //						   auth_context->keyblock);
    //	if (retval)
    //		return retval;
    //	repl.subkey = auth_context->send_subkey;
//   }
    //else
#ifdef TESTAPREP
    memcpy(auth_context->authentp->subkey->contents, testsubkey, sizeof(testsubkey));
#endif
    repl.subkey = auth_context->authentp->subkey;
#ifdef TESTAPREP
    auth_context->local_seq_number = testseqnum;
#endif
    repl.seq_number = htonl(auth_context->local_seq_number);
#ifdef TESTAPREP
    memcpy(auth_context->keyblock->contents, testkey, sizeof(testkey));
#endif
    /* encode it before encrypting */
    scratch = (krb5_data*)malloc(sizeof(krb5_data));
    if ((retval = encode_krb5_ap_rep_enc_part(&repl, &scratch)))
        return retval;

    if ((retval = krb5_encrypt_helper(context, auth_context->keyblock, KRB5_KEYUSAGE_AP_REP_ENCPART, scratch, &reply.enc_part)))
        goto cleanup_scratch;

    toutbuf = (krb5_data*)malloc(sizeof(krb5_data));
    if (!(retval = encode_krb5_ap_rep(&reply, &toutbuf)))
    {
        *outbuf = *toutbuf;
        krb5_xfree(toutbuf);
    }

    memset(reply.enc_part.ciphertext.data, 0, reply.enc_part.ciphertext.length);
    free(reply.enc_part.ciphertext.data);
    reply.enc_part.ciphertext.length = 0;
    reply.enc_part.ciphertext.data = 0;

cleanup_scratch:
    memset(scratch->data, 0, scratch->length);
    krb5_free_data(context, scratch);

    return retval;
}

/* Checksumming the channel bindings always uses plain MD5.  */
krb5_error_code kg_checksum_channel_bindings(krb5_context context, gss_channel_bindings_t cb, krb5_checksum *cksum, int bigend)

{
//   size_t len;
    //char *buf = 0;
//   char *ptr;
    int sumlen;
//   krb5_data plaind;
    krb5_error_code code;
//   void *temp;

    /* initialize the the cksum */
    code = krb5_c_checksum_length(context, CKSUMTYPE_RSA_MD5, &sumlen);
    if (code)
        return(code);

    cksum->checksum_type = CKSUMTYPE_RSA_MD5;
    cksum->length = sumlen;

    /* generate a buffer full of zeros if no cb specified */

    if (cb == GSS_C_NO_CHANNEL_BINDINGS) {
        if ((cksum->contents = (krb5_octet *) malloc(cksum->length)) == NULL) {
            return(-1);
        }
        memset(cksum->contents, '\0', cksum->length);
        return(0);
    }
    return -1;
}

krb5_error_code krb5_auth_con_getlocalseqnumber(krb5_context context, krb5_auth_context auth_context, krb5_int32 *seqnumber)
{
    *seqnumber = auth_context->local_seq_number;
    return 0;
}

static unsigned int der_length_size(int length)
{
    if (length < (1<<7))
        return(1);
    else if (length < (1<<8))
        return(2);
#if INT_MAX == 0x7fff
    else
        return(3);
#else
    else if (length < (1<<16))
        return(3);
    else if (length < (1<<24))
        return(4);
    else
        return(5);
#endif
}

/* returns the length of a token, given the mech oid and the body size */

unsigned int g_token_size(const gss_OID_desc * mech, unsigned int body_size)
{
    /* set body_size to sequence contents size */
    body_size += 4 + (int) mech->length;         /* NEED overflow check */
    return(1 + der_length_size(body_size) + body_size);
}

static void der_write_length(buf, length)
unsigned char **buf;
int length;
{
    if (length < (1<<7)) {
        *(*buf)++ = (unsigned char) length;
    } else {
        *(*buf)++ = (unsigned char) (der_length_size(length)+127);
#if INT_MAX > 0x7fff
        if (length >= (1<<24))
            *(*buf)++ = (unsigned char) (length>>24);
        if (length >= (1<<16))
            *(*buf)++ = (unsigned char) ((length>>16)&0xff);
#endif
        if (length >= (1<<8))
            *(*buf)++ = (unsigned char) ((length>>8)&0xff);
        *(*buf)++ = (unsigned char) (length&0xff);
    }
}


/* fills in a buffer with the token header.  The buffer is assumed to
   be the right size.  buf is advanced past the token header */

void g_make_token_header(const gss_OID_desc * mech,unsigned int body_size, unsigned char **buf, int tok_type)
{
    *(*buf)++ = 0x60;
    der_write_length(buf, (tok_type == -1) ?2:4 + mech->length + body_size);
    *(*buf)++ = 0x06;
    *(*buf)++ = (unsigned char) mech->length;
    TWRITE_STR(*buf, mech->elements, mech->length);
    if (tok_type != -1) {
        *(*buf)++ = (unsigned char) ((tok_type>>8)&0xff);
        *(*buf)++ = (unsigned char) (tok_type&0xff);
    }
}



void krb5_free_authenticator(krb5_context context, krb5_authenticator *val)
{
    krb5_free_authenticator_contents(context, val);
    krb5_xfree(val);
}

void krb5_free_data_contents(krb5_context context, krb5_data *val)
{
    if (val->data)
    {
        krb5_xfree(val->data);
        val->data = 0;
    }
}
/*
 * This is an internal routine which validates the krb5_timestamps
 * field in a krb5_ticket.
 */


krb5_error_code krb5_timeofday(krb5_context context, register krb5_timestamp *timeret)
{
#if 0
    krb5_os_context os_ctx = context->os_context;
    time_t tval;

    if (os_ctx->os_flags & KRB5_OS_TOFFSET_TIME) {
        *timeret = os_ctx->time_offset;
        return 0;
    }
    tval = time(0);
    if (tval == (time_t) -1)
        return (krb5_error_code) errno;
    if (os_ctx->os_flags & KRB5_OS_TOFFSET_VALID)
        tval += os_ctx->time_offset;
    *timeret = tval;
#endif
    time_t tval;
    tval = time(0);
    *timeret = tval;
    return 0;
}

static krb5_error_code actx_copy_addr(krb5_context context, const krb5_address *inad, krb5_address **outad)
{
    krb5_address *tmpad;

    if (!(tmpad = (krb5_address *)malloc(sizeof(*tmpad))))
        return -1;
    *tmpad = *inad;
    if (!(tmpad->contents = (krb5_octet *)malloc(inad->length))) {
        krb5_xfree(tmpad);
        return -1;
    }
    memcpy((char *)tmpad->contents, (char *)inad->contents, inad->length);
    *outad = tmpad;
    return 0;
}

krb5_error_code krb5_auth_con_setaddrs(krb5_context context, krb5_auth_context auth_context, krb5_address *local_addr,
                                       krb5_address *remote_addr)
{
    krb5_error_code	retval;
    if (auth_context->local_addr)
        (void) krb5_free_address(context, auth_context->local_addr);
    if (auth_context->remote_addr)
        (void) krb5_free_address(context, auth_context->remote_addr);
    retval = 0;
    if (local_addr)
        retval = actx_copy_addr(context,
                                local_addr,
                                &auth_context->local_addr);
    else
        auth_context->local_addr = NULL;
    if (!retval && remote_addr)
        retval = actx_copy_addr(context,
                                remote_addr,
                                &auth_context->remote_addr);
    else
        auth_context->remote_addr = NULL;
    return retval;
}

krb5_error_code krb5int_c_mandatory_cksumtype (krb5_context ctx, krb5_enctype etype, krb5_cksumtype *cksumtype)
{
    int i;
    for (i = 0; i < krb5_enctypes_length; i++)
    {
        if (krb5_enctypes_list[i].etype == etype)
        {
            *cksumtype = krb5_enctypes_list[i].required_ctype;
            return 0;
        }
    }
    return KRB5_BAD_ENCTYPE;
}

OM_uint32 krb5_gss_accept_sec_context(
    OM_uint32 *minor_status,
    gss_ctx_id_t *context_handle,
    gss_cred_id_t verifier_cred_handle,
    gss_buffer_t input_token,
    gss_channel_bindings_t input_chan_bindings,
    gss_name_t *src_name,
    gss_OID *mech_type,
    gss_buffer_t output_token,
    OM_uint32 *ret_flags,
    OM_uint32 *time_rec,
    gss_cred_id_t *delegated_cred_handle)
{
    krb5_gss_cred_id_t cred = 0;
    //unsigned char *sptr;
    krb5_auth_context auth_context = NULL;
    int md5len;
    int i;
    long tmp;
    int bigend;
    krb5_ticket * ticket = NULL;
    int option_id;
    krb5_data option;
    krb5_checksum reqcksum;
    unsigned char *ptr, *ptr2;
    unsigned char *sptr;
    krb5_ui_4 gss_flags = 0;
    int decode_req_message = 0;
    krb5_gss_ctx_id_rec *ctx = 0;
    krb5_principal name = NULL;
    const gss_OID_desc *mech_used = NULL;
    //krb5_authenticator *authdat = 0;
//	int option_id;
//    krb5_data option;

    //OM_uint32 *time_rec = 0;
    gss_buffer_desc token;
    //gss_cred_id_t *delegated_cred_handle = 0;
    krb5_context context;
    krb5_data ap_rep, ap_req;
    OM_uint32 major_status = GSS_S_FAILURE;
    //krb5_gss_cred_id_t deleg_cred = NULL;
    krb5_error_code code;
    krb5_address addr, *paddr;
    int cred_rcache = 0;
    krb5_timestamp now;
    gss_cred_id_t cred_handle = NULL;
    //krb5_gss_cred_id_t deleg_cred = NULL;
//	krb5int_access kaccess;

    //initialize by hand for test


    code = krb5_gss_init_context(&context);
    if (code)
    {
        *minor_status = code;
        return GSS_S_FAILURE;
    }

    /* set up returns to be freeable */
    if (src_name)
        *src_name = (gss_name_t) NULL;
    output_token->length = 0;
    output_token->value = NULL;
    token.value = 0;
    reqcksum.contents = 0;
    ap_req.data = 0;
    ap_rep.data = 0;

    if (mech_type)
        *mech_type = GSS_C_NULL_OID;

    /* return a bogus cred handle */
    if (delegated_cred_handle)
        *delegated_cred_handle = GSS_C_NO_CREDENTIAL;

    /*
     * Context handle must be unspecified.  Actually, it must be
     * non-established, but currently, accept_sec_context never returns
     * a non-established context handle.
     */
    /*SUPPRESS 29*/
    if (*context_handle != GSS_C_NO_CONTEXT)
    {
        *minor_status = 0;
        krb5_free_context(context);
        return(GSS_S_FAILURE);
    }
    /* handle default cred handle */
    if (verifier_cred_handle == GSS_C_NO_CREDENTIAL)
    {

        major_status = krb5_gss_acquire_cred(minor_status, GSS_C_NO_NAME,
                                             GSS_C_INDEFINITE, GSS_C_NO_OID_SET,
                                             GSS_C_ACCEPT, &cred_handle,
                                             NULL, NULL);


        if (major_status != GSS_S_COMPLETE)
        {
            code = *minor_status;
            goto fail;
        }
    }
    else
    {
        //   major_status = krb5_gss_validate_cred(minor_status,
        //	     verifier_cred_handle);
        //major_status = 0;
        //   if (GSS_ERROR(major_status))
        //{
        // code = *minor_status;
        // goto fail;
        //   }
        cred_handle = verifier_cred_handle;
    }
    cred = (krb5_gss_cred_id_t) cred_handle;
    /* make sure the supplied credentials are valid for accept */

    if ((cred->usage != GSS_C_ACCEPT) &&
            (cred->usage != GSS_C_BOTH)) {
        code = 0;
        major_status = GSS_S_NO_CRED;
        goto fail;
    }
    /* verify the token's integrity, and leave the token in ap_req.
       figure out which mech oid was used, and save it */

    ptr = (unsigned char *) input_token->value;

    if (!(code = g_verify_token_header(gss_mech_krb5, (unsigned int*) &(ap_req.length),
                                       &ptr, KG_TOK_CTX_AP_REQ, (unsigned int)input_token->length, 1)))
    {
        mech_used = gss_mech_krb5;
    }
    else
    {
        major_status = GSS_S_DEFECTIVE_TOKEN;
        goto fail;
    }
    sptr = ptr;
    TREAD_STR(sptr, ap_req.data, ap_req.length);
    decode_req_message = 1;

    if ((input_chan_bindings != GSS_C_NO_CHANNEL_BINDINGS) &&
            (input_chan_bindings->initiator_addrtype == GSS_C_AF_INET)) {
        addr.addrtype = ADDRTYPE_INET;
        addr.length = input_chan_bindings->initiator_address.length;
        addr.contents = input_chan_bindings->initiator_address.value;
        paddr = &addr;
    } else {
        paddr = NULL;
    }
    /* decode the AP_REQ message */

    /* decode the message */

    if ((code = krb5_auth_con_init(context, &auth_context))) {
        major_status = GSS_S_FAILURE;
        goto fail;
    }
    if ((code = krb5_auth_con_setaddrs(context, auth_context, NULL, paddr))) {
        major_status = GSS_S_FAILURE;
        goto fail;
    }

    if ((code = krb5_rd_req(context, &auth_context, &ap_req, cred->princ,
                            cred->keytab, NULL, &ticket))) {
        major_status = GSS_S_FAILURE;
        goto fail;
    }

    krb5_auth_con_setflags(context, auth_context, KRB5_AUTH_CONTEXT_DO_SEQUENCE);

    //krb5_auth_con_getauthenticator(context, auth_context, &authdat);

    /* stash this now, for later. */
    code = krb5_c_checksum_length(context, CKSUMTYPE_RSA_MD5, &md5len);
    if (code)
    {
        major_status = GSS_S_FAILURE;
        goto fail;
    }

    /* verify that the checksum is correct */

    /*
    The checksum may be either exactly 24 bytes, in which case
    no options are specified, or greater than 24 bytes, in which case
    one or more options are specified. Currently, the only valid
    option is KRB5_GSS_FOR_CREDS_OPTION ( = 1 ).
    */

    if ((auth_context->authentp->checksum->checksum_type != CKSUMTYPE_KG_CB) ||
            (auth_context->authentp->checksum->length < 24))
    {
        code = 0;
        major_status = GSS_S_BAD_BINDINGS;
        goto fail;
    }

    /*
    	 "Be liberal in what you accept, and
    	 conservative in what you send"
    	 -- rfc1123

    	 This code will let this acceptor interoperate with an initiator
    	 using little-endian or big-endian integer encoding.
    */

    ptr = (unsigned char *) auth_context->authentp->checksum->contents;
    bigend = 0;

    TREAD_INT(ptr, tmp, bigend);

    if (tmp != md5len) {
        ptr = (unsigned char *) auth_context->authentp->checksum->contents;
        bigend = 1;

        TREAD_INT(ptr, tmp, bigend);

        if (tmp != md5len) {
            code = KG_BAD_LENGTH;
            major_status = GSS_S_FAILURE;
            goto fail;
        }
    }

    /* at this point, bigend is set according to the initiator's
    	  byte order */


    /*
       The following section of code attempts to implement the
       optional channel binding facility as described in RFC2743.

       Since this facility is optional channel binding may or may
       not have been provided by either the client or the server.

       If the server has specified input_chan_bindings equal to
       GSS_C_NO_CHANNEL_BINDINGS then we skip the check.  If
       the server does provide channel bindings then we compute
       a checksum and compare against those provided by the
       client.         */

    if ((code = kg_checksum_channel_bindings(context,
                input_chan_bindings,
                &reqcksum, bigend))) {
        major_status = GSS_S_BAD_BINDINGS;
        goto fail;
    }

    /* Always read the clients bindings - eventhough we might ignore them */
    TREAD_STR(ptr, ptr2, reqcksum.length);

    if (input_chan_bindings != GSS_C_NO_CHANNEL_BINDINGS ) {
        if (memcmp(ptr2, reqcksum.contents, reqcksum.length) != 0) {
            free(reqcksum.contents);
            reqcksum.contents = 0;
            code = 0;
            major_status = GSS_S_BAD_BINDINGS;
            goto fail;
        }

    }

    free(reqcksum.contents);
    reqcksum.contents = 0;

    TREAD_INT(ptr, gss_flags, bigend);
#if 0
    gss_flags &= ~GSS_C_DELEG_FLAG; /* mask out the delegation flag; if
					  there's a delegation, we'll set
					  it below */
#endif
    decode_req_message = 0;

    if (auth_context->authentp->checksum->length > 24 && (gss_flags & GSS_C_DELEG_FLAG))
    {
        i = auth_context->authentp->checksum->length - 24;
        if (i >= 4)
        {
            TREAD_INT16(ptr, option_id, bigend);
            TREAD_INT16(ptr, option.length, bigend);
            i -= 4;
            if (i < option.length || option.length < 0)
            {
                code = KG_BAD_LENGTH;
                major_status = GSS_S_FAILURE;
                goto fail;
            }
            TREAD_STR(ptr, ptr2, option.length);
            option.data = (unsigned char *) ptr2;
            i -= option.length;
            if (option_id != KRB5_GSS_FOR_CREDS_OPTION)
            {
                major_status = GSS_S_FAILURE;
                goto fail;
            }
        } /* if i >= 4 */
    }
    /* create the ctx struct and start filling it in */

    if ((ctx = (krb5_gss_ctx_id_rec *) malloc(sizeof(krb5_gss_ctx_id_rec))) == NULL)
    {
        code = -1;
        major_status = GSS_S_FAILURE;
        goto fail;
    }

    memset(ctx, 0, sizeof(krb5_gss_ctx_id_rec));
    ctx->mech_used = (gss_OID) mech_used;
    ctx->auth_context = auth_context;
    ctx->initiate = 0;
    ctx->gss_flags = (GSS_C_TRANS_FLAG |
                      ((gss_flags) & (GSS_C_INTEG_FLAG | GSS_C_CONF_FLAG |
                                      GSS_C_MUTUAL_FLAG | GSS_C_REPLAY_FLAG |
                                      GSS_C_SEQUENCE_FLAG | GSS_C_DELEG_FLAG)));
    ctx->seed_init = 0;
    ctx->big_endian = bigend;
    ctx->cred_rcache = cred_rcache;

    /* Intern the ctx pointer so that delete_sec_context works */
    //if (! kg_save_ctx_id((gss_ctx_id_t) ctx)) {
    //    xfree(ctx);
    //    ctx = 0;

    //    code = G_VALIDATE_FAILED;
    //    major_status = GSS_S_FAILURE;
    //    goto fail;
    //}
    ctx->ticket = ticket;
    //if ((code = krb5_copy_principal(context, ticket->server, &ctx->here))) {
    //    major_status = GSS_S_FAILURE;
    //    goto fail;
    //}
    ctx->here = ticket->server;

    ctx->there = auth_context->authentp->client;
    //if ((code = krb5_copy_principal(context, authdat->client, &ctx->there))) {
    //    major_status = GSS_S_FAILURE;
    //    goto fail;
    //}

    ctx->subkey = auth_context->recv_subkey;
    //if ((code = krb5_auth_con_getrecvsubkey(context, auth_context,
    //		   &ctx->subkey))) {
    //    major_status = GSS_S_FAILURE;
    //    goto fail;
    //}

    /* use the session key if the subkey isn't present */

    //if (ctx->subkey == NULL) {
    //    if ((code = krb5_auth_con_getkey(context, auth_context,
    //		&ctx->subkey))) {
    // major_status = GSS_S_FAILURE;
    // goto fail;
    //    }
    //}

    //if (ctx->subkey == NULL) {
    //    /* this isn't a very good error, but it's not clear to me this
    //can actually happen */
    //    major_status = GSS_S_FAILURE;
    //    code = KRB5KDC_ERR_NULL_KEY;
    //    goto fail;
    //}

    ctx->proto = 0;
    switch (ctx->subkey->enctype)
    {
    case ENCTYPE_DES_CBC_MD5:
    case ENCTYPE_DES_CBC_CRC:
        ctx->subkey->enctype = ENCTYPE_DES_CBC_RAW;
        ctx->signalg = SGN_ALG_DES_MAC_MD5;
        ctx->cksum_size = 8;
        ctx->sealalg = SEAL_ALG_DES;

        /* fill in the encryption descriptors */

        if ((code = krb5_copy_keyblock(context, ctx->subkey, &ctx->enc))) {
            major_status = GSS_S_FAILURE;
            goto fail;
        }

        for (i=0; i<ctx->enc->length; i++)
            /*SUPPRESS 113*/
            ctx->enc->contents[i] ^= 0xf0;

        goto copy_subkey_to_seq;

    case ENCTYPE_DES3_CBC_SHA1:
        ctx->subkey->enctype = ENCTYPE_DES3_CBC_RAW;
        ctx->signalg = SGN_ALG_HMAC_SHA1_DES3_KD;
        ctx->cksum_size = 20;
        ctx->sealalg = SEAL_ALG_DES3KD;

        /* fill in the encryption descriptors */
copy_subkey:
        ctx->enc = ctx->subkey;
        //if ((code = krb5_copy_keyblock(context, ctx->subkey, &ctx->enc))) {
        //major_status = GSS_S_FAILURE;
        //goto fail;
        //}
copy_subkey_to_seq:
        ctx->seq = ctx->subkey;
        //if ((code = krb5_copy_keyblock(context, ctx->subkey, &ctx->seq))) {
        //major_status = GSS_S_FAILURE;
        //goto fail;
        //}
        break;

    case ENCTYPE_ARCFOUR_HMAC:
        ctx->signalg = SGN_ALG_HMAC_MD5 ;
        ctx->cksum_size = 8;
        ctx->sealalg = SEAL_ALG_MICROSOFT_RC4 ;
        goto copy_subkey;

    default:
        ctx->signalg = -1;
        ctx->sealalg = -1;
        ctx->proto = 1;
        //code = (*kaccess.krb5int_c_mandatory_cksumtype)(context, ctx->subkey->enctype,
        //  &ctx->cksumtype);
        code = krb5int_c_mandatory_cksumtype(context, ctx->subkey->enctype, &ctx->cksumtype);
        if (code)
            goto fail;
        code = krb5_c_checksum_length(context, ctx->cksumtype,
                                      &ctx->cksum_size);
        if (code)
            goto fail;
        ctx->have_acceptor_subkey = 0;
        goto copy_subkey;
    }

    ctx->endtime = ticket->enc_part2->times.endtime;
    ctx->krb_flags = ticket->enc_part2->flags;

    //krb5_free_ticket(context, ticket); /* Done with ticket */

    {
        krb5_ui_4 seq_temp;
        krb5_auth_con_getremoteseqnumber(context, auth_context, (krb5_int32 *)&seq_temp);
        ctx->seq_recv = seq_temp;
    }

#ifndef TEST_GSSAPI
    if ((code = krb5_timeofday(context, &now))) {
        major_status = GSS_S_FAILURE;
        goto fail;
    }

    if (ctx->endtime < now) {
        code = 0;
        major_status = GSS_S_CREDENTIALS_EXPIRED;
        goto fail;
    }
#endif

    // g_order_init(&(ctx->seqstate), ctx->seq_recv,
    //(ctx->gss_flags & GSS_C_REPLAY_FLAG) != 0,
    //(ctx->gss_flags & GSS_C_SEQUENCE_FLAG) != 0, ctx->proto);

    /* at this point, the entire context structure is filled in,
       so it can be released.  */

    /* generate an AP_REP if necessary */

    if (ctx->gss_flags & GSS_C_MUTUAL_FLAG)
    {
        unsigned char * ptr3;
        krb5_ui_4 seq_temp;
        int cfx_generate_subkey;

        if (ctx->proto == 1)
            cfx_generate_subkey = CFX_ACCEPTOR_SUBKEY;
        else
            cfx_generate_subkey = 0;

        if (cfx_generate_subkey)
        {
            krb5_int32 acflags;
            code = krb5_auth_con_getflags(context, auth_context, &acflags);
            if (code == 0)
            {
                acflags |= KRB5_AUTH_CONTEXT_USE_SUBKEY;
                code = krb5_auth_con_setflags(context, auth_context, acflags);
            }
            if (code)
            {
                major_status = GSS_S_FAILURE;
                goto fail;
            }
        }

        if ((code = krb5_mk_rep(context, auth_context, &ap_rep)))
        {
            major_status = GSS_S_FAILURE;
            goto fail;
        }


        krb5_auth_con_getlocalseqnumber(context, auth_context, (krb5_int32 *)&seq_temp);
        ctx->seq_send = seq_temp & 0xffffffffL;

        if (cfx_generate_subkey)
        {
            // /* Get the new acceptor subkey.  With the code above, there
            // should always be one if we make it to this point.  */
            // code = krb5_auth_con_getsendsubkey(context, auth_context,
            //			  &ctx->acceptor_subkey);
            // if (code != 0) {
            //  major_status = GSS_S_FAILURE;
            //  goto fail;
            // }
            ctx->acceptor_subkey = auth_context->send_subkey;
            // //code = (*kaccess.krb5int_c_mandatory_cksumtype)(context,
            //	//		ctx->acceptor_subkey->enctype,
            code = krb5int_c_mandatory_cksumtype(context, ctx->acceptor_subkey->enctype,
                                                 &ctx->acceptor_subkey_cksumtype);
            if (code)
            {
                major_status = GSS_S_FAILURE;
                goto fail;
            }
            ctx->have_acceptor_subkey = 1;
            //   }
        }
        /* the reply token hasn't been sent yet, but that's ok. */
        ctx->gss_flags |= GSS_C_PROT_READY_FLAG;
        ctx->established = 1;

        token.length = g_token_size(mech_used, ap_rep.length);

        //token.length = ap_rep.length;

        if ((token.value = (unsigned char *) malloc(token.length)) == NULL)
        {
            major_status = GSS_S_FAILURE;
            code = -1;
            goto fail;
        }
        ptr3 = token.value;
        g_make_token_header(mech_used, ap_rep.length, &ptr3, KG_TOK_CTX_AP_REP);

        TWRITE_STR(ptr3, ap_rep.data, ap_rep.length);

        ctx->established = 1;

    }
    else
    {
        token.length = 0;
        token.value = NULL;
        ctx->seq_send = ctx->seq_recv;
        ctx->established = 1;
    }

    /* set the return arguments */

    if (src_name)
    {
        name = ctx->there;
    }

    if (mech_type)
        *mech_type = (gss_OID) mech_used;

    if (time_rec)
        *time_rec = ctx->endtime - now;

    if (ret_flags)
        *ret_flags = ctx->gss_flags;

    *context_handle = (gss_ctx_id_t)ctx;
    *output_token = token;

    if (src_name)
        *src_name = (gss_name_t) name;

    *minor_status = 0;
    major_status = GSS_S_COMPLETE;

fail:
    //if (authdat)
    //    krb5_free_authenticator(context, authdat);

    /* The ctx structure has the handle of the auth_context */
    if (auth_context && !ctx) {
        if (cred_rcache)
            (void)krb5_auth_con_setrcache(context, auth_context, NULL);

        krb5_auth_con_free(context, auth_context);
    }
    if (reqcksum.contents)
        free(reqcksum.contents);
    if (ap_rep.data)
        krb5_free_data_contents(context, &ap_rep);

    if (!GSS_ERROR(major_status) && major_status != GSS_S_CONTINUE_NEEDED) {
        ctx->k5_context = context;
        return(major_status);
    }
    krb5_free_context(context);
    return (major_status);
}

/*
 *  glue routine for get_mech_type
 *
 */

OM_uint32 gssint_get_mech_type( gss_OID	OID,gss_buffer_t token)
{
    unsigned char * buffer_ptr;
    int length;

    /*
     * This routine reads the prefix of "token" in order to determine
     * its mechanism type. It assumes the encoding suggested in
     * Appendix B of RFC 1508. This format starts out as follows :
     *
     * tag for APPLICATION 0, Sequence[constructed, definite length]
     * length of remainder of token
     * tag of OBJECT IDENTIFIER
     * length of mechanism OID
     * encoding of mechanism OID
     * <the rest of the token>
     *
     * Numerically, this looks like :
     *
     * 0x60
     * <length> - could be multiple bytes
     * 0x06
     * <length> - assume only one byte, hence OID length < 127
     * <mech OID bytes>
     *
     * The routine fills in the OID value and returns an error as necessary.
     */

    if (OID == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if ((token == NULL) || (token->value == NULL))
        return (GSS_S_DEFECTIVE_TOKEN);

    /* Skip past the APP/Sequnce byte and the token length */

    buffer_ptr = (unsigned char *) token->value;

    if (*(buffer_ptr++) != 0x60)
        return (GSS_S_DEFECTIVE_TOKEN);
    length = *buffer_ptr++;

    /* check if token length is null */
    if (length == 0)
        return (GSS_S_DEFECTIVE_TOKEN);

    if (length & 0x80)
    {
        if ((length & 0x7f) > 4)
            return (GSS_S_DEFECTIVE_TOKEN);
        buffer_ptr += length & 0x7f;
    }

    if (*(buffer_ptr++) != 0x06)
        return (GSS_S_DEFECTIVE_TOKEN);

    OID->length = (OM_uint32) *(buffer_ptr++);
    OID->elements = (void *) buffer_ptr;
    return (GSS_S_COMPLETE);
}



/*
 * Glue routine for returning the mechanism-specific credential from a
 * external union credential.
 */
gss_cred_id_t gssint_get_mechanism_cred(gss_union_cred_t union_cred, gss_OID mech_type)
{
    int		i;

    if (union_cred == GSS_C_NO_CREDENTIAL)
        return GSS_C_NO_CREDENTIAL;

    for (i=0; i < union_cred->count; i++) {
        if (g_OID_equal(mech_type, &union_cred->mechs_array[i]))
            return union_cred->cred_array[i];
    }
    return GSS_C_NO_CREDENTIAL;
}


static OM_uint32 val_acc_sec_ctx_args(
    OM_uint32 *minor_status,
    gss_ctx_id_t *context_handle,
    gss_cred_id_t verifier_cred_handle,
    gss_buffer_t input_token_buffer,
    gss_channel_bindings_t input_chan_bindings,
    gss_name_t *src_name,
    gss_OID *mech_type,
    gss_buffer_t output_token,
    OM_uint32 *ret_flags,
    OM_uint32 *time_rec,
    gss_cred_id_t *d_cred)
{

    /* Initialize outputs. */

    if (minor_status != NULL)
        *minor_status = 0;

    if (src_name != NULL)
        *src_name = GSS_C_NO_NAME;

    if (mech_type != NULL)
        *mech_type = GSS_C_NO_OID;

    if (output_token != GSS_C_NO_BUFFER) {
        output_token->length = 0;
        output_token->value = NULL;
    }

    if (d_cred != NULL)
        *d_cred = GSS_C_NO_CREDENTIAL;

    /* Validate arguments. */

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (context_handle == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (input_token_buffer == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_READ);

    if (output_token == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    return (GSS_S_COMPLETE);
}


OM_uint32 gss_accept_sec_context(
    OM_uint32 *		minor_status,
    gss_ctx_id_t *		context_handle,
    gss_cred_id_t		server_creds,
    gss_buffer_t		input_token_buffer,
    gss_channel_bindings_t	input_chan_bindings,
    gss_name_t *		src_name,
    gss_OID *		mech_type,
    gss_buffer_t		output_token,
    OM_uint32 *		ret_flags,
    OM_uint32 *		time_rec,
    gss_cred_id_t *		d_cred)
{
    OM_uint32		status, temp_status, temp_minor_status;
    OM_uint32		temp_ret_flags = 0;
    gss_union_ctx_id_t	union_ctx_id;
    gss_union_cred_t	union_cred;
    gss_cred_id_t	input_cred_handle = GSS_C_NO_CREDENTIAL;
    gss_cred_id_t	tmp_d_cred = GSS_C_NO_CREDENTIAL;
    gss_name_t		internal_name = GSS_C_NO_NAME;
    gss_name_t		tmp_src_name = GSS_C_NO_NAME;
    gss_OID_desc	token_mech_type_desc;
    gss_OID		token_mech_type = &token_mech_type_desc;
    gss_mechanism	mech;

    status = val_acc_sec_ctx_args(minor_status,
                                  context_handle,
                                  server_creds,
                                  input_token_buffer,
                                  input_chan_bindings,
                                  src_name,
                                  mech_type,
                                  output_token,
                                  ret_flags,
                                  time_rec,
                                  d_cred);
    if (status != GSS_S_COMPLETE)
        return (status);

    //input_token_buffer->length = sizeof(my_ap_req_spnego);
    //input_token_buffer->value = (void*)my_ap_req_spnego;
    //

    if (*context_handle == GSS_C_NO_CONTEXT)
    {

#define	GSS_EMPTY_BUFFER(buf)	((buf) == NULL ||\
	(buf)->value == NULL || (buf)->length == 0)
        if (GSS_EMPTY_BUFFER(input_token_buffer))
            return (GSS_S_CALL_INACCESSIBLE_READ);

        /* Get the token mech type */
        status = gssint_get_mech_type(token_mech_type, input_token_buffer);
        if (status)
            return status;

        status = GSS_S_FAILURE;
        union_ctx_id = (gss_union_ctx_id_t)malloc(sizeof(gss_union_ctx_id_desc));
        if (!union_ctx_id)
            return (GSS_S_FAILURE);

        /* Get the token mech type */
        union_ctx_id->mech_type = (gss_OID)malloc(sizeof(gss_OID_desc));
        status = gssint_get_mech_type(union_ctx_id->mech_type, input_token_buffer);
        if (status)
            return status;

        union_ctx_id->loopback = union_ctx_id;
        union_ctx_id->internal_ctx_id = GSS_C_NO_CONTEXT;


        //status = generic_gss_copy_oid(&temp_minor_status,
        //				  token_mech_type,
        //				  &union_ctx_id->mech_type);
        if (status != GSS_S_COMPLETE)
        {
            free(union_ctx_id);
            return (status);
        }

        /* set the new context handle to caller's data */
        *context_handle = (gss_ctx_id_t)union_ctx_id;
    }
    else
    {
        union_ctx_id = (gss_union_ctx_id_t)*context_handle;
        token_mech_type = union_ctx_id->mech_type;
    }

    /*
     * get the appropriate cred handle from the union cred struct.
     * defaults to GSS_C_NO_CREDENTIAL if there is no cred, which will
     * use the default credential.
     */
    union_cred = (gss_union_cred_t) server_creds;
    input_cred_handle = gssint_get_mechanism_cred(union_cred, token_mech_type);
    mech = gssint_get_mechanism (token_mech_type);

    //gss_buffer_desc input_token, output_token;


    //output_token->length = 0;
    //output_token->value = (void*)0;



//	*context = GSS_C_NO_CONTEXT;
    //krb5_gss_accept_sec_context(&acc_sec_min_stat, context, server_creds, &input_token, &output_token);
    if (1/*mech && mech->gss_accept_sec_context*/) {
        status = krb5_gss_accept_sec_context(minor_status,
                                             &union_ctx_id->internal_ctx_id, /*0*/
                                             input_cred_handle, /*not null*/
                                             input_token_buffer, /*not null*/
                                             input_chan_bindings, /*0*/
                                             &internal_name, /*0*/
                                             mech_type, /*0*/
                                             output_token,/*0*/
                                             &temp_ret_flags, /*0*/
                                             time_rec, /*0*/
                                             d_cred ? &tmp_d_cred : NULL); /*0*/
        /* If there's more work to do, keep going... */
        if (status == GSS_S_CONTINUE_NEEDED)
            return GSS_S_CONTINUE_NEEDED;

        /* if the call failed, return with failure */
        if (status != GSS_S_COMPLETE)
            goto error_out;
        if (internal_name != NULL)
        {
            temp_status = gssint_convert_name_to_union_name(
                              &temp_minor_status, mech,
                              internal_name, &tmp_src_name);
            if (temp_status != GSS_S_COMPLETE) {
                *minor_status = temp_minor_status;
                if (output_token->length)
                    (void) gss_release_buffer(&temp_minor_status,
                                              output_token);
                if (internal_name != GSS_C_NO_NAME)
                    mech->gss_release_name(
                        mech->context,
                        &temp_minor_status,
                        &internal_name);
                return (temp_status);
            }
            if (src_name != NULL) {
                *src_name = tmp_src_name;
            }
        }
        else if (src_name != NULL) {
            *src_name = GSS_C_NO_NAME;
        }

        if (src_name == NULL && tmp_src_name != NULL)
            (void) gss_release_name(&temp_minor_status, &tmp_src_name);
        if (ret_flags != NULL)
            *ret_flags = temp_ret_flags;
        return	(status);
    }
    else
    {

        status = GSS_S_BAD_MECH;
    }

error_out:
    if (union_ctx_id)
    {
        if (union_ctx_id->mech_type)
        {
            //if (union_ctx_id->mech_type->elements)
            //	free(union_ctx_id->mech_type->elements);
            free(union_ctx_id->mech_type);
            *context_handle = GSS_C_NO_CONTEXT;
        }
        free(union_ctx_id);
    }

    if (output_token->length)
        (void) gss_release_buffer(&temp_minor_status, output_token);

    if (src_name)
        *src_name = GSS_C_NO_NAME;

    if (tmp_src_name != GSS_C_NO_NAME)
        (void) gss_release_buffer(&temp_minor_status, (gss_buffer_t)tmp_src_name);

    return (status);
}



/*
 * Routine to create and copy the gss_buffer_desc structure.
 * Both space for the structure and the data is allocated.
 */
OM_uint32 gssint_create_copy_buffer(const gss_buffer_t srcBuf, gss_buffer_t *destBuf, int addNullChar)
{
    gss_buffer_t aBuf;
    unsigned int len;

    if (destBuf == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    *destBuf = 0;

    aBuf = (gss_buffer_t)malloc(sizeof (gss_buffer_desc));
    if (!aBuf)
        return (GSS_S_FAILURE);

    if (addNullChar)
        len = srcBuf->length + 1;
    else
        len = srcBuf->length;

    if (!(aBuf->value = (void*)malloc(len))) {
        free(aBuf);
        return (GSS_S_FAILURE);
    }


    (void) memcpy(aBuf->value, srcBuf->value, srcBuf->length);
    aBuf->length = srcBuf->length;
    *destBuf = aBuf;

    /* optionally add a NULL character */
    if (addNullChar)
        ((char *)aBuf->value)[aBuf->length] = '\0';

    return (GSS_S_COMPLETE);
} /* ****** gssint_create_copy_buffer  ****** */

#if 0
krb5_error_code krb5_build_principal(krb5_context context, krb5_principal * princ,
                                     unsigned int rlen, const char * realm, const char* sname, char *host)
{
    //va_list ap;
    //krb5_error_code retval;
    krb5_principal pr_ret = (krb5_principal)malloc(sizeof(krb5_principal_data));
    register int /*i,*/ count = 0;
    //register char *next;
    char *tmpdata;
    krb5_data *data;

    if (!pr_ret)
        return -1;

    //va_start(ap, realm);
    //retval = krb5_build_principal_va(context, pr_ret, rlen, realm, ap);


    /* guess at an initial sufficent count of 2 pieces */
    count = 2;

    /* get space for array and realm, and insert realm */
    data = (krb5_data *) malloc(sizeof(krb5_data) * count);
    if (data == 0)
        return -1;
    krb5_princ_set_realm_length(context, pr_ret, rlen);
    //pr_ret->realm.length = rlen;
    tmpdata = (char*)malloc(rlen);
    if (!tmpdata)
    {
        free (data);
        return -1;
    }
    krb5_princ_set_realm_data(context, pr_ret, (unsigned char*)tmpdata);
    memcpy(tmpdata, realm, rlen);

    /* process rest of components */
    data[0].length = strlen(sname);
    data[0].data = sname;
    data[1].length = strlen(host);
    data[1].data = host;

    pr_ret->data = data;
    pr_ret->length = 2;
    pr_ret->type = KRB5_NT_UNKNOWN;
    pr_ret->magic = KV5M_PRINCIPAL;

    //va_end(ap);
    //if (retval == 0)
    *princ = pr_ret;
    return 0;
}
#endif

krb5_error_code krb5_free_host_realm(krb5_context context, char *const *hostlist)
{
    register char * const *cp;

    for (cp = hostlist; *cp; cp++)
        free(*cp);
    krb5_xfree(hostlist);
    return 0;
}

#if 0
krb5_error_code krb5_get_host_realm(krb5_context context, const char *host, char **realmsp)
{
    //char **retrealms;
    char *realm, *cp, *temp_realm;
    krb5_error_code retval = 0;

    cp = host;

    realm = (char *)NULL;
    temp_realm = 0;
    while (cp)
    {
        temp_realm = (char*)malloc(strlen(dpconf->DomainName/*"SCCMTEST.COM"*/)+1);
        strcpy(temp_realm, dpconf->DomainName/*"SCCMTEST.COM"*/);
        if (retval)
            return retval;
        if (temp_realm != (char *)NULL)
            break;	/* Match found */
    }
    //if (temp_realm) {
    //    realm = (char *)malloc(strlen(temp_realm) + 1);
    //    if (!realm) {
    //        return ENOMEM;
    //    }
    //    strcpy(realm, temp_realm);
    //}

//   if (realm == (char *)NULL) {
//       if (!(cp = (char *)malloc(strlen(KRB5_REFERRAL_REALM)+1)))
    //    return ENOMEM;
    //strcpy(cp, KRB5_REFERRAL_REALM);
    //realm = cp;
//   }

//   if (!(retrealms = (char **)calloc(2, sizeof(*retrealms))))
    //{
    //	if (realm != (char *)NULL)
    //		free(realm);
    //	return ENOMEM;
//   }

    //retrealms[0] = realm;
    //retrealms[1] = 0;
//   if(!(retrealms = (char**)malloc(sizeof(*retrealms))))
    //{
    //	return ENOMEM;
    //}
    //retrealms[0] = temp_realm;

    *realmsp = temp_realm;
    return 0;
}

krb5_error_code  krb5_sname_to_principal(krb5_context context, char *hostname, const char *sname, krb5_int32 type,
        krb5_principal *ret_princ)
{
    char *hrealms=0, *realm, *remote_host;
    krb5_error_code retval;
    register char *cp;
    char localname[MAXHOSTNAMELEN];

    if ((type == KRB5_NT_UNKNOWN) || (type == KRB5_NT_SRV_HST)) {

        /* if hostname is NULL, use local hostname */
        if (! hostname)
        {
            strcpy(localname, dpconf->HostName);
            strcpy(localname, ".");
            strcpy(localname, dpconf->DomainName);
            hostname = localname;
        }

        /* if sname is NULL, use "host" */
        if (! sname)
            sname = "host";

        /* copy the hostname into non-volatile storage */

        remote_host = hostname;

        if (type == KRB5_NT_SRV_HST)
            for (cp = remote_host; *cp; cp++)
                if (isupper((unsigned char) (*cp)))
                    *cp = tolower((unsigned char) (*cp));

        if ((retval = krb5_get_host_realm(/*context, */remote_host, &hrealms)))
        {
            //free(remote_host);
            return retval;
        }

        if (!hrealms)
        {
            //free(remote_host);
            krb5_xfree(hrealms);
            return KRB5_ERR_HOST_REALM_UNKNOWN;
        }
        realm = hrealms;

        retval = krb5_build_principal(/*context,*/ ret_princ, strlen(realm), realm, sname, remote_host);

        krb5_princ_type(context, *ret_princ) = type;

#ifdef DEBUG_REFERRALS
        printf("krb5_sname_to_principal returning\n");
        printf("realm: <%s>, sname: <%s>, remote_host: <%s>\n",
               realm,sname,remote_host);
        krb5int_dbgref_dump_principal("krb5_sname_to_principal",*ret_princ);
#endif

        //free(remote_host);

        //krb5_free_host_realm(context, hrealms);
        free(hrealms);
        return retval;
    }
    else {
        return KRB5_SNAME_UNSUPP_NAMETYPE;
    }
}
#endif

krb5_error_code krb5_copy_principal(krb5_context context, krb5_const_principal inprinc, krb5_principal *outprinc)
{
    register krb5_principal tempprinc;
    register int i, nelems;

    tempprinc = (krb5_principal)malloc(sizeof(krb5_principal_data));

    if (tempprinc == 0)
        return -1;

    *tempprinc = *inprinc;

    nelems = (int) krb5_princ_size(context, inprinc);
    tempprinc->data = (krb5_data *)malloc(nelems * sizeof(krb5_data));

    if (tempprinc->data == 0) {
        free((char *)tempprinc);
        return -1;
    }

    for (i = 0; i < nelems; i++) {
        unsigned int len = krb5_princ_component(context, inprinc, i)->length;
        krb5_princ_component(context, tempprinc, i)->length = len;
        if (len) {
            if (((krb5_princ_component(context, tempprinc, i)->data = (unsigned char *)
                    malloc(len)) == 0)) {
                while (--i >= 0)
                    free(krb5_princ_component(context, tempprinc, i)->data);
                free (tempprinc->data);
                free (tempprinc);
                return -1;
            }
            memcpy(krb5_princ_component(context, tempprinc, i)->data,
                   krb5_princ_component(context, inprinc, i)->data, len);
        } else
            krb5_princ_component(context, tempprinc, i)->data = 0;
    }

    tempprinc->realm.data = (unsigned char *)
                            malloc((tempprinc->realm.length = inprinc->realm.length) + 1);
    if (!tempprinc->realm.data)
    {
        for (i = 0; i < nelems; i++)
            free(krb5_princ_component(context, tempprinc, i)->data);

        free(tempprinc->data);
        free(tempprinc);
        return -1;
    }
    memcpy(tempprinc->realm.data, inprinc->realm.data,
           inprinc->realm.length);
    tempprinc->realm.data[tempprinc->realm.length] = 0;

    *outprinc = tempprinc;
    return 0;
}

OM_uint32 krb5_gss_import_name(OM_uint32 *minor_status, gss_buffer_t input_name_buffer, gss_OID input_name_type, gss_name_t *output_name)
{
    krb5_context context;
    krb5_principal princ;
    krb5_error_code code;
    char /**stringrep,*/ *tmp/*,  *cp*/;
    //OM_uint32	length;
#ifndef NO_PASSWORD
    //struct passwd *pw;
#endif

    code = krb5_gss_init_context(&context);
    if (code)
    {
        *minor_status = code;
        return GSS_S_FAILURE;
    }

    /* set up default returns */

    *output_name = NULL;
    *minor_status = 0;

    /* Go find the appropriate string rep to pass into parse_name */

    if ((input_name_type != GSS_C_NULL_OID) &&
            (g_OID_equal(input_name_type, gss_nt_service_name) ||
             g_OID_equal(input_name_type, gss_nt_service_name_v2)))
    {
        char *service, *host;

        if ((tmp = (char *) malloc(input_name_buffer->length + 1)) == NULL)
        {
            *minor_status = -1;
            krb5_free_context(context);
            return(GSS_S_FAILURE);
        }

        memcpy(tmp, input_name_buffer->value, input_name_buffer->length);
        tmp[input_name_buffer->length] = 0;

        service = tmp;
        if ((host = strchr(tmp, '@')))
        {
            *host = '\0';
            host++;
        }

        code = rtk_krb5_sname_to_principal(/*context, */host, service, KRB5_NT_SRV_HST, &princ);

        free(tmp);
    }

    /* at this point, a krb5 function has been called to set princ.  code
       contains the return status */

    if (code)
    {
        *minor_status = (OM_uint32) code;
        krb5_free_context(context);
        return(GSS_S_BAD_NAME);
    }

    krb5_free_context(context);

    /* return it */

    *output_name = (gss_name_t) princ;
    return(GSS_S_COMPLETE);
}

OM_uint32 gssint_import_internal_name (OM_uint32 *minor_status, gss_OID mech_type, gss_union_name_t union_name, gss_name_t *internal_name)
{
    OM_uint32		status;
    gss_mechanism	mech;

    mech = gssint_get_mechanism (mech_type);

    if (mech)
    {
        //if (krb5_gss_import_name)
        status = krb5_gss_import_name (minor_status, union_name->external_name, union_name->name_type, internal_name);
        //else
        //	status = GSS_S_UNAVAILABLE;

        return (status);
    }

    return (GSS_S_BAD_MECH);
}
OM_uint32 krb5_gss_release_name(OM_uint32 *minor_status, gss_name_t *input_name)
{
    krb5_context context;
    krb5_error_code code;
    code = krb5_gss_init_context(&context);
    if (code) {
        *minor_status = code;
        return GSS_S_FAILURE;
    }
    krb5_free_principal(context, (krb5_principal) *input_name, 0);
    krb5_free_context(context);
    *input_name = (gss_name_t) NULL;
    *minor_status = 0;
    return(GSS_S_COMPLETE);
}
static void krb5_free_enc_tkt_part(krb5_context context, krb5_enc_tkt_part *val)
{
    krb5_xfree(val);
}

#if 0
static gss_OID_set create_actual_mechs(const gss_OID mechs_array, int count)
{
    gss_OID_set 	actual_mechs;
    int			i;
    OM_uint32		minor;

    actual_mechs = (gss_OID_set) malloc(sizeof(gss_OID_set_desc));
    if (!actual_mechs)
        return NULL;

    actual_mechs->elements = (gss_OID)
                             malloc(sizeof (gss_OID_desc) * count);
    if (!actual_mechs->elements) {
        free(actual_mechs);
        return NULL;
    }

    actual_mechs->count = 0;

    for (i = 0; i < count; i++) {
        actual_mechs->elements[i].elements = (void *)
                                             malloc(mechs_array[i].length);
        if (actual_mechs->elements[i].elements == NULL) {
            (void) rtk_gss_release_oid_set(&minor, &actual_mechs);
            return (NULL);
        }
        g_OID_copy(&actual_mechs->elements[i], &mechs_array[i]);
        actual_mechs->count++;
    }

    return actual_mechs;
}
#endif

#if 0
static OM_uint32
val_add_cred_args(
    OM_uint32 *minor_status,
    gss_cred_id_t input_cred_handle,
    gss_name_t desired_name,
    gss_OID desired_mech,
    gss_cred_usage_t cred_usage,
    OM_uint32 initiator_time_req,
    OM_uint32 acceptor_time_req,
    gss_cred_id_t *output_cred_handle,
    gss_OID_set *actual_mechs,
    OM_uint32 *initiator_time_rec,
    OM_uint32 *acceptor_time_rec)
{

    /* Initialize outputs. */

    if (minor_status != NULL)
        *minor_status = 0;

    if (output_cred_handle != NULL)
        *output_cred_handle = GSS_C_NO_CREDENTIAL;

    if (actual_mechs != NULL)
        *actual_mechs = GSS_C_NO_OID_SET;

    if (acceptor_time_rec != NULL)
        *acceptor_time_rec = 0;

    if (initiator_time_rec != NULL)
        *initiator_time_rec = 0;

    /* Validate arguments. */

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (input_cred_handle == GSS_C_NO_CREDENTIAL &&
            output_cred_handle == NULL)

        return (GSS_S_CALL_INACCESSIBLE_WRITE | GSS_S_NO_CRED);

    return (GSS_S_COMPLETE);
}
#endif

#if 0

//if (! kg_validate_name(*input_name)) {
//   *minor_status = (OM_uint32) G_VALIDATE_FAILED;
//   krb5_free_context(context);
//   return(GSS_S_CALL_BAD_STRUCTURE|GSS_S_BAD_NAME);
//}

//(void)kg_delete_name(*input_name);




//seesion is freed by auth_context->keyblock
//if (val->session)
//krb5_free_keyblock(context, val->session);
//if (val->client)
//		krb5_free_principal(context, val->client);
// if (val->transited.tr_contents.data)
//krb5_xfree(val->transited.tr_contents.data);
//if (val->caddrs)
//krb5_free_addresses(context, val->caddrs);
//if (val->authorization_data)
//krb5_free_authdata(context, val->authorization_data);

krb5_error_code krb5_c_block_size(krb5_context context, krb5_enctype enctype, int *blocksize)
{
    int i;
    for (i=0; i<krb5_enctypes_length; i++) {
        if (krb5_enctypes_list[i].etype == enctype)
            break;
    }
    if (i == krb5_enctypes_length)
        return(KRB5_BAD_ENCTYPE);

    *blocksize = krb5_enctypes_list[i].enc->block_size;

    return(0);
}


int kg_confounder_size(krb5_context context, krb5_keyblock *key)
{
    krb5_error_code code;
    int blocksize;
    /* We special case rc4*/
    if (key->enctype == ENCTYPE_ARCFOUR_HMAC)
        return 8;
    code = krb5_c_block_size(context, key->enctype, &blocksize);
    if (code)
        return(-1); /* XXX */

    return(blocksize);
}

krb5_error_code krb5_c_decrypt(krb5_context context, const krb5_keyblock *key, krb5_keyusage usage,
                               const krb5_data *ivec, const krb5_enc_data *input, krb5_data *output)
{
    int i;
    for (i=0; i<krb5_enctypes_length; i++) {
        if (krb5_enctypes_list[i].etype == key->enctype)
            break;
    }
    if (i == krb5_enctypes_length)
        return(KRB5_BAD_ENCTYPE);
    if ((input->enctype != ENCTYPE_UNKNOWN) &&
            (krb5_enctypes_list[i].etype != input->enctype))
        return(KRB5_BAD_ENCTYPE);
    return((*(krb5_enctypes_list[i].decrypt))
           (krb5_enctypes_list[i].enc, krb5_enctypes_list[i].hash,
            key, usage, ivec, &input->ciphertext, output, 0));
}
krb5_error_code
kg_decrypt(
    krb5_context context,
    krb5_keyblock *key,
    int usage,
    krb5_pointer iv,
    const krb5_pointer in,
    krb5_pointer out,
    unsigned int length)
{
    krb5_error_code code;
    int blocksize;
    krb5_data ivd, *pivd, outputd;
    krb5_enc_data inputd;
    if (iv) {
        code = krb5_c_block_size(context, key->enctype, &blocksize);
        if (code)
            return(code);
        ivd.length = blocksize;
        ivd.data = malloc(ivd.length);
        if (ivd.data == NULL)
            return -1;
        memcpy(ivd.data, iv, ivd.length);
        pivd = &ivd;
    } else {
        pivd = NULL;
    }
    inputd.enctype = ENCTYPE_UNKNOWN;
    inputd.ciphertext.length = length;
    inputd.ciphertext.data = in;
    outputd.length = length;
    outputd.data = out;

    code = krb5_c_decrypt(context, key, usage, pivd, &inputd, &outputd);
    if (pivd != NULL)
        free(pivd->data);
    return code;
}


krb5_error_code kg_arcfour_docrypt (const krb5_keyblock *longterm_key , int ms_usage,
                                    const unsigned char *kd_data, int kd_data_len,
                                    const unsigned char *input_buf, int input_len, unsigned char *output_buf)
{
    krb5_error_code code;
    krb5_data input, output;
    //krb5int_access kaccess;
    krb5_keyblock seq_enc_key, usage_key;
    unsigned char t[4];

    usage_key.length = longterm_key->length;
    usage_key.contents = malloc(usage_key.length);
    if (usage_key.contents == NULL)
        return (-1);
    seq_enc_key.length = longterm_key->length;
    seq_enc_key.contents = malloc(seq_enc_key.length);
    if (seq_enc_key.contents == NULL) {
        free ((void *) usage_key.contents);
        return (-1);
    }
    //code = krb5int_accessor (&kaccess, KRB5INT_ACCESS_VERSION);
    //if (code)

    t[0] = ms_usage &0xff;
    t[1] = (ms_usage>>8) & 0xff;
    t[2] = (ms_usage>>16) & 0xff;
    t[3] = (ms_usage>>24) & 0xff;
    input.data = (void *) &t;
    input.length = 4;
    output.data = (void *) usage_key.contents;
    output.length = usage_key.length;

    md5_hmac(longterm_key->contents, longterm_key->length, input.data, input.length, output.data);
    //code = (*kaccess.krb5_hmac) (kaccess.md5_hash_provider,
    //	       longterm_key, 1, &input, &output);
    //if (code)
    //  goto cleanup_arcfour;

    input.data = ( void *) kd_data;
    input.length = kd_data_len;
    output.data = (void *) seq_enc_key.contents;
    md5_hmac(usage_key.contents, usage_key.length, input.data, input.length, output.data);
    //code = (*kaccess.krb5_hmac) (kaccess.md5_hash_provider,
    //	       &usage_key, 1, &input, &output);
    //if (code)
    //  goto cleanup_arcfour;
    input.data = ( void * ) input_buf;
    input.length = input_len;
    output.data = (void * ) output_buf;
    output.length = input_len;

    code = k5_arcfour_docrypt(&seq_enc_key, 0, &input, &output);

    //code =  ((*kaccess.arcfour_enc_provider->encrypt)(
    //				    &seq_enc_key, 0,
    //				    &input, &output));
// cleanup_arcfour:
    memset ((void *) seq_enc_key.contents, 0, seq_enc_key.length);
    memset ((void *) usage_key.contents, 0, usage_key.length);
    free ((void *) usage_key.contents);
    free ((void *) seq_enc_key.contents);
    return (code);
}
#endif
#if 0
krb5_error_code kg_get_seq_num(
    krb5_context context,
    krb5_keyblock *key,
    unsigned char *cksum,
    unsigned char *buf,
    int *direction,
    krb5_ui_4 *seqnum)
{
    krb5_error_code code;
    unsigned char plain[8];

    if (key->enctype == ENCTYPE_ARCFOUR_HMAC) {
        code = kg_arcfour_docrypt (key, 0,
                                   cksum, 8,
                                   buf, 8,
                                   plain);
    } else {
        code = kg_decrypt(context, key, KG_USAGE_SEQ, cksum, buf, plain, 8);
    }
    if (code)
        return(code);

    if ((plain[4] != plain[5]) ||
            (plain[4] != plain[6]) ||
            (plain[4] != plain[7]))
        return((krb5_error_code) KG_BAD_SEQ);

    *direction = plain[4];
    if (key->enctype == ENCTYPE_ARCFOUR_HMAC) {
        *seqnum = (plain[3]|(plain[2]<<8) | (plain[1]<<16)| (plain[0]<<24));
    } else {
        *seqnum = ((plain[0]) |
                   (plain[1]<<8) |
                   (plain[2]<<16) |
                   (plain[3]<<24));
    }

    return(0);
}
#endif
#if 0
/* message_buffer is an input if SIGN, output if SEAL, and ignored if DEL_CTX
   conf_state is only valid if SEAL. */

static OM_uint32
kg_unseal_v1(
    krb5_context context,
    OM_uint32 *minor_status,
    krb5_gss_ctx_id_rec *ctx,
    unsigned char *ptr,
    int bodysize,
    gss_buffer_t message_buffer,
    int *conf_state,
    int *qop_state,
    int toktype)
{
    krb5_error_code code;
    int conflen = 0;
    int signalg;
    int sealalg;
    gss_buffer_desc token;
    //krb5_checksum cksum;
    krb5_checksum md5cksum;
    krb5_data plaind;
    char *data_ptr;
    krb5_timestamp now;
    unsigned char *plain;
    unsigned int cksum_len = 0;
    int plainlen;
    int direction;
    krb5_ui_4 seqnum;
    OM_uint32 retval = 0;
    int sumlen;
    krb5_keyusage sign_usage = KG_USAGE_SIGN;

    if (toktype == KG_TOK_SEAL_MSG) {
        message_buffer->length = 0;
        message_buffer->value = NULL;
    }

    /* get the sign and seal algorithms */

    signalg = ptr[0] + (ptr[1]<<8);
    sealalg = ptr[2] + (ptr[3]<<8);

    /* Sanity checks */

    if ((ptr[4] != 0xff) || (ptr[5] != 0xff)) {
        *minor_status = 0;
        return GSS_S_DEFECTIVE_TOKEN;
    }

    if ((toktype != KG_TOK_SEAL_MSG) &&
            (sealalg != 0xffff)) {
        *minor_status = 0;
        return GSS_S_DEFECTIVE_TOKEN;
    }

    /* in the current spec, there is only one valid seal algorithm per
       key type, so a simple comparison is ok */

    if ((toktype == KG_TOK_SEAL_MSG) &&
            !((sealalg == 0xffff) ||
              (sealalg == ctx->sealalg))) {
        *minor_status = 0;
        return GSS_S_DEFECTIVE_TOKEN;
    }

    /* there are several mappings of seal algorithms to sign algorithms,
       but few enough that we can try them all. */

    if ((ctx->sealalg == SEAL_ALG_NONE && signalg > 1) ||
            (ctx->sealalg == SEAL_ALG_1 && signalg != SGN_ALG_3) ||
            (ctx->sealalg == SEAL_ALG_DES3KD &&
             signalg != SGN_ALG_HMAC_SHA1_DES3_KD)||
            (ctx->sealalg == SEAL_ALG_MICROSOFT_RC4 &&
             signalg != SGN_ALG_HMAC_MD5)) {
        *minor_status = 0;
        return GSS_S_DEFECTIVE_TOKEN;
    }

    switch (signalg)
    {
    case SGN_ALG_DES_MAC_MD5:
    case SGN_ALG_MD2_5:
    case SGN_ALG_HMAC_MD5:
        cksum_len = 8;
        if (toktype != KG_TOK_SEAL_MSG)
            sign_usage = 15;
        break;
    case SGN_ALG_3:
        cksum_len = 16;
        break;
    case SGN_ALG_HMAC_SHA1_DES3_KD:
        cksum_len = 20;
        break;
    default:
        *minor_status = 0;
        return GSS_S_DEFECTIVE_TOKEN;
    }

    /* get the token parameters */

    if ((code = kg_get_seq_num(context, ctx->seq, ptr+14, ptr+6, &direction,
                               &seqnum))) {
        *minor_status = code;
        return(GSS_S_BAD_SIG);
    }

    /* decode the message, if SEAL */

    if (toktype == KG_TOK_SEAL_MSG) {
        int tmsglen = bodysize-(14+cksum_len);
        if (sealalg != 0xffff) {
            if ((plain = (unsigned char *) xmalloc(tmsglen)) == NULL) {
                *minor_status = -1;
                return(GSS_S_FAILURE);
            }
            if (ctx->enc->enctype == ENCTYPE_ARCFOUR_HMAC) {
                unsigned char bigend_seqnum[4];
                krb5_keyblock *enc_key;
                int i;
                bigend_seqnum[0] = (seqnum>>24) & 0xff;
                bigend_seqnum[1] = (seqnum>>16) & 0xff;
                bigend_seqnum[2] = (seqnum>>8) & 0xff;
                bigend_seqnum[3] = seqnum & 0xff;
                code = krb5_copy_keyblock (context, ctx->enc, &enc_key);
                if (code)
                {
                    xfree(plain);
                    *minor_status = code;
                    return(GSS_S_FAILURE);
                }

                //assert (enc_key->length == 16);
                for (i = 0; i <= 15; i++)
                    ((char *) enc_key->contents)[i] ^=0xf0;
                code = kg_arcfour_docrypt (enc_key, 0,
                                           &bigend_seqnum[0], 4,
                                           ptr+14+cksum_len, tmsglen,
                                           plain);
                krb5_free_keyblock (context, enc_key);
            } else {
                code = kg_decrypt(context, ctx->enc, KG_USAGE_SEAL, NULL,
                                  ptr+14+cksum_len, plain, tmsglen);
            }
            if (code) {
                xfree(plain);
                *minor_status = code;
                return(GSS_S_FAILURE);
            }
        } else {
            plain = ptr+14+cksum_len;
        }

        plainlen = tmsglen;

        if ((sealalg == 0xffff) && ctx->big_endian) {
            token.length = tmsglen;
        } else {
            conflen = kg_confounder_size(context, ctx->enc);
            token.length = tmsglen - conflen - plain[tmsglen-1];
        }

        if (token.length) {
            if ((token.value = (void *) xmalloc(token.length)) == NULL) {
                if (sealalg != 0xffff)
                    xfree(plain);
                *minor_status = -1;
                return(GSS_S_FAILURE);
            }
            memcpy(token.value, plain+conflen, token.length);
        } else {
            token.value = NULL;
        }
    } else if (toktype == KG_TOK_SIGN_MSG) {
        token = *message_buffer;
        plain = token.value;
        plainlen = token.length;
    } else {
        token.length = 0;
        token.value = NULL;
        plain = token.value;
        plainlen = token.length;
    }

    /* compute the checksum of the message */

    /* initialize the the cksum */
    switch (signalg) {
    case SGN_ALG_DES_MAC_MD5:
    case SGN_ALG_MD2_5:
    case SGN_ALG_DES_MAC:
    case SGN_ALG_3:
        md5cksum.checksum_type = CKSUMTYPE_RSA_MD5;
        break;
    case SGN_ALG_HMAC_MD5:
        md5cksum.checksum_type = CKSUMTYPE_HMAC_MD5_ARCFOUR;
        break;
    case SGN_ALG_HMAC_SHA1_DES3_KD:
        md5cksum.checksum_type = CKSUMTYPE_HMAC_SHA1_DES3;
        break;
    default:
        abort ();
    }

    code = krb5_c_checksum_length(context, md5cksum.checksum_type, &sumlen);
    if (code)
        return(code);
    md5cksum.length = sumlen;

    switch (signalg) {
//   case SGN_ALG_DES_MAC_MD5:
//   case SGN_ALG_3:
        ///* compute the checksum of the message */

        ///* 8 = bytes of token body to be checksummed according to spec */

        //if (! (data_ptr = (void *)
        //       xmalloc(8 + (ctx->big_endian ? token.length : plainlen)))) {
        //    if (sealalg != 0xffff)
        //	xfree(plain);
        //    if (toktype == KG_TOK_SEAL_MSG)
        //	xfree(token.value);
        //    *minor_status = ENOMEM;
        //    return(GSS_S_FAILURE);
        //}

        //(void) memcpy(data_ptr, ptr-2, 8);

        //if (ctx->big_endian)
        //    (void) memcpy(data_ptr+8, token.value, token.length);
        //else
        //    (void) memcpy(data_ptr+8, plain, plainlen);

        //plaind.length = 8 + (ctx->big_endian ? token.length : plainlen);
        //plaind.data = data_ptr;
        //code = krb5_c_make_checksum(context, md5cksum.checksum_type,
        //			    ctx->seq, sign_usage,
        //			    &plaind, &md5cksum);
        //xfree(data_ptr);

        //if (code) {
        //    if (toktype == KG_TOK_SEAL_MSG)
        //	xfree(token.value);
        //    *minor_status = code;
        //    return(GSS_S_FAILURE);
        //}

        //if ((code = kg_encrypt(context, ctx->seq, KG_USAGE_SEAL,
        //		       (g_OID_equal(ctx->mech_used, gss_mech_krb5_old) ?
        //			ctx->seq->contents : NULL),
        //		       md5cksum.contents, md5cksum.contents, 16))) {
        //    krb5_free_checksum_contents(context, &md5cksum);
        //    if (toktype == KG_TOK_SEAL_MSG)
        //	xfree(token.value);
        //    *minor_status = code;
        //    return GSS_S_FAILURE;
        //}

        //if (signalg == 0)
        //    cksum.length = 8;
        //else
        //    cksum.length = 16;
        //cksum.contents = md5cksum.contents + 16 - cksum.length;

        //code = memcmp(cksum.contents, ptr+14, cksum.length);
        //break;

//   case SGN_ALG_MD2_5:
        //if (!ctx->seed_init &&
        //    (code = kg_make_seed(context, ctx->subkey, ctx->seed))) {
        //    krb5_free_checksum_contents(context, &md5cksum);
        //    if (sealalg != 0xffff)
        //	xfree(plain);
        //    if (toktype == KG_TOK_SEAL_MSG)
        //	xfree(token.value);
        //    *minor_status = code;
        //    return GSS_S_FAILURE;
        //}

        //if (! (data_ptr = (void *)
        //       xmalloc(sizeof(ctx->seed) + 8 +
        //	       (ctx->big_endian ? token.length : plainlen)))) {
        //    krb5_free_checksum_contents(context, &md5cksum);
        //    if (sealalg == 0)
        //	xfree(plain);
        //    if (toktype == KG_TOK_SEAL_MSG)
        //	xfree(token.value);
        //    *minor_status = ENOMEM;
        //    return(GSS_S_FAILURE);
        //}
        //(void) memcpy(data_ptr, ptr-2, 8);
        //(void) memcpy(data_ptr+8, ctx->seed, sizeof(ctx->seed));
        //if (ctx->big_endian)
        //    (void) memcpy(data_ptr+8+sizeof(ctx->seed),
        //		  token.value, token.length);
        //else
        //    (void) memcpy(data_ptr+8+sizeof(ctx->seed),
        //		  plain, plainlen);
        //plaind.length = 8 + sizeof(ctx->seed) +
        //    (ctx->big_endian ? token.length : plainlen);
        //plaind.data = data_ptr;
        //krb5_free_checksum_contents(context, &md5cksum);
        //code = krb5_c_make_checksum(context, md5cksum.checksum_type,
        //			    ctx->seq, sign_usage,
        //			    &plaind, &md5cksum);
        //xfree(data_ptr);

        //if (code) {
        //    if (sealalg == 0)
        //	xfree(plain);
        //    if (toktype == KG_TOK_SEAL_MSG)
        //	xfree(token.value);
        //    *minor_status = code;
        //    return(GSS_S_FAILURE);
        //}

        //code = memcmp(md5cksum.contents, ptr+14, 8);
        ///* Falls through to defective-token??  */

//   default:
        //*minor_status = 0;
        //return(GSS_S_DEFECTIVE_TOKEN);

    case SGN_ALG_HMAC_SHA1_DES3_KD:
    case SGN_ALG_HMAC_MD5:
        /* compute the checksum of the message */

        /* 8 = bytes of token body to be checksummed according to spec */

        if (! (data_ptr = (void *)
                          xmalloc(8 + (ctx->big_endian ? token.length : plainlen)))) {
            if (sealalg != 0xffff)
                xfree(plain);
            if (toktype == KG_TOK_SEAL_MSG)
                xfree(token.value);
            *minor_status = -1;
            return(GSS_S_FAILURE);
        }

        (void) memcpy(data_ptr, ptr-2, 8);

        if (ctx->big_endian)
            (void) memcpy(data_ptr+8, token.value, token.length);
        else
            (void) memcpy(data_ptr+8, plain, plainlen);

        plaind.length = 8 + (ctx->big_endian ? token.length : plainlen);
        plaind.data = data_ptr;
        code = krb5_c_make_checksum(context, md5cksum.checksum_type,
                                    ctx->seq, sign_usage,
                                    &plaind, &md5cksum);
        xfree(data_ptr);

        if (code) {
            if (toktype == KG_TOK_SEAL_MSG)
                xfree(token.value);
            *minor_status = code;
            return(GSS_S_FAILURE);
        }

        code = memcmp(md5cksum.contents, ptr+14, cksum_len);
        break;
    }

    krb5_free_checksum_contents(context, &md5cksum);
    if (sealalg != 0xffff)
        xfree(plain);

    /* compare the computed checksum against the transmitted checksum */

    if (code) {
        if (toktype == KG_TOK_SEAL_MSG)
            xfree(token.value);
        *minor_status = 0;
        return(GSS_S_BAD_SIG);
    }


    /* it got through unscathed.  Make sure the context is unexpired */

    if (toktype == KG_TOK_SEAL_MSG)
        *message_buffer = token;

    if (conf_state)
        *conf_state = (sealalg != 0xffff);

    if (qop_state)
        *qop_state = GSS_C_QOP_DEFAULT;

    if ((code = krb5_timeofday(context, &now))) {
        *minor_status = code;
        return(GSS_S_FAILURE);
    }

    if (now > ctx->endtime) {
        *minor_status = 0;
        return(GSS_S_CONTEXT_EXPIRED);
    }

    /* do sequencing checks */

    if ((ctx->initiate && direction != 0xff) ||
            (!ctx->initiate && direction != 0)) {
        if (toktype == KG_TOK_SEAL_MSG) {
            xfree(token.value);
            message_buffer->value = NULL;
            message_buffer->length = 0;
        }
        *minor_status = G_BAD_DIRECTION;
        return(GSS_S_BAD_SIG);
    }

    //retval = g_order_check(&(ctx->seqstate), seqnum);

    /* success or ordering violation */

    *minor_status = 0;
    return(retval);
}

/* message_buffer is an input if SIGN, output if SEAL, and ignored if DEL_CTX
   conf_state is only valid if SEAL. */
OM_uint32
kg_unseal(minor_status, context_handle, input_token_buffer,
          message_buffer, conf_state, qop_state, toktype)
OM_uint32 *minor_status;
gss_ctx_id_t context_handle;
gss_buffer_t input_token_buffer;
gss_buffer_t message_buffer;
int *conf_state;
int *qop_state;
int toktype;
{
    krb5_gss_ctx_id_rec *ctx;
    unsigned char *ptr;
    unsigned int bodysize;
    int err;
    int toktype2;

    /* validate the context handle */
//   if (! kg_validate_ctx_id(context_handle)) {
    //*minor_status = (OM_uint32) G_VALIDATE_FAILED;
    //return(GSS_S_NO_CONTEXT);
//   }

    ctx = (krb5_gss_ctx_id_rec *) context_handle;

    if (! ctx->established) {
        *minor_status = KG_CTX_INCOMPLETE;
        return(GSS_S_NO_CONTEXT);
    }

    /* parse the token, leave the data in message_buffer, setting conf_state */

    /* verify the header */

    ptr = (unsigned char *) input_token_buffer->value;

    if (ctx->proto)
        switch (toktype) {
        case KG_TOK_SIGN_MSG:
            toktype2 = 0x0404;
            break;
        case KG_TOK_SEAL_MSG:
            toktype2 = 0x0504;
            break;
        case KG_TOK_DEL_CTX:
            toktype2 = 0x0405;
            break;
        default:
            toktype2 = toktype;
            break;
        }
    else
        toktype2 = toktype;
    err = g_verify_token_header(ctx->mech_used,
                                &bodysize, &ptr, toktype2,
                                input_token_buffer->length,
                                !ctx->proto);
    if (err) {
        *minor_status = err;
        return GSS_S_DEFECTIVE_TOKEN;
    }

    if (ctx->proto == 0)
        return kg_unseal_v1(ctx->k5_context, minor_status, ctx, ptr, bodysize,
                            message_buffer, conf_state, qop_state,
                            toktype);
//   else
    //return gss_krb5int_unseal_token_v3(&ctx->k5_context, minor_status, ctx,
    //				   ptr, bodysize, message_buffer,
    //				   conf_state, qop_state, toktype);
}
#endif
