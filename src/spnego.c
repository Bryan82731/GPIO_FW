#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lib.h"
#include "b64.h"
#include "gssapi.h"
#include "kerberos.h"
#include "spnego.h"

const gss_OID_desc * gss_mech_spnego = spnego_oids+0;
extern const gss_OID_set_desc * const gss_mech_set_krb5_both;

extern DPCONF *dpconf;

extern RTSkt *stopconn(RTSkt* cs);

extern gss_mechanism spnego_mech_configs[];

extern gss_mech_info g_mechList;
extern gss_mech_info g_mechListTail;

extern gss_OID GSS_C_NT_USER_NAME;
extern gss_OID gss_nt_user_name;

extern gss_OID GSS_C_NT_MACHINE_UID_NAME;
extern gss_OID gss_nt_machine_uid_name;

extern gss_OID GSS_C_NT_STRING_UID_NAME;
extern gss_OID gss_nt_string_uid_name;

extern gss_OID GSS_C_NT_HOSTBASED_SERVICE_X;
extern gss_OID gss_nt_service_name_v2;

extern gss_OID GSS_C_NT_HOSTBASED_SERVICE;
extern gss_OID gss_nt_service_name;

extern gss_OID GSS_C_NT_ANONYMOUS;

extern gss_OID GSS_C_NT_EXPORT_NAME;
extern gss_OID gss_nt_exported_name;

static int rtk_spnego_server_establish_context(RTSkt* s, gss_cred_id_t server_creds, gss_ctx_id_t *context, gss_buffer_t client_name,
        OM_uint32 *ret_flags, gss_buffer_t auth);
static void rtk_release_spnego_ctx(spnego_gss_ctx_id_t *ctx);

/*
 * The Mech OID for SPNEGO:
 * { iso(1) org(3) dod(6) internet(1) security(5)
 *  mechanism(5) spnego(2) }
 */
static struct gss_config spnego_mechanism =
{
    400, "spnego",
    {SPNEGO_OID_LENGTH, SPNEGO_OID},
    NULL,
    NULL,//spnego_gss_acquire_cred,
    NULL,//spnego_gss_release_cred,
    NULL,//spnego_gss_init_sec_context,
    NULL,
    NULL,				/* gss_process_context_token */
    NULL,	/* gss_delete_sec_context */
    NULL,//spnego_gss_context_time,	/* gss_context_time */
    NULL,		/* gss_sign */
    NULL,		/* gss_verify */
    NULL,		/* gss_seal */
    NULL,		/* gss_unseal */
    NULL,//spnego_gss_display_status,
    NULL,				/* gss_indicate_mechs */
    NULL,				/* gss_compare_name */
    NULL,//spnego_gss_display_name,
    NULL,
    NULL,
    NULL,				/* gss_inquire_cred */
    NULL,				/* gss_add_cred */
    NULL,	/* gss_export_sec_context */
    NULL,	/* gss_import_sec_context */
    NULL, 				/* gss_inquire_cred_by_mech */
    NULL,//spnego_gss_inquire_names_for_mech,
    NULL,	/* gss_inquire_context */
    NULL,				/* gss_internal_release_oid */
    NULL,	/* gss_wrap_size_limit */
    NULL,				/* gss_export_name */
    NULL,				/* gss_store_cred */
};

gss_mechanism spnego_mech_configs[] =
{
    &spnego_mechanism, NULL
};

#if 0
OM_uint32 rtk_spnego_gss_wrap_size_limit(void *context, OM_uint32 *minor_status, const gss_ctx_id_t context_handle, int conf_req_flag,
        gss_qop_t qop_req, OM_uint32 req_output_size, OM_uint32 *max_input_size)
{
    OM_uint32 ret;
    ret = gss_wrap_size_limit(minor_status, context_handle, conf_req_flag, qop_req, req_output_size, max_input_size);
    return (ret);
};
#endif

OM_uint32 rtk_spnego_gss_inquire_context(void *context, OM_uint32 *minor_status, const gss_ctx_id_t context_handle,
        gss_name_t *src_name, gss_name_t *targ_name, OM_uint32 *lifetime_rec,
        gss_OID *mech_type, OM_uint32 *ctx_flags, int *locally_initiated, int*open)
{
    OM_uint32 ret = GSS_S_COMPLETE;

    ret = gss_inquire_context(minor_status, context_handle, src_name, targ_name, lifetime_rec, mech_type, ctx_flags,
                              locally_initiated, open);

    return (ret);
}


OM_uint32 rtk_gss_verify(OM_uint32 * minor_status, gss_ctx_id_t context_handle, gss_buffer_t message_buffer,
                         gss_buffer_t token_buffer, int *qop_state)
{
    OM_uint32 status;
    gss_union_ctx_id_t	ctx;
    gss_mechanism mech;

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);
    *minor_status = 0;

    if (context_handle == GSS_C_NO_CONTEXT)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);

    if ((message_buffer == GSS_C_NO_BUFFER) || GSS_EMPTY_BUFFER(token_buffer))
        return (GSS_S_CALL_INACCESSIBLE_READ);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) context_handle;
    mech = gssint_get_mechanism (ctx->mech_type);

    if (mech)
    {
        if (mech->gss_verify)
            status = mech->gss_verify(mech->context, minor_status, ctx->internal_ctx_id, message_buffer, token_buffer, qop_state);
        else
            status = GSS_S_UNAVAILABLE;

        return(status);
    }

    return (GSS_S_BAD_MECH);
}

OM_uint32 rtk_gss_verify_mic(OM_uint32 *minor_status, gss_ctx_id_t context_handle, gss_buffer_t message_buffer,
                             gss_buffer_t token_buffer, gss_qop_t *qop_state)
{
    return (rtk_gss_verify(minor_status, context_handle, message_buffer, token_buffer, (int *) qop_state));
}

#if 0
OM_uint32
rtk_spnego_gss_export_sec_context(void *context, OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_buffer_t interprocess_token)
{
    OM_uint32 ret;
    ret = gss_export_sec_context(minor_status, context_handle, interprocess_token);
    return (ret);
}


OM_uint32
rtk_spnego_gss_import_sec_context(void *context,
                                  OM_uint32		*minor_status,
                                  const gss_buffer_t	interprocess_token,
                                  gss_ctx_id_t		*context_handle)
{
    OM_uint32 ret;
    ret = gss_import_sec_context(minor_status,
                                 interprocess_token,
                                 context_handle);
    return (ret);
}


/*ARGSUSED*/
OM_uint32
rtk_spnego_gss_import_name(void *ctx,
                           OM_uint32 *minor_status,
                           gss_buffer_t input_name_buffer,
                           gss_OID input_name_type,
                           gss_name_t *output_name)
{
    OM_uint32 status;

//	dsyslog("Entering import_name\n");

    status = gss_import_name(minor_status, input_name_buffer,
                             input_name_type, output_name);

//	dsyslog("Leaving import_name\n");
    return (status);
}


/*ARGSUSED*/
OM_uint32
rtk_spnego_gss_release_name(void *ctx,
                            OM_uint32 *minor_status,
                            gss_name_t *input_name)
{
    OM_uint32 status;

    //dsyslog("Entering release_name\n");

    status = gss_release_name(minor_status, input_name);

//	dsyslog("Leaving release_name\n");
    return (status);
}

OM_uint32
rtk_spnego_gss_unseal(void *context,
                      OM_uint32 *minor_status,
                      gss_ctx_id_t context_handle,
                      gss_buffer_t input_message_buffer,
                      gss_buffer_t output_message_buffer,
                      int *conf_state,
                      int *qop_state)
{
    OM_uint32 ret;
    ret = gss_unseal(minor_status,
                     context_handle,
                     input_message_buffer,
                     output_message_buffer,
                     conf_state,
                     qop_state);

    return (ret);
}

OM_uint32
rtk_spnego_gss_seal(void *context,
                    OM_uint32 *minor_status,
                    gss_ctx_id_t context_handle,
                    int conf_req_flag,
                    int qop_req,
                    gss_buffer_t input_message_buffer,
                    int *conf_state,
                    gss_buffer_t output_message_buffer)
{
    OM_uint32 ret;
    ret = gss_seal(minor_status,
                   context_handle,
                   conf_req_flag,
                   qop_req,
                   input_message_buffer,
                   conf_state,
                   output_message_buffer);

    return (ret);
}


OM_uint32
rtk_spnego_gss_verify(void *context,
                      OM_uint32 *minor_status,
                      const gss_ctx_id_t context_handle,
                      const gss_buffer_t msg_buffer,
                      const gss_buffer_t token_buffer,
                      int *qop_state)
{
    OM_uint32 ret;
    ret = rtk_gss_verify_mic(minor_status,
                             context_handle,
                             msg_buffer,
                             token_buffer,
                             (gss_qop_t *)qop_state); /* XXX */
    return (ret);
}
OM_uint32
rtk_spnego_gss_sign(void *context,
                    OM_uint32 *minor_status,
                    const gss_ctx_id_t context_handle,
                    int  qop_req,
                    const gss_buffer_t message_buffer,
                    gss_buffer_t message_token)
{
    OM_uint32 ret;
    ret = gss_sign(minor_status,
                   context_handle,
                   qop_req,
                   message_buffer,
                   message_token);
    return (ret);
}
#endif

OM_uint32 rtk_spnego_gss_delete_sec_context(OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_buffer_t output_token)
{
    OM_uint32 ret = GSS_S_COMPLETE;
    spnego_gss_ctx_id_t *ctx = (spnego_gss_ctx_id_t *)context_handle;

    if (context_handle == NULL)
        return (GSS_S_FAILURE);

    krb5_gss_delete_sec_context(minor_status, &((*ctx)->ctx_handle), NULL);
    //free((*ctx)->ctx_handle);
    /*
     * If this is still an SPNEGO mech, release it locally.
     */
    if (*ctx != NULL && (*ctx)->magic_num == SPNEGO_MAGIC_ID)
    {
        (void) rtk_release_spnego_ctx(ctx);
    }
    else
    {
        ret = gss_delete_sec_context(minor_status, context_handle, output_token);
    }

    return (ret);
}

unsigned int rtk_gss_import_name(OM_uint32 * minor_status, gss_buffer_t input_name_buffer, gss_OID input_name_type, gss_name_t *output_name)
{
    gss_union_name_t union_name;
    union_name = (gss_union_name_t) malloc (sizeof(gss_union_name_desc));
    union_name->loopback = 0;
    union_name->mech_type = 0;
    union_name->mech_name = 0;
    union_name->name_type = 0;
    union_name->external_name = (gss_buffer_t)malloc(sizeof (gss_buffer_desc));
    union_name->external_name->value = input_name_buffer->value;
    union_name->external_name->length = input_name_buffer->length;
    union_name->name_type = (gss_OID) malloc(sizeof(gss_OID_desc));
    union_name->name_type->elements = input_name_type->elements;
    union_name->name_type->length = input_name_type->length;

    union_name->loopback = union_name;
    *output_name = (gss_name_t)union_name;
    return (GSS_S_COMPLETE);
}

unsigned int rtk_gss_acquire_cred(OM_uint32 *minor_status, gss_name_t desired_name, OM_uint32 time_req, gss_OID_set desired_mechs,
                                  int cred_usage, gss_cred_id_t *output_cred_handle, gss_OID_set *actual_mechs, OM_uint32 *time_rec)
{
    unsigned int major = GSS_S_FAILURE;
    OM_uint32 initTimeOut = 0, acceptTimeOut = 0, outTime = GSS_C_INDEFINITE;
    gss_mechanism mech;
    gss_OID_set mechs;
    gss_union_cred_t creds;
    gss_OID_desc default_OID;
    gss_OID_set_desc default_OID_set;

    major = GSS_S_FAILURE;
    if (desired_mechs == GSS_C_NULL_OID_SET)
    {
        mech = gssint_get_mechanism(NULL);

        mechs = &default_OID_set;
        default_OID_set.count = 1;
        default_OID_set.elements = &default_OID;
        default_OID.length = mech->mech_type.length;
        default_OID.elements = mech->mech_type.elements;
    }

    creds = (gss_union_cred_t)malloc(sizeof (gss_union_cred_desc));
    memset(creds, 0, sizeof (gss_union_cred_desc));
    creds->loopback = creds;

    major = rtk_gss_add_cred(minor_status, (gss_cred_id_t)creds, desired_name,
                             &mechs->elements[0], cred_usage, time_req, time_req, NULL,
                             NULL, &initTimeOut, &acceptTimeOut);
    if (major == GSS_S_COMPLETE) {
        /* update the credential's time */
        if (cred_usage == GSS_C_ACCEPT)
        {
            if (outTime > acceptTimeOut)
                outTime = acceptTimeOut;
        }
        else if (cred_usage == GSS_C_INITIATE)
        {
            if (outTime > initTimeOut)
                outTime = initTimeOut;
        }
        else
        {
            /*
             * time_rec is the lesser of the
             * init/accept times
             */
            if (initTimeOut > acceptTimeOut)
                outTime = (outTime > acceptTimeOut) ? acceptTimeOut : outTime;
            else
                outTime = (outTime > initTimeOut) ? initTimeOut : outTime;
        }
    }
    creds->loopback = creds;
    *output_cred_handle = (gss_cred_id_t)creds;
    return (GSS_S_COMPLETE);

}

