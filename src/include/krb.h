#ifndef _KRB_DEFINED
#define _KRB_DEFINED
#include "krb_error.h"

#define HARD_ERROR(v) ((v) != GSS_S_COMPLETE && (v) != GSS_S_CONTINUE_NEEDED)

#ifndef NULL				/* ANSI C #defines NULL everywhere. */
#define	NULL		0
#endif
#define KRB5_OS_TOFFSET_VALID	1
#define KRB5_OS_TOFFSET_TIME	2

#define KRB5_PRINCIPAL_COMPARE_IGNORE_REALM         1
#define KRB5_PRINCIPAL_COMPARE_ENTERPRISE           2 /* compare UPNs as real principals */
#define KRB5_PRINCIPAL_COMPARE_CASEFOLD             4 /* case-insensitive comparison */
#define KRB5_PRINCIPAL_COMPARE_UTF8                 8 /* treat principals as UTF-8 */

/* per Kerberos v5 protocol spec */
#define ENCTYPE_NULL            0x0000
#define ENCTYPE_DES_CBC_CRC     0x0001  /* DES cbc mode with CRC-32 */
#define ENCTYPE_DES_CBC_MD4     0x0002  /* DES cbc mode with RSA-MD4 */
#define ENCTYPE_DES_CBC_MD5     0x0003  /* DES cbc mode with RSA-MD5 */
#define ENCTYPE_DES_CBC_RAW     0x0004  /* DES cbc mode raw */
/* XXX deprecated? */
#define ENCTYPE_DES3_CBC_SHA    0x0005  /* DES-3 cbc mode with NIST-SHA */
#define ENCTYPE_DES3_CBC_RAW    0x0006  /* DES-3 cbc mode raw */
#define ENCTYPE_DES_HMAC_SHA1   0x0008
#define ENCTYPE_DES3_CBC_SHA1   0x0010
#define ENCTYPE_AES128_CTS_HMAC_SHA1_96 0x0011
#define ENCTYPE_AES256_CTS_HMAC_SHA1_96 0x0012
#define ENCTYPE_ARCFOUR_HMAC    0x0017
#define ENCTYPE_ARCFOUR_HMAC_EXP 0x0018
#define ENCTYPE_UNKNOWN         0x01ff
/* local crud */
/* marc's DES-3 with 32-bit length */
#define ENCTYPE_LOCAL_DES3_HMAC_SHA1 0x7007

#define CKSUMTYPE_CRC32         0x0001
#define CKSUMTYPE_RSA_MD4       0x0002
#define CKSUMTYPE_RSA_MD4_DES   0x0003
#define CKSUMTYPE_DESCBC        0x0004
/* des-mac-k */
/* rsa-md4-des-k */
#define CKSUMTYPE_RSA_MD5       0x0007
#define CKSUMTYPE_RSA_MD5_DES   0x0008
#define CKSUMTYPE_NIST_SHA      0x0009
#define CKSUMTYPE_HMAC_SHA1_DES3        0x000c
#define CKSUMTYPE_HMAC_SHA1_96_AES128   0x000f
#define CKSUMTYPE_HMAC_SHA1_96_AES256   0x0010
#define CKSUMTYPE_HMAC_MD5_ARCFOUR -138 /*Microsoft md5 hmac cksumtype*/

#define KRB5_KEYUSAGE_AS_REQ_PA_ENC_TS          1
#define KRB5_KEYUSAGE_KDC_REP_TICKET            2
#define KRB5_KEYUSAGE_AS_REP_ENCPART            3
#define KRB5_KEYUSAGE_TGS_REQ_AD_SESSKEY        4
#define KRB5_KEYUSAGE_TGS_REQ_AD_SUBKEY         5
#define KRB5_KEYUSAGE_TGS_REQ_AUTH_CKSUM        6
#define KRB5_KEYUSAGE_TGS_REQ_AUTH              7
#define KRB5_KEYUSAGE_TGS_REP_ENCPART_SESSKEY   8
#define KRB5_KEYUSAGE_TGS_REP_ENCPART_SUBKEY    9
#define KRB5_KEYUSAGE_AP_REQ_AUTH_CKSUM         10
#define KRB5_KEYUSAGE_AP_REQ_AUTH               11
#define KRB5_KEYUSAGE_AP_REP_ENCPART            12
#define KRB5_KEYUSAGE_KRB_PRIV_ENCPART          13
#define KRB5_KEYUSAGE_KRB_CRED_ENCPART          14
#define KRB5_KEYUSAGE_KRB_SAFE_CKSUM            15
#define KRB5_KEYUSAGE_APP_DATA_ENCRYPT          16
#define KRB5_KEYUSAGE_APP_DATA_CKSUM            17
#define KRB5_KEYUSAGE_KRB_ERROR_CKSUM           18
#define KRB5_KEYUSAGE_AD_KDCISSUED_CKSUM        19
#define KRB5_KEYUSAGE_AD_MTE                    20
#define KRB5_KEYUSAGE_AD_ITE                    21

#define KV5M_NONE                                (-1760647424L)
#define KV5M_PRINCIPAL                           (-1760647423L)
#define KV5M_DATA                                (-1760647422L)
#define KV5M_KEYBLOCK                            (-1760647421L)
#define KV5M_CHECKSUM                            (-1760647420L)
#define KV5M_ENCRYPT_BLOCK                       (-1760647419L)
#define KV5M_ENC_DATA                            (-1760647418L)
#define KV5M_CRYPTOSYSTEM_ENTRY                  (-1760647417L)
#define KV5M_CS_TABLE_ENTRY                      (-1760647416L)
#define KV5M_CHECKSUM_ENTRY                      (-1760647415L)
#define KV5M_AUTHDATA                            (-1760647414L)
#define KV5M_TRANSITED                           (-1760647413L)
#define KV5M_ENC_TKT_PART                        (-1760647412L)
#define KV5M_TICKET                              (-1760647411L)
#define KV5M_AUTHENTICATOR                       (-1760647410L)
#define KV5M_TKT_AUTHENT                         (-1760647409L)
#define KV5M_CREDS                               (-1760647408L)
#define KV5M_LAST_REQ_ENTRY                      (-1760647407L)
#define KV5M_PA_DATA                             (-1760647406L)
#define KV5M_KDC_REQ                             (-1760647405L)
#define KV5M_ENC_KDC_REP_PART                    (-1760647404L)
#define KV5M_KDC_REP                             (-1760647403L)
#define KV5M_ERROR                               (-1760647402L)
#define KV5M_AP_REQ                              (-1760647401L)
#define KV5M_AP_REP                              (-1760647400L)
#define KV5M_AP_REP_ENC_PART                     (-1760647399L)
#define KV5M_RESPONSE                            (-1760647398L)
#define KV5M_SAFE                                (-1760647397L)
#define KV5M_PRIV                                (-1760647396L)
#define KV5M_PRIV_ENC_PART                       (-1760647395L)
#define KV5M_CRED                                (-1760647394L)
#define KV5M_CRED_INFO                           (-1760647393L)
#define KV5M_CRED_ENC_PART                       (-1760647392L)
#define KV5M_PWD_DATA                            (-1760647391L)
#define KV5M_ADDRESS                             (-1760647390L)
#define KV5M_KEYTAB_ENTRY                        (-1760647389L)
#define KV5M_CONTEXT                             (-1760647388L)
#define KV5M_OS_CONTEXT                          (-1760647387L)
#define KV5M_ALT_METHOD                          (-1760647386L)
#define KV5M_ETYPE_INFO_ENTRY                    (-1760647385L)
#define KV5M_DB_CONTEXT                          (-1760647384L)
#define KV5M_AUTH_CONTEXT                        (-1760647383L)
#define KV5M_KEYTAB                              (-1760647382L)
#define KV5M_RCACHE                              (-1760647381L)
#define KV5M_CCACHE                              (-1760647380L)
#define KV5M_PREAUTH_OPS                         (-1760647379L)
#define KV5M_SAM_CHALLENGE                       (-1760647378L)
#define KV5M_SAM_CHALLENGE_2                     (-1760647377L)
#define KV5M_SAM_KEY                             (-1760647376L)
#define KV5M_ENC_SAM_RESPONSE_ENC                (-1760647375L)
#define KV5M_ENC_SAM_RESPONSE_ENC_2              (-1760647374L)
#define KV5M_SAM_RESPONSE                        (-1760647373L)
#define KV5M_SAM_RESPONSE_2                      (-1760647372L)
#define KV5M_PREDICTED_SAM_RESPONSE              (-1760647371L)
#define KV5M_PASSWD_PHRASE_ELEMENT               (-1760647370L)
#define KV5M_GSS_OID                             (-1760647369L)
#define KV5M_GSS_QUEUE                           (-1760647368L)
#define ERROR_TABLE_BASE_kv5m (-1760647424L)

