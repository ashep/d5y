#include "dy/bt.h"
#include <string.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_event.h"
#include "dy/util/uthash.h"

#define LTAG "DY_BT"

static char device_name_prefix[DY_BT_DEVICE_NAME_PREFIX_MAX_LEN] = "D5Y";
static char device_name[16];

typedef struct {
    esp_bt_uuid_t uuid;
    dy_bt_chrc_reader_t reader;
    dy_bt_chrc_writer_t writer;
    esp_gatt_if_t gatts_if;
    uint16_t conn_id;
    uint16_t attr_handle;

    // UTHash
    int id;
    UT_hash_handle hh;
} dy_bt_chrc_t;

static dy_bt_chrc_t *characteristics = NULL;

static esp_gatt_srvc_id_t service_id = {
        .id = {.uuid = {.len = ESP_UUID_LEN_16, .uuid = {.uuid16 = DY_BT_DEFAULT_SVC_UUID}}},
        .is_primary = true,
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

static void on_gatts_app_register(esp_gatt_if_t iface, struct gatts_reg_evt_param evt) {
    esp_err_t err;

    ESP_LOGI(LTAG, "GATTS: application registered: gatts_if=%d, status=%d, id=%d",
             iface, evt.status, evt.app_id);

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

    if ((err = esp_ble_gatts_create_service(iface, &service_id, 16)) != ESP_OK) {
        ESP_LOGE(LTAG, "GATTS: create service failed: %s", esp_err_to_name(err));
        return;
    }

    dy_bt_evt_ready_t ready_evt = {};
    memcpy(ready_evt.address, addr, 6);
    if ((err = esp_event_post(DY_BT_EVENT_BASE, DY_BT_EVENT_READY, &ready_evt, sizeof(ready_evt), 10)) != ESP_OK) {
        ESP_LOGE(LTAG, "GAP: post ready event failed: %s", esp_err_to_name(err));
    }

    if ((err = esp_ble_gap_config_adv_data(&adv_data)) != ESP_OK) {
        ESP_LOGE(LTAG, "GAP: advertising data set failed: %s", esp_err_to_name(err));
        return;
    }
}

static void on_gatts_service_create(struct gatts_create_evt_param evt) {
    esp_err_t err;
    dy_bt_chrc_t *chrc;

    ESP_LOGI(LTAG, "GATTS: service created, status=%d, handle=%d", evt.status, evt.service_handle);

    for (chrc = characteristics; chrc != NULL; chrc = chrc->hh.next) {
        esp_gatt_perm_t perm = 0;
        esp_gatt_char_prop_t prop = 0;

        if (chrc->reader != NULL) {
            perm |= ESP_GATT_PERM_READ;
            prop |= ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        }

        if (chrc->writer != NULL) {
            perm |= ESP_GATT_PERM_WRITE;
            prop |= ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
        }

        // No reader, no writer -- no characteristic
        if (perm == 0) {
            continue;
        }

        err = esp_ble_gatts_add_char(evt.service_handle, &chrc->uuid, perm, prop, NULL, NULL);
        if (err != ESP_OK) {
            ESP_LOGE(LTAG, "GATTS: add characteristic failed: %s", esp_err_to_name(err));
            return;
        }
    }

    err = esp_ble_gatts_start_service(evt.service_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "GATTS: start service failed: %s", esp_err_to_name(err));
        return;
    }
}

static void on_gatts_characteristic_add(struct gatts_add_char_evt_param evt) {
    if (evt.status != ESP_GATT_OK) {
        ESP_LOGE(LTAG, "GATTS: add characteristic failed: uuid=0x%x, status=%d, handle=%d",
                 evt.char_uuid.uuid.uuid16, evt.status, evt.attr_handle);
        return;
    }

    dy_bt_chrc_t *chrc = NULL;
    int id = evt.char_uuid.uuid.uuid16;
    HASH_FIND_INT(characteristics, &id, chrc);
    if (chrc == NULL) {
        ESP_LOGE(LTAG, "GATTS: unknown characteristic added: uuid=0x%x, status=%d, handle=%d",
                 evt.char_uuid.uuid.uuid16, evt.status, evt.attr_handle);
        return;
    }

    chrc->attr_handle = evt.attr_handle;
    ESP_LOGI(LTAG, "GATTS: characteristic added: uuid=0x%x, status=%d, handle=%d",
             evt.char_uuid.uuid.uuid16, evt.status, evt.attr_handle);
}

