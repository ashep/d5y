#include <stdbool.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "cronus_wifi.h"
#include "cronus_bt.h"

#define LTAG "CRONUS_WIFI"

static SemaphoreHandle_t mux;

wifi_ap_record_t wifi_scan_result[5];

// byte 0:       bits 0-3: state; bits 4-7: error reason
// byte 1-32:    connected SSID
// byte 33-64:   scanned SSID 1
// byte 65-96:   scanned SSID 2
// byte 97-128:  scanned SSID 3
// byte 129-160: scanned SSID 4
// byte 161-192: scanned SSID 5
static uint8_t wifi_info[193] = {};

static esp_err_t update_wifi_info_state(enum cronus_wifi_state st, enum cronus_wifi_err_reason er) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "semaphore take failed");
        return ESP_ERR_INVALID_STATE;
    }

    wifi_info[0] = (uint8_t) st | (uint8_t) er;

    xSemaphoreGive(mux);

    return ESP_OK;
}

static esp_err_t update_wifi_info_connected_ssid(const uint8_t *ssid) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "semaphore take failed");
        return ESP_ERR_INVALID_STATE;
    }

    strncpy((char *) &wifi_info[1], (const char *) ssid, 32);

    xSemaphoreGive(mux);

    return ESP_OK;
}

static esp_err_t cronus_wifi_connect(const char *ssid, const char *password) {
    esp_err_t err;

    wifi_config_t cfg = {};
    strncpy((char *) cfg.sta.ssid, (const char *) ssid, 32);
    strncpy((char *) cfg.sta.password, (const char *) password, 64);

    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "esp_wifi_set_config failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_connect();
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "esp_wifi_connect failed: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

static void on_bt_read(uint16_t *len, uint8_t **val) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "semaphore take failed");
        return;
    }

    *len = sizeof(wifi_info);
    *val = wifi_info;

    xSemaphoreGive(mux);
}

static esp_err_t on_bt_write(uint16_t len, uint16_t offset, const uint8_t *val) {
    ESP_LOGI(LTAG, "%s: len=%d, offset=%d", __func__, len, offset);

    // byte 0: op
    // byte 1-32: SSID (optional)
    // byte 33-97: password (optional)
    if (len != 1 && len != 97) {
        ESP_LOGE(LTAG, "%s: invalid request len", __func__);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err;
    switch (val[0]) {
        case CRONUS_WIFI_OP_DISCONNECT:
            if ((err = esp_wifi_disconnect()) != ESP_OK) {
                ESP_LOGE(LTAG, "%s: esp_wifi_disconnect: %s", __func__, esp_err_to_name(err));
            }
            break;
        case CRONUS_WIFI_OP_SCAN:
            if ((err = esp_wifi_scan_start(NULL, 0)) != ESP_OK) {
                ESP_LOGE(LTAG, "%s: esp_wifi_scan_start: %s", __func__, esp_err_to_name(err));
            }
            break;
        case CRONUS_WIFI_OP_CONNECT:
            update_wifi_info_state(CRONUS_WIFI_ST_CONNECTING, CRONUS_WIFI_ERR_NONE);
            ESP_LOGW(LTAG, "wifi connect request");
            // TODO: implement
            cronus_wifi_connect((const char *) &val[1], (const char *) &val[33]);
            break;
        default:
            ESP_LOGE(LTAG, "%s: unexpected op: %d", __func__, val[0]);
            break;
    }

    return ESP_OK;
}

static void on_sta_connect(wifi_event_sta_connected_t *ev) {
    ESP_LOGI(LTAG, "sta connected: ssid=%s, auth_mode=%d", ev->ssid, ev->authmode);
    update_wifi_info_state(CRONUS_WIFI_ST_CONNECTED, CRONUS_WIFI_ERR_NONE);
    update_wifi_info_connected_ssid(ev->ssid);
}

static void on_sta_disconnect(wifi_event_sta_disconnected_t *ev) {
    ESP_LOGE(LTAG, "sta disconnected: reason=%d", ev->reason);
    enum cronus_wifi_err_reason rsn = CRONUS_WIFI_ERR_NONE;
    if (ev->reason != WIFI_REASON_UNSPECIFIED) {
        rsn = CRONUS_WIFI_ERR_UNKNOWN; // TODO: make error info more meaningful
    }

    update_wifi_info_state(CRONUS_WIFI_ST_DISCONNECTED, rsn);
}

static void on_scan_done(wifi_event_sta_scan_done_t *data) {
    ESP_LOGI(LTAG, "%s: status=%ld, number=%d", __func__, data->status, data->number);

    uint16_t rec_num = sizeof(wifi_scan_result) / sizeof(wifi_scan_result[0]);

    esp_err_t err = esp_wifi_scan_get_ap_records(&rec_num, wifi_scan_result);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_scan_get_ap_records: %s", __func__, esp_err_to_name(err));
    }

    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__);
        return;
    }

    int buf_offset = 33;
    memset(&wifi_info[buf_offset], 0, sizeof(wifi_info) - buf_offset);
    for (int i = 0; i < rec_num; i++) {
        ESP_LOGI(LTAG, "%s: record: ssid=%s, rssi=%d", __func__, wifi_scan_result[i].ssid, wifi_scan_result[i].rssi);
        strncpy((char *) &wifi_info[buf_offset], (const char *) wifi_scan_result[i].ssid, 32);
        buf_offset += 32;
    }

    xSemaphoreGive(mux);
}


static void wifi_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data) {
    switch (event_id) {
        case WIFI_EVENT_WIFI_READY:
            ESP_LOGI(LTAG, "%s: WIFI_EVENT_WIFI_READY", __func__);
            break;

        case WIFI_EVENT_SCAN_DONE:
            on_scan_done((wifi_event_sta_scan_done_t *) data);
            break;

        case WIFI_EVENT_STA_START:
            ESP_LOGI(LTAG, "%s: WIFI_EVENT_STA_START", __func__);
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

static void ip_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(LTAG, "IP_EVENT_STA_GOT_IP");
            break;

        case IP_EVENT_STA_LOST_IP:
            ESP_LOGI(LTAG, "IP_EVENT_STA_LOST_IP");
            break;

        default:
            ESP_LOGI(LTAG, "IP_EVENT: %ld", event_id);
            break;
    }
}

esp_err_t cronus_wifi_init() {
    esp_err_t err;

    err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "esp_netif_init failed: %s", esp_err_to_name(err));
        return err;
    }

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_ev_handler, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "esp_event_handler_register (WIFI_EVENT) failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_ev_handler, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "esp_event_handler_register (IP_EVENT) failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "esp_wifi_start failed: %s", esp_err_to_name(err));
        return err;
    }

    mux = xSemaphoreCreateMutex();
    if (mux == NULL) {
        ESP_LOGE(LTAG, "create mutex failed");
        return ESP_ERR_NO_MEM;
    }

    cronus_bt_register_chrc_reader(CRONUS_BT_CHRC_ID_1, on_bt_read);
    cronus_bt_register_chrc_writer(CRONUS_BT_CHRC_ID_1, on_bt_write);

    return ESP_OK;
}