#define GSS_C_AF_UNSPEC     0
#define GSS_C_AF_LOCAL      1
#define GSS_C_AF_INET       2
#define GSS_C_AF_IMPLINK    3
#define GSS_C_AF_PUP        4
#define GSS_C_AF_CHAOS      5
#define GSS_C_AF_NS         6
#define GSS_C_AF_NBS        7
#define GSS_C_AF_ECMA       8
#define GSS_C_AF_DATAKIT    9
#define GSS_C_AF_CCITT      10
#define GSS_C_AF_SNA        11
#define GSS_C_AF_DECnet     12
#define GSS_C_AF_DLI        13
#define GSS_C_AF_LAT        14
#define GSS_C_AF_HYLINK     15
#define GSS_C_AF_APPLETALK  16
#define GSS_C_AF_BSC        17
#define GSS_C_AF_DSS        18
#define GSS_C_AF_OSI        19
#define GSS_C_AF_X25        21
#define GSS_C_AF_INET6      24
#define GSS_C_AF_NULLADDR   255
#define ADDRTYPE_INET           0x0002
#define ADDRTYPE_CHAOS          0x0005
#define ADDRTYPE_XNS            0x0006
#define ADDRTYPE_ISO            0x0007
#define ADDRTYPE_DDP            0x0010
#define ADDRTYPE_INET6          0x0018
#define ADDRTYPE_ADDRPORT       0x0100
#define ADDRTYPE_IPPORT         0x0101

typedef int krb5_int32;
typedef unsigned char   krb5_octet;
typedef unsigned int    krb5_ui_4;
typedef unsigned short  krb5_ui_2;
typedef unsigned long long krb5_ui_8;
typedef unsigned int krb5_boolean;
typedef unsigned int krb5_msgtype;
typedef unsigned int krb5_kvno;
typedef krb5_int32 krb5_addrtype;
typedef krb5_int32 krb5_enctype;
typedef krb5_int32 krb5_authdatatype;
typedef krb5_int32 krb5_keyusage;
typedef krb5_int32 krb5_preauthtype; /* This may change, later on */
typedef krb5_int32 krb5_flags;
typedef krb5_int32 krb5_timestamp;
typedef krb5_int32 krb5_error_code;
typedef krb5_int32 krb5_deltat;
typedef krb5_int32 krb5_cksumtype;
typedef krb5_int32 krb5_cryptotype;
typedef krb5_error_code krb5_magic;
typedef unsigned int OM_uint32;
typedef unsigned int gss_uint32;
/*
 * Various Null values
 */
#define GSS_C_NO_NAME ((gss_name_t) 0)
#define GSS_C_NO_BUFFER ((gss_buffer_t) 0)
#define GSS_C_NO_OID ((gss_OID) 0)
#define GSS_C_NO_OID_SET ((gss_OID_set) 0)
#define GSS_C_NO_CONTEXT ((gss_ctx_id_t) 0)
#define GSS_C_NO_CREDENTIAL ((gss_cred_id_t) 0)
#define GSS_C_NO_CHANNEL_BINDINGS ((gss_channel_bindings_t) 0)
#define GSS_C_EMPTY_BUFFER {0, NULL}

/* GSSAPI structure */

#define GSS_ERROR(x) \
  ((x) & ((GSS_C_CALLING_ERROR_MASK << GSS_C_CALLING_ERROR_OFFSET) | \
          (GSS_C_ROUTINE_ERROR_MASK << GSS_C_ROUTINE_ERROR_OFFSET)))
/*
 * Some alternate names for a couple of the above values.  These are defined
 * for V1 compatibility.
 */
#define GSS_C_NULL_OID          GSS_C_NO_OID
#define GSS_C_NULL_OID_SET      GSS_C_NO_OID_SET

/* Major status codes */

#define GSS_S_COMPLETE 0
/*
 * Some "helper" definitions to make the status code macros obvious.
 */
#define GSS_C_CALLING_ERROR_OFFSET 24
#define GSS_C_ROUTINE_ERROR_OFFSET 16
#define GSS_C_SUPPLEMENTARY_OFFSET 0
#define GSS_C_CALLING_ERROR_MASK ((OM_uint32) 0377ul)
#define GSS_C_ROUTINE_ERROR_MASK ((OM_uint32) 0377ul)
#define GSS_C_SUPPLEMENTARY_MASK ((OM_uint32) 0177777ul)

/*
 * Calling errors:
 */
#define GSS_S_CALL_INACCESSIBLE_READ (((OM_uint32) 1ul) << GSS_C_CALLING_ERROR_OFFSET)
#define GSS_S_CALL_INACCESSIBLE_WRITE (((OM_uint32) 2ul) << GSS_C_CALLING_ERROR_OFFSET)
#define GSS_S_CALL_BAD_STRUCTURE (((OM_uint32) 3ul) << GSS_C_CALLING_ERROR_OFFSET)

/*
 * Routine errors:
 */
#define GSS_S_BAD_MECH (1ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_NAME (2ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_NAMETYPE (3ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_BINDINGS (4ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_STATUS (5ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_SIG (6ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_MIC GSS_S_BAD_SIG
#define GSS_S_NO_CRED (7ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_NO_CONTEXT (8ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_DEFECTIVE_TOKEN (9ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_DEFECTIVE_CREDENTIAL (10ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_CREDENTIALS_EXPIRED (11ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_CONTEXT_EXPIRED (12ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_FAILURE (13ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_QOP (14ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_UNAUTHORIZED (15ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_UNAVAILABLE (16ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_DUPLICATE_ELEMENT (17ul << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_NAME_NOT_MN (18ul << GSS_C_ROUTINE_ERROR_OFFSET)

#define ASN1_EXPLICIT_TAG_0 0x00
#define ASN1_EXPLICIT_TAG_1 0x01
#define ASN1_EXPLICIT_TAG_2 0x02
#define ASN1_EXPLICIT_TAG_3 0x03
#define ASN1_EXPLICIT_TAG_4 0x04
#define ASN1_EXPLICIT_TAG_5 0x05
#define ASN1_EXPLICIT_TAG_6 0x06
#define ASN1_EXPLICIT_TAG_7 0x07
#define ASN1_EXPLICIT_TAG_8 0x08
#define ASN1_EXPLICIT_TAG_9 0x09
#define ASN1_EXPLICIT_TAG_10 0x0a

#define BLOCK_SIZE  16
typedef int gss_int32;
#define GSS_C_INDEFINITE ((OM_uint32) 0xfffffffful)
#define GSS_C_BOTH 0
#define GSS_C_INITIATE 1
#define GSS_C_ACCEPT 2

//#define ENOMEM KRB5KRB_ERR_GENERIC
/* Kerberos Message Types */
#define ASN1_KRB_AS_REQ		10
#define ASN1_KRB_AS_REP		11
#define ASN1_KRB_TGS_REQ	12
#define ASN1_KRB_TGS_REP	13
#define ASN1_KRB_AP_REQ		14
#define ASN1_KRB_AP_REP		15
#define ASN1_KRB_SAFE		20
#define ASN1_KRB_PRIV		21
#define ASN1_KRB_CRED		22
#define ASN1_KRB_ERROR		30

#define KVNO 5

#define AP_OPTS_RESERVED                0x80000000
#define AP_OPTS_USE_SESSION_KEY         0x40000000
#define AP_OPTS_MUTUAL_REQUIRED         0x20000000

/** helper macros **/

#define g_OID_equal(o1, o2) (((o1)->length == (o2)->length) && \
	(memcmp((o1)->elements, (o2)->elements, (o1)->length) == 0))

/*
 * Per V5 spec on definition of principal types
 */

/* Name type not known */
#define KRB5_NT_UNKNOWN         0
/* Just the name of the principal as in DCE, or for users */
#define KRB5_NT_PRINCIPAL       1
/* Service and other unique instance (krbtgt) */
#define KRB5_NT_SRV_INST        2
/* Service with host name as instance (telnet, rcommands) */
#define KRB5_NT_SRV_HST         3
/* Service with host as remaining components */
#define KRB5_NT_SRV_XHST        4
/* Unique ID */
#define KRB5_NT_UID             5

/* constant version thereof: */
#define krb5_princ_realm(context, princ) (&(princ)->realm)
#define krb5_princ_set_realm(context, princ,value) ((princ)->realm = *(value))
#define krb5_princ_set_realm_length(context, princ,value) (princ)->realm.length = (value)
#define krb5_princ_set_realm_data(context, princ,value) (princ)->realm.data = (value)
#define krb5_princ_size(context, princ) (princ)->length
#define krb5_princ_type(context, princ) (princ)->type
#define krb5_princ_name(context, princ) (princ)->data
#define krb5_princ_component(context, princ,i) ((princ)->data + i)

