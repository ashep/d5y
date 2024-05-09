#ifndef CRONUS_WIFI_H
#define CRONUS_WIFI_H

enum cronus_wifi_state {
    CRONUS_WIFI_ST_DISCONNECTED,
    CRONUS_WIFI_ST_CONNECTING,
    CRONUS_WIFI_ST_CONNECTED,
    CRONUS_WIFI_ST_ERROR,
    CRONUS_WIFI_ST_MAX = 15, // do not use larger values to not overlap with cronus_wifi_err_reason
};

enum cronus_wifi_err_reason {
    CRONUS_WIFI_ERR_NONE = 0,
    CRONUS_WIFI_ERR_UNKNOWN = 16, // values must start from 16 to not overlap with cronus_wifi_state
};

enum cronus_wifi_op {
    CRONUS_WIFI_OP_NOP,
    CRONUS_WIFI_OP_SCAN,
    CRONUS_WIFI_OP_CONNECT,
    CRONUS_WIFI_OP_DISCONNECT,
};

esp_err_t cronus_wifi_init();

#endif  // CRONUS_WIFI_H
