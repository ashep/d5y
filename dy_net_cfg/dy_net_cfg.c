#include "dy/net_cfg.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "dy/error.h"
#include "dy/net.h"
#include "dy/bt.h"

// byte 0:       bits 0-3: state; bits 4-7: error reason
// byte 1-32:    connected SSID
// byte 33-64:   scanned SSID 1
// byte 65-96:   scanned SSID 2
// byte 97-128:  scanned SSID 3
// byte 129-160: scanned SSID 4
// byte 161-192: scanned SSID 5
static uint8_t cfg_storage[193] = {};

static SemaphoreHandle_t mux;
static wifi_ap_record_t scan_result[5];

#define LTAG "DY_NET_CFG"

static void set_state(enum dy_net_cfg_conn_state st, enum dy_net_cfg_err_reason er) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__);
        return;
    }
    cfg_storage[0] = (uint8_t) st | (uint8_t) er;
    xSemaphoreGive(mux);
}

static void set_connected_ssid(const uint8_t *ssid) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__);
        return;
    }
    strncpy((char *) &cfg_storage[1], (const char *) ssid, 32);
    xSemaphoreGive(mux);
}

static void set_ssid_list(wifi_ap_record_t *records, int len) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__);
        return;
    }

    int buf_offset = 33;
    memset(&cfg_storage[buf_offset], 0, sizeof(cfg_storage) - buf_offset);
    for (int i = 0; i < len; i++) {
        ESP_LOGI(LTAG, "%s: record: ssid=%s, rssi=%d", __func__, records[i].ssid, records[i].rssi);
        strncpy((char *) &cfg_storage[buf_offset], (const char *) records[i].ssid, 32);
        buf_offset += 32;
    }

    xSemaphoreGive(mux);
}

static void on_wifi_scan_done(wifi_event_sta_scan_done_t *data) {
    ESP_LOGI(LTAG, "%s: status=%ld, number=%d", __func__, data->status, data->number);

    uint16_t len = sizeof(scan_result) / sizeof(scan_result[0]);
    esp_err_t err = esp_wifi_scan_get_ap_records(&len, scan_result);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_scan_get_ap_records: %s", __func__, esp_err_to_name(err));
        return;
    }

    set_ssid_list(scan_result, len);
}

static void on_wifi_connect(wifi_event_sta_connected_t *ev) {
    ESP_LOGI(LTAG, "wifi sta connected: ssid=%s, auth_mode=%d", ev->ssid, ev->authmode);
    set_state(DY_NET_CFG_CONN_CONNECTED, DY_NET_CFG_ERR_NONE);
    set_connected_ssid(ev->ssid);
}

static void on_wifi_disconnect(wifi_event_sta_disconnected_t *ev) {
    ESP_LOGW(LTAG, "wifi sta disconnected: reason=%d", ev->reason);

    enum dy_net_cfg_err_reason rsn = DY_NET_CFG_ERR_NONE;
    if (ev->reason != WIFI_REASON_UNSPECIFIED) {
        rsn = DY_NET_CFG_ERR_UNKNOWN; // TODO: describe error
    }

    set_state(DY_NET_CFG_CONN_DISCONNECTED, rsn);
}

static void wifi_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data) {
    switch (event_id) {
        case WIFI_EVENT_SCAN_DONE:
            on_wifi_scan_done((wifi_event_sta_scan_done_t *) data);
            break;
        case WIFI_EVENT_STA_CONNECTED:
            on_wifi_connect((wifi_event_sta_connected_t *) data);
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            on_wifi_disconnect((wifi_event_sta_disconnected_t *) data);
            break;
        default:
            ESP_LOGI(LTAG, "%s: WIFI_EVENT: %ld", __func__, event_id);
            break;
    }
}

static dy_err_t on_bt_chrc_read(esp_bt_uuid_t uuid, uint8_t *val, size_t *len) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "xSemaphoreTake failed");
    }

    if (sizeof(cfg_storage) > *len) {
        xSemaphoreGive(mux);
        return dy_err(DY_ERR_INVALID_SIZE, "buffer too small: %zu < %zu", *len, sizeof(cfg_storage));
    }

    memcpy(val, cfg_storage, sizeof(cfg_storage));
    *len = sizeof(cfg_storage);

    xSemaphoreGive(mux);

    return dy_ok();
}

static dy_err_t on_bt_chrc_write(esp_bt_uuid_t uuid, const uint8_t *val, size_t len) {
    // byte 0: op
    // byte 1-32: SSID (optional)
    // byte 33-97: password (optional)
    if (len != 1 && len != 97) {
        return dy_err(DY_ERR_INVALID_ARG, "invalid request len");
    }

    esp_err_t err;
    switch (val[0]) {
        case DY_NET_CFG_OP_CLEAR_CONFIG:
            ESP_LOGI(LTAG, "wifi disconnect request");
            dy_net_clear_config_and_disconnect();
            break;
        case DY_NET_CFG_OP_SCAN:
            ESP_LOGI(LTAG, "wifi scan request");
            if ((err = esp_wifi_scan_start(NULL, 0)) != ESP_OK) {
                return dy_err(DY_ERR_FAILED, "esp_wifi_scan_start: %s", esp_err_to_name(err));
            }
            break;
        case DY_NET_CFG_OP_CONNECT:
            ESP_LOGI(LTAG, "wifi connect request");
            set_state(DY_NET_CFG_CONN_CONNECTING, DY_NET_CFG_ERR_NONE);
            dy_net_set_config_and_connect((const char *) &val[1], (const char *) &val[33]);
            break;
        default:
            return dy_err(DY_ERR_FAILED, "unexpected op: %d", val[0]);
    }

    return dy_ok();
}

bool dy_net_cfg_net_connected() {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__);
        return false;
    }
    bool res = (cfg_storage[0] & DY_NET_CFG_CONN_STATE_MAX) == DY_NET_CFG_CONN_CONNECTED;
    xSemaphoreGive(mux);
    return res;
}

dy_err_t dy_net_cfg_init() {
    dy_err_t err;

    mux = xSemaphoreCreateMutex();
    if (mux == NULL) {
        return dy_err(DY_ERR_NO_MEM, "xSemaphoreCreateMutex returned null");
    }

    err = dy_bt_register_characteristic(0xff01, on_bt_chrc_read, on_bt_chrc_write);
    if (dy_is_err(err)) {
        return dy_err_pfx("dy_bt_register_characteristic", err);
    }

    esp_err_t esp_err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_ev_handler, NULL);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_handler_register failed: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}