#ifndef CRONUS_FS_H
#define CRONUS_FS_H

#include "semphr.h"

#ifndef APP_FS_PARTITION_NAME
#define APP_FS_PARTITION_NAME "storage"
#endif

typedef struct {
    SemaphoreHandle_t mux;
} app_fs_t;

/**
 * @brief  Initializes filesystem
 */
esp_err_t app_fs_init();

#endif //CRONUS_FS_H
