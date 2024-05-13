#include "stdbool.h"
#include "driver/gpio.h"

#include "dy/max7219.h"

dy_err_t dy_max7219_init(
    dy_max7219_config_t *cfg,
    gpio_num_t cs,
    gpio_num_t clk,
    gpio_num_t data,
    uint8_t nx,
    uint8_t ny,
    bool rx,
    bool ry
) {
    int err;

    cfg->pin_cs = cs;
    cfg->pin_clk = clk;
    cfg->pin_data = data;
    cfg->nx = nx;
    cfg->ny = ny;
    cfg->rx = rx;
    cfg->ry = ry;

    cfg->decode = DY_MAX7219_DECODE_MODE_NONE;
    cfg->intensity = DY_MAX7219_INTENSITY_MIN;
    cfg->scan_limit = DY_MAX7219_SCAN_LIMIT_8;
    cfg->power = DY_MAX7219_POWER_ON;
    cfg->test = DY_MAX7219_TEST_MODE_DISABLE;

    gpio_config_t gpio_cfg = {
        .pin_bit_mask = BIT(cfg->pin_cs) | BIT(cfg->pin_clk) | BIT(cfg->pin_data),
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
    };

    if ((err = gpio_config(&gpio_cfg)) != ESP_OK) {
        return dy_error(DY_ERR_OP_FAILED, "gpio_config failed: esp_err=%d", err);
    }

    if ((err = dy_max7219_refresh(cfg)) != DY_OK) {
        return dy_error(err, "dy_max7219_refresh failed");
    }

    if ((err = dy_max7219_clear(cfg)) != DY_OK) {
        return dy_error(err, "dy_max7219_clear failed");
    }

    return dy_error(DY_OK, "");
}

dy_err_code_t dy_max7219_latch(const dy_max7219_config_t *cfg) {
    esp_err_t err;

    err = gpio_set_level(cfg->pin_cs, 1);
    if (err != ESP_OK) {
        return esp_to_dy_err(err);
    }

    err = gpio_set_level(cfg->pin_cs, 0);
    if (err != ESP_OK) {
        return esp_to_dy_err(err);
    }

    return DY_OK;
}

dy_err_code_t dy_max7219_send(const dy_max7219_config_t *cfg, dy_max7219_addr_t addr, uint8_t data) {
    esp_err_t esp_err;

    // Setup pins
    if ((esp_err = gpio_set_level(cfg->pin_cs, 0)) != ESP_OK) {
        return esp_to_dy_err(esp_err);
    }
    if ((esp_err = gpio_set_level(cfg->pin_clk, 0)) != ESP_OK) {
        return esp_to_dy_err(esp_err);
    }
    if ((esp_err = gpio_set_level(cfg->pin_data, 0)) != ESP_OK) {
        return esp_to_dy_err(esp_err);
    }

    // 0-7:   data
    // 8-11:  address
    // 12-15: not used
    uint16_t frame = ((uint16_t) addr) << 8 | data;

    // MSB goes first
    for (int8_t i = 15; i >= 0; i--) {
        if ((esp_err = gpio_set_level(cfg->pin_data, 1 & frame >> i)) != ESP_OK) {
            return esp_to_dy_err(esp_err);
        }

        // Load data on rising edge
        if ((esp_err = gpio_set_level(cfg->pin_clk, 1)) != ESP_OK) {
            return esp_to_dy_err(esp_err);
        }

        // Prepare for the next load cycle
        if ((esp_err = gpio_set_level(cfg->pin_clk, 0)) != ESP_OK) {
            return esp_to_dy_err(esp_err);
        }
    }

    return DY_OK;
}

dy_err_code_t dy_max7219_send_all(const dy_max7219_config_t *cfg, dy_max7219_addr_t addr, uint8_t data) {
    dy_err_code_t err;

    for (uint8_t i = 0; i < cfg->nx * cfg->ny; i++) {
        err = dy_max7219_send(cfg, addr, data);
        if (err != DY_OK) {
            return err;
        }
    }

    return dy_max7219_latch(cfg);
}

dy_err_code_t dy_max7219_refresh(const dy_max7219_config_t *cfg) {
    dy_err_code_t err;

    err = dy_max7219_send_all(cfg, DY_MAX7219_ADDR_DECODE_MODE, cfg->decode);
    if (err != DY_OK) {
        return err;
    }

    err = dy_max7219_send_all(cfg, DY_MAX7219_ADDR_SCAN_LIMIT, cfg->scan_limit);
    if (err != DY_OK) {
        return err;
    }

    err = dy_max7219_send_all(cfg, DY_MAX7219_ADDR_INTENSITY, cfg->intensity);
    if (err != DY_OK) {
        return err;
    }

    err = dy_max7219_send_all(cfg, DY_MAX7219_ADDR_SHUTDOWN, cfg->power);
    if (err != DY_OK) {
        return err;
    }

    err = dy_max7219_send_all(cfg, DY_MAX7219_ADDR_TEST, cfg->test);
    if (err != DY_OK) {
        return err;
    }

    return DY_OK;
}

dy_err_code_t dy_max7219_clear(const dy_max7219_config_t *cfg) {
    esp_err_t err;
    for (int i = DY_MAX7219_ADDR_DIGIT_0; i <= DY_MAX7219_ADDR_DIGIT_7; i++) {
        err = dy_max7219_send_all(cfg, i, 0);
        if (err != DY_OK) {
            return err;
        }
    }

    return DY_OK;
}
