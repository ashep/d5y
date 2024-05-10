#ifndef D5Y_BT_H
#define D5Y_BT_H

#include "esp_bt_defs.h"
#include "esp_gatts_api.h"

#define D5Y_BT_SVC_UUID 0xFFFF
#define D5Y_BT_CHRC_1_UUID 0xFF01
#define D5Y_BT_CHRC_2_UUID 0xFF02

#define D5Y_BT_DEVICE_NAME_PREFIX "Cronus"

enum cronus_bt_chrc_id {
    D5Y_BT_CHRC_ID_1,
    D5Y_BT_CHRC_ID_2,
    D5Y_BT_CHRC_ID_MAX,
};

typedef void (*cronus_bt_chrc_chrc_reader_t)(uint16_t *len, uint8_t **val);

typedef esp_err_t (*cronus_bt_chrc_chrc_writer_t)(uint16_t len, uint16_t offset, const uint8_t *val);

esp_err_t cronus_bt_register_chrc_reader(uint8_t idx, cronus_bt_chrc_chrc_reader_t reader);
esp_err_t cronus_bt_register_chrc_writer(uint8_t idx, cronus_bt_chrc_chrc_writer_t writer);
esp_err_t cronus_bt_notify(enum cronus_bt_chrc_id chrc_id, uint16_t len, uint8_t *val);
esp_err_t cronus_bt_init();

#endif // D5Y_BT_H