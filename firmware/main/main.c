/**
 * @brief     Cronus Digital Clock
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include "string.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"

#include "i2c.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "cronus/main.h"
#include "cronus/fs.h"
#include "cronus/dtime.h"
#include "cronus/keyboard.h"
#include "cronus/alarm.h"
#include "cronus/network.h"

void app_main() {
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // App mode
    app_mode_t *mode = malloc(sizeof (app_mode_t));
    if (mode == NULL) {
        ESP_LOGE(APP_NAME, "failed to allocate memory for app mode");
        return;
    }
    *mode = APP_MODE_SHOW_MIN + 1;

    // NVS
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(nvs_open(APP_NAME, NVS_READWRITE, &nvs));

    // SPIFFS
    app_fs_t *fs = app_fs_init();

    // Time
    app_time_t *time = app_time_init(mode, nvs);
    if (time == NULL) {
        vTaskDelete(NULL);
        return;
    }

    // Alarm
    ESP_ERROR_CHECK(app_alarm_init());

    // Weather
    app_weather_t *weather = malloc(sizeof(app_weather_t));
    if (weather == NULL) {
        ESP_LOGE(APP_NAME, "failed to allocate memory for weather");
        return;
    }
    memset(weather, 0, sizeof(app_weather_t));

    // Display
    app_display_t *display = app_display_hw_ver_1_init(mode, time, weather, nvs);
    if (display == NULL) {
        return;
    }

    // Keyboard
    app_keyboard_t *kb = app_keyboard_init(mode, time, display, weather, nvs);
    if (kb == NULL) {
        return;
    }

    // Network
    app_net_t *net = NULL;
    ESP_ERROR_CHECK(app_net_init(net, time, weather));

    vTaskDelete(NULL);
}
