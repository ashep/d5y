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
#include "aespl_spiffs.h"
#include "aespl_httpd.h"
#include "aespl_service.h"
#include "aespl_http_client.h"
#include "app_main.h"

static esp_err_t fetch_data(app_t *app, const char *url) {
    aespl_http_response resp;

    // Make request
    esp_err_t err = aespl_http_client_get_json(&resp, url, NULL);
    if (err) {
        aespl_http_client_free(&resp);
        ESP_LOGE(APP_NAME, "error while requesting %s: %d", url, err);
        return ESP_FAIL;
    }

    // Check response status
    if (resp.status_code != 200) {
        aespl_http_client_free(&resp);
        ESP_LOGE(APP_NAME, "error while requesting %s, status %d", url, resp.status_code);
        return ESP_FAIL;
    }

    // Lock
    if (xSemaphoreTake(app->mux, (TickType_t)10) != pdTRUE) {
        aespl_http_client_free(&resp);
        ESP_LOGE(APP_NAME, "error while locking");
        return ESP_FAIL;
    }

    // Update RTC
    if (strcmp(url, APP_API_URL_TIME) == 0) {
        app->ds3231.sec = cJSON_GetObjectItem(resp.json, "sec")->valueint;
        app->ds3231.min = cJSON_GetObjectItem(resp.json, "min")->valueint;
        app->ds3231.hour = cJSON_GetObjectItem(resp.json, "hour")->valueint;
        app->ds3231.dow = cJSON_GetObjectItem(resp.json, "dow")->valueint;
        app->ds3231.day = cJSON_GetObjectItem(resp.json, "day")->valueint;
        app->ds3231.mon = cJSON_GetObjectItem(resp.json, "month")->valueint;
        app->ds3231.year = cJSON_GetObjectItem(resp.json, "year")->valueint;
        aespl_ds3231_set_data(&app->ds3231, pdMS_TO_TICKS(APP_DS3231_TIMEOUT));
    } else if (strcmp(url, APP_API_URL_WEATHER) == 0) {
        app->weather.temp = cJSON_GetObjectItem(resp.json, "the_temp")->valuedouble;
    }

    // Free HTTP client resources
    aespl_http_client_free(&resp);

    // Unlock
    if (xSemaphoreGive(app->mux) != pdTRUE) {
        ESP_LOGE(APP_NAME, "error while unlocking");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void time_fetcher(void *args) {
    app_t *app = (app_t *)args;

    for (;;) {
        if (fetch_data(app, APP_API_URL_TIME) == ESP_OK) {
            ESP_LOGI(APP_NAME, "network time updated");
            vTaskDelay(pdMS_TO_TICKS(APP_NET_UPDATE_TIME_INETRVAL * APP_HOUR));
        } else {
            ESP_LOGE(APP_NAME, "network time update failed");
            vTaskDelay(pdMS_TO_TICKS(APP_SECOND * 10));
        }
    }

    vTaskDelete(NULL);
}

static void weather_fetcher(void *args) {
    app_t *app = (app_t *)args;

    for (;;) {
        if (fetch_data(app, APP_API_URL_WEATHER) == ESP_OK) {
            app->weather.update_ok = true;
            ESP_LOGI(APP_NAME, "network weather updated");
            vTaskDelay(pdMS_TO_TICKS(APP_NET_UPDATE_WEATHER_INETRVAL * APP_HOUR));
        } else {
            app->weather.update_ok = false;
            ESP_LOGE(APP_NAME, "network weather update failed");
            vTaskDelay(pdMS_TO_TICKS(APP_SECOND * 10));
        }
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
            ESP_ERROR_CHECK(aespl_service_init(&app->httpd, NULL));

            aespl_spiffs_register("storage", "");
            aespl_httpd_handle_spiffs(&app->httpd);

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

    switch (ev_id) {
        // Station received an IP-address
        case IP_EVENT_STA_GOT_IP:;
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(APP_NAME, "got IP address: %s", ip4addr_ntoa(&event->ip_info.ip));

            xTaskCreate(time_fetcher, "time_fetcher", 4096, (void *)app, 0, NULL);
            xTaskCreate(weather_fetcher, "weather_fetcher", 4096, (void *)app, 0, NULL);
            break;
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
