/**
 * Author: Oleksandr Shepetko <a@shepetko.com>
 * License: MIT
 */

#ifndef DY_GFX_DRAW_H
#define DY_GFX_DRAW_H

#include "dy/gfx/buffer.h"
#include "dy/gfx/geometry.h"

/**
 * Draws a line
 */
void dy_gfx_line(dy_gfx_buf_t *buf, const dy_gfx_line_t *line, uint32_t color);

/**
 * Draws a polyline.
 */
void dy_gfx_poly(dy_gfx_buf_t *buf, const dy_gfx_poly_t *poly, uint32_t color);

/**
 * Draws a rectangle.
 */
void dy_gfx_rect(dy_gfx_buf_t *buf, dy_gfx_point_t p1, dy_gfx_point_t p2, uint32_t color);

/**
 * Draws a triangle.
 */
void dy_gfx_tri(
    dy_gfx_buf_t *buf,
    const dy_gfx_point_t p1,
    const dy_gfx_point_t p2,
    const dy_gfx_point_t p3,
    uint32_t color
);

#endif // DY_GFX_DRAW_H
