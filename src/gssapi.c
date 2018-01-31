
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "aes.h"
//#include "krb5_md5.h"
#include "rc4.h"
#include "gssapi.h"
#include "kerberos.h"
#include "b64.h"
#include "x509.h"
#include "spnego.h"

extern void *dbgmalloc(int size, char *fn, int line);
extern void dbgfree(void *ptr, char *fn, int line);
extern OS_EVENT *SktSem;

extern gss_mechanism spnego_mech_configs[];
extern const gss_OID_desc * const gss_mech_krb5;
extern const gss_OID_desc * const gss_mech_krb5_old;
extern const gss_OID_desc * const gss_mech_krb5_wrong;
extern const gss_OID_desc * const gss_nt_krb5_name;
extern const gss_OID_desc * const gss_nt_krb5_principal;
extern const gss_OID_desc * const GSS_KRB5_NT_PRINCIPAL_NAME;
extern RTSkt *stopconn(RTSkt* cs);

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

//static gss_OID_set_desc g_mechSet = { 0, NULL };

static OM_uint32 val_sign_args(OM_uint32 *minor_status, gss_ctx_id_t context_handle, int qop_req,
                               gss_buffer_t message_buffer, gss_buffer_t msg_token);

gss_mech_info searchMechList(const gss_OID oid)
{
    gss_mech_info aMech = g_mechList;

    /* if oid is null -> then get default which is the first in the list */
    if (oid == GSS_C_NULL_OID)
        return (aMech);

    while (aMech != NULL)
    {
        if (g_OID_equal(aMech->mech_type, oid))
            return (aMech);
        aMech = aMech->next;
    }

    /* none found */
    return ((gss_mech_info) NULL);
} /* searchMechList */

void register_mech(gss_mechanism mech, const char *namestr, void *dl_handle)
{
    gss_mech_info cf, new_cf;

    new_cf = malloc(sizeof(*new_cf));
    if (new_cf == NULL)
        return;

    memset(new_cf, 0, sizeof(*new_cf));
    new_cf->kmodName = NULL;

    //new_cf->uLibName = strdup(namestr);
    new_cf->uLibName = (char *)malloc(strlen(namestr) + 1);
    strcpy(new_cf->uLibName, namestr);
    //new_cf->mechNameStr = strdup(mech->mechNameStr);
    new_cf->mechNameStr = (char *)malloc(strlen(mech->mechNameStr) + 1);
    strcpy(new_cf->mechNameStr, mech->mechNameStr);
    new_cf->mech_type = &mech->mech_type;
    new_cf->mech = mech;
    new_cf->next = NULL;

    if (g_mechList == NULL)
    {
        g_mechList = new_cf;
        g_mechListTail = new_cf;
        return;
    }
    else if (mech->priority < g_mechList->mech->priority) {
        new_cf->next = g_mechList;
        g_mechList = new_cf;
        return;
    }
    for (cf = g_mechList; cf != NULL; cf = cf->next)
    {
        if (cf->next == NULL || mech->priority < cf->next->mech->priority)
        {
            new_cf->next = cf->next;
            cf->next = new_cf;
            if (g_mechListTail == cf)
            {
                g_mechListTail = new_cf;
            }
            break;
        }
    }
}

void updateMechList(void)
{
    //extern gss_mechanism *krb5_gss_get_mech_configs(void);
    //extern gss_mechanism *spnego_gss_get_mech_configs(void);
    gss_mechanism *cflist;
    static int inited;

    if (inited)
        return;

    //cflist = krb5_gss_get_mech_configs();
    cflist = krb5_mech_configs;
    if (cflist == NULL)
        return;

    for ( ; *cflist != NULL; cflist++)
    {
        register_mech(*cflist, "<builtin krb5>", NULL);
    }
    cflist = spnego_mech_configs;
    if (cflist == NULL)
        return;
    for ( ; *cflist != NULL; cflist++)
    {
        register_mech(*cflist, "<builtin spnego>", NULL);
    }
    inited = 1;

} /* updateMechList */

gss_mechanism gssint_get_mechanism(gss_OID oid)
{
    //gss_mechanism * cflist = krb5_mech_configs;
    //gss_mech_info aMech = NULL;
    //register_mech(*cflist, "<builtin krb5>", NULL);
    //aMech = g_mechList;
    //return aMech->mech;
    gss_mech_info aMech;

    //if (gssint_initialize_library())
    //	return NULL;

//	(void) k5_mutex_lock(&g_mechListLock);
    /* check if the mechanism is already loaded */
    if ((aMech = searchMechList(oid)) != NULL && aMech->mech) {
        //	(void) k5_mutex_unlock(&g_mechListLock);
        return (aMech->mech);
    }

    /*
     * might need to re-read the configuration file before loading
     * the mechanism to ensure we have the latest info.
     */
    updateMechList();

    aMech = searchMechList(oid);

    /* is the mechanism present in the list ? */
    if (aMech == NULL)
    {
        //(void) k5_mutex_unlock(&g_mechListLock);
        return ((gss_mechanism)NULL);
    }

    /* has another thread loaded the mech */
    if (aMech->mech)
    {
        //(void) k5_mutex_unlock(&g_mechListLock);
        return (aMech->mech);
    }
    else
    {
        return NULL;
    }

}

OM_uint32 gss_release_buffer (OM_uint32 * minor_status,gss_buffer_t buffer)
{
    if (minor_status)
        *minor_status = 0;

    /* if buffer is NULL, return */

    if (buffer == GSS_C_NO_BUFFER)
        return(GSS_S_COMPLETE);

    if ((buffer->length) && (buffer->value))
    {
        free(buffer->value);
        buffer->length = 0;
        buffer->value = NULL;
    }

    return (GSS_S_COMPLETE);
}


OM_uint32 gss_unseal (OM_uint32 *minor_status, gss_ctx_id_t	context_handle, gss_buffer_t input_message_buffer,
                      gss_buffer_t	output_message_buffer, int *conf_state, int *qop_state)
{
    /* EXPORT DELETE START */
    OM_uint32		status;
    gss_union_ctx_id_t	ctx;
    gss_mechanism	mech;

    if (minor_status != NULL)
        *minor_status = 0;

    if (output_message_buffer != GSS_C_NO_BUFFER)
    {
        output_message_buffer->length = 0;
        output_message_buffer->value = NULL;
    }

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (context_handle == GSS_C_NO_CONTEXT)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);

    if (input_message_buffer == GSS_C_NO_BUFFER || GSS_EMPTY_BUFFER(input_message_buffer))

        return (GSS_S_CALL_INACCESSIBLE_READ);

    if (output_message_buffer == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) context_handle;
    mech = gssint_get_mechanism (ctx->mech_type);

    if (mech)
    {
        if (mech->gss_unseal)
            status = mech->gss_unseal(mech->context, minor_status, ctx->internal_ctx_id, input_message_buffer,
                                      output_message_buffer, conf_state, qop_state);
        else
            status = GSS_S_UNAVAILABLE;

        return(status);
    }

    /* EXPORT DELETE END */

    return (GSS_S_BAD_MECH);
}

OM_uint32 gss_unwrap(OM_uint32 *minor_status, gss_ctx_id_t context_handle, gss_buffer_t input_message_buffer,
                     gss_buffer_t output_message_buffer, int *conf_state, gss_qop_t *qop_state)
{
    return (gss_unseal(minor_status, (gss_ctx_id_t)context_handle, (gss_buffer_t)input_message_buffer,
                       output_message_buffer, conf_state, (int *) qop_state));
}

