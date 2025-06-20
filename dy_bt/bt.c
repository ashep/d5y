#include <string.h>

#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_event.h"
#include "freertos/semphr.h"

#include "dy/bt.h"

#define LTAG "DY_BT"

static char device_name_prefix[DY_BT_DEVICE_NAME_PREFIX_MAX_LEN + 1] = "D5Y";
static char device_name[16];

typedef struct {
    SemaphoreHandle_t mux;
    esp_bt_uuid_t uuid;
    dy_bt_chrc_chrc_reader_t read;
    dy_bt_chrc_chrc_writer_t write;
    esp_gatt_if_t gatts_if;
    uint16_t conn_id;
    uint16_t attr_handle;
} dy_bt_chrc_t;

static esp_gatt_srvc_id_t service_id = {
    .id = {.uuid = {.len = ESP_UUID_LEN_16, .uuid = {.uuid16 = DY_BT_DEFAULT_SVC_UUID}}},
    .is_primary = true,
};

static dy_bt_chrc_t chrcs[DY_BT_CHRC_MAX] = {
    [DY_BT_CHRC_1] = {
        .mux = NULL,
        .uuid = {ESP_UUID_LEN_16, {.uuid16 = DY_BT_DEFAULT_CHRC_1_UUID}},
        .read = NULL,
        .write = NULL,
        .gatts_if = 0,
        .conn_id = 0,
        .attr_handle = 0,
    },
    [DY_BT_CHRC_2] = {
        .mux = NULL,
        .uuid = {ESP_UUID_LEN_16, {.uuid16 = DY_BT_DEFAULT_CHRC_2_UUID}},
        .read = NULL,
        .write = NULL,
        .gatts_if = 0,
        .conn_id = 0,
        .attr_handle = 0,
    }
};

