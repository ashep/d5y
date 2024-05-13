#ifndef DY_WIFI_H
#define DY_WIFI_H

#include "dy/error.h"

enum dy_wifi_state {
    DY_WIFI_ST_DISCONNECTED,
    DY_WIFI_ST_CONNECTING,
    DY_WIFI_ST_CONNECTED,
    DY_WIFI_ST_ERROR,
    DY_WIFI_ST_MAX = 15, // do not use larger values to not overlap with cronus_wifi_err_reason
};

enum dy_wifi_err_reason {
    DY_WIFI_ERR_NONE = 0,
    DY_WIFI_ERR_UNKNOWN = 16, // values must start from 16 to not overlap with cronus_wifi_state
};

enum dy_wifi_op {
    DY_WIFI_OP_NOP,
    DY_WIFI_OP_SCAN,
    DY_WIFI_OP_CONNECT,
    DY_WIFI_OP_DISCONNECT,
};

dy_err_t dy_wifi_init();

#endif  // DY_WIFI_H
