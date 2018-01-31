#include <stdlib.h>
#include <string.h>
#include "pac.h"
#include "md5.h"
#include "sha1.h"
#include "hash.h"
#include "krb_aes.h"
#include "kerberos.h"

#define REALM_SEP	'@'
#define	COMPONENT_SEP	'/'
#define QUOTECHAR	'\\'
#define FCOMPNUM	10
#define NT_TIME_EPOCH               11644473600LL
#define K5CLENGTH 5 /* 32 bit net byte order integer + one byte seed */
extern const struct krb5_keytypes krb5_enctypes_list[];

extern const int krb5_enctypes_length;

extern const struct krb5_cksumtypes krb5_cksumtypes_list[];

extern const int krb5_cksumtypes_length;
//extern const unsigned char rc4hmac_pass[16];
extern krb5_error_code krb5_c_checksum_length(krb5_context context, krb5_cksumtype cksumtype, int *length);

krb5_error_code krb5_c_verify_checksum(krb5_context context, const krb5_keyblock *key, krb5_keyusage usage,
                                       const krb5_data *data, const krb5_checksum *cksum, krb5_boolean *valid);

static krb5_error_code k5_pac_locate_buffer(krb5_context context, const krb5_pac pac, krb5_ui_4 type, krb5_data *data);

//
//krb5_error_code
//krb5_parse_name_flags(krb5_context context, const char *name,
//                      int flags, krb5_principal *nprincipal)
//{
//    return k5_parse_name(context, name, flags, nprincipal);
//}

unsigned short load_16_le (const void *cvp)
{
    const unsigned char *p = (const unsigned char *) cvp;
    return (p[0] | (p[1] << 8));
}

unsigned int load_32_le (const void *cvp)
{
    const unsigned char *p = (const unsigned char *) cvp;
    return (p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));

}

unsigned long long load_64_le (const void *cvp)
{
    const unsigned char *p = (const unsigned char *) cvp;
    return ((unsigned long long)load_32_le(p+4) << 32) | load_32_le(p);
}

/*
 * Free a PAC
 */
void krb5_pac_free(krb5_context context, krb5_pac pac)
{
    if (pac != NULL)
    {
        if (pac->data.data != NULL)
        {
            //memset(pac->data.data, 0, pac->data.length);
            //free(pac->data.data);
        }
        if (pac->pac != NULL)
            free(pac->pac);
        memset(pac, 0, sizeof(*pac));
        free(pac);
    }
}

/*
 * Initialize PAC
 */
krb5_error_code krb5_pac_init(krb5_context context, krb5_pac *ppac, unsigned int cbuffers, unsigned char *p)
{
    krb5_pac pac;

    pac = (krb5_pac)malloc(sizeof(*pac));
    if (pac == NULL)
        return -1;

    //pac->pac = (PACTYPE *)malloc(sizeof(PACTYPE));
    pac->pac = (PACTYPE*)malloc(sizeof(PACTYPE) + ((cbuffers - 1) * sizeof(PAC_INFO_BUFFER)));
    if (pac->pac == NULL)
    {
        free(pac);
        return -1;
    }

    pac->pac->cBuffers = 0;
    pac->pac->Version = 0;

    pac->data.length = PACTYPE_LENGTH;
    pac->data.data = p;
    //if (pac->data.data == NULL)
    // krb5_pac_free(context, pac);
    //return ENOMEM;
    // }

    pac->verified = 0;

    *ppac = pac;

    return 0;
}

/* conv UCS-2 to UTF-8, not used */
int krb5int_ucs4_to_utf8(krb5_ucs4 c, char *buf)
{
    int len = 0;
    unsigned char *p = (unsigned char *) buf;

    /* not a valid Unicode character */
    if (c < 0)
        return 0;

    /* Just return length, don't convert */
    if (buf == NULL) {
        if (c < 0x80) return 1;
        else if (c < 0x800) return 2;
        else if (c < 0x10000) return 3;
        else if (c < 0x200000) return 4;
        else if (c < 0x4000000) return 5;
        else return 6;
    }

    if (c < 0x80)
    {
        p[len++] = c;
    }
    else if (c < 0x800)
    {
        p[len++] = 0xc0 | ( c >> 6 );
        p[len++] = 0x80 | ( c & 0x3f );
    }
    else if (c < 0x10000)
    {
        p[len++] = 0xe0 | ( c >> 12 );
        p[len++] = 0x80 | ( (c >> 6) & 0x3f );
        p[len++] = 0x80 | ( c & 0x3f );
    }
    else if (c < 0x200000)
    {
        p[len++] = 0xf0 | ( c >> 18 );
        p[len++] = 0x80 | ( (c >> 12) & 0x3f );
        p[len++] = 0x80 | ( (c >> 6) & 0x3f );
        p[len++] = 0x80 | ( c & 0x3f );
    }
    else if (c < 0x4000000)
    {
        p[len++] = 0xf8 | ( c >> 24 );
        p[len++] = 0x80 | ( (c >> 18) & 0x3f );
        p[len++] = 0x80 | ( (c >> 12) & 0x3f );
        p[len++] = 0x80 | ( (c >> 6) & 0x3f );
        p[len++] = 0x80 | ( c & 0x3f );
    }
    else /* if( c < 0x80000000 ) */
    {
        p[len++] = 0xfc | ( c >> 30 );
        p[len++] = 0x80 | ( (c >> 24) & 0x3f );
        p[len++] = 0x80 | ( (c >> 18) & 0x3f );
        p[len++] = 0x80 | ( (c >> 12) & 0x3f );
        p[len++] = 0x80 | ( (c >> 6) & 0x3f );
        p[len++] = 0x80 | ( c & 0x3f );
    }

    return len;
}


