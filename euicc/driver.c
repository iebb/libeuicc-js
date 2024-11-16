#include "driver.h"
#include <emscripten.h>


#define EUICC_INTERFACE_BUFSZ 16384


EM_ASYNC_JS(int, sendApdu, (const uint8_t* str, int length, uint8_t* resp_str), {
    const byteArray = new Uint8Array(Module.HEAPU8.buffer, str, length);
    const hexString = Array.from(byteArray).map(b => b.toString(16).padStart(2, '0')).join("");
    const resultHexString = await Module.jsSendApdu(hexString);
    const resp_length = resultHexString.length / 2;

    // Convert the hex result back to binary
    const binaryResult = new Uint8Array(resp_length);
    for (let i = 0; i < binaryResult.length; i++) {
        binaryResult[i] = parseInt(resultHexString.substr(i * 2, 2), 16);
    }

    Module.HEAPU8.set(binaryResult, resp_str);
    return resp_length;
});


// Define your function with EM_ASYNC_JS
EM_ASYNC_JS(int, postRequest, (const char* url, uint32_t* rcode, uint8_t** rx, uint32_t* rx_len, const uint8_t* tx, uint32_t tx_len), {
    try {
        const [ok, resultStatusCode, resultString] = await Module.jsFetch(UTF8ToString(url), UTF8ToString(tx));
        const bufferPtr = _malloc(resultString.length);
        for (let i = 0; i < resultString.length; i++) {
            HEAPU8[bufferPtr + i] = resultString.charCodeAt(i);
        } // Only works for ASCII
        setValue(rx, bufferPtr, '*');
        setValue(rx_len, resultString.length, 'i32');
        setValue(rcode, resultStatusCode, 'i32');
        return 0;
    } catch (error) {
        setValue(rcode, 0, 'i32');
        setValue(rx_len, 0, 'i32');
        return -1;
    }
});


// {"type":"http","payload":{"rcode":404,"rx":"333435"}}
int http_transmit(const char *url, uint32_t *rcode, uint8_t **rx, uint32_t *rx_len, const uint8_t *tx, uint32_t tx_len)
{
    return postRequest(url, rcode, rx, rx_len, tx, tx_len);
}


int pcsc_transmit_lowlevel(uint8_t *rx, uint32_t *rx_len, const uint8_t *tx, const uint8_t tx_len)
{
    uint32_t rx_len_merged = *rx_len;

    rx_len_merged = sendApdu((uint8_t *)tx, tx_len, rx);
    if (rx_len_merged <= 0)
    {
        fprintf(stderr, "SCardTransmit() failed: %08X\n", rx_len_merged);
        return -1;
    }

    *rx_len = rx_len_merged;
    return 0;
}

int apdu_transmit(uint8_t **rx, uint32_t *rx_len, const uint8_t *tx, uint32_t tx_len)
{
    *rx = malloc(EUICC_INTERFACE_BUFSZ);
    *rx_len = EUICC_INTERFACE_BUFSZ;

    if (pcsc_transmit_lowlevel(*rx, rx_len, tx, tx_len) < 0)
    {
        free(*rx);
        *rx_len = 0;
        return -1;
    }

    return 0;
}
