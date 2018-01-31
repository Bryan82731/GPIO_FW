#ifndef _SPNEGO_INCLUDED
#define _SPNEGO_INCLUDED

#include "krb.h"
#include "rtskt.h"


#define	SEC_CONTEXT_TOKEN 1
#define	SPNEGO_SIZE_OF_INT 4

#define	ACCEPT_COMPLETE 0
#define	ACCEPT_INCOMPLETE 1
#define	REJECT 2
#define REQUEST_MIC 3
#define	ACCEPT_DEFECTIVE_TOKEN 0xffffffffUL

#define	MECH_OID		0x06
#define	OCTET_STRING		0x04
#define	CONTEXT			0xa0
#define	SEQUENCE		0x30
#define	SEQUENCE_OF		0x30
#define	BIT_STRING		0x03
#define	BIT_STRING_LENGTH	0x02
#define	BIT_STRING_PADDING	0x01
#define	ENUMERATED		0x0a
#define	ENUMERATION_LENGTH	1
#define	HEADER_ID		0x60

/*
 * The magic number must be less than a standard pagesize
 * to avoid a possible collision with a real address.
 */
#define	SPNEGO_MAGIC_ID  0x00000fed

/*
 * SPNEGO specific error codes (minor status codes)
 */
#define	ERR_SPNEGO_NO_MECHS_AVAILABLE		0x20000001
#define	ERR_SPNEGO_NO_CREDS_ACQUIRED		0x20000002
#define	ERR_SPNEGO_NO_MECH_FROM_ACCEPTOR	0x20000003
#define	ERR_SPNEGO_NEGOTIATION_FAILED		0x20000004
#define	ERR_SPNEGO_NO_TOKEN_FROM_ACCEPTOR	0x20000005

/*
 * send_token_flag is used to indicate in later steps what type
 * of token, if any should be sent or processed.
 * NO_TOKEN_SEND = no token should be sent
 * INIT_TOKEN_SEND = initial token will be sent
 * CONT_TOKEN_SEND = continuing tokens to be sent
 * CHECK_MIC = no token to be sent, but have a MIC to check.
 * ERROR_TOKEN_SEND = error token from peer needs to be sent.
 */

typedef	enum {NO_TOKEN_SEND, INIT_TOKEN_SEND, CONT_TOKEN_SEND,
		CHECK_MIC, ERROR_TOKEN_SEND} send_token_flag;

/*
 * The Mech OID:
 * { iso(1) org(3) dod(6) internet(1) security(5)
 *  mechanism(5) spnego(2) }
 */

#define	SPNEGO_OID_LENGTH 6
#define	SPNEGO_OID "\053\006\001\005\005\002"
struct _work_plan
{
    RTSkt*    s;
    gss_cred_id_t server_creds;
};

typedef void *spnego_token_t;

/* spnego name structure for internal representation. */
typedef struct 
{
	gss_OID type;
	gss_buffer_t buffer;
	gss_OID	mech_type;
	gss_name_t	mech_name;
} spnego_name_desc, *spnego_name_t;

/* Structure for context handle */
typedef struct 
{
	OM_uint32	magic_num;
	gss_buffer_desc DER_mechTypes;
	gss_OID internal_mech;
	gss_ctx_id_t ctx_handle;
	char  *optionStr;
	gss_cred_id_t default_cred;
	int mic_reqd;
	int mic_sent;
	int mic_rcvd;
	int firstpass;
	int mech_complete;
	int nego_done;
	OM_uint32 ctx_flags;
	gss_name_t internal_name;
	gss_OID actual_mech;
} spnego_gss_ctx_id_rec, *spnego_gss_ctx_id_t;


/*
 * The magic number must be less than a standard pagesize
 * to avoid a possible collision with a real address.
 */
/* SPNEGO oid structure */
static const gss_OID_desc spnego_oids[] = 
{
	{SPNEGO_OID_LENGTH, SPNEGO_OID},
};

static const gss_OID_set_desc spnego_oidsets[] = 
{
	{1, (gss_OID) spnego_oids+0},
};

#define	g_OID_equal(o1, o2) (((o1)->length == (o2)->length) && (memcmp((o1)->elements, (o2)->elements, (o1)->length) == 0))

unsigned int rtk_gss_import_name(OM_uint32 * minor_status,  gss_buffer_t input_name_buffer, 
								 gss_OID input_name_type,  gss_name_t *output_name);

unsigned int rtk_gss_acquire_cred(OM_uint32 *minor_status, gss_name_t desired_name, OM_uint32 time_req, gss_OID_set desired_mechs, 
								  int cred_usage, gss_cred_id_t *output_cred_handle, gss_OID_set *actual_mechs, OM_uint32 *time_rec);

gss_mechanism rtk_gss_get_mechanism(gss_OID oid);

