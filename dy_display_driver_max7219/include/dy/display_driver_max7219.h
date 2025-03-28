#pragma once

#include "driver/gpio.h"
#include "dy/error.h"

dy_err_t
dy_display_driver_max7219_init(uint8_t id, gpio_num_t cs, gpio_num_t clk, gpio_num_t data, uint8_t nx, uint8_t ny,
                               bool reverse);
