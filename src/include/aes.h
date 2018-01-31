/**
 * \file aes.h
 */
#ifndef XYSSL_AES_H
#define XYSSL_AES_H

#define AES_ENCRYPT     0
#define AES_DECRYPT     1
#define AES_ENCRYPT_SW     0
#define AES_DECRYPT_SW     1



/**
 * \brief          AES context structure
 */
typedef struct
{
    int nr;                     /*!<  number of rounds  */
    unsigned long *rk;          /*!<  AES round keys    */
    unsigned long buf[68];      /*!<  unaligned data    */
}
aes_context, aes_context_sw;

typedef struct _aesHwKey
{
#if CONFIG_VERSION  >= IC_VERSION_FP_RevA
	unsigned char AES_Key[32];
#else
	unsigned char AES_Key[16];
#endif
	unsigned char IV[16];
}aesHwKey;
#ifdef __cplusplus
extern "C" {
#endif

void aes_setkey_enc( aes_context *ctx, unsigned char *key, int keysize );
    /**
     * \brief          AES-CBC buffer encryption/decryption
     *
     * \param ctx      AES context
     * \param mode     AES_ENCRYPT or AES_DECRYPT
     * \param length   length of the input data
     * \param iv       initialization vector (updated after use)
     * \param input    buffer holding the input data
     * \param output   buffer holding the output data
     */
    void aes_crypt_cbc( aes_context *ctx,
                        int mode,
                        int length,
                        unsigned char iv[16],
                        unsigned char *input,
                        unsigned char *output);
/**
 * \brief          AES key schedule (encryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      encryption key
 * \param keysize  must be 128, 192 or 256
 */
void aes_setkey_enc_sw( aes_context_sw *ctx, unsigned char *key, int keysize );

/**
 * \brief          AES key schedule (decryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      decryption key
 * \param keysize  must be 128, 192 or 256
 */
void aes_setkey_dec_sw( aes_context_sw *ctx, unsigned char *key, int keysize );

/**
 * \brief          AES-ECB block encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param input    16-byte input block
 * \param output   16-byte output block
 */
void aes_crypt_ecb_sw( aes_context_sw *ctx,
                    int mode,
                    unsigned char input[16],
                    unsigned char output[16] );

/**
 * \brief          AES-CBC buffer encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 */
void aes_crypt_cbc_sw( aes_context_sw *ctx,
                    int mode,
                    int length,
                    unsigned char iv[16],
                    unsigned char *input,
                    unsigned char *output );

/**
 * \brief          AES-CFB buffer encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param length   length of the input data
 * \param iv_off   offset in IV (updated after use)
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 */
void aes_crypt_cfb_sw( aes_context_sw *ctx,
                    int mode,
                    int length,
                    int *iv_off,
                    unsigned char iv[16],
                    unsigned char *input,
                    unsigned char *output );                

void aes_crypt(aes_context *ctx, unsigned char *src, unsigned char *dest, unsigned char type);
void aes_crypt_sw(aes_context *ctx, unsigned char *src, unsigned char *dest, unsigned char type);
#ifdef __cplusplus
}
#endif

#endif /* aes.h */
