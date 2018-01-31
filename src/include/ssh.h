#ifndef _SSH_INCLUDED
#define _SSH_INCLUDED
#include <time.h>
#include "rtskt.h"
#include "lib.h"
#include "rsa.h"
#include "aes.h"

/* SHA1 is 20 bytes == 160 bits */
#define SHA1_HASH_SIZE 20
#define SHA1_96_HASH_SIZE 12
/* SHA512 is 64 bytes == 512 bits */
#define SHA512_HASH_SIZE 64

/* AES requires a 16 byte key and >= SHA_HASH_SIZE*/
#define MAX_KEY_LEN 20
#define MAX_IV_LEN 20 /* must be same as max blocksize, 
						 and >= SHA1_HASH_SIZE */
#define MAX_MAC_KEY 20

/* service types */
#define SSH_SERVICE_USERAUTH "ssh-userauth"
#define SSH_SERVICE_USERAUTH_LEN 12
#define SSH_SERVICE_CONNECTION "ssh-connection"
#define SSH_SERVICE_CONNECTION_LEN 14


#define RECV_MAXWINDOW 6000 /* tweak */
#define RECV_MAXPACKET 1400 /* tweak */

#define PACKET_PADDING_OFF 4
#define PACKET_PAYLOAD_OFF 5

/* message numbers */
#define SSH_MSG_DISCONNECT             1
#define SSH_MSG_IGNORE                 2
#define SSH_MSG_UNIMPLEMENTED          3
#define SSH_MSG_DEBUG                  4
#define SSH_MSG_SERVICE_REQUEST        5
#define SSH_MSG_SERVICE_ACCEPT         6
#define SSH_MSG_KEXINIT                20
#define SSH_MSG_NEWKEYS                21
#define SSH_MSG_KEXDH_INIT             30
#define SSH_MSG_KEXDH_REPLY            31

/* userauth message numbers */
#define SSH_MSG_USERAUTH_REQUEST            50
#define SSH_MSG_USERAUTH_FAILURE            51
#define SSH_MSG_USERAUTH_SUCCESS            52
#define SSH_MSG_USERAUTH_BANNER             53
#define SSH_MSG_USERAUTH_PK_OK				60

/* connect message numbers */
#define SSH_MSG_GLOBAL_REQUEST                  80
#define SSH_MSG_REQUEST_SUCCESS                 81
#define SSH_MSG_REQUEST_FAILURE                 82
#define SSH_MSG_CHANNEL_OPEN                    90
#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION       91 
#define SSH_MSG_CHANNEL_OPEN_FAILURE            92
#define SSH_MSG_CHANNEL_WINDOW_ADJUST           93
#define SSH_MSG_CHANNEL_DATA                    94
#define SSH_MSG_CHANNEL_EXTENDED_DATA           95
#define SSH_MSG_CHANNEL_EOF                     96
#define SSH_MSG_CHANNEL_CLOSE                   97
#define SSH_MSG_CHANNEL_REQUEST                 98
#define SSH_MSG_CHANNEL_SUCCESS                 99
#define SSH_MSG_CHANNEL_FAILURE                 100

#define AUTH_TYPE_PUBKEY	1 << 0
#define AUTH_TYPE_PASSWORD	1 << 1

/* auth types, "none" means we should return list of acceptable types */
#define AUTH_METHOD_NONE	"none"
#define AUTH_METHOD_NONE_LEN 4
#define AUTH_METHOD_PUBKEY "publickey"
#define AUTH_METHOD_PUBKEY_LEN 9
#define AUTH_METHOD_PASSWORD "password"
#define AUTH_METHOD_PASSWORD_LEN 8

#define CHANNEL_ID_NONE 0
#define CHANNEL_ID_SESSION 1
#define CHANNEL_ID_X11 2

#define SSH_SIGNKEY_RSA "ssh-rsa"
#define SSH_SIGNKEY_RSA_LEN 7

#define STORE32H(x, y)                                                                     \
     { (y)[0] = (unsigned char)(((x)>>24)&255); (y)[1] = (unsigned char)(((x)>>16)&255);   \
       (y)[2] = (unsigned char)(((x)>>8)&255); (y)[3] = (unsigned char)((x)&255); }

#define LOAD32H(x, y)                            \
     { x = ((unsigned long)((y)[0] & 255)<<24) | \
           ((unsigned long)((y)[1] & 255)<<16) | \
           ((unsigned long)((y)[2] & 255)<<8)  | \
           ((unsigned long)((y)[3] & 255)); }


#undef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#undef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))

#define NVT_CTL_OPEN	0
#define NVT_CTL_SE		240
#define NVT_CTL_IP		244
#define NVT_CTL_SB		250
#define NVT_CTL_WILL	251
#define NVT_CTL_WONT	252
#define NVT_CTL_DO		253
#define NVT_CTL_DONT	254
#define NVT_CTL_IAC		255
#define NVT_CTL_CLOSE	256
#define NVT_CTL_CHAR	0x80
#define MAX_TLENET_BUF_SIZE 256
#define TEL_OPT_BIN				0
#define TEL_OPT_ECHO			1
#define TEL_OPT_GA				3
#define TEL_OPT_STATUS			5
#define TEL_OPT_TERM_TYPE		24
#define TEL_OPT_WIN_SIZE		31
#define TEL_OPT_LINE_MODE		34
#define TEL_INIT			0
#define TEL_LOGIN			1
#define TEL_GET_USERNAME	2
#define TEL_PWD				3
#define TEL_GET_PASSWORD	4
#define TEL_AUTH_PASS		5

