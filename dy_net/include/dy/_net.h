#ifndef _DY_NET_H
#define _DY_NET_H

#include "esp_err.h"
#include "esp_wifi.h"
#include "dy/error.h"
#include "dy/net.h"

#define LTAG "DY_NET"

dy_err_t init_bt_cfg();

void bt_cfg_set_wifi_state(enum dy_wifi_state st, enum dy_wifi_err_reason er);

void bt_cfg_set_ssid_list(wifi_ap_record_t *records, int len);

void bt_cfg_set_connected_ssid(const uint8_t *ssid);

void on_bt_chrc_read(uint16_t *len, uint8_t **val);

esp_err_t on_bt_chrc_write(uint16_t len, uint16_t offset, const uint8_t *val);

void ip_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

#endif // _DY_NET_H
