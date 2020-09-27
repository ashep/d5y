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
#include "esp_err.h"
#include "esp_log.h"
#include "aespl_gfx.h"
#include "aespl_gfx_font8.h"
#include "aespl_gfx_font8_clock_1.h"
#include "aespl_gfx_font8_clock_2.h"
#include "aespl_max7219.h"
#include "aespl_max7219_matrix.h"
#include "app_main.h"

/**
 * @brief Draws current time on the screen
 */
static esp_err_t draw_time(app_t *app) {
    char *s = malloc(9);  // "XX:XX:XX\0"
    if (app->time.sep) {
        sprintf(s, "%02d:%02d", app->time.hour, app->time.minute);
    } else {
        sprintf(s, "%02d %02d", app->time.hour, app->time.minute);
    }

    aespl_gfx_point_t pos = {1, 0};
    aespl_gfx_puts(&app->gfx_buf, &font8_clock_2, &pos, s, 1, 1);

    free(s);

    return ESP_OK;
}

/**
 * @brief Draws current date on the screen
 */
static void draw_date(app_t *app) {
    char *s = malloc(11);  // "XX.XX.XXXX\0"
    sprintf(s, "%02d.%02d", app->time.day, app->time.month);
    aespl_gfx_point_t pos = {1, 0};
    aespl_gfx_puts(&app->gfx_buf, &font8_clock_2, &pos, s, 1, 1);
    free(s);
}

/**
 * @brief Draws ambient temperature
 */
static void draw_ambient_temp(app_t *app) {
    char *s = malloc(11);  // "-XX\0"
    sprintf(s, "%d,", (int)round(app->ds3231.temp - 1));
    aespl_gfx_point_t pos = {0, 0};
    aespl_gfx_puts(&app->gfx_buf, &font8_clock_2, &pos, s, 1, 1);
    free(s);
}

/**
 * @brief Draws current weather temperature
 */
static void draw_weather_temp(app_t *app) {
    char *s = malloc(11);  // "-XX\0"
    sprintf(s, "%d,", (int)round(app->weather.temp));
    aespl_gfx_point_t pos = {0, 0};
    aespl_gfx_puts(&app->gfx_buf, &font8_clock_2, &pos, s, 1, 1);
    free(s);
}

/**
 * @brief Refresh display task
 */
static void refresh(void *args) {
    ESP_LOGI(APP_NAME, "display refresh task started");
    uint8_t time_sep_max = 1000 / APP_SCREEN_REFRESH_RATE - 1;
    app_t *app = (app_t *)args;

    for (;;) {
        aespl_gfx_clear(&app->gfx_buf);

        // Update hour/minute separator state
        app->time.sep = false;
        if (app->time.sep_cnt++ >= time_sep_max / 2) {
            app->time.sep = true;
        }
        if (app->time.sep_cnt == time_sep_max) {
            app->time.sep_cnt = 0;
        }

        switch (app->mode) {
            case APP_MODE_SHOW_TIME:
                draw_time(app);
                break;
            case APP_MODE_SHOW_DATE:
                draw_date(app);
                break;
            case APP_MODE_SHOW_AMBIENT_TEMP:
                draw_ambient_temp(app);
                break;
            case APP_MODE_SHOW_WEATHER_TEMP:
                draw_weather_temp(app);
                break;
            case APP_MODE_SETTINGS_TIME_HOUR:
                // TODO
                break;
            case APP_MODE_SETTINGS_TIME_MINUTE:
                // TODO
                break;
            case APP_MODE_SETTINGS_DATE_YEAR:
                // TODO
                break;
            case APP_MODE_SETTINGS_DATE_MONTH:
                // TODO
                break;
            case APP_MODE_SETTINGS_DATE_DAY:
                // TODO
                break;
            case APP_MODE_SETTINGS_DATE_DOW:
                // TODO
                break;
            default:
                break;
        }

        switch (APP_DISPLAY_DRIVER) {
            case APP_DISPLAY_MAX7219:
                aespl_max7219_matrix_draw(&app->max7219_matrix, &app->gfx_buf);
                break;

            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(APP_SCREEN_REFRESH_RATE));
    }

    vTaskDelete(NULL);
}

esp_err_t app_display_init(app_t *app) {
    switch (APP_DISPLAY_DRIVER) {
        case APP_DISPLAY_MAX7219:
            aespl_gfx_init_buf(&app->gfx_buf, APP_MAX7219_DISP_X * 8, APP_MAX7219_DISP_Y * 8, AESPL_GFX_CMODE_MONO);
            ESP_ERROR_CHECK(aespl_max7219_init(&app->max7219,
                                               APP_MAX7219_PIN_CS, APP_MAX7219_PIN_CLK, APP_MAX7219_PIN_DATA,
                                               APP_MAX7219_DISP_X * APP_MAX7219_DISP_Y, AESPL_MAX7219_DECODE_NONE));
            ESP_ERROR_CHECK(aespl_max7219_matrix_init(&app->max7219_matrix, &app->max7219,
                                                      APP_MAX7219_DISP_X, APP_MAX7219_DISP_Y));
            break;

        default:
            break;
    }

    xTaskCreate(refresh, "display_refresh", 4096, (void *)app, 0, NULL);

    ESP_LOGD(APP_NAME, "display initialized");

    return ESP_OK;
}
