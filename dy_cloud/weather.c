#include "dy/cloud.h"

#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "esp_log.h"
#include "cJSON.h"

#include "dy/error.h"
#include "dy/net_cfg.h"

#define API_URL "https://api.d5y.xyz/v2/weather"
#define SYNC_PERIOD 900 // 15 min
#define URL_MAX_LEN 128
#define LTAG "DY_CLOUD"

extern dy_err_t http_get_json(const char *url, cJSON **rsp_json);

static char weather_url[URL_MAX_LEN] = {0};

static dy_err_t get_weather() {
    dy_err_t err;
    cJSON *json;

    dy_cloud_weather_t res;
    memset(&res, 0, sizeof(res));

    err = http_get_json(weather_url, &json);
    if (dy_is_err(err)) {
        return dy_err_pfx("http_get_json", err);
    }

    cJSON *id = cJSON_GetObjectItem(json, "id");
    if (id != NULL) {
        res.id = (int8_t) round(cJSON_GetNumberValue(id));
    }

    cJSON *title = cJSON_GetObjectItem(json, "title");
    if (title != NULL) {
        strlcpy(res.title, cJSON_GetStringValue(title), DY_CLOUD_WEATHER_TITLE_LEN);
    }

    cJSON *is_day = cJSON_GetObjectItem(json, "is_day");
    if (is_day != NULL) {
        res.is_day = (bool) cJSON_GetNumberValue(is_day);
    }

    cJSON *temp = cJSON_GetObjectItem(json, "temp");
    if (temp != NULL) {
        res.temp = (int8_t) round(cJSON_GetNumberValue(temp));
    }

    cJSON *feels = cJSON_GetObjectItem(json, "feels_like");
    if (feels != NULL) {
        res.feels = (int8_t) round(cJSON_GetNumberValue(feels));
    }

    cJSON_Delete(json);

    ESP_LOGI(LTAG, "got weather: id=%d, title=%s; is_day=%d; temp=%d; feels=%d",
             res.id, res.title, res.is_day, res.temp, res.feels);

    res.ts = time(NULL);

    esp_err_t esp_err = esp_event_post(DY_CLOUD_EV_BASE, DY_CLOUD_EV_WEATHER_UPDATED, &res, sizeof(res), 10);
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
        } else if (dy_is_err(err = get_weather())) {
            ESP_LOGE(LTAG, "get_weather: %s", dy_err_str(err));
            delay_sec = 10;
        } else {
            delay_sec = SYNC_PERIOD;
        }

        vTaskDelay(pdMS_TO_TICKS(1000 * delay_sec));
    }
}

dy_err_t dy_cloud_weather_scheduler_start() {
    int n = snprintf(weather_url, URL_MAX_LEN, "%s?lat=%.5f&lng=%.5f", API_URL,
                     dy_cloud_get_location_lat(), dy_cloud_get_location_lng());
    if (n < 0 || n >= URL_MAX_LEN) {
        return dy_err(DY_ERR_FAILED, "api url is too long");
    }

    BaseType_t res = xTaskCreate(task, "dy_cloud_weather", 4096, NULL, tskIDLE_PRIORITY, NULL);
    if (res != pdPASS) {
        return dy_err(DY_ERR_FAILED, "xTaskCreate");
    }

    return dy_ok();
}
