/**
 * @brief     Cronus Digital Clock RTC Functions
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_err.h"
#include "driver/i2c.h"

#include "aespl_ds3231.h"

#include "cronus_main.h"
#include "cronus_rtc.h"
#include "cronus_nvs.h"
#include "cronus_alarm.h"

static esp_err_t set_app_time_from_rtc(app_t *app) {
    if (xSemaphoreTake(app->mux, 10) != pdTRUE) {
        ESP_LOGE(APP_NAME, "set_app_time_from_rtc: error while locking");
        return ESP_FAIL;
    }

    esp_err_t err = aespl_ds3231_get_data(&app->ds3231, pdMS_TO_TICKS(APP_DS3231_TIMEOUT));
    if (err == ESP_OK) {
        app->time.second = app->ds3231.sec;
        app->time.minute = app->ds3231.min;
        app->time.hour = app->ds3231.hour;
        app->time.dow = app->ds3231.dow;
        app->time.day = app->ds3231.day;
        app->time.month = app->ds3231.mon;
        app->time.year = app->ds3231.year;
        app->time.alarm_minute = app->ds3231.alarm_1_min;
        app->time.alarm_hour = app->ds3231.alarm_1_hour;
    } else {
        ESP_LOGE(APP_NAME, "aespl_ds3231_get_data error: %d", err);
    }

    if (xSemaphoreGive(app->mux) != pdTRUE) {
        ESP_LOGE(APP_NAME, "set_app_time_from_rtc: error while unlocking");
        err = ESP_FAIL;
    }

    return err;
}

static esp_err_t set_rtc_from_app_time(app_t *app) {
    if (xSemaphoreTake(app->mux, 10) != pdTRUE) {
        ESP_LOGE(APP_NAME, "set_rtc_from_app_time: error while locking");
        return ESP_FAIL;
    }

    app->ds3231.sec = app->time.second;
    app->ds3231.min = app->time.minute;
    app->ds3231.hour = app->time.hour;
    app->ds3231.dow = app->time.dow;
    app->ds3231.day = app->time.day;
    app->ds3231.mon = app->time.month;
    app->ds3231.year = app->time.year;
    app->ds3231.alarm_1_min = app->time.alarm_minute;
    app->ds3231.alarm_1_hour = app->time.alarm_hour;

    esp_err_t err = aespl_ds3231_set_data(&app->ds3231, pdMS_TO_TICKS(APP_DS3231_TIMEOUT));
    if (err != ESP_OK) {
        ESP_LOGE(APP_NAME, "aespl_ds3231_set_data error: %d", err);
    }

    if (xSemaphoreGive(app->mux) != pdTRUE) {
        ESP_LOGE(APP_NAME, "set_rtc_from_app_time: error while unlocking");
        err = ESP_FAIL;
    }

    return err;
}

static void time_reader(void *args) {
    esp_err_t err;
    app_t *app = (app_t *) args;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Don't do anything in settings mode
        if (app->mode >= APP_MODE_SETTINGS_MIN) {
            continue;
        }

        if (app->time.flush_to_rtc) {
            // flush app time to the RTC
            err = set_rtc_from_app_time(app);
            if (err) {
                ESP_LOGE(APP_NAME, "failed to flush datetime to the RTC");
            } else {
                app->time.flush_to_rtc = false;
                ESP_LOGI(APP_NAME, "datetime has been flushed to the RTC");
            }
        } else {
            // read time from the RTC to the app
            err = set_app_time_from_rtc(app);
            if (err) {
                ESP_LOGE(APP_NAME, "failed to read datetime from the RTC");
            }
        }

        // Start/stop alarm
        app_date_time_t t = app->time;
        if (t.alarm_enabled) {
            if (t.alarm_started) {
                if (t.hour != t.alarm_hour || t.minute != t.alarm_minute) {
                    app_alarm_stop(app);
                }
            } else if (t.hour == t.alarm_hour && t.minute == t.alarm_minute && t.second < 2) {
                app_alarm_start(app);
            }
        }
    }
}

esp_err_t app_rtc_init(app_t *app) {
    esp_err_t err;

    i2c_config_t i2c = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = APP_DS3231_SDA_PIN,
            .sda_pullup_en = APP_DS3231_SDA_PULLUP,
            .scl_io_num = APP_DS3231_SCL_PIN,
            .scl_pullup_en = APP_DS3231_SCL_PULLUP,
    };

    err = i2c_param_config(I2C_NUM_0, &i2c);
    if (err) {
        return err;
    }

    err = aespl_ds3231_init(&app->ds3231);
    if (err) {
        return err;
    }

    // Alarm enabled/disabled setting from NVS
    app_nvs_get_u8(app, "alarm_en", &app->time.alarm_enabled);

    xTaskCreate(time_reader, "rtc_time_reader", 4096, (void *) app, 0, NULL);

    ESP_LOGI(APP_NAME, "RTC initialized");

    return ESP_OK;
}