#define krb5_xfree(val) free((char *)(val))

/* from gssapi_err_generic.h */
#define G_BAD_SERVICE_NAME                       (-2045022976L)
#define G_BAD_STRING_UID                         (-2045022975L)
#define G_NOUSER                                 (-2045022974L)
#define G_VALIDATE_FAILED                        (-2045022973L)
#define G_BUFFER_ALLOC                           (-2045022972L)
#define G_BAD_MSG_CTX                            (-2045022971L)
#define G_WRONG_SIZE                             (-2045022970L)
#define G_BAD_USAGE                              (-2045022969L)
#define G_UNKNOWN_QOP                            (-2045022968L)
#define G_NO_HOSTNAME                            (-2045022967L)
#define G_BAD_HOSTNAME                           (-2045022966L)
#define G_WRONG_MECH                             (-2045022965L)
#define G_BAD_TOK_HEADER                         (-2045022964L)
#define G_BAD_DIRECTION                          (-2045022963L)
#define G_TOK_TRUNC                              (-2045022962L)
#define G_REFLECT                                (-2045022961L)
#define G_WRONG_TOKID                            (-2045022960L)

/* Internal auth_context_flags */
#define KRB5_AUTH_CONN_INITIALIZED      0x00010000
#define KRB5_AUTH_CONN_USED_W_MK_REQ    0x00020000
#define KRB5_AUTH_CONN_USED_W_RD_REQ    0x00040000
#define KRB5_AUTH_CONN_SANE_SEQ         0x00080000
#define KRB5_AUTH_CONN_HEIMDAL_SEQ      0x00100000

/** constants **/

#define CKSUMTYPE_KG_CB         0x8003

#define KG_TOK_CTX_AP_REQ       0x0100
#define KG_TOK_CTX_AP_REP       0x0200
#define KG_TOK_CTX_ERROR        0x0300
#define KG_TOK_SIGN_MSG         0x0101
#define KG_TOK_SEAL_MSG         0x0201
#define KG_TOK_MIC_MSG          0x0101
#define KG_TOK_WRAP_MSG         0x0201
#define KG_TOK_DEL_CTX          0x0102

#define KG2_TOK_INITIAL         0x0101
#define KG2_TOK_RESPONSE        0x0202
#define KG2_TOK_MIC             0x0303
#define KG2_TOK_WRAP_INTEG      0x0404
#define KG2_TOK_WRAP_PRIV       0x0505

#define KRB5_GSS_FOR_CREDS_OPTION 1

#define KG2_RESP_FLAG_ERROR             0x0001
#define KG2_RESP_FLAG_DELEG_OK          0x0002

/*
 * Supplementary info bits:
 */
#define GSS_S_CONTINUE_NEEDED (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 0))
#define GSS_S_DUPLICATE_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 1))
#define GSS_S_OLD_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 2))
#define GSS_S_UNSEQ_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 3))
#define GSS_S_GAP_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 4))

#define GSS_C_QOP_DEFAULT 0


#define GSS_MECH_KRB5_OID_LENGTH 9
#define GSS_MECH_KRB5_OID "\052\206\110\206\367\022\001\002\002"

#define GSS_MECH_KRB5_OLD_OID_LENGTH 5
#define GSS_MECH_KRB5_OLD_OID "\053\005\001\005\002"

/* Incorrect krb5 mech OID emitted by MS. */
#define GSS_MECH_KRB5_WRONG_OID_LENGTH 9
#define GSS_MECH_KRB5_WRONG_OID "\052\206\110\202\367\022\001\002\002"

/*
 * Flag bits for context-level services.
 */
#define GSS_C_DELEG_FLAG 1 
#define GSS_C_MUTUAL_FLAG 2 
#define GSS_C_REPLAY_FLAG 4 
#define GSS_C_SEQUENCE_FLAG 8
#define GSS_C_CONF_FLAG 16 
#define GSS_C_INTEG_FLAG 32 
#define GSS_C_ANON_FLAG 64 
#define GSS_C_PROT_READY_FLAG 128  
#define GSS_C_TRANS_FLAG 256

#define	g_OID_copy(o1, o2)					\
do {								\
	memcpy((o1)->elements, (o2)->elements, (o2)->length);	\
	(o1)->length = (o2)->length;				\
} while (0)


#define	GSS_EMPTY_BUFFER(buf)	((buf) == NULL ||\
	(buf)->value == NULL || (buf)->length == 0)

/* Token types remctl: Remote Authenticated Command Service*/
#define TOKEN_NOOP		(1<<0)
#define TOKEN_CONTEXT		(1<<1)
#define TOKEN_DATA		(1<<2)
#define TOKEN_MIC		(1<<3)

/* Token flags */
#define TOKEN_CONTEXT_NEXT	(1<<4)
#define TOKEN_WRAPPED		(1<<5)
#define TOKEN_ENCRYPTED		(1<<6)
#define TOKEN_SEND_MIC		(1<<7)

#define GSSINT_CHK_LOOP(p) (!((p) != NULL && (p)->loopback == (p)))

#define KRB5_CKSUMFLAG_DERIVE		0x0001
#define KRB5_CKSUMFLAG_NOT_COLL_PROOF	0x0002
	

#define	RSA_MD5_CKSUM_LENGTH		16
#define	OLD_RSA_MD5_DES_CKSUM_LENGTH	16
#define	NEW_RSA_MD5_DES_CKSUM_LENGTH	24
#define	RSA_MD5_DES_CONFOUND_LENGTH	8

/* from gssapi_err_krb5.h */
#define KG_CCACHE_NOMATCH                        (39756032L)
#define KG_KEYTAB_NOMATCH                        (39756033L)
#define KG_TGT_MISSING                           (39756034L)
#define KG_NO_SUBKEY                             (39756035L)
#define KG_CONTEXT_ESTABLISHED                   (39756036L)
#define KG_BAD_SIGN_TYPE                         (39756037L)
#define KG_BAD_LENGTH                            (39756038L)
#define KG_CTX_INCOMPLETE                        (39756039L)
#define KG_CONTEXT                               (39756040L)
#define KG_CRED                                  (39756041L)
#define KG_ENC_DESC                              (39756042L)
#define KG_BAD_SEQ                               (39756043L)
#define KG_EMPTY_CCACHE                          (39756044L)
#define KG_NO_CTYPES                             (39756045L)

#define CFX_ACCEPTOR_SUBKEY 1


#define TWRITE_INT(ptr, num, bigend) \
   (ptr)[0] = (char) ((bigend)?((num)>>24):((num)&0xff)); \
   (ptr)[1] = (char) ((bigend)?(((num)>>16)&0xff):(((num)>>8)&0xff)); \
   (ptr)[2] = (char) ((bigend)?(((num)>>8)&0xff):(((num)>>16)&0xff)); \
   (ptr)[3] = (char) ((bigend)?((num)&0xff):((num)>>24)); \
   (ptr) += 4;

#define TWRITE_INT16(ptr, num, bigend) \
   (ptr)[0] = (char) ((bigend)?((num)>>24):((num)&0xff)); \
   (ptr)[1] = (char) ((bigend)?(((num)>>16)&0xff):(((num)>>8)&0xff)); \
   (ptr) += 2;

#define TREAD_INT(ptr, num, bigend) \
   (num) = (((ptr)[0]<<((bigend)?24: 0)) | \
            ((ptr)[1]<<((bigend)?16: 8)) | \
            ((ptr)[2]<<((bigend)? 8:16)) | \
            ((ptr)[3]<<((bigend)? 0:24))); \
   (ptr) += 4;

#define TREAD_INT16(ptr, num, bigend) \
   (num) = (((ptr)[0]<<((bigend)?24: 0)) | \
            ((ptr)[1]<<((bigend)?16: 8))); \
   (ptr) += 2;

#define TWRITE_STR(ptr, str, len) \
   memcpy((ptr), (char *) (str), (len)); \
   (ptr) += (len);

#define TREAD_STR(ptr, str, len) \
   (str) = (ptr); \
   (ptr) += (len);

#define TWRITE_BUF(ptr, buf, bigend) \
   TWRITE_INT((ptr), (buf).length, (bigend)); \
   TWRITE_STR((ptr), (buf).value, (buf).length);	

#define PAC_INFO_BUFFER_LENGTH  16

/* ulType */
#define PAC_LOGON_INFO          1
#define PAC_CREDENTIALS_INFO    2
#define PAC_SERVER_CHECKSUM     6
#define PAC_PRIVSVR_CHECKSUM    7
#define PAC_CLIENT_INFO         10
#define PAC_DELEGATION_INFO     11
#define PAC_UPN_DNS_INFO        12

