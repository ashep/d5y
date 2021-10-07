#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "cronus/main.h"
#include "cronus/alarm.h"

esp_err_t app_alarm_init() {
    gpio_config_t io = {
            .pin_bit_mask = 1UL << APP_ALARM_PIN,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .mode = GPIO_MODE_OUTPUT,
    };

    esp_err_t err = gpio_config(&io);
    if (err != ESP_OK) {
        ESP_LOGE(APP_NAME, "gpio_config() failed");
    } else {
        ESP_LOGI(APP_NAME, "alarm initialized");
    }

    return err;
}

void app_alarm_beep() {
    vTaskDelay(7);
    gpio_set_level(APP_ALARM_PIN, 1);
    vTaskDelay(7);
    gpio_set_level(APP_ALARM_PIN, 0);
}

static void alarm_task(void *args) {
    app_time_t *time = (app_time_t *) args;

    for (;;) {
        for (uint8_t i = 0; i < 4; i++) {
            app_alarm_beep();
        }

        vTaskDelay(100);

        if (!time->alarm_started) {
            break;
        }
    }

    vTaskDelete(NULL);
}

void app_alarm_start(app_time_t *time) {
    xTaskCreate(alarm_task, "alarm", 4096, (void *) time, 0, time->alarm_task);
    time->alarm_started = true;
}

void app_alarm_stop(app_time_t *time) {
    time->alarm_started = false;
}