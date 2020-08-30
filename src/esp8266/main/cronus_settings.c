#include "stdlib.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "cronus_main.h"

static nvs_handle nvs_h = NULL;

esp_err_t cronus_settings_set_str(const char *key, const char *value)
{
    esp_err_t err;

    err = nvs_set_str(nvs_h, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE(APP_TAG "_settings", "Error %d while setting value '%s' for key '%s'", err, value, key);
    }
    else
    {
        err = nvs_commit(nvs_h);
        if (err != ESP_OK) {
            ESP_LOGE(APP_TAG "_settings", "Error %d while committing value '%s' for key '%s'", err, value, key);
        }
    }
    
    return err;
}

/**
 * WARNING: don't forget to free out_value in the caller
 */
esp_err_t cronus_settings_get_str(const char *key, char **out_value)
{
    size_t size;
    esp_err_t err;

    err = nvs_get_str(nvs_h, key, NULL, &size);
    if (err != ESP_OK)
    {
        switch (err)
        {
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGW(APP_TAG "_settings", "Value for key '%s' doesn't exist", key);
            break;

        default:
            ESP_LOGE(APP_TAG "_settings", "Error %d while calculating value size for key '%s'", err, key);
            break;
        }

        return err;
    }

    *out_value = malloc(size);
    err = nvs_get_str(nvs_h, key, *out_value, &size);

    if (err != ESP_OK)
        ESP_LOGE(APP_TAG "_settings", "Error %d while getting value for key '%s'", err, key);

    return err;
}

void cronus_settings_init()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(nvs_open(APP_TAG, NVS_READWRITE, &nvs_h));
    ESP_LOGI(APP_TAG "_settings", "Initialized");
}
