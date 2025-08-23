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

#define DY_CFG2_VALUE_TYPE_U8    0x01
#define DY_CFG2_VALUE_TYPE_FLOAT 0x02
#define DY_CFG2_VALUE_TYPE_STR   0x03

#define DY_CFG2_EVENT_BASE "DY_CFG2_EVENT_BASE"
#define DY_CFG2_STR_MAX_LEN 256

enum {
    DY_CFG2_EVENT_SET,
};

typedef struct {
    int id;
    uint8_t type;
    void *val;
} dy_cfg2_evt_set_t;

/**
 * @brief Checks if a configuration parameter is set.
 *
 * @param id The configuration parameter ID.
 * @return true if the parameter is set, false otherwise.
 */
bool dy_cfg2_is_set(int id);

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
 * @brief Gets a configuration value of type float.
 *
 * @param id The configuration parameter ID.
 * @param dst Pointer to store the retrieved value.
 */
dy_err_t dy_cfg2_get_float(int id, float *dst);

/**
 * @brief Gets a configuration value of type float with a default value.
 *
 * If the configuration ID does not exist, the default value is returned.
 *
 * @param id The configuration parameter ID.
 * @param dst Pointer to store the retrieved value.
 * @param dft Default value to return if the configuration ID does not exist.
 */
dy_err_t dy_cfg2_get_float_dft(int id, float *dst, float dft);

/**
 * @brief Sets a configuration value of type float.
 *
 * @param id The configuration parameter ID.
 * @param val The value to set.
 */
dy_err_t dy_cfg2_set_float(int id, float val);

/**
 * @brief Gets a configuration value of type string.
 *
 * @param id The configuration parameter ID.
 * @param dst Pointer to store the retrieved string.
 */
dy_err_t dy_cfg2_get_str(int id, char *dst);

/**
 * @brief Gets a configuration value of type string with a default value.
 *
 * If the configuration ID does not exist, the default string is returned.
 *
 * @param id The configuration parameter ID.
 * @param dst Pointer to store the retrieved string.
 * @param dft Default string to return if the configuration ID does not exist.
 */
dy_err_t dy_cfg2_get_str_dft(int id, char *dst, const char *dft);

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