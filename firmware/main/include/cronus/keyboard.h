#ifndef CRONUS_KEYBOARD_H
#define CRONUS_KEYBOARD_H

#include "aespl/button.h"
#include "cronus/display.h"
#include "cronus/mode.h"
#include "cronus/weather.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

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

typedef struct {
    SemaphoreHandle_t mux;
    aespl_button_t btn_a;
    aespl_button_t btn_b;
    app_mode_t *app_mode;
    app_time_t *time;
    app_display_t *display;
    nvs_handle_t nvs;
    app_weather_t *weather;
    TimerHandle_t app_mode_timer;
} app_keyboard_t;

/**
 * Initializes keyboard related things.
 *
 * @param app Application
 * @return
 */
app_keyboard_t *app_keyboard_init(app_mode_t *mode, app_time_t *time, app_display_t *display, app_weather_t *weather, nvs_handle_t nvs);

#endif  // CRONUS_KEYBOARD_H
