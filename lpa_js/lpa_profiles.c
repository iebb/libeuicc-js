
#include "main.h"
#include "state_serializer.h"

cJSON * metadata_to_cjson(const struct es10c_profile_info_list *rptr) {
    cJSON* jprofile = cJSON_CreateObject();
    cJSON_AddStringOrNullToObject(jprofile, "iccid", rptr->iccid);
//    cJSON_AddStringOrNullToObject(jprofile, "isdpAid", rptr->isdpAid);
    cJSON_AddNumberToObject(jprofile, "profileState", rptr->profileState);
    cJSON_AddStringOrNullToObject(jprofile, "profileOwnerMccMnc", euicc_mccmnc2str(rptr->profileOwner.mccmnc));
    cJSON_AddStringOrNullToObject(jprofile, "profileOwnerGid1", rptr->profileOwner.gid1);
    cJSON_AddStringOrNullToObject(jprofile, "profileOwnerGid2", rptr->profileOwner.gid2);
    cJSON_AddStringOrNullToObject(jprofile, "profileNickname", rptr->profileNickname);
    cJSON_AddStringOrNullToObject(jprofile, "serviceProviderName", rptr->serviceProviderName);
    cJSON_AddStringOrNullToObject(jprofile, "profileName", rptr->profileName);
//    cJSON_AddStringOrNullToObject(jprofile, "profileClass", euicc_profileclass2str(rptr->profileClass));
//    cJSON_AddStringOrNullToObject(jprofile, "iconType", euicc_icontype2str(rptr->iconType));
//    cJSON_AddStringOrNullToObject(jprofile, "icon", rptr->icon);
//    cJSON *notification = cJSON_CreateArray();
//
//    struct notification_configuration_information *cfg_info;
//    cfg_info = rptr->notificationConfigurationInfo;
//    while (cfg_info) {
//        cJSON *notification_cfg_item = NULL;
//        notification_cfg_item = cJSON_CreateObject();
//        cJSON_AddNumberToObject(notification_cfg_item, "operation", cfg_info->profileManagementOperation);
//        cJSON_AddStringOrNullToObject(notification_cfg_item, "notification_address", cfg_info->notificationAddress);
//        cJSON_AddItemReferenceToArray(notification, notification_cfg_item);
//        cfg_info = cfg_info->next;
//    }
//    cJSON_AddItemReferenceToObject(jprofile, "notificationConfigurationInfo", notification);
    return jprofile;
}

cJSON * metadata_to_cjson_full(const struct es10c_profile_info_list *rptr) {
    cJSON* jprofile = cJSON_CreateObject();
    cJSON_AddStringOrNullToObject(jprofile, "iccid", rptr->iccid);
    cJSON_AddStringOrNullToObject(jprofile, "isdpAid", rptr->isdpAid);
    cJSON_AddStringOrNullToObject(jprofile, "profileState", euicc_profilestate2str(rptr->profileState));
    cJSON_AddStringOrNullToObject(jprofile, "profileOwnerMccMnc", euicc_mccmnc2str(rptr->profileOwner.mccmnc));
    cJSON_AddStringOrNullToObject(jprofile, "profileOwnerGid1", rptr->profileOwner.gid1);
    cJSON_AddStringOrNullToObject(jprofile, "profileOwnerGid2", rptr->profileOwner.gid2);
    cJSON_AddStringOrNullToObject(jprofile, "profileNickname", rptr->profileNickname);
    cJSON_AddStringOrNullToObject(jprofile, "serviceProviderName", rptr->serviceProviderName);
    cJSON_AddStringOrNullToObject(jprofile, "profileName", rptr->profileName);
    cJSON_AddStringOrNullToObject(jprofile, "iconType", euicc_icontype2str(rptr->iconType));
    cJSON_AddStringOrNullToObject(jprofile, "icon", rptr->icon);
    cJSON_AddStringOrNullToObject(jprofile, "profileClass", euicc_profileclass2str(rptr->profileClass));
    cJSON *notification = cJSON_CreateArray();

    struct notification_configuration_information *cfg_info;
    cfg_info = rptr->notificationConfigurationInfo;
    while (cfg_info) {
        cJSON *notification_cfg_item = NULL;
        notification_cfg_item = cJSON_CreateObject();
        cJSON_AddNumberToObject(notification_cfg_item, "operation", cfg_info->profileManagementOperation);
        cJSON_AddStringOrNullToObject(notification_cfg_item, "notification_address", cfg_info->notificationAddress);
        cJSON_AddItemReferenceToArray(notification, notification_cfg_item);
        cfg_info = cfg_info->next;
    }
    cJSON_AddItemReferenceToObject(jprofile, "notificationConfigurationInfo", notification);
    return jprofile;
}


char* get_profiles()
{
    struct es10c_profile_info_list *profiles, *rptr;
    if (es10c_get_profiles_info(&euicc_ctx, &profiles)) return NULL;

    cJSON *jdata = cJSON_CreateArray();
    rptr = profiles;

    while (rptr)
    {
        cJSON_AddItemToArray(jdata, metadata_to_cjson(rptr));
        rptr = rptr->next;
    }
    es10c_profile_info_list_free_all(profiles);
    char* ret = cJSON_PrintUnformatted(jdata);
    free(jdata);
    return ret;
}

