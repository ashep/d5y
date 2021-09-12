#ifndef CRONUS_NET_H
#define CRONUS_NET_H

#ifndef APP_API_HOST
#define APP_API_HOST "cronus.33v.xyz"
#endif

#ifndef APP_API_PATH
#define APP_API_PATH "/api/1"
#endif

#ifndef APP_WIFI_STA_HOSTNAME
#define APP_WIFI_STA_HOSTNAME "Cronus"
#endif

#ifndef APP_WIFI_AP_MAX_CONN
#define APP_WIFI_AP_MAX_CONN 5
#endif

typedef struct {
    bool wifi_connected;
    int update_delay;
} app_net_t;

#endif //CRONUS_NET_H
