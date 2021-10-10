#ifndef CRONUS_FS_H
#define CRONUS_FS_H

#include "semphr.h"

typedef struct {
    SemaphoreHandle_t mux;
} app_fs_t;

/**
 * @brief  Initializes filesystem
 *
 * @return
 *      - pointer to FS struct;
 *      - NULL in case of error.
 */
app_fs_t *app_fs_init();

#endif //CRONUS_FS_H
