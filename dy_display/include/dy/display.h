#pragma once

#include "dy/error.h"
#include "dy/gfx/gfx.h"

#define DY_DISPLAY_ID_MAX 4

#define DY_DISPLAY_BRI_MAX 15

typedef struct {
    void *cfg;
    dy_err_t (*write)(void *cfg, dy_gfx_buf_t *buf);
    dy_err_t (*set_brightness)(void *cfg, uint8_t value);
    dy_err_t (*refresh)(void *cfg);
} dy_display_driver_t;

dy_err_t dy_display_set_driver(uint8_t id, dy_display_driver_t drv);

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
