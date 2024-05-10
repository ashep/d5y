#include <stdio.h>

#include "string.h"

#include "esp_log.h"
#include "esp_event.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "d5y_bt.h"
#include "d5y_wifi.h"

#define LTAG "CRONUS_MAIN"

void app_main(void) {
    esp_err_t err;

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGI(LTAG, "default event loop created");

    ESP_ERROR_CHECK(cronus_wifi_init());
    ESP_ERROR_CHECK(cronus_bt_init()); // must be called last
}
