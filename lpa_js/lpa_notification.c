
#include "main.h"
#include "state_serializer.h"

char* get_notifications()
{
    struct es10b_notification_metadata_list *notifications, *rptr;
    if (es10b_list_notification(&euicc_ctx, &notifications)) return NULL;

    cJSON *jdata = cJSON_CreateArray();
    rptr = notifications;
    while (rptr)
    {
        cJSON *jnotification = NULL;

        jnotification = cJSON_CreateObject();
        cJSON_AddNumberToObject(jnotification, "seqNumber", rptr->seqNumber);
        cJSON_AddNumberToObject(jnotification, "profileManagementOperation", rptr->profileManagementOperation);
        cJSON_AddStringOrNullToObject(jnotification, "notificationAddress", rptr->notificationAddress);
        cJSON_AddStringOrNullToObject(jnotification, "iccid", rptr->iccid);
        cJSON_AddItemToArray(jdata, jnotification);

        rptr = rptr->next;
    }
    es10b_notification_metadata_list_free_all(notifications);

    // es10c_profile_info_list_free_all(profiles);
    char* ret = cJSON_PrintUnformatted(jdata);
    free(jdata);
    return ret;
}

// ES10B_PROFILE_MANAGEMENT_OPERATION_INSTALL = 0x80,
// ES10B_PROFILE_MANAGEMENT_OPERATION_ENABLE = 0x40,
// ES10B_PROFILE_MANAGEMENT_OPERATION_DISABLE = 0x20,
// ES10B_PROFILE_MANAGEMENT_OPERATION_DELETE = 0x10,

char* process_notifications(char* iccid, int mask, int need_delete)
{
    struct es10b_notification_metadata_list *notifications, *rptr;
    struct es10b_pending_notification notification;

    cJSON *result = cJSON_CreateObject();
    if (es10b_list_notification(&euicc_ctx, &notifications)) goto fail;


    char* ret;
    rptr = notifications;
    for (; rptr; rptr = rptr->next)
    {
        if (strlen(iccid) && strcmp(rptr->iccid, iccid)) {
            continue;
        }
        if ((rptr->profileManagementOperation & mask) == rptr->profileManagementOperation) {
            if (es10b_retrieve_notifications_list(&euicc_ctx, &notification, rptr->seqNumber)) {
                goto fail;
            }
            euicc_ctx.http._internal.server_address = notification.notificationAddress;
            if (es9p_handle_notification(&euicc_ctx, notification.b64_PendingNotification)) {
                goto fail;
            }
            es10b_pending_notification_free(&notification);
            if (need_delete) {
                es10b_remove_notification_from_list(&euicc_ctx, rptr->seqNumber);
            }
        }
    }
    fail:
    // es10c_profile_info_list_free_all(profiles);
    cJSON_AddNumberToObject(result, "result", 0);
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}

char* process_notifications_masked(char* iccid, int process_mask, int delete_mask)
{
    struct es10b_notification_metadata_list *notifications, *rptr;
    struct es10b_pending_notification notification;

    cJSON *result = cJSON_CreateObject();
    int result_val = es10b_list_notification(&euicc_ctx, &notifications);
    if (result_val) {
        cJSON_AddNumberToObject(result, "result", result_val);
    } else {
        int success = 0;
        int fail = 0;


        rptr = notifications;
        for (; rptr; rptr = rptr->next)
        {
            if (strlen(iccid) && strcmp(rptr->iccid, iccid)) {
                continue;
            }
            if ((rptr->profileManagementOperation & process_mask) == rptr->profileManagementOperation) {
                if (es10b_retrieve_notifications_list(&euicc_ctx, &notification, rptr->seqNumber)) {
                    fail += 1;
                    continue;
                }
                euicc_ctx.http._internal.server_address = notification.notificationAddress;
                if (es9p_handle_notification(&euicc_ctx, notification.b64_PendingNotification)) {
                    fail += 1;
                    continue;
                }
                es10b_pending_notification_free(&notification);
                if ((rptr->profileManagementOperation & delete_mask & process_mask)) {
                    es10b_remove_notification_from_list(&euicc_ctx, rptr->seqNumber);
                }
                success += 1;
            }
        }

        cJSON_AddNumberToObject(result, "result", 0);
        cJSON_AddNumberToObject(result, "success", success);
        cJSON_AddNumberToObject(result, "fail", fail);
    }

    char* ret;
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}

/*
char* fetch_notifications_masked(int mask)
{
    struct es10b_notification_metadata_list *notifications, *rptr;
    struct es10b_pending_notification notification;

    cJSON *result = cJSON_CreateObject();

    if (es10b_retrieve_notifications_list(&euicc_ctx, &notification, rptr->seqNumber)) {
        fail += 1;
        continue;
    }


    if (result_val) {
        cJSON_AddNumberToObject(result, "result", result_val);
    } else {
        int success = 0;
        int fail = 0;


        rptr = notifications;
        for (; rptr; rptr = rptr->next)
        {
            if (strlen(iccid) && strcmp(rptr->iccid, iccid)) {
                continue;
            }
            if ((rptr->profileManagementOperation & process_mask) == rptr->profileManagementOperation) {

                euicc_ctx.http._internal.server_address = notification.notificationAddress;
                if (es9p_handle_notification(&euicc_ctx, notification.b64_PendingNotification)) {
                    fail += 1;
                    continue;
                }
                es10b_pending_notification_free(&notification);
                if ((rptr->profileManagementOperation & delete_mask & process_mask)) {
                    es10b_remove_notification_from_list(&euicc_ctx, rptr->seqNumber);
                }
                success += 1;
            }
        }

        cJSON_AddNumberToObject(result, "result", 0);
        cJSON_AddNumberToObject(result, "success", success);
        cJSON_AddNumberToObject(result, "fail", fail);
    }

    char* ret;
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}
*/

char* process_notification_single(int seqNumber)
{
    struct es10b_pending_notification notification;

    cJSON *result = cJSON_CreateObject();
    if (es10b_retrieve_notifications_list(&euicc_ctx, &notification, seqNumber)) {
        cJSON_AddNumberToObject(result, "result", -1);
        goto fail;
    }
    euicc_ctx.http._internal.server_address = notification.notificationAddress;
    if (es9p_handle_notification(&euicc_ctx, notification.b64_PendingNotification)) {
        cJSON_AddNumberToObject(result, "result", -1);
        goto fail;
    }
    es10b_pending_notification_free(&notification);
    cJSON_AddNumberToObject(result, "result", 0);
    char* ret;
    fail:
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}


char* get_notification_single(int seqNumber)
{
    struct es10b_pending_notification notification;

    cJSON *result = cJSON_CreateObject();
    if (es10b_retrieve_notifications_list(&euicc_ctx, &notification, seqNumber)) {
        cJSON_AddNumberToObject(result, "result", -1);
        goto fail;
    }
    cJSON_AddStringOrNullToObject(result, "notificationAddress", notification.notificationAddress);
    cJSON_AddStringOrNullToObject(result, "b64_PendingNotification", notification.b64_PendingNotification);
    es10b_pending_notification_free(&notification);
    cJSON_AddNumberToObject(result, "result", 0);
    char* ret;
    fail:
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}

char* delete_notification_single(int seqNumber)
{
    cJSON *result = cJSON_CreateObject();
    cJSON_AddNumberToObject(result, "result", es10b_remove_notification_from_list(&euicc_ctx, seqNumber));
    char* ret;
    ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}
