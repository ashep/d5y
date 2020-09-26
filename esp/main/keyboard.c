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

static void btn_a_l_press(void *args) {
    app_t *app = (app_t *)args;

    ets_printf("'A' long pressed %d\n", app->mode);
}

static void switch_show_mode(app_t *app) {
    if (app->mode + 1 == APP_MODE_SHOW_MAX) {
        app->mode = APP_MODE_SHOW_MIN + 1;
    } else {
        app->mode++;
    }

    ets_printf("Show mode switched: %d\n", app->mode);
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
                ets_printf("HOUR+\n");
                break;
            case APP_MODE_SETTINGS_TIME_MINUTE:
                ets_printf("MINUTE+\n");
                break;
            case APP_MODE_SETTINGS_DATE_YEAR:
                ets_printf("YEAR+\n");
                break;
            case APP_MODE_SETTINGS_DATE_MONTH:
                ets_printf("MONTH+\n");
                break;
            case APP_MODE_SETTINGS_DATE_DAY:
                ets_printf("DAY+\n");
                break;
            case APP_MODE_SETTINGS_DATE_DOW:
                ets_printf("DOW+\n");
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
        ets_printf("Exit settings mode: %d\n", app->mode);
    }
}

static void btn_b_release(void *args) {
    app_t *app = (app_t *)args;

    if (app->mode > APP_MODE_SHOW_MAX) {
        app->mode++;
        if (app->mode == APP_MODE_SETTINGS_MAX) {
            app->mode = APP_MODE_SETTINGS_TIME_HOUR;
        }
        ets_printf("Next settings mode: %d\n", app->mode);
    }
}

static void show_mode_switcher(void *args) {
    app_t *app = (app_t *)args;

    for (;;) {
        if (app->mode == APP_MODE_SHOW_TIME) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_TIME_DURATION * 1000));
        }
        else if (app->mode == APP_MODE_SHOW_DATE) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_DATE_DURATION * 1000));
        }
        else if (app->mode == APP_MODE_SHOW_WEATHER_TEMP) {
            vTaskDelay(pdMS_TO_TICKS(APP_SHOW_WEATHER_TEMP_DURATION * 1000));
        }
        switch_show_mode(app);
    }

    vTaskDelete(NULL);
}

esp_err_t app_buttons_init(app_t *app) {
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
    xTaskCreate(show_mode_switcher, "show_mode_switcher", 4096, (void *)app, 0, NULL);

    return ESP_OK;
}
