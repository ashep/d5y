#pragma once

#include <stdint.h>
#include "dy/error.h"

/**
 * @brief Initializes the dy_cfg2 module.
 *
 * @warning Ensure to call nvs_flash_init() before calling this function.
 * @return
 *  - DY_ERR_FAILED if initialization fails.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_cfg2_init();

/**
 * @brief Sets a configuration value of type uint8_t.
 *
 * @param id The configuration ID.
 * @param val The value to set.
 * @return
 *  - DY_ERR_NOT_CONFIGURED if the module is not initialized.
 *  - DY_ERR_FAILED in case of other unexpected errors.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_cfg2_set_u8(uint16_t id, uint8_t val);

/**
 * @brief Sets a configuration value of type int8_t.
 *
 * @param id The configuration ID.
 * @param val The value to set.
 * @return
 *  - DY_ERR_NOT_CONFIGURED if the module is not initialized.
 *  - DY_ERR_FAILED in case of other unexpected errors.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_cfg2_set_i8(uint16_t id, int8_t val);

/**
 * @brief Gets a configuration value of type uint8_t.
 *
 * @param id The configuration ID.
 * @param dst Pointer to store the retrieved value.
 * @return
 *  - DY_ERR_INVALID_ARG if dst is NULL.
 *  - DY_ERR_NOT_CONFIGURED if the module is not initialized.
 *  - DY_ERR_NOT_FOUND if the key does not exist.
 *  - DY_ERR_FAILED in case of other unexpected errors.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_cfg2_get_u8(uint16_t id, uint8_t *dst);

/**
 * @brief Gets a configuration value of type int8_t.
 *
 * @param id The configuration ID.
 * @param dst Pointer to store the retrieved value.
 * @return
 *  - DY_ERR_INVALID_ARG if dst is NULL.
 *  - DY_ERR_NOT_CONFIGURED if the module is not initialized.
 *  - DY_ERR_NOT_FOUND if the key does not exist.
 *  - DY_ERR_FAILED in case of other unexpected errors.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_cfg2_get_i8(uint16_t id, int8_t *dst);
