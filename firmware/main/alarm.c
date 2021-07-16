#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"

#include "cronus_main.h"
#include "cronus_alarm.h"

esp_err_t app_alarm_init(app_t *app) {
    gpio_config_t io = {
            .pin_bit_mask = APP_ALARM_PIN,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .mode = GPIO_MODE_OUTPUT,
    };

    return gpio_config(&io);
}

static void alarmTask(void *args) {
    app_t *app = (app_t *) args;

    for (;;) {
        for (uint8_t i = 0; i < 4; i++) {
            vTaskDelay(7);
            gpio_set_level(GPIO_NUM_15, 1);
            vTaskDelay(7);
            gpio_set_level(GPIO_NUM_15, 0);
        }

        vTaskDelay(100);
    }
}

void app_alarm_start(app_t *app) {
    xTaskCreate(alarmTask, "alarm", 4096, (void *) app, 0, app->alarm_task);
}

void app_alarm_stop(app_t *app) {
    vTaskDelete(app->alarm_task);
}

