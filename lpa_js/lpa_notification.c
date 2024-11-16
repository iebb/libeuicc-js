
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
    char* ret = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    return ret;
}
