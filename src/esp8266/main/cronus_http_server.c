#include "esp_log.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "cronus_main.h"

static httpd_handle_t server = NULL;

esp_err_t settings_get_handler(httpd_req_t *req)
{
    const char *resp_str = "{\"wifi\":true}";

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp_str, strlen(resp_str));

    return ESP_OK;
}

httpd_uri_t uri_settings_get = {
    .uri = "/settings",
    .method = HTTP_GET,
    .handler = settings_get_handler,
};

void cronus_http_server_stop()
{
    httpd_stop(server);
}

void cronus_http_server_start()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(APP_TAG, "Starting HTTP server on port %d", config.server_port);
    
    if (httpd_start(&server, &config) == ESP_OK)
    {
        ESP_LOGI(APP_TAG, "Registering HTTP URI handlers");
        httpd_register_uri_handler(server, &uri_settings_get);
        return;
    }

    ESP_LOGI(APP_TAG, "Error starting HTTP server");
}