static esp_ble_adv_params_t advrt_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static uint8_t mnf_data[2] = {0xE5, 0x02}; // Espressif

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .appearance = 0x00, // https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf
    .manufacturer_len = sizeof(mnf_data),
    .p_manufacturer_data = mnf_data,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static bool initialized = false;

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    esp_err_t err;

    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(LTAG, "GAP: advertising data set: status=%d", param->adv_data_cmpl.status);

            if ((err = esp_ble_gap_start_advertising(&advrt_params)) != ESP_OK) {
                ESP_LOGE(LTAG, "GAP: advertising start failed: %s", esp_err_to_name(err));
                return;
            }

            break;

        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(LTAG, "GAP: scan response data set: status=%d", param->scan_rsp_data_cmpl.status);
            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            ESP_LOGI(LTAG, "GAP: advertising started: status=%d", param->adv_start_cmpl.status);
            break;

        default:
            ESP_LOGW(LTAG, "GAP: unknown event: %d", event);
            break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    esp_err_t err;

    switch (event) {
        // This event occurs first after esp_ble_gatts_app_register is called
        case ESP_GATTS_REG_EVT:
            ESP_LOGI(LTAG, "GATTS: application registered: gatts_if=%d, status=%d, id=%d",
                     gatts_if, param->reg.status, param->reg.app_id);

            const uint8_t *addr = esp_bt_dev_get_address();

            if (snprintf(device_name, 16, "%s-%x%x%x", device_name_prefix, addr[3], addr[4], addr[5]) < 0) {
                ESP_LOGE(LTAG, "prepare device name failed: insufficient buffer");
                return;
            }

            if ((err = esp_bt_dev_set_device_name(device_name)) != ESP_OK) {
                ESP_LOGE(LTAG, "GATTS: set device name failed: %s", esp_err_to_name(err));
                return;
            }
            ESP_LOGI(LTAG, "GATTS: device name set: %s", device_name);

            if ((err = esp_ble_gatts_create_service(gatts_if, &service_id, 16)) != ESP_OK) {
                ESP_LOGE(LTAG, "GATTS: create service failed: %s", esp_err_to_name(err));
                return;
            }

            dy_bt_evt_ready_t evt = {};
            memcpy(evt.address, addr, 6);
            if ((err = esp_event_post(DY_BT_EVENT_BASE, DY_BT_EVENT_READY, &evt, sizeof(evt), 10)) != ESP_OK) {
                ESP_LOGE(LTAG, "GAP: post ready event failed: %s", esp_err_to_name(err));
            }

            if ((err = esp_ble_gap_config_adv_data(&adv_data)) != ESP_OK) {
                ESP_LOGE(LTAG, "GAP: advertising data set failed: %s", esp_err_to_name(err));
                return;
            }

            break;

        case ESP_GATTS_READ_EVT:
            ESP_LOGI(LTAG, "GATTS: read request: handle=%d, trans_id=%lu, need_rsp=%d, is_long=%d, offset=%d",
                     param->read.handle, param->read.trans_id, param->read.need_rsp, param->read.is_long,
                     param->read.offset);

            for (int i = 0; i < DY_BT_CHRC_MAX; i++) {
                if (chrcs[i].attr_handle != param->read.handle) {
                    continue;
                }

                if (chrcs[i].read == NULL) {
                    ESP_LOGW(LTAG, "GATTS: no characteristic reader registered: uuid=0x%x", chrcs[i].uuid.uuid.uuid16);
                    return;
                }

                esp_gatt_rsp_t rsp = {.attr_value = {
                    .handle = param->read.handle,
                }};

                // Ask the reader to point us to the length and the value
                uint8_t *val;
                chrcs[i].read(&rsp.attr_value.len, &val);

                rsp.attr_value.len -= param->read.offset;
                memcpy(rsp.attr_value.value, val + param->read.offset, rsp.attr_value.len);

                esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
            }

            break;

        case ESP_GATTS_WRITE_EVT:
            ESP_LOGI(LTAG, "GATTS: write request: handle=%d, need_rsp=%d, trans_id=%lu, len=%d",
                     param->write.handle, param->write.need_rsp, param->write.trans_id, param->write.len);

            for (int i = 0; i < DY_BT_CHRC_MAX; i++) {
                if (chrcs[i].attr_handle != param->write.handle) {
                    continue;
                }

                if (chrcs[i].write == NULL) {
                    ESP_LOGW(LTAG, "GATTS: no characteristic writer registered: uuid=0x%x", chrcs[i].uuid.uuid.uuid16);
                    return;
                }

                // Ask the writer to write the value
                dy_err_t w_err = chrcs[i].write(param->write.len, param->write.offset, param->write.value);
                if (dy_is_err(w_err)) {
                    ESP_LOGW(LTAG, "GATTS: characteristic write failed: %s", dy_err_str(w_err));
                    return;
                }
            }

            break;

        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(LTAG, "GATTS: mtu set: conn_id=%d, mtu=%d",
                     param->mtu.conn_id, param->mtu.mtu);
            break;

        case ESP_GATTS_CREATE_EVT:
            ESP_LOGI(LTAG, "GATTS: service created, status=%d, handle=%d",
                     param->create.status, param->create.service_handle);

            for (int i = 0; i < DY_BT_CHRC_MAX; i++) {
                esp_gatt_perm_t perm = 0;
                esp_gatt_char_prop_t prop = 0;

                if (chrcs[i].read != NULL) {
                    perm |= ESP_GATT_PERM_READ;
                    prop |= ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
                }

                if (chrcs[i].write != NULL) {
                    perm |= ESP_GATT_PERM_WRITE;
                    prop |= ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
                }

                if (perm == 0) {
                    continue;
                }

                err = esp_ble_gatts_add_char(param->create.service_handle, &chrcs[i].uuid, perm, prop, NULL, NULL);
                if (err != ESP_OK) {
                    ESP_LOGE(LTAG, "GATTS: add characteristic failed: %s", esp_err_to_name(err));
                    return;
                }
            }

            if ((err = esp_ble_gatts_start_service(param->create.service_handle) != ESP_OK)) {
                ESP_LOGE(LTAG, "GATTS: start service failed: %s", esp_err_to_name(err));
                return;
            }

            break;

        case ESP_GATTS_ADD_CHAR_EVT:
            if (param->add_char.status != ESP_GATT_OK) {
                ESP_LOGE(LTAG, "GATTS: add characteristic failed: uuid=0x%x, status=%d, handle=%d",
                         param->add_char.char_uuid.uuid.uuid16, param->add_char.status, param->add_char.attr_handle);
                return;
            }

            bool known = false;
            for (int i = 0; i < DY_BT_CHRC_MAX; i++) {
                if (chrcs[i].uuid.uuid.uuid16 == param->add_char.char_uuid.uuid.uuid16) {
                    chrcs[i].attr_handle = param->add_char.attr_handle;
                    known = true;
                    break;
                }
            }

            if (!known) {
                ESP_LOGE(LTAG, "GATTS: unknown characteristic: uuid=0x%x, status=%d, handle=%d",
                         param->add_char.char_uuid.uuid.uuid16, param->add_char.status, param->add_char.attr_handle);
                return;
            }

            ESP_LOGI(LTAG, "GATTS: characteristic added: uuid=0x%x, status=%d, handle=%d",
                     param->add_char.char_uuid.uuid.uuid16, param->add_char.status, param->add_char.attr_handle);

            break;

        case ESP_GATTS_START_EVT:
            ESP_LOGI(LTAG, "GATTS: service started: status=%d, handle=%d",
                     param->start.status, param->start.service_handle);
            break;

        case ESP_GATTS_CONNECT_EVT:
            ESP_LOGI(LTAG, "GATTS: client connected: id=%d, role=%d",
                     param->connect.conn_id, param->connect.link_role);

            for (int i = 0; i < DY_BT_CHRC_MAX; i++) {
                chrcs[i].gatts_if = gatts_if;
                chrcs[i].conn_id = param->connect.conn_id;
            }

            break;

        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(LTAG, "GATTS: client disconnected: id=%d, reason=%x",
                     param->disconnect.conn_id, param->disconnect.reason);

            for (int i = 0; i < DY_BT_CHRC_MAX; i++) {
                chrcs[i].gatts_if = 0;
                chrcs[i].conn_id = 0;
            }

            if ((err = esp_ble_gap_start_advertising(&advrt_params)) != ESP_OK) {
                ESP_LOGE(LTAG, "GAP: advertising start failed: %s", esp_err_to_name(err));
                return;
            }

            break;

        case ESP_GATTS_RESPONSE_EVT:
            ESP_LOGI(LTAG, "GATTS: response sent: status=%d, handle=%d",
                     param->rsp.status, param->rsp.handle);
            break;

        default:
            ESP_LOGI(LTAG, "GATTS: unknown event: %d (if %d)", event, gatts_if);
            break;
    }
}

