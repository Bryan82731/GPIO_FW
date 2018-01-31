#ifndef ASN1_H
#define ASN1_H
#include "rsa.h"
#include "aes.h"
/*
 * DER constants
 */
/*
#define ASN1_BOOLEAN                 0x01
#define ASN1_INTEGER                 0x02
#define ASN1_BIT_STRING              0x03
#define ASN1_OCTET_STRING            0x04
#define ASN1_NULL                    0x05
#define ASN1_OID                     0x06
#define ASN1_UTF8_STRING             0x0C
#define ASN1_SEQUENCE                0x10
#define ASN1_SET                     0x11
#define ASN1_PRINTABLE_STRING        0x13
#define ASN1_T61_STRING              0x14
#define ASN1_IA5_STRING              0x16
#define ASN1_UTC_TIME                0x17
#define ASN1_UNIVERSAL_STRING        0x1C
#define ASN1_BMP_STRING              0x1E
#define ASN1_PRIMITIVE               0x00
#define ASN1_CONSTRUCTED             0x20
#define ASN1_CONTEXT_SPECIFIC        0x80 //<< 
*/
enum
{
	CLASS_UNIVERSAL = 0x00,
	CLASS_APPLICATION = 0x010,
	CLASS_CONTEXT_SPECIFIC = 0x10,
	CLASS_PRIVATE = 0x11 //<< 6
};
#if 0
typedef struct _TLV
{
	unsigned char tag;
	unsigned char len;
}TLV;
#endif
typedef struct _Subject
{
	char Country[8];
	char Organization[56];
	char CommonName1[64];
	char CommonName2[64];
}Subject;
#if 0
typedef struct _Attributes
{
	unsigned char* attribute;
}Attributes;

typedef struct _CertReqInfo
{
	int version;
	struct _Subject subject;
	unsigned char* subjectPKInfo;
	struct _Attributes attributes;
}CertReqInfo;

typedef struct _CSR
{
	struct _CertReqInfo certificationRequestInfo;
	int signatureAlgorithm;
	unsigned char *signatureValue;
}CSR;

#endif
#if CONFIG_VERSION < 6
#define CERT_BUFFER_NOT_ENOUGH  0x90000052
#define DECRYPTION_ERROR 0x90000053
void generateCipher(aes_context *encrypt, char* inData, unsigned char* pBuffer, int* length);
int RtkOutCredential(rsa_context* pctx, unsigned char* pCert, unsigned char* pBuf, int* pBufLen);
void cert_write(INT8U *buf, INT32S len);
#endif
INT8S getRSACtx(rsa_context *ctx);

void initCsr();
int set_asn1_CertificationRequestInfo_Init(unsigned char version);
int set_asn1_subject(Subject *sub);
int set_asn1_subjectPKInfo(int pubLen);
int set_asn1_attributes(int pubLen);
int set_asn1_sig(int pubLen);
int destroyCsr();
int bin2b64(int *tsize);
#ifdef csr_self_test
int write_csr();
int self_test();
int write_csr_test();
#endif
#endif