static void on_gatts_client_connect(esp_gatt_if_t iface, struct gatts_connect_evt_param evt) {
    ESP_LOGI(LTAG, "GATTS: client connected: id=%d, role=%d", evt.conn_id, evt.link_role);

    for (dy_bt_chrc_t *chrc = characteristics; chrc != NULL; chrc = chrc->hh.next) {
        chrc->gatts_if = iface;
        chrc->conn_id = evt.conn_id;
    }
}

static void on_gatts_client_disconnect(struct gatts_disconnect_evt_param evt) {
    ESP_LOGI(LTAG, "GATTS: client disconnected: id=%d, reason=%x", evt.conn_id, evt.reason);

    for (dy_bt_chrc_t *chrc = characteristics; chrc != NULL; chrc = chrc->hh.next) {
        chrc->gatts_if = 0;
        chrc->conn_id = 0;
    }

    esp_err_t err = esp_ble_gap_start_advertising(&advrt_params);
    if (err != ESP_OK) {
        ESP_LOGE(LTAG, "GAP: advertising start failed: %s", esp_err_to_name(err));
        return;
    }
}

static void on_gatts_read(esp_gatt_if_t iface, struct gatts_read_evt_param evt) {
    esp_err_t esp_err;
    dy_err_t err;
    dy_bt_chrc_t *chrc = NULL;

    ESP_LOGI(LTAG, "GATTS: read request: handle=%d, trans_id=%lu, need_rsp=%d, is_long=%d, offset=%d",
             evt.handle, evt.trans_id, evt.need_rsp, evt.is_long, evt.offset);

    if (evt.offset > 0) {
        ESP_LOGW(LTAG, "GATTS: read write requests are not supported yet: handle=%d", evt.handle);
        return;
    }

    for (chrc = characteristics; chrc != NULL; chrc = chrc->hh.next) {
        if (chrc->attr_handle != evt.handle) {
            continue;
        }
        if (chrc->reader == NULL) {
            ESP_LOGW(LTAG, "GATTS: no characteristic reader registered: uuid=0x%x", chrc->uuid.uuid.uuid16);
            return;
        }

        esp_gatt_rsp_t rsp = {
                .attr_value = {
                        .handle = evt.handle,
                },
        };

        size_t len = ESP_GATT_MAX_ATTR_LEN; // tell the reader how much space we have
        err = chrc->reader(rsp.attr_value.value, &len); // reader fills the value and updates len
        if (dy_is_err(err)) {
            ESP_LOGW(LTAG, "GATTS: characteristic read failed: %s", dy_err_str(err));
            return;
        }

        rsp.attr_value.len = len;

        esp_err = esp_ble_gatts_send_response(iface, evt.conn_id, evt.trans_id, ESP_GATT_OK, &rsp);
        if (esp_err != ESP_OK) {
            ESP_LOGE(LTAG, "GATTS: send response failed: %s", esp_err_to_name(esp_err));
            return;
        }

        return;
    }

    ESP_LOGW(LTAG, "GATTS: read request for an unknown characteristic: handle=%d", evt.handle);
}

static void on_gatts_write(struct gatts_write_evt_param evt) {
    dy_bt_chrc_t *chrc = NULL;

    ESP_LOGI(LTAG, "GATTS: write request: handle=%d, need_rsp=%d, trans_id=%lu, len=%d, offset=%d",
             evt.handle, evt.need_rsp, evt.trans_id, evt.len, evt.offset);

    // Long read requests with offset are not supported yet
    if (evt.offset > 0) {
        ESP_LOGW(LTAG, "GATTS: long write requests are not supported yet: handle=%d", evt.handle);
        return;
    }

    for (chrc = characteristics; chrc != NULL; chrc = chrc->hh.next) {
        if (chrc->attr_handle != evt.handle) {
            continue;
        }

        if (chrc->writer == NULL) {
            ESP_LOGW(LTAG, "GATTS: no characteristic writer registered: uuid=0x%x", chrc->uuid.uuid.uuid16);
            return;
        }

        dy_err_t w_err = chrc->writer(evt.value, evt.len);
        if (dy_is_err(w_err)) {
            ESP_LOGW(LTAG, "GATTS: characteristic write failed: %s", dy_err_str(w_err));
            return;
        }

        return;
    }

    ESP_LOGW(LTAG, "GATTS: write request for an unknown characteristic: handle=%d", evt.handle);
}

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

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t iface, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
        // This event occurs first after esp_ble_gatts_app_register is called
        case ESP_GATTS_REG_EVT:
            on_gatts_app_register(iface, param->reg);
            break;

        case ESP_GATTS_READ_EVT:
            on_gatts_read(iface, param->read);
            break;

        case ESP_GATTS_WRITE_EVT:
            on_gatts_write(param->write);
            break;

        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(LTAG, "GATTS: mtu set: conn_id=%d, mtu=%d", param->mtu.conn_id, param->mtu.mtu);
            break;

        case ESP_GATTS_CREATE_EVT:
            on_gatts_service_create(param->create);
            break;

        case ESP_GATTS_ADD_CHAR_EVT:
            on_gatts_characteristic_add(param->add_char);
            break;

        case ESP_GATTS_START_EVT:
            ESP_LOGI(LTAG, "GATTS: service started: status=%d, handle=%d",
                     param->start.status, param->start.service_handle);
            break;

        case ESP_GATTS_CONNECT_EVT:
            on_gatts_client_connect(iface, param->connect);
            break;

        case ESP_GATTS_DISCONNECT_EVT:
            on_gatts_client_disconnect(param->disconnect);
            break;

        case ESP_GATTS_RESPONSE_EVT:
            ESP_LOGI(LTAG, "GATTS: response sent: status=%d, handle=%d", param->rsp.status, param->rsp.handle);
            break;

        default:
            ESP_LOGW(LTAG, "GATTS: unexpected event: %d; iface=%d", event, iface);
            break;
    }
}

