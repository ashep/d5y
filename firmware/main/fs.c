/**
 * @brief     Cronus Digital Clock SPIFFS Functions
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#include "cronus/fs.h"

#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "cronus/main.h"
#include "cronus/version.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

esp_err_t app_fs_init() {
    app_fs_t *fs = malloc(sizeof(app_fs_t));
    if (fs == NULL) {
        ESP_LOGE(APP_NAME, "failed to allocate memory for the filesystem");
        return ESP_FAIL;
    }

    fs->mux = xSemaphoreCreateBinary();
    if (fs->mux == NULL) {
        free(fs);
        ESP_LOGE(APP_NAME, "failed to allocate a semaphore for the filesystem");
        return ESP_FAIL;
    }
    xSemaphoreGive(fs->mux);

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "",
        .partition_label = APP_FS_PARTITION_NAME,
        .max_files = 5,
        .format_if_mount_failed = true,
    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err != ESP_OK) {
        free(fs);

        if (err == ESP_FAIL) {
            ESP_LOGE(APP_NAME, "failed to mount or format filesystem");
        } else if (err == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(APP_NAME, "failed to find SPIFFS partition");
        } else {
            ESP_LOGE(APP_NAME, "failed to initialize SPIFFS: %s",
                     esp_err_to_name(err));
        }

        return err;
    }

    size_t total = 0, used = 0;
    err = esp_spiffs_info(APP_FS_PARTITION_NAME, &total, &used);
    if (err != ESP_OK) {
        free(fs);
        ESP_LOGE(APP_NAME, "failed to get partition information; err=%s",
                 esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(APP_NAME, "filesystem '%s': total=%d, used=%d",
             APP_FS_PARTITION_NAME, total, used);

    return ESP_OK;
}