char* enable_profile(char *iccid, char* refreshflag) {
    int val = es10c_enable_profile(&euicc_ctx, iccid, refreshflag[0] == '1');
    free(iccid);
    free(refreshflag);
    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "result", val);
    char* ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}

char* disable_profile(char *iccid, char* refreshflag) {
    int val = es10c_disable_profile(&euicc_ctx, iccid, refreshflag[0] == '1');
    free(iccid);
    free(refreshflag);
    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "result", val);
    char* ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}

char* rename_profile(char *iccid, char* nickname) {
    int val = es10c_set_nickname(&euicc_ctx, iccid, nickname);
    free(iccid);
    free(nickname);
    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "result", val);
    char* ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}

char* delete_profile(char *iccid) {
    int val = es10c_delete_profile(&euicc_ctx, iccid);
    free(iccid);
    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "result", val);
    char* ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}

char* dump_cert(char* smdp) {
    int val;
    char* ret;
    char* step;
    char* message;
    cJSON *result = cJSON_CreateObject();
    struct euicc_ctx ctx = {0};
    val = es10b_get_euicc_challenge_and_info(&ctx);
    ctx.http._internal.server_address = smdp;
    val = es9p_initiate_authentication(&ctx);
    if (val) {
        step = "es9p_initiate_authentication";
        message = ctx.http.status.message;
        goto error;
    }
    val = es10b_authenticate_server(&ctx, "", "114514191981003");
    if (val) {
        step = "es10b_authenticate_server";
        message = "es10b_authenticate_server failed";
        goto error;
    }
    cJSON_AddStringToObject(result, "authenticate_server_response", ctx.http._internal.b64_authenticate_server_response);
    cJSON_AddBoolToObject(result, "success", 1);
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;

    error:
    cJSON_AddBoolToObject(result, "success", 0);
    cJSON_AddStringOrNullToObject(result, "step", step);
    cJSON_AddStringOrNullToObject(result, "message", message);
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}


char* authenticate_profile(char* smdp, char* matchingId, char* imei) {
    int val;
    char* ret;
    char* step;
    char* message;
    char* reasonCode;
    char* subjectCode;
    cJSON *result = cJSON_CreateObject();
    struct euicc_ctx ctx = {0};
    val = es10b_get_euicc_challenge_and_info(&ctx);
    if (val) {
        step = "es10b_get_euicc_challenge";
        message = "es10b_get_euicc_challenge failed";
        goto error;
    }
    ctx.http._internal.server_address = smdp;

    printf("es10b_get_euicc_challenge_and_info ok!\n");
    val = es9p_initiate_authentication(&ctx);
    if (val) {
        step = "es9p_initiate_authentication";
        message = ctx.http.status.message;
        goto error;
    }

    printf("es9p_initiate_authentication ok!\n");
    val = es10b_authenticate_server(&ctx, matchingId, imei);
    if (val) {
        step = "es10b_authenticate_server";
        message = "es10b_authenticate_server failed";
        goto error;
    }
    cJSON_AddStringToObject(result, "authenticate_server_response", ctx.http._internal.b64_authenticate_server_response);

    printf("es10b_authenticate_server ok!\n");

    val = es9p_authenticate_client(&ctx);

    if (val) {
        step = "es9p_authenticate_client";
        message = ctx.http.status.message;
        reasonCode = ctx.http.status.reasonCode;
        subjectCode = ctx.http.status.subjectCode;
        goto error;
    }

    uint32_t smdpSigned2_len = strlen(ctx.http._internal.prepare_download_param->b64_smdpSigned2);
    unsigned char* smdpSigned2 = malloc(smdpSigned2_len + 1);
    smdpSigned2_len = euicc_base64_decode(smdpSigned2, ctx.http._internal.prepare_download_param->b64_smdpSigned2);


    struct euicc_derutil_node node, tmpnode;
    euicc_derutil_unpack_first(&node, smdpSigned2, smdpSigned2_len);

    tmpnode.self.ptr = node.value;
    tmpnode.self.length = 0;
    euicc_derutil_unpack_next(&tmpnode, &tmpnode, smdpSigned2, smdpSigned2_len); // tx-id
    euicc_derutil_unpack_next(&tmpnode, &tmpnode, smdpSigned2, smdpSigned2_len); // isCcRequired

    cJSON_AddBoolToObject(result, "isCcRequired", *tmpnode.value);

    cJSON_AddStringToObject(result, "profile_metadata_b64", ctx.http._internal.prepare_download_param->b64_profileMetadata);

    uint32_t metadata_len = euicc_base64_decode_len(ctx.http._internal.prepare_download_param->b64_profileMetadata);
    unsigned char* tmp = malloc(metadata_len + 1);
    metadata_len = euicc_base64_decode(tmp, ctx.http._internal.prepare_download_param->b64_profileMetadata);
    euicc_derutil_unpack_find_tag(&node, 0xBF25, tmp, metadata_len);
    struct es10c_profile_info_list *rptr = decode_profile_metadata(&node);
    cJSON *json = metadata_to_cjson(rptr);
    es10c_profile_info_list_free_all(rptr);
    free(tmp);

    cJSON_AddItemReferenceToObject(result, "profile", json);
    char* transaction_id = malloc(2 * ctx.http._internal.transaction_id_bin_len + 1);
    for (int i = 0; i < ctx.http._internal.transaction_id_bin_len; i++) {
        sprintf(transaction_id + 2 * i, "%02x", ctx.http._internal.transaction_id_bin[i]);
    }

    cJSON_AddStringToObject(result, "http_transaction_id", ctx.http._internal.transaction_id_http);
    cJSON_AddStringToObject(result, "transaction_id_hex", transaction_id);
    cJSON_AddStringToObject(result, "_internal", serialize_internal(&ctx.http._internal));
    free(transaction_id);
    cJSON_AddBoolToObject(result, "success", 1);
    cJSON_AddNumberToObject(result, "result", val);
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;

    error:
    cJSON_AddBoolToObject(result, "success", 0);
    cJSON_AddStringOrNullToObject(result, "step", step);
    cJSON_AddStringOrNullToObject(result, "message", message);
    cJSON_AddStringOrNullToObject(result, "reasonCode", reasonCode);
    cJSON_AddStringOrNullToObject(result, "subjectCode", subjectCode);
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;

}