int krb5int_ucs2_to_utf8(krb5_ucs2 c, char *buf)
{
    return krb5int_ucs4_to_utf8((krb5_ucs4)c, buf);
}

/*-----------------------------------------------------------------------------
  Convert a wide char string to a UTF-8 string.
  No more than 'count' bytes will be written to the output buffer.
  Return the # of bytes written to the output buffer, excl null terminator.

  ucs2len is -1 if the UCS-2 string is NUL terminated, otherwise it is the
  length of the UCS-2 string in characters
*/
static int k5_ucs2s_to_utf8s(char *utf8str, const krb5_ucs2 *ucs2str, int count, int ucs2len, int little_endian)
{
    int len = 0;
    int n;
    char *p = utf8str;
    krb5_ucs2 empty = 0, ch;

    if (ucs2str == NULL)        /* Treat input ptr NULL as an empty string */
        ucs2str = &empty;

    if (utf8str == NULL)        /* Just compute size of output, excl null */
    {
        while (ucs2len == -1 ? *ucs2str : --ucs2len >= 0)
        {
            /* Get UTF-8 size of next wide char */
            ch = *ucs2str++;
#ifdef K5_BE
            if (little_endian)
                ch = SWAP16(ch);
#endif

            n = krb5int_ucs2_to_utf8(ch, NULL);
            if (n < 1)
                return -1;
            if (len + n < len)
                return -1; /* overflow */
            len += n;
        }

        return len;
    }

    /* Do the actual conversion. */

    n = 1;                                      /* In case of empty ucs2str */
    while (ucs2len == -1 ? *ucs2str != 0 : --ucs2len >= 0)
    {
        ch = *ucs2str++;
#ifdef K5_BE
        if (little_endian)
            ch = SWAP16(ch);
#endif

        n = krb5int_ucs2_to_utf8(ch, p);

        if (n < 1)
            break;

        p += n;
        count -= n;                     /* Space left in output buffer */
    }

    /* If not enough room for last character, pad remainder with null
       so that return value = original count, indicating buffer full. */
    if (n == 0)
    {
        while (count--)
            *p++ = 0;
    }
    /* Add a null terminator if there's room. */
    else if (count)
        *p = 0;

    if (n == -1)                        /* Conversion encountered invalid wide char. */
        return -1;

    /* Return the number of bytes written to output buffer, excl null. */
    return (p - utf8str);
}


int krb5int_ucs2lecs_to_utf8s(const unsigned char *ucs2les, int ucs2leslen, char **utf8s, int *utf8slen)
{
    int len;

    //if (ucs2leslen > SSIZE_MAX)
    //    return -1;

    len = k5_ucs2s_to_utf8s(NULL, (krb5_ucs2 *)ucs2les, 0, (int)ucs2leslen, 1);
    if (len < 0)
        return -1;

    *utf8s = (char *)malloc((int)len + 1);
    if (*utf8s == NULL)
    {
        return -1;
    }

    len = k5_ucs2s_to_utf8s(*utf8s, (krb5_ucs2 *)ucs2les, (int)len + 1, (int)ucs2leslen, 1);
    if (len < 0)
    {
        free(*utf8s);
        *utf8s = NULL;
        return -1;
    }

    if (utf8slen != NULL)
    {
        *utf8slen = len;
    }

    return 0;
}

static krb5_error_code k5_time_to_seconds_since_1970(unsigned long long ntTime, krb5_timestamp *elapsedSeconds)
{
    krb5_ui_8 abstime;

    ntTime /= 10000000;

    abstime = ntTime > 0 ? ntTime - NT_TIME_EPOCH : -ntTime;

    //if (abstime > KRB5_INT32_MAX)
    //    return -1;

    *elapsedSeconds = abstime;

    return 0;
}

krb5_error_code krb5_get_default_realm(krb5_context context, char **lrealm)
{
    char *realm = 0;
    char *cp;
    //krb5_error_code retval;

    if (!context || (context->magic != KV5M_CONTEXT))
        return KV5M_CONTEXT;

    if (!context->default_realm)
    {
        /*
         * XXX should try to figure out a reasonable default based
         * on the host's DNS domain.
         */
        context->default_realm = 0;
        //if (context->profile != 0) {
        //retval = profile_get_string(context->profile, "libdefaults",
        //                            "default_realm", 0, 0,
        //                            &realm);

        //if (!retval && realm) {
        //    context->default_realm = malloc(strlen(realm) + 1);
        //    if (!context->default_realm) {
        //        profile_release_string(realm);
        //        return ENOMEM;
        //    }
        //    strcpy(context->default_realm, realm);
        //    profile_release_string(realm);
        //}
        //}
#ifndef KRB5_DNS_LOOKUP
        //else
        //    return KRB5_CONFIG_CANTOPEN;
#else /* KRB5_DNS_LOOKUP */
        if (context->default_realm == 0) {
            int use_dns =  _krb5_use_dns_realm(context);
            if ( use_dns ) {
                /*
                * Since this didn't appear in our config file, try looking
                * it up via DNS.  Look for a TXT records of the form:
                *
                * _kerberos.<localhost>
                * _kerberos.<domainname>
                * _kerberos.<searchlist>
                *
                */
                char localhost[MAX_DNS_NAMELEN+1];
                char * p;

                krb5int_get_fq_local_hostname (localhost, sizeof(localhost));

                if ( localhost[0] ) {
                    p = localhost;
                    do {
                        retval = krb5_try_realm_txt_rr("_kerberos", p,
                                                       &context->default_realm);
                        p = strchr(p,'.');
                        if (p)
                            p++;
                    } while (retval && p && p[0]);

                    if (retval)
                        retval = krb5_try_realm_txt_rr("_kerberos", "",
                                                       &context->default_realm);
                } else {
                    retval = krb5_try_realm_txt_rr("_kerberos", "",
                                                   &context->default_realm);
                }
                if (retval) {
                    return(KRB5_CONFIG_NODEFREALM);
                }
            }
        }
#endif /* KRB5_DNS_LOOKUP */
    }

    if (context->default_realm == 0)
        return(KRB5_CONFIG_NODEFREALM);
    if (context->default_realm[0] == 0)
    {
        free (context->default_realm);
        context->default_realm = 0;
        return KRB5_CONFIG_NODEFREALM;
    }

    realm = context->default_realm;

    if (!(*lrealm = cp = malloc((unsigned int) strlen(realm) + 1)))
        return -1;
    strcpy(cp, realm);
    return(0);
}