unsigned int rtk_gss_add_cred(unsigned int *minor_status, gss_cred_id_t creds, gss_name_t desired_name, gss_OID desired_mech,
                              gss_cred_usage_t cred_usage, unsigned int initiator_time_req, unsigned int acceptor_time_req,
                              gss_cred_id_t *output_cred_handle, gss_OID_set *actual_mechs, unsigned int *initiator_time_rec,
                              unsigned int *acceptor_time_rec)
{
    gss_mechanism	mech;
    gss_union_cred_t new_union_cred_for_out, union_cred;
    gss_union_name_t union_name;
    gss_name_t internal_name = GSS_C_NO_NAME;
    gss_name_t allocated_name = GSS_C_NO_NAME;
    OM_uint32 time_req, time_rec;
    unsigned int status;
    gss_cred_id_t cred = NULL;
    gss_OID new_mechs_array = NULL;
    gss_cred_id_t *	new_cred_array = NULL;

    mech = gssint_get_mechanism(desired_mech);
    union_cred = (gss_union_cred_t)creds;
    if (gssint_get_mechanism_cred(union_cred, desired_mech) != GSS_C_NO_CREDENTIAL)
        return (GSS_S_DUPLICATE_ELEMENT);

    if (desired_name)
    {
        union_name = (gss_union_name_t)desired_name;
        if (rtk_gss_import_internal_name(minor_status, &mech->mech_type, union_name, &allocated_name) != GSS_S_COMPLETE)
            return (GSS_S_BAD_NAME);

        internal_name = allocated_name;
    }
    if (cred_usage == GSS_C_ACCEPT)
        time_req = acceptor_time_req;
    else if (cred_usage == GSS_C_INITIATE)
        time_req = initiator_time_req;
    else if (cred_usage == GSS_C_BOTH)
        time_req = (acceptor_time_req > initiator_time_req) ? acceptor_time_req : initiator_time_req;

    krb5_gss_acquire_cred(minor_status, internal_name, time_req, GSS_C_NULL_OID_SET, cred_usage, &cred, NULL, &time_rec);
    if (union_cred->auxinfo.creation_time == 0)
    {
        union_cred->auxinfo.creation_time = time(NULL);
        union_cred->auxinfo.time_rec = time_rec;
        union_cred->auxinfo.cred_usage = cred_usage;
    }

    /* now add the new credential elements */
    new_mechs_array = (gss_OID)malloc(sizeof (gss_OID_desc) * (union_cred->count+1));

    new_cred_array = (gss_cred_id_t *)malloc(sizeof (gss_cred_id_t) * (union_cred->count+1));

    if (!new_mechs_array || !new_cred_array)
    {
        status = GSS_S_FAILURE;
        goto errout;
    }
    /*
     * OK, expand the mechanism array and the credential array
     */
    (void) memcpy(new_mechs_array, union_cred->mechs_array, sizeof (gss_OID_desc) * union_cred->count);
    (void) memcpy(new_cred_array, union_cred->cred_array, sizeof (gss_cred_id_t) * union_cred->count);

    new_cred_array[union_cred->count] = cred;
    if ((new_mechs_array[union_cred->count].elements = (void*)malloc(mech->mech_type.length)) == NULL)
        goto errout;

    g_OID_copy(&new_mechs_array[union_cred->count],
               &mech->mech_type);

    if (output_cred_handle == NULL)
    {
        //free(union_cred->mechs_array);
        //free(union_cred->cred_array);
        new_union_cred_for_out = union_cred;
    }
    else
    {
        //new_union_cred = (gss_union_cred_t)malloc(sizeof (gss_union_cred_desc));
        //if (new_union_cred == NULL)
        //{
        //	free(new_mechs_array[union_cred->count].elements);
        //	goto errout;
        //}
        //*new_union_cred = *union_cred;
        //*output_cred_handle = (gss_cred_id_t)new_union_cred;
    }

    new_union_cred_for_out->mechs_array = new_mechs_array;
    new_union_cred_for_out->cred_array = new_cred_array;
    new_union_cred_for_out->count++;
    new_union_cred_for_out->loopback = new_union_cred_for_out;

    //union_cred->mechs_array = /*&mech->mech_type*/;
    //union_cred->cred_array = /*&cred*/;
    //union_cred->count++;
    //union_cred->loopback = union_cred;


    return (GSS_S_COMPLETE);
errout:
    if (new_mechs_array)
        free(new_mechs_array);
    if (new_cred_array)
        free(new_cred_array);

//   if (cred != NULL && mech->gss_release_cred)
    //mech->gss_release_cred(mech->context,
    //		       &temp_minor_status, &cred);

//   if (allocated_name)
    //(void) gssint_release_internal_name(&temp_minor_status,
    //				   &mech->mech_type,
    //				   &allocated_name);

    if (creds == GSS_C_NO_CREDENTIAL && union_cred)
    {
        if (union_cred->auxinfo.name.value)
            free(union_cred->auxinfo.name.value);
        free(union_cred);
    }

    return (status);
}

unsigned int rtk_gss_import_internal_name(OM_uint32	*minor_status, gss_OID mech_type, gss_union_name_t union_name, gss_name_t *internal_name)
{
    OM_uint32		status;
    gss_mechanism	mech;

    mech = gssint_get_mechanism (mech_type);

    if (mech)
    {
        status = rtk_krb5_gss_import_name (minor_status, union_name->external_name, union_name->name_type, internal_name);
        return (status);
    }

    return (GSS_S_BAD_MECH);
}


/*
 * We will release everything but the ctx_handle so that it
 * can be passed back to init/accept context. This routine should
 * not be called until after the ctx_handle memory is assigned to
 * the supplied context handle from init/accept context.
 */
static void rtk_release_spnego_ctx(spnego_gss_ctx_id_t *ctx)
{
    spnego_gss_ctx_id_t context;
    OM_uint32 minor_stat;
    context = *ctx;

    if (context != NULL)
    {
        (void) gss_release_buffer(&minor_stat, &context->DER_mechTypes);

        (void) generic_gss_release_oid(&minor_stat, &context->internal_mech, 1);

        if (context->optionStr != NULL)
        {
            free(context->optionStr);
            context->optionStr = NULL;
        }
        free(context);
        *ctx = NULL;
    }
}

#ifdef TEST_GSSAPI
unsigned char test[]=
{
    0x60,0x82,0x05,0x0b,0x06,0x06,0x2b,0x06,0x01,0x05,0x05,0x02,0xa0,0x82,0x04,0xff,0x30,0x82,0x04,0xfb,0xa0,0x24,0x30,0x22,0x06,0x09,0x2a,0x86,0x48,0x82,0xf7,0x12,0x01,0x02,0x02,0x06,0x09
    ,0x2a,0x86,0x48,0x86,0xf7,0x12,0x01,0x02,0x02,0x06,0x0a,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x02,0x02,0x0a,0xa2,0x82,0x04,0xd1,0x04,0x82,0x04,0xcd,0x60,0x82,0x04,0xc9,0x06,0x09,0x2a,0x86
    ,0x48,0x86,0xf7,0x12,0x01,0x02,0x02,0x01,0x00,0x6e,0x82,0x04,0xb8,0x30,0x82,0x04,0xb4,0xa0,0x03,0x02,0x01,0x05,0xa1,0x03,0x02,0x01,0x0e,0xa2,0x07,0x03,0x05,0x00,0x20,0x00,0x00,0x00,0xa3
    ,0x82,0x03,0xd9,0x61,0x82,0x03,0xd5,0x30,0x82,0x03,0xd1,0xa0,0x03,0x02,0x01,0x05,0xa1,0x0e,0x1b,0x0c,0x53,0x43,0x43,0x4d,0x54,0x45,0x53,0x54,0x2e,0x43,0x4f,0x4d,0xa2,0x26,0x30,0x24,0xa0
    ,0x03,0x02,0x01,0x02,0xa1,0x1d,0x30,0x1b,0x1b,0x04,0x48,0x54,0x54,0x50,0x1b,0x13,0x61,0x64,0x74,0x65,0x73,0x74,0x2e,0x73,0x63,0x63,0x6d,0x74,0x65,0x73,0x74,0x2e,0x63,0x6f,0x6d,0xa3,0x82
    ,0x03,0x90,0x30,0x82,0x03,0x8c,0xa0,0x03,0x02,0x01,0x17,0xa1,0x03,0x02,0x01,0x02,0xa2,0x82,0x03,0x7e,0x04,0x82,0x03,0x7a,0x21,0x46,0x4a,0x63,0xca,0x7c,0x82,0x5f,0x36,0x56,0x6a,0x4c,0x36
    ,0x10,0xc1,0xd1,0x76,0x59,0xec,0x23,0xf0,0x60,0xd2,0xcc,0xc5,0x38,0xee,0x4e,0x5b,0x58,0xec,0x9b,0x0d,0xdb,0x51,0x90,0x0b,0x4b,0xc6,0x2c,0x68,0x4b,0x47,0xe6,0xe7,0x7c,0xef,0x1a,0x34,0xf3
    ,0xe0,0x0c,0x32,0x26,0x23,0x57,0x5b,0xfc,0xe0,0xbb,0x13,0x97,0x61,0x74,0x86,0xc9,0x2a,0xdd,0xc6,0xb4,0xf3,0x7f,0x45,0xcb,0x00,0x22,0xec,0x09,0xaf,0xd1,0x24,0xe2,0x06,0xe2,0xf0,0xdc,0x51
    ,0x6d,0x93,0x71,0x4e,0x87,0xdf,0x56,0x9b,0x95,0xa4,0xad,0x4b,0xc2,0x75,0xbd,0xd1,0x40,0xc5,0x67,0xd7,0xdc,0x12,0x14,0x82,0xe9,0x3b,0x8d,0x59,0xbc,0x2e,0x5d,0xa5,0x92,0x89,0x67,0xa9,0x1d
    ,0x20,0x6e,0x69,0xb6,0x72,0xa6,0x40,0xc5,0x6c,0xca,0xc6,0x6a,0x7a,0xf7,0x8f,0x33,0x86,0xa3,0x8f,0x1a,0xb9,0x63,0xf1,0xf4,0x64,0x0b,0x64,0x6e,0x86,0xc8,0xeb,0xf5,0x6f,0xc8,0xea,0xac,0x08
    ,0x22,0x8e,0x5c,0x13,0x75,0xe9,0x57,0xed,0x81,0x8b,0xa0,0x06,0x1e,0x47,0xfa,0x9b,0x20,0x1e,0xbc,0x62,0xe0,0x90,0x43,0x66,0xfb,0xb9,0xe5,0x11,0x0f,0xc5,0xbb,0x9a,0x4d,0xc0,0xe1,0x87,0x5b
    ,0x86,0x1b,0x38,0x26,0xcc,0x6a,0x32,0xa8,0x6b,0x44,0xd5,0x31,0x75,0xa8,0x6f,0xd1,0x1f,0xde,0x69,0x72,0xe9,0xbe,0x45,0x8a,0x64,0x04,0x9c,0xd7,0xfb,0x26,0x5a,0xc3,0x5d,0x64,0x73,0xb0,0x0c
    ,0x24,0x6b,0xe5,0x89,0x17,0x35,0x99,0x1b,0x1e,0x11,0xe5,0xde,0x41,0x48,0xfc,0x71,0x8a,0x94,0x15,0x93,0x4f,0x53,0x1c,0xa1,0x36,0x49,0x82,0xea,0x9a,0x68,0xd0,0x2a,0x29,0x1f,0xcc,0x8d,0x85
    ,0x7a,0x17,0x74,0x5d,0x4a,0xc2,0x86,0x88,0x9a,0xb5,0x98,0x50,0xf3,0x1e,0xe2,0xca,0xb9,0xda,0x2f,0x8e,0x2d,0xcd,0xfc,0xe8,0x5c,0x61,0x57,0x2b,0x89,0x50,0xfc,0x5e,0x4e,0xbd,0x4a,0x24,0x6e
    ,0x61,0x53,0x5a,0xe9,0x0d,0x0c,0xff,0x8f,0x21,0xcb,0x90,0x0f,0x74,0x01,0x3c,0x5a,0x2c,0x0e,0x49,0xea,0x3c,0xc5,0xd8,0xf2,0x23,0xb9,0xff,0xb3,0x3c,0x4d,0xd7,0x2d,0x65,0xb7,0x51,0x2a,0xde
    ,0xe0,0x17,0xba,0x18,0x00,0x1b,0x74,0x8a,0xa1,0xde,0x27,0x29,0x93,0x01,0xd0,0xd9,0x32,0x2a,0x00,0x90,0x20,0xf9,0xab,0x16,0xe6,0xb9,0x39,0xa9,0x1e,0x0b,0x2f,0x18,0xb3,0x9a,0xbb,0x87,0x3b
    ,0x61,0xfb,0xe0,0xce,0x0b,0xe4,0xb2,0xdb,0x75,0xe9,0x38,0x22,0x1b,0x5c,0xb2,0x0c,0x93,0x86,0x37,0x40,0xd5,0x37,0x0b,0x08,0x0a,0x53,0xa9,0xf9,0x8a,0x39,0x34,0x57,0x82,0x29,0xea,0xda,0x61
    ,0x49,0xbe,0xc1,0x8d,0x4d,0x53,0xda,0xce,0xb1,0x9a,0xf3,0xbc,0xef,0x20,0x19,0xca,0x73,0x2c,0x6b,0xfe,0xf4,0xeb,0x01,0x17,0x0d,0x59,0xbe,0x79,0x6b,0xda,0x0c,0x7e,0xd3,0xf0,0x48,0x45,0xf5
    ,0xa3,0x5e,0x23,0x55,0x4f,0x0e,0x30,0x76,0x8d,0xa3,0x3e,0x12,0xa1,0x18,0x1d,0x79,0x21,0xd3,0x60,0xbe,0x1e,0x23,0x25,0x50,0x4b,0x79,0xaa,0xf5,0x30,0x1e,0x41,0x95,0x43,0xe0,0xf0,0xd0,0xa1
    ,0x32,0xa7,0x3b,0x23,0x73,0xb1,0x00,0xcd,0x36,0x47,0xce,0x34,0xe2,0x14,0x26,0x05,0x53,0x90,0x33,0xff,0x43,0x6d,0xcb,0x41,0x83,0xc7,0xa3,0x62,0xb6,0x8b,0x02,0xa5,0x0b,0x41,0xe0,0x53,0xe7
    ,0x01,0x22,0x2a,0xcf,0x62,0x18,0x54,0x8e,0x72,0x69,0x65,0x69,0x33,0x3a,0x78,0x3d,0x56,0xd5,0x86,0x6d,0xaf,0x56,0x52,0xe2,0x4f,0xfe,0x19,0xcf,0x29,0x88,0x2a,0xdd,0xe2,0x0e,0x44,0x4c,0x6c
    ,0x4c,0xc1,0xfc,0x0f,0x5d,0x2b,0x87,0xfe,0x06,0x08,0xf6,0x66,0xac,0x7d,0x55,0xe5,0x5b,0x11,0x74,0x19,0xa6,0xe6,0xc2,0xe2,0xf0,0x75,0xcc,0xd8,0xc3,0x1c,0x21,0xca,0x36,0x00,0xcb,0x5a,0x12
    ,0x5b,0x38,0x8f,0x09,0x62,0x05,0x26,0xf5,0xdc,0xeb,0x0a,0x70,0x49,0x92,0xb2,0x09,0xfe,0x1f,0xfc,0x81,0x1a,0x14,0x13,0x9e,0xb4,0x17,0x0a,0xfa,0x52,0x26,0xd9,0x57,0xfc,0x52,0x5b,0xc0,0x69
    ,0x2f,0x8b,0x53,0x90,0x89,0x3f,0x94,0xf5,0xed,0xd2,0x13,0x6b,0x24,0x7b,0x2d,0xf5,0xb1,0x61,0x72,0xfe,0x5c,0xbf,0xf1,0xf5,0x0d,0x6b,0x9e,0x85,0x6e,0x97,0x0b,0x5d,0xe0,0xdc,0x82,0x48,0x16
    ,0xd0,0x12,0xa9,0x94,0x9b,0x95,0x75,0x7b,0xe6,0x32,0xf0,0x05,0xba,0x16,0x19,0xbb,0x11,0x6e,0xbb,0x58,0x20,0xf4,0x9a,0x5c,0x87,0xf4,0xa4,0x0f,0x3d,0x83,0xb0,0x09,0x5d,0x38,0x2a,0x68,0xfb
    ,0x54,0xf9,0xea,0x28,0x87,0x5b,0x04,0xb3,0x2e,0x69,0xa9,0x3e,0xe8,0xa5,0x10,0x74,0xd9,0xcf,0x34,0x15,0x7a,0xa9,0xd5,0xa4,0x35,0xdd,0x62,0xc9,0x3a,0xee,0x71,0xa6,0x57,0xbf,0x96,0x60,0x96
    ,0xb1,0xd5,0x2b,0xf9,0x21,0x77,0x1a,0x7f,0xea,0xb1,0x88,0x03,0x57,0xde,0x9a,0xc9,0xe1,0x28,0x8d,0x07,0xe9,0x30,0x5a,0x29,0xba,0x1d,0xbc,0x9c,0x03,0xfb,0x78,0x6f,0x90,0x72,0x04,0x62,0x87
    ,0x0d,0x3d,0x92,0xfe,0xe8,0x74,0x97,0x8f,0x6c,0xdd,0x1b,0xf8,0x76,0x76,0x9b,0x8d,0x97,0x15,0x57,0x68,0x9e,0xde,0xd9,0x0c,0x63,0x84,0xd1,0x6e,0xbb,0xc3,0xdd,0xa6,0x00,0x59,0x67,0x29,0x87
    ,0x30,0x54,0xe6,0x34,0xce,0xde,0x2d,0x39,0x42,0xc9,0x37,0x45,0x0e,0xa2,0xf6,0xd3,0x42,0xd9,0x1d,0x85,0x2f,0x4f,0x5c,0xfa,0x67,0x6a,0x77,0x19,0xd6,0xe7,0x60,0xed,0x9e,0x60,0x80,0x11,0x97
    ,0xff,0xf1,0xd4,0x5d,0x26,0xb2,0xcf,0xf4,0x43,0xd5,0x9a,0x19,0x26,0x00,0xfd,0xc3,0x08,0xd4,0x81,0x48,0x20,0x5f,0x28,0xf2,0x98,0xa0,0xa4,0x81,0xc1,0x30,0x81,0xbe,0xa0,0x03,0x02,0x01,0x17
    ,0xa2,0x81,0xb6,0x04,0x81,0xb3,0xe5,0x48,0xe8,0xc8,0xfa,0x94,0xeb,0x6f,0x9a,0x42,0x46,0x54,0x3a,0x85,0xd5,0x8d,0x1c,0x78,0x2d,0x39,0x91,0x1d,0x0b,0x14,0x69,0xf1,0x2f,0xf9,0xc5,0x66,0xc1
    ,0xee,0xa4,0x35,0x59,0x60,0xda,0x61,0xcb,0xd1,0x71,0x0b,0xd9,0xd8,0x4d,0x51,0x79,0xf9,0xd5,0x60,0x51,0x86,0x3d,0x27,0x6b,0x7d,0x9f,0xd0,0x2f,0x56,0xa0,0x21,0xb3,0x9c,0xd7,0xdb,0x90,0x40
    ,0x53,0xc5,0xe2,0x53,0x58,0x86,0xeb,0xa5,0x99,0xd3,0x35,0x12,0x77,0xf8,0x68,0x89,0xfb,0x69,0x14,0x39,0x78,0xf4,0x91,0x54,0x3a,0x5b,0xf0,0x9d,0x8c,0x77,0x7b,0xab,0x4e,0xcd,0x5e,0xf8,0x6b
    ,0x6b,0xfa,0xac,0x26,0x1b,0x93,0x96,0xff,0x4f,0x13,0x06,0x8a,0x51,0xb2,0xd0,0x60,0xf7,0x83,0x47,0x19,0x0e,0x52,0xbd,0xce,0xef,0x7b,0x35,0xb9,0x63,0x7a,0xf3,0xf7,0x60,0x5c,0x92,0x8f,0x46
    ,0xcd,0x67,0xf1,0xbe,0xbf,0x76,0x0c,0xad,0x20,0xdc,0xff,0x99,0x78,0x22,0x10,0xba,0x38,0x48,0xac,0x75,0xb0,0xd0,0xec,0x07,0x28,0xc0,0x80,0x8e,0x25,0xe9,0x4e,0x13,0x06,0x30,0xf3,0xb0,0x73
};
#endif



