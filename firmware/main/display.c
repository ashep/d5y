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

#include "aespl_gfx.h"
#include "aespl_gfx_buffer.h"
#include "aespl_gfx_text.h"
#include "aespl_gfx_font_1.h"
#include "aespl_gfx_animation.h"
#include "aespl_max7219.h"
#include "aespl_max7219_matrix.h"

#include "cronus/main.h"
#include "cronus/font_8_2.h"
#include "cronus/display.h"
#include "cronus/weather.h"

/**
 * @brief Draws current time on the screen
 */
static void draw_time(app_display_t *display) {
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
    if (*display->mode >= APP_MODE_SETTINGS_ALARM_HOUR) {
        sprintf(s, "%02d", display->time->alarm_hour);
    } else {
        sprintf(s, "%02d", display->time->hour);
    }
    aespl_gfx_puts(buf_h, &font8_clock_2, pos, s, 1, 1);

    // Draw minute
    if (*display->mode >= APP_MODE_SETTINGS_ALARM_HOUR) {
        sprintf(s, "%02d", display->time->alarm_minute);
    } else {
        sprintf(s, "%02d", display->time->minute);
    }
    aespl_gfx_puts(buf_m, &font8_clock_2, pos, s, 1, 1);

    // Draw separator
    aespl_gfx_set_px(buf_sep, 1, 1, 1);
    aespl_gfx_set_px(buf_sep, 0, 2, 1);
    aespl_gfx_set_px(buf_sep, 0, 6, 1);
    aespl_gfx_set_px(buf_sep, 1, 5, 1);

    // Double separator bottom dot height in alarm mode
    if (display->time->alarm_enabled || *display->mode >= APP_MODE_SETTINGS_ALARM_HOUR) {
        aespl_gfx_set_px(buf_sep, 0, 1, 1);
        aespl_gfx_set_px(buf_sep, 1, 6, 1);
    }

    // Hour blink
    if ((*display->mode == APP_MODE_SETTINGS_TIME_HOUR || *display->mode == APP_MODE_SETTINGS_ALARM_HOUR) &&
        !display->sep_visible) {
        aespl_gfx_clear_buf(buf_h);
    }

    // Minute blink
    if ((*display->mode == APP_MODE_SETTINGS_TIME_MINUTE || *display->mode == APP_MODE_SETTINGS_ALARM_MINUTE) &&
        !display->sep_visible) {
        aespl_gfx_clear_buf(buf_m);
    }

    // Separator blink
    if (*display->mode < APP_MODE_SHOW_MAX && !display->sep_visible) {
        aespl_gfx_clear_buf(buf_sep);
    }

    // Merge buffers
    aespl_gfx_merge(display->buf, buf_h, (aespl_gfx_point_t) {0, 0}, (aespl_gfx_point_t) {0, 0});
    aespl_gfx_merge(display->buf, buf_sep, (aespl_gfx_point_t) {15, 0}, (aespl_gfx_point_t) {0, 0});
    aespl_gfx_merge(display->buf, buf_m, (aespl_gfx_point_t) {19, 0}, (aespl_gfx_point_t) {0, 0});

    // Cleanup
    aespl_gfx_free_buf(buf_h);
    aespl_gfx_free_buf(buf_m);
    aespl_gfx_free_buf(buf_sep);
}

/**
 * @brief Draws current date on the screen
 */
static void draw_date(app_display_t *display) {
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
    sprintf(s, "%02d", display->time->day);
    aespl_gfx_puts(buf_d, &font8_clock_2, pos, s, 1, 1);

    // Draw month
    sprintf(s, "%02d", display->time->month);
    aespl_gfx_puts(buf_m, &font8_clock_2, pos, s, 1, 1);

    // Draw separator
    aespl_gfx_set_px(buf_sep, 0, 7, 1);
    aespl_gfx_set_px(buf_sep, 1, 7, 1);

    // Day blink in settings mode
    if (*display->mode == APP_MODE_SETTINGS_DATE_DAY && !display->sep_visible) {
        aespl_gfx_clear_buf(buf_d);
    }

    // Month blink in settings mode
    if (*display->mode == APP_MODE_SETTINGS_DATE_MONTH && !display->sep_visible) {
        aespl_gfx_clear_buf(buf_m);
    }

    // Merge buffers
    aespl_gfx_merge(display->buf, buf_d, (aespl_gfx_point_t) {0, 0}, (aespl_gfx_point_t) {0, 0});
    aespl_gfx_merge(display->buf, buf_sep, (aespl_gfx_point_t) {15, 0}, (aespl_gfx_point_t) {0, 0});
    aespl_gfx_merge(display->buf, buf_m, (aespl_gfx_point_t) {19, 0}, (aespl_gfx_point_t) {0, 0});

    // Cleanup
    aespl_gfx_free_buf(buf_d);
    aespl_gfx_free_buf(buf_m);
    aespl_gfx_free_buf(buf_sep);
}