/*
 * May the fleas of a thousand camels infest the ISO, they who think
 * that arbitrarily large multi-component names are a Good Thing.....
 */
krb5_error_code krb5_parse_name(krb5_context context, const char *name, krb5_principal *nprincipal)
{
    register const char	*cp;
    register char	*q;
    register int    i,c,size;
    int		components = 0;
    const char	*parsed_realm = NULL;
    int		fcompsize[FCOMPNUM];
    unsigned int	realmsize = 0;
    char		*default_realm = NULL;
    int		default_realm_size = 0;
    char		*tmpdata;
    krb5_principal	principal;
    krb5_error_code retval;

    /*
     * Pass 1.  Find out how many components there are to the name,
     * and get string sizes for the first FCOMPNUM components.
     */
    size = 0;
    for (i=0,cp = name; (c = *cp); cp++)
    {
        if (c == QUOTECHAR)
        {
            cp++;
            if (!(c = *cp))
                /*
                 * QUOTECHAR can't be at the last
                 * character of the name!
                 */
                return(KRB5_PARSE_MALFORMED);
            size++;
            continue;
        }
        else if (c == COMPONENT_SEP)
        {
            if (parsed_realm)
                /*
                 * Shouldn't see a component separator
                 * after we've parsed out the realm name!
                 */
                return(KRB5_PARSE_MALFORMED);
            if (i < FCOMPNUM)
            {
                fcompsize[i] = size;
            }
            size = 0;
            i++;
        }
        else if (c == REALM_SEP)
        {
            if (parsed_realm)
                /*
                 * Multiple realm separaters
                 * not allowed; zero-length realms are.
                 */
                return(KRB5_PARSE_MALFORMED);
            parsed_realm = cp+1;
            if (i < FCOMPNUM)
            {
                fcompsize[i] = size;
            }
            size = 0;
        }
        else
            size++;
    }
    if (parsed_realm)
        realmsize = size;
    else if (i < FCOMPNUM)
        fcompsize[i] = size;
    components = i + 1;
    /*
     * Now, we allocate the principal structure and all of its
     * component pieces
     */
    principal = (krb5_principal)malloc(sizeof(krb5_principal_data));
    if (!principal)
    {
        return(-1);
    }
    principal->data = (krb5_data *) malloc(sizeof(krb5_data) * components);
    if (!principal->data)
    {
        free((char *)principal);
        return -1;
    }
    principal->length = components;
    /*
     * If a realm was not found, then use the defualt realm....
     */
    if (!parsed_realm)
    {
        if (!default_realm)
        {
            retval = krb5_get_default_realm(context, &default_realm);
            if (retval)
            {
                krb5_xfree(principal->data);
                krb5_xfree((char *)principal);
                return(retval);
            }
            default_realm_size = strlen(default_realm);
        }
        realmsize = default_realm_size;
    }
    /*
     * Pass 2.  Happens only if there were more than FCOMPNUM
     * component; if this happens, someone should be shot
     * immediately.  Nevertheless, we will attempt to handle said
     * case..... <martyred sigh>
     */
    if (components >= FCOMPNUM)
    {
        size = 0;
        parsed_realm = NULL;
        for (i=0,cp = name; (c = *cp); cp++)
        {
            if (c == QUOTECHAR)
            {
                cp++;
                size++;
            }
            else if (c == COMPONENT_SEP)
            {
                if (krb5_princ_size(context, principal) > i)
                    krb5_princ_component(context, principal, i)->length = size;
                size = 0;
                i++;
            }
            else if (c == REALM_SEP)
            {
                if (krb5_princ_size(context, principal) > i)
                    krb5_princ_component(context, principal, i)->length = size;
                size = 0;
                parsed_realm = cp+1;
            }
            else
                size++;
        }
        if (parsed_realm)
            krb5_princ_realm(context, principal)->length = size;
        else if (krb5_princ_size(context, principal) > i)
            krb5_princ_component(context, principal, i)->length = size;
#if 0
        if (i + 1 != components) {
#if !defined(_WIN32)
            fprintf(stderr,
                    "Programming error in krb5_parse_name!");
#endif
            assert(i + 1 == components);
            abort();

        }
#endif
    }
    else
    {
        /*
         * If there were fewer than FCOMPSIZE components (the
         * usual case), then just copy the sizes to the
         * principal structure
         */
        for (i=0; i < components; i++)
            krb5_princ_component(context, principal, i)->length = fcompsize[i];
    }
    /*
     * Now, we need to allocate the space for the strings themselves.....
     */
    tmpdata = malloc(realmsize+1);
    if (tmpdata == 0)
    {
        krb5_xfree(principal->data);
        krb5_xfree(principal);
        krb5_xfree(default_realm);
        return -1;
    }
    krb5_princ_set_realm_length(context, principal, realmsize);
    krb5_princ_set_realm_data(context, principal, tmpdata);
    for (i=0; i < components; i++)
    {
        char *tmpdata2 = malloc((principal->data + i)->length + 1);
        //malloc(krb5_princ_component(context, principal, i)->length + 1);
        if (!tmpdata2)
        {
            for (i--; i >= 0; i--)
                krb5_xfree(krb5_princ_component(context, principal, i)->data);
            krb5_xfree(krb5_princ_realm(context, principal)->data);
            krb5_xfree(principal->data);
            krb5_xfree(principal);
            krb5_xfree(default_realm);
            return(-1);
        }
        krb5_princ_component(context, principal, i)->data = tmpdata2;
        krb5_princ_component(context, principal, i)->magic = KV5M_DATA;
    }

    /*
     * Pass 3.  Now we go through the string a *third* time, this
     * time filling in the krb5_principal structure which we just
     * allocated.
     */
    q = krb5_princ_component(context, principal, 0)->data;
    for (i=0,cp = name; (c = *cp); cp++)
    {
        if (c == QUOTECHAR)
        {
            cp++;
            switch (c = *cp)
            {
            case 'n':
                *q++ = '\n';
                break;
            case 't':
                *q++ = '\t';
                break;
            case 'b':
                *q++ = '\b';
                break;
            case '0':
                *q++ = '\0';
                break;
            default:
                *q++ = c;
            }
        }
        else if ((c == COMPONENT_SEP) || (c == REALM_SEP))
        {
            i++;
            *q++ = '\0';
            if (c == COMPONENT_SEP)
                q = krb5_princ_component(context, principal, i)->data;
            else
                q = krb5_princ_realm(context, principal)->data;
        }
        else
            *q++ = c;
    }
    *q++ = '\0';
    if (!parsed_realm)
        strcpy((char *)krb5_princ_realm(context, principal)->data, default_realm);
    /*
     * Alright, we're done.  Now stuff a pointer to this monstrosity
     * into the return variable, and let's get out of here.
     */
    krb5_princ_type(context, principal) = KRB5_NT_PRINCIPAL;
    principal->magic = KV5M_PRINCIPAL;
    principal->realm.magic = KV5M_DATA;
    *nprincipal = principal;

    if (default_realm != NULL)
        krb5_xfree(default_realm);
    return(0);
}

