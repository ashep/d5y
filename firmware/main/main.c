/**
 * @brief     Cronus Digital Clock
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"

#include "aespl_gfx_buffer.h"

#include "cronus_main.h"
#include "cronus_nvs.h"
#include "cronus_keyboard.h"
#include "cronus_rtc.h"
#include "cronus_alarm.h"
#include "cronus_net.h"
#include "cronus_display.h"

static app_t app = {
        .mode = APP_MODE_SHOW_TIME,
};

void app_main() {
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    app.mux = xSemaphoreCreateBinary();
    xSemaphoreGive(app.mux);

    sprintf(app.signature, APP_NAME "/" APP_VERSION ":%d", APP_HW_VERSION);

    // Keyboard
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(app_keyboard_init(&app));

    // NVS flash
    ESP_ERROR_CHECK(app_nvs_init(&app));
//    ESP_ERROR_CHECK(app_nvs_erase(&app));

    // RTC
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER));
    ESP_ERROR_CHECK(app_rtc_init(&app));

    // Alarm
    ESP_ERROR_CHECK(app_alarm_init(&app));

    // Display
    ESP_ERROR_CHECK(app_display_init(&app));

    // Network
    ESP_ERROR_CHECK(app_net_init(&app));

    vTaskDelete(NULL);
}
