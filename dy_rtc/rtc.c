#include <string.h>
#include <time.h>

#include "esp_err.h"
#include "esp_netif_types.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "dy/error.h"
#include "dy/cloud.h"
#include "dy/ds3231.h"

#define LTAG "DY_RTC"

static dy_ds3231_handle_t *ds3231 = NULL;

static void localtime_from_ds3231() {
    if (ds3231 == NULL) {
        ESP_LOGE(LTAG, "set local time from ds3231: no device handle set up");
        return;
    }

    dy_ds3231_data_t dt;
    dy_err_t ds3231_err = dy_ds3231_read(ds3231, &dt);
    if (dy_nok(ds3231_err)) {
        ESP_LOGE(LTAG, "set local time from ds3231: dy_ds3231_read: %s", dy_err_str(ds3231_err));
        return;
    }

    struct tm tp = {
        .tm_sec = dt.sec,
        .tm_min = dt.min,
        .tm_hour = dt.hour,
        .tm_wday = dt.dow - 1, // DS3231 stores week day starting from 1
        .tm_mday = dt.day,
        .tm_mon = dt.mon - 1,  // DS3231 stores month starting from 1
        .tm_year = dt.year,
    };

    struct timespec ts = {.tv_sec = mktime(&tp)};

    int c_err;
    if ((c_err = clock_settime(CLOCK_REALTIME, &ts)) != 0) {
        ESP_LOGE(LTAG, "set local time from ext rtc failed: clock_settime: %d", c_err);
        return;
    }

    char b[20];
    time_t tt = time(NULL);
    struct tm *ti = localtime(&tt);
    strftime(b, 20, "%Y-%m-%d %H:%M:%S", ti);
    ESP_LOGI(LTAG, "local time set from ds3231: %s", b);
}

static void localtime_to_ds3231() {
    if (ds3231 == NULL) {
        ESP_LOGE(LTAG, "set local time from ds3231: no device handle set up");
        return;
    }

    time_t tt = time(NULL);
    struct tm *ti = localtime(&tt);

    dy_err_t ds3231_err;
    dy_ds3231_data_t dt;

    // Read all the data from DS3231, because we want to preserve alarm settings
    ds3231_err = dy_ds3231_read(ds3231, &dt);
    if (dy_nok(ds3231_err)) {
        ESP_LOGE(LTAG, "store local time to ext rtc failed: dy_ds3231_read: %s", dy_err_str(ds3231_err));
        return;
    }

    dt.time_12 = false;
    dt.sec = ti->tm_sec;
    dt.min = ti->tm_min;
    dt.hour = ti->tm_hour;
    dt.dow = ti->tm_wday + 1; // DS3231 stores week day starting from 1
    dt.day = ti->tm_mday;
    dt.mon = ti->tm_mon + 1; // DS3231 stores month starting from 1
    dt.year = ti->tm_year; // years since 1900

    ds3231_err = dy_ds3231_write(ds3231, &dt);
    if (dy_nok(ds3231_err)) {
        ESP_LOGE(LTAG, "write local time to ext rtc failed: dy_ds3231_write: %s", dy_err_str(ds3231_err));
        return;
    }

    char b[20];
    strftime(b, 20, "%Y-%m-%d %H:%M:%S", ti);
    ESP_LOGI(LTAG, "local time written to ds3231: %s", b);
}

static void cloud_update_handler(void *arg, esp_event_base_t base, int32_t id, void *data) {
    dy_cloud_time_t *dt = (dy_cloud_time_t *) data;

    if (strlen(dt->tzd) == 0) {
        ESP_LOGE(LTAG, "got empty tzd from cloud");
        return;
    }

    setenv("TZ", dt->tzd, true);

    int c_err;
    struct timespec ts = {.tv_sec = dt->ts};
    if ((c_err = clock_settime(CLOCK_REALTIME, &ts)) != 0) {
        ESP_LOGE(LTAG, "clock_settime: %d", c_err);
        return;
    }

    char b[20];
    time_t tt = time(NULL);
    struct tm *ti = localtime(&tt);
    strftime(b, 20, "%Y-%m-%d %H:%M:%S", ti);
    ESP_LOGI(LTAG, "local time set from cloud: %s", b);

    if (ds3231 != NULL) {
        localtime_to_ds3231();
    }
}

dy_err_t dy_rtc_init(dy_ds3231_handle_t *ds3231_hdl) {
    if (ds3231_hdl) {
        ds3231 = ds3231_hdl;
        localtime_from_ds3231();
    }

    esp_err_t esp_err = esp_event_handler_register(
        DY_CLOUD_EV_BASE, DY_CLOUD_EV_TIME_UPDATED, cloud_update_handler, NULL);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_handler_register: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}
