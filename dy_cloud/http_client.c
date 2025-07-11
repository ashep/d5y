#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"

#include "dy/appinfo.h"
#include "dy/error.h"

#define HTTP_REQ_TIMEOUT 5000
#define HTTP_RSP_LEN 1024
#define HTTP_AUTHORIZATION_LEN 128

typedef struct {
    esp_http_client_method_t method;
    const char *url;
    int *rsp_status;
    int64_t *rsp_len;
    char *rsp_body;
} dy_cloud_http_req_t;

static char authorization[HTTP_AUTHORIZATION_LEN + 1];
static char response[HTTP_RSP_LEN + 1];
static QueueHandle_t mux = NULL;

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

static esp_err_t http_cli_ev_handler(esp_http_client_event_t *evt) {
    int len;

    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            memset(response, 0, HTTP_RSP_LEN);

            len = HTTP_RSP_LEN;
            if (evt->data_len < len) {
                len = evt->data_len;
            }

            strlcpy(response, evt->data, len);
            break;

        default:
            break;
    }

    return ESP_OK;
}

dy_err_t http_request(dy_cloud_http_req_t *req) {
    dy_err_t err;
    esp_err_t esp_err;
    esp_http_client_handle_t cli;

    dy_appinfo_info_t ai;
    if (dy_is_err(err = dy_appinfo_get(&ai))) {
        return dy_err_pfx("dy_appinfo_get", err);
    }

    if (mux == NULL) {
        mux = xSemaphoreCreateMutex();
        if (mux == NULL) {
            return dy_err(DY_ERR_FAILED, "xSemaphoreCreateMutex returned null");
        }
    }

    if (xSemaphoreTake(mux, (TickType_t) 100) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "xSemaphoreTake failed");
    }

    esp_http_client_config_t cfg = {
        .method = req->method,
        .url = req->url,
        .user_agent = ai.id,
        .timeout_ms = HTTP_REQ_TIMEOUT,
        .event_handler = http_cli_ev_handler,
        .keep_alive_enable = false,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    cli = esp_http_client_init(&cfg);
    if (cli == NULL) {
        xSemaphoreGive(mux);
        return dy_err(DY_ERR_NO_MEM, "esp_http_client_init failed");
    }

    if (strlen(ai.auth) > 0) {
        memset(authorization, 0, HTTP_AUTHORIZATION_LEN);
        strcpy(authorization, "Bearer ");
        strncat(authorization, ai.auth, HTTP_AUTHORIZATION_LEN - strlen(authorization));
        esp_http_client_set_header(cli, "Authorization", authorization);
    }

    if ((esp_err = esp_http_client_perform(cli)) != ESP_OK) {
        esp_http_client_cleanup(cli);
        xSemaphoreGive(mux);
        return dy_err(DY_ERR_FAILED, "esp_http_client_perform failed: %d", esp_err_to_name(esp_err));
    }

    *req->rsp_status = esp_http_client_get_status_code(cli);
    *req->rsp_len = esp_http_client_get_content_length(cli);

    memset(req->rsp_body, 0, HTTP_RSP_LEN);
    strlcpy(req->rsp_body, response, HTTP_RSP_LEN);

    if ((esp_err = esp_http_client_cleanup(cli)) != ESP_OK) {
        xSemaphoreGive(mux);
        return dy_err(DY_ERR_FAILED, "esp_http_client_cleanup failed: %d", esp_err_to_name(esp_err));
    }

    xSemaphoreGive(mux);

    return dy_ok();
}

dy_err_t http_get_json(const char *url, cJSON **rsp_json) {
    int rsp_status = 0;
    int64_t rsp_len = 0;

    char *rsp_body = malloc(HTTP_RSP_LEN);
    if (rsp_body == NULL) {
        return dy_err(DY_ERR_NO_MEM, "response buffer allocation failed");
    }
    dy_cloud_http_req_t req = {
        .method = HTTP_METHOD_GET,
        .url = url,
        .rsp_status = &rsp_status,
        .rsp_len = &rsp_len,
        .rsp_body = rsp_body,
    };

    dy_err_t err = http_request(&req);
    if (dy_is_err(err)) {
        free(rsp_body);
        return dy_err_pfx("http request failed", err);
    }

    if (rsp_status == 204) {
        free(rsp_body);
        return dy_err(DY_ERR_NO_CONTENT, "no content");
    } else if (rsp_status == HttpStatus_NotFound) {
        free(rsp_body);
        return dy_err(DY_ERR_NOT_FOUND, "not found");
    } else if (rsp_status > HttpStatus_BadRequest) {
        free(rsp_body);
        return dy_err(DY_ERR_FAILED, "bad http response status: %d", rsp_status);
    }

    *rsp_json = cJSON_ParseWithLength(rsp_body, rsp_len);
    free(rsp_body);
    if (*rsp_json == NULL) {
        return json_err();
    }

    return dy_ok();
}
