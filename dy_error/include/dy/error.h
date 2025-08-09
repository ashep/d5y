/**
 * Author: Oleksandr Shepetko <a@shepetko.com>
 * License: MIT
 */

#ifndef DY_ERROR_H
#define DY_ERROR_H

#include <stdbool.h>
#include "esp_err.h"

#define DY_ERROR_DESC_MAX_LEN 128

typedef enum {
    DY_OK,
    DY_ERR_UNKNOWN,

    DY_ERR_FAILED,
    DY_ERR_NO_MEM,
    DY_ERR_NO_CONTENT,
    DY_ERR_NOT_FOUND,
    DY_ERR_NOT_CONFIGURED,
    DY_ERR_TIMEOUT,

    DY_ERR_INVALID_ARG,
    DY_ERR_INVALID_STATE,
    DY_ERR_INVALID_SIZE,
    DY_ERR_INVALID_VERSION,

    DY_ERR_GPIO_SET,
    DY_ERR_GPIO_GET,

    DY_ERR_JSON_PARSE,
} dy_err_code_t;

/**
 * Error.
 */
typedef struct {
    dy_err_code_t code;
    char desc[DY_ERROR_DESC_MAX_LEN];
} dy_err_struct_t, *dy_err_t;

/**
 * Value with error result.
 */
typedef struct {
    dy_err_t err;
    void *val;
} dy_val_t;

/**
 * Creates an error.
 */
dy_err_t dy_err(dy_err_code_t code, const char *fmt, ...);

/**
 * Formats error as a string.
 */
char *dy_err_str(dy_err_t err);

/**
 * Make an DY_OK error.
 */
dy_err_t dy_ok();

/**
 * Checks whether err is not DY_OK.
 */
bool dy_is_err(dy_err_t err);

/**
 * Creates a new error from another one prefixed by a string.
 */
dy_err_t dy_err_pfx(const char *prefix, dy_err_t err);

/**
 * Get human readable error name.
 */
const char *dy_err_code_str(dy_err_code_t e);

#endif // DY_ERROR_H