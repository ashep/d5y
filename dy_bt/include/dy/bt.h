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

/**
 * @brief Sets the device name prefix.
 *
 * @note Must be called before dy_bt_init().
 * @param s Pointer to a null-terminated string that will be used as a prefix for the device name.
 * @return
 *  - DY_ERR_INVALID_STATE if the module is already initialized.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_bt_set_device_name_prefix(const char *s);

/**
 * @brief Sets the device appearance.
 *
 * @note Must be called before dy_bt_init().
 * @param appearance 16-bit appearance value as defined in the Bluetooth specification.
 * @return
 *  - DY_ERR_INVALID_STATE if the module is already initialized.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_bt_set_device_appearance(uint16_t appearance);

/**
 * @brief Sets the service UUID.
 *
 * @note Must be called before dy_bt_init().
 * @param svc_uuid 16-bit UUID of the service.
 * @return
 *  - DY_ERR_INVALID_STATE if the module is already initialized.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_bt_set_service_uuid(uint16_t svc_uuid);

/**
 * @brief Registers a characteristic.
 *
 * @note Must be called before dy_bt_init().
 * @param uuid 16-bit UUID of the characteristic.
 * @param r Reader function that will be called when a read request is received for this characteristic.
 * @param w Writer function that will be called when a write request is received for this characteristic.
 * @return
 *  - DY_ERR_INVALID_STATE if the module is already initialized.
 *  - DY_ERR_NO_MEM if memory allocation failed.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_bt_register_characteristic(uint16_t uuid, dy_bt_chrc_reader_t r, dy_bt_chrc_writer_t w);

/**
 * @brief Initializes the Bluetooth stack and sets up the GATT service.
 * 
 * @note Must be called at last order, after all characteristics are registered.
 * @return
 *  - DY_ERR_INVALID_STATE if Bluetooth is already initialized.
 *  - DY_ERR_FAILED if there was an error during initialization.
 *  - DY_ERR_OK on success.
 */
dy_err_t dy_bt_init();
