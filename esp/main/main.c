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
#include "driver/i2c.h"

#include "aespl_settings.h"
#include "aespl_httpd.h"
#include "aespl_cfs.h"

#include "app_main.h"

static app_t app = {
    .mode = APP_MODE_SHOW_TIME,
};

void app_main() {
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER));

    app.mux = xSemaphoreCreateBinary(),
    xSemaphoreGive(app.mux);

    ESP_ERROR_CHECK(app_keyboard_init(&app));
    ESP_ERROR_CHECK(app_rtc_init(&app));
    ESP_ERROR_CHECK(app_display_init(&app));
    ESP_ERROR_CHECK(app_net_init(&app));

    // for (;;) {
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
}
