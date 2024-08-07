#include "esp_log.h"

#include "dy/gfx/gfx.h"
#include "dy/display.h"
#include "dy/max7219.h"

dy_err_t set_driver(
    uint8_t id,
    void *cfg,
    display_drv_write_t write,
    display_drv_set_brightness_t set_bri,
    display_drv_refresh_t refresh
);

static dy_err_t draw(void *cfg, dy_gfx_buf_t *buf) {
    dy_err_code_t err;

    dy_max7219_config_t *cf = (dy_max7219_config_t *) cfg;

    if((err = dy_max7219_refresh(cf)) != DY_OK) {
        return dy_err(err, "dy_max7219_refresh failed");
    }

    // Split incoming buffer into 8x8 chunks; each chunk contains data for a single device
    dy_gfx_buf_array_t *chunks = dy_gfx_split(buf, 8, 8);
    if (chunks == NULL) {
        return dy_err(DY_ERR_UNKNOWN, "dy_gfx_split failed");
    }

    for (uint8_t row_n = 0; row_n <= 7; row_n++) {
        int dev_start = cf->rx ? 0 : chunks->length - 1;
        int dev_stop = cf->rx ? chunks->length : -1;
        int dev_step = cf->rx ? 1 : -1;

        for (int dev_n = dev_start; dev_n != dev_stop; dev_n = dev_n + dev_step) {
            uint8_t row_data = *(chunks->buffers[dev_n]->content[row_n]) >> 24;
            if ((err = dy_max7219_send(cf, row_n + 1, row_data)) != DY_OK) {
                return dy_err(err, "dy_max7219_send failed");
            }
        }

        if ((err = dy_max7219_latch(cf)) != DY_OK) {
            return dy_err(err, "dy_max7219_latch failed");
        }
    }

    dy_gfx_free_buf_array(chunks);

    return dy_ok();
}

static dy_err_t set_brightness(void *cfg, uint8_t value) {
    if (value > DY_MAX7219_INTENSITY_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "value must not be greater than %d", DY_MAX7219_INTENSITY_MAX);
    }

    dy_max7219_config_t *cfg_t = (dy_max7219_config_t *) cfg;
    cfg_t->intensity = value;

    dy_err_code_t err_code = dy_max7219_send_all(cfg_t, DY_MAX7219_ADDR_INTENSITY, value);
    if (err_code != DY_OK) {
        return dy_err(err_code, "dy_max7219_send_all failed");
    }

    return dy_ok();
}

static dy_err_t refresh(void *cfg) {
    dy_err_code_t err_code = dy_max7219_refresh((dy_max7219_config_t *) cfg);
    if (err_code != DY_OK) {
        return dy_err(err_code, "dy_max7219_refresh failed");
    }

    return dy_ok();
}

dy_err_t dy_display_init_driver_max7219(uint8_t id, dy_max7219_config_t *cfg) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "display id must be lower than %d", DY_DISPLAY_ID_MAX);
    }

    return set_driver(id, cfg, draw, set_brightness, refresh);
}
