#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"

#include "aespl_settings.h"
#include "aespl_http_server.h"
#include "aespl_cfs.h"

#include "cronus_esp_main.h"

void app_main() {
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(aespl_settings_init(APP_NAME));
    ESP_ERROR_CHECK(cronus_net_init());
    ESP_ERROR_CHECK(aespl_httpd_start(APP_NAME, NULL));
    ESP_ERROR_CHECK(aespl_cfs_init(NULL));
}