#define TEL_DEFAULT_MODE	0
#define TEL_CHAR_MODE		1
#define TEL_LINE_MODE		2

enum
{
    SSH_INIT = 0x0,
	SRV_SSH_PROTOCOL_EX = 0x1,
	SRV_SSH_KEY_INIT_EX = 0x2,
	CLI_SSH_KEY_INIT_EX = 0x3,
	DH_KEX_INIT = 0x4,
	DH_KEX_REPLY = 0x5,
	CLI_SSH_NEW_KEY = 0x6,
	CLI_SSH_SRV_REQ = 0x7,
	CLI_SSH_USERAUTH_REQ = 0x8,
	CLI_SSH_CHAN_OPEN = 0x9,
	CLI_SSH_CHAN_REQ = 0x0a,
	CLI_SSH_CHAN_DATA = 0x0b,
	CLI_SSH_CHAN_EOF = 0x0c,
	CLI_SSH_CHAN_CLOSE = 0x0d,
};

typedef struct buf 
{
	unsigned char * data;
	unsigned int len; /* the used size */
}buffer;

struct key_context {
	unsigned char recv_algo_crypt[16];	
	unsigned char trans_algo_crypt[16];
	unsigned char *recv_IV;
	unsigned char *trans_IV;	
	unsigned char recvmackey[MAX_MAC_KEY];
	unsigned char transmackey[MAX_MAC_KEY];
};
typedef struct _SSH SSH;

typedef struct _TelnetSession
{
	RTSkt* cs;
	UserInfo *uInfo;
	unsigned short nvtCTL;
	unsigned short optBufOffset;
	unsigned short sendBufOffset;
	unsigned short cmdBufOffset;
	unsigned short wsWidth;
	unsigned short wsHeigh;
	unsigned char sendBuf[MAX_TLENET_BUF_SIZE];
	unsigned char optBuf[MAX_TLENET_BUF_SIZE];
	unsigned char cmdBuf[MAX_TLENET_BUF_SIZE];
	unsigned char srvWillEcho:1, cliWillTermType:1, cliWillWinSize:1;
	unsigned char state:4, vt100:1, ANSI:1, echo:1, shell:1;
	SSH *ssh;
}TelnetSession;

typedef struct _Chan
{
	unsigned int senderChanId;
	unsigned int recvChanId;
	unsigned int winSize;
	unsigned int maxPktSize;
	unsigned char type; //session
	unsigned int termw;
	unsigned int termh;
	unsigned int termc;
	unsigned int termr; /* width, height, col, rows */
	unsigned char sentclosed:1, transeof:1, recveof:1;
	TelnetSession session;
}Channel;

struct _SSH
{	
	unsigned char state:4, banner:1, failcount:3;
	unsigned int transseq, recvseq; /* Sequence IDs */
	unsigned char hash[20]; /* the hash*/
	unsigned char *rxbuf;
	unsigned char *session_id; /*this is the hash from the first kex*/
	unsigned char *remoteident;
	unsigned char authtypes:4, authdone:1;	
	unsigned char macsize;
	unsigned char keysize;	
	unsigned short connecttime;
	char *username;	
	unsigned char *decryptedpkt;
	int decryptedlen;
	struct key_context *newkeys;	
	struct key_context *keys;		
	rsa_context *dphostkey;
	buffer* kexhashbuf; /* session hash buffer calculated from various packets*/
	buffer* srvkeyinit; /* session hash buffer calculated from various packets*/
	mpi *K;/* SSH_MSG_KEXDH_REPLY and sending SSH_MSH_NEWKEYS */
	Channel *chan;
	RTSkt *cs;	
};

typedef struct _SSHTranLayerHdr
{
	int length;
	char padLen;
}SSHTranLayerHdr;


buffer * buf_new(unsigned int size);
void buf_free(buffer* buf);

/* Used to force mp_ints to be initialised */

//typedef char               mp_sign;
//typedef long unsigned int     mp_digit;  /* 2 byte type */
//typedef unsigned int       mp_word;   /* 4 byte type */
//typedef unsigned int       mp_size;
//int doSSHAccept(unsigned char *pktbuf, int pktlen, SSH *ssh);

int sshEncodeData(PKT *pkt, int len, SSH *ssh);

int sshEncodeBanner(PKT *pkt, int len, SSH *ssh);

int send_msg_channel_close(SSH *ssh);

void setInt(unsigned char *dst, unsigned int len);

void SSHFree(SSH **ssh);

void sshSrv(void *data);

PKT* sshrx(SSH **ssh, RTSkt* cs, unsigned char iskexauth, INT32S* status);

#endif