int rtk_gssint_get_der_length(unsigned char **buf, unsigned int buf_len, unsigned int *bytes)
{
    /* p points to the beginning of the buffer */
    unsigned char *p = *buf;
    int length, new_length;
    int octets;

    if (buf_len < 1)
        return (-1);

    /* We should have at least one byte */
    *bytes = 1;

    /*
     * If the High order bit is not set then the length is just the value
     * of *p.
     */
    if (*p < 128)
    {
        *buf = p+1;	/* Advance the buffer */
        return (*p);		/* return the length */
    }

    /*
     * if the High order bit is set, then the low order bits represent
     * the number of bytes that contain the DER encoding of the length.
     */

    octets = *p++ & 0x7f;
    *bytes += octets;

    /* See if the supplied buffer contains enough bytes for the length. */
    if (octets > buf_len - 1)
        return (-1);

    /*
     * Calculate a multibyte length. The length is encoded as an
     * unsigned integer base 256.
     */
    for (length = 0; octets; octets--)
    {
        new_length = (length << 8) + *p++;
        if (new_length < length)  /* overflow */
            return (-1);
        length = new_length;
    }

    *buf = p; /* Advance the buffer */

    return (length);
}

/* verify token header. */
int rtk_g_verify_token_header(const gss_OID_desc * mech, unsigned int *body_size, unsigned char **buf_in, int tok_type, unsigned int toksize)
{
    unsigned char *buf = *buf_in;
    int seqsize;
    gss_OID_desc toid;
    int ret = 0;
    unsigned int bytes;

    if (toksize-- < 1)
        return (G_BAD_TOK_HEADER);

    if (*buf++ != HEADER_ID)
        return (G_BAD_TOK_HEADER);

    if ((seqsize = rtk_gssint_get_der_length(&buf, toksize, &bytes)) < 0)
        return (G_BAD_TOK_HEADER);

    if ((seqsize + bytes) != toksize)
        return (G_BAD_TOK_HEADER);

    if (toksize-- < 1)
        return (G_BAD_TOK_HEADER);


    if (*buf++ != MECH_OID)
        return (G_BAD_TOK_HEADER);

    if (toksize-- < 1)
        return (G_BAD_TOK_HEADER);

    toid.length = *buf++;

    if (toksize < toid.length)
        return (G_BAD_TOK_HEADER);
    else
        toksize -= toid.length;

    toid.elements = buf;
    buf += toid.length;

    if (!g_OID_equal(&toid, mech))
        ret = G_WRONG_MECH;

    /*
     * G_WRONG_MECH is not returned immediately because it's more important
     * to return G_BAD_TOK_HEADER if the token header is in fact bad
     */
    if (toksize < 2)
        return (G_BAD_TOK_HEADER);
    else
        toksize -= 2;

    if (!ret) {
        *buf_in = buf;
        *body_size = toksize;
    }

    return (ret);
}


/*
 * NOTE: This checks that the length returned by
 * gssint_get_der_length() is not greater than the number of octets
 * remaining, even though gssint_get_der_length() already checks, in
 * theory.
 */
static int rtk_g_get_tag_and_length(unsigned char **buf, int tag,
                                    unsigned int buflen, unsigned int *outlen)
{
    unsigned char *ptr = *buf;
    int ret = -1; /* pessimists, assume failure ! */
    unsigned int encoded_len;
    int tmplen = 0;

    *outlen = 0;
    if (buflen > 1 && *ptr == tag) {
        ptr++;
        tmplen = gssint_get_der_length(&ptr, buflen - 1,
                                       &encoded_len);
        if (tmplen < 0) {
            ret = -1;
        } else if (tmplen > buflen - (ptr - *buf)) {
            ret = -1;
        } else
            ret = 0;
    }
    *outlen = tmplen;
    *buf = ptr;
    return (ret);
}



int g_verify_neg_token_init(unsigned char **buf_in, unsigned int cur_size)
{
    unsigned char *buf = *buf_in;
    unsigned char *endptr = buf + cur_size;
    unsigned int seqsize;
    int ret = 0;
    unsigned int bytes;

    /*
     * Verify this is a NegotiationToken type token
     * - check for a0(context specific identifier)
     * - get length and verify that enoughd ata exists
     */
    if (rtk_g_get_tag_and_length(&buf, CONTEXT, cur_size, &seqsize) < 0)
        return (G_BAD_TOK_HEADER);

    cur_size = seqsize; /* should indicate bytes remaining */

    /*
     * Verify the next piece, it should identify this as
     * a strucure of type NegTokenInit.
     */
    if (*buf++ == SEQUENCE)
    {
        if ((seqsize = gssint_get_der_length(&buf, cur_size, &bytes)) < 0)
            return (G_BAD_TOK_HEADER);
        /*
         * Make sure we have the entire buffer as described
         */
        if (buf + seqsize > endptr)
            return (G_BAD_TOK_HEADER);
    }
    else
    {
        return (G_BAD_TOK_HEADER);
    }

    cur_size = seqsize; /* should indicate bytes remaining */

    /*
     * Verify that the first blob is a sequence of mechTypes
     */
    if (*buf++ == CONTEXT)
    {
        if ((seqsize = gssint_get_der_length(&buf, cur_size, &bytes)) < 0)
            return (G_BAD_TOK_HEADER);
        /*
         * Make sure we have the entire buffer as described
         */
        if (buf + bytes > endptr)
            return (G_BAD_TOK_HEADER);
    }
    else
    {
        return (G_BAD_TOK_HEADER);
    }

    /*
     * At this point, *buf should be at the beginning of the
     * DER encoded list of mech types that are to be negotiated.
     */
    *buf_in = buf;

    return (ret);

}

int put_input_token(unsigned char **buf_out, gss_buffer_t input_token, unsigned int buflen)
{
    int ret;

    /* if token length is 0, we do not want to send */
    if (input_token->length == 0)
        return (0);

    if (input_token->length > buflen)
        return (-1);

    *(*buf_out)++ = OCTET_STRING;
    if ((ret = gssint_put_der_length(input_token->length, buf_out, input_token->length)))
        return (ret);
    TWRITE_STR(*buf_out, input_token->value, input_token->length);
    return (0);
}

OM_uint32 krb5_gss_indicate_mechs(OM_uint32 *minor_status, gss_OID_set *mech_set)
{
    *minor_status = 0;
    *mech_set = (gss_OID_set_desc *)gss_mech_set_krb5_both;
    return(GSS_S_COMPLETE);
}