#define PAC_ALIGNMENT               8
#define PACTYPE_LENGTH              8U
#define PAC_SIGNATURE_DATA_LENGTH   4U
#define PAC_CLIENT_INFO_LENGTH      10U

# define krb5int_zap_data(ptr, len) memset((void *)ptr, 0, len)

#define MAXSUBAUTHS 15 /* max sub authorities in a SID */

/* clueless as to what maximum length should be */
#define MAX_UNISTRLEN 256
#define MAX_STRINGLEN 256
#define MAX_BUFFERLEN 512

/* RIDs - well-known groups ... */
#define RTK_DOMAIN_GROUP_RID_ADMINS        512
#define RTK_DOMAIN_GROUP_RID_USERS         513
#define RTK_DOMAIN_GROUP_RID_GUESTS        514

#define krb5_roundup(x, y) ((((x) + (y) - 1)/(y))*(y))

typedef enum asn1_error_number{
        ASN1_BAD_TIMEFORMAT = 1859794432,
        ASN1_MISSING_FIELD = 1859794433,
        ASN1_MISPLACED_FIELD = 1859794434,
        ASN1_TYPE_MISMATCH = 1859794435,
        ASN1_OVERFLOW = 1859794436,
        ASN1_OVERRUN = 1859794437,
        ASN1_BAD_ID = 1859794438,
        ASN1_BAD_LENGTH = 1859794439,
        ASN1_BAD_FORMAT = 1859794440,
        ASN1_PARSE_ERROR = 1859794441
} asn1_error_code;

/* flags for krb5_auth_con_setflags */
enum {
    KRB5_AUTH_CONTEXT_DO_TIME                   = 1,
    KRB5_AUTH_CONTEXT_RET_TIME                  = 2,
    KRB5_AUTH_CONTEXT_DO_SEQUENCE               = 4,
    KRB5_AUTH_CONTEXT_RET_SEQUENCE              = 8,
    KRB5_AUTH_CONTEXT_PERMIT_ALL                = 16,
    KRB5_AUTH_CONTEXT_USE_SUBKEY                = 32,
    KRB5_AUTH_CONTEXT_CLEAR_FORWARDED_CRED      = 64
};

typedef struct _krb5_data {
        krb5_magic magic;
        unsigned int length;
        unsigned char *data;
} krb5_data;

typedef struct _krb5_keyblock {
    krb5_magic magic;
    krb5_enctype enctype;
    int length;
    krb5_octet *contents;
} krb5_keyblock;

typedef struct krb5_crypto_iov {
    unsigned int flags;
    /* ignored */
#define KRB5_CRYPTO_TYPE_EMPTY          0
    /* OUT krb5_crypto_length(KRB5_CRYPTO_TYPE_HEADER) */
#define KRB5_CRYPTO_TYPE_HEADER         1
    /* IN and OUT */
#define KRB5_CRYPTO_TYPE_DATA           2
    /* IN */
#define KRB5_CRYPTO_TYPE_SIGN_ONLY      3
   /* (only for encryption) OUT krb5_crypto_length(KRB5_CRYPTO_TYPE_TRAILER) */
#define KRB5_CRYPTO_TYPE_PADDING        4
   /* OUT krb5_crypto_length(KRB5_CRYPTO_TYPE_TRAILER) */
#define KRB5_CRYPTO_TYPE_TRAILER        5
   /* OUT krb5_crypto_length(KRB5_CRYPTO_TYPE_CHECKSUM) */
#define KRB5_CRYPTO_TYPE_CHECKSUM       6
    krb5_data data;
} krb5_crypto_iov;
/* Internal structure of an opaque key identifier */
typedef struct krb5_key_st 
{
    krb5_keyblock keyblock;
    int refcount;
    struct derived_key *derived;
    /*
     * Cache of data private to the cipher implementation, which we
     * don't want to have to recompute for every operation.  This may
     * include key schedules, iteration counts, etc.
     *
     * The cipher implementation is responsible for setting this up
     * whenever needed, and the enc_provider key_cleanup method must
     * then be provided to dispose of it.
     */
    void *cache;
}*krb5_key;

struct derived_key 
{
    krb5_data constant;
    krb5_key dkey;
    struct derived_key *next;
};

/* new encryption provider api */
struct krb5_enc_provider 
{
	/* keybytes is the input size to make_key; 
		keylength is the output size */
	int block_size, keybytes, keylength;
	/* cipher-state == 0 fresh state thrown away at end */
	krb5_error_code (*encrypt) (const krb5_keyblock *key, const krb5_data *cipher_state, const krb5_data *input,
				krb5_data *output);
	krb5_error_code (*decrypt) (const krb5_keyblock *key, const krb5_data *ivec, const krb5_data *input,
				krb5_data *output);
	krb5_error_code (*make_key) (const krb5_data *randombits,
				 krb5_keyblock *key);
	krb5_error_code (*init_state) (const krb5_keyblock *key, krb5_keyusage keyusage, krb5_data *out_state);
	krb5_error_code (*free_state) (krb5_data *state);
	/* May be NULL if there is no key-derived data cached.  */
	void (*key_cleanup)(krb5_key key);
};

struct krb5_hash_provider 
{
    int hashsize, blocksize;
    /* this takes multiple inputs to avoid lots of copying. */
    krb5_error_code (*hash) (unsigned int icount, const krb5_data *input, krb5_data *output);
};

typedef void (*krb5_encrypt_length_func) (const struct krb5_enc_provider *enc,
  const struct krb5_hash_provider *hash, int inputlen, int *length);

typedef krb5_error_code (*krb5_crypt_func) (const struct krb5_enc_provider *enc,
  const struct krb5_hash_provider *hash, const krb5_keyblock *key, krb5_keyusage keyusage,
  const krb5_data *ivec, const krb5_data *input, krb5_data *output, char);

typedef krb5_error_code (*krb5_str2key_func) (const struct krb5_enc_provider *enc, 
	const krb5_data *string, const krb5_data *salt, const krb5_data *parm, krb5_keyblock *key);

typedef krb5_error_code (*krb5_prf_func)(const struct krb5_enc_provider *enc, 
	const struct krb5_hash_provider *hash, const krb5_keyblock *key, const krb5_data *in, krb5_data *out);

struct krb5_keytypes 
{
	krb5_enctype etype;
	char *in_string;
	char *out_string;
	const struct krb5_enc_provider *enc;
	const struct krb5_hash_provider *hash;
	int prf_length;
	krb5_encrypt_length_func encrypt_len;
	krb5_crypt_func encrypt;
	krb5_crypt_func decrypt;
	krb5_str2key_func str2key;
	krb5_prf_func prf;
	krb5_cksumtype required_ctype;
	char shift;
};

typedef struct krb5_principal_data 
{
    krb5_magic magic;
    krb5_data realm;
    krb5_data *data;            /* An array of strings */
    krb5_int32 length;
    krb5_int32 type;
} krb5_principal_data;

typedef krb5_principal_data * krb5_principal;

typedef const krb5_principal_data *krb5_const_principal;

typedef struct _krb5_enc_data 
{
    krb5_magic magic;
    krb5_enctype enctype;
    krb5_kvno kvno;
    krb5_data ciphertext;
} krb5_enc_data;

/* structure for address */
typedef struct _krb5_address 
{
    krb5_magic magic;
    krb5_addrtype addrtype;
    unsigned int length;
    krb5_octet *contents;
} krb5_address;

/* Time set */
typedef struct _krb5_ticket_times 
{
    krb5_timestamp authtime; /* XXX ? should ktime in KDC_REP == authtime in ticket? otherwise client can't get this */
    krb5_timestamp starttime;           /* optional in ticket, if not present, use authtime */
    krb5_timestamp endtime;
    krb5_timestamp renew_till;
} krb5_ticket_times;

/* structure for auth data */
typedef struct _krb5_authdata 
{
    krb5_magic magic;
    krb5_authdatatype ad_type;
    unsigned int length;
    krb5_octet *contents;
} krb5_authdata;

/* structure for transited encoding */
typedef struct _krb5_transited 
{
    krb5_magic magic;
    krb5_octet tr_type;
    krb5_data tr_contents;
} krb5_transited;

typedef struct _krb5_enc_tkt_part 
{
    krb5_magic magic;
    /* to-be-encrypted portion */
    krb5_flags flags;                   /* flags */
    krb5_keyblock *session;             /* session key: includes enctype */
    krb5_principal client;              /* client name/realm */
    krb5_transited transited;           /* list of transited realms */
    krb5_ticket_times times;            /* auth, start, end, renew_till */
    krb5_address **caddrs;      /* array of ptrs to addresses */
    krb5_authdata **authorization_data; /* auth data */
} krb5_enc_tkt_part;

