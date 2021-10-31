/**
 * @brief     Cronus Digital Clock Datetime Functions
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include <string.h>

#include "aespl/ds3231.h"
#include "cronus/alarm.h"
#include "cronus/main.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs.h"

static esp_err_t set_app_time_from_rtc(app_time_t *time) {
    if (xSemaphoreTake(time->mux, 10) != pdTRUE) {
        ESP_LOGE(APP_NAME, "set_app_time_from_rtc: error while locking");
        return ESP_FAIL;
    }

    esp_err_t err =
        aespl_ds3231_get_data(time->rtc, pdMS_TO_TICKS(APP_DS3231_TIMEOUT));
    if (err == ESP_OK) {
        time->second = time->rtc->sec;
        time->minute = time->rtc->min;
        time->hour = time->rtc->hour;
        time->dow = time->rtc->dow;
        time->day = time->rtc->day;
        time->month = time->rtc->mon;
        time->year = time->rtc->year;
        time->alarm_minute = time->rtc->alarm_1_min;
        time->alarm_hour = time->rtc->alarm_1_hour;
    } else {
        ESP_LOGE(APP_NAME, "aespl_ds3231_get_data error: %d", err);
    }

    if (xSemaphoreGive(time->mux) != pdTRUE) {
        ESP_LOGE(APP_NAME, "set_app_time_from_rtc: error while unlocking");
        err = ESP_FAIL;
    }

    return err;
}

static esp_err_t set_rtc_from_app_time(app_time_t *time) {
    if (xSemaphoreTake(time->mux, 10) != pdTRUE) {
        ESP_LOGE(APP_NAME, "set_rtc_from_app_time: error while locking");
        return ESP_FAIL;
    }

    time->rtc->sec = time->second;
    time->rtc->min = time->minute;
    time->rtc->hour = time->hour;
    time->rtc->dow = time->dow;
    time->rtc->day = time->day;
    time->rtc->mon = time->month;
    time->rtc->year = time->year;
    time->rtc->alarm_1_min = time->alarm_minute;
    time->rtc->alarm_1_hour = time->alarm_hour;

    esp_err_t err =
        aespl_ds3231_set_data(time->rtc, pdMS_TO_TICKS(APP_DS3231_TIMEOUT));
    if (err != ESP_OK) {
        ESP_LOGE(APP_NAME, "aespl_ds3231_set_data error: %d", err);
    }

    if (xSemaphoreGive(time->mux) != pdTRUE) {
        ESP_LOGE(APP_NAME, "set_rtc_from_app_time: error while unlocking");
        err = ESP_FAIL;
    }

    return err;
}

static void time_reader(void *args) {
    esp_err_t err;
    app_time_t *time = (app_time_t *)args;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Don't do anything in settings mode
        if (*time->app_mode >= APP_MODE_SETTINGS_MIN) {
            continue;
        }

        if (time->flush_to_rtc) {
            // flush app time to the RTC
            err = set_rtc_from_app_time(time);
            if (err) {
                ESP_LOGE(APP_NAME, "failed to flush datetime to the RTC");
            } else {
                time->flush_to_rtc = false;
                ESP_LOGI(APP_NAME, "datetime has been flushed to the RTC");
            }
        } else {
            // read time from the RTC to the app
            err = set_app_time_from_rtc(time);
            if (err) {
                ESP_LOGE(APP_NAME, "failed to read datetime from the RTC");
            }
        }

        // Start/stop alarm
        if (time->alarm_enabled) {
            if (time->alarm_started) {
                if (time->hour != time->alarm_hour ||
                    time->minute != time->alarm_minute) {
                    app_alarm_stop(time);
                }
            } else if (time->hour == time->alarm_hour &&
                       time->minute == time->alarm_minute && time->second < 2) {
                app_alarm_start(time);
            }
        }
    }
}

app_time_t *app_time_init(app_mode_t *app_mode, nvs_handle_t nvs) {
    esp_err_t err;

    err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER);
    if (err != ESP_OK) {
        ESP_LOGE(APP_NAME, "i2c_driver_install failed; err=%d", err);
        return NULL;
    }

    app_time_t *time = malloc(sizeof(app_time_t));
    if (time == NULL) {
        ESP_LOGE(APP_NAME, "failed to allocate memory for time");
        return NULL;
    }
    memset(time, 0, sizeof(app_time_t));

    time->mux = xSemaphoreCreateBinary();
    if (time->mux == NULL) {
        free(time);
        ESP_LOGE(APP_NAME, "failed to allocate a semaphore for time");
        return NULL;
    }
    xSemaphoreGive(time->mux);

    time->app_mode = app_mode;

    i2c_config_t i2c = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = APP_DS3231_SDA_PIN,
        .sda_pullup_en = APP_DS3231_SDA_PULLUP,
        .scl_io_num = APP_DS3231_SCL_PIN,
        .scl_pullup_en = APP_DS3231_SCL_PULLUP,
    };
    err = i2c_param_config(I2C_NUM_0, &i2c);
    if (err != ESP_OK) {
        free(time);
        ESP_LOGE(APP_NAME, "failed to initialize RTC I2C bus; err=%d", err);
        return NULL;
    }
    ESP_LOGI(APP_NAME, "RTC I2C bus initialized");

    // RTC connection
    time->rtc = malloc(sizeof(aespl_ds3231_t));
    if (time->rtc == NULL) {
        free(time);
        ESP_LOGE(APP_NAME, "failed to allocate memory for the RTC");
        return NULL;
    }
    err = aespl_ds3231_init(time->rtc);
    if (err != ESP_OK) {
        free(time->rtc);
        free(time);
        ESP_LOGE(APP_NAME, "failed to initialize the the RTC; err=%d", err);
        return NULL;
    }
    ESP_LOGI(APP_NAME, "RTC initialized");

    // Alarm enabled/disabled setting from NVS
    nvs_get_u8(nvs, "alarm_en", &time->alarm_enabled);

    xTaskCreate(time_reader, "rtc_time_reader", 4096, (void *)time, 0, NULL);
    ESP_LOGI(APP_NAME, "time functions initialized");

    return time;
}
