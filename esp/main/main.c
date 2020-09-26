/**
 * @brief     Cronus Digital Clock Main
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "driver/gpio.h"

#include "aespl_settings.h"
#include "aespl_httpd.h"
#include "aespl_cfs.h"

#include "app_main.h"

static app_t app = {
    .mode = APP_MODE_SHOW_TIME,
};

void app_main() {
    esp_log_level_set(APP_NAME, ESP_LOG_NONE);

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    app.mux = xSemaphoreCreateBinary(),
    xSemaphoreGive(app.mux);

    ESP_ERROR_CHECK(app_display_init(&app));
    ESP_ERROR_CHECK(app_net_init(&app));
    ESP_ERROR_CHECK(app_buttons_init(&app));

    // for (;;) {
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
}
