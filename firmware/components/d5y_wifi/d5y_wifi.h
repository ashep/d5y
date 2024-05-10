#ifndef D5Y_WIFI_H
#define D5Y_WIFI_H

enum cronus_wifi_state {
    D5Y_WIFI_ST_DISCONNECTED,
    D5Y_WIFI_ST_CONNECTING,
    D5Y_WIFI_ST_CONNECTED,
    D5Y_WIFI_ST_ERROR,
    D5Y_WIFI_ST_MAX = 15, // do not use larger values to not overlap with cronus_wifi_err_reason
};

enum cronus_wifi_err_reason {
    D5Y_WIFI_ERR_NONE = 0,
    D5Y_WIFI_ERR_UNKNOWN = 16, // values must start from 16 to not overlap with cronus_wifi_state
};

enum cronus_wifi_op {
    D5Y_WIFI_OP_NOP,
    D5Y_WIFI_OP_SCAN,
    D5Y_WIFI_OP_CONNECT,
    D5Y_WIFI_OP_DISCONNECT,
};

esp_err_t cronus_wifi_init();

#endif  // D5Y_WIFI_H