OM_uint32 gss_sign(OM_uint32 *minor_status, gss_ctx_id_t context_handle, int qop_req, gss_buffer_t message_buffer,
                   gss_buffer_t msg_token)
{
    OM_uint32		status;
    gss_union_ctx_id_t	ctx;
    gss_mechanism	mech;

    status = val_sign_args(minor_status, context_handle, qop_req, message_buffer, msg_token);
    if (status != GSS_S_COMPLETE)
        return (status);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) context_handle;
    mech = gssint_get_mechanism (ctx->mech_type);

    if (mech)
    {
        if (mech->gss_sign)
            status = mech->gss_sign(mech->context, minor_status, ctx->internal_ctx_id, qop_req, message_buffer, msg_token);
        else
            status = GSS_S_UNAVAILABLE;

        return(status);
    }

    return (GSS_S_BAD_MECH);
}

OM_uint32 gss_get_mic(OM_uint32 *minor_status, gss_ctx_id_t context_handle, gss_qop_t qop_req, gss_buffer_t message_buffer,
                      gss_buffer_t		msg_token)
{
    return (gss_sign(minor_status, context_handle, (int) qop_req, message_buffer, msg_token));
}

/* This is trivial since we're a single mechanism implementation */

static OM_uint32 val_canon_name_args(OM_uint32 *minor_status, const gss_name_t input_name, const gss_OID mech_type,
                                     gss_name_t *output_name)
{

    /* Initialize outputs. */

    if (minor_status != NULL)
        *minor_status = 0;

    if (output_name != NULL)
        *output_name = GSS_C_NO_NAME;

    /* Validate arguments. */

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (input_name == GSS_C_NO_NAME || mech_type == GSS_C_NULL_OID)
        return (GSS_S_CALL_INACCESSIBLE_READ);

    return (GSS_S_COMPLETE);
}

OM_uint32 gssint_release_internal_name (OM_uint32 *minor_status, gss_OID mech_type, gss_name_t *internal_name)
{
    OM_uint32		status;
    gss_mechanism	mech;

    mech = gssint_get_mechanism (mech_type);
    if (mech)
    {
        status = krb5_gss_release_name(minor_status, internal_name);
        //if (mech->gss_release_name)
        //    status = mech->gss_release_name (
        //				     //mech->context,
        //				     minor_status,
        //				     internal_name);

        //status = GSS_S_UNAVAILABLE;

        return (status);
    }

    return (GSS_S_BAD_MECH);
}

OM_uint32 gss_canonicalize_name(OM_uint32 *minor_status, const gss_name_t input_name, const gss_OID mech_type,
                                gss_name_t *output_name)
{
    gss_union_name_t in_union, out_union = NULL, dest_union = NULL;
    OM_uint32 major_status = GSS_S_FAILURE;

    major_status = val_canon_name_args(minor_status, input_name, mech_type, output_name);

    if (major_status != GSS_S_COMPLETE)
        return (major_status);

    /* Initial value needed below. */
    major_status = GSS_S_FAILURE;

    in_union = (gss_union_name_t)input_name;
    /*
     * If the caller wants to reuse the name, and the name has already
     * been converted, then there is nothing for us to do.
     */
    if (!output_name && in_union->mech_type && g_OID_equal(in_union->mech_type, mech_type))
        return (GSS_S_COMPLETE);

    /* ok, then we need to do something - start by creating data struct */
    if (output_name)
    {
        out_union = (gss_union_name_t)malloc(sizeof (gss_union_name_desc));
        if (!out_union)
            goto allocation_failure;

        out_union->mech_type = 0;
        out_union->mech_name = 0;
        out_union->name_type = 0;
        out_union->external_name = 0;
        out_union->loopback = out_union;

        /* Allocate the buffer for the user specified representation */
        if (gssint_create_copy_buffer(in_union->external_name, &out_union->external_name, 1))
            goto allocation_failure;

        if (in_union->name_type != GSS_C_NULL_OID)
        {
            if ((major_status = generic_gss_copy_oid(minor_status, in_union->name_type, &out_union->name_type)))
                goto allocation_failure;
        }

    }

    /*
     * might need to delete any old mechanism names if we are
     * reusing the buffer.
     */
    if (!output_name)
    {
        if (in_union->mech_type)
        {
            (void) gssint_release_internal_name(minor_status, in_union->mech_type, &in_union->mech_name);
            (void) gss_release_oid(minor_status, &in_union->mech_type);
            in_union->mech_type = 0;
        }
        dest_union = in_union;
    }
    else
        dest_union = out_union;

    /* now let's create the new mech name */
    if ((major_status = generic_gss_copy_oid(minor_status, mech_type, &dest_union->mech_type)))
        goto allocation_failure;

    if ((major_status = gssint_import_internal_name(minor_status, mech_type, dest_union, &dest_union->mech_name)))
        goto allocation_failure;

    if (output_name)
        *output_name = (gss_name_t)dest_union;

    return (GSS_S_COMPLETE);

allocation_failure:
    /* do not delete the src name external name format */
    if (output_name)
    {
        if (out_union->external_name)
        {
            if (out_union->external_name->value)
                free(out_union->external_name->value);
            free(out_union->external_name);
        }
        if (out_union->name_type)
            (void) gss_release_oid(minor_status, &out_union->name_type);

        dest_union = out_union;
    }
    else
        dest_union = in_union;

    /*
     * delete the partially created mech specific name
     * applies for both src and dest which ever is being used for output
     */

    if (dest_union->mech_name)
    {
        (void) gssint_release_internal_name(minor_status, dest_union->mech_type, &dest_union->mech_name);
    }

    if (dest_union->mech_type)
        (void) gss_release_oid(minor_status, &dest_union->mech_type);


    if (output_name)
        free(out_union);

    return (major_status);
}

OM_uint32 gss_inquire_cred(OM_uint32 *minor_status, gss_cred_id_t server_creds, gss_name_t *name, OM_uint32 *lifetime,
                           int *cred_usage, gss_OID_set *mechanisms)


{
    OM_uint32		status, /*elapsed_time,*/ temp_minor_status;
    gss_union_cred_t	union_cred;
    //gss_mechanism	mech;
    //gss_name_t		internal_name;
    int			i;

    /* Initialize outputs. */

    if (minor_status != NULL)
        *minor_status = 0;

    if (name != NULL)
        *name = GSS_C_NO_NAME;

    if (mechanisms != NULL)
        *mechanisms = GSS_C_NO_OID_SET;

    /* Validate arguments. */
    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    /* get the cred_handle cast as a union_credentials structure */

    union_cred = (gss_union_cred_t) server_creds;

    /*
     * get the information out of the union_cred structure that was
     * placed there during gss_acquire_cred.
     */

    if (cred_usage != NULL)
        *cred_usage = union_cred->auxinfo.cred_usage;

    if (lifetime != NULL)
    {
        //elapsed_time = time(0) - union_cred->auxinfo.creation_time;
        //*lifetime = union_cred->auxinfo.time_rec < elapsed_time ? 0 :
        //union_cred->auxinfo.time_rec - elapsed_time;
    }

    /*
     * if name is non_null,
     * call gss_import_name(), giving it the printable name held within
     * union_cred in order to get an internal name to pass back to the
     * caller. If this call fails, return failure to our caller.
     */

    if (name != NULL)
    {
        //if ((gss_import_name(&temp_minor_status,
        //			 &union_cred->auxinfo.name,
        //			 union_cred->auxinfo.name_type,
        //			 name) != GSS_S_COMPLETE) ||
        //	(gss_canonicalize_name(minor_status, *name,
        //			   &union_cred->mechs_array[0],
        //			   NULL) != GSS_S_COMPLETE)) {
        //	status = GSS_S_DEFECTIVE_CREDENTIAL;
        //	goto error;
        //}
    }

    /*
     * copy the mechanism set in union_cred into an OID set and return in
     * the mechanisms parameter.
     */

    if (mechanisms != NULL)
    {
        status = GSS_S_FAILURE;
        *mechanisms = (gss_OID_set) malloc(sizeof(gss_OID_set_desc));
        if (*mechanisms == NULL)
            goto error;

        (*mechanisms)->count = union_cred->count;
        (*mechanisms)->elements = union_cred->mechs_array;
        //(*mechanisms)->elements =
        //	(gss_OID) malloc(sizeof(gss_OID_desc) *
        //			 union_cred->count);

        //if ((*mechanisms)->elements == NULL)
        //{
        //	free(*mechanisms);
        //	*mechanisms = NULL;
        //	goto error;
        //}

        for (i=0; i < union_cred->count; i++)
        {
            //(*mechanisms)->elements[i].elements = (void *)malloc(union_cred->mechs_array[i].length);
            //if ((*mechanisms)->elements[i].elements == NULL)
            //	goto error;
            //g_OID_copy(&(*mechanisms)->elements[i], &union_cred->mechs_array[i]);
            //(*mechanisms)->count++;

        }
    }

    return(GSS_S_COMPLETE);

error:
    /*
     * cleanup any allocated memory - we can just call
     * gss_release_oid_set, because the set is constructed so that
     * count always references the currently copied number of
     * elements.
     */
//   if (mechanisms && *mechanisms != NULL)
    //(void) gss_release_oid_set(&temp_minor_status, mechanisms);

    if (name && *name != NULL)
        (void) gss_release_name(&temp_minor_status, name);

    return (status);
}




