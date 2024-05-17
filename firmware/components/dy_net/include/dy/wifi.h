#ifndef DY_NET_H
#define DY_NET_H

#include "dy/error.h"

enum dy_wifi_state {
    DY_NET_ST_DISCONNECTED,
    DY_NET_ST_CONNECTING,
    DY_NET_ST_CONNECTED,
    DY_NET_ST_ERROR,
    DY_NET_ST_MAX = 15, // do not use larger values to not overlap with cronus_wifi_err_reason
};

enum dy_wifi_err_reason {
    DY_NET_ERR_NONE = 0,
    DY_NET_ERR_UNKNOWN = 16, // values must start from 16 to not overlap with cronus_wifi_state
};

enum dy_wifi_op {
    DY_NET_OP_NOP,
    DY_NET_OP_SCAN,
    DY_NET_OP_CONNECT,
    DY_NET_OP_DISCONNECT,
};

dy_err_t dy_net_init();

#endif  // DY_NET_H
