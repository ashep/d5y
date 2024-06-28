#include "freertos/FreeRTOS.h"

#include "esp_err.h"
#include "esp_http_client.h"

#include "dy/error.h"
#include "dy/_cloud.h"

esp_http_client_handle_t client = NULL;
char user_agent[16] = {0};
static char response[MAX_HTTP_RSP_LEN] = {};

static SemaphoreHandle_t mux = NULL;

static esp_err_t http_cli_ev_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            memset(response, 0, MAX_HTTP_RSP_LEN);
            strncpy(response, evt->data, MAX_HTTP_RSP_LEN - 1);
            break;

        default:
            break;
    }

    return ESP_OK;
}

static dy_err_t init_client(const char *url) {
    snprintf(user_agent, 16, HTTP_USER_AGENT_FMT, 1, 0, 0); // TODO: use app version instead

    esp_http_client_config_t cfg = {
        .url = url,
        .user_agent = user_agent,
        .timeout_ms = HTTP_REQ_TIMEOUT_MS,
        .event_handler = http_cli_ev_handler,
    };

    client = esp_http_client_init(&cfg);
    if (client == NULL) {
        return dy_err(DY_ERR_NO_MEM, "esp_http_client_init failed");
    }

    esp_err_t esp_err = esp_http_client_set_header(client, "Authorization", "Bearer " HTTP_BEARER_TOKEN);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_http_client_set_header: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}

dy_err_t http_request(const char *url, char *rsp) {
    dy_err_t err;
    esp_err_t esp_err;

    if (mux == NULL) {
        mux = xSemaphoreCreateMutex();
        if (mux == NULL) {
            return dy_err(DY_ERR_FAILED, "xSemaphoreCreateMutex returned null");
        }
    }

    if (client == NULL && dy_nok(err = init_client(url))) {
        return dy_err_pfx("init http client", err);
    }

    if (xSemaphoreTake(mux, portTICK_PERIOD_MS * 100) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "xSemaphoreTake failed");
    }

    esp_http_client_set_url(client, url);

    if ((esp_err = esp_http_client_perform(client)) != ESP_OK) {
        xSemaphoreGive(mux);
        return dy_err(DY_ERR_FAILED, "esp_http_client_perform failed: %d", esp_err_to_name(esp_err));
    }

    int rsp_st = esp_http_client_get_status_code(client);
    if (rsp_st >= HttpStatus_BadRequest) {
        xSemaphoreGive(mux);
        return dy_err(DY_ERR_FAILED, "bad response status: %d", rsp_st);
    }

    strncpy(rsp, response, MAX_HTTP_RSP_LEN);

    xSemaphoreGive(mux);

    return dy_ok();
}