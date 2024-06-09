#include <stdbool.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif_sntp.h"

#include "freertos/FreeRTOS.h"

#include "dy/net.h"
#include "dy/_net.h"

#define LTAG "DY_NET"
#define WATCHDOG_PERIOD 10000
#define WATCHDOG_STACK_SIZE 4096

static SemaphoreHandle_t mux;
static wifi_ap_record_t scan_result[5];
static bool sta_configured = false;
static bool sta_connected = false;

void wifi_set_config_and_connect(const char *ssid, const char *password) {
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

void wifi_clear_config_and_disconnect() {
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

static void on_scan_done(wifi_event_sta_scan_done_t *data) {
    ESP_LOGI(LTAG, "%s: status=%ld, number=%d", __func__, data->status, data->number);

    uint16_t len = sizeof(scan_result) / sizeof(scan_result[0]);
    esp_err_t err = esp_wifi_scan_get_ap_records(&len, scan_result);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_scan_get_ap_records: %s", __func__, esp_err_to_name(err));
        return;
    }

    bt_cfg_set_ssid_list(scan_result, len);
}

static void on_sta_connect(wifi_event_sta_connected_t *ev) {
    ESP_LOGI(LTAG, "sta connected: ssid=%s, auth_mode=%d", ev->ssid, ev->authmode);

    if (xSemaphoreTake(mux, (TickType_t) 10) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: xSemaphoreTake failed", __func__ );
        return;
    }

    bt_cfg_set_wifi_state(DY_NET_ST_CONNECTED, DY_NET_ERR_NONE);
    bt_cfg_set_connected_ssid(ev->ssid);

    sta_configured = true;
    sta_connected = true;

    xSemaphoreGive(mux);
}

static void on_sta_disconnect(wifi_event_sta_disconnected_t *ev) {
    ESP_LOGE(LTAG, "sta disconnected: reason=%d", ev->reason);

    if (xSemaphoreTake(mux, (TickType_t) 10) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: xSemaphoreTake failed", __func__ );
        return;
    }

    enum dy_wifi_err_reason rsn = DY_NET_ERR_NONE;
    if (ev->reason != WIFI_REASON_UNSPECIFIED) {
        rsn = DY_NET_ERR_UNKNOWN; // TODO: describe error
    }

    sta_connected = false;
    bt_cfg_set_wifi_state(DY_NET_ST_DISCONNECTED, rsn);

    xSemaphoreGive(mux);
}

static void wifi_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data) {
    switch (event_id) {
        case WIFI_EVENT_SCAN_DONE:
            on_scan_done((wifi_event_sta_scan_done_t *) data);
            break;

        case WIFI_EVENT_STA_CONNECTED:
            on_sta_connect((wifi_event_sta_connected_t *) data);
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            on_sta_disconnect((wifi_event_sta_disconnected_t *) data);
            break;

        default:
            ESP_LOGI(LTAG, "%s: WIFI_EVENT: %ld", __func__, event_id);
            break;
    }
}

static void watchdog() {
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
    dy_err_t err;
    esp_err_t esp_err;

    mux = xSemaphoreCreateMutex();
    if (mux == NULL) {
        return dy_err(DY_ERR_NO_MEM, "xSemaphoreCreateMutex returned null");
    }

    if ((esp_err = esp_netif_init()) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_netif_init failed: %s", esp_err_to_name(esp_err));
    }

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if ((esp_err = esp_wifi_init(&cfg)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_wifi_init failed: %s", esp_err_to_name(esp_err));
    }

    esp_err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_ev_handler, NULL);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_handler_register (WIFI) failed: %s", esp_err_to_name(esp_err));
    }

    esp_err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_ev_handler, NULL);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_handler_register (IP) failed: %s", esp_err_to_name(esp_err));
    }

    if ((esp_err = esp_wifi_set_mode(WIFI_MODE_STA)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_wifi_set_mode failed: %s", esp_err_to_name(esp_err));
    }

    if ((esp_err = esp_wifi_start()) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_wifi_start failed: %s", esp_err_to_name(esp_err));
    }

    if (dy_nok(err = init_bt_cfg())) {
        return err;
    }

    // Initial attempt to connect since at start we have no idea whether connection is configured
    if ((esp_err = esp_wifi_connect()) != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_connect: %s", __func__, esp_err_to_name(esp_err));
    }

    xTaskCreate(watchdog, "watchdog", WATCHDOG_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    esp_sntp_config_t sntp_cfg = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    if ((esp_err = esp_netif_sntp_init(&sntp_cfg)) != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_netif_sntp_init: %s", __func__, esp_err_to_name(esp_err));
    }

    return dy_ok();
}
