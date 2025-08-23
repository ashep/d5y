#include "dy/cfg2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"
#include "esp_event.h"
#include "nvs.h"
#include "dy/error.h"
#include "dy/util/uthash.h"

#define DY_CFG2_NVS_NAMESPACE "dy_cfg"
#define DY_CFG2_NVS_KEY_LEN 5 // 4 hex digits + null terminator

#define LTAG "DY_CFG2"

typedef struct {
    int id;
    uint8_t type;
    union {
        uint8_t val_u8;
        float val_float;
        char val_str[DY_CFG2_STR_MAX_LEN];
    };
    UT_hash_handle hh;
} dy_cfg2_cache_item_t;

static dy_cfg2_cache_item_t *cache = NULL;
static nvs_handle_t nvs_hdl;
static bool nvs_initialized = false;

static char *id2key(int id, char *dst) {
    snprintf(dst, DY_CFG2_NVS_KEY_LEN, "%04x", id);
    return dst;
}

static dy_err_t commit_nvs() {
    esp_err_t err = nvs_commit(nvs_hdl);
    if (err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_commit: %s", esp_err_to_name(err));
    }
    return dy_ok();
}

static dy_err_t set_cache(int id, uint8_t type, void *src) {
    dy_cfg2_cache_item_t *item = NULL;

    HASH_FIND_INT(cache, &id, item);
    if (item == NULL) {
        item = malloc(sizeof(dy_cfg2_cache_item_t));
        if (item == NULL) {
            return dy_err(DY_ERR_NO_MEM, "malloc failed");
        }
        item->id = id;
        item->type = type;
        HASH_ADD_INT(cache, id, item);
    } else if (item->type != type) {
        return dy_err(DY_ERR_INVALID_ARG, "type mismatch: expected %d, got %d", item->type, type);
    }

    switch (item->type) {
        case DY_CFG2_VALUE_TYPE_U8:
            item->val_u8 = *(uint8_t *) src;
            break;
        case DY_CFG2_VALUE_TYPE_FLOAT:
            item->val_float = *(float *) src;
            break;
        case DY_CFG2_VALUE_TYPE_STR:
            if (strlen((char *) src) >= DY_CFG2_STR_MAX_LEN) {
                return dy_err(DY_ERR_INVALID_ARG, "string value is too long");
            }
            strlcpy(item->val_str, (char *) src, DY_CFG2_STR_MAX_LEN);
            break;
        default:
            return dy_err(DY_ERR_INVALID_ARG, "unexpected type %d", type);
    }

    return dy_ok();
}

static dy_err_t get_from_cache(int id, uint8_t type, void *value) {
    dy_cfg2_cache_item_t *item = NULL;
    HASH_FIND_INT(cache, &id, item);
    if (item == NULL) {
        return dy_err(DY_ERR_NOT_FOUND, "not found", id);
    }
    if (item->type != type) {
        return dy_err(DY_ERR_INVALID_ARG, "type mismatch: expected %d, got %d", type, item->type);
    }

    switch (item->type) {
        case DY_CFG2_VALUE_TYPE_U8:
            *(uint8_t *) value = item->val_u8;
            break;
        case DY_CFG2_VALUE_TYPE_FLOAT:
            *(float *) value = item->val_float;
            break;
        case DY_CFG2_VALUE_TYPE_STR:
            strlcpy((char *) value, item->val_str, DY_CFG2_STR_MAX_LEN);
            break;
        default:
            return dy_err(DY_ERR_INVALID_ARG, "unexpected type %d", item->type);
    }

    return dy_ok();
}

bool dy_cfg2_is_set(int id) {
    if (!nvs_initialized) {
        return false;
    }

    dy_cfg2_cache_item_t *item = NULL;
    HASH_FIND_INT(cache, &id, item);
    if (item != NULL) {
        return true;
    }

    char key[DY_CFG2_NVS_KEY_LEN];
    id2key(id, key);

    nvs_type_t kt;
    if (nvs_find_key(nvs_hdl, key, &kt) == ESP_OK) {
        return true;
    }

    return false;
}

static dy_err_t get(int id, uint8_t type, void *dst) {
    dy_err_t err;
    esp_err_t esp_err;
    uint32_t bits;

    if (dst == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "dst is null");
    }
    if (!nvs_initialized) {
        return dy_err(DY_ERR_NOT_CONFIGURED, "dy_cfg2_init must be called first");
    }

    if (!dy_is_err(err = get_from_cache(id, type, dst))) {
        return dy_ok();
    } else if (err->code != DY_ERR_NOT_FOUND) {
        return dy_err_pfx("get_from_cache", err);
    }

    char key[DY_CFG2_NVS_KEY_LEN];
    id2key(id, key);

    switch (type) {
        case DY_CFG2_VALUE_TYPE_U8:
            esp_err = nvs_get_u8(nvs_hdl, key, (uint8_t *) dst);
            break;
        case DY_CFG2_VALUE_TYPE_FLOAT:
            esp_err = nvs_get_u32(nvs_hdl, key, &bits);
            if (esp_err == ESP_OK) {
                memcpy((float *) dst, &bits, sizeof(bits));
            }
            break;
        case DY_CFG2_VALUE_TYPE_STR: {
            size_t len = DY_CFG2_STR_MAX_LEN;
            esp_err = nvs_get_str(nvs_hdl, key, (char *) dst, &len);
            break;
        }
        default:
            return dy_err(DY_ERR_INVALID_ARG, "unexpected type %d", type);
    }

    if (esp_err == ESP_ERR_NVS_NOT_FOUND) {
        return dy_err(DY_ERR_NOT_FOUND, "key not found: %s", key);
    } else if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_get: %s", esp_err_to_name(esp_err));
    }

    if (dy_is_err(err = set_cache(id, type, dst))) {
        return dy_err_pfx("set_cache", err);
    }

    return err;
}