static void draw_dow(app_display_t *display) {
    if (*display->mode == APP_MODE_SETTINGS_DATE_DOW && !display->sep_visible) {
        // In settings mode numbers are blinking, so just keep buffer clear
    } else {
        char s[8];
        sprintf(s, "- %d -", display->time->dow + 1);
        aespl_gfx_puts(display->buf, &font8_clock_2, (aespl_gfx_point_t) {6, 0}, s, 1, 1);
    }
}

static void draw_year(app_display_t *display) {
    char s[6];

    // 6px * (4-digit year) + 1px * (3 whitespace) = 27px
    aespl_gfx_buf_t *buf_y = aespl_gfx_make_buf(27, 8, AESPL_GFX_C_MODE_MONO);

    // Draw year
    sprintf(s, "20%02d", display->time->year);
    aespl_gfx_puts(buf_y, &font8_clock_2, (aespl_gfx_point_t) {0, 0}, s, 1, 1);

    // Settings mode
    if (*display->mode == APP_MODE_SETTINGS_DATE_YEAR && !display->sep_visible) {
        aespl_gfx_clear_buf(buf_y);
    }

    // Merge buffers
    aespl_gfx_merge(display->buf, buf_y, (aespl_gfx_point_t) {2, 0}, (aespl_gfx_point_t) {0, 0});

    // Cleanup
    aespl_gfx_free_buf(buf_y);
}

/**
 * @brief Draws maximum brightness level
 */
static void draw_max_brightness(app_display_t *display) {
    char s[6];
    sprintf(s, "$ %02d", display->max_brightness + 1);
    aespl_gfx_puts(display->buf, &font8_clock_2, (aespl_gfx_point_t) {4, 0}, s, 1, 1);
}

static void make_temperature_str(char sign, char *s, int temp) {
    if (temp < -9) {
        sprintf(s, "%c%d,", sign, temp);
    } else if (temp >= -9 && temp < 0) {
        sprintf(s, "%c %d,", sign, temp);
    } else if (temp >= 0 && temp < 10) {
        sprintf(s, "%c   %d,", sign, temp);
    } else { // >= 10
        sprintf(s, "%c %d,", sign, temp);
    }
}

/**
 * @brief Draws ambient temperature
 */
//static void draw_ambient_temp(app_display_t *display) {
//    char s[5];
//    make_temperature_str('#', s, (int) round(display->ds3231.temp - 4));
//    aespl_gfx_puts(display->buf, &font8_clock_2, (aespl_gfx_point_t) {0, 0}, s, 1, 1);
//}

/**
 * @brief Draws current weather temperature
 */
static void draw_weather_temp(app_display_t *display) {
    char s[5];
    make_temperature_str('!', s, (int) round(display->weather->temp));
    aespl_gfx_puts(display->buf, &font8_clock_2, (aespl_gfx_point_t) {0, 0}, s, 1, 1);
}

/**
 * @brief Refresh display task
 */
