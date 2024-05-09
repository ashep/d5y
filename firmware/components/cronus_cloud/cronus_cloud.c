#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "cronus_cloud.h"

#define LTAG "CRONUS_CLOUD"

static void on_wifi_connect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    ESP_LOGI(LTAG, "got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
}

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ESP_LOGI(LTAG, "lost IP address");
}

esp_err_t cronus_cloud_init() {
    esp_err_t err;

    err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, on_wifi_connect, NULL);
    if (err != ESP_OK) {
        return err;
    }

    err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, on_wifi_disconnect, NULL);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}