OM_uint32 gss_release_cred(OM_uint32 *minor_status, gss_cred_id_t *cred_handle)
{
    OM_uint32		status, temp_status;
    int			j;
    gss_union_cred_t	union_cred;
    gss_mechanism	mech;

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    *minor_status = 0;

    if (cred_handle == NULL)
        return (GSS_S_NO_CRED | GSS_S_CALL_INACCESSIBLE_READ);

    /*
     * Loop through the union_cred struct, selecting the approprate
     * underlying mechanism routine and calling it. At the end,
     * release all of the storage taken by the union_cred struct.
     */

    union_cred = (gss_union_cred_t) *cred_handle;
    if (GSSINT_CHK_LOOP(union_cred))
        return (GSS_S_NO_CRED | GSS_S_CALL_INACCESSIBLE_READ);
    *cred_handle = NULL;

    if (union_cred == (gss_union_cred_t)GSS_C_NO_CREDENTIAL)
        return (GSS_S_COMPLETE);

    status = GSS_S_COMPLETE;

    for (j=0; j < union_cred->count; j++)
    {

        mech = gssint_get_mechanism (&union_cred->mechs_array[j]);

        if (union_cred->mechs_array[j].elements)
            free(union_cred->mechs_array[j].elements);
        if (mech)
        {
            //if (mech->gss_release_cred) {
            temp_status = krb5_gss_release_cred
                          (//mech->context,
                              minor_status,
                              &union_cred->cred_array[j]);

            if (temp_status != GSS_S_COMPLETE)
                status = GSS_S_NO_CRED;

            //} else
            //status = GSS_S_UNAVAILABLE;
        }
        else
            status = GSS_S_DEFECTIVE_CREDENTIAL;
    }

    //gss_release_buffer(minor_status, &union_cred->auxinfo.name);
    free(union_cred->cred_array);
    free(union_cred->mechs_array);
    free(union_cred);

    return(status);
}

OM_uint32 generic_gss_test_oid_set_member(OM_uint32 *minor_status, const gss_OID_desc * const member, gss_OID_set set, int *present)
{
    OM_uint32	i;
    int		result;

    *minor_status = 0;

    if (member == NULL || set == NULL)
        return (GSS_S_CALL_INACCESSIBLE_READ);

    if (present == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    result = 0;
    for (i=0; i<set->count; i++)
    {
        if ((set->elements[i].length == member->length) &&
                !memcmp(set->elements[i].elements, member->elements, (size_t) member->length))
        {
            result = 1;
            break;
        }
    }
    *present = result;
    return(GSS_S_COMPLETE);
}

OM_uint32 gss_test_oid_set_member(OM_uint32 *minor_status, gss_OID member, gss_OID_set set, int *present)
{
    return generic_gss_test_oid_set_member(minor_status, member, set, present);
}

OM_uint32 gss_release_name(OM_uint32 *minor_status, gss_name_t *input_name)
{
    gss_union_name_t	union_name;

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);
    *minor_status = 0;

    /* if input_name is NULL, return error */
    if (input_name == 0)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_BAD_NAME);

    if (*input_name == GSS_C_NO_NAME)
        return GSS_S_COMPLETE;

    /*
     * free up the space for the external_name and then
     * free the union_name descriptor
     */

    union_name = (gss_union_name_t) *input_name;
    if (GSSINT_CHK_LOOP(union_name))
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_BAD_NAME);
    *input_name = 0;
    *minor_status = 0;

    if (union_name->name_type)
        gss_release_oid(minor_status, &union_name->name_type);

    //free(union_name->external_name->value);
    free(union_name->external_name);

    if (union_name->mech_type)
    {
        //gssint_release_internal_name(minor_status, union_name->mech_type,
        //			&union_name->mech_name);
        gss_release_oid(minor_status, &union_name->mech_type);
    }

    free(union_name);

    return(GSS_S_COMPLETE);
}

OM_uint32 gss_add_oid_set_member(OM_uint32 *minor_status, gss_OID member_oid, gss_OID_set *oid_set)
{
    return generic_gss_add_oid_set_member(minor_status, member_oid, oid_set);
}

static OM_uint32 val_seal_args(OM_uint32 *minor_status, gss_ctx_id_t context_handle, int conf_req_flag, int qop_req,
                               gss_buffer_t input_message_buffer, int *conf_state, gss_buffer_t output_message_buffer)
{

    /* Initialize outputs. */

    if (minor_status != NULL)
        *minor_status = 0;

    if (output_message_buffer != GSS_C_NO_BUFFER)
    {
        output_message_buffer->length = 0;
        output_message_buffer->value = NULL;
    }

    /* Validate arguments. */

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (context_handle == GSS_C_NO_CONTEXT)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);

    if (input_message_buffer == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_READ);

    if (output_message_buffer == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    return (GSS_S_COMPLETE);
}


OM_uint32 gss_seal(OM_uint32 *minor_status, gss_ctx_id_t context_handle, int conf_req_flag, int qop_req,
                   gss_buffer_t input_message_buffer, int *conf_state, gss_buffer_t output_message_buffer)

{
    /* EXPORT DELETE START */

    OM_uint32		status;
    gss_union_ctx_id_t	ctx;
    gss_mechanism	mech;

    status = val_seal_args(minor_status, context_handle, conf_req_flag, qop_req, input_message_buffer, conf_state,
                           output_message_buffer);
    if (status != GSS_S_COMPLETE)
        return (status);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) context_handle;
    mech = gssint_get_mechanism (ctx->mech_type);

    if (mech) {
        if (mech->gss_seal)
            status = mech->gss_seal(mech->context, minor_status, ctx->internal_ctx_id, conf_req_flag,
                                    qop_req, input_message_buffer, conf_state, output_message_buffer);
        else
            status = GSS_S_UNAVAILABLE;

        return(status);
    }
    /* EXPORT DELETE END */

    return (GSS_S_BAD_MECH);
}

#if 0
OM_uint32 gss_wrap (
    OM_uint32 *		minor_status,
    gss_ctx_id_t		context_handle,
    int			conf_req_flag,
    gss_qop_t		qop_req,
    gss_buffer_t		input_message_buffer,
    int *			conf_state,
    gss_buffer_t		output_message_buffer)

{
    return gss_seal(minor_status, (gss_ctx_id_t)context_handle,
                    conf_req_flag, (int) qop_req,
                    (gss_buffer_t)input_message_buffer, conf_state,
                    output_message_buffer);
}

/*
 * New for V2
 */