OM_uint32 acc_ctx_vfy_oid(OM_uint32 *minor_status, spnego_gss_ctx_id_t sc, gss_OID mechoid, OM_uint32 *negState, send_token_flag *tokflag)
{
    OM_uint32 ret, tmpmin;
    gss_mechanism mech = NULL;
    gss_OID_set mech_set = GSS_C_NO_OID_SET;
    int present = 0;

    if (g_OID_equal(sc->internal_mech, mechoid))
        return GSS_S_COMPLETE;

    mech = gssint_get_mechanism(sc->internal_mech);
    if (mech == NULL || mech->gss_indicate_mechs == NULL)
    {
        *minor_status = ERR_SPNEGO_NEGOTIATION_FAILED;
        *negState = REJECT;
        *tokflag = ERROR_TOKEN_SEND;
        return GSS_S_BAD_MECH;
    }
    ret = krb5_gss_indicate_mechs(minor_status, &mech_set);
    if (ret != GSS_S_COMPLETE)
    {
        *tokflag = NO_TOKEN_SEND;
        goto cleanup;
    }
    ret = gss_test_oid_set_member(minor_status, mechoid,
                                  mech_set, &present);
    if (ret != GSS_S_COMPLETE)
        goto cleanup;
    if (!present)
    {
        *minor_status = ERR_SPNEGO_NEGOTIATION_FAILED;
        *negState = REJECT;
        *tokflag = ERROR_TOKEN_SEND;
        ret = GSS_S_BAD_MECH;
    }
cleanup:
    rtk_gss_release_oid_set(&tmpmin, &mech_set);
    return ret;
}

int put_negResult(unsigned char **buf_out, OM_uint32 negResult,
                  unsigned int buflen)
{
    if (buflen < 3)
        return (-1);
    *(*buf_out)++ = ENUMERATED;
    *(*buf_out)++ = ENUMERATION_LENGTH;
    *(*buf_out)++ = (unsigned char) negResult;
    return (0);
}

int put_mech_oid(unsigned char **buf_out, const gss_OID mech, unsigned int buflen)
{
    if (buflen < mech->length + 2)
        return (-1);
    *(*buf_out)++ = MECH_OID;
    *(*buf_out)++ = (unsigned char) mech->length;
    memcpy((void *)(*buf_out), mech->elements, mech->length);
    *buf_out += mech->length;
    return (0);
}

/*
 * create the server side spnego token passed back to
 * gss_accept_sec_context and eventually up to the application program
 * and over to the client.
 */
int rtk_make_spnego_tokenTarg_msg(OM_uint32 status, gss_OID mech_wanted, gss_buffer_t mechtok_out, gss_buffer_t mechListMIC,
                                  send_token_flag sendtoken, gss_buffer_t outbuf)
{
    unsigned int tlen = 0;
    unsigned int ret = 0;
    unsigned int NegTokenTargSize = 0;
    unsigned int NegTokenSize = 0;
    unsigned int rspTokenSize = 0;
    unsigned int micTokenSize = 0;
    unsigned int dataLen = 0;
    unsigned char *t;
    unsigned char *ptr;

    if (outbuf == GSS_C_NO_BUFFER)
        return (GSS_S_DEFECTIVE_TOKEN);

    outbuf->length = 0;
    outbuf->value = NULL;

    /*
     * ASN.1 encoding of the negResult
     * ENUMERATED type is 3 bytes
     *  ENUMERATED TAG, Length, Value,
     * Plus 2 bytes for the CONTEXT id and length.
     */
    dataLen = 5;

    /*
     * calculate data length
     *
     * If this is the initial token, include length of
     * mech_type and the negotiation result fields.
     */
    if (sendtoken == INIT_TOKEN_SEND)
    {
        int mechlistTokenSize;
        /*
         * 1 byte for the CONTEXT ID(0xa0),
         * 1 byte for the OID ID(0x06)
         * 1 byte for OID Length field
         * Plus the rest... (OID Length, OID value)
         */
        mechlistTokenSize = 3 + mech_wanted->length + gssint_der_length_size(mech_wanted->length);

        dataLen += mechlistTokenSize;
    }
    if (mechtok_out != NULL && mechtok_out->length > 0)
    {
        /* Length of the inner token */
        rspTokenSize = 1 + gssint_der_length_size(mechtok_out->length) + mechtok_out->length;

        dataLen += rspTokenSize;

        /* Length of the outer token */
        dataLen += 1 + gssint_der_length_size(rspTokenSize);
    }
    if (mechListMIC != NULL)
    {
        /* Length of the inner token */
        micTokenSize = 1 + gssint_der_length_size(mechListMIC->length) + mechListMIC->length;

        dataLen += micTokenSize;

        /* Length of the outer token */
        dataLen += 1 + gssint_der_length_size(micTokenSize);
    }
    /*
     * Add size of DER encoded:
     * NegTokenTarg [ SEQUENCE ] of
     *    NegResult[0] ENUMERATED {
     *	accept_completed(0),
     *	accept_incomplete(1),
     *	reject(2) }
     *    supportedMech [1] MechType OPTIONAL,
     *    responseToken [2] OCTET STRING OPTIONAL,
     *    mechListMIC   [3] OCTET STRING OPTIONAL
     *
     * size = data->length + MechListMic + SupportedMech len +
     *	Result Length + ASN.1 overhead
     */
    NegTokenTargSize = dataLen;
    dataLen += 1 + gssint_der_length_size(NegTokenTargSize);

    /*
     * NegotiationToken [ CHOICE ]{
     *    negTokenInit  [0]	 NegTokenInit,
     *    negTokenTarg  [1]	 NegTokenTarg }
     */
    NegTokenSize = dataLen;
    dataLen += 1 + gssint_der_length_size(NegTokenSize);

    tlen = dataLen;
    t = (unsigned char *) malloc(tlen);

    if (t == NULL)
    {
        ret = GSS_S_DEFECTIVE_TOKEN;
        goto errout;
    }

    ptr = t;

    /*
     * Indicate that we are sending CHOICE 1
     * (NegTokenTarg)
     */
    *ptr++ = CONTEXT | 0x01;
    if (gssint_put_der_length(NegTokenSize, &ptr, dataLen) < 0)
    {
        ret = GSS_S_DEFECTIVE_TOKEN;
        goto errout;
    }
    *ptr++ = SEQUENCE;
    if (gssint_put_der_length(NegTokenTargSize, &ptr, tlen - (int)(ptr-t)) < 0)
    {
        ret = GSS_S_DEFECTIVE_TOKEN;
        goto errout;
    }

    /*
     * First field of the NegTokenTarg SEQUENCE
     * is the ENUMERATED NegResult.
     */
    *ptr++ = CONTEXT;
    if (gssint_put_der_length(3, &ptr, tlen - (int)(ptr-t)) < 0)
    {
        ret = GSS_S_DEFECTIVE_TOKEN;
        goto errout;
    }
    if (put_negResult(&ptr, status, tlen - (int)(ptr - t)) < 0)
    {
        ret = GSS_S_DEFECTIVE_TOKEN;
        goto errout;
    }
    if (sendtoken == INIT_TOKEN_SEND)
    {
        /*
         * Next, is the Supported MechType
         */
        *ptr++ = CONTEXT | 0x01;
        if (gssint_put_der_length(mech_wanted->length + 2, &ptr, tlen - (int)(ptr - t)) < 0)
        {
            ret = GSS_S_DEFECTIVE_TOKEN;
            goto errout;
        }
        if (memcmp(mech_wanted->elements, GSS_MECH_KRB5_OID, GSS_MECH_KRB5_OID_LENGTH) == 0)
        {
            memcpy(mech_wanted->elements, GSS_MECH_KRB5_WRONG_OID, GSS_MECH_KRB5_WRONG_OID_LENGTH);
        }
        if (put_mech_oid(&ptr, mech_wanted,tlen - (int)(ptr - t)) < 0)
        {
            ret = GSS_S_DEFECTIVE_TOKEN;
            goto errout;
        }
    }
    if (mechtok_out != NULL && mechtok_out->length > 0)
    {
        *ptr++ = CONTEXT | 0x02;
        if (gssint_put_der_length(rspTokenSize, &ptr, tlen - (int)(ptr - t)) < 0)
        {
            ret = GSS_S_DEFECTIVE_TOKEN;
            goto errout;
        }
        if (put_input_token(&ptr, mechtok_out, tlen - (int)(ptr - t)) < 0)
        {
            ret = GSS_S_DEFECTIVE_TOKEN;
            goto errout;
        }
    }
    if (mechListMIC != NULL)
    {
        *ptr++ = CONTEXT | 0x03;
        if (gssint_put_der_length(micTokenSize, &ptr, tlen - (int)(ptr - t)) < 0)
        {
            ret = GSS_S_DEFECTIVE_TOKEN;
            goto errout;
        }
        if (put_input_token(&ptr, mechListMIC, tlen - (int)(ptr - t)) < 0)
        {
            ret = GSS_S_DEFECTIVE_TOKEN;
            goto errout;
        }
    }
    ret = GSS_S_COMPLETE;
errout:
    if (ret != GSS_S_COMPLETE)
    {
        if (t)
            free(t);
    }
    else
    {
        outbuf->length = ptr - t;
        outbuf->value = (void *) t;
    }

    return (ret);
}


gss_OID rtk_get_mech_oid(OM_uint32 *minor_status, unsigned char **buff_in, int length)
{
    //OM_uint32	status;
    gss_OID 	toid = 0;
    //gss_OID		mech_out = NULL;
    unsigned char		*start, *end;

    if (length < 1 || **buff_in != MECH_OID)
        return (NULL);

    start = *buff_in;
    end = start + length;

    (*buff_in)++;
    toid = (gss_OID)malloc(sizeof(gss_OID_desc));
    toid->length = *(*buff_in)++;

    if ((*buff_in + toid->length) > end)
        return (NULL);

    toid->elements = *buff_in;
    *buff_in += toid->length;

    return toid;
}

OM_uint32 rtk_gss_add_oid_set_member(OM_uint32 *minor_status, gss_OID_desc *member_oid, gss_OID_set *oid_set)
{
    gss_OID	elist;
    gss_OID	lastel;

    *minor_status = 0;

    if (member_oid == NULL || member_oid->length == 0 || member_oid->elements == NULL)
        return (GSS_S_CALL_INACCESSIBLE_READ);

    elist = (*oid_set)->elements;
    /* Get an enlarged copy of the array */
    if (((*oid_set)->elements = (gss_OID) malloc(((*oid_set)->count+1) * sizeof(gss_OID_desc))))
    {
        /* Copy in the old junk */
        if (elist)
            memcpy((*oid_set)->elements,
                   elist,
                   ((*oid_set)->count * sizeof(gss_OID_desc)));

        /* Duplicate the input element */
        lastel = &(*oid_set)->elements[(*oid_set)->count];
        lastel->elements = member_oid->elements;
        lastel->length = member_oid->length;
        //if ((lastel->elements =
        //     (void *) malloc((size_t) member_oid->length))) {
        //    /* Success - copy elements */
        //    memcpy(lastel->elements, member_oid->elements,
        //	   (size_t) member_oid->length);
        //    /* Set length */
        //    lastel->length = member_oid->length;

        //    /* Update count */
        (*oid_set)->count++;
        if (elist)
            free(elist);
        *minor_status = 0;
        return(GSS_S_COMPLETE);
        //}
        //else
        //    free((*oid_set)->elements);
    }
    /* Failure - restore old contents of list */
    (*oid_set)->elements = elist;
    *minor_status = -1;
    return(GSS_S_FAILURE);
}

gss_OID_set rkt_get_mech_set(unsigned int *minor_status, unsigned char **buff_in, unsigned int buff_length)
{
    gss_OID_set returned_mechSet = 0;
    unsigned int major_status;
    unsigned int length, i;
    unsigned int bytes;
    unsigned int set_length;
    //gss_OID *ptr = 0;
    unsigned char *p, *start;
    //gss_OID	lastel;

    if (**buff_in != SEQUENCE_OF)
        return (NULL);

    start = *buff_in;
    (*buff_in)++;

    length = gssint_get_der_length(buff_in, buff_length, &bytes);
    p = *buff_in;

    major_status = gss_create_empty_oid_set(minor_status,
                                            &returned_mechSet);
    if (major_status != GSS_S_COMPLETE)
        return (NULL);

    for (set_length = 0, i = 0; set_length < length; i++)
    {
        gss_OID_desc *temp = rtk_get_mech_oid(minor_status, buff_in, buff_length - (*buff_in - start));
        if (temp != NULL)
        {
            major_status = rtk_gss_add_oid_set_member(minor_status, temp, &returned_mechSet);
            if (major_status == GSS_S_COMPLETE)
            {
                set_length += returned_mechSet->elements[i].length +2;
                generic_gss_release_oid(minor_status, &temp, 0);
            }
        }
    }
    return (returned_mechSet);
}

/*
 * Verify that buff_in is pointing to a BIT_STRING with the correct
 * length and padding for the req_flags. If it is, decode req_flags
 * and return them, otherwise, return NULL.
 */
static OM_uint32 get_req_flags(unsigned char **buff_in, OM_uint32 bodysize, OM_uint32 *req_flags)
{
    unsigned int len;

    if (**buff_in != (CONTEXT | 0x01))
        return (0);

    if (rtk_g_get_tag_and_length(buff_in, (CONTEXT | 0x01), bodysize, &len) < 0)
        return GSS_S_DEFECTIVE_TOKEN;

    if (*(*buff_in)++ != BIT_STRING)
        return GSS_S_DEFECTIVE_TOKEN;

    if (*(*buff_in)++ != BIT_STRING_LENGTH)
        return GSS_S_DEFECTIVE_TOKEN;

    if (*(*buff_in)++ != BIT_STRING_PADDING)
        return GSS_S_DEFECTIVE_TOKEN;

    *req_flags = (OM_uint32) (*(*buff_in)++ >> 1);
    return (0);
}

/*
 * verify that buff_in points to an octet string, if it does not,
 * return NULL and don't advance the pointer. If it is an octet string
 * decode buff_in into a gss_buffer_t and return it, advancing the
 * buffer pointer.
 */