#if 0
krb5_error_code krb5_parse_name_flags(krb5_context context, const char *name,
                                      krb5_principal *nprincipal)
{
    return krb5_parse_name(context, name,  nprincipal);
}
#endif

static krb5_boolean realm_compare_flags(krb5_context context, krb5_const_principal princ1, krb5_const_principal princ2, int flags)
{
    const krb5_data *realm1 = krb5_princ_realm(context, princ1);
    const krb5_data *realm2 = krb5_princ_realm(context, princ2);

    if (realm1->length != realm2->length)
        return 0;

    return (flags & KRB5_PRINCIPAL_COMPARE_CASEFOLD) ?
           //(strnicmp(realm1->data, realm2->data, realm2->length) == 0) :
           (memcmp(realm1->data, realm2->data, realm2->length) == 0) :
           (memcmp(realm1->data, realm2->data, realm2->length) == 0);
}

/* Some data comparison and conversion functions.  */
static int data_eq(krb5_data d1, krb5_data d2)
{
    return (d1.length == d2.length && !memcmp(d1.data, d2.data, d1.length));
}

krb5_boolean krb5_principal_compare_flags(krb5_context context, krb5_const_principal princ1, krb5_const_principal princ2, int flags)
{
    register int i;
    krb5_int32 nelem;
    //unsigned int utf8 = (flags & KRB5_PRINCIPAL_COMPARE_UTF8) != 0;
    //unsigned int casefold = (flags & KRB5_PRINCIPAL_COMPARE_CASEFOLD) != 0;
    krb5_principal upn1 = NULL;
    krb5_principal upn2 = NULL;
    krb5_boolean ret = 0;

    //if (flags & KRB5_PRINCIPAL_COMPARE_ENTERPRISE) {
    /* Treat UPNs as if they were real principals */
    //if (krb5_princ_type(context, princ1) == KRB5_NT_ENTERPRISE_PRINCIPAL) {
    //    if (upn_to_principal(context, princ1, &upn1) == 0)
    //        princ1 = upn1;
    //}
    //if (krb5_princ_type(context, princ2) == KRB5_NT_ENTERPRISE_PRINCIPAL) {
    //    if (upn_to_principal(context, princ2, &upn2) == 0)
    //        princ2 = upn2;
    //}
    //}

    nelem = krb5_princ_size(context, princ1);
    if (nelem != krb5_princ_size(context, princ2))
        goto out;

    if ((flags & KRB5_PRINCIPAL_COMPARE_IGNORE_REALM) == 0 && !realm_compare_flags(context, princ1, princ2, flags))
        goto out;

    for (i = 0; i < (int) nelem; i++)
    {
        const krb5_data *p1 = krb5_princ_component(context, princ1, i);
        const krb5_data *p2 = krb5_princ_component(context, princ2, i);
        krb5_boolean eq;

        //if (casefold) {
        //if (utf8)
        //    eq = (krb5int_utf8_normcmp(p1, p2, KRB5_UTF8_CASEFOLD) == 0);
        //else
        //    eq = (p1->length == p2->length
        //          && strncasecmp(p1->data, p2->data, p2->length) == 0);
        //} else
        eq = data_eq(*p1, *p2);

        if (!eq)
            goto out;
    }

    ret = 1;

out:
    if (upn1 != NULL)
        krb5_free_principal(context, upn1, 0);
    if (upn2 != NULL)
        krb5_free_principal(context, upn2, 0);

    return ret;
}

