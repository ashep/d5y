#include <stdbool.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "dy/wifi.h"

#define LTAG "DY_WIFI"

extern dy_err_t setup_bt_cfg();

extern void bt_cfg_set_wifi_state(enum dy_wifi_state st, enum dy_wifi_err_reason er);

extern void bt_cfg_set_ssid_list(wifi_ap_record_t *records, int len);

extern void bt_cfg_set_connected_ssid(const uint8_t *ssid);

extern void on_bt_chrc_read(uint16_t *len, uint8_t **val);

extern esp_err_t on_bt_chrc_write(uint16_t len, uint16_t offset, const uint8_t *val);

extern void ip_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

static wifi_ap_record_t scan_result[5];

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

    err = esp_wifi_connect();
    if (err != ESP_OK) {
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

static void wifi_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data) {
    wifi_event_sta_connected_t *ev_connect;
    wifi_event_sta_disconnected_t *ev_disconnect;

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
            ev_connect = (wifi_event_sta_connected_t *) data;
            ESP_LOGI(LTAG, "sta connected: ssid=%s, auth_mode=%d", ev_connect->ssid, ev_connect->authmode);
            bt_cfg_set_wifi_state(DY_WIFI_ST_CONNECTED, DY_WIFI_ERR_NONE);
            bt_cfg_set_connected_ssid(ev_connect->ssid);
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ev_disconnect = (wifi_event_sta_disconnected_t *) data;
            ESP_LOGE(LTAG, "sta disconnected: reason=%d", ev_disconnect->reason);
            enum dy_wifi_err_reason rsn = DY_WIFI_ERR_NONE;
            if (ev_disconnect->reason != WIFI_REASON_UNSPECIFIED) {
                rsn = DY_WIFI_ERR_UNKNOWN; // TODO: make error info more meaningful
            }
            bt_cfg_set_wifi_state(DY_WIFI_ST_DISCONNECTED, rsn);
            break;

        default:
            ESP_LOGI(LTAG, "%s: WIFI_EVENT: %ld", __func__, event_id);
            break;
    }
}

dy_err_t dy_wifi_init() {
    dy_err_t err;
    esp_err_t esp_err;

    if ((esp_err = esp_netif_init()) != ESP_OK) {
        return dy_error(DY_ERR_OP_FAILED, "esp_netif_init failed: %s", esp_err_to_name(esp_err));
    }

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if ((esp_err = esp_wifi_init(&cfg)) != ESP_OK) {
        return dy_error(DY_ERR_OP_FAILED, "esp_wifi_init failed: %s", esp_err_to_name(esp_err));
    }

    esp_err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_ev_handler, NULL);
    if (esp_err != ESP_OK) {
        return dy_error(DY_ERR_OP_FAILED, "esp_event_handler_register (WIFI) failed: %s", esp_err_to_name(esp_err));
    }

    esp_err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_ev_handler, NULL);
    if (esp_err != ESP_OK) {
        return dy_error(DY_ERR_OP_FAILED, "esp_event_handler_register (IP) failed: %s", esp_err_to_name(esp_err));
    }

    if ((esp_err = esp_wifi_set_mode(WIFI_MODE_STA)) != ESP_OK) {
        return dy_error(DY_ERR_OP_FAILED, "esp_wifi_set_mode failed: %s", esp_err_to_name(esp_err));
    }

    if ((esp_err = esp_wifi_start()) != ESP_OK) {
        return dy_error(DY_ERR_OP_FAILED, "esp_wifi_start failed: %s", esp_err_to_name(esp_err));
    }

    if (dy_nok(err = setup_bt_cfg())) {
        return err;
    }

    if ((esp_err = esp_wifi_connect()) != ESP_OK) {
        ESP_LOGW(LTAG, "%s: esp_wifi_connect: %s", __func__, esp_err_to_name(esp_err));
    }

    return dy_ok();
}
