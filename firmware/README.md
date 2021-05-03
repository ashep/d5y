# Cronus Firmware

## Preparation

1. Download and setup the [ESP8266 RTOS SDK](https://github.com/espressif/ESP8266_RTOS_SDK) and ESP toolchain.
2. Add ESP toolchain `bin` location to the `PATH` env variable.
3. Point the `ESP_IDF` env variable to the ESP8266 RTOS SDK location.
4. Setup the connection between your ESP8266 and computer.
5. Switch your ESP8266 to firmware upload mode.

Building
--------

```shell
make flash flash_storage
```
