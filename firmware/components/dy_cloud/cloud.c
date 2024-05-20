#include <stdio.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"

#include "dy/error.h"
#include "dy/cloud.h"

#define LTAG "DY_CLOUD"

#define HTTP_USER_AGENT_FMT "d5y/%d.%d.%d"
#define HTTP_BEARER_TOKEN "tbd"
#define HTTP_REQ_TIMEOUT_MS 5000

#define API_URL "http://api.d5y.xyz/v2/%s"
#define EP_ME "me"

#define MAX_URL_LEN 128
#define MAX_HTTP_BODY_LEN 1024

static uint8_t app_ver_maj = 0;
static uint8_t app_ver_min = 0;
static uint8_t app_ver_patch = 0;

char user_agent[16] = {0};
char url[MAX_URL_LEN] = {0};

esp_http_client_handle_t http_cli = NULL;

static char http_resp_buf[MAX_HTTP_BODY_LEN] = {};

static esp_err_t http_cli_ev_handler(esp_http_client_event_t *evt) {
    int b_len;

    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            b_len = evt->data_len;
            if (b_len > MAX_HTTP_BODY_LEN) {
                b_len = MAX_HTTP_BODY_LEN;
            }
            strncpy(http_resp_buf, evt->data, b_len);
            break;

        default:
            break;
    }

    return ESP_OK;
}

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

dy_err_t dy_cloud_get_me(dy_cloud_resp_me_t *res) {
    if (http_cli == NULL) {
        return dy_err(DY_ERR_INVALID_STATE, "dy_cloud_init has to be called before");
    }

    esp_err_t esp_err;

    snprintf(url, MAX_URL_LEN, API_URL, EP_ME);
    esp_http_client_set_url(http_cli, url);

    if ((esp_err = esp_http_client_perform(http_cli)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_http_client_perform failed: %d", esp_err_to_name(esp_err));
    }

    int rsp_st = esp_http_client_get_status_code(http_cli);
    if (rsp_st != HttpStatus_Ok) {
        return dy_err(DY_ERR_FAILED, "bad response status: %d", rsp_st);
    }

    cJSON *json = cJSON_ParseWithLength(http_resp_buf, strlen(http_resp_buf));
    if (json == NULL) {
        return json_err();
    }

    cJSON *time = cJSON_GetObjectItem(json, "timestamp");
    if (time != NULL) {
        cJSON *tz = cJSON_GetObjectItem(time, "tz");
        if (tz != NULL) {
            strncpy(res->time.tz, cJSON_GetStringValue(tz), DY_CLOUD_RESP_TIME_TZ_LEN);
        }

        cJSON *tz_data = cJSON_GetObjectItem(time, "tz_data");
        if (tz_data != NULL) {
            strncpy(res->time.tz_data, cJSON_GetStringValue(tz_data), DY_CLOUD_RESP_TIME_TZ_DATA_LEN);
        }

        cJSON *ts = cJSON_GetObjectItem(time, "value");
        if (tz_data != NULL) {
            res->time.ts = (int) cJSON_GetNumberValue(ts);
        }
    }

    cJSON_free(json);



    return dy_ok();
}

dy_err_t dy_cloud_init() {
    snprintf(user_agent, 16, HTTP_USER_AGENT_FMT, app_ver_maj, app_ver_min, app_ver_patch);

    esp_http_client_config_t cfg = {
        .url = API_URL,
        .user_agent = user_agent,
        .timeout_ms = HTTP_REQ_TIMEOUT_MS,
        .event_handler = http_cli_ev_handler,
    };

    http_cli = esp_http_client_init(&cfg);
    if (http_cli == NULL) {
        return dy_err(DY_ERR_NO_MEM, "init http client failed");
    }

    esp_err_t esp_err = esp_http_client_set_header(http_cli, "Authorization", "Bearer " HTTP_BEARER_TOKEN);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_http_client_set_header: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}