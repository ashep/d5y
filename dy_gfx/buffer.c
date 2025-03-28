#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"

#include "dy/error.h"
#include "dy/gfx/gfx.h"

static void print_bin(uint32_t v, uint8_t width) {
    for (uint8_t i = width; i > 0; i--) {
        printf("%lu", (v >> (i - 1)) & 1);
    }
}

dy_gfx_buf_t *dy_gfx_make_buf(uint16_t width, uint16_t height) {
    dy_gfx_buf_t *buf = malloc(sizeof(dy_gfx_buf_t));
    if (!buf) {
        return NULL;
    }

    buf->width = width;
    buf->height = height;

    buf->content = calloc(width * height, sizeof(dy_gfx_px_t));
    if (!buf->content) {
        free(buf);
        return NULL;
    }

    return buf;
}

void dy_gfx_free_buf(dy_gfx_buf_t *buf) {
    free(buf->content);
    free(buf);
}

dy_gfx_buf_array_t *dy_gfx_make_buf_array(uint8_t length, uint16_t width, uint16_t height) {
    dy_gfx_buf_array_t *buf_arr = malloc(sizeof(dy_gfx_buf_array_t));
    if (!buf_arr) {
        return NULL;
    }

    buf_arr->len = length;
    buf_arr->buffers = calloc(length, sizeof(dy_gfx_buf_t *));

    for (uint8_t i = 0; i < length; i++) {
        dy_gfx_buf_t *b = dy_gfx_make_buf(width, height);
        if (!b) {
            for (uint8_t k = 0; k < i; k++) {
                free(buf_arr->buffers[k]);
            }
            dy_gfx_free_buf(b);
            free(buf_arr);
            return NULL;
        }

        buf_arr->buffers[i] = b;
    }

    return buf_arr;
}

void dy_gfx_free_buf_array(dy_gfx_buf_array_t *buf_arr) {
    for (uint16_t i = 0; i < buf_arr->len; i++) {
        dy_gfx_free_buf(buf_arr->buffers[i]);
    }

    free(buf_arr->buffers);
    free(buf_arr);
}

void dy_gfx_clear_buf(dy_gfx_buf_t *buf) {
    if (buf->content == NULL) {
        return;
    }
    memset(buf->content, 0, buf->width * buf->height);
}

uint32_t dy_gfx_get_px_pos(const dy_gfx_buf_t *buf, uint16_t x, uint16_t y) {
    return y * buf->width + x;
}

void dy_gfx_set_px(dy_gfx_buf_t *buf, uint16_t x, uint16_t y, dy_gfx_px_t px) {
    // It's okay to set a pixel outside buffer's boundaries
    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        return;
    }

    buf->content[dy_gfx_get_px_pos(buf, x, y)] = px;
}

dy_gfx_px_t dy_gfx_get_px(const dy_gfx_buf_t *buf, uint16_t x, uint16_t y) {
    // It's okay to get a pixel outside buffer's boundaries
    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        return (dy_gfx_px_t) {};
    }

    return buf->content[dy_gfx_get_px_pos(buf, x, y)];
}

dy_err_code_t dy_gfx_merge(dy_gfx_buf_t *dst, const dy_gfx_buf_t *src, dy_gfx_point_t d_pos, dy_gfx_point_t s_pos) {
    if (s_pos.x >= src->width || s_pos.y >= src->height ||
        d_pos.x >= dst->width || d_pos.y >= dst->height) {
        return DY_ERR_INVALID_ARG;
    }

    int32_t dst_x = d_pos.x;
    for (int32_t src_x = s_pos.x; src_x < src->width && dst_x < dst->width; src_x++, dst_x++) {
        int32_t dst_y = d_pos.y;
        for (int32_t src_y = s_pos.y; src_y < src->height && dst_y < dst->height; src_y++, dst_y++) {
            dy_gfx_set_px(dst, dst_x, dst_y, dy_gfx_get_px(src, src_x, src_y));
        }
    }

    return DY_OK;
}

dy_gfx_buf_array_t *dy_gfx_split(const dy_gfx_buf_t *src, uint8_t chunk_w, uint8_t chunk_h) {
    uint8_t nx = src->width / chunk_w;
    uint8_t ny = src->height / chunk_h;

    dy_gfx_buf_array_t *dst = dy_gfx_make_buf_array(nx * ny, chunk_w, chunk_h);
    if (!dst) {
        return NULL;
    }

    uint8_t i = 0;
    for (uint8_t n_y = 0; n_y < ny; n_y++) {
        for (uint8_t n_x = 0; n_x < nx; n_x++) {
            dy_gfx_point_t src_pos = {n_x * chunk_w, n_y * chunk_h};
            if (dy_gfx_merge(dst->buffers[i], src, (dy_gfx_point_t) {0, 0}, src_pos) != DY_OK) {
                dy_gfx_free_buf_array(dst);
                return NULL;
            }
            i++;
        }
    }

    return dst;
}

dy_err_code_t dy_gfx_move(dy_gfx_buf_t *buf, dy_gfx_point_t pos) {
    dy_err_code_t err;
    dy_gfx_buf_t *tmp_buf = dy_gfx_make_buf(buf->width, buf->height);
    if (!tmp_buf) {
        return DY_ERR_UNKNOWN;
    }

    err = dy_gfx_merge(tmp_buf, buf, pos, (dy_gfx_point_t) {0, 0});
    if (err) {
        return err;
    }

    dy_gfx_clear_buf(buf);

    err = dy_gfx_merge(buf, tmp_buf, (dy_gfx_point_t) {0, 0}, (dy_gfx_point_t) {0, 0});
    if (err) {
        return err;
    }

    dy_gfx_free_buf(tmp_buf);

    return DY_OK;
}
