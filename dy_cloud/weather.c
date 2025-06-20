#include <string.h>
#include <math.h>

#include "esp_log.h"
#include "cJSON.h"

#include "dy/error.h"
#include "dy/net_cfg.h"

#include "dy/_cloud.h"
#include "dy/cloud.h"

dy_err_t get_weather() {
    dy_err_t err;
    cJSON *json;
    dy_cloud_weather_t res;

    if (dy_is_err(err = http_get_json(API_URL_WEATHER, &json))) {
        return dy_err_pfx("http_get_json", err);
    }

    cJSON *id = cJSON_GetObjectItem(json, "id");
    if (id != NULL) {
        res.id = (int8_t) round(cJSON_GetNumberValue(id));
    }

    cJSON *title = cJSON_GetObjectItem(json, "title");
    if (title != NULL) {
        strncpy(res.title, cJSON_GetStringValue(title), DY_CLOUD_WEATHER_TITLE_LEN);
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

    cJSON_free(json);

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
        if (!dy_net_cfg_net_ready()) {
            delay_sec = 10;
        } else if (dy_is_err(err = get_weather())) {
            ESP_LOGE(LTAG, "get_weather: %s", dy_err_str(err));
            delay_sec = 10;
        } else {
            delay_sec = WEATHER_SYNC_PERIOD;
        }

        vTaskDelay(pdMS_TO_TICKS(1000 * delay_sec));
    }
}

dy_err_t dy_cloud_weather_start_scheduler() {
    BaseType_t res = xTaskCreate(task, "dy_cloud_weather", 4096, NULL, tskIDLE_PRIORITY, NULL);
    if (res != pdPASS) {
        return dy_err(DY_ERR_FAILED, "xTaskCreate");
    }

    return dy_ok();
}
