#pragma once

#include <stdio.h>
#include "dy/gfx/gfx.h"

typedef struct {
    const uint16_t width;
    const uint16_t height;
    const dy_gfx_px_t *data;
} dy_gfx_sprite_t;

dy_err_t dy_gfx_write_sprite(dy_gfx_buf_t *buf, uint16_t bx, uint16_t by, const dy_gfx_sprite_t *sp);