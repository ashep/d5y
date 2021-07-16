/**
 * @brief AESPL Clock Font 1
 *
 * @author    Alexnader Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#ifndef CRONUS_FONT_8_1_H
#define CRONUS_FONT_8_1_H

#include "aespl_gfx_text.h"

static const uint8_t _font8_clock_1[] =
        {
                1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,  // . - 46
                4, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80,  // / - 47
                5, 0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70,  // 0 - 48
                5, 0x20, 0x60, 0xA0, 0x20, 0x20, 0x20, 0x20, 0xF8,  // 1 - 49
                5, 0x70, 0x88, 0x08, 0x08, 0x70, 0x80, 0x80, 0xF8,  // 2 - 50
                5, 0x70, 0x88, 0x08, 0x30, 0x08, 0x08, 0x88, 0x70,  // 3 - 51
                5, 0x88, 0x88, 0x88, 0x88, 0xF8, 0x08, 0x08, 0x08,  // 4 - 52
                5, 0xF8, 0x80, 0x80, 0xF0, 0x08, 0x08, 0x08, 0xF0,  // 5 - 53
                5, 0x70, 0x88, 0x80, 0xF0, 0x88, 0x88, 0x88, 0x70,  // 6 - 54
                5, 0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0x80, 0x80,  // 7 - 55
                5, 0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x88, 0x70,  // 8 - 56
                5, 0x70, 0x88, 0x88, 0x88, 0x78, 0x08, 0x88, 0x70,  // 9 - 57
                1, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00,  // : - 58
        };

aespl_gfx_font_t font8_clock_1 = {
        .ascii_offset = 46,
        .length = 13,
        .width = AESPL_GFX_FONT_WIDTH_8,
        .height = 8,
        .content.c8 = _font8_clock_1,
};

#endif // CRONUS_FONT_8_1_H
