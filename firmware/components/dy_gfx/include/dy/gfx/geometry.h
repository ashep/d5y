/**
 * @author    Oleksandr Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#ifndef DY_GFX_GEOMETRY_H
#define DY_GFX_GEOMETRY_H

#include <stdio.h>

/**
 * Point.
 */
typedef struct {
    int32_t x;
    int32_t y;
} dy_gfx_point_t;

/**
 * Line.
 */
typedef struct {
    dy_gfx_point_t p1;
    dy_gfx_point_t p2;
} dy_gfx_line_t;

/**
 * Polygon.
 */
typedef struct {
    uint8_t n_corners;
    dy_gfx_point_t *edges;
} dy_gfx_poly_t;

#endif // DY_GFX_GEOMETRY_H
