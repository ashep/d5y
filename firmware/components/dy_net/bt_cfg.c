#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "dy/error.h"
#include "dy/wifi.h"
#include "dy/bt.h"

#define LTAG "DY_NET"

extern void wifi_set_config_and_connect(const char *ssid, const char *password);
extern void wifi_clear_config_and_disconnect();

// byte 0:       bits 0-3: state; bits 4-7: error reason
// byte 1-32:    connected SSID
// byte 33-64:   scanned SSID 1
// byte 65-96:   scanned SSID 2
// byte 97-128:  scanned SSID 3
// byte 129-160: scanned SSID 4
// byte 161-192: scanned SSID 5
static uint8_t buf1[193] = {};

static SemaphoreHandle_t mux;

void bt_cfg_set_wifi_state(enum dy_wifi_state st, enum dy_wifi_err_reason er) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__ );
    }

    buf1[0] = (uint8_t) st | (uint8_t) er;

    xSemaphoreGive(mux);
}

void bt_cfg_set_connected_ssid(const uint8_t *ssid) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__ );
    }

    strncpy((char *) &buf1[1], (const char *) ssid, 32);

    xSemaphoreGive(mux);
}

void bt_cfg_set_ssid_list(wifi_ap_record_t *records, int len) {
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

void on_bt_chrc_read(uint16_t *len, uint8_t **val) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(LTAG, "%s: semaphore take failed", __func__ );
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
            wifi_clear_config_and_disconnect();
            break;

        case DY_NET_OP_SCAN:
            if ((err = esp_wifi_scan_start(NULL, 0)) != ESP_OK) {
                ESP_LOGE(LTAG, "%s: esp_wifi_scan_start: %s", __func__, esp_err_to_name(err));
            }
            break;

        case DY_NET_OP_CONNECT:
            bt_cfg_set_wifi_state(DY_NET_ST_CONNECTING, DY_NET_ERR_NONE);
            ESP_LOGW(LTAG, "wifi connect request");
            wifi_set_config_and_connect((const char *) &val[1], (const char *) &val[33]);
            break;

        default:
            ESP_LOGE(LTAG, "%s: unexpected op: %d", __func__, val[0]);
            break;
    }

    return ESP_OK;
}

dy_err_t init_bt_cfg() {
    dy_err_t err;

    mux = xSemaphoreCreateMutex();
    if (mux == NULL) {
        return dy_err(DY_ERR_NO_MEM, "xSemaphoreCreateMutex returned null");
    }

    err = dy_bt_register_chrc_reader(DY_BT_CHRC_ID_1, on_bt_chrc_read);
    if (dy_nok(err)) {
        return dy_err_pfx("dy_bt_register_chrc_reader failed", err);
    }

    err = dy_bt_register_chrc_writer(DY_BT_CHRC_ID_1, on_bt_chrc_write);
    if (err.code != DY_OK) {
        return dy_err_pfx("dy_bt_register_chrc_writer failed", err);
    }

    return dy_ok();
}