static gss_buffer_t get_input_token(unsigned char **buff_in, unsigned int buff_length)
{
    gss_buffer_t input_token;
    unsigned int bytes;

    if (**buff_in != OCTET_STRING)
        return (NULL);

    (*buff_in)++;
    input_token = (gss_buffer_t)malloc(sizeof (gss_buffer_desc));

    if (input_token == NULL)
        return (NULL);

    input_token->length = gssint_get_der_length(buff_in, buff_length, &bytes);
    if ((int)input_token->length == -1)
    {
        free(input_token);
        return (NULL);
    }
    input_token->value = *buff_in;
    //input_token->value = malloc(input_token->length);

    //if (input_token->value == NULL) {
    //	free(input_token);
    //	return (NULL);
    //}

    //(void) memcpy(input_token->value, *buff_in, input_token->length);
    *buff_in += input_token->length;
    return (input_token);
}

unsigned int get_negTokenInit(OM_uint32 *minor_status, gss_buffer_t buf, gss_buffer_t der_mechSet, gss_OID_set *mechSet,
                              OM_uint32 *req_flags, gss_buffer_t *mechtok, gss_buffer_t *mechListMIC)
{
    OM_uint32 err;
    unsigned char *ptr, *bufstart;
    unsigned int len;
    gss_buffer_desc tmpbuf;

    *minor_status = 0;
    der_mechSet->length = 0;
    der_mechSet->value = NULL;
    *mechSet = GSS_C_NO_OID_SET;
    *req_flags = 0;
    *mechtok = *mechListMIC = GSS_C_NO_BUFFER;

    ptr = bufstart = buf->value;
#define REMAIN (buf->length - (ptr - bufstart))
    err = rtk_g_verify_token_header(gss_mech_spnego, &len, &ptr, 0, REMAIN);
    if (err)
    {
        *minor_status = err;
        return GSS_S_FAILURE;
    }
    *minor_status = g_verify_neg_token_init(&ptr, REMAIN);
    if (*minor_status)
        return GSS_S_FAILURE;

    /* alias into input_token */
    tmpbuf.value = ptr;
    tmpbuf.length = REMAIN;

    *mechSet = rkt_get_mech_set(minor_status, &ptr, REMAIN);//3 types of mechanism
    if (*mechSet == NULL)
        return GSS_S_FAILURE;

    tmpbuf.length = ptr - (unsigned char *)tmpbuf.value;
    der_mechSet->value = (void*)malloc(tmpbuf.length);
    if (der_mechSet->value == NULL)
        return GSS_S_FAILURE;
    memcpy(der_mechSet->value, tmpbuf.value, tmpbuf.length);
    der_mechSet->length = tmpbuf.length;

    err = get_req_flags(&ptr, REMAIN, req_flags);
    if (err != GSS_S_COMPLETE)
    {
        return err;
    }
    if (rtk_g_get_tag_and_length(&ptr, (CONTEXT | 0x02), REMAIN, &len) >= 0)
    {
        *mechtok = get_input_token(&ptr, len);
        if (*mechtok == GSS_C_NO_BUFFER)
        {
            return GSS_S_FAILURE;
        }
    }
    if (rtk_g_get_tag_and_length(&ptr, (CONTEXT | 0x03), REMAIN, &len) >= 0)
    {
        *mechListMIC = get_input_token(&ptr, len);
        if (*mechListMIC == GSS_C_NO_BUFFER)
        {
            return GSS_S_FAILURE;
        }
    }

    return GSS_S_COMPLETE;
#undef REMAIN
}

static gss_OID negotiate_mech_type(OM_uint32 *minor_status, gss_OID_set supported_mechSet, gss_OID_set mechset, OM_uint32 *negResult)
{
    gss_OID returned_mech;
    OM_uint32 status;
    int present;
    int i;

    for (i = 0; i < mechset->count; i++)
    {
        if (memcmp((mechset->elements[i]).elements, GSS_MECH_KRB5_WRONG_OID, (mechset->elements[i]).length) == 0)
        {
            memcpy((mechset->elements[i]).elements, GSS_MECH_KRB5_OID, GSS_MECH_KRB5_OID_LENGTH);
        }
        gss_test_oid_set_member(minor_status, &mechset->elements[i], supported_mechSet, &present);
        if (!present)
            continue;

        if (i == 0)
            *negResult = ACCEPT_INCOMPLETE;
        else
            *negResult = REQUEST_MIC;


        status = generic_gss_copy_oid(minor_status,
                                      &mechset->elements[i],
                                      &returned_mech);
        if (status != GSS_S_COMPLETE) {
            *negResult = REJECT;
            return (NULL);
        }
        return (returned_mech);
        //return (&mechset->elements[i]);
    }
    *negResult = REJECT;
    return (NULL);
}

char * gssint_get_modOptions(const gss_OID oid)
{
    gss_mech_info aMech;
    char *modOptions = NULL;

    /* make sure we have fresh data */
    //(void) k5_mutex_lock(&g_mechListLock);
    //updateMechList();

    if ((aMech = searchMechList(oid)) == NULL || aMech->optionStr == NULL)
    {
        //(void) k5_mutex_unlock(&g_mechListLock);
        return (NULL);
    }

    if (aMech->optionStr)
    {
        modOptions = malloc(sizeof(aMech->optionStr));
        strcpy(modOptions, aMech->optionStr);
    }
    //(void) k5_mutex_unlock(&g_mechListLock);

    return (modOptions);

} /* gssint_get_modOptions */


static void
check_spnego_options(spnego_gss_ctx_id_t spnego_ctx)
{
    spnego_ctx->optionStr = gssint_get_modOptions((const gss_OID)&spnego_oids[0]);
}

static spnego_gss_ctx_id_t create_spnego_ctx(void)
{
    spnego_gss_ctx_id_t spnego_ctx = NULL;
    spnego_ctx = (spnego_gss_ctx_id_t)malloc(sizeof (spnego_gss_ctx_id_rec));

    if (spnego_ctx == NULL)
    {
        return (NULL);
    }

    spnego_ctx->magic_num = SPNEGO_MAGIC_ID;
    spnego_ctx->ctx_handle = GSS_C_NO_CONTEXT;
    spnego_ctx->internal_mech = NULL;
    spnego_ctx->optionStr = NULL;
    spnego_ctx->DER_mechTypes.length = 0;
    spnego_ctx->DER_mechTypes.value = NULL;
    spnego_ctx->default_cred = GSS_C_NO_CREDENTIAL;
    spnego_ctx->mic_reqd = 0;
    spnego_ctx->mic_sent = 0;
    spnego_ctx->mic_rcvd = 0;
    spnego_ctx->mech_complete = 0;
    spnego_ctx->nego_done = 0;
    spnego_ctx->internal_name = GSS_C_NO_NAME;
    spnego_ctx->actual_mech = GSS_C_NO_OID;

    check_spnego_options(spnego_ctx);

    return (spnego_ctx);
}

OM_uint32 gss_release_oid_set(OM_uint32 *minor_status, gss_OID_set *set)
{
    OM_uint32 index;
    gss_OID oid;
    if (minor_status)
        *minor_status = 0;

    if (set ==NULL)
        return GSS_S_COMPLETE;

    if (*set == GSS_C_NULL_OID_SET)
        return(GSS_S_COMPLETE);

    for (index=0; (unsigned int)index<(*set)->count; index++)
    {
        oid = &(*set)->elements[index];
        //free(oid->elements);
    }
    free((*set)->elements);
    free(*set);

    *set = GSS_C_NULL_OID_SET;

    return(GSS_S_COMPLETE);
}

static OM_uint32 rtk_acc_ctx_new(OM_uint32 *minor_status, gss_buffer_t buf, gss_ctx_id_t *context_handle,
                                 gss_cred_id_t server_creds, gss_buffer_t *mechToken, gss_buffer_t *mechListMIC,
                                 OM_uint32 *negState, send_token_flag *return_token)
{
    OM_uint32 tmpmin, ret, req_flags;
    gss_OID_set supported_mechSet, mechTypes;
    gss_buffer_desc der_mechTypes;
    gss_OID mech_wanted;
    spnego_gss_ctx_id_t sc = NULL;

    *context_handle = GSS_C_NO_CONTEXT;
    ret = GSS_S_DEFECTIVE_TOKEN;
    der_mechTypes.length = 0;
    der_mechTypes.value = NULL;
    *mechToken = *mechListMIC = GSS_C_NO_BUFFER;
    supported_mechSet = mechTypes = GSS_C_NO_OID_SET;
    *return_token = ERROR_TOKEN_SEND;
    *negState = REJECT;
    *minor_status = 0;

    ret = get_negTokenInit(minor_status, buf, &der_mechTypes, &mechTypes, &req_flags, mechToken, mechListMIC);
    if (ret != GSS_S_COMPLETE)
    {
        goto cleanup;
    }
    if (server_creds != GSS_C_NO_CREDENTIAL)
    {
        ret = gss_inquire_cred(minor_status, server_creds, NULL, NULL,
                               NULL, &supported_mechSet);
        if (ret != GSS_S_COMPLETE)
        {
            *return_token = NO_TOKEN_SEND;
            goto cleanup;
        }
    }
    else
        goto cleanup;
    /*
     * Select the best match between the list of mechs
     * that the initiator requested and the list that
     * the acceptor will support.
     */
    mech_wanted = negotiate_mech_type(minor_status, supported_mechSet, /*we support */mechTypes, /*packet bring*/ negState);
    if (*negState == REJECT)
    {
        ret = GSS_S_BAD_MECH;
        goto cleanup;
    }
    sc = create_spnego_ctx();
    if (sc == NULL)
    {
        ret = GSS_S_FAILURE;
        *return_token = NO_TOKEN_SEND;
        generic_gss_release_oid(&tmpmin, &mech_wanted, 0);
        goto cleanup;
    }

    sc->internal_mech = mech_wanted;
    sc->DER_mechTypes = der_mechTypes;
    der_mechTypes.length = 0;
    der_mechTypes.value = NULL;

    if (*negState == REQUEST_MIC)
        sc->mic_reqd = 1;

    *return_token = INIT_TOKEN_SEND;
    sc->firstpass = 1;
    *context_handle = (gss_ctx_id_t)sc;
    ret = GSS_S_COMPLETE;
cleanup:
    gss_release_oid_set(&tmpmin, &mechTypes);
    free(supported_mechSet);
    //gss_release_oid_set(&tmpmin, &supported_mechSet);
    if (der_mechTypes.length != 0)
        gss_release_buffer(&tmpmin, &der_mechTypes);

    return ret;
}

/*
 * Perform the actual verification and/or generation of mechListMIC.
 */
static OM_uint32 process_mic(OM_uint32 *minor_status, gss_buffer_t mic_in, spnego_gss_ctx_id_t sc, gss_buffer_t *mic_out,
                             OM_uint32 *negState, send_token_flag *tokflag)
{
    OM_uint32 ret, tmpmin;
    gss_qop_t qop_state;
    gss_buffer_desc tmpmic = GSS_C_EMPTY_BUFFER;
    ret = GSS_S_FAILURE;
    if (mic_in != GSS_C_NO_BUFFER)
    {
        ret = rtk_gss_verify_mic(minor_status, sc->ctx_handle, &sc->DER_mechTypes, mic_in, &qop_state);
        if (ret != GSS_S_COMPLETE)
        {
            *negState = REJECT;
            *tokflag = ERROR_TOKEN_SEND;
            return ret;
        }
        sc->mic_reqd = 1;
        sc->mic_rcvd = 1;
    }
    if (sc->mic_reqd && !sc->mic_sent)
    {
        ret = gss_get_mic(minor_status, sc->ctx_handle, GSS_C_QOP_DEFAULT, &sc->DER_mechTypes, &tmpmic);
        if (ret != GSS_S_COMPLETE)
        {
            gss_release_buffer(&tmpmin, &tmpmic);
            *tokflag = NO_TOKEN_SEND;
            return ret;
        }
        *mic_out = malloc(sizeof(gss_buffer_desc));
        if (*mic_out == GSS_C_NO_BUFFER)
        {
            gss_release_buffer(&tmpmin, &tmpmic);
            *tokflag = NO_TOKEN_SEND;
            return GSS_S_FAILURE;
        }
        **mic_out = tmpmic;
        sc->mic_sent = 1;
    }
    return GSS_S_COMPLETE;
}

/*
 * Both initiator and acceptor call here to verify and/or create
 * mechListMIC, and to consistency-check the MIC state.
 */
static OM_uint32 handle_mic(OM_uint32 *minor_status, gss_buffer_t mic_in, int send_mechtok, spnego_gss_ctx_id_t sc,
                            gss_buffer_t *mic_out, OM_uint32 *negState, send_token_flag *tokflag)
{
    OM_uint32 ret;
    ret = GSS_S_FAILURE;
    *mic_out = GSS_C_NO_BUFFER;
    if (mic_in != GSS_C_NO_BUFFER)
    {
        if (sc->mic_rcvd)
        {
            *negState = REJECT;
            *tokflag = ERROR_TOKEN_SEND;
            return GSS_S_DEFECTIVE_TOKEN;
        }
    }
    else if (sc->mic_reqd && !send_mechtok)
    {
        *negState = REJECT;
        *tokflag = ERROR_TOKEN_SEND;
        return GSS_S_DEFECTIVE_TOKEN;
    }
    ret = process_mic(minor_status, mic_in, sc, mic_out, negState, tokflag);
    if (ret != GSS_S_COMPLETE)
    {
        return ret;
    }
    if (sc->mic_reqd)
    {
        assert(sc->mic_sent || sc->mic_rcvd);
    }
    if (sc->mic_sent && sc->mic_rcvd)
    {
        ret = GSS_S_COMPLETE;
        *negState = ACCEPT_COMPLETE;
        if (*mic_out == GSS_C_NO_BUFFER)
        {
            assert(!send_mechtok);
            *tokflag = NO_TOKEN_SEND;
        } else {
            *tokflag = CONT_TOKEN_SEND;
        }
    }
    else if (sc->mic_reqd)
    {
        *negState = ACCEPT_INCOMPLETE;
        ret = GSS_S_CONTINUE_NEEDED;
    }
    else if (*negState == ACCEPT_COMPLETE)
    {
        ret = GSS_S_COMPLETE;
    }
    else
    {
        ret = GSS_S_CONTINUE_NEEDED;
    }
    return ret;
}

