#pragma once

#include "esp_bt_defs.h"
#include "esp_gatts_api.h"
#include "dy/error.h"

#define DY_BT_DEVICE_NAME_PREFIX_MAX_LEN 10
#define DY_BT_DEFAULT_SVC_UUID 0xFFFF
#define DY_BT_DEFAULT_CHRC_1_UUID 0xFF01
#define DY_BT_DEFAULT_CHRC_2_UUID 0xFF02

#define DY_BT_EVENT_BASE "DY_BT_EVENT_BASE"

enum {
    DY_BT_EVENT_READY, // Bluetooth is set up and ready
};

typedef enum {
    DY_BT_CHRC_1,
    DY_BT_CHRC_2,
    DY_BT_CHRC_MAX,
} dy_bt_chrc_num;

typedef void (*dy_bt_chrc_chrc_reader_t)(uint16_t *len, uint8_t **val);

typedef dy_err_t (*dy_bt_chrc_chrc_writer_t)(uint16_t len, uint16_t offset, const uint8_t *val);

typedef struct {
    uint8_t address[6];
} dy_bt_evt_ready_t;

dy_err_t dy_bt_register_chrc_reader(dy_bt_chrc_num num, dy_bt_chrc_chrc_reader_t reader);

dy_err_t dy_bt_register_chrc_writer(dy_bt_chrc_num num, dy_bt_chrc_chrc_writer_t writer);

dy_err_t dy_bt_set_device_name_prefix(const char *s);

dy_err_t dy_bt_set_device_appearance(uint16_t appearance);

dy_err_t dy_bt_set_service_uuid(uint16_t svc_uuid);

dy_err_t dy_bt_init();