dy_err_t dy_bt_register_chrc_reader(dy_bt_chrc_num num, dy_bt_chrc_chrc_reader_t reader) {
    if (initialized) {
        return dy_err(DY_ERR_INVALID_STATE, "bluetooth is already initialized");
    }

    if (num >= DY_BT_CHRC_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "characteristic num is too big; max=%d", DY_BT_CHRC_MAX - 1);
    }

    if (chrcs[num].read != NULL) {
        return dy_err(DY_ERR_INVALID_STATE, "characteristic reader is already registered; num=%x", num);
    }

    chrcs[num].read = reader;

    return dy_ok();
}

dy_err_t dy_bt_register_chrc_writer(dy_bt_chrc_num num, dy_bt_chrc_chrc_writer_t writer) {
    if (initialized) {
        return dy_err(DY_ERR_INVALID_STATE, "bluetooth is already initialized");
    }

    if (num >= DY_BT_CHRC_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "characteristic num is too big; max=%d", DY_BT_CHRC_MAX - 1);
    }

    if (chrcs[num].write != NULL) {
        return dy_err(DY_ERR_INVALID_STATE, "characteristic writer is already registered; num=%x", num);
    }

    chrcs[num].write = writer;

    return dy_ok();
}

dy_err_t dy_bt_set_service_uuid(uint16_t svc_uuid) {
    if (initialized) {
        return dy_err(DY_ERR_INVALID_STATE, "bluetooth is already initialized");
    }

    service_id.id.uuid.uuid.uuid16 = svc_uuid;

    return dy_ok();
}

dy_err_t dy_bt_set_device_name_prefix(const char *s) {
    if (initialized) {
        return dy_err(DY_ERR_INVALID_STATE, "bluetooth is already initialized");
    }

    strncpy(device_name_prefix, s, DY_BT_DEVICE_NAME_PREFIX_MAX_LEN);

    return dy_ok();
}

dy_err_t dy_bt_set_device_appearance(uint16_t appearance) {
    if (initialized) {
        return dy_err(DY_ERR_INVALID_STATE, "bluetooth is already initialized");
    }

    adv_data.appearance = appearance;

    return dy_ok();
}

dy_err_t dy_bt_init() {
    esp_err_t err;

    // We don't need classic mode, so release memory it occupies
    if ((err = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_bt_controller_mem_release failed: %s", esp_err_to_name(err));
    }

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_bt_controller_init failed: %s", esp_err_to_name(err));
    }

    if ((err = esp_bt_controller_enable(ESP_BT_MODE_BLE)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_bt_controller_enable failed: %s", esp_err_to_name(err));
    }

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    if ((err = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_bluedroid_init_with_cfg failed: %s", esp_err_to_name(err));
    }

    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_bluedroid_enable failed: %s", esp_err_to_name(err));
    }

    if ((err = esp_ble_gap_register_callback(gap_event_handler)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_ble_gap_register_callback failed: %s", esp_err_to_name(err));
    }

    if ((err = esp_ble_gatts_register_callback(gatts_event_handler)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_ble_gatts_register_callback failed: %s", esp_err_to_name(err));
    }

    if ((err = esp_ble_gatts_app_register(0)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_ble_gatts_app_register failed: %s", esp_err_to_name(err));
    }

    initialized = true;

    return dy_ok();
}
