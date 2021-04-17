/**
 * @brief     Cronus Digital Clock Display Functions
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/adc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "agfxl_buffer.h"
#include "agfxl_text.h"
#include "agfxl_animation.h"
#include "agfxl_font_1.h"
#include "aespl_max7219.h"
#include "aespl_max7219_matrix.h"
#include "app_main.h"
#include "app_font_clock_8_1.h"
#include "app_font_clock_8_2.h"

/**
 * @brief Draws current time on the screen
 */
static void draw_time(app_t *app) {
    agfxl_point_t pos = {0, 0};
    char *s = malloc(4);

    // Prepare buffers:
    // 2-digit hour * 6px + 1px whitespace: 13px
    // 2-digit minute * 6px + 1px whitespace: 13px
    // separator: 2px
    agfxl_buf_t *buf_h = agfxl_make_buf(13, 8, AGFXL_C_MODE_MONO);
    agfxl_buf_t *buf_m = agfxl_make_buf(13, 8, AGFXL_C_MODE_MONO);
    agfxl_buf_t *buf_sep = agfxl_make_buf(2, 8, AGFXL_C_MODE_MONO);

    // Draw hour
    sprintf(s, "%02d", app->time.hour);
    agfxl_puts(buf_h, &font8_clock_2, pos, s, 1, 1);

    // Draw minute
    sprintf(s, "%02d", app->time.minute);
    agfxl_puts(buf_m, &font8_clock_2, pos, s, 1, 1);

    // Draw separator
    agfxl_set_px(buf_sep, 0, 2, 1);
    agfxl_set_px(buf_sep, 1, 2, 1);
    agfxl_set_px(buf_sep, 0, 5, 1);
    agfxl_set_px(buf_sep, 1, 5, 1);
    if (!app->time.al_enabled) {
        agfxl_set_px(buf_sep, 0, 6, 1);
        agfxl_set_px(buf_sep, 1, 6, 1);
    }

    // Hour blink
    if (app->mode == APP_MODE_SETTINGS_TIME_HOUR && !app->time.sep) {
        agfxl_clear_buf(buf_h);
    }

    // Minute blink
    if (app->mode == APP_MODE_SETTINGS_TIME_MINUTE && !app->time.sep) {
        agfxl_clear_buf(buf_m);
    }

    // Separator blink
    if (app->mode < APP_MODE_SHOW_MAX && !app->time.sep) {
        agfxl_clear_buf(buf_sep);
    }

    // Merge buffers
    agfxl_merge(app->gfx_buf, buf_h, (agfxl_point_t){0, 0}, (agfxl_point_t){0, 0});
    agfxl_merge(app->gfx_buf, buf_sep, (agfxl_point_t){15, 0}, (agfxl_point_t){0, 0});
    agfxl_merge(app->gfx_buf, buf_m, (agfxl_point_t){19, 0}, (agfxl_point_t){0, 0});

    // Cleanup
    free(s);
    agfxl_free_buf(buf_h);
    agfxl_free_buf(buf_m);
    agfxl_free_buf(buf_sep);
}

/**
 * @brief Draws current date on the screen
 */
static void draw_date(app_t *app) {
    agfxl_point_t pos = {0, 0};
    char *s = malloc(4);

    // Prepare buffers:
    // 2-digit day * 6px + 1px whitespace: 13px
    // 2-digit month * 6px + 1px whitespace: 13px
    // separator: 2px
    agfxl_buf_t *buf_d = agfxl_make_buf(13, 8, AGFXL_C_MODE_MONO);
    agfxl_buf_t *buf_m = agfxl_make_buf(13, 8, AGFXL_C_MODE_MONO);
    agfxl_buf_t *buf_sep = agfxl_make_buf(2, 8, AGFXL_C_MODE_MONO);

    // Draw day
    sprintf(s, "%02d", app->time.day);
    agfxl_puts(buf_d, &font8_clock_2, pos, s, 1, 1);

    // Draw month
    sprintf(s, "%02d", app->time.month);
    agfxl_puts(buf_m, &font8_clock_2, pos, s, 1, 1);

    // Draw separator
    agfxl_set_px(buf_sep, 0, 7, 1);
    agfxl_set_px(buf_sep, 1, 7, 1);

    // Day blink
    if (app->mode == APP_MODE_SETTINGS_DATE_DAY && !app->time.sep) {
        agfxl_clear_buf(buf_d);
    }

    // Month blink
    if (app->mode == APP_MODE_SETTINGS_DATE_MONTH && !app->time.sep) {
        agfxl_clear_buf(buf_m);
    }

    // Merge buffers
    agfxl_merge(app->gfx_buf, buf_d, (agfxl_point_t){0, 0}, (agfxl_point_t){0, 0});
    agfxl_merge(app->gfx_buf, buf_sep, (agfxl_point_t){15, 0}, (agfxl_point_t){0, 0});
    agfxl_merge(app->gfx_buf, buf_m, (agfxl_point_t){19, 0}, (agfxl_point_t){0, 0});

    // Cleanup
    free(s);
    agfxl_free_buf(buf_d);
    agfxl_free_buf(buf_m);
    agfxl_free_buf(buf_sep);
}