dy_err_t dy_bt_register_characteristic(uint16_t uuid, dy_bt_chrc_reader_t r, dy_bt_chrc_writer_t w) {
    if (initialized) {
        return dy_err(DY_ERR_INVALID_STATE, "already initialized");
    }

    dy_bt_chrc_t *chrc = (dy_bt_chrc_t *) malloc(sizeof(dy_bt_chrc_t));
    if (chrc == NULL) {
        return dy_err(DY_ERR_NO_MEM, "malloc failed");
    }

    chrc->id = uuid;
    chrc->uuid = (esp_bt_uuid_t) {.len = ESP_UUID_LEN_16, .uuid = {.uuid16 = uuid}};
    chrc->reader = r;
    chrc->writer = w;

    HASH_ADD_INT(characteristics, id, chrc);
    ESP_LOGI(LTAG, "characteristic registered: 0x%04x", chrc->uuid.uuid.uuid16);

    return dy_ok();
}

dy_err_t dy_bt_set_service_uuid(uint16_t svc_uuid) {
    if (initialized) {
        return dy_err(DY_ERR_INVALID_STATE, "already initialized");
    }

    service_id.id.uuid.uuid.uuid16 = svc_uuid;

    return dy_ok();
}

dy_err_t dy_bt_set_device_name_prefix(const char *s) {
    if (initialized) {
        return dy_err(DY_ERR_INVALID_STATE, "bluetooth is already initialized");
    }

    strlcpy(device_name_prefix, s, DY_BT_DEVICE_NAME_PREFIX_MAX_LEN);

    return dy_ok();
}

dy_err_t dy_bt_set_device_appearance(uint16_t appearance) {
    if (initialized) {
        return dy_err(DY_ERR_INVALID_STATE, "already initialized");
    }

    adv_data.appearance = appearance;

    return dy_ok();
}

dy_err_t dy_bt_init() {
    esp_err_t err;

    if (initialized) {
        return dy_err(DY_ERR_INVALID_STATE, "already initialized");
    }

    // We don't need classic mode, so release memory it occupies
    if ((err = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_bt_controller_mem_release: %s", esp_err_to_name(err));
    }

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_bt_controller_init: %s", esp_err_to_name(err));
    }

    if ((err = esp_bt_controller_enable(ESP_BT_MODE_BLE)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_bt_controller_enable: %s", esp_err_to_name(err));
    }

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    if ((err = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_bluedroid_init_with_cfg: %s", esp_err_to_name(err));
    }

    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_bluedroid_enable: %s", esp_err_to_name(err));
    }

    if ((err = esp_ble_gap_register_callback(gap_event_handler)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_ble_gap_register_callback: %s", esp_err_to_name(err));
    }

    if ((err = esp_ble_gatts_register_callback(gatts_event_handler)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_ble_gatts_register_callback: %s", esp_err_to_name(err));
    }

    if ((err = esp_ble_gatts_app_register(0)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_ble_gatts_app_register: %s", esp_err_to_name(err));
    }

    initialized = true;

    return dy_ok();
}
