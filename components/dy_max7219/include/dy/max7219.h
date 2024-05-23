/**
 * Author: Oleksandr Shepetko <a@shepetko.com>
 * License: MIT
 */

#ifndef DY_MAX7219_H
#define DY_MAX7219_H

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
} dy_max7219_addr_t;

typedef enum {
    DY_MAX7219_DECODE_MODE_NONE = 0x00,
    DY_MAX7219_DECODE_MODE_1 = 0x01,
    DY_MAX7219_DECODE_MODE_2 = 0x0f,
    DY_MAX7219_DECODE_MODE_3 = 0xff,
} dy_max7219_decode_mode_t;

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
} dy_max7219_intensity_t;

typedef enum {
    DY_MAX7219_SCAN_LIMIT_1,
    DY_MAX7219_SCAN_LIMIT_2,
    DY_MAX7219_SCAN_LIMIT_3,
    DY_MAX7219_SCAN_LIMIT_4,
    DY_MAX7219_SCAN_LIMIT_5,
    DY_MAX7219_SCAN_LIMIT_6,
    DY_MAX7219_SCAN_LIMIT_7,
    DY_MAX7219_SCAN_LIMIT_8,
} dy_max7219_scan_limit_t;

typedef enum {
    DY_MAX7219_POWER_OFF,
    DY_MAX7219_POWER_ON,
} dy_max7219_power_mode_t;

typedef enum {
    DY_MAX7219_TEST_MODE_DISABLE,
    DY_MAX7219_TEST_MODE_ENABLE,
} dy_max7219_test_mode_t;

typedef struct {
    gpio_num_t pin_cs;
    gpio_num_t pin_clk;
    gpio_num_t pin_data;
    uint8_t nx; // number of devices by X axis
    uint8_t ny; // number of devices by Y axis
    bool rx; // reverse X axis
    bool ry; // reverse Y axis
    dy_max7219_decode_mode_t decode;
    dy_max7219_intensity_t intensity;
    dy_max7219_scan_limit_t scan_limit;
    dy_max7219_power_mode_t power;
    dy_max7219_test_mode_t test;
} dy_max7219_config_t;

/**
 * Initialize an MCU to work with MAX7219.
 */
dy_err_t dy_max7219_init(
    dy_max7219_config_t *cfg,
    gpio_num_t cs,
    gpio_num_t clk,
    gpio_num_t data,
    uint8_t nx,
    uint8_t ny,
    bool rx,
    bool ry);

/**
 * Latch device's registers.
 */
dy_err_code_t dy_max7219_latch(const dy_max7219_config_t *cfg);

/**
 * Send a command to single device.
 */
dy_err_code_t dy_max7219_send(const dy_max7219_config_t *cfg, dy_max7219_addr_t addr, uint8_t data);

/**
 * Send a command to all devices.
 */
dy_err_code_t dy_max7219_send_all(const dy_max7219_config_t *cfg, dy_max7219_addr_t addr, uint8_t data);

/**
 * Sometimes data sent from an MCU to a device over wires can be
 * corrupted which sometimes leads to improper interpretation by the device,
 * which in its turn leads to final image corruption. The idea is to
 * periodically send configuration data to the device to refresh its registers
 * with proper values. Use this function in periodic call, for example one time
 * per minute or rarely, which depends on particular schematic and amount of
 * noise from your PSU or other sources.
 */
dy_err_code_t dy_max7219_refresh(const dy_max7219_config_t *cfg);

/**
 * Clear digits of all devices
 */
dy_err_code_t dy_max7219_clear(const dy_max7219_config_t *cfg);

#endif // DY_MAX7219_H
