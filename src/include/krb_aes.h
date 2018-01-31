#ifndef KRB_AES_H
#define KRB_AES_H

#include "krb.h"
#include "aes.h"

void krb5_dk_encrypt_length
(const struct krb5_enc_provider *enc,
		const struct krb5_hash_provider *hash,
		int input, size_t *length);

krb5_error_code krb5_dk_encrypt
(const struct krb5_enc_provider *enc,
		const struct krb5_hash_provider *hash,
		const krb5_keyblock *key, krb5_keyusage usage,
		const krb5_data *ivec,
		const krb5_data *input, krb5_data *output);

void krb5int_aes_encrypt_length
(const struct krb5_enc_provider *enc,
		const struct krb5_hash_provider *hash,
		int input, int *length);

krb5_error_code krb5int_aes_dk_encrypt
(const struct krb5_enc_provider *enc,
		const struct krb5_hash_provider *hash,
		const krb5_keyblock *key, krb5_keyusage usage,
		const krb5_data *ivec,
		const krb5_data *input, krb5_data *output, char shift);

krb5_error_code krb5_dk_decrypt
(const struct krb5_enc_provider *enc,
		const struct krb5_hash_provider *hash,
		const krb5_keyblock *key, krb5_keyusage usage,
		const krb5_data *ivec, const krb5_data *input,
		krb5_data *arg_output);

krb5_error_code krb5int_aes_dk_decrypt
(const struct krb5_enc_provider *enc,
		const struct krb5_hash_provider *hash,
		const krb5_keyblock *key, krb5_keyusage usage,
		const krb5_data *ivec, const krb5_data *input,
		krb5_data *arg_output, char shift);

krb5_error_code krb5int_dk_string_to_key
(const struct krb5_enc_provider *enc, 
		const krb5_data *string, const krb5_data *salt,
		const krb5_data *params, krb5_keyblock *key);
void
krb5_nfold(unsigned int inbits, const unsigned char *in, unsigned int outbits,
	   unsigned char *out);

krb5_error_code krb5int_aes_encrypt(aes_context *aes_ctx, const krb5_data *ivec,  krb5_data *input, krb5_data *output);

krb5_error_code krb5int_aes_decrypt(aes_context *aes_ctx, const krb5_data *ivec,  const krb5_data *input, krb5_data *output);


krb5_error_code

krb5_derive_key_aes(aes_context *aes_ctx, krb5_keyblock *outkey,
		const krb5_data *in_constant);
#endif