OM_uint32
gss_wrap_size_limit(
    OM_uint32		*minor_status,
    gss_ctx_id_t	context_handle,
    int			conf_req_flag,
    gss_qop_t		qop_req,
    OM_uint32		req_output_size,
    OM_uint32		*max_input_size)
{
    gss_union_ctx_id_t	ctx;
    gss_mechanism	mech;

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);
    *minor_status = 0;

    if (context_handle == GSS_C_NO_CONTEXT)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);

    if (max_input_size == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) context_handle;
    mech = gssint_get_mechanism (ctx->mech_type);

    if (!mech)
        return (GSS_S_BAD_MECH);

    if (!mech->gss_wrap_size_limit)
        return (GSS_S_UNAVAILABLE);

    return (mech->gss_wrap_size_limit(//mech->context,
                minor_status,
                ctx->internal_ctx_id, conf_req_flag, qop_req,
                req_output_size, max_input_size));
}
#endif

OM_uint32 gss_process_context_token (OM_uint32 *minor_status, gss_ctx_id_t context_handle, gss_buffer_t token_buffer)

{
    OM_uint32		status;
    gss_union_ctx_id_t	ctx;
    gss_mechanism	mech;

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);
    *minor_status = 0;

    if (context_handle == GSS_C_NO_CONTEXT)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);

    if (token_buffer == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_READ);

    if (GSS_EMPTY_BUFFER(token_buffer))
        return (GSS_S_CALL_INACCESSIBLE_READ);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) context_handle;
    mech = gssint_get_mechanism (ctx->mech_type);

    if (mech)
    {

        if (mech->gss_process_context_token)
            status = mech->gss_process_context_token(mech->context, minor_status, ctx->internal_ctx_id, token_buffer);
        else
            status = GSS_S_UNAVAILABLE;

        return(status);
    }

    return (GSS_S_BAD_MECH);
}


OM_uint32 gss_context_time(OM_uint32 *minor_status,
                           gss_ctx_id_t context_handle,
                           OM_uint32 *	time_rec)
{
    OM_uint32		status;
    gss_union_ctx_id_t	ctx;
    gss_mechanism	mech;

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);
    *minor_status = 0;

    if (time_rec == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (context_handle == GSS_C_NO_CONTEXT)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) context_handle;
    mech = gssint_get_mechanism (ctx->mech_type);

    if (mech) {

        if (mech->gss_context_time)
            status = mech->gss_context_time(
                         mech->context,
                         minor_status,
                         ctx->internal_ctx_id,
                         time_rec);
        else
            status = GSS_S_UNAVAILABLE;

        return(status);
    }

    return (GSS_S_BAD_MECH);
}

static OM_uint32 val_exp_sec_ctx_args(
    OM_uint32 *minor_status,
    gss_ctx_id_t *context_handle,
    gss_buffer_t interprocess_token)
{

    /* Initialize outputs. */

    if (minor_status != NULL)
        *minor_status = 0;

    if (interprocess_token != GSS_C_NO_BUFFER) {
        interprocess_token->length = 0;
        interprocess_token->value = NULL;
    }

    /* Validate arguments. */

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (context_handle == NULL || *context_handle == GSS_C_NO_CONTEXT)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);

    if (interprocess_token == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    return (GSS_S_COMPLETE);
}


OM_uint32 gss_export_sec_context(
    OM_uint32 * minor_status,
    gss_ctx_id_t * context_handle,
    gss_buffer_t interprocess_token)
{
    OM_uint32		status;
    OM_uint32 		length;
    gss_union_ctx_id_t	ctx;
    gss_mechanism	mech;
    gss_buffer_desc	token;
    char		*buf;

    status = val_exp_sec_ctx_args(minor_status,
                                  context_handle, interprocess_token);
    if (status != GSS_S_COMPLETE)
        return (status);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) *context_handle;
    mech = gssint_get_mechanism (ctx->mech_type);
    if (!mech)
        return GSS_S_BAD_MECH;
    if (!mech->gss_export_sec_context)
        return (GSS_S_UNAVAILABLE);

    status = mech->gss_export_sec_context(//mech->context,
                 minor_status,
                 &ctx->internal_ctx_id, &token);
    if (status != GSS_S_COMPLETE)
        return (status);

    length = token.length + 4 + ctx->mech_type->length;
    interprocess_token->length = length;
    interprocess_token->value = malloc(length);
    if (interprocess_token->value == 0) {
        (void) gss_release_buffer(minor_status, &token);
        return (GSS_S_FAILURE);
    }
    buf = interprocess_token->value;
    length = ctx->mech_type->length;
    buf[3] = (unsigned char) (length & 0xFF);
    length >>= 8;
    buf[2] = (unsigned char) (length & 0xFF);
    length >>= 8;
    buf[1] = (unsigned char) (length & 0xFF);
    length >>= 8;
    buf[0] = (unsigned char) (length & 0xFF);
    memcpy(buf+4, ctx->mech_type->elements, (size_t) ctx->mech_type->length);
    memcpy(buf+4+ctx->mech_type->length, token.value, token.length);

    (void) gss_release_buffer(minor_status, &token);

    free(ctx->mech_type->elements);
    free(ctx->mech_type);
    free(ctx);
    *context_handle = 0;

    return(GSS_S_COMPLETE);
}

static OM_uint32 val_imp_sec_ctx_args(
    OM_uint32 *minor_status,
    gss_buffer_t interprocess_token,
    gss_ctx_id_t *context_handle)
{

    /* Initialize outputs. */
    if (minor_status != NULL)
        *minor_status = 0;

    if (context_handle != NULL)
        *context_handle = GSS_C_NO_CONTEXT;

    /* Validate arguments. */

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (context_handle == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (interprocess_token == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_DEFECTIVE_TOKEN);

    if (GSS_EMPTY_BUFFER(interprocess_token))
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_DEFECTIVE_TOKEN);

    return (GSS_S_COMPLETE);
}


OM_uint32 gss_import_sec_context(
    OM_uint32 *	minor_status,
    gss_buffer_t interprocess_token,
    gss_ctx_id_t * context_handle
)
{
    OM_uint32		length = 0;
    OM_uint32		status;
    char		*p;
    gss_union_ctx_id_t	ctx;
    gss_buffer_desc	token;
    gss_mechanism	mech;

    status = val_imp_sec_ctx_args(minor_status,
                                  interprocess_token, context_handle);
    if (status != GSS_S_COMPLETE)
        return (status);

    /* Initial value needed below. */
    status = GSS_S_FAILURE;

    ctx = (gss_union_ctx_id_t) malloc(sizeof(gss_union_ctx_id_desc));
    if (!ctx)
        return (GSS_S_FAILURE);

    ctx->mech_type = (gss_OID) malloc(sizeof(gss_OID_desc));
    if (!ctx->mech_type) {
        free(ctx);
        return (GSS_S_FAILURE);
    }

    if (interprocess_token->length >= sizeof (OM_uint32)) {
        p = interprocess_token->value;
        length = (OM_uint32)*p++;
        length = (OM_uint32)(length << 8) + *p++;
        length = (OM_uint32)(length << 8) + *p++;
        length = (OM_uint32)(length << 8) + *p++;
    }

    if (length == 0 ||
            length > (interprocess_token->length - sizeof (OM_uint32))) {
        free(ctx);
        return (GSS_S_CALL_BAD_STRUCTURE | GSS_S_DEFECTIVE_TOKEN);
    }

    ctx->mech_type->length = length;
    ctx->mech_type->elements = malloc(length);
    if (!ctx->mech_type->elements) {
        goto error_out;
    }
    memcpy(ctx->mech_type->elements, p, length);
    p += length;

    token.length = interprocess_token->length - sizeof (OM_uint32) - length;
    token.value = p;

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    mech = gssint_get_mechanism (ctx->mech_type);
    if (!mech) {
        status = GSS_S_BAD_MECH;
        goto error_out;
    }
    if (!mech->gss_import_sec_context) {
        status = GSS_S_UNAVAILABLE;
        goto error_out;
    }

    status = mech->gss_import_sec_context(//mech->context,
                 minor_status,
                 &token, &ctx->internal_ctx_id);

    if (status == GSS_S_COMPLETE) {
        ctx->loopback = ctx;
        *context_handle = ctx;
        return (GSS_S_COMPLETE);
    }

error_out:
    if (ctx) {
        if (ctx->mech_type) {
            if (ctx->mech_type->elements)
                free(ctx->mech_type->elements);
            free(ctx->mech_type);
        }
        free(ctx);
    }
    return status;
}

