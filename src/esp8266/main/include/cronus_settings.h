#ifndef _CRONUS_SETTINGS_H_
#define _CRONUS_SETTINGS_H_

#include "esp_err.h"

#define CS_WIFI_STA_SSID "wifi_sta_ssid"
#define CS_WIFI_STA_PASSWORD "wifi_sta_pwd"

esp_err_t cronus_settings_set_str(const char *key, const char *value);
esp_err_t cronus_settings_get_str(const char *key, char *out_value);
void cronus_settings_init();

#endif