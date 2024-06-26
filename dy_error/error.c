#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "esp_log.h"
#include "dy/error.h"

// FIXME: concurrent usage
char buf1[DY_ERROR_DESC_MAX_LEN] = {0};
char buf2[DY_ERROR_DESC_MAX_LEN] = {0};

dy_err_t dy_err(dy_err_code_t code, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf1, DY_ERROR_DESC_MAX_LEN, fmt, args);
    va_end(args);

    return (dy_err_t) {code, buf1};
}

char *dy_err_str(dy_err_t err) {
    snprintf(buf2, DY_ERROR_DESC_MAX_LEN, "%s: %s", dy_err_code_str(err.code), err.desc);
    return buf2;
}

dy_err_t dy_ok() {
    return (dy_err_t) {DY_OK, NULL};
}

bool dy_nok(dy_err_t err) {
    return err.code != DY_OK;
}

dy_err_t dy_err_pfx(const char *prefix, dy_err_t err) {
    return dy_err(err.code, "%s: %s", prefix, err.desc);
}

const char *dy_err_code_str(dy_err_code_t e) {
    switch (e) {
        case DY_OK:
            return "no error";

        case DY_ERR_FAILED:
            return "operation failed";
        case DY_ERR_NO_MEM:
            return "no memory";
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

        case DY_ERR_JSON_PARSE:
            return "json parse failed";

        default:
            return "unknown error";
    }
}
