#include "dy/gfx/gfx.h"

dy_gfx_px_t dy_gfx_new_px(uint8_t r, uint8_t g, uint8_t b) {
    return (dy_gfx_px_t) {.r=r, .g=g, .b=b};
}
