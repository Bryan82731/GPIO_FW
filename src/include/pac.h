#ifndef _PAC_DEFINED
#define _PAC_DEFINED
#include "krb.h"

krb5_error_code krb5_pac_verify(krb5_context context, const krb5_pac pac, krb5_timestamp authtime, krb5_const_principal principal, 

								const krb5_keyblock *server, const krb5_keyblock *privsvr);
krb5_error_code mspac_verify(krb5_context kcontext, krb5_pac *pac, const krb5_keyblock *key, const krb5_ticket *ticket);
krb5_error_code krb5_pac_parse(krb5_context context, void *ptr, int len, krb5_pac *ppac);
             //krb5_authdata_context context,
             //void *plugin_context,
             //struct mspac_context *request_context,
             //const krb5_auth_context *auth_context,


void krb5_pac_free(krb5_context context, krb5_pac pac);
krb5_error_code krb5_pac_init(krb5_context context, krb5_pac *ppac, unsigned int cbuffers, unsigned char *p);
krb5_error_code krb5_c_make_checksum(krb5_context context, krb5_cksumtype cksumtype, const krb5_keyblock *key, 
									 krb5_keyusage usage, const krb5_data *input, krb5_checksum *cksum);


#endif
