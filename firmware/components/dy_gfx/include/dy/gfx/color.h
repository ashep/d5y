/**
 * @author    Oleksandr Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#ifndef DY_GFX_COLOR_H
#define DY_GFX_COLOR_H

#include <stdio.h>

/**
 * Color modes.
 */
typedef enum {
    DY_GFX_C_MODE_MONO,
    DY_GFX_C_MODE_RGB565,
    DY_GFX_C_MODE_ARGB888,
} dy_gfx_c_mode_t;

/**
 * @brief Makes an RGB565 value from separate R, G and B values.
 */
uint16_t dy_gfx_make_rgb565(uint8_t r, uint8_t g, uint8_t b);

#endif // DY_GFX_COLOR_H