static void refresh(void *args) {
    app_display_t *d = (app_display_t *) args;

    for (;;) {
        aespl_gfx_clear_buf(d->buf);

        // Update hour/minute separator state
        d->sep_visible = true;
        if (d->refresh_cnt++ > d->refresh_cnt_max / 2) {
            d->sep_visible = false;
        }
        if (d->refresh_cnt > d->refresh_cnt_max) {
            d->refresh_cnt = 0;
        }

        // Fill buffer
        switch (*d->mode) {
            case APP_MODE_SHOW_TIME:
            case APP_MODE_SETTINGS_TIME_HOUR:
            case APP_MODE_SETTINGS_TIME_MINUTE:
            case APP_MODE_SETTINGS_ALARM_HOUR:
            case APP_MODE_SETTINGS_ALARM_MINUTE:
                draw_time(d);
                break;

            case APP_MODE_SHOW_DATE:
            case APP_MODE_SETTINGS_DATE_MONTH:
            case APP_MODE_SETTINGS_DATE_DAY:
                draw_date(d);
                break;

            case APP_MODE_SHOW_DOW:
            case APP_MODE_SETTINGS_DATE_DOW:
                draw_dow(d);
                break;

            case APP_MODE_SETTINGS_DATE_YEAR:
                draw_year(d);
                break;

//            case APP_MODE_SHOW_AMBIENT_TEMP:
//                draw_ambient_temp(display);
//                break;

            case APP_MODE_SHOW_WEATHER_TEMP:
                draw_weather_temp(d);
                break;

            case APP_MODE_SETTINGS_BRIGHTNESS:
                draw_max_brightness(d);
                break;

            default:
                break;
        }

        // Output buffer to hardware
        switch (APP_HW_VERSION) {
            case APP_HW_VER_1_0:
            case APP_HW_VER_1_1:
                aespl_max7219_refresh(&d->max7219);
                aespl_max7219_matrix_draw(&d->max7219_matrix, d->buf);
                break;

            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(APP_DISPLAY_REFRESH_RATE));
    }
}

