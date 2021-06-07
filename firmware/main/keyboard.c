/**
 * @brief     Cronus Digital Clock Keyboard Functions
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_err.h"
#include "aespl_button.h"
#include "app_main.h"

static void switch_show_mode(app_t *app) {
    app->mode++;

    if (app->mode == APP_MODE_SHOW_WEATHER_TEMP && !app->weather.update_ok) {
        app->mode++;
    }

    if (app->mode == APP_MODE_SHOW_MAX) {
        app->mode = APP_MODE_SHOW_MIN + 1;
    }

    ets_printf("Show mode switched: %d\n", app->mode);
}

static void inc_hour(app_t *app) {
    app->display_refresh_cnt = 0;
    app->time.hour++;
    if (app->time.hour > 23) {
        app->time.hour = 0;
    }
}

static void inc_minute(app_t *app) {
    app->display_refresh_cnt = 0;
    app->time.minute++;
    if (app->time.minute > 59) {
        app->time.minute = 0;
    }
}

static void inc_day(app_t *app) {
    app->display_refresh_cnt = 0;
    app->time.day++;
    if (app->time.day > 31) {
        app->time.day = 1;
    }
}

static void inc_month(app_t *app) {
    app->display_refresh_cnt = 0;
    app->time.month++;
    if (app->time.month > 12) {
        app->time.month = 1;
    }
}

static void btn_a_l_press(void *args) {
    app_t *app = (app_t *)args;
    ets_printf("'A' long pressed %d\n", app->mode);

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
        default:
            break;
    }
}

static void btn_a_release(void *args) {
    app_t *app = (app_t *)args;

    // Show mode
    if (app->mode < APP_MODE_SHOW_MAX) {
        switch_show_mode(app);
    }
    // Settings mode
    else if (app->mode > APP_MODE_SHOW_MAX) {
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
                ets_printf("DOW+\n");
                break;
            case APP_MODE_SETTINGS_DATE_YEAR:
                ets_printf("YEAR+\n");
                break;
            default:
                break;
        }
    }
}

static void btn_b_l_press(void *args) {
    app_t *app = (app_t *)args;

    if (app->mode < APP_MODE_SHOW_MAX) {
        app->mode = APP_MODE_SETTINGS_MIN + 1;
        ets_printf("Enter settings mode: %d\n", app->mode);
    } else if (app->mode > APP_MODE_SHOW_MAX) {
        app->mode = APP_MODE_SHOW_MIN + 1;
        set_rtc_from_app(app);
        ets_printf("Exit settings mode: %d\n", app->mode);
    }
}

static void btn_b_release(void *args) {
    app_t *app = (app_t *)args;

    if (app->mode > APP_MODE_SHOW_MAX) {
        app->mode++;
        if (app->mode == APP_MODE_SETTINGS_MAX) {
            app->mode = APP_MODE_SHOW_MIN + 1;
            ets_printf("Exit settings mode: %d\n", app->mode);
        } else {
            ets_printf("Next settings mode: %d\n", app->mode);
        }
    }
}

static void show_mode_switcher(void *args) {
    app_t *app = (app_t *)args;

    for (;;) {
        if (app->mode == APP_MODE_SHOW_TIME) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_TIME_DURATION * APP_SECOND));
        } else if (app->mode == APP_MODE_SHOW_DATE) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_DATE_DURATION * APP_SECOND));
        } else if (app->mode == APP_MODE_SHOW_AMBIENT_TEMP) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_AMBIENT_TEMP_DURATION * APP_SECOND));
        } else if (app->mode == APP_MODE_SHOW_WEATHER_TEMP) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_WEATHER_TEMP_DURATION * APP_SECOND));
        }
        switch_show_mode(app);
    }

    vTaskDelete(NULL);
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
    err = aespl_button_on_l_press(&app->btn_a, btn_a_l_press, (void *)app);
    if (err) {
        return err;
    }
    err = aespl_button_on_l_press(&app->btn_b, btn_b_l_press, (void *)app);
    if (err) {
        return err;
    }

    // On release
    err = aespl_button_on_release(&app->btn_a, btn_a_release, (void *)app);
    if (err) {
        return err;
    }
    err = aespl_button_on_release(&app->btn_b, btn_b_release, (void *)app);
    if (err) {
        return err;
    }

    // Setup show mode toggler timer
    // xTaskCreate(show_mode_switcher, "show_mode_switcher", 4096, (void *)app, 0, NULL);

    return ESP_OK;
}
