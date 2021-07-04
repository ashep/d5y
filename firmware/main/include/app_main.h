/**
 * @brief     Cronus Digital Clock
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#ifndef _CRONUS_ESP_MAIN_H_
#define _CRONUS_ESP_MAIN_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "aespl_gfx.h"
#include "aespl_button.h"
#include "aespl_httpd.h"
#include "aespl_ds3231.h"
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

#ifndef APP_SHOW_TIME_DURATION
#define APP_SHOW_TIME_DURATION 50  // seconds
#endif

#ifndef APP_SHOW_DATE_DURATION
#define APP_SHOW_DATE_DURATION 5  // seconds
#endif

#ifndef APP_SHOW_AMBIENT_TEMP_DURATION
#define APP_SHOW_AMBIENT_TEMP_DURATION 5  // seconds
#endif

#ifndef APP_SHOW_WEATHER_TEMP_DURATION
#define APP_SHOW_WEATHER_TEMP_DURATION 5  // seconds
#endif

#ifndef APP_NET_UPDATE_TIME_INETRVAL
#define APP_NET_UPDATE_TIME_INETRVAL 12  // hours
#endif

#ifndef APP_NET_UPDATE_WEATHER_INETRVAL
#define APP_NET_UPDATE_WEATHER_INETRVAL 1  // hours
#endif

#ifndef APP_API_HOST
#define APP_API_HOST "cronus.33v.xyz"
#endif

#ifndef APP_API_PATH_TIME
#define APP_API_PATH_TIME "/api/1/time"
#endif

#ifndef APP_API_PATH_WEATHER
#define APP_API_PATH_WEATHER "/api/1/weather"
#endif

#ifndef APP_API_TIME_REFRESH_PERIOD
#define APP_API_TIME_REFRESH_PERIOD 3600  // seconds
#endif

#ifndef APP_WIFI_STA_HOSTNAME
#define APP_WIFI_STA_HOSTNAME "Cronus"
#endif

#ifndef APP_WIFI_AP_SSID
#define APP_WIFI_AP_SSID "Cronus"
#endif

#ifndef APP_WIFI_AP_PASS
#define APP_WIFI_AP_PASS "cronus20"
#endif

#ifndef APP_WIFI_AP_MAX_CONN
#define APP_WIFI_AP_MAX_CONN 1
#endif

#ifndef APP_BTN_A
#define APP_BTN_A GPIO_NUM_0
#endif

#ifndef APP_BTN_A_MODE
#define APP_BTN_A_MODE AESPL_BUTTON_PRESS_LOW
#endif

#ifndef APP_BTN_B
#define APP_BTN_B GPIO_NUM_2
#endif

#ifndef APP_BTN_B_MODE
#define APP_BTN_B_MODE AESPL_BUTTON_PRESS_LOW
#endif

#ifndef APP_DS3231_SDA_PIN
#define APP_DS3231_SDA_PIN GPIO_NUM_4
#endif

#ifndef APP_DS3231_SDA_PULLUP
#define APP_DS3231_SDA_PULLUP GPIO_PULLUP_DISABLE
#endif

#ifndef APP_DS3231_SCL_PIN
#define APP_DS3231_SCL_PIN GPIO_NUM_5
#endif

#ifndef APP_DS3231_SCL_PULLUP
#define APP_DS3231_SCL_PULLUP GPIO_PULLUP_DISABLE
#endif

#ifndef APP_DS3231_TIMEOUT
#define APP_DS3231_TIMEOUT 100  // milliseconds
#endif

#ifndef APP_SCREEN_REFRESH_RATE
#define APP_SCREEN_REFRESH_RATE 100  // milliseconds
#endif

#ifndef APP_DISPLAY_PIN_EN
#define APP_DISPLAY_PIN_EN GPIO_NUM_0
#endif

#if APP_HW_VERSION == APP_HW_VER_1
    #define APP_MAX7219_DISP_X 4
    #define APP_MAX7219_DISP_Y 1
    #define APP_MAX7219_PIN_DATA GPIO_NUM_12
    #define APP_MAX7219_PIN_CLK GPIO_NUM_14
    #define APP_MAX7219_PIN_CS GPIO_NUM_13
#else
    #warning "Unknown hardware version"
#endif

#define APP_SECOND 1000
#define APP_MINUTE APP_SECOND * 60
#define APP_HOUR APP_MINUTE * 60

typedef enum {
    APP_MODE_SHOW_MIN,
    APP_MODE_SHOW_TIME,
    APP_MODE_SHOW_DATE,
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
    APP_MODE_SETTINGS_MAX,
} app_mode_t;

typedef struct {
    bool update_ok;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t dow;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t al_hour;
    uint8_t al_minute;
    bool sep;
    bool al_enabled;
} app_date_time_t;

typedef struct {
    bool update_ok;
    double temp;
} app_weather_t;

typedef struct {
    SemaphoreHandle_t mux;
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
} app_t;

void app_main();

/**
 * Initializes keyboard related things.
 *
 * @param app Application
 * @return
 */
esp_err_t app_keyboard_init(app_t *app);

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
 * Initializes RTC related things.
 *
 * @param app Application
 * @return
 */
esp_err_t app_rtc_init(app_t *app);

/**
 * Sets RTC values from app->time.
 *
 * @param app Application
 * @return
 */
esp_err_t set_rtc_from_app(app_t *app);

#endif
