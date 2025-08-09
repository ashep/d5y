#pragma once

#include "dy/error.h"
#include "dy/bt.h"

#define DY_CFG_EVENT_BASE "DY_CFG_EVENT_BASE"

#define DY_CFG_ID_APP_VER_MAJOR 0
#define DY_CFG_ID_APP_VER_MINOR 1
#define DY_CFG_ID_APP_VER_PATCH 2
#define DY_CFG_ID_APP_VER_ALPHA 3

#define DY_CFG_ID_MIN 10

enum {
    DY_CFG_EVENT_SET,
};

typedef struct {
    uint8_t id;
    uint8_t val;
} dy_cfg_evt_set_t;

void dy_cfg_must_set_initial(uint8_t id, uint8_t val);

dy_err_t dy_cfg_set(uint8_t id, uint8_t val);

dy_err_t dy_cfg_get_p(uint8_t id, uint8_t *val);

uint8_t dy_cfg_get(uint8_t id, uint8_t def);

dy_err_t dy_cfg_init();
