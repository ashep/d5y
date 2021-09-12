/**
 * @brief     Cronus Digital Clock
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#ifndef CRONUS_MAIN_H
#define CRONUS_MAIN_H

#ifndef APP_NAME
#define APP_NAME "Cronus"
#endif

#ifndef APP_VERSION
#define APP_VERSION "1.0"
#endif

#define APP_HW_VER_1_0 0x1
#define APP_HW_VER_1_1 0x2

#ifndef APP_HW_VERSION
#define APP_HW_VERSION APP_HW_VER_1_1
#endif

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

#include "cronus_display.h"
#include "cronus_net.h"
#include "cronus_weather.h"

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
    APP_MODE_SETTINGS_BRIGHTNESS,
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
    uint8_t sep_visible;
    uint8_t alarm_enabled;
    uint8_t alarm_started;
    uint8_t flush_to_rtc; // whether is datetime and alarm settings should be written toe RTC
} app_date_time_t;

typedef struct {
    char signature[100];
    SemaphoreHandle_t mux;
    nvs_handle nvs;
    TimerHandle_t show_mode_timer;
    app_mode_t mode;
    app_net_t net;
    app_date_time_t time;
    app_weather_t weather;
    aespl_httpd_t httpd;
    aespl_button_t btn_a;
    aespl_button_t btn_b;
    aespl_ds3231_t ds3231;
    app_display_t display;
    TaskHandle_t alarm_task;
} app_t;

/**
 * Initializes display related things.
 *
 * @param app Application
 * @return
 */
esp_err_t app_display_init(app_t *app);

/**
 * Initializes network related things.
 *
 * @param app Application
 * @return
 */
esp_err_t app_net_init(app_t *app);

/**
 * RTOS app entry point.
 */
void app_main();

#endif // CRONUS_MAIN_H