static void brightness_regulator(void *args) {
    app_display_t *display = (app_display_t *) args;
    uint16_t data = 0;

    for (;;) {
        // Maximum brightness in settings mode to let user make estimation
        if (*display->mode == APP_MODE_SETTINGS_BRIGHTNESS) {
            display->max7219.intensity = display->max_brightness;
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        // Store maximum brightness level to the NVS
        if (display->max_brightness_changed) {
            nvs_set_u8(display->nvs, "scr_max_bri", display->max_brightness);
            display->max_brightness_changed = false;
        }

        adc_read(&data);

        display->brightness = data / 64;
        if (display->brightness < display->min_brightness) {
            display->brightness = display->min_brightness;
        } else if (display->brightness > display->max_brightness) {
            display->brightness = display->max_brightness;
        }

        display->max7219.intensity = display->brightness;
        vTaskDelay(pdMS_TO_TICKS(APP_DISPLAY_BRIGHTNESS_REG_TIMEOUT));
    }
}

static aespl_gfx_anim_state_t splash_screen_animation(void *args, uint32_t frame_n) {
    app_display_t *d = (app_display_t *) args;

    aespl_gfx_clear_buf(d->buf);
    aespl_gfx_puts(d->buf, &font_1, (aespl_gfx_point_t) {32 - frame_n, 0}, d->splash_screen_text, 1, 1);
    aespl_max7219_matrix_draw(&d->max7219_matrix, d->buf);

    if (frame_n < 70) {
        return AESPL_GFX_ANIM_CONTINUE;
    }

    return AESPL_GFX_ANIM_STOP;
}

static void splash_screen(app_display_t *display) {
    aespl_gfx_animate(splash_screen_animation, (void *) display, 15);
    vTaskDelay(pdMS_TO_TICKS(6000));

    for (uint8_t x = 0; x < 32; x = x + 2) {
        for (uint8_t y = 0; y < 8; y = y + 2) {
            aespl_gfx_set_px(display->buf, x, y, 1);
        }
    }
    aespl_max7219_matrix_draw(&display->max7219_matrix, display->buf);
    vTaskDelay(pdMS_TO_TICKS(100));

    for (uint8_t x = 1; x < 32; x = x + 2) {
        for (uint8_t y = 1; y < 8; y = y + 2) {
            aespl_gfx_set_px(display->buf, x, y, 1);
        }
    }
    aespl_max7219_matrix_draw(&display->max7219_matrix, display->buf);
    vTaskDelay(pdMS_TO_TICKS(100));

    for (uint8_t x = 1; x < 32; x = x + 2) {
        for (uint8_t y = 0; y < 8; y = y + 2) {
            aespl_gfx_set_px(display->buf, x, y, 1);
        }
    }
    aespl_max7219_matrix_draw(&display->max7219_matrix, display->buf);
    vTaskDelay(pdMS_TO_TICKS(100));

    for (uint8_t x = 0; x < 32; x = x + 2) {
        for (uint8_t y = 1; y < 8; y = y + 2) {
            aespl_gfx_set_px(display->buf, x, y, 1);
        }
    }
    aespl_max7219_matrix_draw(&display->max7219_matrix, display->buf);
    vTaskDelay(pdMS_TO_TICKS(100));

    for (int i = APP_DISPLAY_MIN_BRIGHTNESS; i <= APP_DISPLAY_MAX_BRIGHTNESS; i++) {
        display->max7219.intensity = i;
        aespl_max7219_refresh(&display->max7219);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    for (int i = APP_DISPLAY_MAX_BRIGHTNESS; i >= APP_DISPLAY_MIN_BRIGHTNESS; i--) {
        display->max7219.intensity = i;
        aespl_max7219_refresh(&display->max7219);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    aespl_gfx_clear_buf(display->buf);
    aespl_max7219_matrix_draw(&display->max7219_matrix, display->buf);
}

static void init_display_hw_ver_1(app_display_t *display) {
    display->buf = aespl_gfx_make_buf(APP_MAX7219_DISP_X * 8, APP_MAX7219_DISP_Y * 8, AESPL_GFX_C_MODE_MONO);

    ESP_ERROR_CHECK(aespl_max7219_init(
            &display->max7219,
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
            &display->max7219_matrix,
            &display->max7219,
            APP_MAX7219_DISP_X,
            APP_MAX7219_DISP_Y,
            APP_MAX7219_DISP_REVERSE
    ));
}

app_display_t *app_display_init(app_mode_t *mode, app_time_t *time, app_weather_t *weather, nvs_handle_t nvs) {
    int err;
    app_display_t *d = NULL;

    d = malloc(sizeof(app_display_t));
    if (d == NULL) {
        ESP_LOGE(APP_NAME, "failed to allocate memory for display");
        return NULL;
    }
    memset(d, 0, sizeof(app_display_t));

    d->mux = xSemaphoreCreateBinary();
    if (d->mux == NULL) {
        free(d);
        ESP_LOGE(APP_NAME, "failed to allocate a semaphore for display");
        return NULL;
    }
    xSemaphoreGive(d->mux);

    d->mode = mode;
    d->nvs = nvs;
    d->time = time;
    d->weather = weather;
    d->refresh_cnt = 0;
    d->refresh_cnt_max = 1000 / APP_DISPLAY_REFRESH_RATE - 1;
    d->min_brightness = APP_DISPLAY_MIN_BRIGHTNESS;

    // Set maximum allowed brightness
    nvs_get_u8(d->nvs, "src_max_bri", &d->max_brightness);
    if (d->max_brightness > APP_DISPLAY_MAX_BRIGHTNESS) {
        d->max_brightness = APP_DISPLAY_MAX_BRIGHTNESS;
    }

    // Initialize display
    switch (APP_HW_VERSION) {
        case APP_HW_VER_1_0:
        case APP_HW_VER_1_1:
            init_display_hw_ver_1(d);
            break;

        default:
            free(d);
            ESP_LOGE(APP_NAME, "unknown hardware version %d", APP_HW_VERSION);
            return NULL;
    }

    // Show splash screen
    sprintf(d->splash_screen_text, "Cronus     v%s", APP_VERSION);
    splash_screen(d);

    // Brightness regulator
    adc_config_t adc = {.mode = ADC_READ_TOUT_MODE};
    err = adc_init(&adc);
    if (err != ESP_OK) {
        free(d);
        ESP_LOGE(APP_NAME, "adc_init() failed; err=%d", err);
        return NULL;
    }
    xTaskCreate(brightness_regulator, "display_brightness", 4096, (void *) d, 0, NULL);
    ESP_LOGI(APP_NAME, "brightness regulator initialized");

    // Screen draw task
    xTaskCreate(refresh, "display", 4096, (void *) d, 0, NULL);
    ESP_LOGI(APP_NAME, "display initialized");

    return d;
}
