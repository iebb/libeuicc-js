//
// Created by ieb on 2024/11/04.
//

#ifndef JS_DRIVER_H
#define JS_DRIVER_H

#include <stdio.h>
#include <stdlib.h>

#include <euicc/interface.h>

int http_transmit(const char *url, uint32_t *rcode, uint8_t **rx, uint32_t *rx_len, const uint8_t *tx, uint32_t tx_len);
int apdu_transmit(uint8_t **rx, uint32_t *rx_len, const uint8_t *tx, uint32_t tx_len);



#endif //JS_DRIVER_H
