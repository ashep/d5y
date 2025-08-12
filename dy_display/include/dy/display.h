/**
 * @brief Display driver interface.
 * @warning It is NOT thread-safe.
 */

#pragma once

#include "dy/error.h"
#include "dy/gfx/gfx.h"

#define DY_DISPLAY_ID_MAX 4
#define DY_DISPLAY_BRI_MAX 15

/**
 * @brief Display driver configuration structure.
 */
typedef struct {
    void *cfg;
    dy_err_t (*write)(void *cfg, dy_gfx_buf_t *buf);
    dy_err_t (*set_brightness)(void *cfg, uint8_t value);
    dy_err_t (*refresh)(void *cfg);
} dy_display_driver_t;

dy_err_t dy_display_set_driver(uint8_t id, dy_display_driver_t drv);

/**
 * @brief Writes a graphics buffer to the display.
 */
dy_err_t dy_display_write(uint8_t id, dy_gfx_buf_t *buf);

/**
 * @brief Sets the brightness of the display.
 */
dy_err_t dy_display_set_brightness(uint8_t id, uint8_t value);

/**
 * @brief Gets the current brightness value of the display.
 * @return Brightness value, 0 if the ID is invalid.
 */
uint8_t dy_display_get_brightness(uint8_t id);

/**
 * @brief Refreshes the display.
 */
dy_err_t dy_display_refresh(uint8_t id);
