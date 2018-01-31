#ifndef _GSSAPI_INCLUDED
#define _GSSAPI_INCLUDED

#include "krb.h"
#include "rtskt.h"

#define	MSO_BIT (8*(sizeof (int) - 1))  /* Most significant octet bit */

static struct gss_config krb5_mechanism = {
    100, "kerberos_v5",
    { GSS_MECH_KRB5_OID_LENGTH, GSS_MECH_KRB5_OID }
};

static gss_mechanism krb5_mech_configs[] = {
    &krb5_mechanism, /*&krb5_mechanism_old, &krb5_mechanism_wrong, */0
};

   // int     export;


gss_mechanism gssint_get_mechanism(gss_OID oid);

unsigned int gssint_der_length_size(unsigned int len);

int gssint_put_der_length(unsigned int length, unsigned char **buf, unsigned int max_len);

int gssint_get_der_length(unsigned char **buf, unsigned int buf_len, unsigned int *bytes);
    
void updateMechList(void);

gss_mech_info searchMechList(const gss_OID oid);

OM_uint32 gss_unwrap(OM_uint32 *minor_status, gss_ctx_id_t context_handle, gss_buffer_t input_message_buffer, 
					 gss_buffer_t output_message_buffer, int *conf_state, gss_qop_t *qop_state);

OM_uint32 gss_get_mic (OM_uint32 *minor_status, gss_ctx_id_t context_handle, gss_qop_t qop_req, 
					   gss_buffer_t message_buffer, gss_buffer_t msg_token);

OM_uint32 gss_release_cred(OM_uint32 *minor_status, gss_cred_id_t *cred_handle);

OM_uint32 gss_test_oid_set_member(OM_uint32	*minor_status, gss_OID member, gss_OID_set set, int *present);

OM_uint32 gss_test_oid_set_member(OM_uint32 *minor_status, gss_OID member, gss_OID_set set, int *present);

OM_uint32 gss_add_oid_set_member(OM_uint32 *minor_status, gss_OID member_oid, gss_OID_set *oid_set);

OM_uint32 gss_wrap(OM_uint32 *minor_status, gss_ctx_id_t context_handle, int conf_req_flag, gss_qop_t qop_req, 
				   gss_buffer_t input_message_buffer, int *conf_state, gss_buffer_t output_message_buffer);

OM_uint32 gss_process_context_token (OM_uint32 *minor_status, gss_ctx_id_t context_handle, gss_buffer_t token_buffer);

OM_uint32 gss_context_time (OM_uint32 *minor_status, gss_ctx_id_t context_handle, OM_uint32 *time_rec);

OM_uint32 gss_export_sec_context(OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_buffer_t interprocess_token);

OM_uint32 gss_import_sec_context(OM_uint32 *minor_status, gss_buffer_t interprocess_token, gss_ctx_id_t *context_handle);

OM_uint32 gss_inquire_context(OM_uint32 *minor_status, gss_ctx_id_t	context_handle, gss_name_t *src_name, gss_name_t *targ_name, 
							  OM_uint32 *lifetime_rec, gss_OID *mech_type, OM_uint32 *ctx_flags, int *locally_initiated, int *open);

OM_uint32 gss_create_empty_oid_set(OM_uint32 *minor_status, gss_OID_set *oid_set);



OM_uint32 gssint_convert_name_to_union_name(OM_uint32 *minor_status, gss_mechanism	mech, gss_name_t internal_name, 
    gss_name_t	*external_name);

OM_uint32 generic_gss_add_oid_set_member(OM_uint32 *minor_status, const gss_OID_desc *const member_oid, gss_OID_set *oid_set);



OM_uint32 gss_release_oid(OM_uint32 *minor_status, gss_OID *oid);

extern OM_uint32 gss_import_name(OM_uint32 * minor_status, gss_buffer_t input_name_buffer, gss_OID input_name_type, 

								 gss_name_t *output_name);

OM_uint32 gss_release_name(OM_uint32 *minor_status, gss_name_t *input_name);
OM_uint32 generic_gss_copy_oid(	OM_uint32	*minor_status, const gss_OID_desc * const oid, gss_OID *new_oid);

OM_uint32 gssint_release_internal_name(OM_uint32 *minor_status, gss_OID mech_type, gss_name_t *internal_name);

OM_uint32 rtk_gss_release_oid_set(OM_uint32 *minor_status, gss_OID_set *set);


OM_uint32 generic_gss_release_oid(OM_uint32	* minor_status, gss_OID	*oid, int releaseElement);

OM_uint32 gss_delete_sec_context (OM_uint32 *minor_status,gss_ctx_id_t *context_handle,gss_buffer_t		output_token);


OM_uint32 gss_unseal(OM_uint32 *minor_status, gss_ctx_id_t context_handle, gss_buffer_t	input_message_buffer, 


					 gss_buffer_t output_message_buffer, int *conf_state, int *qop_state);

OM_uint32 gss_wrap_size_limit(OM_uint32 *minor_status, gss_ctx_id_t	context_handle, int	conf_req_flag, gss_qop_t qop_req, 
							  OM_uint32 req_output_size, OM_uint32 *max_input_size);

OM_uint32 gss_seal(OM_uint32 *minor_status, gss_ctx_id_t context_handle, int conf_req_flag, int qop_req, 
				   gss_buffer_t input_message_buffer, int *conf_state, gss_buffer_t output_message_buffer);

OM_uint32 gss_sign(OM_uint32 *minor_status, gss_ctx_id_t context_handle, int qop_req, gss_buffer_t message_buffer, 
	gss_buffer_t msg_token);



OM_uint32 gssint_create_copy_buffer(const gss_buffer_t srcBuf, gss_buffer_t *destBuf, int addNullChar);

OM_uint32 gssint_import_internal_name ( OM_uint32 *minor_status, gss_OID mech_type, gss_union_name_t union_name, 
									   gss_name_t *internal_name);

OM_uint32 gss_release_buffer ( OM_uint32 *		minor_status, gss_buffer_t		buffer);

#endif
