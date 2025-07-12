#include "dy/cfg2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nvs.h"
#include "dy/error.h"

#define DY_CFG2_NVS_NAMESPACE "dy_cfg"
#define DY_CFG2_NVS_KEY_LEN 5 // 4 hex digits + null terminator

static nvs_handle_t nvs_hdl;
static bool nvs_initialized = false;

static char *id2key(uint16_t id, char *dst) {
    snprintf(dst, DY_CFG2_NVS_KEY_LEN, "%04x", id);
    return dst;
}

static dy_err_t commit() {
    esp_err_t err = nvs_commit(nvs_hdl);
    if (err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_commit: %s", esp_err_to_name(err));
    }

    return dy_ok();
}

dy_err_t dy_cfg2_get_u8(uint16_t id, uint8_t *dst) {
    if (dst == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "dst is null");
    }

    if (!nvs_initialized) {
        return dy_err(DY_ERR_NOT_CONFIGURED, "dy_cfg2_init must be called first");
    }

    char key[DY_CFG2_NVS_KEY_LEN];
    id2key(id, key);
    esp_err_t err = nvs_get_u8(nvs_hdl, key, dst);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return dy_err(DY_ERR_NOT_FOUND, "key not found: %s", key);
    } else if (err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_get_u8: %s", esp_err_to_name(err));
    }

    return dy_ok();
}

dy_err_t dy_cfg2_set_u8(uint16_t id, uint8_t val) {
    if (!nvs_initialized) {
        return dy_err(DY_ERR_NOT_CONFIGURED, "dy_cfg2_init must be called first");
    }

    char key[DY_CFG2_NVS_KEY_LEN];
    esp_err_t err = nvs_set_u8(nvs_hdl, id2key(id, key), val);
    if (err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_set_u8: %s", esp_err_to_name(err));
    }

    return commit();
}

dy_err_t dy_cfg2_get_i8(uint16_t id, int8_t *dst) {
    if (dst == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "dst is null");
    }

    if (!nvs_initialized) {
        return dy_err(DY_ERR_NOT_CONFIGURED, "dy_cfg2_init must be called first");
    }

    char key[DY_CFG2_NVS_KEY_LEN];
    id2key(id, key);
    esp_err_t err = nvs_get_i8(nvs_hdl, key, dst);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return dy_err(DY_ERR_NOT_FOUND, "key not found: %s", key);
    } else if (err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_get_i8: %s", esp_err_to_name(err));
    }

    return dy_ok();
}

dy_err_t dy_cfg2_set_i8(uint16_t id, int8_t val) {
    if (!nvs_initialized) {
        return dy_err(DY_ERR_NOT_CONFIGURED, "dy_cfg2_init must be called first");
    }

    char key[DY_CFG2_NVS_KEY_LEN];
    esp_err_t err = nvs_set_i8(nvs_hdl, id2key(id, key), val);
    if (err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_set_i8: %s", esp_err_to_name(err));
    }

    return commit();
}

dy_err_t dy_cfg2_get_str(uint16_t id, char *dst, size_t *len) {
    if (!nvs_initialized) {
        return dy_err(DY_ERR_NOT_CONFIGURED, "dy_cfg2_init must be called first");
    }
    if (dst == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "dst is null");
    }
    if (len == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "len is null");
    }
    if (*len == 0) {
        return dy_err(DY_ERR_INVALID_ARG, "len must be greater than 0");
    }

    char key[DY_CFG2_NVS_KEY_LEN];
    id2key(id, key);
    esp_err_t err = nvs_get_str(nvs_hdl, key, dst, len);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return dy_err(DY_ERR_NOT_FOUND, "key not found: %s", key);
    } else if (err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_get_str: %s", esp_err_to_name(err));
    }

    return dy_ok();
}

dy_err_t dy_cfg2_set_str(uint16_t id, const char *val) {
    if (!nvs_initialized) {
        return dy_err(DY_ERR_NOT_CONFIGURED, "dy_cfg2_init must be called first");
    }

    char key[DY_CFG2_NVS_KEY_LEN];
    esp_err_t err = nvs_set_str(nvs_hdl, id2key(id, key), val);
    if (err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_set_str: %s", esp_err_to_name(err));
    }

    return commit();
}

dy_err_t dy_cfg2_init() {
    esp_err_t esp_err = nvs_open(DY_CFG2_NVS_NAMESPACE, NVS_READWRITE, &nvs_hdl);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_open: %s", esp_err_to_name(esp_err));
    }

    nvs_initialized = true;

    return dy_ok();
}
