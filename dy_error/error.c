#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "dy/error.h"

#define MAX_ERROR 16

static dy_err_struct_t errors[MAX_ERROR];
static char error_strings[MAX_ERROR][DY_ERROR_DESC_MAX_LEN];
static dy_err_struct_t ok = {.code=DY_OK, .desc="no error"};
static uint8_t cur_err;
static uint8_t cur_err_str;

dy_err_t dy_err(dy_err_code_t code, const char *fmt, ...) {
    dy_err_t err = &errors[cur_err];

    cur_err++;
    if (cur_err == MAX_ERROR) {
        cur_err = 0;
    }

    memset(err, 0, sizeof(dy_err_struct_t));
    err->code = code;

    va_list args;
    va_start(args, fmt);
    vsnprintf(err->desc, DY_ERROR_DESC_MAX_LEN - 1, fmt, args);
    va_end(args);

    return err;
}

char *dy_err_str(dy_err_t err) {
    char *err_str = error_strings[cur_err_str];
    if (cur_err_str == MAX_ERROR) {
        cur_err_str = 0;
    }

    memset(err_str, 0, DY_ERROR_DESC_MAX_LEN);

    const char *code_str = dy_err_code_str(err->code);
    strncpy(err_str, code_str, strlen(code_str));
    strncat(err_str, ": ", DY_ERROR_DESC_MAX_LEN - strlen(err_str) - 1);
    strncat(err_str, err->desc, DY_ERROR_DESC_MAX_LEN - strlen(err_str) - 1);

    return err_str;
}

dy_err_t dy_ok() {
    return (dy_err_t) &ok;
}

bool dy_is_err(dy_err_t err) {
    return err->code != DY_OK;
}

dy_err_t dy_err_pfx(const char *prefix, dy_err_t err) {
    return dy_err(err->code, "%s: %s", prefix, err->desc);
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
        case DY_ERR_NOT_CONFIGURED:
            return "not configured";
        case DY_ERR_NO_CONTENT:
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
