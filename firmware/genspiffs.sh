#!/bin/bash

set -e

spiffsgen.py --page-size 256 --obj-name-len 32 --meta-len 4 --use-magic --use-magic-len --aligned-obj-ix-tables 262144 ./spiffs ./build/storage.bin
ls -l ./build/storage.bin

if [ "$1" == "w" ]; then
  if [ "$ESP_TOOL_PORT" == "" ]; then
    echo "ESP_TOOL_PORT is not set"
    exit 1
  fi

  esptool.py --port $ESP_TOOL_PORT --after no_reset write_flash 0x100000 ./build/storage.bin
fi
