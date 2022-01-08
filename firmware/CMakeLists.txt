cmake_minimum_required(VERSION 3.19)

set(EXCLUDE_COMPONENTS bootloader coap console esp-wolfssl esp_https_ota fatfs freemodbus jsmn libsodium mdns mqtt protobuf-c protocomm smartconfig_ack spi_ram openssl wear_levelling wifi_provisioning)
set(EXTRA_COMPONENT_DIRS lib/aespl/components)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(cronus C)