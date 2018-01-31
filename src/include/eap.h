#ifndef _EAP_INCLUDED
#define _EAP_INCLUDED

#include "lib.h"

void eapol();

typedef enum {
	EAP_TYPE_NONE = 0,
	EAP_TYPE_IDENTITY = 1 /* RFC 3748 */,
	EAP_TYPE_NOTIFICATION = 2 /* RFC 3748 */,
	EAP_TYPE_NAK = 3 /* Response only, RFC 3748 */,
	EAP_TYPE_MD5 = 4, /* RFC 3748 */
	EAP_TYPE_OTP = 5 /* RFC 3748 */,
	EAP_TYPE_GTC = 6, /* RFC 3748 */
	EAP_TYPE_TLS = 13 /* RFC 2716 */,
	EAP_TYPE_LEAP = 17 /* Cisco proprietary */,
	EAP_TYPE_SIM = 18 /* RFC 4186 */,
	EAP_TYPE_TTLS = 21 /* RFC 5281 */,
	EAP_TYPE_AKA = 23 /* RFC 4187 */,
	EAP_TYPE_PEAP = 25 /* draft-josefsson-pppext-eap-tls-eap-06.txt */,
	EAP_TYPE_MSCHAPV2 = 26 /* draft-kamath-pppext-eap-mschapv2-00.txt */,
	EAP_TYPE_TLV = 33 /* draft-josefsson-pppext-eap-tls-eap-07.txt */,
	EAP_TYPE_TNC = 38 /* TNC IF-T v1.0-r3; note: tentative assignment;
			   * type 38 has previously been allocated for
			   * EAP-HTTP Digest, (funk.com) */,
	EAP_TYPE_FAST = 43 /* RFC 4851 */,
	EAP_TYPE_PAX = 46 /* RFC 4746 */,
	EAP_TYPE_PSK = 47 /* RFC 4764 */,
	EAP_TYPE_SAKE = 48 /* RFC 4763 */,
	EAP_TYPE_IKEV2 = 49 /* RFC 5106 */,
	EAP_TYPE_AKA_PRIME = 50 /* draft-arkko-eap-aka-kdf-10.txt */,
	EAP_TYPE_GPSK = 51 /* RFC 5433 */,
	EAP_TYPE_EXPANDED = 254 /* RFC 3748 */
} EapType;

#define EAP_REQUEST_PKT       1
#define EAP_RESPONSE_PKT      2
#define EAP_SUCCESS_PKT       3
#define EAP_FAILURE_PKT       4

#define MS_CHAPV2_CHALLENGE     1
#define MS_CHAPV2_RESPONSE      2
#define MS_CHAPV2_SUCCESS       3
#define MS_CHAPV2_FAILURE       4
#define MS_CHAPV2_CHANGE_PWD    7

/** EAP CODE FIELD - RFC 2284 Sec 2.1**/
#define EAP_REQUEST           0x1       /* Request code for EAP */
#define EAP_RESPONSE          0x2       /* Response code for EAP */
#define EAP_SUCCESS           0x3       /* Success Code */
#define EAP_FAILURE           0x4       /* Failure Code */

#define EAP_TYPE_ID		  1
#define EAP_TYPE_NOTIFY   2
#define EAP_TYPE_NAK      3



typedef struct _EAPOLPKT 
{
	unsigned char ver;
	unsigned char type;
	unsigned short len;
	unsigned char *eappkt;
}EAPOLPKT;

typedef struct _e8021XPKT
{
	EthHdr ethHdr;
	unsigned char ver;
	unsigned char type;
	unsigned short len;
}e8021XPKT;

typedef struct _EAPPKT
{
	e8021XPKT auth;
	unsigned char code;
	unsigned char id;
	unsigned short len2;
	unsigned char type2;
}EAPPKT;

#pragma pack(1)
struct eap_header {
	unsigned char eap_code;
	unsigned char eap_identifier;
	unsigned short eap_length;
	unsigned char eap_type;
};
#pragma pack( )

#pragma pack(1)
struct eap_hdr {
	unsigned char code;
	unsigned char identifier;
	unsigned short length; /* including code and identifier; network byte order */
	/* followed by length-4 octets of data */
} ;
#pragma pack( )

typedef struct _EAPTLSPKT
{
	e8021XPKT auth;
	unsigned char code;
	unsigned char id;
	unsigned short len2;
	unsigned char type2;
	unsigned char flags;
	
}EAPTLSPKT;

