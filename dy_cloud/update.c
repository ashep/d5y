#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "esp_crt_bundle.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"

#include "dy/error.h"
#include "dy/net_cfg.h"
#include "dy/appinfo.h"

#include "dy/_cloud.h"

static bool allow_alpha_versions = false;

static dy_err_t check(dy_cloud_resp_fw_update_t *res) {
    dy_err_t err;

    dy_appinfo_info_t ai;
    if (dy_is_err(err = dy_appinfo_get(&ai))) {
        return dy_err_pfx("dy_appinfo_get", err);
    }


    char *req_url = malloc(URL_LEN + 1);
    if (!req_url) {
        return dy_err(DY_ERR_NO_MEM, "request url buffer allocation failed");
    }

    memset(req_url, 0, URL_LEN + 1);
    snprintf(req_url, URL_LEN, "%s?app=%s&to_alpha=%d", API_URL_FW_UPDATE, ai.id, allow_alpha_versions);

    cJSON *json;
    memset(res, 0, sizeof(*res));

    err = http_get_json(req_url, &json);
    free(req_url);

    if (err->code == DY_ERR_NOT_FOUND) {
        return err;
    } else if (dy_is_err(err)) {
        return dy_err_pfx("http_get_json", err);
    }

    cJSON *r_url = cJSON_GetObjectItem(json, "url");
    if (r_url != NULL) {
        strncpy(res->url, cJSON_GetStringValue(r_url), URL_LEN - 1);
    }

    cJSON *sha256 = cJSON_GetObjectItem(json, "sha256");
    if (sha256 != NULL) {
        strncpy(res->sha256, cJSON_GetStringValue(sha256), 64);
    }

    cJSON *size = cJSON_GetObjectItem(json, "size");
    if (size != NULL) {
        res->size = (int) cJSON_GetNumberValue(size);
    }

    cJSON_free(json);

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

static dy_err_t perform(dy_cloud_resp_fw_update_t *res) {
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

    esp_http_client_config_t http_cfg = {
        .user_agent = ai.id,
        .method = HTTP_METHOD_GET,
        .url = res->url,
        .timeout_ms = HTTP_REQ_TIMEOUT,
        .keep_alive_enable = false,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .buffer_size_tx = 2048,
    };

    esp_https_ota_config_t ota_cfg = {
        .http_config = &http_cfg,
        .partial_http_download = true,
    };

    esp_err_t esp_err = esp_https_ota(&ota_cfg);
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
    dy_err_t err;
    dy_cloud_resp_fw_update_t upd;

    while (1) {
        if (!dy_net_cfg_net_ready()) {
            vTaskDelay(pdMS_TO_TICKS(1000 * 10));
            continue;
        }

        err = check(&upd);

        if (err->code == DY_ERR_NOT_FOUND) {
            ESP_LOGI(LTAG, "no firmware update found");
            vTaskDelay(pdMS_TO_TICKS(1000 * UPDATE_CHECK_PERIOD));
            continue;
        } else if (dy_is_err(err)) {
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

dy_err_t dy_cloud_update_start_scheduler(bool allow_alpha) {
    allow_alpha_versions = allow_alpha;

    if (xTaskCreate(task, "dy_cloud_fw_upd", 8192, NULL, tskIDLE_PRIORITY, NULL) != pdTRUE) {
        return dy_err(DY_ERR_FAILED, "task create failed");
    }

    return dy_ok();
}