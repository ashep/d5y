/**
 * @brief     Cronus Digital Clock Keyboard Functions
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "esp_err.h"

#include "aespl_button.h"

#include "cronus_main.h"
#include "cronus_keyboard.h"
#include "cronus_nvs.h"
#include "cronus_rtc.h"
#include "cronus_display.h"
#include "cronus_alarm.h"

static void switch_show_mode(app_t *app) {
    if (app->mode > APP_MODE_SETTINGS_MIN) {
        return;
    }

    app->mode++;

    // Will work in version 1.1
    if (app->mode == APP_MODE_SHOW_AMBIENT_TEMP) {
        app->mode++;
    }

    if (app->mode == APP_MODE_SHOW_WEATHER_TEMP && !app->weather.update_ok) {
        app->mode++;
    }

    if (app->mode == APP_MODE_SHOW_MAX) {
        app->mode = APP_MODE_SHOW_MIN + 1;
    }
}

static void inc_hour(app_t *app) {
    app->display.refresh_cnt = 0;
    app->time.hour++;
    if (app->time.hour > 23) {
        app->time.hour = 0;
    }
}

static void inc_alarm_hour(app_t *app) {
    app->display.refresh_cnt = 0;
    app->time.alarm_hour++;
    if (app->time.alarm_hour > 23) {
        app->time.alarm_hour = 0;
    }
}

static void inc_minute(app_t *app) {
    app->display.refresh_cnt = 0;
    app->time.minute++;
    if (app->time.minute > 59) {
        app->time.minute = 0;
    }
}

static void inc_alarm_minute(app_t *app) {
    app->display.refresh_cnt = 0;
    app->time.alarm_minute++;
    if (app->time.alarm_minute > 59) {
        app->time.alarm_minute = 0;
    }
}

static void inc_day(app_t *app) {
    app->display.refresh_cnt = 0;
    app->time.day++;
    if (app->time.day > 31) {
        app->time.day = 1;
    }
}

static void inc_month(app_t *app) {
    app->display.refresh_cnt = 0;
    app->time.month++;
    if (app->time.month > 12) {
        app->time.month = 1;
    }
}

static void inc_dow(app_t *app) {
    app->display.refresh_cnt = 0;
    app->time.dow++;
    if (app->time.dow > 6) {
        app->time.dow = 0;
    }
}

static void inc_year(app_t *app) {
    app->display.refresh_cnt = 0;
    app->time.year++;
    if (app->time.year > 99) {
        app->time.year = 21;
    }
}

static void inc_screen_brightness(app_t *app) {
    app->display.refresh_cnt = 0;
    app->display.max_brightness++;
    app->display.max_brightness_changed = true;
    if (app->display.max_brightness > APP_SCREEN_MAX_BRIGHTNESS) {
        app->display.max_brightness = 0;
    }
}

static bool btn_a_l_press(void *args) {
    app_t *app = (app_t *) args;

    if (app->time.alarm_started) {
        app_alarm_stop(app);
    }

    switch (app->mode) {
        case APP_MODE_SHOW_TIME:
            app->time.alarm_enabled = !app->time.alarm_enabled;
            app_nvs_set_u8(app, "alarm_en", app->time.alarm_enabled);
            app_alarm_beep();
            return false;
        case APP_MODE_SETTINGS_TIME_HOUR:
            inc_hour(app);
            break;
        case APP_MODE_SETTINGS_TIME_MINUTE:
            inc_minute(app);
            break;
        case APP_MODE_SETTINGS_DATE_DAY:
            inc_day(app);
            break;
        case APP_MODE_SETTINGS_DATE_MONTH:
            inc_month(app);
            break;
        case APP_MODE_SETTINGS_DATE_DOW:
            inc_dow(app);
            break;
        case APP_MODE_SETTINGS_DATE_YEAR:
            inc_year(app);
            break;
        case APP_MODE_SETTINGS_ALARM_HOUR:
            inc_alarm_hour(app);
            break;
        case APP_MODE_SETTINGS_ALARM_MINUTE:
            inc_alarm_minute(app);
            break;
        case APP_MODE_SETTINGS_BRIGHTNESS:
            inc_screen_brightness(app);
            break;
        default:
            break;
    }

    return true;
}

static bool btn_a_release(void *args) {
    app_t *app = (app_t *) args;

    if (app->time.alarm_started) {
        app_alarm_stop(app);
    }

    if (app->mode < APP_MODE_SHOW_MAX) { // Show mode
        switch_show_mode(app);
    } else if (app->mode > APP_MODE_SHOW_MAX) { // Settings mode
        switch (app->mode) {
            case APP_MODE_SETTINGS_TIME_HOUR:
                inc_hour(app);
                break;
            case APP_MODE_SETTINGS_TIME_MINUTE:
                inc_minute(app);
                break;
            case APP_MODE_SETTINGS_DATE_DAY:
                inc_day(app);
                break;
            case APP_MODE_SETTINGS_DATE_MONTH:
                inc_month(app);
                break;
            case APP_MODE_SETTINGS_DATE_DOW:
                inc_dow(app);
                break;
            case APP_MODE_SETTINGS_DATE_YEAR:
                inc_year(app);
                break;
            case APP_MODE_SETTINGS_ALARM_HOUR:
                inc_alarm_hour(app);
                break;
            case APP_MODE_SETTINGS_ALARM_MINUTE:
                inc_alarm_minute(app);
                break;
            case APP_MODE_SETTINGS_BRIGHTNESS:
                inc_screen_brightness(app);
                break;
            default:
                break;
        }
    }

    return true;
}

static bool btn_b_l_press(void *args) {
    app_t *app = (app_t *) args;

    if (app->time.alarm_started) {
        app_alarm_stop(app);
    }

    if (app->mode < APP_MODE_SHOW_MAX) {
        app->mode = APP_MODE_SETTINGS_MIN + 1;
        ets_printf("Enter settings mode: %d\n", app->mode);
    } else if (app->mode > APP_MODE_SHOW_MAX) {
        app->time.flush_to_rtc = true;
        app->mode = APP_MODE_SHOW_MIN + 1;
        ets_printf("Exit settings mode: %d\n", app->mode);
    }

    return false;
}

static bool btn_b_release(void *args) {
    app_t *app = (app_t *) args;

    if (app->time.alarm_started) {
        app_alarm_stop(app);
    }

    if (app->mode > APP_MODE_SHOW_MAX) {
        app->mode++;
        if (app->mode == APP_MODE_SETTINGS_MAX) {
            app->time.flush_to_rtc = true;
            app->mode = APP_MODE_SHOW_MIN + 1;
            ets_printf("Exit settings mode: %d\n", app->mode);
        } else {
            ets_printf("Next settings mode: %d\n", app->mode);
        }
    }

    return true;
}

static void show_mode_switcher(void *args) {
    app_t *app = (app_t *) args;
    int last_mode = app->mode;

    for (;;) {
        if (last_mode == APP_MODE_SHOW_TIME) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_TIME_DURATION));
        } else if (last_mode == APP_MODE_SHOW_DATE) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_DATE_DURATION));
        } else if (last_mode == APP_MODE_SHOW_DOW) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_DATE_DURATION));
        } else if (last_mode == APP_MODE_SHOW_AMBIENT_TEMP) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_AMBIENT_TEMP_DURATION));
        } else if (last_mode == APP_MODE_SHOW_WEATHER_TEMP && app->weather.update_ok) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_WEATHER_TEMP_DURATION));
        }

        // Switch only if mode hasn't been changed while this task was sleeping
        if (app->mode == last_mode) {
            switch_show_mode(app);
            last_mode = app->mode;
        } else if (app->mode < APP_MODE_SETTINGS_MIN) {
            last_mode = app->mode;
        }
    }
}

esp_err_t app_keyboard_init(app_t *app) {
    esp_err_t err;

    // Init
    err = aespl_button_init(&app->btn_a, APP_BTN_A, APP_BTN_A_MODE, true);
    if (err) {
        return err;
    }
    err = aespl_button_init(&app->btn_b, APP_BTN_B, APP_BTN_B_MODE, false);
    if (err) {
        return err;
    }

    // On long press
    err = aespl_button_on_l_press(&app->btn_a, btn_a_l_press, (void *) app);
    if (err) {
        return err;
    }
    err = aespl_button_on_l_press(&app->btn_b, btn_b_l_press, (void *) app);
    if (err) {
        return err;
    }

    // On release
    err = aespl_button_on_release(&app->btn_a, btn_a_release, (void *) app);
    if (err) {
        return err;
    }
    err = aespl_button_on_release(&app->btn_b, btn_b_release, (void *) app);
    if (err) {
        return err;
    }

    // Setup show mode switcher timer
    xTaskCreate(show_mode_switcher, "show_mode_switcher", 4096, (void *) app, 0, NULL);

    return ESP_OK;
}
