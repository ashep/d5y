#ifndef CRONUS_DISPLAY_H
#define CRONUS_DISPLAY_H

#include "semphr.h"
#include "nvs.h"

#include "aespl_gfx_buffer.h"
#include "aespl_max7219.h"
#include "aespl_max7219_matrix.h"

#include "cronus/mode.h"
#include "cronus/weather.h"
#include "cronus/dtime.h"

#ifndef APP_SHOW_TIME_DURATION
#define APP_SHOW_TIME_DURATION 50000
#endif

#ifndef APP_SHOW_DATE_DURATION
#define APP_SHOW_DATE_DURATION 5000
#endif

#ifndef APP_SHOW_AMBIENT_TEMP_DURATION
#define APP_SHOW_AMBIENT_TEMP_DURATION 5000
#endif

#ifndef APP_SHOW_WEATHER_TEMP_DURATION
#define APP_SHOW_WEATHER_TEMP_DURATION 5000
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

#ifndef APP_DISPLAY_BRIGHTNESS_REG_TIMEOUT
#define APP_DISPLAY_BRIGHTNESS_REG_TIMEOUT 2500
#endif


#if APP_HW_VERSION == APP_HW_VER_1_0 || \
    APP_HW_VERSION == APP_HW_VER_1_1
#define APP_MAX7219_DISP_X 4
#define APP_MAX7219_DISP_Y 1
#define APP_MAX7219_DISP_REVERSE 0
#define APP_MAX7219_PIN_DATA GPIO_NUM_12
#define APP_MAX7219_PIN_CLK GPIO_NUM_14
#define APP_MAX7219_PIN_CS GPIO_NUM_13
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
app_display_t *app_display_init(app_mode_t *mode, app_time_t *time, app_weather_t *weather, nvs_handle_t nvs);

#endif // CRONUS_DISPLAY_H
