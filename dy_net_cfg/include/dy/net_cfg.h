#pragma once

#include "esp_wifi.h"
#include "dy/error.h"

#ifdef CONFIG_BT_ENABLED
#include "dy/bt.h"
#endif

bool dy_net_cfg_net_ready();

#ifdef CONFIG_BT_ENABLED
dy_err_t dy_net_cfg_init();
#endif
