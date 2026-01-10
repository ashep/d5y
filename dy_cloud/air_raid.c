#include "dy/cloud.h"
#include <string.h>
#include <stdbool.h>
#include "esp_log.h"
#include "esp_event.h"
#include "cJSON.h"
#include "dy/error.h"
#include "dy/net_cfg.h"

#define API_URL "https://api.d5y.xyz/v3/air/raid"
#define SYNC_PERIOD 60
#define URL_MAX_LEN 128
#define LTAG "DY_CLOUD"

extern dy_err_t http_get_json(const char *url, cJSON **rsp_json);

static bool scheduler_enabled = false;

static dy_err_t get_air_raid_alert() {
    cJSON *json;
    dy_cloud_air_raid_alert_t res = {0};
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

    cJSON *active = cJSON_GetObjectItem(json, "active");
    if (active == NULL) {
        cJSON_Delete(json);
        return dy_err(DY_ERR_FAILED, "missing 'active' field in response");
    }
    if (!cJSON_IsBool(active)) {
        cJSON_Delete(json);
        return dy_err(DY_ERR_FAILED, "'active' field is not a boolean");
    }

    res.is_active = cJSON_IsTrue(active);

    cJSON_Delete(json);

    res.ts = time(NULL);
    ESP_LOGI(LTAG, "got air raid alert: is_active=%d, ts=%lld", res.is_active, res.ts);

    esp_err_t esp_err = esp_event_post(DY_CLOUD_EV_BASE, DY_CLOUD_EV_AIR_RAID_ALERT_UPDATED, &res, sizeof(res), 10);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_post: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}

_Noreturn static void task() {
    dy_err_t err;
    int delay_sec;

    while (true) {
        if (!(scheduler_enabled && dy_net_cfg_net_connected())) {
            delay_sec = 10;
        } else if (dy_is_err(err = get_air_raid_alert())) {
            ESP_LOGE(LTAG, "get_air_raid_alert: %s", dy_err_str(err));
            delay_sec = 10;
        } else {
            delay_sec = SYNC_PERIOD;
        }

        vTaskDelay(pdMS_TO_TICKS(1000 * delay_sec));
    }
}

dy_err_t dy_cloud_air_raid_alert_scheduler_start() {
    BaseType_t res = xTaskCreate(task, "dy_cloud_airraid", 4096, NULL, tskIDLE_PRIORITY, NULL);
    if (res != pdPASS) {
        return dy_err(DY_ERR_FAILED, "xTaskCreate");
    }

    return dy_ok();
}

void dy_cloud_air_raid_alert_set_enabled(bool v) {
    scheduler_enabled = v;
    ESP_LOGI(LTAG, "air raid alert scheduler enabled: %d", v);
}
