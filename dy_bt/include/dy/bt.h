#pragma once

#include "esp_bt_defs.h"
#include "esp_gatts_api.h"
#include "dy/error.h"

#define DY_BT_DEVICE_NAME_PREFIX_MAX_LEN 10
#define DY_BT_DEFAULT_SVC_UUID 0xFFFF

#define DY_BT_EVENT_BASE "DY_BT_EVENT_BASE"

enum {
    DY_BT_EVENT_READY, // Bluetooth is set up and ready
};

typedef dy_err_t (*dy_bt_chrc_reader_t)(uint8_t *val, size_t *len);

typedef dy_err_t (*dy_bt_chrc_writer_t)(const uint8_t *val, size_t len, uint16_t offset);

typedef struct {
    uint8_t address[6];
} dy_bt_evt_ready_t;

dy_err_t dy_bt_set_device_name_prefix(const char *s);

dy_err_t dy_bt_set_device_appearance(uint16_t appearance);

dy_err_t dy_bt_set_service_uuid(uint16_t svc_uuid);

dy_err_t dy_bt_register_characteristic(uint16_t uuid, dy_bt_chrc_reader_t r, dy_bt_chrc_writer_t w);

/**
 * @brief Initializes the Bluetooth stack and sets up the GATT service.
 * @return
 *  - DY_ERR_INVALID_STATE if Bluetooth is already initialized.
 *  - DY_ERR_FAILED if there was an error during initialization.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_bt_init();
