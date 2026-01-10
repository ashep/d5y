#include "dy/cloud.h"
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "cJSON.h"
#include "dy/error.h"
#include "dy/net_cfg.h"

#define SYNC_PERIOD 42949 // ~11 hours, limited by max value of uint32
#define API_URL "https://api.d5y.xyz/v3/time"
#define URL_MAX_LEN 128
#define LTAG "DY_CLOUD"

ESP_EVENT_DEFINE_BASE(DY_CLOUD_EVENT_BASE);

extern dy_err_t http_get_json(const char *url, cJSON **rsp_json);

static dy_err_t get_cloud_time() {
    cJSON *json;
    dy_cloud_time_t res = {0};
    char url[URL_MAX_LEN] = {0};

    const int n = snprintf(url, URL_MAX_LEN, "%s?lat=%.5f&lng=%.5f", API_URL,
                           dy_cloud_get_location_lat(), dy_cloud_get_location_lng());
    if (n < 0 || n >= URL_MAX_LEN) {
        return dy_err(DY_ERR_FAILED, "api url is too long");
    }

    dy_err_t err = http_get_json(url, &json);
    if (dy_is_err(err)) {
        return dy_err_pfx("http_get_json", err);
    }

    cJSON *tz = cJSON_GetObjectItem(json, "tz");
    if (tz != NULL) {
        strlcpy(res.tz, cJSON_GetStringValue(tz), DY_CLOUD_TIME_TZ_LEN);
    }

    cJSON *tz_data = cJSON_GetObjectItem(json, "tz_data");
    if (tz_data != NULL) {
        strlcpy(res.tzd, cJSON_GetStringValue(tz_data), DY_CLOUD_TIME_TZ_DATA_LEN);
    }

    cJSON *ts = cJSON_GetObjectItem(json, "value");
    if (ts != NULL) {
        res.ts = (int) cJSON_GetNumberValue(ts);
    }

    cJSON_Delete(json);

    ESP_LOGI(LTAG, "got time: %lu; %s; %s", res.ts, res.tz, res.tzd);

    esp_err_t esp_err = esp_event_post(DY_CLOUD_EV_BASE, DY_CLOUD_EV_TIME_UPDATED, &res, sizeof(res), 10);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_post: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}

_Noreturn static void task() {
    dy_err_t err;
    int delay_sec;

    while (true) {
        if (!dy_net_cfg_net_connected()) {
            delay_sec = 10;
        } else if (dy_is_err(err = get_cloud_time())) {
            ESP_LOGE(LTAG, "set_localtime_from_cloud: %s", dy_err_str(err));
            delay_sec = 10;
        } else {
            delay_sec = SYNC_PERIOD;
        }

        vTaskDelay(pdMS_TO_TICKS(1000 * delay_sec));
    }
}

dy_err_t dy_cloud_time_scheduler_start() {
    BaseType_t res = xTaskCreate(task, "dy_cloud_time", 4096, NULL, tskIDLE_PRIORITY, NULL);
    if (res != pdPASS) {
        return dy_err(DY_ERR_FAILED, "xTaskCreate");
    }

    return dy_ok();
}
