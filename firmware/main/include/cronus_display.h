#ifndef CRONUS_DISPLAY_H
#define CRONUS_DISPLAY_H

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

#ifndef APP_SCREEN_REFRESH_RATE
#define APP_SCREEN_REFRESH_RATE 100  // milliseconds
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

/**
 * Initializes display related things.
 *
 * @param app Application
 * @return
 */
esp_err_t app_display_init(app_t *app);

#endif // CRONUS_DISPLAY_H
