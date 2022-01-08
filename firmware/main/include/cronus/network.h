#ifndef CRONUS_NET_H
#define CRONUS_NET_H

#include "aespl/httpd.h"
#include "cronus/dtime.h"
#include "cronus/weather.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifndef APP_NET_URI_SETUP
#define APP_NET_URI_SETUP "/setup"
#endif

#ifndef APP_NET_REMOTE_API_HOST
#define APP_NET_REMOTE_API_HOST "cronus.33v.xyz"
#endif

#ifndef APP_NET_REMOTE_API_PATH
#define APP_NET_REMOTE_API_PATH "/api/1"
#endif

#ifndef APP_NET_STA_HOSTNAME
#define APP_NET_STA_HOSTNAME "Cronus"
#endif

#ifndef APP_NET_AP_MAX_CONN
#define APP_NET_AP_MAX_CONN 5
#endif

typedef struct {
    xSemaphoreHandle mux;
    char signature[100];
    int update_delay;
    aespl_httpd_t httpd;
    app_time_t *time;
    app_weather_t *weather;
} app_net_t;

esp_err_t app_net_init(app_net_t *net, app_time_t *time, app_weather_t *weather);

#endif  // CRONUS_NET_H