unsigned int rtk_gss_add_cred(unsigned int *minor_status, gss_cred_id_t input_cred_handle, gss_name_t desired_name, 
							  gss_OID desired_mech, gss_cred_usage_t cred_usage, unsigned int initiator_time_req, 
							  unsigned int acceptor_time_req, gss_cred_id_t *output_cred_handle, gss_OID_set *actual_mechs, 
							  unsigned int *initiator_time_rec, unsigned int *acceptor_time_rec);

unsigned int rtk_gss_import_internal_name(OM_uint32 *minor_status, gss_OID mech_type, gss_union_name_t union_name, gss_name_t *internal_name);

OM_uint32 rtk_spnego_gss_accept_sec_context(OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_cred_id_t server_creds,
											gss_buffer_t input_token, gss_channel_bindings_t input_chan_bindings, gss_name_t *src_name, 
											gss_OID *mech_type, gss_buffer_t output_token, OM_uint32 *ret_flags, OM_uint32 *time_rec, 
											gss_cred_id_t *delegated_cred_handle);

int rtk_gssint_get_der_length(unsigned char **buf, unsigned int buf_len, unsigned int *bytes);

OM_uint32 rtk_spnego_gss_delete_sec_context(OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_buffer_t output_token);

OM_uint32 rtk_spnego_gss_unseal(void *context, OM_uint32 *minor_status, gss_ctx_id_t context_handle, gss_buffer_t input_message_buffer, 
					  gss_buffer_t output_message_buffer, int *conf_state, int *qop_state);

OM_uint32 rtk_spnego_gss_seal(void *context, OM_uint32 *minor_status, gss_ctx_id_t context_handle, int conf_req_flag, int qop_req, 
					gss_buffer_t input_message_buffer, int *conf_state, gss_buffer_t output_message_buffer);

OM_uint32 rtk_spnego_gss_import_name(void *ctx, OM_uint32 *minor_status, gss_buffer_t input_name_buffer, 
									 gss_OID input_name_type, gss_name_t *output_name);

OM_uint32 rtk_spnego_gss_release_name(void *ctx, OM_uint32 *minor_status, gss_name_t *input_name);

OM_uint32 rtk_spnego_gss_inquire_context(void *context, OM_uint32 *minor_status, const gss_ctx_id_t context_handle, 
										 gss_name_t *src_name, gss_name_t *targ_name, OM_uint32 *lifetime_rec, 
										 gss_OID *mech_type, OM_uint32 *ctx_flags, int *locally_initiated, int *open);

OM_uint32 rtk_spnego_gss_wrap_size_limit(void *context, OM_uint32 *minor_status, const gss_ctx_id_t context_handle, int conf_req_flag,
										 gss_qop_t qop_req, OM_uint32 req_output_size, OM_uint32 *max_input_size);

OM_uint32 rtk_spnego_gss_verify(void *context, OM_uint32 *minor_status, const gss_ctx_id_t context_handle, const gss_buffer_t msg_buffer,
								const gss_buffer_t token_buffer, int *qop_state);

OM_uint32 rtk_acc_ctx_call_acc(OM_uint32 *minor_status, spnego_gss_ctx_id_t sc, gss_cred_id_t cred, gss_buffer_t mechtok_in, 
							   gss_OID *mech_type, gss_buffer_t mechtok_out, OM_uint32 *ret_flags, OM_uint32 *time_rec, 
							   gss_cred_id_t *delegated_cred_handle, OM_uint32 *negState, send_token_flag *tokflag);

krb5_error_code krb5_auth_con_free(krb5_context context, krb5_auth_context auth_context);

OM_uint32 krb5_gss_delete_sec_context(OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_buffer_t output_token);

OM_uint32 rtk_gss_verify_mic (OM_uint32 *minor_status, gss_ctx_id_t context_handle, gss_buffer_t message_buffer, 
							  gss_buffer_t token_buffer, gss_qop_t *qop_state);

OM_uint32 rtk_gss_verify (OM_uint32 *minor_status, gss_ctx_id_t	context_handle, gss_buffer_t message_buffer, gss_buffer_t token_buffer, 
						  int *qop_state);

int put_input_token(unsigned char **buf_out, gss_buffer_t input_token, unsigned int buflen);

int put_mech_oid(unsigned char **buf_out, const gss_OID mech, unsigned int buflen);

int put_negResult(unsigned char **buf_out, OM_uint32 negResult, unsigned int buflen);

OM_uint32 acc_ctx_vfy_oid(OM_uint32 *minor_status, spnego_gss_ctx_id_t sc, gss_OID mechoid, OM_uint32 *negState, send_token_flag *tokflag);

int rtk_server_acquire_creds(char *service_name, gss_cred_id_t *server_creds);

gss_OID_set rkt_get_mech_set(unsigned int *minor_status, unsigned char **buff_in, unsigned int buff_length);

int rtk_spnego_sign_server(RTSkt * s, gss_cred_id_t server_creds, gss_buffer_t auth);

int spnego_worker_bee(void *param, gss_buffer_t auth);

void spnego(void *data);

#endif