static krb5_error_code k5_pac_validate_client(krb5_context context, const krb5_pac pac, krb5_timestamp authtime, krb5_const_principal principal)
{

    krb5_error_code ret;
    krb5_data client_info;
    char *pac_princname;
    unsigned char *p;
    krb5_timestamp pac_authtime;
    krb5_ui_2 pac_princname_length;
    unsigned long long pac_nt_authtime;
    krb5_principal pac_principal;

    ret = k5_pac_locate_buffer(context, pac, PAC_CLIENT_INFO, &client_info);
    if (ret != 0)
        return ret;

    if (client_info.length < PAC_CLIENT_INFO_LENGTH)
        return -1;

    p = (unsigned char *)client_info.data;
    pac_nt_authtime = load_64_le(p);
    p += 8;
    pac_princname_length = load_16_le(p);
    p += 2;

    ret = k5_time_to_seconds_since_1970(pac_nt_authtime, &pac_authtime);
    if (ret != 0)
        return ret;

    if (client_info.length < PAC_CLIENT_INFO_LENGTH + pac_princname_length ||
            pac_princname_length % 2)
        return -1;

    ret = krb5int_ucs2lecs_to_utf8s(p, (int)pac_princname_length / 2, &pac_princname, NULL);
    if (ret != 0)
        return ret;

    ret = krb5_parse_name(context, pac_princname, &pac_principal);
    if (ret != 0)
    {
        free(pac_princname);
        return ret;
    }

    free(pac_princname);

    if (pac_authtime != authtime || !krb5_principal_compare_flags(context, pac_principal,principal, KRB5_PRINCIPAL_COMPARE_IGNORE_REALM))
    {
        ret = KRB5KRB_AP_WRONG_PRINC;
    }

    krb5_free_principal(context, pac_principal, 1);

    return ret;
}

static krb5_error_code k5_pac_locate_buffer(krb5_context context, const krb5_pac pac, krb5_ui_4 type, krb5_data *data)
{
    PAC_INFO_BUFFER *buffer = NULL;
    unsigned int i;

    if (pac == NULL)
        return -1;

    for (i = 0; i < pac->pac->cBuffers; i++) {
        if (pac->pac->Buffers[i].ulType == type) {
            if (buffer == NULL)
                buffer = &pac->pac->Buffers[i];
            else
                return -1;
        }
    }

    if (buffer == NULL)
        return -1;

    //assert(buffer->Offset + buffer->cbBufferSize <= pac->data.length);

    if (data != NULL) {
        data->length = buffer->cbBufferSize;
        data->data = pac->data.data + buffer->Offset;
    }

    return 0;
}

static krb5_error_code k5_pac_verify_kdc_checksum(krb5_context context, const krb5_pac pac, const krb5_keyblock *privsvr)
{
    krb5_error_code ret;
    krb5_data server_checksum, privsvr_checksum;
    krb5_checksum checksum;
    krb5_boolean valid;
    krb5_octet *p;

    ret = k5_pac_locate_buffer(context, pac,
                               PAC_PRIVSVR_CHECKSUM, &privsvr_checksum);
    if (ret != 0)
        return ret;

    if (privsvr_checksum.length < PAC_SIGNATURE_DATA_LENGTH)
        return KRB5_BAD_MSIZE;

    ret = k5_pac_locate_buffer(context, pac,
                               PAC_SERVER_CHECKSUM, &server_checksum);
    if (ret != 0)
        return ret;

    if (server_checksum.length < PAC_SIGNATURE_DATA_LENGTH)
        return KRB5_BAD_MSIZE;

    p = (krb5_octet *)privsvr_checksum.data;
    checksum.checksum_type = load_32_le(p);
    checksum.length = privsvr_checksum.length - PAC_SIGNATURE_DATA_LENGTH;
    checksum.contents = p + PAC_SIGNATURE_DATA_LENGTH;

    server_checksum.data += PAC_SIGNATURE_DATA_LENGTH;
    server_checksum.length -= PAC_SIGNATURE_DATA_LENGTH;

    ret = krb5_c_verify_checksum(context, privsvr, KRB5_KEYUSAGE_APP_DATA_CKSUM, &server_checksum, &checksum, &valid);
    if (ret != 0)
        return ret;

    if (valid == 0)
        ret = KRB5KRB_AP_ERR_BAD_INTEGRITY;

    return ret;
}

static krb5_error_code k5_pac_zero_signature(krb5_context context, const krb5_pac pac, krb5_ui_4 type, krb5_data *data)
{
    PAC_INFO_BUFFER *buffer = NULL;
    unsigned int i;

    //assert(type == PAC_SERVER_CHECKSUM || type == PAC_PRIVSVR_CHECKSUM);
    //assert(data->length >= pac->data.length);

    for (i = 0; i < pac->pac->cBuffers; i++) {
        if (pac->pac->Buffers[i].ulType == type)
        {
            buffer = &pac->pac->Buffers[i];
            break;
        }
    }

    if (buffer == NULL)
        return -1;

    if (buffer->Offset + buffer->cbBufferSize > pac->data.length)
        return -1;

    if (buffer->cbBufferSize < PAC_SIGNATURE_DATA_LENGTH)
        return KRB5_BAD_MSIZE;

    /* Zero out the data portion of the checksum only */
    memset(data->data + buffer->Offset + PAC_SIGNATURE_DATA_LENGTH,
           0,
           buffer->cbBufferSize - PAC_SIGNATURE_DATA_LENGTH);

    return 0;
}

