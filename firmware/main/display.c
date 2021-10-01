/**
 * @brief     Cronus Digital Clock Display Functions
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/adc.h"
#include "esp_err.h"
#include "esp_log.h"

#include "aespl_gfx_buffer.h"
#include "aespl_gfx_text.h"
#include "aespl_max7219.h"
#include "aespl_max7219_matrix.h"

#include "cronus_main.h"
#include "cronus_font_8_2.h"
#include "cronus_display.h"
#include "cronus_nvs.h"

/**
 * @brief Draws current time on the screen
 */
static void draw_time(app_t *app) {
    aespl_gfx_point_t pos = {0, 0};
    char s[4];

    // Prepare buffers:
    // 2-digit hour * 6px + 1px whitespace: 13px
    // 2-digit minute * 6px + 1px whitespace: 13px
    // separator: 2px
    aespl_gfx_buf_t *buf_h = aespl_gfx_make_buf(13, 8, AESPL_GFX_C_MODE_MONO);
    aespl_gfx_buf_t *buf_m = aespl_gfx_make_buf(13, 8, AESPL_GFX_C_MODE_MONO);
    aespl_gfx_buf_t *buf_sep = aespl_gfx_make_buf(2, 8, AESPL_GFX_C_MODE_MONO);

    // Draw hour
    if (app->mode >= APP_MODE_SETTINGS_ALARM_HOUR) {
        sprintf(s, "%02d", app->time.alarm_hour);
    } else {
        sprintf(s, "%02d", app->time.hour);
    }
    aespl_gfx_puts(buf_h, &font8_clock_2, pos, s, 1, 1);

    // Draw minute
    if (app->mode >= APP_MODE_SETTINGS_ALARM_HOUR) {
        sprintf(s, "%02d", app->time.alarm_minute);
    } else {
        sprintf(s, "%02d", app->time.minute);
    }
    aespl_gfx_puts(buf_m, &font8_clock_2, pos, s, 1, 1);

    // Draw separator
    aespl_gfx_set_px(buf_sep, 1, 1, 1);
    aespl_gfx_set_px(buf_sep, 0, 2, 1);
    aespl_gfx_set_px(buf_sep, 0, 6, 1);
    aespl_gfx_set_px(buf_sep, 1, 5, 1);

    // Double separator bottom dot height in alarm mode
    if (app->time.alarm_enabled || app->mode >= APP_MODE_SETTINGS_ALARM_HOUR) {
        aespl_gfx_set_px(buf_sep, 0, 1, 1);
        aespl_gfx_set_px(buf_sep, 1, 6, 1);
    }

    // Hour blink
    if ((app->mode == APP_MODE_SETTINGS_TIME_HOUR || app->mode == APP_MODE_SETTINGS_ALARM_HOUR) &&
        !app->time.sep_visible) {
        aespl_gfx_clear_buf(buf_h);
    }

    // Minute blink
    if ((app->mode == APP_MODE_SETTINGS_TIME_MINUTE || app->mode == APP_MODE_SETTINGS_ALARM_MINUTE) &&
        !app->time.sep_visible) {
        aespl_gfx_clear_buf(buf_m);
    }

    // Separator blink
    if (app->mode < APP_MODE_SHOW_MAX && !app->time.sep_visible) {
        aespl_gfx_clear_buf(buf_sep);
    }

    // Merge buffers
    aespl_gfx_merge(app->display.buf, buf_h, (aespl_gfx_point_t) {0, 0}, (aespl_gfx_point_t) {0, 0});
    aespl_gfx_merge(app->display.buf, buf_sep, (aespl_gfx_point_t) {15, 0}, (aespl_gfx_point_t) {0, 0});
    aespl_gfx_merge(app->display.buf, buf_m, (aespl_gfx_point_t) {19, 0}, (aespl_gfx_point_t) {0, 0});

    // Cleanup
    aespl_gfx_free_buf(buf_h);
    aespl_gfx_free_buf(buf_m);
    aespl_gfx_free_buf(buf_sep);
}

/**
 * @brief Draws current date on the screen
 */
