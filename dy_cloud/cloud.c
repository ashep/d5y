#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "cJSON.h"

#include "dy/error.h"
#include "dy/cloud.h"
#include "dy/_cloud.h"

static char http_response[MAX_HTTP_RSP_LEN] = {0};
static SemaphoreHandle_t mux = NULL;

static dy_err_t json_err() {
    dy_err_t err;

    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
        err = dy_err(DY_ERR_JSON_PARSE, "%s", error_ptr);
    } else {
        err = dy_err(DY_ERR_JSON_PARSE, "reason unknown");
    }

    return err;
}

dy_err_t dy_cloud_time(dy_cloud_resp_time_t *res) {
    if (mux == NULL) {
        mux = xSemaphoreCreateMutex();
        if (mux == NULL) {
            return dy_err(DY_ERR_FAILED, "xSemaphoreCreateMutex returned null");
        }
    }

    if (xSemaphoreTake(mux, portTICK_PERIOD_MS * 100) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "xSemaphoreTake failed");
    }

    dy_err_t err = http_request(API_URL_TIME, http_response);
    if (dy_nok(err)) {
        xSemaphoreGive(mux);
        return dy_err_pfx("http request failed", err);
    }

    cJSON *json = cJSON_ParseWithLength(http_response, strlen(http_response));
    if (json == NULL) {
        xSemaphoreGive(mux);
        return json_err();
    }

    cJSON *tz = cJSON_GetObjectItem(json, "tz");
    if (tz != NULL) {
        strncpy(res->tz, cJSON_GetStringValue(tz), DY_CLOUD_RESP_TIME_TZ_LEN);
    }

    cJSON *tz_data = cJSON_GetObjectItem(json, "tz_data");
    if (tz_data != NULL) {
        strncpy(res->tzd, cJSON_GetStringValue(tz_data), DY_CLOUD_RESP_TIME_TZ_DATA_LEN);
    }

    cJSON *ts = cJSON_GetObjectItem(json, "value");
    if (tz_data != NULL) {
        res->ts = (int) cJSON_GetNumberValue(ts);
    }

    cJSON_free(json);

    xSemaphoreGive(mux);

    return dy_ok();
}

dy_err_t dy_cloud_weather(dy_cloud_resp_weather_t *res) {
    if (mux == NULL) {
        mux = xSemaphoreCreateMutex();
        if (mux == NULL) {
            return dy_err(DY_ERR_FAILED, "xSemaphoreCreateMutex returned null");
        }
    }

    if (xSemaphoreTake(mux, portTICK_PERIOD_MS * 100) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "xSemaphoreTake failed");
    }

    dy_err_t err = http_request(API_URL_WEATHER, http_response);
    if (dy_nok(err)) {
        xSemaphoreGive(mux);
        return dy_err_pfx("http request failed", err);
    }

    cJSON *json = cJSON_ParseWithLength(http_response, strlen(http_response));
    if (json == NULL) {
        xSemaphoreGive(mux);
        return json_err();
    }

    cJSON *title = cJSON_GetObjectItem(json, "title");
    if (title != NULL) {
        strncpy(res->title, cJSON_GetStringValue(title), DY_CLOUD_RESP_WEATHER_TITLE_DATA_LEN);
    }

    cJSON *temp = cJSON_GetObjectItem(json, "temp");
    if (temp != NULL) {
        res->temp = (int8_t) round(cJSON_GetNumberValue(temp));
    }

    cJSON *feels = cJSON_GetObjectItem(json, "feels_like");
    if (temp != NULL) {
        res->feels = (int8_t) round(cJSON_GetNumberValue(feels));
    }

    cJSON *pres = cJSON_GetObjectItem(json, "pressure");
    if (temp != NULL) {
        res->pressure = (uint16_t) cJSON_GetNumberValue(pres);
    }

    cJSON *hum = cJSON_GetObjectItem(json, "humidity");
    if (temp != NULL) {
        res->humidity = (uint16_t) cJSON_GetNumberValue(hum);
    }

    cJSON_free(json);

    xSemaphoreGive(mux);

    return dy_ok();
}
