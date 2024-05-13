#include <stdio.h>
#include <stdarg.h>
#include "esp_err.h"
#include "esp_log.h"
#include "dy/error.h"

dy_err_t dy_error(dy_err_code_t code, const char *fmt, ...) {
    char desc[DY_ERROR_DESC_MAX_LEN] = {0};

    va_list args;
    va_start(args, fmt);
    vsnprintf(desc, 128, fmt, args);
    va_end(args);

    return (dy_err_t) {code, desc};
}

dy_err_t dy_ok() {
    return (dy_err_t) {DY_OK, NULL};
}

char *dy_err_desc(dy_err_code_t e) {
    switch (e) {
        case DY_OK:
            return "no error";
        case DY_ERR_NO_MEM:
            return "no memory";
        case DY_ERR_OP_FAILED:
            return "operation failed";
        case DY_ERR_NOT_FOUND:
            return "not found";
        case DY_ERR_TIMEOUT:
            return "timeout";
        case DY_ERR_INVALID_ARG:
            return "invalid argument";
        case DY_ERR_INVALID_STATE:
            return "invalid state";
        case DY_ERR_INVALID_SIZE:
            return "invalid size";
        case DY_ERR_INVALID_VERSION:
            return "invalid version";
        default:
            return "unknown error";
    }
}

dy_err_code_t esp_to_dy_err(esp_err_t e) {
    switch (e) {
        case ESP_OK:
            return DY_OK;
        case ESP_ERR_NO_MEM:
            return DY_ERR_NO_MEM;
        case ESP_ERR_INVALID_ARG:
            return DY_ERR_INVALID_ARG;
        case ESP_ERR_INVALID_STATE:
            return DY_ERR_INVALID_STATE;
        case ESP_ERR_INVALID_SIZE:
            return DY_ERR_INVALID_SIZE;
        case ESP_ERR_NOT_FOUND:
            return DY_ERR_NOT_FOUND;
        case ESP_ERR_TIMEOUT:
            return DY_ERR_TIMEOUT;
        case ESP_ERR_INVALID_VERSION:
            return DY_ERR_INVALID_VERSION;
        default:
            return DY_ERR_UNKNOWN;
    }
}
