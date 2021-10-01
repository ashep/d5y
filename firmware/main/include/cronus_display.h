#ifndef CRONUS_DISPLAY_H
#define CRONUS_DISPLAY_H

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

#ifndef APP_SCREEN_REFRESH_RATE
#define APP_SCREEN_REFRESH_RATE 100
#endif

#ifndef APP_SCREEN_MIN_BRIGHTNESS
#define APP_SCREEN_MIN_BRIGHTNESS 0
#endif

#ifndef APP_SCREEN_MAX_BRIGHTNESS
#define APP_SCREEN_MAX_BRIGHTNESS 15
#endif

#ifndef APP_SCREEN_BRIGHTNESS_REG_TIMEOUT
#define APP_SCREEN_BRIGHTNESS_REG_TIMEOUT 2500
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
    aespl_gfx_buf_t *buf;
    aespl_max7219_config_t max7219;
    aespl_max7219_matrix_config_t max7219_matrix;
    uint16_t refresh_cnt;
    uint16_t refresh_cnt_max;
    uint8_t brightness;
    uint8_t min_brightness;
    uint8_t max_brightness;
    bool max_brightness_changed;
} app_display_t;


#endif // CRONUS_DISPLAY_H
