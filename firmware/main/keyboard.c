/**
 * @brief     Cronus Digital Clock Keyboard Functions
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include "cronus/keyboard.h"

#include "FreeRTOS.h"
#include "aespl/button.h"
#include "cronus/alarm.h"
#include "cronus/display.h"
#include "cronus/dtime.h"
#include "cronus/main.h"
#include "cronus/weather.h"
#include "esp_err.h"
#include "esp_log.h"
#include "timers.h"

static void switch_show_mode(app_keyboard_t *kb) {
    // Don't switch while app is in settings mode
    if (*kb->app_mode > APP_MODE_SETTINGS_MIN) {
        return;
    }

    (*kb->app_mode)++;

    // Will work in version 1.2, skip for now
    if (*kb->app_mode == APP_MODE_SHOW_AMBIENT_TEMP) {
        (*kb->app_mode)++;
    }

    // Don't show weather temperature if it wasn't received properly
    if (*kb->app_mode == APP_MODE_SHOW_WEATHER_TEMP &&
        !kb->weather->update_ok) {
        (*kb->app_mode)++;
    }

    // Reached last show mode, need to start from the first one
    if (*kb->app_mode == APP_MODE_SHOW_MAX) {
        *kb->app_mode = APP_MODE_SHOW_MIN + 1;
    }

    // Adjust automatic show mode switcher
    switch (*kb->app_mode) {
        case APP_MODE_SHOW_TIME:
            xTimerChangePeriod(kb->app_mode_timer,
                               pdMS_TO_TICKS(APP_SHOW_TIME_DURATION), 0);
            break;
        case APP_MODE_SHOW_DATE:
        case APP_MODE_SHOW_DOW:
            xTimerChangePeriod(kb->app_mode_timer,
                               pdMS_TO_TICKS(APP_SHOW_DATE_DURATION), 0);
            xTimerChangePeriod(kb->app_mode_timer,
                               pdMS_TO_TICKS(APP_SHOW_DATE_DURATION), 0);
            break;
        case APP_MODE_SHOW_WEATHER_TEMP:
        case APP_MODE_SHOW_AMBIENT_TEMP:
            xTimerChangePeriod(kb->app_mode_timer,
                               pdMS_TO_TICKS(APP_SHOW_TEMP_DURATION), 0);
            break;
        default:
            break;
    }
}

static void inc_setting_hour(app_keyboard_t *kb) {
    kb->display->refresh_cnt = 0;
    kb->time->hour++;
    if (kb->time->hour > 23) {
        kb->time->hour = 0;
    }
}

static void inc_setting_alarm_hour(app_keyboard_t *kb) {
    kb->display->refresh_cnt = 0;
    kb->time->alarm_hour++;
    if (kb->time->alarm_hour > 23) {
        kb->time->alarm_hour = 0;
    }
}

static void inc_setting_minute(app_keyboard_t *kb) {
    kb->display->refresh_cnt = 0;
    kb->time->minute++;
    if (kb->time->minute > 59) {
        kb->time->minute = 0;
    }
}

static void inc_setting_alarm_minute(app_keyboard_t *kb) {
    kb->display->refresh_cnt = 0;
    kb->time->alarm_minute++;
    if (kb->time->alarm_minute > 59) {
        kb->time->alarm_minute = 0;
    }
}

static void inc_setting_day(app_keyboard_t *kb) {
    kb->display->refresh_cnt = 0;
    kb->time->day++;
    if (kb->time->day > 31) {
        kb->time->day = 1;
    }
}

static void inc_setting_month(app_keyboard_t *kb) {
    kb->display->refresh_cnt = 0;
    kb->time->month++;
    if (kb->time->month > 12) {
        kb->time->month = 1;
    }
}

static void inc_setting_dow(app_keyboard_t *kb) {
    kb->display->refresh_cnt = 0;
    kb->time->dow++;
    if (kb->time->dow > 6) {
        kb->time->dow = 0;
    }
}

static void inc_setting_year(app_keyboard_t *kb) {
    kb->display->refresh_cnt = 0;
    kb->time->year++;
    if (kb->time->year > 99) {
        kb->time->year = 21;
    }
}

static void inc_setting_brightness(app_keyboard_t *kb) {
    kb->display->refresh_cnt = 0;
    kb->display->max_brightness++;
    kb->display->max_brightness_changed = true;
    if (kb->display->max_brightness > APP_DISPLAY_MAX_BRIGHTNESS) {
        kb->display->max_brightness = 0;
    }
}

static bool btn_a_l_press(void *args) {
    app_keyboard_t *kb = (app_keyboard_t *)args;

    if (kb->time->alarm_started) {
        app_alarm_stop(kb->time);
    }

    switch (*kb->app_mode) {
        case APP_MODE_SHOW_TIME:
            kb->time->alarm_enabled = !kb->time->alarm_enabled;
            nvs_set_u8(kb->nvs, "alarm_en", kb->time->alarm_enabled);
            app_alarm_beep();
            return false;
        case APP_MODE_SETTINGS_TIME_HOUR:
            inc_setting_hour(kb);
            break;
        case APP_MODE_SETTINGS_TIME_MINUTE:
            inc_setting_minute(kb);
            break;
        case APP_MODE_SETTINGS_DATE_DAY:
            inc_setting_day(kb);
            break;
        case APP_MODE_SETTINGS_DATE_MONTH:
            inc_setting_month(kb);
            break;
        case APP_MODE_SETTINGS_DATE_DOW:
            inc_setting_dow(kb);
            break;
        case APP_MODE_SETTINGS_DATE_YEAR:
            inc_setting_year(kb);
            break;
        case APP_MODE_SETTINGS_ALARM_HOUR:
            inc_setting_alarm_hour(kb);
            break;
        case APP_MODE_SETTINGS_ALARM_MINUTE:
            inc_setting_alarm_minute(kb);
            break;
        case APP_MODE_SETTINGS_BRIGHTNESS:
            inc_setting_brightness(kb);
            break;
        default:
            break;
    }

    return true;
}

static bool btn_a_release(void *args) {
    app_keyboard_t *kb = (app_keyboard_t *)args;

    if (kb->time->alarm_started) {
        app_alarm_stop(kb->time);
    }

    if (*kb->app_mode < APP_MODE_SHOW_MAX) {  // Show mode
        switch_show_mode(kb);
    } else if (*kb->app_mode > APP_MODE_SHOW_MAX) {  // Settings mode
        switch (*kb->app_mode) {
            case APP_MODE_SETTINGS_TIME_HOUR:
                inc_setting_hour(kb);
                break;
            case APP_MODE_SETTINGS_TIME_MINUTE:
                inc_setting_minute(kb);
                break;
            case APP_MODE_SETTINGS_DATE_DAY:
                inc_setting_day(kb);
                break;
            case APP_MODE_SETTINGS_DATE_MONTH:
                inc_setting_month(kb);
                break;
            case APP_MODE_SETTINGS_DATE_DOW:
                inc_setting_dow(kb);
                break;
            case APP_MODE_SETTINGS_DATE_YEAR:
                inc_setting_year(kb);
                break;
            case APP_MODE_SETTINGS_ALARM_HOUR:
                inc_setting_alarm_hour(kb);
                break;
            case APP_MODE_SETTINGS_ALARM_MINUTE:
                inc_setting_alarm_minute(kb);
                break;
            case APP_MODE_SETTINGS_BRIGHTNESS:
                inc_setting_brightness(kb);
                break;
            default:
                break;
        }
    }

    return true;
}

static bool btn_b_l_press(void *args) {
    app_keyboard_t *kb = (app_keyboard_t *)args;

    if (kb->time->alarm_started) {
        app_alarm_stop(kb->time);
    }

    if (*kb->app_mode < APP_MODE_SHOW_MAX) {
        *kb->app_mode = APP_MODE_SETTINGS_MIN + 1;  // enter settings mode
    } else if (*kb->app_mode > APP_MODE_SHOW_MAX) {
        kb->time->flush_to_rtc = true;
        *kb->app_mode = APP_MODE_SHOW_MIN + 1;  // exit from settings mode
    }

    return false;
}

static bool btn_b_release(void *args) {
    app_keyboard_t *kb = (app_keyboard_t *)args;

    if (kb->time->alarm_started) {
        app_alarm_stop(kb->time);
    }

    if (*kb->app_mode > APP_MODE_SHOW_MAX) {
        // Switch to the next settings mode
        (*kb->app_mode)++;

        // Exit from settings mode if reached the last one
        if (*kb->app_mode == APP_MODE_SETTINGS_MAX) {
            kb->time->flush_to_rtc = true;
            *kb->app_mode = APP_MODE_SHOW_MIN + 1;
        }
    }

    return true;
}

static void show_mode_switcher(TimerHandle_t timer) {
    switch_show_mode((app_keyboard_t *)pvTimerGetTimerID(timer));
}

app_keyboard_t *app_keyboard_init(app_mode_t *mode, app_time_t *time,
                                  app_display_t *display,
                                  app_weather_t *weather, nvs_handle_t nvs) {
    esp_err_t err;

    err = gpio_install_isr_service(0);
    if (err != ESP_OK) {
        ESP_LOGE(APP_NAME, "gpio_install_isr_service() failed");
        return NULL;
    }

    app_keyboard_t *kb = malloc(sizeof(app_keyboard_t));
    if (kb == NULL) {
        ESP_LOGE(APP_NAME, "failed to allocate memory for the keyboard");
        return NULL;
    }

    kb->mux = xSemaphoreCreateBinary();
    if (kb->mux == NULL) {
        free(kb);
        ESP_LOGE(APP_NAME, "failed to allocate a semaphore for the keyboard");
        return NULL;
    }
    xSemaphoreGive(kb->mux);

    kb->app_mode = mode;
    kb->time = time;
    kb->display = display;
    kb->nvs = nvs;
    kb->weather = weather;

    // Init
    err = aespl_button_init(&kb->btn_a, APP_BTN_A, APP_BTN_A_MODE, true);
    if (err != ESP_OK) {
        free(kb);
        ESP_LOGE(APP_NAME, "failed to init keyboard button 'A'; err=%d", err);
        return NULL;
    }
    err = aespl_button_init(&kb->btn_b, APP_BTN_B, APP_BTN_B_MODE, false);
    if (err != ESP_OK) {
        free(kb);
        ESP_LOGE(APP_NAME, "failed to init keyboard button 'B'; err=%d", err);
        return NULL;
    }

    // On long press
    err = aespl_button_on_l_press(&kb->btn_a, btn_a_l_press, (void *)kb);
    if (err != ESP_OK) {
        free(kb);
        ESP_LOGE(
            APP_NAME,
            "failed to init keyboard button 'A' long press handler; err=%d",
            err);
        return NULL;
    }
    err = aespl_button_on_l_press(&kb->btn_b, btn_b_l_press, (void *)kb);
    if (err != ESP_OK) {
        free(kb);
        ESP_LOGE(
            APP_NAME,
            "failed to init keyboard button 'B' long press handler; err=%d",
            err);
        return NULL;
    }

    // On release
    err = aespl_button_on_release(&kb->btn_a, btn_a_release, (void *)kb);
    if (err != ESP_OK) {
        free(kb);
        ESP_LOGE(APP_NAME,
                 "failed to init keyboard button 'A' release handler; err=%d",
                 err);
        return NULL;
    }
    err = aespl_button_on_release(&kb->btn_b, btn_b_release, (void *)kb);
    if (err != ESP_OK) {
        free(kb);
        ESP_LOGE(APP_NAME,
                 "failed to init keyboard button 'B' release handler; err=%d",
                 err);
        return NULL;
    }

    // Setup time based automatic show mode switcher
    kb->app_mode_timer =
        xTimerCreate("mode_switch", pdMS_TO_TICKS(APP_SHOW_TIME_DURATION),
                     pdTRUE, (void *)kb, show_mode_switcher);
    if (kb->app_mode_timer == NULL) {
        free(kb);
        ESP_LOGE(APP_NAME, "failed to init automatic mode switcher");
        return NULL;
    }
    if (xTimerStart(kb->app_mode_timer, 0) != pdPASS) {
        free(kb);
        ESP_LOGE(APP_NAME, "failed to start automatic mode switcher");
        return NULL;
    }
    ESP_LOGI(APP_NAME, "automatic mode switcher initialized");

    ESP_LOGI(APP_NAME, "keyboard initialized");
    return kb;
}