OM_uint32 rtk_spnego_gss_accept_sec_context(OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_cred_id_t server_creds,
        gss_buffer_t input_token, gss_channel_bindings_t input_chan_bindings, gss_name_t *src_name,
        gss_OID *mech_type, gss_buffer_t output_token, OM_uint32 *ret_flags, OM_uint32 *time_rec,
        gss_cred_id_t *delegated_cred_handle)
{
    OM_uint32 ret, tmpret, negState;
    send_token_flag return_token;
    gss_buffer_t mechtok_in, mic_in, mic_out;
    gss_buffer_desc mechtok_out = GSS_C_EMPTY_BUFFER;
    spnego_gss_ctx_id_t sc = NULL;
    OM_uint32 mechstat = GSS_S_FAILURE;

    mechtok_in = mic_in = mic_out = GSS_C_NO_BUFFER;

    if (*context_handle == GSS_C_NO_CONTEXT)
    {
        if (src_name != NULL)
            *src_name = GSS_C_NO_NAME;
        if (mech_type != NULL)
            *mech_type = GSS_C_NO_OID;
        if (time_rec != NULL)
            *time_rec = 0;
        if (ret_flags != NULL)
            *ret_flags = 0;
        if (delegated_cred_handle != NULL)
            *delegated_cred_handle = GSS_C_NO_CREDENTIAL;
        /* Can set negState to REQUEST_MIC */
        ret = rtk_acc_ctx_new(minor_status, input_token, context_handle, server_creds, &mechtok_in, &mic_in, &negState, 
        		&return_token);
        if (ret != GSS_S_COMPLETE)
            goto cleanup;
        ret = GSS_S_CONTINUE_NEEDED;
    }
    sc = (spnego_gss_ctx_id_t)*context_handle;
    mechstat = GSS_S_FAILURE;
    if (negState != REQUEST_MIC && mechtok_in != GSS_C_NO_BUFFER)
    {
        ret = rtk_acc_ctx_call_acc(minor_status, sc, server_creds, mechtok_in, mech_type, &mechtok_out, ret_flags, 
			time_rec, delegated_cred_handle, &negState, &return_token);
    }
    else if (negState == REQUEST_MIC)
    {
        mechstat = GSS_S_CONTINUE_NEEDED;
    }
    if (!HARD_ERROR(ret) && sc->mech_complete && (sc->ctx_flags & GSS_C_INTEG_FLAG))
    {
        ret = handle_mic(minor_status, mic_in, (mechtok_out.length != 0), sc, &mic_out, &negState, &return_token);
    }
cleanup:
    if (return_token != NO_TOKEN_SEND && return_token != CHECK_MIC)
    {
        tmpret = rtk_make_spnego_tokenTarg_msg(negState, sc->internal_mech, &mechtok_out, mic_out, return_token, 
			output_token);
        if (tmpret < 0)
            ret = GSS_S_FAILURE;
    }
    if (ret == GSS_S_COMPLETE)
    {
        //*context_handle = (gss_ctx_id_t)sc->ctx_handle;
        if (sc->internal_name != GSS_C_NO_NAME && src_name != NULL)
        {
            *src_name = sc->internal_name;
            sc->internal_name = GSS_C_NO_NAME;
        }
    }
    else if (ret != GSS_S_CONTINUE_NEEDED)
    {
        if (sc != NULL)
        {
            //rtk_spnego_gss_delete_sec_context(&tmpmin, (gss_ctx_id_t *)&sc, NULL);
            //gss_delete_sec_context(&tmpmin, &sc->ctx_handle,
            //		       GSS_C_NO_BUFFER);
            //release_spnego_ctx(&sc);
        }
        //*context_handle = GSS_C_NO_CONTEXT;
    }
    free(mechtok_in);
    free(mechtok_out.value);
    //free(sc->internal_mech->elements);
    //free(sc->internal_mech);
    return ret;
}

/*
 * Wrap call to gss_accept_sec_context() and update state
 * accordingly.
 */
OM_uint32 rtk_acc_ctx_call_acc(OM_uint32 *minor_status, spnego_gss_ctx_id_t sc, gss_cred_id_t server_creds, gss_buffer_t mechtok_in,
                               gss_OID *mech_type, gss_buffer_t output_token, OM_uint32 *ret_flags, OM_uint32 *time_rec,
                               gss_cred_id_t *delegated_cred_handle, OM_uint32 *negState, send_token_flag *tokflag)
{
    OM_uint32 ret;
    gss_OID_desc mechoid;

    /*
     * mechoid is an alias; don't free it.
     */
    ret = gssint_get_mech_type(&mechoid, mechtok_in);
    if (ret != GSS_S_COMPLETE)
    {
        *tokflag = NO_TOKEN_SEND;
        return ret;
    }
    ret = acc_ctx_vfy_oid(minor_status, sc, &mechoid, negState, tokflag);
    if (ret != GSS_S_COMPLETE)
        return ret;

    ret = gss_accept_sec_context(minor_status, &sc->ctx_handle, server_creds, mechtok_in, GSS_C_NO_CHANNEL_BINDINGS,
                                 &sc->internal_name, mech_type, output_token, &sc->ctx_flags, time_rec, delegated_cred_handle);
    if (ret == GSS_S_COMPLETE)
    {
#ifdef MS_BUG_TEST
        /*
         * Force MIC to be not required even if we previously
         * requested a MIC.
         */
        char *envstr = getenv("MS_FORCE_NO_MIC");

        if (envstr != NULL && strcmp(envstr, "1") == 0 &&
                !(sc->ctx_flags & GSS_C_MUTUAL_FLAG) &&
                sc->mic_reqd) {

            sc->mic_reqd = 0;
        }
#endif
        sc->mech_complete = 1;
        if (ret_flags != NULL)
            *ret_flags = sc->ctx_flags;

        if (!sc->mic_reqd)
        {
            *negState = ACCEPT_COMPLETE;
            ret = GSS_S_COMPLETE;
        }
        else
        {
            ret = GSS_S_CONTINUE_NEEDED;
        }
    }
    else if (ret != GSS_S_CONTINUE_NEEDED)
    {
        *negState = REJECT;
        *tokflag = ERROR_TOKEN_SEND;
    }
    return ret;
}

int spnego_worker_bee(void *param, gss_buffer_t auth)
{
    struct _work_plan *work = (struct _work_plan*)param;
    int ret = rtk_spnego_sign_server(work->s, work->server_creds, auth);

    return ret;
}
#ifdef TEST_GSSAPI
const unsigned char rep_http_header[] =
{
    0x48,0x54,0x54,0x50,0x2f,0x31,0x2e,0x31,0x20,0x32,0x30,0x30,0x20,0x4f,0x4b,0x0d
    ,0x0a, 0x44,0x61,0x74,0x65,0x3a,0x20,0x4d,0x6f,0x6e,0x2c,0x20,0x30,0x38,0x20,0x46,0x65
    ,0x62,0x20,0x32,0x30,0x31,0x30,0x20,0x30,0x33,0x3a,0x30,0x36,0x3a,0x34,0x30,0x20
    ,0x47,0x4d,0x54,0x0d,0x0a,
    0x53,0x65,0x72,0x76,0x65,0x72,0x3a,0x20,0x41,0x70,0x61,0x63,0x68,0x65,0x2f,0x32
    ,0x2e,0x32,0x2e,0x38,0x20,0x28,0x46,0x65,0x64,0x6f,0x72,0x61,0x29,0x0d,0x0a,
    0x57,0x57,0x57,0x2d,0x41,0x75,0x74,0x68,0x65,0x6e,0x74,0x69,0x63,0x61,0x74,0x65
    ,0x3a,0x20,0x4e,0x65,0x67,0x6f,0x74,0x69,0x61,0x74,0x65,0x20
};
const unsigned char rep_content_header[] =
{
    0x0d, 0x0a,0x4c,0x61,0x73,0x74,0x2d,0x4d,0x6f,0x64,0x69,0x66,0x69,0x65,0x64,0x3a,0x20,0x4d
    ,0x6f,0x6e,0x2c,0x20,0x30,0x38,0x20,0x46,0x65,0x62,0x20,0x32,0x30,0x31,0x30,0x20
    ,0x30,0x32,0x3a,0x34,0x38,0x3a,0x31,0x32,0x20,0x47,0x4d,0x54,0x0d,0x0a,0x45,0x54,0x61,0x67,
    0x3a,0x20,0x22,0x36,0x37,0x65,0x35,0x31,0x2d,0x63,0x2d,0x34,0x37,0x66,0x30,0x64,0x64,0x35,
    0x30,0x34,0x62,0x33,0x30,0x30,0x22,0x0d,0x0a,0x41,0x63,0x63,0x65,0x70,0x74,0x2d,0x52,0x61,
    0x6e,0x67,0x65,0x73,0x3a,0x20,0x62,0x79,0x74,0x65,0x73,0x0d,0x0a,0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x4c,0x65,0x6e,0x67,0x74,0x68,0x3a,0x20
    ,0x31,0x32,0x0d,0x0a,0x43,0x6f,0x6e,0x6e,0x65,0x63,0x74,0x69,0x6f,0x6e,0x3a,0x20,0x63,0x6c,0x6f,0x73
    ,0x65,0x0d,0x0a,0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x54,0x79,0x70,0x65,0x3a,0x20,0x74,0x65
    ,0x78,0x74,0x2f,0x68,0x74,0x6d,0x6c,0x3b,0x20,0x63,0x68,0x61,0x72,0x73,0x65,0x74
    ,0x3d,0x55,0x54,0x46,0x2d,0x38,0x0d,0x0a,0x0d,0x0a,0x74,0x65,0x73,0x74,0x77,0x65,0x62,0x72,0x6f,0x6f,0x74,0x0a
};
#endif

static int rtk_spnego_server_establish_context(RTSkt* s, gss_cred_id_t server_creds, gss_ctx_id_t *context, gss_buffer_t client_name,
        OM_uint32 *ret_flags, gss_buffer_t auth)
{
    gss_buffer_desc send_tok, recv_tok;
    gss_name_t client;
    gss_OID doid;
    OM_uint32 maj_stat, min_stat, acc_sec_min_stat;
#ifndef TEST_GSSAPI
    recv_tok.length = auth->length;
    recv_tok.value = auth->value;
#else
    recv_tok.length = sizeof(test);
    recv_tok.value = (void*)test;
#endif
    *context = GSS_C_NO_CONTEXT;
    do
    {
        maj_stat = rtk_spnego_gss_accept_sec_context(&acc_sec_min_stat, context, server_creds, &recv_tok, GSS_C_NO_CHANNEL_BINDINGS,
                   &client, &doid, &send_tok, ret_flags, NULL,	/* ignore time_rec */ NULL); /* ignore del_cred_handle */
        if (maj_stat ==0 && send_tok.length != 0)
        {
#ifndef TEST_GSSAPI
            gss_release_buffer(&min_stat, auth);
            maj_stat = gss_release_name(&min_stat, &client);
            if (maj_stat != GSS_S_COMPLETE)
            {
                return -1;
            }
            auth->length = send_tok.length;
            auth->value = send_tok.value;
#else
            int tmplen = send_tok.length*2;
            unsigned char*tmp = (unsigned char*)malloc( tmplen );
            memset(tmp, 0, tmplen );
            b64_encode(tmp, &tmplen, (unsigned char*)send_tok.value, tmplen/2);
            pkt = allocPkt(tmplen + sizeof(rep_http_header) + sizeof(rep_content_header));
            memcpy(pkt->wp, (char*)rep_http_header, sizeof(rep_http_header));
            memcpy(pkt->wp + sizeof(rep_http_header), (char*)tmp, tmplen);
            memcpy(pkt->wp + sizeof(rep_http_header) + tmplen, (char*)rep_content_header, sizeof(rep_content_header));
            pkt->len = tmplen + sizeof(rep_http_header) + sizeof(rep_content_header);
            rtSktSend(s, pkt, tmplen + sizeof(rep_http_header) + sizeof(rep_content_header));
            free(tmp);
#endif
        }
#ifdef TEST_GSSAPI
        s= stopconn(s);
        (void) gss_release_buffer(&min_stat, &send_tok);
#endif
        if (maj_stat != GSS_S_COMPLETE && maj_stat != GSS_S_CONTINUE_NEEDED)
        {
            if (*context != GSS_C_NO_CONTEXT)
                rtk_spnego_gss_delete_sec_context(&min_stat, context, GSS_C_NO_BUFFER);
            return -1;
        }
    } while (maj_stat == GSS_S_CONTINUE_NEEDED);
    return 0;
}

int rtk_spnego_sign_server(RTSkt * s, gss_cred_id_t server_creds, gss_buffer_t auth)
{
    gss_buffer_desc client_name;
    OM_uint32 maj_stat, min_stat;
    int    /* i, conf_state,*/ ret_flags;
    gss_ctx_id_t context = 0;
    if (rtk_spnego_server_establish_context(s, server_creds, &context,
                                            &client_name, (OM_uint32 *)&ret_flags, auth) < 0)
        return (-1);
    if (context != GSS_C_NO_CONTEXT)
    {
        maj_stat = rtk_spnego_gss_delete_sec_context(&min_stat, &context, NULL);
        if (maj_stat != GSS_S_COMPLETE)
        {
            return (-1);
        }
    }
    return 0;
}