typedef struct _krb5_ticket 
{
    krb5_magic magic; /* cleartext portion */
    krb5_principal server;              /* server name/realm */
    krb5_enc_data enc_part;             /* encryption type, kvno, encrypted encoding */
    krb5_enc_tkt_part *enc_part2;       /* ptr to decrypted version, if available */
	krb5_data *scratch;
} krb5_ticket;

typedef struct _krb5_checksum 
{
    krb5_magic magic;
    krb5_cksumtype checksum_type;       /* checksum type */
    int length;
    krb5_octet *contents;
} krb5_checksum;

/* the unencrypted version */
typedef struct _krb5_authenticator 
{
    krb5_magic magic;
    krb5_principal client;              /* client name/realm */
    krb5_checksum *checksum;        /* checksum, includes type, optional */
    krb5_int32 cusec;                   /* client usec portion */
    krb5_timestamp ctime;               /* client sec portion */
    krb5_keyblock *subkey;          /* true session key, optional */
    krb5_int32 seq_number;              /* sequence #, optional */
    krb5_authdata **authorization_data; /* New add by Ari, auth data */
	krb5_data *scratch;
} krb5_authenticator;

typedef struct _krb5_ap_req 
{
    krb5_magic magic;
    krb5_flags ap_options;              /* requested options */
    krb5_ticket *ticket;            /* ticket */
    krb5_enc_data authenticator;        /* authenticator (already encrypted) */
} krb5_ap_req;

typedef enum 
{
    KRB5_PROMPT_TYPE_PASSWORD           = 0x1,
    KRB5_PROMPT_TYPE_NEW_PASSWORD       = 0x2,
    KRB5_PROMPT_TYPE_NEW_PASSWORD_AGAIN = 0x3,
    KRB5_PROMPT_TYPE_PREAUTH            = 0x4
} krb5_prompt_type;

struct _krb5_context 
{
	krb5_magic      magic;
	krb5_enctype    *in_tkt_ktypes;
	int             in_tkt_ktype_count;
	krb5_enctype    *tgs_ktypes;
	int             tgs_ktype_count;
	void            *os_context;
	char            *default_realm;
	//profile_t       profile;
	void            *db_context;
	int             ser_ctx_count;
	void            *ser_ctx;
	/* allowable clock skew */
	krb5_deltat     clockskew;
	krb5_cksumtype  kdc_req_sumtype;
	krb5_cksumtype  default_ap_req_sumtype;
	krb5_cksumtype  default_safe_sumtype;
	krb5_flags      kdc_default_options;
	krb5_flags      library_options;
	krb5_boolean    profile_secure;
	int             fcc_default_format;
	int             scc_default_format;
	krb5_prompt_type *prompt_types;
	/* Message size above which we'll try TCP first in send-to-kdc
	type code.  Aside from the 2**16 size limit, we put no
	absolute limit on the UDP packet size.  */
	int             udp_pref_limit;

	/* This is the tgs_ktypes list as read from the profile, or
	set to compiled-in defaults.  The application code cannot
	override it.  This is used for session keys for
	intermediate ticket-granting tickets used to acquire the
	requested ticket (the session key of which may be
	constrained by tgs_ktypes above).  */
	krb5_enctype    *conf_tgs_ktypes;
	int             conf_tgs_ktypes_count;
	/* Use the _configured version?  */
	krb5_boolean    use_conf_ktypes;

#ifdef KRB5_DNS_LOOKUP
	krb5_boolean    profile_in_memory;
#endif /* KRB5_DNS_LOOKUP */
};

struct _krb5_auth_context;
typedef struct _krb5_context * krb5_context;
typedef struct _krb5_auth_context * krb5_auth_context;
typedef krb5_error_code (*krb5_mk_req_checksum_func) 
	(krb5_context context, krb5_auth_context auth_context, void*, krb5_data**);
typedef void * krb5_pointer;

typedef struct krb5_rc_st
{
	krb5_magic magic;
	//const struct _krb5_kt_ops *ops;
	krb5_pointer data;
	//k5_mutex_t lock;
}*krb5_rcache;

struct _krb5_auth_context 
{
    krb5_magic          magic;
    krb5_address      * remote_addr;
    krb5_address      * remote_port;
    krb5_address      * local_addr;
    krb5_address      * local_port;
    krb5_keyblock     * keyblock;
    krb5_keyblock     * send_subkey;
    krb5_keyblock     * recv_subkey;

    krb5_int32          auth_context_flags;
    krb5_ui_4           remote_seq_number;
    krb5_ui_4           local_seq_number;
    krb5_authenticator *authentp;               /* mk_req, rd_req, mk_rep, ...*/
    krb5_cksumtype      req_cksumtype;          /* mk_safe, ... */
    krb5_cksumtype      safe_cksumtype;         /* mk_safe, ... */
    krb5_pointer        i_vector;               /* mk_priv, rd_priv only */
    krb5_rcache         rcache;
    krb5_enctype      * permitted_etypes;       /* rd_req */
	krb5_mk_req_checksum_func checksum_func;
  void *checksum_func_data;
};

typedef struct gss_OID_desc_struct 
{
      OM_uint32 length;
      void      *elements;
} gss_OID_desc, *gss_OID;

/* Reserved static storage for GSS_oids.  Comments are quotes from RFC 2744. */

#define oids ((gss_OID_desc *)const_oids)
static const gss_OID_desc const_oids[] = {
    /*
     * The implementation must reserve static storage for a
	 * gss_OID_desc object containing the value */
    {10, (void *)"\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x01"},
    /* corresponding to an object-identifier value of
	 * {iso(1) member-body(2) United States(840) mit(113554)
	 * infosys(1) gssapi(2) generic(1) user_name(1)}.  The constant
	 * GSS_C_NT_USER_NAME should be initialized to point
	 * to that gss_OID_desc.
	 */                                
    
    /*
	 * The implementation must reserve static storage for a
	 * gss_OID_desc object containing the value */
    {10, (void *)"\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x02"},
    /* corresponding to an object-identifier value of
	 * {iso(1) member-body(2) United States(840) mit(113554)
	 * infosys(1) gssapi(2) generic(1) machine_uid_name(2)}.
	 * The constant GSS_C_NT_MACHINE_UID_NAME should be
	 * initialized to point to that gss_OID_desc.
	 */
     
    /*
    * The implementation must reserve static storage for a
    * gss_OID_desc object containing the value */
    {10, (void *)"\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x03"},
    /* corresponding to an object-identifier value of
    * {iso(1) member-body(2) United States(840) mit(113554)
    * infosys(1) gssapi(2) generic(1) string_uid_name(3)}.
    * The constant GSS_C_NT_STRING_UID_NAME should be
    * initialized to point to that gss_OID_desc.
    */
    
    /*
     * The implementation must reserve static storage for a
     * gss_OID_desc object containing the value */
    {6, (void *)"\x2b\x06\x01\x05\x06\x02"},
    /* corresponding to an object-identifier value of
     * {iso(1) org(3) dod(6) internet(1) security(5)
     * nametypes(6) gss-host-based-services(2)).  The constant
     * GSS_C_NT_HOSTBASED_SERVICE_X should be initialized to point
     * to that gss_OID_desc.  This is a deprecated OID value, and
     * implementations wishing to support hostbased-service names
     * should instead use the GSS_C_NT_HOSTBASED_SERVICE OID,
     * defined below, to identify such names;
     * GSS_C_NT_HOSTBASED_SERVICE_X should be accepted a synonym
     * for GSS_C_NT_HOSTBASED_SERVICE when presented as an input
     * parameter, but should not be emitted by GSS-API
     * implementations
     */
    
    /*
     * The implementation must reserve static storage for a
     * gss_OID_desc object containing the value */
    {10, (void *)"\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x04"}, 
    /* corresponding to an object-identifier value of 
     * {iso(1) member-body(2) Unites States(840) mit(113554) 
     * infosys(1) gssapi(2) generic(1) service_name(4)}.  
     * The constant GSS_C_NT_HOSTBASED_SERVICE should be 
     * initialized to point to that gss_OID_desc.
     */

    /*
     * The implementation must reserve static storage for a
     * gss_OID_desc object containing the value */
    {6, (void *)"\x2b\x06\01\x05\x06\x03"},
    /* corresponding to an object identifier value of
     * {1(iso), 3(org), 6(dod), 1(internet), 5(security),
     * 6(nametypes), 3(gss-anonymous-name)}.  The constant
     * and GSS_C_NT_ANONYMOUS should be initialized to point
     * to that gss_OID_desc.
     */
    
    /*
     * The implementation must reserve static storage for a
     * gss_OID_desc object containing the value */
    {6, (void *)"\x2b\x06\x01\x05\x06\x04"},
    /* corresponding to an object-identifier value of
     * {1(iso), 3(org), 6(dod), 1(internet), 5(security),
     * 6(nametypes), 4(gss-api-exported-name)}.  The constant
     * GSS_C_NT_EXPORT_NAME should be initialized to point
     * to that gss_OID_desc.
     */
};

