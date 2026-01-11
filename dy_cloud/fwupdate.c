#include "dy/cloud.h"

#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "cJSON.h"
#include "esp_log.h"
#include "esp_crt_bundle.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"

#include "dy/error.h"
#include "dy/net_cfg.h"
#include "dy/appinfo.h"

#define LTAG "DY_CLOUD"
#define API_URL "https://api.d5y.xyz/v3/firmware/update"
#define CHECK_PERIOD 42949 // ~11 hours, limited by max value of uint32
#define URL_MAX_LEN 512 // URL can be really long
#define HTTP_REQ_TIMEOUT 5000
#define DELAY_BEFORE_PERFORM_SEC 5

extern dy_err_t http_get_json(const char *url, cJSON **rsp_json);

static bool allow_alpha_versions = false;
static bool in_progress = false;
static QueueHandle_t mux = NULL;

typedef struct {
    char url[URL_MAX_LEN];
    size_t size;
    char sha256[65];
} dy_cloud_resp_fw_update_t;

static dy_err_t check(dy_cloud_resp_fw_update_t *res) {
    dy_err_t err;

    dy_appinfo_info_t ai;
    if (dy_is_err(err = dy_appinfo_get(&ai))) {
        return dy_err_pfx("dy_appinfo_get", err);
    }

    char url[URL_MAX_LEN] = {0};
    const int n = snprintf(url, URL_MAX_LEN, "%s?app=%s&to_alpha=%d", API_URL, ai.id, allow_alpha_versions);
    if (n < 0 || n >= URL_MAX_LEN) {
        return dy_err(DY_ERR_INVALID_ARG, "firmware update url is too long");
    }

    cJSON *json;
    err = http_get_json(url, &json);
    if (err->code == DY_ERR_NOT_FOUND) {
        return err;
    }
    if (dy_is_err(err)) {
        return dy_err_pfx("http_get_json", err);
    }

    memset(res, 0, sizeof(*res));

    cJSON *r_url = cJSON_GetObjectItem(json, "url");
    if (r_url != NULL) {
        strlcpy(res->url, cJSON_GetStringValue(r_url), URL_MAX_LEN);
    }

    cJSON *sha256 = cJSON_GetObjectItem(json, "sha256");
    if (sha256 != NULL) {
        strlcpy(res->sha256, cJSON_GetStringValue(sha256), sizeof(res->sha256));
    }

    cJSON *size = cJSON_GetObjectItem(json, "size");
    if (size != NULL) {
        res->size = (int) cJSON_GetNumberValue(size);
    }

    cJSON_Delete(json);

    return dy_ok();
}

static dy_err_t print_part_info() {
    const esp_partition_t *run_part = esp_ota_get_running_partition();
    if (run_part == NULL) {
        return dy_err(DY_ERR_FAILED, "esp_ota_get_running_partition returned null");
    }
    ESP_LOGI(LTAG, "running partition: label=%s; size=%lu", run_part->label, run_part->size);

    const esp_partition_t *boot_part = esp_ota_get_boot_partition();
    if (boot_part == NULL) {
        return dy_err(DY_ERR_FAILED, "esp_ota_get_boot_partition returned null");
    }
    ESP_LOGI(LTAG, "boot partition: label=%s; size=%lu", boot_part->label, boot_part->size);

    return dy_ok();
}

static dy_err_t perform(dy_cloud_resp_fw_update_t *info) {
    dy_err_t err;

    dy_appinfo_info_t ai;
    if (dy_is_err(err = dy_appinfo_get(&ai))) {
        return dy_err_pfx("dy_appinfo_get", err);
    }

    ESP_LOGI(LTAG, "getting partition setup before update");
    if (dy_is_err(err = print_part_info())) {
        ESP_LOGW(LTAG, "print_part_info: %s", dy_err_str(err));
    }

    ESP_LOGI(LTAG, "starting firmware update");

    esp_err_t esp_err = esp_event_post(
        DY_CLOUD_EV_BASE, DY_CLOUD_EV_FW_UPDATE_STARTED, info, sizeof(dy_cloud_resp_fw_update_t), 10);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_post: %s", esp_err_to_name(esp_err));
    }

    if (xSemaphoreTake(mux, 100) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "xSemaphoreTake failed");
    }
    in_progress = true;
    xSemaphoreGive(mux);

    // Give some time for the event to be processed by other components
    ESP_LOGI(LTAG, "waiting %d seconds before performing firmware update", DELAY_BEFORE_PERFORM_SEC);
    vTaskDelay(pdMS_TO_TICKS(1000 * DELAY_BEFORE_PERFORM_SEC));

    esp_http_client_config_t http_cfg = {
            .user_agent = ai.id,
            .method = HTTP_METHOD_GET,
            .url = info->url,
            .timeout_ms = HTTP_REQ_TIMEOUT,
            .keep_alive_enable = false,
            .crt_bundle_attach = esp_crt_bundle_attach,
            .buffer_size_tx = 2048,
    };

    esp_https_ota_config_t ota_cfg = {
            .http_config = &http_cfg,
            .partial_http_download = true,
    };

    esp_err = esp_https_ota(&ota_cfg);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_https_ota: %s", esp_err_to_name(esp_err));
    }

    ESP_LOGI(LTAG, "getting partition setup after update");
    if (dy_is_err(err = print_part_info())) {
        ESP_LOGW(LTAG, "print_part_info: %s", dy_err_str(err));
    }

    ESP_LOGI(LTAG, "the firmware has been updated successfully");

    return dy_ok();
}

_Noreturn static void task() {
    dy_cloud_resp_fw_update_t upd;

    while (1) {
        if (!dy_net_cfg_net_connected()) {
            vTaskDelay(pdMS_TO_TICKS(1000 * 10));
            continue;
        }

        dy_err_t err = check(&upd);

        if (err->code == DY_ERR_NOT_FOUND) {
            ESP_LOGI(LTAG, "no firmware update found");
            vTaskDelay(pdMS_TO_TICKS(1000 * CHECK_PERIOD));
            continue;
        }
        if (dy_is_err(err)) {
            ESP_LOGE(LTAG, "firmware update check: %s", dy_err_str(err));
            vTaskDelay(pdMS_TO_TICKS(1000 * 10));
            continue;
        }

        ESP_LOGI(LTAG, "firmware update found: url=%s; size=%d; sha256=%s", upd.url, upd.size, upd.sha256);

        if (dy_is_err(err = perform(&upd))) {
            ESP_LOGE(LTAG, "firmware update failed: %s", dy_err_str(err));
            continue;
        }

        ESP_LOGI(LTAG, "update succeeded");
        ESP_LOGI(LTAG, "restarting...");

        esp_restart();
    }
}

bool dy_cloud_fwupdate_in_progress() {
    bool res = false;
    if (xSemaphoreTake(mux, portMAX_DELAY) == pdTRUE) {
        res = in_progress;
        xSemaphoreGive(mux);
    }
    return res;
}

dy_err_t dy_cloud_fwupdate_scheduler_start(bool allow_alpha) {
    allow_alpha_versions = allow_alpha;

    mux = xSemaphoreCreateMutex();
    if (mux == NULL) {
        return dy_err(DY_ERR_FAILED, "xSemaphoreCreateMutex returned null");
    }

    if (xTaskCreate(task, "dy_cloud_fw_upd", 8192, NULL, tskIDLE_PRIORITY, NULL) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "task create failed");
    }

    return dy_ok();
}