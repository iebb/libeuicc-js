//
// Created by ieb on 2024/11/05.
//

#include "euicc/euicc.h"
#ifndef STATE_SERIALIZER_H
#define STATE_SERIALIZER_H


int deserialize_internal(struct http_internals *data, const char *json_str);
char* serialize_internal(const struct http_internals *data);

#endif //STATE_SERIALIZER_H
