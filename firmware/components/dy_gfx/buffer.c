#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dy/error.h"
#include "dy/gfx/buffer.h"
#include "dy/gfx/geometry.h"
#include "dy/gfx/color.h"

void print_bin(uint32_t v, uint8_t width) {
    for (uint8_t i = width; i > 0; i--) {
        printf("%lu", (v >> (i - 1)) & 1);
    }
}

dy_gfx_buf_t *dy_gfx_make_buf(uint16_t width, uint16_t height, dy_gfx_c_mode_t c_mode) {
    dy_gfx_buf_t *buf = malloc(sizeof(dy_gfx_buf_t));
    if (!buf) {
        return NULL;
    }
    buf->width = width;
    buf->height = height;
    buf->c_mode = c_mode;

    buf->content = calloc(height, sizeof(buf->content));  // pointers to rows
    if (!buf->content) {
        free(buf);
        return NULL;
    }

    // Pixels per row
    switch (c_mode) {
        case DY_GFX_C_MODE_MONO:
            buf->ppw = sizeof(**buf->content) * 8;  // 8 pixels per word
            break;
        case DY_GFX_C_MODE_RGB565:
            buf->ppw = sizeof(**buf->content) * 8 / 16;  // 2 pixels per word
            break;
        case DY_GFX_C_MODE_ARGB888:
            buf->ppw = 1;  // 1 pixel per word
            break;
    }

    // Words per row
    buf->wpr = 1 + ((width - 1) / buf->ppw);

    // Allocate memory for content
    for (uint16_t r = 0; r < height; r++) {
        // Allocate memory for each row
        buf->content[r] = calloc(buf->wpr, sizeof(*buf->content));
        if (!buf->content[r]) {
            free(buf->content);
            free(buf);
            return NULL;
        }
    }

    // Fill buffer with zeros
    dy_gfx_clear_buf(buf);

    return buf;
}

void dy_gfx_free_buf(dy_gfx_buf_t *buf) {
    for (uint16_t r = 0; r < buf->height; r++) {
        free(buf->content[r]);
    }

    free(buf->content);
    free(buf);
}

dy_gfx_buf_array_t *dy_gfx_make_buf_array(uint8_t length, uint16_t width, uint16_t height, dy_gfx_c_mode_t c_mode) {
    dy_gfx_buf_array_t *buf_arr = malloc(sizeof(dy_gfx_buf_array_t));
    if (!buf_arr) {
        return NULL;
    }

    buf_arr->length = length;
    buf_arr->c_mode = c_mode;
    buf_arr->buffers = calloc(length, sizeof(dy_gfx_buf_t *));

    for (uint8_t i = 0; i < length; i++) {
        buf_arr->buffers[i] = dy_gfx_make_buf(width, height, c_mode);
    }

    return buf_arr;
}

void dy_gfx_free_buf_array(dy_gfx_buf_array_t *buf_arr) {
    for (uint16_t i = 0; i < buf_arr->length; i++) {
        dy_gfx_free_buf(buf_arr->buffers[i]);
    }

    free(buf_arr->buffers);
    free(buf_arr);
}

void dy_gfx_clear_buf(dy_gfx_buf_t *buf) {
    if (buf->content == NULL) {
        return;
    }

    for (uint16_t r = 0; r < buf->height; r++) {
        if (buf->content[r] == NULL) {
            continue;
        }

        memset(buf->content[r], 0, buf->wpr * sizeof(**buf->content));
    }
}

void dy_gfx_dump_buf(const dy_gfx_buf_t *buf) {
    for (uint16_t r = 0; r < buf->height; r++) {
        for (uint8_t w = buf->wpr; w > 0; w--) {
            if (w == buf->wpr) {
                printf("[%4u] ", r);
            }

            print_bin(buf->content[r][w - 1], sizeof(buf->content) * 8);

            if (w == 1) {
                printf(" ");
                for (uint16_t w2 = buf->wpr; w2 > 0; w2--) {
                    printf("%lu", buf->content[r][w2 - 1]);
                    if (w2 == 1)
                        printf("\n");
                    else
                        printf("|");
                }
            } else {
                printf("|");
            }
        }
    }
}

void dy_gfx_set_px(dy_gfx_buf_t *buf, int16_t x, int16_t y, uint32_t color) {
    // It's okay to set a pixel outside buffer's boundaries
    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        return;
    }

    size_t word_bits = sizeof(**buf->content) * 8;
    uint16_t word_n = buf->wpr - 1 - x / buf->ppw;

    switch (buf->c_mode) {
        case DY_GFX_C_MODE_MONO:
            if (color == 0) {
                buf->content[y][word_n] &= ~(1 << (word_bits - x - 1 % word_bits));
            } else {
                buf->content[y][word_n] |= 1 << (word_bits - x - 1 % word_bits);
            }
            break;

        case DY_GFX_C_MODE_RGB565:
            buf->content[y][word_n] |= color << ((x % 2) ? 0 : 16);
            break;

        case DY_GFX_C_MODE_ARGB888:
            buf->content[y][word_n] = color;
            break;
    }
}

uint32_t dy_gfx_get_px(const dy_gfx_buf_t *buf, int16_t x, int16_t y) {
    // It's okay to get a pixel outside buffer's boundaries
    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        return 0x0;
    }

    size_t word_bits = sizeof(**buf->content) * 8;
    uint16_t word_n = buf->wpr - 1 - x / buf->ppw;
    uint32_t w = buf->content[y][word_n];

    switch (buf->c_mode) {
        case DY_GFX_C_MODE_MONO:
            return 1 & (w >> (word_bits - x - 1 % word_bits));

        case DY_GFX_C_MODE_RGB565:
            return 0xffff & (w >> ((x % 2) ? 0 : 16));

        case DY_GFX_C_MODE_ARGB888:
            return w;
    }

    return 0x0;
}

dy_err_code_t dy_gfx_merge(dy_gfx_buf_t *dst, const dy_gfx_buf_t *src, dy_gfx_point_t d_pos, dy_gfx_point_t s_pos) {
    if (s_pos.x >= src->width || s_pos.y >= src->height ||
        d_pos.x >= dst->width || d_pos.y >= dst->height) {
        return DY_ERR_INVALID_ARG;
    }

    int16_t dst_x = d_pos.x;
    for (int16_t src_x = s_pos.x; src_x < src->width && dst_x < dst->width; src_x++, dst_x++) {
        int16_t dst_y = d_pos.y;
        for (int16_t src_y = s_pos.y; src_y < src->height && dst_y < dst->height; src_y++, dst_y++) {
            dy_gfx_set_px(dst, dst_x, dst_y, dy_gfx_get_px(src, src_x, src_y));
        }
    }

    return DY_OK;
}

dy_gfx_buf_array_t *dy_gfx_split(const dy_gfx_buf_t *src, uint8_t chunk_w, uint8_t chunk_h) {
    uint8_t nx = src->width / chunk_w;
    uint8_t ny = src->height / chunk_h;

    dy_gfx_buf_array_t *dst = dy_gfx_make_buf_array(nx * ny, chunk_w, chunk_h, src->c_mode);
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
    dy_gfx_buf_t *tmp_buf = dy_gfx_make_buf(buf->width, buf->height, buf->c_mode);
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
