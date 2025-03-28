#pragma once

#include <stdint.h>
#include "dy/error.h"

/**
 * Pixel.
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} dy_gfx_px_t;

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
    dy_gfx_px_t *content;
} dy_gfx_buf_t;

/**
 * Array of buffers.
 */
typedef struct {
    uint16_t len;
    dy_gfx_buf_t **buffers;
} dy_gfx_buf_array_t;

//    led_strip_set_pixel(cfg->ls, 15, 128, 128, 128); // silver

#define DY_GFX_C_BLACK (dy_gfx_px_t) {.r=0, .g=0, .b=0}
#define DY_GFX_C_RED (dy_gfx_px_t) {.r=255, .g=0, .b=0}
#define DY_GFX_C_GREEN (dy_gfx_px_t) {.r=0, .g=255, .b=0}
#define DY_GFX_C_BLUE (dy_gfx_px_t) {.r=0, .g=0, .b=255}
#define DY_GFX_C_CYAN (dy_gfx_px_t) {.r=0, .g=255, .b=255}
#define DY_GFX_C_MAGENTA (dy_gfx_px_t) {.r=255, .g=0, .b=255}
#define DY_GFX_C_YELLOW (dy_gfx_px_t) {.r=255, .g=255, .b=0}
#define DY_GFX_C_WHITE (dy_gfx_px_t) {.r=255, .g=255, .b=255}

#define DY_GFX_C_ORANGE (dy_gfx_px_t) {.r=255, .g=64, .b=0}
#define DY_GFX_C_PURPLE (dy_gfx_px_t) {.r=128, .g=64, .b=128}
#define DY_GFX_C_PINK (dy_gfx_px_t) {.r=128, .g=105, .b=180}
#define DY_GFX_C_LIME (dy_gfx_px_t) {.r=191, .g=255, .b=0}
#define DY_GFX_C_TEAL (dy_gfx_px_t) {.r=0, .g=128, .b=129}
#define DY_GFX_C_BROWN (dy_gfx_px_t) {.r=129, .g=69, .b=19}
#define DY_GFX_C_GOLD (dy_gfx_px_t) {.r=255, .g=128, .b=0}
#define DY_GFX_C_SILVER (dy_gfx_px_t) {.r=128, .g=128, .b=128}

/**
 * Initializes a buffer.
 */
dy_gfx_buf_t *dy_gfx_make_buf(uint16_t width, uint16_t height);

/**
 * Frees resources allocated by `dy_gfx_make_buf()`.
 */
void dy_gfx_free_buf(dy_gfx_buf_t *buf);

/**
 * Creates an array of buffers.
 */
dy_gfx_buf_array_t *dy_gfx_make_buf_array(uint8_t length, uint16_t width, uint16_t height);

/**
 * Frees resources allocated by `dy_gfx_make_buf_array()`.
 */
void dy_gfx_free_buf_array(dy_gfx_buf_array_t *buf_arr);

/**
 * Fills a buffer with zeroes.
 */
void dy_gfx_clear_buf(dy_gfx_buf_t *buf);

/**
 * Returns linear pixel offset in the buffer.
 */
uint32_t dy_gfx_get_px_pos(const dy_gfx_buf_t *buf, uint16_t x, uint16_t y);

/**
 * Sets buffer pixel's value.
 */
void dy_gfx_set_px(dy_gfx_buf_t *buf, uint16_t x, uint16_t y, dy_gfx_px_t px);

/**
 * Returns buffer pixel's value.
 */
dy_gfx_px_t dy_gfx_get_px(const dy_gfx_buf_t *buf, uint16_t x, uint16_t y);

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
