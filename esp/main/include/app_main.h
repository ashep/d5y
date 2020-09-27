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
#include "aespl_button.h"
#include "aespl_httpd.h"
#include "aespl_ds3231.h"
#include "aespl_gfx.h"
#include "aespl_max7219.h"
#include "aespl_max7219_matrix.h"

typedef enum {
    APP_DISPLAY_MAX7219,
} app_display_t;

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

#ifndef APP_API_URL_TIME
#define APP_API_URL_TIME "http://cronus.33v.xyz/api/1/time"
#endif

#ifndef APP_API_URL_WEATHER
#define APP_API_URL_WEATHER "http://cronus.33v.xyz/api/1/weather"
#endif

#ifndef APP_API_TIME_REFRESH_PERIOD
#define APP_API_TIME_REFRESH_PERIOD 3600
#endif

#ifndef APP_WIFI_STA_HOSTNAME
#define APP_WIFI_STA_HOSTNAME "cronus"
#endif

#ifndef APP_WIFI_AP_SSID
#define APP_WIFI_AP_SSID "cronus"
#endif

#ifndef APP_WIFI_AP_PASS
#define APP_WIFI_AP_PASS "cronus20"
#endif

#ifndef APP_WIFI_AP_MAX_CONN
#define APP_WIFI_AP_MAX_CONN 1
#endif

#ifndef APP_BTN_A
#define APP_BTN_A GPIO_NUM_2
#endif

#ifndef APP_BTN_A_MODE
#define APP_BTN_A_MODE AESPL_BUTTON_PRESS_LOW
#endif

#ifndef APP_BTN_B
#define APP_BTN_B GPIO_NUM_12
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

#ifndef APP_DISPLAY_DRIVER
#define APP_DISPLAY_DRIVER APP_DISPLAY_MAX7219
#endif

#ifndef APP_MAX7219_PIN_CS
#define APP_MAX7219_PIN_CS GPIO_NUM_15
#endif

#ifndef APP_MAX7219_PIN_CLK
#define APP_MAX7219_PIN_CLK GPIO_NUM_14
#endif

#ifndef APP_MAX7219_PIN_DATA
#define APP_MAX7219_PIN_DATA GPIO_NUM_13
#endif

#ifndef APP_MAX7219_DISP_X
#define APP_MAX7219_DISP_X 4
#endif

#ifndef APP_MAX7219_DISP_Y
#define APP_MAX7219_DISP_Y 1
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
    APP_MODE_SETTINGS_DATE_YEAR,
    APP_MODE_SETTINGS_DATE_MONTH,
    APP_MODE_SETTINGS_DATE_DAY,
    APP_MODE_SETTINGS_DATE_DOW,
    APP_MODE_SETTINGS_MAX,
} app_mode_t;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t dow;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    bool sep;
    uint8_t sep_cnt;
} app_date_time_t;

typedef struct {
    bool update_ok;
    double temp;
} app_weather_t;

typedef struct {
    SemaphoreHandle_t mux;
    TimerHandle_t show_mode_timer;
    app_mode_t mode;
    app_date_time_t time;
    app_weather_t weather;
    aespl_httpd_t httpd;
    aespl_button_t btn_a;
    aespl_button_t btn_b;
    aespl_ds3231_t ds3231;
    aespl_gfx_buf_t gfx_buf;
    aespl_max7219_config_t max7219;
    aespl_max7219_matrix_config_t max7219_matrix;
} app_t;

void app_main();
esp_err_t cronus_net_init();
esp_err_t app_keyboard_init(app_t *app);
esp_err_t app_rtc_init(app_t *app);
esp_err_t app_display_init(app_t *app);
esp_err_t app_net_init(app_t *app);

#endif
