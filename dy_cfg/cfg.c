#include <string.h>

#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "nvs.h"

#include "dy/error.h"
#include "dy/appinfo.h"

#ifdef CONFIG_BT_ENABLED
#include "dy/bt.h"
#endif

#include "dy/cfg.h"

#define LTAG "DY_CFG"

static SemaphoreHandle_t mux = NULL;
static nvs_handle_t nvs_hdl;
static uint8_t cfg_buf[256];

static dy_err_t load() {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "xSemaphoreTake failed");
    }

    size_t len = sizeof(cfg_buf);
    esp_err_t err = nvs_get_blob(nvs_hdl, "config", cfg_buf, &len);

    if (err != ESP_OK) {
        xSemaphoreGive(mux);

        if (err == ESP_ERR_NVS_NOT_FOUND) {
            return dy_err(DY_ERR_NOT_FOUND, "nvs entry not found");
        }

        return dy_err(DY_ERR_FAILED, "nvs_get_blob failed: %s", esp_err_to_name(err));
    }

    xSemaphoreGive(mux);

    return dy_ok();
}

static dy_err_t save() {
    dy_err_t err;
    esp_err_t esp_err;

    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "xSemaphoreTake failed");
    }

    dy_appinfo_info_t ai;
    if (dy_is_err(err = dy_appinfo_get(&ai))) {
        return dy_err_pfx("dy_appinfo_get", err);
    }

    cfg_buf[DY_CFG_ID_APP_VER_MAJOR] = ai.ver.major;
    cfg_buf[DY_CFG_ID_APP_VER_MINOR] = ai.ver.minor;
    cfg_buf[DY_CFG_ID_APP_VER_PATCH] = ai.ver.patch;
    cfg_buf[DY_CFG_ID_APP_VER_ALPHA] = ai.ver.alpha;

    esp_err = nvs_set_blob(nvs_hdl, "config", cfg_buf, sizeof(cfg_buf));

    xSemaphoreGive(mux);

    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_set_blob failed: %s", esp_err_to_name(esp_err));
    }

    ESP_LOGI(LTAG, "config saved");

    return dy_ok();
}

static dy_err_t on_bt_chrc_read(uint8_t *val, size_t *len) {
    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "xSemaphoreTake failed");
    }

    if (sizeof(cfg_buf) > *len) {
        xSemaphoreGive(mux);
        return dy_err(DY_ERR_INVALID_SIZE, "buffer too small: %zu < %zu", *len, sizeof(cfg_buf));
    }

    memcpy(val, cfg_buf, sizeof(cfg_buf));
    *len = sizeof(cfg_buf);

    xSemaphoreGive(mux);

    return dy_ok();
}

static dy_err_t on_bt_chrc_write(const uint8_t *val, size_t len) {
    dy_err_t err;

    if (len != 2) {
        return dy_err(DY_ERR_INVALID_ARG, "unexpected input length: %d", len);
    }

    if (dy_is_err(err = dy_cfg_set(val[0], val[1]))) {
        return dy_err_pfx("dy_cfg_set", err);
    }

    if (dy_is_err(err = save())) {
        return dy_err_pfx("save", err);
    }

    return dy_ok();
}

void dy_cfg_must_set_initial(uint8_t id, uint8_t val) {
    if (id < DY_CFG_ID_MIN) {
        ESP_LOGE(LTAG, "%s: reserved id: %d", __func__, id);
        abort();
    }

    if (mux != NULL) {
        ESP_LOGE(LTAG, "%s: must be called before dy_cfg_init", __func__);
        abort();
    }

    cfg_buf[id] = val;
}

dy_err_t dy_cfg_set(uint8_t id, uint8_t val) {
    if (id < DY_CFG_ID_MIN) {
        return dy_err(DY_ERR_INVALID_ARG, "reserved id: %d", id);
    }

    if (mux == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "%s must not be called before dy_cfg_init", __func__);
    }

    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "%s: xSemaphoreTake failed", __func__);
    }

    cfg_buf[id] = val;

    xSemaphoreGive(mux);

    return dy_ok();
}

dy_err_t dy_cfg_get_p(uint8_t id, uint8_t *val) {
    if (mux == NULL) {
        *val = cfg_buf[id];
        return dy_ok();
    }

    if (xSemaphoreTake(mux, portTICK_PERIOD_MS) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "%s: xSemaphoreTake failed", __func__);
    }

    *val = cfg_buf[id];

    xSemaphoreGive(mux);

    return dy_ok();
}

uint8_t dy_cfg_get(uint8_t id, uint8_t def) {
    uint8_t r;

    dy_err_t err = dy_cfg_get_p(id, &r);
    if (dy_is_err(err)) {
        ESP_LOGE(LTAG, "dy_cfg_get_p(%d): %s", id, dy_err_str(err));
        return def;
    }

    return r;
}

#ifdef CONFIG_BT_ENABLED // FIXME: decouple from BT
dy_err_t dy_cfg_init() {
    dy_err_t err;
    esp_err_t esp_err;

    mux = xSemaphoreCreateMutex();
    if (mux == NULL) {
        return dy_err(DY_ERR_NO_MEM, "xSemaphoreCreateMutex returned null");
    }

    esp_err = nvs_open("config", NVS_READWRITE, &nvs_hdl);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "nvs_open failed: %s", esp_err_to_name(esp_err));
    }

    err = load();
    if (err->code == DY_ERR_NOT_FOUND) {
        err = save(); // write current values that was presumably set before via dy_cfg_must_set_initial()
        if (dy_is_err(err)) {
            return dy_err_pfx("initial data save failed", err);
        }
    } else if (dy_is_err(err)) {
        return dy_err_pfx("initial data load failed", err);
    }

    err = dy_bt_register_characteristic(0xff02, on_bt_chrc_read, on_bt_chrc_write);
    if (dy_is_err(err)) {
        return dy_err_pfx("dy_bt_register_characteristic", err);
    }

    return dy_ok();
}
#endif