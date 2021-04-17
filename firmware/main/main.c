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

#include "app_main.h"

static app_t app = {
    .mode = APP_MODE_SHOW_TIME,
};

void app_main() {
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    app.mux = xSemaphoreCreateBinary(),
    xSemaphoreGive(app.mux);

    // Initialize keyboard
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(app_keyboard_init(&app));

    // Initialize RTC
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER));
    ESP_ERROR_CHECK(app_rtc_init(&app));

    // Initialize display
    ESP_ERROR_CHECK(app_display_init(&app));

    // Initialize network
    ESP_ERROR_CHECK(app_net_init(&app));

    // for (;;) {
    //     vTaskDelay(1);
    // }

    vTaskDelete(NULL);
}
