#pragma once

#include "esp_wifi.h"
#include "dy/error.h"
#include "dy/bt.h"

bool dy_net_cfg_net_ready();

dy_err_t dy_net_cfg_init(dy_bt_chrc_num btc_n);
