/**
 * @brief     Cronus Digital Clock Network Functions
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_client.h"

#include "cJSON.h"

#include "aespl_httpd.h"
#include "aespl_service.h"

#include "cronus_main.h"
#include "cronus_net.h"
#include "cronus_rtc.h"

#define APP_HTTP_BODY_MAX_LEN 2048

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    static int output_pos = 0, total_len = 0;

    switch (evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            output_pos = 0;
            total_len = 0;
            break;

        case HTTP_EVENT_ON_DATA:
            total_len += evt->data_len;
            if (total_len > APP_HTTP_BODY_MAX_LEN) {
                ESP_LOGE(APP_NAME, "response is bigger than %d", APP_HTTP_BODY_MAX_LEN);
                return ESP_FAIL;
            }
            memset(evt->user_data + output_pos, 0, evt->data_len);
            memcpy(evt->user_data + output_pos, evt->data, evt->data_len);
            output_pos += evt->data_len;
            break;

        case HTTP_EVENT_ERROR:
        case HTTP_EVENT_ON_FINISH:
        case HTTP_EVENT_DISCONNECTED:
        case HTTP_EVENT_HEADERS_SENT:
        case HTTP_EVENT_ON_HEADER:
            break;
    }

    return ESP_OK;
}

static esp_err_t fetch_data(app_t *app, const char *host, const char *path) {
    char data_buf[APP_HTTP_BODY_MAX_LEN] = {0};

    // Initialize an HTTP client
    esp_http_client_config_t cli_cfg = {
            .host = host,
            .path = path,
            .timeout_ms = 30000,
            .event_handler = http_event_handler,
            .user_data = data_buf,
    };
    esp_http_client_handle_t cli = esp_http_client_init(&cli_cfg);
    if (!cli) {
        return ESP_FAIL;
    }

    // Set user agent
    esp_http_client_set_header(cli, "User-Agent", app->signature);

    // Perform a request
    esp_err_t err = esp_http_client_perform(cli);
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(cli);
        if (status == 200) {
            ESP_LOGD(APP_NAME, "response: %s", data_buf);
        } else {
            ESP_LOGE(APP_NAME, "bad status code: %d", status);
        }
    } else {
        ESP_LOGE(APP_NAME, "request failed: %s", esp_err_to_name(err));
    }

    // Free resources
    esp_http_client_close(cli);
    esp_http_client_cleanup(cli);

    if (err != ESP_OK) {
        return err;
    }

    // Parse response
    cJSON *data = cJSON_Parse(data_buf);
    if (data == NULL) {
        const char *json_err = cJSON_GetErrorPtr();

        if (json_err == NULL) {
            ESP_LOGE(APP_NAME, "unknown error while parsing JSON");
        } else {
            ESP_LOGE(APP_NAME, "JSON parsing error near: %s", json_err);
        }

        return ESP_FAIL;
    }

    // Lock
    if (xSemaphoreTake(app->mux, (TickType_t) 10) != pdTRUE) {
        cJSON_Delete(data);
        ESP_LOGE(APP_NAME, "error while locking");
        return ESP_FAIL;
    }

    // Update
    app->time.second = cJSON_GetObjectItem(data, "second")->valueint;
    app->time.minute = cJSON_GetObjectItem(data, "minute")->valueint;
    app->time.hour = cJSON_GetObjectItem(data, "hour")->valueint;
    app->time.dow = cJSON_GetObjectItem(data, "dow")->valueint - 1;
    app->time.day = cJSON_GetObjectItem(data, "day")->valueint;
    app->time.month = cJSON_GetObjectItem(data, "month")->valueint;
    app->time.year = cJSON_GetObjectItem(data, "year")->valueint;
    app->time.flush_to_rtc = 1;

    app->weather.update_ok = (bool) cJSON_GetObjectItem(data, "weather")->valueint;
    if (app->weather.update_ok) {
        app->weather.temp = cJSON_GetObjectItem(data, "feels_like")->valuedouble;
    }

    // Free resources
    cJSON_Delete(data);

    // Unlock
    if (xSemaphoreGive(app->mux) != pdTRUE) {
        ESP_LOGE(APP_NAME, "error while unlocking");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void data_fetcher(void *args) {
    app_t *app = (app_t *) args;
    bool need_update = true;

    // Delay first run to let application fetch initial data from the RTC
    vTaskDelay(pdMS_TO_TICKS(APP_SECOND * 5));

    for (;;) {
        if (need_update) {
            esp_err_t err = fetch_data(app, APP_API_HOST, APP_API_PATH);

            if (err == ESP_OK) {
                ESP_LOGI(APP_NAME, "network update completed");
                app->net.update_ok = true;
            } else {
                ESP_LOGE(APP_NAME, "network update failed");
                app->net.update_ok = false;
            }
        }

        if (!app->net.update_ok) {
            need_update = true;
        } else {
            switch (app->time.minute) {
                case 0:
                case 15:
                case 30:
                case 45:
                    need_update = true;
                    break;
                default:
                    need_update = false;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(APP_MINUTE));
    }
}

// WiFi events handler
static void wifi_eh(void *arg, esp_event_base_t ev_base, int32_t ev_id, void *ev_data) {
    app_t *app = (app_t *) arg;

    switch (ev_id) {
        case WIFI_EVENT_STA_START: // WiFi station started
            ESP_LOGI(APP_NAME, "WiFi started");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(APP_NAME, "WiFi connected");
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(APP_NAME, "WiFi disconnected, trying to reconnect");
            app->net.update_ok = false;
            esp_wifi_connect();
            break;

        case WIFI_EVENT_AP_START: // the access point started
            ESP_LOGI(APP_NAME, "WiFi access point started");
            ESP_ERROR_CHECK(aespl_httpd_start(&app->httpd, NULL));
            ESP_ERROR_CHECK(aespl_service_init(&app->httpd, NULL));
            break;

        case WIFI_EVENT_AP_STACONNECTED:; // a station connected to the access point
            wifi_event_ap_staconnected_t *d_ap_con = (wifi_event_ap_staconnected_t *) ev_data;
            ESP_LOGI(APP_NAME, "WiFi station connected: %d, " MACSTR, d_ap_con->aid, MAC2STR(d_ap_con->mac));
            break;

        case WIFI_EVENT_AP_STADISCONNECTED:; // a station disconnected from the access point
            wifi_event_ap_stadisconnected_t *d_ap_dis = (wifi_event_ap_stadisconnected_t *) ev_data;
            ESP_LOGI(APP_NAME, "WiFi station disconnected: %d, " MACSTR, d_ap_dis->aid, MAC2STR(d_ap_dis->mac));
            break;
    }
}

// IP events handler
static void ip_eh(void *arg, esp_event_base_t ev_base, int32_t ev_id, void *event_data) {
    app_t *app = (app_t *) arg;

    switch (ev_id) {
        // Station received an IP-address
        case IP_EVENT_STA_GOT_IP:;
            ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
            ESP_LOGI(APP_NAME, "got IP address: %s", ip4addr_ntoa(&event->ip_info.ip));
            xTaskCreate(data_fetcher, "data_fetcher", 4096, (void *) app, 0, NULL);
            break;
    }
}

esp_err_t app_net_init(app_t *app) {
    esp_err_t err;

    // Initialize WiFi subsystem
    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        return err;
    }

    // Register WiFi events handler
    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_eh, (void *) app);
    if (err != ESP_OK) {
        return err;
    }

    // Register IP events handler
    err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, ip_eh, (void *) app);
    if (err != ESP_OK) {
        return err;
    }

    // Access point configuration
    wifi_config_t wifi_config = {
            .ap = {
                    .ssid = APP_WIFI_AP_SSID,
                    .ssid_len = strlen(APP_WIFI_AP_SSID),
                    .password = APP_WIFI_AP_PASS,
                    .max_connection = APP_WIFI_AP_MAX_CONN,
                    .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            },
    };
    if (strlen(APP_WIFI_AP_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    // Initialize access point
    err = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if (err != ESP_OK) {
        return err;
    }
    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    if (err != ESP_OK) {
        return err;
    }
    err = esp_wifi_start();
    if (err != ESP_OK) {
        return err;
    }

    // Set hostname
    err = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, APP_WIFI_STA_HOSTNAME);
    if (err) {
        return err;
    }

    // Set device signature
    uint8_t mac[6];
    char mac_s[13] = {0};
    char sig[50] = {0};
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    sprintf(mac_s, "%02x%02x%02x%02x%02x%02x", MAC2STR(mac));
    strncpy(sig, app->signature, 50);
    sprintf(app->signature, "%s/%s", sig, mac_s);

    ESP_LOGI(APP_NAME, "network stack initialized");

    return ESP_OK;
}
