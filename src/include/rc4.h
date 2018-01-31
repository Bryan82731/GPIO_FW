#ifndef ARCFOUR_H
#define ARCFOUR_H

#include "krb.h"
#include "md4.h"
#include <string.h>
#include <stdlib.h>

#define krb5_roundup(x, y) ((((x) + (y) - 1)/(y))*(y))

void
krb5_arcfour_encrypt_length(
			const struct krb5_enc_provider *,
			const struct krb5_hash_provider *,
			int,
			int *);

 
krb5_error_code krb5_arcfour_encrypt(
			const struct krb5_enc_provider *,
			const struct krb5_hash_provider *,
			const krb5_keyblock *,
			krb5_keyusage,
			const krb5_data *,
     			const krb5_data *,
			krb5_data *,
			char);

krb5_error_code krb5_arcfour_decrypt(
		const struct krb5_enc_provider *,
		const struct krb5_hash_provider *hash,
		const krb5_keyblock *key, 
		krb5_keyusage usage, 
		const krb5_data *ivec,
		const krb5_data *input,
		krb5_data *output,
		char shift);

#if 0
krb5_error_code krb5int_arcfour_string_to_key(
     const struct krb5_enc_provider *,
     const krb5_data *,
     const krb5_data *,
     const krb5_data *,
     krb5_keyblock *);
#endif
krb5_error_code k5_arcfour_docrypt(
	const krb5_keyblock *key, const krb5_data *state,
	const krb5_data *input, krb5_data *output);

//const struct krb5_enc_provider krb5int_enc_arcfour;


#define CONFOUNDERLENGTH 8

typedef struct
{
   unsigned int x;
   unsigned int y;
   unsigned char state[256];
} ArcfourContext;

typedef struct {
  int initialized;
  ArcfourContext ctx;
} ArcFourCipherState;

krb5_keyusage krb5int_arcfour_translate_usage(krb5_keyusage usage);


#endif /* ARCFOUR_H */
