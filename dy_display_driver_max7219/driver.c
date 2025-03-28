#include "esp_log.h"

#include "dy/gfx/gfx.h"
#include "dy/display.h"
#include "dy/_max7219.h"

static dy_err_t write(void *cfg, dy_gfx_buf_t *buf) {
    dy_err_t err;
    max7219_config_t *cfg_t = (max7219_config_t *) cfg;

    if (dy_is_err(err = max7219_refresh(cfg_t))) {
        return dy_err_pfx("max7219_refresh", err);
    }

    // Split the buffer into 8x8 chunks; each chunk contains data for a single device
    dy_gfx_buf_array_t *chunks = dy_gfx_split(buf, 8, 8);
    if (chunks == NULL) {
        return dy_err(DY_ERR_UNKNOWN, "dy_gfx_split failed");
    }

    for (uint8_t row_n = 0; row_n <= 7; row_n++) {
        int dev_start = cfg_t->reverse ? 0 : chunks->len - 1;
        int dev_stop = cfg_t->reverse ? chunks->len : -1;
        int dev_step = cfg_t->reverse ? 1 : -1;

        for (int dev_n = dev_start; dev_n != dev_stop; dev_n = dev_n + dev_step) {
            uint8_t row_data = 0;
            dy_gfx_buf_t *dev_buf = chunks->buffers[dev_n];

            // Each row has 8-pixel width
            for (uint16_t x = 0; x < 8; x++) {
                uint8_t px_val = dy_gfx_get_px(dev_buf, x, row_n).r >> 7;
                row_data |= px_val << (7 - x);
            }

            if (dy_is_err(err = max7219_send(cfg_t, row_n + 1, row_data))) {
                return dy_err_pfx("dy_max7219_send", err);
            }
        }

        if (dy_is_err(err = max7219_latch(cfg_t))) {
            return dy_err_pfx("max7219_latch", err);
        }
    }

    dy_gfx_free_buf_array(chunks);

    return dy_ok();
}

static dy_err_t set_brightness(void *cfg, uint8_t value) {
    if (value > DY_MAX7219_INTENSITY_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "value must not be greater than %d", DY_MAX7219_INTENSITY_MAX);
    }

    max7219_config_t *cfg_t = (max7219_config_t *) cfg;
    cfg_t->intensity = value;

    dy_err_t err;
    if (dy_is_err(err = max7219_send_all(cfg_t, DY_MAX7219_ADDR_INTENSITY, value))) {
        return dy_err_pfx("max7219_send_all", err);
    }

    return dy_ok();
}

static dy_err_t refresh(void *cfg) {
    return max7219_refresh((max7219_config_t *) cfg);
}

dy_err_t dy_display_driver_max7219_init(uint8_t id, gpio_num_t cs, gpio_num_t clk, gpio_num_t data,
                                        uint8_t nx, uint8_t ny, bool reverse) {
    dy_err_t err;
    max7219_config_t *cfg = malloc(sizeof(max7219_config_t));

    if (dy_is_err(err = max7219_init(cfg, cs, clk, data, nx, ny, reverse))) {
        return dy_err_pfx("max7219_init", err);
    }

    dy_display_driver_t driver = {
            .cfg = cfg,
            .write = write,
            .set_brightness = set_brightness,
            .refresh = refresh,
    };

    return dy_display_set_driver(id, driver);
}

