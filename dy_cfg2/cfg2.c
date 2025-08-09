#include "dy/cfg2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nvs.h"
#include "dy/error.h"
#include "dy/util/uthash.h"

#define DY_CFG2_NVS_NAMESPACE "dy_cfg"
#define DY_CFG2_NVS_KEY_LEN 5 // 4 hex digits + null terminator

#define DY_CFG2_VALUE_TYPE_U8  0x01
#define DY_CFG2_VALUE_TYPE_I8  0x02
#define DY_CFG2_VALUE_TYPE_STR 0x03

typedef struct {
    int id;
    uint8_t type;
    union {
        uint8_t val_u8;
        int8_t val_i8;
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
        case DY_CFG2_VALUE_TYPE_I8:
            item->val_i8 = *(int8_t *) src;
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
        case DY_CFG2_VALUE_TYPE_I8:
            *(int8_t *) value = item->val_i8;
            break;
        case DY_CFG2_VALUE_TYPE_STR:
            strlcpy((char *) value, item->val_str, DY_CFG2_STR_MAX_LEN);
            break;
        default:
            return dy_err(DY_ERR_INVALID_ARG, "unexpected type %d", item->type);
    }

    return dy_ok();
}

static dy_err_t get(int id, uint8_t type, void *dst) {
    dy_err_t err;
    esp_err_t esp_err;

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
        case DY_CFG2_VALUE_TYPE_I8:
            esp_err = nvs_get_i8(nvs_hdl, key, (int8_t *) dst);
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

    err = set_cache(id, type, dst);
    if (dy_is_err(err)) {
        return dy_err_pfx("set_cache", err);
    }

    return err;
}

static dy_err_t set(int id, uint8_t type, void *src, bool overwrite) {
    esp_err_t esp_err;
    dy_err_t err;

    if (!nvs_initialized) {
        return dy_err(DY_ERR_NOT_CONFIGURED, "dy_cfg2_init must be called first");
    }

    if (!overwrite) {
        uint8_t tmp[DY_CFG2_STR_MAX_LEN];
        err = get(id, type, &tmp);
        if (!dy_is_err(err)) {
            return dy_ok(); // value is already exists
        }
        if(dy_is_err(err) && err->code != DY_ERR_NOT_FOUND) {
            return dy_err(DY_ERR_FAILED, "get: %s", dy_err_str(err));
        }
    }

    char key[DY_CFG2_NVS_KEY_LEN];
    id2key(id, key);

    switch (type) {
        case DY_CFG2_VALUE_TYPE_U8:
            esp_err = nvs_set_u8(nvs_hdl, key, *(uint8_t *) src);
            break;
        case DY_CFG2_VALUE_TYPE_I8:
            esp_err = nvs_set_i8(nvs_hdl, key, *(int8_t *) src);
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

    return dy_ok();
}

dy_err_t dy_cfg2_get_u8(int id, uint8_t *dst) {
    return get(id, DY_CFG2_VALUE_TYPE_U8, dst);
}

dy_err_t dy_cfg2_set_u8(int id, uint8_t val) {
    return set(id, DY_CFG2_VALUE_TYPE_U8, &val, true);
}

dy_err_t dy_cfg2_set_u8_if_not_set(int id, uint8_t val) {
    return set(id, DY_CFG2_VALUE_TYPE_U8, &val, false);
}

dy_err_t dy_cfg2_get_i8(int id, int8_t *dst) {
    return get(id, DY_CFG2_VALUE_TYPE_I8, dst);
}

dy_err_t dy_cfg2_set_i8(int id, int8_t val) {
    return set(id, DY_CFG2_VALUE_TYPE_I8, &val, true);
}

dy_err_t dy_cfg2_get_str(int id, char *dst) {
    return get(id, DY_CFG2_VALUE_TYPE_STR, dst);
}

dy_err_t dy_cfg2_set_str(int id, const char *val) {
    return set(id, DY_CFG2_VALUE_TYPE_STR, (void *) val, true);
}

dy_err_t dy_cfg2_init() {
    esp_err_t esp_err = nvs_open(DY_CFG2_NVS_NAMESPACE, NVS_READWRITE, &nvs_hdl);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_open: %s", esp_err_to_name(esp_err));
    }

    nvs_initialized = true;

    return dy_ok();
}
