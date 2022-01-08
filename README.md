# Cronus Digital Clock

![Cronus MAX7219 32x8 matrix](./media/cronus-max7219-32x8-red.jpg)

## Description

**Cronus** is an electronic clock able to show and automatically synchronize time, date and weather state.

## Motivation

Working on this project I am pursuing following goals:

- the device should be easily reproducible using widely available components and home tools;
- the project should be documented enough to be easily replicated by other people;
- the circuit, PCB and the firmware should support several kinds of displays out of the box;
- no fucking Arduino;

## About displays

One of the main ideas behind this project is to develop a single circuit and firmware which will be compatible with several kinds of
displays. Currently only one display is supported, MAX7219-based single color 32x8 dot matrix, which is widely popular among DIYers and can
be easily purchased on [AliExpress](https://www.aliexpress.com/wholesale?SearchText=max7219+matrix) or any local reseller.

## How to get source code

```shell
cd ~/src && git clone --recurse-submodules https://github.com/ashep/cronus.git
```

## How to build electronic part

Coming soon.

## How yo build enclosure

Coming soon.

## How to build and upload firmware

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

### v1.1 (2022-??-??)

Schematic:

- Added forgotten pull-down to GPIO 15 (R8).
- Added forgotten pull-up to GPIO 2 (R8).
- NC pins of DS3231 connected to GND as its datasheet requires.

Firmware:

- Lots of bug fixes and improvements.

### v1.0 (2021-07-13)

First unstable development version.
