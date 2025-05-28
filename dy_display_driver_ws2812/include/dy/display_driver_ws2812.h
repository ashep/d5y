#pragma once

#include "led_strip.h"
#include "dy/error.h"


typedef struct {
    uint8_t ppx; // pixels per one segment, x axis
    uint8_t ppy; // pixels per one segment, y axis
    uint8_t cols; // number of segments, x axis
    uint8_t rows; // number of segments, y axis
} dy_display_driver_ws2812_segments_config_t;

typedef struct {
    gpio_num_t pin_data;
    dy_display_driver_ws2812_segments_config_t segments;
    led_strip_handle_t handle;
    uint8_t brightness;
} dy_ws2812_config_t;

dy_err_t dy_display_driver_ws2812_init(
        uint8_t id,
        gpio_num_t pin_data,
        dy_display_driver_ws2812_segments_config_t segments
);