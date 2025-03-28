#include "stdbool.h"
#include "driver/gpio.h"

#include "dy/_max7219.h"

dy_err_t max7219_init(max7219_config_t *cfg, gpio_num_t cs, gpio_num_t clk, gpio_num_t data,
                      uint8_t nx, uint8_t ny, bool rx) {
    dy_err_t err;
    esp_err_t esp_err;

    cfg->pin_cs = cs;
    cfg->pin_clk = clk;
    cfg->pin_data = data;
    cfg->nx = nx;
    cfg->ny = ny;
    cfg->reverse = rx;

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

    if ((esp_err = gpio_config(&gpio_cfg)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "gpio_config: %s", esp_err_to_name(esp_err));
    }

    if (dy_is_err(err = max7219_refresh(cfg))) {
        return dy_err_pfx("refresh", err);
    }

    if (dy_is_err(err = max7219_clear(cfg))) {
        return dy_err_pfx("clear", err);
    }

    return dy_ok();
}

dy_err_t max7219_latch(const max7219_config_t *cfg) {
    if (gpio_set_level(cfg->pin_cs, 1) != ESP_OK) {
        return dy_err(DY_ERR_GPIO_SET, "gpio_set_level failed");
    }

    if (gpio_set_level(cfg->pin_cs, 0) != ESP_OK) {
        return dy_err(DY_ERR_GPIO_SET, "gpio_set_level failed");
    }

    return dy_ok();
}

dy_err_t max7219_send(const max7219_config_t *cfg, max7219_addr_t addr, uint8_t data) {
    // Setup pins
    if (gpio_set_level(cfg->pin_cs, 0) != ESP_OK) {
        return dy_err(DY_ERR_GPIO_SET, "gpio_set_level failed");
    }
    if (gpio_set_level(cfg->pin_clk, 0) != ESP_OK) {
        return dy_err(DY_ERR_GPIO_SET, "gpio_set_level failed");
    }
    if (gpio_set_level(cfg->pin_data, 0) != ESP_OK) {
        return dy_err(DY_ERR_GPIO_SET, "gpio_set_level failed");
    }

    // 0-7:   data
    // 8-11:  address
    // 12-15: not used
    uint16_t frame = ((uint16_t) addr) << 8 | data;

    // MSB goes first
    for (int8_t i = 15; i >= 0; i--) {
        if (gpio_set_level(cfg->pin_data, 1 & frame >> i) != ESP_OK) {
            return dy_err(DY_ERR_GPIO_SET, "gpio_set_level failed");
        }

        // Load data on rising edge
        if (gpio_set_level(cfg->pin_clk, 1) != ESP_OK) {
            return dy_err(DY_ERR_GPIO_SET, "gpio_set_level failed");
        }

        // Prepare for the next load cycle
        if (gpio_set_level(cfg->pin_clk, 0) != ESP_OK) {
            return dy_err(DY_ERR_GPIO_SET, "gpio_set_level failed");
        }
    }

    return dy_ok();
}

dy_err_t max7219_refresh(const max7219_config_t *cfg) {
    dy_err_t err;

    err = max7219_send_all(cfg, DY_MAX7219_ADDR_DECODE_MODE, cfg->decode);
    if (dy_is_err(err)) {
        return err;
    }

    err = max7219_send_all(cfg, DY_MAX7219_ADDR_SCAN_LIMIT, cfg->scan_limit);
    if (dy_is_err(err)) {
        return err;
    }

    err = max7219_send_all(cfg, DY_MAX7219_ADDR_INTENSITY, cfg->intensity);
    if (dy_is_err(err)) {
        return err;
    }

    err = max7219_send_all(cfg, DY_MAX7219_ADDR_SHUTDOWN, cfg->power);
    if (dy_is_err(err)) {
        return err;
    }

    err = max7219_send_all(cfg, DY_MAX7219_ADDR_TEST, cfg->test);
    if (dy_is_err(err)) {
        return err;
    }

    return dy_ok();
}

dy_err_t max7219_send_all(const max7219_config_t *cfg, max7219_addr_t addr, uint8_t data) {
    dy_err_t err;

    for (uint8_t i = 0; i < cfg->nx * cfg->ny; i++) {
        err = max7219_send(cfg, addr, data);
        if (dy_is_err(err)) {
            return err;
        }
    }

    return max7219_latch(cfg);
}

dy_err_t max7219_clear(const max7219_config_t *cfg) {
    dy_err_t err;

    for (int i = DY_MAX7219_ADDR_DIGIT_0; i <= DY_MAX7219_ADDR_DIGIT_7; i++) {
        if (dy_is_err(err = max7219_send_all(cfg, i, 0))) {
            return err;
        }
    }

    return dy_ok();
}
