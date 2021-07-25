#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "cronus_main.h"
#include "cronus_nvs.h"

esp_err_t app_nvs_init(app_t *app) {
    esp_err_t err;

    err = nvs_flash_init();
    if (err == ESP_OK) {
        ESP_LOGI(APP_NAME, "NVS flash initialized");
    } else {
        ESP_LOGE(APP_NAME, "NVS flash initialization error: %d", err);
        return err;
    }

    err = nvs_open(APP_NAME, NVS_READWRITE, &app->nvs);
    if (err == ESP_OK) {
        ESP_LOGI(APP_NAME, "NVS opened");
    } else {
        ESP_LOGE(APP_NAME, "NVS open error: %d", err);
    }

    return err;
}

esp_err_t app_nvs_set_u8(app_t *app, const char *key, uint8_t value) {
    return nvs_set_u8(app->nvs, key, value);
}

esp_err_t app_nvs_get_u8(app_t *app, const char *key, uint8_t *value) {
    return nvs_get_u8(app->nvs, key, value);
}

esp_err_t app_nvs_erase(app_t *app) {
    esp_err_t err = ESP_OK;

    err = nvs_erase_all(app->nvs);
    if (err) {
        ESP_LOGE(APP_NAME, "failed to erase NVS: %d", err);
        return err;
    }

    err = nvs_commit(app->nvs);
    if (err) {
        ESP_LOGE(APP_NAME, "failed to commit NVS: %d", err);
        return err;
    }

    return err;
}
