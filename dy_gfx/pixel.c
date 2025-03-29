#include "dy/gfx/gfx.h"

dy_gfx_px_t dy_gfx_new_px(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) |  // Top 5 bits of red
           ((g & 0xFC) << 3) |  // Top 6 bits of green
           (b >> 3);            // Top 5 bits of blue
}

dy_gfx_rgb888_t dy_gfx_px_to_rgb888(dy_gfx_px_t px) {
    // Extract bits
    uint8_t r5 = (px >> 11) & 0x1F;
    uint8_t g6 = (px >> 5) & 0x3F;
    uint8_t b5 = px & 0x1F;

    return (dy_gfx_rgb888_t) {
            .r = (r5 << 3) | (r5 >> 2), // 5-bit to 8-bit
            .g = (g6 << 2) | (g6 >> 4), // 6-bit to 8-bit
            .b = (b5 << 3) | (b5 >> 2), // 5-bit to 8-bit
    };
}