static OM_uint32 val_inq_ctx_args(
    OM_uint32 *minor_status,
    gss_ctx_id_t context_handle,
    gss_name_t *src_name,
    gss_name_t *targ_name,
    OM_uint32 *lifetime_rec,
    gss_OID *mech_type,
    OM_uint32 *ctx_flags,
    int *locally_initiated,
    int *open)
{

    /* Initialize outputs. */

    if (minor_status != NULL)
        *minor_status = 0;

    if (src_name != NULL)
        *src_name = GSS_C_NO_NAME;

    if (targ_name != NULL)
        *targ_name = GSS_C_NO_NAME;

    if (mech_type != NULL)
        *mech_type = GSS_C_NO_OID;

    /* Validate arguments. */

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (context_handle == GSS_C_NO_CONTEXT)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);

    return (GSS_S_COMPLETE);
}


/* Last argument new for V2 */
OM_uint32 gss_inquire_context(
    OM_uint32 *	minor_status,
    gss_ctx_id_t	context_handle,
    gss_name_t *	src_name,
    gss_name_t *	targ_name,
    OM_uint32 *	lifetime_rec,
    gss_OID *	mech_type,
    OM_uint32 *	ctx_flags,
    int *           locally_initiated,
    int *		open)
{
    gss_union_ctx_id_t	ctx;
    gss_mechanism	mech;
    OM_uint32		status, temp_minor;
    gss_name_t localTargName = NULL, localSourceName = NULL;

    status = val_inq_ctx_args(minor_status,
                              context_handle,
                              src_name, targ_name,
                              lifetime_rec,
                              mech_type, ctx_flags,
                              locally_initiated, open);
    if (status != GSS_S_COMPLETE)
        return (status);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) context_handle;
    mech = gssint_get_mechanism (ctx->mech_type);

    if (!mech || !mech->gss_inquire_context || !mech->gss_display_name ||
            !mech->gss_release_name) {
        return (GSS_S_UNAVAILABLE);
    }

    status = mech->gss_inquire_context(
                 //mech->context,
                 minor_status,
                 ctx->internal_ctx_id,
                 (src_name ? &localSourceName : NULL),
                 (targ_name ? &localTargName : NULL),
                 lifetime_rec,
                 NULL,
                 ctx_flags,
                 locally_initiated,
                 open);

    if (status != GSS_S_COMPLETE) {
        return status;
    }

    /* need to convert names */

    if (src_name) {
        status = gssint_convert_name_to_union_name(minor_status, mech,
                 localSourceName, src_name);

        if (status != GSS_S_COMPLETE) {
            if (localTargName)
                mech->gss_release_name(mech->context,
                                       &temp_minor, &localTargName);
            return (status);
        }

    }

    if (targ_name) {
        status = gssint_convert_name_to_union_name(minor_status, mech,
                 localTargName, targ_name);

        if (status != GSS_S_COMPLETE) {
            if (src_name)
                (void) gss_release_name(&temp_minor, src_name);

            return (status);
        }
    }

    /* spec says mech type must point to static storage */
    if (mech_type)
        *mech_type = &mech->mech_type;
    return(GSS_S_COMPLETE);
}

OM_uint32 generic_gss_create_empty_oid_set(OM_uint32 *minor_status, gss_OID_set	*oid_set)
{
    *minor_status = 0;

    if ((*oid_set = (gss_OID_set) malloc(sizeof(gss_OID_set_desc))))
    {
        memset(*oid_set, 0, sizeof(gss_OID_set_desc));
        return(GSS_S_COMPLETE);
    }
    else
    {
        *minor_status = -1;
        return(GSS_S_FAILURE);
    }
}

OM_uint32 gss_create_empty_oid_set(OM_uint32 *minor_status, gss_OID_set	*oid_set)
{
    return generic_gss_create_empty_oid_set(minor_status, oid_set);
}

int gssint_get_der_length(unsigned char **buf, unsigned int buf_len, unsigned int *bytes)
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
    if (*p < 128) {
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
    for (length = 0; octets; octets--) {
        new_length = (length << 8) + *p++;
        if (new_length < length)  /* overflow */
            return (-1);
        length = new_length;
    }

    *buf = p; /* Advance the buffer */

    return (length);
}


/*
 * put_der_length: Encode the supplied length into the buffer pointed to
 * by buf. max_length represents the maximum length of the buffer pointed
 * to by buff. We will advance buf to point to the character after the newly
 * DER encoded length. We return 0 on success or -l it the length cannot
 * be encoded in max_len characters.
 */
int gssint_put_der_length(unsigned int length, unsigned char **buf, unsigned int max_len)
{
    unsigned char *s = *buf, *p;
    unsigned int buf_len = 0;
    int i, first;

    /* Oops */
    if (buf == 0 || max_len < 1)
        return (-1);

    /* Single byte is the length */
    if (length < 128) {
        *s++ = length;
        *buf = s;
        return (0);
    }

    /* First byte contains the number of octets */
    p = s + 1;

    /* Running total of the DER encoding length */
    buf_len = 0;

    /*
     * Encode MSB first. We do the encoding by setting a shift
     * factor to MSO_BIT (24 for 32 bit words) and then shifting the length
     * by the factor. We then encode the resulting low order byte.
     * We subtract 8 from the shift factor and repeat to ecnode the next
     * byte. We stop when the shift factor is zero or we've run out of
     * buffer to encode into.
     */
    first = 0;
    for (i = MSO_BIT; i >= 0 && buf_len <= max_len; i -= 8) {
        unsigned int v;
        v = (length >> i) & 0xff;
        if ((v) || first) {
            buf_len += 1;
            *p++ = v;
            first = 1;
        }
    }
    if (i >= 0)			/* buffer overflow */
        return (-1);

    /*
     * We go back now and set the first byte to be the length with
     * the high order bit set.
     */
    *s = buf_len | 0x80;
    *buf = p;

    return (0);
}

/*
 * der_length_size: Return the number of bytes to encode a given length.
 */
unsigned int gssint_der_length_size(unsigned int len)
{
    int i;

    if (len < 128)
        return (1);

    for (i = 0; len; i++) {
        len >>= 8;
    }

    return (i+1);
}

static OM_uint32 val_sign_args(
    OM_uint32 *minor_status,
    gss_ctx_id_t context_handle,
    int qop_req,
    gss_buffer_t message_buffer,
    gss_buffer_t msg_token)
{

    /* Initialize outputs. */

    if (minor_status != NULL)
        *minor_status = 0;

    if (msg_token != GSS_C_NO_BUFFER) {
        msg_token->value = NULL;
        msg_token->length = 0;
    }

    /* Validate arguments. */

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (context_handle == GSS_C_NO_CONTEXT)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);

    if (message_buffer == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_READ);

    if (msg_token == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    return (GSS_S_COMPLETE);
}



/*
 * This function converts an internal gssapi name to a union gssapi
 * name.  Note that internal_name should be considered "consumed" by
 * this call, whether or not we return an error.
 */
OM_uint32 gssint_convert_name_to_union_name(
    OM_uint32 *minor_status,
    gss_mechanism	mech,
    gss_name_t	internal_name,
    gss_name_t	*external_name)

