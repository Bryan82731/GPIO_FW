#ifndef KERBEROS_H
#define KERBEROS_H

#include "krb.h"

#define KG_USAGE_SEAL 22
#define KG_USAGE_SIGN 23
#define KG_USAGE_SEQ  24
OM_uint32 krb5_gss_delete_sec_context(OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_buffer_t output_token);

unsigned int krb5_gss_acquire_cred(OM_uint32 *minor_status, gss_name_t desired_name, OM_uint32 time_req, gss_OID_set desired_mechs,
								   gss_cred_usage_t cred_usage, gss_cred_id_t *cred, gss_OID_set *actual_mechs, OM_uint32 *time_rec);

OM_uint32 gss_accept_sec_context(OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_cred_id_t server_creds, 
								 gss_buffer_t input_token_buffer, gss_channel_bindings_t input_chan_bindings, 
								 gss_name_t *src_name, gss_OID *mech_type, gss_buffer_t output_token, OM_uint32 *ret_flags, 
								 OM_uint32 *time_rec, gss_cred_id_t *d_cred);

OM_uint32 gssint_get_mech_type( gss_OID	OID,gss_buffer_t token);

krb5_error_code krb5_get_host_realm(char *host, char **realmsp);

krb5_error_code krb5_build_principal(krb5_principal * princ, unsigned int rlen, const char* realm, char* sname, char* host);

krb5_error_code rtk_krb5_sname_to_principal(char *hostname, char *sname,krb5_int32 type, krb5_principal *ret_princ);

unsigned int rtk_krb5_gss_import_name(unsigned int *minor_status, gss_buffer_t input_name_buffer, gss_OID input_name_type, gss_name_t *output_name);

void krb5_free_address(krb5_context context, krb5_address *val);

krb5_error_code krb5_auth_con_free(krb5_context context, krb5_auth_context auth_context);

krb5_error_code krb5_auth_con_setrcache(krb5_context context, krb5_auth_context auth_context, krb5_rcache rcache);

void krb5_free_ticket(krb5_context context, krb5_ticket *val);

void krb5_free_context(krb5_context ctx);

void krb5_free_authenticator_contents(krb5_context context, krb5_authenticator *val);

void krb5_free_keyblock(krb5_context context, register krb5_keyblock *val);

void krb5_free_checksum(krb5_context context, register krb5_checksum *val);

void krb5_free_authenticator(krb5_context context, krb5_authenticator *val);

void krb5_free_data_contents(krb5_context context, krb5_data *val);

void krb5_free_authdata(krb5_context context, krb5_authdata **val);

void krb5_free_principal(krb5_context context, krb5_principal val, int type);

void krb5int_c_free_keyblock_contents(krb5_context context, register krb5_keyblock *key);

gss_cred_id_t gssint_get_mechanism_cred(gss_union_cred_t union_cred, gss_OID mech_type);

OM_uint32 krb5_gss_release_cred(OM_uint32 *minor_status, gss_cred_id_t *cred_handle);

OM_uint32 krb5_gss_release_name(OM_uint32 *minor_status, gss_name_t *input_name);

krb5_error_code krb5_copy_principal(krb5_context context, krb5_const_principal inprinc, krb5_principal *outprinc);

OM_uint32 gssint_create_copy_buffer(const gss_buffer_t srcBuf, gss_buffer_t *destBuf, int addNullChar);

OM_uint32 gssint_import_internal_name ( OM_uint32 *minor_status, gss_OID mech_type, gss_union_name_t union_name, gss_name_t *internal_name);

OM_uint32 gss_release_buffer ( OM_uint32 *minor_status, gss_buffer_t buffer);

krb5_error_code krb5_c_random_make_octets(krb5_context context, krb5_data *data);
 
#endif
