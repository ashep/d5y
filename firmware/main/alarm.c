#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"

#include "cronus_main.h"
#include "cronus_alarm.h"

esp_err_t app_alarm_init(app_t *app) {
    gpio_config_t io = {
            .pin_bit_mask = 1UL << APP_ALARM_PIN,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .mode = GPIO_MODE_OUTPUT,
    };

    return gpio_config(&io);
}

void app_alarm_beep() {
    vTaskDelay(7);
    gpio_set_level(APP_ALARM_PIN, 1);
    vTaskDelay(7);
    gpio_set_level(APP_ALARM_PIN, 0);
}

static void alarm_task(void *args) {
    app_t *app = (app_t *) args;

    for (;;) {
        for (uint8_t i = 0; i < 4; i++) {
            app_alarm_beep();
        }

        vTaskDelay(100);

        if (!app->time.alarm_started) {
            break;
        }
    }

    vTaskDelete(NULL);
}

void app_alarm_start(app_t *app) {
    xTaskCreate(alarm_task, "alarm", 4096, (void *) app, 0, app->alarm_task);
    app->time.alarm_started = true;
}

void app_alarm_stop(app_t *app) {
    app->time.alarm_started = false;
}