{
    OM_uint32 major_status = 1,tmp;
    gss_union_name_t union_name;

    union_name = (gss_union_name_t) malloc (sizeof(gss_union_name_desc));
    if (!union_name) {
        goto allocation_failure;
    }
    union_name->mech_type = 0;
    union_name->mech_name = internal_name;
    union_name->name_type = 0;
    union_name->external_name = 0;

    major_status = generic_gss_copy_oid(minor_status, &mech->mech_type,
                                        &union_name->mech_type);
    if (major_status != GSS_S_COMPLETE)
        goto allocation_failure;

    union_name->external_name =
        (gss_buffer_t) malloc(sizeof(gss_buffer_desc));
    if (!union_name->external_name) {
        goto allocation_failure;
    }

    //  major_status = mech->gss_display_name(//mech->context,
    //minor_status,
    //			  internal_name,
    //			  union_name->external_name,
    //			  &union_name->name_type);
    if (major_status != GSS_S_COMPLETE)
        goto allocation_failure;

    union_name->loopback = union_name;
    *external_name = /*(gss_name_t) CHECK */union_name;
    return (GSS_S_COMPLETE);

allocation_failure:
    if (union_name) {
        if (union_name->external_name) {
            if (union_name->external_name->value)
                free(union_name->external_name->value);
            free(union_name->external_name);
        }
        if (union_name->name_type)
            gss_release_oid(&tmp, &union_name->name_type);
        if (union_name->mech_type)
            gss_release_oid(&tmp, &union_name->mech_type);
        free(union_name);
    }
    /*
     * do as the top comment says - since we are now owners of
     * internal_name, we must clean it up
     */
//   if (internal_name)
    //(void) gssint_release_internal_name(&tmp, &mech->mech_type,
    //				   &internal_name);
    return (major_status);
}


OM_uint32 rtk_gss_release_oid_set (
    OM_uint32 *		minor_status,
    gss_OID_set *		set)
{
    OM_uint32 index;
    gss_OID oid;
    if (minor_status)
        *minor_status = 0;

    if (set ==NULL)
        return GSS_S_COMPLETE;

    if (*set == GSS_C_NULL_OID_SET)
        return(GSS_S_COMPLETE);

    for (index=0; index<(*set)->count; index++) {
        oid = &(*set)->elements[index];
        free(oid->elements);
    }
    free((*set)->elements);
    free(*set);

    *set = GSS_C_NULL_OID_SET;

    return(GSS_S_COMPLETE);
}

OM_uint32 generic_gss_add_oid_set_member(
    OM_uint32 *minor_status, const gss_OID_desc * const member_oid, gss_OID_set *oid_set)
{
    gss_OID	elist;
    gss_OID	lastel;

    *minor_status = 0;

    if (member_oid == NULL || member_oid->length == 0 ||
            member_oid->elements == NULL)
        return (GSS_S_CALL_INACCESSIBLE_READ);

    elist = (*oid_set)->elements;
    /* Get an enlarged copy of the array */
    if (((*oid_set)->elements = (gss_OID) malloc(((*oid_set)->count+1) *
                                sizeof(gss_OID_desc)))) {
        /* Copy in the old junk */
        if (elist)
            memcpy((*oid_set)->elements,
                   elist,
                   ((*oid_set)->count * sizeof(gss_OID_desc)));

        /* Duplicate the input element */
        lastel = &(*oid_set)->elements[(*oid_set)->count];
        if ((lastel->elements =
                    (void *) malloc((size_t) member_oid->length))) {
            /* Success - copy elements */
            memcpy(lastel->elements, member_oid->elements,
                   (size_t) member_oid->length);
            /* Set length */
            lastel->length = member_oid->length;

            /* Update count */
            (*oid_set)->count++;
            if (elist)
                free(elist);
            *minor_status = 0;
            return(GSS_S_COMPLETE);
        }
        else
            free((*oid_set)->elements);
    }
    /* Failure - restore old contents of list */
    (*oid_set)->elements = elist;
    *minor_status = -1;
    return(GSS_S_FAILURE);
}

OM_uint32 krb5_gss_internal_release_oid(OM_uint32	*minor_status, gss_OID	*oid)
{
    /*
     * This function only knows how to release internal OIDs. It will
     * return GSS_S_CONTINUE_NEEDED for any OIDs it does not recognize.
     */

    if ((*oid != gss_mech_krb5) &&
            (*oid != gss_mech_krb5_old) &&
            (*oid != gss_mech_krb5_wrong) &&
            (*oid != gss_nt_krb5_name) &&
            (*oid != gss_nt_krb5_principal)) {
        /* We don't know about this OID */
        return(GSS_S_CONTINUE_NEEDED);
    }
    else {
        *oid = GSS_C_NO_OID;
        *minor_status = 0;
        return(GSS_S_COMPLETE);
    }
}

/*
 * function used to reclaim the memory used by a gss_OID structure.
 * This routine requires direct access to the mechList.
 */
OM_uint32 gss_release_oid(OM_uint32 *minor_status, gss_OID *oid)
{
    OM_uint32 major;
    gss_mech_info aMech;

    //if (gssint_initialize_library())
    //	return GSS_S_FAILURE;

    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);

    *minor_status = 0;

    //k5_mutex_lock(&g_mechListLock);
    aMech = g_mechList;
    while (aMech != NULL) {

        /*
         * look through the loaded mechanism libraries for
         * gss_internal_release_oid until one returns success.
         * gss_internal_release_oid will only return success when
         * the OID was recognized as an internal mechanism OID. if no
         * mechanisms recognize the OID, then call the generic version.
         */
        //if (aMech->mech && aMech->mech->gss_internal_release_oid) {
        //major = aMech->mech->gss_internal_release_oid(
        major = krb5_gss_internal_release_oid(
                    //aMech->mech->context,
                    minor_status, oid);
        if (major == GSS_S_COMPLETE) {
            //	k5_mutex_unlock(&g_mechListLock);
            return (GSS_S_COMPLETE);
        }
        //}
        aMech = aMech->next;
    } /* while */
    //k5_mutex_unlock(&g_mechListLock);

    return (generic_gss_release_oid(minor_status, oid, 1));
} /* gss_release_oid */

OM_uint32 generic_gss_release_oid(OM_uint32	* minor_status, gss_OID	*oid, int releaseElement)
{
    if (minor_status)
        *minor_status = 0;

    if (oid == NULL || *oid == GSS_C_NO_OID)
        return(GSS_S_COMPLETE);

    /*
     * The V2 API says the following!
     *
     * gss_release_oid[()] will recognize any of the GSSAPI's own OID values,
     * and will silently ignore attempts to free these OIDs; for other OIDs
     * it will call the C free() routine for both the OID data and the
     * descriptor.  This allows applications to freely mix their own heap-
     * allocated OID values with OIDs returned by GSS-API.
     */

    /*
     * We use the official OID definitions instead of the unofficial OID
     * defintions. But we continue to support the unofficial OID
     * gss_nt_service_name just in case if some gss applications use
     * the old OID.
     */

    if ((*oid != GSS_C_NT_USER_NAME) &&
            (*oid != GSS_C_NT_MACHINE_UID_NAME) &&
            (*oid != GSS_C_NT_STRING_UID_NAME) &&
            (*oid != GSS_C_NT_HOSTBASED_SERVICE) &&
            (*oid != GSS_C_NT_ANONYMOUS) &&
            (*oid != GSS_C_NT_EXPORT_NAME) &&
            (*oid != gss_nt_service_name))
    {
        if (releaseElement == 1)
            free((*oid)->elements);
        free(*oid);
    }
    *oid = GSS_C_NO_OID;
    return(GSS_S_COMPLETE);
}

OM_uint32 gss_delete_sec_context (OM_uint32 * minor_status, gss_ctx_id_t * context_handle, gss_buffer_t output_token)
{
    OM_uint32		status;
    gss_union_ctx_id_t	ctx;
    gss_mechanism	mech;

    //status = val_del_sec_ctx_args(minor_status, context_handle, output_token);
    //if (status != GSS_S_COMPLETE)
    //return (status);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) *context_handle;
    //if(!((ctx) != NULL && (ctx)->loopback == (ctx)))
//   //if (GSSINT_CHK_LOOP(ctx))
    //	return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);
    if (ctx == NULL)
        return (GSS_S_NO_CONTEXT);
    mech = gssint_get_mechanism (ctx->mech_type);

    if (mech) {

        //if (mech->gss_delete_sec_context)
        status = krb5_gss_delete_sec_context(
                     //mech->context,
                     minor_status,
                     &ctx->internal_ctx_id,
                     output_token);
        //else
        //    status = GSS_S_UNAVAILABLE;

        /* now free up the space for the union context structure */
        free(ctx->mech_type->elements);
        free(ctx->mech_type);
        free(*context_handle);
        *context_handle = NULL;

        return(status);
    }

    return (GSS_S_BAD_MECH);

}

