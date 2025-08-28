#include "dy/net.h"
#include <stdbool.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif_sntp.h"
#include "freertos/FreeRTOS.h"

#define LTAG "DY_NET"
#define WATCHDOG_PERIOD 10000
#define WATCHDOG_STACK_SIZE 4096

static bool sta_configured = false;
static bool sta_connected = false;

static void on_got_addr(ip_event_got_ip_t *ev) {
    ESP_LOGI(LTAG, "IP_EVENT_STA_GOT_IP: addr="IPSTR"; mask="IPSTR"; gw="IPSTR,
             IP2STR(&ev->ip_info.ip), IP2STR(&ev->ip_info.netmask), IP2STR(&ev->ip_info.gw));
}

static void on_lost_addr() {
    ESP_LOGW(LTAG, "IP_EVENT_STA_LOST_IP");
    ESP_LOGI(LTAG, "trying to reconnect");

    esp_err_t err;

    if ((err = esp_wifi_disconnect()) != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_disconnect failed: %s", __func__, esp_err_to_name(err));
    }
}

static void ip_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data) {
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            on_got_addr((ip_event_got_ip_t *) data);
            break;

        case IP_EVENT_STA_LOST_IP:
            on_lost_addr();
            break;

        default:
            ESP_LOGI(LTAG, "IP_EVENT: %ld", event_id);
            break;
    }
}

static void wifi_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data) {
    switch (event_id) {
        case WIFI_EVENT_STA_CONNECTED:
            sta_configured = true;
            sta_connected = true;
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            sta_connected = false;
            break;

        default:
            ESP_LOGI(LTAG, "%s: WIFI_EVENT: %ld", __func__, event_id);
            break;
    }
}

void dy_net_set_config_and_connect(const char *ssid, const char *password) {
    esp_err_t err;

    wifi_config_t cfg = {};
    strncpy((char *) cfg.sta.ssid, (const char *) ssid, 32);
    strncpy((char *) cfg.sta.password, (const char *) password, 64);

    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_set_config failed: %s", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_wifi_connect()) != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_connect failed: %s", __func__, esp_err_to_name(err));
        return;
    }
}

void dy_net_clear_config_and_disconnect() {
    wifi_config_t cfg = {};
    memset(cfg.sta.ssid, 0, 32);
    memset(cfg.sta.ssid, 0, 64);

    esp_err_t err = esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_set_config failed: %s", __func__, esp_err_to_name(err));
    }

    err = esp_wifi_disconnect();
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_disconnect failed: %s", __func__, esp_err_to_name(err));
    }

    sta_configured = false;
}

_Noreturn static void watchdog() {
    esp_err_t err;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(WATCHDOG_PERIOD));

        if (!sta_configured || sta_connected) {
            continue;
        }

        ESP_LOGI(LTAG, "%s: reconnecting...", __func__ );
        if ((err = esp_wifi_connect()) != ESP_OK) {
            ESP_LOGW(LTAG, "%s: esp_wifi_connect: %s", __func__, esp_err_to_name(err));
        }
    }
}

dy_err_t dy_net_init() {
    esp_err_t esp_err;

    if ((esp_err = esp_netif_init()) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_netif_init failed: %s", esp_err_to_name(esp_err));
    }

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if ((esp_err = esp_wifi_init(&cfg)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_wifi_init failed: %s", esp_err_to_name(esp_err));
    }

    esp_err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_ev_handler, NULL);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_handler_register (IP) failed: %s", esp_err_to_name(esp_err));
    }

    esp_err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_ev_handler, NULL);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_handler_register (WIFI) failed: %s", esp_err_to_name(esp_err));
    }

    if ((esp_err = esp_wifi_set_mode(WIFI_MODE_STA)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_wifi_set_mode failed: %s", esp_err_to_name(esp_err));
    }

    if ((esp_err = esp_wifi_start()) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_wifi_start failed: %s", esp_err_to_name(esp_err));
    }

    // Initial attempt to connect since at start we have no idea whether connection is configured
    if ((esp_err = esp_wifi_connect()) != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_connect: %s", __func__, esp_err_to_name(esp_err));
    }

    xTaskCreate(watchdog, "watchdog", WATCHDOG_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    return dy_ok();
}
