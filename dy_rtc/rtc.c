#include <string.h>
#include <time.h>

#include "esp_err.h"
#include "esp_netif_types.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "freertos/task.h"

#include "dy/error.h"
#include "dy/cloud.h"
#include "dy/ds3231.h"

#define LTAG "DY_RTC"
#define CLOUD_SYNC_PERIOD 1800 // 30 min

static dy_ds3231_handle_t *ds3231 = NULL;

static void set_localtime_from_ds3231() {
    if (!ds3231) {
        ESP_LOGE(LTAG, "set local time from ext rtc failed: no device handle");
        return;
    }

    dy_ds3231_data_t dt;
    dy_err_t ds3231_err = dy_ds3231_read(ds3231, &dt);
    if (dy_nok(ds3231_err)) {
        ESP_LOGE(LTAG, "set local time from ext rtc failed: dy_ds3231_read: %s", dy_err_str(ds3231_err));
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
    ESP_LOGI(LTAG, "local time set from ext rtc: %s", b);
}

static void write_localtime_to_ds3231() {
    if (!ds3231) {
        ESP_LOGE(LTAG, "store local time to ext rtc failed: no device handle");
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
    ESP_LOGI(LTAG, "local time written to ext rtc: %s", b);
}

static void set_localtime_from_cloud() {
    dy_err_t err;
    dy_cloud_resp_time_t cloud_time;

    if (dy_nok(err = dy_cloud_time(&cloud_time))) {
        ESP_LOGE(LTAG, "set local time from cloud failed: dy_cloud_time: %s", dy_err_str(err));
        return;
    }

    if (strlen(cloud_time.tzd) == 0) {
        ESP_LOGE(LTAG, "set time from cloud failed: dy_cloud_time returned empty tz_data field");
        return;
    }

    setenv("TZ", cloud_time.tzd, true);
    ESP_LOGI(LTAG, "got local time from the cloud: %lu; %s; %s", cloud_time.ts, cloud_time.tz, cloud_time.tzd);

    int c_err;
    struct timespec ts = {.tv_sec = cloud_time.ts};
    if ((c_err = clock_settime(CLOCK_REALTIME, &ts)) != 0) {
        ESP_LOGE(LTAG, "set local time from cloud failed: clock_settime: %d", c_err);
    }

    char b[20];
    time_t tt = time(NULL);
    struct tm *ti = localtime(&tt);
    strftime(b, 20, "%Y-%m-%d %H:%M:%S", ti);
    ESP_LOGI(LTAG, "local time set from the cloud: %s", b);

    if (ds3231) {
        write_localtime_to_ds3231();
    }
}

static void cloud_sync() {
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000 * CLOUD_SYNC_PERIOD));
        set_localtime_from_cloud();
    }
}

dy_err_t dy_rtc_init(dy_ds3231_handle_t *ds3231_hdl) {
    esp_err_t esp_err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &set_localtime_from_cloud, NULL);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_handler_register (IP) failed: %s", esp_err_to_name(esp_err));
    }

    xTaskCreate(cloud_sync, "dy_rtc_cloud_sync", 4096, NULL, tskIDLE_PRIORITY, NULL);

    if (ds3231_hdl) {
        ds3231 = ds3231_hdl;
        set_localtime_from_ds3231();
    }

    return dy_ok();
}