typedef struct gss_buffer_desc_struct
{
  unsigned int length;
  void *value;
} gss_buffer_desc, *gss_buffer_t;

struct gss_name_struct
{
	struct gss_name_struct *loopback;
	gss_OID name_type;
	gss_buffer_t external_name;
	gss_OID mech_type;
	struct gss_name_struct * mech_name;
};

typedef struct gss_name_struct *gss_name_t;
typedef struct gss_name_struct gss_union_name_desc;
typedef struct gss_name_struct *gss_union_name_t;

typedef struct gss_channel_bindings_struct
{
	OM_uint32 initiator_addrtype;
	gss_buffer_desc initiator_address;
	OM_uint32 acceptor_addrtype;
	gss_buffer_desc acceptor_address;
	gss_buffer_desc application_data;
} *gss_channel_bindings_t;

typedef struct gss_ctx_id_struct
{
	struct gss_ctx_id_struct *loopback;
	gss_OID mech_type;
	struct gss_ctx_id_struct* internal_ctx_id;
}*gss_ctx_id_t,gss_union_ctx_id_desc, *gss_union_ctx_id_t;

typedef int gss_cred_usage_t;

typedef krb5_principal krb5_gss_name_t;

typedef struct _krb5_kt
{
	krb5_magic magic;
	//const struct _krb5_kt_ops *ops;
	krb5_pointer data;
}*krb5_keytab;

typedef struct _krb5_ccache 
{
    krb5_magic magic;
    krb5_pointer data;
}  *krb5_ccache;

typedef struct 
{
    char loc_last, loc_created;
    char os;
    char stats;
} k5_mutex_t;

typedef struct _krb5_gss_cred_id_rec 
{
   /* protect against simultaneous accesses */
   //k5_mutex_t lock;
	char dum[4];
   /* name/type of credential */
   gss_cred_usage_t usage;
   krb5_principal princ;	/* this is not interned as a gss_name_t */
   int prerfc_mech;
   int rfc_mech;

   /* keytab (accept) data */
   krb5_keytab keytab;
   krb5_rcache rcache;

   /* ccache (init) data */
   krb5_ccache ccache;
   krb5_timestamp tgt_expire;
   krb5_enctype *req_enctypes;	/* limit negotiated enctypes to this list */
} krb5_gss_cred_id_rec, *krb5_gss_cred_id_t; 

typedef struct gss_OID_set_desc_struct  
{
  int     count;
  gss_OID    elements;
} gss_OID_set_desc, *gss_OID_set;

typedef struct gss_cred_id_t_desc_struct 
{
  gss_name_t principal;
  struct krb5_keytab_data *keytab;
  OM_uint32 lifetime;
  gss_cred_usage_t usage;
  gss_OID_set mechanisms;
  struct krb5_ccache_data *ccache;
} gss_cred_id_t_desc;

typedef unsigned long gssint_uint64;

typedef struct _krb5_gss_ctx_id_rec 
{
   unsigned int initiate : 1;	/* nonzero if initiating, zero if accepting */
   unsigned int established : 1;
   unsigned int big_endian : 1;
   unsigned int have_acceptor_subkey : 1;
   unsigned int seed_init : 1;	/* XXX tested but never actually set */
   OM_uint32 gss_flags;
   unsigned char seed[16];
   krb5_principal here;
   krb5_principal there;
   krb5_keyblock *subkey;
   krb5_ticket *ticket;
   int signalg;
   int cksum_size;
   int sealalg;
   krb5_keyblock *enc;
   krb5_keyblock *seq;
   krb5_timestamp endtime;
   krb5_flags krb_flags;
   /* XXX these used to be signed.  the old spec is inspecific, and
      the new spec specifies unsigned.  I don't believe that the change
      affects the wire encoding. */
   gssint_uint64 seq_send;
   gssint_uint64 seq_recv;
   void *seqstate;
   krb5_context k5_context;
   krb5_auth_context auth_context;
   gss_OID_desc *mech_used;
    /* Protocol spec revision
       0 => RFC 1964 with 3DES and RC4 enhancements
       1 => draft-ietf-krb-wg-gssapi-cfx-01
       No others defined so far.  */
   int proto;
   krb5_cksumtype cksumtype;	/* for "main" subkey */
   krb5_keyblock *acceptor_subkey; /* CFX only */
   krb5_cksumtype acceptor_subkey_cksumtype;
   int cred_rcache;		/* did we get rcache from creds? */
} krb5_gss_ctx_id_rec, *krb5_gss_ctx_id_t;

/*
 * Structure for holding list of mechanism-specific name types
 */
typedef struct gss_mech_spec_name_t {
    gss_OID	name_type;
    gss_OID	mech;
    struct gss_mech_spec_name_t	*next, *prev;
} gss_mech_spec_name_desc, *gss_mech_spec_name;

/*
 * Credential auxiliary info, used in the credential structure
 */
typedef struct gss_union_cred_auxinfo {
	gss_buffer_desc		name;
	gss_OID			name_type;
	OM_uint32		creation_time;
	OM_uint32		time_rec;
	int			cred_usage;
} gss_union_cred_auxinfo;

/*
 * Set of Credentials typed on mechanism OID
 */
typedef struct gss_cred_id_struct {
	struct gss_cred_id_struct *loopback;
	int			count;
	gss_OID			mechs_array;
	struct gss_cred_id_struct		**cred_array;
	gss_union_cred_auxinfo	auxinfo;
} gss_union_cred_desc, *gss_union_cred_t;

typedef struct gss_cred_id_struct *gss_cred_id_t;

typedef OM_uint32 gss_qop_t;