static void draw_date(app_t *app) {
    aespl_gfx_point_t pos = {0, 0};
    char s[4];

    // Prepare buffers:
    // (2-digit day) * 6px + 1px whitespace: 13px
    // (2-digit month) * 6px + 1px whitespace: 13px
    // separator: 2px
    aespl_gfx_buf_t *buf_d = aespl_gfx_make_buf(13, 8, AESPL_GFX_C_MODE_MONO);
    aespl_gfx_buf_t *buf_m = aespl_gfx_make_buf(13, 8, AESPL_GFX_C_MODE_MONO);
    aespl_gfx_buf_t *buf_sep = aespl_gfx_make_buf(2, 8, AESPL_GFX_C_MODE_MONO);

    // Draw day
    sprintf(s, "%02d", app->time.day);
    aespl_gfx_puts(buf_d, &font8_clock_2, pos, s, 1, 1);

    // Draw month
    sprintf(s, "%02d", app->time.month);
    aespl_gfx_puts(buf_m, &font8_clock_2, pos, s, 1, 1);

    // Draw separator
    aespl_gfx_set_px(buf_sep, 0, 7, 1);
    aespl_gfx_set_px(buf_sep, 1, 7, 1);

    // Day blink in settings mode
    if (app->mode == APP_MODE_SETTINGS_DATE_DAY && !app->time.sep_visible) {
        aespl_gfx_clear_buf(buf_d);
    }

    // Month blink in settings mode
    if (app->mode == APP_MODE_SETTINGS_DATE_MONTH && !app->time.sep_visible) {
        aespl_gfx_clear_buf(buf_m);
    }

    // Merge buffers
    aespl_gfx_merge(app->display.buf, buf_d, (aespl_gfx_point_t) {0, 0}, (aespl_gfx_point_t) {0, 0});
    aespl_gfx_merge(app->display.buf, buf_sep, (aespl_gfx_point_t) {15, 0}, (aespl_gfx_point_t) {0, 0});
    aespl_gfx_merge(app->display.buf, buf_m, (aespl_gfx_point_t) {19, 0}, (aespl_gfx_point_t) {0, 0});

    // Cleanup
    aespl_gfx_free_buf(buf_d);
    aespl_gfx_free_buf(buf_m);
    aespl_gfx_free_buf(buf_sep);
}

static void draw_dow(app_t *app) {
    if (app->mode == APP_MODE_SETTINGS_DATE_DOW && !app->time.sep_visible) {
        // In settings mode numbers are blinking, so just keep buffer clear
    } else {
        char s[8];
        sprintf(s, "- %d -", app->time.dow + 1);
        aespl_gfx_puts(app->display.buf, &font8_clock_2, (aespl_gfx_point_t) {6, 0}, s, 1, 1);
    }
}

static void draw_year(app_t *app) {
    char s[6];

    // 6px * (4-digit year) + 1px * (3 whitespace) = 27px
    aespl_gfx_buf_t *buf_y = aespl_gfx_make_buf(27, 8, AESPL_GFX_C_MODE_MONO);

    // Draw year
    sprintf(s, "20%02d", app->time.year);
    aespl_gfx_puts(buf_y, &font8_clock_2, (aespl_gfx_point_t) {0, 0}, s, 1, 1);

    // Settings mode
    if (app->mode == APP_MODE_SETTINGS_DATE_YEAR && !app->time.sep_visible) {
        aespl_gfx_clear_buf(buf_y);
    }

    // Merge buffers
    aespl_gfx_merge(app->display.buf, buf_y, (aespl_gfx_point_t) {2, 0}, (aespl_gfx_point_t) {0, 0});

    // Cleanup
    aespl_gfx_free_buf(buf_y);
}

/**
 * @brief Draws maximum brightness level
 */
static void draw_max_brightness(app_t *app) {
    char s[6];
    sprintf(s, "$ %02d", app->display.max_brightness + 1);
    aespl_gfx_puts(app->display.buf, &font8_clock_2, (aespl_gfx_point_t) {4, 0}, s, 1, 1);
}

static void make_temperature_str(char sign, char *s, int temp) {
    if (temp < -9) {
        sprintf(s, "%c%d,", sign, temp);
    } else if (temp >= -9 && temp < 0) {
        sprintf(s, "%c %d,", sign, temp);
    } else if (temp >= 0 && temp < 10) {
        sprintf(s, "%c  %d,", sign, temp);
    } else { // >= 10
        sprintf(s, "%c %d,", sign, temp);
    }
}

/**
 * @brief Draws ambient temperature
 */
static void draw_ambient_temp(app_t *app) {
    char s[5];
    make_temperature_str('#', s, (int) round(app->ds3231.temp - 4));
    aespl_gfx_puts(app->display.buf, &font8_clock_2, (aespl_gfx_point_t) {0, 0}, s, 1, 1);
}

/**
 * @brief Draws current weather temperature
 */
static void draw_weather_temp(app_t *app) {
    char s[5];
    make_temperature_str('!', s, (int) round(app->weather.temp));
    aespl_gfx_puts(app->display.buf, &font8_clock_2, (aespl_gfx_point_t) {0, 0}, s, 1, 1);
}

/**
 * @brief Refresh display task
 */
