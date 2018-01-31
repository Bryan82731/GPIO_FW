
#ifndef	KRB5_RSA_MD5__
#define	KRB5_RSA_MD5__
#include "krb.h"

krb5_error_code
k5_md5_hash(unsigned int icount, const krb5_data *input,
	    krb5_data *output);

/* Some useful types */

typedef krb5_octet	SHS_BYTE;
typedef krb5_ui_4	SHS_LONG;

/* Define the following to use the updated SHS implementation */
#define NEW_SHS         /**/

/* The SHS block size and message digest sizes, in bytes */

#define SHA1_DATASIZE    64
#define SHA1_DIGESTSIZE  20

krb5_error_code krb5_hmac
(const struct krb5_hash_provider *hash,
		const krb5_keyblock *key, unsigned int icount,
		const krb5_data *input, krb5_data *output);

/* Keyed Message digest functions (hmac_sha.c) */
krb5_error_code hmac_sha(krb5_octet *text,
			int text_len,
			krb5_octet *key,
			int key_len,
			krb5_octet *digest);


#define NIST_SHA_CKSUM_LENGTH		SHS_DIGESTSIZE
#define HMAC_SHA_CKSUM_LENGTH		SHS_DIGESTSIZE

#endif /* KRB5_RSA_MD5__ */
