#include <stdio.h>
#include <stdarg.h>
#include "esp_err.h"
#include "esp_log.h"
#include "dy/error.h"

dy_err_t dy_error(dy_err_code_t code, const char *fmt, ...) {
    char buf[DY_ERROR_DESC_MAX_LEN] = {0};

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 128, fmt, args);
    va_end(args);

    return (dy_err_t) {code, buf};
}

char *dy_error_str(dy_err_t err) {
    static char buf[DY_ERROR_DESC_MAX_LEN + 16] = {0};
    snprintf(buf, sizeof(buf), "%s: %s", dy_err_desc(err.code), err.desc);
    return buf;
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

        case DY_ERR_GPIO_SET:
            return "gpio set level failed";
        case DY_ERR_GPIO_GET:
            return "gpio get level failed";

        default:
            return "unknown error";
    }
}