int rtk_server_acquire_creds(char *service_name, gss_cred_id_t *server_creds)
{
    gss_buffer_desc name_buf;
    gss_name_t server_name;
    unsigned int min_stat;

    name_buf.value = service_name;
    name_buf.length = strlen((char*)name_buf.value) + 1;
    rtk_gss_import_name(&min_stat, &name_buf, (gss_OID) gss_nt_service_name, &server_name);

    rtk_gss_acquire_cred(&min_stat, server_name/*host*/, 0, GSS_C_NULL_OID_SET, GSS_C_ACCEPT, server_creds, NULL, NULL);
    return 0;
}

#if 0
const unsigned char authorization_required [] =
    {   0x48,0x54,0x54,0x50,0x2f,0x31,0x2e,0x31,0x20,0x34
        ,0x30,0x31,0x20,0x41,0x75,0x74,0x68,0x6f,0x72,0x69,0x7a,0x61,0x74,0x69,0x6f,0x6e
        ,0x20,0x52,0x65,0x71,0x75,0x69,0x72,0x65,0x64,0x0d,0x0a,0x44,0x61,0x74,0x65,0x3a
        ,0x20,0x4d,0x6f,0x6e,0x2c,0x20,0x30,0x38,0x20,0x46,0x65,0x62,0x20,0x32,0x30,0x31
        ,0x30,0x20,0x30,0x33,0x3a,0x30,0x36,0x3a,0x34,0x30,0x20,0x47,0x4d,0x54,0x0d,0x0a
        ,0x53,0x65,0x72,0x76,0x65,0x72,0x3a,0x20,0x41,0x70,0x61,0x63,0x68,0x65,0x2f,0x32
        ,0x2e,0x32,0x2e,0x38,0x20,0x28,0x46,0x65,0x64,0x6f,0x72,0x61,0x29,0x0d,0x0a,0x57
        ,0x57,0x57,0x2d,0x41,0x75,0x74,0x68,0x65,0x6e,0x74,0x69,0x63,0x61,0x74,0x65,0x3a
        ,0x20,0x4e,0x65,0x67,0x6f,0x74,0x69,0x61,0x74,0x65,0x0d,0x0a,0x43,0x6f,0x6e,0x74
        ,0x65,0x6e,0x74,0x2d,0x4c,0x65,0x6e,0x67,0x74,0x68,0x3a,0x20,0x34,0x38,0x35,0x0d
        ,0x0a,0x43,0x6f,0x6e,0x6e,0x65,0x63,0x74,0x69,0x6f,0x6e,0x3a,0x20,0x63,0x6c,0x6f
        ,0x73,0x65,0x0d,0x0a,0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x54,0x79,0x70,0x65
        ,0x3a,0x20,0x74,0x65,0x78,0x74,0x2f,0x68,0x74,0x6d,0x6c,0x3b,0x20,0x63,0x68,0x61
        ,0x72,0x73,0x65,0x74,0x3d,0x69,0x73,0x6f,0x2d,0x38,0x38,0x35,0x39,0x2d,0x31,0x0d
        ,0x0a,0x0d,0x0a,0x3c,0x21,0x44,0x4f,0x43,0x54,0x59,0x50,0x45,0x20,0x48,0x54,0x4d
        ,0x4c,0x20,0x50,0x55,0x42,0x4c,0x49,0x43,0x20,0x22,0x2d,0x2f,0x2f,0x49,0x45,0x54
        ,0x46,0x2f,0x2f,0x44,0x54,0x44,0x20,0x48,0x54,0x4d,0x4c,0x20,0x32,0x2e,0x30,0x2f
        ,0x2f,0x45,0x4e,0x22,0x3e,0x0a,0x3c,0x68,0x74,0x6d,0x6c,0x3e,0x3c,0x68,0x65,0x61
        ,0x64,0x3e,0x0a,0x3c,0x74,0x69,0x74,0x6c,0x65,0x3e,0x34,0x30,0x31,0x20,0x41,0x75
        ,0x74,0x68,0x6f,0x72,0x69,0x7a,0x61,0x74,0x69,0x6f,0x6e,0x20,0x52,0x65,0x71,0x75
        ,0x69,0x72,0x65,0x64,0x3c,0x2f,0x74,0x69,0x74,0x6c,0x65,0x3e,0x0a,0x3c,0x2f,0x68
        ,0x65,0x61,0x64,0x3e,0x3c,0x62,0x6f,0x64,0x79,0x3e,0x0a,0x3c,0x68,0x31,0x3e,0x41
        ,0x75,0x74,0x68,0x6f,0x72,0x69,0x7a,0x61,0x74,0x69,0x6f,0x6e,0x20,0x52,0x65,0x71
        ,0x75,0x69,0x72,0x65,0x64,0x3c,0x2f,0x68,0x31,0x3e,0x0a,0x3c,0x70,0x3e,0x54,0x68
        ,0x69,0x73,0x20,0x73,0x65,0x72,0x76,0x65,0x72,0x20,0x63,0x6f,0x75,0x6c,0x64,0x20
        ,0x6e,0x6f,0x74,0x20,0x76,0x65,0x72,0x69,0x66,0x79,0x20,0x74,0x68,0x61,0x74,0x20
        ,0x79,0x6f,0x75,0x0a,0x61,0x72,0x65,0x20,0x61,0x75,0x74,0x68,0x6f,0x72,0x69,0x7a
        ,0x65,0x64,0x20,0x74,0x6f,0x20,0x61,0x63,0x63,0x65,0x73,0x73,0x20,0x74,0x68,0x65
        ,0x20,0x64,0x6f,0x63,0x75,0x6d,0x65,0x6e,0x74,0x0a,0x72,0x65,0x71,0x75,0x65,0x73
        ,0x74,0x65,0x64,0x2e,0x20,0x20,0x45,0x69,0x74,0x68,0x65,0x72,0x20,0x79,0x6f,0x75
        ,0x20,0x73,0x75,0x70,0x70,0x6c,0x69,0x65,0x64,0x20,0x74,0x68,0x65,0x20,0x77,0x72
        ,0x6f,0x6e,0x67,0x0a,0x63,0x72,0x65,0x64,0x65,0x6e,0x74,0x69,0x61,0x6c,0x73,0x20
        ,0x28,0x65,0x2e,0x67,0x2e,0x2c,0x20,0x62,0x61,0x64,0x20,0x70,0x61,0x73,0x73,0x77
        ,0x6f,0x72,0x64,0x29,0x2c,0x20,0x6f,0x72,0x20,0x79,0x6f,0x75,0x72,0x0a,0x62,0x72
        ,0x6f,0x77,0x73,0x65,0x72,0x20,0x64,0x6f,0x65,0x73,0x6e,0x27,0x74,0x20,0x75,0x6e
        ,0x64,0x65,0x72,0x73,0x74,0x61,0x6e,0x64,0x20,0x68,0x6f,0x77,0x20,0x74,0x6f,0x20
        ,0x73,0x75,0x70,0x70,0x6c,0x79,0x0a,0x74,0x68,0x65,0x20,0x63,0x72,0x65,0x64,0x65
        ,0x6e,0x74,0x69,0x61,0x6c,0x73,0x20,0x72,0x65,0x71,0x75,0x69,0x72,0x65,0x64,0x2e
        ,0x3c,0x2f,0x70,0x3e,0x0a,0x3c,0x68,0x72,0x3e,0x0a,0x3c,0x61,0x64,0x64,0x72,0x65
        ,0x73,0x73,0x3e,0x41,0x70,0x61,0x63,0x68,0x65,0x2f,0x32,0x2e,0x32,0x2e,0x38,0x20
        ,0x28,0x46,0x65,0x64,0x6f,0x72,0x61,0x29,0x20,0x53,0x65,0x72,0x76,0x65,0x72,0x20
        ,0x61,0x74,0x20,0x61,0x64,0x74,0x65,0x73,0x74,0x2e,0x73,0x63,0x63,0x6d,0x74,0x65
        ,0x73,0x74,0x2e,0x63,0x6f,0x6d,0x20,0x50,0x6f,0x72,0x74,0x20,0x38,0x30,0x3c,0x2f
        ,0x61,0x64,0x64,0x72,0x65,0x73,0x73,0x3e,0x0a,0x3c,0x2f,0x62,0x6f,0x64,0x79,0x3e
        ,0x3c,0x2f,0x68,0x74,0x6d,0x6c,0x3e,0x0a
    };

void spnego(void *data)
{
    char *service_name = "host";

    gss_cred_id_t server_creds;
    struct _work_plan *work;
    RTSkt* s = 0;
    RTSkt *gssapics = 0;
    PKT * pkt = 0;
    int status = 0;
    //PKT * pkt2 = 0;
    //int count = 1;
    int state = 0;
    int tmp = 0;
    int datasize = 0;
    OM_uint32 min_stat;
    gss_buffer_desc auth_buffer;
    char *spnegoAuth = NULL;
    char *auth;
    s = rtSkt(IPv4, IP_PROTO_TCP);
    rtSktBind(s, 623);
    rtSktListen(s, 1);
    spnegoAuth = (char*)malloc(4096);
    memset(spnegoAuth, 0, 4096);
    rtk_server_acquire_creds(service_name, &server_creds);

begin:
    gssapics = rtSktAccept( s );

    while (gssapics)
    {
        while ( ( pkt = rtSktRx( gssapics, 1000, &status ) ) == 0 )
        {
            if ( status == SKTDISCONNECT || status == OS_ERR_TIMEOUT )
            {
                gssapics = stopconn( gssapics );
                goto begin;
                if (state == 2) //something wrong when negotiate
                    state = 0;
                //state = 0;
                //datasize = 0;
                break;
            }
        }

        memcpy(spnegoAuth+datasize, pkt->wp, pkt->len);

        *(spnegoAuth+datasize + pkt->len) = 0;
        if (state == 0 )
        {
            freePkt(pkt);
            datasize = 0;
            if (strstr(spnegoAuth, "Authorization: Negotiate") < 0)
            {
                continue;
            }
            else
            {
                state = 1;
                tmp = sizeof(authorization_required);
                pkt = allocPkt(tmp);
                memset(pkt->wp, 0, sizeof(authorization_required));
                memcpy(pkt->wp, authorization_required, tmp);
                pkt->len = sizeof(authorization_required);
                rtSktSend(gssapics, pkt, sizeof(authorization_required));
                //gssapics = stopconn( gssapics );
                //goto begin;
            }
        }
        else
        {
            char *ptr = strstr(spnegoAuth,"\r\n\r\n");
            datasize += pkt->len;
            freePkt(pkt);
            if (!ptr)
            {
                continue;
            }
            state = 2;
            datasize = 0;
            pkt = 0;
            auth = strstr(spnegoAuth, "Authorization");
            if (auth)
            {
                auth_buffer.value = (void*)(auth + strlen("Authorization: Negotiate "));
                auth = strstr(auth_buffer.value, "\r\n\r\n");
                if (auth)
                {
                    auth_buffer.length = auth-(char*)auth_buffer.value;
                }
                //tmpauth = (unsigned char*)malloc(auth_buffer.length);
                //memset(tmpauth,0,auth_buffer.length);
                auth_buffer.length = b64_decode((char*)auth_buffer.value, (unsigned char*)spnegoAuth, auth_buffer.length);
                auth_buffer.value = spnegoAuth;

                work = (struct _work_plan*)malloc(sizeof(struct _work_plan));
                work->server_creds = server_creds;
                work->s = gssapics;
                if (spnego_worker_bee((void*)work, &auth_buffer))
                {
                    printf("Authorization required\n");
                    state = 0;
                }
                state = 0;
                goto begin;
            }
        }
    }
    (void) gss_release_cred(&min_stat, &server_creds);
}
#endif