typedef struct gss_config {
    OM_uint32	    priority;
    char *	    mechNameStr;
    gss_OID_desc    mech_type;
    void *	    context;
    OM_uint32       (*gss_acquire_cred)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_name_t,		/* desired_name */
		    OM_uint32,		/* time_req */
		    gss_OID_set,	/* desired_mechs */
		    int,		/* cred_usage */
		    gss_cred_id_t*,	/* output_cred_handle */
		    gss_OID_set*,	/* actual_mechs */
		    OM_uint32*		/* time_rec */
		    );

    OM_uint32       (*gss_release_cred)
	(
		    void*,		/* context */		       
		    OM_uint32*,		/* minor_status */
		    gss_cred_id_t*	/* cred_handle */
		    );
    OM_uint32       (*gss_init_sec_context)
	(
		    void*,			/* context */
		    OM_uint32*,			/* minor_status */
		    gss_cred_id_t,		/* claimant_cred_handle */
		    gss_ctx_id_t*,		/* context_handle */
		    gss_name_t,			/* target_name */
		    gss_OID,			/* mech_type */
		    OM_uint32,			/* req_flags */
		    OM_uint32,			/* time_req */
		    gss_channel_bindings_t,	/* input_chan_bindings */
		    gss_buffer_t,		/* input_token */
		    gss_OID*,			/* actual_mech_type */
		    gss_buffer_t,		/* output_token */
		    OM_uint32*,			/* ret_flags */
		    OM_uint32*			/* time_rec */
		    );
    OM_uint32       (*gss_accept_sec_context)
	(
		    void*,			/* context */
		    OM_uint32*,			/* minor_status */
		    gss_ctx_id_t*,		/* context_handle */
		    gss_cred_id_t,		/* verifier_cred_handle */
		    gss_buffer_t,		/* input_token_buffer */
		    gss_channel_bindings_t,	/* input_chan_bindings */
		    gss_name_t*,		/* src_name */
		    gss_OID*,			/* mech_type */
		    gss_buffer_t,		/* output_token */
		    OM_uint32*,			/* ret_flags */
		    OM_uint32*,			/* time_rec */
		    gss_cred_id_t*		/* delegated_cred_handle */
		    );
    OM_uint32       (*gss_process_context_token)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    gss_buffer_t	/* token_buffer */
		    );
    OM_uint32       (*gss_delete_sec_context)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t*,	/* context_handle */
		    gss_buffer_t	/* output_token */
		    );
    OM_uint32       (*gss_context_time)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    OM_uint32*		/* time_rec */
		    );
    OM_uint32       (*gss_sign)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    int,		/* qop_req */
		    gss_buffer_t,	/* message_buffer */
		    gss_buffer_t	/* message_token */
		    );
    OM_uint32       (*gss_verify)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    gss_buffer_t,	/* message_buffer */
		    gss_buffer_t,	/* token_buffer */
		    int*		/* qop_state */
		    );
    OM_uint32       (*gss_seal)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    int,		/* conf_req_flag */
		    int,		/* qop_req */
		    gss_buffer_t,	/* input_message_buffer */
		    int*,		/* conf_state */
		    gss_buffer_t	/* output_message_buffer */
		    );
    OM_uint32       (*gss_unseal)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    gss_buffer_t,	/* input_message_buffer */
		    gss_buffer_t,	/* output_message_buffer */
		    int*,		/* conf_state */
		    int*		/* qop_state */
		    );
    OM_uint32       (*gss_display_status)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    OM_uint32,		/* status_value */
		    int,		/* status_type */
		    gss_OID,		/* mech_type */
		    OM_uint32*,		/* message_context */
		    gss_buffer_t	/* status_string */
		    );
    OM_uint32       (*gss_indicate_mechs)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_OID_set*	/* mech_set */
		    );
    OM_uint32       (*gss_compare_name)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_name_t,		/* name1 */
		    gss_name_t,		/* name2 */
		    int*		/* name_equal */
		    );
    OM_uint32       (*gss_display_name)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_name_t,		/* input_name */
		    gss_buffer_t,	/* output_name_buffer */
		    gss_OID*		/* output_name_type */
		    );
    OM_uint32       (*gss_import_name)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_buffer_t,	/* input_name_buffer */
		    gss_OID,		/* input_name_type */
		    gss_name_t*		/* output_name */
		    );
    OM_uint32       (*gss_release_name)
	(
		    void*,		/* context */
		    OM_uint32*,		/* minor_status */
		    gss_name_t*		/* input_name */
		    );
    OM_uint32       (*gss_inquire_cred)
	(
		    void*,			/* context */
		    OM_uint32 *,		/* minor_status */
		    gss_cred_id_t,		/* cred_handle */
		    gss_name_t *,		/* name */
		    OM_uint32 *,		/* lifetime */
		    int *,			/* cred_usage */
		    gss_OID_set *		/* mechanisms */
		    );
    OM_uint32	    (*gss_add_cred)
	(
		    //void*,		/* context */
		    OM_uint32 *,	/* minor_status */
		    gss_cred_id_t,	/* input_cred_handle */
		    gss_name_t,		/* desired_name */
		    gss_OID,		/* desired_mech */
		    gss_cred_usage_t,	/* cred_usage */
		    OM_uint32,		/* initiator_time_req */
		    OM_uint32,		/* acceptor_time_req */
		    gss_cred_id_t *,	/* output_cred_handle */
		    gss_OID_set *,	/* actual_mechs */
		    OM_uint32 *,	/* initiator_time_rec */
		    OM_uint32 *		/* acceptor_time_rec */
		    );
    OM_uint32	    (*gss_export_sec_context)
	(
		    //void*,		/* context */
		    OM_uint32 *,	/* minor_status */
		    gss_ctx_id_t *,	/* context_handle */
		    gss_buffer_t	/* interprocess_token */
		    );
    OM_uint32	    (*gss_import_sec_context)
	(
		    //void *,		/* context */
		    OM_uint32 *,	/* minor_status */
		    gss_buffer_t,	/* interprocess_token */
		    gss_ctx_id_t *	/* context_handle */
		    );
    OM_uint32 	    (*gss_inquire_cred_by_mech)
	(
		   // void *,		/* context */
		    OM_uint32 *,	/* minor_status */
		    gss_cred_id_t,	/* cred_handle */
		    gss_OID,		/* mech_type */
		    gss_name_t *,	/* name */
		    OM_uint32 *,	/* initiator_lifetime */
		    OM_uint32 *,	/* acceptor_lifetime */
		    gss_cred_usage_t *	/* cred_usage */
		    );
    OM_uint32	    (*gss_inquire_names_for_mech)
	(
		   // void *,		/* context */
		    OM_uint32 *,	/* minor_status */
		    gss_OID,		/* mechanism */
		    gss_OID_set *	/* name_types */
		    );
    OM_uint32	(*gss_inquire_context)
	(
		   // void *,		/* context */
		    OM_uint32 *,	/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    gss_name_t *,	/* src_name */
		    gss_name_t *,	/* targ_name */
		    OM_uint32 *,	/* lifetime_rec */
		    gss_OID *,		/* mech_type */
		    OM_uint32 *,	/* ctx_flags */
		    int *,           	/* locally_initiated */
		    int *		/* open */
		    );
    OM_uint32	    (*gss_internal_release_oid)
	(
		    //void *,		/* context */
		    OM_uint32 *,	/* minor_status */
		    gss_OID *		/* OID */
	 );
    OM_uint32	     (*gss_wrap_size_limit)
	(
		    //void *,		/* context */
		    OM_uint32 *,	/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    int,		/* conf_req_flag */
		    gss_qop_t,		/* qop_req */
		    OM_uint32,		/* req_output_size */
		    OM_uint32 *		/* max_input_size */
	 );
	OM_uint32		(*gss_export_name)
	(
		//void *,			/* context */
		OM_uint32 *,		/* minor_status */
		const gss_name_t,	/* input_name */
		gss_buffer_t		/* exported_name */
	/* */);
	OM_uint32	(*gss_store_cred)
	(
		//void *,			/* context */
		OM_uint32 *,		/* minor_status */
		const gss_cred_id_t,	/* input_cred */
		gss_cred_usage_t,	/* cred_usage */
		const gss_OID,		/* desired_mech */
		OM_uint32,		/* overwrite_cred */
		OM_uint32,		/* default_cred */
		gss_OID_set *,		/* elements_stored */
		gss_cred_usage_t *	/* cred_usage_stored */
	/* */);	
} *gss_mechanism;

typedef struct gss_mech_config {
	char *kmodName;			/* kernel module name */
	char *uLibName;			/* user library name */
	char *mechNameStr;		/* mechanism string name */
	char *optionStr;		/* optional mech parameters */
	void *dl_handle;		/* RTLD object handle for the mech */
	gss_OID mech_type;		/* mechanism oid */
	gss_mechanism mech;		/* mechanism initialization struct */
	struct gss_mech_config *next;	/* next element in the list */
} *gss_mech_info;


OM_uint32 gss_acquire_cred(
	OM_uint32 *		minor_status,
	gss_name_t		desired_name,
	OM_uint32		time_req,
	gss_OID_set		desired_mechs,
	int			cred_usage,
	gss_cred_id_t *		output_cred_handle,
	gss_OID_set *		actual_mechs,
	OM_uint32 *		time_rec);

OM_uint32 gss_inquire_cred(
	OM_uint32 *		minor_status,
	gss_cred_id_t 		cred_handle,
	gss_name_t *		name,
	OM_uint32 *		lifetime,
	int *			cred_usage,
	gss_OID_set *		mechanisms);

krb5_error_code krb5_derive_key(const struct krb5_enc_provider *enc,
		const krb5_keyblock *inkey, krb5_keyblock *outkey,
		const krb5_data *in_constant);

struct krb5_keyhash_provider {
    int hashsize;

    krb5_error_code (*hash) (const krb5_keyblock *key,
			     krb5_keyusage keyusage,
			     const krb5_data *ivec,
			     const krb5_data *input,
			     krb5_data *output);

    krb5_error_code (*verify) (const krb5_keyblock *key,
			       krb5_keyusage keyusage,
			       const krb5_data *ivec,
			       const krb5_data *input,
			       const krb5_data *hash,
			       krb5_boolean *valid);
};

struct krb5_cksumtypes {
    krb5_cksumtype ctype;
    unsigned int flags;
    char *in_string;
    char *out_string;
    /* if the hash is keyed, this is the etype it is keyed with.
       Actually, it can be keyed by any etype which has the same
       enc_provider as the specified etype.  DERIVE checksums can
       be keyed with any valid etype. */
    krb5_enctype keyed_etype;
    /* I can't statically initialize a union, so I'm just going to use
       two pointers here.  The keyhash is used if non-NULL.  If NULL,
       then HMAC/hash with derived keys is used if the relevant flag
       is set.  Otherwise, a non-keyed hash is computed.  This is all
       kind of messy, but so is the krb5 api. */
    const struct krb5_keyhash_provider *keyhash;
    const struct krb5_hash_provider *hash;
    /* This just gets uglier and uglier.  In the key derivation case,
       we produce an hmac.  To make the hmac code work, we can't hack
       the output size indicated by the hash provider, but we may want
       a truncated hmac.  If we want truncation, this is the number of
       bytes we truncate to; it should be 0 otherwise.  */
    unsigned int trunc_size;
};

/* These are to be stored in little-endian order, i.e., des-mac is
   stored as 02 00.  */
