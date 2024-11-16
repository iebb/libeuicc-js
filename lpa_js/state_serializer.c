//
// Created by ieb on 2024/11/05.
//

#include "state_serializer.h"
#include "main.h"


// Function to convert binary data to hex
char* hexlify(const uint8_t *data, uint32_t len) {
    char *hex_str = malloc(len * 2 + 1);  // Each byte -> 2 hex chars + 1 for null
    for (uint32_t i = 0; i < len; ++i) {
        sprintf(hex_str + i * 2, "%02X", data[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

// Function to convert hex string back to binary data
uint8_t* unhexlify(const char *hex_str, uint32_t len) {
    uint8_t *data = malloc(len);
    for (uint32_t i = 0; i < len; ++i) {
        sscanf(hex_str + i * 2, "%02hhX", &data[i]);
    }
    return data;
}


// Serialization function for es10b_authenticate_server_param
cJSON* serialize_authenticate_server_param(const struct es10b_authenticate_server_param *param) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "b64_serverSigned1", param->b64_serverSigned1);
    cJSON_AddStringToObject(json, "b64_serverSignature1", param->b64_serverSignature1);
    cJSON_AddStringToObject(json, "b64_euiccCiPKIdToBeUsed", param->b64_euiccCiPKIdToBeUsed);
    cJSON_AddStringToObject(json, "b64_serverCertificate", param->b64_serverCertificate);
    return json;
}

// Serialization function for es10b_prepare_download_param
cJSON* serialize_prepare_download_param(const struct es10b_prepare_download_param *param) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "b64_profileMetadata", param->b64_profileMetadata);
    cJSON_AddStringToObject(json, "b64_smdpSigned2", param->b64_smdpSigned2);
    cJSON_AddStringToObject(json, "b64_smdpSignature2", param->b64_smdpSignature2);
    cJSON_AddStringToObject(json, "b64_smdpCertificate", param->b64_smdpCertificate);
    return json;
}

// Serialization function for internal struct
char* serialize_internal(const struct http_internals *data) {
    cJSON *json = cJSON_CreateObject();

    cJSON_AddStringToObject(json, "server_address", data->server_address);
    cJSON_AddStringToObject(json, "transaction_id_http", data->transaction_id_http);

    cJSON_AddStringToObject(json, "b64_euicc_challenge", data->b64_euicc_challenge);
    cJSON_AddStringToObject(json, "b64_euicc_info_1", data->b64_euicc_info_1);

    // Add nested structs
    if (data->authenticate_server_param) {
        cJSON *auth_json = serialize_authenticate_server_param(data->authenticate_server_param);
        cJSON_AddItemToObject(json, "authenticate_server_param", auth_json);
    }
    cJSON_AddStringToObject(json, "b64_authenticate_server_response", data->b64_authenticate_server_response);

    if (data->prepare_download_param) {
        cJSON *prepare_json = serialize_prepare_download_param(data->prepare_download_param);
        cJSON_AddItemToObject(json, "prepare_download_param", prepare_json);
    }
    // cJSON_AddStringToObject(json, "b64_prepare_download_response", data->b64_prepare_download_response);
    // cJSON_AddStringToObject(json, "b64_bound_profile_package", data->b64_bound_profile_package);
    // cJSON_AddStringToObject(json, "b64_cancel_session_response", data->b64_cancel_session_response);

    // Convert cJSON to string
    char *serialized_data = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    return serialized_data;
}

// Deserialization functions
struct es10b_authenticate_server_param* deserialize_authenticate_server_param(cJSON *json) {
    struct es10b_authenticate_server_param* param = malloc(sizeof(struct es10b_authenticate_server_param));
    param->b64_serverSigned1 = strdup(cJSON_GetObjectItem(json, "b64_serverSigned1")->valuestring);
    param->b64_serverSignature1 = strdup(cJSON_GetObjectItem(json, "b64_serverSignature1")->valuestring);
    param->b64_euiccCiPKIdToBeUsed = strdup(cJSON_GetObjectItem(json, "b64_euiccCiPKIdToBeUsed")->valuestring);
    param->b64_serverCertificate = strdup(cJSON_GetObjectItem(json, "b64_serverCertificate")->valuestring);
    return param;
}

struct es10b_prepare_download_param* deserialize_prepare_download_param(cJSON *json) {
    struct es10b_prepare_download_param* param = malloc(sizeof(struct es10b_prepare_download_param));
    param->b64_profileMetadata = strdup(cJSON_GetObjectItem(json, "b64_profileMetadata")->valuestring);
    param->b64_smdpSigned2 = strdup(cJSON_GetObjectItem(json, "b64_smdpSigned2")->valuestring);
    param->b64_smdpSignature2 = strdup(cJSON_GetObjectItem(json, "b64_smdpSignature2")->valuestring);
    param->b64_smdpCertificate = strdup(cJSON_GetObjectItem(json, "b64_smdpCertificate")->valuestring);
    return param;
}

int deserialize_internal(struct http_internals *data, const char *json_str) {
    cJSON *json = cJSON_Parse(json_str);

    data->server_address = strdup(cJSON_GetObjectItem(json, "server_address")->valuestring);

    data->transaction_id_http = strdup(cJSON_GetObjectItem(json, "transaction_id_http")->valuestring);

    // Convert hex string back to binary data
    const char *hex_transaction_id_bin = cJSON_GetObjectItem(json, "transaction_id_http")->valuestring;
    data->transaction_id_bin = unhexlify(hex_transaction_id_bin, data->transaction_id_bin_len);

    data->b64_euicc_challenge = strdup(cJSON_GetObjectItem(json, "b64_euicc_challenge")->valuestring);
    data->b64_euicc_info_1 = strdup(cJSON_GetObjectItem(json, "b64_euicc_info_1")->valuestring);

    cJSON *auth_json = cJSON_GetObjectItem(json, "authenticate_server_param");
    if (auth_json) {
        data->authenticate_server_param = deserialize_authenticate_server_param(auth_json);
    }

    data->b64_authenticate_server_response = strdup(cJSON_GetObjectItem(json, "b64_authenticate_server_response")->valuestring);

    cJSON *prepare_json = cJSON_GetObjectItem(json, "prepare_download_param");
    if (prepare_json) {
        data->prepare_download_param = deserialize_prepare_download_param(prepare_json);
    }

    // cJSON_Delete(json);
    return 0;
}
