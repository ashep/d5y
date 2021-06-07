#!/bin/bash

set -e

if [ "${APP_NAME}" == "" ]; then
  echo "APP_NAME env variable is not set"
  exit 1
fi

if [ "${FLASH_PORT}" == "" ]; then
  FLASH_PORT="/dev/ttyUSB0"
fi

if [ "${FLASH_SIZE}" == "" ]; then
  FLASH_SIZE="4MB"
fi

if [ "${FLASH_MODE}" == "" ]; then
  FLASH_MODE="dio"
fi

if [ "${FLASH_FREQ}" == "" ]; then
  FLASH_FREQ="26m"
fi

# Local build
if [ "$1" == "build" ]; then
  "${IDF_PATH}"/tools/idf.py build
fi

# Local flash
if [ "$1" == "flash" ] || [ "$2" == "flash" ]; then
  "${IDF_PATH}"/tools/idf.py flash
fi

# Remote flash
if [ "$1" == "r-flash" ] || [ "$2" == "r-flash" ]; then
  [ "${REMOTE_HOST}" == "" ] && echo "REMOTE_HOST env variable is not set" && exit 1

  if [ "${REMOTE_ROOT}" == "" ]; then
    echo "REMOTE_ROOT env variable is not set"
    exit 1
  fi

  if [ "${REMOTE_ESPTOOL}" == "" ]; then
    echo "REMOTE_ESPTOOL env variable is not set"
    exit 1
  fi

  # Copy local build
  ssh ${REMOTE_HOST} mkdir -p "${REMOTE_ROOT}"
  scp build/partition_table/partition-table.bin ${REMOTE_HOST}:"${REMOTE_ROOT}"/partition-table.bin
  scp build/bootloader/bootloader.bin ${REMOTE_HOST}:"${REMOTE_ROOT}"/bootloader.bin
  scp build/${APP_NAME}.bin ${REMOTE_HOST}:${REMOTE_ROOT}/${APP_NAME}.bin

  # Kill probably working remote serial monitor
  [ $(ssh ${REMOTE_HOST} killall pyserial-miniterm) ] || echo -n ""

  # Execute flash command
  ssh ${REMOTE_HOST} ${REMOTE_ESPTOOL} -p ${FLASH_PORT} -b 460800 --after hard_reset \
    write_flash --flash_mode ${FLASH_MODE} --flash_freq ${FLASH_FREQ} --flash_size ${FLASH_SIZE} \
    0x0 ${REMOTE_ROOT}/bootloader.bin \
    0x8000 ${REMOTE_ROOT}/partition-table.bin \
    0x10000 ${REMOTE_ROOT}/${APP_NAME}.bin

  ssh -t ${REMOTE_HOST} pyserial-miniterm ${FLASH_PORT} 74880
fi
