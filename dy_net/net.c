#include "dy/net.h"
#include <stdbool.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "dy/internal.h"
#include "dy/cfg2.h"

#define LTAG "DY_NET"
#define WATCHDOG_PERIOD 10000
#define WATCHDOG_STACK_SIZE 4096
#define WATCHDOG_PAUSE_MAX_MS 20000 // safety net in case WIFI_EVENT_SCAN_DONE never arrives
#define WIFI_STATE_RETRY_ATTEMPTS 10
#define WIFI_STATE_RETRY_DELAY_MS 50

static volatile bool sta_configured = false;
static volatile bool sta_connected = false;

// While paused, the watchdog does not issue reconnect attempts. Used to make room for a scan: ESP-IDF refuses to scan
// (ESP_ERR_WIFI_STATE) while the station is connecting, and the watchdog keeps it connecting almost all the time
// when the configured AP is unreachable.
static volatile bool watchdog_paused = false;
static volatile TickType_t watchdog_paused_at = 0;

static void watchdog_pause() {
    watchdog_paused_at = xTaskGetTickCount();
    watchdog_paused = true;
}

static void watchdog_resume() {
    watchdog_paused = false;
}

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
    dy_err_t err;

    switch (event_id) {
        case WIFI_EVENT_STA_CONNECTED:
            sta_configured = true;
            sta_connected = true;

            uint8_t check;
            dy_cfg2_get_u8_dft(DY_INTERNAL_CFG_ID_NET_CONFIGURED, &check, 0);
            if (!check) {
                if (dy_is_err(err = dy_cfg2_set_u8(DY_INTERNAL_CFG_ID_NET_CONFIGURED, 1))) {
                    ESP_LOGE(LTAG, "%s: dy_cfg2_set_u8(DY_INTERNAL_CFG_ID_NET_CONFIGURED) failed: %s",
                             __func__, dy_err_str(err));
                }
            }
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            sta_connected = false;
            break;

        case WIFI_EVENT_SCAN_DONE:
            watchdog_resume();
            break;

        default:
            ESP_LOGI(LTAG, "%s: WIFI_EVENT: %ld", __func__, event_id);
            break;
    }
}

void dy_net_set_config_and_connect(const char *ssid, const char *password) {
    wifi_config_t cfg = {};
    strncpy((char *) cfg.sta.ssid, ssid, 32);
    strncpy((char *) cfg.sta.password, password, 64);

    esp_err_t err;

    // ESP-IDF refuses to change the STA config while the station is connecting (ESP_ERR_WIFI_STATE),
    // so abort any ongoing connection attempt first and keep the watchdog from starting a new one meanwhile.
    watchdog_pause();
    if ((err = esp_wifi_disconnect()) != ESP_OK) {
        ESP_LOGW(LTAG, "%s: esp_wifi_disconnect: %s", __func__, esp_err_to_name(err));
    }

    // The disconnect is processed asynchronously by the Wi-Fi task, so the station may still be in the connecting
    // state for a short moment.
    for (int i = 0; i < WIFI_STATE_RETRY_ATTEMPTS; i++) {
        err = esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg);
        if (err != ESP_ERR_WIFI_STATE) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(WIFI_STATE_RETRY_DELAY_MS));
    }
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_set_config failed: %s", __func__, esp_err_to_name(err));
        watchdog_resume();
        return;
    }

    if ((err = esp_wifi_connect()) != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_connect failed: %s", __func__, esp_err_to_name(err));
    }

    watchdog_resume();
}

dy_err_t dy_net_scan_start() {
    esp_err_t err;

    // Scanning is allowed while connected, but not while a connection attempt is in progress.
    // In the latter case abort the attempt and pause the watchdog until WIFI_EVENT_SCAN_DONE.
    if (!sta_connected) {
        watchdog_pause();
        if ((err = esp_wifi_disconnect()) != ESP_OK) {
            ESP_LOGW(LTAG, "%s: esp_wifi_disconnect: %s", __func__, esp_err_to_name(err));
        }
    }

    // See the comment in dy_net_set_config_and_connect().
    for (int i = 0; i < WIFI_STATE_RETRY_ATTEMPTS; i++) {
        err = esp_wifi_scan_start(NULL, false);
        if (err != ESP_ERR_WIFI_STATE) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(WIFI_STATE_RETRY_DELAY_MS));
    }

    if (err != ESP_OK) {
        watchdog_resume();
        return dy_err(DY_ERR_FAILED, "esp_wifi_scan_start: %s", esp_err_to_name(err));
    }

    return dy_ok();
}

void dy_net_clear_config_and_disconnect() {
    wifi_config_t cfg = {};
    memset(cfg.sta.ssid, 0, 32);
    memset(cfg.sta.password, 0, 64);

    esp_err_t esp_err = esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg);
    if (esp_err != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_set_config failed: %s", __func__, esp_err_to_name(esp_err));
    }

    dy_err_t err = dy_cfg2_set_u8(DY_INTERNAL_CFG_ID_NET_CONFIGURED, 0);
    if (dy_is_err(err)) {
        ESP_LOGE(LTAG, "%s: dy_cfg2_set_u8(DY_INTERNAL_CFG_ID_NET_CONFIGURED) failed: %s", __func__, dy_err_str(err));
    }

    esp_err = esp_wifi_disconnect();
    if (esp_err != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_disconnect failed: %s", __func__, esp_err_to_name(esp_err));
    }

    sta_configured = false;
}

_Noreturn static void watchdog() {
    esp_err_t err;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(WATCHDOG_PERIOD));

        if (watchdog_paused) {
            if (pdTICKS_TO_MS(xTaskGetTickCount() - watchdog_paused_at) < WATCHDOG_PAUSE_MAX_MS) {
                continue;
            }
            ESP_LOGW(LTAG, "%s: pause timed out, resuming", __func__);
            watchdog_resume();
        }

        if (!sta_configured || sta_connected) {
            continue;
        }

        ESP_LOGI(LTAG, "%s: reconnecting...", __func__);
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

    uint8_t configured = 0;
    dy_err_t err = dy_cfg2_get_u8_dft(DY_INTERNAL_CFG_ID_NET_CONFIGURED, &configured, 0);
    if (dy_is_err(err)) {
        return dy_err_pfx("dy_cfg2_get_u8_dft(DY_INTERNAL_CFG_ID_NET_CONFIGURED)", err);
    }
    sta_configured = configured != 0;

    xTaskCreate(watchdog, "watchdog", WATCHDOG_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    return dy_ok();
}
