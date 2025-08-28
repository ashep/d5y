# D5Y

A minimalistic ESP-IDF library for building ESP32-based devices.

## Installation

```shell
cd your-project
git submodule add --name d5y git@github.com:ashep/d5y.git vendor/d5y
```

Add the following to your project's `CMakeLists.txt`:

```cmake
set(EXTRA_COMPONENT_DIRS ./vendor/d5y)
```

## Available Modules

- [appinfo](dy_appinfo/README.md). Store and get application info.
- [bt](dy_bt/README.md). Bluetooth helpers.
- [cfg2](dy_cfg2/README.md). Configuration storage.
- [cloud](dy_cloud/README.md). [D5Y Cloud](https://github.com/ashep/d5y-cloud) client.
- [display](dy_display/README.md). Display output functions.
- [display_driver_max7219](dy_display_driver_max7219/README.md). MAX7219-based display driver.
- [display_driver_ws2812](dy_display_driver_ws2812/README.md). WS2812-based display driver.
- [ds3231](dy_ds3231/README.md). DS3231 real-time clock driver.
- [error](dy_error/README.md). Error-related helpers.
- [gfx](dy_gfx/README.md). Framebuffer and graphics operations.
- [net](dy_net/README.md). Network helpers.
- [net_cfg](dy_net_cfg/README.md). Network configuration over Bluetooth.
- [rtc](dy_rtc/README.md). Real-time clock helpers.
- [util](dy_util/README.md). Miscellaneous utilities.

## Changelog

## Authors

- [Oleksandr Shepetko](https://shepetko.com).
