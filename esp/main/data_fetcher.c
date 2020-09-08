#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "http_parser.h"
#include "aespl_http_client.h"
#include "cronus_esp_main.h"

void vDataFetcherTask(void *pvParameters) {
    esp_err_t err;
    aespl_http_response resp;

    http_header_handle_t hdr = http_header_init();

    for (;;) {
        aespl_http_client_get_json(&resp, CRONUS_API_SERVER_URL, hdr);

        printf("Status: %d\n", resp.status_code);
        printf("Content-Length: %llu\n", resp.content_length);
        printf("Body: %s\n", resp.body);
        // printf("JSON second: %d\n", cJSON_GetObjectItem( resp.json, "sec")->valueint);
        fflush(stdout);

        aespl_http_client_free(&resp);

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}