static dy_err_t set(int id, uint8_t type, void *src) {
    esp_err_t esp_err;
    dy_err_t err;
    uint32_t bits;

    if (!nvs_initialized) {
        return dy_err(DY_ERR_NOT_CONFIGURED, "dy_cfg2_init must be called first");
    }

    char key[DY_CFG2_NVS_KEY_LEN];
    id2key(id, key);

    switch (type) {
        case DY_CFG2_VALUE_TYPE_U8:
            esp_err = nvs_set_u8(nvs_hdl, key, *(uint8_t *) src);
            break;
        case DY_CFG2_VALUE_TYPE_FLOAT:
            memcpy(&bits, (float *) src, sizeof(bits));
            esp_err = nvs_set_u32(nvs_hdl, key, *(uint32_t *) src);
            break;
        case DY_CFG2_VALUE_TYPE_STR:
            esp_err = nvs_set_str(nvs_hdl, key, (const char *) src);
            break;
        default:
            return dy_err(DY_ERR_INVALID_ARG, "unexpected type %d", type);
    }

    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_set: %s", esp_err_to_name(esp_err));
    }

    if (dy_is_err(err = commit_nvs())) {
        return dy_err_pfx("commit_nvs", err);
    }

    if (dy_is_err(err = set_cache(id, type, src))) {
        return dy_err_pfx("set_cache", err);
    }

    dy_cfg2_evt_set_t evt = {.id = id, .type = type, .val = src};
    if ((esp_err = esp_event_post(DY_CFG2_EVENT_BASE, DY_CFG2_EVENT_SET, &evt, sizeof(evt), 10)) != ESP_OK) {
        ESP_LOGE(LTAG, "%s: post set event failed: %s", __func__, esp_err_to_name(esp_err));
    }

    return dy_ok();
}

dy_err_t dy_cfg2_get_u8(int id, uint8_t *dst) {
    return get(id, DY_CFG2_VALUE_TYPE_U8, dst);
}

dy_err_t dy_cfg2_get_u8_dft(int id, uint8_t *dst, uint8_t dft) {
    dy_err_t err = get(id, DY_CFG2_VALUE_TYPE_U8, dst);
    if (err->code == DY_ERR_NOT_FOUND) {
        *dst = dft;
        return dy_ok();
    }
    return err;
}

dy_err_t dy_cfg2_set_u8(int id, uint8_t val) {
    return set(id, DY_CFG2_VALUE_TYPE_U8, &val);
}

dy_err_t dy_cfg2_get_float(int id, float *dst) {
    return get(id, DY_CFG2_VALUE_TYPE_FLOAT, dst);
}

dy_err_t dy_cfg2_get_float_dft(int id, float *dst, float dft) {
    dy_err_t err = get(id, DY_CFG2_VALUE_TYPE_FLOAT, dst);
    if (err->code == DY_ERR_NOT_FOUND) {
        *dst = dft;
        return dy_ok();
    }
    return err;
}

dy_err_t dy_cfg2_set_float(int id, float val) {
    return set(id, DY_CFG2_VALUE_TYPE_FLOAT, &val);
}

dy_err_t dy_cfg2_get_str(int id, char *dst) {
    return get(id, DY_CFG2_VALUE_TYPE_STR, dst);
}

dy_err_t dy_cfg2_get_str_dft(int id, char *dst, const char *dft) {
    dy_err_t err = get(id, DY_CFG2_VALUE_TYPE_STR, dst);
    if (err->code == DY_ERR_NOT_FOUND) {
        strlcpy(dst, dft, DY_CFG2_STR_MAX_LEN);
        return dy_ok();
    }
    return err;
}

dy_err_t dy_cfg2_set_str(int id, const char *val) {
    return set(id, DY_CFG2_VALUE_TYPE_STR, (void *) val);
}

dy_err_t dy_cfg2_init() {
    esp_err_t esp_err = nvs_open(DY_CFG2_NVS_NAMESPACE, NVS_READWRITE, &nvs_hdl);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_open: %s", esp_err_to_name(esp_err));
    }

    nvs_initialized = true;

    return dy_ok();
}
