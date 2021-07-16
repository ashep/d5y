//
// Created by Alexander on 16.07.2021.
//

#ifndef CRONUS_NET_H
#define CRONUS_NET_H

#ifndef APP_NET_UPDATE_TIME_INTERVAL
#define APP_NET_UPDATE_TIME_INTERVAL 12  // hours
#endif

#ifndef APP_NET_UPDATE_WEATHER_INTERVAL
#define APP_NET_UPDATE_WEATHER_INTERVAL 1  // hours
#endif

#ifndef APP_API_HOST
#define APP_API_HOST "cronus.33v.xyz"
#endif

#ifndef APP_API_PATH_TIME
#define APP_API_PATH_TIME "/api/1/time"
#endif

#ifndef APP_API_PATH_WEATHER
#define APP_API_PATH_WEATHER "/api/1/weather"
#endif

#ifndef APP_WIFI_STA_HOSTNAME
#define APP_WIFI_STA_HOSTNAME "Cronus"
#endif

#ifndef APP_WIFI_AP_SSID
#define APP_WIFI_AP_SSID "Cronus"
#endif

#ifndef APP_WIFI_AP_PASS
#define APP_WIFI_AP_PASS "cronus20"
#endif

#ifndef APP_WIFI_AP_MAX_CONN
#define APP_WIFI_AP_MAX_CONN 1
#endif

/**
 * Initializes network related things.
 *
 * @param app Application
 * @return
 */
esp_err_t app_net_init(app_t *app);


#endif //CRONUS_NET_H
