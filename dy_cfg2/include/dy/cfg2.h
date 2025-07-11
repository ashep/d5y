#pragma once

#include "dy/error.h"

dy_err_t dy_cfg2_init();

dy_err_t dy_cfg2_set_u8(uint16_t id, uint8_t val);
dy_err_t dy_cfg2_set_i8(uint16_t id, int8_t val);

dy_err_t dy_cfg2_get_u8(uint16_t id, uint8_t *dst);
dy_err_t dy_cfg2_get_i8(uint16_t id, int8_t *dst);
