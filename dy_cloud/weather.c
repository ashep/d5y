#include "string.h"
#include "math.h"
#include "cJSON.h"
#include "dy/error.h"
#include "dy/cloud.h"
#include "dy/_cloud.h"

static char http_response[MAX_HTTP_RSP_LEN] = {0};

dy_err_t dy_cloud_weather(dy_cloud_resp_weather_t *res) {
    dy_err_t err = http_request(API_URL_WEATHER, http_response);
    if (dy_nok(err)) {
        return dy_err_pfx("http request failed", err);
    }

    cJSON *json = cJSON_ParseWithLength(http_response, strlen(http_response));
    if (json == NULL) {
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

    return dy_ok();
}
