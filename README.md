# Cronus Digital Clock

## Download sources

```shell
cd ~/src && git clone --recurse-submodules https://github.com/ashep/cronus.git
```

## Electronics

Coming soon.

## Enclosure

Coming soon.

## Firmware

### Download and setup ESP8266 RTOS SDK and toolchain

```shell
cd ~/src && git clone https://github.com/espressif/ESP8266_RTOS_SDK.git && ~/src/ESP8266_RTOS_SDK/install.sh
```

### Build and flash firmware

Replace `PORT` with your port device, e. g. `/dev/ttyUSB0`.

```shell
source ~/src/ESP8266_RTOS_SDK/export.sh && cd ~/src/cronus/firmware/ && idf.py -p PORT flash
```

### Flash SPIFFS image

Replace `PORT` with your port device, e. g. `/dev/ttyUSB0`.

```shell
cd ~/src/cronus/firmware/ && ./spiffs.sh w PORT
```

### Backend

Coming soon.

## Changelog

### v1 (2021-??-??)

First version.
