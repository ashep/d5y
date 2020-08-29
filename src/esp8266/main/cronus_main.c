#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_event.h"
#include "cronus_main.h"
#include "cronus_settings.h"
#include "cronus_net.h"
// #include "cronus_http_server.h"

void app_main()
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    cronus_settings_init();
    cronus_net_init();
    // cronus_http_server_start();

    for (;;)
    {
        printf(".");
        fflush(stdout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
