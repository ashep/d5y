/**
 * Author: Oleksandr Shepetko <a@shepetko.com>
 * License: MIT
 */

#ifndef DY_DISPLAY
#define DY_DISPLAY

#include "dy/error.h"
#include "dy/gfx/gfx.h"
#include "dy/max7219.h"

#define DY_DISPLAY_ID_MAX 4

typedef dy_err_t (*display_drv_write_t)(void *cfg, dy_gfx_buf_t *buf);

typedef dy_err_t (*display_drv_set_brightness_t)(void *cfg, uint8_t value);

typedef dy_err_t (*display_drv_refresh_t)(void *cfg);

typedef struct {
    void *cfg;
    display_drv_write_t write;
    display_drv_set_brightness_t set_brightness;
    display_drv_refresh_t refresh;
} dy_display_driver_t;

/**
 * Initializes MAX7219 display driver.
 */
dy_err_t dy_display_init_driver_max7219(uint8_t id, dy_max7219_config_t *cfg);

/**
 * Outputs a buffer to the device.
 */
dy_err_t dy_display_write(uint8_t id, dy_gfx_buf_t *buf);

/**
 * Sets display brightness.
 */
dy_err_t dy_display_set_brightness(uint8_t id, uint8_t value);

/**
 * Refreshes display.
 */
dy_err_t dy_display_refresh(uint8_t id);

#endif // DY_DISPLAY
