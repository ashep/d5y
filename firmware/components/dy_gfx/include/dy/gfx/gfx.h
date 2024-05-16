/**
 * Author: Oleksandr Shepetko <a@shepetko.com>
 * License: MIT
 */

#ifndef DY_GFX_H
#define DY_GFX_H

#include <stdint.h>
#include "dy/error.h"

/**
 * Color modes.
 */
typedef enum {
    DY_GFX_COLOR_MONO,
    DY_GFX_COLOR_RGB565,
    DY_GFX_COLOR_ARGB888,
} dy_gfx_color_mode_t;

/**
 * Point.
 */
typedef struct {
    int32_t x;
    int32_t y;
} dy_gfx_point_t;

/**
 * Buffer.
 */
typedef struct {
    uint16_t width;
    uint16_t height;
    dy_gfx_color_mode_t c_mode;
    uint8_t ppw;             // pixels per word
    uint8_t wpr;             // words per row
    uint32_t **content;
} dy_gfx_buf_t;

/**
 * Array of buffers.
 */
typedef struct {
    uint16_t length;         // number of buffers
    dy_gfx_color_mode_t c_mode;  // color mode
    dy_gfx_buf_t **buffers;  // buffers
} dy_gfx_buf_array_t;

/**
 * Makes an RGB565 value from separate R, G and B values.
 */
uint16_t dy_gfx_make_rgb565(uint8_t r, uint8_t g, uint8_t b);

/**
 * Initializes a buffer.
 */
dy_gfx_buf_t *dy_gfx_make_buf(uint16_t width, uint16_t height, dy_gfx_color_mode_t c_mode);

/**
 * Frees resources allocated by `dy_gfx_make_buf()`.
 */
void dy_gfx_free_buf(dy_gfx_buf_t *buf);

/**
 * Creates an array of buffers.
 */
dy_gfx_buf_array_t *dy_gfx_make_buf_array(uint8_t length, uint16_t width, uint16_t height, dy_gfx_color_mode_t c_mode);

/**
 * Frees resources allocated by `dy_gfx_make_buf_array()`.
 */
void dy_gfx_free_buf_array(dy_gfx_buf_array_t *buf_arr);

/**
 * Fills a buffer with zeroes.
 */
void dy_gfx_clear_buf(dy_gfx_buf_t *buf);

/**
 * Dumps buffer's content to the stdout.
 */
void dy_gfx_dump_buf(const dy_gfx_buf_t *buf);

/**
 * Sets buffer pixel's value.
 */
void dy_gfx_set_px(dy_gfx_buf_t *buf, int16_t x, int16_t y, uint32_t color);

/**
 * Returns buffer pixel's value.
 */
uint32_t dy_gfx_get_px(const dy_gfx_buf_t *buf, int16_t x, int16_t y);

/**
 * Merges two buffers.
 */
dy_err_code_t dy_gfx_merge(dy_gfx_buf_t *dst, const dy_gfx_buf_t *src, dy_gfx_point_t d_pos, dy_gfx_point_t s_pos);

/**
 * Splits a buffer,
 */
dy_gfx_buf_array_t *dy_gfx_split(const dy_gfx_buf_t *src, uint8_t chunk_w, uint8_t chunk_h);

/**
 * @brief Moves buffer's content to a new position.
 *
 * @param buf  A buffer.
 * @param pos  A new position relative to the current one.
 *
 * @return Result of the operation.
 */
dy_err_code_t dy_gfx_move(dy_gfx_buf_t *buf, dy_gfx_point_t pos);

#endif // DY_GFX_H
