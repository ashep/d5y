#ifndef _CRONUS_ESP_MAIN_H_
#define _CRONUS_ESP_MAIN_H_

#ifndef CRONUS_API_SERVER_URL
#define CRONUS_API_SERVER_URL "http://cronus.33v.xyz/api"
#endif

#ifndef CRONUS_WIFI_AP_SSID
#define CRONUS_WIFI_AP_SSID "cronus"
#endif

#ifndef CRONUS_WIFI_AP_PASS
#define CRONUS_WIFI_AP_PASS "cronus20"
#endif

#ifndef CRONUS_WIFI_AP_MAX_STA_CONN
#define CRONUS_WIFI_AP_MAX_STA_CONN 1
#endif

void app_main();
esp_err_t cronus_net_init();
void vDataFetcherTask(void *pvParameters);

#endif
