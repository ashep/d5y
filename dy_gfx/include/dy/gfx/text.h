#pragma once

#include <stdio.h>
#include "dy/gfx/gfx.h"

/**
 * Font widths.
 */
typedef enum {
    DY_GFX_FONT_SYMBOL_WIDTH_8 = 8,   // eight bits per row
    DY_GFX_FONT_SYMBOL_WIDTH_16 = 16, // sixteen bits per row
} dy_gfx_font_width_t;

/**
 * Font.
 */
typedef struct {
    uint8_t ascii_offset;  // char code offset relative to ASCII table
    uint8_t length;        // number of covered ASCII codes staring from ascii_offset
    uint8_t width;         // number of bits per row
    uint8_t height;        // number of rows per character
    union {
        const uint8_t *c8;   // pointer to 1-byte content
        const uint16_t *c16; // pointer to 2-byte content
    } content;
} dy_gfx_font_t;

/**
 * Draws a character.
 */
int8_t dy_gfx_putc(dy_gfx_buf_t *buf, const dy_gfx_font_t *font, dy_gfx_point_t pos, uint8_t ch, dy_gfx_px_t color);

/**
 * @brief Draws a string.
 */
dy_gfx_point_t dy_gfx_puts(dy_gfx_buf_t *buf, const dy_gfx_font_t *font, dy_gfx_point_t pos, const char *s,
                           dy_gfx_px_t color, uint8_t space);

/**
 * Returns width of a character.
 */
int8_t dy_gfx_ch_width(const dy_gfx_font_t *font, char ch);

/**
 * Get width of a string.
 */
int16_t dy_gfx_str_width(const dy_gfx_font_t *font, const char *str, uint8_t space);

/**
 * @brief Creates a graphics buffer and put a string into it.
 */
dy_gfx_buf_t *dy_gfx_make_str_buf(const dy_gfx_font_t *font, const char *str, dy_gfx_px_t color, uint8_t space);