static void refresh(void *args) {
    ESP_LOGI(APP_NAME, "display refresh task started");
    app_t *app = (app_t *) args;

    for (;;) {
        aespl_gfx_clear_buf(app->display.buf);

        // Update hour/minute separator state
        app->time.sep_visible = true;
        if (app->display.refresh_cnt++ > app->display.refresh_cnt_max / 2) {
            app->time.sep_visible = false;
        }
        if (app->display.refresh_cnt > app->display.refresh_cnt_max) {
            app->display.refresh_cnt = 0;
        }

        // Fill buffer
        switch (app->mode) {
            case APP_MODE_SHOW_TIME:
            case APP_MODE_SETTINGS_TIME_HOUR:
            case APP_MODE_SETTINGS_TIME_MINUTE:
            case APP_MODE_SETTINGS_ALARM_HOUR:
            case APP_MODE_SETTINGS_ALARM_MINUTE:
                draw_time(app);
                break;

            case APP_MODE_SHOW_DATE:
            case APP_MODE_SETTINGS_DATE_MONTH:
            case APP_MODE_SETTINGS_DATE_DAY:
                draw_date(app);
                break;

            case APP_MODE_SHOW_DOW:
            case APP_MODE_SETTINGS_DATE_DOW:
                draw_dow(app);
                break;

            case APP_MODE_SETTINGS_DATE_YEAR:
                draw_year(app);
                break;

            case APP_MODE_SHOW_AMBIENT_TEMP:
                draw_ambient_temp(app);
                break;

            case APP_MODE_SHOW_WEATHER_TEMP:
                draw_weather_temp(app);
                break;

            case APP_MODE_SETTINGS_BRIGHTNESS:
                draw_max_brightness(app);
                break;

            default:
                break;
        }

        // Output buffer to hardware
        switch (APP_HW_VERSION) {
            case APP_HW_VER_1_0:
            case APP_HW_VER_1_1:
                aespl_max7219_refresh(&app->display.max7219);
                aespl_max7219_matrix_draw(&app->display.max7219_matrix, app->display.buf);
                break;

            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(APP_SCREEN_REFRESH_RATE));
    }
}

static void brightness_regulator(void *args) {
    app_t *app = (app_t *) args;
    uint16_t data = 0;

    for (;;) {
        // Maximum brightness in settings mode to let user make estimation
        if (app->mode == APP_MODE_SETTINGS_BRIGHTNESS) {
            app->display.max7219.intensity = app->display.max_brightness;
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        // Store maximum brightness level to the NVS
        if (app->display.max_brightness_changed) {
            app_nvs_set_u8(app, CRONUS_NVS_K_MAX_BRIGHTNESS, app->display.max_brightness);
            app->display.max_brightness_changed = false;
        }

        adc_read(&data);

        app->display.brightness = data / 64;
        if (app->display.brightness < app->display.min_brightness) {
            app->display.brightness = app->display.min_brightness;
        } else if (app->display.brightness > app->display.max_brightness) {
            app->display.brightness = app->display.max_brightness;
        }

        app->display.max7219.intensity = app->display.brightness;
        vTaskDelay(pdMS_TO_TICKS(APP_SCREEN_BRIGHTNESS_REG_TIMEOUT));
    }
}

static void init_display_hw_ver_1(app_t *app) {
    app->display.buf = aespl_gfx_make_buf(APP_MAX7219_DISP_X * 8, APP_MAX7219_DISP_Y * 8, AESPL_GFX_C_MODE_MONO);

    ESP_ERROR_CHECK(aespl_max7219_init(
            &app->display.max7219,
            APP_MAX7219_PIN_CS,
            APP_MAX7219_PIN_CLK,
            APP_MAX7219_PIN_DATA,
            AESPL_MAX7219_DECODE_NONE,
            AESPL_MAX7219_INTENSITY_MIN,
            AESPL_MAX7219_SCAN_LIMIT_8,
            AESPL_MAX7219_POWER_ON,
            AESPL_MAX7219_TEST_MODE_DISABLE,
            APP_MAX7219_DISP_X * APP_MAX7219_DISP_Y
    ));

    ESP_ERROR_CHECK(aespl_max7219_matrix_init(
            &app->display.max7219_matrix,
            &app->display.max7219,
            APP_MAX7219_DISP_X,
            APP_MAX7219_DISP_Y,
            APP_MAX7219_DISP_REVERSE
    ));
}

esp_err_t app_display_init(app_t *app) {
    int err;

    app->display.refresh_cnt = 0;
    app->display.refresh_cnt_max = 1000 / APP_SCREEN_REFRESH_RATE - 1;

    app_nvs_get_u8(app, CRONUS_NVS_K_MAX_BRIGHTNESS, &app->display.max_brightness);
    if (app->display.max_brightness > APP_SCREEN_MAX_BRIGHTNESS) {
        app->display.max_brightness = APP_SCREEN_MAX_BRIGHTNESS;
    }

    // Initialize display
    switch (APP_HW_VERSION) {
        case APP_HW_VER_1_0:
        case APP_HW_VER_1_1:
            init_display_hw_ver_1(app);
            break;

        default:
            ESP_LOGE(APP_NAME, "unknown hardware version %d", APP_HW_VERSION);
            return ESP_FAIL;
    }

    // ADC for ambient light measurement
    adc_config_t adc = {
            .mode = ADC_READ_TOUT_MODE,
    };
    err = adc_init(&adc);
    if (err) {
        return err;
    }

    xTaskCreate(refresh, "display_refresh", 4096, (void *) app, 0, NULL);
    xTaskCreate(brightness_regulator, "brightness_regulator", 4096, (void *) app, 0, NULL);

    ESP_LOGI(APP_NAME, "display initialized");

    return ESP_OK;
}
