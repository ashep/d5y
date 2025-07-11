#include "esp_log.h"
#include "nvs.h"
#include "dy/error.h"

static nvs_handle_t nvs_hdl;

static char *id2key(uint16_t id, char *dst) {
    snprintf(dst, 6, "%x", id);
    return dst;
}

static dy_err_t commit() {
    esp_err_t err;

    if ((err = nvs_commit(nvs_hdl)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_commit: %s", esp_err_to_name(err));
    }

    return dy_ok();
}

dy_err_t dy_cfg2_set_u8(uint16_t id, uint8_t val) {
    esp_err_t err;
    char key[6];

    if ((err = nvs_set_u8(nvs_hdl, id2key(id, key), val)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_set_u8: %s", esp_err_to_name(err));
    }

    return commit();
}

dy_err_t dy_cfg2_set_i8(uint16_t id, int8_t val) {
    esp_err_t err;
    char key[6];

    if ((err = nvs_set_i8(nvs_hdl, id2key(id, key), val)) != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_set_i8: %s", esp_err_to_name(err));
    }

    return commit();
}

dy_err_t dy_cfg2_get_u8(uint16_t id, uint8_t *dst) {
    char key[6];

    if (dst == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "dst is null");
    }

    esp_err_t err = nvs_get_u8(nvs_hdl, id2key(id, key), dst);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return dy_err(DY_ERR_NOT_FOUND, "key not found");
    } else if (err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_get_u8: %s", esp_err_to_name(err));
    }

    return dy_ok();
}

dy_err_t dy_cfg2_get_i8(uint16_t id, int8_t *dst) {
    char key[6];

    if (dst == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "dst is null");
    }

    esp_err_t err = nvs_get_i8(nvs_hdl, id2key(id, key), dst);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return dy_err(DY_ERR_NOT_FOUND, "key not found");
    } else if (err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_get_i8: %s", esp_err_to_name(err));
    }

    return dy_ok();
}

dy_err_t dy_cfg2_init() {
    esp_err_t esp_err = nvs_open("config", NVS_READWRITE, &nvs_hdl);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_open: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}