#ifdef TEST_SPNEGO
unsigned char my_ap_req_spnego []=
{
    0x60,0x82,0x05,0x0f,0x06,0x06,0x2b,0x06,0x01,0x05,0x05,0x02,0xa0,0x82,0x05,0x03,0x30,0x82,0x04,0xff,0xa0,0x24,0x30,0x22,0x06,0x09,0x2a,0x86,0x48,0x82
    ,0xf7,0x12,0x01,0x02,0x02,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x12,0x01,0x02,0x02,0x06,0x0a,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x02,0x02,0x0a,0xa2,0x82
    ,0x04,0xd5,0x04,0x82,0x04,0xd1,0x60,0x82,0x04,0xcd,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x12,0x01,0x02,0x02,0x01,0x00,0x6e,0x82,0x04,0xbc,0x30,0x82,0x04
    ,0xb8,0xa0,0x03,0x02,0x01,0x05,0xa1,0x03,0x02,0x01,0x0e,0xa2,0x07,0x03,0x05,0x00,0x20,0x00,0x00,0x00,0xa3,0x82,0x03,0xdd,0x61,0x82,0x03,0xd9,0x30,0x82
    ,0x03,0xd5,0xa0,0x03,0x02,0x01,0x05,0xa1,0x0e,0x1b,0x0c,0x53,0x43,0x43,0x4d,0x54,0x45,0x53,0x54,0x2e,0x43,0x4f,0x4d,0xa2,0x2a,0x30,0x28,0xa0,0x03,0x02
    ,0x01,0x02,0xa1,0x21,0x30,0x1f,0x1b,0x04,0x48,0x54,0x54,0x50,0x1b,0x17,0x64,0x65,0x6c,0x6c,0x2e,0x73,0x63,0x63,0x6d,0x74,0x65,0x73,0x74,0x2e,0x63,0x6f
    ,0x6d,0x3a,0x31,0x36,0x39,0x39,0x32,0xa3,0x82,0x03,0x90,0x30,0x82,0x03,0x8c,0xa0,0x03,0x02,0x01,0x17,0xa1,0x03,0x02,0x01,0x03,0xa2,0x82,0x03,0x7e,0x04
    ,0x82,0x03,0x7a,0x83,0x74,0x19,0x1b,0x1b,0x6a,0x68,0x64,0x77,0xaa,0x22,0x69,0x2f,0x91,0x7f,0x4f,0xb2,0xdc,0x83,0xd0,0xea,0xd5,0x50,0x41,0x32,0x80,0x38
    ,0xdb,0x15,0x8c,0x30,0x1d,0xb2,0xd1,0xbf,0x2d,0xee,0x4e,0xb1,0x48,0x41,0x59,0xc4,0x3c,0xa4,0x28,0x86,0xa9,0x23,0x60,0x7c,0x20,0x86,0xcc,0xc6,0xa8,0x8d
    ,0xa3,0x05,0xb1,0xd6,0xb3,0xd0,0x4d,0xb0,0x3f,0x02,0x21,0xfe,0x8e,0xbe,0x36,0xfc,0xd4,0xbf,0xe2,0x71,0x39,0xdd,0x58,0x1c,0x4e,0x54,0xe5,0x0b,0x6a,0x07
    ,0x2f,0x98,0xc0,0x27,0x2b,0xcc,0x9f,0xc3,0xe7,0x2b,0x5a,0x54,0xc0,0x38,0xfc,0x4f,0x53,0x87,0x36,0x88,0xfc,0xb1,0x49,0x2d,0x24,0x4c,0xed,0xcb,0x13,0xe6
    ,0x09,0xd7,0x94,0x26,0x26,0xcb,0xa3,0xed,0xa0,0xa6,0x11,0x59,0xa9,0x14,0xc9,0xc0,0xe1,0x90,0x50,0xee,0x0b,0x5d,0x0a,0x53,0xdb,0x9a,0x2f,0x99,0x85,0xbf
    ,0xa8,0xa6,0x62,0x5e,0x1d,0x2e,0x41,0x93,0x6f,0xf8,0x61,0xce,0xa2,0xbd,0x84,0x0e,0x94,0xeb,0x4d,0x43,0xa6,0x10,0x61,0xcf,0xdb,0x42,0xf3,0x8b,0x8b,0x7b
    ,0xc4,0x4c,0x2f,0xe8,0x79,0x3c,0x95,0x78,0x7a,0x18,0x74,0xee,0xc7,0x00,0xfb,0x30,0x81,0x33,0xb3,0xb4,0x37,0x7a,0x6c,0xde,0xa0,0x53,0x18,0xf3,0xde,0x54
    ,0x4d,0x1f,0x5e,0x07,0xf8,0x05,0x17,0xc3,0xa4,0xd5,0x27,0x74,0x93,0xd4,0x89,0xcc,0x05,0xe6,0x79,0xc0,0x0f,0xab,0x88,0x4b,0x7a,0x51,0x32,0x71,0xde,0xf0
    ,0x1d,0xfe,0x43,0x59,0xb4,0x48,0x3a,0x89,0x46,0x1f,0x9a,0x87,0xe1,0x93,0x8c,0xcd,0x35,0xd9,0x8c,0x2a,0xa8,0x4e,0x9e,0x8b,0x3c,0x2f,0xb7,0x69,0xc1,0x7b
    ,0x5e,0x18,0x2c,0xd1,0x4c,0xbd,0x6f,0xaf,0x38,0xeb,0x48,0x1e,0x6b,0x2e,0x77,0xa2,0x2c,0x0a,0xf5,0xa2,0xb9,0xc3,0xdc,0x2f,0xb1,0x45,0x03,0x5a,0x5e,0x59
    ,0x47,0x7d,0x0a,0xe3,0xf3,0x0a,0x27,0xf7,0xf9,0x59,0xb3,0x8c,0x2a,0xa4,0x6b,0x51,0x42,0xf9,0x7f,0xf5,0x58,0x04,0x70,0x43,0x68,0x7f,0xce,0x1c,0x71,0xde
    ,0x43,0x3e,0xa1,0x09,0x3f,0x12,0x2d,0x57,0xf6,0xcb,0xfe,0xca,0xc2,0x00,0x39,0xe0,0xf1,0x1d,0x4b,0xa1,0xda,0xb1,0xc8,0x63,0x58,0xc4,0x4c,0x42,0x40,0x91
    ,0xca,0xd3,0x41,0x2d,0x6d,0xcd,0x78,0xa6,0x8c,0x1b,0xb2,0x49,0x06,0x67,0x16,0x53,0xc8,0x87,0xa7,0x75,0x08,0x06,0x61,0xd8,0x3b,0x84,0x72,0xc4,0x97,0x4f
    ,0x9f,0xcd,0x0d,0xc6,0xf6,0x42,0xd2,0x64,0x8c,0x89,0x5d,0xee,0x87,0x90,0x00,0x1e,0x91,0xd2,0xb2,0xea,0x09,0x32,0x23,0xfc,0xa3,0x34,0x53,0x9e,0x54,0x92
    ,0x6f,0x0a,0xc8,0x6c,0x33,0x33,0x0a,0x93,0x76,0x70,0x14,0xc8,0x32,0x12,0xb7,0x05,0xb6,0xa7,0x02,0xae,0x14,0xae,0x3e,0x50,0x27,0xfd,0xeb,0xdb,0x40,0x91
    ,0x1e,0x53,0xc3,0x2d,0x84,0xf0,0x3e,0xed,0x55,0x0d,0xbb,0x3d,0xc7,0x6f,0xf8,0xdc,0x94,0xdc,0x86,0xe2,0x83,0x20,0xf1,0x91,0x46,0xcf,0xfe,0x39,0x60,0xc6
    ,0x9e,0xd2,0xc9,0x4f,0xe2,0x02,0x7a,0x26,0xb3,0xd1,0xea,0xac,0x71,0xf2,0x33,0xe1,0x55,0x02,0x94,0x13,0x6e,0xee,0xab,0x79,0x93,0x0e,0x9d,0x0e,0xa5,0x3c
    ,0xb6,0x3e,0x00,0xcf,0xe3,0x41,0x7b,0x1e,0xe4,0x63,0x80,0x12,0x94,0x8d,0x81,0x75,0xf6,0xcb,0x49,0x5d,0x1c,0x29,0x0d,0xff,0x37,0x99,0x5e,0x3c,0x8b,0x2d
    ,0x34,0x16,0x1e,0x21,0x21,0xf4,0x66,0x39,0xe6,0x1c,0xa0,0x16,0x86,0x32,0xb1,0x71,0xf7,0x60,0x9d,0x50,0xbd,0xdb,0x2a,0x62,0xb1,0xcf,0xfb,0x5d,0x62,0xe9
    ,0xb6,0x85,0x53,0x8f,0xfc,0x10,0xa7,0xb0,0x1b,0xb6,0xc1,0x8c,0xb1,0x56,0x71,0xb1,0xd3,0xae,0xf6,0xf8,0xde,0xa8,0xb3,0x6e,0xb0,0x2d,0xc8,0xac,0xac,0x55
    ,0x87,0x1d,0x9a,0xf6,0xf2,0x2c,0xfd,0x31,0x6c,0xb9,0xeb,0x7a,0x61,0x15,0xc8,0x7b,0xe2,0x1f,0x04,0x97,0xe9,0xdc,0x95,0x21,0x4c,0x2b,0xda,0xa7,0x0f,0x02
    ,0x79,0x64,0x8f,0x2a,0xfd,0xf4,0xb1,0x99,0x12,0x3a,0xa8,0x87,0x32,0x9a,0x83,0x77,0x73,0x0d,0x6f,0xc3,0xbf,0xb8,0xc2,0xac,0xfb,0x1b,0x79,0x97,0x9c,0x62
    ,0x7b,0x11,0xe3,0x4a,0x0c,0xc5,0x69,0x6c,0xf9,0xbb,0x58,0x68,0x01,0xa1,0x8a,0xbc,0x0d,0xbc,0x26,0x64,0x1d,0xcf,0x09,0x86,0xd7,0x4f,0x55,0x92,0xe8,0x9a
    ,0x23,0x07,0xd1,0xbe,0x9f,0xe7,0xb6,0x72,0xcc,0xc7,0x34,0x39,0xbb,0x24,0x46,0x46,0x03,0xcf,0xeb,0xe8,0x3c,0xc5,0x5e,0xcd,0x43,0x84,0x6d,0x6e,0x08,0x58
    ,0x87,0x72,0xb1,0x08,0x14,0x20,0x3a,0xae,0xad,0x6e,0xd0,0x8f,0x18,0xfc,0x69,0x2f,0x88,0x18,0xe1,0x3a,0x29,0xed,0x32,0x37,0xd4,0x80,0x3c,0x80,0x57,0xd7
    ,0x0f,0xc3,0x56,0x42,0x3b,0x2c,0x02,0x04,0x65,0x3f,0x99,0x55,0x0a,0x7d,0x8f,0xa2,0xce,0x56,0x30,0x70,0x9a,0x35,0x82,0xac,0x7d,0x96,0x6a,0xfe,0xad,0x09
    ,0x45,0xfc,0x55,0xed,0x50,0xea,0xd0,0x65,0xf5,0xba,0xb1,0x56,0x8b,0xcc,0x4a,0xc2,0x9c,0x0b,0xef,0xa7,0x33,0x92,0x36,0x94,0x7f,0x34,0x65,0x35,0x4d,0x27
    ,0x02,0xf5,0xae,0xdc,0x90,0xda,0xef,0x15,0xd7,0xd6,0xc4,0x01,0x6e,0xc8,0x63,0xb2,0xe8,0x2b,0xeb,0x9e,0x03,0xe2,0xe6,0xd3,0x06,0x89,0xcd,0x55,0xac,0xfa
    ,0x19,0x86,0x72,0x7e,0x25,0x97,0x74,0x6f,0x68,0x48,0xd7,0x7f,0xc8,0x28,0x38,0x95,0x96,0xd6,0x17,0x69,0x35,0xe8,0x38,0x5b,0xab,0xcf,0x1b,0xb9,0xe6,0x08
    ,0x28,0x95,0xce,0xbc,0xc3,0x85,0x90,0x4a,0xf8,0xae,0xd0,0x9c,0x46,0xd2,0x89,0xe6,0x81,0x4d,0x14,0xeb,0xb4,0x7e,0xf2,0xa4,0x81,0xc1,0x30,0x81,0xbe,0xa0
    ,0x03,0x02,0x01,0x17,0xa2,0x81,0xb6,0x04,0x81,0xb3,0x9c,0x4b,0x43,0xc9,0x87,0x03,0xee,0x14,0xf9,0x8d,0x6b,0x83,0x4e,0xe2,0xe7,0x3f,0xaa,0xbc,0x15,0xd9
    ,0x09,0x23,0x95,0x59,0x8b,0xa9,0xe3,0x4a,0xcf,0x35,0x91,0x6b,0xe6,0x54,0x5e,0xc1,0x16,0xa7,0x8a,0xe5,0xa9,0x10,0x82,0x9d,0x07,0x84,0x15,0xe8,0x47,0xd6
    ,0xaa,0xf6,0x63,0x40,0x54,0x67,0x7f,0x27,0x02,0xf3,0x37,0xe0,0x72,0x33,0x16,0x58,0x73,0x20,0x42,0x39,0x91,0x7b,0x45,0xe5,0x75,0x82,0xa3,0x31,0x2d,0x8e
    ,0x48,0x90,0x65,0xcc,0x1c,0x4b,0x93,0x76,0x22,0x5d,0xb0,0xf3,0xb7,0x01,0xbd,0xb1,0xfd,0x59,0xb3,0x81,0x14,0xbf,0x33,0xe0,0xa7,0x24,0x56,0x72,0x8f,0x89
    ,0xc7,0x12,0x58,0xda,0x27,0xc3,0xc7,0x95,0x95,0x9e,0xa8,0xf0,0x0e,0x8a,0xf5,0x3f,0xb5,0x35,0x1a,0xbb,0x6f,0xf9,0x7d,0xda,0x21,0xb0,0x25,0x13,0x09,0xda
    ,0x84,0xc4,0x7d,0x0e,0x02,0xd2,0x54,0xf1,0xa1,0xa7,0x1c,0x07,0xcf,0xf1,0x83,0x47,0x6d,0xae,0xfd,0xcb,0xe4,0xeb,0xa8,0xb7,0x26,0x18,0x4c,0x91,0x42,0x4f
    ,0x6f,0x7a,0xa6,0xd5,0xca,0x02,0x01,0x38,0x07
};

void test_spnego(void *data)
{
    krb5_error_code min_stat;
    char *service_name = "host";
    gss_cred_id_t server_creds;
    struct _work_plan *work;
    RTSkt* s = 0;
    RTSkt *gssapics = 0;
    PKT pkt;
    int status = 0;
    gss_buffer_desc auth_buffer;


    strcpy(dpconf->domain_realm, "SCCMTEST.COM");
    rtk_server_acquire_creds(service_name, &server_creds);

    work = (struct _work_plan*)malloc(sizeof(struct _work_plan));
    work->server_creds = server_creds;

    auth_buffer.value = my_ap_req_spnego;
    auth_buffer.length = sizeof(my_ap_req_spnego);

    spnego_worker_bee((void*)work, &auth_buffer);
    //}
    gss_release_cred(&min_stat, &server_creds);

    for (;;)
    {
        OSTimeDlyHMSM(1, 1, 1, 0);              // delay task for 1 second
    }
}
#endif
