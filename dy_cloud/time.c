#include <string.h>

#include "esp_log.h"
#include "cJSON.h"

#include "dy/error.h"
#include "dy/net_cfg.h"

#include "dy/_cloud.h"
#include "dy/cloud.h"

ESP_EVENT_DEFINE_BASE(DY_CLOUD_EVENT_BASE);

static dy_err_t get_cloud_time() {
    cJSON *json;
    dy_cloud_time_t res;

    dy_err_t err = http_get_json(API_URL_TIME, &json);
    if (dy_nok(err)) {
        return dy_err_pfx("http_get_json", err);
    }

    cJSON *tz = cJSON_GetObjectItem(json, "tz");
    if (tz != NULL) {
        strncpy(res.tz, cJSON_GetStringValue(tz), DY_CLOUD_TIME_TZ_LEN);
    }

    cJSON *tz_data = cJSON_GetObjectItem(json, "tz_data");
    if (tz_data != NULL) {
        strncpy(res.tzd, cJSON_GetStringValue(tz_data), DY_CLOUD_TIME_TZ_DATA_LEN);
    }

    cJSON *ts = cJSON_GetObjectItem(json, "value");
    if (tz_data != NULL) {
        res.ts = (int) cJSON_GetNumberValue(ts);
    }

    cJSON_free(json);

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
        if (!dy_net_cfg_net_ready()) {
            delay_sec = 10;
        } else if (dy_nok(err = get_cloud_time())) {
            ESP_LOGE(LTAG, "set_localtime_from_cloud: %s", dy_err_str(err));
            delay_sec = 10;
        } else {
            delay_sec = TIME_SYNC_PERIOD;
        }

        vTaskDelay(pdMS_TO_TICKS(1000 * delay_sec));
    }
}

dy_err_t dy_cloud_time_start_scheduler() {
    BaseType_t res = xTaskCreate(task, "dy_cloud_time", 4096, NULL, tskIDLE_PRIORITY, NULL);
    if (res != pdPASS) {
        return dy_err(DY_ERR_FAILED, "xTaskCreate");
    }

    return dy_ok();
}
