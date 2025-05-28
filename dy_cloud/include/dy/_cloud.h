#pragma once

#include "cJSON.h"
#include "esp_http_client.h"

#define LTAG "DY_CLOUD"

#define URL_LEN 512
#define HTTP_REQ_TIMEOUT 5000
#define HTTP_RSP_LEN 1024
#define HTTP_AUTHORIZATION_LEN 128

#define API_URL_TIME "https://api.d5y.xyz/v2/time"
#define API_URL_WEATHER "https://api.d5y.xyz/v2/weather"
#define API_URL_FW_UPDATE "https://api.d5y.xyz/v2/firmware/update"

#define TIME_SYNC_PERIOD 42949 // ~11 hours, limited by max value of uint32
#define WEATHER_SYNC_PERIOD 900 // 15 min
#define UPDATE_CHECK_PERIOD 42949 // ~11 hours, limited by max value of uint32

typedef struct {
    esp_http_client_method_t method;
    const char *url;
    int *rsp_status;
    int64_t *rsp_len;
    char *rsp_body;
} dy_cloud_http_req_t;

typedef struct {
    char url[URL_LEN];
    size_t size;
    char sha256[65];
} dy_cloud_resp_fw_update_t;

dy_err_t json_err();

dy_err_t http_request(dy_cloud_http_req_t *req);

// The caller is responsible for freeing rsp_json
dy_err_t http_get_json(const char *url, cJSON **rsp_json);