krb5_error_code krb5int_c_copy_keyblock_contents(krb5_context context, const krb5_keyblock *from, krb5_keyblock *to)
{
    *to = *from;
    if (to->length)
    {
        to->contents = malloc(to->length);
        if (!to->contents)
            return -1;
        memcpy(to->contents, from->contents, to->length);
    }
    else
        to->contents = 0;
    return 0;
}

/* Free the memory used by a krb5_key. */
void krb5_k_free_key(krb5_context context, krb5_key key)
{
    struct derived_key *dk;
    //const struct krb5_keytypes *ktp;

    if (key == NULL || --key->refcount > 0)
        return;

    /* Free the derived key cache. */
    while ((dk = key->derived) != NULL)
    {
        key->derived = dk->next;
        free(dk->constant.data);
        krb5_k_free_key(context, dk->dkey);
        free(dk);
    }
    krb5int_c_free_keyblock_contents(context, &key->keyblock);
    //if (key->cache) {
    //    ktp = find_enctype(key->keyblock.enctype);
    //    if (ktp && ktp->enc->key_cleanup)
    //        ktp->enc->key_cleanup(key);
    //}
    free(key);
}

#if 0
/* Create a krb5_key from the enctype and key data in a keyblock. */
krb5_error_code krb5_k_create_key(krb5_context context, const krb5_keyblock *key_data, krb5_key *out)
{
    krb5_key key = NULL;
    krb5_error_code code;

    *out = NULL;

    key = malloc(sizeof(*key));
    if (key == NULL)
        return -1;
    code = krb5int_c_copy_keyblock_contents(context, key_data, &key->keyblock);
    if (code)
        goto cleanup;

    key->refcount = 1;
    key->derived = NULL;
    key->cache = NULL;
    *out = key;
    return 0;

cleanup:
    free(key);
    return code;
}


static const struct krb5_cksumtypes *find_cksumtype(krb5_cksumtype ctype)
{
    int i;

    for (i = 0; i < krb5_cksumtypes_length; i++) {
        if (krb5_cksumtypes_list[i].ctype == ctype)
            break;
    }

    if (i == krb5_cksumtypes_length)
        return NULL;
    return &krb5_cksumtypes_list[i];
}

static const struct krb5_keytypes * find_enctype(krb5_enctype enctype)
{
    int i;

    for (i = 0; i < krb5_enctypes_length; i++) {
        if (krb5_enctypes_list[i].etype == enctype)
            break;
    }

    if (i == krb5_enctypes_length)
        return NULL;
    return &krb5_enctypes_list[i];
}
#endif

krb5_error_code krb5_dk_make_checksum(const struct krb5_hash_provider *hash, const krb5_keyblock *key, krb5_keyusage usage,
                                      const krb5_data *input, krb5_data *output)
{
    int i;
    const struct krb5_enc_provider *enc;
    int blocksize, keybytes, keylength;
    krb5_error_code ret;
    unsigned char constantdata[K5CLENGTH];
    krb5_data datain;
    unsigned char *kcdata;
    krb5_keyblock kc;

#if 0
    if (key->enctype == ENCTYPE_ARCFOUR_HMAC)
    {
        enc = &krb5int_enc_arcfour;

    }
    else if (key->enctype == ENCTYPE_AES128_CTS_HMAC_SHA1_96)
    {
        enc = &krb5int_enc_aes128;
    }
#endif
#if 1
    for (i=0; i<krb5_enctypes_length; i++)
    {
        if (krb5_enctypes_list[i].etype == key->enctype)
            break;
    }

    if (i == krb5_enctypes_length)
        return(KRB5_BAD_ENCTYPE);

    enc = krb5_enctypes_list[i].enc;
#endif

    /* allocate and set to-be-derived keys */

    blocksize = enc->block_size;
    keybytes = enc->keybytes;
    keylength = enc->keylength;

    /* key->length will be tested in enc->encrypt
       output->length will be tested in krb5_hmac */

    if ((kcdata = (unsigned char *) malloc(keylength)) == NULL)
        return(-1);

    kc.contents = kcdata;
    kc.length = keylength;

    /* derive the key */

    datain.data = (char *) constantdata;
    datain.length = K5CLENGTH;

    datain.data[0] = (usage>>24)&0xff;
    datain.data[1] = (usage>>16)&0xff;
    datain.data[2] = (usage>>8)&0xff;
    datain.data[3] = usage&0xff;

    datain.data[4] = (char) 0x99;

    if (key->enctype == ENCTYPE_AES128_CTS_HMAC_SHA1_96)
    {
        aes_context aes_ctx;
        aes_setkey_enc_sw( &aes_ctx, (unsigned char *) (key->contents), 16*8 );
        if ((ret = krb5_derive_key_aes(&aes_ctx, &kc, &datain)) != 0)
            goto cleanup;
    }
    else/*if (key->enctype == ENCTYPE_ARCFOUR_HMAC)*/
    {
        if ((ret = krb5_derive_key(enc, key, &kc, &datain)) != 0)
            goto cleanup;
    }


    /* hash the data */

    datain = *input;

    //if(key->enctype == rc4_hmac)
    {
        //md5_hmac(kc.contents, kc.length, datain.data, datain.length, output->data);

        if ((ret = krb5_hmac(hash, &kc, 1, &datain, output)) != 0)
            memset(output->data, 0, output->length);
    }


    /* ret is set correctly by the prior call */

cleanup:
    memset(kcdata, 0, keylength);

    free(kcdata);

    return(ret);
}


