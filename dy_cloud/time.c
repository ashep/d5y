#include "string.h"
#include "cJSON.h"
#include "dy/error.h"
#include "dy/cloud.h"
#include "dy/_cloud.h"

static char http_response[MAX_HTTP_RSP_LEN] = {0};

dy_err_t dy_cloud_time(dy_cloud_resp_time_t *res) {
    dy_err_t err = http_request(API_URL_TIME, http_response);
    if (dy_nok(err)) {
        return dy_err_pfx("http request failed", err);
    }

    cJSON *json = cJSON_ParseWithLength(http_response, strlen(http_response));
    if (json == NULL) {
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

    return dy_ok();
}
