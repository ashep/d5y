/**
 * Author: Oleksandr Shepetko <a@shepetko.com>
 * License: MIT
 */

#ifndef DY_ERROR_H
#define DY_ERROR_H

#include "esp_err.h"

#define DY_ERROR_DESC_MAX_LEN 128

typedef enum {
    DY_OK,
    DY_ERR_UNKNOWN,

    DY_ERR_NO_MEM,
    DY_ERR_OP_FAILED,
    DY_ERR_NOT_FOUND,
    DY_ERR_TIMEOUT,

    DY_ERR_INVALID_ARG,
    DY_ERR_INVALID_STATE,
    DY_ERR_INVALID_SIZE,
    DY_ERR_INVALID_VERSION,
} dy_err_code_t;

/**
 * Verbose error.
 */
typedef struct {
    dy_err_code_t code;
    char *desc;
} dy_err_t;

/**
 * Formats a verbose error.
 */
dy_err_t dy_error(dy_err_code_t code, const char *fmt, ...);

/**
 * Make an DY_OK error.
 */
dy_err_t dy_ok();

/**
 * Get human readable error name.
 */
char *dy_err_desc(dy_err_code_t e);

/**
 * Convert an ESP error to a corresponding D5Y one.
 */
dy_err_code_t esp_to_dy_err(esp_err_t e);

#endif // DY_ERROR_H