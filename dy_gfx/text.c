#include "dy/gfx/text.h"

int8_t dy_gfx_putc(dy_gfx_buf_t *buf, const dy_gfx_font_t *font, dy_gfx_point_t pos, uint8_t ch, dy_gfx_px_t color) {
    int8_t ch_width = 0;

    // Check if the character is covered by the font
    if (ch - font->ascii_offset + 1 > font->length) {
        return DY_ERR_INVALID_ARG;
    }

    // Offset from the beginning of the font content
    uint16_t offset = (ch - font->ascii_offset) * (font->height + 1);

    // Locate character's position and determine its actual width from first byte/word
    void *ch_p = NULL;
    switch (font->width) {
        case DY_GFX_FONT_SYMBOL_WIDTH_8:
            ch_p = (uint8_t *) &font->content.c8[offset];
            ch_width = *(int8_t *) ch_p++;
            break;
        case DY_GFX_FONT_SYMBOL_WIDTH_16:
            ch_p = (uint16_t *) &font->content.c16[offset];
            ch_width = *(int16_t *) ch_p++;
            break;
    }

    // Each row
    for (uint8_t row_n = 0; row_n < font->height; row_n++) {
        uint64_t row;

        switch (font->width) {
            case DY_GFX_FONT_SYMBOL_WIDTH_8:
                row = *(uint8_t *) ch_p++;
                break;
            case DY_GFX_FONT_SYMBOL_WIDTH_16:
                row = *(uint16_t *) ch_p++;
                break;
            default:
                return DY_ERR_INVALID_ARG;
        }

        // Each column
        for (int8_t n = 0, col_n = font->width - 1; n < ch_width; n++, col_n--) {
            if (1 & (row >> col_n)) {  // if pixel is set
                dy_gfx_set_px(buf, pos.x + n, pos.y + row_n, color);
            }
        }
    }

    return ch_width;
}

dy_gfx_point_t dy_gfx_puts(
        dy_gfx_buf_t *buf,
        const dy_gfx_font_t *font,
        dy_gfx_point_t pos,
        const char *s,
        dy_gfx_px_t color,
        uint8_t space
) {
    int8_t ch_width;

    while (*s) {
        ch_width = dy_gfx_putc(buf, font, pos, *s++, color);
        if (ch_width < 0) {
            return pos;
        }
        pos.x += ch_width + space;
    }

    return pos;
}

int8_t dy_gfx_ch_width(const dy_gfx_font_t *font, char ch) {
    // Check if the character is covered by the font
    if (ch - font->ascii_offset + 1 > font->length) {
        return DY_ERR_INVALID_ARG;
    }

    // Offset from the beginning of the font content
    uint16_t offset = (ch - font->ascii_offset) * (font->height + 1);

    switch (font->width) {
        case DY_GFX_FONT_SYMBOL_WIDTH_8:
            return font->content.c8[offset];
        case DY_GFX_FONT_SYMBOL_WIDTH_16:
            return font->content.c16[offset];
    }

    return DY_ERR_UNKNOWN;
}

int16_t dy_gfx_str_width(const dy_gfx_font_t *font, const char *str, uint8_t space) {
    int16_t w = 0, ch_w = 0;
    char *c = (char *) str;

    while (*c) {
        ch_w = dy_gfx_ch_width(font, *c++);
        if (ch_w < 0) {
            return ch_w;
        }
        w += ch_w + space;
    }

    return w;
}

dy_gfx_buf_t *dy_gfx_make_str_buf(const dy_gfx_font_t *font, const char *str, dy_gfx_px_t color, uint8_t space) {
    // Calculate buffer's width
    int16_t str_w = dy_gfx_str_width(font, str, space);
    if (str_w < 0) {
        return NULL;
    }

    // Create a buffer
    dy_gfx_buf_t *buf = dy_gfx_new_buf(str_w, font->height);
    if (!buf) {
        return NULL;
    }

    // Draw the string
    dy_gfx_puts(buf, font, (dy_gfx_point_t) {0, 0}, str, color, space);

    return buf;
}