OM_uint32 generic_gss_copy_oid(OM_uint32 *minor_status, const gss_OID_desc * const oid, gss_OID *new_oid)
{
    gss_OID		p;

    *minor_status = 0;

    p = (gss_OID) malloc(sizeof(gss_OID_desc));
    if (!p) {
        *minor_status = -1;
        return GSS_S_FAILURE;
    }
    p->length = oid->length;
    p->elements = (void*)malloc(p->length);
    if (!p->elements) {
        free(p);
        return GSS_S_FAILURE;
    }
    memcpy(p->elements, oid->elements, p->length);
    *new_oid = p;
    return(GSS_S_COMPLETE);
}

static OM_uint32 val_imp_name_args(OM_uint32 *minor_status, gss_buffer_t input_name_buffer,
                                   gss_OID input_name_type, gss_name_t *output_name)
{
    if (minor_status != NULL)
        *minor_status = 0;
    if (output_name != NULL)
        *output_name = GSS_C_NO_NAME;
    if (minor_status == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);
    if (output_name == NULL)
        return (GSS_S_CALL_INACCESSIBLE_WRITE);
    if (input_name_buffer == GSS_C_NO_BUFFER)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_BAD_NAME);
    if (input_name_buffer->length == 0)
        return GSS_S_BAD_NAME;
    if (input_name_buffer->value == NULL)
        return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_BAD_NAME);
    return (GSS_S_COMPLETE);
}
OM_uint32 gss_import_name(OM_uint32 * minor_status, gss_buffer_t input_name_buffer,
                          gss_OID input_name_type, gss_name_t *output_name)
{
    gss_union_name_t	union_name;
    OM_uint32		tmp, major_status = GSS_S_FAILURE;
    major_status = val_imp_name_args(minor_status,
                                     input_name_buffer, input_name_type,
                                     output_name);
    if (major_status != GSS_S_COMPLETE)
        return (major_status);
    union_name = (gss_union_name_t) malloc (sizeof(gss_union_name_desc));
    if (!union_name)
        return (GSS_S_FAILURE);
    union_name->loopback = 0;
    union_name->mech_type = 0;
    union_name->mech_name = 0;
    union_name->name_type = 0;
    union_name->external_name = 0;
    major_status = gssint_create_copy_buffer(input_name_buffer,
                   &union_name->external_name, 0);
    if (major_status != GSS_S_COMPLETE) {
        free(union_name);
        return (major_status);
    }
    if (input_name_type != GSS_C_NULL_OID) {
        major_status = generic_gss_copy_oid(minor_status,
                                            input_name_type,
                                            &union_name->name_type);
        if (major_status != GSS_S_COMPLETE)
            goto allocation_failure;
    }
#if 0
    if (input_name_type != GSS_C_NULL_OID &&
            g_OID_equal(input_name_type, GSS_C_NT_EXPORT_NAME)) {
        major_status = importExportName(minor_status, union_name);
        if (major_status != GSS_S_COMPLETE)
            goto allocation_failure;
    }
#endif
    union_name->loopback = union_name;
    *output_name = (gss_name_t)union_name;
    return (GSS_S_COMPLETE);
allocation_failure:
    if (union_name) {
        if (union_name->external_name) {
            if (union_name->external_name->value)
                free(union_name->external_name->value);
            free(union_name->external_name);
        }
        if (union_name->name_type)
            generic_gss_release_oid(&tmp, &union_name->name_type, 1);
        if (union_name->mech_type)
            generic_gss_release_oid(&tmp, &union_name->mech_type, 1);
        free(union_name);
    }
    return (major_status);
}
int server_acquire_creds(char   *service_name, gss_cred_id_t *server_creds)
{
    gss_buffer_desc name_buf;
    gss_name_t server_name;
    OM_uint32 maj_stat, min_stat;


    name_buf.value = service_name;
    name_buf.length = strlen((char*)name_buf.value) + 1;
    maj_stat = gss_import_name(&min_stat, &name_buf,
                               (gss_OID) gss_nt_service_name, &server_name);
    if (maj_stat != GSS_S_COMPLETE) {
        //display_status("importing name", maj_stat, min_stat);
        return -1;
    }

    maj_stat=rtk_gss_acquire_cred(&min_stat, server_name/*host*/, 0,
                                  GSS_C_NULL_OID_SET, GSS_C_ACCEPT,
                                  server_creds, NULL, NULL);
    if (maj_stat != GSS_S_COMPLETE) {
        //display_status("acquiring credentials", maj_stat, min_stat);
        return -1;
    }

    (void) gss_release_name(&min_stat, &server_name);

    return 0;
}