static void make_temperature_str(char *s, int temp) {
    if (temp > 0) {
        sprintf(s, "+%d,", temp);
    } else if (temp < 0) {
        sprintf(s, "-%d,", temp);
    } else {
        sprintf(s, "%d,", temp);
    }
}

/**
 * @brief Draws ambient temperature
 */
static void draw_ambient_temp(app_t *app) {
    char *s = malloc(9);
    strcpy(s, "#"); // home sign
    make_temperature_str(s + 1, (int)round(app->ds3231.temp - 2));
    agfxl_puts(app->gfx_buf, &font8_clock_2, (agfxl_point_t){0, 0}, s, 1, 1);
    free(s);
}

/**
 * @brief Draws current weather temperature
 */
static void draw_weather_temp(app_t *app) {
    char *s = malloc(9);
    strcpy(s, "!"); // thermometer sign
    make_temperature_str(s + 1, (int)round(app->weather.temp));
    agfxl_puts(app->gfx_buf, &font8_clock_2, (agfxl_point_t){0, 0}, s, 1, 1);
    free(s);
}

/**
 * @brief Refresh display task
 */
static void refresh(void *args) {
    ESP_LOGI(APP_NAME, "display refresh task started");
    app_t *app = (app_t *)args;

    for (;;) {
        agfxl_clear_buf(app->gfx_buf);

        // Update hour/minute separator state
        app->time.sep = true;
        if (app->display_refresh_cnt++ > app->display_refresh_cnt_max / 2) {
            app->time.sep = false;
        }
        if (app->display_refresh_cnt > app->display_refresh_cnt_max) {
            app->display_refresh_cnt = 0;
        }

        switch (app->mode) {
            case APP_MODE_SHOW_TIME:
            case APP_MODE_SETTINGS_TIME_HOUR:
            case APP_MODE_SETTINGS_TIME_MINUTE:
                draw_time(app);
                break;
            case APP_MODE_SHOW_DATE:
            case APP_MODE_SETTINGS_DATE_MONTH:
            case APP_MODE_SETTINGS_DATE_DAY:
                draw_date(app);
                break;
            case APP_MODE_SHOW_AMBIENT_TEMP:
                draw_ambient_temp(app);
                break;
            case APP_MODE_SHOW_WEATHER_TEMP:
                draw_weather_temp(app);
                break;
            case APP_MODE_SETTINGS_DATE_YEAR:
                // TODO
                break;
            case APP_MODE_SETTINGS_DATE_DOW:
                // TODO
                break;
            default:
                break;
        }

        switch (APP_HW_VERSION) {
            case APP_HW_VER_1:
                aespl_max7219_matrix_draw(&app->max7219_matrix, app->gfx_buf);
                break;

            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(APP_SCREEN_REFRESH_RATE));
    }

    vTaskDelete(NULL);
}

static void brightness_regulator(void *args) {
    app_t *app = (app_t *)args;
    uint16_t data = 0;

    for (;;) {
        adc_read(&data);

        int8_t intensity = data / 64 - 1;
        if (intensity < 0) {
            intensity = 0;
        }

        // aespl_max7219_send_all(&app->max7219, AESPL_MAX7219_ADDR_INTENSITY, intensity);
        aespl_max7219_send_all(&app->max7219, AESPL_MAX7219_ADDR_INTENSITY, AESPL_MAX7219_INTENSITY_MIN);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}

static void init_display_hw_ver_1(app_t *app) {
    app->gfx_buf = agfxl_make_buf(APP_MAX7219_DISP_X * 8, APP_MAX7219_DISP_Y * 8, AGFXL_C_MODE_MONO);
    ESP_ERROR_CHECK(aespl_max7219_init(&app->max7219,
                                       APP_MAX7219_PIN_CS, APP_MAX7219_PIN_CLK, APP_MAX7219_PIN_DATA,
                                       APP_MAX7219_DISP_X * APP_MAX7219_DISP_Y, AESPL_MAX7219_DECODE_NONE));
    ESP_ERROR_CHECK(aespl_max7219_matrix_init(&app->max7219_matrix, &app->max7219,
                                              APP_MAX7219_DISP_X, APP_MAX7219_DISP_Y));
}

esp_err_t app_display_init(app_t *app) {
    esp_err_t err;

    app->display_refresh_cnt = 0;
    app->display_refresh_cnt_max = 1000 / APP_SCREEN_REFRESH_RATE - 1;

    // "Enable display" pin configuration
    gpio_set_direction(APP_DISPLAY_PIN_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(APP_DISPLAY_PIN_EN, 1);

    // Initialize display
    switch (APP_HW_VERSION) {
        case APP_HW_VER_1:
            init_display_hw_ver_1(app);
            break;

        default:
            ESP_LOGE(APP_NAME, "unknown hardware version %d", APP_HW_VERSION);
            return ESP_FAIL;
            break;
    }

    // ADC for ambient light measurement
    // adc_config_t adc = {
    //     .mode = ADC_READ_TOUT_MODE,
    // };
    // err = adc_init(&adc);
    // if (err) {
    //     return err;
    // }

    xTaskCreate(refresh, "display_refresh", 4096, (void *)app, 0, NULL);
    // xTaskCreate(brightness_regulator, "brightness_regulator", 4096, (void *)app, 0, NULL);

    ESP_LOGI(APP_NAME, "display initialized");

    return ESP_OK;
}
