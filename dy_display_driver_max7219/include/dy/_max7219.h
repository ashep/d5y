#pragma once

#include "driver/gpio.h"
#include "dy/error.h"

typedef enum {
    DY_MAX7219_ADDR_NOP = 0x0,
    DY_MAX7219_ADDR_DIGIT_0 = 0x1,
    DY_MAX7219_ADDR_DIGIT_1 = 0x2,
    DY_MAX7219_ADDR_DIGIT_2 = 0x3,
    DY_MAX7219_ADDR_DIGIT_3 = 0x4,
    DY_MAX7219_ADDR_DIGIT_4 = 0x5,
    DY_MAX7219_ADDR_DIGIT_5 = 0x6,
    DY_MAX7219_ADDR_DIGIT_6 = 0x7,
    DY_MAX7219_ADDR_DIGIT_7 = 0x8,
    DY_MAX7219_ADDR_DECODE_MODE = 0x9,
    DY_MAX7219_ADDR_INTENSITY = 0xa,
    DY_MAX7219_ADDR_SCAN_LIMIT = 0xb,
    DY_MAX7219_ADDR_SHUTDOWN = 0xc,
    DY_MAX7219_ADDR_TEST = 0xf,
} max7219_addr_t;

typedef enum {
    DY_MAX7219_DECODE_MODE_NONE = 0x00,
    DY_MAX7219_DECODE_MODE_1 = 0x01,
    DY_MAX7219_DECODE_MODE_2 = 0x0f,
    DY_MAX7219_DECODE_MODE_3 = 0xff,
} max7219_decode_mode_t;

typedef enum {
    DY_MAX7219_INTENSITY_MIN,
    DY_MAX7219_INTENSITY_1,
    DY_MAX7219_INTENSITY_2,
    DY_MAX7219_INTENSITY_3,
    DY_MAX7219_INTENSITY_4,
    DY_MAX7219_INTENSITY_5,
    DY_MAX7219_INTENSITY_6,
    DY_MAX7219_INTENSITY_7,
    DY_MAX7219_INTENSITY_8,
    DY_MAX7219_INTENSITY_9,
    DY_MAX7219_INTENSITY_10,
    DY_MAX7219_INTENSITY_11,
    DY_MAX7219_INTENSITY_12,
    DY_MAX7219_INTENSITY_13,
    DY_MAX7219_INTENSITY_14,
    DY_MAX7219_INTENSITY_MAX,
} max7219_intensity_t;

typedef enum {
    DY_MAX7219_SCAN_LIMIT_1,
    DY_MAX7219_SCAN_LIMIT_2,
    DY_MAX7219_SCAN_LIMIT_3,
    DY_MAX7219_SCAN_LIMIT_4,
    DY_MAX7219_SCAN_LIMIT_5,
    DY_MAX7219_SCAN_LIMIT_6,
    DY_MAX7219_SCAN_LIMIT_7,
    DY_MAX7219_SCAN_LIMIT_8,
} max7219_scan_limit_t;

typedef enum {
    DY_MAX7219_POWER_OFF,
    DY_MAX7219_POWER_ON,
} max7219_power_mode_t;

typedef enum {
    DY_MAX7219_TEST_MODE_DISABLE,
    DY_MAX7219_TEST_MODE_ENABLE,
} max7219_test_mode_t;

typedef struct {
    gpio_num_t pin_cs;
    gpio_num_t pin_clk;
    gpio_num_t pin_data;
    uint8_t nx; // number of devices by X axis
    uint8_t ny; // number of devices by Y axis
    bool reverse; // true if the last device connected first
    max7219_decode_mode_t decode;
    max7219_intensity_t intensity;
    max7219_scan_limit_t scan_limit;
    max7219_power_mode_t power;
    max7219_test_mode_t test;
} max7219_config_t;

dy_err_t
max7219_init(max7219_config_t *cfg, gpio_num_t cs, gpio_num_t clk, gpio_num_t data, uint8_t nx, uint8_t ny, bool rx);

dy_err_t max7219_latch(const max7219_config_t *cfg);

dy_err_t max7219_send(const max7219_config_t *cfg, max7219_addr_t addr, uint8_t data);

dy_err_t max7219_send_all(const max7219_config_t *cfg, max7219_addr_t addr, uint8_t data);

/**
 * Sometimes data sent from an MCU to a device over wires can be
 * corrupted which leads to improper interpretation by the device,
 * which in its turn leads to final image corruption. The idea is to
 * periodically send configuration data to the device to refresh its registers
 * with proper values. Use this function in periodic call, for example one time
 * per minute or rarely, which depends on particular schematic and amount of
 * noise from your PSU or other sources.
 */
dy_err_t max7219_refresh(const max7219_config_t *cfg);

dy_err_t max7219_clear(const max7219_config_t *cfg);