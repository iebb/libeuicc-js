#include "es8p.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "derutil.h"
#include "hexutil.h"
#include "base64.h"

int es8p_metadata_parse(struct es8p_metadata **stru_metadata, const char *b64_Metadata)
{
    int ret;
    uint8_t *metadata = NULL;
    int metadata_len = 0;
    struct euicc_derutil_node n_metadata, n_iter;
    struct es8p_metadata *p = NULL;

    *stru_metadata = NULL;

    memset(&n_metadata, 0x00, sizeof(n_metadata));
    memset(&n_iter, 0x00, sizeof(n_iter));

    metadata = malloc(euicc_base64_decode_len(b64_Metadata));
    if (!metadata)
    {
        goto err;
    }

    if ((metadata_len = euicc_base64_decode(metadata, b64_Metadata)) < 0)
    {
        goto err;
    }

    if (euicc_derutil_unpack_find_tag(&n_metadata, 0xBF25, metadata, metadata_len) < 0)
    {
        goto err;
    }

    if (!(p = malloc(sizeof(struct es8p_metadata))))
    {
        goto err;
    }

    memset(p, 0, sizeof(*p));

    n_iter.self.ptr = n_metadata.value;
    n_iter.self.length = 0;

    p->profileClass = ES10C_PROFILE_CLASS_NULL;
    p->iconType = ES10C_ICON_TYPE_NULL;

    while (euicc_derutil_unpack_next(&n_iter, &n_iter, n_metadata.value, n_metadata.length) == 0)
    {
        int tmplong;
        switch (n_iter.tag)
        {
        case 0x5A:
            euicc_hexutil_bin2gsmbcd(p->iccid, sizeof(p->iccid), n_iter.value, n_iter.length);
            break;
        case 0x91:
            p->serviceProviderName = malloc(n_iter.length + 1);
            if (p->serviceProviderName)
            {
                memcpy(p->serviceProviderName, n_iter.value, n_iter.length);
                p->serviceProviderName[n_iter.length] = '\0';
            }
            break;
        case 0x92:
            p->profileName = malloc(n_iter.length + 1);
            if (p->profileName)
            {
                memcpy(p->profileName, n_iter.value, n_iter.length);
                p->profileName[n_iter.length] = '\0';
            }
            break;
        case 0x93:
            tmplong = euicc_derutil_convert_bin2long(n_iter.value, n_iter.length);
            switch (tmplong)
            {
            case ES10C_ICON_TYPE_JPEG:
            case ES10C_ICON_TYPE_PNG:
                p->iconType = tmplong;
                break;
            default:
                p->iconType = ES10C_ICON_TYPE_UNDEFINED;
                break;
            }
            break;
        case 0x94:
            p->icon = malloc(euicc_base64_encode_len(n_iter.length));
            if (p->icon)
            {
                euicc_base64_encode(p->icon, n_iter.value, n_iter.length);
            }
            break;
        case 0x95:
            tmplong = euicc_derutil_convert_bin2long(n_iter.value, n_iter.length);
            switch (tmplong)
            {
            case ES10C_PROFILE_CLASS_TEST:
            case ES10C_PROFILE_CLASS_PROVISIONING:
            case ES10C_PROFILE_CLASS_OPERATIONAL:
                p->profileClass = tmplong;
                break;
            default:
                p->profileClass = ES10C_PROFILE_CLASS_UNDEFINED;
                break;
            }
            break;
            case 0xB7: {
                struct euicc_derutil_node owner;
                owner.self.ptr = n_iter.value;
                owner.self.length = 0;
                p->profileOwner.mccmnc = malloc(11);
                while (euicc_derutil_unpack_next(&owner, &owner, n_iter.value, n_iter.length) == 0) {
                    switch (owner.tag) {
                        case 0x80:
                            euicc_hexutil_bin2gsmbcd_nb(p->profileOwner.mccmnc, 6 + 1, owner.value, owner.length);
                            break;
                        case 0x81:
                            p->profileOwner.gid1 = malloc(owner.length + 1);
                            memcpy(p->profileOwner.gid1, owner.value, owner.length);
                            p->profileOwner.gid1[owner.length] = '\0';
                            break;
                        case 0x82:
                            p->profileOwner.gid2 = malloc(owner.length + 1);
                            memcpy(p->profileOwner.gid2, owner.value, owner.length);
                            p->profileOwner.gid2[owner.length] = '\0';
                            break;
                    }
                    break;
                }
                break;
            }
            case 0xB6: {
                struct euicc_derutil_node notify_info_list;
                notify_info_list.self.ptr = n_iter.value;
                notify_info_list.self.length = 0;
                struct notification_configuration_information *first_cfg_info = NULL;
                struct notification_configuration_information *current_cfg_info = NULL;
                struct notification_configuration_information *last_cfg_info = NULL;

                while (euicc_derutil_unpack_next(&notify_info_list, &notify_info_list, n_iter.value, n_iter.length) == 0) {
                    struct euicc_derutil_node notify_info;
                    notify_info.self.ptr = notify_info_list.value;
                    notify_info.self.length = 0;
                    current_cfg_info = (struct notification_configuration_information *)malloc(sizeof(struct notification_configuration_information));
                    memset(current_cfg_info, 0, sizeof(struct notification_configuration_information));
                    while (euicc_derutil_unpack_next(&notify_info, &notify_info, notify_info_list.value, notify_info_list.length) == 0) {
                        switch (notify_info.tag) {
                            case 0x80:
                                current_cfg_info->profileManagementOperation = notify_info.value[1];
                                break;
                            case 0x81:
                                current_cfg_info->notificationAddress = malloc(notify_info.length + 1);
                                memcpy(current_cfg_info->notificationAddress, notify_info.value, notify_info.length);
                                current_cfg_info->notificationAddress[notify_info.length] = '\0';
                        }
                    }
                    current_cfg_info->next = 0;
                    if (!last_cfg_info) {
                        first_cfg_info = current_cfg_info;
                        last_cfg_info = first_cfg_info;
                    } else {
                        last_cfg_info->next = current_cfg_info;
                        last_cfg_info = current_cfg_info;
                    }
                }
                p->notificationConfigurationInfo = first_cfg_info;
                break;
            }
        case 0x99:
            // fprintf(stderr, "\n[PLEASE REPORT][TODO][TAG %02X]: ", n_iter.tag);
            // for (uint32_t i = 0; i < n_iter.self.length; i++)
            // {
            //     fprintf(stderr, "%02X ", n_iter.self.ptr[i]);
            // }
            // fprintf(stderr, "\n");
            break;
        }
    }

    *stru_metadata = p;
    ret = 0;
    goto exit;

err:
    ret = -1;
    free(*stru_metadata);
    *stru_metadata = NULL;
    free(p);
    p = NULL;
exit:
    free(metadata);
    metadata = NULL;

    return ret;
}

void es8p_metadata_free(struct es8p_metadata **stru_metadata)
{
    struct es8p_metadata *p = *stru_metadata;

    if (p == NULL)
    {
        return;
    }

    free(p->serviceProviderName);
    free(p->profileName);
    free(p->icon);
    free(p);

    *stru_metadata = NULL;
}
