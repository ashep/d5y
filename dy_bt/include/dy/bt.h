#ifndef DY_BT_H
#define DY_BT_H

#include "esp_bt_defs.h"
#include "esp_gatts_api.h"
#include "dy/error.h"

#define DY_BT_DEVICE_NAME_PREFIX_MAX_LEN 10

#define DY_BT_SVC_UUID 0xFFFF
#define DY_BT_CHRC_1_UUID 0xFF01
#define DY_BT_CHRC_2_UUID 0xFF02

enum dy_bt_chrc_id {
    DY_BT_CHRC_ID_1,
    DY_BT_CHRC_ID_2,
    DY_BT_CHRC_ID_MAX,
};

typedef void (*dy_bt_chrc_chrc_reader_t)(uint16_t *len, uint8_t **val);

typedef esp_err_t (*dy_bt_chrc_chrc_writer_t)(uint16_t len, uint16_t offset, const uint8_t *val);

dy_err_t dy_bt_register_chrc_reader(uint8_t id, dy_bt_chrc_chrc_reader_t reader);
dy_err_t dy_bt_register_chrc_writer(uint8_t id, dy_bt_chrc_chrc_writer_t writer);
esp_err_t dy_bt_notify(enum dy_bt_chrc_id chrc_id, uint16_t len, uint8_t *val);
dy_err_t dy_bt_init(const char *dev_name_prefix);

#endif // DY_BT_H
