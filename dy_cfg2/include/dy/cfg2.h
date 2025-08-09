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

#define DY_CFG2_STR_MAX_LEN 64

/**
 * @brief Gets a configuration value of type uint8_t.
 *
 * @param id The configuration parameter ID.
 * @param dst Pointer to store the retrieved value.
 */
dy_err_t dy_cfg2_get_u8(int id, uint8_t *dst);

/**
 * @brief Sets a configuration value of type uint8_t.
 *
 * @param id The configuration parameter ID.
 * @param val The value to set.
 */
dy_err_t dy_cfg2_set_u8(int id, uint8_t val);

/**
 * @brief Sets a configuration value of type uint8_t if it is not already set.
 *
 * This function will only set the value if the configuration ID does not already have a value set.
 *
 * @param id The configuration parameter ID.
 * @param val The value to set.
 */
dy_err_t dy_cfg2_set_u8_if_not_set(int id, uint8_t val);

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