enum sgn_alg {
  SGN_ALG_DES_MAC_MD5           = 0x0000,
  SGN_ALG_MD2_5                 = 0x0001,
  SGN_ALG_DES_MAC               = 0x0002,
  SGN_ALG_3			= 0x0003, /* not published */
  SGN_ALG_HMAC_MD5              = 0x0011, /* microsoft w2k;  */
  SGN_ALG_HMAC_SHA1_DES3_KD     = 0x0004
};
enum seal_alg {
  SEAL_ALG_NONE            = 0xffff,
  SEAL_ALG_DES             = 0x0000,
  SEAL_ALG_1		   = 0x0001, /* not published */
  SEAL_ALG_MICROSOFT_RC4   = 0x0010, /* microsoft w2k;  */
  SEAL_ALG_DES3KD          = 0x0002
};

typedef struct _krb5_ap_rep {
    krb5_magic magic;
    krb5_enc_data enc_part;
} krb5_ap_rep;

typedef struct _krb5_ap_rep_enc_part {
    krb5_magic magic;
    krb5_timestamp ctime;               /* client time, seconds portion */
    krb5_int32 cusec;                   /* client time, microseconds portion */
    krb5_keyblock *subkey;          /* true session key, optional */
    krb5_int32 seq_number;              /* sequence #, optional */
} krb5_ap_rep_enc_part;

extern const struct krb5_enc_provider krb5int_enc_arcfour;
extern const struct krb5_hash_provider krb5int_hash_md5;
extern const struct krb5_keyhash_provider krb5int_keyhash_hmac_md5;
extern const struct krb5_hash_provider krb5int_hash_sha1;
extern const struct krb5_enc_provider krb5int_enc_aes128;

typedef struct _PAC_INFO_BUFFER {
    krb5_ui_4 ulType;
    krb5_ui_4 cbBufferSize;
    krb5_ui_8 Offset;
} PAC_INFO_BUFFER;

typedef struct _PACTYPE {
    krb5_ui_4 cBuffers;
    krb5_ui_4 Version;
    struct _PAC_INFO_BUFFER Buffers[1];
} PACTYPE;

typedef struct krb5_pac_data {
    struct _PACTYPE *pac;       /* PAC header + info buffer array */
    krb5_data data;     /* PAC data (including uninitialised header) */
    krb5_boolean verified;
}*krb5_pac;

typedef unsigned short  krb5_ucs2;
typedef int     krb5_ucs4;
typedef struct _krb5_authdata_context {
    krb5_magic magic;
    int n_modules;
    struct _krb5_authdata_context_module {
        krb5_authdatatype ad_type;
        void *plugin_context;
        //authdata_client_plugin_fini_proc client_fini;
        krb5_flags flags;
        //krb5plugin_authdata_client_ftable_v0 *ftable;
        //authdata_client_request_init_proc client_req_init;
        //authdata_client_request_fini_proc client_req_fini;
        const char *name;
        void *request_context;
        void **request_context_pp;
    } *modules;
    //struct plugin_dir_handle plugins;
}*krb5_authdata_context;

typedef struct _NTTIME 
{
	unsigned int dwLowDateTime;
	unsigned int dwHighDateTime;
}NTTIME;

/* UNIHDR - unicode string header */
typedef struct unihdr_info
{
  unsigned short uni_str_len;
  unsigned short uni_max_len;
  unsigned int buffer; /* usually has a value of 4 */

} UNIHDR;

/* UNIHDR2 - unicode string header and undocumented buffer */
typedef struct unihdr2_info
{
  UNIHDR unihdr;
  unsigned int buffer; /* 32 bit buffer pointer */

} UNIHDR2;

/* DOM_GID - group id + user attributes */
typedef struct gid_info
{
  unsigned int g_rid;  /* a group RID */
  unsigned int attr;

} DOM_GID;

/* DOM_SID - security id */
typedef struct sid_info
{
  unsigned char sid_rev_num;             /* SID revision number */
  unsigned char num_auths;               /* number of sub-authorities */
  unsigned char id_auth[6];              /* Identifier Authority */
  unsigned int *sub_auths/*[MAXSUBAUTHS]*/;  /* pointer to sub-authorities. */

} DOM_SID;

/* DOM_SID2 - security id */
typedef struct sid_info_2
{
        unsigned int num_auths; /* length, bytes, including length of len :-) */

       struct sid_info sid;

} DOM_SID2;

/* UNISTR2 - unicode string size (in uint16 unicode chars) and buffer */
typedef struct unistr2_info
{
  unsigned int uni_max_len;
  unsigned int offset;
  unsigned int uni_str_len;
  unsigned short *buffer/*[MAX_UNISTRLEN]*/; /* unicode characters. **NOT** necessarily null-terminated */

} UNISTR2;

typedef struct group_membership {
        unsigned int rid;
        unsigned int attrs;
} GROUP_MEMBERSHIP;

typedef struct group_membership_array {
        unsigned int count;
        GROUP_MEMBERSHIP *group_membership;
} GROUP_MEMBERSHIP_ARRAY;

typedef struct krb_sid_and_attrs {
        unsigned int sid_ptr;
        unsigned int attrs;
        DOM_SID2 *sid;
} KRB_SID_AND_ATTRS;

typedef struct krb_sid_and_attr_array {
        unsigned int count;
        KRB_SID_AND_ATTRS *krb_sid_and_attrs;
} KRB_SID_AND_ATTR_ARRAY;

/* This is awfully similar to a samr_user_info_23, but not identical.
   Many of the field names have been swiped from there, because it is
   so similar that they are likely the same, but many have been verified.
   Some are in a different order, though... */
typedef struct KERB_VALIDATION_INFO {
		//unsigned int dummy;
        NTTIME logon_time;            /* logon time */
        NTTIME logoff_time;           /* logoff time */
        NTTIME kickoff_time;          /* kickoff time */
        NTTIME pass_last_set_time;    /* password last set time */
        NTTIME pass_can_change_time;  /* password can change time */
        NTTIME pass_must_change_time; /* password must change time */

        UNIHDR hdr_user_name;    /* user name unicode string header */
        UNIHDR hdr_full_name;    /* user's full name unicode string header */
        UNIHDR hdr_logon_script; /* these last 4 appear to be in a different */
        UNIHDR hdr_profile_path; /* order than in the info23 */
        UNIHDR hdr_home_dir;
        UNIHDR hdr_dir_drive;

        unsigned short logon_count; /* number of times user has logged onto domain */
        unsigned short reserved12;

        unsigned int user_rid;
        unsigned int group_rid;
        unsigned int group_count;
        unsigned int group_membership_ptr;
        unsigned int user_flags;

        unsigned int reserved13[4];
        UNIHDR hdr_dom_controller;
        UNIHDR hdr_dom_name;

        unsigned int ptr_dom_sid;

        unsigned int reserved16[2];
        unsigned int reserved17;      /* looks like it may be acb_info */
        unsigned int reserved18[7];

        unsigned int sid_count;
        unsigned int ptr_extra_sids;

        unsigned int ptr_res_group_dom_sid;
        unsigned int res_group_count;
        unsigned int ptr_res_groups;
#if 0
        UNISTR2 uni_user_name;    /* user name unicode string header */
        UNISTR2 uni_full_name;    /* user's full name unicode string header */
        UNISTR2 uni_logon_script; /* these last 4 appear to be in a different*/
        UNISTR2 uni_profile_path; /* order than in the info23 */
        UNISTR2 uni_home_dir;
        UNISTR2 uni_dir_drive;
        UNISTR2 uni_dom_controller;
        UNISTR2 uni_dom_name;
        DOM_SID2 dom_sid;
        GROUP_MEMBERSHIP_ARRAY groups;
        KRB_SID_AND_ATTR_ARRAY extra_sids;
        DOM_SID2 res_group_dom_sid;
        GROUP_MEMBERSHIP_ARRAY res_groups;
#endif
} KVI;

typedef struct pac_logon_info2
{
        UNISTR2 uni_user_name;    /* user name unicode string header */
        UNISTR2 uni_full_name;    /* user's full name unicode string header */
        UNISTR2 uni_logon_script; /* these last 4 appear to be in a different*/
        UNISTR2 uni_profile_path; /* order than in the info23 */
        UNISTR2 uni_home_dir;
        UNISTR2 uni_dir_drive;
        
		GROUP_MEMBERSHIP_ARRAY groups;
        KRB_SID_AND_ATTR_ARRAY extra_sids;
        DOM_SID2 res_group_dom_sid;
        GROUP_MEMBERSHIP_ARRAY res_groups;

        UNISTR2 uni_domain_controller;
        UNISTR2 uni_domain_name;
        DOM_SID2 dom_sid;
}PLI2;


//#define TEST_GSSAPI
//#undef TEST_GSSAPI
//#define TEST_SPNEGO

#endif

