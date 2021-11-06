#!/bin/bash

set -e

./tools/spiffsgen.py --page-size 256 --obj-name-len 32 --meta-len 4 --use-magic --use-magic-len --aligned-obj-ix-tables 262144 ./spiffs ./build/storage.bin
ls -l ./build/storage.bin

if [ "$1" == "w" ]; then
  if [ "$2" == "" ]; then
    echo "Port is not specified"
    exit 1
  fi

  esptool.py --port $2 --after no_reset write_flash 0x100000 ./build/storage.bin
fi
