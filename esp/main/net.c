/**
 * @brief     Cronus Digital Clock Network Functions
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "http_parser.h"
#include "aespl_httpd.h"
#include "aespl_cfs.h"
#include "aespl_http_client.h"
#include "app_main.h"

static void fetch_data(app_t *app, const char *url) {
    aespl_http_response resp;
    http_header_handle_t hdr = http_header_init();

    esp_err_t err = aespl_http_client_get_json(&resp, url, hdr);
    if (err) {
        aespl_http_client_free(&resp);
        ESP_LOGE(APP_NAME, "error while requesting %s: %d", url, err);
        return;
    }

    if (resp.status_code != 200) {
        aespl_http_client_free(&resp);
        ESP_LOGE(APP_NAME, "error while requesting %s, status %d", url, resp.status_code);
        return;
    }

    ESP_LOGD(APP_NAME, "request OK: %s", url);

    if (xSemaphoreTake(app->mux, (TickType_t)10) != pdTRUE) {
        aespl_http_client_free(&resp);
        ESP_LOGE(APP_NAME, "error while taking semaphore");
        return;
    }

    if (strcmp(url, APP_API_URL_TIME) == 0) {
        app->time.year = cJSON_GetObjectItem(resp.json, "year")->valueint;
        app->time.month = cJSON_GetObjectItem(resp.json, "month")->valueint;
        app->time.day = cJSON_GetObjectItem(resp.json, "day")->valueint;
        app->time.wday = cJSON_GetObjectItem(resp.json, "wday")->valueint;
        app->time.hour = cJSON_GetObjectItem(resp.json, "hour")->valueint;
        app->time.minute = cJSON_GetObjectItem(resp.json, "min")->valueint;
        app->time.second = cJSON_GetObjectItem(resp.json, "sec")->valueint;
    } else if (strcmp(url, APP_API_URL_WEATHER) == 0) {
        app->weather.temp = cJSON_GetObjectItem(resp.json, "the_temp")->valuedouble;
    }

    xSemaphoreGive(app->mux);

    aespl_http_client_free(&resp);
}

static void time_fetcher(void *args) {
    app_t *app = (app_t *)args;

    for (;;) {
        fetch_data(app, APP_API_URL_TIME);
        // vTaskDelay(pdMS_TO_TICKS(3600000)); // 60 minutes
        vTaskDelay(pdMS_TO_TICKS(10000));
    }

    vTaskDelete(NULL);
}

static void weather_fetcher(void *args) {
    app_t *app = (app_t *)args;

    for (;;) {
        fetch_data(app, APP_API_URL_WEATHER);
        // vTaskDelay(pdMS_TO_TICKS(1800000)); // 30 minutes
        vTaskDelay(pdMS_TO_TICKS(10000));
    }

    vTaskDelete(NULL);
}

// WiFi events handler
static void wifi_eh(void *arg, esp_event_base_t ev_base, int32_t ev_id, void *event_data) {
    app_t *app = (app_t *)arg;

    switch (ev_id) {
        // The access point started
        case WIFI_EVENT_AP_START:
            ESP_LOGI(APP_NAME, "access point started");
            ESP_ERROR_CHECK(aespl_httpd_start(&app->httpd, NULL));
            ESP_ERROR_CHECK(aespl_cfs_init(&app->httpd, NULL));
            break;

        // A station connected to the access point
        case WIFI_EVENT_AP_STACONNECTED:;
            wifi_event_ap_staconnected_t *ev_st_conn = (wifi_event_ap_staconnected_t *)event_data;
            ESP_LOGI(APP_NAME, "station connected: %d, " MACSTR, ev_st_conn->aid, MAC2STR(ev_st_conn->mac));
            break;

        // A station disconnected from the access point
        case WIFI_EVENT_AP_STADISCONNECTED:;
            wifi_event_ap_stadisconnected_t *ev_st_dis = (wifi_event_ap_stadisconnected_t *)event_data;
            ESP_LOGI(APP_NAME, "station disconnected: %d, " MACSTR, ev_st_dis->aid, MAC2STR(ev_st_dis->mac));
            break;

        // WiFi station started
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;
    }
}

// IP events handler
static void ip_eh(void *arg, esp_event_base_t ev_base, int32_t ev_id, void *event_data) {
    app_t *app = (app_t *)arg;

    if (ev_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(APP_NAME, "got IP address: %s", ip4addr_ntoa(&event->ip_info.ip));

        xTaskCreate(time_fetcher, "time_fetcher", 4096, (void *)app, 0, NULL);
        xTaskCreate(weather_fetcher, "weather_fetcher", 4096, (void *)app, 0, NULL);
    }
}

esp_err_t app_net_init(app_t *app) {
    esp_err_t err;

    tcpip_adapter_init();

    // Initialize WiFi subsystem
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        return err;
    }

    // Register WiFi events handler
    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_eh, (void *)app);
    if (err != ESP_OK) {
        return err;
    }

    // Register IP events handler
    err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, ip_eh, (void *)app);
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

    err = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, APP_WIFI_STA_HOSTNAME);
    if (err) {
        return err;
    }

    ESP_LOGI(APP_NAME, "network stack initialized");

    return ESP_OK;
}
