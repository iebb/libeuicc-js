#pragma once

#include <inttypes.h>
#include "interface.h"
#include "es10b.h"

struct http_internals
{
    char *server_address;
    char *transaction_id_http;
    uint8_t *transaction_id_bin;
    uint32_t transaction_id_bin_len;
    char *b64_euicc_challenge;
    char *b64_euicc_info_1;
    struct es10b_authenticate_server_param *authenticate_server_param;
    char *b64_authenticate_server_response;
    struct es10b_prepare_download_param *prepare_download_param;
    char *b64_prepare_download_response;
    char *b64_bound_profile_package;
    char *b64_cancel_session_response;
};

struct euicc_ctx
{
    const uint8_t *aid;
    uint8_t aid_len;
    uint32_t bpp_len;
    struct
    {
        struct
        {
            int logic_channel;
            struct
            {
                uint8_t apdu_header[5];
                uint8_t body[255];
            } __attribute__((packed)) request_buffer;
        } _internal;
    } apdu;
    struct
    {
        struct
        {
            char subjectCode[8 + 1];
            char reasonCode[8 + 1];
            char subjectIdentifier[128 + 1];
            char message[128 + 1];
        } status;
        struct http_internals _internal;
    } http;
    void *userdata;
};

void euicc_http_cleanup(struct euicc_ctx *ctx);
char* euicc_ctx_dump(struct euicc_ctx *ctx);