const unsigned char gss_server_encrypted_by_aes[]=
{
    0x60,0x82,0x01,0xf9,0x06,0x09,0x2a,0x86,0x48,0x86
    ,0xf7,0x12,0x01,0x02,0x02,0x01,0x00,0x6e,0x82,0x01,0xe8,0x30,0x82,0x01,0xe4,0xa0
    ,0x03,0x02,0x01,0x05,0xa1,0x03,0x02,0x01,0x0e,0xa2,0x07,0x03,0x05,0x00,0x20,0x00
    ,0x00,0x00,0xa3,0x82,0x01,0x0b,0x61,0x82,0x01,0x07,0x30,0x82,0x01,0x03,0xa0,0x03
    ,0x02,0x01,0x05,0xa1,0x0e,0x1b,0x0c,0x53,0x43,0x43,0x4d,0x54,0x45,0x53,0x54,0x2e
    ,0x43,0x4f,0x4d,0xa2,0x24,0x30,0x22,0xa0,0x03,0x02,0x01,0x03,0xa1,0x1b,0x30,0x19
    ,0x1b,0x04,0x68,0x6f,0x73,0x74,0x1b,0x11,0x74,0x65,0x73,0x74,0x2e,0x73,0x63,0x63
    ,0x6d,0x74,0x65,0x73,0x74,0x2e,0x63,0x6f,0x6d,0xa3,0x81,0xc5,0x30,0x81,0xc2,0xa0
    ,0x03,0x02,0x01,0x11,0xa1,0x03,0x02,0x01,0x03,0xa2,0x81,0xb5,0x04,0x81,0xb2,0x00
    ,0xfb,0xa4,0x4e,0xf6,0x46,0x2b,0x44,0x1c,0xaf,0x3c,0xe6,0xce,0xce,0x7e,0x39,0x47
    ,0xb2,0xea,0xc7,0x83,0x6f,0x13,0x88,0x70,0x26,0xa4,0x76,0x99,0x98,0x90,0x71,0x2f
    ,0x2f,0x3c,0xa1,0x5a,0xfb,0x6c,0xf1,0x04,0x13,0x46,0xb0,0x9f,0x47,0x5b,0xce,0x70
    ,0xe5,0xff,0xe3,0xb1,0x3e,0x57,0x2f,0x80,0xf6,0x67,0xec,0x7d,0x43,0x36,0x81,0xc4
    ,0x6e,0x77,0x94,0x57,0x34,0x1b,0x79,0x67,0xfc,0x77,0xf1,0xd9,0x4f,0x70,0xb7,0x4b
    ,0xa2,0xb2,0x02,0x06,0xa7,0xaf,0x34,0x57,0xa1,0x2c,0xc1,0x6f,0x3b,0x54,0xa9,0x6d
    ,0xa9,0x20,0x83,0xa9,0x0c,0xb6,0x50,0x73,0x59,0xb2,0x74,0xc0,0x1b,0x42,0x0c,0x45
    ,0xbc,0x60,0xb2,0xe3,0x35,0x7f,0x1f,0xfe,0xd0,0x03,0xd7,0xcb,0x2b,0xe2,0xbf,0x18
    ,0x40,0x00,0xed,0xd8,0x5e,0x4e,0x47,0x14,0x17,0xff,0xf5,0xc1,0xed,0x76,0xb0,0x7c
    ,0xa3,0xe6,0x73,0x41,0xd9,0x51,0xd0,0xe5,0x12,0xef,0xf8,0xb8,0x6b,0x9c,0x17,0x04
    ,0x03,0xa7,0x80,0xd5,0xa1,0xe3,0x7a,0x3b,0x0f,0xff,0x1a,0xbf,0x94,0x23,0x37,0x56
    ,0x09,0xa4,0x81,0xbf,0x30,0x81,0xbc,0xa0,0x03,0x02,0x01,0x11,0xa2,0x81,0xb4,0x04
    ,0x81,0xb1,0x8d,0xaf,0x64,0x09,0xd4,0x3d,0x0c,0xaa,0x46,0x1e,0x65,0x48,0x41,0xbb
    ,0x3c,0xed,0x3b,0x9a,0xa1,0x7f,0xe6,0xcc,0xdb,0xd7,0x98,0x0e,0x07,0x27,0x41,0xfd
    ,0xfb,0xc3,0x56,0x73,0x45,0x02,0xa4,0x72,0x04,0xd6,0x1a,0x0a,0xe4,0x60,0x6f,0x1c
    ,0xcf,0x76,0x18,0x4c,0x62,0xb3,0x8e,0x60,0x45,0x3e,0xac,0x08,0x06,0xea,0xc3,0xb6
    ,0xfc,0x09,0xf9,0xc3,0x00,0xec,0x34,0xdc,0x5d,0xd3,0xa5,0xab,0xa6,0x49,0x50,0xd1
    ,0xf2,0x02,0x8b,0x85,0x9c,0x9a,0x14,0x11,0x72,0x80,0x4a,0x57,0x02,0x74,0x43,0xe0
    ,0xaa,0x5a,0x52,0x9d,0x3a,0x3a,0x20,0xf2,0xd8,0xb9,0xec,0xaa,0xa6,0x4e,0xca,0x4a
    ,0x4e,0x91,0x18,0x5a,0xe1,0x0e,0x7a,0xfc,0x1c,0x75,0x97,0x96,0x74,0xcc,0xdf,0x51
    ,0xe8,0x53,0x0e,0xbd,0xc5,0x70,0x13,0x3e,0xf2,0x21,0x86,0xfa,0x9b,0x86,0xcb,0xf5
    ,0x3d,0x96,0x63,0x02,0x86,0xd1,0x35,0xee,0x82,0xc6,0x59,0x75,0x75,0xea,0xba,0x23
    ,0x88,0xdf,0x39,0x60,0x75,0xcd,0x2b,0x92,0xd2,0x1f,0x4e,0x7c,0x6d,0x9f,0xab,0x3e
    ,0x09,0xc9,0xf9
};


static int server_establish_context(RTSkt *s, gss_cred_id_t server_creds, gss_ctx_id_t *context,
                                    gss_buffer_t client_name, OM_uint32 *ret_flags)
{
    gss_buffer_desc send_tok, recv_tok;
    gss_name_t client;
    gss_OID doid;
    OM_uint32 maj_stat, /*min_stat,*/ acc_sec_min_stat;
    //gss_buffer_desc oid_name;
    //int     token_flags;

    recv_tok.value = gss_server_encrypted_by_aes;
    recv_tok.length = sizeof(gss_server_encrypted_by_aes);

    maj_stat = gss_accept_sec_context(&acc_sec_min_stat, context, server_creds, &recv_tok, GSS_C_NO_CHANNEL_BINDINGS,
                                      &client, &doid, &send_tok, ret_flags, NULL,	/* ignore time_rec */ NULL);/* ignore del_cred_handle */
    return 0;
}

int sign_server(RTSkt *s, gss_cred_id_t server_creds, PKT* pkt)
{
    gss_buffer_desc client_name, xmit_buf, msg_buf;
    gss_ctx_id_t context = 0;
    OM_uint32 maj_stat, min_stat;
    int     /*i,*/ conf_state, ret_flags;
    //char   *cp;
    int     token_flags = 0;


    /* Establish a context with the client */
    if (server_establish_context(s, server_creds, &context,
                                 &client_name, (OM_uint32 *)&ret_flags) < 0)
        return (-1);

    if (context == GSS_C_NO_CONTEXT)
    {
        printf("Accepted unauthenticated connection.\n");
    }
    else
    {
        printf("Accepted connection: \"%.*s\"\n", (int) client_name.length, (char *) client_name.value);
        (void) gss_release_buffer(&min_stat, &client_name);
    }
    do
    {
        //if (token_flags & TOKEN_WRAPPED)
        if (1)
        {
            maj_stat = gss_unwrap(&min_stat, context, &xmit_buf, &msg_buf, &conf_state, (gss_qop_t *) NULL);
            if (maj_stat != GSS_S_COMPLETE)
            {
                if (xmit_buf.value)
                {
                    free(xmit_buf.value);
                    xmit_buf.value = 0;
                }
                return (-1);
            }
            else if (!conf_state && (token_flags & TOKEN_ENCRYPTED))
            {
                //fprintf(stderr, "Warning!  Message not encrypted.\n");
            }

            if (xmit_buf.value)
            {
                free(xmit_buf.value);
                xmit_buf.value = 0;
            }
        }
        else
        {
            msg_buf = xmit_buf;
        }

        if (token_flags & TOKEN_SEND_MIC)
        {
            /* Produce a signature block for the message */
            maj_stat = gss_get_mic(&min_stat, context, GSS_C_QOP_DEFAULT,
                                   &msg_buf, &xmit_buf);
            if (maj_stat != GSS_S_COMPLETE) {
                //display_status("signing message", maj_stat, min_stat);
                return (-1);
            }

            if (msg_buf.value) {
                free(msg_buf.value);
                msg_buf.value = 0;
            }

            /* Send the signature block to the client */
            //if (send_token(s, TOKEN_MIC, &xmit_buf) < 0)
            //	return (-1);

            if (xmit_buf.value) {
                free(xmit_buf.value);
                xmit_buf.value = 0;
            }
        }
        else
        {
            if (msg_buf.value)
            {
                free(msg_buf.value);
                msg_buf.value = 0;
            }
            //if (send_token(s, TOKEN_NOOP, empty_token) < 0)
            //return (-1);
        }
    } while (1 /* loop will break if NOOP received */ );

    if (context != GSS_C_NO_CONTEXT)
    {
        /* Delete context */
        maj_stat = rtk_spnego_gss_delete_sec_context(&min_stat, &context, NULL);
        if (maj_stat != GSS_S_COMPLETE)
        {
            //display_status("deleting context", maj_stat, min_stat);
            return (-1);
        }
    }
    return (0);

}

#if 0
static void worker_bee(void *param, PKT* pkt)
{
    struct _work_plan *work = (struct _work_plan*)param;
    sign_server(work->s, work->server_creds, pkt);
    free(work);
}
#endif

#ifdef TEST_GSSAPI
void test_gssapi(void *data)
{

    char *service_name = "host";
    gss_cred_id_t server_creds;
    struct _work_plan *work;
    //RTSkt* s = 0;
    //RTSkt *gssapics = 0;
    PKT pkt;
    //int status = 0;
    pkt.wp = NULL;
    pkt.len = 0;
    server_acquire_creds(service_name, &server_creds);

    work = (struct _work_plan*)malloc(sizeof(struct _work_plan));
    work->server_creds = server_creds;

    worker_bee((void*)work, &pkt);
    //}

    for (;;)
    {
        OSTimeDlyHMSM(1, 1, 1, 0);              // delay task for 1 second
    }
}
#endif
