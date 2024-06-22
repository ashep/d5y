#include <string.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "dy/error.h"
#include "dy/bt.h"
#include "dy/net.h"
#include "dy/net_cfg_bt.h"

// byte 0:       bits 0-3: state; bits 4-7: error reason
// byte 1-32:    connected SSID
// byte 33-64:   scanned SSID 1
// byte 65-96:   scanned SSID 2
// byte 97-128:  scanned SSID 3
// byte 129-160: scanned SSID 4
// byte 161-192: scanned SSID 5
static uint8_t buf1[193] = {};

static SemaphoreHandle_t mux;

static wifi_ap_record_t scan_result[5];

#define LTAG "DY_NET_CFG_BT"

static void store_state(enum dy_wifi_state st, enum dy_wifi_err_reason er) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__);
    }

    buf1[0] = (uint8_t) st | (uint8_t) er;

    xSemaphoreGive(mux);
}

static void store_connected_ssid(const uint8_t *ssid) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__);
    }

    strncpy((char *) &buf1[1], (const char *) ssid, 32);

    xSemaphoreGive(mux);
}

static void store_ssid_list(wifi_ap_record_t *records, int len) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__);
        return;
    }

    int buf_offset = 33;
    memset(&buf1[buf_offset], 0, sizeof(buf1) - buf_offset);
    for (int i = 0; i < len; i++) {
        ESP_LOGI(LTAG, "%s: record: ssid=%s, rssi=%d", __func__, records[i].ssid, records[i].rssi);
        strncpy((char *) &buf1[buf_offset], (const char *) records[i].ssid, 32);
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

    store_ssid_list(scan_result, len);
}

static void on_wifi_connect(wifi_event_sta_connected_t *ev) {
    ESP_LOGI(LTAG, "wifi sta connected: ssid=%s, auth_mode=%d", ev->ssid, ev->authmode);

    store_state(DY_NET_ST_CONNECTED, DY_NET_ERR_NONE);
    store_connected_ssid(ev->ssid);
}

static void on_wifi_disconnect(wifi_event_sta_disconnected_t *ev) {
    ESP_LOGW(LTAG, "wifi sta disconnected: reason=%d", ev->reason);

    enum dy_wifi_err_reason rsn = DY_NET_ERR_NONE;
    if (ev->reason != WIFI_REASON_UNSPECIFIED) {
        rsn = DY_NET_ERR_UNKNOWN; // TODO: describe error
    }

    store_state(DY_NET_ST_DISCONNECTED, rsn);
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

void on_bt_chrc_read(uint16_t *len, uint8_t **val) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__);
        return;
    }

    *len = sizeof(buf1);
    *val = buf1;

    xSemaphoreGive(mux);
}

esp_err_t on_bt_chrc_write(uint16_t len, uint16_t offset, const uint8_t *val) {
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
        case DY_NET_OP_DISCONNECT:
            dy_net_clear_config_and_disconnect();
            break;

        case DY_NET_OP_SCAN:
            if ((err = esp_wifi_scan_start(NULL, 0)) != ESP_OK) {
                ESP_LOGE(LTAG, "%s: esp_wifi_scan_start: %s", __func__, esp_err_to_name(err));
            }
            break;

        case DY_NET_OP_CONNECT:
            store_state(DY_NET_ST_CONNECTING, DY_NET_ERR_NONE);
            ESP_LOGW(LTAG, "wifi connect request");
            dy_net_set_config_and_connect((const char *) &val[1], (const char *) &val[33]);
            break;

        default:
            ESP_LOGE(LTAG, "%s: unexpected op: %d", __func__, val[0]);
            break;
    }

    return ESP_OK;
}

dy_err_t dy_net_cfg_bt_init(dy_bt_chrc_num btc_n) {
    dy_err_t err;

    mux = xSemaphoreCreateMutex();
    if (mux == NULL) {
        return dy_err(DY_ERR_NO_MEM, "xSemaphoreCreateMutex returned null");
    }

    err = dy_bt_register_chrc_reader(btc_n, on_bt_chrc_read);
    if (dy_nok(err)) {
        return dy_err_pfx("dy_bt_register_chrc_reader failed", err);
    }

    err = dy_bt_register_chrc_writer(btc_n, on_bt_chrc_write);
    if (dy_nok(err)) {
        return dy_err_pfx("dy_bt_register_chrc_writer failed", err);
    }

    esp_err_t esp_err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_ev_handler, NULL);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_handler_register failed: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}