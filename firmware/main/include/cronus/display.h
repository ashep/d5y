#ifndef CRONUS_DISPLAY_H
#define CRONUS_DISPLAY_H

#include "aespl/gfx_buffer.h"
#include "aespl/max7219.h"
#include "aespl/max7219_matrix.h"
#include "cronus/dtime.h"
#include "cronus/mode.h"
#include "cronus/version.h"
#include "cronus/weather.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs.h"

#ifndef APP_SHOW_TIME_DURATION
#define APP_SHOW_TIME_DURATION 50000
#endif

#ifndef APP_SHOW_DATE_DURATION
#define APP_SHOW_DATE_DURATION 5000
#endif

#ifndef APP_SHOW_TEMP_DURATION
#define APP_SHOW_TEMP_DURATION 5000
#endif

#ifndef APP_DISPLAY_REFRESH_RATE
#define APP_DISPLAY_REFRESH_RATE 100
#endif

#ifndef APP_DISPLAY_MIN_BRIGHTNESS
#define APP_DISPLAY_MIN_BRIGHTNESS 0
#endif

#ifndef APP_DISPLAY_MAX_BRIGHTNESS
#define APP_DISPLAY_MAX_BRIGHTNESS 15
#endif

#ifndef APP_BRIGHTNESS_REG_TIMEOUT
#define APP_BRIGHTNESS_REG_TIMEOUT 2500
#endif

#ifndef APP_SPLASH_SCREEN_ENABLED
#define APP_SPLASH_SCREEN_ENABLED 1
#endif

#if APP_HW_VER_MAJ == 1
#define APP_HW_V1_DISPLAYS_X 4
#define APP_HW_V1_DISPLAYS_Y 1
#define APP_HW_V1_DISPLAY_HORIZ_REVERSE 0
#define APP_HW_V1_DISPLAY_PIN_DATA GPIO_NUM_12
#define APP_HW_V1_DISPLAY_PIN_CLK GPIO_NUM_14
#define APP_HW_V1_DISPLAY_PIN_CS GPIO_NUM_13
#else
#warning "Unknown hardware version"
#endif

typedef struct {
    SemaphoreHandle_t mux;
    aespl_gfx_buf_t *buf;
    aespl_max7219_config_t max7219;
    aespl_max7219_matrix_config_t max7219_matrix;
    uint16_t refresh_cnt;
    uint16_t refresh_cnt_max;
    uint8_t brightness;
    uint8_t min_brightness;
    uint8_t max_brightness;
    xTimerHandle brightness_timer;
    bool sep_visible;
    bool max_brightness_changed;
    char splash_screen_text[20];
    app_mode_t *mode;
    nvs_handle_t nvs;
    app_time_t *time;
    app_weather_t *weather;
} app_display_t;

/**
 * Initializes display.
 *
 * @return
 */
app_display_t *app_display_hw_ver_1_init(app_mode_t *mode, app_time_t *time, app_weather_t *weather, nvs_handle_t nvs);

#endif  // CRONUS_DISPLAY_H
