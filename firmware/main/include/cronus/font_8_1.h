/**
 * @brief AESPL Clock Font 2
 *
 * @author    Alexnader Shepetko <a@shepetko.com>
 * @copyright https://www.dafont.com/computer-pixel-7.font
 */

#ifndef CRONUS_FONT_8_1_H
#define CRONUS_FONT_8_1_H

#include "aespl_gfx_text.h"

static const uint8_t fnt[] =
        {
                1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //   - 32 (whitespace)
                8, 0x44, 0x4A, 0xE4, 0x40, 0x40, 0x48, 0x48, 0x30,  // ! - 33 (thermometer sign)
                3, 0xA0, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // " - 34
                8, 0x10, 0x28, 0x44, 0x82, 0x54, 0x44, 0x44, 0x7C,  // # - 35 (home sign)
                7, 0x00, 0x10, 0x54, 0x38, 0xEE, 0x38, 0x54, 0x10,  // $ - 36 (brightness sign)
                8, 0xE4, 0xA8, 0xE8, 0x10, 0x30, 0x37, 0x65, 0x67,  // % - 37
                8, 0x18, 0x24, 0x42, 0x42, 0x42, 0x81, 0xFF, 0x18,  // & - 38 (transparent bell)
                8, 0x18, 0x3C, 0x7E, 0x7E, 0x7E, 0xFF, 0xFF, 0x18,  // ' - 39 (opaque bell)
                3, 0x60, 0x80, 0x80, 0x80, 0xC0, 0xC0, 0xC0, 0x60,  // ( - 40
                3, 0xC0, 0x20, 0x20, 0x20, 0x60, 0x60, 0x60, 0xC0,  // ) - 41
                5, 0x00, 0x00, 0x50, 0x20, 0xF8, 0x20, 0x50, 0x00,  // * - 42
                5, 0x00, 0x00, 0x20, 0x20, 0xF8, 0x20, 0x20, 0x00,  // + - 43
                3, 0x40, 0xA0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,  // , - 44 (degree sign)
                4, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00,  // - - 45
                2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,  // . - 46
                4, 0x10, 0x10, 0x20, 0x20, 0x60, 0x60, 0xC0, 0xC0,  // / - 47
                6, 0x78, 0x8C, 0x8C, 0x8C, 0xC4, 0xC4, 0xC4, 0x78,  // 0 - 48
                6, 0x70, 0x10, 0x10, 0x10, 0x30, 0x30, 0x30, 0xFC,  // 1 - 49
                6, 0xF8, 0x04, 0x04, 0x04, 0x78, 0xC0, 0xC0, 0xFC,  // 2 - 50
                6, 0xF8, 0x04, 0x04, 0x78, 0x0C, 0x0C, 0x0C, 0xF8,  // 3 - 51
                6, 0x84, 0x84, 0x84, 0x84, 0xFC, 0x0C, 0x0C, 0x0C,  // 4 - 52
                6, 0xFC, 0x80, 0x80, 0xF8, 0x0C, 0x0C, 0x0C, 0xF8,  // 5 - 53
                6, 0x78, 0x80, 0x80, 0xF8, 0xC4, 0xC4, 0xC4, 0x78,  // 6 - 54
                6, 0xFC, 0x04, 0x04, 0x04, 0x0C, 0x0C, 0x0C, 0x0C,  // 7 - 55
                6, 0x78, 0x84, 0x84, 0x78, 0xCC, 0xCC, 0xCC, 0x78,  // 8 - 56
                6, 0x78, 0x84, 0x84, 0x84, 0x7C, 0x0C, 0x0C, 0x78,  // 9 - 57
                2, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xC0, 0x00, 0x00,  // : - 58
        };

aespl_gfx_font_t font8_clock_2 = {
        .ascii_offset = 32,
        .length = 27,
        .width = AESPL_GFX_FONT_WIDTH_8,
        .height = 8,
        .content.c8 = fnt,
};

#endif // CRONUS_FONT_8_1_H
