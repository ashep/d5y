#include <stdlib.h>

#include "dy/gfx/geometry.h"

void dy_gfx_line(dy_gfx_buf_t *buf, const dy_gfx_line_t *line, dy_gfx_px_t color) {
    uint16_t x1 = line->p1.x, x2 = line->p2.x, y1 = line->p1.y, y2 = line->p2.y;

    if (x2 >= buf->width) {
        x2 = buf->width - 1;
    }
    if (y2 >= buf->height) {
        y2 = buf->height - 1;
    }

    float x = x1, y = y1, dx = x2 - x1, dy = y2 - y1;
    uint16_t step, abs_dx = abs(dx), abs_dy = abs(dy);

    step = abs_dx >= abs_dy ? abs_dx : abs_dy;
    dx /= step;
    dy /= step;

    uint16_t i = 0;
    while (i <= step) {
        if (i == step) {
            if (x < x2) {
                x = x2;
            }
            if (y < y2) {
                y = y2;
            }
        }

        dy_gfx_set_px(buf, abs(x), abs(y), color);

        x += dx;
        y += dy;

        ++i;
    }
}

void dy_gfx_poly(dy_gfx_buf_t *buf, const dy_gfx_poly_t *poly, dy_gfx_px_t color) {
    dy_gfx_line_t l;

    for (uint8_t i = 0; i < poly->n_corners; i++) {
        if (i < poly->n_corners - 1) {
            l = (dy_gfx_line_t) {poly->edges[i], poly->edges[i + 1]};
        } else {
            l = (dy_gfx_line_t) {poly->edges[i], poly->edges[0]};
        }

        dy_gfx_line(buf, &l, color);
    }
}

void dy_gfx_rect(dy_gfx_buf_t *buf, dy_gfx_point_t p1, dy_gfx_point_t p2, dy_gfx_px_t color) {
    dy_gfx_point_t points[4] = {
            p1,
            (dy_gfx_point_t) {p2.x, p1.y},
            p2,
            (dy_gfx_point_t) {p1.x, p2.y},
    };

    dy_gfx_poly(buf, &((dy_gfx_poly_t) {4, points}), color);
}

void dy_gfx_tri(dy_gfx_buf_t *buf, dy_gfx_point_t p1, dy_gfx_point_t p2, dy_gfx_point_t p3, dy_gfx_px_t color) {
    dy_gfx_point_t points[3] = {p1, p2, p3};
    dy_gfx_poly(buf, &((dy_gfx_poly_t) {3, points}), color);
}
