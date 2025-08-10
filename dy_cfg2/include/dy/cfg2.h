/**
 * @file cfg2.h
 * @brief Configuration management module header file.
 *
 * Functions to get and set configuration values of various types,
 *
 * Common errors:
 *  - DY_ERR_INVALID_ARG if the provided arguments are invalid (e.g., NULL pointers).
 *  - DY_ERR_NOT_CONFIGURED if dy_cfg2_init() was not called prior to using other functions.
 *  - DY_ERR_NOT_FOUND if a configuration value does not exist.
 *  - DY_ERR_FAILED for unexpected errors.
 *  - DY_ERR_OK on success.
 */

#pragma once

#include <stdint.h>
#include "dy/error.h"

#define DY_CFG2_VALUE_TYPE_U8  0x01
#define DY_CFG2_VALUE_TYPE_I8  0x02
#define DY_CFG2_VALUE_TYPE_STR 0x03

#define DY_CFG2_EVENT_BASE "DY_CFG2_EVENT_BASE"
#define DY_CFG2_STR_MAX_LEN 64

enum {
    DY_CFG2_EVENT_SET,
};

typedef struct {
    int id;
    uint8_t type;
    void *val;
} dy_cfg2_evt_set_t;

/**
 * @brief Gets a configuration value of type uint8_t.
 *
 * @param id The configuration parameter ID.
 * @param dst Pointer to store the retrieved value.
 */
dy_err_t dy_cfg2_get_u8(int id, uint8_t *dst);

/**
 * @brief Gets a configuration value of type uint8_t with a default value.
 *
 * If the configuration ID does not exist, the default value is returned.
 *
 * @param id The configuration parameter ID.
 * @param dst Pointer to store the retrieved value.
 * @param dft Default value to return if the configuration ID does not exist.
 */
dy_err_t dy_cfg2_get_u8_dft(int id, uint8_t *dst, uint8_t dft);

/**
 * @brief Sets a configuration value of type uint8_t.
 *
 * @param id The configuration parameter ID.
 * @param val The value to set.
 */
dy_err_t dy_cfg2_set_u8(int id, uint8_t val);

/**
 * @brief Gets a configuration value of type int8_t.
 *
 * @param id The configuration parameter ID.
 * @param dst Pointer to store the retrieved value.
 */
dy_err_t dy_cfg2_get_i8(int id, int8_t *dst);

/**
 * @brief Sets a configuration value of type int8_t.
 *
 * @param id The configuration parameter ID.
 * @param val The value to set.
 */
dy_err_t dy_cfg2_set_i8(int id, int8_t val);

/**
 * @brief Gets a configuration value of type string.
 *
 * @param id The configuration parameter ID.
 * @param dst Pointer to store the retrieved string.
 */
dy_err_t dy_cfg2_get_str(int id, char *dst);

/**
 * @brief Sets a configuration value of type string.
 *
 * @param id The configuration parameter ID.
 * @param val The string to set.
 */
dy_err_t dy_cfg2_set_str(int id, const char *val);

/**
 * @brief Initializes the dy_cfg2 module.
 * @warning Ensure to call nvs_flash_init() before calling this function.
 */
dy_err_t dy_cfg2_init();