#pragma once

#include "esp_wifi.h"
#include "dy/error.h"
#include "dy/bt.h"

enum dy_net_cfg_conn_state {
    DY_NET_CFG_CONN_DISCONNECTED,
    DY_NET_CFG_CONN_CONNECTING,
    DY_NET_CFG_CONN_CONNECTED,
    DY_NET_CFG_CONN_ERROR,
    DY_NET_CFG_CONN_STATE_MAX = 15, // do not use larger values to not overlap with cronus_wifi_err_reason
};

enum dy_net_cfg_err_reason {
    DY_NET_CFG_ERR_NONE = 0,
    DY_NET_CFG_ERR_UNKNOWN = 16, // values must start from 16 to not overlap with cronus_wifi_state
};

enum dy_net_cfg_op {
    DY_NET_CFG_OP_NOP,
    DY_NET_CFG_OP_SCAN,
    DY_NET_CFG_OP_CONNECT,
    DY_NET_CFG_OP_CLEAR_CONFIG,
};

bool dy_net_cfg_net_connected();
dy_err_t dy_net_cfg_init();
