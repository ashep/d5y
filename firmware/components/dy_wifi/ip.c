#include "esp_log.h"
#include "esp_netif_sntp.h"

#define LTAG "DY_WIFI"

void ip_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(LTAG, "IP_EVENT_STA_GOT_IP");
            break;

        case IP_EVENT_STA_LOST_IP:
            ESP_LOGI(LTAG, "IP_EVENT_STA_LOST_IP");
            break;

        default:
            ESP_LOGI(LTAG, "IP_EVENT: %ld", event_id);
            break;
    }
}