krb5_error_code krb5_c_make_checksum(krb5_context context, krb5_cksumtype cksumtype, const krb5_keyblock *key, krb5_keyusage usage,
                                     const krb5_data *input, krb5_checksum *cksum)
{
    int i, e1, e2;
    krb5_data data;
    krb5_error_code ret;
    int cksumlen;

    for (i=0; i<krb5_cksumtypes_length; i++)
    {
        if (krb5_cksumtypes_list[i].ctype == cksumtype)
            break;
    }

    if (i == krb5_cksumtypes_length)
        return(KRB5_BAD_ENCTYPE);

    if (krb5_cksumtypes_list[i].keyhash)
        cksumlen = krb5_cksumtypes_list[i].keyhash->hashsize;
    else
        cksumlen = krb5_cksumtypes_list[i].hash->hashsize;

    cksum->length = cksumlen;

    if ((cksum->contents = (krb5_octet *) malloc(cksum->length)) == NULL)
        return(-1);

    data.length = cksum->length;
    data.data = (char *) cksum->contents;

    if (krb5_cksumtypes_list[i].keyhash)
    {
        /* check if key is compatible */

        if (krb5_cksumtypes_list[i].keyed_etype)
        {
            for (e1=0; e1<krb5_enctypes_length; e1++)
                if (krb5_enctypes_list[e1].etype == krb5_cksumtypes_list[i].keyed_etype)
                    break;

            for (e2=0; e2<krb5_enctypes_length; e2++)
                if (krb5_enctypes_list[e2].etype == key->enctype)
                    break;

            if ((e1 == krb5_enctypes_length) || (e2 == krb5_enctypes_length) || (krb5_enctypes_list[e1].enc != krb5_enctypes_list[e2].enc))
            {
                ret = KRB5_BAD_ENCTYPE;
                goto cleanup;
            }
        }
        ret = (*(krb5_cksumtypes_list[i].keyhash->hash))(key, usage, 0, input, &data);
    }
    else if (krb5_cksumtypes_list[i].flags & KRB5_CKSUMFLAG_DERIVE)
    {
        ret = krb5_dk_make_checksum(krb5_cksumtypes_list[i].hash, key, usage, input, &data);
    }
    else
    {
        /* no key is used */

        ret = (*(krb5_cksumtypes_list[i].hash->hash))(1, input, &data);
    }

    if (!ret)
    {
        cksum->magic = KV5M_CHECKSUM;
        cksum->checksum_type = cksumtype;
        if (krb5_cksumtypes_list[i].trunc_size)
        {
            krb5_octet *trunc;
            cksum->length = krb5_cksumtypes_list[i].trunc_size;
            trunc = (krb5_octet *) realloc(cksum->contents, cksum->length);
            if (trunc)
                cksum->contents = trunc;
        }
    }

cleanup:
    if (ret)
    {
        memset(cksum->contents, 0, cksum->length);
        free(cksum->contents);
        cksum->contents = NULL;
    }

    return(ret);
}


krb5_error_code krb5_c_verify_checksum(krb5_context context, const krb5_keyblock *key, krb5_keyusage usage, const krb5_data *data,
                                       const krb5_checksum *cksum, krb5_boolean *valid)
{
    int i;
    int hashsize;
    krb5_error_code ret;
    krb5_data indata;
    krb5_checksum computed;

    for (i=0; i<krb5_cksumtypes_length; i++)
    {
        if (krb5_cksumtypes_list[i].ctype == cksum->checksum_type)
            break;
    }

    if (i == krb5_cksumtypes_length)
        return(KRB5_BAD_ENCTYPE);

    /* if there's actually a verify function, call it */

    indata.length = cksum->length;
    indata.data = (char *) cksum->contents;

    if (krb5_cksumtypes_list[i].keyhash && krb5_cksumtypes_list[i].keyhash->verify)
        return((*(krb5_cksumtypes_list[i].keyhash->verify))(key, usage, 0, data, &indata, valid));

    /* otherwise, make the checksum again, and compare */

    if ((ret = krb5_c_checksum_length(context, cksum->checksum_type, &hashsize)))
        return(ret);

    if (cksum->length != hashsize)
        return(KRB5_BAD_MSIZE);

    computed.length = hashsize;

    if ((ret = krb5_c_make_checksum(context, cksum->checksum_type, key, usage, data, &computed)))
    {
        free(computed.contents);
        return(ret);
    }

    *valid = (memcmp(computed.contents, cksum->contents, hashsize) == 0);

    free(computed.contents);

    return(0);

}
static krb5_error_code k5_pac_verify_server_checksum(krb5_context context, const krb5_pac pac, const krb5_keyblock *server)
{
    krb5_error_code ret;
    krb5_data pac_data; /* PAC with zeroed checksums */
    krb5_checksum checksum;
    krb5_data checksum_data;
    krb5_boolean valid;
    krb5_octet *p;
    //krb5_keyblock k2;

    ret = k5_pac_locate_buffer(context, pac, PAC_SERVER_CHECKSUM, &checksum_data);
    if (ret != 0)
        return ret;

    if (checksum_data.length < PAC_SIGNATURE_DATA_LENGTH)
        return KRB5_BAD_MSIZE;

    p = (krb5_octet *)checksum_data.data;
    checksum.checksum_type = load_32_le(p);
    checksum.length = checksum_data.length - PAC_SIGNATURE_DATA_LENGTH;
    checksum.contents = p + PAC_SIGNATURE_DATA_LENGTH;

    pac_data.length = pac->data.length;
    pac_data.data = malloc(pac->data.length);
    if (pac_data.data == NULL)
        return -1;

    memcpy(pac_data.data, pac->data.data, pac->data.length);

    /* Zero out both checksum buffers */
    ret = k5_pac_zero_signature(context, pac, PAC_SERVER_CHECKSUM, &pac_data);
    if (ret != 0)
    {
        free(pac_data.data);
        return ret;
    }

    ret = k5_pac_zero_signature(context, pac, PAC_PRIVSVR_CHECKSUM, &pac_data);
    if (ret != 0)
    {
        free(pac_data.data);
        return ret;
    }

    //k2.contents = rc4hmac_pass;
    //k2.enctype = 0x17;
    //k2.length = sizeof(rc4hmac_pass);
    //ret = krb5_c_verify_checksum(context, &k2, KRB5_KEYUSAGE_APP_DATA_CKSUM, &pac_data, &checksum, &valid);

    ret = krb5_c_verify_checksum(context, server, KRB5_KEYUSAGE_APP_DATA_CKSUM, &pac_data, &checksum, &valid);
    free(pac_data.data);

    if (ret != 0)
    {
        return ret;
    }

    if (valid == 0)
        ret = KRB5KRB_AP_ERR_BAD_INTEGRITY;

    return ret;
}

