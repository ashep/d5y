#include "esp_log.h"
#include "led_strip_types.h"
#include "led_strip.h"
#include "dy/error.h"
#include "dy/gfx/gfx.h"
#include "dy/display.h"
#include "dy/display_driver_ws2812.h"

// Returns absolute linear pixel position.
static uint32_t px_offset(dy_ws2812_config_t *cfg, uint32_t x, uint32_t y) {
    uint32_t seg_col = x / cfg->segments.ppx;
    uint32_t seg_row = y / cfg->segments.ppy;

    // Segment number
    uint32_t seg_pos = seg_col + seg_row * cfg->segments.cols;

    // Linear pixel position relative to a segment
    uint32_t pos = (x - seg_col * cfg->segments.ppx) + ((y - seg_row * cfg->segments.ppy) * cfg->segments.ppx);

    // Absolute linear pixel position
    return pos + cfg->segments.ppx * cfg->segments.ppy * seg_pos;
}

static dy_err_t write(void *cf, dy_gfx_buf_t *buf) {
    dy_ws2812_config_t *cfg = (dy_ws2812_config_t *) cf;

    for (uint16_t y = 0; y < buf->height; y++) {
        for (uint16_t x = 0; x < buf->width; x++) {
            dy_gfx_px_t px = dy_gfx_get_px(buf, x, y);
            led_strip_set_pixel(cfg->handle, px_offset(cfg, x, y), px.r, px.g, px.b);
        }
    }

    esp_err_t esp_err = led_strip_refresh(cfg->handle);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_INVALID_ARG, esp_err_to_name(esp_err));
    }

    return dy_ok();
}

static dy_err_t set_brightness(void *cfg, uint8_t value) {
    // not implemented
    return dy_ok();
}

static dy_err_t refresh(void *cfg) {
    // not implemented
    return dy_ok();
}

dy_err_t dy_display_driver_ws2812_init(
        uint8_t id,
        gpio_num_t pin_data,
        dy_display_driver_ws2812_segments_config_t segments
) {
    led_strip_config_t strip_config = {
            .strip_gpio_num = pin_data,
            .max_leds = segments.ppx * segments.cols * segments.ppy * segments.rows,
            .led_model = LED_MODEL_WS2812,
            .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
            .flags = {
                    .invert_out = false,
            }
    };

    led_strip_rmt_config_t rmt_config = {
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz =  10 * 1000 * 1000,
            .mem_block_symbols = 64,
            .flags = {
                    .with_dma = true,
            }
    };

    led_strip_handle_t handle;
    esp_err_t esp_err = led_strip_new_rmt_device(&strip_config, &rmt_config, &handle);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "led_strip_new_rmt_device: %s", esp_err_to_name(esp_err));
    }

    dy_ws2812_config_t *cfg = malloc(sizeof(dy_ws2812_config_t));
    if (cfg == NULL) {
        return dy_err(DY_ERR_NO_MEM, "malloc dy_ws2812_config_t failed");
    }

    cfg->segments = segments;
    cfg->handle = handle;

    dy_display_driver_t drv = {
            .cfg=cfg,
            .write=write,
            .set_brightness=set_brightness,
            .refresh=refresh,
    };

    return dy_display_set_driver(id, drv);
}