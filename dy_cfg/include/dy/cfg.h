#pragma once

#include "dy/error.h"

#ifdef CONFIG_BT_ENABLED
#include "dy/bt.h"
#endif

#define DY_CFG_ID_APP_VER_MAJOR 0
#define DY_CFG_ID_APP_VER_MINOR 1
#define DY_CFG_ID_APP_VER_PATCH 2
#define DY_CFG_ID_APP_VER_ALPHA 3

#define DY_CFG_ID_MIN 10
#define DY_CFG_ID_MAX 255

void dy_cfg_must_set_initial(uint8_t id, uint8_t val);

dy_err_t dy_cfg_set(uint8_t id, uint8_t val);

dy_err_t dy_cfg_get_p(uint8_t id, uint8_t *val);

uint8_t dy_cfg_get(uint8_t id, uint8_t def);

#ifdef CONFIG_BT_ENABLED
dy_err_t dy_cfg_init(dy_bt_chrc_num btc_n);
#endif