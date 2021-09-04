/**
 * @brief     Cronus Digital Clock
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#ifndef CRONUS_MAIN_H
#define CRONUS_MAIN_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "nvs.h"

#include "aespl_httpd.h"
#include "aespl_button.h"
#include "aespl_ds3231.h"
#include "aespl_gfx_buffer.h"
#include "aespl_max7219.h"
#include "aespl_max7219_matrix.h"

/**
 * Hardware versions
 */
#define APP_HW_VER_1 0x1

#ifndef APP_HW_VERSION
#define APP_HW_VERSION APP_HW_VER_1
#endif

#ifndef APP_NAME
#define APP_NAME "cronus"
#endif

typedef enum {
    APP_MODE_SHOW_MIN,
    APP_MODE_SHOW_TIME,
    APP_MODE_SHOW_DATE,
    APP_MODE_SHOW_DOW,
    APP_MODE_SHOW_AMBIENT_TEMP,
    APP_MODE_SHOW_WEATHER_TEMP,
    APP_MODE_SHOW_MAX,
    APP_MODE_SETTINGS_MIN,
    APP_MODE_SETTINGS_TIME_HOUR,
    APP_MODE_SETTINGS_TIME_MINUTE,
    APP_MODE_SETTINGS_DATE_DAY,
    APP_MODE_SETTINGS_DATE_MONTH,
    APP_MODE_SETTINGS_DATE_DOW,
    APP_MODE_SETTINGS_DATE_YEAR,
    APP_MODE_SETTINGS_ALARM_HOUR,
    APP_MODE_SETTINGS_ALARM_MINUTE,
    APP_MODE_SETTINGS_MAX,
} app_mode_t;

typedef struct {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t dow;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t alarm_hour;
    uint8_t alarm_minute;
    uint8_t update_ok;
    uint8_t sep_visible;
    uint8_t alarm_enabled;
    uint8_t alarm_started;
    uint8_t flush_to_rtc; // whether is datetime and alarm settings should be written toe RTC
} app_date_time_t;

typedef struct {
    bool update_ok;
    double temp;
} app_weather_t;

typedef struct {
    SemaphoreHandle_t mux;
    nvs_handle nvs;
    TimerHandle_t show_mode_timer;
    uint16_t display_refresh_cnt;
    uint16_t display_refresh_cnt_max;
    app_mode_t mode;
    app_date_time_t time;
    app_weather_t weather;
    aespl_httpd_t httpd;
    aespl_button_t btn_a;
    aespl_button_t btn_b;
    aespl_ds3231_t ds3231;
    aespl_gfx_buf_t *gfx_buf;
    aespl_max7219_config_t max7219;
    aespl_max7219_matrix_config_t max7219_matrix;
    TaskHandle_t alarm_task;
} app_t;

/**
 * RTOS app entry point.
 */
void app_main();

#endif // CRONUS_MAIN_H