typedef struct {
	unsigned char OpCode;
	unsigned char MS_CHAPv2_ID;
	unsigned short MS_Length;
	unsigned char Value_Size;
	unsigned char Challenge[16];
}MSCHV2_CHAN;

typedef struct {
	unsigned char *AuthenticatorChallenge;
	unsigned char *PeerChallenge;
	unsigned char *NtResponse;
	unsigned char *keyingMaterial;
	unsigned char MS_CHAPv2_ID;
	unsigned char eap_fast_mode;
	char *password;
}MSCHV2_VARS;

typedef struct {
	unsigned char OpCode;
	unsigned char MS_CHAPv2_ID;
	unsigned short MS_Length;
	unsigned char Value_Size;
	unsigned char Peer_Challenge[16];
	unsigned char Reserved[8];
	unsigned char NT_Response[24];
	unsigned char Flags;
}MSCHV2_RES;

typedef struct {	
	unsigned short tlv_type;
	unsigned short tlv_length;
} PEAP_TLV_HEADER;

typedef struct {	
	unsigned char reserved;
	unsigned char version;
	unsigned char recvVersion;
	unsigned char subType;
	unsigned char nonce[32];
	unsigned char compoundMac[20];
} PEAP_TLV_CRYPTOBINDING_DATA;

void EAPTask(void *data);
void initEAPSettings();
void eapinit();
void eapfree();
void recvEAPReq(EAPPKT **pkt, int len);
void sendEAPRep(EAPPKT *rxpkt, unsigned char type2);
void sendEAPRepId(EAPPKT *rxpkt, unsigned char type2);
int recvEAPReqTLS(EAPPKT **rxpkt, unsigned char type2, int len);
void sendEAPClientHello(EAPPKT *rxpkt, unsigned char type2);
void addEAPTLSRecord(unsigned char *p, unsigned char type, unsigned short len);
void addEAPClientHello(unsigned char *p);
int doEAPClientHandshake(EAPPKT **rxpkt, int len);
int parseEAPRecordLayer(unsigned char *p);
int parseEAPServerHello(unsigned char *p, unsigned int recvlen);
int parseEAPServerCertificate(unsigned char *p, unsigned int recvlen);
int parseEAPCertificateReq(unsigned char *p, unsigned int recvlen);
int parseEAPServerHelloDone(unsigned char *p, unsigned int recvlen);
int parseEAPChangeCipherSpec(unsigned char *p, unsigned int recvlen);
int parseEAPServerFinish(unsigned char *p, unsigned int recvlen);
void sendEAPClientHello(EAPPKT *rxpkt, unsigned char type2);
int sendEAPCertificateSequence(unsigned char id, unsigned char type2);
int addEAPCertificate(unsigned char **p);
int addEAPCliKeyXhg(unsigned char **p);
int addEAPCertificateVerify(unsigned char **p);
void addEAPChangeCipherspec(unsigned char **p);
int addEAPClientFinish(unsigned char **p);
void ssl_calc_verify( unsigned char hash[36] );
void sendEAPTLSPkt(unsigned char *buf, unsigned int totallen, unsigned short authlen, unsigned char flags, unsigned char id);
int parseTlsData(EAPPKT* rxpkt, unsigned char *p, unsigned int recvlen);
void sendTlsData(unsigned char **p, unsigned char *data, unsigned int datalen, unsigned char id);
void eapmschapv2_challenge(MSCHV2_CHAN *challenge, MSCHV2_VARS *myvars);
void NtPasswordHash(char *Password, char *PasswordHash);
void ChallengeHash(char *PeerChallenge, char *AuthenticatorChallenge, char *UserName, char *Challenge);
int eapmschapv2_success(unsigned char *p, MSCHV2_VARS *myvars);
void eapmschapv2_challenge_resp(MSCHV2_CHAN *challenge, MSCHV2_VARS *myvars, unsigned char id, unsigned char *sendbuf);
void eapmschapv2_success_resp(MSCHV2_VARS *myvars, unsigned char id, unsigned char *sendbuf);
void do_peap_version0(unsigned char *p, int len, unsigned char id);
void do_peap_version1(unsigned char *p, int len, unsigned char id, unsigned char eframe);
void peap_extensions_process(unsigned char *p, int len, unsigned char *out, unsigned short * out_size, MSCHV2_VARS* myvars);
void peap_soh_extensions_process(unsigned char *p, int len, unsigned char *out, unsigned short * out_size, unsigned char id);
void peap_extensions_process_result_tlv(unsigned char * in, unsigned short in_size, unsigned char * out, unsigned short * out_size);
#endif