krb5_error_code krb5_pac_verify(krb5_context context, const krb5_pac pac, krb5_timestamp authtime, krb5_const_principal principal,
                                const krb5_keyblock *server, const krb5_keyblock *privsvr)
{
    krb5_error_code ret;

    if (server == NULL)
        return -1;

    ret = k5_pac_verify_server_checksum(context, pac, server);
    if (ret != 0)
        return ret;

    if (privsvr != NULL)
    {
        ret = k5_pac_verify_kdc_checksum(context, pac, privsvr);
        if (ret != 0)
            return ret;
    }

    if (principal != NULL)
    {
        ret = k5_pac_validate_client(context, pac, authtime, principal);
        if (ret != 0)
            return ret;
    }

    pac->verified = 1;

    return 0;
}


krb5_error_code mspac_verify(krb5_context kcontext, krb5_pac *pac, const krb5_keyblock *key, const krb5_ticket *ticket)
{
    krb5_error_code code;

    code = krb5_pac_verify(kcontext, *pac, ticket->enc_part2->times.authtime,
                           ticket->enc_part2->client, key, NULL);

    return code;
}

krb5_error_code krb5_pac_parse(krb5_context context, void *ptr, int len, krb5_pac *ppac)
{
    krb5_error_code ret;
    int i;
    const unsigned char *p = (const unsigned char *)ptr;
    krb5_pac pac;
    int header_len;
    krb5_ui_4 cbuffers, version;

    *ppac = NULL;

    if (len < PACTYPE_LENGTH)
        return -1;

    cbuffers = load_32_le(p);
    p += 4;
    version = load_32_le(p);
    p += 4;

    if (version != 0)
        return -1;

    header_len = PACTYPE_LENGTH + (cbuffers * PAC_INFO_BUFFER_LENGTH);
    if (len < header_len)
        return -1;

    ret = krb5_pac_init(context, &pac, cbuffers, ptr);
    if (ret != 0)
        return ret;

    //if(sizeof(PACTYPE) + ((cbuffers - 1) * sizeof(PAC_INFO_BUFFER)) > sizeof(krb5_pac))
    {
        //free(pac->pac);
        //pac->pac = (PACTYPE *)malloc(sizeof(PACTYPE) + ((cbuffers - 1) * sizeof(PAC_INFO_BUFFER)));
        //if (pac->pac == NULL)
        //{
        //  krb5_pac_free(context, pac);
        //  return -1;
        //}
    }
    //pac->pac = (PACTYPE *)realloc(pac->pac, sizeof(PACTYPE) + ((cbuffers - 1) * sizeof(PAC_INFO_BUFFER)));
    //if (pac->pac == NULL) {
    //    krb5_pac_free(context, pac);
    //    return -1;
    //}

    pac->pac->cBuffers = cbuffers;
    pac->pac->Version = version;

    for (i = 0; i < pac->pac->cBuffers; i++)
    {
        PAC_INFO_BUFFER *buffer = &pac->pac->Buffers[i];

        buffer->ulType = load_32_le(p);
        p += 4;
        buffer->cbBufferSize = load_32_le(p);
        p += 4;
        buffer->Offset = load_64_le(p);
        p += 8;

        if (buffer->Offset % PAC_ALIGNMENT)
        {
            krb5_pac_free(context, pac);
            return -1;
        }
        if (buffer->Offset < header_len || buffer->Offset + buffer->cbBufferSize > len)
        {
            krb5_pac_free(context, pac);
            return -1;
        }
    }

    //if(len > pac->data.length)
    {
        //free(pac->data.data);
        //pac->data.data = (unsigned char*)malloc(len);
        //if (pac->data.data == NULL) {
        //    krb5_pac_free(context, pac);
        //    return -1;
        // }
    }
    //memcpy(pac->data.data, ptr, len);

    pac->data.length = len;

    *ppac = pac;

    return 0;
}