char* cancel_download(const char * _internal) {
    char *ret;
    struct euicc_ctx ctx = {0};
    deserialize_internal(&ctx.http._internal, _internal);
    es10b_cancel_session(&ctx, 0);

    es9p_cancel_session(&ctx);
    cJSON *result = cJSON_CreateObject();
    cJSON_AddBoolToObject(result, "success", 1);
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}

char* download_profile(const char *_internal, char* confirmation_code) {

    char* ret;
    char* step;
    char* message;
    char* reasonCode;
    char* subjectCode;
    struct euicc_ctx ctx = {0};
    deserialize_internal(&ctx.http._internal, _internal);

    struct es10c_ex_euiccinfo2 euiccinfo2;
    es10c_ex_get_euiccinfo2(&euicc_ctx, &euiccinfo2);
    int previousMemory = euiccinfo2.extCardResource.freeNonVolatileMemory;

    cJSON *result = cJSON_CreateObject();

    struct es10b_load_bound_profile_package_result download_result = {0};
    int val;

    val = es10b_prepare_download(&ctx, confirmation_code);

    if (val) {
        step = "es10b_prepare_download";
        message = "es10b_prepare_download failed";
        goto error;
    }

    if (es9p_get_bound_profile_package(&ctx))
    {
        step = "es9p_get_bound_profile_package";
        message = ctx.http.status.message;
        reasonCode = ctx.http.status.reasonCode;
        subjectCode = ctx.http.status.subjectCode;
        goto error;
    }

    // cJSON_AddStringToObject(result, "download_bpp", ctx.http._internal.b64_bound_profile_package);
    cJSON_AddNumberToObject(result, "download_bpp_size", strlen(ctx.http._internal.b64_bound_profile_package));

    printf("es9p_get_bound_profile_package!\n");
    if (es10b_load_bound_profile_package(&ctx, &download_result))
    {
        cJSON_AddNumberToObject(result, "bppCommandId", download_result.bppCommandId);
        cJSON_AddNumberToObject(result, "errorReason", download_result.errorReason);
        step = "es10b_load_bound_profile_package";
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Command %d: %s\nReason %d: %s",
                 download_result.bppCommandId,
                 euicc_bppcommandid2str(download_result.bppCommandId),
                 download_result.errorReason,
                 euicc_errorreason2str(download_result.errorReason)
                 );
        message = buffer;


        es10c_ex_get_euiccinfo2(&euicc_ctx, &euiccinfo2);
        int afterMemory = euiccinfo2.extCardResource.freeNonVolatileMemory;
        cJSON_AddNumberToObject(result, "download_size", ctx.bpp_len);
        cJSON_AddNumberToObject(result, "space_consumed", previousMemory - afterMemory);
        goto error;
    }

    es10c_ex_get_euiccinfo2(&euicc_ctx, &euiccinfo2);
    int afterMemory = euiccinfo2.extCardResource.freeNonVolatileMemory;

    printf("es10c_ex_get_euiccinfo2!\n");

    cJSON_AddBoolToObject(result, "success", 1);
    cJSON_AddBoolToObject(result, "download_success", 1);
    cJSON_AddNumberToObject(result, "download_size", ctx.bpp_len);
    cJSON_AddNumberToObject(result, "space_consumed", previousMemory - afterMemory);
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;

    error:
    cJSON_AddBoolToObject(result, "success", 0);
    cJSON_AddBoolToObject(result, "download_success", 0);
    cJSON_AddStringOrNullToObject(result, "step", step);
    cJSON_AddStringOrNullToObject(result, "message", message);
    cJSON_AddStringOrNullToObject(result, "reasonCode", reasonCode);
    cJSON_AddStringOrNullToObject(result, "subjectCode", subjectCode);
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}
