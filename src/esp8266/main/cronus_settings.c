#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "cronus_main.h"

static nvs_handle nvs_h = NULL;

void cronus_settings_init()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(nvs_open(APP_TAG, NVS_READWRITE, &nvs_h));
    ESP_LOGI(APP_TAG, "NVS initialized");
}
