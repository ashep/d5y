#include "esp_log.h"
#include "esp_event.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "dy/error.h"
#include "dy/bt.h"
#include "dy/wifi.h"

#include "main.h"

static dy_err_t init_nvs() {
    esp_err_t esp_err;

    esp_err = nvs_flash_init();
    if (esp_err == ESP_ERR_NVS_NO_FREE_PAGES || esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        if ((esp_err == nvs_flash_erase()) != ESP_OK) {
            return dy_error(DY_ERR_OP_FAILED, "nvs_flash_erase failed: %s", esp_err_to_name(esp_err));
        }

        if ((esp_err == nvs_flash_init()) != ESP_OK) {
            return dy_error(DY_ERR_OP_FAILED, "nvs_flash_init failed: %s", esp_err_to_name(esp_err));
        }
    } else if (esp_err != ESP_OK) {
        return dy_error(DY_ERR_OP_FAILED, "nvs_flash_init failed: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}

static dy_err_t init_max7219() {
    dy_err_t err;

    dy_max7219_config_t *cfg = malloc(sizeof(dy_max7219_config_t));
    if (cfg == NULL) {
        return dy_error(DY_ERR_NO_MEM, "dy_max7219_config_t malloc failed");
    }

    err = dy_max7219_init(
        cfg,
        CONFIG_DY_DISPLAY_DRIVER_MAX7219_PIN_CS,
        CONFIG_DY_DISPLAY_DRIVER_MAX7219_PIN_CLK,
        CONFIG_DY_DISPLAY_DRIVER_MAX7219_PIN_DATA,
        CONFIG_DY_DISPLAY_DRIVER_MAX7219_NX,
        CONFIG_DY_DISPLAY_DRIVER_MAX7219_NY,
        CONFIG_DY_DISPLAY_DRIVER_MAX7219_RX,
        CONFIG_DY_DISPLAY_DRIVER_MAX7219_RY);
    if (err.code != DY_OK) {
        return err;
    }

    err = dy_display_init_driver_max7219(CONFIG_DY_DISPLAY_DRIVER_MAX7219_ID, cfg);
    if (err.code != DY_OK) {
        return err;
    }

    ESP_LOGI(LTAG, "MAX7219 display driver initialized; cs=%d; clk=%d; data=%d; nx=%d; ny=%d; rx=%d; ry=%d",
             cfg->pin_cs, cfg->pin_clk, cfg->pin_data, cfg->nx, cfg->ny, cfg->rx, cfg->ry
    );

    return dy_ok();
}

dy_err_t init_display() {
#ifdef CONFIG_DY_DISPLAY_DRIVER_MAX7219_ENABLED
    return init_max7219();
#else
    return DY_ERR_INVALID_ARG;
#endif
}

void app_main(void) {
    esp_err_t esp_err;
    dy_err_t err;

    err = init_nvs();
    if (err.code != DY_OK) {
        ESP_LOGE(LTAG, "init_nvs: %s", dy_error_str(err));
        abort();
    }

    esp_err = esp_event_loop_create_default();
    if (esp_err != ESP_OK) {
        ESP_LOGE(LTAG, "esp_event_loop_create_default: %s", dy_error_str(err));
        abort();
    }

    err = init_display();
    if (err.code != DY_OK) {
        ESP_LOGE(LTAG, "init_display: %s", dy_error_str(err));
        abort();
    }

    err = dy_wifi_init();
    if (err.code != DY_OK) {
        ESP_LOGE(LTAG, "dy_wifi_init: %s", dy_error_str(err));
        abort();
    }

    // Must be called last
    err = dy_bt_init();
    if (err.code != DY_OK) {
        ESP_LOGE(LTAG, "dy_bt_init: %s", dy_error_str(err));
        abort();
    }
}
